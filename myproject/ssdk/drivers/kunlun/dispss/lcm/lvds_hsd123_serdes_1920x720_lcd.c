/**
 * @file lvds_hsd123_serdes_1920x720_lcd.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <board.h>
#include "sdm_panel.h"
#include <disp_panels.h>
#include "ds90ub9xx.h"

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

static struct info_lvds lvds_data = {
    .vbias_sel = 0x1,
    .txen = 0x1,
    .txdrv_vol = LVDS_PAD_TXDRV_VOL_55_220MV,
    .txdrv_cur = LVDS_PAD_TXDRV_CUR_RTO_0,
    .tx_cm = 0x1,
    .rterm_en = 0x1,
    .bias_en = 0x1,
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

#if defined(X9U_A)
    ds90ub9xx_write_reg(adap, ser_addr, 0x4f, 0xc0);
    ds90ub9xx_write_reg(adap, ser_addr, 0x03, 0x08);
    ds90ub9xx_write_reg(adap, ser_addr, 0x17, 0x9e);
#else
#ifndef CONFIG_MP
    ret = ds90ub9xx_write_reg(adap, ser_addr, 0x4f, 0x80);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);
#endif
#endif
    //COAX:Enable FPD-Link III for coaxial cabling
#ifndef CONFIG_MP
    ret = ds90ub9xx_write_reg(adap, ser_addr, 0x5b, 0x20);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);
#endif

    //@NOTE disable serdes 947 avmute function
    //      fix cluster display err while android install apk
    ret = ds90ub9xx_write_reg(adap, ser_addr, 0x04, 0x90);

    LOGD("%s down\n", __func__);
    return ret;
}

struct sdm_panel lvds_hsd123_serdes_1920x720_lcd = {
    .panel_name = "lvds_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE,
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 56,
    .display_timing = &timing,
    .lvds = &lvds_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_backlight_init = sdm_panel_default_pwmbl_init,
};
