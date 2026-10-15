/**
 * @file controlpanel_lvgl.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv controlpanel application.
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
#if LV_USE_GPU
#include <lv_gpu_sdrv_dma2d.h>
#endif
#include <disp_data_type.h>
#include <sd_graphics.h>
#include "controlpanel_def.h"
#include "controlpanel_cfg.h"
#include "controlpanel.h"
#include "app_log.h"
#include "disp_utility.h"
#include "arbitrator.h"
#include "launcher.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "controlpanel"

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

#define CP_PNG_PATH(x)  "S:/early_app/controlpanel/"x".png"
#define CP_TEMP_VALUE   "16.0\n16.5\n17.0\n18.0\n18.5\n19.0\n19.5\n20.0\n20.5\n21.0\n21.5\n22.0\n22.5\n23.0\n23.5\n24.0\n24.5\n25.0\n25.5\n26.0\n26.5\n27.0\n27.5\n28.0"

#define CP_LV_OBJ_DEL(obj) \
        if (obj) { \
            lv_obj_del(obj); \
            obj = NULL; \
        }

enum {
    CHAIR_H = 0,
    CHAIR_M,
    CHAIR_L,
    CHAIR_MAX
};

typedef struct _cp_chair_ctx_t {
    lv_obj_t* layout;
    lv_obj_t* chair;
    lv_obj_t* chair_heat;
    lv_obj_t* left_heat;
    lv_obj_t* right_heat;
    lv_obj_t* gif[CHAIR_MAX];
    lv_anim_t ani;
} cp_chair_ctx_t;

typedef struct _cp_temp_ctx_t {
    lv_obj_t* layout;
    lv_obj_t* front_roller;
    lv_obj_t* back_roller;
    lv_obj_t* front_select;
    lv_obj_t* back_select;
    lv_obj_t* front_c_label;
    lv_obj_t* back_c_label;
    lv_obj_t* front_o_label;
    lv_obj_t* back_o_label;
} cp_temp_ctx_t;

typedef struct _cp_fan_ctx_t {
    lv_obj_t* layout;
    lv_obj_t* front_fan;
    lv_obj_t* back_fan;
    lv_obj_t* front_ib;
    lv_obj_t* back_ib;
    lv_obj_t* front_slider;
    lv_obj_t* back_slider;
    lv_obj_t* front_label;
    lv_obj_t* back_label;
    lv_anim_t front_ani;
    lv_anim_t back_ani;
    int16_t front_level;
    int16_t back_level;
} cp_fan_ctx_t;

typedef struct _cp_btn_ctx_t {
    lv_obj_t* layout;
    lv_obj_t* all_ac;
    lv_obj_t* back_ac;
    lv_obj_t* sync;
    lv_obj_t* auto_ac;
    lv_obj_t* loop;
    lv_obj_t* front_defog;
    lv_obj_t* back_defog;
    lv_obj_t* front_label;
    lv_obj_t* back_label;
    bool all_ac_off;
    bool back_ac_off;
    bool sync_off;
    bool auto_ac_off;
    bool inner_loop_off;
    bool front_defog_off;
    bool back_defog_off;
} cp_btn_ctx_t;

static lv_style_t cp_style_obj;
static lv_style_t cp_style_roller;
static lv_style_t cp_style_roller_seleted;
static lv_style_t cp_style_btn;
static lv_style_t cp_style_slider;
static lv_obj_t* parent_screen = NULL;
static lv_obj_t* background;
static cp_chair_ctx_t chair_ctx;
static cp_temp_ctx_t temp_ctx;
static cp_fan_ctx_t fan_ctx = { .front_level = 0, .back_level = 0 };
static cp_btn_ctx_t btn_ctx = {
        .all_ac_off = true,
        .back_ac_off = true,
        .sync_off = true,
        .auto_ac_off = true,
        .inner_loop_off = true,
        .front_defog_off = true,
        .back_defog_off = true,
    };

static inline void cp_lvgl_fan_slide_value_changed(int16_t is_front, int16_t value)
{
    cp_fan_ctx_t* ctx = &fan_ctx;
    lv_obj_t* ib = is_front ? ctx->front_ib : ctx->back_ib;
    lv_obj_t* label = is_front ? ctx->front_label : ctx->back_label;
    char fan_path[64] = { 0 };
    char fan_value[8] = { 0 };
    snprintf(fan_path, sizeof(fan_path), "S:/early_app/controlpanel/fan%d.png", value);
    snprintf(fan_value, sizeof(fan_value), "%d", value);
    lv_img_set_src(ib, fan_path);
    lv_label_set_text(label, fan_value);
}

static inline void cp_lvgl_sync_proc()
{
    if (!btn_ctx.sync_off) {
        /* button & fan */
        if (btn_ctx.back_ac_off != btn_ctx.all_ac_off) {
            btn_ctx.back_ac_off = btn_ctx.all_ac_off;
            lv_imgbtn_set_state(btn_ctx.back_ac, !btn_ctx.back_ac_off ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED);
            if (!btn_ctx.back_ac_off) {
                lv_anim_start(&fan_ctx.back_ani);
                fan_ctx.back_level = fan_ctx.front_level;
                cp_lvgl_fan_slide_value_changed(0, fan_ctx.back_level);
            }
            else {
                lv_img_set_angle(fan_ctx.back_fan, 0);
                lv_anim_custom_del(&fan_ctx.back_ani, NULL);
                cp_lvgl_fan_slide_value_changed(0, 0);
            }
        }
        else if (fan_ctx.front_level != fan_ctx.back_level) {
            if (!btn_ctx.back_ac_off) {
                fan_ctx.back_level = fan_ctx.front_level;
                cp_lvgl_fan_slide_value_changed(0, fan_ctx.back_level);
            }
            else {
                /* ac off, no need sync fan level */
            }
        }

        /* temperature roller */
        {
            uint16_t front_select = lv_roller_get_selected(temp_ctx.front_roller);
            uint16_t back_select = lv_roller_get_selected(temp_ctx.back_roller);
            if (front_select != back_select) {
                lv_roller_set_selected(temp_ctx.back_roller, front_select, LV_ANIM_ON);
            }
        }
    }
}

