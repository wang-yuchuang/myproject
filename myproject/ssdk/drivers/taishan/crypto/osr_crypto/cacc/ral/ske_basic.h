/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_BASIC_H
#define SKE_BASIC_H

#include "register_base_addr.h"
#include "utility.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUPPORT_SKE_DES
#define SUPPORT_SKE_TDES_128
#define SUPPORT_SKE_TDES_192
#define SUPPORT_SKE_AES_128
#define SUPPORT_SKE_AES_192
#define SUPPORT_SKE_AES_256
#define SUPPORT_SKE_SM4

#define SUPPORT_SKE_MODE_ECB
#define SUPPORT_SKE_MODE_CBC
#define SUPPORT_SKE_MODE_CFB
#define SUPPORT_SKE_MODE_OFB
#define SUPPORT_SKE_MODE_CTR
#define SUPPORT_SKE_MODE_XTS
#define SUPPORT_SKE_MODE_GCM
#define SUPPORT_SKE_MODE_CMAC
#define SUPPORT_SKE_MODE_CBC_MAC

#define SKE_HP_DMA_FUNCTION

#define SKE_SECURE_PORT_FUNCTION
#ifdef SKE_SECURE_PORT_FUNCTION
/* if key is from secure port, the max key index(or the number of keys) */
#define SKE_MAX_KEY_IDX                                (9)
#endif

/* some register offset */
#define SKE_HP_REVERSE_BYTE_ORDER_IN_WORD_OFFSET       (24)
#define SKE_HP_MODE_OFFSET                             (28)
#define SKE_HP_CRYPTO_OFFSET                           (11)
#define SKE_HP_UP_CFG_OFFSET                           (12)
#define SKE_HP_DMA_OFFSET                              (16)
#define SKE_HP_DMA_LL_OFFSET                           (17)
#define SKE_HP_LAST_DATA_OFFSET                        (16)

/* SKE register struct */
typedef struct ske_hp_reg {
    uint32_t ctrl;                      /* Offset: 0x000 (W1S) SKE Control Register */
    uint32_t cfg;                       /* Offset: 0x004 (R/W) SKE Config Register */
    uint32_t sr;                        /* Offset: 0x008 (R)   SKE Status Register */
    uint32_t risr;                      /* Offset: 0x00C (W0C) SKE Interrupt Source Register */
    uint32_t imcr;                      /* Offset: 0x010 (R/W) SKE Interrupt Enable Register */
    uint32_t misr;                      /* Offset: 0x014 (R)   SKE Interrupt Output Register */
    uint32_t rev1[1];
    uint32_t sp;                        /* Offset: 0x01C (R/W) SKE Secure Port Register */
    uint32_t key1[8];                   /* Offset: 0x020 (R/W) Key1 */
    uint32_t key2[8];                   /* Offset: 0x040 (R/W) Key2 */
    uint32_t ske_a_len_l;               /* Offset: 0x060 (R/W) CCM/GCM mode AAD length low Register */
    uint32_t ske_a_len_h;               /* Offset: 0x064 (R/W) CCM/GCM mode AAD length high Register */
    uint32_t ske_c_len_l;               /* Offset: 0x068 (R/W) CCM/GCM/XTS mode plaintext/ciphertext length low Register */
    uint32_t ske_c_len_h;               /* Offset: 0x06C (R/W) CCM/GCM/XTS mode plaintext/ciphertext length high Register */
    uint32_t iv[4];                     /* Offset: 0x070 (R/W) Initial Vector */
    uint32_t m_din_cr;                  /* Offset: 0x080 (R/W) SKE Input Register */
    uint32_t rev3[3];
    uint32_t m_din[4];                  /* Offset: 0x090 (R/W) SKE Input Register */
    uint32_t rev4[4];
    uint32_t m_dout[4];                 /* Offset: 0x0B0 (R)   SKE Output Register */
    uint32_t rev5[15];
    uint32_t ske_version;               /* Offset: 0x0FC (R)   SKE Version Register */
    uint32_t ske_seed[36];              /* Offset: 0x100 (R/W) SKE Seed Register */
    uint32_t ske_alarm;                 /* Offset: 0x190 (R/W) SKE Alarm Register */
    uint32_t rev6[91];
    uint32_t dma_cr;                    /* Offset: 0x300 (R/W) DMA Config register */
    uint32_t dma_sr;                    /* Offset: 0x304 (W0C) DMA Status register */
    uint32_t dma_to;                    /* Offset: 0x308 (R/W) DMA Timeout Threshold register */
    uint32_t rev7[1];
    uint32_t dma_sa_l;                  /* Offset: 0x310 (R/W) DMA Source Address Low part register */
    uint32_t dma_sa_h;                  /* Offset: 0x314 (R/W) DMA Source Address High part register */
    uint32_t rev8[2];
    uint32_t dma_da_l;                  /* Offset: 0x320 (R/W) DMA Destination Address Low part register */
    uint32_t dma_da_h;                  /* Offset: 0x324 (R/W) DMA Destination Address High part register */
    uint32_t rev9[2];
    uint32_t dma_rlen;                  /* Offset: 0x330 (R/W) DMA read Length register */
    uint32_t dma_wlen;                  /* Offset: 0x334 (R/W) DMA write Length register */
    uint32_t rev10[2];
    uint32_t dma_awcc;                  /* Offset: 0x340 (R/W) DMA AWCC register */
    uint32_t dma_arcc;                  /* Offset: 0x344 (R/W) DMA ARCC register */
    uint32_t dma_llp_l;
    uint32_t dma_llp_h;
} ske_hp_reg_t;

