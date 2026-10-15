/**
 * @file scs_test.c
 * @brief camera service test program
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
#define tester_warn(x...) ssdk_printf(SSDK_ERR, x)
#define tester_info(x...) ssdk_printf(SSDK_ERR, x)
#define tester_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#include "cam_hal.h"
#include "cam_service.h"

#define CAM_PAGE_SIZE (1 << 12)
#define CAM_PAGE_MASK (~(CAM_PAGE_SIZE - 1))

#define CAMERA_TIMEOUT_MS 5000
#define DISPLAY_NUM 1
#define CAM_BUF_CNT 4

#define LCM_WIDTH 1920
#define LCM_HEIGHT 720

typedef struct cam_test_param_t {
    int cam_id;
    int m_type;
    int user;
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

#ifdef CONFIG_DISPSS
#define DISPLAY_TEMPLATE                                                       \
    {                                                                          \
        1,                        /*layer*/                                    \
            0,                    /*layer_dirty*/                              \
            1,                    /*layer_en*/                                 \
            COLOR_YUYV,           /*fmt*/                                      \
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

static uint32_t to_local_fmt(uint32_t client_fmt)
{
    switch (client_fmt) {
    case V4L2_PIX_FMT_YUYV:
        return VDEV_PIX_FMT_YUYV;

    case V4L2_PIX_FMT_UYVY:
        return VDEV_PIX_FMT_UYVY;

    case V4L2_PIX_FMT_NV16:
        return VDEV_PIX_FMT_NV16;

    case V4L2_PIX_FMT_NV61:
        return VDEV_PIX_FMT_NV61;

    case V4L2_PIX_FMT_YUV422P:
        return VDEV_PIX_FMT_YUV422P;

    case V4L2_PIX_FMT_RGB24:
        return VDEV_PIX_FMT_RGB24;

    case V4L2_PIX_FMT_BGR24:
        return VDEV_PIX_FMT_BGR24;

    case V4L2_PIX_FMT_RGB565:
        return VDEV_PIX_FMT_RGB16;

    default:
        return VDEV_PIX_FMT_YUYV;
    }

    return VDEV_PIX_FMT_YUYV;
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


int scs_csi_set_buf(uint32_t id, uint32_t index);
struct {

	uint8_t *buf_base;
	uint8_t *cam_buf[CAM_BUF_CNT];
	uint32_t cam_w, cam_h, cam_fmt, cam_img_size, pix_bytes;

} g_cam_data[CAMERA_NUM];

static void *scs_handle[CAMERA_NUM];
int scs_csi_init(uint32_t id, uint32_t *w, uint32_t *h, uint32_t *fmt)
{
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl;

    scs_handle[id] = scs_open(id);
    if (!scs_handle[id]) {
        tester_err("%s() fail to open cam%d\n", __func__, id);
        return -1;
    }

    ctl.op = SCS_OP_GET_FORMAT, ctl.instance = (uint16_t)id;
    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("%s() fail to get fmt\n", __func__);
        scs_close(scs_handle[id]);
        scs_handle[id] = NULL;
        return -1;
    }

	print_falgs;
    *w = r.msg.g_fmt.width;
    *h = r.msg.g_fmt.height;
    *fmt = r.msg.g_fmt.pixelformat;
    /* UYVY is not supported by display */
    if (r.msg.g_fmt.pixelformat == V4L2_PIX_FMT_UYVY)
        *fmt = V4L2_PIX_FMT_YUYV;

    ctl.op = SCS_OP_SET_FORMAT;
    ctl.msg.s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ctl.msg.s_fmt.width = r.msg.g_fmt.width;
    ctl.msg.s_fmt.height = r.msg.g_fmt.height;
    ctl.msg.s_fmt.pixelformat = *fmt;
    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("%s() fail to set fmt\n", __func__);
        scs_close(scs_handle[id]);
        scs_handle[id] = NULL;
        return -1;
    }

    return 0;
}