static void cp_lvgl_chair_animation(void* var, lv_anim_value_t value)
{
    if (value >=0 && value <= 12) {
        char gif[64] = { 0 };
        lv_obj_t** obj = (lv_obj_t**)var;
        snprintf(gif, sizeof(gif), "S:/early_app/controlpanel/gif%d.png", value);
        lv_img_set_src(obj[0], gif);
        lv_img_set_src(obj[1], gif);
        lv_img_set_src(obj[2], gif);
    }
}

static void cp_lvgl_fan_animation(void* var, lv_anim_value_t value)
{
    lv_img_set_angle((lv_obj_t*)var, value);
}

static void cp_lvgl_temp_roller_evt_handler(lv_obj_t* obj, lv_event_t event)
{
    APP_DBG("obj:%p, event:%d", obj, event);
    if (event == LV_EVENT_DRAG_END) {
        cp_temp_ctx_t* ctx = &temp_ctx;
        int isFront = ctx->front_roller == obj ? 1 : 0;
        if (isFront) {
            cp_lvgl_sync_proc();
        }
        else {
            btn_ctx.sync_off = true;
            lv_imgbtn_set_state(btn_ctx.sync, LV_BTN_STATE_RELEASED);
        }
    }
}

static void cp_lvgl_fan_slide_evt_handler(lv_obj_t* obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        cp_fan_ctx_t* ctx = &fan_ctx;
        int16_t isFront = ctx->front_slider == obj ? 1 : 0;
        int16_t value = lv_slider_get_value(obj);
        if (isFront) {
            ctx->front_level = value;
            if (btn_ctx.all_ac_off) {
                btn_ctx.all_ac_off = false;
                lv_imgbtn_set_state(btn_ctx.all_ac, LV_BTN_STATE_CHECKED_RELEASED);
            }
            if (!btn_ctx.sync_off) {
                btn_ctx.back_ac_off = btn_ctx.all_ac_off;
                lv_imgbtn_set_state(btn_ctx.back_ac, lv_imgbtn_get_state(btn_ctx.all_ac));
                cp_lvgl_fan_slide_value_changed(0, value);
            }
        }
        else {
            ctx->back_level = value;
            if (btn_ctx.back_ac_off) {
                btn_ctx.back_ac_off = false;
                lv_imgbtn_set_state(btn_ctx.back_ac, LV_BTN_STATE_CHECKED_RELEASED);
            }
            btn_ctx.sync_off = true;
            lv_imgbtn_set_state(btn_ctx.sync, LV_BTN_STATE_RELEASED);
        }
        APP_INFO("%s fan:%d", isFront ? "front" : "back", value);
        cp_lvgl_fan_slide_value_changed(isFront, value);
    }
}

