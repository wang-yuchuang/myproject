/*
 * emirror_lvgl.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: emirror-app UI interface
 *
 * Revision History:
 * -----------------
 */

#ifndef __EMIRROR_LVGL_H__
#define __EMIRROR_LVGL_H__
#include "lvgl_gui.h"

typedef struct _lv_obj_rvm {
    lv_obj_t *obj;
    lv_point_t refPos;
    lv_coord_t width;
    lv_coord_t height;
    lv_event_cb_t event_cb;
    char *label;
} lv_obj_rvm;

void set_full_size(int32_t w, int32_t h);
void set_default_roi(int32_t x, int32_t y, int32_t w, int32_t h);
void get_cur_roi(int *update, int32_t *x, int32_t *y, int32_t *w, int32_t *h);

void get_screen_size(lv_obj_t *scr, uint32_t *width, uint32_t *height);
void create_rvm_switch(lv_obj_rvm *obj, uint32_t num, int scr_id,
                       lv_coord_t width, lv_coord_t height, lv_coord_t xpos,
                       lv_coord_t ypos);

void set_off_swap(int *swap);
void create_distortion_switch(lv_obj_rvm *obj, uint32_t num, int scr_id,
                       lv_coord_t width, lv_coord_t height, lv_coord_t xpos,
                       lv_coord_t ypos);
void stop_lvgl(void);
void lvglkickmainloop(void);
#endif
