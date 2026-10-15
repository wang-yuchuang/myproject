/*
 * board_pinmux.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level pinmux configuration.
 *
 * Revision History:
 * -----------------
 */
#include <types.h>
#include <bits.h>
#include <param.h>
#include <reg.h>
#include <string.h>
#include <cmsis_os2.h>
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "pinmux_cfg.h"
#include "board.h"

/*
 * Each domain in the SOC has one pinctrl controller:
 *   safety: 5 dedicated JTAG pins, GPIO 0 ~ 47, SEM_FAULT, GPIO_SAF
 *   ap: GPIO 48 ~ 155
 *   rtc: 12 dedicated sys/rtc pins
 *
 * To make life easier, we define following "virtual" pinctrl devices,
 * each controlling a continuous range of pins.
 */

/* pinctrl_safety [0:47]:      GPIO 0~47 */
#define PINCTRL_SAFETY_PIN_START    0
#define PINCTRL_SAFETY_PIN_NUM      48

/* pinctrl_ap [48:155]:        GPIO 48~155 */
#define PINCTRL_AP_PIN_START        48
#define PINCTRL_AP_PIN_NUM          108
#define PINCTRL_AP_EMMC_PIN_START   132
#define PINCTRL_AP_EMMC_PIN_NUM     24

/* Number of "normal" GPIO pins - GPIO [0:155] */
#define PINCTRL_GPIO_PIN_NR         156

/* pinctrl_safety1 [156:160]:  JTAG TMS/TCK/TDI/TDO/TRST_N */
#define PINCTRL_SAFETY1_PIN_START   156
#define PINCTRL_SAFETY1_PIN_NUM     5

/* pinctrl_safety2 [161:162]:  SEM_FAULT, GPIO_SAF */
#define PINCTRL_SAFETY2_PIN_START   161
#define PINCTRL_SAFETY2_PIN_NUM     2

/* pinctrl_rtc [163:174]:      sys/rtc pins */
#define PINCTRL_RTC_PIN_START       163
#define PINCTRL_RTC_PIN_NUM         12

static sdrv_pinctrl_t pinctrl_rtc_dev = {
    .base = APB_IOMUXC_RTC_BASE,
    .pad_reg_offset = 0,
    .mux_reg_offset = 0x200,
};

static sdrv_pinctrl_t pinctrl_safety_dev = {
    .base = APB_IOMUXC_SAF_BASE,
    .pad_reg_offset = 0x14,
    .mux_reg_offset = 0x200,
};

static sdrv_pinctrl_t pinctrl_ap_dev = {
    .base = APB_IOMUXC_SEC_BASE,
    .pad_reg_offset = 0,
    .mux_reg_offset = 0x200,
    .emmc_pad_start = PINCTRL_AP_EMMC_PIN_START,
    .emmc_pad_nr = PINCTRL_AP_EMMC_PIN_NUM,
};

pinctrl_t g_pinctrl_rtc = {
    .pin_start = PINCTRL_RTC_PIN_START,
    .pin_nr = PINCTRL_RTC_PIN_NUM,
};

pinctrl_t g_pinctrl_safety = {
    .pin_start = PINCTRL_SAFETY_PIN_START,
    .pin_nr = PINCTRL_SAFETY_PIN_NUM,
};

pinctrl_t g_pinctrl_ap = {
    .pin_start = PINCTRL_AP_PIN_START,
    .pin_nr = PINCTRL_AP_PIN_NUM,
};

#if 0
static const uint32_t gpio_ap1_irqs[] = {
    GPIO4_GPIO_INT0_INTR_NUM,
    GPIO4_GPIO_INT1_INTR_NUM,
    GPIO4_GPIO_INT2_INTR_NUM,
    GPIO4_GPIO_INT3_INTR_NUM,
    GPIO4_GPIO_INT4_INTR_NUM,
    GPIO4_GPIO_INT5_INTR_NUM,
};
#endif
static const uint32_t gpio_ap1_irqs[] = {
    GPIO4_GPIO_0_INTR_NUM,
    GPIO4_GPIO_1_INTR_NUM,
    GPIO4_GPIO_2_INTR_NUM,
    GPIO4_GPIO_3_INTR_NUM,
    GPIO4_GPIO_4_INTR_NUM,
    GPIO4_GPIO_5_INTR_NUM,
};


gpio_dev_t g_gpio_ap1;
gpio_dev_t g_gpio_ap2;

void board_port_init(void)
{
    /* Initialize pinctrl devices. */
    printf("*******board_port_init*****\n");

    sdrv_pinctrl_init(&g_pinctrl_rtc, &pinctrl_rtc_dev);
    sdrv_pinctrl_init(&g_pinctrl_safety, &pinctrl_safety_dev);
    sdrv_pinctrl_init(&g_pinctrl_ap, &pinctrl_ap_dev);

    /* Initialize the GPIO controller. */
    sdrv_gpio_init(&g_gpio_ap1, APB_GPIO4_BASE,
                   0, gpio_ap1_irqs);
}