static void cp_lvgl_btn_all_ac_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.all_ac_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("all ac %s", btn_ctx.all_ac_off ? "off" : "on");
        if (!btn_ctx.all_ac_off) {
            lv_anim_start(&fan_ctx.front_ani);
            fan_ctx.front_level = fan_ctx.front_level ? : 1;
            cp_lvgl_fan_slide_value_changed(1, fan_ctx.front_level);
            cp_lvgl_sync_proc();
        }
        else {
            lv_img_set_angle(fan_ctx.front_fan, 0);
            lv_anim_custom_del(&fan_ctx.front_ani, NULL);
            cp_lvgl_fan_slide_value_changed(1, 0);
            cp_lvgl_sync_proc();
        }
    }
}

static void cp_lvgl_btn_back_ac_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.back_ac_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("back ac %s", btn_ctx.back_ac_off ? "off" : "on");
        if (!btn_ctx.back_ac_off) {
            lv_anim_start(&fan_ctx.back_ani);
            fan_ctx.back_level = fan_ctx.back_level ? : 1;
            cp_lvgl_fan_slide_value_changed(1, fan_ctx.back_level);
        }
        else {
            lv_img_set_angle(fan_ctx.back_fan, 0);
            lv_anim_custom_del(&fan_ctx.back_ani, NULL);
            cp_lvgl_fan_slide_value_changed(1, 0);
        }
        btn_ctx.sync_off = true;
        lv_imgbtn_set_state(btn_ctx.sync, LV_BTN_STATE_RELEASED);
    }
}

static void cp_lvgl_btn_sync_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.sync_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("sync %s", btn_ctx.sync_off ? "off" : "on");
        cp_lvgl_sync_proc();
    }
}

static void cp_lvgl_btn_auto_ac_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.auto_ac_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("auto ac %s", btn_ctx.auto_ac_off ? "off" : "on");
    }
}

static void cp_lvgl_btn_loop_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.inner_loop_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("inner loop %s", btn_ctx.inner_loop_off ? "off" : "on");
    }
}

static void cp_lvgl_btn_front_defog_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.front_defog_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("front defog %s", btn_ctx.front_defog_off ? "off" : "on");
    }
}

static void cp_lvgl_btn_back_defog_cb(lv_obj_t* obj, lv_event_t evt)
{
    if (evt == LV_EVENT_CLICKED) {
        btn_ctx.back_defog_off = lv_imgbtn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED ? false : true;
        APP_INFO("back defog %s", btn_ctx.back_defog_off ? "off" : "on");
    }
}

