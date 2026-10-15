/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <stdio.h>
#include "ske_basic.h"
#include "utility.h"
#include "trng.h"

/* ske_hp register pointer */
volatile static ske_hp_reg_t *const g_ske_hp_reg = (ske_hp_reg_t *)(SKE_HP_BASE_ADDR);

/* function: get ske IP version
 * parameters: none
 * return: ske IP version
 * caution:
 */
uint32_t ske_get_version(void)
{
    return g_ske_hp_reg->ske_version;
}

/* function: set ske_hp to be CPU mode
 * parameters: none
 * return: none
 * caution:
 */
void ske_hp_set_cpu_mode(void)
{
    g_ske_hp_reg->cfg &= (~((0x1) << SKE_HP_DMA_OFFSET));
}

/* function: set ske_hp to be DMA mode
 * parameters: none
 * return: none
 * caution:
 */
void ske_hp_set_dma_mode(void)
{
    g_ske_hp_reg->cfg |= (0x1) << SKE_HP_DMA_OFFSET;
}

/* function: enable ske_hp DMA linked list function
 * parameters: none
 * return: none
 * caution:
 *     1. this works when DMA mode is enabled
 */
void ske_hp_enable_dma_linked_list(void)
{
    g_ske_hp_reg->cfg |= (0x1) << SKE_HP_DMA_LL_OFFSET;
}

/* function: disable ske_hp DMA linked list function
 * parameters: none
 * return: none
 * caution:
 */
void ske_hp_disable_dma_linked_list(void)
{
    g_ske_hp_reg->cfg &= ~((0x1) << SKE_HP_DMA_LL_OFFSET);
}

/* function: set the ske_hp endian
 * parameters: none
 * return: none
 * caution:
 *     1. actually, this config works for only CPU mode now
 */
void ske_hp_set_endian_uint32(void)
{
    /* clear bit[25:24], and now CPU is big-endian */
    g_ske_hp_reg->cfg &= ~(((uint32_t)3) << SKE_HP_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);

    /* CPU is little-endian, input and output reversed by hardware----ske IP */
#ifndef SKE_HP_CPU_BIG_ENDIAN
    g_ske_hp_reg->cfg |= (((uint32_t)2) << SKE_HP_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);
#endif
}

/* function: enable ske_hp secure port
 * parameters:
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: none
 * caution:
 */
void ske_hp_enable_secure_port(uint16_t sp_key_idx)
{
#ifdef SKE_SECURE_PORT_FUNCTION

    g_ske_hp_reg->sp |= 0x1;

    OTP_KEY_CTRL = 0x0;

    /* use the low 128bit of the 256bit key */
    if (sp_key_idx & 0x8000) {
        OTP_KEY_CTRL = (0x1 << 24);
    }

    /* set key idx */
    if (sp_key_idx == 9) {
        OTP_KEY_CTRL |= 0x100000;
    }
    else {
        sp_key_idx -= 1;
        OTP_KEY_CTRL |= ((uint32_t)(sp_key_idx & 0x7FFF)) << 16;
    }

    OTP_KEY_CTRL |= 0x1;

#endif
}

/* function: disable ske_hp secure port
 * parameters: none
 * return: none
 * caution:
 */
void ske_hp_disable_secure_port(void)
{
    g_ske_hp_reg->sp &= ~(0x1);
}

/* function: set ske_hp encrypting or decrypting
 * parameters:
 *     crypto --------------------- input, SKE_CRYPTO_ENCRYPT or SKE_CRYPTO_DECRYPT
 * return: none
 * caution:
 *     1. please make sure crypto is valid
 */
void ske_hp_set_crypto(SKE_CRYPTO crypto)
{
    g_ske_hp_reg->cfg &= ~(((uint32_t)1) << SKE_HP_CRYPTO_OFFSET);
    g_ske_hp_reg->cfg |= (((uint32_t)crypto) << SKE_HP_CRYPTO_OFFSET);
}

/* function: set ske_hp alg
 * parameters:
 *     ske_alg -------------------- input, ske_hp algorithm
 * return: none
 * caution:
 *     1. please make sure ske_alg is valid
 */
