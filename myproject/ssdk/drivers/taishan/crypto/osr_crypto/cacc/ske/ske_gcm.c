/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske_gcm.h>
#ifdef SUPPORT_SKE_MODE_GCM

/* function: ske_hp gcm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value,
 *                                  including 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could not be zero at the same time
 */
uint32_t ske_hp_gcm_init(ske_gcm_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                         uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t tmp[4];

    if (NULL == ctx || NULL == iv) {
        return SKE_BUFFER_NULL;
    }
    else if (12 != iv_bytes) {
        return SKE_INPUT_INVALID;
    }
    else if ((0 == aad_bytes) && (0 == c_bytes)) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ske_hp_set_cpu_mode();

    memcpy_(tmp, iv, iv_bytes);
    tmp[3] = 0;

    ctx->aad_bytes = aad_bytes;
    ske_hp_set_aad_len_uint32(aad_bytes);

    ctx->c_bytes = c_bytes;
    ske_hp_set_c_len_uint32(c_bytes);

    ctx->current_bytes = 0;
    ctx->crypto = crypto;

    return ske_hp_init_internal(ctx->ske_gcm_ctx, alg, SKE_MODE_GCM, crypto, key, sp_key_idx, (uint8_t *)tmp);
}

/* function: ske_hp gcm mode input aad
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     aad ------------------------ input, aad, its length is ctx->aad_bytes, please make sure
 *                                         aad here is integral
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_hp_gcm_init()
 *     2. if there is no aad, this function could be omitted
 */
