
#ifdef _CONFIG_OS_SAFETY_
#include <app.h>
#include <arch.h>
#include <debug.h>
#include <err.h>
#include <ext_data.h>
#include <hal_port.h>
#include <heap.h>
#include <image_cfg.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>
#include <kernel/vm.h>
#include <lib/console.h>
#include <macros.h>
#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>

//#include "res.h"
//#include "chip_res.h"
//#include "disp_data_type.h"

#define tester_err(x...) dprintf(ALWAYS, x)
#define tester_warn(x...) dprintf(ALWAYS, x)
#define tester_info(x...) dprintf(INFO, x)
#define tester_debug(x...) dprintf(SPEW, x)

#else /* _CONFIG_OS_SSDK_ */

#include <cmsis_os2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLI_type.h"
#include "regs_base.h"

#ifdef CONFIG_DISPSS
#include "driver/taishan/dispss/disp.h"
#include "driver/taishan/dispss/disp_data_type.h"
#endif

#define tester_err(x...) ssdk_printf(SSDK_ERR, x)
#define tester_warn(x...) ssdk_printf(SSDK_WARNING, x)
#define tester_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define tester_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#endif

#include "cam_hal.h"

/* for ti926 DEBUG */
#ifdef CONFIG_TI926
#include "ex_ti926.h"
#endif

#define CAM_PAGE_SIZE (1 << 12)
#define CAM_PAGE_MASK (~(CAM_PAGE_SIZE - 1))

#define CAMERA_TIMEOUT_MS 500
#define DISPLAY_NUM 1
#define CAM_BUF_CNT 4

/* mem_size = frame_size * CIRCLE_MEM_FACTOR / CIRCLE_MEM_BASE */
#define USE_CIRCLE_MEM
#define CIRCLE_MEM_FACTOR 1000
#define CIRCLE_MEM_BASE 1000

#define LCM_WIDTH 1280
#define LCM_HEIGHT 768
#define SKIP_FRAME 3

static int csi_reg_show(uint32_t base, uint32_t start, uint32_t end);

typedef struct cam_test_param_t {
    int cam_id;
    int display;
    int crop_en;
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
    char *mem_ptr[3];

    uint32_t stride[3];
    uint32_t buf_size[3];
    char *buffers[CAM_BUF_CNT][3];
};

struct cam_crop_info {
    int crop_update;
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
};

static struct cam_crop_info crop_setting[4] = {
    {1, 180, 0, 1280, 720},
    {1, 360, 0, 1280, 720},
    {1, 540, 0, 1280, 720},
    {1, 640, 0, 1280, 720},
};

static int crop_sel;
static struct cam_crop_info g_crop = {1, 0, 0, 1280, 720};

#ifdef CONFIG_DISPSS

extern struct dc_dev g_dc_dev;

static void disp_post_init(struct post_cfg *post, struct surface *layer,
                           uint32_t width, uint32_t height, int fmt,
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

    layer->dst.x = 0;
    layer->dst.y = (LCM_HEIGHT - height) / 2;
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
            osFree(pmem->mem_ptr[i]);
            pmem->mem_ptr[i] = NULL;
        }
    }
}

static int cam_buffer_alloc(struct cam_mem_info *pmem, uint32_t pixfmt)
{
    uint32_t i, n;
    uint32_t planes;
    uint32_t pix_bytes;
    char *base;

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
        pmem->stride[1] = pmem->w * 3;
        pmem->stride[2] = pmem->w * 3;
        pmem->buf_size[0] = pmem->w * pmem->h * 3;
        pmem->buf_size[1] = pmem->w * pmem->h * 3;
        pmem->buf_size[2] = pmem->w * pmem->h * 3;
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
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] =
            pmem->buf_size[i] * CIRCLE_MEM_FACTOR / CIRCLE_MEM_BASE;
        pmem->mem_size[i] += CAM_PAGE_SIZE - 1;
        pmem->mem_size[i] &= CAM_PAGE_MASK;
        pmem->mem_ptr[i] = osAllocAlign(CAM_PAGE_SIZE, pmem->mem_size[i]);
        memset(pmem->mem_ptr[i], 0xAA, pmem->mem_size[i]);
        base = pmem->mem_ptr[i];
        for (n = 0; n < CAM_BUF_CNT; n++) {
            pmem->buffers[n][i] = base;
            tester_info("plane %d, buffer Idx.%d, circle ptr 0x%08x\n", i, n,
                        (uint32_t)base);
            if (pmem->mem_size[i] > (pmem->buf_size[i] + CAM_PAGE_SIZE)) {
                base += pmem->buf_size[i];
                while (base >= (pmem->mem_ptr[i] + pmem->mem_size[i]))
                    base -= pmem->mem_size[i];
            }
        }
    }
    return 0;

