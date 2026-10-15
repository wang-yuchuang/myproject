/**
 * @file touch_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:touch config header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_TOUCH_CONFIG_H
#define INCLUDE_TOUCH_CONFIG_H
#ifdef CONFIG_TOUCH_KUNLUN_PLATFORM

#include <types.h>
#include <driver/pinctrl.h>
#include <driver/gpio.h>
#include <driver/kunlun/touch/touch.h>


extern gpio_dev_t g_gpio_dev;
extern touch_dev_t* g_touch_dev[SCREEN_MAX];

extern int32_t board_touch_init();

#endif /* end CONFIG_TOUCH_KUNLUN_PLATFORM */
#endif /* INCLUDE_TOUCH_CONFIG_H */
/* end of header file touch_cfg.h */