void ske_hp_set_alg(SKE_ALG ske_alg)
{
    uint32_t cfg;

    switch (ske_alg) {
#ifdef SUPPORT_SKE_DES

        case SKE_ALG_DES:
            cfg = 3;
            break;
#endif

#ifdef SUPPORT_SKE_TDES_128

        case SKE_ALG_TDES_128:
#endif
#ifdef SUPPORT_SKE_TDES_192
        case SKE_ALG_TDES_192:
#endif
#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_192))
            cfg = 4;
            break;
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128

        case SKE_ALG_TDES_EEE_128:
#endif
#ifdef SUPPORT_SKE_TDES_EEE_192
        case SKE_ALG_TDES_EEE_192:
#endif
#if (defined(SUPPORT_SKE_TDES_EEE_128) || defined(SUPPORT_SKE_TDES_EEE_192))
            cfg = 5;
            break;
#endif

#ifdef SUPPORT_SKE_AES_128

        case SKE_ALG_AES_128:
            cfg = (1 << 6) | (1 << 4) | (1);
            break;
#endif

#ifdef SUPPORT_SKE_AES_192

        case SKE_ALG_AES_192:
            cfg = (2 << 6) | (2 << 4) | (1);
            break;
#endif

#ifdef SUPPORT_SKE_AES_256

        case SKE_ALG_AES_256:
            cfg = (3 << 6) | (3 << 4) | (1);
            break;
#endif

#ifdef SUPPORT_SKE_SM4

        case SKE_ALG_SM4:
            cfg = 2;
            break;
#endif

        default:
            cfg = 2;
    }

    g_ske_hp_reg->cfg &= (~(0x000000FFU));
    g_ske_hp_reg->cfg |= cfg;
}

/* function: set ske_hp alg operation mode
 * parameters:
 *     mode ----------------------- input, operation mode
 * return: none
 * caution:
 *     1. please make sure mode is valid
 */
void ske_hp_set_mode(SKE_MODE mode)
{
    g_ske_hp_reg->cfg &= (~(0x0000000F << SKE_HP_MODE_OFFSET));
    g_ske_hp_reg->cfg |= (((uint32_t)mode) << SKE_HP_MODE_OFFSET);
}

/* function: set whether ske_hp current input data is the last data or not
 * parameters:
 *     is_last_block -------------- input, 0:no, other:yes
 * return: none
 * caution:
 *     1. just for CMAC/CCM/GCM/XTS mode
 */
void ske_hp_set_last_block(uint32_t is_last_block)
{
    if (is_last_block) {
        g_ske_hp_reg->m_din_cr |= (((uint32_t)1) << SKE_HP_LAST_DATA_OFFSET);
    }
    else {
        g_ske_hp_reg->m_din_cr &= ~(((uint32_t)1) << SKE_HP_LAST_DATA_OFFSET);
    }
}

/* function: set ske_hp current input data bit length
 * parameters:
 *     bytes ---------------------- input, byte length of current input data
 * return: none
 * caution:
 *     1. just for CMAC
 */
void ske_hp_set_last_block_len(uint32_t bytes)
{
    g_ske_hp_reg->m_din_cr &= ~0x000000FF;
    g_ske_hp_reg->m_din_cr |= (bytes << 3);
}

/* function: set ske_hp seed
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t ske_hp_set_seed(void)
{
    if (TRNG_SUCCESS != get_rand((uint8_t *)(g_ske_hp_reg->ske_seed), sizeof(g_ske_hp_reg->ske_seed))) {
        return SKE_ERROR;
    }
    else {
        return SKE_SUCCESS;
    }
}

/* function: ske_hp start to expand key or calc
 * parameters:none
 * return: none
 * caution:
 */
void ske_hp_start(void)
{
    g_ske_hp_reg->risr = 0x0;
    g_ske_hp_reg->ctrl |= 0x1;
}

uint32_t ske_hp_check_attack_state(void)
{

    if (g_ske_hp_reg->ske_alarm & 0x1) {
        return SKE_ATTACK_ALARM;
    }
    else {
        return SKE_SUCCESS;
    }

}

/* function: wait till ske_hp expanding key is done
 * parameters:
 * return: none
 * caution:
 */
uint32_t ske_hp_expand_key_wait_till_done()
{
    uint32_t state_val;

    while (!(g_ske_hp_reg->sr & 0x1)) {
        state_val = ske_hp_check_attack_state();

        if (SKE_SUCCESS == state_val) {
            continue;
        }
        else {
            return state_val;
        }
    }

    return SKE_SUCCESS;
}

/* function: wait till ske_hp is waiting to input
 * parameters:
 * return: none
 * caution:
 */
