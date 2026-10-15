/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include "utility.h"

#define LOCAL_TRACE 1

#ifdef PKE_PRINT_BUF

void print_buf_U8(uint8_t buf[], uint32_t byteLen, char name[])
{
    uint32_t i;

    printf("\r\n %s: ", name);
    printf(" addr:0x%x:", (uint32_t)buf);

    for (i = 0; i < byteLen; i++) {
        printf("%02x", buf[i]);
    }

    printf("\r\n");
}

void print_buf_U32(uint32_t buf[], uint32_t wordLen, char name[])
{
    uint32_t i;

    OSR_LTRACEF("\r\n %s: %08x\r\n", name, (uint32_t)buf);

    for (i = 0; i < wordLen; i++) {
        printf("%08x", buf[i]);
    }

    OSR_LTRACEF("\r\n");
}

void print_BN_buf_U32(uint32_t buf[], uint32_t wordLen, char name[])
{
    uint32_t i;

    OSR_LTRACEF("\r\n %08x %s: ", (uint32_t)buf, name);

    for (i = 0; i < wordLen; i++) {
        OSR_LTRACEF("%08x", buf[wordLen - 1 - i]);
    }

    OSR_LTRACEF("\r\n");
}
#endif


void memcpy_(void *dst, void *src, uint32_t size)
{
    uint8_t *a = (uint8_t *)dst;
    uint8_t *b = (uint8_t *)src;

    uint32_t *aa = (uint32_t *)dst;
    uint32_t *bb = (uint32_t *)src;
    uint32_t i, count, tmp;

    if ((((uint32_t)dst) & 3) || (((uint32_t)src) & 3)) {
        while (size--) {
            *a++ = *b++;
        }
    }
    else {
        count = size / 4;

        for (i = 0; i < count; i++) {
            *aa++ = *bb++;
        }

        tmp = size & 3;

        if (tmp) {
            a += (size & (~0x03));
            b += (size & (~0x03));

            while (tmp--) {
                *a++ = *b++;
            }
        }
    }
}

void memset_(void *dst, uint8_t value, uint32_t size)
{
    uint8_t *a = (uint8_t *)dst;
    uint32_t i, count, tmp;

    tmp = ((uint32_t)dst) & 3;

    if (tmp) {
        if (size > 4 - tmp) {
            for (i = 0; i < 4 - tmp; i++) {
                *a++ = value;
            }

            size -= (4 - tmp);
        }
        else {
            for (i = 0; i < size; i++) {
                *a++ = value;
            }

            return;
        }
    }

    count = size / 4;

    if (count) {
        tmp = value;
        tmp = (tmp << 8) | value;
        tmp = (tmp << 8) | value;
        tmp = (tmp << 8) | value;

        for (i = 0; i < count; i++) {
            *((uint32_t *)a) = tmp;
            a += 4;
        }
    }

    tmp = size & 3;

    if (tmp) {
        for (i = 0; i < tmp; i++) {
            *a++ = value;
        }
    }

}

int8_t memcmp_(void *m1, void *m2, uint32_t size)
{
    int8_t *a = (int8_t *)m1;
    int8_t *b = (int8_t *)m2;
    int8_t c;

    while (size--) {
        c = (*a++ - *b++);

        if (c) {
            return c;
        }
    }

    return 0;
}

/* function: set uint32 buffer
 * parameters:
 *     a -------------------------- output, output word buffer
 *     value ---------------------- input, input word value
 *     wordLen -------------------- input, word length of buffer a
 * return: none
 * caution:
 */
void uint32_set(uint32_t *a, uint32_t value, uint32_t wordLen)
{
    while (wordLen) {
        a[--wordLen] = value;
    }
}

/* function: copy uint32 buffer
 * parameters:
 *     dst ------------------------ output, output word buffer
 *     src ------------------------ input, input word buffer
 *     wordLen -------------------- input, word length of buffer dst or src
 * return: none
 * caution:
 */
