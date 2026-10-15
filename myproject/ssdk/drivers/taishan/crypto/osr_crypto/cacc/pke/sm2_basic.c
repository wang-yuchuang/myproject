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

#define SM2_DEFAULT_ID_BYTE_LEN         (16)
static const char g_sm2_default_id[] = "1234567812345678";

/*SM2 algorithm parameters*/
const uint32_t sm2p256v1_p[8]    = {0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
const uint32_t sm2p256v1_p_h[8]  = {0x00000003, 0x00000002, 0xFFFFFFFF, 0x00000002, 0x00000001, 0x00000001, 0x00000002, 0x00000004};
const uint32_t sm2p256v1_a[8]    = {0xFFFFFFFC, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
const uint32_t sm2p256v1_b[8]    = {0x4D940E93, 0xDDBCBD41, 0x15AB8F92, 0xF39789F5, 0xCF6509A7, 0x4D5A9E4B, 0x9D9F5E34, 0x28E9FA9E};
const uint32_t sm2p256v1_Gx[8]   = {0x334C74C7, 0x715A4589, 0xF2660BE1, 0x8FE30BBF, 0x6A39C994, 0x5F990446, 0x1F198119, 0x32C4AE2C};
const uint32_t sm2p256v1_Gy[8]   = {0x2139F0A0, 0x02DF32E5, 0xC62A4740, 0xD0A9877C, 0x6B692153, 0x59BDCEE3, 0xF4F6779C, 0xBC3736A2};
const uint32_t sm2p256v1_n[8]    = {0x39D54123, 0x53BBF409, 0x21C6052B, 0x7203DF6B, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
const uint32_t sm2p256v1_n_h[8]  = {0x7C114F20, 0x901192AF, 0xDE6FA2FA, 0x3464504A, 0x3AFFE0D4, 0x620FC84C, 0xA22B3D3B, 0x1EB5E412};

/*SM2 para (n-1), for private key checking*/
const uint32_t sm2p256v1_n_1[8]  = {0x39D54122, 0x53BBF409, 0x21C6052B, 0x7203DF6B, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};

/*[2^128]G, for [k]G of high speed*/
const uint32_t sm2p256v1_2_128_G_x[8] = {0xD13A42ED, 0xEAE3D9A9, 0x484E1B38, 0x2B2308F6, 0x88C21F3A, 0x3DB7B248, 0x74D55DA9, 0xB692E5B5};
const uint32_t sm2p256v1_2_128_G_y[8] = {0xE295E5AB, 0xD186469D, 0x73438E6D, 0xDB61AC17, 0x544926F9, 0x5A924F85, 0x0F3FB613, 0xA175051B};

const eccp_curve_t sm2_curve[1] = {
    {
        256,
        256,
        (uint32_t *)sm2p256v1_p,
        (uint32_t *)sm2p256v1_p_h,
        (uint32_t *)sm2p256v1_a,
        (uint32_t *)sm2p256v1_b,
        (uint32_t *)sm2p256v1_Gx,
        (uint32_t *)sm2p256v1_Gy,
        (uint32_t *)sm2p256v1_n,
        (uint32_t *)sm2p256v1_n_h,
        (uint32_t *)sm2p256v1_2_128_G_x,
        (uint32_t *)sm2p256v1_2_128_G_y,
    }
};
/* function: a=a+1 (for 1+dA in SM2 signing)
 * parameters:
 *     a -------------------------- input, destination data
 *     wordLen -------------------- input, word length of data
 * return: none
 * caution:
 *     1. if a of wordLen words can not hold the carry, then the carry will be discarded,
 *        actually this is used in sm2 signing(1+dA)
 */
void uint32_BigNum_Add_One(uint32_t *a, uint32_t wordLen)
{
    uint32_t i, carry;

    carry = 1;

    for (i = 0; i < wordLen; i++) {
        a[i] += carry;

        if (a[i] < carry) {
            carry = 1;
        }
        else {
            break;
        }
    }
}

/* function: a=a+1 (for SM2 KDF counter addition)
 * parameters:
 *     a[4] ----------------------- input, count of 4 bytes, big-endian
 * return: none
 * caution: if a of 4 bytes can not hold the carry, then the carry will be discarded
 */
void sm2_kdf_counter_add_one(uint8_t a[4])
{
    int32_t i;
    uint8_t carry;

    carry = 1;

    for (i = 3; i >= 0; i--) {
        a[i] += carry;

        if (a[i] < carry) {
            carry = 1;
        }
        else {
            break;
        }
    }
}

/* function: SM2 kdf (for SM2 encrypting, decrypting and key exchange)
 * parameters:
 *     in ------------------------- input, sm2 kdf input
 *     inByteLen ------------------ input, byte length of in
 *     m -------------------------- input, message, if no message, please set this para as NULL
 *     k -------------------------- output, k = kdf(in) if m is NULL, or k = (kdf(in) XOR m) if m is available
 *     kByteLen ------------------- input, byte length of input m and output k
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1.
 */
uint32_t sm2_kdf_with_xor(uint8_t *in, uint32_t inByteLen, uint8_t *m, uint8_t *k, uint32_t kByteLen)
{
    uint8_t *k_buf = k;
    uint8_t digest[SM3_DIGEST_BYTE_LEN];
    uint32_t i, t;
    uint8_t counter[4] = {0x00, 0x00, 0x00, 0x01};
    hash_ctx_t ctx[1];
    uint8_t zero_check = 0;
    uint32_t ret;
    /* t = kByteLen/32;*/
    t = kByteLen >> 5;

    for (i = 0; i < t; i++) {
        ret = hash_init(ctx, HASH_SM3);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, in, inByteLen);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, counter, 4);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        if (NULL == m) {
            ret = hash_final(ctx, k_buf);

            if (HASH_SUCCESS != ret) {
                goto end;
            }
            else
            {;}

            zero_check |= !uint8_bignum_check_zero(k_buf, SM2_BYTE_LEN);
        }
        else {
            ret = hash_final(ctx, digest);

            if (HASH_SUCCESS != ret) {
                goto end;
            }
            else
            {;}

            zero_check |= !uint8_bignum_check_zero(digest, SM2_BYTE_LEN);

            uint8_xor(m, digest, k_buf, SM2_BYTE_LEN);
            m += SM2_BYTE_LEN;
        }

        k_buf += SM2_BYTE_LEN;

        sm2_kdf_counter_add_one(counter);
    }

    t = kByteLen & 0x1F;

    if (t) {
        ret = hash_init(ctx, HASH_SM3);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, in, inByteLen);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_update(ctx, counter, 4);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        ret = hash_final(ctx, digest);

        if (HASH_SUCCESS != ret) {
            goto end;
        }
        else
        {;}

        zero_check |= !uint8_bignum_check_zero(digest, t);

        if (NULL == m) {
            memcpy_(k_buf, digest, t);
        }
        else {
            uint8_xor(m, digest, k_buf, t);
        }
    }

    if (0 == zero_check) {
        ret = SM2_ZERO_ALL;
        goto end;
    }
    else {
        ret = SM2_SUCCESS;
    }

end:

    if (SM2_SUCCESS != ret) {
        memset_(k, 0, kByteLen);
    }
    else
    {;}

    return ret;
}