uint32_t ske_hp_wait_till_input()
{
    uint32_t state_val;

    while (!(g_ske_hp_reg->sr & (1 << 16))) {
        state_val = ske_hp_check_attack_state();

        if (SKE_SUCCESS == state_val) {
            continue;
        }
        else {
            return state_val;
        }
    }

    return SKE_SUCCESS;
}

/* function: wait till ske_hp output is ready
 * parameters:
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_hp_wait_till_output()
{
    uint32_t state_val;

    while (!(g_ske_hp_reg->sr & (1 << 17))) {
        state_val = ske_hp_check_attack_state();

        if (SKE_SUCCESS == state_val) {
            continue;
        }
        else {
            return state_val;
        }
    }

    return SKE_SUCCESS;
}

/* function: wait till ske_hp calculating is done
 * parameters:
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_hp_calc_wait_till_done(void)
{
    uint32_t state_val;

    while (!(g_ske_hp_reg->risr & 0x7)) {
        state_val = ske_hp_check_attack_state();

        if (SKE_SUCCESS == state_val) {
            continue;
        }
        else {
            return state_val;
        }
    }

    return SKE_SUCCESS;
}

/* function: set ske_hp key
 * parameters:
 *     key ------------------------ input, key in word buffer
 *     idx ------------------------ input, key index, only 1 and 2 are valid
 *     key_words ------------------ input, word length of key
 * return: none
 * caution:
 *     1. if idx is 1, set key1 register, else if idx is 2, set key2 register, please
 *        make sure idx is valid
 */
void ske_hp_set_key_uint32(uint32_t *key, uint32_t idx, uint32_t key_words)
{
    volatile uint32_t *key_reg;
    int32_t i;

    if (1 == idx) {
        key_reg = g_ske_hp_reg->key1;
    }
    else {
        key_reg = g_ske_hp_reg->key2;
    }

    for (i = key_words; i > 0; i--) {
        key_reg[i - 1] = key[key_words - i];
    }
}

/* function: set ske_hp iv
 * parameters:
 *     iv ------------------------- input, iv in word buffer
 *     block_words ---------------- input, word length of ske_hp block
 * return: none
 * caution:
 *     1. please make sure the three parameters are valid
 */
void ske_hp_set_iv_uint32(uint32_t *iv, uint32_t block_words)
{
    int32_t i;

    for (i = block_words; i > 0; i--) {
        g_ske_hp_reg->iv[i - 1] = iv[block_words - i];
    }

}

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
/* function: set aad bits(just for ske_hp ccm/gcm mode)
 * parameters:
 *     aad_bytes ------------------ input, byte length of aad
 * return: none
 * caution:
 *     1. this function is just for CCM/GCM mode
 */
void ske_hp_set_aad_len_uint32(uint32_t aad_bytes)
{
    g_ske_hp_reg->ske_a_len_l = ((aad_bytes) << 3) & 0xFFFFFFFF;
    g_ske_hp_reg->ske_a_len_h = aad_bytes >> (32 - 3);
}
#endif

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_XTS))
/* function: set plaintext/ciphertext bits(just for ske_hp ccm/gcm/xts mode)
 * parameters:
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext
 * return: none
 * caution:
 *     1. this function is just for CCM/GCM/XTS mode
 */
void ske_hp_set_c_len_uint32(uint32_t c_bytes)
{
    g_ske_hp_reg->ske_c_len_l = ((c_bytes) << 3) & 0xFFFFFFFF;
    g_ske_hp_reg->ske_c_len_h = c_bytes >> (32 - 3);
}
#endif

/* function: input one block
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext in word buffer
 *     block_words ---------------- input, word length of ske_hp block
 * return: none
 * caution:
 *     1. in is a word buffer of only one block.
 */
void ske_hp_simple_set_input_block(uint32_t *in, uint32_t block_words)
{
    int32_t i;

    for (i = block_words; i > 0; i--) {
        g_ske_hp_reg->m_din[i - 1] = in[block_words - i];
    }
}

/* function: output one block
 * parameters:
 *     out ------------------------ output, one block output of ske_hp in word buffer
 *     block_words ---------------- input, word length of ske_hp block
 * return: none
 * caution:
 */
void ske_hp_simple_get_output_block(uint32_t *out, uint32_t block_words)
{
    int32_t i;

    /* trigger to pop */
    g_ske_hp_reg->ctrl |= 0x02;

    for (i = block_words; i > 0; i--) {
        out[block_words - i] = g_ske_hp_reg->m_dout[i - 1];
    }
}

