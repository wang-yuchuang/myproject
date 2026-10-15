/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include "hash_basic.h"

/* hash register pointer */
volatile static hash_reg_t *const g_hash_reg = (hash_reg_t *)HASH_BASE_ADDR;

/* function: get hash IP version
 * parameters: none
 * return: hash IP version
 * caution:
 */
uint32_t hash_get_version(void)
{
    return g_hash_reg->hash_version;
}

/* function: set hash to be CPU mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_cpu_mode(void)
{
    g_hash_reg->hash_cfg &= (~((0x1) << HASH_DMA_OFFSET));
}

/* function: set hash to be DMA mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_dma_mode(void)
{
    g_hash_reg->hash_cfg |= ((0x1) << HASH_DMA_OFFSET);
}

/* function: set hash mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hash_mode(void)
{
    g_hash_reg->hash_cfg &= (~(((uint32_t)1) << HASH_HMAC_OFFSET));
}

/* function: set hmac mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hmac_mode(void)
{
    g_hash_reg->hash_cfg |= (((uint32_t)1) << HASH_HMAC_OFFSET);
}

/* function: set hmac key mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hmac_key_mode(void)
{
    g_hash_reg->hash_mdin_cr |= 0x1;
}

/* function: clear hmac key mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_clear_hmac_key_mode(void)
{
    g_hash_reg->hash_mdin_cr &= ~0x1;
}

/* function: clear hash risp regist
 * parameters: none
 * return: none
 * caution:
 */
void hash_clear_risp(void)
{
    g_hash_reg->hash_risr &= ~0x3;
}

/* function: set opreated hmac key bitlen to regist
 * parameters:
 *     cnt ------------------- input, operated key bitlen
 * return: none
 * caution:none
 */
void hash_set_hmac_key_cnt(uint32_t bitlen)
{
    g_hash_reg->hash_key_cnt = bitlen;
}

/* function: set total hmac key bitlen to regist
 * parameters:
 *     bitlen ------------------- input, key bitlen
 * return: none
 * caution:none
 */
void hash_set_hmac_key_len(uint32_t bitlen)
{
    g_hash_reg->hash_key_len = bitlen;
}

/* function: hmac key opr for one block
 * parameters:none
 * return: none
 * caution:none
 */
void hash_hmac_key_opr_one_block(uint32_t *key, uint32_t block_byte_len)
{
    uint32_t i;
    uint32_t block_words_len;

    block_words_len = block_byte_len >> 2;

    hash_set_hmac_key_len(block_byte_len << 3);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);
    hash_start();

    for (i = 0; i < block_words_len; i++) {
        g_hash_reg->hash_m_din = key[i];
    }

    hash_wait_till_done();
}

/* function: hmac key opr
 * parameters:none
 * return: none
 * caution:none
 */
void hash_hmac_key_opr(uint8_t *key, uint32_t key_bytes)
{
    hash_set_hmac_key_len(key_bytes << 3);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);
    hash_start();
    hash_input_msg_u8(key, key_bytes);
    hash_wait_till_done();
}

/* function: hmac secure port key opr for one block
 * parameters:none
 * return: none
 * caution:none
 */
void hash_hmac_sp_key_opr(uint32_t key_bits)
{
    hash_set_hmac_key_len(key_bits);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);
    hash_start();
    hash_wait_till_done();
}

/* function: set the specific hash algorithm
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: none
 * caution:
 *     1. please make sure hash_alg is valid
 */
void hash_set_alg(HASH_ALG hash_alg)
{
    g_hash_reg->hash_cfg &= (~0x0000000F);
    g_hash_reg->hash_cfg |= hash_alg;
}

/* function: check whether hash_alg is SHA3
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: 1(yes), 0(no)
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint32_t hash_check_whether_sha3_alg(HASH_ALG hash_alg)
{
    if(hash_alg < HASH_SHA3_224) {
        return 0;
    }
    else if(hash_alg <= HASH_SHA3_512) {
        return 1;
    }
    else {
        return 0;
    }
}

/* function: update hash config
 * parameters:none
 * return: none
 * caution: none
 */
void hash_update_config(void)
{
    g_hash_reg->hash_cfg |= (0x00001000);
    g_hash_reg->hash_ctrl |= 0x1;
    hash_wait_till_done();
    g_hash_reg->hash_cfg &= (~0x00001000);
}

/* function: enable hash interruption in CPU mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_enable_cpu_interruption(void)
{
    g_hash_reg->hash_imcr |= 0x1;
}

/* function: disable hash interruption in CPU mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_disable_cpu_interruption(void)
{
    g_hash_reg->hash_imcr &= (~0x1);
}

/* function: enable hash interruption in DMA mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_enable_dma_interruption(void)
{
    g_hash_reg->hash_imcr |= ((0x1) << 1);
}

/* function: disable hash interruption in DMA mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_disable_dma_interruption(void)
{
    g_hash_reg->hash_imcr &= (~((0x1) << 1));
}

/* function: set dma wlen
 * parameters:
 *     wlen ------------------------ input, byte length of write back data
 * return: none
 * caution: none
 */
