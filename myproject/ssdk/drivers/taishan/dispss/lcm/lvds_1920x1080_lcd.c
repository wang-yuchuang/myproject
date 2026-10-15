/*
* lvds_1920x1080_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 14/06/2022 BILLY create this file
*/
#include <driver/taishan/dispss/disp.h>

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 80,
    .hback_porch = 80,
    .hsync_len = 80,

    .vactive = 1080,
    .vfront_porch = 15,
    .vback_porch = 15,
    .vsync_len = 14,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

struct sdrv_panel lvds_1920x1080_lcd = {
    .panel_name = "lvds_1920x1080_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};
