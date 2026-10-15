/**
 * @file lsd_demo.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: lsd demo application.
 *
 * Revision History:
 * -----------------
 */
#include <CLI_type.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/gpio.h>
#include <driver/irq.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/btm/sdrv_btm_drv.h>
#include <driver/timer.h>
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

sdrv_btm_dev_t g_sdrv_btm_diagTimer;
timer_dev_t *diagTimer;

enum lsd_status {
    LSD_OFF,
    LSD_ON,
};

static int status = LSD_OFF;

static uint32_t lsd_pin;

static void lsd_set(uint32_t, uint8_t);

static const sdrv_btm_cfg_t diagTimer_config = {
    .base = DEVICE_BASE(BTM3),
    .irq = DEVICE_INTR(BTM3_O_BTM),
    .tmr_id = SDRV_BTM_G1,
    .tmr_cfg = {
        .si_val = 23,
        .inc_val = 1,
        .frc_rld_rst_cnt_en = true,
        .term_use_mode = SDRV_BTM_DIRECT,
        .cmp_use_mode = SDRV_BTM_DIRECT,
        .cnt_dir = SDRV_BTM_CNT_UP,
        .cnt_mode = SDRV_BTM_CONTINOUS_MODE,
    }};

static void lsd_diag(void *arg)
{
    uint8_t ret;
    uint32_t lsd_sts = GPIO_E11;

    switch (lsd_pin) {
    case GPIO_E10:
        lsd_sts = GPIO_E11; // LSD1 Status PIN
        break;
    case GPIO_E17:
        lsd_sts = GPIO_E18; // LSD2 Status PIN
        break;
    case GPIO_E19:
        lsd_sts = GPIO_E20; // LSD3 Status PIN
        break;
    case GPIO_E21:
        lsd_sts = GPIO_E22; // LSD4 Status PIN
        break;
    case GPIO_E25:
        lsd_sts = GPIO_E26; // LSD5 Status PIN
        break;
    case GPIO_E23:
        lsd_sts = GPIO_E24; // LSD6 Status PIN
        break;
    case GPIO_E13:
        lsd_sts = GPIO_E14; // LSD7 Status PIN
        break;
    case GPIO_E15:
        lsd_sts = GPIO_E16; // LSD8 Status PIN
        break;

    default:
        printf("lsd pin error\n");
    }

    gpio_dev_t *gpio_dev = GPIO_INSTANCE(lsd_sts);
    pinctrl_t *pinctrl = PINCTRL_INSTANCE(lsd_sts);

    pinctrl_config(pinctrl, lsd_sts, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);
    gpio_config(gpio_dev, lsd_sts, GPIO_IN);

    ret = gpio_get(gpio_dev, lsd_sts);
    if (ret == 0) {
        lsd_set(lsd_pin, 0);
        timer_stop(diagTimer);
        printf("lsd over temperature\n");
    }
}

static void lsd_set(uint32_t lsd_pin, uint8_t on)
{
    gpio_dev_t *gpio_dev = GPIO_INSTANCE(lsd_pin);
    pinctrl_t *pinctrl = PINCTRL_INSTANCE(lsd_pin);

    pinctrl_config(pinctrl, lsd_pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);
    gpio_config(gpio_dev, lsd_pin, GPIO_OUT);

    if (!diagTimer) {
        diagTimer = sdrv_btm_probe(&g_sdrv_btm_diagTimer, &diagTimer_config);
        timer_init(diagTimer);
    }

    if (on) {
        if (status != LSD_ON) {
            gpio_set(gpio_dev, lsd_pin, 1);
            timer_set_callback(diagTimer, lsd_diag, NULL);
            timer_start(diagTimer, TMR_TYPE_PERIOD, 500);
            status = LSD_ON;
        }
    } else {
        if (status != LSD_OFF) {
            gpio_set(gpio_dev, lsd_pin, 0);
            status = LSD_OFF;
        }
    }
}

static void lsd_usage(void)
{
    printf("lsd_demo <pin> <on or off>");
    printf("\t\t:lsd output enable or disable & diag\n");

    printf("\ne3 gateway lsd pin list:\n");
    printf("lsd1:197\tlsd2:204\nlsd3:206\tlsd4:208\n");
    printf("lsd5:212\tlsd6:210\nlsd7:200\tlsd8:202\n");
}

static int lsd_demo_main(int argc, char *argv[])
{
    if (argc == 0) {
        lsd_usage();
        return 0;
    }

    lsd_pin = atoi(argv[0]);
    if (!strcmp(argv[1], "on")) {
        lsd_set(lsd_pin, 1);

    } else if (!strcmp(argv[1], "off")) {
        lsd_set(lsd_pin, 0);
    } else {
        printf("error cmd\n");
    }

    return CLI_FALSE;
}

CLI_CMD("lsd_demo", "\r\nlsd_demo:\r\n lsd device demo\n", lsd_demo_main);