/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hash.h>
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hmac.h>

/* function: init HMAC
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_t context pointer
 *     key_flag ------------------- input, whether key byte length bigger than block byte length
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash_alg is valid
 */
static void hmac_key_state_recover(hmac_ctx_t *ctx, uint32_t key_flag)
{
    hash_set_cpu_mode();
    hash_set_hmac_mode();
    hash_set_endian_uint32();
    hash_disable_cpu_interruption();
    hash_set_alg(ctx->hash_alg);
    hash_set_hmac_key_mode();
    hash_update_config();

    if (1 == key_flag) {
        hash_set_IV(ctx->hash_alg, ctx->hash_ctx->iterator_word_len);
        hash_hmac_key_opr_one_block(ctx->K0, ctx->hash_ctx->block_byte_len);
    }
    else if (2 == key_flag) {
        hash_set_iterator(ctx->K0, ctx->hash_ctx->iterator_word_len);
        hash_set_hmac_key_len(0);
        hash_set_hmac_key_cnt(0);
        hash_set_last_block(1);
        hash_start();
        hash_wait_till_done();
    }
    else
    {;}

    hash_clear_hmac_key_mode();
}

/* function: init HMAC
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_t context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint32_t hmac_init(hmac_ctx_t *ctx, HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes)
{
    if (NULL == ctx) {
        return HASH_BUFFER_NULL;
    }
    else if (HASH_SUCCESS != check_hash_alg(hash_alg)) {
        return HASH_INPUT_INVALID;
    }
    else if (key_bytes & 0xE0000000) {
        /*bit length overflow*/
        return HASH_INPUT_INVALID;
    }
    else if (NULL == key) {
#ifdef HMAC_SECURE_PORT_FUNCTION
        if ((0 == sp_key_idx) || (sp_key_idx > HMAC_MAX_KEY_IDX) || (0 == key_bytes) || (key_bytes > HMAC_MAX_SP_KEY_SIZE)) {
            return HASH_INPUT_INVALID;
        }
        else if ((sp_key_idx == HMAC_MAX_KEY_IDX) && (key_bytes > HMAC_MAX_SP_KEY_SIZE / 2)) {
            return HASH_INPUT_INVALID;
        }
        else
        {;}
#else
        key_bytes = 0;
#endif
    }
    else
    {;}

    /*set hash_ctx config*/
    memset_(ctx, 0, sizeof(hmac_ctx_t));

    if (key) {
        /*key is from user input*/
        hash_hmac_disable_secure_port();
    }
    else {
        /*key is from secure port*/
        hash_hmac_enable_secure_port(sp_key_idx);
        hash_hmac_enable_secure_port(sp_key_idx + 1);
    }

    hash_set_cpu_mode();
    hash_set_hmac_mode();
    hash_set_hmac_key_mode();
    hash_set_endian_uint32();
    hash_disable_cpu_interruption();
    hash_set_alg(hash_alg);
    hash_update_config();

    ctx->hash_alg = hash_alg;
    ctx->hash_ctx->hash_alg = hash_alg;
    ctx->hash_ctx->block_byte_len = hash_get_block_word_len(hash_alg) << 2;
    ctx->hash_ctx->iterator_word_len = hash_get_iterator_word_len(hash_alg);
    ctx->hash_ctx->digest_byte_len = hash_get_digest_word_len(hash_alg) << 2;
    ctx->hash_ctx->status.busy = 0;
    ctx->hash_ctx->first_update_flag = 1;
    ctx->hash_ctx->finish_flag = 0;

    if (key) {
        /*key is from user input*/
        hash_set_IV(hash_alg, ctx->hash_ctx->iterator_word_len);

        /*get K0*/
        if (key_bytes <= ctx->hash_ctx->block_byte_len) {
            /*K0 = (key)||000..00*/
            ctx->key_flag = 1;
            memcpy_((uint8_t *)(ctx->K0), key, key_bytes);
            memset_(((uint8_t *)(ctx->K0)) + key_bytes, 0, ctx->hash_ctx->block_byte_len - key_bytes);
            hash_hmac_key_opr_one_block(ctx->K0, ctx->hash_ctx->block_byte_len);
        }
        else {
            ctx->key_flag = 2;
            hash_hmac_key_opr(key, key_bytes);
            hash_get_iterator(((uint8_t *)(ctx->K0)), ctx->hash_ctx->digest_byte_len >> 2);
            memset_(((uint8_t *)(ctx->K0)) + ctx->hash_ctx->digest_byte_len, 0, ctx->hash_ctx->block_byte_len - ctx->hash_ctx->digest_byte_len);
        }
    }
    else {
        /*key is from secure port*/
        hash_hmac_sp_key_opr(key_bytes << 3);
    }

    hash_clear_hmac_key_mode();
    return HASH_SUCCESS;
}


