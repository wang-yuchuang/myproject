#ifndef _LVGL_LCD_CONFIG_H
#define _LVGL_LCD_CONFIG_H

/* lvgl include */
#include "lvgl_gui.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

lv_disp_t *lvgl_lcd_display_init(sdm_display_t *sdm);

#ifdef __cplusplus
}
#endif

#endif /* _LVGL_LCD_CONFIG_H */
