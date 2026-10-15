/*
* lvds_WallfacerA2_hsd123_serdes_1920x720_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 09/09/2022 BI create this file
*/

#include <board.h>
#include "sdm_panel.h"
#include <disp_panels.h>
#include "maxim_rw.h"

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

static int max_pattern_test(i2c_adap_dev_t *i2c_handle) {
    const u8 max96735_i2c_addr = 0x20;

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1C8, 0xE3);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1C9, 0x09);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1ca, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1cb, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1cc, 0x00);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1cd, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1ce, 0x17);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1cf, 0xc4);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d0, 0x17);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d1, 0x2d);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d2, 0x7c);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d3, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d4, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d5, 0x00);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d6, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d7, 0x18);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d8, 0x07);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1d9, 0xd4);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1da, 0x02);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1db, 0xf0);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1dc, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1dd, 0xbe);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1de, 0x58);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1df, 0x07);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e0, 0x80);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e1, 0x00);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e2, 0x6c);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e3, 0x02);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e4, 0xd0);

    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e5, 0x02);
    maxim_write_reg(i2c_handle,max96735_i2c_addr, 0x1e6, 0x04);

    return 0;
}

static int panel_post_begin(struct sdm_panel_info * panel_info)
{
    return 0;
}

static int panel_post_end(struct sdm_panel_info * panel_info)
{
    return 0;
}

static int panel_pre_power_on(struct sdm_panel_info * panel_info)
{
    uint8_t value = 0;

    i2c_adap_dev_t *i2c_handle = panel_info->serdes->adap;
    uint32_t max96735_i2c_addr = panel_info->serdes->ser_addr;
    uint32_t max96752_i2c_addr = panel_info->serdes->des_addr;
    uint32_t flags = panel_info->serdes->flags;

    /************************************************************************************************/
    /* 2. Config 96735                                      */
    /************************************************************************************************/
    /*read id check*/
    maxim_read_reg(i2c_handle, max96735_i2c_addr, 0x24, &value);
    if (value != 0x25) {
        printf("max96735 is offline\n");
        return -1;
    }

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x700, 0x8B);

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x760, 0x8B);

    if (flags & SERDES_PORTA_LINESWAP)
        maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x701, 0x1B);
    else
        maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x701, 0xE4);

    if (flags & SERDES_PORTB_LINESWAP)
        maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x761, 0x1B);
    else
        maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x761, 0xE4);

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x01, 0x08);

    /************************************************************************************************/
    /* 3. Config 96752                                      */
    /************************************************************************************************/
    if (flags & SERDES_PORTAB_SWAP)
        maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x01CE, 0x5e);
    else
        maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x01CE, 0x4e);
//  mdelay(50);

    //for backlight, max96735 gpio2 pass through max96752 gpio2
    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x2C4, 0x03);
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x206, 0x04);

    //max_pattern_test();
    return 0;
}

struct sdm_panel lvds_WallfacerA2_hsd123_serdes_1920x720_lcd = {
    .panel_name = "lvds_WallfacerA2_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .fps = 60,
    .display_timing = &timing,
    .panel_pre_power_on = panel_pre_power_on,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
    .panel_backlight_init = sdm_panel_default_pwmbl_init,
};

