/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/md5.h>

#ifdef SUPPORT_HASH_MD5

/* function: init md5
 * parameters:
 *     ctx ------------------------ input, md5_ctx_t context pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t md5_init(md5_ctx_t *ctx)
{
    return hash_init(ctx, HASH_MD5);
}

/* function: md5 update message
 * parameters:
 *     ctx ------------------------ input, md5_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t md5_update(md5_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    return hash_update(ctx, msg, msg_bytes);
}

/* function: message update done, get the md5 digest
 * parameters:
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t md5_final(md5_ctx_t *ctx, uint8_t *digest)
{
    return hash_final(ctx, digest);
}

/* function: input whole message and get its md5 digest
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t md5_(uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    return hash(HASH_MD5, msg, msg_bytes, digest);
}

#ifdef HASH_DMA_FUNCTION
/* function: init dma md5
 * parameters:
 *     ctx ------------------------ input, md5_dma_ctx_t context pointer
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t md5_dma_init(md5_dma_ctx_t *ctx, HASH_CALLBACK callback)
{
    return hash_dma_init(ctx, HASH_MD5, callback);
}

/* function: dma md5 update some message blocks
 * parameters:
 *     ctx ------------------------ input, md5_dma_ctx_t context pointer
 *     msg ------------------------ input, message blocks
 *     msg_words ------------------ input, word length of the input message, must be a multiple of md5
 *                                  block word length(16)
 *     iterator ------------------- output, md5 temporary result
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t md5_dma_update_blocks(md5_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator)
{
    return hash_dma_update_blocks(ctx, msg, msg_words, iterator);
}

/* function: dma md5 final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, md5_dma_ctx_t context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message, must be in [0, BLOCK_BYTE_LEN-1],
 *                                  here BLOCK_BYTE_LEN is block byte length of md5, it is 64.
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t md5_dma_final(md5_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest)
{
    return hash_dma_final(ctx, remainder_msg, remainder_bytes, digest);
}

/* function: dma md5 digest calculate
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, md5 digest, 16 bytes
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
uint32_t md5_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback)
{
    return hash_dma(HASH_MD5, msg, msg_bytes, digest, callback);
}
#endif

#endif
