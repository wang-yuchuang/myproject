/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <ral/pke.h>

#ifdef SUPPORT_SM2
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hash.h>
#include <ral/trng.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/sm2.h>

/* function: Generate SM2 Signature r and s with rand k
 * parameters:
 *     e[8]   --------------------- input, e value, 8 words, little-endian
 *     k[8]   --------------------- input, random number k, 8 words, little-endian
 *     dA[8]  --------------------- input, private key, 8 words, little-endian
 *     r[8]   --------------------- output, Signature r, 8 words, little-endian
 *     s[8]   --------------------- output, Signature s, 8 words, little-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. e and dA can not be modified
 *     2. e must be less than n(order of the SM2 curve)
 *     3. dA must be in [1, n-2]
 */
uint32_t sm2_sign_with_k(uint32_t e[8], uint32_t k[8], uint32_t dA[8], uint32_t r[8], uint32_t s[8])
{
    uint32_t tmp1[SM2_WORD_LEN], tmp2[SM2_WORD_LEN];
    uint32_t ret;

    if (NULL == e || NULL == k || NULL == dA || NULL == r || NULL == s) {
        return SM2_BUFFER_NULL;
    }
    else
    {;}

    /*make sure k in [1, n-1]*/
    if (uint32_bignum_check_zero(k, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else if (uint32_bignumcmp(k, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        return SM2_INTEGER_TOO_BIG;
    }
    else
    {;}

#ifdef SM2_HIGH_SPEED
    ret = eccp_pointMul_base((eccp_curve_t *)sm2_curve, k, tmp1, NULL);
#else
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, k, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, tmp1, NULL);
#endif

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*tmp1 = x1 mod n*/
    if (uint32_bignumcmp(tmp1, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = pke_sub(tmp1, (uint32_t *)sm2p256v1_n, tmp1, SM2_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*r = e + x1 mod n*/
    ret = pke_modadd((uint32_t *)sm2p256v1_n, e, tmp1, r, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*make sure r is not zero*/
    if (uint32_bignum_check_zero(r, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else
    {;}

    /*tmp1 = r + k mod n*/
    ret = pke_modadd((uint32_t *)sm2p256v1_n, r, k, tmp1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else if (uint32_bignum_check_zero(tmp1, SM2_WORD_LEN)) {
        /*make sure r+k is not n*/
        return SM2_ZERO_ALL;
    }
    else
    {;}

    /*tmp1 =  r*dA mod n*/
    pke_load_pre_calc_mont((uint32_t *)sm2p256v1_n_h, SM2_WORD_LEN);
    ret = pke_modmul_internal((uint32_t *)sm2p256v1_n, r, dA, tmp1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*tmp1 =  (k - r*dA) mod n*/
    ret = pke_modsub((uint32_t *)sm2p256v1_n, k, tmp1, tmp1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*tmp2 = (1+dA)^(-1) mod n*/
    uint32_copy(tmp2, dA, SM2_WORD_LEN);
    uint32_BigNum_Add_One(tmp2, SM2_WORD_LEN);
    ret = pke_modinv((uint32_t *)sm2p256v1_n, tmp2, tmp2, SM2_WORD_LEN, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*s = ((1+dA)^(-1))*(k - r*dA) mod n*/
    ret = pke_modmul_internal((uint32_t *)sm2p256v1_n, tmp1, tmp2, s, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*make sure s is not zero*/
    if (uint32_bignum_check_zero(s, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else {
        return SM2_SUCCESS;
    }
}

/* function: Generate SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     rand_k[32] ----------------- input, random big integer k in signing, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     signature[64] -------------- output, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 */
uint32_t sm2_sign(uint8_t E[32], uint8_t rand_k[32], uint8_t priKey[32], uint8_t signature[64])
{
    uint32_t e[SM2_WORD_LEN], k[SM2_WORD_LEN], dA[SM2_WORD_LEN], r[SM2_WORD_LEN], s[SM2_WORD_LEN];
    uint32_t ret;

    if (NULL == E || NULL == priKey || NULL == signature) {
        return SM2_BUFFER_NULL;
    }
    else
    {;}

    /*e = e mod n*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(E, e, SM2_WORD_LEN);
#else
    reverse_byte_array(E, (uint8_t *)e, SM2_BYTE_LEN);
#endif

    if (uint32_bignumcmp(e, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = pke_sub(e, (uint32_t *)sm2p256v1_n, e, SM2_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*make sure priKey in [1, n-2]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(priKey, dA, SM2_WORD_LEN);
#else
    reverse_byte_array(priKey, (uint8_t *)dA, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(dA, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else if (uint32_bignumcmp(dA, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n_1, SM2_WORD_LEN) >= 0) {
        return SM2_INTEGER_TOO_BIG;
    }
    else
    {;}

    if (rand_k) {
        reverse_byte_array(rand_k, (uint8_t *)k, SM2_BYTE_LEN);
    }
    else {
SM2_SIGN_LOOP:
        ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);

        if (TRNG_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }

    ret = sm2_sign_with_k(e, k, dA, r, s);

    if ((SM2_ZERO_ALL == ret || SM2_INTEGER_TOO_BIG == ret) && (NULL == rand_k)) {
        goto SM2_SIGN_LOOP;
    }
    else
    {;}

    if (SM2_SUCCESS != ret) {
        return ret;
    }
    else {
#ifdef PKE_BIG_ENDIAN
        if (((uint32_t)(signature)) & 3) {
            convert_word_array((uint8_t *)r, r, SM2_WORD_LEN);
            convert_word_array((uint8_t *)s, s, SM2_WORD_LEN);
            memcpy_(signature, r, SM2_BYTE_LEN);
            memcpy_(signature + SM2_BYTE_LEN, s, SM2_BYTE_LEN);
        }
        else {
            convert_word_array((uint8_t *)r, (uint32_t *)signature, SM2_WORD_LEN);
            convert_word_array((uint8_t *)s, (uint32_t *)(signature + SM2_BYTE_LEN), SM2_WORD_LEN);
        }

#else
        reverse_byte_array((uint8_t *)r, signature, SM2_BYTE_LEN);
        reverse_byte_array((uint8_t *)s, signature + SM2_BYTE_LEN, SM2_BYTE_LEN);
#endif

        return SM2_SUCCESS;
    }
}

/* function: Verify SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     pubKey[65] ----------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     signature[64] -------------- input, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success, the signature is valid); other(error or the signature is invalid)
 * caution:
 */
uint32_t sm2_verify(uint8_t E[32], uint8_t pubKey[65], uint8_t signature[64])
{
    uint32_t e[SM2_WORD_LEN], r[SM2_WORD_LEN], s[SM2_WORD_LEN], tmp[SM2_WORD_LEN * 4];
    uint32_t *t = e;
    uint32_t ret;

    if (NULL == E || NULL == pubKey || NULL == signature) {
        return SM2_BUFFER_NULL;
    }
    else if (POINT_NOT_COMPRESSED != pubKey[0]) {
        /*make sure pubKey[0] is POINT_NOT_COMPRESSED*/
        return SM2_INPUT_INVALID;
    }
    else
    {;}

    /*get PA and check PA*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(pubKey + 1, tmp + 2 * SM2_WORD_LEN, SM2_WORD_LEN);
    convert_word_array(pubKey + 1 + SM2_BYTE_LEN, tmp + 3 * SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array(pubKey + 1, (uint8_t *)(tmp + 2 * SM2_WORD_LEN), SM2_BYTE_LEN);
    reverse_byte_array(pubKey + 1 + SM2_BYTE_LEN, (uint8_t *)(tmp + 3 * SM2_WORD_LEN), SM2_BYTE_LEN);
#endif
    ret = eccp_pointVerify((eccp_curve_t *)sm2_curve, (uint32_t *)(tmp + 2 * SM2_WORD_LEN), (uint32_t *)(tmp + 3 * SM2_WORD_LEN));

    if (PKE_SUCCESS != ret) {
        return SM2_NOT_ON_CURVE;
    }
    else
    {;}

    /*make sure r in [1, n-1]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(signature, r, SM2_WORD_LEN);
#else
    reverse_byte_array(signature, (uint8_t *)r, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(r, SM2_WORD_LEN)) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else if (uint32_bignumcmp(r, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = SM2_INTEGER_TOO_BIG;
        goto end;
    }
    else
    {;}

    /*make sure s in [1, n-1]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(signature + SM2_BYTE_LEN, s, SM2_WORD_LEN);
#else
    reverse_byte_array(signature + SM2_BYTE_LEN, (uint8_t *)s, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(s, SM2_WORD_LEN)) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else if (uint32_bignumcmp(s, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = SM2_INTEGER_TOO_BIG;
        goto end;
    }
    else
    {;}

    /*t = (r+s) mod n*/
    ret = pke_modadd((uint32_t *)sm2p256v1_n, r, s, t, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*if t is 0, refuse the signature*/
    if (uint32_bignum_check_zero(t, SM2_WORD_LEN)) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else
    {;}

#ifdef SM2_HIGH_SPEED
    ret = eccp_pointMul_Shamir_safe((eccp_curve_t *)sm2_curve,
                                    s, (uint32_t *)sm2p256v1_Gx, (uint32_t *)sm2p256v1_Gy,
                                    t, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN,
                                    tmp, NULL);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#else
    /*[s]G*/
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, s, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, tmp, tmp + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*[t]PA*/
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, t, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN, tmp + 2 * SM2_WORD_LEN,
                        tmp + 3 * SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*[s]G + [t]PA*/
    ret = eccp_pointAdd((eccp_curve_t *)sm2_curve, tmp, tmp + SM2_WORD_LEN, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN,
                        tmp, NULL);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#endif

    /*e = e mod n*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(E, e, SM2_WORD_LEN);
#else
    reverse_byte_array(E, (uint8_t *)e, SM2_BYTE_LEN);
#endif

    if (uint32_bignumcmp(e, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = pke_sub(e, (uint32_t *)sm2p256v1_n, e, SM2_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }
    else
    {;}

    /*tmp = x1 mod n*/
    if (uint32_bignumcmp(tmp, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        ret = pke_sub(tmp, (uint32_t *)sm2p256v1_n, tmp, SM2_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }
    else
    {;}

    /*tmp = e + x1 mod n*/
    ret = pke_modadd((uint32_t *)sm2p256v1_n, e, tmp, tmp, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*cmp*/
    if (uint32_bignumcmp(tmp, SM2_WORD_LEN, r, SM2_WORD_LEN)) {
        ret = SM2_VERIFY_FAILED;
        goto end;
    }
    else
    {;}

    /*success*/
    ret = SM2_SUCCESS;

end:

    return ret;
}

/* function: SM2 Encryption with rand k
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     pubkey_x ------------------- input, x coordinate of public key point, 8 words, little-endian
 *     pubkey_y ------------------- input, y coordinate of public key point, 8 words, little-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. please make sure pubkey_x and pubkey_y are valid
 */
uint32_t sm2_encrypt_with_k(uint8_t *M, uint32_t MByteLen, uint32_t *k,
                            uint32_t *pubkey_x, uint32_t *pubkey_y,
                            sm2_cipher_order_e order,
                            uint8_t *C, uint32_t *CByteLen)
{
    uint32_t xy[SM2_WORD_LEN << 1];
    uint8_t *C2, *C3;
    int32_t i;
    hash_ctx_t ctx[1];
    uint32_t ret;

    if (NULL == M || NULL == k || NULL == pubkey_x || NULL == pubkey_y || NULL == C || NULL == CByteLen) {
        return SM2_BUFFER_NULL;
    }
    else if (MByteLen == 0) {
        return SM2_INPUT_INVALID;
    }
    else if (order > SM2_C1C2C3) {
        return SM2_INPUT_INVALID;
    }
    else
    {;}

    C2 = C + 1 + 2 * SM2_BYTE_LEN + ((SM2_C1C2C3 == order) ? 0 : SM2_BYTE_LEN);
    C3 = C + 1 + 2 * SM2_BYTE_LEN + ((SM2_C1C2C3 == order) ? MByteLen : 0);

    /*not support M and C crossing, but support M = C*/
    if (M > C) {
        if (C + MByteLen + 1 + 3 * SM2_BYTE_LEN > M) {
            return SM2_INPUT_INVALID;
        }
        else
        {;}
    }
    else if (M < C) {
        if (M + MByteLen > C) {
            return SM2_INPUT_INVALID;
        }
        else
        {;}
    }
    else {
        /*move M to C2, and now M = C2*/
        for (i = MByteLen - 1; i >= 0; i--) {
            C2[i] = M[i];
        }

        M = C2;
    }

    /*make sure k in [1, n-1]*/
    if (uint32_bignum_check_zero(k, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else if (uint32_bignumcmp(k, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n, SM2_WORD_LEN) >= 0) {
        return SM2_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*get [k]G*/
#ifdef SM2_HIGH_SPEED
    ret = eccp_pointMul_base((eccp_curve_t *)sm2_curve, k, xy, xy + SM2_WORD_LEN);
#else
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, k, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, xy, xy + SM2_WORD_LEN);
#endif

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*output C1*/
    C[0] = POINT_NOT_COMPRESSED;
#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)xy, xy, SM2_WORD_LEN);
    convert_word_array((uint8_t *)(xy + SM2_WORD_LEN), xy + SM2_WORD_LEN, SM2_WORD_LEN);
    memcpy_(C + 1, xy, SM2_BYTE_LEN);
    memcpy_(C + 1 + SM2_BYTE_LEN, xy + SM2_WORD_LEN, SM2_BYTE_LEN);
#else
    reverse_byte_array((uint8_t *)xy, C + 1, SM2_BYTE_LEN);
    reverse_byte_array((uint8_t *)(xy + SM2_WORD_LEN), C + 1 + SM2_BYTE_LEN, SM2_BYTE_LEN);
#endif

    /*get [k]PB*/
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, k, pubkey_x, pubkey_y, xy, xy + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*get x2||y2*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)xy, xy, SM2_WORD_LEN);
    convert_word_array((uint8_t *)(xy + SM2_WORD_LEN), xy + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)xy, (uint8_t *)xy, SM2_BYTE_LEN);
    reverse_byte_array((uint8_t *)(xy + SM2_WORD_LEN), (uint8_t *)(xy + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif

    /*get C3*/
    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)xy, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, M, MByteLen);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)(xy + SM2_WORD_LEN), SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(ctx, C3);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*get C2*/
    ret = sm2_kdf_with_xor((uint8_t *)xy, SM2_BYTE_LEN << 1, M, C2, MByteLen);

    if (SM2_SUCCESS != ret) {
        return ret;
    }
    else {
        *CByteLen = MByteLen + 1 + 3 * SM2_BYTE_LEN;

        return SM2_SUCCESS;
    }
}

/* function: SM2 Encryption
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     rand_k[32] ----------------- input, random big integer k in encrypting, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     pubKey[65] ----------------- input, public key, 65 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 *     3. please make sure pubKey is valid
 */
uint32_t sm2_encrypt(uint8_t *M, uint32_t MByteLen, uint8_t rand_k[32], uint8_t pubKey[65],
                     sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t k[SM2_WORD_LEN];
    uint32_t pubkey_x[SM2_WORD_LEN], pubkey_y[SM2_WORD_LEN];
    uint32_t ret;

    if (NULL == pubKey) {
        return SM2_BUFFER_NULL;
    }
    else if (POINT_NOT_COMPRESSED != pubKey[0]) {
        return SM2_INPUT_INVALID;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array(pubKey + 1, pubkey_x, SM2_WORD_LEN);
    convert_word_array(pubKey + 1 + SM2_BYTE_LEN, pubkey_y, SM2_WORD_LEN);
#else
    reverse_byte_array(pubKey + 1, (uint8_t *)pubkey_x, SM2_BYTE_LEN);
    reverse_byte_array(pubKey + 1 + SM2_BYTE_LEN, (uint8_t *)pubkey_y, SM2_BYTE_LEN);
#endif
    ret = eccp_pointVerify((eccp_curve_t *)sm2_curve, pubkey_x, pubkey_y);

    if (PKE_SUCCESS != ret) {
        return SM2_NOT_ON_CURVE;
    }
    else
    {;}

    if (rand_k) {
        reverse_byte_array(rand_k, (uint8_t *)k, SM2_BYTE_LEN);
    }
    else {
SM2_ENCRYPT_LOOP:

        ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);

        if (TRNG_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }

    /*encrypt*/
    ret = sm2_encrypt_with_k(M, MByteLen, k, pubkey_x, pubkey_y, order, C, CByteLen);

    if ((SM2_ZERO_ALL == ret || SM2_INTEGER_TOO_BIG == ret) && (NULL == rand_k)) {
        goto SM2_ENCRYPT_LOOP;
    }
    else
    {;}

    return ret;
}

/* function: SM2 Decryption
 * parameters:
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- input, byte length of C, make sure MByteLen>97
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     M -------------------------- output, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- output, byte length of M, should be CByteLen-97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 */
uint32_t sm2_decrypt(uint8_t *C, uint32_t CByteLen, uint8_t priKey[32],
                     sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen)
{
    uint32_t i, temLen;
    uint32_t dA[SM2_WORD_LEN], xy[SM2_WORD_LEN << 1];
    uint8_t digest[SM2_BYTE_LEN];
    uint8_t C3_buf[SM2_BYTE_LEN];
    uint8_t *C2, *C3;
    hash_ctx_t ctx[1];
    uint32_t ret;

    if (NULL == C || NULL == priKey || NULL == M || NULL == MByteLen) {
        return SM2_BUFFER_NULL;
    }
    else if (CByteLen <= 1 + 3 * SM2_BYTE_LEN) {
        return SM2_INPUT_INVALID;
    }
    else if (order > SM2_C1C2C3) {
        return SM2_INPUT_INVALID;
    }
    else
    {;}

    temLen = CByteLen - 1 - (3 * SM2_BYTE_LEN);

    C2 = C + 1 + 2 * SM2_BYTE_LEN + ((SM2_C1C2C3 == order) ? 0 : SM2_BYTE_LEN);
    C3 = C + 1 + 2 * SM2_BYTE_LEN + ((SM2_C1C2C3 == order) ? temLen : 0);

    /*not support M and C crossing, but support M = C*/
    if (M > C) {
        if (C + CByteLen > M) {
            return SM2_INPUT_INVALID;
        }
        else
        {;}
    }
    else if (M < C) {
        if (M + temLen > C) {
            return SM2_INPUT_INVALID;
        }
        else
        {;}
    }
    else
    {;}

    /*make sure C1 is on the SM2 curve*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(C + 1, xy, SM2_WORD_LEN);
    convert_word_array(C + 1 + SM2_BYTE_LEN, xy + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array(C + 1, (uint8_t *)xy, SM2_BYTE_LEN);
    reverse_byte_array(C + 1 + SM2_BYTE_LEN, (uint8_t *)(xy + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif
    ret = eccp_pointVerify((eccp_curve_t *)sm2_curve, xy, xy + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return SM2_NOT_ON_CURVE;
    }
    else
    {;}

    if (M == C) {
        /*keep C3*/
        memcpy_(C3_buf, C3, SM2_BYTE_LEN);
        C3 = C3_buf;

        /*move C2 to M, and now M = C2*/
        for (i = 0; i < temLen; i++) {
            M[i] = C2[i];
        }

        C2 = M;
    }
    else
    {;}

    /*make sure priKey in [1, n-2]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(priKey, dA, SM2_WORD_LEN);
#else
    reverse_byte_array(priKey, (uint8_t *)dA, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(dA, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else if (uint32_bignumcmp(dA, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n_1, SM2_WORD_LEN) >= 0) {
        return SM2_INTEGER_TOO_BIG;
    }
    else
    {;}

    /* [dA]C1 */
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, dA, xy, xy + SM2_WORD_LEN, xy, xy + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)xy, xy, SM2_WORD_LEN);
    convert_word_array((uint8_t *)(xy + SM2_WORD_LEN), xy + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)xy, (uint8_t *)xy, SM2_BYTE_LEN);
    reverse_byte_array((uint8_t *)(xy + SM2_WORD_LEN), (uint8_t *)(xy + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif

    ret = sm2_kdf_with_xor((uint8_t *)xy, SM2_BYTE_LEN << 1, C2, M, temLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)xy, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, M, temLen);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)(xy + SM2_WORD_LEN), SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(ctx, digest);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (memcmp_(C3, digest, SM2_BYTE_LEN)) {
        return SM2_DECRYPT_VERIFY_FAILED;
    }
    else {
        *MByteLen = temLen;
        return SM2_SUCCESS;
    }
}

/* function: SM2 Key Exchange
 * parameters:
 *     role        ------ input, SM2_ROLE_SPONSOR - sponsor, SM2_Role_Responsor - responsor
 *     dA[32]      ------ input, sponsor's permanent private key
 *     PB[65]      ------ input, responsor's permanent public key
 *     rA[32]      ------ input, sponsor's temporary private key
 *     RA[65]      ------ input, sponsor's temporary public key
 *     RB[65]      ------ input, responsor's temporary public key
 *     ZA[32]      ------ input, sponsor's Z value
 *     ZB[32]      ------ input, responsor's Z value
 *     kByteLen    ------ input, byte length of output key, should be less than (2^32 - 1)bit
 *     KA[kByteLen]------ output, output key
 *     S1[32]      ------ output, sponsor's S1, or responsor's S2, this is optional
 *     SA[32]      ------ output, sponsor's SA, or responsor's SB, this is optional
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution: *
 *     1. please make sure the inputs are valid
 *     2. S1 and SA are optional, if you don't need, please set S1 and SA as NULL
 *     3. in case that S1(S2) and SA(SB) exist, if S1=SB,S2=SA, then exchange success.
 */
uint32_t sm2_exchangekey(sm2_exchange_role_e role,
                         uint8_t *dA, uint8_t *PB,
                         uint8_t *rA, uint8_t *RA,
                         uint8_t *RB,
                         uint8_t *ZA, uint8_t *ZB,
                         uint32_t kByteLen,
                         uint8_t *KA, uint8_t *S1, uint8_t *SA)
{
    uint32_t x1[SM2_WORD_LEN], t1[SM2_WORD_LEN], tmp[SM2_WORD_LEN << 2];
    hash_ctx_t ctx[1];
    uint32_t ret;

    if (NULL == dA || NULL == PB || NULL == rA || NULL == RA || NULL == RB) {
        return SM2_BUFFER_NULL;
    }
    else if (NULL == ZA || NULL == ZB || NULL == KA) {
        return SM2_BUFFER_NULL;
    }
    else if (role > SM2_ROLE_RESPONSOR) {
        return SM2_EXCHANGE_ROLE_INVALID;
    }
    else if (0 == kByteLen) {
        return SM2_INPUT_INVALID;
    }
    else if ((POINT_NOT_COMPRESSED != PB[0]) || (POINT_NOT_COMPRESSED != RA[0]) || (POINT_NOT_COMPRESSED != RB[0])) {
        return SM2_INPUT_INVALID;
    }
    else
    {;}

    uint32_clear(x1 + SM2_WORD_LEN / 2, SM2_WORD_LEN / 2);
#ifdef PKE_BIG_ENDIAN
    convert_word_array(RA + 1 + (SM2_BYTE_LEN / 2), x1, SM2_WORD_LEN / 2);
#else
    reverse_byte_array(RA + 1 + (SM2_BYTE_LEN / 2), (uint8_t *)x1, SM2_BYTE_LEN / 2);
#endif
    x1[(SM2_WORD_LEN / 2) - 1] |= 0x80000000;

    /*make sure rA in [1, n-2]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(rA, t1, SM2_WORD_LEN);
#else
    reverse_byte_array(rA, (uint8_t *)t1, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(t1, SM2_WORD_LEN)) {
        return SM2_ZERO_ALL;
    }
    else if (uint32_bignumcmp(t1, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n_1, SM2_WORD_LEN) >= 0) {
        return SM2_INTEGER_TOO_BIG;
    }
    else
    {;}

    /*t1 = x1*rA mod n*/
    pke_load_pre_calc_mont((uint32_t *)sm2p256v1_n_h, SM2_WORD_LEN);
    ret = pke_modmul_internal((uint32_t *)sm2p256v1_n, x1, t1, t1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*make sure dA in [1, n-2]*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(dA, x1, SM2_WORD_LEN);
#else
    reverse_byte_array(dA, (uint8_t *)x1, SM2_BYTE_LEN);
#endif

    if (uint32_bignum_check_zero(x1, SM2_WORD_LEN)) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else if (uint32_bignumcmp(x1, SM2_WORD_LEN, (uint32_t *)sm2p256v1_n_1, SM2_WORD_LEN) >= 0) {
        ret = SM2_INTEGER_TOO_BIG;
        goto end;
    }
    else
    {;}

    /*t1 = (dA + x1*rA) mod n, and it must not be 0*/
    ret = pke_modadd((uint32_t *)sm2p256v1_n, t1, x1, t1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else if (uint32_bignum_check_zero(t1, SM2_WORD_LEN)) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else
    {;}

    /*make sure RB on the SM2 curve*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(RB + 1, tmp + 2 * SM2_WORD_LEN, SM2_WORD_LEN);
    convert_word_array(RB + 1 + SM2_BYTE_LEN, tmp + 3 * SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array(RB + 1, (uint8_t *)(tmp + 2 * SM2_WORD_LEN), SM2_BYTE_LEN);
    reverse_byte_array(RB + 1 + SM2_BYTE_LEN, (uint8_t *)(tmp + 3 * SM2_WORD_LEN), SM2_BYTE_LEN);
#endif
    ret = eccp_pointVerify((eccp_curve_t *)sm2_curve, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return SM2_NOT_ON_CURVE;
    }
    else
    {;}

    uint32_clear(x1 + SM2_WORD_LEN / 2, SM2_WORD_LEN / 2);
#ifdef PKE_BIG_ENDIAN
    convert_word_array(RB + 1 + (SM2_BYTE_LEN / 2), x1, SM2_WORD_LEN / 2);
#else
    reverse_byte_array(RB + 1 + (SM2_BYTE_LEN / 2), (uint8_t *)x1, SM2_BYTE_LEN / 2);
#endif
    x1[(SM2_WORD_LEN / 2) - 1] |= 0x80000000;

#ifdef SM2_HIGH_SPEED
    /*x1 = tA*x2 mod n*/
    pke_load_pre_calc_mont((uint32_t *)sm2p256v1_n_h, SM2_WORD_LEN);
    ret = pke_modmul_internal((uint32_t *)sm2p256v1_n, t1, x1, x1, SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*get PB point and verify*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(PB + 1, tmp, SM2_WORD_LEN);
    convert_word_array(PB + 1 + SM2_BYTE_LEN, tmp + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array(PB + 1, (uint8_t *)(tmp), SM2_BYTE_LEN);
    reverse_byte_array(PB + 1 + SM2_BYTE_LEN, (uint8_t *)(tmp + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif
    ret = eccp_pointVerify((eccp_curve_t *)sm2_curve, tmp, tmp + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*[tA]PB +[tA*x2 mod n]RB*/
    ret = eccp_pointMul_Shamir_safe((eccp_curve_t *)sm2_curve,
                                    t1, tmp, tmp + SM2_WORD_LEN,
                                    x1, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN,
                                    tmp, tmp + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#else
    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, x1, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*get PB point(caution: do not delete this)*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array(PB + 1, tmp, SM2_WORD_LEN);
    convert_word_array(PB + 1 + SM2_BYTE_LEN, tmp + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array(PB + 1, (uint8_t *)(tmp), SM2_BYTE_LEN);
    reverse_byte_array(PB + 1 + SM2_BYTE_LEN, (uint8_t *)(tmp + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif

    ret = eccp_pointAdd((eccp_curve_t *)sm2_curve, tmp, tmp + SM2_WORD_LEN, tmp + 2 * SM2_WORD_LEN, tmp + 3 * SM2_WORD_LEN,
                        tmp, tmp + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = eccp_pointMul((eccp_curve_t *)sm2_curve, t1, tmp, tmp + SM2_WORD_LEN, tmp, tmp + SM2_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#endif

    /*xU||yU*/
#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)tmp, tmp, SM2_WORD_LEN);
    convert_word_array((uint8_t *)(tmp + SM2_WORD_LEN), tmp + SM2_WORD_LEN, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)tmp, (uint8_t *)tmp, SM2_BYTE_LEN);
    reverse_byte_array((uint8_t *)(tmp + SM2_WORD_LEN), (uint8_t *)(tmp + SM2_WORD_LEN), SM2_BYTE_LEN);
#endif

    if (SM2_ROLE_SPONSOR == role) {
        memcpy_(tmp + 2 * SM2_WORD_LEN, ZA, SM2_BYTE_LEN);
        memcpy_(tmp + 3 * SM2_WORD_LEN, ZB, SM2_BYTE_LEN);
    }
    else {
        memcpy_(tmp + 2 * SM2_WORD_LEN, ZB, SM2_BYTE_LEN);
        memcpy_(tmp + 3 * SM2_WORD_LEN, ZA, SM2_BYTE_LEN);
    }

    /*KA*/
    ret = sm2_kdf_with_xor((uint8_t *)tmp, SM2_BYTE_LEN << 2, NULL, KA, kByteLen);

    if (SM2_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    /*check value is optional*/
    if (NULL == S1 || NULL == SA) {
        ret = SM2_SUCCESS;
        goto end;
    }
    else
    {;}

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    if (SM2_ROLE_SPONSOR == role) {
        ret = hash_update(ctx, ZA, SM2_BYTE_LEN);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, ZB, SM2_BYTE_LEN);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, RA + 1, SM2_BYTE_LEN << 1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, RB + 1, SM2_BYTE_LEN << 1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }
    else {
        ret = hash_update(ctx, ZB, SM2_BYTE_LEN);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, ZA, SM2_BYTE_LEN);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, RB + 1, SM2_BYTE_LEN << 1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, RA + 1, SM2_BYTE_LEN << 1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }

    ret = hash_final(ctx, (uint8_t *)t1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    *(((uint8_t *)(tmp + SM2_WORD_LEN)) - 1) = 0x03;
    ret = hash_update(ctx, ((uint8_t *)(tmp + SM2_WORD_LEN)) - 1, SM2_BYTE_LEN + 1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)t1, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    if (SM2_ROLE_SPONSOR == role) {
        ret = hash_final(ctx, SA);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }
    else {
        ret = hash_final(ctx, S1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    *(((uint8_t *)(tmp + SM2_WORD_LEN)) - 1) = 0x02;
    ret = hash_update(ctx, ((uint8_t *)(tmp + SM2_WORD_LEN)) - 1, SM2_BYTE_LEN + 1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, (uint8_t *)t1, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    if (SM2_ROLE_SPONSOR == role) {
        ret = hash_final(ctx, S1);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }
    else {
        ret = hash_final(ctx, SA);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}
    }

    ret = SM2_SUCCESS;

end:

    return ret;
}

#endif
