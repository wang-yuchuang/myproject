/**
 * @file kunlun_adc.h
 * @brief kunlun_adc api header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef KUNLUN_ADC_H_
#define KUNLUN_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_MAX_CHANNEL_NUM (1<<7)

#include <driver/adc.h>


#define ADC_CH_SEL_KUNLUN(ch, mux)  (((mux & 0xFu) << 3) | (ch & 0x7u))


int kunlun_adc_probe(adc_dev_t *device, const adc_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_H */