static void cp_lvgl_init_style()
{
    lv_style_init(&cp_style_roller);
    lv_style_set_text_font(&cp_style_roller, LV_STATE_DEFAULT, &lv_font_montserrat_40);
    lv_style_set_bg_grad_color(&cp_style_roller, LV_STATE_DEFAULT, LV_COLOR_TRANSP);
    lv_style_set_image_opa(&cp_style_roller, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&cp_style_roller, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&cp_style_roller, LV_STATE_FOCUSED, 0);
    lv_style_set_bg_opa(&cp_style_roller, LV_STATE_DEFAULT, 0);
    lv_style_set_value_opa(&cp_style_roller, LV_STATE_DEFAULT, 0);

    lv_style_init(&cp_style_roller_seleted);
    lv_style_set_text_font(&cp_style_roller_seleted, LV_STATE_DEFAULT,  &lv_font_montserrat_46);
    lv_style_set_bg_grad_color(&cp_style_roller_seleted, LV_STATE_DEFAULT,  LV_COLOR_TRANSP);
    lv_style_set_image_opa(&cp_style_roller_seleted, LV_STATE_DEFAULT,  0);
    lv_style_set_border_opa(&cp_style_roller_seleted, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&cp_style_roller_seleted, LV_STATE_FOCUSED, 0);
    lv_style_set_bg_opa(&cp_style_roller_seleted, LV_STATE_DEFAULT, 0);
    lv_style_set_value_opa(&cp_style_roller_seleted, LV_STATE_DEFAULT, 0);

    lv_style_init(&cp_style_btn);
    lv_style_set_text_font(&cp_style_btn, LV_STATE_DEFAULT,  &lv_font_montserrat_30);
    lv_style_set_text_color(&cp_style_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_style_init(&cp_style_obj);
    lv_style_set_border_opa(&cp_style_obj, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&cp_style_obj, LV_STATE_FOCUSED, 0);
    lv_style_set_bg_opa(&cp_style_obj, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&cp_style_obj, LV_STATE_FOCUSED, 0);
    lv_style_set_text_font(&cp_style_obj,  LV_STATE_DEFAULT,  &lv_font_montserrat_20);
    lv_style_set_text_color(&cp_style_obj, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_style_init(&cp_style_slider);
    lv_style_set_bg_opa(&cp_style_slider, LV_STATE_DEFAULT, 0);
    lv_style_set_opa_scale(&cp_style_slider, LV_STATE_DEFAULT, 0);
    lv_style_set_opa_scale(&cp_style_slider, LV_STATE_DISABLED, 0);
    lv_style_set_value_opa(&cp_style_slider, LV_STATE_DEFAULT, 0);
}

static void cp_lvgl_chair_create(lv_obj_t* parent)
{
    APP_INFO("");
    cp_chair_ctx_t* ctx = &chair_ctx;
    ctx->layout = lv_obj_create(parent, NULL);
    lv_obj_set_size(ctx->layout, 600, 400);
    lv_obj_add_style(ctx->layout, LV_CONT_PART_MAIN, &cp_style_obj);
    lv_obj_set_pos(ctx->layout, 660, 80);

    ctx->chair = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->chair, 143, 20);
    lv_img_set_src(ctx->chair, CP_PNG_PATH("chair"));

    ctx->gif[CHAIR_H] = lv_img_create(ctx->layout, NULL);
    lv_img_set_src(ctx->gif[CHAIR_H], CP_PNG_PATH("gif0"));
    lv_obj_set_pos(ctx->gif[CHAIR_H], 143, 0);

    ctx->gif[CHAIR_M] = lv_img_create(ctx->layout, ctx->gif[CHAIR_H]);
    lv_obj_set_pos(ctx->gif[CHAIR_M], 100, 70);

    ctx->gif[CHAIR_L] = lv_img_create(ctx->layout, ctx->gif[CHAIR_H]);
    lv_obj_set_pos(ctx->gif[CHAIR_L], 20, 130);

    lv_anim_init(&ctx->ani);
    lv_anim_set_var(&ctx->ani, ctx->gif);
    lv_anim_set_time(&ctx->ani, 1200);
    lv_anim_set_values(&ctx->ani, 0, 12);
    lv_anim_set_exec_cb(&ctx->ani, cp_lvgl_chair_animation);
    lv_anim_set_repeat_count(&ctx->ani, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&ctx->ani);

    ctx->chair_heat = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->chair_heat, 187, 322);
    lv_img_set_src(ctx->chair_heat, CP_PNG_PATH("chair_heat"));

    ctx->left_heat = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->left_heat, 210, 390);
    lv_img_set_src(ctx->left_heat, CP_PNG_PATH("left_heat_1"));

    ctx->right_heat = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->right_heat, 320, 390);
    lv_img_set_src(ctx->right_heat, CP_PNG_PATH("right_heat_1"));
}

