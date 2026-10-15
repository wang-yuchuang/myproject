/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_TCA_GPIO_CFG_H
#define BOARDS_TCA_GPIO_CFG_H
#include "driver/i2c.h"

#ifdef CONFIG_TCA6408
#include "driver/kunlun/gpio/tca6408.h"
/* exported tca9539 gpio devices */
extern gpio_dev_t g_gpio_tca6408;
/* tca9539 init functions */
extern void board_tca6408_init(void);
#endif

#ifdef CONFIG_TCA9539
#include "driver/kunlun/gpio/tca9539.h"
/* exported tca9539 gpio devices */
extern gpio_dev_t g_gpio_tca9539;
/* tca9539 init functions */
extern void board_tca9539_init(void);

#endif

#endif
