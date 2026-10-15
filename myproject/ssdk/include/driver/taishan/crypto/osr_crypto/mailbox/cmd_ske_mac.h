/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SKE_MAC_H
#define CMD_SKE_MAC_H

#include "cmd_common.h"
#include "cmd_trng.h"
#include "cmd_hmac.h"
#include "cmd_ske_basic.h"

uint32_t cmd_ske_gen_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, uint8_t *key, uint16_t key_id,
                         uint8_t *mac, uint8_t mac_bytes, uint8_t *mac_buf);

uint32_t cmd_ske_verify_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, uint8_t *key, uint16_t key_id,
                            uint8_t *mac, uint8_t mac_bytes, uint8_t *mac_buf);
#endif