/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SHA384_H
#define SHA384_H

#include "hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_HASH_SHA384

typedef hash_ctx_t sha384_ctx_t;

#ifdef HASH_DMA_FUNCTION
typedef hash_dma_ctx_t sha384_dma_ctx_t;
#endif

uint32_t sha384_init(sha384_ctx_t *ctx);

uint32_t sha384_update(sha384_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t sha384_final(sha384_ctx_t *ctx, uint8_t *digest);

uint32_t sha384(uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef HASH_DMA_FUNCTION
uint32_t sha384_dma_init(sha384_dma_ctx_t *ctx, HASH_CALLBACK callback);

uint32_t sha384_dma_update_blocks(sha384_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator);

uint32_t sha384_dma_final(sha384_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t sha384_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