/* function: get SM2 Z value = SM3(bitLenofID||ID||a||b||Gx||Gy||Px||Py)
 * parameters:
 *     ID ------------------------- input, User ID
 *     byteLenofID ---------------- input, byte length of ID, must be less than 2^13
 *     pubKey --------------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     Z -------------------------- output, Z value, SM3 digest, 32 bytes
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. bit length of ID must be less than 2^16, thus byte length must be less than 2^13
 *     2. if ID is NULL, then replace it with sm2 default ID
 *     3. please make sure the pubKey is valid
 */
uint32_t sm2_getZ(uint8_t *ID, uint32_t byteLenofID, uint8_t pubKey[65], uint8_t Z[32])
{
    uint32_t tmp[SM2_WORD_LEN];
    hash_ctx_t ctx[1];
    uint32_t ret;
    uint8_t tmp_u8;

    if (NULL == pubKey || NULL == Z) {
        return SM2_BUFFER_NULL;
    }
    else if (POINT_NOT_COMPRESSED != pubKey[0]) {
        return SM2_INPUT_INVALID;
    }
    else if (byteLenofID >= SM2_MAX_ID_BYTE_LEN) {
        return SM2_INPUT_INVALID;
    }
    else if ((NULL == ID) || (0 == byteLenofID)) {
        ID = (uint8_t *)g_sm2_default_id;
        byteLenofID = SM2_DEFAULT_ID_BYTE_LEN;
    }
    else
    {;}

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    byteLenofID <<= 3;
    tmp_u8 = (byteLenofID >> 8) & 0xFF;
    ret = hash_update(ctx, (uint8_t *)&tmp_u8, 1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    tmp_u8 = byteLenofID & 0xFF;
    ret = hash_update(ctx, (uint8_t *)&tmp_u8, 1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    byteLenofID >>= 3;
    ret = hash_update(ctx, ID, byteLenofID);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)sm2p256v1_a, tmp, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)sm2p256v1_a, (uint8_t *)tmp, SM2_BYTE_LEN);
