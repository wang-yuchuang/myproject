/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef HASH_BASIC_H
#define HASH_BASIC_H

#include "utility.h"
#include "register_base_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUPPORT_HASH_SM3
#define SUPPORT_HASH_MD5
#define SUPPORT_HASH_SHA256
#define SUPPORT_HASH_SHA384
#define SUPPORT_HASH_SHA512
#define SUPPORT_HASH_SHA1
#define SUPPORT_HASH_SHA224
#define SUPPORT_HASH_SHA512_224
#define SUPPORT_HASH_SHA512_256
#define SUPPORT_HASH_SHA3_224
#define SUPPORT_HASH_SHA3_256
#define SUPPORT_HASH_SHA3_384
#define SUPPORT_HASH_SHA3_512

#define HASH_DMA_FUNCTION

#define HMAC_SECURE_PORT_FUNCTION
#ifdef HMAC_SECURE_PORT_FUNCTION
#define HMAC_MAX_KEY_IDX                             (8)   /* if key is from secure port, the max key index(or the number of keys) */
#define HMAC_MAX_SP_KEY_SIZE                         (64)  /* for secure port key, max bytes of one key */
#endif

/* some register offset */
#define HASH_HMAC_OFFSET                             (4)
#define HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET       (8)
#define HASH_LAST_BLOCK_OFFSET                       (16)
#define HASH_DMA_OFFSET                              (16)

/* HASH register struct */
typedef struct hash_reg {
    uint32_t hash_ctrl;                   /* Offset: 0x000 (R/W)  Control register */
    uint32_t hash_cfg;                    /* Offset: 0x004 (R/W)  Config register */
    uint32_t rev_1[2];
    uint32_t hash_risr;                   /* Offset: 0x010 (W0C)  Resource of Interrupt Status register */
    uint32_t hash_imcr;                   /* Offset: 0x014 (R/W)  Interrupt Management and Control register */
    uint32_t hash_misr;                   /* Offset: 0x018 (R)    Multiple Interrupt Status register */
    uint32_t rev_2[5];
    uint32_t hash_msg_len[4];             /* Offset: 0x030 (R/W)  message total length register */
    uint32_t hash_msg_cnt[4];             /* Offset: 0x040 (R/W)  message been handled length register */
    uint32_t rev_3[4];
    uint32_t hash_key_len;                /* Offset: 0x060 (R/W)  HMAC KEY total length register */
    uint32_t rev_4[3];
    uint32_t hash_key_cnt;                /* Offset: 0x070 (R/W)  HMAC KEY been handled length register */
    uint32_t rev_5[15];
    uint32_t hash_mdin_cr;                /* Offset: 0x0B0 (R/W)  Data flag register */
    uint32_t rev_6[3];
    uint32_t hash_m_din;                  /* Offset: 0x0C0 (W)    Hash message Input register */
    uint32_t rev_7[14];
    uint32_t hash_version;                /* Offset: 0x0FC (R)    Version register */
    uint32_t hash_in[50];                 /* Offset: 0x100 (W)    Hash iterator Input register */
    uint32_t rev_8[14];
    uint32_t hash_out[50];                /* Offset: 0x200 (R)    Output register */
    uint32_t rev_9[14];
    uint32_t dma_cr;                      /* Offset: 0x300 (R)    DMA config register */
    uint32_t dma_sr;                      /* Offset: 0x304 (R/W)  DMA Status register */
    uint32_t dma_to;                      /* Offset: 0x308 (R/W)  DMA Status register */
    uint32_t rev_10[1];
    uint32_t dma_sa_l;                    /* Offset: 0x310 (R/W)  DMA Source Address Low part register */
    uint32_t dma_sa_h;                    /* Offset: 0x314 (R/W)  DMA Source Address High part register */
    uint32_t rev_11[2];
    uint32_t dma_da_l;                    /* Offset: 0x320 (R/W)  DMA Destination Address Low part register */
    uint32_t dma_da_h;                    /* Offset: 0x324 (R/W)  DMA Destination Address High part register */
    uint32_t rev_12[2];
    uint32_t dma_rlen;                    /* Offset: 0x330 (R/W)  DMA read data length register */
    uint32_t dma_wlen;                    /* Offset: 0x334 (R/W)  DMA write data length register */
} hash_reg_t;

/* HASH max length */
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
#define HASH_DIGEST_MAX_WORD_LEN       (16)
#define HASH_BLOCK_MAX_WORD_LEN        (36)
#elif (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) ||defined(SUPPORT_HASH_SHA512_224) ||defined(SUPPORT_HASH_SHA512_256))
#define HASH_DIGEST_MAX_WORD_LEN       (16)
#define HASH_BLOCK_MAX_WORD_LEN        (32)
#else
#define HASH_DIGEST_MAX_WORD_LEN       (8)
#define HASH_BLOCK_MAX_WORD_LEN        (16)
#endif

