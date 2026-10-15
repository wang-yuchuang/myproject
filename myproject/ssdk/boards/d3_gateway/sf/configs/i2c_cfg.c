
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_I2C

#include <board.h>
#include <clock_cfg.h>
#include <driver/taishan/i2c/sdrv_i2c_drv.h>
#include <types.h>

#include "i2c_cfg.h"

static sdrv_i2c_dev_t i2c6_sdrv_dev;
i2c_adap_dev_t *i2c6_adap_dev;

i2c_adap_cfg_t i2c6_adap_cfg = {
    .base = DEVICE_BASE(I2C6),
    .irq = DEVICE_INTR(I2C6),
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_SSPEED,
};

/* i2c early initialization */
void board_i2c_early_init(void)
{

    i2c6_adap_cfg.clk = clk_get_rate(CLK_NODE(g_ckgen_ip_i2c_sf_5_to_8));
    i2c6_adap_dev = sdrv_i2c_adap_probe(&i2c6_sdrv_dev, &i2c6_adap_cfg, 6);
}

/* i2c initialization */
void board_i2c_init(void) { i2c_adap_init(i2c6_adap_dev); }
#endif