int scs_csi_config(uint32_t id, uint8_t *buf, uint32_t buf_size,
                   uint32_t buf_cnt, uint32_t m_type)
{
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_QUEUE_SETUP,
        .instance = (uint16_t)id,
        .msg.queue.num_bufs = (uint8_t)buf_cnt,
		.msg.queue.m_type = (uint8_t)m_type,
		.msg.queue.buf_size = buf_size,
    };

    if (!scs_handle[id])
        return -1;

    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("%s() fail to setup queue\n", __func__);
        goto fail_close;
    }

	if (m_type != SCS_MEMORY_MMAP) {
	    for (uint32_t j = 0; j < buf_cnt; j++) {
	        ctl.op = SCS_OP_SET_BUFINFO;
	        ctl.instance = (uint16_t)id, ctl.msg.s_bufinfo.index = (uint8_t)j;
	        ctl.msg.s_bufinfo.addr[0] =
		            (uint32_t)p2ap((paddr_t)(buf + buf_size * j));
				g_cam_data[id].cam_buf[j] = buf + j * buf_size;
		        if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
		            tester_err("%s() %d fail to init buf[%d] %08x\n",
		                    __func__, __LINE__, j,
		                    ctl.msg.s_bufinfo.addr[0]);
		            goto fail_close;
		        }
	    }
	}

	if (m_type == SCS_MEMORY_MMAP) {
	    for (uint32_t j = 0; j < buf_cnt; j++) {
	        ctl.op = SCS_OP_GET_BUFINFO;
	        ctl.instance = (uint16_t)id, r.msg.g_bufinfo.index = (uint8_t)j;

	        if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
	            tester_err("%s() %d fail to init buf[%d] %08x\n",
	                    __func__, __LINE__, j,
	                    ctl.msg.s_bufinfo.addr[0]);
	            goto fail_close;
	        }
			g_cam_data[id].cam_buf[j] = (uint8_t *)(ap2p((paddr_t)r.msg.g_bufinfo.addr[0]));
	    }
	}

	for (uint32_t j = 0; j < buf_cnt; j++) {
		scs_csi_set_buf(id, j);
	}

    return 0;

fail_close:
    scs_close(scs_handle[id]);
    scs_handle[id] = NULL;

    return -1;
}

int scs_csi_start(uint32_t id)
{
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_STREAM_ON,
        .instance = (uint16_t)id,
    };

    if (!scs_handle[id])
        return -1;

    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("%s() fail to start\n", __func__);
        scs_close(scs_handle[id]);
        scs_handle[id] = NULL;
        return -1;
    }

    return 0;
}

int scs_csi_get_buf(uint32_t id, uint32_t *index, uint64_t *timestamp, u32 *addr)
{
    if (!scs_handle[id])
        return -1;

    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_DQBUF,
        .instance = (uint16_t)id,
    };
    struct scs_ioctl_result r;
    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        return -1;
    }
	*addr = r.msg.g_bufinfo.addr[0];

    *index = r.msg.g_bufinfo.index;
    *timestamp = ((uint64_t)r.msg.g_bufinfo.timestamp_h << 32);
    *timestamp |= (uint64_t)r.msg.g_bufinfo.timestamp;

    return 0;
}

int scs_csi_set_buf(uint32_t id, uint32_t index)
{
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_QBUF,
        .instance = (uint16_t)id,
        .msg.s_bufinfo.index = (uint32_t)index,
    };

    if (!scs_handle[id])
        return -1;

    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("cam%d %s() fail to set buf\n",id, __func__);
        return -1;
    }

    return 0;
}

int scs_csi_stop(uint32_t id)
{
    if (!scs_handle[id])
        return -1;

    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_STREAM_OFF,
        .instance = (uint16_t)id,
    };
    struct scs_ioctl_result r;
    if (scs_ioctl(scs_handle[id], &ctl, &r) < 0) {
        tester_err("%s() fail to stop\n", __func__);
        scs_close(scs_handle[id]);
        scs_handle[id] = NULL;
        return -1;
    }

    return 0;
}