static void cp_lvgl_temp_create(lv_obj_t* parent)
{
    APP_INFO("");
    cp_temp_ctx_t* ctx = &temp_ctx;
    ctx->layout = lv_obj_create(parent, NULL);
    lv_obj_set_size(ctx->layout, 1420, 300);
    lv_obj_add_style(ctx->layout, LV_CONT_PART_MAIN, &cp_style_obj);
    lv_obj_set_pos(ctx->layout, 250, 100);

    /* left */
    ctx->front_roller = lv_roller_create(ctx->layout, NULL);
    lv_obj_set_size(ctx->front_roller, 300, 300);
    lv_obj_add_style(ctx->front_roller, LV_CONT_PART_MAIN, &cp_style_roller);
    lv_obj_add_style(ctx->front_roller, LV_ROLLER_PART_SELECTED, &cp_style_roller_seleted);
    lv_roller_set_auto_fit(ctx->front_roller, false);
    lv_roller_set_align(ctx->front_roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_options(ctx->front_roller, CP_TEMP_VALUE, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(ctx->front_roller, 2, LV_ANIM_ON);
    lv_roller_set_visible_row_count(ctx->front_roller, 3);
    lv_obj_set_pos(ctx->front_roller, 0, 0);
    lv_obj_set_event_cb(ctx->front_roller, cp_lvgl_temp_roller_evt_handler);

    ctx->front_select = lv_img_create(ctx->layout, NULL);
    lv_img_set_src(ctx->front_select, CP_PNG_PATH("background"));
    lv_obj_set_pos(ctx->front_select, 20, 50);

    ctx->front_c_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->front_c_label, "C");
    lv_obj_add_style(ctx->front_c_label, LV_CONT_PART_MAIN, &cp_style_roller_seleted);
    lv_obj_set_pos(ctx->front_c_label, 190, 65);

    ctx->front_o_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->front_o_label, "o");
    lv_obj_set_pos(ctx->front_o_label, 180, 65);

    /* right */
    ctx->back_roller = lv_roller_create(ctx->layout, NULL);
    lv_obj_set_size(ctx->back_roller, 300, 300);
    lv_obj_add_style(ctx->back_roller, LV_CONT_PART_MAIN, &cp_style_roller);
    lv_obj_add_style(ctx->back_roller, LV_ROLLER_PART_SELECTED, &cp_style_roller_seleted);
    lv_roller_set_auto_fit(ctx->back_roller, false);
    lv_roller_set_align(ctx->back_roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_options(ctx->back_roller, CP_TEMP_VALUE, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(ctx->back_roller, 2, LV_ANIM_ON);
    lv_roller_set_visible_row_count(ctx->back_roller, 3);
    lv_obj_set_pos(ctx->back_roller, 1120, 0);
    lv_obj_set_event_cb(ctx->back_roller, cp_lvgl_temp_roller_evt_handler);

    ctx->back_select = lv_img_create(ctx->layout, NULL);
    lv_img_set_src(ctx->back_select, CP_PNG_PATH("background"));
    lv_obj_set_pos(ctx->back_select, 1140, 50);

    ctx->back_c_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->back_c_label, "C");
    lv_obj_add_style(ctx->back_c_label, LV_CONT_PART_MAIN, &cp_style_roller_seleted);
    lv_obj_set_pos(ctx->back_c_label, 1310, 65);

    ctx->back_o_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->back_o_label, "o");
    lv_obj_set_pos(ctx->back_o_label, 1300, 65);
}

