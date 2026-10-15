/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SM2_H
#define SM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>
#include "sm2_basic.h"

/*only available for PKE_HP*/
#define SM2_HIGH_SPEED

/*some sm2 length*/
#define SM2_BIT_LEN               (256)
#define SM2_BYTE_LEN              (32)
#define SM2_WORD_LEN              (8)
#define SM3_DIGEST_BYTE_LEN       SM2_BYTE_LEN
#define SM2_MAX_ID_BYTE_LEN       (1<<13)

uint32_t sm2_sign(uint8_t E[32], uint8_t rand_k[32], uint8_t priKey[32], uint8_t signature[64]);

uint32_t sm2_verify(uint8_t E[32], uint8_t pubKey[65], uint8_t signature[64]);

uint32_t sm2_encrypt(uint8_t *M, uint32_t MByteLen, uint8_t rand_k[32], uint8_t pubKey[65],
                     sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen);

uint32_t sm2_decrypt(uint8_t *C, uint32_t CByteLen, uint8_t priKey[32],
                     sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen);

uint32_t sm2_exchangekey(sm2_exchange_role_e role,
                         uint8_t *dA, uint8_t *PB,
                         uint8_t *rA, uint8_t *RA,
                         uint8_t *RB,
                         uint8_t *ZA, uint8_t *ZB,
                         uint32_t kByteLen,
                         uint8_t *KA, uint8_t *S1, uint8_t *SA);

#ifdef PKE_SEC

#define SM2_SEC
#ifdef SM2_SEC

enum SM2_RET_CODE_S {
    SM2_SUCCESS_S = 0x3E2FDB1A,
    SM2_ERROR_S = 0xCBAD735E
};

uint32_t sm2_sign_s(uint8_t E[32], uint8_t rand_k[32], uint8_t priKey[32], uint8_t signature[64]);

uint32_t sm2_verify_s(uint8_t E[32], uint8_t pubKey[65], uint8_t signature[64]);

uint32_t sm2_encrypt_s(uint8_t *M, uint32_t MByteLen, uint8_t rand_k[32], uint8_t pubKey[65],
                       sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen);

uint32_t sm2_decrypt_s(uint8_t *C, uint32_t CByteLen, uint8_t priKey[32],
                       sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen);

uint32_t sm2_exchangekey_s(sm2_exchange_role_e role,
                           uint8_t *dA, uint8_t *PB,
                           uint8_t *rA, uint8_t *RA,
                           uint8_t *RB,
                           uint8_t *ZA, uint8_t *ZB,
                           uint32_t kByteLen,
                           uint8_t *KA, uint8_t *S1, uint8_t *SA);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
