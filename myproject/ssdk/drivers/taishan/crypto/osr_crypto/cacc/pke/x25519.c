/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <ral/pke.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/x25519.h>
#include <ral/trng.h>

#ifdef SUPPORT_C25519

/*Curve25519 parameters*/
uint32_t const curve25519_p[8]    = {0xFFFFFFED, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF,};
uint32_t const curve25519_p_h[8]  = {0x000005A4, 0, 0, 0, 0, 0, 0, 0,};
uint32_t const curve25519_a24[8]  = {0x0001DB41, 0, 0, 0, 0, 0, 0, 0,};
uint32_t const curve25519_u[]     = {0x00000009, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,};
uint32_t const curve25519_v[]     = {0x7ECED3D9, 0x29E9C5A2, 0x6D7C61B2, 0x923D4D7E, 0x7748D14C, 0xE01EDD2C, 0xB8A086B4, 0x20AE19A1,};
uint32_t const curve25519_n[]     = {0x5CF5D3ED, 0x5812631A, 0xA2F79CD6, 0x14DEF9DE, 0x00000000, 0x00000000, 0x00000000, 0x10000000,};
uint32_t const curve25519_n_h[8]  = {0x449C0F01, 0xA40611E3, 0x68859347, 0xD00E1BA7, 0x17F5BE65, 0xCEEC73D2, 0x7C309A3D, 0x0399411B,};
uint32_t const curve25519_h       = 8;


const mont_curve_t c25519[1] = {
    {
        255,
        (uint32_t *)curve25519_p,
        (uint32_t *)curve25519_p_h,
        (uint32_t *)curve25519_a24,
        (uint32_t *)curve25519_u,
        (uint32_t *)curve25519_v,
        (uint32_t *)curve25519_n,
        (uint32_t *)curve25519_n_h,
        (uint32_t *) &curve25519_h,
    },
};




/* Function: decode X25519 scalar for point multiplication
 * Parameters:
 *     k -------------------------- input,
 *     out ------------------------ output, big scalar in little-endian
 *     bytes ---------------------- input, byte length of k and out
 * Return: none
 * Caution:
 *     1.
 */
void x25519_decode_scalar(uint8_t *k, uint8_t *out, uint32_t bytes)
{
    if (k != out) {
        memcpy_(out, k, bytes);
    }
    else
    {;}

    /*clear lowest 3 bits*/
    out[0] &= 0xF8;
    /*clear highest 1 bit*/
    out[bytes - 1] &= 0x7F;
    /*set second highest bit as 1*/
    out[bytes - 1] |= 0x40;
}


/* Function: decode X25519 u coordinate for point multiplication
 * Parameters:
 *     u -------------------------- input,
 *     p -------------------------- input, modulus in little-endian
 *     out ------------------------ output, big scalar in little-endian
 *     bytes ---------------------- input, byte length of u, p and out
 * Return: none
 * Caution:
 *     1.
 */
uint32_t x25519_decode_u(uint8_t *u, uint32_t *p, uint8_t *out, uint32_t bytes)
{
    uint8_t ret;

    if (u != out) {
        memcpy_(out, u, bytes);
    }
    else
    {;}

    /*clear highest bit*/
    out[bytes - 1] &= 0x7F;

    /*mod p*/
    if (uint32_bignumcmp((uint32_t *)out, (bytes + 3) / 4, p, (bytes + 3) / 4) >= 0) {
        ret = pke_sub((uint32_t *)out, p, (uint32_t *)out, (bytes + 3) / 4);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    return PKE_SUCCESS;
}


/* Function: get X25519 public key from private key
 * Parameters:
 *     prikey --------------------- input, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: X25519_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t x25519_get_pubkey_from_prikey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t t[C25519_WORD_LEN];
    uint32_t ret;

    if (NULL == prikey || NULL == pubkey) {
        return X25519_POINTER_NULL;
    }
    else
    {;}

    x25519_decode_scalar(prikey, (uint8_t *)t, C25519_BYTE_LEN);

    /*it could be proved that here t is not multiple of c25519->n, so no need to compare*/
    /*(t mod c25519->n) with c25519->n*/

    ret = x25519_pointMul((mont_curve_t *)c25519, t, c25519->u, t);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    memcpy_(pubkey, t, C25519_BYTE_LEN);

    return X25519_SUCCESS;
}


/* Function: get x25519 random key pair
 * Parameters:
 *     prikey --------------------- output, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: X25519_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t x25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ret;

    if (NULL == prikey || NULL == pubkey) {
        return X25519_POINTER_NULL;
    }
    else
    {;}

    ret = get_rand(prikey, C25519_BYTE_LEN);

    if (TRNG_SUCCESS != ret) {
        return ret;
    }
    else {
        return x25519_get_pubkey_from_prikey(prikey, pubkey);
    }
}


/* Function:  X25519 key agreement
 * Parameters:
 *     local_prikey --------------- input, local private key, 32 bytes, little-endian
 *     peer_pubkey ---------------- input, peer Public key, 32 bytes, little-endian
 *     key ------------------------ output, derived key
 *     keyByteLen ----------------- input, byte length of output key
 *     kdf ------------------------ input, KDF function
 * Return: X25519_SUCCESS(success); other(error)
 * Caution:
 *     1. if no KDF function, please set kdf to be NULL
 */
uint32_t x25519_compute_key(uint8_t local_prikey[32], uint8_t peer_pubkey[32], uint8_t *key, uint32_t keyByteLen,
                            KDF_FUNC kdf)
{
    uint32_t k[C25519_WORD_LEN], u[C25519_WORD_LEN];
    uint32_t ret;

    if (NULL == local_prikey || NULL == peer_pubkey || NULL == key) {
        return X25519_POINTER_NULL;
    }
    else if (0 == keyByteLen) {
        return X25519_INVALID_INPUT;
    }
    else
    {;}

    /*decode u*/
    ret = x25519_decode_u(peer_pubkey, c25519->p, (uint8_t *)u, C25519_BYTE_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*u could not be zero, otherwise it will return PKE_NO_MODINV no matter what the scalar is.*/
    if (uint32_bignum_check_zero(u, C25519_WORD_LEN)) {
        return X25519_INVALID_INPUT;
    }
    else
    {;}

    /*decode scalar*/
    x25519_decode_scalar(local_prikey, (uint8_t *)k, C25519_BYTE_LEN);

    ret = x25519_pointMul((mont_curve_t *)c25519, k, u, u);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*make sure u is not zero*/
    k[0] = 0;

    for (ret = 0; ret < C25519_WORD_LEN; ret++) {
        k[0] |= u[ret];
    }

    if (0 == k[0]) {
        return X25519_ZERO_ALL;
    }
    else
    {;}

    if (kdf) {
        kdf(u, C25519_BYTE_LEN, key, keyByteLen);
    }
    else {
        if (keyByteLen > C25519_BYTE_LEN) {
            keyByteLen = C25519_BYTE_LEN;
        }
        else
        {;}

        memcpy_(key, u, keyByteLen);
    }

    return X25519_SUCCESS;
}

#endif
