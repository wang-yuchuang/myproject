/**
 * @file cluster_lvgl.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv cluster application.
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <lvgl_gui.h>
#include <param.h>
#include <malloc.h>
#include <res_loader.h>
#include <sdm_display.h>
#include <input_service.h>
#include <lv_gpu_sdrv_dma2d.h>
#include <disp_data_type.h>
#include <sd_graphics.h>
#include <board.h>
#include "cluster_def.h"
#include "cluster_cfg.h"
#include "cluster.h"
#include "app_log.h"
#include "disp_utility.h"
#include "arbitrator.h"
#include "launcher.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cluster"

#define CLS_SDMBUF_TEMPLATE { \
    {\
    .layer = 0, \
    .layer_dirty = 0, \
    .layer_en = 1, \
    .fmt = COLOR_ARGB8888, \
    .src = { 0, 0, 1920, 720 }, \
    .addr = { 0, 0, 0, 0 }, \
    .src_stride = { 7680, 0, 0, 0 }, \
    .start = { 0, 0, 1920, 720 }, \
    .dst = { 0, 0, 1920, 720 }, \
    .ckey_en = 0, \
    .ckey = 0, \
    .alpha_en = 1, \
    .alpha = 0xff, \
    .z_order = 0, \
    .security = 0 \
    }, \
    {\
    .layer = 1, \
    .layer_dirty = 0, \
    .layer_en = 1, \
    .fmt = COLOR_ARGB8888, \
    .src = { 0, 0, 1920, 720 }, \
    .addr = { 0, 0, 0, 0 }, \
    .src_stride = { 7680, 0, 0, 0 }, \
    .start = { 0, 0, 1920, 720 }, \
    .dst = { 0, 0, 1920, 720 }, \
    .ckey_en = 0, \
    .ckey = 0, \
    .alpha_en = 0, \
    .alpha = 0, \
    .z_order = 1, \
    .security = 0 \
    }, \
}

#define CLS_SET_AREA(a, x, y, w, h) \
        a.x1 = x; \
        a.y1 = y; \
        a.x2 = x+w-1; \
        a.y2 = y+h-1;

#define CLS_BG_PATH         "S:/early_app/cluster/cluster_bg.png"
#define CLS_RPM_PATH        "S:/early_app/cluster/rpm_bg_number.png"
#define CLS_SPEED_PATH      "S:/early_app/cluster/speed_bg_number.png"
#define CLS_NEEDLE_PATH     "S:/early_app/cluster/needle.png"
#define CLS_LEFT_TURN_PATH  "S:/early_app/cluster/LeftTurn.png"
#define CLS_RIGHT_TURN_PATH "S:/early_app/cluster/rightTurn.png"
#define CLS_RES_PATH        "S:/early_app/cluster/res.bin"
#define CLS_RPM_SMALL_PATH  "S:/early_app/cluster/rpm_bg_small.png"
#define CLS_SPEED_SMALL_PATH "S:/early_app/cluster/speed_bg_small.png"
#define CLS_TOP_BAR_PATH    "S:/early_app/cluster/top_bar.png"
#define CLS_BOTTOM_BAR_PATH "S:/early_app/cluster/botom_bar.png"

#define CLS_FPS             (60)
#define CLS_TURN_CNT        (30)
#define CLS_NEEDLE_START    (1<<0)
#define CLS_NEEDLE_END      (1<<1)
#define CLS_NEEDLE_SIG_MASK (CLS_NEEDLE_START|CLS_NEEDLE_END)

#define number_size         (32)
#define RES_DATA_SIZE       (number_size*number_size*4*11)

static sdm_display_t* cls_disp;
static uint32_t disp_w;
static uint32_t disp_h;
static const lv_color_t blank_color = { .full = 0x00ff0000 };
static int needle_ani = 0;
osEventFlagsId_t needle_sig = NULL; /* bit0:start, bit1:end */

