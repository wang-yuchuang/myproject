/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef ECDSA_H
#define ECDSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>

/*ECDSA return code*/
enum ECDSA_RET_CODE {
    ECDSA_SUCCESS = PKE_SUCCESS,
    ECDSA_POINTOR_NULL = PKE_SUCCESS + 0x50,
    ECDSA_INVALID_INPUT,
    ECDSA_ZERO_ALL,
    ECDSA_INTEGER_TOO_BIG,
    ECDSA_VERIFY_FAILED,
};

uint32_t ecdsa_sign(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *rand_k, uint8_t *priKey,
                    uint8_t *signature);

uint32_t ecdsa_verify(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *pubKey, uint8_t *signature);

#ifdef PKE_SEC

#define ECDSA_SEC
#ifdef ECDSA_SEC

enum ECDSA_RET_CODE_S {
    ECDSA_SUCCESS_S = 0x7D5FEB14,
    ECDSA_ERROR_S = 0xB4C0BC5A
};

uint32_t ecdsa_sign_s(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *rand_k, uint8_t *priKey,
                      uint8_t *signature);

uint32_t ecdsa_verify_s(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *pubKey, uint8_t *signature);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
