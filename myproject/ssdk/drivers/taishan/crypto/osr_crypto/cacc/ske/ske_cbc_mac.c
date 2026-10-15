/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <stdio.h>
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske_cbc_mac.h>

#ifdef SUPPORT_SKE_MODE_CBC_MAC

/* function: ske_hp cbc mac internal init config
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_cbc_mac_init_internal(ske_cbc_mac_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx)
{
    uint32_t iv[4];

    uint32_clear(iv, 4);

    return ske_hp_init_internal(ctx->ske_cbc_mac_ctx, alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, (uint8_t *)iv);
}

/* function: ske_hp cbc mac init(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     padding -------------------- input, ske_hp cbc mac padding scheme, like SKE_NO_PADDING,SKE_ZERO_PADDING.
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_cbc_mac_init(ske_cbc_mac_ctx_t *ctx, SKE_ALG alg, SKE_PADDING padding, uint8_t *key, uint16_t sp_key_idx)
{
    /*check and keep the padding scheme and ctx->left_bytes = 0*/
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (padding > SKE_ZERO_PADDING) {
        return SKE_INPUT_INVALID;
    }
    else {
        ctx->is_updated = 0;
        ctx->padding = padding;
        ctx->left_bytes = 0;
    }

    ske_hp_set_cpu_mode();

    return ske_hp_cbc_mac_init_internal(ctx, alg, key, sp_key_idx);
}

/* function: ske_hp cbc_mac update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes could be any value.
 */
