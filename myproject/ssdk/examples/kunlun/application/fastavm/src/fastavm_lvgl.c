/**
 * @file fastavm_lvgl.c
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
#define LOG_TAG "fastavm"

#include <stdlib.h>
#include <lvgl_gui.h>
#include <param.h>
#include <malloc.h>
#include <res_loader.h>
#include <sdm_display.h>
#include <input_service.h>
#include "fastavm_def.h"
#include "fastavm_cfg.h"
#include "fastavm.h"
#include "app_log.h"
#include "disp_utility.h"
#include "arbitrator.h"
#include "launcher.h"

#define AVM_OFFSET_X(x) ((x-AVM_WIDTH-CAM_WIDTH)>>1)
#define AVM_SWITCH_NUM  (5)
#define AVM_SWITCH_H    (80)
#ifdef CONFIG_MP
#define AVM_CAR_PATH    "early_app/fastavm_mp/car.bin"
#else
#define AVM_CAR_PATH    "early_app/fastavm/car.bin"
#endif

typedef struct _fastavm_btn_ctx_t{
    lv_obj_t* parent;
    lv_obj_t* label;
    lv_obj_t* btn;
    lv_point_t refPos;
    lv_coord_t width;
    lv_coord_t height;
    lv_event_cb_t event_cb;
    char* text;
} fastavm_btn_ctx_t;

static uint32_t disp_w;
static uint32_t disp_h;
static lv_obj_t* parent_screen = NULL;
static void* car_data = NULL;
static lv_img_dsc_t car_dsc;
static lv_obj_t* car_obj = NULL;
static const char* clabel[] = { "Front", "Right", "Back", "Left", "Close" };
static lv_style_t btn_style;
static lv_style_t label_style;
static int fastavm_close = 0;
static fastavm_btn_ctx_t sw_btn[AVM_SWITCH_NUM];

static void fastavm_front_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        fastavm_set_primary_cam(0);
    }
}

static void fastavm_back_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        fastavm_set_primary_cam(1);
    }
}

static void fastavm_left_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        fastavm_set_primary_cam(2);
    }
}

static void fastavm_right_cb(lv_obj_t* obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        fastavm_set_primary_cam(3);
    }
}

static void fastavm_close_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED == event) {
        lv_obj_set_event_cb(obj, NULL);
        fastavm_close = 1;
    }
}

static inline int fastavm_load_car_img()
{
    if (!car_data) {
        int car_size = ROUNDUP(res_size(AVM_CAR_PATH), 32);
        car_data = memalign(32, car_size);
        if (!car_data) {
            APP_ERR("car_data memalign fail, car_size:%d", car_size);
            return -1;
        }
        res_load(AVM_CAR_PATH, car_data, car_size, 0);
    }
    return 0;
}

static inline void fastavm_release_car_img()
{
    car_dsc.data = NULL;
    if (car_data) {
        free(car_data);
        car_data = NULL;
    }
}

void fastavm_set_style(void)
{
    lv_style_init(&btn_style);
    lv_style_set_radius(&btn_style, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_opa(&btn_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&btn_style, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_bg_grad_color(&btn_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_style_set_bg_grad_dir(&btn_style, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);

    lv_style_set_border_color(&btn_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_opa(&btn_style, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_border_width(&btn_style, LV_STATE_DEFAULT, 5);

    lv_style_init(&label_style);
    lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &lv_font_montserrat_48);
}

static inline void fastavm_set_img_pos(lv_obj_t* obj, lv_img_dsc_t* dsc, int16_t width, int16_t height, int16_t xpos)
{
    if (obj && dsc) {
        lv_img_set_src(obj, dsc);
        lv_obj_set_pos(obj, (width-dsc->header.w)/2 + xpos, (height-dsc->header.h)/2);
        lv_obj_set_size(obj, dsc->header.w, dsc->header.h);
    }
}

static inline void fastavm_create_switch(fastavm_btn_ctx_t* avm_btn, uint32_t num, lv_coord_t height, lv_coord_t xpos)
{
    memset(avm_btn, 0, sizeof(fastavm_btn_ctx_t)*num);
    fastavm_set_style();
    uint32_t width = IMG_WIDTH/num;
    lv_event_cb_t eHandler[AVM_SWITCH_NUM] = {
        fastavm_front_cb,
        fastavm_right_cb,
        fastavm_back_cb,
        fastavm_left_cb,
        fastavm_close_cb
    };

    for (int i = 0; i < num; i++, avm_btn++) {
        avm_btn->parent = parent_screen;
        avm_btn->refPos.x = xpos + i*width;
        avm_btn->refPos.y = IMG_HEIGHT-height;
        avm_btn->width = width;
        avm_btn->height = height;
        avm_btn->text = (char*)clabel[i];
        avm_btn->event_cb = eHandler[i];
        avm_btn->btn = lv_btn_create(avm_btn->parent, NULL);
        lv_obj_add_style(avm_btn->btn, LV_BTN_PART_MAIN, &btn_style);
        lv_obj_set_pos(avm_btn->btn, avm_btn->refPos.x, avm_btn->refPos.y);
        lv_obj_set_size(avm_btn->btn, avm_btn->width, avm_btn->height);
        lv_obj_set_event_cb(avm_btn->btn, avm_btn->event_cb);
        avm_btn->label = lv_label_create(avm_btn->btn, NULL);
        lv_obj_add_style(avm_btn->label, LV_CONT_PART_MAIN, &label_style);
        lv_label_set_text(avm_btn->label, avm_btn->text);
    }
}

static inline int fastavm_lvgl_init(fastavm_task_token_t* task)
{
    APP_INFO("");
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, INFOTAINMENT);
    if (disp) {
        fastavm_close = 0;
        utils_get_sdm_disp_rect(disp, &disp_w, &disp_h);
        lvgl_init();
        if (!fastavm_load_car_img()) {
            car_dsc.header = *((lv_img_header_t*)car_data);
            car_dsc.data = (uint8_t*)car_data+4;
            car_dsc.data_size = car_dsc.header.w*car_dsc.header.h*LV_IMG_PX_SIZE_ALPHA_BYTE;
            parent_screen = lv_disp_get_scr_act(get_display(INFOTAINMENT));
            lv_obj_set_style_local_bg_opa(parent_screen, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
            car_obj = lv_img_create(parent_screen, NULL);
            fastavm_set_img_pos(car_obj, &car_dsc, AVM_WIDTH, disp_h, AVM_OFFSET_X(disp_w));
            fastavm_create_switch(sw_btn, AVM_SWITCH_NUM, AVM_SWITCH_H, AVM_WIDTH+AVM_OFFSET_X(disp_w));
            return 0;
        }
    }
    return -1;
}

static inline void fastavm_lvgl_entry(fastavm_task_token_t* task)
{
    APP_INFO("");
    int ret = -1;
    //lv_refr_now(get_display(INFOTAINMENT));
    while (task && task->running && !fastavm_close) {
        if (ret) ret = touch_service_send_msg(TS_MSG_AVM_START);
        if (task->disp_sts&FASTAVM_MASK(INFOTAINMENT)) {
            lv_task_handler();
        }
        osDelay(50);
    }
    touch_service_send_msg(TS_MSG_AVM_EXIT);
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, INFOTAINMENT);
    if (task && task->disp_sts&FASTAVM_MASK(INFOTAINMENT)) {
        if (disp && disp->handle) {
            sdm_clear_display(disp->handle);
        }
    }
}

static inline void fastavm_lvgl_exit(fastavm_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        car_dsc.data = NULL;
        if (car_data) {
            free(car_data);
            car_data = NULL;
        }
        if (car_obj) {
            lv_obj_del(car_obj);
            car_obj = NULL;
        }
        for (int i = 0; i < AVM_SWITCH_NUM; i++) {
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
        osEventFlagsSet(task->terminate_sig, FASTAVM_MASK(task->id));
        msg_t msg = { .msgid = fastavm_evt_stop, .param = task->id };
        fastavm_set_evt(&msg);
    }
    osThreadExit();
}

void fastavm_lvgl_task(void* args)
{
    if (args) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)args;
        if (!fastavm_lvgl_init(task)) {
            fastavm_lvgl_entry(task);
        }
        fastavm_lvgl_exit(task);
    }
}
/* EOF */
