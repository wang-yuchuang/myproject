/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include "hmac.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_HASH_SHA256

typedef hmac_ctx_t HMAC_SHA256_CTX;

#ifdef HASH_DMA_FUNCTION
typedef hmac_dma_ctx_t HMAC_SHA256_DMA_CTX;
#endif

uint32_t hmac_sha256_init(HMAC_SHA256_CTX *ctx, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes);

uint32_t hmac_sha256_update(HMAC_SHA256_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t hmac_sha256_final(HMAC_SHA256_CTX *ctx, uint8_t *mac);

uint32_t hmac_sha256(uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint8_t *msg, uint32_t msg_bytes, uint8_t *mac);


#ifdef HASH_DMA_FUNCTION
uint32_t hmac_sha256_dma_init(HMAC_SHA256_DMA_CTX *ctx, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes,
                              HASH_CALLBACK callback);

uint32_t hmac_sha256_dma_update_blocks(HMAC_SHA256_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *tmp_iterator);

uint32_t hmac_sha256_dma_final(HMAC_SHA256_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes,
                               uint32_t *tmp_iterator, uint8_t *mac);

uint32_t hmac_sha256_dma(uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, uint32_t msg_bytes,
                         uint32_t *tmp_iterator, uint8_t *mac, HASH_CALLBACK callback);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
