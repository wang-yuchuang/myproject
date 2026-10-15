/*
 * board_pinmux.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level pinmux configuration.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_PINCTRL

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

static sdrv_pinctrl_t pinctrl_ap_dev = {
    .base = APB_IOMUXC_SEC_BASE,
    .pad_reg_offset = 0,
    .mux_reg_offset = 0x200,
    .emmc_pad_start = PINCTRL_AP_EMMC_PIN_START,
    .emmc_pad_nr = PINCTRL_AP_EMMC_PIN_NUM,
};

pinctrl_t g_pinctrl_safety = {
    .pin_start = PINCTRL_SAFETY_PIN_START,
    .pin_nr = PINCTRL_SAFETY_PIN_NUM,
};

pinctrl_t g_pinctrl_ap = {
    .pin_start = PINCTRL_AP_PIN_START,
    .pin_nr = PINCTRL_AP_PIN_NUM,
};

static const uint32_t gpio_mp_irqs[] = {
    GPIO3_GPIO_INT0_INTR_NUM,
    GPIO3_GPIO_INT1_INTR_NUM,
    GPIO3_GPIO_INT2_INTR_NUM,
    GPIO3_GPIO_INT3_INTR_NUM,
    GPIO3_GPIO_INT4_INTR_NUM,
    GPIO3_GPIO_INT5_INTR_NUM,
};

/* Just a name for gpio dev, have nothing to do with safety domain/core. */
gpio_dev_t g_gpio_dev;

static void pin_mux_init(void)
{
    for (int i = 0; i < NUM_OF_CONFIGURED_PINS; i++) {
        pin_settings_config_t *config = &g_pin_init_config[i];
        if ((uint32_t)config->controller_id == GPIO_MP) {
            if (config->pin_index >= PINCTRL_AP_PIN_START) {
                pinctrl_t *pinctrl = &g_pinctrl_ap;
                uint32_t drive = config->open_drain == PIN_OPEN_DRAIN_ENABLED ?
                                PINCTRL_DRIVE_OPEN_DRAIN : PINCTRL_DRIVE_PUSH_PULL;

                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FUNCTION, config->mux);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE, drive);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_PULL, config->pull_config);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE_STRENGTH, config->drive_strength);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_SLEW_RATE, config->slew_rate);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_INPUT_SCHMITT, config->input_select);
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FORCE_INPUT, config->force_input);
            }
            if (config->mux == PAD_MUX_FUNC_GPIO) {
                gpio_type_e gpio_type = GPIO_OUT;
                if (config->data_direction == PIN_INPUT_DIRECTION) {
                    switch (config->interrupt_config) {
                        case PIN_INTERRUPT_DISABLED:
                            gpio_type = GPIO_IN;
                            break;
                        case PIN_INTERRUPT_HIGH_LEVEL:
                            gpio_type = GPIO_INTR_HIGH_LEVEL;
                            break;
                        case PIN_INTERRUPT_LOW_LEVEL:
                            gpio_type = GPIO_INTR_LOW_LEVEL;
                            break;
                        case PIN_INTERRUPT_RISING_EDGE:
                            gpio_type = GPIO_INTR_RISING_EDGE;
                            break;
                        case PIN_INTERRUPT_FALLING_EDGE:
                            gpio_type = GPIO_INTR_FALLING_EDGE;
                            break;
                        case PIN_INTERRUPT_BOTH_EDGE:
                            gpio_type = GPIO_INTR_BOTH_EDGE;
                            break;
                        default:
                            gpio_type = GPIO_IN;
                            break;
                    }
                }
                gpio_config(&g_gpio_dev, config->pin_index, gpio_type);
                gpio_set(&g_gpio_dev, config->pin_index, config->initial_value);
            }
        }
    }
}

void board_port_init(void)
{
    /* Initialize pinctrl devices. */
    sdrv_pinctrl_init(&g_pinctrl_ap, &pinctrl_ap_dev);

    /* Initialize the GPIO controller. */
    sdrv_gpio_init(&g_gpio_dev, APB_GPIO3_BASE,
                   ARRAY_SIZE(gpio_mp_irqs), gpio_mp_irqs);

    pin_mux_init();
}

#endif