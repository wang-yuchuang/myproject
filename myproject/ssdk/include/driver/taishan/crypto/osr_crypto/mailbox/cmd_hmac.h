/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_HMAC_H
#define CMD_HMAC_H

#include "cmd_common.h"
#include "cmd_trng.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t mode;
    uint8_t mac_len;
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t rev2[2];
    uint8_t key_len[2];
    uint8_t mac_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_generateMAC_t;


typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t mode;
    uint8_t mac_len;
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t rev2[2];
    uint8_t key_len[2];
    uint8_t mac_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_verifyMAC_t;


uint32_t cmd_hmac_generate_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, const uint8_t *key, uint16_t key_id,
                               uint32_t key_bytes, uint8_t mac_bytes, uint8_t *out, uint8_t *mac_buf);

uint32_t cmd_hmac_verify_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, const uint8_t *key, uint16_t key_id,
                             uint32_t key_bytes, uint8_t hmac_bytes, uint8_t *hmac, uint8_t *mac_buf);
#endif