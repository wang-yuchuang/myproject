/*
 * emirror_lvgl.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: emirror-app UI
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdbool.h>

#include "emirror_lvgl.h"

#include "driver/taishan/dispss/disp_data_type.h"
#include "driver/timer.h"
#include "lvgl_gui.h"

#define emirror_info(x...) ssdk_printf(SSDK_NOTICE, x)

LV_IMG_DECLARE(on);
LV_IMG_DECLARE(off);

typedef struct _rvm_roi {
    int update;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} rvm_roi_t;

typedef struct _rvm_size {
    int32_t w;
    int32_t h;
} rvm_size_t;

static bool rvm_lvgl_stop = false;

/* 0:Up; 1:Down; 2:Right; 3:Select Right */
const char *rvm_clabel[] = {
    "Left",
    "Up",
    "Down",
    "Right",
};
// const char *rvm_clabel[] = {"L", "<=", "Up", "Down", "=>", "R"};
static lv_style_t style_btn;
static lv_style_t style_label;

static int32_t step_w = 16;
static int32_t step_h = 12;

static rvm_roi_t left_roi = {0, 0, 0, 960, 540};
static rvm_size_t left_size = {1280, 720};
static lv_obj_t *back_camera_btns[4];

void set_full_size(int32_t w, int32_t h)
{
    left_size.w = w;
    left_size.h = h;
}

void set_default_roi(int32_t x, int32_t y, int32_t w, int32_t h)
{
    left_roi.update = 1;
    left_roi.x = x;
    left_roi.y = y;
    left_roi.w = w;
    left_roi.h = h;
}

void get_cur_roi(int *update, int32_t *x, int32_t *y, int32_t *w, int32_t *h)
{
    *update = left_roi.update;
    *x = left_roi.x;
    *y = left_roi.y;
    *w = left_roi.w;
    *h = left_roi.h;
    left_roi.update = 0;
}

void get_screen_size(lv_obj_t *scr, uint32_t *width, uint32_t *height)
{
    *width = scr->coords.x2 - scr->coords.x1 + 1;
    *height = scr->coords.y2 - scr->coords.y1 + 1;
}

static void _set_style(void)
{
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_SILVER);

    lv_style_set_bg_grad_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_style_set_bg_grad_dir(&style_btn, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);

    lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 5);

    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, LV_STATE_DEFAULT,
                           &lv_font_montserrat_48);
}

static void _createBtn(lv_obj_rvm *rvmBtn, char *picName, char *focus_picName)
{
    static int num = 0;
    lv_obj_t *parent = rvmBtn->obj;
    lv_obj_t *btn = lv_imgbtn_create(parent, NULL);

    back_camera_btns[num++] = btn;
    lv_imgbtn_set_src(btn, LV_STATE_DEFAULT, picName);
    lv_imgbtn_set_src(btn, LV_STATE_CHECKED, focus_picName);
    lv_obj_set_pos(btn, rvmBtn->refPos.x, rvmBtn->refPos.y);
    lv_obj_set_size(btn, rvmBtn->width, rvmBtn->height);
    lv_obj_set_event_cb(btn, rvmBtn->event_cb);
}

static void _rvm_up_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED != event)
        return;

    emirror_info("_rvm_up_cb------------------------------\n");

    if ((left_roi.y - step_h) < 0)
        left_roi.y = 0;
    else
        left_roi.y -= step_h;
    left_roi.update = 1;
}

static void _rvm_down_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED != event)
        return;

    emirror_info("_rvm_down_cb------------------------------\n");

    if ((left_roi.y + step_h + left_roi.h) > left_size.h)
        left_roi.y = (left_size.h - left_roi.h);
    else
        left_roi.y += step_h;
}

static void _rvm_left_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED != event)
        return;

    emirror_info("_rvm_left_cb------------------------------\n");

    if ((left_roi.x - step_w) < 0)
        left_roi.x = 0;
    else
        left_roi.x -= step_w;
}