/* SKE Operation Mode */
typedef enum {
    SKE_MODE_BYPASS               = 0,

#ifdef SUPPORT_SKE_MODE_ECB
    SKE_MODE_ECB                  = 1,
#endif

#ifdef SUPPORT_SKE_MODE_XTS
    SKE_MODE_XTS                  = 2,
#endif

#ifdef SUPPORT_SKE_MODE_CBC
    SKE_MODE_CBC                  = 3,
#endif

#ifdef SUPPORT_SKE_MODE_CFB
    SKE_MODE_CFB                  = 4,
#endif

#ifdef SUPPORT_SKE_MODE_OFB
    SKE_MODE_OFB                  = 5,
#endif

#ifdef SUPPORT_SKE_MODE_CTR
    SKE_MODE_CTR                  = 6,
#endif

#ifdef SUPPORT_SKE_MODE_CMAC
    SKE_MODE_CMAC                 = 7,
#endif

#ifdef SUPPORT_SKE_MODE_CBC_MAC
    SKE_MODE_CBC_MAC              = 8,
#endif

#ifdef SUPPORT_SKE_MODE_GCM
    SKE_MODE_GCM                  = 9,
#endif

#ifdef SUPPORT_SKE_MODE_CCM
    SKE_MODE_CCM                  = 10,
#endif
} SKE_MODE;

typedef enum {
    SKE_CRYPTO_ENCRYPT       = 0,
    SKE_CRYPTO_DECRYPT,
} SKE_CRYPTO;

typedef enum {
#ifdef SUPPORT_SKE_DES
    SKE_ALG_DES            = 0,
#endif

#ifdef SUPPORT_SKE_TDES_128
    SKE_ALG_TDES_128       = 1,
#endif

#ifdef SUPPORT_SKE_TDES_192
    SKE_ALG_TDES_192       = 2,
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    SKE_ALG_TDES_EEE_128   = 3,
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
    SKE_ALG_TDES_EEE_192   = 4,
#endif

#ifdef SUPPORT_SKE_AES_128
    SKE_ALG_AES_128        = 5,
#endif

#ifdef SUPPORT_SKE_AES_192
    SKE_ALG_AES_192        = 6,
#endif

#ifdef SUPPORT_SKE_AES_256
    SKE_ALG_AES_256        = 7,
#endif

#ifdef SUPPORT_SKE_SM4
    SKE_ALG_SM4            = 8,
#endif
} SKE_ALG;

enum SKE_RET_CODE {
    SKE_SUCCESS = 0,
    SKE_BUFFER_NULL,
    SKE_CONFIG_INVALID,
    SKE_INPUT_INVALID,
    SKE_ATTACK_ALARM,
    SKE_ERROR,
};

typedef enum {
    SKE_NO_PADDING,
    SKE_ZERO_PADDING,
} SKE_PADDING;

typedef struct {
    uint8_t block_bytes;
    uint8_t block_words;
} ske_ctx_t;

typedef struct {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t next_llp;
    uint32_t last_len;
} dma_ll_node_t;

uint32_t ske_get_version(void);

void ske_hp_set_cpu_mode(void);

void ske_hp_set_dma_mode(void);

void ske_hp_enable_dma_linked_list(void);

void ske_hp_disable_dma_linked_list(void);

void ske_hp_set_endian_uint32(void);

void ske_hp_enable_secure_port(uint16_t sp_key_idx);

void ske_hp_disable_secure_port(void);

void ske_hp_set_crypto(SKE_CRYPTO crypto);

void ske_hp_set_alg(SKE_ALG ske_alg);

void ske_hp_set_mode(SKE_MODE mode);

void ske_hp_set_last_block(uint32_t is_last_block);

void ske_hp_set_last_block_len(uint32_t bytes);

uint32_t ske_hp_set_seed(void);

void ske_hp_start(void);

uint32_t ske_hp_calc_wait_till_done(void);

void ske_hp_set_key_uint32(uint32_t *key, uint32_t idx, uint32_t key_words);

void ske_hp_set_iv_uint32(uint32_t *iv, uint32_t block_words);

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
void ske_hp_set_aad_len_uint32(uint32_t aad_bytes);
#endif

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_XTS))
void ske_hp_set_c_len_uint32(uint32_t c_bytes);
#endif

void ske_hp_simple_set_input_block(uint32_t *in, uint32_t block_words);

void ske_hp_simple_get_output_block(uint32_t *out, uint32_t block_words);

uint32_t ske_hp_expand_key(void);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_operate(ske_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t in_words, uint32_t out_words);

#if (defined(SUPPORT_SKE_MODE_CMAC) || defined(SUPPORT_SKE_MODE_CMAC))
uint32_t ske_hp_dma_operate_without_output(ske_ctx_t *ctx, uint32_t *in, uint32_t in_words);
#endif
#endif

uint32_t ske_hp_update_blocks_no_output(ske_ctx_t *ctx, uint8_t *in, uint32_t bytes);

uint32_t ske_hp_update_blocks_internal(ske_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_wait_till_output();

#ifdef __cplusplus
}
#endif


#endif