/* function: hash iterate calc with some blocks
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     msg ------------------------ input, message of some blocks
 *     block_count ---------------- input, count of blocks
 * return: none
 * caution:
 *     1. please make sure the three parameters is valid
 */
static void hash_hmac_calc_blocks(hash_ctx_t *ctx, const uint8_t *msg, uint32_t block_count)
{
    uint8_t block_word_len = (ctx->block_byte_len) >> 2;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    /*set the input iterator data*/
    if (1 != ctx->first_update_flag) {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
    else
    {;}

#endif

    /*set the bit length of the input blocks*/
    hash_set_tx_bit_len(ctx, block_count);

    hash_set_last_block(0);

    ctx->first_update_flag = 0;

    hash_start_calculate(ctx);

    while (block_count--) {
        /*input the block message*/
        hash_input_msg((uint8_t *)msg, block_word_len);
        msg += ctx->block_byte_len;
    }

    hash_wait_till_done();

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    /*if message update not done, get the new iterator hash value*/
    if (1 != ctx->finish_flag) {
        hash_get_iterator((uint8_t *)(ctx->iterator), ctx->iterator_word_len);
    }
    else
    {;}

#endif
}


/* function: hmac update message
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_update(hmac_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t count;
    uint8_t left, fill;

    if (NULL == ctx) {
        return HASH_BUFFER_NULL;
    }
    else if ((NULL == msg) || (0 == msg_bytes)) {
        return HASH_SUCCESS;
    }
    else
    {;}
    /*start to update processing*/
    ctx->hash_ctx->status.busy = 1;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    hmac_key_state_recover(ctx, ctx->key_flag);
#endif
    /*byte length of valid message left in block buffer*/
    /*byte length that block buffer need to fill a block*/
    left = ctx->hash_ctx->total[0] % (ctx->hash_ctx->block_byte_len);
    fill = (ctx->hash_ctx->block_byte_len) - left;

    /*update total byte length*/
    if (hash_total_len_add_uint32(ctx->hash_ctx->total, ctx->hash_ctx->block_byte_len / 32, msg_bytes)) {
        return HASH_LEN_OVERFLOW;
    }
    else
    {;}

    if (left) {
        if (msg_bytes >= fill) {
            memcpy_(ctx->hash_ctx->hash_buffer + left, (uint8_t *)msg, fill);
            hash_hmac_calc_blocks(ctx->hash_ctx, ctx->hash_ctx->hash_buffer, 1);
            msg_bytes -= fill;
            msg += fill;
        }
        else {
            memcpy_(ctx->hash_ctx->hash_buffer + left, (uint8_t *)msg, msg_bytes);
            goto end;
        }
    }
    else
    {;}

    /*process some blocks*/
    count = msg_bytes / (ctx->hash_ctx->block_byte_len);

    if (count) {
        hash_hmac_calc_blocks(ctx->hash_ctx, msg, count);
    }
    else
    {;}

    /*process the remainder*/
    msg += (ctx->hash_ctx->block_byte_len) * count;
    msg_bytes = msg_bytes % (ctx->hash_ctx->block_byte_len);

    if (msg_bytes) {
        memcpy_(ctx->hash_ctx->hash_buffer, (uint8_t *)msg, msg_bytes);
    }
    else
    {;}

end:
    /*update end, status becomes idle*/
    ctx->hash_ctx->status.busy = 0;

    return HASH_SUCCESS;
}


/* function: message update done, get the mac
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_t context pointer
 *     mac ------------------------ output, mac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the mac buffer is sufficient
 */
uint32_t hmac_final(hmac_ctx_t *ctx, uint8_t *mac)
{
    uint8_t tmp;

    if (NULL == ctx || NULL == mac) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    hmac_key_state_recover(ctx, ctx->key_flag);

    if (0 == ctx->hash_ctx->first_update_flag) {
        hash_set_iterator((uint8_t *)(ctx->hash_ctx->iterator), ctx->hash_ctx->iterator_word_len);
    }
    /*set not the last block*/
    hash_set_last_block(0);
#endif
    /*the last block calc*/
    ctx->hash_ctx->finish_flag = 1;

    /*get the byte length of the remainder msg(less than one block)*/
    tmp = ctx->hash_ctx->total[0] % (ctx->hash_ctx->block_byte_len);

    /*set total msg bit length*/
    hash_total_bytelen_2_bitlen(ctx->hash_ctx->total, (ctx->hash_ctx->block_byte_len) / 32);

    hash_set_msg_total_bit_len(ctx->hash_ctx->total, ctx->hash_ctx->block_byte_len);

    hash_set_last_block(1);

    hash_start();

    hash_input_msg((uint8_t *)ctx->hash_ctx->hash_buffer, (tmp + 3) / 4);

    hash_wait_till_done();

    /*get the hash result*/
    hash_get_iterator(mac, (ctx->hash_ctx->digest_byte_len) >> 2);

    memset_(ctx, 0, sizeof(hmac_ctx_t));

    return HASH_SUCCESS;
}


