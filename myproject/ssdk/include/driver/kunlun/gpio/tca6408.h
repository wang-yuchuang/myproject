/*
 * tca6408.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TCA6408_H__
#define __TCA6408_H__

#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "driver/gpio.h"

#define TCA6408_NAME "tca6408"

typedef enum {
    TCA6408_P00 = 0x0,
    TCA6408_P01,
    TCA6408_P02,
    TCA6408_P03,
    TCA6408_P04,
    TCA6408_P05,
    TCA6408_P06,
    TCA6408_P07,
} tca6408_pin_t;

typedef struct tca6408_conf {
    uint8_t addr;
} tca6408_conf_t;

extern tca6408_conf_t g_tca6408_conf;

void tca6408_gpio_init(gpio_dev_t *dev, i2c_adap_dev_t *i2c_dev, int irq_cnt, const uint32_t *irqs);

#endif
