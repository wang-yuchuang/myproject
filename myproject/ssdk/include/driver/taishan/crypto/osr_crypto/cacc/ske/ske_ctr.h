/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SKE_CTR_H
#define SKE_CTR_H

#include "ske.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ske_hp_ctr_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv);

uint32_t ske_hp_ctr_update_blocks(uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_hp_ctr_final(void);

uint32_t ske_hp_ctr_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint8_t *in, uint8_t *out, uint32_t bytes);

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_ctr_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv);

uint32_t ske_hp_dma_ctr_update_blocks(uint32_t *in, uint32_t *out, uint32_t words);

uint32_t ske_hp_dma_ctr_final(void);

uint32_t ske_hp_dma_ctr_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                               uint32_t *in, uint32_t *out, uint32_t words);
#endif

#ifdef __cplusplus
}
#endif

#endif