void hash_set_dma_wlen(uint32_t wlen)
{
    g_hash_reg->dma_wlen = wlen;
}

/* function: clear dma_sa and dma_da
 * parameters:none
 * return: none
 * caution: none
 */
void hash_clear_dma_sa_da(void)
{
    g_hash_reg->dma_sa_l = 0x0;
    g_hash_reg->dma_sa_h = 0x0;

    g_hash_reg->dma_da_l = 0x0;
    g_hash_reg->dma_da_h = 0x0;
}

/* function: set the tag whether current block is the last message block or not
 * parameters:
 *     tag ------------------------ input, 0(no), other(yes)
 * return: none
 * caution:
 *     1. if it is the last block, please config hash_reg->hash_msg_len,
 *        then the hardware will do the padding and post-processing.
 */
void hash_set_last_block(uint8_t tag)
{
    if (tag) {  /* current block is the last one of the message */
        g_hash_reg->hash_mdin_cr |= (((uint32_t)1) << HASH_LAST_BLOCK_OFFSET);
    }
    else {      /* current block is not the last one of the message */
        g_hash_reg->hash_mdin_cr &= (~(((uint32_t)1) << HASH_LAST_BLOCK_OFFSET));
    }
}

/* function: set the hash endian
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_endian_uint32(void)
{
    /* clear bit[9:8], and now CPU is big-endian */
    g_hash_reg->hash_cfg &= (~(((uint32_t)3) << HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET));

#ifndef HASH_CPU_BIG_ENDIAN
    /* CPU is little-endian, input and output reversed by hardware----hash IP */
    g_hash_reg->hash_cfg |= ((((uint32_t)2) << HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET));
#endif
}

/* function: enable hmac secure port
 * parameters:
 *     sp_key_idx ----------------- input, index of secure port key
 * return: none
 * caution:
 */
void hash_hmac_enable_secure_port(uint16_t sp_key_idx)
{
#ifdef HMAC_SECURE_PORT_FUNCTION

    g_hash_reg->hash_cfg |= ((0x1) << 5);

    sp_key_idx -= 1;
    OTP_KEY_CTRL = ((uint32_t)(sp_key_idx & 0x7FFF)) << 16;

    OTP_KEY_CTRL |= 0x1;

#endif
}

/* function: disable hmac secure port
 * parameters:
 * return: none
 * caution:
 */
void hash_hmac_disable_secure_port(void)
{
    g_hash_reg->hash_cfg &= (~(((uint32_t)1) << 5));
}

/* function: get current HASH iterator value
 * parameters:
 *     iterator ------------------- output, current hash iterator
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 *     1.
 */
void hash_get_iterator(uint8_t *iterator, uint8_t hash_iterator_words)
{
    uint32_t temp;
    uint32_t i;

    /* for the case that iterator is not aligned by word */
    if (((uint32_t)iterator) & 3) {
        for (i = 0; i < hash_iterator_words; i++) {
            temp = g_hash_reg->hash_out[i];
            memcpy_(iterator + (i << 2), &temp, 4);
        }
    }
    else {
        for (i = 0; i < hash_iterator_words; i++) {
            ((uint32_t *)iterator)[i] = g_hash_reg->hash_out[i];
        }
    }
}

/* function: input current iterator value
 * parameters:
 *     iterator ------------------- input, hash iterator value
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 *     1. iterator must be word aligned
 */
void hash_set_iterator(uint32_t *iterator, uint8_t hash_iterator_words)
{
    uint32_t i;

    if(iterator != NULL) {
        for (i = 0; i < hash_iterator_words; i++) {
            g_hash_reg->hash_in[i] = iterator[i];
        }
    }
    else {
        /*for SHA3 init, iv is zero of 1600 bits*/
        for (i = 0; i < hash_iterator_words; i++) {
            g_hash_reg->hash_in[i] = 0;
        }
    }
}

/* function: set hash_msg_len and hash_msg_cnt
 * parameters:
 *     bytelen ------------------- input, msg byte length
 * return: none
 * caution:none
 */
void hash_set_msg_len(uint32_t bytelen)
{
    volatile uint32_t flag = 0;

    g_hash_reg->hash_msg_len[0] = bytelen << 3;
    g_hash_reg->hash_msg_len[1] = bytelen >> (32 - 3);
    g_hash_reg->hash_msg_len[2] = flag;
    g_hash_reg->hash_msg_len[3] = flag;

    g_hash_reg->hash_msg_cnt[0] = flag;
    g_hash_reg->hash_msg_cnt[1] = flag;
    g_hash_reg->hash_msg_cnt[2] = flag;
    g_hash_reg->hash_msg_cnt[3] = flag;
}

