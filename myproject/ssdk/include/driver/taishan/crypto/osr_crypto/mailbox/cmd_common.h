/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_COMMON_H
#define CMD_COMMON_H

#include <stdint.h>
#include <cmsis_os2.h>

#include <ral/utility.h>

#include <arch/cache.h>

#include "soc_otp_key.h"

#define SEIP_SUCCESS   (0xA5)
#define SEIP_ERROR     (0x37)
#define SEIP_SKE_ALARM     (0x38)

/*this mem share between ehsm and r5, defined by customer, and must be STRONGORDERED type*/
#define CMD_TEST_DMA_RAM_BASE         (0x600000UL)

#define ROM_VENDER_SM2_PUBKEY_ADDR    (0x000)
#define ROM_FIRMWARE_ADDR             (0x000)
/*if key is from secure port, the max key index(or the number of keys)*/
#define SKE_MAX_KEY_IDX                                (9)

#define KEYID_0                                 (0)
#define KEYID_1                                 (1)
#define KEYID_2                                 (2)
#define KEYID_3                                 (3)
#define KEYID_4                                 (4)
#define KEYID_5                                 (5)
#define KEYID_6                                 (6)
#define KEYID_7                                 (7)
#define KEYID_8                                 (8)
#define KEYID_9                                 (9)
#define KEYID_MAX                               SKE_MAX_KEY_IDX
#define KEYID_HALF_MASK                         (0x8000)

#define BASIC_MGMT_CMD_BASE                     (0x00)
#define GET_DEVICE_INFO                         (BASIC_MGMT_CMD_BASE+1)
#define GET_DEVICE_STATUS                       (BASIC_MGMT_CMD_BASE+2)
#define GENERATE_RAND                           (BASIC_MGMT_CMD_BASE+3)
#define GENERATE_KEY                            (BASIC_MGMT_CMD_BASE+4)
#define SELF_DESTROY                            (BASIC_MGMT_CMD_BASE+5)
#define SEIP_JUMP                               (BASIC_MGMT_CMD_BASE+6)
#define SEIP_DISABLE                            (BASIC_MGMT_CMD_BASE+7)
#define SENSOR_CLK_CFG                          (BASIC_MGMT_CMD_BASE+8)

#define MAC_CMD_BASE                            (0x10)
#define GENERATE_MAC                            (MAC_CMD_BASE+1)
#define VERIFY_MAC                              (MAC_CMD_BASE+2)
#define GENERATE_AUTH_MAC                       (MAC_CMD_BASE+3)

#define SKE_CMD_BASE                            (0x20)
#define SKE_ENCRYPT                             (SKE_CMD_BASE+1)
#define SKE_DECRYPT                             (SKE_CMD_BASE+2)
#define SKE_AEAD_ENCRYPT_GCM                    (SKE_CMD_BASE+3)
#define SKE_AEAD_DECRYPT_GCM                    (SKE_CMD_BASE+4)

#define SM2_CMD_BASE                            (0x40)
#define SM2_GET_Z                               (SM2_CMD_BASE+1)
#define SM2_GET_E                               (SM2_CMD_BASE+2)
#define SM2_GENERATE_KEY                        (SM2_CMD_BASE+3)
#define SM2_GENERATE_SIGNATURE                  (SM2_CMD_BASE+4)
#define SM2_VERIFY_SIGNATURE                    (SM2_CMD_BASE+5)
#define SM2_ENCRYPT                             (SM2_CMD_BASE+6)
#define SM2_DECRYPT                             (SM2_CMD_BASE+7)
#define SM2_EXCHANGE_KEY                        (SM2_CMD_BASE+8)
#define SM2_GET_PUB_KEY                         (SM2_CMD_BASE+9)

