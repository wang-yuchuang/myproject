/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef ED25519_H
#define ED25519_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>

extern const edward_curve_t ed25519[1];

/*Ed25519 mode*/
typedef enum
{
	Ed25519_DEFAULT = 0,
	Ed25519_CTX,
	Ed25519_PH,
}Ed25519_MODE;

/*Ed25519 return code*/
enum {
	EdDSA_SUCCESS = PKE_SUCCESS,
	EdDSA_POINTOR_NULL = PKE_SUCCESS+0x80,
	EdDSA_INVALID_INPUT,
	EdDSA_VERIFY_FAIL,
};

uint32_t ed25519_get_pubkey_from_prikey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t ed25519_getkey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t ed25519_sign(Ed25519_MODE mode, uint8_t prikey[32], uint8_t pubkey[32], uint8_t *ctx, uint8_t ctxByteLen,
		uint8_t *M, uint32_t MByteLen, uint8_t RS[64]);

uint32_t ed25519_verify(Ed25519_MODE mode, uint8_t pubkey[32], uint8_t *ctx, uint8_t ctxByteLen, uint8_t *M,
		uint32_t MByteLen, uint8_t RS[64]);

#ifdef __cplusplus
}
#endif

#endif