static void cp_lvgl_fans_create(lv_obj_t* parent)
{
    APP_INFO("");
    cp_fan_ctx_t* ctx = &fan_ctx;
    ctx->layout = lv_obj_create(parent, NULL);
    lv_obj_set_size(ctx->layout, 1620, 112);
    lv_obj_add_style(ctx->layout, LV_CONT_PART_MAIN, &cp_style_obj);
    lv_obj_set_pos(ctx->layout, 150, 400);

    /* front */
    ctx->front_fan = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->front_fan, 0, 40);
    lv_img_set_src(ctx->front_fan, CP_PNG_PATH("fan_normal"));

    lv_anim_init(&ctx->front_ani);
    lv_anim_set_var(&ctx->front_ani, ctx->front_fan);
    lv_anim_set_time(&ctx->front_ani, 1800);
    lv_anim_set_values(&ctx->front_ani, 0, 3600);
    lv_anim_set_exec_cb(&ctx->front_ani, cp_lvgl_fan_animation);
    lv_anim_set_repeat_count(&ctx->front_ani, LV_ANIM_REPEAT_INFINITE);

    ctx->front_ib = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->front_ib, 70, 50);
    lv_img_set_src(ctx->front_ib, CP_PNG_PATH("fan0"));

    ctx->front_slider = lv_slider_create(ctx->front_ib, NULL);
    lv_obj_add_style(ctx->front_slider, LV_SLIDER_PART_BG, &cp_style_slider);
    lv_obj_add_style(ctx->front_slider, LV_SLIDER_PART_INDIC, &cp_style_slider);
    lv_obj_add_style(ctx->front_slider, LV_SLIDER_PART_KNOB, &cp_style_slider);
    lv_obj_set_event_cb(ctx->front_slider, cp_lvgl_fan_slide_evt_handler);
    lv_slider_set_range(ctx->front_slider, 1, 7);
    lv_obj_set_pos(ctx->front_slider, 0, 0);
    lv_obj_set_size(ctx->front_slider, 342, 40);
    lv_slider_set_value(ctx->front_slider, ctx->front_level, true);

    ctx->front_label = lv_label_create(ctx->layout, NULL);
    lv_obj_add_style(ctx->front_label, LV_CONT_PART_MAIN, &cp_style_btn);
    lv_obj_set_pos(ctx->front_label, 420, 50);
    lv_label_set_text(ctx->front_label, "0");

    /* back */
    ctx->back_fan = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->back_fan, 1150, 40);
    lv_img_set_src(ctx->back_fan, CP_PNG_PATH("fan_normal"));

    lv_anim_init(&ctx->back_ani);
    lv_anim_set_var(&ctx->back_ani, ctx->back_fan);
    lv_anim_set_time(&ctx->back_ani, 2000);
    lv_anim_set_values(&ctx->back_ani, 0, 3600);
    lv_anim_set_exec_cb(&ctx->back_ani, cp_lvgl_fan_animation);
    lv_anim_set_repeat_count(&ctx->back_ani, LV_ANIM_REPEAT_INFINITE);

    ctx->back_ib = lv_img_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->back_ib, 1219, 50);
    lv_img_set_src(ctx->back_ib, CP_PNG_PATH("fan0"));

    ctx->back_slider = lv_slider_create(ctx->back_ib, NULL);
    lv_obj_add_style(ctx->back_slider, LV_SLIDER_PART_BG, &cp_style_slider);
    lv_obj_add_style(ctx->back_slider, LV_SLIDER_PART_INDIC, &cp_style_slider);
    lv_obj_add_style(ctx->back_slider, LV_SLIDER_PART_KNOB, &cp_style_slider);
    lv_obj_set_event_cb(ctx->back_slider, cp_lvgl_fan_slide_evt_handler);
    lv_slider_set_range(ctx->back_slider, 1, 7);
    lv_obj_set_pos(ctx->back_slider, 0, 0);
    lv_obj_set_size(ctx->back_slider, 342, 40);
    lv_slider_set_value(ctx->back_slider, ctx->back_level, true);

    ctx->back_label = lv_label_create(ctx->layout, NULL);
    lv_obj_set_pos(ctx->back_label, 1580, 50);
    lv_obj_add_style(ctx->back_label, LV_CONT_PART_MAIN, &cp_style_btn);
    lv_label_set_text(ctx->back_label, "0");
}