#define HASH_BLOCK_MAX_BYTE_LEN        (HASH_BLOCK_MAX_WORD_LEN<<2)

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
#define HASH_ITERATOR_MAX_WORD_LEN     (50)
#else
#define HASH_ITERATOR_MAX_WORD_LEN     HASH_DIGEST_MAX_WORD_LEN
#endif

#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) ||defined(SUPPORT_HASH_SHA512_224) ||defined(SUPPORT_HASH_SHA512_256))
#define HASH_TOTAL_LEN_MAX_WORD_LEN     (4)
#else
#define HASH_TOTAL_LEN_MAX_WORD_LEN     (2)
#endif

/* HASH algorithm definition */
typedef enum {
#ifdef SUPPORT_HASH_SM3
    HASH_SM3                      = 0,
#endif

#ifdef SUPPORT_HASH_MD5
    HASH_MD5                      = 1,
#endif

#ifdef SUPPORT_HASH_SHA256
    HASH_SHA256                   = 2,
#endif

#ifdef SUPPORT_HASH_SHA384
    HASH_SHA384                   = 3,
#endif

#ifdef SUPPORT_HASH_SHA512
    HASH_SHA512                   = 4,
#endif

#ifdef SUPPORT_HASH_SHA1
    HASH_SHA1                     = 5,
#endif

#ifdef SUPPORT_HASH_SHA224
    HASH_SHA224                   = 6,
#endif

#ifdef SUPPORT_HASH_SHA512_224
    HASH_SHA512_224               = 7,
#endif

#ifdef SUPPORT_HASH_SHA512_256
    HASH_SHA512_256               = 8,
#endif

#ifdef SUPPORT_HASH_SHA3_224
    HASH_SHA3_224                 = 9,
#endif

#ifdef SUPPORT_HASH_SHA3_256
    HASH_SHA3_256                 = 10,
#endif

#ifdef SUPPORT_HASH_SHA3_384
    HASH_SHA3_384                 = 11,
#endif

#ifdef SUPPORT_HASH_SHA3_512
    HASH_SHA3_512                 = 12,
#endif
} HASH_ALG;

/* HASH return code */
enum HASH_RET_CODE {
    HASH_SUCCESS = 0,
    HASH_BUFFER_NULL,
    HASH_CONFIG_INVALID,
    HASH_INPUT_INVALID,
    HASH_LEN_OVERFLOW,
    HASH_ERROR,
};

typedef void (*HASH_CALLBACK)(void);

uint32_t hash_get_version(void);

void hash_set_cpu_mode(void);

void hash_set_dma_mode(void);

void hash_set_hash_mode(void);

void hash_set_hmac_mode(void);

void hash_set_hmac_key_mode(void);

void hash_clear_hmac_key_mode(void);

void hash_clear_risp(void);

void hash_set_hmac_key_cnt(uint32_t bitlen);

void hash_set_hmac_key_len(uint32_t bitlen);

void hash_hmac_key_opr_one_block(uint32_t *key, uint32_t block_byte_len);

void hash_hmac_key_opr(uint8_t *key, uint32_t key_bytes);

void hash_hmac_disable_secure_port(void);

void hash_hmac_enable_secure_port(uint16_t sp_key_idx);

void hash_hmac_sp_key_opr(uint32_t key_bits);

void hash_set_alg(HASH_ALG hash_alg);

uint32_t hash_check_whether_sha3_alg(HASH_ALG hash_alg);

void hash_update_config(void);

void hash_enable_cpu_interruption(void);

void hash_disable_cpu_interruption(void);

void hash_enable_dma_interruption(void);

void hash_disable_dma_interruption(void);

void hash_clear_dma_ch_en(void);

void hash_set_last_block(uint8_t tag);

void hash_set_endian_uint32(void);

void hash_get_iterator(uint8_t *iterator, uint8_t hash_iterator_words);

void hash_set_iterator(uint32_t *iterator, uint8_t hash_iterator_words);

void hash_clear_msg_len(void);

void hash_set_msg_len(uint32_t bytelen);

void hash_set_msg_total_bit_len(uint32_t *msg_total_bits, uint32_t block_byte_len);

void hash_start(void);

void hash_wait_till_done(void);

void hash_input_msg(uint8_t *msg, uint32_t msg_words);

void hash_input_msg_u8(uint8_t *msg, uint32_t msg_bytes);

void hash_set_dma_ctl0(uint32_t ctl0);

void hash_set_dma_ctl1(uint32_t ctl1);

void hash_set_dma_ch_ctl(uint32_t ch_ctl);

void hash_set_dma_sublen(uint32_t sublen);

void hash_set_dma_ch_rctl(uint32_t ch_rctl);

void hash_set_dma_ch_wctl(uint32_t ch_wctl);

void hash_set_dma_wlen(uint32_t wlen);

void hash_clear_dma_sa_da(void);

#ifdef HASH_DMA_FUNCTION
void hash_dma_operate(uint32_t *msg, uint32_t *iterator, uint32_t msgBitLen, HASH_CALLBACK callback);
#endif

#ifdef __cplusplus
}
#endif

#endif

