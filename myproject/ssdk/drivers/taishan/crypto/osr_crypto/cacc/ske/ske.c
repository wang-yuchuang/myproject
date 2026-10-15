/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske.h>
static ske_ctx_t ske_ctx[1];

/* function: check whether the ske algorithm is valid or not
 * parameters:
 *     ske_alg -------------------- input, specific ske algorithm
 * return: SKE_SUCCESS(valid), other(invalid)
 * caution:
 *     1.
 */
uint8_t ske_hp_check_alg(SKE_ALG ske_alg)
{
    uint8_t ret;

    switch (ske_alg) {
#ifdef SUPPORT_SKE_DES
        case SKE_ALG_DES:
#endif

#ifdef SUPPORT_SKE_TDES_128
        case SKE_ALG_TDES_128:
#endif

#ifdef SUPPORT_SKE_TDES_192
        case SKE_ALG_TDES_192:
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
        case SKE_ALG_TDES_EEE_128:
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
        case SKE_ALG_TDES_EEE_192:
#endif

#ifdef SUPPORT_SKE_AES_128
        case SKE_ALG_AES_128:
#endif

#ifdef SUPPORT_SKE_AES_192
        case SKE_ALG_AES_192:
#endif

#ifdef SUPPORT_SKE_AES_256
        case SKE_ALG_AES_256:
#endif

#ifdef SUPPORT_SKE_SM4
        case SKE_ALG_SM4:
#endif
            ret = SKE_SUCCESS;
            break;

        default:
            ret = SKE_INPUT_INVALID;
            break;
    }

    return ret;
}

/* function: check whether the ske algorithm mode is valid or not
 * parameters:
 *     ske_alg -------------------- input, specific ske algorithm
 *     ske_mode ------------------- input, specific ske algorithm mode
 * return: SKE_SUCCESS(valid), other(invalid)
 * caution:
 *     1.
 */
uint8_t ske_hp_check_mode(SKE_ALG ske_alg, SKE_MODE ske_mode)
{
    uint8_t ret;

    switch (ske_mode) {
#ifdef SUPPORT_SKE_MODE_ECB
        case SKE_MODE_ECB:
#endif

#ifdef SUPPORT_SKE_MODE_CBC
        case SKE_MODE_CBC:
#endif

#ifdef SUPPORT_SKE_MODE_CFB
        case SKE_MODE_CFB:
#endif

#ifdef SUPPORT_SKE_MODE_OFB
        case SKE_MODE_OFB:
#endif

#ifdef SUPPORT_SKE_MODE_CTR
        case SKE_MODE_CTR:
#endif

#ifdef SUPPORT_SKE_MODE_CBC_MAC
        case SKE_MODE_CBC_MAC:
#endif
            ret = SKE_SUCCESS;
            break;

/*for DES/3DES, CAMC is not supported at present*/
#ifdef SUPPORT_SKE_MODE_CMAC
        case SKE_MODE_CMAC:
            switch (ske_alg) {
#ifdef SUPPORT_SKE_AES_128
                case SKE_ALG_AES_128 :
#endif
#ifdef SUPPORT_SKE_AES_192
                case SKE_ALG_AES_192 :
#endif
#ifdef SUPPORT_SKE_AES_256
                case SKE_ALG_AES_256 :
#endif
#ifdef SUPPORT_SKE_SM4
                case SKE_ALG_SM4 :
#endif
#if (defined(SUPPORT_SKE_AES_128) || defined(SUPPORT_SKE_AES_192) || defined(SUPPORT_SKE_AES_256) || defined(SUPPORT_SKE_SM4))
                    ret = SKE_SUCCESS;
                    break;
#endif

                default:
                    ret = SKE_INPUT_INVALID;
            }

            break;
#endif

            /*for DES/3DES, XTS, CCM and GCM mode are not supported due to the definition or standard*/
#ifdef SUPPORT_SKE_MODE_XTS
        case SKE_MODE_XTS:
#endif
#ifdef SUPPORT_SKE_MODE_CCM
        case SKE_MODE_CCM:
#endif
#ifdef SUPPORT_SKE_MODE_GCM
        case SKE_MODE_GCM:
#endif
#if (defined(SUPPORT_SKE_MODE_XTS) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_GCM))
            switch (ske_alg) {
#ifdef SUPPORT_SKE_AES_128
                case SKE_ALG_AES_128 :
#endif
#ifdef SUPPORT_SKE_AES_192
                case SKE_ALG_AES_192 :
#endif
#ifdef SUPPORT_SKE_AES_256
                case SKE_ALG_AES_256 :
#endif
#ifdef SUPPORT_SKE_SM4
                case SKE_ALG_SM4 :
#endif
#if (defined(SUPPORT_SKE_AES_128) || defined(SUPPORT_SKE_AES_192) || defined(SUPPORT_SKE_AES_256) || defined(SUPPORT_SKE_SM4))
                    ret = SKE_SUCCESS;
                    break;
#endif

                default:
                    ret = SKE_INPUT_INVALID;
                    break;
            }

            break;
#endif

        default:
            ret = SKE_INPUT_INVALID;
            break;
    }

    return ret;
}

