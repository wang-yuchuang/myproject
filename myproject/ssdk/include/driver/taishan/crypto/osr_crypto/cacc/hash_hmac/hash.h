/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/hash_basic.h>

/*HASH status*/
typedef struct {
    uint32_t busy             : 1;        /* calculate busy flag*/
} hash_status_t;

/*HASH context*/
typedef struct {
    HASH_ALG hash_alg;                              /*current hash algorithm*/
    uint8_t block_byte_len;
    uint8_t iterator_word_len;
    uint8_t digest_byte_len;
    hash_status_t status;                             /*hash update status, .busy=1 means doing =0 means idle*/
    uint8_t first_update_flag;                        /*whether first time to update message(1:yes, 0:no)*/
    uint8_t finish_flag;                              /*whether the whole message has been inputted(1:yes, 0:no)*/
    uint8_t hash_buffer[HASH_BLOCK_MAX_BYTE_LEN];     /*block buffer*/
    uint32_t total[HASH_TOTAL_LEN_MAX_WORD_LEN];      /*total byte length of the whole message*/

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];    /*keep current hash iterator value for multiple thread*/
#endif
} hash_ctx_t;

#ifdef HASH_DMA_FUNCTION
/*HASH DMA context*/
typedef struct {
	HASH_ALG hash_alg;
    uint32_t total[HASH_TOTAL_LEN_MAX_WORD_LEN];      /*total byte length of the whole message*/
    uint8_t block_word_len;
    HASH_CALLBACK callback;
} hash_dma_ctx_t;
#endif

/*APIs*/
uint8_t check_hash_alg(HASH_ALG hash_alg);

uint8_t hash_get_block_word_len(HASH_ALG hash_alg);

uint8_t hash_get_digest_word_len(HASH_ALG hash_alg);

uint8_t hash_get_iterator_word_len(HASH_ALG hash_alg);

void hash_set_tx_bit_len(hash_ctx_t *ctx, uint32_t block_count);

void hash_start_calculate(hash_ctx_t *ctx);

uint8_t hash_total_len_add_uint32(uint32_t *a, uint32_t a_words, uint32_t b);

void hash_total_bytelen_2_bitlen(uint32_t *a, uint32_t a_words);

uint32_t hash_init(hash_ctx_t *ctx, HASH_ALG hash_alg);

uint32_t hash_update(hash_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t hash_final(hash_ctx_t *ctx, uint8_t *digest);

uint32_t hash(HASH_ALG hash_alg, uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

void hash_set_IV(HASH_ALG hash_alg, uint8_t hash_iterator_words);

#ifdef HASH_DMA_FUNCTION
uint32_t hash_dma_init(hash_dma_ctx_t *ctx, HASH_ALG hash_alg, HASH_CALLBACK callback);

uint32_t hash_dma_update_blocks(hash_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator);

uint32_t hash_dma_final(hash_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t hash_dma(HASH_ALG hash_alg, uint32_t *msg, uint32_t msg_bytes, uint32_t *digest,
                  HASH_CALLBACK callback);
#endif

#ifdef __cplusplus
}
#endif

#endif