static void cp_lvgl_btns_create(lv_obj_t* parent)
{
    APP_INFO("");
    cp_btn_ctx_t* ctx = &btn_ctx;
    ctx->layout = lv_obj_create(parent, NULL);
    lv_obj_set_size(ctx->layout, 1720, 112);
    lv_obj_add_style(ctx->layout, LV_CONT_PART_MAIN, &cp_style_obj);
    lv_obj_set_pos(ctx->layout, 100, 570);

    ctx->all_ac = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->all_ac, cp_lvgl_btn_all_ac_cb);
    lv_imgbtn_set_src(ctx->all_ac, LV_BTN_STATE_RELEASED, CP_PNG_PATH("ac_normal"));
    lv_imgbtn_set_src(ctx->all_ac, LV_BTN_STATE_PRESSED, CP_PNG_PATH("ac_normal_click"));
    lv_imgbtn_set_src(ctx->all_ac, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("ac_select"));
    lv_imgbtn_set_src(ctx->all_ac, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("ac_select_click"));
    lv_obj_set_pos(ctx->all_ac, 54, 0);
    lv_btn_set_checkable(ctx->all_ac, true);

    ctx->front_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->front_label, "Front");
    lv_obj_set_pos(ctx->front_label, 85, 20);

    ctx->back_ac = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->back_ac, cp_lvgl_btn_back_ac_cb);
    lv_imgbtn_set_src(ctx->back_ac, LV_BTN_STATE_RELEASED, CP_PNG_PATH("ac_normal"));
    lv_imgbtn_set_src(ctx->back_ac, LV_BTN_STATE_PRESSED, CP_PNG_PATH("ac_normal_click"));
    lv_imgbtn_set_src(ctx->back_ac, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("ac_select"));
    lv_imgbtn_set_src(ctx->back_ac, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("ac_select_click"));
    lv_obj_set_pos(ctx->back_ac, 304, 0);
    lv_btn_set_checkable(ctx->back_ac, true);

    ctx->back_label = lv_label_create(ctx->layout, NULL);
    lv_label_set_text(ctx->back_label, "Back");
    lv_obj_set_pos(ctx->back_label, 334, 20);

    ctx->sync = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->sync, cp_lvgl_btn_sync_cb);
    lv_imgbtn_set_src(ctx->sync, LV_BTN_STATE_RELEASED, CP_PNG_PATH("sync_normal"));
    lv_imgbtn_set_src(ctx->sync, LV_BTN_STATE_PRESSED, CP_PNG_PATH("sync_normal_click"));
    lv_imgbtn_set_src(ctx->sync, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("sync_select"));
    lv_imgbtn_set_src(ctx->sync, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("sync_select_click"));
    lv_obj_set_pos(ctx->sync, 554, 0);
    lv_btn_set_checkable(ctx->sync, true);

    ctx->auto_ac = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->auto_ac, cp_lvgl_btn_auto_ac_cb);
    lv_imgbtn_set_src(ctx->auto_ac, LV_BTN_STATE_RELEASED, CP_PNG_PATH("auto_normal"));
    lv_imgbtn_set_src(ctx->auto_ac, LV_BTN_STATE_PRESSED, CP_PNG_PATH("auto_normal_click"));
    lv_imgbtn_set_src(ctx->auto_ac, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("auto_select"));
    lv_imgbtn_set_src(ctx->auto_ac, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("auto_select_click"));
    lv_obj_set_pos(ctx->auto_ac, 804, 0);
    lv_btn_set_checkable(ctx->auto_ac, true);

    ctx->loop = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->loop, cp_lvgl_btn_loop_cb);
    lv_imgbtn_set_src(ctx->loop, LV_BTN_STATE_RELEASED, CP_PNG_PATH("inner_circulate_select"));
    lv_imgbtn_set_src(ctx->loop, LV_BTN_STATE_PRESSED, CP_PNG_PATH("inner_circulate_select_click"));
    lv_imgbtn_set_src(ctx->loop, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("inner_circulate_select"));
    lv_imgbtn_set_src(ctx->loop, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("inner_circulate_select_click"));
    lv_obj_set_pos(ctx->loop, 1054, 0);
    lv_btn_set_checkable(ctx->loop, true);

    ctx->front_defog = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->front_defog, cp_lvgl_btn_front_defog_cb);
    lv_imgbtn_set_src(ctx->front_defog, LV_BTN_STATE_RELEASED, CP_PNG_PATH("front_defrost_normal"));
    lv_imgbtn_set_src(ctx->front_defog, LV_BTN_STATE_PRESSED, CP_PNG_PATH("front_defrost_normal_click"));
    lv_imgbtn_set_src(ctx->front_defog, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("front_defrost_select"));
    lv_imgbtn_set_src(ctx->front_defog, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("front_defrost_select_click"));
    lv_obj_set_pos(ctx->front_defog, 1304, 0);
    lv_btn_set_checkable(ctx->front_defog, true);

    ctx->back_defog = lv_imgbtn_create(ctx->layout, NULL);
    lv_obj_set_event_cb(ctx->back_defog, cp_lvgl_btn_back_defog_cb);
    lv_imgbtn_set_src(ctx->back_defog, LV_BTN_STATE_RELEASED, CP_PNG_PATH("back_defrost_normal"));
    lv_imgbtn_set_src(ctx->back_defog, LV_BTN_STATE_PRESSED, CP_PNG_PATH("back_defrost_normal_click"));
    lv_imgbtn_set_src(ctx->back_defog, LV_BTN_STATE_CHECKED_RELEASED, CP_PNG_PATH("back_defrost_select"));
    lv_imgbtn_set_src(ctx->back_defog, LV_BTN_STATE_CHECKED_PRESSED, CP_PNG_PATH("back_defrost_select_click"));
    lv_obj_set_pos(ctx->back_defog, 1554, 0);
    lv_btn_set_checkable(ctx->back_defog, true);
}