uint32_t ske_hp_gcm_aad(ske_gcm_ctx_t *ctx, uint8_t *aad)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t ret;

    if (NULL == ctx || (NULL == aad && ctx->aad_bytes != 0)) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    blocks_bytes = (ctx->aad_bytes) & (~0x0F);
    remainder_bytes = (ctx->aad_bytes) & 0x0F;

    if (remainder_bytes) {
        ret = ske_hp_update_blocks_no_output(ctx->ske_gcm_ctx, aad, blocks_bytes);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ske_hp_set_last_block(1);
        memcpy_(ctx->buf, aad + blocks_bytes, remainder_bytes);
        memset_(ctx->buf + remainder_bytes, 0, 16 - remainder_bytes);
        ret = ske_hp_update_blocks_no_output(ctx->ske_gcm_ctx, ctx->buf, 16);

        if (SKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else {
        if (blocks_bytes) {
            ret = ske_hp_update_blocks_no_output(ctx->ske_gcm_ctx, aad, blocks_bytes - 16);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}

            ske_hp_set_last_block(1);
            ret = ske_hp_update_blocks_no_output(ctx->ske_gcm_ctx, aad + blocks_bytes - 16, 16);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else
        {;}
    }

    ske_hp_set_last_block(0);

    ctx->current_bytes = 0;

    return SKE_SUCCESS;
}

/* function: ske_hp gcm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_hp_gcm_aad()
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     4. if plaintext/ciphertext is too long, you could divide it by block(16 bytes), and if bytes
 *        is not a multiple of 16, please make sure the last section contains the tail, then call
 *        this function to input the sections respectively. for example, if bytes is 65, the input
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
uint32_t ske_hp_gcm_update(ske_gcm_ctx_t *ctx, uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t ret;

    if (NULL == ctx || NULL == in || NULL == out) {
        return SKE_BUFFER_NULL;
    }
    else if (0 == bytes) {
        return SKE_SUCCESS;
    }
    else
    {;}

    if (ctx->current_bytes + bytes > ctx->c_bytes) {
        return SKE_INPUT_INVALID;
    }
    else if (ctx->current_bytes + bytes == ctx->c_bytes) {
        blocks_bytes = bytes & (~0x0F);
        remainder_bytes = bytes & 0x0F;

        if (0 == remainder_bytes) {
            if (blocks_bytes) {
                blocks_bytes -= 16;
                ret = ske_hp_update_blocks_internal(ctx->ske_gcm_ctx, in, out, blocks_bytes);

                if (SKE_SUCCESS != ret) {
                    return ret;
                }
                else
                {;}

                ske_hp_set_last_block(1);
                ret = ske_hp_update_blocks_internal(ctx->ske_gcm_ctx, in + blocks_bytes, out + blocks_bytes, 16);

                if (SKE_SUCCESS != ret) {
                    return ret;
                }
                else
                {;}
            }
            else
            {;}
        }
        else {
            ret = ske_hp_update_blocks_internal(ctx->ske_gcm_ctx, in, out, blocks_bytes);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}

            ske_hp_set_last_block(1);
            memcpy_(ctx->buf, in + blocks_bytes, remainder_bytes);
            memset_(ctx->buf + remainder_bytes, 0, 16 - remainder_bytes);
            ret = ske_hp_update_blocks_internal(ctx->ske_gcm_ctx, ctx->buf, ctx->buf, 16);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}

            memcpy_(out + blocks_bytes, ctx->buf, remainder_bytes);
        }
    }
    else {
        if (bytes & (16 - 1)) {
            return SKE_INPUT_INVALID;
        }
        else {
            ret = ske_hp_update_blocks_internal(ctx->ske_gcm_ctx, in, out, bytes);

            if (SKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}

            ctx->current_bytes += bytes;
        }
    }

    return SKE_SUCCESS;
}

/* function: ske_hp gcm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     tag ------------------------ input(for decryption), output(for encryption)
 *     tag_bytes ------------------ input, byte length of tag.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_hp_gcm_update()
 *     2. tag_bytes could be 0, but not bigger than SKE_HP_GCM_MAX_BYTES
 *     3. for encryption, tag is output; and for decryption, tag is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_hp_gcm_final(ske_gcm_ctx_t *ctx, uint8_t *tag, uint32_t tag_bytes)
{
    uint32_t ret;

    if (NULL == ctx || NULL == tag) {
        return SKE_BUFFER_NULL;
    }
    else if (tag_bytes > SKE_HP_GCM_MAX_BYTES) {
        return SKE_INPUT_INVALID;
    }

    ret = ske_hp_wait_till_output();

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ske_hp_simple_get_output_block((uint32_t *)ctx->buf, ctx->ske_gcm_ctx->block_words);

    if (SKE_CRYPTO_ENCRYPT == ctx->crypto) {
        memcpy_(tag, ctx->buf, tag_bytes);
        ret = SKE_SUCCESS;
    }
    else {
        ret = memcmp_(tag, ctx->buf, tag_bytes);
    }

    memset_(ctx, 0, sizeof(ske_gcm_ctx_t));

    return ret;
}

/* function: ske_hp gcm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad ------------------------ input, aad, please make sure aad here is integral
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value,
 *                                  including 0
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     tag ------------------------ input(for decryption), output(for encryption)
 *     tag_bytes ------------------ input, byte length of tag.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could not be zero at the same time
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     7. tag_bytes could be 0, but not bigger than SKE_HP_GCM_MAX_BYTES
 *     8. for encryption, tag is output; and for decryption, tag is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_hp_gcm_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint32_t iv_bytes, uint8_t *aad, uint32_t aad_bytes, uint8_t *in, uint8_t *out, uint32_t c_bytes,
                           uint8_t *tag, uint32_t tag_bytes)
{
    ske_gcm_ctx_t ctx[1];
    uint32_t ret;

    ret = ske_hp_gcm_init(ctx, alg, crypto, key, sp_key_idx, iv, iv_bytes, aad_bytes, c_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    ret = ske_hp_gcm_aad(ctx, aad);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    ret = ske_hp_gcm_update(ctx, in, out, c_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    return ske_hp_gcm_final(ctx, tag, tag_bytes);
}

#ifdef SKE_HP_DMA_FUNCTION

/* function: ske_hp dma gcm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value,
 *                                  including 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could not be zero at the same time
 */
uint32_t ske_hp_dma_gcm_init(ske_gcm_ctx_t *ctx, SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx,
                             uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t tmp[4];

    if (NULL == ctx || NULL == iv) {
        return SKE_BUFFER_NULL;
    }
    else if (12 != iv_bytes) {
        return SKE_INPUT_INVALID;
    }
    else if ((0 == aad_bytes) && (0 == c_bytes)) {
        return SKE_INPUT_INVALID;
    }
    else
    {;}

    ske_hp_set_dma_mode();

    memcpy_(tmp, iv, iv_bytes);
    tmp[3] = 0;

    ctx->aad_bytes = aad_bytes;
    ske_hp_set_aad_len_uint32(aad_bytes);

    ctx->c_bytes = c_bytes;
    ske_hp_set_c_len_uint32(c_bytes);

    ctx->current_bytes = 0;
    ctx->crypto = crypto;

    return ske_hp_init_internal(ctx->ske_gcm_ctx, alg, SKE_MODE_GCM, crypto, key, sp_key_idx, (uint8_t *)tmp);
}

/* function: ske_hp dma gcm mode input aad+plaintext/ciphertext, get ciphertext/plaintext+tag
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 *     in ------------------------- input, aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+tag
 *     tag_bytes ------------------ input, byte length of tag.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_hp_dma_gcm_init()
 *     2. aad must be blocks, if not, please pad it with 0
 *     3. plaintext/ciphertext must be blocks without padding 0
 *     4. the output will be blocks too, and the last block is tag with padding 0 if necessary
 *     5. please make sure aad+plaintext/ciphertext is integral
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
uint32_t ske_hp_dma_gcm_update_blocks(ske_gcm_ctx_t *ctx, uint32_t *in, uint32_t *out, uint32_t tag_bytes)
{
    uint32_t aad_blocks_words;
    uint32_t c_blocks_words;
    uint32_t c_bytes, i, ret;

    if (NULL == ctx || NULL == in || NULL == out) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    aad_blocks_words = ((ctx->aad_bytes + 15) / 16) * 4;
    c_blocks_words = ((ctx->c_bytes + 15) / 16) * 4;
    c_bytes = ctx->c_bytes;

    ske_hp_set_last_block(1);
    ret = ske_hp_dma_operate(ctx->ske_gcm_ctx, in, out, aad_blocks_words + c_blocks_words, c_blocks_words + 4);

    if ((SKE_SUCCESS == ret)) {
        /*get MSB(tag) of tag_bytes*/
        if (c_bytes & 0x0F) {
            i = (ctx->c_bytes) / 4;
            c_bytes &= 0x03;

            if (c_bytes) {
#ifdef SKE_HP_CPU_BIG_ENDIAN
                out[i] &= (1 << (32 - c_bytes * 8)) - 1;
#else
                out[i] &= (1 << (c_bytes * 8)) - 1;
#endif
                i++;
            }

            while (i < c_blocks_words) {
                out[i++] = 0;
            }
        }

        out += c_blocks_words;

        if (16 != tag_bytes) {

            i = tag_bytes / 4;
            tag_bytes &= 3;

            if (tag_bytes) {
#ifdef SKE_HP_CPU_BIG_ENDIAN
                out[i] &= (1 << (32 - tag_bytes * 8)) - 1;
#else
                out[i] &= (1 << (tag_bytes * 8)) - 1;
#endif
                i++;
            }

            while (i < 4) {
                out[i++] = 0;
            }
        }
    }
    else
    {;}

    return ret;
}

