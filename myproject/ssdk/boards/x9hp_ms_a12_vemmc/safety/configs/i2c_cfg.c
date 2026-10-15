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

/* i2c device cfg info */
#ifdef I2C1_CFG_ENABLE
dw_i2c_context g_dw_i2c1_cxt;
i2c_adap_dev_t g_i2c1_adap_dev;
i2c_adap_cfg_t i2c1_adap_cfg = {
    .base = DEVICE_BASE(I2C1),
    .irq = DEVICE_INTR(I2C1_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C2_CFG_ENABLE
dw_i2c_context g_dw_i2c2_cxt;
i2c_adap_dev_t g_i2c2_adap_dev;
i2c_adap_cfg_t i2c2_adap_cfg = {
    .base = DEVICE_BASE(I2C2),
    .irq = DEVICE_INTR(I2C2_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C3_CFG_ENABLE
dw_i2c_context g_dw_i2c3_cxt;
i2c_adap_dev_t g_i2c3_adap_dev;
i2c_adap_cfg_t i2c3_adap_cfg = {
    .base = DEVICE_BASE(I2C3),
    .irq = DEVICE_INTR(I2C3_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C4_CFG_ENABLE
dw_i2c_context g_dw_i2c4_cxt;
i2c_adap_dev_t g_i2c4_adap_dev;
i2c_adap_cfg_t i2c4_adap_cfg = {
    .base = DEVICE_BASE(I2C4),
    .irq = DEVICE_INTR(I2C4_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C5_CFG_ENABLE
dw_i2c_context g_dw_i2c5_cxt;
i2c_adap_dev_t g_i2c5_adap_dev;
i2c_adap_cfg_t i2c5_adap_cfg = {
    .base = DEVICE_BASE(I2C5),
    .irq = DEVICE_INTR(I2C5_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C6_CFG_ENABLE
dw_i2c_context g_dw_i2c6_cxt;
i2c_adap_dev_t g_i2c6_adap_dev;
i2c_adap_cfg_t i2c6_adap_cfg = {
    .base = DEVICE_BASE(I2C6),
    .irq = DEVICE_INTR(I2C6_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C7_CFG_ENABLE
dw_i2c_context g_dw_i2c7_cxt;
i2c_adap_dev_t g_i2c7_adap_dev;
i2c_adap_cfg_t i2c7_adap_cfg = {
    .base = DEVICE_BASE(I2C7),
    .irq = DEVICE_INTR(I2C7_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C8_CFG_ENABLE
dw_i2c_context g_dw_i2c8_cxt;
i2c_adap_dev_t g_i2c8_adap_dev;
i2c_adap_cfg_t i2c8_adap_cfg = {
    .base = DEVICE_BASE(I2C8),
    .irq = DEVICE_INTR(I2C8_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C9_CFG_ENABLE
dw_i2c_context g_dw_i2c9_cxt;
i2c_adap_dev_t g_i2c9_adap_dev;
i2c_adap_cfg_t i2c9_adap_cfg = {
    .base = DEVICE_BASE(I2C9),
    .irq = DEVICE_INTR(I2C9_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C10_CFG_ENABLE
dw_i2c_context g_dw_i2c10_cxt;
i2c_adap_dev_t g_i2c10_adap_dev;
i2c_adap_cfg_t i2c10_adap_cfg = {
    .base = DEVICE_BASE(I2C10),
    .irq = DEVICE_INTR(I2C10_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C11_CFG_ENABLE
dw_i2c_context g_dw_i2c11_cxt;
i2c_adap_dev_t g_i2c11_adap_dev;
i2c_adap_cfg_t i2c11_adap_cfg = {
    .base = DEVICE_BASE(I2C11),
    .irq = DEVICE_INTR(I2C11_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C12_CFG_ENABLE
dw_i2c_context g_dw_i2c12_cxt;
i2c_adap_dev_t g_i2c12_adap_dev;
i2c_adap_cfg_t i2c12_adap_cfg = {
    .base = DEVICE_BASE(I2C12),
    .irq = DEVICE_INTR(I2C12_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C13_CFG_ENABLE
dw_i2c_context g_dw_i2c13_cxt;
i2c_adap_dev_t g_i2c13_adap_dev;
i2c_adap_cfg_t i2c13_adap_cfg = {
    .base = DEVICE_BASE(I2C13),
    .irq = DEVICE_INTR(I2C13_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C14_CFG_ENABLE
dw_i2c_context g_dw_i2c14_cxt;
i2c_adap_dev_t g_i2c14_adap_dev;
i2c_adap_cfg_t i2c14_adap_cfg = {
    .base = DEVICE_BASE(I2C14),
    .irq = DEVICE_INTR(I2C14_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C15_CFG_ENABLE
dw_i2c_context g_dw_i2c15_cxt;
i2c_adap_dev_t g_i2c15_adap_dev;
i2c_adap_cfg_t i2c15_adap_cfg = {
    .base = DEVICE_BASE(I2C15),
    .irq = DEVICE_INTR(I2C15_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

#ifdef I2C16_CFG_ENABLE
dw_i2c_context g_dw_i2c16_cxt;
i2c_adap_dev_t g_i2c16_adap_dev;
i2c_adap_cfg_t i2c16_adap_cfg = {
    .base = DEVICE_BASE(I2C16),
    .irq = DEVICE_INTR(I2C16_IC),
    .clk = 24000000,
    .enable = true,
    .opmode = I2C_MASTER,
    .speed = I2C_FSPEED,
};
#endif

/* i2c early initialization */
void board_i2c_early_init(void)
{
#ifdef I2C1_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c1_adap_dev, &i2c1_adap_cfg, &g_dw_i2c1_cxt, 1);
#endif

#ifdef I2C2_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c2_adap_dev, &i2c2_adap_cfg, &g_dw_i2c2_cxt, 2);
#endif

#ifdef I2C3_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c3_adap_dev, &i2c3_adap_cfg, &g_dw_i2c3_cxt, 3);
#endif

#ifdef I2C4_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c4_adap_dev, &i2c4_adap_cfg, &g_dw_i2c4_cxt, 4);
#endif

#ifdef I2C5_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c5_adap_dev, &i2c5_adap_cfg, &g_dw_i2c5_cxt, 5);
#endif

#ifdef I2C6_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c6_adap_dev, &i2c6_adap_cfg, &g_dw_i2c6_cxt, 6);
#endif

#ifdef I2C7_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c7_adap_dev, &i2c7_adap_cfg, &g_dw_i2c7_cxt, 7);
#endif

#ifdef I2C8_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c8_adap_dev, &i2c8_adap_cfg, &g_dw_i2c8_cxt, 8);
#endif

#ifdef I2C9_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c9_adap_dev, &i2c9_adap_cfg, &g_dw_i2c9_cxt, 9);
#endif

#ifdef I2C10_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c10_adap_dev, &i2c10_adap_cfg, &g_dw_i2c10_cxt, 10);
#endif

#ifdef I2C11_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c11_adap_dev, &i2c11_adap_cfg, &g_dw_i2c11_cxt, 11);
#endif

#ifdef I2C12_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c12_adap_dev, &i2c12_adap_cfg, &g_dw_i2c12_cxt, 12);
#endif

#ifdef I2C13_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c13_adap_dev, &i2c13_adap_cfg, &g_dw_i2c13_cxt, 13);
#endif

#ifdef I2C14_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c14_adap_dev, &i2c14_adap_cfg, &g_dw_i2c14_cxt, 14);
#endif

#ifdef I2C15_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c15_adap_dev, &i2c15_adap_cfg, &g_dw_i2c15_cxt, 15);
#endif

#ifdef I2C16_CFG_ENABLE
    dw_i2c_adap_probe(&g_i2c16_adap_dev, &i2c16_adap_cfg, &g_dw_i2c16_cxt, 16);
#endif
}

/* i2c initialization */
void board_i2c_init(void)
{
#ifdef I2C1_CFG_ENABLE
    i2c_adap_init(&g_i2c1_adap_dev);
#endif

#ifdef I2C2_CFG_ENABLE
    i2c_adap_init(&g_i2c2_adap_dev);
#endif

#ifdef I2C3_CFG_ENABLE
    i2c_adap_init(&g_i2c3_adap_dev);
#endif

#ifdef I2C4_CFG_ENABLE
    i2c_adap_init(&g_i2c4_adap_dev);
#endif

#ifdef I2C5_CFG_ENABLE
    i2c_adap_init(&g_i2c5_adap_dev);
#endif

#ifdef I2C6_CFG_ENABLE
    i2c_adap_init(&g_i2c6_adap_dev);
#endif

#ifdef I2C7_CFG_ENABLE
    i2c_adap_init(&g_i2c7_adap_dev);
#endif

#ifdef I2C8_CFG_ENABLE
    i2c_adap_init(&g_i2c8_adap_dev);
#endif

#ifdef I2C9_CFG_ENABLE
    i2c_adap_init(&g_i2c9_adap_dev);
#endif

#ifdef I2C10_CFG_ENABLE
    i2c_adap_init(&g_i2c10_adap_dev);
#endif

#ifdef I2C11_CFG_ENABLE
    i2c_adap_init(&g_i2c11_adap_dev);
#endif

#ifdef I2C12_CFG_ENABLE
    i2c_adap_init(&g_i2c12_adap_dev);
#endif

#ifdef I2C13_CFG_ENABLE
    i2c_adap_init(&g_i2c13_adap_dev);
#endif

#ifdef I2C14_CFG_ENABLE
    i2c_adap_init(&g_i2c14_adap_dev);
#endif

#ifdef I2C15_CFG_ENABLE
    i2c_adap_init(&g_i2c15_adap_dev);
#endif

#ifdef I2C16_CFG_ENABLE
    i2c_adap_init(&g_i2c16_adap_dev);
#endif
}
#endif


