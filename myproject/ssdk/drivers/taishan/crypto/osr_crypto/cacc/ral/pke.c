/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <stdio.h>
#include "utility.h"
#include "pke.h"
#include "trng.h"

static uint32_t g_oper_step;

/* function: get pke IP version
 * parameters: none
 * return: pke IP version
 * caution:
 */
uint32_t pke_get_version(void)
{
    return PKE_VERSION;
}

/* function: load input operand to baseaddr
 * parameters:
 *     baseaddr ------------------- output, destination data
 *     data ----------------------- input, source data
 *     wordlen -------------------- input, word length of data
 * return: none
 * caution:
 */
void pke_load_operand(uint32_t *baseaddr, uint32_t *data, uint32_t wordlen)
{
    uint32_t i;

    if (baseaddr != data) {
        for (i = 0; i < wordlen; i++) {
            *((volatile uint32_t *)baseaddr + i) = data[i];
        }
    }
    else
    {;}
}

/* function: get result operand from baseaddr
 * parameters:
 *     baseaddr ------------------- input, source data
 *     data ----------------------- output, destination data
 *     wordlen -------------------- input, word length of data
 * return: none
 * caution:
 */
void pke_read_operand(uint32_t *baseaddr, uint32_t *data, uint32_t wordlen)
{
    uint32_t i;

    if (baseaddr != data) {
        for (i = 0; i < wordlen; i++) {
            data[i] = *((volatile uint32_t *)baseaddr + i);
        }
    }
    else
    {;}
}

/* function: clear finished and interrupt tag
 * parameters: none
 * return: none
 * caution:
 */
void pke_clear_interrupt(void)
{
    PKE_RISR &= ~0x1;
}

/* function: enable pke interrupt
 * parameters: none
 * return: none
 * caution:
 */
static void pke_enable_interrupt(void)
{
    PKE_CFG |= (((uint32_t)1) << PKE_INT_ENABLE_OFFSET);
}

/* function: disable pke interrupt
 * parameters: none
 * return: none
 * caution:
 */
static void pke_disable_interrupt(void)
{
    PKE_CFG &= ~(((uint32_t)1) << PKE_INT_ENABLE_OFFSET);
}

/* function: set operand width
 * parameters:
 *     bitlen --------------------- input, bit length of operand
 * return: none
 * caution: please make sure 0 < bitlen <= OPERAND_MAX_BIT_LEN
 */
void pke_set_operand_width(uint32_t bitlen)
{
    uint32_t cfg, len;

    len = (bitlen + 255) / 256;

    if (1 == len) {
        cfg = 2;
        g_oper_step = 0x20;
    }
    else if (2 == len) {
        cfg = 3;
        g_oper_step = 0x40;
    }
    else if (len <= 4) {
        cfg = 4;
        g_oper_step = 0x80;
    }
    else if (len <= 8) {
        cfg = 5;
        g_oper_step = 0x100;
    }
    else if (len <= 16) {
        cfg = 6;
        g_oper_step = 0x200;
    }
    else {
        return;
    }

    cfg = (cfg << 16) | (len << 8);

    PKE_CFG &= ~(0x07FFFF);
    PKE_CFG |= cfg;
}

/* function: get current operand byte length
 * parameters: none
 * return: current operand byte length
 * caution: none
 */
uint32_t pke_get_operand_bytes(void)
{
    return g_oper_step;
}

/* function: set operation micro code
 * parameters:
 *     addr ----------------------- input, specific micro code
 * return: none
 * caution:
 */
void pke_set_microcode(uint32_t addr)
{
    PKE_MC_PTR = addr;
}

/* function: start pke calc
 * parameters: none
 * return: none
 * caution:
 */
void pke_start(void)
{
    PKE_CTRL |= PKE_START_CALC;
}

/* function: return calc return code
 * parameters: none
 * return 0(success), other(error)
 * caution:
 */
uint32_t pke_check_rt_code(void)
{
    return (uint8_t)(PKE_RT_CODE & 0x07);
}

/* function: wait till done
 * parameters: none
 * return: none
 * caution:
 */
void pke_wait_till_done(void)
{
    while (!(PKE_RISR & 0x1))
    {;}
}

/* function: set operation micro code, start hardware, wait till done, and return code
 * parameters:
 *     micro_code ----------------- input, specific micro code
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 */
uint32_t pke_set_micro_code_start_wait_return_code(uint32_t micro_code)
{
    pke_set_microcode(micro_code);

    pke_clear_interrupt();

    pke_start();

    pke_wait_till_done();

    return pke_check_rt_code();
}

/* function: ainv = a^(-1) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     ainv ----------------------- output, ainv = a^(-1) mod modulus
 *     modwordlen ----------------- input, word length of modulus and ainv
 *     awordlen ------------------- input, word length of a
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 *     1. please make sure awordlen <= modwordlen <= OPERAND_MAX_WORD_LEN and a < modulus
 */
uint32_t pke_modinv(const uint32_t *modulus, const uint32_t *a, uint32_t *ainv, uint32_t modwordlen,
                    uint32_t awordlen)
{
    uint32_t ret;

    pke_set_operand_width(modwordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, modwordlen);

    if ((g_oper_step / 4) > modwordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + modwordlen, (g_oper_step / 4) - modwordlen);
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)a, awordlen);

    if ((g_oper_step / 4) > awordlen) {
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + awordlen, (g_oper_step / 4) - awordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODINV);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), modwordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), awordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), modwordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), ainv, modwordlen);

        return PKE_SUCCESS;
    }
}