/* function: get block byte length for spcific ske_hp alg
 * parameters:
 *     ske_alg -------------------- input, ske_hp algorithm
 * return: block byte length for ske_hp alg
 * caution:
 *     1. please make sure ske_alg is valid
 */
uint8_t ske_hp_get_block_byte_len(SKE_ALG ske_alg)
{
    uint8_t byteLen;

    switch (ske_alg) {
#ifdef SUPPORT_SKE_DES
        case SKE_ALG_DES :
#endif
#ifdef SUPPORT_SKE_TDES_128
        case SKE_ALG_TDES_128 :
#endif
#ifdef SUPPORT_SKE_TDES_192
        case SKE_ALG_TDES_192 :
#endif
#ifdef SUPPORT_SKE_TDES_EEE_128
        case SKE_ALG_TDES_EEE_128 :
#endif
#ifdef SUPPORT_SKE_TDES_EEE_192
        case SKE_ALG_TDES_EEE_192 :
#endif
#if (defined(SUPPORT_SKE_DES) ||defined(SUPPORT_SKE_TDES_128) ||defined(SUPPORT_SKE_TDES_192)   \
    ||defined(SUPPORT_SKE_TDES_EEE_128) ||defined(SUPPORT_SKE_TDES_EEE_192))
            byteLen = 8;
            break;
#endif
#ifdef SUPPORT_SKE_AES_128
        case SKE_ALG_AES_128 :
#endif
#ifdef SUPPORT_SKE_AES_192
        case SKE_ALG_AES_192 :
#endif
#ifdef SUPPORT_SKE_AES_256
        case SKE_ALG_AES_256 :
#endif
#ifdef SUPPORT_SKE_SM4
        case SKE_ALG_SM4 :
#endif
#if (defined(SUPPORT_SKE_AES_128) ||defined(SUPPORT_SKE_AES_192) ||defined(SUPPORT_SKE_AES_256) ||defined(SUPPORT_SKE_SM4))
            byteLen = 16;
            break;
#endif

        default:
            byteLen = 16;
    }

    return byteLen;
}

/* function: get key byte length for spcific ske_hp alg
 * parameters:
 *     ske_alg -------------------- input, ske_hp algorithm
 * return: key byte length for ske_hp alg
 * caution:
 *     1. please make sure ske_alg is valid
 */
uint8_t ske_hp_get_key_byte_len(SKE_ALG ske_alg)
{
    uint8_t byte_len;

    switch (ske_alg) {
#ifdef SUPPORT_SKE_DES
        case SKE_ALG_DES :
            byte_len = 8;
            break;
#endif

#ifdef SUPPORT_SKE_TDES_128
        case SKE_ALG_TDES_128 :
#endif
#ifdef SUPPORT_SKE_TDES_EEE_128
        case SKE_ALG_TDES_EEE_128 :
#endif
#ifdef SUPPORT_SKE_AES_128
        case SKE_ALG_AES_128 :
#endif
#ifdef SUPPORT_SKE_SM4
        case SKE_ALG_SM4 :
#endif
#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_EEE_128) || defined(SUPPORT_SKE_AES_128) \
    ||defined(SUPPORT_SKE_SM4))
            byte_len = 16;
            break;
#endif

#ifdef SUPPORT_SKE_TDES_192
        case SKE_ALG_TDES_192 :
#endif
#ifdef SUPPORT_SKE_TDES_EEE_192
        case SKE_ALG_TDES_EEE_192 :
#endif
#ifdef SUPPORT_SKE_AES_192
        case SKE_ALG_AES_192 :
#endif
#if (defined(SUPPORT_SKE_TDES_192) || defined(SUPPORT_SKE_TDES_EEE_192) || defined(SUPPORT_SKE_AES_192))
            byte_len = 24;
            break;
#endif

#ifdef SUPPORT_SKE_AES_256
        case SKE_ALG_AES_256 :
            byte_len = 32;
            break;
#endif

        default:
            byte_len = 16;
    }

    return byte_len;
}

/* function: set ske_hp iv
 * parameters:
 *     iv ------------------------- input, initial vector
 *     block_bytes ---------------- input, byte length of current ske_hp block
 * return: none
 * caution:
 *     1. please make sure the inputs are valid
 */