#define SM9_CMD_BASE                            (0x50)
#define SM9_SIGN_GENERATE_MASTER_KEY            (SM9_CMD_BASE+1)
#define SM9_SIGN_GENERATE_USER_PRIVATE_KEY      (SM9_CMD_BASE+2)
#define SM9_GENERATE_SIGNATURE                  (SM9_CMD_BASE+3)
#define SM9_VERIFY_SIGNATURE                    (SM9_CMD_BASE+4)
#define SM9_ENC_GENERATE_MASTER_KEY             (SM9_CMD_BASE+5)
#define SM9_ENC_GENERATE_USER_PRIVATE_KEY       (SM9_CMD_BASE+6)
#define SM9_WRAP_KEY                            (SM9_CMD_BASE+7)
#define SM9_UNWRAP_KEY                          (SM9_CMD_BASE+8)
#define SM9_ENCRYPT                             (SM9_CMD_BASE+9)
#define SM9_DECRYPT                             (SM9_CMD_BASE+10)
#define SM9_EXCKEY_GENERATE_MASTER_KEY          (SM9_CMD_BASE+11)
#define SM9_EXCKEY_GENERATE_USER_PRIVATE_KEY    (SM9_CMD_BASE+12)
#define SM9_EXCKEY_GENERATE_USER_TMP_KEY        (SM9_CMD_BASE+13)
#define SM9_EXCHANGE_KEY                        (SM9_CMD_BASE+14)

#define ECCP_CMD_BASE                           (0x70)
#define ECCP_POINT_DOUBLING                     (ECCP_CMD_BASE+1)
#define ECCP_POINT_ADDITION                     (ECCP_CMD_BASE+2)
#define ECCP_POINT_MULTIPLICATION               (ECCP_CMD_BASE+3)
#define ECCP_POINT_VERIFY                       (ECCP_CMD_BASE+4)
#define ECCP_GENERATE_KEY                       (ECCP_CMD_BASE+5)
#define ECDH_EXCHANGE_KEY                       (ECCP_CMD_BASE+6)
#define ECDSA_GENERATE_SIGNATURE                (ECCP_CMD_BASE+7)
#define ECDSA_VERIFY_SIGNATURE                  (ECCP_CMD_BASE+8)

#define RSA_CMD_BASE                            (0x80)
#define RSA_GENERATE_PRIME                      (RSA_CMD_BASE+1)
#define RSA_GENERATE_KEY                        (RSA_CMD_BASE+2)
#define RSA_GENERATE_CRT_KEY                    (RSA_CMD_BASE+3)
#define RSA_ENCRYPT                             (RSA_CMD_BASE+4)
#define RSA_DECRYPT                             (RSA_CMD_BASE+5)
#define RSA_CRT_DECRYPT                         (RSA_CMD_BASE+6)
#define RSA_GENERATE_SIGNATURE                  (RSA_CMD_BASE+7)
#define RSA_CRT_GENERATE_SIGNATURE              (RSA_CMD_BASE+8)
#define RSA_VERIFY_SIGNATURE                    (RSA_CMD_BASE+9)

#define HASH_CMD_BASE                           (0xA0)
#define HASH_CALC                               (HASH_CMD_BASE+1)

#define CMD_BASIC_MAC_BUFF_LEN 32
#define CMD_BASIC_TWO_KEY_MAC_BUFF_LEN 64

typedef enum {
    CMD_KEY_INTERNAL = 0,
    CMD_KEY_EXTERNAL_PLAINTEXT,
    CMD_KEY_EXTERNAL_CIPHERTEXT,
    CMD_KEY_MAX,
} cmd_key_type_e;

typedef enum {
    CMD_RETURN_SUCESS = 0,
    CMD_RETURN_FAIL,
    CMD_RETURN_MAX,
} cmd_return_e;

enum {
    HASH_ALG_SM3           = 0,      // 0
    HASH_ALG_SHA256,                 //
    HASH_ALG_SHA384,                 //
    HASH_ALG_SHA512,                 //
    HASH_ALG_SHA224,                 //
    HASH_ALG_SHA512_224,             //
    HASH_ALG_SHA512_256,             // 6

    HMAC_ALG_SM3,                    // 7
    HMAC_ALG_SHA256,                 //
    HMAC_ALG_SHA384,                 //
    HMAC_ALG_SHA512,                 //
    HMAC_ALG_SHA224,                 //
    HMAC_ALG_SHA512_224,             //
    HMAC_ALG_SHA512_256,             // 13

