/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_GCM_H
#define SKE_GCM_H

#include "ske.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SKE_HP_GCM_MAX_BYTES    (16)

typedef struct {
    uint8_t buf[16];
    ske_ctx_t ske_gcm_ctx[1];
    SKE_CRYPTO crypto;
    uint32_t aad_bytes;
    uint32_t c_bytes;
    uint8_t current_bytes;
} ske_gcm_ctx_t;

uint32_t ske_hp_gcm_init(ske_gcm_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                         uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes);

uint32_t ske_hp_gcm_aad(ske_gcm_ctx_t *ctx, uint8_t *aad);

uint32_t ske_hp_gcm_update(ske_gcm_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_gcm_final(ske_gcm_ctx_t *ctx, uint8_t *tag, uint32_t tag_bytes);

uint32_t ske_hp_gcm_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint32_t iv_bytes, uint8_t *aad, uint32_t aad_bytes, uint8_t *in, uint8_t *out, uint32_t c_bytes,
                           uint8_t *tag, uint32_t tag_bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_gcm_init(ske_gcm_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                             uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes);

uint32_t ske_hp_dma_gcm_update_blocks(ske_gcm_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t tag_bytes);

uint32_t ske_hp_dma_gcm_final(ske_gcm_ctx_t *ctx);

uint32_t ske_hp_dma_gcm_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                               uint32_t iv_bytes, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, uint32_t tag_bytes);
#endif

#ifdef __cplusplus
}
#endif

#endif
