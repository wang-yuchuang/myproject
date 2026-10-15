/*
 * power_monitor_d3_disp.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level monitor monitor example.
 *
 * Revision History:
 * -----------------
 */

#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/adc/taishan_adc.h>

#include "power_monitor.h"


struct voltage_channel board_voltage_channels[] = {
    {
        .name = "SYS_5V",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_POS, ADC2_CH4P_MUX_A15),
        .pin = GPIO_A15,
        .ratio_mul = 30 + 20,
        .ratio_div = 30
    },
    {
        .name = "SYS_5V",
        .adc = 2,
        .adc_chnl = ADC_CH_SEL_TAISHAN(4, ADC_CH_POLAR_NEG, ADC2_CH4N_MUX_A14),
        .pin = GPIO_A14,
        .diff_n = true,
    },
    {
        .name = "VDD_RTC",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A9),
        .pin = GPIO_A9,
    },
    {
        .name = "VDD_RTC",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_NEG, ADC1_CH5N_MUX_A8),
        .pin = GPIO_A8,
        .diff_n = true,
    },
};
