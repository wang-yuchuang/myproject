/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <string.h>
#include <ral/pke.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/ed25519.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/x25519.h>
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hash.h>
#include <ral/trng.h>

#ifdef SUPPORT_C25519

const char *Ed25519_sign_string = "SigEd25519 no Ed25519 collisions";

extern uint32_t const curve25519_p[8];
extern uint32_t const curve25519_p_h[8];
extern uint32_t const curve25519_p_n0[1];
extern uint32_t const curve25519_n[8];
extern uint32_t const curve25519_n_h[8];
extern uint32_t const curve25519_n_n0[1];

uint32_t const ed25519_d[]       = {0x135978A3, 0x75EB4DCA, 0x4141D8AB, 0x00700A4D, 0x7779E898, 0x8CC74079, 0x2B6FFE73, 0x52036CEE,};
uint32_t const ed25519_Gx[]      = {0x8F25D51A, 0xC9562D60, 0x9525A7B2, 0x692CC760, 0xFDD6DC5C, 0xC0A4E231, 0xCD6E53FE, 0x216936D3,};
uint32_t const ed25519_Gy[]      = {0x66666658, 0x66666666, 0x66666666, 0x66666666, 0x66666666, 0x66666666, 0x66666666, 0x66666666,};

const edward_curve_t ed25519[1] = {
    {
        255,
        (uint32_t *)curve25519_p,
        (uint32_t *)curve25519_p_h,
        (uint32_t *)ed25519_d,
        (uint32_t *)ed25519_Gx,
        (uint32_t *)ed25519_Gy,
        (uint32_t *)curve25519_n,
        (uint32_t *)curve25519_n_h,
    },
};

/* function: edwards25519 curve point mul(random point), Q=[k]P, secure version
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar, it could be 0 here
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could be zero here.
 */
uint32_t ed25519_pointMul_s(edward_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                            uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordLen = GET_WORD_LEN(curve->p_bitLen);

    if (uint32_bignum_check_zero(k, wordLen)) {
        uint32_clear(Qx, wordLen);
        uint32_clear(Qy, wordLen);
        Qy[0] = 1;

        return PKE_SUCCESS;
    }
    else {
        return ed25519_pointMul(curve, k, Px, Py, Qx, Qy);
    }
}


/* Function: get Ed25519 public key from private key
 * Parameters:
 *     prikey --------------------- input, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t ed25519_get_pubkey_from_prikey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t h[16];
    uint32_t ret;

    if (NULL == prikey || NULL == pubkey) {
        return EdDSA_POINTOR_NULL;
    }
    else
    {;}

    ret = hash(HASH_SHA512, prikey, 32, (uint8_t *)h);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*decode to get the scalar*/
    x25519_decode_scalar((uint8_t *)h, (uint8_t *)h, Ed25519_BYTE_LEN);

    ret = ed25519_pointMul_s((edward_curve_t *)ed25519, h, ed25519->Gx, ed25519->Gy, h, h + 8);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*encode pubkey*/
    memcpy_(pubkey, h + 8, Ed25519_BYTE_LEN);

    if (h[0] & 1) {
        pubkey[Ed25519_BYTE_LEN - 1] |= 0x80;
    }
    else
    {;}

    return EdDSA_SUCCESS;
}

/* Function: generate Ed25519 random key pair
 * Parameters:
 *     prikey --------------------- output, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t ed25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ret;

    if (NULL == prikey || NULL == pubkey) {
        return EdDSA_POINTOR_NULL;
    }
    else
    {;}

    ret = get_rand(prikey, Ed25519_BYTE_LEN);

    if (TRNG_SUCCESS != ret) {
        return ret;
    }
    else {
        return ed25519_get_pubkey_from_prikey(prikey, pubkey);
    }
}

/* Function: Ed25519 sign
 * Parameters:
 *     mode -------------- input, Ed25519 signature mode
 *     prikey ------------ input, private key, 32 bytes, little-endian
 *     pubkey ------------ input, public key, 32 bytes, little-endian, if no pubkey, please set it to be NULL
 *     ctx --------------- input, 0-255 bytes
 *     ctxByteLen -------- input, byte length of ctx
 *     M ----------------- input, message, M could be empty, in this case please set M to be NULL
 *     MByteLen ---------- input, byte length of M, M could be empty, so it could be 0
 *     RS ---------------- output, signature
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1. if no public key, please set pubkey to be NULL, it will be generated inside
 *     2. M could be empty(please set M to be NULL), so no need to check M and MByteLen
 *     3. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     4. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     5. if mode is Ed25519_PH, ctx length is from 0 to 255, default length is 0, thus ctx could be empty
 */
