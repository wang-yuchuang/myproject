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
#include "i2c_cfg.h"

extern void sdm_display_init(struct display_resource *disp_res, int res_count, int os);

struct display_resource disp_res[] = {
	{
		.name = "Cluster",
		.type = CLUSTER,
		.dp_res = DP_RES_DP2,
		.dc_res = DC_RES_DC2,
		.if_res = IF_RES_LVDS1_LVDS2,
		.lvds_mode = LVDS_MODE_DUAL,
		.pixel_bpp = PIXEL_BPP_24,
		.gui_enable = 1,
		.i2c_adap = &g_i2c14_adap_dev,
		.ser_addr = 0x1A,
		.des_addr = 0x2C,
		.panel = &lvds_hsd123_serdes_1920x720_lcd,
	},
	{
		.name = "Infotainment",
		.type = INFOTAINMENT,
		.dp_res = DP_RES_DP1,
		.dc_res = DC_RES_DC1,
		.if_res = IF_RES_MIPI_DSI1,
		.lvds_mode = LVDS_MODE_NC,
		.pixel_bpp = PIXEL_BPP_24,
		.gui_enable = 1,
		.i2c_adap = &g_i2c16_adap_dev,
		.ser_addr = 0x0C,
		.des_addr = 0x2C,
		.panel = &mipi_hsd123_serdes_1920x720_lcd,
	},
	{
		.name = "Entertainment",
		.type = ENTERTAINMENT,
		.dp_res = DP_RES_DP3,
		.dc_res = DC_RES_DC4,
		.if_res = IF_RES_MIPI_DSI2,
		.lvds_mode = LVDS_MODE_NC,
		.pixel_bpp = PIXEL_BPP_24,
		.gui_enable = 1,
		.i2c_adap = &g_i2c16_adap_dev,
		.ser_addr = 0x1A,
		.des_addr = 0x3C,
		.panel = &mipi_hsd123_serdes_1920x720_2_lcd,
	},
	{
		.name = "ControlPanel",
		.type = CONTROLPANEL,
		.dp_res = DP_RES_NC,
		.dc_res = DC_RES_DC3,
		.if_res = IF_RES_LVDS3_LVDS4,
		.lvds_mode = LVDS_MODE_DUAL,
		.pixel_bpp = PIXEL_BPP_24,
		.gui_enable = 1,
		.i2c_adap = &g_i2c15_adap_dev,
		.ser_addr = 0x1A,
		.des_addr = 0x2C,
		.panel = &lvds_hsd123_serdes_1920x720_lcd,
	},
};

static int res_count = ARRAY_SIZE(disp_res);

void board_disp_init(void)
{
    int os = 0;
    os = SSDK;

    sdm_display_init(disp_res, res_count, os);
}
#endif /* CONFIG_DISPSS */
