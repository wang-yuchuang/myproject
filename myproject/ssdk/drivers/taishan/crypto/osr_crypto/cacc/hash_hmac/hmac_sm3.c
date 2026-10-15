/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hmac_sm3.h>

#ifdef SUPPORT_HASH_SM3


/* function: init hmac-sm3
 * parameters:
 *     ctx ------------------------ input, hmac_sm3_ctx_t context pointer
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t hmac_sm3_init(hmac_sm3_ctx_t *ctx, uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes)
{
    return hmac_init(ctx, HASH_SM3, key, sp_key_idx, key_bytes);
}


/* function: hmac-sm3 update message
 * parameters:
 *     ctx ------------------------ input, hmac_sm3_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_sm3_update(hmac_sm3_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    return hmac_update(ctx, msg, msg_bytes);
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
uint32_t hmac_sm3_final(hmac_sm3_ctx_t *ctx, uint8_t *mac)
{
    return hmac_final(ctx, mac);
}


/* function: input key and whole message, get the mac
 * parameters:
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
uint32_t hmac_sm3(uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint8_t *msg, uint32_t msg_bytes, uint8_t *mac)
{
    return hmac(HASH_SM3, key, sp_key_idx, key_bytes, msg, msg_bytes, mac);
}


#ifdef HASH_DMA_FUNCTION
/* function: init dma hmac-sm3
 * parameters:
 *     ctx ------------------------ input, hmac_sm3_dma_ctx_t context pointer
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_sm3_dma_init(hmac_sm3_dma_ctx_t *ctx, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes,
                           HASH_CALLBACK callback)
{
    return hmac_dma_init(ctx, HASH_SM3, key, sp_key_idx, key_bytes, callback);
}


/* function: dma hmac-sm3 update message
 * parameters:
 *     ctx ------------------------ input, hmac_sm3_dma_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_words ------------------ input, word length of the input message, must be multiple of block word length
 *                                  of SM3(16)
 *     tmp_iterator --------------- output, temporary hash iterator
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t hmac_sm3_dma_update_blocks(hmac_sm3_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *tmp_iterator)
{
    return hmac_dma_update_blocks(ctx, msg, msg_words, tmp_iterator);
}


/* function: dma hmac-sm3 message update done, get the mac
 * parameters:
 *     ctx ------------------------ input, hmac_sm3_dma_ctx_t context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the last message, must be in [0, BLOCK_BYTE_LEN-1],
 *                                  here BLOCK_BYTE_LEN is block byte length of SM3(64)
 *     tmp_iterator --------------- output, temporary hash iterator
 *     mac ------------------------ output, mac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_sm3_dma_final(hmac_sm3_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes,
                            uint32_t *tmp_iterator, uint8_t *mac)
{
    return hmac_dma_final(ctx, remainder_msg, remainder_bytes, tmp_iterator, mac);
}


/* function: dma hmac-sm3 input key and message, get the mac
 * parameters:
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
uint32_t hmac_sm3_dma(uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, uint32_t msg_bytes,
                      uint32_t *tmp_iterator, uint8_t *mac, HASH_CALLBACK callback)
{
    return hmac_dma(HASH_SM3, key, sp_key_idx, key_bytes, msg, msg_bytes, tmp_iterator, mac, callback);
}

#endif


#endif