/* function: clear hash_msg_len and hash_msg_cnt
 * parameters:none
 * return: none
 * caution:none
 */
void hash_clear_msg_len(void)
{
    volatile uint32_t flag = 0;

    g_hash_reg->hash_msg_len[0] = flag;
    g_hash_reg->hash_msg_len[1] = flag;
    g_hash_reg->hash_msg_len[2] = flag;
    g_hash_reg->hash_msg_len[3] = flag;

    g_hash_reg->hash_msg_cnt[0] = flag;
    g_hash_reg->hash_msg_cnt[1] = flag;
    g_hash_reg->hash_msg_cnt[2] = flag;
    g_hash_reg->hash_msg_cnt[3] = flag;
}

/* function: set the total bit length of the whole message
 * parameters:
 *     msg_total_bits ------------- input, total bit length of the whole message
 *     words ---------------------- input, word length of array msg_total_bits
 * return: none
 * caution:
 *     1.
 */
void hash_set_msg_total_bit_len(uint32_t *msg_total_bits, uint32_t block_byte_len)
{
    uint32_t words = HASH_BLOCK_MAX_WORD_LEN / 8;

    while (words--) {
        g_hash_reg->hash_msg_len[words] = msg_total_bits[words];
        g_hash_reg->hash_msg_cnt[words] = msg_total_bits[words];
    }

    if (block_byte_len == 64) {
        g_hash_reg->hash_msg_cnt[0] &= 0xFFFFFE00;
    }
    else {
        g_hash_reg->hash_msg_cnt[0] &= 0xFFFFFC00;
    }
}

/* function: start HASH iteration calc
 * parameters: none
 * return: none
 * caution:
 */
void hash_start(void)
{
    g_hash_reg->hash_risr = 0x0;
    g_hash_reg->hash_ctrl |= 0x1;
}

/* function: wait till done
 * parameters: none
 * return: none
 * caution:
 */
void hash_wait_till_done(void)
{
    while ((g_hash_reg->hash_ctrl & 0x1))
    {;}
}

/* function: DMA wait till done
 * parameters:
 *     callback ------------------- callback function pointer
 * return: none
 * caution:
 */
void hash_dma_wait_till_done(HASH_CALLBACK callback)
{
    while ((g_hash_reg->hash_ctrl & 0x1)) {

        if (callback) {
            callback();
        }
        else
        {;}
    }
}

/* function: input message
 * parameters:
 *     msg ------------------------ input, message
 *     msg_words ------------------ input, word length of msg
 * return: none
 * caution:
 *     1. if msg does not contain the last block, please make sure msg_words is a multiple of the
 *        hash block word length.
 */
void hash_input_msg(uint8_t *msg, uint32_t msg_words)
{
    uint32_t tmp;

    if (((uint32_t)msg) & 3) {
        while (msg_words--) {
            memcpy_(&tmp, msg, 4);
            g_hash_reg->hash_m_din = tmp;
            msg += 4;
        }
    }
    else {
        while (msg_words--) {
            g_hash_reg->hash_m_din = *((uint32_t *)msg);
            msg += 4;
        }
    }
}

/* function: input message
 * parameters:
 *     msg ------------------------ input, message
 *     msg_words ------------------ input, byte length of msg
 * return: none
 * caution:
 *     1. if msg does not contain the last block, please make sure msg_words is a multiple of the
 *        hash block word length.
 */
void hash_input_msg_u8(uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t tmp1, tmp2;
    hash_input_msg(msg, msg_bytes >> 2);
    tmp1 = msg_bytes & 0x00000003;

    if (0 != tmp1) {
        tmp2 = 0;
        memcpy_((uint8_t *)&tmp2, msg + (msg_bytes & 0xFFFFFFFC), tmp1);
        hash_input_msg((uint8_t *)&tmp2, 1);
    }
    else
    {;}
}

#ifdef HASH_DMA_FUNCTION

/* function: basic HASH DMA operation
 * parameters:
 *     msg ------------------------ input, message of some blocks, or message including the last byte(last block)
 *     iterator ------------------- output, temporary iterator or hash digest
 *     msgBitLen ------------------ input, actual bit length of msg
 *     callback ------------------- callback function pointer
 * return: none
 * caution:
 *     1. for DMA operation, the unit of input and output is 4 words, so, please make sure the buffer
 *        iterator is sufficient.
 */
void hash_dma_operate(uint32_t *msg, uint32_t *iterator, uint32_t msgBitLen, HASH_CALLBACK callback)
{
    g_hash_reg->dma_sa_l = (uint32_t)msg;
    g_hash_reg->dma_sa_h = 0x0;

    g_hash_reg->dma_da_l = (uint32_t)iterator;
    g_hash_reg->dma_da_h = 0x0;

    g_hash_reg->dma_rlen = msgBitLen;

    hash_start();
    hash_dma_wait_till_done(callback);
}
#endif
