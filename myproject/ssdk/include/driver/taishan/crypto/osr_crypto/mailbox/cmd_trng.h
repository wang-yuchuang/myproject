/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_TRNG_H
#define CMD_TRNG_H

#include "cmd_common.h"
#include "soc_otp_key.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t random_ptr[4];
    uint8_t random_len[4];
} cmd_tng_t;

uint32_t cmd_trng_get_rand(uint8_t *rand_output, uint32_t rand_bytes);

#endif