void ske_hp_set_iv(uint8_t *iv, uint32_t block_bytes)
{
    uint32_t tmp[4];

    if (((uint32_t)iv) & 3) {
        memcpy_(tmp, iv, block_bytes);
        ske_hp_set_iv_uint32(tmp, block_bytes / 4);
    }
    else {
        ske_hp_set_iv_uint32((uint32_t *)iv, block_bytes / 4);
    }
}

/* function: set ske_hp key
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     key ------------------------ input, key
 *     key_bytes ------------------ input, byte length of key
 *     key_idx -------------------- input, key index, only 1 and 2 are valid
 * return: none
 * caution:
 *     1. please make sure the inputs are valid
 */
void ske_hp_set_key(SKE_ALG alg, uint8_t *key, uint16_t key_bytes, uint16_t key_idx)
{
    uint32_t tmp[8];

    memcpy_(tmp, key, key_bytes);

    /*for 3DES-2key, set key3=key1*/
    switch (alg) {
#ifdef SUPPORT_SKE_TDES_128
        case SKE_ALG_TDES_128 :
#endif
#ifdef SUPPORT_SKE_TDES_EEE_128
        case SKE_ALG_TDES_EEE_128 :
#endif
#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_EEE_128))
            memcpy_(tmp + 4, key, 8);
            key_bytes += 8;
            break;
#endif

        default:
            break;
    }

    ske_hp_set_key_uint32(tmp, key_idx, key_bytes / 4);
}

/* function: ske_hp init config
 * parameters:
 *     ctx ------------------------ input, ske_ctx_t context pointer
 *     alg ------------------------ input, ske_hp algorithm
 *     mode ----------------------- input, ske_hp algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is common for CPU/DMA/DMA-LL
 *     2. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     3. if mode is CMAC/CBC-MAC, the iv must be a block of all zero
 *     4. if key is from user input, please make sure the argument key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_init_internal(ske_ctx_t *ctx, SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key,
                              uint16_t sp_key_idx, uint8_t *iv)
{
    uint32_t key_bytes;

    if (NULL == ctx) {
        return SKE_BUFFER_NULL;
    }
    else if (SKE_SUCCESS != ske_hp_check_alg(alg)) {
        return SKE_INPUT_INVALID;
    }
    else if (SKE_SUCCESS != ske_hp_check_mode(alg, mode)) {
        return SKE_INPUT_INVALID;
    }
    else if (crypto > SKE_CRYPTO_DECRYPT) {
        return SKE_INPUT_INVALID;
    }
    else if (NULL == key) {
        /*secure port key idx is from 1 to MAX_KEY_IDX*/
#ifdef SKE_SECURE_PORT_FUNCTION
        key_bytes = sp_key_idx & (0x7FFF);

        if ((0 == key_bytes) || (key_bytes > SKE_MAX_KEY_IDX)) {
            return SKE_INPUT_INVALID;
        }

#else
        return SKE_INPUT_INVALID;
#endif
    }
    else
    {;}

    if (SKE_MODE_ECB == mode) {
        iv = NULL;
    }
    else if (NULL == iv) {
        return SKE_BUFFER_NULL;
    }
    else
    {;}

    /*keep the block length*/
    ctx->block_bytes = ske_hp_get_block_byte_len(alg);
    ctx->block_words = ctx->block_bytes / 4;

    /*config*/
    ske_hp_set_endian_uint32();
    ske_hp_set_alg(alg);
    ske_hp_set_mode(mode);
    ske_hp_set_crypto(crypto);
    ske_hp_set_last_block(0);

    /*set iv or nonce*/
    if (NULL != iv) {
        ske_hp_set_iv(iv, ctx->block_bytes);
    }

    if (NULL != key) {
        /*key is from user input*/
        ske_hp_disable_secure_port();

        key_bytes = ske_hp_get_key_byte_len(alg);
        ske_hp_set_key(alg, key, key_bytes, 1);

#if defined(SUPPORT_SKE_MODE_XTS)

        if (SKE_MODE_XTS == mode) {
            ske_hp_set_key(alg, key + key_bytes, key_bytes, 2);
        }

#endif
    }
    else {
        /*key is from secure port*/
#if defined(SUPPORT_SKE_MODE_XTS)
        if (SKE_MODE_XTS == mode) {
            ske_hp_enable_secure_port(sp_key_idx + 1);
            ske_hp_expand_key();
        }

#endif
        ske_hp_enable_secure_port(sp_key_idx);
    }

    return ske_hp_expand_key();
}

