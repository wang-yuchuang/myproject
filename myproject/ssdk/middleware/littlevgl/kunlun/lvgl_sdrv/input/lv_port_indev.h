
/**
 * @file lv_port_indev_templ.h
 *
 */

 /*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#ifdef CONFIG_APP_CLUSTER
#define LVGL_DISPLAY_WIDTH 1440
#define LVGL_DISPLAY_HEIGHT 540
#define LVGL_DISPLAY_OFFSET_X 0
#define LVGL_DISPLAY_OFFSET_Y 0
#elif defined CONFIG_APP_EMIRROR
#define LVGL_DISPLAY_WIDTH 64
#define LVGL_DISPLAY_HEIGHT 64
#define LVGL_DISPLAY_OFFSET_X 1216
#define LVGL_DISPLAY_OFFSET_Y 352
#else
#define LVGL_DISPLAY_WIDTH 1280
#define LVGL_DISPLAY_HEIGHT 768
#define LVGL_DISPLAY_OFFSET_X 0
#define LVGL_DISPLAY_OFFSET_Y 0
#endif

void lv_port_indev_init(lv_disp_t *disp);

#ifdef __cplusplus
} /* extern "C" */
#endif



#endif /*LV_PORT_INDEV_H*/

#endif /*Disable/Enable content*/
