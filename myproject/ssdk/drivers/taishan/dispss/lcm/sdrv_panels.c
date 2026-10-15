/*
* lvds_reg.h
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: lvds register defs file.
*
* Revision History:
* -----------------
* 011, 11/12/2021 BILLY create this file
*/
#include <driver/taishan/dispss/disp.h>

extern struct sdrv_panel lvds_800x600_lcd;
extern struct sdrv_panel hdmi_adv_800x600_lcd;
extern struct sdrv_panel rgb_1280x720_lcd;
extern struct sdrv_panel rgb_800x600_lcd_z1;
extern struct sdrv_panel lvds_1440x540_lcd;
extern struct sdrv_panel lvds_1280x768_lcd;
extern struct sdrv_panel rgb_480x272_lcd;
extern struct sdrv_panel lvds_lt9211_to_mipi_480x1920_lcd;
extern struct sdrv_panel lvds_1920x1080_lcd;
extern struct sdrv_panel lvds_lt9211_1port_to_2port_1920x720_lcd;

struct sdrv_panel *sdrv_panels[] = {
#if CONFIG_LVDS_800X600_LCD
    &lvds_800x600_lcd,
#endif

#if CONFIG_HDMI_ADV_800X600_LCD
    &hdmi_adv_800x600_lcd,
#endif

#if CONFIG_RGB_1280X800_LCD
    &rgb_1280x720_lcd,
#endif

#if CONFIG_RGB_800X600_LCD_Z1
    &rgb_800x600_lcd_z1,
#endif

#if CONFIG_LVDS_1440X540_LCD
    &lvds_1440x540_lcd,
#endif

#if CONFIG_LVDS_1280X768_LCD
    &lvds_1280x768_lcd,
#endif

#if CONFIG_RGB_480X272_LCD
    &rgb_480x272_lcd,
#endif

#if CONFIG_LVDS_LT9211_TO_MIPI_480x1920_LCD
    &lvds_lt9211_to_mipi_480x1920_lcd,
#endif

#if CONFIG_LVDS_1920X1080_LCD
    &lvds_1920x1080_lcd,
#endif

#if CONFIG_LVDS_LT9211_1PORT_TO_2PORT_1920x720_LCD
    &lvds_lt9211_1port_to_2port_1920x720_lcd,
#endif
};