/* function: out = (a+b) mod modulus or out = (a-b) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b mod modulus or out = (a-b) mod modulus
 *     wordlen -------------------- input, word length of modulus, a, b
 *     micro_code ----------------- input, must be MICROCODE_MODADD or MICROCODE_MODSUB
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a,b must be less than modulus
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modadd_modsub_internal(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
                                    uint32_t *out, uint32_t wordlen, uint32_t micro_code)
{
    uint32_t ret;

    pke_set_operand_width(wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, wordlen);
    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)a, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)b, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(micro_code);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, wordlen);

        return PKE_SUCCESS;
    }
}

/* function: out = (a+b) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b mod modulus
 *     wordlen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a,b must be less than modulus
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modadd(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
                    uint32_t *out, uint32_t wordlen)
{
    return pke_modadd_modsub_internal(modulus, a, b, out, wordlen, MICROCODE_MODADD);
}

/* function: out = (a-b) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a-b mod modulus
 *     wordlen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a,b must be less than modulus
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modsub(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
                    uint32_t *out, uint32_t wordlen)
{
    return pke_modadd_modsub_internal(modulus, a, b, out, wordlen, MICROCODE_MODSUB);
}

/* function: out = a+b or out = a-b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b or out = a-b
 *     wordlen -------------------- input, word length of a, b, out
 *     micro_code ----------------- input, must be MICROCODE_INTADD or MICROCODE_INTSUB
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if a+b output may overflow, if a-b please make sure a > b
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_add_sub_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen,
                              uint32_t micro_code)
{
    uint32_t ret;

    pke_set_operand_width(wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)a, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)b, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(micro_code);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, wordlen);

        return PKE_SUCCESS;
    }
}

/* function: out = a+b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b
 *     wordlen -------------------- input, word length of a, b, out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a+b may overflow
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_add(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen)
{
    return pke_add_sub_internal(a, b, out, wordlen, MICROCODE_INTADD);
}

/* function: out = a-b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a-b
 *     wordlen -------------------- input, word length of a, b, out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure a > b
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_sub(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen)
{
    return pke_add_sub_internal(a, b, out, wordlen, MICROCODE_INTSUB);
}

/* function: out = a*b
 * parameters:
 *     a -------------------------- input, integer a
 *     a_wordlen ------------------ input, word length of a
 *     b -------------------------- input, integer b
 *     b_wordlen ------------------ input, word length of b
 *     out ------------------------ output, out = a*b
 *     out_wordlen----------------- input, word length of out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure out buffer word length is bigger than (2*max_bit_len(a,b)+0x1F)>>5
 *     2. please make sure ab_wordLen is not bigger than OPERAND_MAX_WORD_LEN/2
 */
uint32_t pke_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t a_wordlen,
                          uint32_t b_wordlen, uint32_t out_wordlen)
{
    uint32_t ret;

    pke_set_operand_width(GET_MAX_LEN(out_wordlen << 5, 512));

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)a, a_wordlen);

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)b, b_wordlen);

    uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + a_wordlen, (g_oper_step / 4) - a_wordlen);
    uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + b_wordlen, (g_oper_step / 4) - b_wordlen);

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_INTMUL);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), a_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), b_wordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, out_wordlen);

        return PKE_SUCCESS;
    }
}

/* function: out = a*b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b
 *     wordlen -------------------- input, word length of a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure out buffer word length is bigger than (2*max_bit_len(a,b)+0x1F)>>5
 *     2. please make sure ab_wordLen is not bigger than OPERAND_MAX_WORD_LEN/2
 */
uint32_t pke_mul(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t ab_wordLen)
{
    uint32_t bitlen, tempLen;

    bitlen = get_valid_bits(a, ab_wordLen);
    tempLen = get_valid_bits(b, ab_wordLen);

    bitlen = GET_MAX_LEN(bitlen, tempLen);
    tempLen = GET_WORD_LEN(bitlen << 1);

    if (tempLen < (ab_wordLen << 1)) {
        tempLen = (ab_wordLen << 1) - 1;
    }
    else {
        tempLen = (ab_wordLen << 1);
    }

    return pke_mul_internal(a, b, out, ab_wordLen, ab_wordLen, tempLen);
}

/* function: calc n0(- modulus ^(-1) mod 2^w) for modMul, and pointMul. etc.
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling, please make sure the modulus is set in PKE_A(a, 0)
 *     2. please make sure the modulus is odd, and word length of the modulus
 *        is not bigger than OPERAND_MAX_WORD_LEN
 *     3. the result is set in the internal register, no need to output.
 */
uint32_t pke_pre_calc_mont_N0(void)
{
    pke_set_microcode(MICROCODE_MGMR_PRE_N0);

    pke_clear_interrupt();

    pke_start();

    pke_wait_till_done();

    return pke_check_rt_code();
}

/* function: calc H(R^2 mod modulus) for modMul, and pointMul. etc.
 * parameters:
 *     modulus -------------------- input, modulus
 *     bitlen --------------------- input, bit length of modulus
 *     H -------------------------- output, R^2 mod modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure word length of buffer H is equal to word length of modulus
 *     3. bitlen must not be bigger than OPERAND_MAX_BIT_LEN
 */
