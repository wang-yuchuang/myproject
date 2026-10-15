/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef RSA_H
#define RSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>

/*RSA return code*/
enum RSA_RET_CODE {
    RSA_SUCCESS = PKE_SUCCESS,
    RSA_BUFFER_NULL = PKE_SUCCESS + 0x30,
    RSA_INPUT_TOO_LONG,
    RSA_INPUT_INVALID,
};

uint32_t RSA_ModExp(uint32_t *a, uint32_t *e, uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_CRTModExp(uint32_t *a, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq,
                       uint32_t *u, uint32_t *out,  uint32_t nBitLen);

uint32_t RSA_GetKey(uint32_t *e, uint32_t *d, uint32_t *n, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_GetCRTKey(uint32_t *e, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u,
                       uint32_t *n, uint32_t eBitLen, uint32_t nBitLen);

#ifdef PKE_SEC

#define RSA_SEC
#ifdef RSA_SEC

/*RSA SEC return code*/
enum RSA_RET_CODE_S {
    RSA_SUCCESS_S = 0x3AEBA318,
    RSA_ERROR_S = 0x45DF3DAE
};

uint32_t RSA_ModExp_with_pub(uint32_t *a, uint32_t *e, uint32_t *d, uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_CRTModExp_with_pub(uint32_t *a, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u, uint32_t *e,
                                uint32_t *out, uint32_t eBitLen,  uint32_t nBitLen);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