/* function: ske_hp init config(CPU style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     mode ----------------------- input, ske_hp algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit must be a block
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_init(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv)
{
    ske_hp_set_cpu_mode();
    ske_hp_set_c_len_uint32(0);

    return ske_hp_init_internal(ske_ctx, alg, mode, crypto, key, sp_key_idx, iv);
}

/* function: ske_hp encryption or decryption(CPU style)
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit must be a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. bytes must be a multiple of block byte length.
 */
uint32_t ske_hp_update_blocks(uint8_t *in, uint8_t *out, uint32_t bytes)
{
    if ((NULL == in) || (NULL == out)) {
        return SKE_BUFFER_NULL;
    }
    else if (bytes & (ske_ctx->block_bytes - 1)) {
        return SKE_INPUT_INVALID;
    }

    ske_hp_update_blocks_internal(ske_ctx, in, out, bytes);

    return SKE_SUCCESS;
}

/* function: ske_hp finish
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_hp_final(void)
{
    memset_(ske_ctx, 0, sizeof(ske_ctx_t));

    return SKE_SUCCESS;
}

/* function: ske_hp encrypting or decrypting(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     mode ----------------------- input, ske_hp algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit is a block
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. bytes must be a multiple of block byte length.
 */
uint32_t ske_hp_crypto(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                       uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ret;

    ske_hp_set_cpu_mode();
    ske_hp_set_c_len_uint32(0);

    ret = ske_hp_init_internal(ske_ctx, alg, mode, crypto, key, sp_key_idx, iv);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else {
        return ske_hp_update_blocks(in, out, bytes);
    }
}

#ifdef SKE_HP_DMA_FUNCTION
/* function: ske_hp init config(DMA style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     mode ----------------------- input, ske_hp algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, must be a block
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit is a block
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 */
uint32_t ske_hp_dma_init(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv)
{
    ske_hp_set_dma_mode();
    ske_hp_set_c_len_uint32(0);
    ske_hp_disable_dma_linked_list();

    return ske_hp_init_internal(ske_ctx, alg, mode, crypto, key, sp_key_idx, iv);
}

/* function: ske_hp encryption or decryption(DMA style)
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     words ---------------------- input, word length of input or output, must be multiples of 4(128bits)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit is a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. words must be a multiple of block word length.
 */
uint32_t ske_hp_dma_update_blocks(uint32_t *in, uint32_t *out, uint32_t words)
{
    if (0 == words) {
        return SKE_SUCCESS;
    }
    else if (words & (ske_ctx->block_words - 1)) {
        return SKE_INPUT_INVALID;
    }
    else {
        return ske_hp_dma_operate(ske_ctx, in, out, words, words);
    }
}

/* function: ske_hp finish(DMA style)
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_hp_dma_final(void)
{
    memset_(ske_ctx, 0, sizeof(ske_ctx_t));

    return SKE_SUCCESS;
}

/* function: ske_hp encrypting or decrypting(DMA style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske_hp algorithm
 *     mode ----------------------- input, ske_hp algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     words ---------------------- input, word length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR/XTS modes, and input/output unit is a block
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. words must be a multiple of block word length.
 */
uint32_t ske_hp_dma_crypto(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv,
                           uint32_t *in, uint32_t *out, uint32_t words)
{
    uint32_t ret;

    ske_hp_set_dma_mode();
    ske_hp_set_c_len_uint32(0);
    ske_hp_disable_dma_linked_list();

    ret = ske_hp_init_internal(ske_ctx, alg, mode, crypto, key, sp_key_idx, iv);

    if (SKE_SUCCESS != ret) {
        return ret;
    }
    else {
        return ske_hp_dma_update_blocks(in, out, words);
    }
}
#endif

#ifdef SKE_HP_DMA_LL_FUNCTION
uint32_t ske_hp_dma_ll_init(SKE_ALG alg, SKE_MODE mode, SKE_CRYPTO crypto, uint8_t *key, uint16_t sp_key_idx, uint8_t *iv)
{
    ske_hp_set_dma_mode();
    ske_hp_enable_dma_linked_list();

    return ske_hp_init_internal(ske_ctx, alg, mode, crypto, key, sp_key_idx, iv);
}

uint32_t ske_hp_dma_ll_crypto(uint32_t *in, uint32_t *out, uint32_t wordLen, dma_ll_node_t *llp)
{
    ske_hp_dma_ll_operate(in, out, wordLen, llp);

    return SKE_SUCCESS;
}

/* function: ske_hp finish(DMA style)
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_hp_dma_ll_final()
{
    memset_(ske_ctx, 0, sizeof(ske_ctx_t));

    return SKE_SUCCESS;
}
#endif
