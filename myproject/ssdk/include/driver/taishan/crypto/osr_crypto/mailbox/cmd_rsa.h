/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_RSA_H
#define CMD_RSA_H

#include "cmd_common.h"
#include "cmd_trng.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t prime_len[2];
    uint8_t prime_ptr[4];
} cmd_rsa_getprime_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t e_len[2];
    uint8_t n_len[2];
    uint8_t e_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t d_ptr[4];
    uint8_t n_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_generatekey_t;

typedef struct {
    uint8_t p_ptr[4];
    uint8_t q_ptr[4];
    uint8_t dp_ptr[4];
    uint8_t dq_ptr[4];
    uint8_t u_ptr[4];
} rsa_crt_pri_key_ptr_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t e_len[2];
    uint8_t n_len[2];
    uint8_t e_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t priKey_ptr[4];
    uint8_t n_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_generateCRTkey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t n_len[2];
    uint8_t data_ptr[4];
    uint8_t e_ptr[4];
    uint8_t n_ptr[4];
    uint8_t cipher_ptr[4];
} cmd_rsa_encrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t n_len[2];
    uint8_t cipher_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t e_ptr[4];
    uint8_t d_ptr[4];
    uint8_t n_ptr[4];
    uint8_t data_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_decrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t n_len[2];
    uint8_t cipher_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t e_ptr[4];
    uint8_t priKey_ptr[4];
    uint8_t data_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_CRT_decrypt_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t n_len[2];
    uint8_t data_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t e_ptr[4];
    uint8_t d_ptr[4];
    uint8_t n_ptr[4];
    uint8_t signature_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_generate_signature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t need_high_sec_ver;
    uint8_t n_len[2];
    uint8_t data_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id[2];
    uint8_t e_ptr[4];
    uint8_t priKey_ptr[4];
    uint8_t signature_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_rsa_CRT_generate_signature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t n_len[2];
    uint8_t data_ptr[4];
    uint8_t e_ptr[4];
    uint8_t n_ptr[4];
    uint8_t signature_ptr[4];
} cmd_rsa_verify_t;

uint32_t cmd_rsa_get_prime(uint16_t primebitlen, uint8_t *prime);

uint32_t cmd_rsa_generate_key(uint16_t ebitlen, uint16_t nbitlen, cmd_key_type_e generate_key_type, uint16_t generate_key_id,
                              uint8_t *n, uint8_t *e, uint8_t *d, uint8_t *mac_buf);

uint32_t cmd_rsa_encrypt(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out);

uint32_t cmd_rsa_decrypt(uint8_t *msg, uint8_t *d, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, uint8_t *mac_buf);

uint32_t cmd_rsa_generate_signature(uint8_t *msg, uint8_t *d, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e sign_key_type, uint16_t sign_key_id, uint8_t *mac_buf);

uint32_t cmd_rsa_verify_signature(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *sign);

uint32_t cmd_rsa_set_crt_prikey_ptr(uint8_t *p, uint8_t *q, uint8_t *dp, uint8_t *dq, uint8_t *u, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key);

uint32_t cmd_rsa_generate_crt_key(uint16_t ebitlen, uint16_t nbitlen, cmd_key_type_e generate_key_type, uint16_t generate_key_id,
                                  uint8_t *n, uint8_t *e, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf);

uint32_t cmd_rsa_crt_decrypt(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen,
                             uint8_t *out, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf);

uint32_t cmd_rsa_crt_generate_signature(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen,
                                        uint8_t *out, cmd_key_type_e sign_key_type, uint16_t sign_key_id, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf);

#endif