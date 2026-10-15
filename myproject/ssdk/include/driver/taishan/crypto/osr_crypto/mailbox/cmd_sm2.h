/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SM2_H
#define CMD_SM2_H

#include "cmd_trng.h"
#include "cmd_common.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t id_len[2];
    uint8_t id_ptr[4];
    uint8_t pubKey_ptr[4];
    uint8_t z[4];
} cmd_sm2_getZ_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t z[4];
    uint8_t e[4];
} cmd_sm2_getE_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t key_id[2];
    uint8_t pubKey_ptr[4];
    uint8_t priKey_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_sm2_generatekey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t priKey_ptr[4];
    uint8_t pubKey_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_sm2_get_pubkey_from_prikey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t rev1[2];
    uint8_t e_ptr[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t priKey_ptr[4];
    uint8_t signature_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_sm2_generatesignature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t rev1[2];
    uint8_t e_ptr[4];
    uint8_t pubKey_ptr[4];
    uint8_t signature_ptr[4];
} cmd_sm2_verifysignature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t rev1[2];
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t pubKey_ptr[4];
    uint8_t cipher_ptr[4];
} cmd_sm2_encrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t rev1[2];
    uint8_t cipher_ptr[4];
    uint8_t cipher_len[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t priKey_ptr[4];
    uint8_t data_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_sm2_decrypt_t;

typedef struct {
    uint8_t dA_ptr[4];
    uint8_t RA_ptr[4];
    uint8_t rA_ptr[4];
    uint8_t PB_ptr[4];
    uint8_t RB_ptr[4];
    uint8_t ZA_ptr[4];
    uint8_t ZB_ptr[4];
} sm2_exc_key_input_ptr_t;

typedef struct {
    cmd_key_type_e dA_key_type;
    uint16_t dA_key_id;
    cmd_key_type_e rA_key_type;
    uint16_t rA_key_id;
    uint16_t enc_key_id;
    cmd_key_type_e output_key_type;
    uint16_t output_key_id;
} cmd_sm2_exchangekey_keyinfo_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t role;
    uint8_t key_len;
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t key_id2[2];
    uint8_t key_id3[2];
    uint8_t input_ptr[4];
    uint8_t outputKey_ptr[4];
    uint8_t s1_s2_ptr[4];
    uint8_t sa_sb_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_sm2_exchangekey_t;

typedef enum {
    CMD_SM2_ROLE_SPONSOR = 0,
    CMD_SM2_ROLE_RESPONSOR,
    CMD_SM2_ROLE_MAX
} cmd_sm2_exchange_role_e;

uint32_t cmd_sm2_get_z(uint8_t *id, uint32_t id_len, uint8_t *pubkey, uint8_t *z_buf);

uint32_t cmd_sm2_get_e(uint8_t *msg, uint32_t msg_len, uint8_t *z_buf, uint8_t *e_buf);

uint32_t cmd_sm2_get_pubkey_from_prikey(uint8_t *prikey, uint8_t *pubkey);

uint32_t cmd_sm2_generatekey(cmd_key_type_e generate_key_type,  uint16_t generate_key_id, uint8_t *prikey, uint8_t *pubkey, uint8_t *mac_buf);

uint32_t cmd_sm2_sign(uint8_t *e, uint8_t *prikey, cmd_key_type_e generate_sign_key_type, uint16_t generate_sign_key_id, uint8_t *signature, uint8_t *mac_buf);

uint32_t cmd_sm2_verify(uint8_t *e, uint8_t *pubkey, uint8_t *signature);

uint32_t cmd_sm2_enc(uint8_t *msg, uint32_t msg_len, uint8_t *pubkey, uint8_t *cipher);

uint32_t cmd_sm2_dec(uint8_t *prikey,  uint8_t *msg, uint8_t *cipher, uint32_t cipher_len, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, uint8_t *mac_buf);

uint32_t cmd_sm2_set_exckey_input_ptr(uint8_t *dA, uint8_t *RA, uint8_t *rA, uint8_t *RB, uint8_t *PB, uint8_t *ZA, uint8_t *ZB, sm2_exc_key_input_ptr_t *cmd_sm2_exc_key_input);

/*mac_buf len is CMD_BASIC_TWO_KEY_MAC_BUFF_LEN 64*/
uint32_t cmd_sm2_exchange_key(cmd_sm2_exchange_role_e role, cmd_sm2_exchangekey_keyinfo_t *cmd_sm2_exckey_keyinfo,
                              sm2_exc_key_input_ptr_t *cmd_sm2_exc_key_input, uint32_t kbytelen,
                              uint8_t *S1, uint8_t *SA, uint8_t *KA, uint8_t *mac_buf);
#endif