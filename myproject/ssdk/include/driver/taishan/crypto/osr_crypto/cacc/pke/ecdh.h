/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef ECDH_H
#define ECDH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/eccp_curve.h>
#include <ral/pke.h>

/*ECDH return code*/
enum ECDH_RET_CODE {
    ECDH_SUCCESS = PKE_SUCCESS,
    ECDH_POINTOR_NULL = PKE_SUCCESS + 0x60,
    ECDH_INVALID_INPUT,
};

uint32_t ecdh_compute_key(eccp_curve_t *curve, uint8_t *local_prikey, uint8_t *peer_pubkey, uint8_t *key,
                          uint32_t keyByteLen, KDF_FUNC kdf);

#ifdef PKE_SEC

#define ECDH_SEC
#ifdef ECDH_SEC

enum ECDH_RET_CODE_S {
    ECDH_SUCCESS_S = 0x8B9BC1E1,
    ECDH_ERROR_S = 0xCBC192A3
};

uint32_t ecdh_compute_key_s(eccp_curve_t *curve, uint8_t *local_prikey, uint8_t *peer_pubkey, uint8_t *key,
                            uint32_t keyByteLen, KDF_FUNC kdf);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
