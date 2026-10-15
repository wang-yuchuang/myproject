/**
 * @file cam_test.c
 * @brief camera test program (call cam cal api)
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#include <cmsis_os2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/memmap.h>

#include "CLI_type.h"
#include "regs_base.h"


#ifdef CONFIG_DISPSS
#include "sdm_display.h"
#include "disp_data_type.h"
#endif

#define tester_err(x...) ssdk_printf(SSDK_ERR, x)
#define tester_warn(x...) ssdk_printf(SSDK_WARNING, x)
#define tester_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define tester_debug(x...) ssdk_printf(SSDK_DEBUG, x)


#include "cam_hal.h"

#define CAM_PAGE_SIZE (1 << 12)
#define CAM_PAGE_MASK (~(CAM_PAGE_SIZE - 1))

#define CAMERA_TIMEOUT_MS 5000
#define DISPLAY_NUM 1
#define CAM_BUF_CNT 3

#define LCM_WIDTH 1920
#define LCM_HEIGHT 720


struct cam_mem_info {
    uint32_t planes;
    uint32_t pix_bytes;
    uint32_t w;
    uint32_t h;

    uint32_t mem_size[3];
    void *mem_ptr[3];

    uint32_t stride[3];
    uint32_t buf_size[3];
    void *buffers[CAM_BUF_CNT][3];
};

typedef struct cam_test_param_t {
    int cam_id;
    int display;
    int width;
    int height;
    int x_off;
    int y_off;
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

struct cam_crop_info {
    int crop_update;
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
};

struct cam_crop_info g_crop = {0, 0, 0, 0, 0};


#ifdef CONFIG_DISPSS
#define DISPLAY_TEMPLATE                                                       \
    {                                                                          \
        1,                        /*layer*/                                    \
            0,                    /*layer_dirty*/                              \
            1,                    /*layer_en*/                                 \
            COLOR_RGB888,         /*fmt*/                                      \
            {0, 0, 0, 0},         /*x,y,w,h src */                             \
            {0x0, 0x0, 0x0, 0x0}, /*y,u,v,a*/                                  \
            {0, 0, 0, 0},         /*stride*/                                   \
            {0, 0, 0, 0},         /*start*/                                    \
            {0, 0, 0, 0},         /*dest*/                                     \
            0,                    /*ckey_en*/                                  \
            0,                    /*ckey*/                                     \
            1,                    /*alpha_en*/                                 \
            0xff,                 /*alpha*/                                    \
            0,                    /*z-order*/                                  \
            0                     /*security*/                                 \
    }

typedef struct disp_buffer_t {
    struct sdm_buffer sdm_bufs[2];
    struct sdm_post_config post_data;
} disp_buffer_t;

static const struct sdm_buffer db_template = DISPLAY_TEMPLATE;
static void disp_buffer_init(disp_buffer_t *db,
                             const struct sdm_buffer *template1,
                             const struct sdm_buffer *template2)
{
    memset(db, 0, sizeof(disp_buffer_t));
    db->post_data.bufs = &db->sdm_bufs[0];
    db->post_data.n_bufs = 0;
    db->post_data.custom_data = NULL;
    db->post_data.custom_data_size = 0;

    if (template1) {
        memcpy(&db->post_data.bufs[0], template1, sizeof(struct sdm_buffer));
        db->post_data.n_bufs++;
    }

    if (template2) {
        memcpy(&db->post_data.bufs[1], template2, sizeof(struct sdm_buffer));
        db->post_data.n_bufs++;
    }
}

static int to_disp_fmt(uint32_t cam_pixfmt)
{
    switch (cam_pixfmt) {
    case VDEV_PIX_FMT_UYVY:
        return COLOR_YUYV;

    case VDEV_PIX_FMT_YUYV:
        return COLOR_YUYV;

    case VDEV_PIX_FMT_NV16:
        return COLOR_NV16;

    case VDEV_PIX_FMT_NV61:
        return COLOR_NV61;

    case VDEV_PIX_FMT_YUV422P:
        return COLOR_YUV422P;

    case VDEV_PIX_FMT_RGB24:
        return COLOR_RGB888;

    case VDEV_PIX_FMT_BGR24:
        return COLOR_BGR888;

    case VDEV_PIX_FMT_RGB16:
        return COLOR_RGB565;

    default:
        return COLOR_YUYV;
    }

    return COLOR_YUYV;
}

