/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SKE_XTS_H
#define CMD_SKE_XTS_H

#include "cmd_common.h"
#include "cmd_trng.h"
#include "cmd_ske_basic.h"

uint32_t cmd_ske_xts_enc(uint8_t mode, uint8_t *plain, uint32_t bytelen, cmd_key_type_e key_type, uint8_t *key, uint16_t key_id,
                         uint8_t *iv, uint8_t *cipher, uint8_t *mac_buf);

uint32_t cmd_ske_xts_dec(uint8_t mode, uint8_t *replain, cmd_key_type_e key_type, uint8_t *key, uint16_t key_id,
                         uint8_t *iv, uint8_t *cipher, uint32_t cipherbytelen, uint8_t *mac_buf);
#endif