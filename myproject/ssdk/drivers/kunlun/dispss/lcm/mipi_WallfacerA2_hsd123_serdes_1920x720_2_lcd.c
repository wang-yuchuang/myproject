/*
* mipi_WallfacerA2_hsd123_serdes_dual_1920x720_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 09/15/2022 BI create this file
*/
#include <board.h>
#include "sdm_panel.h"
#include <disp_panels.h>
#include "maxim_rw.h"

#define CLU_MAX96789_I2C    0x40
#define CLU_MAX96752_I2C    0x48
#define CLU_MAX96752_2_I2C  0x4a

#define MAX96789_PWDNB PortConf_PIN_GPIO_C11

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 136,
    .hback_porch = 80,
    .hsync_len = 24,
    .vactive = 720,
    .vfront_porch = 14,
    .vback_porch = 25,
    .vsync_len = 12,
    .pll_clk = 700000000,
};

static struct info_mipi mipi_data = {
    .burst_mode = 0,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 600000,
};

static int panel_post_begin(struct sdm_panel_info * panel_info)
{
    return 0;
}

static void max789752_pattern_test(i2c_adap_dev_t *i2c_handle)
{
    const u8 max96789_i2c_addr = 0x40;

    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x0053, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x0057, 0x1);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x005B, 0x2);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x005F, 0x3);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01C8, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01C8, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01C8, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CA, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CB, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CC, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CD, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CE, 0x65);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01CF, 0x40);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D0, 0x19);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D1, 0x04);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D2, 0x10);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D3, 0x00);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D4, 0x00);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D5, 0x00);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D6, 0x00);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D7, 0x18);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D8, 0x08);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01D9, 0x58);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DA, 0x03);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DB, 0x03);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DC, 0x01);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DD, 0x38);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DE, 0x98);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01DF, 0x07);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E0, 0x80);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E1, 0x00);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E2, 0xF0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E3, 0x02);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E4, 0xD0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E5, 0x2);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01E6, 0x4);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x0007, 0x0);
    maxim_write_reg(i2c_handle,max96789_i2c_addr, 0x01C8, 0xE3);
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    i2c_adap_dev_t *i2c_handle = panel_info->serdes->adap;
    uint32_t max96789_i2c_addr = panel_info->serdes->ser_addr;
    uint32_t max96752_i2c_addr = panel_info->serdes->des_addr;

    uint8_t value = 0;

    /************************************************************************************************/
    /* 2. Config 96789                                      */
    /************************************************************************************************/
    /*read id check*/
    maxim_read_reg(i2c_handle, max96789_i2c_addr, 0x0D, &value);
    if (value != 0x80) {
        printf("max96789 is offline\n");
        return -1;
    }

    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x50, 0x2);
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x1ce, 0x4e);

    // for backlight configuration, max96789 gpio9 pass through max96752 gpio2
    maxim_write_reg(i2c_handle, max96789_i2c_addr, 0x2D9, 0x03);
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x207, 0xa9);
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x208, 0x69);
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x206, 0x04);

    return 0;
}

struct sdm_panel mipi_WallfacerA2_hsd123_serdesl_1920x720_2_lcd = {
    .panel_name = "mipi_WallfacerA2_hsd123_serdesl_1920x720_2_lcd",
    .if_type = IF_TYPE_DSI,
    .fps = 60,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_backlight_init = sdm_panel_default_pwmbl_init,
};


