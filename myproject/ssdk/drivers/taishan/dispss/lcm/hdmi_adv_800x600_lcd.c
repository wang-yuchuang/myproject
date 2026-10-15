/*
* hdmi_adv_800x600_lcd.c
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
    .hfront_porch = 40,
    .hback_porch = 88,
    .hsync_len = 128,

    .vactive = 600,
    .vfront_porch = 2,
    .vback_porch = 23,
    .vsync_len = 4,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,
};

struct sdrv_panel hdmi_adv_800x600_lcd = {
    .panel_name = "hdmi_adv_800x600_lcd",
    .if_type = IF_TYPE_HDMI,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 24,
    .timing = &timing,
};