/* function: input key and whole message, get the mac
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of the key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, mac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the mac buffer is sufficient
 */
uint32_t hmac(HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint8_t *msg, uint32_t msg_bytes, uint8_t *mac)
{
    hmac_ctx_t ctx[1];
    uint32_t ret;

    ret = hmac_init(ctx, hash_alg, key, sp_key_idx, key_bytes);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hmac_update(ctx, msg, msg_bytes);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return hmac_final(ctx, mac);
}


#ifdef HASH_DMA_FUNCTION
/* function: init dma hmac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_t context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_dma_init(hmac_dma_ctx_t *ctx, HASH_ALG hash_alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes,
                       HASH_CALLBACK callback)
{
    uint32_t ret;

    if (NULL == ctx || NULL == callback) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

    ret = hmac_init(ctx->hmac_ctx, hash_alg, (uint8_t *)key, sp_key_idx, key_bytes);

    if (HASH_SUCCESS == ret) {
        hash_clear_dma_sa_da();

        hash_set_dma_wlen(hash_get_digest_word_len(hash_alg) << 5);

        hash_set_dma_mode();
        hash_set_last_block(0);

        ctx->hash_dma_ctx->block_word_len = hash_get_block_word_len(hash_alg);
        ctx->hash_dma_ctx->callback = callback;
        uint32_clear(ctx->hash_dma_ctx->total, ctx->hash_dma_ctx->block_word_len / 8);
    }
    else
    {;}

    return ret;
}


/* function: dma hmac update message
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_words ------------------ input, word length of the input message, must be multiple of block word length of HASH
 *     tmp_iterator --------------- output, temporary hash iterator
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t hmac_dma_update_blocks(hmac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *tmp_iterator)
{
    if (NULL == ctx) {
        return HASH_BUFFER_NULL;
    }
    else {
        return hash_dma_update_blocks(ctx->hash_dma_ctx, msg, msg_words, tmp_iterator);
    }
}


/* function: dma hmac message update done, get the mac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_t context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the last message, must be in [0, BLOCK_BYTE_LEN-1],
 *                                  here BLOCK_BYTE_LEN is block byte length of HASH
 *     tmp_iterator --------------- output, temporary hash iterator
 *     mac ------------------------ output, mac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_dma_final(hmac_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *tmp_iterator, uint8_t *mac)
{
    uint32_t ret;

    if ((NULL == ctx) || (NULL == tmp_iterator) || (NULL == mac)) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

    ret = hash_dma_final(ctx->hash_dma_ctx, remainder_msg, remainder_bytes, tmp_iterator);

    if (HASH_SUCCESS == ret) {
        memcpy_(mac, (uint8_t *)tmp_iterator, ctx->hmac_ctx->hash_ctx->digest_byte_len);
    }

    return ret;
}


/* function: dma hmac input key and message, get the mac
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     tmp_iterator --------------- output, for temporary hash iterator or digest(DMA)
 *     mac ------------------------ output, mac
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_dma(HASH_ALG hash_alg, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, uint32_t msg_bytes,
                  uint32_t *tmp_iterator, uint8_t *mac, HASH_CALLBACK callback)
{
    uint32_t blocks_words, remainder_bytes;
    uint32_t ret;
    hmac_dma_ctx_t ctx[1];

    ret = hmac_dma_init(ctx, hash_alg, key, sp_key_idx, key_bytes, callback);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    remainder_bytes = msg_bytes % ctx->hmac_ctx->hash_ctx->block_byte_len;
    blocks_words = (msg_bytes - remainder_bytes) / 4;
    ret = hash_dma_update_blocks(ctx->hash_dma_ctx, msg, blocks_words, tmp_iterator);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return hmac_dma_final(ctx, (uint32_t *)(msg + blocks_words), remainder_bytes, tmp_iterator, mac);
}
#endif

