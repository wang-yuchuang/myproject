/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_common.h>
#include <driver/taishan/crypto/osr_crypto/mailbox/soc_seip_reg.h>

uint32_t send_cmd_and_wait_response(uint32_t cmd[8])
{
    rSOCMBOX_CMD_D0  = cmd[0];
    rSOCMBOX_CMD_D1  = cmd[1];
    rSOCMBOX_CMD_D2  = cmd[2];
    rSOCMBOX_CMD_D3  = cmd[3];
    rSOCMBOX_CMD_D4  = cmd[4];
    rSOCMBOX_CMD_D5  = cmd[5];
    rSOCMBOX_CMD_D6  = cmd[6];
    rSOCMBOX_CMD_D7  = cmd[7];
    /*enable irq*/
    rSOCMBOX_INT_EN = 0x02;
    rSOCMBOX_INT_STA = 0x01;

    /*wait for seip work done*/
    while (rSOCMBOX_INT_STA != 0x02) {
    }

    rSOCMBOX_INT_STA = 0x02;

    return rSOCMBOX_RSP_D0;
}

uint16_t get_big_endian_2byte(void *addr)
{
    uint8_t *pdata = (uint8_t *)addr;
    return (((uint16_t)(*pdata) << 8) + * (pdata + 1));
}

uint32_t get_big_endian_4byte(void *addr)
{
    uint8_t *pdata = (uint8_t *)addr;
    return (((uint32_t)(*pdata) << 24) +
            ((uint32_t)(*(pdata + 1)) << 16) +
            ((uint32_t)(*(pdata + 2)) << 8) +
            * (pdata + 3)
           );
}

void set_big_endian_2byte_back(uint8_t *addr, uint16_t data)
{
    uint8_t *pdata = (uint8_t *)addr;
    *(pdata) = data & 0xFF;
    *(pdata + 1) = (data >> 8) & 0xFF;
}

void set_big_endian_2byte(uint8_t *addr, uint16_t data)
{
    uint8_t *pdata = (uint8_t *)addr;
    *(pdata) = (data >> 8) & 0xFF;
    *(pdata + 1) = data & 0xFF;
}

void set_big_endian_4byte_back(uint8_t *addr, uint32_t data)
{
    uint32_t *p_32;
    uint8_t *p_8;

    if (((uint32_t)addr) & 3) {
        p_8 = (uint8_t *)addr;
        *p_8     = data & 0xFF;
        *(p_8 + 1) = (data >> 8) & 0xFF;
        *(p_8 + 2) = (data >> 16) & 0xFF;
        *(p_8 + 3) = (data >> 24) & 0xFF;
    }
    else {
        p_32 = (uint32_t *)addr;
        *p_32  = data;
    }
}

void set_big_endian_4byte(uint8_t *addr, uint32_t data)
{
    uint32_t *p_32;
    uint8_t *p_8;

    if (((uint32_t)addr) & 3) {
        p_8 = (uint8_t *)addr;
        *p_8     = (data >> 24) & 0xFF;
        *(p_8 + 1) = (data >> 16) & 0xFF;
        *(p_8 + 2) = (data >> 8) & 0xFF;
        *(p_8 + 3) = data & 0xFF;
    }
    else {
        p_32 = (uint32_t *)addr;
        *p_32  = (data << 24) & 0xFF000000;
        *p_32 |= (data << 8)  & 0x00FF0000;
        *p_32 |= (data >> 8)  & 0x0000FF00;
        *p_32 |= (data >> 24) & 0x000000FF;
    }
}

void *get_real_pointer(uint8_t buf[4])
{
    uint32_t base = 0;
    uint32_t p = 0;

    p += ((uint32_t)(buf[0])) << 24;
    p += ((uint32_t)(buf[1])) << 16;
    p += ((uint32_t)(buf[2])) << 8;
    p += ((uint32_t)(buf[3]));

    base += p;

    return (void *)(base);
}

