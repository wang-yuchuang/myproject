
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLI_type.h"
#include "board.h"
#include "regs_base.h"

#ifdef CONFIG_DISPSS
#include "driver/taishan/dispss/disp.h"
#include "driver/taishan/dispss/disp_data_type.h"
#endif

#ifdef CONFIG_ASW
#include "disp_demo_api.h"
#endif

#include "emirror_lvgl.h"

#define ASW_DWRAP_SWITCH 1

#define tester_err(x...) ssdk_printf(SSDK_ERR, x)
#define tester_warn(x...) ssdk_printf(SSDK_WARNING, x)
#define tester_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define tester_debug(x...) ssdk_printf(SSDK_DEBUG, x)


#include "cam_hal.h"

#define CAM_PAGE_SIZE (1 << 12)
#define CAM_PAGE_MASK (~(CAM_PAGE_SIZE - 1))

#define CAMERA_TIMEOUT_MS 500
#define CAM_BUF_CNT 4

#define USE_IRAM 1

/* mem_size = frame_size * CIRCLE_MEM_FACTOR / CIRCLE_MEM_BASE */
#define USE_CIRCLE_MEM
#define CIRCLE_MEM_FACTOR 1000
#define CIRCLE_MEM_BASE 1000

#define LCM_WIDTH 1280
#define LCM_HEIGHT 768

struct cam_crop_info {
    int crop_update;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};

typedef struct cam_test_param_t {
    int cam_id;
    int display;
    int width;
    int height;
    int dis_width;
    int dis_height;
    int total_frame;
    int dump_frame_total;
    int dump_frame_interval;
    int cam_fmt;
    int dis_fmt;
    char video_name[128];
    char pic_file_name[256];
} cam_test_param;

struct cam_mem_info {
    uint32_t planes;
    uint32_t pix_bytes;
    uint32_t w;
    uint32_t h;

    int circle_mem;
    uint32_t mem_size[3];
    void *mem_ptr[3];

    uint32_t stride[3];
    uint32_t buf_size[3];
    void *buffers[CAM_BUF_CNT][3];
};

#ifdef USE_IRAM
static uint8_t emirror_reserved_buf[2*1280*720] __SECTION(".apdata");
uint8_t *emirror_data_y = &emirror_reserved_buf[0];
uint8_t *emirror_data_uv = &emirror_reserved_buf[1280*720];
#endif

#ifdef CONFIG_DISPSS

extern struct dc_dev g_dc_dev;

static void disp_post_init(struct post_cfg *post, struct surface *layer,
                           uint32_t width, uint32_t height, uint32_t x_off, uint32_t y_off, int fmt,
                           void *addr_y, void *addr_u, void *addr_v,
                           int stride_y, int stride_u, int stride_v)
{
    memset(post, 0, sizeof(struct post_cfg));
    memset(layer, 0, sizeof(struct surface));

    layer->id = 1;
    layer->dirty = 1;
    layer->en = 1;
    layer->fmt = fmt;

    layer->src.x = 0;
    layer->src.y = 0;
    layer->src.w = width;
    layer->src.h = height;

    layer->start.x = 0;
    layer->start.y = 0;
    layer->start.w = width;
    layer->start.h = height;

    layer->addr[0] = (unsigned long)addr_y;
    layer->src_stride[0] = stride_y;

    layer->addr[1] = (unsigned long)addr_u;
    layer->src_stride[1] = stride_u;

    layer->addr[2] = (unsigned long)addr_v;
    layer->src_stride[2] = stride_v;

    layer->dst.x = x_off;
    layer->dst.y = y_off;
    layer->dst.w = width;
    layer->dst.h = height;

    layer->z_order = 0;
    layer->alpha_en = 1;
    layer->alpha = 0xff;
    layer->ckey_en = 0;

    post->num_layers = 1;
    post->layers = layer;
}

