/*
 * tca9539.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TCA9539_H__
#define __TCA9539_H__

#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "driver/gpio.h"

#define TCA9539_NAME "tca9539"

enum tca9539_port {
    TCA9539_P00 = 0,
    TCA9539_P01,
    TCA9539_P02,
    TCA9539_P03,
    TCA9539_P04,
    TCA9539_P05,
    TCA9539_P06,
    TCA9539_P07,
    TCA9539_P10,
    TCA9539_P11,
    TCA9539_P12,
    TCA9539_P13,
    TCA9539_P14,
    TCA9539_P15,
    TCA9539_P16,
    TCA9539_P17,
};


void tca9539_gpio_init(gpio_dev_t *dev, i2c_adap_dev_t *i2c_dev, int irq_cnt, const uint32_t *irqs);

#endif
