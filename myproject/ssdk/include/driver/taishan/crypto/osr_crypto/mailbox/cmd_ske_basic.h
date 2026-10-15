/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SKE_BASIC_H
#define CMD_SKE_BASIC_H

#include "cmd_common.h"
#include "cmd_trng.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t rev1[1];
    uint8_t mode;
    uint8_t iv_ptr[4];
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t cipher_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_ske_encrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t rev1[1];
    uint8_t mode;
    uint8_t iv_ptr[4];
    uint8_t cipher_ptr[4];
    uint8_t cipher_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t data_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_ske_decrypt_t;

typedef struct {
    uint8_t iv_ptr[4];
    uint8_t aad_data_ptr[4];
} gcm_enc_input_ptr_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t mode;
    uint8_t tag_len;
    uint8_t gcm_enc_input_ptr[4];
    uint8_t iv_len[2];
    uint8_t aad_len[2];
    uint8_t data_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t cipher_tag_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_aead_encrypt_gcm_t;

typedef struct {
    uint8_t iv_ptr[4];
    uint8_t aad_cipher_ptr[4];
    uint8_t tag_ptr[4];
} gcm_dec_input_ptr_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_seed;
    uint8_t mode;
    uint8_t tag_len;
    uint8_t gcm_dec_input_ptr[4];
    uint8_t iv_len[2];
    uint8_t aad_len[2];
    uint8_t cipher_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_ptr[4];
    uint8_t data_tag_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_aead_decrypt_gcm_t;

uint32_t cmd_ske_basic_enc(uint8_t mode, uint8_t *plain, uint32_t bytelen, cmd_key_type_e key_type, uint8_t *key_buf,
                           uint16_t key_id, uint8_t *iv, uint8_t *cipher, uint8_t *mac_buf);

uint32_t cmd_ske_basic_dec(uint8_t mode, uint8_t *replain, uint32_t bytelen, cmd_key_type_e key_type, uint8_t *key_buf,
                           uint16_t key_id, uint8_t *iv, uint8_t *cipher, uint8_t *mac_buf);

#endif