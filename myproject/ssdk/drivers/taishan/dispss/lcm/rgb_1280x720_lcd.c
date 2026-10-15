/*
* rgb_1280x720_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/11/2021 BILLY create this file
*/
#include <driver/taishan/dispss/disp.h>

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 110,
    .hback_porch = 220,
    .hsync_len = 40,

    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,
};

struct sdrv_panel rgb_1280x720_lcd = {
    .panel_name = "rgb_1280x720_lcd",
    .if_type = IF_TYPE_NONE,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};

