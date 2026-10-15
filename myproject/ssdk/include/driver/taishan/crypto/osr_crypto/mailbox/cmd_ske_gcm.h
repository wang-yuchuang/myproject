/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SKE_GCM_H
#define CMD_SKE_GCM_H

#include "cmd_common.h"
#include "cmd_trng.h"
#include "cmd_ske_basic.h"

uint32_t cmd_ske_gcm_get_msg(uint8_t *plain, uint32_t plainlen, uint8_t *aad, uint8_t aadbytelen, uint8_t *msg, uint32_t *msgbytelen);

uint32_t cmd_ske_gcm_enc(uint8_t mode, uint8_t *msg, uint32_t msgbytelen, cmd_key_type_e key_type, uint8_t *key,
                         uint16_t key_id, uint8_t *iv, uint8_t ivbytelen, uint8_t aadbytelen,
                         uint8_t *cipher, uint32_t tagbytelen, uint8_t *mac_buf, gcm_enc_input_ptr_t *cmd_gcm_enc_input);

uint32_t cmd_ske_gcm_dec(uint8_t mode, uint8_t *cipher, uint32_t cipherbytelen, cmd_key_type_e key_type, uint8_t *key,
                         uint16_t key_id, uint8_t *iv, uint8_t ivbytelen, uint8_t aadbytelen,
                         uint8_t *msg, uint8_t *tag, uint32_t tagbytelen, uint8_t *mac_buf, gcm_dec_input_ptr_t *cmd_gcm_dec_input);
#endif