/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef TRNG_H
#define TRNG_H

#include "register_base_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TRNG register address */
#define TRNG_CR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0000))
#define TRNG_MSEL           (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0004))
#define TRNG_SR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0008))
#define TRNG_DR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x000C))
#define TRNG_VERSION        (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0010))
#define TRNG_RESEED         (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0040))
#define TRNG_HT_CR          (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0060))
#define TRNG_HT_SR          (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0070))
#define RO_SRC_EN1          (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0080))
#define RO_SRC_EN2          (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0084))
#define SCLK_FREQ           (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x0088))

#define TERO_CR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00B0))
#define TERO_THOLD          (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00B4))
#define TERO_CNT(i)         (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00C0 + 4*i))
#define TERO_SR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00D0))
#define TERO_DR             (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00D4))
#define TERO_RCR(i)         (*(volatile uint32_t *)(TRNG_BASE_ADDR + 0x00E0 + 4*i))

/* TRNG freq config */
#define TRNG_RO_FREQ_4      (0)
#define TRNG_RO_FREQ_8      (1)
#define TRNG_RO_FREQ_16     (2)
#define TRNG_RO_FREQ_32     (3)

/* TRNG action offset */
#define TRNG_GLOBAL_INT_OFFSET          (24)
#define TRNG_READ_EMPTY_INT_OFFSET      (17)
#define TRNG_DATA_INT_OFFSET            (16)
#define TRNG_RO_ENTROPY_OFFSET          (4)
#define TRNG_TERO_THRESHOLD_OFFSET      (24)
#define TRNG_TERO_ENTROPY_OFFSET        (8)

/* TRNG return code */
enum TRNG_RET_CODE {
    TRNG_SUCCESS = 0,
    TRNG_BUFFER_NULL,
    TRNG_INVALID_INPUT,
    TRNG_INVALID_CONFIG,
    TRNG_HT_ERROR,
    TRNG_ERROR
};

uint32_t trng_get_version(void);

void trng_global_int_enable(void);

void trng_global_int_disable(void);

void trng_empty_read_int_enable(void);

void trng_empty_read_int_disable(void);

void trng_data_int_enable(void);

void trng_data_int_disable(void);

void trng_enable(void);

void trng_disable(void);

uint32_t trng_ro_entropy_config(uint8_t cfg);

uint32_t trng_ro_sub_entropy_config(uint8_t sn, uint16_t cfg);

void trng_set_mode(uint8_t with_post_processing);

void trng_reseed(void);

uint32_t trng_set_freq(uint8_t freq);

uint32_t get_rand_internal(uint8_t *a, uint32_t bytes);

uint32_t get_rand_fast(uint8_t *rand, uint32_t bytes);

uint32_t get_rand(uint8_t *rand, uint32_t bytes);

void tero_enable(void);

void tero_disable(void);

uint32_t tero_set_stop_threshold(uint8_t threshold_value);

uint32_t tero_entropy_config(uint8_t cfg);

void tero_set_output_rng(void);

void tero_set_output_osc_times(void);

void tero_set_osc_sel(uint8_t cfg);

void tero_set_osc_times_lower_limit(uint16_t value);

void tero_set_osc_times_upper_limit(uint16_t value);

uint32_t get_tero_rand(uint8_t *a, uint32_t bytes);

#ifdef __cplusplus
}
#endif


#endif

