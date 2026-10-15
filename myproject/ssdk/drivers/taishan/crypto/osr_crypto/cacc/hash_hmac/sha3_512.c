/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/sha3_512.h>

#ifdef SUPPORT_HASH_SHA3_512

/* function: init sha3_512
 * parameters:
 *     ctx ------------------------ input, sha3_512_ctx_t context pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t sha3_512_init(sha3_512_ctx_t *ctx)
{
    return hash_init(ctx, HASH_SHA3_512);
}


/* function: sha3_512 update message
 * parameters:
 *     ctx ------------------------ input, sha3_512_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t sha3_512_update(sha3_512_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    return hash_update(ctx, msg, msg_bytes);
}


/* function: message update done, get the sha3_512 digest
 * parameters:
 *     digest --------------------- output, sha3_512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t sha3_512_final(sha3_512_ctx_t *ctx, uint8_t *digest)
{
    return hash_final(ctx, digest);
}


/* function: input whole message and get its sha3_512 digest
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, sha3_512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t sha3_512(uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    return hash(HASH_SHA3_512, msg, msg_bytes, digest);
}


#ifdef HASH_DMA_FUNCTION
/* function: init dma sha3_512
 * parameters:
 *     ctx ------------------------ input, sha3_512_dma_ctx_t context pointer
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sha3_512_dma_init(sha3_512_dma_ctx_t *ctx, HASH_CALLBACK callback)
{
    return hash_dma_init(ctx, HASH_SHA3_512, callback);
}


/* function: dma sha3_512 update some message blocks
 * parameters:
 *     ctx ------------------------ input, sha3_512_dma_ctx_t context pointer
 *     msg ------------------------ input, message blocks
 *     msg_words ------------------ input, word length of the input message, must be multiple of sha3_512
 *                                  block word length(18)
 *     iterator ------------------- output, sha3_512 temporary result
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t sha3_512_dma_update_blocks(sha3_512_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator)
{
    return hash_dma_update_blocks(ctx, msg, msg_words, iterator);
}


/* function: dma sha3_512 final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, sha3_512_dma_ctx_t context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message, must be in [0, BLOCK_BYTE_LEN-1],
 *                                  here BLOCK_BYTE_LEN is block byte length of sha3_512, it is 72.
 *     digest --------------------- output, sha3_512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t sha3_512_dma_final(sha3_512_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest)
{
    return hash_dma_final(ctx, remainder_msg, remainder_bytes, digest);
}


/* function: dma sha3_512 digest calculate
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, sha3_512 digest, 64 bytes
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
uint32_t sha3_512_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback)
{
    return hash_dma(HASH_SHA3_512, msg, msg_bytes, digest, callback);
}
#endif

#endif
