/**
 * @file mipi_hsd123_serdes_1920x720_2_lcd.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <sdm_panel.h>
#include <disp_panels.h>
#include <ds90ub9xx.h>

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 40,
    .hback_porch = 40,
    .hsync_len = 40,
    .vactive = 720,
    .vfront_porch = 50,
    .vback_porch = 50,
    .vsync_len = 10,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 600 * 1000,
};

static int panel_post_begin(struct sdm_panel_info * panel_info)
{
    struct serdes_info *serdes = panel_info->serdes;
    i2c_adap_dev_t *adap = (i2c_adap_dev_t *)serdes->adap;
    uint32_t ser_addr = serdes->ser_addr;

    if (serdes->link_status == SER_DUAL_LINK) {
        panel_check_link(panel_info);
        ds90ub941as_Enable_Dualdsi_DualPort(adap, ser_addr);
    }
    return 0;
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    int ret = 0;
#if 0
    uint8_t bl_gpio = D_GPIO0;
    i2c_adap_dev_t *adap = panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;
    uint32_t des_addr = panel_info->serdes->des_addr;

    LOGD("%s enter\n", __func__);

    if (panel_info->serdes->link_status == SER_DUAL_LINK)
        bl_gpio = GPIO0;

#ifndef BACKLIGHT_GPIO
    ret = ds90ub94x_948_gpio_config(adap, ser_addr, des_addr, bl_gpio, OUTPUT, DES_PORT0);
    if (ret < 0)
        LOGE("%s backlight GPIO0 config error !\n", __func__);
#endif
    LOGD("%s down\n", __func__);

#endif
    return ret;
}

struct sdm_panel mipi_hsd123_serdes_1920x720_2_lcd = {
    .panel_name = "mipi_hsd123_serdes_1920x720_2_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 51,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_backlight_init = sdm_panel_default_pwmbl_init,
};