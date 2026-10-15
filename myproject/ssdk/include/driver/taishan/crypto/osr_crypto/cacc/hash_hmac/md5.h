/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef MD5_H
#define MD5_H

#include "hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_HASH_MD5

typedef hash_ctx_t md5_ctx_t;

#ifdef HASH_DMA_FUNCTION
typedef hash_dma_ctx_t md5_dma_ctx_t;
#endif

uint32_t md5_init(md5_ctx_t *ctx);

uint32_t md5_update(md5_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t md5_final(md5_ctx_t *ctx, uint8_t *digest);

uint32_t md5_(uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef HASH_DMA_FUNCTION
uint32_t md5_dma_init(md5_dma_ctx_t *ctx, HASH_CALLBACK callback);

uint32_t md5_dma_update_blocks(md5_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator);

uint32_t md5_dma_final(md5_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t md5_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
