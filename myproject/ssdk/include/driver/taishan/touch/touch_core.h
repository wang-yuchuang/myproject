/**
 * @file touch_core.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */

#ifndef _TOUCH_CORE_H_
#define _TOUCH_CORE_H_

#include <driver/i2c.h>
#include <driver/gpio.h>

#define TS_ENABLE true
#define TS_DISABLE false

struct ts_coord_config {
    int x_max;
    int y_max;
    int x_offset;
    int y_offset;
    int swap_xy;
    int invert_x;
    int invert_y;
};

struct touch_device {
    bool enable;
    const char *device_name;
    i2c_adap_dev_t **adap;
    uint16_t i2c_addr;
    uint32_t screen_id;
    uint32_t rst_pin;
    uint32_t irq_pin;
    gpio_dev_t *rst_gpiodev;
    gpio_dev_t *irq_gpiodev;
    struct ts_coord_config coord_cfg;
};

struct touch_driver {
    const char *driver_name;
    int (*probe)(const struct touch_device *ts_dev);
};

extern void touch_init(struct touch_device *ts_dev, int ts_dev_num);
#endif

