/**
 * @file lvds_hsd123_serdes_1920x720_x9u_b_lcd.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <sdm_panel.h>
#include <disp_panels.h>
#include <ds90ub9xx.h>

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 40,
    .hback_porch = 40,
    .hsync_len = 40,

    .vactive = 720,
    .vfront_porch = 50,
    .vback_porch = 50,
    .vsync_len = 10,

    .dsp_clk_pol = LCM_POLARITY_FALLING,
    .de_pol      = LCM_POLARITY_HIGH_ACTIVE,
    .vsync_pol   = LCM_POLARITY_HIGH_ACTIVE,
    .hsync_pol   = LCM_POLARITY_HIGH_ACTIVE,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

static int panel_post_begin(struct sdm_panel_info * panel_info)
{
    struct serdes_info *serdes = panel_info->serdes;

    if (serdes->link_status == SER_DUAL_LINK)
        panel_check_link(panel_info);

    return 0;
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    int ret = 0;
    LOGD("%s enter\n", __func__);

    i2c_adap_dev_t *adap = panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;
    uint32_t des_addr = panel_info->serdes->des_addr;

    ds90ub9xx_write_reg(adap, ser_addr, 0x4f, 0xc0);
    ret = ds90ub9xx_write_reg(adap, ser_addr, 0x5b, 0x20);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);

#ifndef BACKLIGHT_GPIO
    ret = ds90ub94x_948_gpio_config(adap, ser_addr, des_addr, GPIO0, OUTPUT, DES_PORT0);
    if (ret < 0)
        LOGE("%s backlight GPIO0 config error !\n", __func__);
#endif

    LOGD("%s down\n", __func__);
    return 0;

}

struct sdm_panel lvds_hsd123_serdes_1920x720_x9u_b_lcd = {
    .panel_name = "lvds_hsd123_serdes_1920x720_x9u_b_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE,
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 56,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};