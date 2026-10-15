/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_HASH_H
#define CMD_HASH_H

#include "cmd_common.h"
#include "cmd_trng.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[2];
    uint8_t mode;
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t digest_ptr[4];
} cmd_hash_t;


uint32_t cmd_hash_calc(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#endif