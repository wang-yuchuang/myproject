/*
* lvds_lt9211_to_mipi_480x1920_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 02/24/2022 BILLY create this file
*/
#include <driver/taishan/dispss/disp.h>

static struct display_timing timing = {
    .hactive = 480,
    .hfront_porch = 30,
    .hback_porch = 30,
    .hsync_len = 30,

    .vactive = 1920,
    .vfront_porch = 6,
    .vback_porch = 6,
    .vsync_len = 6,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

struct sdrv_panel lvds_lt9211_to_mipi_480x1920_lcd = {
    .panel_name = "lvds_lt9211_to_mipi_480x1920_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};

