/*
* rgb_480x272_lcd.c
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
    .hactive = 480,
    .hfront_porch = 8,
    .hback_porch = 43,
    .hsync_len = 4,

    .vactive = 272,
    .vfront_porch = 8,
    .vback_porch = 12,
    .vsync_len = 4,
};

struct sdrv_panel rgb_480x272_lcd = {
    .panel_name = "rgb_480x272_lcd",
    .if_type = IF_TYPE_NONE,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .pixel_bpp = 16,
    .timing = &timing,
};