uint32_t pke_pre_calc_mont(const uint32_t *modulus, uint32_t bitlen, uint32_t *H)
{
    uint32_t wordlen = GET_WORD_LEN(bitlen);
    uint32_t ret;

    pke_set_operand_width(bitlen);

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }

    ret = pke_pre_calc_mont_N0();

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    if (256 == bitlen || 512 == bitlen || 1024 == bitlen || 2048 == bitlen || 4096 == bitlen) {
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MGMR_PRE_H_MM);
    }
    else {
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MGMR_PRE_H);
    }

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(0, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else if (NULL != H) {
        pke_read_operand((uint32_t *)(PKE_B(0, g_oper_step)), H, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}

/* function: like function pke_pre_calc_mont(), but this one is without output here
 * parameters:
 *     modulus -------------------- input, modulus
 *     wordlen -------------------- input, word length of modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_pre_calc_mont_no_output(const uint32_t *modulus, uint32_t wordlen)
{
    return pke_pre_calc_mont(modulus, get_valid_bits(modulus, wordlen), NULL);
}

/* function: load the pre-calculated mont parameters H(R^2 mod modulus)
 * parameters:
 *     H -------------------------- input, R^2 mod modulus
 *     wordlen -------------------- input, word length of modulus or H
 * return: none
 * caution:
 *     1. please make sure the 2 input parameters are both valid
 *     2. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
void pke_load_pre_calc_mont(uint32_t *H,  uint32_t wordlen)
{
    pke_set_operand_width(wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_B(0, g_oper_step)), H, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_B(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}
}

/* function: out = a*b mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b mod modulus
 *     wordlen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. a, b must less than modulus
 *     3. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 *     4. before calling this function, please make sure the pre-calculated mont argument
 *        of modulus is located in the right address.
 */
uint32_t pke_modmul_internal(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, uint32_t *out,
                             uint32_t wordlen)
{
    uint32_t ret;

    pke_set_operand_width(wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_pre_calc_mont_N0();

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)a, wordlen);

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)b, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else {

        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, wordlen);

        return PKE_SUCCESS;
    }
}

/* function: out = a*b mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b mod modulus
 *     wordlen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. a, b must less than modulus
 *     3. wordlen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modmul(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen)
{
    uint32_t ret;

    ret = pke_pre_calc_mont(modulus, get_valid_bits(modulus, wordlen), NULL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else {
        return pke_modmul_internal((uint32_t *)(PKE_A(0, g_oper_step)), a, b, out, wordlen);
    }
}

/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordlen ---------------- input, word length of modulus and base number
 *     exp_wordlen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure R^2 mod modulus, the pre-calculated
 *        mont arguments of modulus is located in the right address
 *     2. modulus must be odd
 *     3. please make sure exp_wordlen <= mod_wordlen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
                    uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen)
{
    uint32_t ret;

    pke_set_operand_width(mod_wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), (uint32_t *)exponent, exp_wordlen);

    if ((g_oper_step / 4) > exp_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + exp_wordlen, (g_oper_step / 4) - exp_wordlen);
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, mod_wordlen);

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)base, mod_wordlen);

    if ((g_oper_step / 4) > mod_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
    }
    else
    {;}

    ret = pke_pre_calc_mont_N0();

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), mod_wordlen << 2);
#endif
        return ret;
    }
    else {

        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, mod_wordlen);

        return PKE_SUCCESS;
    }
}

/* function: c = a mod b
 * parameters:
 *     a -------------------------- input, integer a
 *     awordlen ------------------- input, word length of integer
 *     b -------------------------- input, integer b, modulus
 *     b_h ------------------------ input, H parameter of b
 *     bwordlen ------------------- input, word length of integer b and b_h
 *     c -------------------------- output, c = a mod b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. b must be odd, and please make sure bwordlen is real word length of b
 *     2. pleae make sure awordlen <= 2*OPERAND_MAX_WORD_LEN, bwordlen <= OPERAND_MAX_WORD_LEN,
 *     3. real bit length of a can not be bigger than 2*(real bit length of b)
 */