/* function: ske_hp dma gcm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_t context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is optional
 */
uint32_t ske_hp_dma_gcm_final(ske_gcm_ctx_t *ctx)
{
    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }

    memset_(ctx, 0, sizeof(ske_gcm_ctx_t));

    return SKE_SUCCESS;
}

/* function: ske_hp dma gcm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     in ------------------------- input, aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+tag
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value,
 *                                  including 0
 *     tag_bytes ------------------ input, byte length of tag.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could not be zero at the same time
 *     6. aad must be blocks, if not, please pad it with 0
 *     7. plaintext/ciphertext must be blocks without padding 0
 *     8. the output will be blocks too, and the last block is tag with padding 0 if necessary
 *     9. please make sure aad+plaintext/ciphertext is integral
 *     10. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
uint32_t ske_hp_dma_gcm_crypto(SKE_ALG alg, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                               uint32_t iv_bytes, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, uint32_t tag_bytes)
{
    ske_gcm_ctx_t ctx[1];
    uint32_t ret;

    ret = ske_hp_dma_gcm_init(ctx, alg, crypto, key, sp_key_idx, iv, iv_bytes, aad_bytes, c_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    ret = ske_hp_dma_gcm_update_blocks(ctx, in, out, tag_bytes);

    if (SKE_SUCCESS != ret) {
        return ret;
    }

    return ske_hp_dma_gcm_final(ctx);
}

#endif

#endif