static void *g2d_init(void)
{
    void *handle = NULL;
#ifdef G2D_SCALE
    hal_g2dlite_creat_handle(&handle, RES_G2D_G2D2);

    if (handle == NULL) {
        printf("ERROR: g2dlite creat handle failed\n");
        return NULL;
    }

    hal_g2dlite_init(handle);
#endif
    return handle;
}

static void g2d_scale(void *handle, unsigned long dst, unsigned long src,
                      uint32_t src_w, uint32_t src_h, uint32_t dst_w,
                      uint32_t dst_h, uint32_t fmt)
{
#ifdef G2D_SCALE
    struct g2dlite_output_cfg *output;
    struct g2dlite_input_cfg *layer;
    struct g2dlite_input g2dlite_scaler;

    memset(&g2dlite_scaler, 0, sizeof(struct g2dlite_input));
    g2dlite_scaler.layer_num = 1;
    layer = &g2dlite_scaler.layer[0];
    layer->layer = 0;
    layer->layer_en = 1;
    layer->fmt = fmt;
    layer->zorder = 0;
    layer->src.x = 0;
    layer->src.y = 0;
    layer->src.w = src_w;
    layer->src.h = src_h;
    layer->src_stride[0] = src_w * 2;
    layer->dst.x = 0;
    layer->dst.y = 0;
    layer->dst.w = dst_w;
    layer->dst.h = dst_h;
    layer->ckey.en = 0;
    layer->blend = BLEND_PIXEL_NONE;
    layer->alpha = 0xFF;
    layer->addr[0] = src;

    output = &g2dlite_scaler.output;
    output->width = dst_w;
    output->height = dst_h;
    output->fmt = fmt;
    output->stride[0] = dst_w * 2;
    output->rotation = 0;
    output->addr[0] = dst;

    hal_g2dlite_blend(handle, &g2dlite_scaler);
#endif
}
#endif

static void cam_buffer_free(struct cam_mem_info *pmem)
{
    uint32_t i;

    for (i = 0; i < 3; i++) {
        if (pmem->mem_ptr[i]) {
            free(pmem->mem_ptr[i]);
        }
        pmem->mem_ptr[i] = NULL;
    }
}

static int cam_buffer_alloc(struct cam_mem_info *pmem, uint32_t pixfmt)
{
    uint32_t i, n;
    uint32_t planes;
    uint32_t pix_bytes;
    void *base;

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

    tester_info("pixfmt %d, planes %d, pix_bytes %d, size %d %d %d\n", pixfmt,
                planes, pix_bytes, pmem->buf_size[0], pmem->buf_size[1],
                pmem->buf_size[2]);

    pmem->planes = planes;
    pmem->pix_bytes = pix_bytes;

    /* normal mem */
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] = pmem->buf_size[i] * CAM_BUF_CNT;
        pmem->mem_ptr[i] = malloc(pmem->mem_size[i]);
        for (n = 0; n < CAM_BUF_CNT; n++) {
            base = pmem->mem_ptr[i] + n * pmem->buf_size[i];
            pmem->buffers[n][i] = base;
            memset(base, (0x30 * n) & 0xFF, pmem->buf_size[i]);
            arch_sync_cache_range((addr_t)base, pmem->mem_size[i]);
            tester_info("plane %d, buffer Idx.%d, ptr 0x%08x\n", i, n,
                        (uint32_t)base);
        }
    }

    return 0;
}

static void get_cam_crop(struct cam_crop_info *crop)
{
    *crop = g_crop;
    g_crop.crop_update = 0;
}