static void _rvm_right_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED != event)
        return;

    emirror_info("_rvm_right_cb------------------------------\n");

    if ((left_roi.x + step_w + left_roi.w) > left_size.w)
        left_roi.x = (left_size.w - left_roi.w);
    else
        left_roi.x += step_w;
}

void static setBtnLocation(lv_obj_rvm *obj, uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           lv_obj_t *parent_scr, uint16_t btn_num,
                           lv_event_cb_t cb, char *picName, char *focus_picName)
{
    obj->obj = parent_scr;
    obj->refPos.x = x;
    obj->refPos.y = y;
    obj->width = width;
    obj->height = height;
    obj->label = (char *)rvm_clabel[btn_num];
    obj->event_cb = cb;
    _createBtn(obj, picName, focus_picName);
    obj++;
}

void create_rvm_switch(lv_obj_rvm *obj, uint32_t num, int scr_id,
                       lv_coord_t width, lv_coord_t height, lv_coord_t xpos,
                       lv_coord_t ypos)
{
    lv_obj_t *parent_scr = NULL;
    lv_event_cb_t eHandler[] = {
        _rvm_left_cb,
        _rvm_up_cb,
        _rvm_down_cb,
        _rvm_right_cb,
    };

    memset(obj, 0, sizeof(lv_obj_rvm) * num);
    parent_scr = lv_disp_get_scr_act(get_display(scr_id));

    setBtnLocation(obj, xpos + width + 40 * 2, ypos, width, height, parent_scr,
                   0, eHandler[0],
                   (char *)("S:/early_app/fastrvm/direction_left.png"),
                   (char *)("S:/early_app/fastrvm/direction_left_focus.png"));

    setBtnLocation(obj, xpos + width * 2 + 40 * 3, ypos - 80 - 30, width,
                   height, parent_scr, 1, eHandler[1],
                   (char *)("S:/early_app/fastrvm/direction_up.png"),
                   (char *)("S:/early_app/fastrvm/direction_up_focus.png"));

    setBtnLocation(obj, xpos + width * 2 + 40 * 3, ypos + 80 + 30, width,
                   height, parent_scr, 2, eHandler[2],
                   (char *)("S:/early_app/fastrvm/direction_down.png"),
                   (char *)("S:/early_app/fastrvm/direction_down_focus.png"));

    setBtnLocation(obj, xpos + width * 3 + 40 * 4, ypos, width, height,
                   parent_scr, 3, eHandler[3],
                   (char *)("S:/early_app/fastrvm/direction_right.png"),
                   (char *)("S:/early_app/fastrvm/direction_right_focus.png"));
}

static int *aswdev_swap;
void set_off_swap(int *swap) { aswdev_swap = swap; }

static int distortion_off;
static void _distortion_on_off_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESSED != event)
        return;

    if (aswdev_swap == NULL)
        return;

    if (distortion_off == 1) {
        distortion_off = 0;
        *aswdev_swap = 0;
        lv_imgbtn_set_src(back_camera_btns[0], LV_STATE_DEFAULT, &on);
    } else {
        distortion_off = 1;
        *aswdev_swap = 1;
        lv_imgbtn_set_src(back_camera_btns[0], LV_STATE_DEFAULT, &off);
    }
    emirror_info("----------_distortion_on_off_cb-----------------\n");
}

void create_distortion_switch(lv_obj_rvm *obj, uint32_t num, int scr_id,
                              lv_coord_t width, lv_coord_t height,
                              lv_coord_t xpos, lv_coord_t ypos)
{
    lv_obj_t *parent_scr = NULL;
    lv_event_cb_t eHandler[] = {
        _distortion_on_off_cb,
    };

    memset(obj, 0, sizeof(lv_obj_rvm) * num);
    parent_scr = lv_disp_get_scr_act(get_display(scr_id));

    setBtnLocation(obj, xpos, ypos, width, height, parent_scr, 0, eHandler[0],
                   (char *)&on, (char *)(""));
}

void stop_lvgl(void) { rvm_lvgl_stop = true; }

void lvglkickmainloop(void)
{
    while (rvm_lvgl_stop == false) {
        lvgl_mainloop();
    }
}
