
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "i2c_cfg.h"
#include "tca_gpio_cfg.h"

#ifdef CONFIG_TCA6408

gpio_dev_t g_gpio_tca6408;

void board_tca6408_init(void)
{
    tca6408_gpio_init(&g_gpio_tca6408, &g_i2c9_adap_dev, 0, NULL);
}
#endif


#ifdef CONFIG_TCA9539

gpio_dev_t g_gpio_tca9539;

void board_tca9539_init(void)
{
    tca9539_gpio_init(&g_gpio_tca9539, &g_i2c10_adap_dev, 0, NULL);
}
#endif