void uint32_copy(uint32_t *dst, uint32_t *src, uint32_t wordLen)
{
    uint32_t i;

    if (dst != src) {
        for (i = 0; i < wordLen; i++) {
            dst[i] = src[i];
        }
    }
}


/* function: clear uint32 buffer
 * parameters:
 *     a -------------------------- input&output, word buffer a
 *     aWordLen ------------------- input, word length of buffer a
 * return: none
 * caution:
 */
void uint32_clear(uint32_t *a, uint32_t wordLen)
{
    uint32_t i = wordLen;

    while (i) {
        a[--i] = 0;
    }
}


static void uint32_sleep1(uint32_t count)
{
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t result = 0;
    uint32_t i;

    for (i = 0; i < count; i++) {
        result |= ((a + i) - (b + i));
    }
}

static void uint32_sleep2(uint32_t count)
{
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t result = 0;
    uint32_t i;

    for (i = 0; i < count; i++) {
        result |= ((a + i) ^ (b + i));
    }
}


/* function: sleep for a while
 * parameters:
 *     count ---------------------- input, count
 * return: none
 * caution:
 */

void uint32_sleep(uint32_t count, uint8_t rand)
{
    uint8_t rand1 = rand & 0x01;

    if (0 == rand1) {
        uint32_sleep1(count);
    }
    else {
        uint32_sleep2(count);
    }
}


/* function: convert 0x1122334455667788 to 0x4433221188776655
 * parameters:
 *     in ------------------------- source address
 *     out ------------------------ destination address
 *     wordLen -------------------- word length of in/out
 * return: none
 * caution:
 */
void uint32_endian_reverse(uint8_t *in, uint8_t *out, uint32_t wordLen)
{
    uint8_t tmp;

    if (in == out) {
        while (wordLen > 0) {
            tmp = *in;
            *in = *(in + 3);
            *(in + 3) = tmp;
            in += 1;
            tmp = *in;
            *in = *(in + 1);
            *(in + 1) = tmp;
            wordLen--;
            in += 3;
        }
    }
    else {
        while (wordLen > 0) {
            *(out)   = *(in + 3);
            *(out + 1) = *(in + 2);
            *(out + 2) = *(in + 1);
            *(out + 3) = *(in);
            wordLen--;
            in += 4;
            out += 4;
        }
    }
}

/* function: reverse word array
 * parameters:
 *     in ------------------------- input, input buffer
 *     out ------------------------ output, output buffer
 *     wordLen -------------------- input, word length of in or out
 * return: none
 * caution:
 *    1. in and out could point the same buffer
 */
void reverse_word_array(uint8_t *in, uint32_t *out, uint32_t wordLen)
{
    uint32_t idx, round = wordLen >> 1;
    uint32_t tmp;
    uint32_t *p_in;

    if (((uint32_t)(in)) & 3) {
        memcpy_(out, in, wordLen << 2);
        p_in = out;
    }
    else {
        p_in = (uint32_t *)in;
    }

    for (idx = 0; idx < round; idx++) {
        tmp = p_in[idx];
        out[idx] = p_in[wordLen - 1 - idx];
        out[wordLen - 1 - idx] = tmp;
    }

    if ((wordLen & 0x1) && (p_in != out)) {
        out[round] = p_in[round];
    }
}

/* function: reverse byte array
 * parameters:
 *     in ------------------------- input, input buffer
 *     out ------------------------ output, output buffer
 *     byteLen -------------------- input, byte length of in or out
 * return: none
 * caution:
 *    1. in and out could point the same buffer
 */
void reverse_byte_array(uint8_t *in, uint8_t *out, uint32_t byteLen)
{
    uint32_t idx, round = byteLen >> 1;
    uint8_t tmp;

    for (idx = 0; idx < round; idx++) {
        tmp = in[idx];
        out[idx] = in[byteLen - 1 - idx];
        out[byteLen - 1 - idx] = tmp;
    }

    if ((byteLen & 0x1) && (in != out)) {
        out[round] = in[round];
    }
}



