/**
 * @file emirror_camera.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv emirror application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "emirror"

#include <driver/mbox.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <memmap.h>
#include <param.h>
#include <gpu_hal.h>
#include <malloc.h>
#include <mbox_cfg.h>
#include <driver/timer.h>
#include <vdev_defs.h>
#include <disp_data_type.h>
#include "disp_utility.h"
#include "emirror_def.h"
#include "emirror_cfg.h"
#include "emirror.h"
#include "container.h"
#include "app_log.h"
#include "vdsp_utility.h"

#define YUV  (2)
#define RGB  (3)
#define RGBA (4)
#ifdef CONFIG_MP
#define VDSP_ELF_PATH "early_app/fastrvm_mp/vdsp.elf"
#define MAP_TLB_PATH  "early_app/fastrvm_mp/MappingTable.bin"
#define SD_LOGO_PATH  "early_app/fastrvm_mp/sdv_logo.rgb"
#else
#define VDSP_ELF_PATH "early_app/fastrvm/vdsp.elf"
#define MAP_TLB_PATH  "early_app/fastrvm/MappingTable.bin"
#define SD_LOGO_PATH  "early_app/fastrvm/sdv_logo.rgb"
#endif

#define EMI_CSI_OPS(f, args...) ({ \
        int ret = 0; \
        if (emi_csi_ops.f) \
            ret = (emi_csi_ops.f)(args); \
        ret; \
        })

#define G2D_CONVERT_LAYER(w, h) { \
        .layer = 0, \
        .layer_en = 1, \
        .fmt = COLOR_YUYV, \
        .zorder = 0, \
        .src = { 0, 0, w, h }, \
        .src_stride = { w*YUV, 0, 0, 0 }, \
        .dst = { 0, 0, w, h }, \
        .ckey.en = 0, \
        .blend = BLEND_PIXEL_NONE, \
        .alpha = 0xFF, \
    }

#define G2D_CONVERT_OUTPUT(w, h) { \
        .width = w, \
        .height = h, \
        .fmt = COLOR_RGB888, \
        .stride[0] = w*RGB, \
        .rotation = 0, \
    }

#define G2D_ROTATION_LAYER(w, h) { \
        .layer = 0, \
        .layer_en = 1, \
        .fmt = COLOR_RGB888, \
        .zorder = 0, \
        .src = { 0, 0, w, h }, \
        .src_stride = { w*RGB, 0, 0, 0 }, \
        .dst = { 0, 0, w, h }, \
        .ckey.en = 0, \
        .blend = BLEND_PIXEL_NONE, \
        .alpha = 0xFF, \
    }

#define G2D_ROTATION_OUTPUT(w, h) { \
        .width = w, \
        .height = h, \
        .fmt = COLOR_RGB888, \
        .stride[0] = w*RGB, \
        .rotation = ROTATION_TYPE_HFLIP, \
    }

#define G2D_SCALE_LAYER(sr, dr, w) { \
        .layer = 0, \
        .layer_en = 1, \
        .fmt = COLOR_RGB888, \
        .zorder = 0, \
        .src = { sr.x, sr.y, sr.w, sr.h }, \
        .src_stride = { w*RGB, 0, 0, 0 }, \
        .dst = { dr.x, dr.y, dr.w, dr.h }, \
        .ckey.en = 0, \
        .blend = BLEND_PIXEL_NONE, \
        .alpha = 0xFF, \
    }

#define G2D_SCALE_OUTPUT(w, h) { \
        .width = w, \
        .height = h, \
        .fmt = COLOR_RGB888, \
        .stride[0] = w*RGB, \
        .rotation = 0, \
    }

#define G2D_BLEND_LAYER0(sr, dr) { \
        .layer = 0, \
        .layer_en = 1, \
        .fmt = COLOR_YUYV, \
        .zorder = 0, \
        .src = { sr.x, sr.y, sr.w, sr.h }, \
        .src_stride = { sr.w*YUV, 0, 0, 0 }, \
        .dst = { dr.x, dr.y, dr.w, dr.h }, \
        .ckey.en = 0, \
        .blend = BLEND_PIXEL_NONE, \
        .alpha = 0xFF, \
    }

#define G2D_BLEND_LAYER1(sr, dr) { \
        .layer = 1, \
        .layer_en = 1, \
        .fmt = COLOR_RGB888, \
        .zorder = 1, \
        .src = { sr.x, sr.y, sr.w, sr.h }, \
        .src_stride = { sr.w*RGB, 0, 0, 0 }, \
        .dst = { dr.x, dr.y, dr.w, dr.h }, \
        .ckey.en = 0, \
        .blend = BLEND_PIXEL_NONE, \
        .alpha = 0xFF, \
    }

#define G2D_BLEND_OUTPUT(w, h) { \
        .width = w, \
        .height = h, \
        .fmt = COLOR_RGB888, \
        .stride[0] = w*RGB, \
        .rotation = 0, \
    }

typedef struct _vdsp_msg_t {
    paddr_t output_addr;
    paddr_t input_addr;
} vdsp_msg_t;

typedef struct _emirror_camera_ctx {
    uint32_t left_w;
    uint32_t left_h;
    uint32_t right_w;
    uint32_t right_h;
    emi_csi_buff_t csi_buff;
    uint8_t* output_buff[CAM_NUM];
    uint8_t* rgb_buff;
    uint8_t* flip_buff;
    uint8_t* dwarp_buff;
    uint8_t* scale_buff;
    void* g2dlite_handle;
    mbox_chan_t* vdsp_mb_chan;
    osEventFlagsId_t vdsp_sig;
    void* vdsp_map_tbl;
} emirror_camera_ctx;

extern emirror_csi_ops_t emi_csi_ops;
static emirror_camera_ctx emi_cam_ctx;
static emirror_rect_t emi_roi[CAM_NUM];

static int emirror_camera_vdsp_wait_signal(uint32_t timeout)
{
    APP_DBG("");
    if (!emi_cam_ctx.vdsp_sig)
        return -1;

    uint32_t sig = osEventFlagsWait(emi_cam_ctx.vdsp_sig, 1, osFlagsWaitAny, timeout);
    osEventFlagsClear(emi_cam_ctx.vdsp_sig, 1);
    return sig == 1 ? 0 : -1;
}

static int emirror_camera_vdsp_signal(void *arg, uint8_t *data, uint32_t len)
{
    APP_DBG("");
    if (!arg)
        return -1;

    osEventFlagsId_t sig = (osEventFlagsId_t)arg;
    return osOK == osEventFlagsSet(sig, 1) ? 0 : -1;
}

static int emirror_camera_vdsp_start_one_frame(uint32_t pin, uint32_t pout)
{
    if (!emi_cam_ctx.vdsp_mb_chan)
        return -1;

    vdsp_msg_t msg;
    msg.output_addr = p2ap((uintptr_t)pout);
    msg.input_addr = p2ap((uintptr_t)pin);
    APP_DBG("input:%x, output:%x", msg.input_addr, msg.output_addr);
    int ret = mbox_send_nobuf(emi_cam_ctx.vdsp_mb_chan, (uint8_t*)&msg, sizeof(msg), 1000);
    return ret < 0 ? -1 : 0;
}

static int emirror_camera_vdsp_set_mapping_table()
{
    if (!emi_cam_ctx.vdsp_mb_chan)
        return -1;

    vdsp_msg_t msg;
    msg.output_addr = p2ap((paddr_t)emi_cam_ctx.vdsp_map_tbl);
    msg.input_addr = p2ap(0x123FF);
    APP_INFO("input:%x, output:%x", msg.input_addr, msg.output_addr);
    int ret = mbox_send_nobuf(emi_cam_ctx.vdsp_mb_chan, (uint8_t*)&msg, sizeof(msg), 1000);
    APP_INFO("send vdsp mapping table %s", !ret ? "success" : "fail");
    if (ret)
        return -1;

    ret = emirror_camera_vdsp_wait_signal(3000);
    APP_INFO("vdsp mapping table load %s", !ret ? "success" : "fail");
    if (ret)
        return -1;

    return 0;
}

static int emirror_camera_yuv2rgb(uint32_t pin, uint32_t pout)
{
    if (!emi_cam_ctx.g2dlite_handle)
        return -1;

    uint32_t w = emi_cam_ctx.csi_buff.w, h = emi_cam_ctx.csi_buff.h;
    struct g2dlite_input convert = {
        .layer_num = 1,
        .layer[0] = G2D_CONVERT_LAYER(w, h),
        .output = G2D_CONVERT_OUTPUT(w, h)
    };
    convert.layer[0].addr[0] = pin;
    convert.output.addr[0] = pout;
    APP_DBG("pin:0x%08x, pout:0x%08x", pin, pout);
    hal_g2dlite_blend(emi_cam_ctx.g2dlite_handle, &convert);
    return 0;
}

static int emirror_camera_hflip(uint32_t pin, uint32_t pout)
{
    if (!emi_cam_ctx.g2dlite_handle)
        return -1;

    uint32_t w = emi_cam_ctx.csi_buff.w, h = emi_cam_ctx.csi_buff.h;
    struct g2dlite_input rotation = {
        .layer_num = 1,
        .layer[0] = G2D_ROTATION_LAYER(w, h),
        .output = G2D_ROTATION_OUTPUT(w, h),
    };
    rotation.layer[0].addr[0] = pin;
    rotation.output.addr[0] = pout;
    APP_DBG("pin:0x%08x, pout:0x%08x", pin, pout);
    hal_g2dlite_rotaion(emi_cam_ctx.g2dlite_handle, &rotation);
    return 0;
}

static int emirror_camera_scale(uint32_t pin, uint32_t pout, emirror_rect_t* rect, uint32_t w, uint32_t h)
{
    if (!emi_cam_ctx.g2dlite_handle || !rect)
        return -1;

    struct g2dlite_input scale = {
        .layer_num = 1,
        .layer[0] = G2D_SCALE_LAYER(rect[0], rect[1], w),
        .output = G2D_SCALE_OUTPUT(w, h),
    };
    scale.layer[0].addr[0] = pin;
    scale.output.addr[0] = pout;
    APP_DBG("pin:0x%08x, pout:0x%08x", pin, pout);
    hal_g2dlite_blend(emi_cam_ctx.g2dlite_handle, &scale);
    return 0;
}

static int emirror_camera_fastcopy(uint32_t pin, uint32_t pout,
                                    uint32_t in_w, uint32_t in_h,
                                    uint32_t out_w)
{
    struct fcopy_t in = {
        .addr = (addr_t)pin,
        .width = in_w,
        .height = in_h,
        .stride = in_w*RGB,
    };
    struct fcopy_t out = {
        .addr = (addr_t)pout,
        .width = in_w,
        .height = in_h,
        .stride = out_w*RGB,
    };
    hal_g2dlite_fastcopy(emi_cam_ctx.g2dlite_handle, &in, &out);
    return 0;
}

static int emirror_camera_blend(uint32_t pin0, uint32_t pin1, uint32_t pout,
                                    emirror_rect_t* rect0, emirror_rect_t* rect1,
                                    uint32_t w, uint32_t h)
{
    if (!emi_cam_ctx.g2dlite_handle || !rect0 || !rect1)
        return -1;

    struct g2dlite_input blend = {
        .layer_num = 2,
        .layer[0] = G2D_BLEND_LAYER0(rect0[0], rect0[1]),
        .layer[1] = G2D_BLEND_LAYER1(rect1[1], rect1[1]),
        .output = G2D_BLEND_OUTPUT(w, h),
    };
    blend.layer[0].addr[0] = pin0;
    blend.layer[1].addr[0] = pin1;
    blend.output.addr[0] = pout;
    APP_DBG("pin0:0x%08x, pin1:0x%08x, pout:0x%08x", pin0, pin1, pout);
    hal_g2dlite_blend(emi_cam_ctx.g2dlite_handle, &blend);
    return 0;
}

static int emirror_camera_csi_init()
{
    int ret = 0;

    /* csi init */
    if (!ret) {
        ret = EMI_CSI_OPS(p_init, 0 , &emi_cam_ctx.csi_buff.w, &emi_cam_ctx.csi_buff.h, &emi_cam_ctx.csi_buff.fmt, &emi_cam_ctx.csi_buff.fps);
        APP_INFO("csi init :%s", !ret ? "success" : "fail");
    }
    if (!ret) {
        ret = EMI_CSI_OPS(p_alloc, 0, &emi_cam_ctx.csi_buff);
        APP_INFO("csi buffer malloc :%s", !ret ? "success" : "fail");
    }
    if (!ret) {
        emi_cam_ctx.output_buff[CAM_LEFT] = (uint8_t*)memalign(0x1000, emi_cam_ctx.left_w*emi_cam_ctx.left_h*RGB);
        emi_cam_ctx.output_buff[CAM_RIGHT] = (uint8_t*)memalign(0x1000, emi_cam_ctx.right_w*emi_cam_ctx.right_h*RGB);
        ret = (emi_cam_ctx.output_buff[CAM_LEFT] && emi_cam_ctx.output_buff[CAM_RIGHT] ? 0 : -1);
        APP_INFO("output buffer malloc :%s", !ret ? "success" : "fail");
    }
    if (!ret) {
        uint32_t w = emi_cam_ctx.csi_buff.w, h = emi_cam_ctx.csi_buff.h;
        emi_cam_ctx.rgb_buff = (uint8_t*)memalign(0x1000, w*h*RGB*CAM_NUM);
        emi_cam_ctx.flip_buff = (uint8_t*)memalign(0x1000, w*h*RGB*CAM_NUM);
        emi_cam_ctx.dwarp_buff = (uint8_t*)memalign(0x1000, w*h*RGB*CAM_NUM);
        emi_cam_ctx.scale_buff = (uint8_t*)memalign(0x1000, w*h*RGB*CAM_NUM);
        ret = (emi_cam_ctx.rgb_buff && emi_cam_ctx.flip_buff && emi_cam_ctx.dwarp_buff && emi_cam_ctx.scale_buff) ? 0 : -1;
        APP_INFO("csi buffer malloc :%s", !ret ? "success" : "fail");
    }
    if (!ret) {
        ret = EMI_CSI_OPS(p_config, 0 , &emi_cam_ctx.csi_buff, CAM_MAX_BUF);
        APP_INFO("csi config :%s", !ret ? "success" : "fail");
    }
    if (ret) {
        goto err;
    }
    return 0;

