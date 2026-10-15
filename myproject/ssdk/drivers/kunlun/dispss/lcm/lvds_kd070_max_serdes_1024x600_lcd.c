/*
* lvds_kd070_max_serdes_1024x600_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/03/2023 BI create this file
*/

#include <board.h>
#include "sdm_panel.h"
#include <disp_panels.h>
#include "maxim_rw.h"

#define CLU_MAX96735_I2C    0x20
#define CLU_MAX96752_I2C    0x48

static struct display_timing timing = {
    .hactive = 1024,
    .hfront_porch = 120,
    .hback_porch = 120,
    .hsync_len = 80,
    .vactive = 600,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 15,
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
    uint8_t value = 0;

    i2c_adap_dev_t *i2c_handle = panel_info->serdes->adap;
    uint32_t max96735_i2c_addr = panel_info->serdes->ser_addr;
    uint32_t max96752_i2c_addr = panel_info->serdes->des_addr;

    /************************************************************************************************/
    /* 2. Config 96735                                      */
    /************************************************************************************************/

    maxim_read_reg(i2c_handle, max96735_i2c_addr, 0x24, &value);
    printf("96735 READ ID=0x%x\n", value);

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x700, 0x8B);
    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x760, 0x8B);

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x701, 0xE4);
    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x761, 0xE4);

    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x01, 0x08);
    maxim_write_reg(i2c_handle, max96735_i2c_addr, 0x7C0, 0x20);

    /************************************************************************************************/
    /* 3. Config 96752                                      */
    /************************************************************************************************/
    maxim_write_reg(i2c_handle, max96752_i2c_addr, 0x01CE, 0x46);
    return 0;
}

struct sdm_panel lvds_kd070_max_serdes_1024x600_lcd = {
    .panel_name = "lvds_kd070_max_serdes_1024x600_lcd",
    .if_type = IF_TYPE_LVDS,
    .fps = 60,
    .display_timing = &timing,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};


