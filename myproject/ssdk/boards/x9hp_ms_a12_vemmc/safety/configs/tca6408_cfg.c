/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_TCA6408

#include "i2c_cfg.h"
#include "tca6408_cfg.h"

gpio_dev_t g_gpio_tca6408;

void board_tca6408_init(void)
{
    tca6408_gpio_init(&g_gpio_tca6408, &g_i2c9_adap_dev, 0, NULL);
    g_tca6408_conf.addr = 0x20;
}
#endif