#endif

    ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)sm2p256v1_b, tmp, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)sm2p256v1_b, (uint8_t *)tmp, SM2_BYTE_LEN);
#endif

    ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)sm2p256v1_Gx, tmp, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)sm2p256v1_Gx, (uint8_t *)tmp, SM2_BYTE_LEN);
#endif

    ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

#ifdef PKE_BIG_ENDIAN
    convert_word_array((uint8_t *)sm2p256v1_Gy, tmp, SM2_WORD_LEN);
#else
    reverse_byte_array((uint8_t *)sm2p256v1_Gy, (uint8_t *)tmp, SM2_BYTE_LEN);
#endif

    ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, pubKey + 1, SM2_BYTE_LEN << 1);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_final(ctx, Z);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = SM2_SUCCESS;

end:

    return ret;
}

/* function: get SM2 E value = SM3(Z||M)
 * parameters:
 *     M      --------------------- input, Message
 *     byteLen -------------------- input, byte length of M
 *     Z      --------------------- input, Z value, 32 bytes
 *     E      --------------------- output, E value, 32 bytes
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_getE(uint8_t *M, uint32_t byteLen, uint8_t Z[32], uint8_t E[32])
{
    hash_ctx_t ctx[1];
    uint32_t ret;

    if (NULL == M || NULL == Z || NULL == E) {
        return SM2_BUFFER_NULL;
    }
    else if (0 == byteLen) {
        return SM2_INPUT_INVALID;
    }
    else
    {;}

    ret = hash_init(ctx, HASH_SM3);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, Z, 32);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_update(ctx, M, byteLen);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = hash_final(ctx, E);

    if (HASH_SUCCESS != ret) {
        goto end;
    }
    else
    {;}

    ret = SM2_SUCCESS;

end:

    return ret;
}

/* function: Generate SM2 public key from private key
 * parameters:
 *     priKey --------------------- input, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_get_pubkey_from_prikey(uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

    if (NULL == priKey || NULL == pubKey) {
        return SM2_BUFFER_NULL;
    }
    else
    {;}

    ret = eccp_get_pubkey_from_prikey((eccp_curve_t *)sm2_curve, priKey, pubKey + 1);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else {
        pubKey[0] = POINT_NOT_COMPRESSED;

        return SM2_SUCCESS;
    }
}

/* function: Generate SM2 random Key pair
 * parameters:
 *     priKey --------------------- output, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

    if (NULL == priKey || NULL == pubKey) {
        return SM2_BUFFER_NULL;
    }
    else
    {;}

    ret = eccp_getkey((eccp_curve_t *)sm2_curve, priKey, pubKey + 1);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else {
        pubKey[0] = POINT_NOT_COMPRESSED;

        return SM2_SUCCESS;
    }

}

#endif