err:
    return -1;
}

static void emirror_camera_csi_deinit()
{
    for (int i = 0; i < 3; i++) {
        if (emi_cam_ctx.csi_buff.mem_ptr[i]) {
            free(emi_cam_ctx.csi_buff.mem_ptr[i]);
            emi_cam_ctx.csi_buff.mem_ptr[i] = NULL;
            emi_cam_ctx.csi_buff.buffers[0][i] = NULL;
            emi_cam_ctx.csi_buff.buffers[1][i] = NULL;
            emi_cam_ctx.csi_buff.buffers[2][i] = NULL;
        }
    }
    for (int i = 0 ; i < CAM_NUM; i++) {
        if (emi_cam_ctx.output_buff[i]) {
            free(emi_cam_ctx.output_buff[i]);
            emi_cam_ctx.output_buff[i] = NULL;
        }
    }
    if (emi_cam_ctx.rgb_buff) {
        free(emi_cam_ctx.rgb_buff);
        emi_cam_ctx.rgb_buff = NULL;
    }
    if (emi_cam_ctx.dwarp_buff) {
        free(emi_cam_ctx.dwarp_buff);
        emi_cam_ctx.dwarp_buff = NULL;
    }
    if (emi_cam_ctx.scale_buff) {
        free(emi_cam_ctx.scale_buff);
        emi_cam_ctx.scale_buff = NULL;
    }
    if (emi_cam_ctx.flip_buff) {
        free(emi_cam_ctx.flip_buff);
        emi_cam_ctx.flip_buff = NULL;
    }

    EMI_CSI_OPS(p_stop, 0);
    //EMI_CSI_OPS(p_deinit, 0);
}