static int to_disp_fmt(uint32_t cam_pixfmt)
{
    switch (cam_pixfmt) {
    case VDEV_PIX_FMT_UYVY:
        return COLOR_YUVY;

    case VDEV_PIX_FMT_YUYV:
        return COLOR_YUYV;

    case VDEV_PIX_FMT_NV16:
        return COLOR_NV16;

    case VDEV_PIX_FMT_NV61:
        return COLOR_NV61;

    case VDEV_PIX_FMT_YUV422P:
        return COLOR_YUV422P;

    case VDEV_PIX_FMT_RGB24:
        return COLOR_BGR888;

    case VDEV_PIX_FMT_BGR24:
        return COLOR_RGB888;

    case VDEV_PIX_FMT_RGB16:
        return COLOR_RGB565;

    default:
        return COLOR_YUYV;
    }
    return COLOR_YUYV;
}

#endif

static void cam_buffer_free(struct cam_mem_info *pmem)
{
    uint32_t i;
    for (i = 0; i < 3; i++) {
        if (pmem->mem_ptr[i]) {
#ifndef USE_IRAM
            osFree(pmem->mem_ptr[i]);
#endif
            pmem->mem_ptr[i] = NULL;
        }
    }
}

static int cam_buffer_alloc(struct cam_mem_info *pmem, uint32_t pixfmt)
{
    uint32_t i, n;
    uint32_t planes;
    uint32_t pix_bytes;
    void *base;

#ifdef USE_CIRCLE_MEM
    pmem->circle_mem = 1;
#else
    pmem->circle_mem = 0;
#endif

    switch (pixfmt) {
    case VDEV_PIX_FMT_UYVY:
    case VDEV_PIX_FMT_YUYV:
    case VDEV_PIX_FMT_RGB16:
        planes = 1;
        pix_bytes = 2;
        pmem->stride[0] = pmem->w * 2;
        pmem->buf_size[0] = pmem->w * pmem->h * 2;
        break;
    case VDEV_PIX_FMT_NV16:
    case VDEV_PIX_FMT_NV61:
        planes = 2;
        pix_bytes = 2;
        pmem->stride[0] = pmem->w;
        pmem->stride[1] = pmem->w;
        pmem->buf_size[0] = pmem->w * pmem->h;
        pmem->buf_size[1] = pmem->w * pmem->h;
        break;
    case VDEV_PIX_FMT_YUV422P:
        planes = 3;
        pix_bytes = 2;
        pmem->stride[0] = pmem->w;
        pmem->stride[1] = pmem->w >> 1;
        pmem->stride[2] = pmem->w >> 1;
        pmem->buf_size[0] = pmem->w * pmem->h;
        pmem->buf_size[1] = pmem->w * pmem->h >> 1;
        pmem->buf_size[2] = pmem->w * pmem->h >> 1;
        break;
    case VDEV_PIX_FMT_RGB24:
    case VDEV_PIX_FMT_BGR24:
    default:
        planes = 1;
        pix_bytes = 3;
        pmem->stride[0] = pmem->w * 3;
        pmem->buf_size[0] = pmem->w * pmem->h * 3;
        break;
    }

    tester_info("pixfmt %d, planes %d, pix_bytes %d, size 0x%x 0x%x 0x%x\n",
                pixfmt, planes, pix_bytes, pmem->buf_size[0], pmem->buf_size[1],
                pmem->buf_size[2]);

    pmem->planes = planes;
    pmem->pix_bytes = pix_bytes;
    if (pmem->circle_mem == 0) {
        goto normal_mem;
    }

    /* circle mem */
#ifndef USE_IRAM
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] =
            pmem->buf_size[i] * CIRCLE_MEM_FACTOR / CIRCLE_MEM_BASE;
        pmem->mem_size[i] += CAM_PAGE_SIZE - 1;
        pmem->mem_size[i] &= CAM_PAGE_MASK;
        pmem->mem_ptr[i] = osAllocAlign(CAM_PAGE_SIZE, pmem->mem_size[i]);
        base = pmem->mem_ptr[i];
        for (n = 0; n < CAM_BUF_CNT; n++) {
            pmem->buffers[n][i] = base;
            tester_info("plane %d, buffer Idx.%d, circle ptr 0x%08x\n", i, n,
                        (uint32_t)base);
            if (pmem->mem_size[i] > (pmem->buf_size[i] + CAM_PAGE_SIZE)) {
                base = (uint8_t *)base + pmem->buf_size[i];
                while ((uint8_t *)base >= (uint8_t *)pmem->mem_ptr[i] + pmem->mem_size[i])
                    base = (uint8_t *)base - pmem->mem_size[i];
            }
        }
    }