static inline void cp_lvgl_content_create(lv_obj_t * parent)
{
    APP_INFO("");
    background = lv_img_create(parent, NULL);
    lv_obj_set_pos(background, 0, 0);
    lv_obj_set_size(background, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_img_set_src(background, CP_PNG_PATH("img_background"));
    cp_lvgl_chair_create(parent);
    cp_lvgl_temp_create(parent);
    cp_lvgl_fans_create(parent);
    cp_lvgl_btns_create(parent);
}

static inline int cp_lvgl_init(cp_task_token_t* task)
{
    APP_INFO("");
    lvgl_init();
    parent_screen = lv_disp_get_scr_act(get_display(CONTROLPANEL));
    cp_lvgl_init_style();
    cp_lvgl_content_create(parent_screen);
    return 0;
}

static inline void cp_lvgl_entry(cp_task_token_t* task)
{
    APP_INFO("disp_sts:%x", task ? task->disp_sts : 0);
    while (task && task->running) {
        if (task->disp_sts&CP_MASK(CONTROLPANEL)) {
            lv_task_handler();
        }
        osDelay(50);
    }

    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, CONTROLPANEL);
    if (task && task->disp_sts&CP_MASK(CONTROLPANEL)) {
        if (disp && disp->handle) {
            sdm_clear_display(disp->handle);
        }
    }
}

static inline void cp_lvgl_exit(cp_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        CP_LV_OBJ_DEL(background);
        CP_LV_OBJ_DEL(chair_ctx.layout);
        CP_LV_OBJ_DEL(temp_ctx.layout);
        CP_LV_OBJ_DEL(fan_ctx.layout);
        CP_LV_OBJ_DEL(btn_ctx.layout);
        parent_screen = NULL;
        osEventFlagsSet(task->terminate_sig, CP_MASK(task->id));
        msg_t msg = { .msgid = cp_evt_stop, .param = task->id };
        cp_set_evt(&msg);
    }
    osThreadExit();
}

void cp_lvgl_task(void* args)
{
    APP_INFO("");
    if (args) {
        cp_task_token_t* task = (cp_task_token_t*)args;
        if (!cp_lvgl_init(task)) {
            cp_lvgl_entry(task);
        }
        cp_lvgl_exit(task);
    }
}
/* EOF */
