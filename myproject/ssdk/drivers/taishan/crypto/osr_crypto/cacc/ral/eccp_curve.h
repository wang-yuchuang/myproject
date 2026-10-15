/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef ECCP_CURVE_H
#define ECCP_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "utility.h"

/* sample ecc curve */
#define SUPPORT_BRAINPOOLP160R1
#define SUPPORT_SECP192R1
#define SUPPORT_SECP224R1
#define SUPPORT_SECP256R1
#define SUPPORT_SECP384R1
#define SUPPORT_BRAINPOOLP512R1
#define SUPPORT_SECP521R1

#define POINT_NOT_COMPRESSED      (0x04)

/* eccp curve struct */
typedef struct {
    uint32_t eccp_p_bitLen;        /* bit length of prime p */
    uint32_t eccp_n_bitLen;        /* bit length of order n */
    uint32_t *eccp_p;
    uint32_t *eccp_p_h;
    uint32_t *eccp_a;
    uint32_t *eccp_b;
    uint32_t *eccp_Gx;
    uint32_t *eccp_Gy;
    uint32_t *eccp_n;
    uint32_t *eccp_n_h;
    uint32_t *eccp_half_Gx;
    uint32_t *eccp_half_Gy;
} eccp_curve_t;

#ifdef SUPPORT_BRAINPOOLP160R1
extern const eccp_curve_t brainpoolp160r1[1];
#endif

#ifdef SUPPORT_SECP192R1
extern const eccp_curve_t secp192r1[1];
#endif

#ifdef SUPPORT_SECP224R1
extern const eccp_curve_t secp224r1[1];
#endif

#ifdef SUPPORT_SECP256R1
extern const eccp_curve_t secp256r1[1];
#endif

#ifdef SUPPORT_SECP384R1
extern const eccp_curve_t secp384r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP512R1
extern const eccp_curve_t brainpoolp512r1[1];
#endif

#ifdef SUPPORT_SECP521R1
extern const eccp_curve_t secp521r1[1];
#endif

/********* Curve25519 struct *********/
typedef struct {
    uint32_t p_bitLen;        /* bit length of prime p */
    uint32_t *p;
    uint32_t *p_h;
    uint32_t *a24;            /* (A-2)/4 */
    uint32_t *u;
    uint32_t *v;
    uint32_t *n;              /* order of curve or point(Gx,Gy) */
    uint32_t *n_h;
    uint32_t *h;
} mont_curve_t;

/********* Edward Curve 25519 struct *********/
typedef struct {
    uint32_t p_bitLen;        /* bit length of prime p */
    uint32_t *p;
    uint32_t *p_h;
    uint32_t *d;
    uint32_t *Gx;
    uint32_t *Gy;
    uint32_t *n;              /* order of curve or point(Gx,Gy) */
    uint32_t *n_h;
    uint32_t *h;
} edward_curve_t;

#ifdef __cplusplus
}
#endif

#endif