/* function: C = A XOR B
 * parameters:
 *     A -------------------------- input, byte buffer a
 *     B -------------------------- input, byte buffer b
 *     C -------------------------- output, C = A XOR B
 *     byteLen -------------------- input, byte length of A,B,C
 * return: none
 * caution:
 */
void uint8_xor(uint8_t *a, uint8_t *b, uint8_t *c, uint32_t byteLen)
{
    uint32_t i;

    for (i = 0; i < byteLen; i++) {
        c[i] = a[i] ^ b[i];
    }
}


/* function: get real bit length of big number a of wordLen words
 */
uint32_t get_valid_bits(const uint32_t *a, uint32_t wordLen)
{
    uint32_t i = 0;
    uint32_t j = 0;

    if (0 == wordLen) {
        return 0;
    }

    for (i = wordLen; i > 0; i--) {
        if (a[i - 1]) {
            break;
        }
    }

    if (0 == i) {
        return 0;
    }

    for (j = 32; j > 0; j--) {
        if (a[i - 1] & (((uint32_t)0x1) << (j - 1))) {
            break;
        }
    }

    return ((i - 1) << 5) + j;
}


/* function: get real word lenth of big number a of max_words words
 * parameters:
 *     a -------------------------- input, big integer a
 *     max_words ------------------ input, max word length of a
 * return: real word lenth of big number a
 * caution:
 */
uint32_t get_valid_words(uint32_t *a, uint32_t max_words)
{
    uint32_t i;

    for (i = max_words; i > 0; i--) {
        if (a[i - 1]) {
            return i;
        }
    }

    return 0;
}


/* function: check whether big number or uint8_t buffer a is all zero or not
 * parameters:
 *     a -------------------------- input, byte buffer a
 *     aByteLen ------------------- input, byte length of a
 * return: 0(a is not zero),1(a is all zero)
 * caution:
 */
uint8_t uint8_bignum_check_zero(uint8_t a[], uint32_t aByteLen)
{
    uint32_t i;

    for (i = 0; i < aByteLen; i++) {
        if (a[i]) {
            return 0;
        }
    }

    return 1;
}


/* function: check whether big number or uint32_t buffer a is all zero or not
 * parameters:
 *     a -------------------------- input, big integer or word buffer a
 *     aWordLen ------------------- input, word length of a
 * return: 0(a is not zero), 1(a is all zero)
 * caution:
 */
uint8_t uint32_bignum_check_zero(uint32_t a[], uint32_t aWordLen)
{
    uint32_t i;

    for (i = 0; i < aWordLen; i++) {
        if (a[i]) {
            return 0;
        }
    }

    return 1;
}


/* function: compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     aWordLen ------------------- input, word length of a
 *     b -------------------------- input, big integer b
 *     bWordLen ------------------- input, word length of b
 * return:
 *     0:a=b,   1:a>b,   -1: a<b
 * caution:
 */
int32_t uint32_bignumcmp(uint32_t *a, uint32_t aWordLen, uint32_t *b, uint32_t bWordLen)
{
    int32_t i;

    aWordLen = get_valid_words(a, aWordLen);
    bWordLen = get_valid_words(b, bWordLen);

    if (aWordLen > bWordLen) {
        return 1;
    }

    if (aWordLen < bWordLen) {
        return -1;
    }

    for (i = (aWordLen - 1); i >= 0; i--) {
        if (a[i] > b[i]) {
            return 1;
        }

        if (a[i] < b[i]) {
            return -1;
        }
    }

    return 0;
}

/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 */
static uint32_t uint32_sec_cmp1(uint32_t *a, uint32_t *b, uint32_t wordLen)
{
    uint32_t i, result;

    result = 0;

    for (i = 0; i < wordLen; i++) {
        result |= (a[i] - b[i]);
    }

    return result;
}


/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 */
static uint32_t uint32_sec_cmp2(uint32_t *a, uint32_t *b, uint32_t wordLen)
{
    uint32_t i, result;

    result = 0;

    for (i = 0; i < wordLen; i++) {
        result |= (a[i] ^ b[i]);
    }

    return result;
}


