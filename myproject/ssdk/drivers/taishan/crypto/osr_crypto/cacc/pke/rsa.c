/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <ral/pke_prime.h>
#include <ral/trng.h>
#include <ral/pke.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/rsa.h>

/* function: out = a^e mod n
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n
 *     e -------------------------- input, uint32_t big integer e, exeponent, make sure e < n
 *     n -------------------------- input, uint32_t big integer n, modulus, make sure n is odd
 *     out ------------------------ output, out = a^e mod n
 *     eBitLen  ------------------- input, real bit length of uint32_t big integer e
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 */
uint32_t RSA_ModExp(uint32_t *a, uint32_t *e, uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t eWordLen = GET_WORD_LEN(eBitLen);
    uint32_t nWordLen = GET_WORD_LEN(nBitLen);
    int32_t ret;

    if (NULL == a || NULL == e || NULL == n || NULL == out) {
        return RSA_BUFFER_NULL;
    }
    else if (nBitLen > MAX_RSA_BIT_LEN || eBitLen > nBitLen) {
        return RSA_INPUT_TOO_LONG;
    }
    else if (!(n[0] & 1)) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    /*a should be in [0,n]*/
    ret = uint32_bignumcmp(a, nWordLen, n, nWordLen);

    if (ret > 0) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    /*if a is 0 or n*/
    if ((0 == ret) || (1 == uint32_bignum_check_zero(a, nWordLen))) {
        if (uint32_bignum_check_zero(e, eWordLen)) {
            /*0^0 mod n*/
            return RSA_INPUT_INVALID;
        }
        else {
            /*if a is 0, e is not 0, the output is 0*/
            uint32_clear(out, nWordLen);
            return RSA_SUCCESS;
        }
    }
    else if (uint32_bignum_check_zero(e, eWordLen)) {
        /*a is in [1,n-1], e is 0, the output is 1*/
        uint32_clear(out, nWordLen);
        *out = 1;
        return RSA_SUCCESS;
    }
    else
    {;}

    ret = pke_pre_calc_mont(n, nBitLen, NULL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    return pke_modexp(n, e, a, out, nWordLen, eWordLen);
}

/* function: out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n=pq
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     out ------------------------ output, out = a^d mod n
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. a and out have the same word length:((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:((nBitLen/2+31)>>5)
 */
uint32_t RSA_CRTModExp(uint32_t *a, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u,
                       uint32_t *out,  uint32_t nBitLen)
{
    uint32_t buf[MAX_RSA_WORD_LEN];
    uint32_t *m1 = buf;
    uint32_t *m2 = buf + (MAX_RSA_WORD_LEN / 2);

    uint32_t tmp_step;
    uint32_t nWordLen = GET_WORD_LEN(nBitLen);
    uint32_t pBitLen = nBitLen / 2;
    uint32_t pWordLen = GET_WORD_LEN(pBitLen);
    int32_t ret;

    if (NULL == a || NULL == p || NULL == q || NULL == dp || NULL == dq
            || NULL == u || NULL == out) {
        return RSA_BUFFER_NULL;
    }
    else if (nBitLen > MAX_RSA_BIT_LEN || (nBitLen & 1)) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    /*get n = p*q*/
    ret = pke_mul(p, q, buf, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*a should be in [0,n]*/
    ret = uint32_bignumcmp(a, nWordLen, buf, nWordLen);

    if (ret > 0) {
        return RSA_INPUT_INVALID;
    }
    else if ((0 == ret) || (1 == uint32_bignum_check_zero(a, nWordLen))) {
        /*if a is 0 or n, the output is 0*/
        uint32_clear(out, nWordLen);
        return RSA_SUCCESS;
    }
    else
    {;}

    /*store the nBitLen step*/
    pke_set_operand_width(pBitLen);
    tmp_step = pke_get_operand_bytes();

    /*do pke_pre_calc_mont() first, because a may be less than p or q, then pke_mod() will not*/
    /*call pke_pre_calc_mont() inside, but pke_modexp() needs the output of pke_pre_calc_mont().*/

    /*m2 = (a) mod q*/
    ret = pke_pre_calc_mont(q, pBitLen, NULL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = pke_mod(a, nWordLen, q, (uint32_t *)(PKE_B(0, tmp_step)), pWordLen, m2);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*m2 = (a)^dq mod q*/
    ret = pke_modexp(q, dq, m2, m2, pWordLen, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*m1 = (a) mod p*/
    ret = pke_pre_calc_mont(p, pBitLen, NULL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = pke_mod(a, nWordLen, p, (uint32_t *)(PKE_B(0, tmp_step)), pWordLen, m1);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*m1 = (a)^dp mod p*/
    ret = pke_modexp(p, dp, m1, m1, pWordLen, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*m2 = m2 mod p*/
    if (uint32_bignumcmp(m2, pWordLen, p, pWordLen) > 0) {
        ret = pke_sub(m2, p, m2, pWordLen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    /*m1 = (m1-m2) mod p*/
    ret = pke_modsub(p, m1, m2, m1, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*m1 = h = u*(m1-m2) mod p*/
    ret = pke_modmul_internal(p, m1, u, m1, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*store the nBitLen step*/
    pke_set_operand_width(nBitLen);
    tmp_step = pke_get_operand_bytes();

    /*A1 = hq*/
    ret = pke_mul(m1, q, (uint32_t *)(PKE_A(1, tmp_step)), pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*out = m2+hq*/
    uint32_copy((uint32_t *)(PKE_B(1, tmp_step)), m2, pWordLen);
    uint32_clear((uint32_t *)(PKE_B(1, tmp_step)) + pWordLen, nWordLen - pWordLen);
    return pke_add((uint32_t *)(PKE_A(1, tmp_step)), (uint32_t *)(PKE_B(1, tmp_step)), out, nWordLen);
}

/* function: get big odd integer e of eBitLen
 * parameters:
 *     e -------------------------- input, uint32_t big odd integer e
 *     eBitLen  ------------------- input, bit length of uint32_t big odd integer e
 * return: 0(success), 1(error: eBitLen<2)
 * caution:
 *     1. eBitLen must be big than 1
 */
uint32_t RSA_Get_E1(uint32_t e[], uint32_t eBitLen)
{
    uint32_t eWordLen = (eBitLen + 0x1F) >> 5;
    uint32_t ret;

    if (eBitLen < 2) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    ret = get_rand((uint8_t *)e, eWordLen << 2);

    if (TRNG_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    eBitLen &= 31;

    if (eBitLen) {
        e[eWordLen - 1] &= (1 << (eBitLen)) - 1;
        e[eWordLen - 1] |= 1 << (eBitLen - 1);
    }
    else {
        e[eWordLen - 1] |= 0x80000000;
    }
    /*make e odd*/
    e[0] |= 0x01;

    return 0;
}

/* function: get big odd integer e of eBitLen, satisfies e < fai_n of bitLen
 * parameters:
 *     e -------------------------- input, uint32_t big odd integer e
 *     fai_n ---------------------- input, uint32_t big even integer fai_n
 *     bitLen   ------------------- input, bit length of uint32_t big odd integer e and n
 * return: 0(success), 1(error: bitLen<66), 2(error, n is 1000000000...000000)
 * caution:
 *     1. eBitLen must be big than 65
 *     2. n can not be 1000000000...000000
 */
uint32_t RSA_Get_E2(uint32_t e[], uint32_t fai_n[], uint32_t bitLen)
{
    uint32_t wordLen;
    int32_t i;
    uint8_t j;

    if (bitLen < 66) {
        return 1;
    }

    RSA_Get_E1(e, bitLen);
    wordLen = (bitLen + 0x1F) >> 5;
    /* namely j = eBitLen%32;*/
    j = bitLen & 31;

    if (j == 0) {
        j = 32;
    }

    j--;
    i = wordLen - 1;

    if (j == 0) {
        i--;
        j = 32;
    }

    while (i >= 0) {

        e[i] &= (~(1 << (j - 1)));

        if (uint32_bignumcmp(e, i + 1, fai_n, i + 1) < 0) {
            /*if e < n*/
            return 0;
        }

        j--;

        if (0 == j) {
            i--;
            j = 32;
        }
    }

    return 2;         /*fail, because n is 1000000000...000000*/
}

/* function: judge whether big integer a is equal to 0x5a5a5a5a5a...5a or not
 * parameters:
 *     a -------------------------- input, uint32_t big integer a
 *     aBitLen -------------------- input, real bit length of a
 * return: 0(a==0x5a5a5a5a5a...5a), 1(a!=0x5a5a5a5a5a...5a)
 * caution:
 *     1. aBitLen can not be 0
 *     2. if aBitLen%32 != 0, then the highest word of a should be 0
 */
uint32_t CheckValue_0x5a5a5a5a(uint32_t a[], uint32_t aBitLen)
{
    uint32_t i, wordLen = aBitLen >> 5;

    if (aBitLen & 0x1F) {
        if (a[wordLen] != 0) {
            return 1;
        }
        else
        {;}
    }
    else
    {;}

    for (i = 0; i < wordLen; i++) {
        if (a[i] != 0x5a5a5a5a) {
            return 1;
        }
        else
        {;}
    }

    return 0;
}

/* function: generate RSA key (e,d,n)
 * parameters:
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     d -------------------------- output, uint32_t big integer, RSA private key d
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. nBitLen can not be even
 *     2. eBitLen must be larger than 1, and less than or equal to nBitLen
 */
uint32_t RSA_GetKey(uint32_t *e, uint32_t *d, uint32_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t buf[MAX_RSA_WORD_LEN];
    uint32_t *p, *q, *in, *out;
    uint32_t pBitLen, pWordLen, eWordLen, nWordLen, tmpLen;
    uint32_t count, flag;

    if (NULL == e || NULL == d || NULL == n) {
        return RSA_BUFFER_NULL;
    }
    else if (nBitLen & 1 || nBitLen < MIN_RSA_BIT_LEN || nBitLen > MAX_RSA_BIT_LEN) {
        /*nBitLen can not be odd*/
        return RSA_INPUT_INVALID;
    }
    else if (eBitLen < 2 || eBitLen > nBitLen) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    p = buf;
    q = buf + MAX_RSA_WORD_LEN / 2;

    pke_set_operand_width(nBitLen);
    tmpLen = pke_get_operand_bytes();
    in = (uint32_t *)(PKE_B(1, tmpLen));
    out = (uint32_t *)(PKE_A(1, tmpLen));

    eWordLen = GET_WORD_LEN(eBitLen);
    nWordLen = GET_WORD_LEN(nBitLen);
    pBitLen = nBitLen >> 1;
    pWordLen = GET_WORD_LEN(pBitLen);

GET_PQ:

    flag = get_prime(p, pBitLen);

    if (flag) {
        return flag;
    }
    else
    {;}

    flag = get_prime(q, pBitLen);

    if (flag) {
        return flag;
    }
    else
    {;}

    p[0]--;
    q[0]--;
    /* get fai(n)=(p-1)(q-1)*/
    pke_mul(p, q, n, pWordLen);

    count = 0;
GET_E:
    count++;

    if (count == 7) {
        goto GET_PQ;
    }
    else
    {;}

    switch (eBitLen) {
        case 2 :  {
            e[0] = 3;
            break;
        }

        case 5 :  {
            e[0] = 17;
            break;
        }

        case 17:  {
            e[0] = 65537;
            break;
        }

        default: {
            if (eBitLen == nBitLen) {
                flag = RSA_Get_E2(e, n, eBitLen);

                if (flag) {
                    return flag;
                }
                else
                {;}
            }
            else {
                flag = RSA_Get_E1(e, eBitLen);

                if (flag) {
                    return flag;
                }
                else
                {;}
            }

            break;
        }
    }

    /*get d = e^(-1) mod n*/
    flag = pke_modinv(n, e, d, nWordLen, eWordLen);

    if (PKE_NO_MODINV == flag) {
        /*if d doesn't exist*/
        if (eBitLen == 2 || eBitLen == 5 || eBitLen == 17) {
            /*if e is prime, and e divide fai(n)*/
            goto GET_PQ;
        }
        else {
            /*1. e is prime, and e divide fai(n) 2.e is not prime, and*/
            /*e, fai(n) have common divisor.*/
            goto GET_E;
        }
    }
    else if (PKE_SUCCESS != flag) {
        return flag;
    }
    else
    {;}

    /*get n = pq*/
    p[0]++;
    q[0]++;
    flag = pke_mul(p, q, n, pWordLen);

    if (PKE_SUCCESS != flag) {
        return flag;
    }
    else
    {;}

    /*Encryption test*/
    if (nBitLen & 0x1F) {
        in[nWordLen - 1] = 0;
    }
    else
    {;}

    tmpLen = nBitLen >> 5;
    uint32_set(in, 0x5a5a5a5a, tmpLen);

    flag = pke_pre_calc_mont(n, nBitLen, NULL);

    if (PKE_SUCCESS != flag) {
        return flag;
    }
    else
    {;}

    flag = pke_modexp(n, e, in, out, nWordLen, eWordLen);

    if (PKE_SUCCESS != flag) {
        return flag;
    }
    else
    {;}

    flag = pke_modexp(n, d, out, out, nWordLen, nWordLen);

    if (PKE_SUCCESS != flag) {
        return flag;
    }
    else
    {;}

    if (CheckValue_0x5a5a5a5a(out, nBitLen)) {
        goto GET_PQ;
    }
    else {
        return RSA_SUCCESS;
    }
}

/* Function: generate RSA-CRT key (e,p,q,dp,dq,u,n)
 * Parameters:
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     p -------------------------- output, uint32_t big integer, RSA private key p
 *     q -------------------------- output, uint32_t big integer, RSA private key q
 *     dp-------------------------- output, uint32_t big integer, RSA private key dp
 *     dq-------------------------- output, uint32_t big integer, RSA private key dq
 *     u -------------------------- output, uint32_t big integer, RSA private key u = q^(-1) mod p
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * Return: RSA_SUCCESS(success), other(error)
 * Caution:
 *     1. nBitLen can not be even
 *     2. eBitLen must be larger than 1, and less than or equal to nBitLen
 */
uint32_t RSA_GetCRTKey(uint32_t *e, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u,
                       uint32_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t buf[MAX_RSA_WORD_LEN];
    uint32_t pBitLen, pWordLen, eWordLen, nWordLen, i, wordLen;
    int32_t count;
    uint32_t ret;

    if (NULL == e || NULL == p || NULL == q || NULL == dp || NULL == dq || NULL == u || NULL == n) {
        return RSA_BUFFER_NULL;
    }
    else if ((nBitLen & 1) || nBitLen < MIN_RSA_BIT_LEN || nBitLen > MAX_RSA_BIT_LEN) {
        /*nBitLen can not be odd*/
        return RSA_INPUT_INVALID;
    }
    else if (eBitLen < 2 || eBitLen > nBitLen) {
        return RSA_INPUT_INVALID;
    }
    else
    {;}

    eWordLen = GET_WORD_LEN(eBitLen);
    nWordLen = GET_WORD_LEN(nBitLen);
    pBitLen = nBitLen >> 1;
    pWordLen = GET_WORD_LEN(pBitLen);

GET_PQ:

    ret = get_prime(p, pBitLen);

    if (ret) {
        return ret;
    }
    else
    {;}

    ret = get_prime(q, pBitLen);

    if (ret) {
        return ret;
    }
    else
    {;}
    /* make p > q, for get u = q^(-1) mod p convenient*/
    count = uint32_bignumcmp(p, pWordLen, q, pWordLen);

    if (count == -1) {
        for (i = 0; i < pWordLen; i++) {
            wordLen = p[i];
            p[i] = q[i];
            q[i] = wordLen;
        }
    }
    else if (count == 0) {
        goto GET_PQ;
    }
    else
    {;}

    p[0]--;
    q[0]--;

    if (eBitLen == nBitLen) {
        ret = pke_mul(p, q, n, pWordLen);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

    count = 0;
GET_E:
    count++;

    if (count == 7) {
        goto GET_PQ;
    }
    else
    {;}

    switch (eBitLen) {
        case 2 :  {
            e[0] = 3;
            break;
        }

        case 5 :  {
            e[0] = 17;
            break;
        }

        case 17:  {
            e[0] = 65537;
            break;
        }

        default: {
            if (eBitLen == nBitLen) {
                ret = RSA_Get_E2(e, n, eBitLen);

                if (ret) {
                    return ret;
                }
                else
                {;}
            }
            else {
                ret = RSA_Get_E1(e, eBitLen);

                if (ret) {
                    return ret;
                }
                else
                {;}
            }

            break;
        }
    }

    /* dp = e^(-1) mod (p-1)*/
    if (uint32_bignumcmp(e, eWordLen, p, pWordLen) > 0) {
        ret = pke_mod(e, eWordLen, p, NULL, pWordLen, u);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        wordLen = pWordLen;
    }
    else {
        uint32_copy(u, e, eWordLen);
        wordLen = eWordLen;
    }

    ret = pke_modinv(p, u, dp, pWordLen, wordLen);

    if (PKE_NO_MODINV == ret) {
        if (eBitLen == 2 || eBitLen == 5 || eBitLen == 17) {
            /*if e is prime, and e divide fai(n)*/
            goto GET_PQ;
        }
        else {
            goto GET_E;
        }
    }
    else if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* dq = e^(-1) mod (q-1)*/
    if (uint32_bignumcmp(e, eWordLen, q, pWordLen) > 0) {
        ret = pke_mod(e, eWordLen, q, NULL, pWordLen, u);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        wordLen = pWordLen;
    }
    else {
        uint32_copy(u, e, eWordLen);
        wordLen = eWordLen;
    }

    ret = pke_modinv(q, u, dq, pWordLen, wordLen);

    if (PKE_NO_MODINV == ret) {
        if (eBitLen == 2 || eBitLen == 5 || eBitLen == 17) {
            goto GET_PQ;
        }
        else {
            goto GET_E;
        }
    }
    else if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    p[0]++;
    q[0]++;

    /* u = q^(-1) mod p*/
    ret = pke_modinv(p, q, u, pWordLen, pWordLen);

    if (PKE_NO_MODINV == ret) {
        goto GET_PQ;
    }
    else if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* get n*/
    ret = pke_mul(p, q, n, pWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /*Encryption test*/
    if (nBitLen & 0x1F) {
        buf[nWordLen - 1] = 0;
    }
    else
    {;}

    wordLen = nBitLen >> 5;
    uint32_set(buf, 0x5a5a5a5a, wordLen);

    ret = pke_pre_calc_mont(n, nBitLen, NULL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = pke_modexp(n, e, buf, buf, nWordLen, eWordLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (!CheckValue_0x5a5a5a5a(buf, nBitLen)) {
        goto GET_PQ;
    }
    else
    {;}

    ret = RSA_CRTModExp(buf, p, q, dp, dq, u, buf, nBitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (CheckValue_0x5a5a5a5a(buf, nBitLen)) {
        goto GET_PQ;
    }
    else {
        return RSA_SUCCESS;
    }
}

