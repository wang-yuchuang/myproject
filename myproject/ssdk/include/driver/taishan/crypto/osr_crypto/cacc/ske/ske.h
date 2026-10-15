/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_H
#define SKE_H
#include <types.h>
#include <cmsis_os2.h>

#include <ral/ske_basic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ske_dev {
    paddr_t             base;
    int                 irq;
    osMutexId_t         lock;
} ske_dev_t;

void ske_hp_big_endian_add_uint8(uint8_t *a, uint32_t a_bytes, uint8_t b);

void ske_hp_little_endian_add_uint32(uint32_t *a, uint32_t a_words, uint32_t b);

uint8_t ske_hp_get_key_byte_len(SKE_ALG ske_alg);

uint8_t ske_hp_get_block_byte_len(SKE_ALG ske_alg);

void ske_hp_set_iv(uint8_t *iv, uint32_t block_bytes);

void ske_hp_set_key(SKE_ALG alg, uint8_t *key, uint16_t key_bytes, uint16_t key_idx);

uint32_t ske_hp_init_internal(ske_ctx_t *ctx, SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key,
                              uint16_t sp_key_idx, uint8_t *iv);

uint32_t ske_hp_init(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv);

uint32_t ske_hp_update_blocks(uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_final(void);

uint32_t ske_hp_crypto(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                       uint8_t *in, uint8_t *out, uint32_t bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_init(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv);

uint32_t ske_hp_dma_update_blocks(uint32_t *in, uint32_t *out, uint32_t words);

uint32_t ske_hp_dma_final(void);

uint32_t ske_hp_dma_crypto(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint32_t *in, uint32_t *out, uint32_t words);
#endif

#ifdef __cplusplus
}
#endif

#endif