static int emirror_camera_g2dlite_init()
{
    hal_g2dlite_init(&emi_cam_ctx.g2dlite_handle);
    if (!emi_cam_ctx.g2dlite_handle)
        return -1;
    return 0;
}

static void emirror_camera_fps_callback(void* argument)
{
    if (argument) {
        osEventFlagsId_t sig = (osEventFlagsId_t)argument;
        osEventFlagsSet(sig, 1);
    }
}

static int emirror_camera_vdsp_init()
{
    APP_INFO("");
    int ret = 0;
    if (!ret) {
        ret |= utils_vdsp_stall();
    }
    if (!ret) {
        ret |= utils_vdsp_core_reset();
    }
    if (!ret) {
        ret |= utils_vdsp_load_mapping_tbl(MAP_TLB_PATH, &emi_cam_ctx.vdsp_map_tbl);
    }
    if (!ret) {
        ret |= utils_vdsp_load_firmware(VDSP_ELF_PATH);
    }
    if (!ret) {
        ret |= utils_vdsp_go();
    }
    if (!ret) {
        sdrv_mbox_chan_req_t ch_req;
        ch_req.rproc = MBOX_RPROC_VDSP;
        ch_req.src_addr = SDRV_MBOX_ANY_ADDR;
        ch_req.dest_addr = MBOX_ADDR_LEGACY;
        emi_cam_ctx.vdsp_mb_chan = mbox_request_channel(g_mbox_dev, (void*)&ch_req);
        APP_INFO("emi_cam_ctx.vdsp_mb_chan:%p, callback:%p", emi_cam_ctx.vdsp_mb_chan, emirror_camera_vdsp_signal);
        if (emi_cam_ctx.vdsp_mb_chan) {
            emi_cam_ctx.vdsp_sig = osEventFlagsNew(NULL);
            osEventFlagsClear(emi_cam_ctx.vdsp_sig, 1);
            mbox_set_callback(emi_cam_ctx.vdsp_mb_chan, emirror_camera_vdsp_signal, (void*)emi_cam_ctx.vdsp_sig);
        }
        ret = emi_cam_ctx.vdsp_mb_chan ? 0 : -1;
    }
    if (ret) {
        goto err;
    }
    return 0;

err:
    return -1;
}

