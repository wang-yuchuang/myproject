/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske_xts.h>
#ifdef SUPPORT_SKE_MODE_XTS

/* function: ske xts mode init config
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it can not be less than
 *                                  block byte length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX], actually, sp_key_idx
 *        is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
uint32_t ske_hp_xts_init(ske_xts_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                         uint8_t *i, uint32_t c_bytes)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (c_bytes < 16) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ctx->c_bytes = c_bytes;
    ske_hp_set_c_len_uint32(c_bytes);

    ctx->current_bytes = 0;

    ske_hp_set_cpu_mode();

    return ske_hp_init_internal(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, crypto, key, sp_key_idx, i);
}

/* function: ske xts mode encryption or decryption
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 *     3. if plaintext/ciphertext is too long, you could divide it by block(16 bytes), then call
 *        this function to input the sections respectively. but if ctx->c_bytes is not a multiple of
 *        block byte length, the input in could not contain the last two blocks of the whole input,
 *        in this case , the last 2 blocks(actually the last block is not full) are left to
 *        function ske_hp_xts_update_including_last_2_blocks().
 */
uint32_t ske_hp_xts_update_blocks(ske_xts_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ret;

    if ((NULL == ctx) || (NULL == in) || (NULL == out)) {
        return SKE_BUFFER_NULL;
    }
    else if (bytes & (ctx->ske_xts_ctx->block_bytes - 1)) {
        return SKE_INPUT_INVALID;
    }
    else if (ctx->current_bytes & (ctx->ske_xts_ctx->block_bytes - 1)) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    if (ctx->c_bytes & 0x0F) {
        if (ctx->c_bytes - 16 - (ctx->c_bytes & 0x0F) < ctx->current_bytes + bytes) {
            return SKE_INPUT_INVALID;
        }
    }
    else {
        if (ctx->c_bytes < ctx->current_bytes + bytes) {
            return SKE_INPUT_INVALID;
        }
    }

    ret = ske_hp_update_blocks_internal(ctx->ske_xts_ctx, in, out, bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ctx->current_bytes += bytes;

    return SKE_SUCCESS;
}

/* function: ske xts mode encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks, actualy, this function is for the case that ctx->c_bytes % 16
 *        is not 0.
 */
uint32_t ske_hp_xts_update_including_last_2_blocks(ske_xts_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t blocks_bytes;
    uint32_t buf[4];
    uint32_t ret;

    if (NULL == ctx || NULL == in || NULL == out) {
        return SKE_BUFFER_NULL;
    }
    else if (bytes <= ctx->ske_xts_ctx->block_bytes || !(bytes & 0x0F)) {
        return SKE_INPUT_INVALID;
    }
    else if (ctx->current_bytes & (ctx->ske_xts_ctx->block_bytes - 1) || ctx->current_bytes + bytes != ctx->c_bytes) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    /*process blocks*/
    blocks_bytes = (bytes & (~0x0F)) - ctx->ske_xts_ctx->block_bytes;

    if (blocks_bytes) {
        ret = ske_hp_xts_update_blocks(ctx, in, out, blocks_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }

        in += blocks_bytes;
        out += blocks_bytes;
        bytes -= blocks_bytes;
    }

    /*process remainder 2 blocks*/
    memcpy_(buf, in, 16);
    ske_hp_simple_set_input_block((uint32_t *)buf, ctx->ske_xts_ctx->block_words);
    ske_hp_start();

    ske_hp_set_last_block(1);

    memcpy_(buf, in + 16, bytes - 16);
    memset_(((uint8_t *)(buf)) + bytes - 16, 0, 32 - bytes);
    ske_hp_simple_set_input_block((uint32_t *)buf, ctx->ske_xts_ctx->block_words);

    ske_hp_start();
    ret = ske_hp_wait_till_output();

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ske_hp_simple_get_output_block((uint32_t *)buf, ctx->ske_xts_ctx->block_words);
    memcpy_(out, buf, 16);

    ske_hp_simple_get_output_block((uint32_t *)buf, ctx->ske_xts_ctx->block_words);
    memcpy_(out + 16, buf, bytes - 16);

    return SKE_SUCCESS;
}

/* function: ske xts mode finish
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is the last step of xts calling, and it is optional
 */
uint32_t ske_hp_xts_final(ske_xts_ctx_t *ctx)
{
    memset_(ctx, 0, sizeof(ske_ctx_t));

    return SKE_SUCCESS;
}

/* function: ske xts mode encrypting/decrypting
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as blcok length
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, actual byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX], actually, sp_key_idx
 *        is reserved at present, please input key directly.
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
uint32_t ske_hp_xts_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *i,
                           uint8_t *in, uint8_t *out, uint32_t c_bytes)
{
    ske_xts_ctx_t ctx[1];
    uint32_t ret;

    ret = ske_hp_xts_init(ctx, alg, crypto, key, sp_key_idx, i, c_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (c_bytes & 0x0F) {
        ret = ske_hp_xts_update_including_last_2_blocks(ctx, in, out, c_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
    }
    else {
        ret = ske_hp_xts_update_blocks(ctx, in, out, c_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
    }

    return ske_hp_xts_final(ctx);
}

#ifdef SKE_HP_DMA_FUNCTION
/* function: ske xts mode dma style init config
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it can not be less than
 *                                  block byte length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX], actually, sp_key_idx
 *        is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
uint32_t ske_hp_dma_xts_init(ske_xts_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                             uint8_t *i, uint32_t c_bytes)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (c_bytes < 16) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ctx->c_bytes = c_bytes;
    ske_hp_set_c_len_uint32(c_bytes);

    ctx->current_bytes = 0;

    ske_hp_set_dma_mode();
    ske_hp_disable_dma_linked_list();

    return ske_hp_init_internal(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, crypto, key, sp_key_idx, i);
}

/* function: ske xts mode dma style encryption or decryption
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     words ---------------------- input, word length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. words must be a multiple of block word length.
 *     3. if plaintext/ciphertext is too long, you could divide it by block(16 bytes), then call
 *        this function to input the sections respectively. but if ctx->c_bytes is not a multiple of
 *        block byte length, the input in could not contain the last two blocks of the whole input,
 *        in this case , the last 2 blocks(actually the last block is with padding 0) are left to
 *        function ske_hp_dma_xts_update_including_last_2_blocks().
 */
uint32_t ske_hp_dma_xts_update_blocks(ske_xts_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t words)
{
    uint32_t ret;

    if ((NULL == ctx) || (NULL == in) || (NULL == out)) {
        return SKE_BUFFER_NULL;
    }
    else if (0 == words) {
        return SKE_SUCCESS;
    }
    else if (words & (ctx->ske_xts_ctx->block_words - 1)) {
        return SKE_INPUT_INVALID;
    }
    else if (ctx->current_bytes & (ctx->ske_xts_ctx->block_bytes - 1)) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    if (ctx->c_bytes & 0x0F) {
        if (ctx->c_bytes - 16 - (ctx->c_bytes & 0x0F) < ctx->current_bytes + (words << 2)) {
            return SKE_INPUT_INVALID;
        }
    }
    else {
        if (ctx->c_bytes < ctx->current_bytes + (words << 2)) {
            return SKE_INPUT_INVALID;
        }
    }

    ret = ske_hp_dma_operate(ctx->ske_xts_ctx, in, out, words, words);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else {
        ctx->current_bytes += (words << 2);

        return SKE_SUCCESS;
    }
}

/* function: ske xts mode dma style encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks(the last block is with padding 0), actualy, this function
 *        is for the case that ctx->c_bytes % 16 is not 0.
 */
uint32_t ske_hp_dma_xts_update_including_last_2_blocks(ske_xts_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t bytes)
{
    uint32_t i, tmp_len;
    uint32_t ret;

    if (NULL == ctx || NULL == in || NULL == out) {
        return SKE_BUFFER_NULL;
    }
    else if (bytes <= ctx->ske_xts_ctx->block_bytes || !(bytes & 0x0F)) {
        return SKE_INPUT_INVALID;
    }
    else if (ctx->current_bytes & (ctx->ske_xts_ctx->block_bytes - 1) || ctx->current_bytes + bytes != ctx->c_bytes) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ske_hp_set_last_block(1);

    ret = ske_hp_dma_operate(ctx->ske_xts_ctx, in, out, (bytes + 15) / 16 * 4, (bytes + 15) / 16 * 4);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    /*clear useless data*/
    i = (bytes) / 4;
    tmp_len = bytes & 0x03;

    if (tmp_len) {
#ifdef SKE_HP_CPU_BIG_ENDIAN
        out[i] &= (1 << (32 - tmp_len * 8)) - 1;
#else
        out[i] &= (1 << (tmp_len * 8)) - 1;
#endif
        i++;
    }

    while (i < (bytes + 15) / 16 * 4) {
        out[i++] = 0;
    }

    return SKE_SUCCESS;
}

/* function: ske xts mode dma style finish
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_t context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is the last step of xts dma style calling, and it is optional
 */
uint32_t ske_hp_dma_xts_final(ske_xts_ctx_t *ctx)
{
    return ske_hp_xts_final(ctx);
}

/* function: ske xts mode dma style encrypting/decrypting
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, actual byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX], actually, sp_key_idx
 *        is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
uint32_t ske_hp_dma_xts_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *i,
                               uint32_t *in, uint32_t *out, uint32_t c_bytes)
{
    ske_xts_ctx_t ctx[1];
    uint32_t ret;

    ret = ske_hp_dma_xts_init(ctx, alg, crypto, key, sp_key_idx, i, c_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (c_bytes & 0x0F) {
        ret = ske_hp_dma_xts_update_including_last_2_blocks(ctx, in, out, c_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
    }
    else {
        ret = ske_hp_dma_xts_update_blocks(ctx, in, out, c_bytes / 4);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
    }

    return ske_hp_xts_final(ctx);
}

#endif

#endif

