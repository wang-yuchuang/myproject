/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <ral/utility.h>
#include <ral/trng.h>
#include <ral/pke.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/ecdsa.h>

/* function: Generate ECDSA Signature in U32 little-endian big integer style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     e -------------------------- input, derived from hash value
 *     k -------------------------- input, internal random integer k
 *     dA ------------------------- input, private key
 *     r -------------------------- output, signature r
 *     s -------------------------- output, signature s
 * return:
 *     ECDSA_SUCCESS(success); other(error)
 * caution:
 *     1. please make sure e is in [0,n-1], dA is in [1,n-1]
 */
uint32_t ecdsa_sign_uint32(eccp_curve_t *curve, uint32_t *e, uint32_t *k, uint32_t *dA, uint32_t *r, uint32_t *s)
{
    uint32_t nWordLen;
    uint32_t pWordLen;
    uint32_t tmp1[ECCP_MAX_WORD_LEN];
    uint32_t ret;

    if (NULL == curve || NULL == e || NULL == k || NULL == dA || NULL == r || NULL == s) {
        return ECDSA_POINTOR_NULL;
    }
    else if (curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN) {
        return ECDSA_INVALID_INPUT;
    }
    else
    {;}

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

    /*make sure k in [1, n-1]*/
    if (uint32_bignum_check_zero(k, nWordLen)) {
        return ECDSA_ZERO_ALL;
    }
    else if (uint32_bignumcmp(k, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        return ECDSA_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*get x1*/
    if (curve->eccp_half_Gx && curve->eccp_half_Gy) {
        ret = eccp_pointMul_base(curve, k, tmp1, NULL);
    }
    else {
        /*y coordinate is not needed*/
        ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, tmp1, NULL);
    }

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*r = x1 mod n*/
    ret = pke_mod(tmp1, pWordLen, curve->eccp_n, curve->eccp_n_h, nWordLen, r);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else if (uint32_bignum_check_zero(r, nWordLen)) {
        /*make sure r is not zero*/
        return ECDSA_ZERO_ALL;
    }
    else
    {;}

    /*tmp1 =  r*dA mod n*/
    if (NULL == curve->eccp_n_h) {
        ret = pke_modmul(curve->eccp_n, r, dA, tmp1, nWordLen);
    }
    else {
        pke_load_pre_calc_mont(curve->eccp_n_h, nWordLen);
        ret = pke_modmul_internal(curve->eccp_n, r, dA, tmp1, nWordLen);
    }

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*tmp1 = e + r*dA mod n*/
    ret = pke_modadd(curve->eccp_n, e, tmp1, tmp1, nWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*s = k^(-1) mod n*/
    ret = pke_modinv(curve->eccp_n, k, s, nWordLen, nWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*s = (k^(-1))*(e + r*dA) mod n*/
    ret = pke_modmul_internal(curve->eccp_n, s, tmp1, s, nWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*make sure s is not zero*/
    if (uint32_bignum_check_zero(s, nWordLen)) {
        return ECDSA_ZERO_ALL;
    }
    else {
        return ECDSA_SUCCESS;
    }
}

/* function: Generate ECDSA Signature in byte string style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     E -------------------------- input, hash value, big-endian
 *     EByteLen ------------------- input, byte length of E
 *     rand_k --------------------- input, random big integer k in signing, big-endian
 *     priKey --------------------- input, private key, big-endian
 *     signature ------------------ output, signature r and s, big-endian
 * return:
 *     ECDSA_SUCCESS(success); other(error)
 * caution:
 *     1. the method of getting big integer e from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_sign(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *rand_k, uint8_t *priKey,
                    uint8_t *signature)
{
    uint32_t tmpLen;
    uint32_t nByteLen;
    uint32_t nWordLen;
    uint32_t e[ECCP_MAX_WORD_LEN], k[ECCP_MAX_WORD_LEN], dA[ECCP_MAX_WORD_LEN];
    uint32_t r[ECCP_MAX_WORD_LEN], s[ECCP_MAX_WORD_LEN];
    uint32_t ret;

    if (NULL == curve || NULL == priKey || NULL == signature) {
        return ECDSA_POINTOR_NULL;
    }
    else if (curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN) {
        return ECDSA_INVALID_INPUT;
    }
    else
    {;}

    /*E could be zero*/
    if (NULL == E) {
        EByteLen = 0;
    }
    else
    {;}

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    /*get integer e from hash value E(according to SEC1-V2 2009)*/
    uint32_clear(e, nWordLen);
    if(NULL != E){
        if (curve->eccp_n_bitLen >= (EByteLen << 3)) {
            /*in this case, make E as e directly*/
            reverse_byte_array((uint8_t *)E, (uint8_t *)e, EByteLen);
        }
        else {
            /*in this case, make left eccp_n_bitLen bits of E as e*/
            reverse_byte_array((uint8_t *)E, (uint8_t *)e, nByteLen);
            tmpLen = (curve->eccp_n_bitLen) & 7;

            if (tmpLen) {
                big_div2n(e, nWordLen, 8 - tmpLen);
            }
            else
            {;}
        }
    }
    /*get e = e mod n, i.e., make sure e in [0, n-1]*/
    if (uint32_bignumcmp(e, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        ret = pke_sub(e, curve->eccp_n, e, nWordLen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*make sure priKey in [1, n-1]*/
    memset_(((uint8_t *)dA) + nByteLen, 0, (nWordLen << 2) - nByteLen);
    reverse_byte_array((uint8_t *)priKey, (uint8_t *)dA, nByteLen);

    if (uint32_bignum_check_zero(dA, nWordLen)) {
        return ECDSA_ZERO_ALL;
    }
    else if (uint32_bignumcmp(dA, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        return ECDSA_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*get k*/
    memset_(((uint8_t *)k) + nByteLen, 0, (nWordLen << 2) - nByteLen);

    if (rand_k) {
        reverse_byte_array(rand_k, (uint8_t *)k, nByteLen);
    }
    else {
ECDSA_SIGN_LOOP:

        ret = get_rand((uint8_t *)k, nByteLen);

        if (TRNG_SUCCESS != ret) {
            return ret;
        }
        else {
            /*make sure k has the same bit length as n*/
            tmpLen = (curve->eccp_n_bitLen) & 0x1F;

            if (tmpLen) {
                k[nWordLen - 1] &= (1 << (tmpLen)) - 1;
            }
            else
            {;}
        }
    }

    /*sign*/
    ret = ecdsa_sign_uint32(curve, e, k, dA, r, s);

    if ((ECDSA_ZERO_ALL == ret || ECDSA_INTEGER_TOO_BIG == ret) && (NULL == rand_k)) {
        goto ECDSA_SIGN_LOOP;
    }
    else
    {;}

    if (ECDSA_SUCCESS != ret) {
        return ret;
    }
    else {
        reverse_byte_array((uint8_t *)r, signature, nByteLen);
        reverse_byte_array((uint8_t *)s, signature + nByteLen, nByteLen);

        return ECDSA_SUCCESS;
    }
}

/* function: Verify ECDSA Signature in byte string style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     E -------------------------- input, hash value, big-endian
 *     EByteLen ------------------- input, byte length of E
 *     pubKey --------------------- input, public key, big-endian
 *     signature ------------------ input, signature r and s, big-endian
 * return:
 *     ECDSA_SUCCESS(success); other(error)
 * caution:
 *     1. the method of getting big integer e from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_verify(eccp_curve_t *curve, uint8_t *E, uint32_t EByteLen, uint8_t *pubKey, uint8_t *signature)
{
    uint32_t tmpLen;
    uint32_t nByteLen;
    uint32_t nWordLen;
    uint32_t pByteLen;
    uint32_t pWordLen;
    uint32_t maxWordLen;
    uint32_t e[ECCP_MAX_WORD_LEN], r[ECCP_MAX_WORD_LEN], s[ECCP_MAX_WORD_LEN];
    uint32_t tmp[ECCP_MAX_WORD_LEN], x[ECCP_MAX_WORD_LEN];
    uint32_t ret;

    if (NULL == curve || NULL == pubKey || NULL == signature) {
        return ECDSA_POINTOR_NULL;
    }
    else if (curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN) {
        return ECDSA_INVALID_INPUT;
    }
    else
    {;}

    /*E could be zero*/
    if (NULL == E) {
        EByteLen = 0;
    }
    else
    {;}

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    maxWordLen = GET_MAX_LEN(nWordLen, pWordLen);

    /*make sure r in [1, n-1]*/
    memset_(((uint8_t *)r) + nByteLen, 0, (nWordLen << 2) - nByteLen);
    reverse_byte_array(signature, (uint8_t *)r, nByteLen);

    if (uint32_bignum_check_zero(r, nWordLen)) {
        return ECDSA_ZERO_ALL;
    }
    else if (uint32_bignumcmp(r, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        return ECDSA_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*make sure s in [1, n-1]*/
    memset_(((uint8_t *)s) + nByteLen, 0, (nWordLen << 2) - nByteLen);
    reverse_byte_array(signature + nByteLen, (uint8_t *)s, nByteLen);

    if (uint32_bignum_check_zero(s, nWordLen)) {
        return ECDSA_ZERO_ALL;
    }
    else if (uint32_bignumcmp(s, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        return ECDSA_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*tmp = s^(-1) mod n*/
    ret = pke_modinv(curve->eccp_n, s, tmp, nWordLen, nWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*get integer e from hash value E(according to SEC1-V2 2009)*/
    uint32_clear(e, nWordLen);

    if(NULL != E){
        if (curve->eccp_n_bitLen >= (EByteLen << 3)) {
            /*in this case, make E as e directly*/
            reverse_byte_array((uint8_t *)E, (uint8_t *)e, EByteLen);
        }
        else {
            /*in this case, make left eccp_n_bitLen bits of E as e*/
            memcpy_(e, E, nByteLen);
            reverse_byte_array((uint8_t *)E, (uint8_t *)e, nByteLen);
            tmpLen = (curve->eccp_n_bitLen) & 7;

            if (tmpLen) {
                big_div2n(e, nWordLen, 8 - tmpLen);
            }
            else
            {;}
        }
    }
    /*get e = e mod n, i.e., make sure e in [0, n-1]*/
    if (uint32_bignumcmp(e, nWordLen, curve->eccp_n, nWordLen) >= 0) {
        ret = pke_sub(e, curve->eccp_n, e, nWordLen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*x =  e*(s^(-1)) mod n*/
    if (NULL == curve->eccp_n_h) {
        ret = pke_modmul(curve->eccp_n, e, tmp, x, nWordLen);
    }
    else {
        pke_load_pre_calc_mont(curve->eccp_n_h, nWordLen);
        ret = pke_modmul_internal(curve->eccp_n, e, tmp, x, nWordLen);
    }

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*tmp =  r*(s^(-1)) mod n*/
    ret = pke_modmul_internal(curve->eccp_n, r, tmp, tmp, nWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*check public key*/
    memset_(e, 0, (maxWordLen << 2) - pByteLen);
    memset_(s, 0, (maxWordLen << 2) - pByteLen);
    reverse_byte_array(pubKey, (uint8_t *)e, pByteLen);
    reverse_byte_array(pubKey + pByteLen, (uint8_t *)s, pByteLen);
    ret = eccp_pointVerify(curve, e, s);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (curve->eccp_half_Gx && curve->eccp_half_Gy) {
        ret = eccp_pointMul_Shamir(curve,
                                   tmp, e, s,
                                   x, curve->eccp_Gx, curve->eccp_Gy,
                                   e, s);
    }
    else {
        ret = ~(PKE_SUCCESS);
    }

    if (PKE_SUCCESS != ret) {
        ret = eccp_pointMul(curve, tmp, e, s, e, s);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        if (!uint32_bignum_check_zero(x, nWordLen)) {
            ret = eccp_pointMul(curve, x, curve->eccp_Gx, curve->eccp_Gy, x, tmp);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}

            ret = eccp_pointAdd(curve, e, s, x, tmp, e, s);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else
        {;}
    }
    else
    {;}

    /*x = x1 mod n*/
    ret = pke_mod(e, pWordLen, curve->eccp_n, curve->eccp_n_h, nWordLen, tmp);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (uint32_bignumcmp(tmp, nWordLen, r, nWordLen)) {
        return ECDSA_VERIFY_FAILED;
    }
    else {
        return ECDSA_SUCCESS;
    }
}

