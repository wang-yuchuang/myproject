/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef PKE_PRIME_H
#define PKE_PRIME_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1:use hardware;  2:use software */
#define BIGINT_DIV_CHOICE     (2)

#if (BIGINT_DIV_CHOICE == 1)
typedef struct {
    uint32_t low;
    uint32_t high;
} double_uint32_t;
#elif (BIGINT_DIV_CHOICE == 2)
typedef uint32_t double_uint32_t;
#endif

/* 1:use Fermat primality test;  2:use Miller�CRabin primality test */
#define PRIMALITY_TEST_CHOICE (1)

#if (PRIMALITY_TEST_CHOICE == 1)
#define FERMAT_ROUND          (3)
#elif (PRIMALITY_TEST_CHOICE == 2)
#define MILLER_RABIN_ROUND    (3)
#endif

/* prime table level(total number of small prime numbers) */
#define PTL_MAX               (400)   /* the max PTL value */
#define PTL_512               (400)   /* the best PTL value for prime bit length 512 (RSA1024) */
#define PTL_1024              (400)   /* the best PTL value for prime bit length 1024 (RSA2048) */

#define NOT_PRIME             (0xFFFFFFFF)
#define MAYBE_PRIME           (0)

extern const uint16_t primetable[PTL_MAX];
extern const double_uint32_t primetable_s[PTL_MAX];
extern const uint16_t primetable_r[PTL_MAX];

uint32_t get_prime(uint32_t p[], uint32_t pbitlen);
uint32_t bigint_div_table_high(uint32_t *a, uint32_t awordlen, uint16_t *r, double_uint32_t *s,
                               uint16_t *high_result, uint32_t PTL);
uint32_t bigint_div_table_low(uint32_t *a, uint16_t *r, double_uint32_t *s, uint16_t *high_result, uint32_t PTL);
#ifdef __cplusplus
}
#endif

#endif
