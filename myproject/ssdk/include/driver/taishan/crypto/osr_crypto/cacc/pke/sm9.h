/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SM9_H
#define SM9_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/trng.h>
#include <ral/pke.h>

#define SM9_MAX_ID_BYTE_LEN           (1<<13)
#define SM9_MAX_MSG_BYTE_LEN          (0xFFFFFF9F)
#define SM9_MAX_ENC_K2_BYTE_LEN       (0x40)
#define SM9_POINT_NOT_COMPRESSED      (0x04)

/*SM9 error code*/
enum SM9_RET_CODE {
    SM9_SUCCESS = 0,
    SM9_BUFFER_NULL = 0x50,
    SM9_NOT_ON_CURVE,
    SM9_EXCHANGE_ROLE_INVALID,
    SM9_INPUT_INVALID,
    SM9_ZERO_ALL,
    SM9_INTEGER_TOO_BIG,
    SM9_VERIFY_FAILED,
    SM9_IN_OUT_SAME_BUFFER,
    SM9_DECRY_VERIFY_FAILED
};

/*SM9 encryption type choice*/
typedef enum {
    SM9_ENC_KDF_STREAM_CIPHER = 0,
    SM9_ENC_KDF_BLOCK_CIPHER
} sm9_enc_type_e;

/*available when choose SM9_ENC_KDF_BLOCK_CIPHER*/
typedef enum {
    SM9_ENC_NO_PADDING = 0,
    SM9_ENC_PKCS7_PADDING,
    SM9_ENC_MAX_PADDING
} sm9_enc_padding_e;

/*SM9 key exchange role*/
typedef enum {
    SM9_ROLE_SPONSOR = 0,
    SM9_ROLE_RESPONSOR
} sm9_exchange_role_e;

uint32_t sm9_sign_gen_mastPubKey_from_mastPriKey(uint8_t ks[32], uint8_t Ppub_s[128]);

uint32_t sm9_sign_gen_mastKeyPair(uint8_t ks[32], uint8_t Ppub_s[128]);

uint32_t sm9_sign_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid, uint8_t ks[32], uint8_t dsA[64]);

uint32_t sm9_sign(const uint8_t *M, uint32_t MByteLen, const uint8_t Ppub_s[128], const uint8_t dsA[64], uint8_t r[32],
                  uint8_t h[32], uint8_t S[65]);

uint32_t sm9_verify(const uint8_t *M, uint32_t MByteLen, const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid,
                    const uint8_t Ppub_s[128], const uint8_t h[32], const uint8_t S[65]);

uint32_t sm9_enc_gen_mastPubKey_from_mastPriKey(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_enc_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_enc_gen_userPriKey(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, uint8_t ke[32], uint8_t deB[128]);

uint32_t sm9_wrap_key(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64], uint8_t r[32],
                      uint8_t C[64], uint32_t KByteLen, uint8_t *K);

uint32_t sm9_unwrap_key(const uint8_t *IDB, uint32_t IDB_byteLen, const uint8_t deB[128], const uint8_t C[64],
                        uint32_t KByteLen, uint8_t *K);

uint32_t sm9_enc(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t *M, uint32_t MByteLen,
                 const uint8_t Ppub_e[64], uint8_t r[32], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type,
                 uint32_t K2ByteLen, uint8_t *C, uint32_t *CByteLen);

uint32_t sm9_dec(const uint8_t *IDB, uint32_t IDB_byteLen, const uint8_t *C, uint32_t CByteLen, const uint8_t deB[128],
                 sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, uint32_t K2ByteLen, uint8_t *M, uint32_t *MByteLen);


uint32_t sm9_exckey_gen_mastPubKey_from_mastPriKey(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_exckey_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_exckey_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid, uint8_t ke[32], uint8_t deA[128]);

uint32_t sm9_exckey_gen_tmpPubKey_from_tmpPriKey(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64],
        uint8_t rA[32], uint8_t RA[64]);

uint32_t sm9_exckey_gen_tmpKeyPair(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64],
                                   uint8_t rA[32], uint8_t RA[64]);

uint32_t sm9_exchangekey(sm9_exchange_role_e role,
                         const uint8_t *IDA, uint32_t IDA_byteLen,
                         const uint8_t *IDB, uint32_t IDB_byteLen,
                         const uint8_t Ppub_e[64],
                         const uint8_t deA[128], const uint8_t rA[32],
                         const uint8_t RA[64], const uint8_t RB[64],
                         uint32_t kByteLen,
                         uint8_t *KA, uint8_t S1[32], uint8_t SA[32]);

#ifdef __cplusplus
}
#endif

#endif