uint32_t cmd_get_ske_key_bytes(uint8_t crypto_alg_mode_choice)
{
    uint32_t key_bytes;

    switch (crypto_alg_mode_choice) {
        case SKE_ALG_DES_ECB :
        case SKE_ALG_DES_CBC :
        case SKE_ALG_DES_CFB :
        case SKE_ALG_DES_OFB :
        case SKE_ALG_DES_CTR :
        case CBC_MAC_SKE_ALG_DES :
            key_bytes = 8;
            break;

        case SKE_ALG_TDES_128_ECB :
        case SKE_ALG_TDES_128_CBC :
        case SKE_ALG_TDES_128_CFB :
        case SKE_ALG_TDES_128_OFB :
        case SKE_ALG_TDES_128_CTR :
        case CBC_MAC_SKE_ALG_TDES_128 :
        case SKE_ALG_AES_128_ECB :
        case SKE_ALG_AES_128_CBC :
        case SKE_ALG_AES_128_CFB :
        case SKE_ALG_AES_128_OFB :
        case SKE_ALG_AES_128_CTR :
        case SKE_ALG_AES_128_GCM :
        case CBC_MAC_SKE_ALG_AES_128 :
        case CMAC_SKE_ALG_AES_128 :
        case SKE_ALG_SM4_ECB :
        case SKE_ALG_SM4_CBC :
        case SKE_ALG_SM4_CFB :
        case SKE_ALG_SM4_OFB :
        case SKE_ALG_SM4_CTR :
        case SKE_ALG_SM4_GCM :
        case CBC_MAC_SKE_ALG_SM4 :
        case CMAC_SKE_ALG_SM4 :
            key_bytes = 16;
            break;

        case SKE_ALG_TDES_192_ECB :
        case SKE_ALG_TDES_192_CBC :
        case SKE_ALG_TDES_192_CFB :
        case SKE_ALG_TDES_192_OFB :
        case SKE_ALG_TDES_192_CTR :
        case CBC_MAC_SKE_ALG_TDES_192 :
        case SKE_ALG_AES_192_ECB :
        case SKE_ALG_AES_192_CBC :
        case SKE_ALG_AES_192_CFB :
        case SKE_ALG_AES_192_OFB :
        case SKE_ALG_AES_192_CTR :
        case SKE_ALG_AES_192_GCM :
        case CBC_MAC_SKE_ALG_AES_192 :
        case CMAC_SKE_ALG_AES_192 :
            key_bytes = 24;
            break;

        case SKE_ALG_AES_256_ECB :
        case SKE_ALG_AES_256_CBC :
        case SKE_ALG_AES_256_CFB :
        case SKE_ALG_AES_256_OFB :
        case SKE_ALG_AES_256_CTR :
        case SKE_ALG_AES_256_GCM :
        case CBC_MAC_SKE_ALG_AES_256 :
        case CMAC_SKE_ALG_AES_256 :
        case SKE_ALG_AES_128_XTS :
        case SKE_ALG_SM4_XTS :
            key_bytes = 32;
            break;

        case SKE_ALG_AES_192_XTS :
            key_bytes = 48;
            break;

        case SKE_ALG_AES_256_XTS :
            key_bytes = 64;
            break;

        default:
            key_bytes = 0;  //error
            break;
    }

    return key_bytes;
}


uint32_t cmd_get_ske_block_bytes(uint8_t crypto_alg_mode_choice)
{
    uint32_t block_bytes;

    switch (crypto_alg_mode_choice) {
        case SKE_ALG_DES_ECB :
        case SKE_ALG_DES_CBC :
        case SKE_ALG_DES_CFB :
        case SKE_ALG_DES_OFB :
        case SKE_ALG_DES_CTR :
        case CBC_MAC_SKE_ALG_DES :

        case SKE_ALG_TDES_128_ECB :
        case SKE_ALG_TDES_128_CBC :
        case SKE_ALG_TDES_128_CFB :
        case SKE_ALG_TDES_128_OFB :
        case SKE_ALG_TDES_128_CTR :
        case CBC_MAC_SKE_ALG_TDES_128 :

        case SKE_ALG_TDES_192_ECB :
        case SKE_ALG_TDES_192_CBC :
        case SKE_ALG_TDES_192_CFB :
        case SKE_ALG_TDES_192_OFB :
        case SKE_ALG_TDES_192_CTR :
        case CBC_MAC_SKE_ALG_TDES_192 :

            block_bytes = 8;
            break;

        case SKE_ALG_AES_128_ECB :
        case SKE_ALG_AES_128_CBC :
        case SKE_ALG_AES_128_CFB :
        case SKE_ALG_AES_128_OFB :
        case SKE_ALG_AES_128_CTR :
        case SKE_ALG_AES_128_GCM :
        case CBC_MAC_SKE_ALG_AES_128 :
        case CMAC_SKE_ALG_AES_128 :
        case SKE_ALG_AES_128_XTS :

        case SKE_ALG_SM4_ECB :
        case SKE_ALG_SM4_CBC :
        case SKE_ALG_SM4_CFB :
        case SKE_ALG_SM4_OFB :
        case SKE_ALG_SM4_CTR :
        case SKE_ALG_SM4_GCM :
        case CBC_MAC_SKE_ALG_SM4 :
        case CMAC_SKE_ALG_SM4 :
        case SKE_ALG_SM4_XTS :

        case SKE_ALG_AES_192_ECB :
        case SKE_ALG_AES_192_CBC :
        case SKE_ALG_AES_192_CFB :
        case SKE_ALG_AES_192_OFB :
        case SKE_ALG_AES_192_CTR :
        case SKE_ALG_AES_192_GCM :
        case CBC_MAC_SKE_ALG_AES_192 :
        case CMAC_SKE_ALG_AES_192 :
        case SKE_ALG_AES_192_XTS :

        case SKE_ALG_AES_256_ECB :
        case SKE_ALG_AES_256_CBC :
        case SKE_ALG_AES_256_CFB :
        case SKE_ALG_AES_256_OFB :
        case SKE_ALG_AES_256_CTR :
        case SKE_ALG_AES_256_GCM :
        case CBC_MAC_SKE_ALG_AES_256 :
        case CMAC_SKE_ALG_AES_256 :
        case SKE_ALG_AES_256_XTS :

            block_bytes = 16;
            break;

        default:
            block_bytes = 0;
            break;
    }

    return block_bytes;
}

uint32_t cmd_get_hash_digest_bytes(uint8_t crypto_alg_mode_choice)
{
    uint32_t digest_bytes;

    switch (crypto_alg_mode_choice) {
        case HASH_ALG_SHA224:
        case HASH_ALG_SHA512_224:
            digest_bytes = 28;
            break;

        case HASH_ALG_SM3:
        case HASH_ALG_SHA256:
        case HASH_ALG_SHA512_256:
            digest_bytes = 32;
            break;

        case HASH_ALG_SHA384:
            digest_bytes = 48;
            break;

        case HASH_ALG_SHA512:
            digest_bytes = 64;
            break;

        default:
            digest_bytes = 0;
    }

    return digest_bytes;
}