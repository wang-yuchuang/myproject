/*
* rgb_800x600_lcd.c
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
    .hactive = 800,
    .hfront_porch = 176,
    .hback_porch = 44,
    .hsync_len = 20,

    .vactive = 600,
    .vfront_porch = 43,
    .vback_porch = 13,
    .vsync_len = 10,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,
};

struct sdrv_panel rgb_800x600_lcd_z1 = {
    .panel_name = "rgb_800x600_lcd_z1",
    .if_type = IF_TYPE_NONE,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};