#else
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] = sizeof(emirror_reserved_buf) / planes;
        pmem->mem_ptr[i] = (i == 0) ? emirror_data_y : emirror_data_uv;
        for (n = 0; n < CAM_BUF_CNT; n++) {
            pmem->buffers[n][i] = pmem->mem_ptr[i];
            tester_info("plane %d, buffer Idx.%d, iram ptr 0x%08x\n", i, n,
                        (uint32_t)pmem->buffers[n][i]);
        }
    }
#endif

    return 0;

normal_mem:
    /* normal mem */
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] = pmem->buf_size[i] * CAM_BUF_CNT;
        pmem->mem_ptr[i] = osAllocAlign(CAM_PAGE_SIZE, pmem->mem_size[i]);
        for (n = 0; n < CAM_BUF_CNT; n++) {
            base = (uint8_t *)pmem->mem_ptr[i] + n * pmem->buf_size[i];
            pmem->buffers[n][i] = base;
            tester_info("plane %d, buffer Idx.%d, ptr 0x%08x\n", i, n,
                        (uint32_t)base);
        }
    }
    return 0;
}

static void parse_options(int argc, char *argv[], cam_test_param *param)
{
    int i;

    memset(param, 0, sizeof(cam_test_param));
    param->cam_id = 0;
    param->display = -1;
    param->width = -1;
    param->height = -1;
    param->dis_width = -1;
    param->dis_height = -1;
    param->total_frame = 0x7fffffff;
    param->dump_frame_total = -1;
    param->dump_frame_interval = -1;
    param->cam_fmt = -1;
    param->dis_fmt = -1;

    for (i = 0; i < argc; i++) {
        if (argv[i][0] != '-')
            continue;

        switch (argv[i][1]) {
        case 'i':
            i++;
            if (i < argc)
                param->cam_id = atoi(argv[i]);
            tester_info("\nget cam_id %d\n", param->cam_id);
            break;

        case 's':
            i++;
            if (i < argc)
                param->width = atoi(argv[i]);
            i++;
            if (i < argc)
                param->height = atoi(argv[i]);
            tester_info("\nget cam size %d %d\n", param->width, param->height);
            break;

        case 'y':
            i++;
            if (i < argc)
                param->display = atoi(argv[i]);
            tester_info("\nget display_id %d\n", param->display);
            break;

        case 'd':
            i++;
            if (i < argc)
                param->dis_width = atoi(argv[i]);
            i++;
            if (i < argc)
                param->dis_height = atoi(argv[i]);
            tester_info("\nget display size %d %d\n", param->dis_width,
                        param->dis_height);
            break;

        case 'n':
            i++;
            if (i < argc)
                param->total_frame = atoi(argv[i]);
            tester_info("\nget total_frame %d\n", param->total_frame);
            break;

        case 'p':
            i++;
            if (i < argc)
                param->cam_fmt = atoi(argv[i]);
            tester_info("\nget cam_fmt %d\n", param->cam_fmt);
            break;

        case 'h':
            tester_info("\nHelp info already displayed\n");
            break;

        default:
            tester_err("\nunknown options: %s\n", argv[i]);
            break;
        }
    }
}

static int lvgl_trigger;
static osThreadId_t lvgl_thread;
static void lvgl_thread_func(void *data)
{
    tester_info("running lvgl_thread_func\n");
    lvgl_trigger = 1;
    lvglkickmainloop();
    osThreadExit();
}

