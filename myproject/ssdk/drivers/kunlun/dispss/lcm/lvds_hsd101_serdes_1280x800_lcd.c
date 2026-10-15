/**
 * @file lvds_hsd101_serdes_1280x800_lcd.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <sdm_panel.h>
#include <disp_panels.h>
#include <ds90ub9xx.h>

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 48,
    .hback_porch = 80,
    .hsync_len = 32,

    .vactive = 800,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 3,

    .dsp_clk_pol = LCM_POLARITY_FALLING,
    .de_pol      = LCM_POLARITY_HIGH_ACTIVE,
    .vsync_pol   = LCM_POLARITY_HIGH_ACTIVE,
    .hsync_pol   = LCM_POLARITY_HIGH_ACTIVE,

    .map_format = LVDS_MAP_FORMAT_JEIDA,
};

static int panel_post_begin(struct sdm_panel_info * panel_info)
{
    return 0;
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    i2c_adap_dev_t *adap = panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;
    uint32_t des_addr = panel_info->serdes->des_addr;

    LOGD("%s enter\n", __func__);

    ds90ub947_init(adap, ser_addr);
    mdelay(10);
    ds90ub948_match_947_init(adap, des_addr);

    LOGD("%s down\n", __func__);
    return 0;
}

struct sdm_panel lvds_hsd101_serdes_1280x800_lcd = {
    .panel_name = "lvds_hsd101_serdes_1280x800_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE,
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};