static int emirror_camera_init(emirror_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        sdm_display_t* left_disp = arb_get_slave_handle(arb_magic_video, LEFT_MIRROR);
        sdm_display_t* right_disp = arb_get_slave_handle(arb_magic_video, RIGHT_MIRROR);
        if (!left_disp || !right_disp) {
            APP_ERR("get disp fail, left:%p, right:%p", left_disp, right_disp);
            goto err;
        }

        utils_get_sdm_disp_rect(left_disp, &emi_cam_ctx.left_w, &emi_cam_ctx.left_h);
        utils_get_sdm_disp_rect(right_disp, &emi_cam_ctx.right_w, &emi_cam_ctx.right_h);
        int ret = emirror_camera_csi_init();
        if (ret) {
            APP_ERR("csi init fail");
            goto err;
        }

        ret = emirror_camera_g2dlite_init();
        if (ret) {
            APP_ERR("g2dlite init fail");
            goto err;
        }

        ret = emirror_camera_vdsp_init();
        if (ret) {
            APP_ERR("vdsp init fail");
            goto err;
        }

        emirror_rect_t rect[CAM_NUM];
        uint32_t w = emi_cam_ctx.csi_buff.w, h = emi_cam_ctx.csi_buff.h/4;
        for (int i = 0; i < CAM_NUM; i++) {
            rect[i].w = w*8/10;
            rect[i].h = h*8/10;
            rect[i].x = (w-rect[i].w)/2;
            rect[i].y = (h-rect[i].h)/2;
            emirror_camera_set_roi(i, &rect[i]);
        }

        return 0;
    }

