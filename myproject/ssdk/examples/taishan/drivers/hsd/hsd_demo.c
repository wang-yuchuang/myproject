/**
 * @file hsd_demo.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: hsd channel on/off demo.
 *
 * Revision History:
 * -----------------
 */

#include <CLI_type.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/gpio.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <pinmux_cfg.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;

#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

#define PINCTRL_INSTANCE(pin)                                                  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

static void hsd_set(uint32_t pin, uint32_t level)
{
    gpio_dev_t *gpio_dev = GPIO_INSTANCE(pin);
    pinctrl_t *pinctrl = PINCTRL_INSTANCE(pin);

    pinctrl_config(pinctrl, pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);

    gpio_config(gpio_dev, pin, GPIO_OUT);
    gpio_set(gpio_dev, pin, level);
}

static void hsd_usage(void)
{
    printf("hsd_demo <pin> <on or off>");
    printf("\t\t:hsd output enable or disable\n");

    printf("\ne3 gateway hsd pin list:\n");
    printf("hsd1:179\thsd2:176\nhsd3:178\thsd4:181\n");
    printf("hsd5:185\thsd6:175\nhsd7:184\thsd8:182\n");
}

static int hsd_demo_main(int argc, char *argv[])
{
    uint32_t pin;

    if (argc == 0) {
        hsd_usage();
        return 0;
    }

    pin = atoi(argv[0]);
    if (!strcmp(argv[1], "on")) {
        hsd_set(pin, 1);
    } else if (!strcmp(argv[1], "off")) {
        hsd_set(pin, 0);
    } else {
        printf("error cmd\n");
    }

    return CLI_FALSE;
}

CLI_CMD("hsd_demo", "\r\nhsd_demo:\r\n hsd device demo\n", hsd_demo_main);