/* function: ske_hp expand key
 * parameters: none
 * return: none
 * caution:  1. must be called after ske_hp_set_crypto() and ske_hp_set_alg(), and the key is set already.
 */
uint32_t ske_hp_expand_key(void)
{
    uint32_t ret;

    g_ske_hp_reg->cfg |= ((0x1) << SKE_HP_UP_CFG_OFFSET);

    ske_hp_start();
    ret = ske_hp_expand_key_wait_till_done();

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    g_ske_hp_reg->cfg &= ~((0x1) << SKE_HP_UP_CFG_OFFSET);

    return SKE_SUCCESS;
}

/************************* DMA *************************/

void dma_set_time_out_threshold(uint16_t cycle_threshold)
{
    g_ske_hp_reg->dma_to = cycle_threshold;
}

#ifdef SKE_HP_DMA_FUNCTION
/* function: basic ske_hp DMA operation
 * parameters:
 *     ctx ------------------------ input, ske_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_words ------------------- input, word length of in, must be multiples of block word length
 *     out_words ------------------ input, word length of out, must be multiples of block word length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. in_words & out_words must be multiples of block words.
 *     2. it could be without output, namely, out can be NULL, out_words can be 0(for CBC_MAC/CMAC)
 */
uint32_t ske_hp_dma_operate(ske_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t in_words, uint32_t out_words)
{
    uint32_t ret;

    if (NULL == in) {
        return SKE_BUFFER_NULL;
    }

    /* src & dst addr low 32bits */
    g_ske_hp_reg->dma_sa_l = (uint32_t)in;
    g_ske_hp_reg->dma_da_l = (uint32_t)out;

    /* src & dst addr high 32bits */
    if (4 == (sizeof(uint32_t *))) {
        /* in this case, if using (((uint64_t)in)>>32), you may get 0xFFFFFFFF, not 0 you expected! */
        g_ske_hp_reg->dma_sa_h = 0x0;
        g_ske_hp_reg->dma_da_h = 0x0;
    }
    else {
        /*  g_ske_hp_reg->dma_sa_h = (((uint64_t)in)>>32); */
        /*  g_ske_hp_reg->dma_da_h = (((uint64_t)out)>>32); */
    }

    /* data word length */
    g_ske_hp_reg->dma_rlen = in_words * 32;
    g_ske_hp_reg->dma_wlen = out_words * 32;

    ske_hp_start();
    ret = ske_hp_calc_wait_till_done();

    if (SKE_SUCCESS != ret) {
        uint32_clear(out, out_words);
    }
    else
    {}

    return ret;
}

#if (defined(SUPPORT_SKE_MODE_CMAC) || defined(SUPPORT_SKE_MODE_CMAC))
/* function: basic ske_hp DMA operation without output(for CBC_MAC/CMAC)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     in_words ------------------- input, word length of in, must be multiples of block word length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. in_words must be a multiple of block words.
 */
uint32_t ske_hp_dma_operate_without_output(ske_ctx_t *ctx, uint32_t *in, uint32_t in_words)
{
    volatile uint32_t flag_0 = 0;

    /* src addr low 32bits */
    g_ske_hp_reg->dma_sa_l = (uint32_t)in;

    /* src addr high 32bits */
    if (4 == (sizeof(uint32_t *))) {
        /* in this case, if using (((uint64_t)in)>>32), you may get 0xFFFFFFFF, not 0 you expected! */
        g_ske_hp_reg->dma_sa_h = flag_0;
    }
    else {
        /* g_ske_hp_reg->dma_sa_h = (((uint64_t)in)>>32); */
    }

    /* data word length */
    g_ske_hp_reg->dma_rlen = in_words * 32;
    g_ske_hp_reg->dma_wlen = flag_0;

    ske_hp_start();
    return ske_hp_calc_wait_till_done();
}
#endif

#ifdef SKE_HP_DMA_LL_FUNCTION
void ske_hp_dma_ll_operate(uint32_t *in, uint32_t *out, uint32_t wordLen, dma_ll_node_t *llp)
{
    /* ll addr */
    g_ske_hp_reg->dma_llp_l = ((uint64_t)llp) & 0xFFFFFFFF;
    g_ske_hp_reg->dma_llp_h = ((uint64_t)llp) >> 32;

    /* src addr */
    g_ske_hp_reg->dma_sa_l = ((uint64_t)in) & 0xFFFFFFFF;
    g_ske_hp_reg->dma_sa_h = ((uint64_t)in) >> 32;

    /* dst addr */
    g_ske_hp_reg->dma_da_l = ((uint64_t)out) & 0xFFFFFFFF;
    g_ske_hp_reg->dma_da_h = ((uint64_t)out) >> 32;

    /* data word length */
    g_ske_hp_reg->dma_rlen = wordLen * 32;

    ske_hp_start();
    ske_hp_calc_wait_till_done();
}
#endif
#endif