static void parse_options(int argc, char *argv[], cam_test_param *param)
{
    int i;

    memset(param, 0, sizeof(cam_test_param));
    param->cam_id = 0;
    param->display = 0;
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

        case 'o':
            i++;
            if (i < argc)
                param->x_off = atoi(argv[i]);
            i++;
            if (i < argc)
                param->y_off = atoi(argv[i]);
            tester_info("\nget display offset %d %d\n", param->x_off,
                        param->y_off);
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

static int osEvent = 1;
static int cam_run(void *data)
{
    int ret = 0;
    cam_test_param *param = (cam_test_param *)data;
    int cam_id = 0, display = -1, frm_cnt = 0, total_frame = 1000;
    uint32_t i, timeout, dqbuf = 0;
    uint32_t width, height, pixfmt, fps;
    uint32_t dis_w, dis_h;
    uint32_t fmt_list[MAX_PIXFMT_COUNT];
    uint32_t x[2], y[2], w[2], h[2];
    struct cam_mem_info mem_info;
    struct cam_crop_info crop_info;
    struct vdev_buffer cam_buf;
    camera_handle *cam = NULL;
#ifdef CONFIG_DISPSS
    display_handle *disp_handle = NULL;
    disp_buffer_t db[2];
    void *g2d_handle = NULL;
    unsigned long cur, disp_buf[2] = {0, 0};
    uint32_t scale = 0;
#endif

    cam_id = (param->cam_id >= 0) ? param->cam_id : 0;
    display = (param->display >= 0) ? param->display : -1;
    total_frame = param->total_frame;

    cam = hal_cam_get_instance(cam_id);

    if (cam == NULL) {
        tester_err("get cam %d failed \n", cam_id);
        return -1;
    }

    ret = hal_cam_init(cam);
    if (ret) {
        hal_cam_put_instance(cam);
        tester_err("hal_cam_init failed \n");
        return -1;
    }

    hal_cam_get_fps(cam, &fps);
    tester_info("camera fps %d\n", fps);

    hal_cam_get_size(cam, &width, &height);
    tester_info("cam size [%d %d]\n", width, height);

    pixfmt = VDEV_PIX_FMT_YUYV;

    if (param->cam_fmt == 0)
        pixfmt = VDEV_PIX_FMT_UYVY;
    else if (param->cam_fmt == 1)
        pixfmt = VDEV_PIX_FMT_YUYV;

    ret = hal_cam_enum_format(cam, fmt_list);
    for (i = 0; i < MAX_PIXFMT_COUNT; i++) {
        if (pixfmt == fmt_list[i])
            break;
    }

    if (i >= MAX_PIXFMT_COUNT)
        pixfmt = fmt_list[0];

    ret = hal_cam_set_format(cam, pixfmt);
    if (ret) {
        tester_err("cam %d set fmt failed\n", cam_id);
        hal_cam_deinit(cam);
        hal_cam_put_instance(cam);
        return -1;
    }

    tester_info("cam size [%d %d], pixfmt %d\n", width, height, pixfmt);

    memset(&mem_info, 0, sizeof(struct cam_mem_info));
    mem_info.w = width;
    mem_info.h = height;
    cam_buffer_alloc(&mem_info, pixfmt);

    memset(&cam_buf, 0, sizeof(struct vdev_buffer));

    for (i = 0; i < CAM_BUF_CNT; i++) {
        cam_buf.buffer_idx = i;
        cam_buf.paddr[0] = (void *)p2ap((paddr_t)mem_info.buffers[i][0]);
        cam_buf.paddr[1] = (void *)p2ap((paddr_t)mem_info.buffers[i][1]);
        cam_buf.paddr[2] = (void *)p2ap((paddr_t)mem_info.buffers[i][2]);
        hal_cam_qbuf(cam, &cam_buf);
        tester_info("camera qbuf done. index %d, addr %p, cam_paddr %p\n", i,
                    mem_info.buffers[i][0], cam_buf.paddr[0]);
    }

    get_cam_crop(&crop_info);
    if (crop_info.crop_update) {
        x[0] = x[1] = crop_info.x;
        y[0] = y[1] = crop_info.y;
        w[0] = w[1] = crop_info.w;
        h[0] = h[1] = crop_info.h;

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

    ret = hal_cam_start(cam);

    if (ret) {
        tester_err("cam start failed\n");
        goto error;
    }

    tester_info("camera start\n");

    dis_w = (w[0] > LCM_WIDTH) ? LCM_WIDTH : w[0];
    dis_h = (h[0] > LCM_HEIGHT) ? LCM_HEIGHT : h[0];
    tester_info(
        "display id %d, display size [%d %d], cam_crop[%d, %d, %d, %d]\n",
        display, dis_w, dis_h, x[0], y[0], w[0], h[0]);

#ifdef CONFIG_DISPSS

    if ((display >= 0) && ((dis_w != w[0]) || (dis_h != h[0])))
        scale = 1;

    /* TODO - finish scaling function, it is not ready now */
    scale = 0;

    if (scale) {
        disp_buf[0] = (unsigned long)malloc(2 * dis_w * dis_h * mem_info.pix_bytes);
        disp_buf[1] = disp_buf[0] + dis_w * dis_h * mem_info.pix_bytes;
        g2d_handle = g2d_init();
    }

#endif
    uint64_t last_timestamp = 0;
    uint64_t tmp_timestamp = 0;

    while (frm_cnt++ < total_frame) {
        i = 0;

        do {
            cam_buf.timeout_ms = CAMERA_TIMEOUT_MS;
            if (osEvent) {
                ret = hal_cam_dqbuf(cam, &cam_buf);
                dqbuf = 1;
            } else {
                int delay = (frm_cnt < 3) ? 90 : 200;
                ret = 0;
                dqbuf = 0;
                osDelay(delay);
                cam_buf.buffer_idx = (frm_cnt % CAM_BUF_CNT);
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

        if (0) printf("dqbuf: idx %d, buf0 addr:0x%lx 0x%lx\n", cam_buf.buffer_idx,
               (unsigned long)cam_buf.paddr[0],
               (unsigned long)(ap2p((paddr_t)cam_buf.paddr[0])));

	   tmp_timestamp = cam_buf.timestamp - last_timestamp;
	   last_timestamp = cam_buf.timestamp;

        if ((frm_cnt % 100 ) == 0) {
            tester_info("cam%d get frm.%d, buf_idx %x,timestamp 0x%llx\n", cam_id,frm_cnt,
                        cam_buf.buffer_idx, cam_buf.timestamp);
            tester_err("cam%d framerate: %lld.%lld fps\n",
            cam_id,  1000 / (tmp_timestamp / 1000), 1000 / (tmp_timestamp % 1000));

        }

        /* for DEBUG */
        if ((frm_cnt & 0x1) == 11) {
            for (i = 0; i < mem_info.planes; i++) {
                uint8_t *pval =
                    (uint8_t *)mem_info.buffers[cam_buf.buffer_idx][0];
                tester_info(
                    "plane%d, paddr:%p,addr:%p: %02x%02x%02x%02x "
                    "%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
                    i, cam_buf.paddr[0], pval, pval[0], pval[1], pval[2],
                    pval[3], pval[4], pval[5], pval[6], pval[7], pval[8],
                    pval[9], pval[10], pval[11], pval[12], pval[13], pval[14],
                    pval[15]);
            }
        }

        get_cam_crop(&crop_info);

        if (crop_info.crop_update) {
            x[0] = x[1] = crop_info.x;
            y[0] = y[1] = crop_info.y;
            w[0] = w[1] = crop_info.w;
            h[0] = h[1] = crop_info.h;
            hal_cam_set_crop(cam, x, y, w, h);
        }

        if (display < 0) {
            if (dqbuf) {
                hal_cam_qbuf(cam, &cam_buf);
                tester_debug("cam_test qbuf done. index %d, addr %p\n",
                         cam_buf.buffer_idx, cam_buf.paddr[0]);
            }
            continue;
        }

#ifdef CONFIG_DISPSS

        if (disp_handle == NULL) {
            if (display == 0)
                disp_handle = hal_get_display_handle(INFOTAINMENT);
            else if (display == 1)
                disp_handle = hal_get_display_handle(ENTERTAINMENT);
            else if (display == 2)
                disp_handle = hal_get_display_handle(CONTROLPANEL);
            else if (display == 3)
                disp_handle = hal_get_display_handle(CLUSTER);
            else
                disp_handle = hal_get_display_handle(INFOTAINMENT);

            disp_buffer_init(&db[0], &db_template, NULL);
            db[0].sdm_bufs[0].fmt = to_disp_fmt(pixfmt);
        }

        if (disp_handle == NULL) {
            if (dqbuf)
                hal_cam_qbuf(cam, &cam_buf);
            continue;
        }

        cur = (unsigned long)mem_info.buffers[cam_buf.buffer_idx][0];

        if (scale && g2d_handle) {
            printf("g2d_scale\n");
            g2d_scale(g2d_handle, disp_buf[frm_cnt & 1], cur, w[0], h[0], dis_w,
                      dis_h, db[0].sdm_bufs[0].fmt);
            cur = disp_buf[frm_cnt & 1];
            printf("g2d_scale done\n");
        }

        db[0].sdm_bufs[0].addr[0] = cur;
        db[0].sdm_bufs[0].src_stride[0] = dis_w * mem_info.pix_bytes;
        db[0].sdm_bufs[0].start.x = param->x_off;
        db[0].sdm_bufs[0].start.y = param->y_off;
        db[0].sdm_bufs[0].start.w = dis_w;
        db[0].sdm_bufs[0].start.h = dis_h;
        db[0].sdm_bufs[0].src.w = dis_w;
        db[0].sdm_bufs[0].src.h = dis_h;
        db[0].sdm_bufs[0].dst.w = dis_w;
        db[0].sdm_bufs[0].dst.h = dis_h;

        sdm_post(disp_handle, &db[0].post_data);
        if ((frm_cnt & 0xF) == 1) {
            tester_info("post frm %d. index %d, addr 0x%08x paddr 0x%08x\n", frm_cnt,
                        cam_buf.buffer_idx, (uint32_t)cur, (uint32_t)cam_buf.paddr[0]);
        }

        if (dqbuf)
            hal_cam_qbuf(cam, &cam_buf);

        if ((frm_cnt & 0xF) == 55) {
            tester_info("cam_test qbuf done. index %d, addr %p\n",
                        cam_buf.buffer_idx, cam_buf.paddr[0]);
        }
#endif
    }

#ifdef CONFIG_DISPSS
    if (disp_handle)
        sdm_clear_display(disp_handle);
#endif

    tester_info("cam_test stop\n");
    hal_cam_stop(cam);

error:
    cam_buffer_free(&mem_info);
    hal_cam_deinit(cam);
    hal_cam_put_instance(cam);

	free(data);

    tester_info("camera test exit\n\n");
    return 0;
}

static void cam_thread_func(void *data)
{
    cam_run(data);
    osThreadExit();
}

static int cam_test(int argc, char *argv[])
{
    cam_test_param *param0 = malloc(sizeof(cam_test_param));
    osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };

    tester_warn("usage0:\n");
    tester_warn("cam_test -h\n");
    tester_warn("  -h just output this usage information\n\n");
    tester_warn("usage1:\n");
    tester_warn("cam_test -[i|s|y|d|n|p]\n");
    tester_warn("  -i cam_id\n");
    tester_warn("  -s cam_w cam_h\n");
    tester_warn("  -y display_id\n");
    tester_warn("  -o display_off_x display_off_y\n");
    tester_warn("  -n total_frame_num\n");
    tester_warn("  -p cam_fmt (0: YUYV, 1: UYVY)\n");

    tester_warn("argc: %d\n", argc);

    if (argc == 1 && !strcmp(argv[0], "-h")) {
        tester_warn("csi-test exit for help massage\n");
        return 0;
    }

    parse_options(argc, argv, param0);

    osThreadNew(cam_thread_func, (void *)param0, &attr);
    return 0;
}

CLI_CMD("cam_test", "Start cam test", cam_test);