uint32_t pke_mod(uint32_t *a, uint32_t awordlen, uint32_t *b, uint32_t *b_h, uint32_t bwordlen, uint32_t *c)
{
    int32_t flag;
    uint32_t bitlen, tmpLen;
    uint32_t *t1, *t2;
    uint32_t ret;

    flag = uint32_bignumcmp(a, awordlen, b, bwordlen);

    if (flag < 0) {
        awordlen = get_valid_words(a, awordlen);
        uint32_copy(c, a, awordlen);
        uint32_clear(c + awordlen, bwordlen - awordlen);

        return PKE_SUCCESS;
    }
    else if (0 == flag) {
        uint32_clear(c, bwordlen);

        return PKE_SUCCESS;
    }
    else
    {;}

    pke_set_operand_width(bwordlen << 5);
    t1 = (uint32_t *)(PKE_A(1, g_oper_step));
    t2 = (uint32_t *)(PKE_B(2, g_oper_step));

    bitlen = get_valid_bits(b, bwordlen) & 0x1F;

    /* get t2 = a high part mod b */
    if (bitlen) {
        tmpLen = awordlen - bwordlen + 1;
        uint32_copy(t2, a + bwordlen - 1, tmpLen);
        big_div2n(t2, tmpLen, bitlen);

        if (tmpLen < bwordlen) {
            uint32_clear(t2 + tmpLen, bwordlen - tmpLen);
        }
        else if (uint32_bignumcmp(t2, bwordlen, b, bwordlen) >= 0) {
            ret = pke_sub(t2, b, t2, bwordlen);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else
        {;}
    }
    else {
        tmpLen = awordlen - bwordlen;

        if (uint32_bignumcmp(a + bwordlen, tmpLen, b, bwordlen) >= 0) {
            ret = pke_sub(a + bwordlen, b, t2, bwordlen);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else {
            uint32_copy(t2, a + bwordlen, tmpLen);
            uint32_clear(t2 + tmpLen, bwordlen - tmpLen);
        }
    }

    /* set the pre-calculated mont parameters */
    if (NULL == b_h) {
        ret = pke_pre_calc_mont(b, get_valid_bits(b, bwordlen), NULL);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else {
        pke_load_pre_calc_mont(b_h, bwordlen);
    }

    /* get t1 = 1000...000 mod b */
    uint32_clear(t1, bwordlen);

    if (bitlen) {
        t1[bwordlen - 1] = 1 << (bitlen);
    }
    else
    {;}

    ret = pke_sub(t1, b, t1, bwordlen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* get t2 = a_high * 1000..000 mod b */
    ret = pke_modmul_internal(b, t1, t2, t2, bwordlen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* get t1 = a low part mod b */
    if (bitlen) {
        uint32_copy(t1, a, bwordlen);
        t1[bwordlen - 1] &= ((1 << (bitlen)) - 1);

        if (uint32_bignumcmp(t1, bwordlen, b, bwordlen) >= 0) {
            ret = pke_sub(t1, b, t1, bwordlen);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else
        {;}
    }
    else {
        if (uint32_bignumcmp(a, bwordlen, b, bwordlen) >= 0) {
            ret = pke_sub(a, b, t1, bwordlen);

            if (PKE_SUCCESS != ret) {
                return ret;
            }
            else
            {;}
        }
        else {
            t1 = a;
        }
    }

    return pke_modadd(b, t1, t2, c, bwordlen);
}

/* function: set modulus and pre-calculated mont parameters H(R^2 mod modulus) and n0' for hardware operation
 * parameters:
 *     modulus -------------------- input, modulus
 *     modulus_h ------------------ input, R^2 mod modulus
 *     bitlen --------------------- input, bit length of modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. bitlen must not be bigger than OPERAND_MAX_BIT_LEN
 */
uint32_t pke_set_modulus_and_pre_mont(uint32_t *modulus, uint32_t *modulus_h, uint32_t bitlen)
{
    uint32_t wordlen = GET_WORD_LEN(bitlen);
    uint32_t ret;

    if (NULL != modulus_h) {
        pke_set_operand_width(bitlen);

        pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), modulus, wordlen);

        if ((g_oper_step / 4) > wordlen) {
            uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        }
        else
        {;}

        ret = pke_pre_calc_mont_N0();

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        pke_load_pre_calc_mont(modulus_h, wordlen);
    }
    else {
        ret = pke_pre_calc_mont(modulus, bitlen, NULL);
    }

    return ret;
}

/********************************** ECCp functions *************************************/

/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. the output may be invalid(return PKE_NO_MODINV), even if input are all valid,
 *        it is suggested to call eccp_pointMul_Shamir_safe()
 */
uint32_t eccp_pointMul_Shamir(eccp_curve_t *curve,
                              uint32_t *k1, uint32_t *P1x, uint32_t *P1y,
                              uint32_t *k2, uint32_t *P2x, uint32_t *P2y,
                              uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    /* set ecc_p, ecc_p_h, etc. */
    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), P1x, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(2, g_oper_step)), P1y, wordlen);
    *((uint32_t *)(PKE_A(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_A(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);

    pke_load_operand((uint32_t *)(PKE_B(5, g_oper_step)), P2x, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(6, g_oper_step)), P2y, wordlen);

    pke_load_operand((uint32_t *)(PKE_B(4, g_oper_step)), curve->eccp_a, wordlen);
    pke_load_operand((uint32_t *)(PKE_A(4, g_oper_step)), k1, wordlen);
    pke_load_operand((uint32_t *)(PKE_A(5, g_oper_step)), k2, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(5, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(6, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(5, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMULF);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(5, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(6, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(4, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(4, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(5, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), Qx, wordlen);

    if (Qy != NULL) {
        pke_read_operand((uint32_t *)(PKE_A(2, g_oper_step)), Qy, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}

/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_Shamir_safe(eccp_curve_t *curve,
                                   uint32_t *k1, uint32_t *P1x, uint32_t *P1y,
                                   uint32_t *k2, uint32_t *P2x, uint32_t *P2y,
                                   uint32_t *Qx, uint32_t *Qy)
{
    uint32_t x[ECCP_MAX_WORD_LEN], y[ECCP_MAX_WORD_LEN];
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    ret = eccp_pointMul_Shamir(curve, k1, P1x, P1y, k2, P2x, P2y, Qx, Qy);

    if (PKE_NO_MODINV == ret) {
        ret = eccp_pointMul(curve, k1, P1x, P1y, x, y);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = eccp_pointMul(curve, k2, P2x, P2y, (uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)(PKE_A(2, g_oper_step)));

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        ret = eccp_pointAdd(curve, (uint32_t *)(PKE_A(1, g_oper_step)), (uint32_t *)(PKE_A(2, g_oper_step)), x, y, Qx, Qy);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}
    }
    else
    {;}

#ifdef PKE_SEC
    get_rand_fast((uint8_t *)x, wordlen << 2);
    get_rand_fast((uint8_t *)y, wordlen << 2);
#endif

    return ret;
}

/* function: ECCP curve point mul(Q = [k]G, here G is the curve base point)
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     k  ------------------------- input, scalar k1
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. the input point is base point, and please make sure curve->eccp_half_Gx
 *        and curve->eccp_half_Gy both are not NULL!
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_base(eccp_curve_t *curve, uint32_t *k, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t nwordlen = GET_WORD_LEN(curve->eccp_n_bitLen);
    uint32_t *k1;
    uint32_t *k2;
    uint32_t tmpbitlen, tmpwordlen;
    uint32_t ret;

    pke_set_operand_width(curve->eccp_p_bitLen);
    k1 = (uint32_t *)(PKE_A(4, g_oper_step));
    k2 = (uint32_t *)(PKE_A(5, g_oper_step));

    /* k2: low half part */
    tmpbitlen = (curve->eccp_n_bitLen) / 2;
    tmpwordlen = GET_WORD_LEN(tmpbitlen);
    uint32_copy(k2, k, tmpwordlen);
    uint32_clear(k2 + tmpwordlen, nwordlen - tmpwordlen);
    tmpbitlen = tmpbitlen & 0x1F;

    if (tmpbitlen) {
        k2[tmpwordlen - 1] &= (1 << tmpbitlen) - 1;
    }
    else
    {;}

    /* k1: high half part */
    if (tmpbitlen) {
        uint32_copy(k1, k + tmpwordlen - 1, nwordlen - tmpwordlen + 1);
        uint32_clear(k1 + nwordlen - tmpwordlen + 1, tmpwordlen - 1);
        big_div2n(k1, nwordlen - tmpwordlen + 1, tmpbitlen);
    }
    else {
        uint32_copy(k1, k + tmpwordlen, nwordlen - tmpwordlen);
        uint32_clear(k1 + nwordlen - tmpwordlen, tmpwordlen);
    }

    tmpbitlen = curve->eccp_n_bitLen - (curve->eccp_n_bitLen) / 2;
    tmpwordlen = GET_WORD_LEN(tmpbitlen);
    tmpbitlen = tmpbitlen & 0x1F;

    if (tmpbitlen) {
        k1[tmpwordlen - 1] &= (1 << tmpbitlen) - 1;
    }
    else
    {;}

    ret = eccp_pointMul_Shamir(curve,
                               k1, curve->eccp_half_Gx, curve->eccp_half_Gy,
                               k2, curve->eccp_Gx, curve->eccp_Gy,
                               Qx, Qy);

    if (PKE_NO_MODINV == ret) {
        ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, Qx, Qy);
    }
    else
    {;}

    return ret;
}

/* function: ECCP curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. even if the input point P is valid, the output may be infinite point, in this case
 *        it will return error.
 */
uint32_t eccp_pointMul(eccp_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                       uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), Px, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(2, g_oper_step)), Py, wordlen);
    *((uint32_t *)(PKE_A(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_A(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);
    pke_load_operand((uint32_t *)(PKE_B(4, g_oper_step)), curve->eccp_a, wordlen);
    pke_load_operand((uint32_t *)(PKE_A(4, g_oper_step)), k, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMUL);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), Qx, wordlen);

    if (NULL != Qy) {
        pke_read_operand((uint32_t *)(PKE_A(2, g_oper_step)), Qy, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}

/* function: ECCP curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     3. even if the input point P1 and P2 is valid, the output may be infinite point,
 *        in this case it will return error.
 */
uint32_t eccp_pointAdd(eccp_curve_t *curve, uint32_t *P1x, uint32_t *P1y, uint32_t *P2x, uint32_t *P2y,
                       uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* pke_pre_calc_mont() may cover A1, so load A1(P1x) here */

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), P1x, wordlen);

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), P1y, wordlen);

    *((uint32_t *)(PKE_B(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_B(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), P2x, wordlen);

    pke_load_operand((uint32_t *)(PKE_B(2, g_oper_step)), P2y, wordlen);

    *((uint32_t *)(PKE_A(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_A(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PADD);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(2, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}


    pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), Qx, wordlen);

    if (NULL != Qy) {

        pke_read_operand((uint32_t *)(PKE_A(2, g_oper_step)), Qy, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}

#ifdef ECCP_POINT_DOUBLE
/* function: ECCP curve point double, Q=[2]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q=[2]P
 *     Qy ------------------------- output, y coordinate of point Q=[2]P
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointDouble(eccp_curve_t *curve, uint32_t *Px, uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* pke_pre_calc_mont() may cover A1, so load A1(Px) and other paras here */

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), Px, wordlen);

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), Py, wordlen);

    *((uint32_t *)(PKE_B(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_B(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);

    pke_load_operand((uint32_t *)(PKE_B(4, g_oper_step)), curve->eccp_a, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PDBL);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(4, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), Qx, wordlen);

    pke_read_operand((uint32_t *)(PKE_A(2, g_oper_step)), Qy, wordlen);

    return PKE_SUCCESS;
}
#endif

/* function: check whether the input point P is on ECCP curve or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     2. after calculation, A1 and A2 will be changed!
 */
uint32_t eccp_pointVerify(eccp_curve_t *curve, uint32_t *Px, uint32_t *Py)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* pke_pre_calc_mont() may cover A1, so load A1(Px) and other paras here */

    pke_load_operand((uint32_t *)(PKE_A(1, g_oper_step)), Px, wordlen);

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), Py, wordlen);

    pke_load_operand((uint32_t *)(PKE_B(4, g_oper_step)), curve->eccp_a, wordlen);

    pke_load_operand((uint32_t *)(PKE_A(4, g_oper_step)), curve->eccp_b, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_A(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PVER);

    if (0 != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(4, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(4, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else {
        return PKE_SUCCESS;
    }
}

/* function: get ECCP public key from private key(the key pair could be used in SM2/ECDSA/ECDH, etc.)
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     prikey --------------------- input, private key, big-endian
 *     pubkey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_get_pubkey_from_prikey(eccp_curve_t *curve, uint8_t *prikey, uint8_t *pubkey)
{
    uint32_t nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    uint32_t nwordlen = GET_WORD_LEN(curve->eccp_n_bitLen);
    uint32_t pbytelen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    uint32_t k[ECCP_MAX_WORD_LEN];
    uint32_t *x;
    uint32_t *y;
    uint32_t ret;
#ifdef SUPPORT_SM2
    uint32_t sm2p256v1_n[8]    = {0x39D54123, 0x53BBF409, 0x21C6052B, 0x7203DF6B, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
#endif

    pke_set_operand_width(curve->eccp_p_bitLen);
    x = (uint32_t *)(PKE_A(1, g_oper_step));
    y = (uint32_t *)(PKE_A(2, g_oper_step));

    /* clear if curve->eccp_n_bitLen is not a multiple of 32 */
    k[nwordlen - 1] = 0;
    reverse_byte_array(prikey, (uint8_t *)k, nByteLen);

    /* make sure k in [1, n-1] */
    if (uint32_bignum_check_zero(k, nwordlen)) {
        return PKE_ZERO_ALL;
    }
    else if (uint32_bignumcmp(k, nwordlen, curve->eccp_n, nwordlen) >= 0) {
        return PKE_INTEGER_TOO_BIG;
    }
    else
    {;}

#ifdef SUPPORT_SM2

    /*sm2p256v1_n sm2_curve->eccp_n[0] - 1 */

    if ((k[0] == sm2p256v1_n[0] - 1) && (0 == uint32_bignumcmp(k + 1, nwordlen - 1, sm2p256v1_n + 1, nwordlen - 1))) {
        return PKE_INTEGER_TOO_BIG;
    }
    else
    {;}

#endif

    /* get pubkey */
    if (curve->eccp_half_Gx && curve->eccp_half_Gy) {
        ret = eccp_pointMul_base(curve, k, x, y);
    }
    else {
        ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, x, y);
    }

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    reverse_byte_array((uint8_t *)x, pubkey, pbytelen);
    reverse_byte_array((uint8_t *)y, pubkey + pbytelen, pbytelen);

    return PKE_SUCCESS;
}

/* function: get ECCP key pair(the key pair could be used in SM2/ECDSA/ECDH)
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     prikey --------------------- output, private key, big-endian
 *     pubkey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_getkey(eccp_curve_t *curve, uint8_t *prikey, uint8_t *pubkey)
{
    uint32_t tmpLen;
    uint32_t nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    uint32_t ret;

ECCP_GETKEY_LOOP:

    ret = get_rand(prikey, nByteLen);

    if (TRNG_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* make sure k has the same bit length as n */
    tmpLen = (curve->eccp_n_bitLen) & 7;

    if (tmpLen) {
        prikey[0] &= (1 << (tmpLen)) - 1;
    }
    else
    {;}

    ret = eccp_get_pubkey_from_prikey(curve, prikey, pubkey);

    if (PKE_ZERO_ALL == ret || PKE_INTEGER_TOO_BIG == ret) {
        goto ECCP_GETKEY_LOOP;
    }
    else {
        return ret;
    }
}

/****************************** ECCp functions finished ********************************/

#ifdef SUPPORT_C25519
/**************************** X25519 & Ed25519 functions *******************************/

/* function: c25519 point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, c25519 curve struct pointer
 *     k -------------------------- input, scalar
 *     Pu ------------------------- input, u coordinate of point P
 *     Qu ------------------------- output, u coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be infinite point, in this case return error.
 *     3. please make sure the curve is c25519
 */
uint32_t x25519_pointMul(mont_curve_t *curve, uint32_t *k, uint32_t *Pu, uint32_t *Qu)
{
    uint32_t wordlen = GET_WORD_LEN(curve->p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->p, curve->p_h, curve->p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)PKE_A(1, g_oper_step), Pu, wordlen);
    pke_load_operand((uint32_t *)PKE_A(2, g_oper_step), curve->a24, wordlen);
    pke_load_operand((uint32_t *)PKE_A(4, g_oper_step), k, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)PKE_A(1, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(2, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(4, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(0, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_C25519_PMUL);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(4, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)PKE_A(1, g_oper_step), Qu, wordlen);

    return PKE_SUCCESS;
}


/* function: Ed25519 decode point
 * parameters:
 *     in_y ----------------------- input, encoded Ed25519 point
 *     out_x ---------------------- output, x coordinate of input point
 *     out_y ---------------------- output, y coordinate of input point
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1.
 */
uint32_t ed25519_decode_point(edward_curve_t *curve, uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32])
{
    uint32_t u[Ed25519_WORD_LEN];
    uint32_t v[Ed25519_WORD_LEN];
    uint32_t t[Ed25519_WORD_LEN] = {0};
    uint32_t t2[Ed25519_WORD_LEN];
    uint32_t t3[Ed25519_WORD_LEN];
    uint32_t ret;

    memcpy_(u, in_y, Ed25519_BYTE_LEN);
    u[Ed25519_WORD_LEN - 1] &= 0x7FFFFFFF;

    /* make sure y < prime p */
    if (uint32_bignumcmp(u, Ed25519_WORD_LEN, curve->p, Ed25519_WORD_LEN) >= 0) {
        return PKE_INVALID_INPUT;
    }
    else
    {;}

    /* set pre-calculated paras */
    if (NULL != curve->p_h) {
        pke_load_pre_calc_mont(curve->p_h, Ed25519_WORD_LEN);
    }
    else {
        pke_pre_calc_mont(curve->p, curve->p_bitLen, NULL);
    }

    /* v = y^2 */
    ret = pke_modmul_internal(curve->p, u, u, v, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    t[0] = 1;
    /* u = y^2 - 1 */
    ret = pke_modsub(curve->p, v, t, u, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* v = d*y^2 */
    ret = pke_modmul_internal(curve->p, curve->d, v, v, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* v = d*y^2 + 1 */
    ret = pke_modadd(curve->p, v, t, v, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = v^2 */
    ret = pke_modmul_internal(curve->p, v, v, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t3 = v^3 */
    ret = pke_modmul_internal(curve->p, v, t2, t3, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t = u*v^3 */
    ret = pke_modmul_internal(curve->p, t3, u, t, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = v^4 */
    ret = pke_modmul_internal(curve->p, t2, t2, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = v^7 */
    ret = pke_modmul_internal(curve->p, t2, t3, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = u*v^7 */
    ret = pke_modmul_internal(curve->p, t2, u, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t3 = (p-5)/8 */
    uint32_copy(t3, curve->p, Ed25519_WORD_LEN);
    t3[0] -= 5;
    big_div2n(t3, Ed25519_WORD_LEN, 3);
    /* t2 = (u*v^7 )^((p-5)/8) */
    ret = pke_modexp(curve->p, t3, t2, t2, Ed25519_WORD_LEN, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t = x = (u*v^3)*(u*v^7 )^((p-5)/8) */
    ret = pke_modmul_internal(curve->p, t2, t, t, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = x^2 */
    ret = pke_modmul_internal(curve->p, t, t, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* t2 = v*x^2 */
    ret = pke_modmul_internal(curve->p, t2, v, t2, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    /* if v x^2 = u (mod p), x is a square root. */
    if (0 == uint32_bignumcmp(t2, Ed25519_WORD_LEN, u, Ed25519_WORD_LEN)) {
        goto result;
    }
    else
    {;}

    /* t3 = -u mod p */
    ret = pke_sub(curve->p, u, t3, Ed25519_WORD_LEN);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else if (0 == uint32_bignumcmp(t2, Ed25519_WORD_LEN, t3, Ed25519_WORD_LEN)) {
        /* v = (p-1)/4 */
        uint32_copy(v, curve->p, Ed25519_WORD_LEN);
        v[0] -= 1;
        big_div2n(v, Ed25519_WORD_LEN, 2);
        /* t2 = 2 */
        uint32_clear(t2, Ed25519_WORD_LEN);
        t2[0] = 2;
        /* u = 2^((p-1)/4) */
        ret = pke_modexp(curve->p, v, t2, u, Ed25519_WORD_LEN, Ed25519_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        /* t = x*(2^((p-1)/4)) */
        ret = pke_modmul_internal(curve->p, t, u, t, Ed25519_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else
        {;}

        goto result;
    }
    else
    {;}

    return PKE_INVALID_INPUT;

result:

    /* if x=0 and x is odd, decode fail */
    if (uint32_bignum_check_zero(t, Ed25519_WORD_LEN) && (in_y[Ed25519_BYTE_LEN - 1] & 0x80)) {
        return PKE_INVALID_INPUT;
    }
    else
    {;}

    /* get out_x */
    if ((uint8_t)((t[0] & 1) << 7) == (in_y[Ed25519_BYTE_LEN - 1] & 0x80)) {
        memcpy_(out_x, t, Ed25519_BYTE_LEN);
    }
    else {
        /* v = -x mod p */
        ret = pke_sub(curve->p, t, v, Ed25519_WORD_LEN);

        if (PKE_SUCCESS != ret) {
            return ret;
        }
        else {
            memcpy_(out_x, v, Ed25519_BYTE_LEN);
        }
    }

    /* get out_y */
    memcpy_(out_y, in_y, Ed25519_BYTE_LEN);
    out_y[Ed25519_BYTE_LEN - 1] &= 0x7F;

    return PKE_SUCCESS;
}


/* function: edwards25519 curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could not be zero now.
 */
uint32_t ed25519_pointMul(edward_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                          uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->p, curve->p_h, curve->p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)PKE_A(1, g_oper_step), Px, wordlen);
    pke_load_operand((uint32_t *)PKE_A(2, g_oper_step), Py, wordlen);
    pke_load_operand((uint32_t *)PKE_A(3, g_oper_step), curve->d, wordlen);
    pke_load_operand((uint32_t *)PKE_A(4, g_oper_step), k, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)PKE_A(1, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(2, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_B(0, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(0, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_Ed25519_PMUL);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(0, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)PKE_A(1, g_oper_step), Qx, wordlen);

    if (NULL != Qy) {
        pke_read_operand((uint32_t *)PKE_A(2, g_oper_step), Qy, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}


/* function: edwards25519 point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. the output point may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 */
uint32_t ed25519_pointAdd(edward_curve_t *curve, uint32_t *P1x, uint32_t *P1y, uint32_t *P2x, uint32_t *P2y,
                          uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->p_bitLen);
    uint32_t ret;

    ret = pke_set_modulus_and_pre_mont(curve->p, curve->p_h, curve->p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)PKE_A(1, g_oper_step), P1x, wordlen);
    pke_load_operand((uint32_t *)PKE_A(2, g_oper_step), P1y, wordlen);
    pke_load_operand((uint32_t *)PKE_B(1, g_oper_step), P2x, wordlen);
    pke_load_operand((uint32_t *)PKE_B(2, g_oper_step), P2y, wordlen);
    pke_load_operand((uint32_t *)PKE_A(3, g_oper_step), curve->d, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)PKE_A(1, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_A(2, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_B(1, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_B(2, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)PKE_B(0, g_oper_step) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_Ed25519_PADD);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(0, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(2, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)PKE_A(1, g_oper_step), Qx, wordlen);
    pke_read_operand((uint32_t *)PKE_A(2, g_oper_step), Qy, wordlen);

    return PKE_SUCCESS;
}

/**************************** X25519 & Ed25519 finished ********************************/
#endif


#ifdef PKE_SEC
/*********************************** secfunctions **************************************/

/* function: pke sec init
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t pke_sec_init(void)
{
    uint32_t rand[4];

    if (TRNG_SUCCESS != get_rand((uint8_t *)&rand, 16)) {
        return PKE_STOP;
    }
    else
    {;}

    PKE_RAND_SEED = rand[0];

    PKE_RC_EN = 0;
    PKE_RC_KEY = rand[1];
    PKE_RC_D_NONCE = rand[2];
    PKE_RC_A_NONCE = rand[3] & 0x0000003F;
    PKE_RC_EN = 1;

    return PKE_SUCCESS;
}


/* function: pke sec uninit
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t pke_sec_uninit(void)
{
    PKE_RC_EN = 0;

    return PKE_SUCCESS;
}


/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordlen ---------------- input, word length of modulus and base number
 *     exp_wordlen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure R^2 mod modulus, the pre-calculated
 *        mont arguments of modulus is located in the right address
 *     2. modulus must be odd
 *     3. please make sure exp_wordlen <= mod_wordlen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp_ladder(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
                           uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen)
{
    uint32_t ret;

    pke_set_operand_width(mod_wordlen << 5);

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), (uint32_t *)exponent, exp_wordlen);

    if ((g_oper_step / 4) > exp_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + exp_wordlen, (g_oper_step / 4) - exp_wordlen);
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, mod_wordlen);
    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)base, mod_wordlen);

    if ((g_oper_step / 4) > mod_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
    }
    else
    {;}

    ret = pke_pre_calc_mont_N0();

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP_MGMR_LADDER);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(1, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), mod_wordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, mod_wordlen);

        return PKE_SUCCESS;
    }
}


/* function: mod exponent, this could be used for rsa decrypting and signing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent, actually private key d
 *     pub ------------------------ input, public key e
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordlen ---------------- input, word length of modulus and base number
 *     exp_wordlen ---------------- input, word length of exponent
 *     pub_wordlen ---------------- input, word length of pub
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure R^2 mod modulus, the pre-calculated
 *        mont arguments of modulus is located in the right address
 *     2. modulus must be odd
 *     3. please make sure exp_wordlen <= mod_wordlen <= OPERAND_MAX_WORD_LEN
 *     4. please make sure pub_wordlen <= 2
 *     5. please make sure value of exponent should be bigger than 1
 */
uint32_t pke_modexp_with_pub(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *pub, const uint32_t *base,
                             uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen, uint32_t pub_wordlen)
{
    uint32_t temp = get_valid_bits(exponent, exp_wordlen);
    uint32_t bitlen = (temp - 1) & 31;
    uint32_t ret;

    exp_wordlen = GET_WORD_LEN(temp);

    (void)get_rand((uint8_t *)(PKE_A(3, g_oper_step)), exp_wordlen << 2);

    pke_set_operand_width(mod_wordlen << 5);

    if (0 == bitlen) {
        *((volatile uint32_t *)((PKE_A(3, g_oper_step)) + exp_wordlen - 1)) = 0;
    }
    else {
        *((volatile uint32_t *)((PKE_A(3, g_oper_step)) + exp_wordlen - 1)) &= (0xFFFFFFFF >> (32 - bitlen));
    }

    pke_load_operand((uint32_t *)(PKE_B(2, g_oper_step)), (uint32_t *)pub, pub_wordlen);

    if ((g_oper_step / 4) > pub_wordlen) {
        uint32_clear((uint32_t *)(PKE_B(2, g_oper_step)) + pub_wordlen, (g_oper_step / 4) - pub_wordlen);
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_A(2, g_oper_step)), (uint32_t *)exponent, exp_wordlen);

    if ((g_oper_step / 4) > exp_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(2, g_oper_step)) + exp_wordlen, (g_oper_step / 4) - exp_wordlen);
        uint32_clear((uint32_t *)(PKE_A(3, g_oper_step)) + exp_wordlen, (g_oper_step / 4) - exp_wordlen);
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_A(0, g_oper_step)), (uint32_t *)modulus, mod_wordlen);
    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), (uint32_t *)base, mod_wordlen);

    if ((g_oper_step / 4) > mod_wordlen) {
        uint32_clear((uint32_t *)(PKE_A(0, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + mod_wordlen, (g_oper_step / 4) - mod_wordlen);
    }
    else
    {;}

    ret = pke_pre_calc_mont_N0();

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(3, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), mod_wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP_WITH_PUBKEY);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_A(0, g_oper_step)), mod_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(2, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(3, g_oper_step)), exp_wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), mod_wordlen << 2);
#endif
        return ret;
    }
    else {
        pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), out, mod_wordlen);

        return PKE_SUCCESS;
    }
}


/* function: ECCP curve sec point mul, Q=[k]P, P is a random point on curve
 * parameters:
 *     curve ---------------------- input, eccp_curve_t curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_sec(eccp_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                           uint32_t *Qx, uint32_t *Qy)
{
    uint32_t wordlen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t ret = 0;

    /*for k = n-1, the hardware does not support(it return code is PKE_NO_MODINV), so here check it.
    actually, now R1 = [n]G, R0 = [n-1]G, but it can not get y coordinate of output point Q since
    R1 can not be represented in affine coordinates.*/
    if (k[0] == curve->eccp_n[0] - 1) {
        if (0 == uint32_bignumcmp(k + 1, wordlen - 1, curve->eccp_n + 1, wordlen - 1)) {
            uint32_copy(Qx, Px, wordlen);
            return pke_sub(curve->eccp_p, Py, Qy, wordlen);
        }
        else
        {;}
    }
    else
    {;}

    /*set ecc_p, ecc_p_h, etc.*/
    ret = pke_set_modulus_and_pre_mont(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);

    if (PKE_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    pke_load_operand((uint32_t *)(PKE_B(1, g_oper_step)), Px, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(2, g_oper_step)), Py, wordlen);
    *((uint32_t *)(PKE_A(3, g_oper_step))) = 1;
    uint32_clear((uint32_t *)(PKE_A(3, g_oper_step)) + 1, (g_oper_step / 4) - 1);

    pke_load_operand((uint32_t *)(PKE_B(3, g_oper_step)), curve->eccp_b, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(4, g_oper_step)), curve->eccp_a, wordlen);
    pke_load_operand((uint32_t *)(PKE_B(5, g_oper_step)), curve->eccp_n, wordlen);
    pke_load_operand((uint32_t *)(PKE_A(4, g_oper_step)), k, wordlen);

    if ((g_oper_step / 4) > wordlen) {
        uint32_clear((uint32_t *)(PKE_B(1, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(2, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(3, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_B(5, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
        uint32_clear((uint32_t *)(PKE_A(4, g_oper_step)) + wordlen, (g_oper_step / 4) - wordlen);
    }
    else
    {;}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMUL_SEC);

    if (PKE_SUCCESS != ret) {
#ifdef PKE_SEC
        get_rand_fast((uint8_t *)(PKE_B(1, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(2, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(3, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(4, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_B(5, g_oper_step)), wordlen << 2);
        get_rand_fast((uint8_t *)(PKE_A(4, g_oper_step)), wordlen << 2);
#endif
        return ret;
    }
    else
    {;}

    pke_read_operand((uint32_t *)(PKE_A(1, g_oper_step)), Qx, wordlen);

    if (NULL != Qy) {
        pke_read_operand((uint32_t *)(PKE_A(2, g_oper_step)), Qy, wordlen);
    }
    else
    {;}

    return PKE_SUCCESS;
}

#endif

