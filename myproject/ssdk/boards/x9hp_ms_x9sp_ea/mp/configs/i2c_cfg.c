/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_I2C

#include <types.h>
#include <board.h>
#include <driver/kunlun/i2c/dw_i2c.h>
#include "i2c_cfg.h"

/* i2c device drivers */
dw_i2c_context g_dw_i2c6_cxt;
i2c_adap_dev_t g_i2c6_adap_dev;
dw_i2c_context g_dw_i2c9_cxt;
i2c_adap_dev_t g_i2c9_adap_dev;
dw_i2c_context g_dw_i2c10_cxt;
i2c_adap_dev_t g_i2c10_adap_dev;
dw_i2c_context g_dw_i2c11_cxt;
i2c_adap_dev_t g_i2c11_adap_dev;
dw_i2c_context g_dw_i2c14_cxt;
i2c_adap_dev_t g_i2c14_adap_dev;
dw_i2c_context g_dw_i2c15_cxt;
i2c_adap_dev_t g_i2c15_adap_dev;
dw_i2c_context g_dw_i2c16_cxt;
i2c_adap_dev_t g_i2c16_adap_dev;

/* */
i2c_adap_cfg_t i2c6_adap_cfg = {
    .base = DEVICE_BASE(I2C6),
    .irq = DEVICE_INTR(I2C6_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_SSPEED,
};

/* camera serdes */
i2c_adap_cfg_t i2c9_adap_cfg = {
    .base = DEVICE_BASE(I2C9),
    .irq = DEVICE_INTR(I2C9_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_SSPEED,
};

/*MAX96722 CSI2 OF A*/
/*MAX96706 (0xF0)*/
/*TCA9539QPWRQ1 (0x75)*/
/*MAX2008 (0x2A)*/
/*TLV320AIC23BIPWRQ1 (0x34)*/
/*iPod_Auth_2.0C/NM (0x22)*/
i2c_adap_cfg_t i2c10_adap_cfg = {
    .base = DEVICE_BASE(I2C10),
    .irq = DEVICE_INTR(I2C10_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};

/* */
i2c_adap_cfg_t i2c11_adap_cfg = {
    .base = DEVICE_BASE(I2C11),
    .irq = DEVICE_INTR(I2C11_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};

/* display serdes */
i2c_adap_cfg_t i2c14_adap_cfg = {
    .base = DEVICE_BASE(I2C14),
    .irq = DEVICE_INTR(I2C14_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
/* display serdes */
i2c_adap_cfg_t i2c15_adap_cfg = {
    .base = DEVICE_BASE(I2C15),
    .irq = DEVICE_INTR(I2C15_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
/* display serdes */
i2c_adap_cfg_t i2c16_adap_cfg = {
    .base = DEVICE_BASE(I2C16),
    .irq = DEVICE_INTR(I2C16_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};

/* i2c early initialization */
void board_i2c_early_init(void)
{
    dw_i2c_adap_probe(&g_i2c6_adap_dev, &i2c6_adap_cfg, &g_dw_i2c6_cxt, 6);
    dw_i2c_adap_probe(&g_i2c9_adap_dev, &i2c9_adap_cfg, &g_dw_i2c9_cxt, 9);
    dw_i2c_adap_probe(&g_i2c10_adap_dev, &i2c10_adap_cfg, &g_dw_i2c10_cxt, 10);
    dw_i2c_adap_probe(&g_i2c11_adap_dev, &i2c11_adap_cfg, &g_dw_i2c11_cxt, 11);
    dw_i2c_adap_probe(&g_i2c14_adap_dev, &i2c14_adap_cfg, &g_dw_i2c14_cxt, 14);
    dw_i2c_adap_probe(&g_i2c15_adap_dev, &i2c15_adap_cfg, &g_dw_i2c15_cxt, 15);
    dw_i2c_adap_probe(&g_i2c16_adap_dev, &i2c16_adap_cfg, &g_dw_i2c16_cxt, 16);
}

/* i2c initialization */
void board_i2c_init(void)
{
    i2c_adap_init(&g_i2c6_adap_dev);
    i2c_adap_init(&g_i2c9_adap_dev);
    i2c_adap_init(&g_i2c10_adap_dev);
    i2c_adap_init(&g_i2c11_adap_dev);
    i2c_adap_init(&g_i2c14_adap_dev);
    i2c_adap_init(&g_i2c15_adap_dev);
    i2c_adap_init(&g_i2c16_adap_dev);
}
#endif
