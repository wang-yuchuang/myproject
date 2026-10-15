/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_ECB_H
#define SKE_ECB_H

#include "ske.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ske_hp_ecb_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_ecb_update_blocks(uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_ecb_final(void);

uint32_t ske_hp_ecb_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                           uint8_t *in, uint8_t *out, uint32_t bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_ecb_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_hp_dma_ecb_update_blocks(uint32_t *in, uint32_t *out, uint32_t words);

uint32_t ske_hp_dma_ecb_final(void);

uint32_t ske_hp_dma_ecb_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                               uint32_t *in, uint32_t *out, uint32_t words);
#endif

#ifdef __cplusplus
}
#endif

#endif