    CBC_MAC_SKE_ALG_DES,             // 14
    CBC_MAC_SKE_ALG_TDES_128,        //
    CBC_MAC_SKE_ALG_TDES_192,        //
    CBC_MAC_SKE_ALG_AES_128,         //
    CBC_MAC_SKE_ALG_AES_192,         //
    CBC_MAC_SKE_ALG_AES_256,         //
    CBC_MAC_SKE_ALG_SM4,             // 20

    CMAC_SKE_ALG_AES_128,            // 21
    CMAC_SKE_ALG_AES_192,            //
    CMAC_SKE_ALG_AES_256,            //
    CMAC_SKE_ALG_SM4,                // 24

    SKE_ALG_DES_ECB,                 // 25
    SKE_ALG_DES_CBC,                 //
    SKE_ALG_DES_CFB,                 //
    SKE_ALG_DES_OFB,                 //
    SKE_ALG_DES_CTR,                 //
    SKE_ALG_TDES_128_ECB,            //
    SKE_ALG_TDES_128_CBC,            // 31
    SKE_ALG_TDES_128_CFB,            //
    SKE_ALG_TDES_128_OFB,            //
    SKE_ALG_TDES_128_CTR,            //
    SKE_ALG_TDES_192_ECB,            // 35
    SKE_ALG_TDES_192_CBC,            //
    SKE_ALG_TDES_192_CFB,            //
    SKE_ALG_TDES_192_OFB,            //
    SKE_ALG_TDES_192_CTR,            // 39

    SKE_ALG_AES_128_ECB,             // 40
    SKE_ALG_AES_128_CBC,             //
    SKE_ALG_AES_128_CFB,             //
    SKE_ALG_AES_128_OFB,             //
    SKE_ALG_AES_128_CTR,             //
    SKE_ALG_AES_128_XTS,             //
    SKE_ALG_AES_128_GCM,             // 46

    SKE_ALG_AES_192_ECB,             // 47
    SKE_ALG_AES_192_CBC,             //
    SKE_ALG_AES_192_CFB,             //
    SKE_ALG_AES_192_OFB,             //
    SKE_ALG_AES_192_CTR,             //
    SKE_ALG_AES_192_XTS,             //
    SKE_ALG_AES_192_GCM,             //

    SKE_ALG_AES_256_ECB,             // 54
    SKE_ALG_AES_256_CBC,             //
    SKE_ALG_AES_256_CFB,             //
    SKE_ALG_AES_256_OFB,             //
    SKE_ALG_AES_256_CTR,             //
    SKE_ALG_AES_256_XTS,             //
    SKE_ALG_AES_256_GCM,             //

    SKE_ALG_SM4_ECB,                 // 61
    SKE_ALG_SM4_CBC,                 //
    SKE_ALG_SM4_CFB,                 //
    SKE_ALG_SM4_OFB,                 //
    SKE_ALG_SM4_CTR,                 //
    SKE_ALG_SM4_XTS,                 //
    SKE_ALG_SM4_GCM,                 //

    PKE_ALG_SM2,                     // 68
    PKE_ALG_RSA,                     //
    PKE_ALG_ECC,                     //

};

uint32_t send_cmd_and_wait_response(uint32_t cmd[8]);

uint16_t get_big_endian_2byte(void *addr);

uint32_t get_big_endian_4byte(void *addr);

void set_big_endian_2byte_back(uint8_t *addr, uint16_t data);

void set_big_endian_2byte(uint8_t *addr, uint16_t data);

void set_big_endian_4byte_back(uint8_t *addr, uint32_t data);

void set_big_endian_4byte(uint8_t *addr, uint32_t data);

void *get_real_pointer(uint8_t buf[4]);

uint32_t cmd_get_ske_key_bytes(uint8_t crypto_alg_mode_choice);

uint32_t cmd_get_ske_block_bytes(uint8_t crypto_alg_mode_choice);

uint32_t cmd_get_hash_digest_bytes(uint8_t crypto_alg_mode_choice);

#endif