/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 */
uint32_t uint32_sec_cmp(uint32_t *a, uint32_t *b, uint32_t wordLen, uint8_t rand)
{
    uint8_t rand1 = rand & 0x01;

    if (0 == rand1) {
        return uint32_sec_cmp1(a, b, wordLen);
    }
    else {
        return uint32_sec_cmp2(a, b, wordLen);
    }
}

/* function: for a = b*2^t, b is odd, get t
 * parameters:
 *     a -------------------------- big integer a
 * return:
 *     number of multiple by 2, for a
 * caution:
 *     1. make sure a != 0
 */
uint32_t get_multiple2_number(uint32_t a[])
{
    uint32_t t, i = 0, j = 0;

    while (0 == (a[i])) {
        i++;
    }

    t = a[i];

    while (!(t & 1)) {
        j++;
        t >>= 1;
    }

    return (i << 5) + j;
}


/* function: a = a/(2^n)
 * parameters:
 *     a -------------------------- big integer a
 *     aWordLen ------------------- word length of a
 *     n -------------------------- exponent of 2^n
 * return:
 *     word length of a = a/(2^n)
 * caution:
 *     1. make sure aWordLen is real word length of a
 *     2. a may be 0, then aWordLen is 0, to make sure aWordLen-1 is available, so data
 *        type of aWordLen is int32_t, not uint32_t
 */
uint32_t big_div2n(uint32_t a[], int32_t aWordLen, uint32_t n)
{
    int32_t i;
    uint32_t j;

    aWordLen = get_valid_words(a, aWordLen);

    if (0 == n) {
        return aWordLen;
    }

    if (!aWordLen) {
        return 0;
    }

    if (n <= 32) {
        for (i = 0; i < aWordLen - 1; i++) {
            a[i] >>= n;
            a[i] |= (a[i + 1] << (32 - n));
        }

        a[i] >>= n;

        if (!a[i]) {
            return i;
        }

        return aWordLen;
    }
    else {
        j = n >> 5;
        n &= 31;

        for (i = 0; i < aWordLen - (int32_t)j - 1; i++) {
            a[i] = a[i + j] >> n;
            a[i] |= (a[i + j + 1] << (32 - n));
        }

        a[i] = a[i + j] >> n;
        uint32_clear(a + aWordLen - j, j);

        if (!a[i]) {
            return i;
        }

        return aWordLen - j;
    }
}


/* Function: check whether a is equal to 1 or not
 * Parameters:
 *     a ---------------- pointer to uint32_t big integer a
 *     aWordLen --------- word length of big integer a
 * Return: 1(a is 1), 0(a is not 1)
 * Caution:
 */
uint8_t bigint_check_1(uint32_t a[], uint32_t aWordLen)
{
    uint32_t i;

    if (!aWordLen) {
        return 0;
    }

    if (a[0] != 1) {
        return 0;
    }

    for (i = 1; i < aWordLen; i++) {
        if (a[i]) {
            return 0;
        }
    }

    return 1;
}


/* function: check whether a is equal to p-1 or not
 * parameters:
 *     a ---------------- pointer to uint32_t big integer a
 *     p ---------------- pointer to uint32_t big integer p, p must be odd
 *     wordLen ---------- word length of a and p
 * return: 1(a is 1), 0(a is not 1)
 * caution:
 *     1. make sure p is odd
 */
uint8_t bigint_check_p_1(uint32_t a[], uint32_t p[], uint32_t wordLen)
{
    uint32_t i;

    if (!wordLen) {
        return 0;
    }

    if (a[0] != p[0] - 1) {
        return 0;
    }

    for (i = 1; i < wordLen; i++) {
        if (a[i] != p[i]) {
            return 0;
        }
    }

    return 1;
}

uint32_t ce_get_current_time(void)
{
#ifdef BOARD_KERNEL_TIMER
    return timer_get_current_time(&g_kernel_timer);
#else
    return 0;
#endif
}