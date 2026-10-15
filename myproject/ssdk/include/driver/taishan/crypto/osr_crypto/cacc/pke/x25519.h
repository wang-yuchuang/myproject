/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef X25519_H
#define X25519_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ral/pke.h>

extern const mont_curve_t c25519[1];

/*X25519 return code*/
enum {
	X25519_SUCCESS = PKE_SUCCESS,
	X25519_POINTER_NULL = PKE_SUCCESS+0x70,
	X25519_ZERO_ALL,
	X25519_INVALID_INPUT,
	X25519_INVALID_OUTPUT,
};

uint32_t x25519_get_pubkey_from_prikey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t x25519_getkey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t x25519_compute_key(uint8_t local_prikey[32], uint8_t peer_pubkey[32], uint8_t *key, uint32_t keyByteLen,
		KDF_FUNC kdf);

void x25519_decode_scalar(uint8_t *k, uint8_t *out, uint32_t bytes);
#ifdef __cplusplus
}
#endif

#endif