int scs_csi_close(uint32_t id)
{
    if (!scs_handle[id])
        return -1;

    scs_close(scs_handle[id]);

    return 0;
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
    param->total_frame = 0;
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
		case 'm':
			i++;

			if (i < argc)
				param->m_type = atoi(argv[i]);
			tester_info("\nget m_type %d\n", param->m_type);
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



int csi_entry(uint32_t cam_id, uint32_t m_type)
{
    int ret = 0;
    uint32_t buf_size, buf_cnt;
    uint8_t *buf = NULL;

    ret = scs_csi_init(cam_id, &g_cam_data[cam_id].cam_w, &g_cam_data[cam_id].cam_h, &g_cam_data[cam_id].cam_fmt);
    if (ret < 0) {
        tester_err("%s(): init error\n", __func__);
        return ret;
    }

    if (g_cam_data[cam_id].cam_fmt == V4L2_PIX_FMT_YUYV || g_cam_data[cam_id].cam_fmt == V4L2_PIX_FMT_UYVY
        || g_cam_data[cam_id].cam_fmt == V4L2_PIX_FMT_RGB565) {
        g_cam_data[cam_id].cam_img_size = g_cam_data[cam_id].cam_w * g_cam_data[cam_id].cam_h * 2;
        g_cam_data[cam_id].pix_bytes = 2;
    } else {
        g_cam_data[cam_id].cam_img_size = g_cam_data[cam_id].cam_w * g_cam_data[cam_id].cam_h * 3;
        g_cam_data[cam_id].pix_bytes = 3;
    }
    buf_size = g_cam_data[cam_id].cam_img_size;

    tester_err("%s(): cam%d, size %d %d, bufsize %d, fmt 0x%08x\n",
            __func__, cam_id, g_cam_data[cam_id].cam_w, g_cam_data[cam_id].cam_h,
            g_cam_data[cam_id].cam_img_size, g_cam_data[cam_id].cam_fmt);

    buf_cnt = CAM_BUF_CNT;
	if (m_type != SCS_MEMORY_MMAP) {//返回虚拟地址
	    buf = malloc(buf_cnt * buf_size);
	    if (buf == NULL) {
	        tester_err("failed to alloc cam buffer\n");
	        ret = -1;
	        goto cam_err;
	    }
	}

    ret = scs_csi_config(cam_id, buf, buf_size, buf_cnt, m_type);
    if (ret < 0) {
        tester_err("%s(): config error\n", __func__);
        goto cam_err;
    }

    ret = scs_csi_start(cam_id);
    if (ret < 0) {
        tester_err("%s(): config error\n", __func__);
        goto cam_err;
    }
    g_cam_data[cam_id].buf_base = buf;
    return 0;

cam_err:
    if (buf)
        free(buf);
    scs_csi_close(cam_id);
    return ret;
}

static int scs_run(void *data)
{
    int ret = 0;
    cam_test_param *param = (cam_test_param *)data;
    int cam_id = 0, display = -1, frm_cnt = 0, total_frame = 1000;
    uint32_t i, timeout;
    uint32_t pixfmt;
    uint32_t dis_w, dis_h;
    uint32_t m_type;
    uint32_t w[2], h[2];
#ifdef CONFIG_DISPSS
    display_handle *disp_handle = NULL;
    disp_buffer_t db[2];
    void *g2d_handle = NULL;
    unsigned long cur, disp_buf[2] = {0, 0};
    uint32_t scale = 0;
#endif

    uint32_t addr;

    cam_id = (param->cam_id >= 0) ? param->cam_id : 0;
    display = (param->display >= 0) ? param->display : -1;
    total_frame = param->total_frame;
	m_type = param->m_type;

    ret = csi_entry(cam_id, m_type);
    if (ret != 0) {
        tester_err("get cam %d failed \n", cam_id);
        return -1;
    }

    pixfmt = to_local_fmt(g_cam_data[cam_id].cam_fmt);
    tester_info("camera start\n");


    dis_w = (g_cam_data[cam_id].cam_w > LCM_WIDTH) ? LCM_WIDTH : g_cam_data[cam_id].cam_w;
    dis_h = (g_cam_data[cam_id].cam_h > LCM_HEIGHT) ? LCM_HEIGHT : g_cam_data[cam_id].cam_h;
    tester_info("display id %d, display size [%d %d], cam_size [%d, %d]\n",
                display, dis_w, dis_h, g_cam_data[cam_id].cam_w, g_cam_data[cam_id].cam_h);

#ifdef SAFETY_DISP_SUPPORT

    w[0] = g_cam_data[cam_id].cam_w;
    h[0] = g_cam_data[cam_id].cam_h;
    if ((display >= 0) && ((dis_w != w[0]) || (dis_h != h[0])))
        scale = 1;

    scale = 0;

    if (scale) {
        disp_buf[0] =
            (unsigned long)memalign(CAM_PAGE_SIZE, 2 * dis_w * dis_h * g_cam_data[cam_id].pix_bytes);
        disp_buf[1] = disp_buf[0] + dis_w * dis_h * g_cam_data[cam_id].pix_bytes;
        g2d_handle = g2d_init();
    }

#endif

    uint32_t buf_index;
    uint64_t timestamp;
    uint64_t last_timestamp = 0;
    uint64_t tmp_timestamp = 0;
    while (frm_cnt++ < total_frame || (total_frame == 0)) {
        int timeout_count = 0;
        i = 0;
        timeout = 0;

RETRY:
        if (scs_csi_get_buf(cam_id, &buf_index, &timestamp, &addr) < 0) {
            timeout_count++;
            //if (timeout_count == 5) {
                //tester_err("camera wait timeout\n");
                //timeout = 1;
                //break;
            //}
            goto RETRY;
        }

		tmp_timestamp = timestamp - last_timestamp;
		last_timestamp = timestamp;

        if (buf_index == 0xFF) {
            tester_err("test camera service switching out and exit\n");
            break;
        }

        if (timeout) {
            tester_err("cam_test： wait timeout for times %d\n", i);
            break;
        }

        if ((frm_cnt % 100 ) == 0) {
            tester_info("cam%d get frm.%d, buf_idx %x,timestamp 0x%llx\n", cam_id,frm_cnt,
                        buf_index, timestamp);
            tester_err("cam%d framerate: %lld.%lld fps\n",
            cam_id,  1000 / (tmp_timestamp / 1000), 1000 / (tmp_timestamp % 1000));

        }

        if (display < 0) {
            scs_csi_set_buf(cam_id, buf_index);
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
            scs_csi_set_buf(cam_id, buf_index);
            continue;
        }

        cur = (unsigned long)(ap2p((paddr_t)addr));
        if (scale && g2d_handle) {
            printf("g2d_scale\n");
            g2d_scale(g2d_handle, disp_buf[frm_cnt & 1], cur, w[0], h[0], dis_w,
                      dis_h, db[0].sdm_bufs[0].fmt);
            cur = disp_buf[frm_cnt & 1];
            printf("g2d_scale done\n");
        }

        db[0].sdm_bufs[0].addr[0] = cur;
        db[0].sdm_bufs[0].src_stride[0] = dis_w * g_cam_data[cam_id].pix_bytes;
        db[0].sdm_bufs[0].start.x = param->x_off;
        db[0].sdm_bufs[0].start.y = param->y_off;
        db[0].sdm_bufs[0].start.w = dis_w;
        db[0].sdm_bufs[0].start.h = dis_h;
        db[0].sdm_bufs[0].src.w = dis_w;
        db[0].sdm_bufs[0].src.h = dis_h;
        db[0].sdm_bufs[0].dst.w = dis_w;
        db[0].sdm_bufs[0].dst.h = dis_h;

        sdm_post(disp_handle, &db[0].post_data);

        scs_csi_set_buf(cam_id, buf_index);
#endif
    }

#ifdef CONFIG_DISPSS
    if (disp_handle)
        sdm_clear_display(disp_handle);
#endif

    tester_info("scs_test stop\n");
    scs_csi_stop(cam_id);
    if (g_cam_data[cam_id].buf_base) {
        free(g_cam_data[cam_id].buf_base);
        g_cam_data[cam_id].buf_base = NULL;
    }

error:
    scs_csi_close(cam_id);
    tester_info("scs test exit\n\n");

	free(data);

    return 0;
}

static void scs_thread_func(void *data)
{
    scs_run(data);
    osThreadExit();
}

static int scs_test(int argc, char *argv[])
{
    cam_test_param *param0 = malloc(sizeof(cam_test_param));

    osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };
		tester_warn("------------0:\n");

    tester_warn("usage0:\n");
    tester_warn("scs_test -h\n");
    tester_warn("  -h just output this usage information\n\n");
    tester_warn("usage1:\n");
    tester_warn("scs_test -[i|s|y|d|n|p]\n");
    tester_warn("  -i cam_id\n");
    tester_warn("  -s cam_w cam_h\n");
    tester_warn("  -y display_id\n");
    tester_warn("  -o display_off_x display_off_y\n");
    tester_warn("  -n total_frame_num\n");
    tester_warn("  -m memory type: 1:mmap 0:user\n");
    tester_warn("  -u user id \n");

    tester_warn("argc: %d\n", argc);

    if (argc == 1 && !strcmp(argv[0], "-h")) {
        tester_warn("csi-test exit for help massage\n");
        return 0;
    }

	parse_options(argc, argv, param0);


    osThreadNew(scs_thread_func, (void *)param0, &attr);

    return 0;
}

#ifdef _CONFIG_OS_SSDK_
CLI_CMD("scs_test", "Start safe camera service test",
        scs_test);
#endif
