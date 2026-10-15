/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske_cfb.h>
#ifdef SUPPORT_SKE_MODE_CFB

uint32_t ske_hp_cfb_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv)
{
    return ske_hp_init(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv);
}

uint32_t ske_hp_cfb_update_blocks(uint8_t *in, uint8_t *out, uint32_t bytes)
{
    return ske_hp_update_blocks(in, out, bytes);
}

uint32_t ske_hp_cfb_final(void)
{
    return ske_hp_final();
}

uint32_t ske_hp_cfb_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint8_t *in, uint8_t *out, uint32_t bytes)
{
    return ske_hp_crypto(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, in, out, bytes);
}

#ifdef SKE_HP_DMA_FUNCTION
uint32_t ske_hp_dma_cfb_init(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv)
{
    return ske_hp_dma_init(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv);
}

uint32_t ske_hp_dma_cfb_update_blocks(uint32_t *in, uint32_t *out, uint32_t words)
{
    return ske_hp_dma_update_blocks(in, out, words);
}

uint32_t ske_hp_dma_cfb_final(void)
{
    return ske_hp_dma_final();
}

uint32_t ske_hp_dma_cfb_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                               uint32_t *in, uint32_t *out, uint32_t words)
{
    return ske_hp_dma_crypto(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, in, out, words);
}

#endif

#endif


