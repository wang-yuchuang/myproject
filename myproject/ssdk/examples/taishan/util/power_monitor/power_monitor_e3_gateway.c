/*
 * power_monitor_e3_gateway.c
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
        .name = "VDD3V3_SAF",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A13),
        .pin = GPIO_A13
    },
    {
        .name = "VDD3V3_SAF",
        .adc = 1,
        .adc_chnl = ADC_DCH_SEL_TAISHAN(5, 5, ADC1_CH5N_MUX_A12),
        .pin = GPIO_A13,
        .diff_n = true,
        .pin_neg = GPIO_A12
    },
    {
        .name = "VDD1V8_SAF",
        .adc = 1,
        .adc_chnl = ADC_CH_SEL_TAISHAN(5, ADC_CH_POLAR_POS, ADC1_CH5P_MUX_A15),
        .pin = GPIO_A15,
    },
    {
        .name = "VDD1V8_SAF",
        .adc = 1,
        .adc_chnl = ADC_DCH_SEL_TAISHAN(5, 5, ADC1_CH5N_MUX_A14),
        .pin = GPIO_A15,
        .diff_n = true,
        .pin_neg = GPIO_A14
    },
};