/* function: update ske_hp some blocks without output
 * parameters:
 *     ctx ------------------------ input, ske_ctx_t context pointer
 *     in ------------------------- input, some blocks
 *     bytes ---------------------- input, byte length of in
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the bytes is a multiple of block byte length ctx->block_bytes
 *     2. this function is called by CCM(input aad)/GCM(input aad)/CMAC/CBC-MAC mode
 */
uint32_t ske_hp_update_blocks_no_output(ske_ctx_t *ctx, uint8_t *in, uint32_t bytes)
{
    uint32_t in_word_align, is_ccm_gcm_mode = 0;
    uint32_t tmp_in[4];
    uint32_t i;
    uint32_t ret;

    if (((uint32_t)in) & 3) {
        in_word_align = 0;
    }
    else {
        in_word_align = 1;
    }

    i = (g_ske_hp_reg->cfg & 0xF0000000) >> 28;

    switch (i) {
#if (defined(SUPPORT_SKE_MODE_GCM))

        case SKE_MODE_GCM:
#endif

#if (defined(SUPPORT_SKE_MODE_CCM))
        case SKE_MODE_CCM:
#endif

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
            is_ccm_gcm_mode = 1;
            break;
#endif

        default:
            is_ccm_gcm_mode = 0;
    }

    /* input one block ---> calculating ---> output one block */
    for (i = 0; i < bytes; i += ctx->block_bytes) {
        if (in_word_align) {
            ske_hp_simple_set_input_block((uint32_t *)in, ctx->block_words);
        }
        else {
            memcpy_(tmp_in, in, ctx->block_bytes);
            ske_hp_simple_set_input_block((uint32_t *)tmp_in, ctx->block_words);
        }

        ske_hp_start();

        if (is_ccm_gcm_mode) {
            ret = ske_hp_wait_till_input();

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else {
            ret = ske_hp_calc_wait_till_done();

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else {
                /* trigger to pop, this can not be deleted */
                g_ske_hp_reg->ctrl |= 0x2;
            }
        }

        in += ctx->block_bytes;
    }

    return SKE_SUCCESS;
}

/* function: update ske_hp some blocks and get the same number of blocks
 * parameters:
 *     ctx ------------------------ input, ske_ctx_t context pointer
 *     in ------------------------- input, some blocks
 *     out ------------------------ output, the same number of blocks
 *     bytes ---------------------- input, byte length of in
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the bytes is a multiple of block byte length ctx->block_bytes
 */
uint32_t ske_hp_update_blocks_internal(ske_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint8_t *out_bak = out;
    uint32_t in_word_align, out_word_align;
    uint32_t tmp_in[4];
    uint32_t i;
    uint32_t ret;

    if (((uint32_t)in) & 3) {
        in_word_align = 0;
    }
    else {
        in_word_align = 1;
    }

    if (((uint32_t)out) & 3) {
        out_word_align = 0;
    }
    else {
        out_word_align = 1;
    }

    /* input one block ---> calculating ---> output one block */
    for (i = 0; i < bytes; i += ctx->block_bytes) {
        if (in_word_align) {
            ske_hp_simple_set_input_block((uint32_t *)in, ctx->block_words);
        }
        else {
            memcpy_(tmp_in, in, ctx->block_bytes);
            ske_hp_simple_set_input_block((uint32_t *)tmp_in, ctx->block_words);
        }

        ske_hp_start();

        ret = ske_hp_wait_till_output();

        if (SKE_SUCCESS != ret) {
            if (out_word_align) {
                uint32_clear((uint32_t *)out_bak, bytes / 4);
            }
            else {
                memset_(out_bak, 0, bytes);
            }

            return ret;
        }
        else
        {;}

        if (out_word_align) {
            ske_hp_simple_get_output_block((uint32_t *)out, ctx->block_words);
        }
        else {
            ske_hp_simple_get_output_block((uint32_t *)tmp_in, ctx->block_words);
            memcpy_(out, tmp_in, ctx->block_bytes);
        }

        in += ctx->block_bytes;
        out += ctx->block_bytes;
    }

    return SKE_SUCCESS;
}

