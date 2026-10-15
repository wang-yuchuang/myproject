/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SM2_BASIC_H
#define SM2_BASIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>

/*only available for PKE_HP*/
#define SM2_HIGH_SPEED

/*some sm2 length*/
#define SM2_BIT_LEN               (256)
#define SM2_BYTE_LEN              (32)
#define SM2_WORD_LEN              (8)
#define SM3_DIGEST_BYTE_LEN       SM2_BYTE_LEN
#define SM2_MAX_ID_BYTE_LEN       (1<<13)

/*SM2 error code*/
enum SM2_RET_CODE {
    SM2_SUCCESS = 0,
    SM2_BUFFER_NULL = PKE_SUCCESS + 0x40,
    SM2_NOT_ON_CURVE,
    SM2_EXCHANGE_ROLE_INVALID,
    SM2_INPUT_INVALID,
    SM2_ZERO_ALL,
    SM2_INTEGER_TOO_BIG,
    SM2_VERIFY_FAILED,
    SM2_DECRYPT_VERIFY_FAILED
};

/*SM2 key exchange role*/
typedef enum {
    SM2_ROLE_SPONSOR = 0,
    SM2_ROLE_RESPONSOR,
    SM2_ROLE_MAX
} sm2_exchange_role_e;

/* SM2 ciphertext order*/
typedef enum {
    SM2_C1C3C2   = 0,
    SM2_C1C2C3,
} sm2_cipher_order_e;

extern const uint32_t sm2p256v1_Gx[8];
extern const uint32_t sm2p256v1_Gy[8];
extern const uint32_t sm2p256v1_n[8];
extern const uint32_t sm2p256v1_n_h[8];
extern const uint32_t sm2p256v1_n_1[8];

extern const eccp_curve_t sm2_curve[1];

uint32_t sm2_getZ(uint8_t *ID, uint32_t byteLenofID, uint8_t pubKey[65], uint8_t Z[32]);

uint32_t sm2_getE(uint8_t *M, uint32_t byteLen, uint8_t Z[32], uint8_t E[32]);

uint32_t sm2_get_pubkey_from_prikey(uint8_t priKey[32], uint8_t pubKey[65]);

uint32_t sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65]);

uint32_t sm2_kdf_with_xor(uint8_t *in, uint32_t inByteLen, uint8_t *m, uint8_t *k, uint32_t kByteLen);

void uint32_BigNum_Add_One(uint32_t *a, uint32_t wordLen);

#ifdef __cplusplus
}
#endif

#endif
