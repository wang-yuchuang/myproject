/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_CBC_MAC_H
#define SKE_CBC_MAC_H

#include "ske.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ske_ctx_t ske_cbc_mac_ctx[1];
    uint8_t is_updated;
    uint8_t left_bytes;
    uint8_t block_buf[16];
    SKE_PADDING padding;
} ske_cbc_mac_ctx_t;

typedef struct {
    ske_ctx_t ske_cbc_mac_ctx[1];
    uint32_t tmp_output_block[4];
} ske_cbc_mac_dma_ctx_t;

uint32_t ske_hp_cbc_mac_init(ske_cbc_mac_ctx_t *ctx, SKE_ALG alg, SKE_PADDING padding, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_cbc_mac_update(ske_cbc_mac_ctx_t *ctx, uint8_t *msg, uint32_t msg_bytes);

uint32_t ske_hp_cbc_mac_final(ske_cbc_mac_ctx_t *ctx, uint8_t *mac, uint8_t mac_bytes);

uint32_t ske_hp_cbc_mac(SKE_ALG alg, SKE_PADDING padding, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg,
                        uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_cbc_mac_init(ske_cbc_mac_dma_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_mac_dma_ctx_t *ctx, uint32_t *msg,
        uint32_t msg_words);

uint32_t ske_hp_dma_cbc_mac_update_including_last_block(ske_cbc_mac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac);

uint32_t ske_hp_dma_cbc_mac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
                            uint32_t *mac);
#endif

#ifdef __cplusplus
}
#endif

#endif