static int osEvent = 1;
static cam_test_param test_param;
int emirror_main(void *data)
{
    int ret = 0;
    cam_test_param *param = (cam_test_param *)data;
    int cam_id = 0, display = -1, frm_cnt = 0, total_frame = 1000;
    int endless = 0;
    uint32_t i, timeout, dqbuf = 0;
    uint32_t width, height, pixfmt, fps;
    uint32_t dis_w, dis_h, x_off, y_off;
    uint32_t x[2], y[2], w[2], h[2];
    struct cam_crop_info crop;
    struct cam_mem_info mem_info;
    struct vdev_buffer cam_buf;
    camera_handle *cam;
    lv_obj_t *scr;
    lv_obj_rvm emirror_btn[4];
#ifdef CONFIG_DISPSS
    struct post_cfg post;
    struct surface layer;
    int disp_fmt, offset;
#endif

    tester_info("emirror_main  enter\n");
    if (data == NULL) {
        param = &test_param;

        memset(param, 0, sizeof(cam_test_param));
        param->cam_id = 0;
        param->display = 1;
        param->width = -1;
        param->height = -1;
        param->dis_width = -1;
        param->dis_height = -1;
        param->total_frame = 0x7fffffff;
        param->dump_frame_total = -1;
        param->dump_frame_interval = -1;
        param->cam_fmt = -1;
        param->dis_fmt = -1;
        endless = 1;
    }

    cam_id = (param->cam_id >= 0) ? param->cam_id : 0;
    display = (param->display >= 0) ? param->display : -1;
    total_frame = param->total_frame;

    cam = hal_cam_get_instance(cam_id);
    if (cam == NULL) {
        tester_err("get cam %d failed \n", cam_id);
        return -1;
    }

    hal_cam_init(cam);

    hal_cam_get_fps(cam, &fps);
    tester_info("camera fps %d\n", fps);

    hal_cam_get_size(cam, &width, &height);
    tester_info("cam size [%d %d]\n", width, height);

    if (param->cam_fmt < 0) {
        uint32_t fmt_list[MAX_PIXFMT_COUNT];
        ret = hal_cam_enum_format(cam, fmt_list);
        pixfmt = fmt_list[0];
        if (display == 1)
            pixfmt = VDEV_PIX_FMT_NV16;
        ret = hal_cam_set_format(cam, pixfmt);
    } else {
        pixfmt = param->cam_fmt;
        ret = hal_cam_set_format(cam, pixfmt);
    }

    if (ret) {
        tester_err("cam %d set fmt failed\n", cam_id);
        hal_cam_deinit(cam);
        hal_cam_put_instance(cam);
        return -1;
    }

    memset(&crop, 0, sizeof(struct cam_crop_info));

    set_full_size(width, height);
    get_cur_roi(&crop.crop_update, &crop.x, &crop.y, &crop.w, &crop.h);

    if (crop.crop_update) {
        x[0] = x[1] = crop.x;
        y[0] = y[1] = crop.y;
        w[0] = w[1] = crop.w;
        h[0] = h[1] = crop.h;
        if ((x[0] + w[0]) > width)
            w[0] = width - x[0];
        if ((y[0] + h[0]) > height)
            h[0] = height - y[0];
        hal_cam_set_crop(cam, x, y, w, h);
    } else {
        x[0] = x[1] = 0;
        y[0] = y[1] = 0;
        w[0] = w[1] = width;
        h[0] = h[1] = height;
    }

    tester_info("cam size [%d %d], crop %d %d %d %d, pixfmt %d\n",
        width, height, x[0], y[0], w[0], h[0], pixfmt);

    memset(&mem_info, 0, sizeof(struct cam_mem_info));
    mem_info.w = w[0];
    mem_info.h = h[0];
    cam_buffer_alloc(&mem_info, pixfmt);

    memset(&cam_buf, 0, sizeof(struct vdev_buffer));
    for (i = 0; i < CAM_BUF_CNT; i++) {
        cam_buf.buffer_idx = i;
        cam_buf.paddr[0] = mem_info.buffers[i][0];
        cam_buf.paddr[1] = mem_info.buffers[i][1];
        cam_buf.paddr[2] = mem_info.buffers[i][2];
        hal_cam_qbuf(cam, &cam_buf);
        tester_info("camera qbuf done. index %d, addr %p\n", i,
                    cam_buf.paddr[0]);
    }

    if (mem_info.circle_mem) {
        uint32_t low[3], up[3];
        tester_info("camera will use circle memory\n");
        for (i = 0; i < mem_info.planes; i++) {
            low[i] = (uint32_t)mem_info.mem_ptr[i];
            up[i] = (uint32_t)mem_info.mem_ptr[i] + mem_info.mem_size[i];
        }
        hal_cam_set_memrange(cam, low, up);
    }

    dis_w = (w[0] > LCM_WIDTH) ? LCM_WIDTH : w[0];
    dis_h = (h[0] > LCM_HEIGHT) ? LCM_HEIGHT : h[0];
    x_off = (LCM_WIDTH - dis_w) >> 1;
    y_off = (LCM_HEIGHT - dis_h) >> 1;
    tester_info(
        "display id %d, display size [%d %d], off [%d %d], cam_crop[%d, %d, %d, %d]\n",
        display, dis_w, dis_h, x_off, y_off, x[0], y[0], w[0], h[0]);

    ret = hal_cam_start(cam);
    if (ret) {
        tester_err("cam start failed\n");
        goto error;
    }
    tester_info("camera start\n");

    /* display & lvgl data */
    lvgl_init();
    tester_info("lvgl_init done\n");

    scr = lv_disp_get_scr_act(get_display(0));
    lv_obj_set_style_local_bg_opa(scr, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
                                  LV_OPA_TRANSP);

    tester_info("create_buttun\n");

#ifdef ASW_DWRAP_SWITCH
    create_distortion_switch(emirror_btn, 1, 0, 64, 64, 0, 0);
#else
    create_rvm_switch(emirror_btn, 4, 0, 80, 80, 0, 0);
#endif
    osThreadAttr_t attr = {
        .name = "emirror", .stack_size = 8192, .priority = osPriorityNormal};
    tester_info("start lvgl_thread_func\n");

    lvgl_thread = osThreadNew(lvgl_thread_func, NULL, &attr);
    if (display != 1)
        goto loop_frm;

#ifdef CONFIG_ASW
    osDelay(60);
    aswdev.app.id = ASW_APP_EMIRROR;

    struct csi_hsk_info info;
    info.src_ry_ba = (int)cam_buf.paddr[0];
    info.src_gu_ba = (int)cam_buf.paddr[1];
    info.src_bv_ba = (int)cam_buf.paddr[2];
    info.format = FMT_YUV422_SEMI_U0V0;
    info.stride_ry = w[0];
    info.stride_gu = w[1];
    tester_info("stride: ry = 0x%x, gu = 0x%x\n", w[0], w[1]);

    printf("asw cfg.....\n");
    asw_case_csi_hsdk(0, &info); // bilinear mode
    // writel(0x964, 0x1030008);

    osDelay(120);
    g_dc_dev.app.id = ASW_APP_EMIRROR;
    dc_wait_csi_irq(&g_dc_dev);

    set_off_swap(&aswdev.app.swap);

    tester_info("asw/dc wait irq done\n");
#endif

loop_frm:
    if (CAM_BUF_CNT <= 2)
        osEvent = 0;
    while (endless || (frm_cnt < total_frame)) {
        i = 0;
        dqbuf = 0;
        do {
            cam_buf.timeout_ms = CAMERA_TIMEOUT_MS;
            if (osEvent) {
                ret = hal_cam_dqbuf(cam, &cam_buf);
                dqbuf = 1;
            } else {
                int delay = (frm_cnt < 3) ? 30 : 100;
                ret = 0;
                osDelay(delay);
            }
            tester_debug("cam_test dqbuf, ret %d\n", ret);
            if (ret == 0) {
                timeout = 0;
                break;
            }
            timeout = 1;
        } while (i++ < 50);

        if (timeout) {
            tester_err("cam_test： wait timeout for times %d\n", i);
            break;
        }
        frm_cnt++;

        if ((frm_cnt & 0xFF) == 1)
            tester_info(
                "cam_test get frm.%d, buf_idx %d, addr 0x%08x, drv frm_cnt %d, "
                "timestamp 0x%08x\n",
                frm_cnt, cam_buf.buffer_idx, (uint32_t)cam_buf.paddr[0],
                cam_buf.frm_cnt, cam_buf.timestamp);

        /* for DEBUG */
        for (i = 0; i < mem_info.planes; i++) {
            uint32_t *pval = (uint32_t *)cam_buf.paddr[i];
            tester_debug("plane%d, addr:0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n",
                         i, (uint32_t)cam_buf.paddr[i], pval[0], pval[1],
                         pval[2], pval[3]);
        }

        get_cur_roi(&crop.crop_update, &crop.x, &crop.y, &crop.w, &crop.h);
        if (crop.crop_update) {
            x[0] = x[1] = crop.x;
            y[0] = y[1] = crop.y;
            w[0] = w[1] = crop.w;
            h[0] = h[1] = crop.h;
            if ((x[0] + w[0]) > width)
                w[0] = width - x[0];
            if ((y[0] + h[0]) > height)
                h[0] = height - y[0];
            hal_cam_set_crop(cam, x, y, w, h);
            tester_info("update crop %d %d %d %d\n", x[0], y[0], w[0], h[0]);
        }

        if (display != 0) {
            if (dqbuf) {
                hal_cam_qbuf(cam, &cam_buf);
                tester_debug("cam_test qbuf done. index %d, addr %p\n",
                             cam_buf.buffer_idx, cam_buf.paddr[0]);
            }
            continue;
        }

#ifdef CONFIG_DISPSS
        disp_fmt = to_disp_fmt(pixfmt);
        offset = 0;
        if (mem_info.planes == 1) {
            offset =
                (h[0] > dis_h) ? (h[0] - dis_h) / 2 * mem_info.stride[0] : 0;
            offset +=
                (w[0] > dis_w) ? (w[0] - dis_w) / 2 * mem_info.pix_bytes : 0;
        }

        disp_post_init(&post, &layer, dis_w, dis_h, x_off, y_off, disp_fmt,
                       (uint8_t *)cam_buf.paddr[0] + offset, cam_buf.paddr[1],
                       cam_buf.paddr[2], mem_info.stride[0], mem_info.stride[1],
                       mem_info.stride[2]);

        if ((frm_cnt & 0xFF) == 1)
            tester_info(
                "disp_cfg: [%d, %d], stride %d, addr 0x%lx, offset %d\n",
                layer.src.w, layer.src.h, layer.src_stride[0], layer.addr[0],
                offset);

        sdrv_display_post(&g_dc_dev, &post);
        tester_debug("sdrv_display_post done\n");

        if (display < 1 && dqbuf) {
            hal_cam_qbuf(cam, &cam_buf);
            if (0 && (frm_cnt & 0xFF) == 10)
                tester_info("cam_test qbuf done. index %d, addr %p\n",
                            cam_buf.buffer_idx, cam_buf.paddr[0]);
        }
#endif
    }

    tester_info("emirror demo stop\n");
    hal_cam_stop(cam);

error:
    cam_buffer_free(&mem_info);
    hal_cam_deinit(cam);
    hal_cam_put_instance(cam);

    if (lvgl_trigger)
        stop_lvgl();
    lvgl_trigger = 0;

    tester_info("emirror demo exit\n\n");
    return 0;
}

