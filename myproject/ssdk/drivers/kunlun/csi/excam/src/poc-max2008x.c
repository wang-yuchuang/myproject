/*
 * Copyright (C) 2021 Semidrive, Inc. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 */
#include "sdrv_serdes_core.h"
#include "exvdev.h"
#include "tca_gpio_cfg.h"
#include "poc-max2008x.h"

enum POC_CHIP_INDEX {
    POC_CHIP_INDEX_MDESER0 = 0,
    POC_CHIP_INDEX_MDESER1 = 1,
    POC_CHIP_INDEX_R_MDESER0 = 2,
    POC_CHIP_INDEX_R_MDESER1 = 3,
    POC_CHIP_INDEX_MDESER_RESER = 4,
};

enum __MAX2008X_REG_LIST {
    MAX2008X_CMD_MASK = 0x00,
    MAX2008X_CMD_CONFIG = 0x01,
    MAX2008X_CMD_ID = 0x02,
    MAX2008X_CMD_STAT1 = 0x03,
    MAX2008X_CMD_STAT2_0 = 0x04,
    MAX2008X_CMD_STAT2_1 = 0x05,
    MAX2008X_CMD_ADC1 = 0x06,
    MAX2008X_CMD_ADC2 = 0x07,
    MAX2008X_CMD_ADC3 = 0x08,
    MAX2008X_CMD_ADC4 = 0x09,
};

#define MAX_POC_CHIP_NUM (5)

#define DISABLE_ALL_LINK_POWER 0x10
#define ENABLE_ALL_LINK_POWER 0x1f

static int max2008x_read_reg(i2c_adap_dev_t *client, u8 address, u8 reg,
                             u8 *val)
{
    return serdes_read_reg_8(client, address, reg, val);
}

static int max2008x_write_reg(i2c_adap_dev_t *i2c_handle, u8 device_address,
                              u8 reg, u8 val)
{
    return serdes_write_reg_8(i2c_handle, device_address, reg, val);
}

int poc_power(i2c_adap_dev_t *client, struct poc_cfg *cfg, int chan, int enable)
{
    u8 val;
    int ret = -1;

    // 1. identify which extgpio is connected to the poc and enable/disable poc_en gpio
#ifdef CONFIG_TCA6408
    if (strcmp(cfg->gpioext_name, TCA6408_NAME) == 0) {
        gpio_config(&g_gpio_tca6408, cfg->poc_gpio, GPIO_OUT);
        gpio_set(&g_gpio_tca6408, cfg->poc_gpio, enable);

    }
#endif
#ifdef CONFIG_TCA9539
    if (strcmp(cfg->gpioext_name, TCA9539_NAME) == 0) {
        serdes_err("gpioext name %s, line %d\n",cfg->gpioext_name, __LINE__);
        gpio_config(&g_gpio_tca9539, cfg->poc_gpio, GPIO_OUT);
        gpio_set(&g_gpio_tca9539, cfg->poc_gpio, enable);

    }
#endif

    // 3. set max2008x reg value
    max2008x_read_reg(client, cfg->poc_i2c_addr, MAX2008X_CMD_CONFIG, &val);

    if (enable == 1) {
        if (chan != 0xf)
            val = val | (1 << chan);
        else
            val = val | chan;

        serdes_info("chan 0x%x set value 0x%x\n", chan, val);
        /*
         * currently for some daughter board(DB5071) the four channels need to
         * be all opened to avoid reverse channel iic failure
         * Will revise later
         */
        ret = max2008x_write_reg(client, cfg->poc_i2c_addr,
                                 MAX2008X_CMD_CONFIG, 0x1f);
    }
    else if (enable == 0) {
        if (chan != 0xf)
            val = val & (~(1 << chan));
        else
            val = val & (~chan);

        ret = max2008x_write_reg(client, cfg->poc_i2c_addr,
                                 MAX2008X_CMD_CONFIG, 0x10);
    }

    max2008x_read_reg(client, cfg->poc_i2c_addr, MAX2008X_CMD_CONFIG, &val);
    serdes_info("MAX208X_CMD_CONFIG = 0x%x\n", val);
    return ret;
}

static int max2008x_device_init(i2c_adap_dev_t *client, struct poc_cfg *cfg)
{
    // Disable camera module power in default.
    max2008x_write_reg(client, cfg->poc_i2c_addr, MAX2008X_CMD_CONFIG,
                       DISABLE_ALL_LINK_POWER);
    return 0;
}
