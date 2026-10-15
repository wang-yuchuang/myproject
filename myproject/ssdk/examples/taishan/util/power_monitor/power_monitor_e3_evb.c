/*
 * power_monitor_e3_evb.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level monitor monitor example.
 *
 * Revision History:
 * -----------------
 */

#include <board.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/adc/taishan_adc.h>
#include <driver/gpio.h>

#include "power_monitor.h"


extern pinctrl_t g_pinctrl_safety;
extern gpio_dev_t g_gpio_safety;

static void taishan_evb_prepare_amux(struct voltage_channel *vol)
{
    unsigned int sel = vol->mux_chnl;

    gpio_set(&g_gpio_safety, GPIO_C13, !!(sel & 1u));
    gpio_set(&g_gpio_safety, GPIO_C14, !!(sel & 2u));
    gpio_set(&g_gpio_safety, GPIO_C15, !!(sel & 4u));
}

struct voltage_channel board_voltage_channels[] = {
    /* U401 */
    {
        .name = "GPIOA0_BOOT0",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 0,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_SFT_0V8",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 1,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_AP_0V8",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 2,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_RTC_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 3,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_SFT_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 4,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_AP_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 5,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "VDD_DCDC_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 6,
        .prepare = taishan_evb_prepare_amux,
        .diff_n = true
    },
    {
        .name = "12V",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A0),
        .pin = GPIO_A0,
        .mux_chnl = 7,
        .prepare = taishan_evb_prepare_amux,
        .ratio_div = 20,
        .ratio_mul = 20 + 100,
        .diff_n = true
    },

    /* U402 */
    {
        .name = "GPIOA1_BOOT1",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 0,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_SFT_0V8",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 1,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_AP_0V8",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 2,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_RTC_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 3,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_SFT_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 4,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_AP_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 5,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_DCDC_3V3",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 6,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "12V",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A1),
        .pin = GPIO_A1,
        .mux_chnl = 7,
        .prepare = taishan_evb_prepare_amux,
        .ratio_div = 20,
        .ratio_mul = 20 + 100
    },

    /* U403 */
    {
        .name = "GPIOA2_BOOT2",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 0,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "AVDD_ADC_3V3",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 1,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "AVDD_LDO_3V3_IN",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 2,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "AVDD_USB_3V3",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 3,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "AVDD_SFT_3V3",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 4,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "AVDD_SFT_1V8",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 5,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_AP_3V3",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 6,
        .prepare = taishan_evb_prepare_amux
    },
    {
        .name = "VDD_AP_1V8",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A2),
        .pin = GPIO_A2,
        .mux_chnl = 7,
        .prepare = taishan_evb_prepare_amux,
    },
    {
        .name = NULL
    }
};