void emirror_run(void *data)
{
    emirror_main(data);
}

static osThreadId_t cam_thread;

void emirror_thread_func(void *data)
{
    emirror_main(data);
    osThreadExit();
}

static int emirror_app(int argc, char *argv[])
{
    cam_test_param *param0;
    osThreadAttr_t attr = {
        .priority = osPriorityRealtime,
    };

    tester_warn("usage0:\n");
    tester_warn("cam_test -h\n");
    tester_warn("  -h just output this usage information\n\n");
    tester_warn("usage1:\n");
    tester_warn("cam_test -[i|s|y|d|n|p]\n");
    tester_warn("  -i cam_id\n");
    tester_warn("  -s cam_w cam_h\n");
    tester_warn("  -y display_id\n");
    tester_warn("  -d display_w display_h\n");
    tester_warn("  -n total_frame_num\n");
    tester_warn("  -p cam_fmt\n");

    if (argc == 1 && !strcmp(argv[0], "-h")) {
        tester_warn("csi-test exit for help massage\n");
        return 0;
    }

    param0 = &test_param;
    parse_options(argc, argv, param0);

    cam_thread = osThreadNew(emirror_thread_func, (void *)param0, &attr);

    return CLI_FALSE;
}

CLI_CMD("emirror_test", "\r\nemirror_test:\r\n Start emirror app\r\n", emirror_app);