err:
    return -1;
}

static void emirror_camera_entry(emirror_task_token_t* task)
{
    APP_INFO("");
    int ret = 0;
    emi_csi_buff_t* csi_buf = &emi_cam_ctx.csi_buff;
    sdm_display_t* left_disp = arb_get_slave_handle(arb_magic_video, LEFT_MIRROR);
    sdm_display_t* right_disp = arb_get_slave_handle(arb_magic_video, RIGHT_MIRROR);

    ret = emirror_camera_vdsp_set_mapping_table();
    if (ret) {
        APP_ERR("vdsp set mappling table fail");
        return;
    }

    ret = EMI_CSI_OPS(p_start, 0);
    if (ret) {
        return;
    }

    int wait_fps = 0;
    osEventFlagsId_t fps_sig = osEventFlagsNew(NULL);
    osEventFlagsClear(fps_sig, 1);
    osTimerId_t fps_timer = osTimerNew(emirror_camera_fps_callback, osTimerPeriodic, (void*)fps_sig, NULL);
    if (fps_sig && fps_timer) {
        osStatus_t ret = osTimerStart(fps_timer, 1000/csi_buf->fps);
        wait_fps = (ret == osOK) ? 1 : 0;
    }

    while (task && task->running) {
        int fail_cnt = 0;
        struct vdev_buffer cam_buf;

        do {
            ret = EMI_CSI_OPS(p_getbuf, 0, &cam_buf);
        } while (fail_cnt++ < 25 && ret);
        if (ret) {
            APP_ERR("read csi timeout");
            break;
        }

        uint32_t pin[CAM_NUM], pout[CAM_NUM];
        uint32_t prgb[CAM_NUM], pflip[CAM_NUM], pdwarp[CAM_NUM], pscale[CAM_NUM], pblend[CAM_NUM];
        uint32_t w = csi_buf->w, h = csi_buf->h/4;
        uint32_t disp_w = emi_cam_ctx.left_w, disp_h = emi_cam_ctx.left_h;
        pin[CAM_LEFT] = (unsigned long)(ap2p((paddr_t)cam_buf.paddr[0]));
        pin[CAM_RIGHT] = pin[CAM_LEFT]+w*h*csi_buf->pix_bytes;
        prgb[CAM_LEFT] = (unsigned long)emi_cam_ctx.rgb_buff;
        prgb[CAM_RIGHT] = prgb[CAM_LEFT]+w*h*RGB;
        pflip[CAM_LEFT] = (unsigned long)emi_cam_ctx.flip_buff;
        pflip[CAM_RIGHT] = pflip[CAM_LEFT]+w*h*RGB;
        pdwarp[CAM_LEFT] = (unsigned long)emi_cam_ctx.dwarp_buff;
        pdwarp[CAM_RIGHT] = pdwarp[CAM_LEFT]+w*h*RGB;
        pscale[CAM_LEFT] = (unsigned long)emi_cam_ctx.scale_buff;
        pscale[CAM_RIGHT] = pscale[CAM_LEFT]+w*h*RGB;
        pblend[CAM_LEFT] = (unsigned long)emi_cam_ctx.output_buff[CAM_LEFT];
        pblend[CAM_RIGHT] = (unsigned long)emi_cam_ctx.output_buff[CAM_RIGHT];
        pout[CAM_LEFT] = pblend[CAM_LEFT];
        pout[CAM_RIGHT] = pblend[CAM_RIGHT];
        APP_DBG("pin:0x%08x, 0x%08x", pin[CAM_LEFT], pin[CAM_RIGHT]);

        /* L1. yuv -> rgb */
        ret = emirror_camera_yuv2rgb(pin[CAM_LEFT], prgb[CAM_LEFT]);
        if (ret) {
            APP_ERR("left yuv convert to rgb fail");
            break;
        }

        /* L2. horizontal flip */
        ret = emirror_camera_hflip(prgb[CAM_LEFT], pflip[CAM_LEFT]);
        if (ret) {
            APP_ERR("left horizontal flip fail");
            break;
        }

        /* L3. dewarp */
        ret = emirror_camera_vdsp_start_one_frame(pflip[CAM_LEFT], pdwarp[CAM_LEFT]);
        if (ret) {
            APP_ERR("left decrawp fail");
            goto waiting_fps;
        }

        /* R1. yuv -> rgb */
        ret = emirror_camera_yuv2rgb(pin[CAM_RIGHT], prgb[CAM_RIGHT]);
        if (ret) {
            APP_ERR("right yuv convert to rgb fail");
            break;
        }

        /* R2. horizontal flip */
        ret = emirror_camera_hflip(prgb[CAM_RIGHT], pflip[CAM_RIGHT]);
        if (ret) {
            APP_ERR("right horizontal flip fail");
            break;
        }

        /* L4. wait vdsp */
        ret = emirror_camera_vdsp_wait_signal(100);
        if (ret) {
            APP_ERR("left wait vdsp signal timeout");
            break;
        }

        /* L5. scale & merge */
        emirror_rect_t left_scale_0[2] = { // cam
            [0] = { .x = 0, .y = 0, .w = w, .h = h }, /* src */
            [1] = { .x = w, .y = 0, .w = (disp_w-w), .h = ((disp_w-w)*h)/w }, /* dest */
        };
        emirror_rect_t left_scale_1[2] = { // mirror
            [0] = { .x = 0, .y = 0, .w = w, .h = h }, /* src */
            [1] = { .x = 0, .y = 0, .w = w, .h = h }, /* dest */
        };
        emirror_camera_get_roi(CAM_LEFT, &left_scale_1[0]);
        ret =  emirror_camera_scale(pdwarp[CAM_LEFT], pscale[CAM_LEFT], left_scale_1, w, h);
        ret &= emirror_camera_blend(pin[CAM_LEFT], pscale[CAM_LEFT], pblend[CAM_LEFT], left_scale_0,  left_scale_1, disp_w, disp_h);
        if (ret) {
            APP_ERR("left blend fail");
            break;
        }

        /* L6. post */
        emirror_vstream_t left_vstream = {
            .disp_handle = left_disp,
            .addr = pout[CAM_LEFT],
            .cam_w = disp_w,
            .cam_h = disp_h,
            .stride = disp_w*RGB,
        };
        issue_wait_async_t left_issue_wait = issue_token_send_one_async(task->issue, "left", &left_vstream, 100);
        if (!left_issue_wait.issue || !left_issue_wait.container)
            APP_ERR("post buff to left player fail");
        //APP_INFO("post buff to left");

        /* R3. dewarp */
        ret = emirror_camera_vdsp_start_one_frame(pflip[CAM_RIGHT], pdwarp[CAM_RIGHT]);
        if (ret) {
            APP_ERR("right decrawp fail");
            goto waiting_fps;
        }

        /* R4. wait vdsp */
        ret = emirror_camera_vdsp_wait_signal(100);
        if (ret) {
            APP_ERR("right wait vdsp signal timeout");
            break;
        }

        /* R5. cropt */
        emirror_rect_t right_scale_0[2] = { // cam
            [0] = { .x = 0, .y = 0, .w = w, .h = h }, /* src */
            [1] = { .x = w, .y = 0, .w = (disp_w-w), .h = ((disp_w-w)*h)/w }, /* dest */
        };
        emirror_rect_t right_scale_1[2] = { // mirror
            [0] = { .x = 0, .y = 0, .w = w, .h = h }, /* src */
            [1] = { .x = 0, .y = 0, .w = w, .h = h }, /* dest */
        };
        emirror_camera_get_roi(CAM_RIGHT, &right_scale_1[0]);
        ret =  emirror_camera_scale(pdwarp[CAM_RIGHT], pscale[CAM_RIGHT], right_scale_1, w, h);
        ret &= emirror_camera_blend(pin[CAM_RIGHT], pscale[CAM_RIGHT], pblend[CAM_RIGHT], right_scale_0,  right_scale_1, disp_w, disp_h);
        if (ret) {
            APP_ERR("right blend fail");
            break;
        }

        /* R6. post */
        emirror_vstream_t right_vstream = {
            .disp_handle = right_disp,
            .addr = pout[CAM_RIGHT],
            .cam_w = disp_w,
            .cam_h = disp_h,
            .stride = disp_w*RGB,
        };
        issue_wait_async_t right_issue_wait = issue_token_send_one_async(task->issue, "right", &right_vstream, 100);
        if (!right_issue_wait.issue || !right_issue_wait.container)
            APP_ERR("post buff to right player fail");
        //APP_INFO("post buff to right");

        /* wait dereferenced */
        ret = issue_token_wait_one_dereferenced(&left_issue_wait, &task->running);
        if (ret) APP_ERR("wait left player dereferenced fail");
        ret = issue_token_wait_one_dereferenced(&right_issue_wait, &task->running);
        if (ret) APP_ERR("wait right player dereferenced fail");

        EMI_CSI_OPS(p_setbuf, 0, &cam_buf);

waiting_fps:
        if (wait_fps) {
            osEventFlagsWait(fps_sig, 1, osFlagsWaitAny, 1000/csi_buf->fps);
        }
        else {
            osDelay(40);
        }
    }
    if (fps_timer) {
        osTimerDelete(fps_timer);
        fps_timer = NULL;
    }
    if (fps_sig) {
        osEventFlagsDelete(fps_sig);
        fps_sig = NULL;
    }
}