uint32_t ed25519_sign(Ed25519_MODE mode, uint8_t prikey[32], uint8_t pubkey[32], uint8_t *ctx, uint8_t ctxByteLen,
                      uint8_t *M, uint32_t MByteLen, uint8_t RS[64])
{
    uint32_t h[16];
    uint32_t *s = h;
    uint8_t *prefix = (uint8_t *)(h + Ed25519_WORD_LEN);

    uint32_t *r = h + Ed25519_WORD_LEN;
    uint32_t k[Ed25519_WORD_LEN << 1];
    uint32_t PH_M[Ed25519_WORD_LEN << 1];

    hash_ctx_t sha512_ctx[1];
    uint32_t ret;
    uint8_t phflag, tmp;

    if (mode > Ed25519_PH) {
        return EdDSA_INVALID_INPUT;
    }
    else if (NULL == prikey || NULL == RS) {
        return EdDSA_POINTOR_NULL;
    }
    else
    {;}

    /*M could be empty, so M could be NUll, MByteLen could be 0, no need to check them*/
    if (NULL == M) {
        MByteLen = 0;
    }
    else
    {;}

    if (Ed25519_CTX == mode) {
        /*in this case ctx can not be empty*/
        if (NULL == ctx || 0 == ctxByteLen) {
            return EdDSA_INVALID_INPUT;
        }
        else
        {;}
    }
    else if (Ed25519_PH == mode) {
        /*in this case ctx could be empty*/
        if (NULL == ctx) {
            ctxByteLen = 0;
        }
        else
        {;}
    }
    else/*Ed25519_DEFAULT mode, ctx is useless*/
    {;}

    /*************** get private scalar s and prefix ***************/
    ret = hash(HASH_SHA512, prikey, Ed25519_BYTE_LEN, (uint8_t *)h);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*decode to get the scalar s*/
    x25519_decode_scalar((uint8_t *)h, (uint8_t *)h, Ed25519_BYTE_LEN);

    /************************* set flag F **************************/
    if (Ed25519_CTX == mode) {
        phflag = 0;
    }
    else if (Ed25519_PH == mode) {
        phflag = 1;
    }
    else
    {;}

    /*PH_M*/
    if (Ed25519_PH == mode) {
        ret = hash(HASH_SHA512, M, MByteLen, (uint8_t *)PH_M);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /******* get k = SHA-512(dom2(F, C) || prefix || PH(M)) ********/
    ret = hash_init(sha512_ctx, HASH_SHA512);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*dom2(phflag, ctx)*/
    if (Ed25519_DEFAULT != mode) {
        tmp = strlen(Ed25519_sign_string);
        ret = hash_update(sha512_ctx, (uint8_t *)Ed25519_sign_string, tmp);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&phflag, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&ctxByteLen, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, ctx, ctxByteLen);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*prefix*/
    ret = hash_update(sha512_ctx, prefix, Ed25519_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*PH(M)*/
    if (Ed25519_PH == mode) {
        ret = hash_update(sha512_ctx, (uint8_t *)PH_M, 64);
    }
    else {
        ret = hash_update(sha512_ctx, M, MByteLen);
    }

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(sha512_ctx, (uint8_t *)k);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /************************ get R = [r]B *************************/
    /*r = k mod n*/
    ret = pke_mod(k + Ed25519_WORD_LEN - 1, Ed25519_WORD_LEN + 1, ed25519->n, ed25519->n_h,
                  Ed25519_WORD_LEN, h + Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    uint32_copy(k + Ed25519_WORD_LEN - 1, h + Ed25519_WORD_LEN, Ed25519_WORD_LEN);
    ret = pke_mod(k, (Ed25519_WORD_LEN << 1) - 1, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, r);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ed25519_pointMul_s((edward_curve_t *)ed25519, r, ed25519->Gx, ed25519->Gy, k, k + Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    memcpy_(RS, k + Ed25519_WORD_LEN, Ed25519_BYTE_LEN);

    if (k[0] & 1) {
        RS[Ed25519_BYTE_LEN - 1] |= 0x80;
    }
    else
    {;}

    /******* get k = SHA-512(dom2(F, C) || R || A || PH(M)) ********/
    ret = hash_init(sha512_ctx, HASH_SHA512);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*dom2(phflag, ctx)*/
    if (Ed25519_DEFAULT != mode) {
        tmp = strlen(Ed25519_sign_string);
        ret = hash_update(sha512_ctx, (uint8_t *)Ed25519_sign_string, tmp);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&phflag, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&ctxByteLen, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, ctx, ctxByteLen);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*R*/
    ret = hash_update(sha512_ctx, RS, Ed25519_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*pubkey(A)*/
    if (NULL == pubkey) {
        ret = ed25519_pointMul_s((edward_curve_t *)ed25519, s, ed25519->Gx, ed25519->Gy, k, k + Ed25519_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else if (k[0] & 1) {
            k[(Ed25519_WORD_LEN << 1) - 1] |= 0x80000000;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)(k + Ed25519_WORD_LEN), Ed25519_BYTE_LEN);
    }
    else {
        ret = hash_update(sha512_ctx, pubkey, Ed25519_BYTE_LEN);
    }

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*PH(M)*/
    if (Ed25519_PH == mode) {
        ret = hash_update(sha512_ctx, (uint8_t *)PH_M, 64);
    }
    else {
        ret = hash_update(sha512_ctx, M, MByteLen);
    }

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(sha512_ctx, (uint8_t *)k);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /***************** get S = (r + k * s) mod n *******************/
    /*PH_M = k mod n*/
    ret = pke_mod(k + Ed25519_WORD_LEN - 1, Ed25519_WORD_LEN + 1, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, PH_M);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    uint32_copy(k + Ed25519_WORD_LEN - 1, PH_M, Ed25519_WORD_LEN);
    ret = pke_mod(k, (Ed25519_WORD_LEN << 1) - 1, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, PH_M);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*k = s mod n*/
    ret = pke_mod(s, Ed25519_WORD_LEN, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, k);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*k = k*s*/
    ret = pke_modmul(ed25519->n, PH_M, k, k, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*k = (r+k*s)mod n*/
    ret = pke_modadd(ed25519->n, k, r, k, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    memcpy_(RS + Ed25519_BYTE_LEN, k, Ed25519_BYTE_LEN);

    return EdDSA_SUCCESS;
}


/* Function: Ed25519 verify
 * Parameters:
 *     mode -------------- input, Ed25519 signature mode
 *     pubkey ------------ input, public key, 32 bytes, little-endian
 *     ctx --------------- input, 0-255 bytes
 *     ctxByteLen -------- input, byte length of ctx
 *     M ----------------- input, message, M could be empty, in this case please set M to be NULL
 *     MByteLen ---------- input, byte length of M, M could be empty, so it could be 0
 *     RS ---------------- input, signature
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1. M could be empty(please set M to be NULL), so no need to check M and MByteLen
 *     2. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     3. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     4. if mode is Ed25519_PH, ctx length is from 0 to 255, default length is 0, thus ctx could be empty
 */
uint32_t ed25519_verify(Ed25519_MODE mode, uint8_t pubkey[32], uint8_t *ctx, uint8_t ctxByteLen, uint8_t *M,
                        uint32_t MByteLen, uint8_t RS[64])
{
    uint32_t k[Ed25519_WORD_LEN << 1];
    uint32_t S[Ed25519_WORD_LEN];
    uint32_t PH_M[Ed25519_WORD_LEN << 1];

    uint32_t pub_x[Ed25519_WORD_LEN], *pub_y = S;
    uint32_t *x = PH_M, *y = PH_M + Ed25519_WORD_LEN;

    hash_ctx_t sha512_ctx[1];
    uint32_t ret;
    uint8_t phflag, tmp;

    if (mode > Ed25519_PH) {
        return EdDSA_INVALID_INPUT;
    }
    else if (NULL == pubkey || NULL == RS) {
        return EdDSA_POINTOR_NULL;
    }
    else
    {;}

    /*M could be empty, so M could be NUll, MByteLen could be 0, no need to check them*/
    if (NULL == M) {
        MByteLen = 0;
    }
    else
    {;}

    if (Ed25519_CTX == mode) {
        /*in this case ctx can not be empty*/
        if (NULL == ctx || 0 == ctxByteLen) {
            return EdDSA_INVALID_INPUT;
        }
        else
        {;}
    }
    else if (Ed25519_PH == mode) {
        /*in this case ctx could be empty*/
        if (NULL == ctx) {
            ctxByteLen = 0;
        }
        else
        {;}
    }
    else/*Ed25519_DEFAULT mode, ctx is useless*/
    {;}

    /*get S (S should be less than order of the base point)*/
    memcpy_(S, RS + Ed25519_BYTE_LEN, Ed25519_BYTE_LEN);

    if (uint32_bignumcmp(S, Ed25519_WORD_LEN, ed25519->n, Ed25519_WORD_LEN) >= 0) {
        return EdDSA_INVALID_INPUT;
    }
    else
    {;}

    /************************* set flag F **************************/
    if (Ed25519_CTX == mode) {
        phflag = 0;
    }
    else if (Ed25519_PH == mode) {
        phflag = 1;
    }
    else
    {;}

    /*PH_M*/
    if (Ed25519_PH == mode) {
        ret = hash(HASH_SHA512, M, MByteLen, (uint8_t *)PH_M);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /******* get k = SHA-512(dom2(F, C) || R || A || PH(M)) ********/
    ret = hash_init(sha512_ctx, HASH_SHA512);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*dom2(phflag, ctx)*/
    if (Ed25519_DEFAULT != mode) {
        tmp = strlen(Ed25519_sign_string);
        ret = hash_update(sha512_ctx, (uint8_t *)Ed25519_sign_string, tmp);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&phflag, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, (uint8_t *)&ctxByteLen, 1);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = hash_update(sha512_ctx, ctx, ctxByteLen);

        if (HASH_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*R*/
    ret = hash_update(sha512_ctx, RS, Ed25519_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*pubkey(A)*/
    ret = hash_update(sha512_ctx, pubkey, Ed25519_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*PH(M)*/
    if (Ed25519_PH == mode) {
        ret = hash_update(sha512_ctx, (uint8_t *)PH_M, 64);
    }
    else {
        ret = hash_update(sha512_ctx, M, MByteLen);
    }

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(sha512_ctx, (uint8_t *)k);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*k = k mod n*/
    ret = pke_mod(k + Ed25519_WORD_LEN - 1, Ed25519_WORD_LEN + 1, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, x);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    uint32_copy(k + Ed25519_WORD_LEN - 1, x, Ed25519_WORD_LEN);
    ret = pke_mod(k, (Ed25519_WORD_LEN << 1) - 1, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, x);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else {
        uint32_copy(k, x, Ed25519_WORD_LEN);
    }

    /*get [S]B*/
    ret = ed25519_pointMul_s((edward_curve_t *)ed25519, S, ed25519->Gx, ed25519->Gy, x, y);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*get [k]A'*/
    ret = ed25519_decode_point((edward_curve_t *)ed25519, (uint8_t *)pubkey, (uint8_t *)pub_x, (uint8_t *)pub_y);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = ed25519_pointMul_s((edward_curve_t *)ed25519, k, pub_x, pub_y, pub_x, pub_y);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*get R*/
    ret = ed25519_decode_point((edward_curve_t *)ed25519, (uint8_t *)RS, (uint8_t *)k, (uint8_t *)(k + Ed25519_WORD_LEN));

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*R + [k]A'*/
    ret = ed25519_pointAdd((edward_curve_t *)ed25519, k, k + Ed25519_WORD_LEN, pub_x, pub_y, k, k + Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*check whether [S]B = R + [k]A\A1\AF*/
    if (uint32_bignumcmp(k, Ed25519_WORD_LEN, x, Ed25519_WORD_LEN) ||
            uint32_bignumcmp(k + Ed25519_WORD_LEN, Ed25519_WORD_LEN, y, Ed25519_WORD_LEN)) {
        return EdDSA_VERIFY_FAIL;
    }
    else {
        return EdDSA_SUCCESS;
    }
}

#endif
