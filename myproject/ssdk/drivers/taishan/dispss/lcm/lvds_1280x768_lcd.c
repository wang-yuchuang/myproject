/*
* lvds_1280x768_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 01/06/2022 BILLY create this file
*/
#include <driver/taishan/dispss/disp.h>

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 28,
    .hback_porch = 28,
    .hsync_len = 28,

    .vactive = 768,
    .vfront_porch = 15,
    .vback_porch = 15,
    .vsync_len = 13,
    .fps = 50.458,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

struct sdrv_panel lvds_1280x768_lcd = {
    .panel_name = "lvds_1280x768_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};