static void emirror_camera_exit(emirror_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        emirror_camera_csi_deinit();
        osEventFlagsSet(task->terminate_sig, EMIRROR_MASK(task->id));
        msg_t msg = { .msgid = emirror_evt_stop, .param = task->id };
        emirror_set_evt(&msg);
    }
    osThreadExit();
}

void emirror_camera_task(void* args)
{
    APP_INFO("");
    if (args) {
        emirror_task_token_t* task = (emirror_task_token_t*)args;
        if (!emirror_camera_init(task)) {
            emirror_camera_entry(task);
        }
        emirror_camera_exit(task);
    }
}

void emirror_camera_get_roi(int cam, emirror_rect_t* rect)
{
    if (cam < CAM_NUM && rect) {
        rect->x = emi_roi[cam].x;
        rect->y = emi_roi[cam].y;
        rect->w = emi_roi[cam].w;
        rect->h = emi_roi[cam].h;
    }
}

void emirror_camera_set_roi(int cam, emirror_rect_t* rect)
{
    if (cam < CAM_NUM && rect) {
        memcpy(&emi_roi[cam], rect, sizeof(emirror_rect_t));
        APP_INFO("emi_roi[%s]: x:%d, y:%d, w:%d, h:%d",
                cam==CAM_LEFT?"left":"right",
                emi_roi[cam].x, emi_roi[cam].y, emi_roi[cam].w, emi_roi[cam].h);
    }
}