static uint8_t* frame_buff[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
static SD1_POINT_T* point = NULL;

static lv_img_decoder_dsc_t bg_dsc;
static lv_img_decoder_dsc_t rpm_dsc;
static lv_img_decoder_dsc_t speed_dsc;
static lv_img_decoder_dsc_t needle_dsc;
static lv_img_decoder_dsc_t left_dsc;
static lv_img_decoder_dsc_t right_dsc;
static lv_img_decoder_dsc_t bottom_bar_dsc;

static int rpm_w = 600;
static int rpm_h = 600;
static int rpm_x = 104;
static int rpm_y = 76;
static int rpm_cx = 404;
static int rpm_cy = 376;
static int speed_w = 600;
static int speed_h = 600;
static int speed_x = 1216;
static int speed_y = 76;
static int speed_cx = 1516;
static int speed_cy = 376;
static int bottombar_x = 480;
static int bottombar_y = 601;
static int bottombar_w = 960;
static int bottombar_h = 119;
static int leftturn_x = 440;
static int leftturn_y = 10;
static int leftturn_w = 44;
static int leftturn_h = 37;
static int rightturn_x = 1436;
static int rightturn_y = 10;
static int rightturn_w = 44;
static int rightturn_h = 37;
static int needle_x = 288;
static int needle_y = 64;
static int needle_w = 24;
static int needle_h = 256;
static int needle_start_angle = -1350;
static int needle_end_angle = 1350;
static int needle_curr_angle = -1350;

static lv_area_t full_area;
static lv_area_t bg_area;
static lv_area_t rpm_area;
static lv_area_t rpm_draw_area;
static lv_area_t speed_area;
static lv_area_t speed_draw_area;
static lv_area_t bottombar_area;
static lv_area_t bottombar_draw_area;
static lv_area_t leftturn_area;
static lv_area_t leftturn_draw_area;
static lv_area_t rightturn_area;
static lv_area_t rightturn_draw_area;

static inline int cluster_lvgl_img_decoder_open()
{
    APP_INFO("");
    lv_res_t ret;
    ret = lv_img_decoder_open(&bg_dsc, CLS_BG_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || bg_dsc.img_data == NULL) {
        APP_ERR("open bg decoder fail, ret:%d, bg_dsc.img_data:%p", ret, bg_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&rpm_dsc, CLS_RPM_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || rpm_dsc.img_data == NULL) {
        APP_ERR("open rpm decoder fail, ret:%d, bg_dsc.img_data:%p", ret, rpm_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&speed_dsc, CLS_SPEED_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || speed_dsc.img_data == NULL) {
        APP_ERR("open speed decoder fail, ret:%d, bg_dsc.img_data:%p", ret, speed_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&needle_dsc, CLS_NEEDLE_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || needle_dsc.img_data == NULL) {
        APP_ERR("open needle decoder fail, ret:%d, bg_dsc.img_data:%p", ret, needle_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&left_dsc, CLS_LEFT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || left_dsc.img_data == NULL) {
        APP_ERR("open left trun decoder fail, ret:%d, bg_dsc.img_data:%p", ret, left_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&right_dsc, CLS_RIGHT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || right_dsc.img_data == NULL) {
        APP_ERR("open right trun decoder fail, ret:%d, bg_dsc.img_data:%p", ret, right_dsc.img_data);
        goto err;
    }
    ret = lv_img_decoder_open(&bottom_bar_dsc, CLS_BOTTOM_BAR_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || bottom_bar_dsc.img_data == NULL) {
        APP_ERR("open bottom bar decoder fail, ret:%d, bg_dsc.img_data:%p", ret, bottom_bar_dsc.img_data);
        goto err;
    }
    return 0;

err:
    return -1;
}

static inline void cluster_lvgl_disp_show(cluster_task_token_t* task, sdm_display_t* disp, uint8_t* bg_buf, uint8_t* fg_buf)
{
    if (task && disp && disp->handle && bg_buf && fg_buf) {
        if (task->disp_sts&CLUSTER_MASK(CLUSTER)) {
            struct sdm_buffer sdm_buf[2] = CLS_SDMBUF_TEMPLATE;
            struct sdm_post_config post_data;
            memset(&post_data, 0, sizeof(struct sdm_post_config));
            sdm_buf[0].addr[0] = (unsigned long)bg_buf;
            sdm_buf[1].addr[0] = (unsigned long)fg_buf;
            sdm_buf[0].src_stride[0] = disp_w*4;
            sdm_buf[1].src_stride[0] = disp_w*4;
            post_data.bufs = &sdm_buf[0];
            post_data.n_bufs = 2;
            sdm_post(disp->handle, &post_data);
        }
    }
}

static inline void cluster_lvgl_disp_clear(cluster_task_token_t* task, sdm_display_t* disp)
{
    if (task && disp && disp->handle) {
        if (task->disp_sts&CLUSTER_MASK(CLUSTER)) {
            sdm_clear_display(disp->handle);
        }
    }
}

static void cluster_lvgl_fps_callback(void* argument)
{
    if (argument) {
        //APP_LOG("%ld", current_time());
        osEventFlagsId_t sig = (osEventFlagsId_t)argument;
        osEventFlagsSet(sig, 1);
    }
}

static void cluster_lvgl_draw_needle(void* act_buf)
{
    if (act_buf) {
        SD_SIZE_T dst_size = { disp_w, disp_h };
        SD_SIZE_T src_size = { needle_w, needle_h };
        SD_POINT_T src_center = { needle_w/2, needle_h-14 };
        int32_t z_param = 4096;
        SD_POINT_T rpm_dest_center = { rpm_cx, rpm_cy };
        SD_POINT_T spd_dest_center = { speed_cx, speed_cy };
        memset(point, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_rotation(dst_size, src_size, rpm_dest_center, src_center, needle_curr_angle, (uint32_t*)needle_dsc.img_data, (uint32_t*)act_buf, z_param, point);
        memset(point, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_rotation(dst_size, src_size, spd_dest_center, src_center, -needle_curr_angle, (uint32_t*)needle_dsc.img_data, (uint32_t*)act_buf, z_param, point);
    }
}

static inline int cluster_lvgl_init(cluster_task_token_t* task)
{
    APP_INFO("");
    cls_disp = arb_get_slave_handle(arb_magic_video, CLUSTER);
    if (cls_disp) utils_get_sdm_disp_rect(cls_disp, &disp_w, &disp_h);
    lvgl_init();
    for (int i = 0; i < ARRAY_SIZE(frame_buff); i++) {
        frame_buff[i] = memalign(0x1000, disp_w*disp_h*4);
        memset(frame_buff[i], 0, disp_w*disp_h*4);
        if (!frame_buff[i]) {
            APP_ERR("memalign frame_buff fail");
            goto err;
        }
    }
    point = memalign(0x1000, 10*1024*sizeof(SD1_POINT_T));
    if (!point) {
        APP_ERR("malloc point fail");
        goto err;
    }

    if (cluster_lvgl_img_decoder_open()) {
        APP_ERR("lvgl img decoer open fail");
        goto err;
    }

    rpm_w = rpm_dsc.header.w;
    rpm_h = rpm_dsc.header.h;
    speed_w = speed_dsc.header.w;
    speed_h = speed_dsc.header.h;
    bottombar_w = bottom_bar_dsc.header.w;
    bottombar_h = bottom_bar_dsc.header.h;
    leftturn_w = left_dsc.header.w;
    leftturn_h = left_dsc.header.h;
    rightturn_w = right_dsc.header.w;
    rightturn_h = right_dsc.header.h;
    needle_w = needle_dsc.header.w;
    needle_h = needle_dsc.header.h;

    CLS_SET_AREA(full_area, 0, 0, disp_w, disp_h);
    CLS_SET_AREA(bg_area, 0, 0, disp_w, disp_h);
    CLS_SET_AREA(rpm_area, 0, 0, rpm_w, rpm_h);
    CLS_SET_AREA(rpm_draw_area, rpm_x, rpm_y, rpm_w, rpm_h);
    CLS_SET_AREA(speed_area, 0, 0, speed_w, speed_h);
    CLS_SET_AREA(speed_draw_area, speed_x, speed_y, speed_w, speed_h);
    CLS_SET_AREA(bottombar_area, 0, 0, bottombar_w, bottombar_h);
    CLS_SET_AREA(bottombar_draw_area, bottombar_x, bottombar_y, bottombar_w, bottombar_h);
    CLS_SET_AREA(leftturn_area, 0, 0, leftturn_w, leftturn_h);
    CLS_SET_AREA(leftturn_draw_area, leftturn_x, leftturn_y, leftturn_w, leftturn_h);
    CLS_SET_AREA(rightturn_area, 0, 0, rightturn_w, rightturn_h);
    CLS_SET_AREA(rightturn_draw_area, rightturn_x, rightturn_y, rightturn_w, rightturn_h);

    return 0;

err:
    return -1;
}

static inline void cluster_lvgl_entry(cluster_task_token_t* task)
{
    APP_INFO("");
    int bg_changed = 1;
    int fg_changed = 1;
    int turnlight_show = 1;
    int wait_fps = 0;
    int frame_cnt = 0;
    int needle_direct = 0; /* 0:up, 1:down */
    osEventFlagsId_t fps_sig = osEventFlagsNew(NULL);
    osEventFlagsClear(fps_sig, 1);
    osTimerId_t fps_timer = osTimerNew(cluster_lvgl_fps_callback, osTimerPeriodic, (void*)fps_sig, NULL);
    if (fps_timer && fps_sig) {
        osStatus_t ret = osTimerStart(fps_timer, 1000/CLS_FPS);
        wait_fps = (ret == osOK) ? 1 : 0;
    }
    uint8_t* act_fg_buf = frame_buff[0];
    uint8_t* act_bg_buf = frame_buff[1];
    uint8_t* needle_buff = frame_buff[2];
    uint8_t* bg_buff = frame_buff[3];
    uint8_t* blank_buff = frame_buff[4];
    uint8_t* turnlight_buff = frame_buff[5];

    while (task && task->running) {
        if (bg_changed) {
            bg_changed = 0;
            /* background */
            lv_gpu_sdrv_g2d_copy((lv_color_t*)(bg_buff), disp_w, (lv_color_t*)(bg_dsc.img_data), disp_w, disp_w, disp_h);
            /* rpm meter */
            lv_gpu_sdrv_g2d_blend_scale((lv_color_t*)act_bg_buf, disp_w*4,
                                        (lv_color_t*)rpm_dsc.img_data, rpm_w*4,
                                        (lv_color_t*)bg_buff, disp_w*4,
                                        &full_area,
                                        &rpm_area, &rpm_draw_area,
                                        &bg_area, &full_area);
            /* speed meter */
            lv_gpu_sdrv_g2d_blend_scale((lv_color_t*)act_bg_buf, disp_w*4,
                                        (lv_color_t*)speed_dsc.img_data, speed_w*4,
                                        (lv_color_t*)act_bg_buf, disp_w*4,
                                        &full_area,
                                        &speed_area, &speed_draw_area,
                                        &bg_area, &full_area);
            /* bottom bar */
            lv_gpu_sdrv_g2d_blend_scale((lv_color_t*)act_bg_buf, disp_w*4,
                                        (lv_color_t*)bottom_bar_dsc.img_data, bottombar_w*4,
                                        (lv_color_t*)act_bg_buf, disp_w*4,
                                        &full_area,
                                        &bottombar_area, &bottombar_draw_area,
                                        &bg_area, &full_area);
        }
        if (fg_changed) {
            fg_changed = 0;
            lv_gpu_sdrv_g2d_copy((lv_color_t*)(act_fg_buf), disp_w, (lv_color_t*)(blank_buff), disp_w, disp_w, disp_h);
            if (turnlight_show) {
                lv_gpu_sdrv_g2d_copy((lv_color_t*)(act_fg_buf)+leftturn_y*disp_w+leftturn_x, disp_w, (lv_color_t*)(left_dsc.img_data), leftturn_w, leftturn_w, leftturn_h);
                lv_gpu_sdrv_g2d_copy((lv_color_t*)(act_fg_buf)+rightturn_y*disp_w+rightturn_x, disp_w, (lv_color_t*)(right_dsc.img_data), rightturn_w, rightturn_w, rightturn_h);
            }
        }
        cluster_lvgl_draw_needle(act_fg_buf);
        cluster_lvgl_disp_show(task, cls_disp, act_bg_buf, act_fg_buf);
        frame_cnt++;
        if (frame_cnt%25 == 0) {
            turnlight_show = !turnlight_show;
            frame_cnt = 0;
            fg_changed = 1;
            //APP_LOG("turn light blink:%d, %ld", turnlight_show, current_time());
        }
        if (!needle_direct) { /* up */
            //fg_changed = 1;
            needle_curr_angle += 10;
            if (needle_curr_angle >= needle_end_angle) {
                needle_curr_angle = needle_end_angle;
                needle_direct = !needle_direct;
            }
        }
        else { /* down */
            //fg_changed = 1;
            needle_curr_angle -= 10;
            if (needle_curr_angle <= needle_start_angle) {
                needle_curr_angle = needle_start_angle;
                needle_direct = !needle_direct;
            }
        }
        if (wait_fps) {
            osEventFlagsWait(fps_sig, 1, osFlagsWaitAny, 1000/CLS_FPS);
        }
        else {
            osDelay(16);
        }
    }
    cluster_lvgl_disp_clear(task ,cls_disp);
    osEventFlagsDelete(fps_sig);
    fps_sig = NULL;
    osTimerDelete(fps_timer);
    fps_timer = NULL;
}

static inline void cluster_lvgl_exit(cluster_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        for (int i = 0; i < ARRAY_SIZE(frame_buff); i++) {
            if (frame_buff[i]) {
                free(frame_buff[i]);
                frame_buff[i] = NULL;
            }
        }
        if (point) {
            free(point);
            point = NULL;
        }
        lv_img_decoder_close(&bg_dsc);
        lv_img_decoder_close(&rpm_dsc);
        lv_img_decoder_close(&speed_dsc);
        lv_img_decoder_close(&needle_dsc);
        lv_img_decoder_close(&left_dsc);
        lv_img_decoder_close(&right_dsc);
        lv_img_decoder_close(&bottom_bar_dsc);
        osEventFlagsSet(task->terminate_sig, CLUSTER_MASK(task->id));
        msg_t msg = { .msgid = cluster_evt_stop, .param = task->id };
        cluster_set_evt(&msg);
    }
    osThreadExit();
}

void cluster_lvgl_task(void* args)
{
    APP_INFO("");
    if (args) {
        cluster_task_token_t* task = (cluster_task_token_t*)args;
        if (!cluster_lvgl_init(task)) {
            cluster_lvgl_entry(task);
        }
        cluster_lvgl_exit(task);
    }
}
/* EOF */