uint32_t ske_hp_cbc_mac_update(ske_cbc_mac_ctx_t *ctx, uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t blocks_bytes;
    uint32_t ret;
    uint8_t fill_bytes, remainder;

    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    if ((NULL == msg) || (0 == msg_bytes)) {
        return SKE_SUCCESS;
    }
    else {
        ctx->is_updated = 1;
    }

    if (ctx->left_bytes) {
        fill_bytes = ctx->ske_cbc_mac_ctx->block_bytes - ctx->left_bytes;

        if (msg_bytes < fill_bytes) {
            memcpy_(ctx->block_buf + ctx->left_bytes, msg, msg_bytes);
            ctx->left_bytes += msg_bytes;
            return SKE_SUCCESS;
        }
        else {
            memcpy_(ctx->block_buf + ctx->left_bytes, msg, fill_bytes);
            ret = ske_hp_update_blocks_no_output(ctx->ske_cbc_mac_ctx, ctx->block_buf, ctx->ske_cbc_mac_ctx->block_bytes);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else {
                ctx->left_bytes = 0;
                msg += fill_bytes;
                msg_bytes -= fill_bytes;
            }
        }
    }

    /*update blocks*/
    blocks_bytes = (msg_bytes / ctx->ske_cbc_mac_ctx->block_bytes) * ctx->ske_cbc_mac_ctx->block_bytes;
    ret = ske_hp_update_blocks_no_output(ctx->ske_cbc_mac_ctx, msg, blocks_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*hold the remainder*/
    remainder = msg_bytes % ctx->ske_cbc_mac_ctx->block_bytes;

    if (remainder) {
        memcpy_(ctx->block_buf, msg + blocks_bytes, remainder);
        ctx->left_bytes = remainder;
    }
    else
    {;}

    return SKE_SUCCESS;
}

/* function: ske_hp cbc_mac finish, and get the mac(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_t context pointer
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. for the case that padding is SKE_NO_PADDING, if the total length of message is not a multiple of
 *        block length, it will return error.
 */
uint32_t ske_hp_cbc_mac_final(ske_cbc_mac_ctx_t *ctx, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t tmp[4];
    uint32_t ret;

    if (NULL == ctx || NULL == mac) {
        return SKE_BUFFER_NULL;
    }
    else if ((0 == mac_bytes) || (mac_bytes > ctx->ske_cbc_mac_ctx->block_bytes)) {
        return SKE_INPUT_INVALID;
    }
    else if (0 == ctx->is_updated) {
        /*no input, it is not valid*/
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    if (0 == ctx->left_bytes) {
        ske_hp_simple_get_output_block(tmp, ctx->ske_cbc_mac_ctx->block_words);
    }
    else {

        if (SKE_NO_PADDING == ctx->padding) {
            return SKE_ERROR;
        }
        else if (SKE_ZERO_PADDING == ctx->padding) {
            memset_(ctx->block_buf + ctx->left_bytes, 0, ctx->ske_cbc_mac_ctx->block_bytes - ctx->left_bytes);
            ret = ske_hp_update_blocks_internal(ctx->ske_cbc_mac_ctx, ctx->block_buf, (uint8_t *)tmp, ctx->ske_cbc_mac_ctx->block_bytes);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else {
            return SKE_ERROR;
        }
    }

    memcpy_(mac, tmp, mac_bytes);

    return SKE_SUCCESS;
}

/* function: ske_hp cbc mac(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     padding -------------------- input, ske_hp cbc mac padding scheme, like SKE_NO_PADDING,SKE_ZERO_PADDING.
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     2. msg_bytes can not be 0
 *     3. for the case that padding is SKE_NO_PADDING, if the total length of message is not a multiple of
 *        block length, it will return error.
 */
uint32_t ske_hp_cbc_mac(SKE_ALG alg, SKE_PADDING padding, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg,
                        uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ret;
    ske_cbc_mac_ctx_t ctx[1];

    ret = ske_hp_cbc_mac_init(ctx, alg, padding, key, sp_key_idx);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ske_hp_cbc_mac_update(ctx, msg, msg_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return ske_hp_cbc_mac_final(ctx, mac, mac_bytes);
}

#ifdef SKE_HP_DMA_FUNCTION
/* function: ske_hp cbc mac dma style init
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_dma_cbc_mac_init(ske_cbc_mac_dma_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    ske_hp_set_dma_mode();
    ske_hp_disable_dma_linked_list();

    return ske_hp_cbc_mac_init_internal((ske_cbc_mac_ctx_t *)ctx, alg, key, sp_key_idx);
}

/* function: ske_hp cbc mac dma style update message blocks(excluding the last block, or the message tail)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     msg ------------------------ input, message of some blocks, excluding last block(or message tail)
 *     msg_words ------------------ input, word length of in, must be a multiple of block word length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
uint32_t ske_hp_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_mac_dma_ctx_t *ctx, uint32_t *msg,
        uint32_t msg_words)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (msg_words & (ctx->ske_cbc_mac_ctx->block_words - 1)) {
        return SKE_INPUT_INVALID;
    }
    else if ((NULL == msg) || (0 == msg_words)) {
        return SKE_SUCCESS;
    }
    else
    {;}

    return ske_hp_dma_operate_without_output(ctx->ske_cbc_mac_ctx, msg, msg_words);
}

/* function: ske_hp cbc mac dma style update message including the last block(or message tail), and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     msg ------------------------ input, message including the last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, can not be 0
 *     mac ------------------------ output, cbc mac, occupies a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the actual message length msg_bytes is not a multiple of block length, please make sure the
 *        last block(or message tail) is padded already.
 */
uint32_t ske_hp_dma_cbc_mac_update_including_last_block(ske_cbc_mac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac)
{
    uint32_t msg_words;

    if ((NULL == ctx) || (NULL == msg) || (NULL == mac)) {
        return SKE_BUFFER_NULL;
    }
    else if (0 == msg_bytes) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    msg_words = (msg_bytes + ctx->ske_cbc_mac_ctx->block_bytes - 1) / ctx->ske_cbc_mac_ctx->block_bytes;
    msg_words *= ctx->ske_cbc_mac_ctx->block_words;

    ske_hp_set_last_block(1);

    return ske_hp_dma_operate(ctx->ske_cbc_mac_ctx, msg, mac, msg_words, ctx->ske_cbc_mac_ctx->block_words);
}

/* function: ske_hp cbc mac, dma style(DMA style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of msg
 *     mac ------------------------ output, mac, occupies a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     2. msg_bytes can not be 0
 *     3. if the actual message length msg_bytes is not a multiple of block length, please make sure the last block
 *        is padded already
 */
uint32_t ske_hp_dma_cbc_mac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
                            uint32_t *mac)
{
    uint32_t ret;
    ske_cbc_mac_dma_ctx_t ctx[1];

    ret = ske_hp_dma_cbc_mac_init(ctx, alg, key, sp_key_idx);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return ske_hp_dma_cbc_mac_update_including_last_block(ctx, msg, msg_bytes, mac);
}
#endif

#ifdef SKE_HP_DMA_LL_FUNCTION
/* function: ske_hp cbc mac dma style init
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_dma_ll_cbc_mac_init(ske_cbc_mac_dma_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx,
                                    uint8_t mac_bytes)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else {
        ctx->is_updated = 0;
    }

    ske_hp_set_dma_mode();
    ske_hp_enable_dma_linked_list();

    return ske_hp_cbc_mac_init_internal((ske_cbc_mac_ctx_t *)ctx, alg, key, sp_key_idx, mac_bytes);
}

/* function: ske_hp cbc mac dma style update message in blocks
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     in ------------------------- input, message in blocks
 *     out ------------------------ output, must has the same length as in
 *     words ---------------------- input, word length of in or out, must be a multiple of block word length
 *     llp ------------------------ input, DMA linked list node
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. here the length unit is ske block length, so please make sure input are some blocks, if the actual
 *        message length is not a multiple of block length, please make sure the last block is padded.
 */
uint32_t ske_hp_dma_ll_cbc_mac_update_blocks(ske_cbc_mac_dma_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t words,
        dma_ll_node_t *llp)
{
    if ((NULL == ctx) || (NULL == in) || (NULL == out)) {
        return SKE_BUFFER_NULL;
    }
    else if (words & (ctx->ske_cbc_mac_ctx->block_words - 1)) {
        return SKE_INPUT_INVALID;
    }
    else {
        ctx->is_updated = 1;
    }

    ske_hp_dma_ll_operate(in, out, words, llp);

    while (0 != llp->next_llp) {
        llp = (dma_ll_node_t *)(llp->next_llp);
    }

    /*keep the last output block*/
    uint32_copy(ctx->tmp_output_block, ((uint32_t *)(llp->dst_addr)) + (((llp->last_len) & 0x7FFFFFFF) >> 5) - ctx->ske_cbc_mac_ctx->block_words,
                ctx->ske_cbc_mac_ctx->block_words);

    return SKE_SUCCESS;
}

/* function: ske_hp cbc mac dma ll style finish, and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_dma_ctx_t context pointer
 *     mac ------------------------ output, mac
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t ske_hp_dma_ll_cbc_mac_final(ske_cbc_mac_dma_ctx_t *ctx, uint8_t *mac)
{
    return ske_hp_dma_cbc_mac_final(ctx, mac);
}

/* function: ske cbc mac, dma ll style(one-off)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     in ------------------------- input, message
 *     out ------------------------ just for temporary output, must have the same length as in
 *     in_words ------------------- input, word length of in and out, must be a multiple of block length.
 *     llp ------------------------ input, DMA linked list node
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     2. in_words must be a multiple of block length
 *     3. if the actual message length is not a multiple of block length, please make sure the last block is
 *        padded
 */
uint32_t ske_hp_dma_ll_cbc_mac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *in, uint32_t *out,
                               uint32_t in_words, dma_ll_node_t *llp, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ret;
    ske_cbc_mac_dma_ctx_t ctx[1];

    ret = ske_hp_dma_ll_cbc_mac_init(ctx, alg, key, sp_key_idx, mac_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ske_hp_dma_ll_cbc_mac_update_blocks(ctx, in, out, in_words, llp);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ske_hp_dma_ll_cbc_mac_final(ctx, mac);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else {
        return SKE_SUCCESS;
    }
}
#endif

#endif


