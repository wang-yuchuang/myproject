/**
 * @file mipi_hsd123_serdes_1920x720_lcd.c
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
    i2c_adap_dev_t *adap = panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;

    LOGD(" link_status:%d\n", serdes->link_status);
    if (serdes->link_status == SER_DUAL_LINK) {
        panel_check_link(panel_info);
        ds90ub941as_Enable_Dualdsi_DualPort(adap, ser_addr);
    }
    return 0;
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    int ret = 0;

    LOGD("%s enter\n", __func__);

#ifndef CONFIG_MP
    i2c_adap_dev_t *adap = panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;
    uint32_t des_addr = panel_info->serdes->des_addr;

    if (panel_info->serdes->link_status != SER_DUAL_LINK)
        ds90ub941as_init(adap, ser_addr);

    ds90ub948_init(adap, des_addr);
#endif
    LOGD("%s down\n", __func__);

    return ret;
}

#if defined(S3SIN1)
static struct sis_info info_sis[2] = {
    {
        .pos_x = 0,
        .pos_y = 0,
        .width = 640,
        .height = 600,
    },
    {
        .pos_x = 640,
        .pos_y = 0,
        .width = 1280,
        .height = 600,
    }
};

static struct sis_info screen = {
    .pos_x = 0,
    .pos_y = 0,
    .width = 1920,
    .height = 720,
};
#endif

struct sdm_panel mipi_hsd123_serdes_1920x720_lcd = {
    .panel_name = "mipi_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 51,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
#if defined(S3SIN1)
    .sis_num = 2,
    .sis = info_sis,
    .rtos_screen = &screen,
#else
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
#endif
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_backlight_init = sdm_panel_default_pwmbl_init,
};