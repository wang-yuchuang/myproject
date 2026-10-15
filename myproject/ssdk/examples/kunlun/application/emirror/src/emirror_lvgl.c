/**
 * @file emirror_lvgl.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "emirror"

#include <stdlib.h>
#include <lvgl_gui.h>
#include <param.h>
#include <malloc.h>
#include <res_loader.h>
#include <sdm_display.h>
#include <input_service.h>
#include "emirror_def.h"
#include "emirror_cfg.h"
#include "emirror.h"
#include "app_log.h"
#include "disp_utility.h"
#include "arbitrator.h"
#include "launcher.h"

#define EMIRROR_SWITCH_H    (80)
#define EMIRROR_SWITCH_W    (80)

#ifdef CONFIG_MP
#define EM_BTN_L_BACK_CAM_PATH       "S:/early_app/fastrvm_mp/l_back_camera.png"
#define EM_BTN_L_BACK_CAM_FOCUS_PATH "S:/early_app/fastrvm_mp/l_back_camera_focus.png"
#define EM_BTN_R_BACK_CAM_PATH       "S:/early_app/fastrvm_mp/r_back_camera.png"
#define EM_BTN_R_BACK_CAM_FOCUS_PATH "S:/early_app/fastrvm_mp/r_back_camera_focus.png"
#define EM_BTN_LF_PATH               "S:/early_app/fastrvm_mp/direction_left.png"
#define EM_BTN_LF_FOCUS_PATH         "S:/early_app/fastrvm_mp/direction_left_focus.png"
#define EM_BTN_RT_PATH               "S:/early_app/fastrvm_mp/direction_right.png"
#define EM_BTN_RT_FOCUS_PATH         "S:/early_app/fastrvm_mp/direction_right_focus.png"
#define EM_BTN_UP_PATH               "S:/early_app/fastrvm_mp/direction_up.png"
#define EM_BTN_UP_FOCUS_PATH         "S:/early_app/fastrvm_mp/direction_up_focus.png"
#define EM_BTN_DN_PATH               "S:/early_app/fastrvm_mp/direction_down.png"
#define EM_BTN_DN_FOCUS_PATH         "S:/early_app/fastrvm_mp/direction_down_focus.png"
#else
#define EM_BTN_L_BACK_CAM_PATH       "S:/early_app/fastrvm/l_back_camera.png"
#define EM_BTN_L_BACK_CAM_FOCUS_PATH "S:/early_app/fastrvm/l_back_camera_focus.png"
#define EM_BTN_R_BACK_CAM_PATH       "S:/early_app/fastrvm/r_back_camera.png"
#define EM_BTN_R_BACK_CAM_FOCUS_PATH "S:/early_app/fastrvm/r_back_camera_focus.png"
#define EM_BTN_LF_PATH               "S:/early_app/fastrvm/direction_left.png"
#define EM_BTN_LF_FOCUS_PATH         "S:/early_app/fastrvm/direction_left_focus.png"
#define EM_BTN_RT_PATH               "S:/early_app/fastrvm/direction_right.png"
#define EM_BTN_RT_FOCUS_PATH         "S:/early_app/fastrvm/direction_right_focus.png"
#define EM_BTN_UP_PATH               "S:/early_app/fastrvm/direction_up.png"
#define EM_BTN_UP_FOCUS_PATH         "S:/early_app/fastrvm/direction_up_focus.png"
#define EM_BTN_DN_PATH               "S:/early_app/fastrvm/direction_down.png"
#define EM_BTN_DN_FOCUS_PATH         "S:/early_app/fastrvm/direction_down_focus.png"
#endif

enum {
    EM_SW_L_BACK_CAM = 0,
    EM_SW_R_BACK_CAM,
    EM_SW_LEFT,
    EM_SW_RIGHT,
    EM_SW_UP,
    EM_SW_DOWN,
    EM_SW_NUM
};

typedef struct _emirror_btn_ctx_t{
    lv_obj_t* parent;
    lv_obj_t* label;
    lv_obj_t* btn;
    lv_point_t refPos;
    lv_coord_t width;
    lv_coord_t height;
    lv_event_cb_t event_cb;
    char* text;
} emirror_btn_ctx_t;

static uint32_t select_mirror = 0xff;
static uint32_t disp_w;
static uint32_t disp_h;
static lv_obj_t* parent_screen = NULL;
static emirror_btn_ctx_t sw_btn[EM_SW_NUM];

static void emirror_l_back_cam_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        if (select_mirror != CAM_LEFT) {
            APP_INFO("select left");
            select_mirror = CAM_LEFT;
            lv_imgbtn_set_src(sw_btn[EM_SW_L_BACK_CAM].btn, LV_STATE_DEFAULT, EM_BTN_L_BACK_CAM_FOCUS_PATH);
            lv_imgbtn_set_src(sw_btn[EM_SW_R_BACK_CAM].btn, LV_STATE_DEFAULT, EM_BTN_R_BACK_CAM_PATH);
            lv_refr_now(get_display(LEFT_MIRROR));
        }
    }
}

static void emirror_r_back_cam_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        if (select_mirror != CAM_RIGHT) {
            APP_INFO("select right");
            select_mirror = CAM_RIGHT;
            lv_imgbtn_set_src(sw_btn[EM_SW_L_BACK_CAM].btn, LV_STATE_DEFAULT, EM_BTN_L_BACK_CAM_PATH);
            lv_imgbtn_set_src(sw_btn[EM_SW_R_BACK_CAM].btn, LV_STATE_DEFAULT, EM_BTN_R_BACK_CAM_FOCUS_PATH);
            lv_refr_now(get_display(LEFT_MIRROR));
        }
    }
}

static void emirror_d_left_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        APP_INFO("adjust left");
        emirror_camera_adj_roi(select_mirror, 0);
    }
}

static void emirror_d_right_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        APP_INFO("adjust right");
        emirror_camera_adj_roi(select_mirror, 1);
    }
}

static void emirror_d_up_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        APP_INFO("adjust up");
        emirror_camera_adj_roi(select_mirror, 2);
    }
}

static void emirror_d_down_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        APP_INFO("adjust down");
        emirror_camera_adj_roi(select_mirror, 3);
    }
}

static inline void emirror_set_btn_location(emirror_btn_ctx_t* obj, uint16_t x, uint16_t y,
                                                char* label, lv_event_cb_t cb,
                                                char* normal_pic, char* focus_pic)
{
    APP_INFO("");
    if (!obj)
        return;

    obj->parent = parent_screen;
    obj->refPos.x = x;
    obj->refPos.y = y;
    obj->width = EMIRROR_SWITCH_W;
    obj->height = EMIRROR_SWITCH_H;
    obj->text = label?label:"";
    obj->event_cb = cb;
    obj->btn = lv_imgbtn_create(obj->parent, NULL);
    lv_imgbtn_set_src(obj->btn, LV_STATE_DEFAULT, normal_pic);
    lv_imgbtn_set_src(obj->btn, LV_STATE_CHECKED, focus_pic);
    lv_obj_set_pos(obj->btn, obj->refPos.x, obj->refPos.y);
    lv_obj_set_size(obj->btn, obj->width, obj->height);
    lv_obj_set_event_cb(obj->btn, obj->event_cb);
}

static void emirror_create_switch(emirror_btn_ctx_t* emirror_btn)
{
    APP_INFO("");
    if (!emirror_btn)
        return;

    uint16_t xpos = 1280, ypos = 520;
    uint16_t offset = 40;
    memset(emirror_btn, 0, sizeof(emirror_btn_ctx_t)*EM_SW_NUM);
    emirror_set_btn_location(&emirror_btn[EM_SW_L_BACK_CAM], xpos+EMIRROR_SWITCH_W*0+offset*1, ypos,                         (char*)"L", emirror_l_back_cam_cb, (char*)EM_BTN_L_BACK_CAM_PATH, (char*)EM_BTN_L_BACK_CAM_FOCUS_PATH);
    emirror_set_btn_location(&emirror_btn[EM_SW_LEFT],       xpos+EMIRROR_SWITCH_W*1+offset*2, ypos,                         (char*)"<", emirror_d_left_cb,     (char*)EM_BTN_LF_PATH,         (char*)EM_BTN_LF_FOCUS_PATH);
    emirror_set_btn_location(&emirror_btn[EM_SW_UP],         xpos+EMIRROR_SWITCH_W*2+offset*3, ypos-EMIRROR_SWITCH_H-offset, (char*)"^", emirror_d_up_cb,       (char*)EM_BTN_UP_PATH,         (char*)EM_BTN_UP_FOCUS_PATH);
    emirror_set_btn_location(&emirror_btn[EM_SW_DOWN],       xpos+EMIRROR_SWITCH_W*2+offset*3, ypos+EMIRROR_SWITCH_H+offset, (char*)"v", emirror_d_down_cb,     (char*)EM_BTN_DN_PATH,         (char*)EM_BTN_DN_FOCUS_PATH);
    emirror_set_btn_location(&emirror_btn[EM_SW_RIGHT],      xpos+EMIRROR_SWITCH_W*3+offset*4, ypos,                         (char*)">", emirror_d_right_cb,    (char*)EM_BTN_RT_PATH,         (char*)EM_BTN_RT_FOCUS_PATH);
    emirror_set_btn_location(&emirror_btn[EM_SW_R_BACK_CAM], xpos+EMIRROR_SWITCH_W*4+offset*5, ypos,                         (char*)"R", emirror_r_back_cam_cb, (char*)EM_BTN_R_BACK_CAM_PATH, (char*)EM_BTN_R_BACK_CAM_FOCUS_PATH);
}

static int emirror_lvgl_init(emirror_task_token_t* task)
{
    APP_INFO("");
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, LEFT_MIRROR);
    if (disp) {
        utils_get_sdm_disp_rect(disp, &disp_w, &disp_h);
        lvgl_init();
        parent_screen = lv_disp_get_scr_act(get_display(LEFT_MIRROR));
        APP_INFO("parent_screen:%p", parent_screen);
        lv_obj_set_style_local_bg_opa(parent_screen, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        emirror_create_switch(sw_btn);
        return 0;
    }
    return -1;
}

static void emirror_lvgl_entry(emirror_task_token_t* task)
{
    APP_INFO("");
    int ret = -1;
    while (task && task->running) {
        //if (ret) ret = touch_service_send_msg(TS_MSG_AVM_START); /* only for debug in android display */
        if (task->disp_sts&EMIRROR_MASK(LEFT_MIRROR)) {
            lv_task_handler();
        }
        osDelay(50);
    }
    //touch_service_send_msg(TS_MSG_AVM_EXIT); /* only for debug in android display */
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, LEFT_MIRROR);
    if (task && task->disp_sts&EMIRROR_MASK(LEFT_MIRROR)) {
        if (disp && disp->handle) {
            sdm_clear_display(disp->handle);
        }
    }
}

static void emirror_lvgl_exit(emirror_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        for (int i = 0; i < EM_SW_NUM; i++) {
            if (sw_btn[i].label) {
                lv_obj_del(sw_btn[i].label);
                sw_btn[i].label = NULL;
            }
            if (sw_btn[i].btn) {
                lv_obj_del(sw_btn[i].btn);
                sw_btn[i].btn = NULL;
            }
            sw_btn[i].parent = NULL;
        }
        parent_screen = NULL;
        osEventFlagsSet(task->terminate_sig, EMIRROR_MASK(task->id));
        msg_t msg = { .msgid = emirror_evt_stop, .param = task->id };
        emirror_set_evt(&msg);
    }
    osThreadExit();
}

void emirror_lvgl_task(void* args)
{
    if (args) {
        emirror_task_token_t* task = (emirror_task_token_t*)args;
        if (!emirror_lvgl_init(task)) {
            emirror_lvgl_entry(task);
        }
        emirror_lvgl_exit(task);
    }
}
/* EOF */
