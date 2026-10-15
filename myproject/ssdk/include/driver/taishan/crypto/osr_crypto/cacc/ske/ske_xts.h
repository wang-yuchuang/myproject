/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_XTS_H
#define SKE_XTS_H

#include "ske.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ske_xts_ctx {
    ske_ctx_t ske_xts_ctx[1];
    SKE_CRYPTO crypto;
    uint32_t c_bytes;
    uint32_t current_bytes;
} ske_xts_ctx_t;

uint32_t ske_hp_xts_init(ske_xts_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                         uint8_t *i, uint32_t c_bytes);

uint32_t ske_hp_xts_update_blocks(ske_xts_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_xts_update_including_last_2_blocks(ske_xts_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_xts_final(ske_xts_ctx_t *ctx);

uint32_t ske_hp_xts_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *i,
                           uint8_t *in, uint8_t *out, uint32_t bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_xts_init(ske_xts_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                             uint8_t *i, uint32_t c_bytes);

uint32_t ske_hp_dma_xts_update_blocks(ske_xts_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t words);

uint32_t ske_hp_dma_xts_update_including_last_2_blocks(ske_xts_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t bytes);

uint32_t ske_hp_dma_xts_final(ske_xts_ctx_t *ctx);

uint32_t ske_hp_dma_xts_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *i,
                               uint32_t *in, uint32_t *out, uint32_t c_bytes);
#endif

#ifdef __cplusplus
}
#endif

#endif
