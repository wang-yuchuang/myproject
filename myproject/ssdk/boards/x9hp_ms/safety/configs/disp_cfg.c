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
	},
};


/*cluster*/
struct serdes_info display_0_serdes = {
    .adap = &g_i2c14_adap_dev,
    .ser_addr = 0x1A,
    .des_addr = 0x2C,
    .link_status = SER_DUAL_LINK,
};
struct serdes_info display_1_serdes = {
    .adap = &g_i2c16_adap_dev,
    .ser_addr = 0x0C,
    .des_addr = 0x2C,
    .link_status = SER_DUAL_LINK,
};
struct serdes_info display_2_serdes = {
    .adap = &g_i2c16_adap_dev,
    .ser_addr = 0x1A,
    .des_addr = 0x3C,
    .link_status = SER_DUAL_LINK,
};
struct serdes_info display_3_serdes = {
    .adap = &g_i2c15_adap_dev,
    .ser_addr = 0x1A,
    .des_addr = 0x2C,
    .link_status = SER_DUAL_LINK,
};

#ifdef CONFIG_PWM

pwm_instance_t disp0_pwm5, disp1_2_pwm3, disp3_pwm4;

static drv_pwm_simple_cfg_t disp0_pwm5_cfg = {
    .phy_num = 5,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_A] = {70, DRV_PWM_PHASE_POLARITY_POS},    // LVDS12
    .cmp_cfg[DRV_PWM_CHN_B] = {70, DRV_PWM_PHASE_POLARITY_POS},
};

static drv_pwm_simple_cfg_t disp1_2_pwm3_cfg = {
    .phy_num = 3,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_A] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI1
    .cmp_cfg[DRV_PWM_CHN_B] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI2
};

#if 0
static drv_pwm_simple_cfg_t disp2_pwm3_cfg = {
    .phy_num = 3,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
//    .cmp_cfg[DRV_PWM_CHN_A] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI1
    .cmp_cfg[DRV_PWM_CHN_B] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI2
};
#endif

static drv_pwm_simple_cfg_t disp3_pwm4_cfg = {
    .phy_num = 4,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_C_D_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_D] = {70, DRV_PWM_PHASE_POLARITY_POS},    // LVDS34
};

struct backlight_info display_0_backlight = {
    .base = DEVICE_BASE(PWM5),
    .pwm_ins = &disp0_pwm5,
    .pwm_cfg = &disp0_pwm5_cfg,
    .pwm_chn = DRV_PWM_CHN_A,
    .priv = &display_0_serdes,
};

struct backlight_info display_1_backlight = {
    .base = DEVICE_BASE(PWM3),
    .pwm_ins = &disp1_2_pwm3,
    .pwm_cfg = &disp1_2_pwm3_cfg,
    .pwm_chn = DRV_PWM_CHN_A,
    .priv = &display_1_serdes,
};
struct backlight_info display_2_backlight = {
    .base = DEVICE_BASE(PWM3),
    .pwm_ins = &disp1_2_pwm3,
    .pwm_cfg = &disp1_2_pwm3_cfg,
    .pwm_chn = DRV_PWM_CHN_B,
    .priv = &display_2_serdes,
};
struct backlight_info display_3_backlight = {
    .base = DEVICE_BASE(PWM4),
    .pwm_ins = &disp3_pwm4,
    .pwm_cfg = &disp3_pwm4_cfg,
    .pwm_chn = DRV_PWM_CHN_D,
    .priv = &display_3_serdes,
};
#else
struct backlight_info display_0_backlight = {
    .base = 0x0,
    .pwm_cfg = NULL,
    .priv = NULL,
};

struct backlight_info display_1_backlight = {
    .base = 0x0,
    .pwm_cfg = NULL,
    .priv = NULL,
};
struct backlight_info display_2_backlight = {
    .base = 0x0,
    .pwm_cfg = NULL,
    .priv = NULL,
};
struct backlight_info display_3_backlight = {
    .base = 0x0,
    .pwm_cfg = NULL,
    .priv = NULL,
};
#endif
/*set lcds for each display*/
/* avoid an error with IAR compiler */
struct sdm_panel dumb_lcd = {
    .panel_name = "dumb_lcd",
};

struct serdes_info dumb_serdes = {
    .adap = &g_i2c14_adap_dev,
    .ser_addr = 0x0,
    .des_addr = 0x0,
};
struct sdm_panel_info display_dumb = {
    .panel = &dumb_lcd,
    .serdes = &dumb_serdes,
};

struct sdm_panel_info display_0_panel = {
    .panel = &lvds_hsd123_serdes_1920x720_lcd,
    .serdes = &display_0_serdes,
    .backlight = &display_0_backlight,
};
struct sdm_panel_info display_1_panel = {
    .panel = &mipi_hsd123_serdes_1920x720_lcd,
    .serdes = &display_1_serdes,
    .backlight = &display_1_backlight,
};
struct sdm_panel_info display_2_panel = {
    .panel = &mipi_hsd123_serdes_1920x720_2_lcd,
    .serdes = &display_2_serdes,
    .backlight = &display_2_backlight,
};
struct sdm_panel_info display_3_panel = {
    .panel = &lvds_hsd123_serdes_1920x720_lcd,
    .serdes = &display_3_serdes,
    .backlight = &display_3_backlight,
};

struct sdm_panel_info *display_0_panels[] = {
    &display_0_panel,
    &display_dumb,
};

/*ivi-master*/
struct sdm_panel_info *display_1_panels[] = {
    &display_1_panel,
    &display_dumb,
};

/*ivi-slave*/
struct sdm_panel_info *display_2_panels[] = {
    &display_2_panel,
    &display_dumb,
};

/*control panel*/
struct sdm_panel_info *display_3_panels[] = {
    &display_3_panel,
    &display_dumb,
};

struct sdm_panel_info **registered_panels[] = {
    display_0_panels,
    display_1_panels,
    display_2_panels,
    display_3_panels,
};

static int res_count = ARRAY_SIZE(disp_res);

void board_disp_init(void)
{
    int os = 0;
    os = SSDK;

    sdm_display_init(disp_res, res_count, os);
}
#endif /* CONFIG_DISPSS */
