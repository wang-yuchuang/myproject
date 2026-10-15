/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <debug.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL            ((void *)0)
#endif

#define GET_MAX_LEN(a,b)         (((a)>(b))?(a):(b))
#define GET_MIN_LEN(a,b)         (((a)>(b))?(b):(a))
#define GET_WORD_LEN(bitLen)     (((bitLen)+31)/32)
#define GET_BYTE_LEN(bitLen)     (((bitLen)+7)/8)

#define OSR_TRACEF(str, x...) do { printf("%s:%d: " str, __PRETTY_FUNCTION__, __LINE__, ## x); } while (0)
/* trace routines that work if LOCAL_TRACE is set */
#define OSR_LTRACEF(x...) do { if (LOCAL_TRACE) { OSR_TRACEF(x); } } while (0)

#define OSR_TRACE(str, x...) do { printf(" " str, ## x); } while (0)
/* trace routines that work if LOCAL_TRACE is set */
#define OSR_LTRACE(x...) do { if (LOCAL_TRACE) { OSR_TRACE(x); } } while (0)

#define PKE_PRINT_BUF
#ifdef PKE_PRINT_BUF
extern void print_buf_U8(uint8_t buf[], uint32_t byteLen, char name[]);
extern void print_buf_U32(uint32_t buf[], uint32_t wordLen, char name[]);
extern void print_BN_buf_U32(uint32_t buf[], uint32_t wordLen, char name[]);
#endif

typedef struct crypto_block {
    uint8_t *addr;   /* Start address of the data (FIFO or contiguous memory) */
    uint32_t len;    /* Length of data expressed in bytes */
} crypto_block_t;

void memcpy_(void *dst, void *src, uint32_t size);

void memset_(void *dst, uint8_t value, uint32_t size);

int8_t memcmp_(void *m1, void *m2, uint32_t size);

void uint32_set(uint32_t *a, uint32_t value, uint32_t wordLen);

void uint32_copy(uint32_t *dst, uint32_t *src, uint32_t wordLen);

void uint32_clear(uint32_t *a, uint32_t wordLen);

void uint32_sleep(uint32_t count, uint8_t rand);

void uint32_endian_reverse(uint8_t *in, uint8_t *out, uint32_t wordLen);

void reverse_byte_array(uint8_t *in, uint8_t *out, uint32_t byteLen);

void reverse_word(uint8_t *in, uint8_t *out, uint32_t bytelen);

void reverse_word_array(uint8_t *in, uint32_t *out, uint32_t wordLen);

void uint8_xor(uint8_t *a, uint8_t *b, uint8_t *c, uint32_t byteLen);

uint32_t get_valid_bits(const uint32_t *a, uint32_t wordLen);

uint32_t get_valid_words(uint32_t *a, uint32_t max_words);

uint8_t uint8_bignum_check_zero(uint8_t a[], uint32_t aByteLen);

uint8_t uint32_bignum_check_zero(uint32_t a[], uint32_t aWordLen);

int32_t uint32_bignumcmp(uint32_t *a, uint32_t aWordLen, uint32_t *b, uint32_t bWordLen);

uint32_t uint32_sec_cmp(uint32_t *a, uint32_t *b, uint32_t wordLen, uint8_t rand);

uint32_t get_multiple2_number(uint32_t a[]);

uint32_t big_div2n(uint32_t a[], int32_t aWordLen, uint32_t n);

uint8_t bigint_check_1(uint32_t a[], uint32_t aWordLen);

uint8_t bigint_check_p_1(uint32_t a[], uint32_t p[], uint32_t wordLen);

uint32_t ce_get_current_time(void);
#ifdef __cplusplus
}
#endif

#endif