normal_mem:
    /* normal mem */
    for (i = 0; i < planes; i++) {
        pmem->mem_size[i] = pmem->buf_size[i] * CAM_BUF_CNT;
        pmem->mem_ptr[i] = osAllocAlign(CAM_PAGE_SIZE, pmem->mem_size[i]);
        for (n = 0; n < CAM_BUF_CNT; n++) {
            base = pmem->mem_ptr[i] + n * pmem->buf_size[i];
            pmem->buffers[n][i] = base;
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
#ifdef CONFIG_TI926 /* for ti926 DEBUG */
        case 't':
            i++;
            if (i < argc)
                test_pattern = atoi(argv[i]);
            tester_info("\nget test_pattern %d\n", test_pattern);
            break;
#endif
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

        case 'c':
            i++;
            if (i < argc)
                param->crop_en = atoi(argv[i]);
            tester_info("\nget test_crop_en %d\n", param->crop_en);
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

static int osEvent = 0;
static cam_test_param test_param;
int cam_run(void *data)
{
    int ret = 0;
    cam_test_param *param = (cam_test_param *)data;
    int cam_id = 0, display = -1, frm_cnt = 0, total_frame = 1000;
    uint32_t i, timeout, dqbuf = 0;
    uint32_t width, height, pixfmt, fps;
    uint32_t dis_w, dis_h;
    uint32_t x[2], y[2], w[2], h[2];
    struct cam_mem_info mem_info;
    struct cam_crop_info crop_info;
    struct vdev_buffer cam_buf;
    camera_handle *cam;
#ifdef CONFIG_DISPSS
    struct post_cfg post;
    struct surface layer;
    int disp_fmt, offset;
#endif

    if (data == NULL) {
        param = &test_param;

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

    tester_info("cam size [%d %d], pixfmt %d\n", width, height, pixfmt);

    memset(&mem_info, 0, sizeof(struct cam_mem_info));
    mem_info.w = LCM_WIDTH;
    mem_info.h = LCM_HEIGHT;
    cam_buffer_alloc(&mem_info, pixfmt);

    memset(&cam_buf, 0, sizeof(struct vdev_buffer));
    for (i = 0; i < CAM_BUF_CNT; i++) {
        cam_buf.buffer_idx = i;
        cam_buf.paddr[0] = (void *)mem_info.buffers[i][0];
        cam_buf.paddr[1] = (void *)mem_info.buffers[i][1];
        cam_buf.paddr[2] = (void *)mem_info.buffers[i][2];
        hal_cam_qbuf(cam, &cam_buf);
        tester_info("camera qbuf done. index %d, addr %p\n", i,
                    cam_buf.paddr[0]);
    }

#ifdef TEST_DISP
    for (i = 0; i < 1; i++) {
        int w = LCM_WIDTH, h = LCM_HEIGHT;
        void *test_buf = (void *)cam_buf.paddr[0];
        uint8_t *val = (uint8_t *)cam_buf.paddr[0];
        int j;
        for (j = 0; j < (w * h * 3); j += 3) {
            val[j + 0] = 0x10;
            val[j + 1] = 0x10;
            val[j + 2] = 0xF0;
        }
        disp_fmt = COLOR_RGB888;
        disp_post_init(&post, &layer, w, h, disp_fmt, test_buf, test_buf,
                       test_buf, w * 3, w * 3, w * 3);
        sdrv_display_post(&g_dc_dev, &post);
        tester_err("test sdrv_display_post %d, buf 0x%08x\n", i,
                   (uint32_t)test_buf);
        usleep_range(60000, 60002);
    }
#endif

    if ((display == 1) && mem_info.circle_mem) {
        uint32_t low[3], up[3];
        tester_info("camera will use circle memory\n");
        for (i = 0; i < mem_info.planes; i++) {
            low[i] = (uint32_t)mem_info.mem_ptr[i];
            up[i] = (uint32_t)mem_info.mem_ptr[i] + mem_info.mem_size[i];
        }
        hal_cam_set_memrange(cam, low, up);
        osEvent = 0;
    }

    if ((param->crop_en == 0) &&
        ((width > LCM_WIDTH) || (height > LCM_HEIGHT))) {
        param->crop_en = 1;
    }
    get_cam_crop(&crop_info);
    if (param->crop_en && crop_info.crop_update) {
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
        crop_info.crop_update = 0;
        x[0] = x[1] = 0;
        y[0] = y[1] = 0;
        w[0] = w[1] = width;
        h[0] = h[1] = height;
    }

    dis_w = (w[0] > LCM_WIDTH) ? LCM_WIDTH : w[0];
    dis_h = (h[0] > LCM_HEIGHT) ? LCM_HEIGHT : h[0];
    tester_info(
        "display id %d, display size [%d %d], cam_crop[%d, %d, %d, %d]\n",
        display, dis_w, dis_h, x[0], y[0], w[0], h[0]);

    ret = hal_cam_start(cam);
    if (ret) {
        tester_err("cam start failed\n");
        goto error;
    }

    tester_info("camera start\n");
    while (frm_cnt++ < total_frame) {
        i = 0;
        dqbuf = 0;
        do {
            cam_buf.timeout_ms = CAMERA_TIMEOUT_MS;
            if (osEvent) {
                ret = hal_cam_dqbuf(cam, &cam_buf);
                dqbuf = 1;
            } else {
                int delay = (frm_cnt < 3) ? 30 : 80;
                ret = 0;
                osDelay(delay);
            }
            tester_debug("cam_test dqbuf, ret %d\n", ret);
            if (ret == 0) {
                timeout = 0;
                break;
            }
            timeout = 1;
        } while (i++ < 50000);

        if (timeout) {
            tester_err("cam_test： wait timeout for times %d\n", i);
            break;
        }

        if ((frm_cnt & 0x3FF) == 2)
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

#if TEST_CSI_BITS
        if ((frm_cnt & 0x7F) == 100) {
            uint8_t R = 0, G = 0, B = 0;
            uint8_t *ptr = (uint8_t *)cam_buf.paddr[0];
            for (i = 0; i < 1280 * 480 * 3; i += 3) {
                R |= ptr[0];
                G |= ptr[1];
                B |= ptr[2];
                ptr += 3;
            }
            printf("R 0x%02x  G 0x%02x  B 0x%02x\n", R, G, B);
        }
#endif

        if (param->crop_en && ((frm_cnt & 0x1FF) == 200)) {
            g_crop = crop_setting[crop_sel++ & 3];
            get_cam_crop(&crop_info);
            if (crop_info.crop_update) {
                x[0] = x[1] = crop_info.x;
                y[0] = y[1] = crop_info.y;
                w[0] = w[1] = crop_info.w;
                h[0] = h[1] = crop_info.h;
                hal_cam_set_crop(cam, x, y, w, h);
            }
        }

        if (display < 0) {
            if (dqbuf) {
                hal_cam_qbuf(cam, &cam_buf);
                tester_debug("cam_test qbuf done. index %d, addr %p\n",
                             cam_buf.buffer_idx, cam_buf.paddr[0]);
            }
            continue;
        }
        if (frm_cnt < SKIP_FRAME) {
            if (dqbuf) {
                hal_cam_qbuf(cam, &cam_buf);

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

        disp_post_init(&post, &layer, dis_w, dis_h, disp_fmt,
                       (uint8_t *)cam_buf.paddr[0] + offset, cam_buf.paddr[1],
                       cam_buf.paddr[2], mem_info.stride[0], mem_info.stride[1],
                       mem_info.stride[2]);

        if ((frm_cnt & 0xFFF) == 3)
            tester_info(
                "disp_cfg: [%d, %d], stride %d, addr 0x%lx, offset %d\n",
                layer.src.w, layer.src.h, layer.src_stride[0], layer.addr[0],
                offset);

        sdrv_display_post(&g_dc_dev, &post);
        tester_debug("sdrv_display_post done\n");

        if (display < 1 && dqbuf) {
            hal_cam_qbuf(cam, &cam_buf);
            if ((frm_cnt & 0xFFF) == 4)
                tester_info("cam_test qbuf done. index %d, addr %p\n",
                            cam_buf.buffer_idx, cam_buf.paddr[0]);
        }
#endif
    }

    tester_info("cam_test stop\n");
    hal_cam_stop(cam);

error:
    cam_buffer_free(&mem_info);
    hal_cam_deinit(cam);
    hal_cam_put_instance(cam);

    tester_info("camera test exit\n");
    return 0;
}

static void cam_thread_func(void *data)
{
    cam_run(data);
    osThreadExit();
}

static osThreadId_t cam_thread;

static int cam_test(int argc, char *argv[])
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
    tester_warn("  -c crop_en\n");
    tester_warn("  -y display_id\n");
    tester_warn("  -d display_w display_h\n");
    tester_warn("  -n total_frame_num\n");
    tester_warn("  -p cam_fmt (0: YUYV, 1: UYVY)\n");

    if (argc == 1 && !strcmp(argv[0], "-h")) {
        tester_warn("csi-test exit for help massage\n");
        return 0;
    }

    param0 = &test_param;
    parse_options(argc, argv, param0);

    cam_thread = osThreadNew(cam_thread_func, (void *)param0, &attr);

    return CLI_FALSE;
}

void camera_preview(void)
{
    char *parameter[] =
    {
        "-i","0","-y","1","-p","UYVY"
    };

    cam_test(sizeof(parameter)/sizeof(parameter[0]), parameter);
}

static int csi_reg_show(uint32_t base, uint32_t start, uint32_t end)
{
    uint32_t i;

    for (i = start; i < end; i += 16) {
        printf("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n", (base + i),
               *(uint32_t *)(base + i + 0), *(uint32_t *)(base + i + 4),
               *(uint32_t *)(base + i + 8), *(uint32_t *)(base + i + 12));
    }
    return 0;
}

static int csi_read(int argc, char *argv[])
{
    uint32_t start, end, offset, cnt;
    uint32_t limit = 0x8A0;

    start = offset = 0;
    end = cnt = limit;

    if (argc > 0) {
        offset = strtol(argv[0], NULL, 16);
        tester_info("get offset 0x%x\n", offset);
    }
    if (argc > 1) {
        cnt = strtol(argv[1], NULL, 16);
        tester_info("get cnt 0x%x\n", cnt);
    }

    offset = ((offset >> 4) << 4);
    if (offset < limit) {
        start = offset;
    }

    end = start + cnt;
    end = (((end + 15) >> 4) << 4);
    if (end > limit) {
        end = limit;
    }
    csi_reg_show(APB_CSI_BASE, start, end);

    return 0;
}

static int csi_write(int argc, char *argv[])
{
    uint32_t offset, val;
    uint32_t limit = 0x8A0;

    if (argc < 2) {
        tester_info("csi_write reg_offset reg_val\n");
        return 0;
    }
    offset = strtol(argv[0], NULL, 16);
    val = strtol(argv[1], NULL, 16);
    tester_info("get offset 0x%x val 0x%x\n", offset, val);

    if (offset < limit) {
        offset += APB_CSI_BASE;
        *(uint32_t *)offset = val;
        tester_info("new val 0x%x\n", *(uint32_t *)offset);
    }

    return 0;
}

#ifdef CONFIG_TI926 /* for ti926 DEBUG */
static int cam_rw(int argc, char *argv[])
{
    uint32_t rw, value, value1;
    uint8_t reg, val;

    if (argc < 3) {
        tester_info("cam_rw rw reg val\n");
        return 0;
    }
    rw = strtol(argv[0], NULL, 16);
    value = strtol(argv[1], NULL, 16);
    value1 = strtol(argv[2], NULL, 16);
    tester_info("get %d 0x%x val 0x%x\n", rw, value, value1);

    reg = (uint8_t)(value & 0xFF);
    val = (uint8_t)(value1 & 0xFF);
    rw = !!rw;
    user_test(rw, reg, &val);

    return 0;
}
CLI_CMD("cam_rw", "\r\ncam_rw:\r\n camera read/write\r\n", cam_rw);
#endif

CLI_CMD("cam_test", "\r\ncam_test:\r\n Start csi test\r\n", cam_test);
CLI_CMD("csi_read", "\r\ncsi_read: read csi regs\r\n", csi_read);
CLI_CMD("csi_write", "\r\ncsi_write: write csi reg\r\n", csi_write);

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("cam_pre", "cam_pre", &cam_test)
STATIC_COMMAND_END(cam_test);
#endif
