/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_CMAC_H
#define SKE_CMAC_H

#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ske_ctx_t ske_cmac_ctx[1];
    uint8_t left_bytes;
    uint8_t block_buf[16];
} ske_cmac_ctx_t;

typedef struct {
    ske_ctx_t ske_cmac_ctx[1];
} ske_cmac_dma_ctx_t;

uint32_t ske_hp_cmac_init(ske_cmac_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_cmac_update(ske_cmac_ctx_t *ctx, uint8_t *msg, uint32_t msg_bytes);

uint32_t ske_hp_cmac_final(ske_cmac_ctx_t *ctx, uint8_t *mac, uint8_t mac_bytes);

uint32_t ske_hp_cmac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg, uint32_t msg_bytes,
                     uint8_t *mac, uint8_t mac_bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_cmac_init(ske_cmac_dma_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_ctx_t *ctx, uint32_t *msg,
        uint32_t msg_words);

uint32_t ske_hp_dma_cmac_update_including_last_block(ske_cmac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac);

uint32_t ske_hp_dma_cmac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
                         uint32_t *mac);
#endif

#ifdef __cplusplus
}
#endif

#endif
