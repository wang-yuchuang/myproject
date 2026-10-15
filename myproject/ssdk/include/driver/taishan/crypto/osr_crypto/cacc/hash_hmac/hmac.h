/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef HMAC_H
#define HMAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"

#define HMAC_IPAD                 (0x36363636)
#define HMAC_OPAD                 (0x5c5c5c5c)
#define HMAC_IPAD_XOR_OPAD        (HMAC_IPAD ^ HMAC_OPAD)

/*HMAC context*/
typedef struct {
    uint32_t K0[HASH_BLOCK_MAX_WORD_LEN];
    uint32_t key_flag;
    HASH_ALG hash_alg;
    hash_ctx_t hash_ctx[1];
} hmac_ctx_t;

/*HMAC DMA context*/
#ifdef HASH_DMA_FUNCTION
typedef struct {
    hmac_ctx_t hmac_ctx[1];
    hash_dma_ctx_t hash_dma_ctx[1];
} hmac_dma_ctx_t;
#endif

uint32_t hmac_init(hmac_ctx_t *ctx, HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes);

uint32_t hmac_update(hmac_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t hmac_final(hmac_ctx_t *ctx, uint8_t *mac);

uint32_t hmac(HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint8_t *msg, uint32_t msg_bytes, uint8_t *mac);

#ifdef HASH_DMA_FUNCTION
uint32_t hmac_dma_init(hmac_dma_ctx_t *ctx, HASH_ALG hash_alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes,
                       HASH_CALLBACK callback);

uint32_t hmac_dma_update_blocks(hmac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *tmp_iterator);

uint32_t hmac_dma_final(hmac_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *tmp_iterator,
                        uint8_t *mac);

uint32_t hmac_dma(HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, uint32_t msg_bytes,
                  uint32_t *tmp_iterator, uint8_t *mac, HASH_CALLBACK callback);
#endif

#ifdef __cplusplus
}
#endif

#endif