void emirror_camera_adj_roi(int cam, int direction)
{
    uint32_t offset = 10;
    uint32_t w = emi_roi[cam].w, h = emi_roi[cam].h;
    uint32_t min_x = 0, min_y = 0;
    uint32_t max_x = emi_cam_ctx.csi_buff.w-w, max_y = emi_cam_ctx.csi_buff.h/4-h;
    if (cam < CAM_NUM) {
        switch (direction) {
        case 0: /* left */
            {
                if (emi_roi[cam].x > offset) {
                    emi_roi[cam].x = emi_roi[cam].x - offset;
                }
                else {
                    emi_roi[cam].x = 0;
                }
                break;
            }
        case 1: /* right */
            {
                if (emi_roi[cam].x + offset >= max_x) {
                    emi_roi[cam].x = max_x;
                }
                else {
                    emi_roi[cam].x = emi_roi[cam].x + offset;
                }
                break;
            }
        case 2: /* up */
            {
                if (emi_roi[cam].y > offset) {
                    emi_roi[cam].y = emi_roi[cam].y - offset;
                }
                else {
                    emi_roi[cam].y = 0;
                }
                break;
            }
        case 3: /* down */
            {
                if (emi_roi[cam].y + offset >= max_y) {
                    emi_roi[cam].y = max_y;
                }
                else {
                    emi_roi[cam].y = emi_roi[cam].y + offset;
                }
                break;
            }
        default:
            break;
        }
        APP_INFO("%s: x:%d, y:%d", cam==CAM_LEFT?"left":"right", emi_roi[cam].x, emi_roi[cam].y);
    }
}

/* EOF */
