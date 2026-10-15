/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_SM9_H
#define CMD_SM9_H

#include "cmd_common.h"
#include "cmd_trng.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t mastPubKey_ptr[4];
    uint8_t mastPriKey_ptr[4];
} cmd_sm9_sign_generate_master_key_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t hid;
    uint8_t id_len[2];
    uint8_t id_ptr[4];
    uint8_t mastPriKey_ptr[4];
    uint8_t userPriKey_ptr[4];
} cmd_sm9_sign_generate_user_prikey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t mastPubKey_ptr[4];
    uint8_t userPriKey_ptr[4];
    uint8_t signature_ptr[4];
} cmd_sm9_generatesignature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t hid;
    uint8_t id_len[2];
    uint8_t id_ptr[4];
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t mastPubKey_ptr[4];
    uint8_t signature_ptr[4];
} cmd_sm9_verifysignature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t mastPubKey_ptr[4];
    uint8_t mastPriKey_ptr[4];
} cmd_sm9_enc_generate_master_key_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t hid;
    uint8_t id_len[2];
    uint8_t id_ptr[4];
    uint8_t mastPriKey_ptr[4];
    uint8_t userPriKey_ptr[4];
} cmd_sm9_enc_generate_user_prikey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t hid;
    uint8_t key_len;
    uint8_t id_ptr[4];
    uint8_t id_len[2];
    uint8_t rev2[2];
    uint8_t mastPubKey_ptr[4];
    uint8_t key_ptr[4];
    uint8_t cipher_ptr[4];
} cmd_sm9_wrapkey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t key_len;
    uint8_t id_len[2];
    uint8_t id_ptr[4];
    uint8_t userPriKey_ptr[4];
    uint8_t cipher_ptr[4];
    uint8_t key_ptr[4];
} cmd_sm9_unwrapkey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t enc_type;
    uint8_t padding_type;
    uint8_t key2_len;
    uint8_t data_ptr[4];
    uint8_t data_len[4];
    uint8_t id_ptr[4];
    uint8_t id_len[2];
    uint8_t rev1[1];
    uint8_t hid;
    uint8_t mastPubKey_ptr[4];
    uint8_t cipher_ptr[4];
    uint8_t cipherLen_ptr[4];
} cmd_sm9_encrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t enc_type;
    uint8_t padding_type;
    uint8_t key2_len;
    uint8_t cipher_ptr[4];
    uint8_t cipher_len[4];
    uint8_t id_ptr[4];
    uint8_t rev1[2];
    uint8_t id_len[2];
    uint8_t userPriKey_ptr[4];
    uint8_t data_ptr[4];
    uint8_t dataLen_ptr[4];
} cmd_sm9_decrypt_t;

typedef cmd_sm9_enc_generate_master_key_t cmd_sm9_exckey_generate_master_key_t;

typedef cmd_sm9_enc_generate_user_prikey_t cmd_sm9_exckey_generate_user_prikey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t hid;
    uint8_t peer_id_len[2];
    uint8_t peer_id_ptr[4];
    uint8_t mastPubKey_ptr[4];
    uint8_t userTmpPriKey_ptr[4];
    uint8_t userTmpPubKey_ptr[4];
} cmd_sm9_exckey_generate_user_tmpkey_t;

typedef struct {
    uint8_t Ppub_e_ptr[4];
    uint8_t deA_ptr[4];
    uint8_t RA_ptr[4];
    uint8_t rA_ptr[4];
    uint8_t RB_ptr[4];
    uint8_t IDA_ptr[4];
    uint8_t IDB_ptr[4];
} cmd_sm9_exc_key_input_ptr_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t key_len;
    uint8_t role;
    uint8_t input_ptr[4];
    uint8_t ida_len[2];
    uint8_t idb_len[2];
    uint8_t outputKey_ptr[4];
    uint8_t s1_s2_ptr[4];
    uint8_t sa_sb_ptr[4];
} cmd_sm9_exchangekey_t;

