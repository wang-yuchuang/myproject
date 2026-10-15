//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_DISPSS
#include <disp_hal.h>
#include <disp_ssystem_link.h>
#include <disp_panels.h>

#include "disp_cfg.h"

extern void sdm_display_init(struct display_resource *disp_res, int res_count, int os);

struct display_resource disp_res[] = {
	{
		.name = "Infotainment",
		.type = INFOTAINMENT,
		.dp_res = DP_RES_DP1,
		.dc_res = DC_RES_DC1,
		.if_res = IF_RES_LVDS1_LVDS2,
		.lvds_mode = LVDS_MODE_DUAL,
		.pixel_bpp = PIXEL_BPP_24,
		.gui_enable = 0,
	},

};

struct sdm_panel dumb_lcd = {
    .panel_name = "dumb_lcd",
};

struct serdes_info dumb_serdes = {
    .adap = NULL,
    .ser_addr = 0x0,
    .des_addr = 0x0,
};
struct sdm_panel_info display_dumb = {
    .panel = &dumb_lcd,
    .serdes = &dumb_serdes,
};

struct sdm_panel_info *display_0_panels[] = {
    &display_dumb,
};

/*ivi-master*/
struct sdm_panel_info *display_1_panels[] = {
    &display_dumb,
};

/*ivi-slave*/
struct sdm_panel_info *display_2_panels[] = {
    &display_dumb,
};

struct sdm_panel_info **registered_panels[] = {
    display_0_panels,
    display_1_panels,
    display_2_panels,
};

static int res_count = ARRAY_SIZE(disp_res);

void board_disp_init(void)
{
    int os = 0;
    os = DIL2;

    sdm_display_init(disp_res, res_count, os);
}

#endif
