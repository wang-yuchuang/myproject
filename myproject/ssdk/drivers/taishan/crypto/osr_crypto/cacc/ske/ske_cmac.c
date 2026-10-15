/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske_cmac.h>
#ifdef SUPPORT_SKE_MODE_CMAC

/* function: ske_hp cmac internal init config
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_cmac_init_internal(ske_cmac_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx)
{
    uint32_t iv[4];

    uint32_clear(iv, 4);

    return ske_hp_init_internal(ctx->ske_cmac_ctx, alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, (uint8_t *)iv);
}

/* function: ske_hp cmac init(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_cmac_init(ske_cmac_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx)
{
    /*check and keep ctx->left_bytes = 0*/
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else {
        ctx->left_bytes = 0;
    }

    ske_hp_set_cpu_mode();

    return ske_hp_cmac_init_internal(ctx, alg, key, sp_key_idx);
}

/* function: ske_hp cmac update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes could be any value.
 */
uint32_t ske_hp_cmac_update(ske_cmac_ctx_t *ctx, uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t blocks_bytes;
    uint32_t ret;
    uint8_t fill_bytes, remainder;

    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (NULL == msg || 0 == msg_bytes) {
        return SKE_SUCCESS;
    }
    else
    {;}

    /*if one block left, process it*/
    if (ctx->ske_cmac_ctx->block_bytes == ctx->left_bytes) {
        ret = ske_hp_update_blocks_no_output(ctx->ske_cmac_ctx, ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else {
            ctx->left_bytes = 0;
        }
    }

    /*padding*/
    if (ctx->left_bytes) {
        fill_bytes = ctx->ske_cmac_ctx->block_bytes - ctx->left_bytes;

        if (msg_bytes <= fill_bytes) {
            memcpy_(ctx->block_buf + ctx->left_bytes, msg, msg_bytes);
            ctx->left_bytes += msg_bytes;
            return SKE_SUCCESS;
        }
        else {
            memcpy_(ctx->block_buf + ctx->left_bytes, msg, fill_bytes);
            ret = ske_hp_update_blocks_no_output(ctx->ske_cmac_ctx, ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);

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

    /*process some blocks*/
    blocks_bytes = (msg_bytes / ctx->ske_cmac_ctx->block_bytes) * ctx->ske_cmac_ctx->block_bytes;
    remainder = msg_bytes % ctx->ske_cmac_ctx->block_bytes;

    /*process remainder*/
    if (remainder) {
        ret = ske_hp_update_blocks_no_output(ctx->ske_cmac_ctx, msg, blocks_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else {
            memcpy_(ctx->block_buf, msg + blocks_bytes, remainder);
            ctx->left_bytes = remainder;
        }
    }
    else {
        blocks_bytes -= ctx->ske_cmac_ctx->block_bytes;
        ret = ske_hp_update_blocks_no_output(ctx->ske_cmac_ctx, msg, blocks_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else {
            memcpy_(ctx->block_buf, msg + blocks_bytes, ctx->ske_cmac_ctx->block_bytes);
            ctx->left_bytes = ctx->ske_cmac_ctx->block_bytes;
        }
    }

    return SKE_SUCCESS;
}

/* function: ske_hp cmac finish, and get the mac(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_t context pointer
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. .
 */
uint32_t ske_hp_cmac_final(ske_cmac_ctx_t *ctx, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t tmp[4];
    uint32_t ret;

    if ((NULL == ctx) || (NULL == mac)) {
        return SKE_BUFFER_NULL;
    }
    else if ((0 == mac_bytes) || (mac_bytes > ctx->ske_cmac_ctx->block_bytes)) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ske_hp_set_last_block(1);
    ske_hp_set_last_block_len(ctx->left_bytes);

    if (ctx->ske_cmac_ctx->block_bytes == ctx->left_bytes) {
        ret = ske_hp_update_blocks_internal(ctx->ske_cmac_ctx, ctx->block_buf, (uint8_t *)tmp, ctx->ske_cmac_ctx->block_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else {
        ctx->block_buf[ctx->left_bytes] = 0x80;
        memset_(ctx->block_buf + ctx->left_bytes + 1, 0, ctx->ske_cmac_ctx->block_bytes - 1 - ctx->left_bytes);
        ret = ske_hp_update_blocks_internal(ctx->ske_cmac_ctx, ctx->block_buf, (uint8_t *)tmp, ctx->ske_cmac_ctx->block_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }

    memcpy_(mac, tmp, mac_bytes);

    return SKE_SUCCESS;
}

/* function: ske_hp cmac(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
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
 *     2. msg_bytes could be any value(including 0).
 */
uint32_t ske_hp_cmac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg, uint32_t msg_bytes,
                     uint8_t *mac, uint8_t mac_bytes)
{
    ske_cmac_ctx_t ctx[1];
    uint32_t ret;

    ret = ske_hp_cmac_init(ctx, alg, key, sp_key_idx);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ske_hp_cmac_update(ctx, msg, msg_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return ske_hp_cmac_final(ctx, mac, mac_bytes);
}

#ifdef SKE_HP_DMA_FUNCTION
/* function: ske_hp cmac dma style init
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_dma_cmac_init(ske_cmac_dma_ctx_t *ctx, SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    ske_hp_set_dma_mode();
    ske_hp_disable_dma_linked_list();

    return ske_hp_cmac_init_internal((ske_cmac_ctx_t *)ctx, alg, key, sp_key_idx);
}

/* function: ske cmac dma style update update message blocks(excluding the last block, or the message tail)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_ctx_t context pointer
 *     msg ------------------------ input, message of some blocks, excluding last block(or message tail)
 *     msg_words ------------------ input, word length of msg, must be a multiple of block word length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
uint32_t ske_hp_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_ctx_t *ctx, uint32_t *msg,
        uint32_t msg_words)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (msg_words & (ctx->ske_cmac_ctx->block_words - 1)) {
        return SKE_INPUT_INVALID;
    }
    else if ((NULL == msg) || (0 == msg_words)) {
        return SKE_SUCCESS;
    }
    else
    {;}

    return ske_hp_dma_operate_without_output(ctx->ske_cmac_ctx, msg, msg_words);
}

/* function: ske_hp cmac dma style update message including the last block(or message tail), and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_ctx_t context pointer
 *     msg ------------------------ input, message including the last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, could be 0
 *     mac ------------------------ output, cmac, occupies a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the whole message length is 0, this case is supported. in this case, msg occupies a block, and
 *        please set msg_bytes to 0.
 */
uint32_t ske_hp_dma_cmac_update_including_last_block(ske_cmac_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac)
{
    uint32_t msg_words;
    uint32_t remainder_bytes;

    if ((NULL == ctx) || (NULL == msg) || (NULL == mac)) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    ske_hp_set_last_block(1);

    /*get last block length and pad padded by software, not hardware, do not delete this padding action*/
    if (0 == msg_bytes) {
        msg[0] = 0x80;
        msg[1] = 0;
        msg[2] = 0;
        msg[3] = 0;

        msg_bytes = 1;
        remainder_bytes = 1;
    }
    else if (msg_bytes & 0x0F) {
        msg[msg_bytes / 4] |= 0x80 << ((msg_bytes & 3) * 8);
        remainder_bytes = msg_bytes & 0x0F;
    }
    else {
        remainder_bytes = ctx->ske_cmac_ctx->block_bytes;
    }

    /*set the last block message length*/
    ske_hp_set_last_block_len(remainder_bytes);

    msg_words = (msg_bytes + ctx->ske_cmac_ctx->block_bytes - 1) / ctx->ske_cmac_ctx->block_bytes;
    msg_words *= ctx->ske_cmac_ctx->block_words;

    return ske_hp_dma_operate(ctx->ske_cmac_ctx, msg, mac, msg_words, ctx->ske_cmac_ctx->block_words);
}

/* function: ske_hp cmac(DMA style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 *     mac ------------------------ output, mac
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     2. msg_bytes is actual byte length of message, it could be any value(including 0).
 *        (1). if msg_bytes is not 0, msg must have (msg_bytes+15)/16 blocks, if the last block is not full,
 *        please pad with zero.
 *        (2). if msg_bytes is 0, msg occupies a block.
 */
uint32_t ske_hp_dma_cmac(SKE_ALG alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
                         uint32_t *mac)
{
    uint32_t ret;
    ske_cmac_dma_ctx_t ctx[1];

    ret = ske_hp_dma_cmac_init(ctx, alg, key, sp_key_idx);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return ske_hp_dma_cmac_update_including_last_block(ctx, msg, msg_bytes, mac);
}
#endif

#endif