//SM9 encryption type choice
typedef enum {
    CMD_SM9_ENC_KDF_STREAM_CIPHER = 0,
    CMD_SM9_ENC_KDF_BLOCK_CIPHER,
    CMD_SM9_ENC_KDF_MAX
} cmd_sm9_enc_type_e;

//available when choose SM9_ENC_KDF_BLOCK_CIPHER
typedef enum {
    CMD_SM9_ENC_NO_PADDING = 0,
    CMD_SM9_ENC_PKCS7_PADDING,
    CMD_SM9_ENC_MAX_PADDING
} cmd_sm9_enc_padding_e;

//SM9 key exchange role
typedef enum {
    CMD_SM9_ROLE_SPONSOR = 0,
    CMD_SM9_ROLE_RESPONSOR,
    CMD_SM9_ROLE_MAX
} cmd_sm9_exchangekey_role_e;

uint8_t cmd_sm9_sign_gen_master_key(uint8_t *ks, uint8_t *pub_s);

uint8_t cmd_sm9_sign_gen_user_prikey(uint8_t *ida, uint32_t ida_len, uint8_t *ks, uint8_t *dsa);

uint8_t cmd_sm9_gen_signature(uint8_t *msg, uint32_t msg_len, uint8_t *pub_s, uint8_t *dsa, uint8_t *signature);

uint8_t cmd_sm9_verify_signature(uint8_t *msg, uint32_t msg_len, uint8_t *ida, uint32_t ida_len, uint8_t *pub_s, uint8_t *signature);

uint8_t cmd_sm9_enc_gen_master_key(uint8_t *ke, uint8_t *pub_e);

uint8_t cmd_sm9_enc_gen_user_prikey(uint8_t *idb, uint32_t idb_len, uint8_t *ke, uint8_t *deb);

uint8_t cmd_sm9_enc_wrap_key(uint8_t *idb, uint32_t idb_len, uint8_t *pub_e, uint8_t *K, uint32_t kbytelen, uint8_t *C);

uint8_t cmd_sm9_enc_unwrap_key(uint8_t *idb, uint32_t idb_len, uint8_t *deb, uint8_t *K_unwrap, uint32_t kbytelen, uint8_t *C);

uint8_t cmd_sm9_enc_kdf_stream_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *pub_e,
                                      uint8_t *msg, uint32_t msg_len, uint8_t *cipher, uint32_t *cipher_len);

uint8_t cmd_sm9_dec_kdf_stream_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *deb,
                                      uint8_t *msg, uint32_t *msg_len, uint8_t *cipher, uint32_t cipher_len);

uint8_t cmd_sm9_enc_kdf_block_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *pub_e,
                                     uint8_t *msg, uint32_t msg_len, uint8_t *cipher, uint32_t *cipher_len, uint8_t padding_type);

uint8_t cmd_sm9_dec_kdf_block_cipher(uint8_t *id, uint32_t id_len, uint32_t key2_len, uint8_t *deb,
                                     uint8_t *msg, uint32_t *msg_len, uint8_t *cipher, uint32_t cipher_len, uint8_t padding_type);

uint8_t cmd_sm9_exckey_gen_master_key(uint8_t *ke, uint8_t *pub_e);

uint8_t cmd_sm9_exckey_gen_user_prikey(uint8_t *id, uint32_t id_len, uint8_t *ke, uint8_t *de);

uint8_t cmd_sm9_exckey_gen_user_tmpkey(uint8_t *id, uint32_t id_len, uint8_t *pub_e, uint8_t *r, uint8_t *R);

uint32_t cmd_sm9_set_exckey_input_ptr(uint8_t *pub_e, uint8_t *dA, uint8_t *RA, uint8_t *rA,
                                      uint8_t *RB, uint8_t *ida, uint8_t *idb, cmd_sm9_exc_key_input_ptr_t *exckey_input);

uint8_t cmd_sm9_exchange_key(cmd_sm9_exchangekey_role_e role, uint32_t ida_len, uint32_t idb_len,
                             uint32_t kByteLen, cmd_sm9_exc_key_input_ptr_t *exckey_input,
                             uint8_t *KA, uint8_t *S1, uint8_t *SA);
#endif