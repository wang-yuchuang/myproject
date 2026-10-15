/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "driver/pinctrl.h"
#include "pinmux_cfg.h"

extern gpio_dev_t g_gpio_dev;

void canfd2_trcv_sleep(void)
{
    gpio_dev_t *gpio_dev;
    gpio_dev = &g_gpio_dev;
    gpio_config(gpio_dev, 45, GPIO_OUT);
    gpio_set(gpio_dev, 45, false);
}

void canfd2_trcv_wakeup(void)
{
    gpio_dev_t *gpio_dev;
    gpio_dev = &g_gpio_dev;
    gpio_config(gpio_dev, 45, GPIO_OUT);
    gpio_set(gpio_dev, 45, true);
    gpio_config(gpio_dev, 24, GPIO_OUT);
    gpio_set(gpio_dev, 24, true);
}
