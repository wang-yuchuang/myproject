/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <ral/pke.h>
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hash.h>
#include <driver/taishan/crypto/osr_crypto/cacc/ske/ske.h>
#include <driver/taishan/crypto/osr_crypto/cacc/pke/sm9.h>

#define SM9_BYTE_LEN     (32)
#define SM9_WORD_LEN     (8)

typedef struct {
    uint32_t re[8];
    uint32_t im[8];
} __mpc_struct;

typedef __mpc_struct mpc_t[1];
typedef __mpc_struct *mpc_ptr;

typedef struct {
    mpc_t ft2;
    mpc_t ft;
    mpc_t f;
} __mpc6_struct;

typedef __mpc6_struct mpc6_t[1];
typedef __mpc6_struct *mpc6_ptr;

typedef struct {
    mpc6_t fW;
    mpc6_t f;
} __mpc12_struct;

typedef __mpc12_struct mpc12_t[1];
typedef __mpc12_struct *mpc12_ptr;

typedef struct {
    uint32_t x[8];
    uint32_t y[8];
    uint32_t z[8];
} FP_POINT;

typedef FP_POINT fp_pt_t[1];

typedef struct {
    mpc_t x;
    mpc_t y;
    mpc_t z;
} FP2_POINT;

typedef FP2_POINT fp2_pt_t[1];

/*sm9 algorithm parameters*/
const uint32_t sm9p256v1_t[2]    = {0x0058F98A, 0x60000000};
const uint32_t sm9p256v1_p[8]    = {0xE351457D, 0xE56F9B27, 0x1A7AEEDB, 0x21F2934B, 0xF58EC745, 0xD603AB4F, 0x02A3A6F1, 0xB6400000};
const uint32_t sm9p256v1_p_h[8]  = {0xB417E2D2, 0x27DEA312, 0xAE1A5D3F, 0x88F8105F, 0xD6706E7B, 0xE479B522, 0x56F62FBD, 0x2EA795A6};
const uint32_t sm9p256v1_p_n1[1] = {0x2F2EE42B,};
const uint32_t sm9p256v1_a[8]    = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
const uint32_t sm9p256v1_b[8]    = {0x00000005, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
const uint32_t sm9p256v1_Gx[8]   = {0x7C66DDDD, 0xE8C4E481, 0x09DC3280, 0xE1E40869, 0x487D01D6, 0xF5ED0704, 0x62BF718F, 0x93DE051D};
const uint32_t sm9p256v1_Gy[8]   = {0x0A3EA616, 0x0C464CD7, 0xFA602435, 0x1C1C00CB, 0x5C395BBC, 0x63106512, 0x4F21E607, 0x21FE8DDA};
const uint32_t sm9p256v1_n[8]    = {0xD69ECF25, 0xE56EE19C, 0x18EA8BEE, 0x49F2934B, 0xF58EC744, 0xD603AB4F, 0x02A3A6F1, 0xB6400000};
const uint32_t sm9p256v1_n_h[8]  = {0xCD750C35, 0x7598CD79, 0xBB6DAEAB, 0xE4A08110, 0x7D78A1F9, 0xBFEE4BAE, 0x63695D0E, 0x8894F5D1};
const uint32_t sm9p256v1_n_n1[1] = {0x51974B53,};

/*sm9 para (n-1), for private key checking*/
uint32_t const sm9p256v1_n_1[8]  = {0xD69ECF24, 0xE56EE19C, 0x18EA8BEE, 0x49F2934B, 0xF58EC744, 0xD603AB4F, 0x02A3A6F1, 0xB6400000};

const eccp_curve_t sm9_curve[1] = {
    {
        256,
        256,
        (uint32_t *)sm9p256v1_p,
        (uint32_t *)sm9p256v1_p_h,
        (uint32_t *)sm9p256v1_a,
        (uint32_t *)sm9p256v1_b,
        (uint32_t *)sm9p256v1_Gx,
        (uint32_t *)sm9p256v1_Gy,
        (uint32_t *)sm9p256v1_n,
        (uint32_t *)sm9p256v1_n_h,
        NULL,
        NULL,
    },
};

/*P1*/
const fp2_pt_t fp2ptP1 = {
    {
        {{{0x7C66DDDD, 0xE8C4E481, 0x09DC3280, 0xE1E40869, 0x487D01D6, 0xF5ED0704, 0x62BF718F, 0x93DE051D}, {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}},
        {{{0x0A3EA616, 0x0C464CD7, 0xFA602435, 0x1C1C00CB, 0x5C395BBC, 0x63106512, 0x4F21E607, 0x21FE8DDA}, {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}},
        {{{0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}}
    }
};

/*P2*/
const fp2_pt_t fp2ptP2 = {
    {
        {{{0xAF82D65B, 0xF9B7213B, 0xD19C17AB, 0xEE265948, 0xD34EC120, 0xD2AAB97F, 0x92130B08, 0x37227552}, {0xD8806141, 0x54806C11, 0x0F5E93C4, 0xF1DD2C19, 0xB441A01F, 0x597B6027, 0x78640C98, 0x85AEF3D0}}},
        {{{0xC999A7C7, 0x6215BBA5, 0xA71A0811, 0x47EFBA98, 0x3D278FF2, 0x5F317015, 0x19BE3DA6, 0xA7CF28D5}, {0x84EBEB96, 0x856DC76B, 0xA347C8BD, 0x0736A96F, 0x2CBEE6ED, 0x66BA0D26, 0x2E845C12, 0x17509B09}}},
        {{{0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}}
    }
};

static void U8Big_to_U32Small_8(uint32_t out[], const uint8_t in[])
{
    out[7] = ((uint32_t)in[3]) | ((uint32_t)in[2] << 8u) | ((uint32_t)in[1] << 16u) | ((uint32_t)in[0] << 24u);
    out[6] = ((uint32_t)in[7]) | ((uint32_t)in[6] << 8u) | ((uint32_t)in[5] << 16u) | ((uint32_t)in[4] << 24u);
    out[5] = ((uint32_t)in[11]) | ((uint32_t)in[10] << 8u) | ((uint32_t)in[9] << 16u) | ((uint32_t)in[8] << 24u);
    out[4] = ((uint32_t)in[15]) | ((uint32_t)in[14] << 8u) | ((uint32_t)in[13] << 16u) | ((uint32_t)in[12] << 24u);
    out[3] = ((uint32_t)in[19]) | ((uint32_t)in[18] << 8u) | ((uint32_t)in[17] << 16u) | ((uint32_t)in[16] << 24u);
    out[2] = ((uint32_t)in[23]) | ((uint32_t)in[22] << 8u) | ((uint32_t)in[21] << 16u) | ((uint32_t)in[20] << 24u);
    out[1] = ((uint32_t)in[27]) | ((uint32_t)in[26] << 8u) | ((uint32_t)in[25] << 16u) | ((uint32_t)in[24] << 24u);
    out[0] = ((uint32_t)in[31]) | ((uint32_t)in[30] << 8u) | ((uint32_t)in[29] << 16u) | ((uint32_t)in[28] << 24u);
}

static void U32Small_to_U8Big(uint8_t out[], const uint32_t in[])
{
    uint32_t i, j;

    for (i = 0; i < 8; i++) {
        j = 28 - (i << 2);
        out[j]     = ((uint8_t)(in[i] >> 24)) & 0xffu;
        out[j + 1] = ((uint8_t)(in[i] >> 16)) & 0xffu;
        out[j + 2] = ((uint8_t)(in[i] >> 8)) & 0xffu;
        out[j + 3] = ((uint8_t)in[i]) & 0xffu;
    }
}

/* Function: get aimed bit value of big integer a
 * Parameters:
 *     a ----------- big integer a
 *     bitLen ------ aimed bit location
 * Return:
 *     bit value of aimed bit
 * Caution:
 *     1. make sure bitLen > 0
 */
static uint32_t Get_BitValue(const uint32_t a[], uint32_t bitLen)
{
    bitLen--;

    if (0u != (a[(bitLen) >> 5u] & ((uint32_t)1u << (bitLen & 31u)))) {
        return 1u;
    }
    else {
        return 0u;
    }
}

/* function: a=a+1 (for SM2 KDF counter addition)
 * parameters:
 *     a[4] ----------------------- input, count of 4 bytes, big-endian
 * return: none
 * caution: if a of 4 bytes can not hold the carry, then the carry will be discarded
 */
void sm9_counter_add_one(uint8_t a[4])
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

/* function: a=a+1
 * parameters:
 *     a -------------------------- input, destination data
 *     wordLen -------------------- input, word length of data
 * return: none
 * caution:
 *     1. if a of wordLen words can not hold the carry, then the carry will be discarded
 */
static void sm9_BigNum_Add_One(uint32_t *a, uint32_t wordLen)
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

static void ibe_kdf_H1_H2_mod(uint32_t *result, uint8_t res[40])
{
    uint32_t t[8];
    uint32_t Ha[8];
    uint32_t Hb[8];
    uint32_t Hc[8];
    uint32_t N_compl[8]  = {0x296130DB, 0x1A911E63, 0xE7157411, 0xB60D6CB4, 0x0A7138BB, 0x29FC54B0, 0xFD5C590E, 0x49BFFFFF};
    uint32_t N2[8]       = {0xFFFFF8A5, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t N2_inv[8]   = {0xD81AE00B, 0x73FFE2F2, 0x3CA00AC3, 0x6EE46995, 0x859A2700, 0xECEE7342, 0xE23BB01C, 0xB707F075};
    uint32_t N2_compl[8] = {0x0000075B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

    Ha[0] = ((uint32_t)res[7]) | ((uint32_t)res[6] << 8u) | ((uint32_t)res[5] << 16u) | ((uint32_t)res[4] << 24u);
    Ha[1] = ((uint32_t)res[3]) | ((uint32_t)res[2] << 8u) | ((uint32_t)res[1] << 16u) | ((uint32_t)res[0] << 24u);
    Ha[2] = 0x00000000;
    Ha[3] = 0x00000000;
    Ha[4] = 0x00000000;
    Ha[5] = 0x00000000;
    Ha[6] = 0x00000000;
    Ha[7] = 0x00000000;

    U8Big_to_U32Small_8(Hb, (uint8_t *)(res + 8));
    pke_modmul(sm9p256v1_n, Ha, N_compl, Hc, 8);

    if (uint32_bignumcmp(Hb, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        pke_sub(Hb, sm9p256v1_n, t, 8);
        pke_modadd(sm9p256v1_n, Hc, t, Hc, 8);
    }
    else {
        pke_modadd(sm9p256v1_n, Hc, Hb, Hc, 8);
    }

    if (uint32_bignumcmp(Hb, 8, Hc, 8) >= 0) {
        pke_sub(Hb, Hc, Hb, 8);
    }
    else {
        pke_sub(Hb, Hc, Hb, 8);

        if (Ha[0] != 0x00000000) {
            Ha[0] -= 0x00000001;
        }
        else {
            Ha[0] = 0xFFFFFFFF;
            Ha[1] -= 0x00000001;
        }
    }

    pke_modmul(N2, Ha, N2_compl, Ha, 8);

    if (uint32_bignumcmp(Hb, 8, N2, 8) >= 0) {
        pke_sub(Hb, N2, Hb, 8);
    }

    pke_modadd(N2, Ha, Hb, Ha, 8);
    pke_modmul(N2, Ha, N2_inv, Hb, 8);

    if (uint32_bignumcmp(Hc, 8, (uint32_t *)sm9p256v1_n_1, 8) >= 0) {
        pke_sub(Hc, sm9p256v1_n_1, Hc, 8);
    }

    pke_modadd(sm9p256v1_n_1, Hc, Hb, result, 8);
    sm9_BigNum_Add_One(result, 8);
}
/*K1 can not be NULL, K2 is ok*/
static void ibe_kdf_wrap_enc(const uint8_t *C1, const uint8_t *W, const uint8_t *ID, uint32_t byteLenofID,
                             uint8_t *K1, uint32_t byteLenofK1, uint8_t *K2, uint32_t byteLenofK2)
{
    uint8_t counter[4] = {0x00, 0x00, 0x00, 0x01};
    uint8_t digest[32];
    uint32_t i, t;

    hash_ctx_t ctx[1];

    t = byteLenofK1 >> 5;

    for (i = 0; i < t; i++) {
        hash_init(ctx, HASH_SM3);
        hash_update(ctx, C1, 64);
        hash_update(ctx, W, 32 * 12);
        hash_update(ctx, ID, byteLenofID);
        hash_update(ctx, counter, 4);
        hash_final(ctx, K1 + (i << 5));
        sm9_counter_add_one(counter);
    }

    i = byteLenofK1 & 0x1F;

    if (i) {
        hash_init(ctx, HASH_SM3);
        hash_update(ctx, C1, 64);
        hash_update(ctx, W, 32 * 12);
        hash_update(ctx, ID, byteLenofID);
        hash_update(ctx, counter, 4);
        hash_final(ctx, digest);
        memcpy_(K1 + (t << 5), digest, i);
    }

    if (K2 && byteLenofK2) {
        if (i) {
            t = 0x20 - i;

            if (byteLenofK2 <= t) {
                memcpy_(K2, digest + i, byteLenofK2);
                return;
            }
            else {
                memcpy_(K2, digest + i, t);
                byteLenofK2 -= t;
                K2 += t;
                sm9_counter_add_one(counter);
            }
        }

        t = byteLenofK2 >> 5;

        for (i = 0; i < t; i++) {
            hash_init(ctx, HASH_SM3);
            hash_update(ctx, C1, 64);
            hash_update(ctx, W, 32 * 12);
            hash_update(ctx, ID, byteLenofID);
            hash_update(ctx, counter, 4);
            hash_final(ctx, K2 + (i << 5));
            sm9_counter_add_one(counter);
        }

        i = byteLenofK2 & 0x1F;

        if (i) {
            hash_init(ctx, HASH_SM3);
            hash_update(ctx, C1, 64);
            hash_update(ctx, W, 32 * 12);
            hash_update(ctx, ID, byteLenofID);
            hash_update(ctx, counter, 4);
            hash_final(ctx, digest);
            memcpy_(K2 + (t << 5), digest, i);
        }
    }
}

static void ibe_kdf_H1_H2(uint8_t *res, hash_ctx_t *ctx, const uint8_t *z1, uint32_t z1ByteLen,
                          const uint8_t *z2, uint32_t z2ByteLen, const uint8_t *cnt, const uint8_t *tag)
{
    hash_init(ctx, HASH_SM3);

    hash_update(ctx, tag, 1);
    hash_update(ctx, z1, z1ByteLen);
    hash_update(ctx, z2, z2ByteLen);
    hash_update(ctx, cnt, 4);

    hash_final(ctx, res);
}


static void ibe_kdf_exc_key(const uint8_t *IDA, uint32_t byteLenofIDA,
                            const uint8_t *IDB, uint32_t byteLenofIDB,
                            const uint8_t *RA, const uint8_t *RB,
                            const uint8_t *g, uint8_t *K, uint32_t byteLenofK)
{
    uint8_t digest[32];
    uint32_t i, t;
    hash_ctx_t ctx[1];
    uint8_t counter[4] = {0x00, 0x00, 0x00, 0x01};

    t = byteLenofK >> 5;

    for (i = 0; i < t; i++) {
        hash_init(ctx, HASH_SM3);
        hash_update(ctx, IDA, byteLenofIDA);
        hash_update(ctx, IDB, byteLenofIDB);
        hash_update(ctx, RA, 64);
        hash_update(ctx, RB, 64);
        hash_update(ctx, g, 32 * 12 * 3);
        hash_update(ctx, counter, 4);
        hash_final(ctx, K + (i << 5));
        sm9_counter_add_one(counter);
    }

    i = byteLenofK & 0x1F;

    if (i) {
        hash_init(ctx, HASH_SM3);
        hash_update(ctx, IDA, byteLenofIDA);
        hash_update(ctx, IDB, byteLenofIDB);
        hash_update(ctx, RA, 64);
        hash_update(ctx, RB, 64);
        hash_update(ctx, g, 32 * 12 * 3);
        hash_update(ctx, counter, 4);
        hash_final(ctx, digest);
        memcpy_(K + (t << 5), digest, i);
    }
}

/*The realization of the functions H1(Z,n) and H2(Z,n), the difference between the two is only the tag value is different
Where Z=z1||z2, n is the order of the elliptic curve, after the function is executed, a large number less than n is generated*/
static void ibe_H1_H2(uint8_t tag, const uint8_t *z1, uint32_t z1ByteLen, const uint8_t *z2, uint32_t z2ByteLen, uint32_t *result)
{
    uint8_t cnt[4] = {0, 0, 0, 1};
    uint8_t res[64];
    hash_ctx_t ctx[1];

    ibe_kdf_H1_H2(res, ctx, z1, z1ByteLen, z2, z2ByteLen, cnt, &tag);

    cnt[3] = 0x02;
    ibe_kdf_H1_H2(res + 32, ctx, z1, z1ByteLen, z2, z2ByteLen, cnt, &tag);

    ibe_kdf_H1_H2_mod(result, res);
}

static void FE2OSP(mpc12_t input, uint8_t *result)
{
    U32Small_to_U8Big(result + 32 * 11, input->f->f->re);
    U32Small_to_U8Big(result + 32 * 10, input->f->f->im);
    U32Small_to_U8Big(result + 32 * 3, input->f->ft->re);
    U32Small_to_U8Big(result + 32 * 2, input->f->ft->im);
    U32Small_to_U8Big(result + 32 * 5, input->f->ft2->re);
    U32Small_to_U8Big(result + 32 * 4, input->f->ft2->im);
    U32Small_to_U8Big(result + 32 * 7, input->fW->f->re);
    U32Small_to_U8Big(result + 32 * 6, input->fW->f->im);
    U32Small_to_U8Big(result + 32 * 9, input->fW->ft->re);
    U32Small_to_U8Big(result + 32 * 8, input->fW->ft->im);
    U32Small_to_U8Big(result + 32, input->fW->ft2->re);
    U32Small_to_U8Big(result, input->fW->ft2->im);
}

static uint8_t uint32_cmp(uint32_t out[8], const uint32_t in[8])
{
    uint32_t i;

    for (i = 0; i < 8u; i++) {
        if (out[i] != in[i]) {
            return 1;
        }
    }

    return 0;
}

static void uint32_mod(uint32_t out[8], const uint32_t in[8], const uint32_t p[8])
{
    const uint32_t tmp[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    (void)pke_modadd(p, in, tmp, out, 8);
}

static void mpc_fp2_set(mpc_t dst, const mpc_t src)
{
    uint32_copy(dst->re, (uint32_t *)src->re, 8);
    uint32_copy(dst->im, (uint32_t *)src->im, 8);
}

static void mpc_fp2_mod(mpc_t dst, const mpc_t src, const uint32_t p[8])
{
    uint32_mod(dst->re, src->re, p);
    uint32_mod(dst->im, src->im, p);
}

static void mpc_fp2_add(mpc_t dst, const mpc_t x, const mpc_t y, const uint32_t p[8])
{
    pke_modadd(p, x->re, y->re, dst->re, 8);
    pke_modadd(p, x->im, y->im, dst->im, 8);
}

static void mpc_fp2_sub(mpc_t dst, const mpc_t x, const mpc_t y, const uint32_t p[8])
{
    pke_modsub(p, x->re, y->re, dst->re, 8);
    pke_modsub(p, x->im, y->im, dst->im, 8);
}

/*Two-element modular multiplication in Fp^2, dst = (x * y) mod p*/
static void mpc_fp2_mul(mpc_t dst, const mpc_t x, const mpc_t y, const uint32_t p[8])
{
    uint32_t a[8];
    uint32_t b[8];
    uint32_t c[8];

    (void)pke_modmul(p, x->re, y->re, a, 8);
    (void)pke_modmul(p, x->im, y->im, b, 8);
    pke_modadd(p, b, b, b, 8);
    pke_modsub(p, a, b, c, 8);

    (void)pke_modmul(p, x->re, y->im, a, 8);
    (void)pke_modmul(p, x->im, y->re, b, 8);
    pke_modadd(p, a, b, dst->im, 8);
    uint32_copy(dst->re, c, 8);
}

static void mpc_fp2_clears(mpc_ptr x)
{
    uint32_clear(x->re, 8);
    uint32_clear(x->im, 8);
}

/*E(Fp^2) point on Jacobian coordinates to point on affine coordinates*/
static void coordinate_convert(mpc_t x_in, mpc_t y_in, mpc_t z_in, mpc_t x_out, mpc_t y_out)
{
    mpc_t z1, z2;
    uint32_t inv1[8];
    uint32_t inv2[8];

    pke_modmul(sm9p256v1_p, z_in->im, z_in->im, inv1, 8);
    pke_sub(sm9p256v1_p, inv1, inv1, 8);
    pke_modadd(sm9p256v1_p, inv1, inv1, inv1, 8);

    pke_modmul(sm9p256v1_p, z_in->re, z_in->re, inv2, 8);
    pke_modsub(sm9p256v1_p, inv1, inv2, inv1, 8);

    (void)pke_modinv(sm9p256v1_p, inv1, inv1, 8, 8);

    mpc_fp2_set(z1, z_in);
    pke_sub(sm9p256v1_p, z1->re, z1->re, 8);

    uint32_copy(z2->re, inv1, 8);
    uint32_clear(z2->im, 8);

    mpc_fp2_mul(z1, z1, z2, sm9p256v1_p);

    mpc_fp2_mul(z2, z1, z1, sm9p256v1_p);
    mpc_fp2_mul(z1, z2, z1, sm9p256v1_p);

    mpc_fp2_mul(x_out, x_in, z2, sm9p256v1_p);
    mpc_fp2_mul(y_out, y_in, z1, sm9p256v1_p);

    mpc_fp2_clears(z_in);
    z_in->re[0] = 0x00000001;
}

static void mpc_fp6_add(mpc6_t dst, mpc6_t a, mpc6_t b, const uint32_t p[8])
{
    mpc_fp2_add(dst->f, a->f, b->f, p);
    mpc_fp2_add(dst->ft, a->ft, b->ft, p);
    mpc_fp2_add(dst->ft2, a->ft2, b->ft2, p);
}

static void mpc_fp6_sub(mpc6_t dst, mpc6_t a, mpc6_t b, const uint32_t p[8])
{
    mpc_fp2_sub(dst->f, a->f, b->f, p);
    mpc_fp2_sub(dst->ft, a->ft, b->ft, p);
    mpc_fp2_sub(dst->ft2, a->ft2, b->ft2, p);
}

static void mpc_fp6_set(mpc6_t dst, mpc6_t src)
{
    mpc_fp2_set(dst->f, src->f);
    mpc_fp2_set(dst->ft, src->ft);
    mpc_fp2_set(dst->ft2, src->ft2);
}

static void mpc_fp6_clears(mpc6_ptr x)
{
    mpc_fp2_clears(x->f);
    mpc_fp2_clears(x->ft);
    mpc_fp2_clears(x->ft2);
}

static void mpc_fp12_clears(mpc12_ptr x)
{
    mpc_fp2_clears(x->f->f);
    mpc_fp2_clears(x->f->ft);
    mpc_fp2_clears(x->f->ft2);
    mpc_fp2_clears(x->fW->f);
    mpc_fp2_clears(x->fW->ft);
    mpc_fp2_clears(x->fW->ft2);
}

/*all declared as mpc_t to fit multiplications in the later functions*/
mpc_t KECI;
mpc_t k1, k2, k3, k4, k5;
mpc_t g1, g2, g3, g4, g5;
mpc_t e1, e2, e3, e4, e5;
mpc_t scal1, scal2;

static void KECI_init(void)
{
    uint32_clear(KECI->re, 8);
    uint32_clear(KECI->im, 8);
    KECI->im[0] = 1;
}

static void kn_init(void)
{
    uint32_t k1_re[SM9_WORD_LEN]  = {0x377b698b, 0xa91d8354, 0x0ddd04ed, 0x47c5c86e, 0x9c086749, 0x843c6cfa, 0xe5720bdb, 0x3f23ea58,};
    uint32_t k2_re[SM9_WORD_LEN]  = {0x7be65334, 0xd5fc1196, 0x4f8b78f4, 0x78027235, 0x02a3a6f2, 0xf3000000, 0x00000000, 0x00000000,};
    uint32_t k3_re[SM9_WORD_LEN]  = {0xda24d011, 0xf5b21fd3, 0x06dc5177, 0x9f9d4118, 0xee0baf15, 0xf55acc93, 0xdc0a3f2c, 0x6c648de5,};
    uint32_t k4_re[SM9_WORD_LEN]  = {0x7be65333, 0xd5fc1196, 0x4f8b78f4, 0x78027235, 0x02a3a6f2, 0xf3000000, 0x00000000, 0x00000000,};
    uint32_t k5_re[SM9_WORD_LEN]  = {0xa2a96686, 0x4c949c7f, 0xf8ff4c8a, 0x57d778a9, 0x520347cc, 0x711e5f99, 0xf6983351, 0x2d40a38c,};

    uint32_copy(k1->re, (uint32_t *)k1_re, SM9_WORD_LEN);
    uint32_copy(k2->re, (uint32_t *)k2_re, SM9_WORD_LEN);
    uint32_copy(k3->re, (uint32_t *)k3_re, SM9_WORD_LEN);
    uint32_copy(k4->re, (uint32_t *)k4_re, SM9_WORD_LEN);
    uint32_copy(k5->re, (uint32_t *)k5_re, SM9_WORD_LEN);

    uint32_clear(k1->im, SM9_WORD_LEN);
    uint32_clear(k2->im, SM9_WORD_LEN);
    uint32_clear(k3->im, SM9_WORD_LEN);
    uint32_clear(k4->im, SM9_WORD_LEN);
    uint32_clear(k5->im, SM9_WORD_LEN);
}

static void kn_clear()
{
    mpc_fp2_clears(k1);
    mpc_fp2_clears(k2);
    mpc_fp2_clears(k3);
    mpc_fp2_clears(k4);
    mpc_fp2_clears(k5);
}

static void gn_init(void)
{
    uint32_t g1_re[SM9_WORD_LEN]  = {0x7be65334, 0xd5fc1196, 0x4f8b78f4, 0x78027235, 0x02a3a6f2, 0xf3000000, 0x00000000, 0x00000000,};
    uint32_t g2_re[SM9_WORD_LEN]  = {0x7be65333, 0xd5fc1196, 0x4f8b78f4, 0x78027235, 0x02a3a6f2, 0xf3000000, 0x00000000, 0x00000000,};
    uint32_t g3_re[SM9_WORD_LEN]  = {0xe351457c, 0xe56f9b27, 0x1a7aeedb, 0x21f2934b, 0xf58ec745, 0xd603ab4f, 0x02a3a6f1, 0xb6400000,};
    uint32_t g4_re[SM9_WORD_LEN]  = {0x676af249, 0x0f738991, 0xcaef75e7, 0xa9f02115, 0xf2eb2052, 0xe303ab4f, 0x02a3a6f0, 0xb6400000,};
    uint32_t g5_re[SM9_WORD_LEN]  = {0x676af24a, 0x0f738991, 0xcaef75e7, 0xa9f02115, 0xf2eb2052, 0xe303ab4f, 0x02a3a6f0, 0xb6400000,};

    uint32_copy(g1->re, (uint32_t *)g1_re, SM9_WORD_LEN);
    uint32_copy(g2->re, (uint32_t *)g2_re, SM9_WORD_LEN);
    uint32_copy(g3->re, (uint32_t *)g3_re, SM9_WORD_LEN);
    uint32_copy(g4->re, (uint32_t *)g4_re, SM9_WORD_LEN);
    uint32_copy(g5->re, (uint32_t *)g5_re, SM9_WORD_LEN);

    uint32_clear(g1->im, SM9_WORD_LEN);
    uint32_clear(g2->im, SM9_WORD_LEN);
    uint32_clear(g3->im, SM9_WORD_LEN);
    uint32_clear(g4->im, SM9_WORD_LEN);
    uint32_clear(g5->im, SM9_WORD_LEN);
}

static void gn_clear(void)
{
    mpc_fp2_clears(g1);
    mpc_fp2_clears(g2);
    mpc_fp2_clears(g3);
    mpc_fp2_clears(g4);
    mpc_fp2_clears(g5);
}

static void en_init(void)
{
    uint32_t e1_re[SM9_WORD_LEN]  = {0xda24d011, 0xf5b21fd3, 0x06dc5177, 0x9f9d4118, 0xee0baf15, 0xf55acc93, 0xdc0a3f2c, 0x6c648de5,};
    uint32_t e2_re[SM9_WORD_LEN]  = {0xe351457c, 0xe56f9b27, 0x1a7aeedb, 0x21f2934b, 0xf58ec745, 0xd603ab4f, 0x02a3a6f1, 0xb6400000,};
    uint32_t e3_re[SM9_WORD_LEN]  = {0x092c756c, 0xefbd7b54, 0x139e9d63, 0x82555233, 0x0783182f, 0xe0a8debc, 0x269967c4, 0x49db721a,};
    uint32_t e4_re[SM9_WORD_LEN]  = {0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,};
    uint32_t e5_re[SM9_WORD_LEN]  = {0xda24d011, 0xf5b21fd3, 0x06dc5177, 0x9f9d4118, 0xee0baf15, 0xf55acc93, 0xdc0a3f2c, 0x6c648de5,};

    kn_init();

    uint32_copy(e1->re, (uint32_t *)e1_re, SM9_WORD_LEN);
    uint32_copy(e2->re, (uint32_t *)e2_re, SM9_WORD_LEN);
    uint32_copy(e3->re, (uint32_t *)e3_re, SM9_WORD_LEN);
    uint32_copy(e4->re, (uint32_t *)e4_re, SM9_WORD_LEN);
    uint32_copy(e5->re, (uint32_t *)e5_re, SM9_WORD_LEN);

    uint32_clear(e1->im, SM9_WORD_LEN);
    uint32_clear(e2->im, SM9_WORD_LEN);
    uint32_clear(e3->im, SM9_WORD_LEN);
    uint32_clear(e4->im, SM9_WORD_LEN);
    uint32_clear(e5->im, SM9_WORD_LEN);
}

static void en_clear(void)
{
    mpc_fp2_clears(e1);
    mpc_fp2_clears(e2);
    mpc_fp2_clears(e3);
    mpc_fp2_clears(e4);
    mpc_fp2_clears(e5);
}

static void scal_init(void)
{
    uint32_t scal1_re[SM9_WORD_LEN]  = {0x676af24a, 0x0f738991, 0xcaef75e7, 0xa9f02115, 0xf2eb2052, 0xe303ab4f, 0x02a3a6f0, 0xb6400000,};
    uint32_t scal2_re[SM9_WORD_LEN]  = {0x092c756c, 0xefbd7b54, 0x139e9d63, 0x82555233, 0x0783182f, 0xe0a8debc, 0x269967c4, 0x49db721a,};

    uint32_copy(scal1->re, (uint32_t *)scal1_re, SM9_WORD_LEN);
    uint32_copy(scal2->re, (uint32_t *)scal2_re, SM9_WORD_LEN);

    uint32_clear(scal1->im, SM9_WORD_LEN);
    uint32_clear(scal2->im, SM9_WORD_LEN);
}

/*Point multiplication operation of curve E(Fp^2)*/
static void PD_fp2(mpc_t x, mpc_t y, mpc_t z, mpc_t xo, mpc_t yo, mpc_t zo, const uint32_t p[8])
{
    mpc_t R1, R2, R3, R5, R6, R7;

    mpc_fp2_set(R1, x);
    mpc_fp2_set(R2, y);
    mpc_fp2_set(R3, z);

    mpc_fp2_mul(R6, R3, R2, p);
    mpc_fp2_mul(R5, R1, R1, p);
    mpc_fp2_add(R1, R1, R1, p);
    mpc_fp2_add(R1, R1, R1, p);
    mpc_fp2_add(R3, R6, R6, p);
    mpc_fp2_add(R6, R5, R5, p);
    mpc_fp2_add(R5, R6, R5, p);
    mpc_fp2_mul(R6, R2, R2, p);
    mpc_fp2_mul(R2, R6, R1, p);
    mpc_fp2_mul(R7, R5, R5, p);
    mpc_fp2_add(R1, R2, R2, p);
    mpc_fp2_sub(R1, R7, R1, p);
    mpc_fp2_mul(R7, R6, R6, p);
    mpc_fp2_sub(R2, R2, R1, p);
    mpc_fp2_mul(R5, R5, R2, p);
    mpc_fp2_add(R7, R7, R7, p);
    mpc_fp2_add(R7, R7, R7, p);
    mpc_fp2_add(R7, R7, R7, p);
    mpc_fp2_sub(R2, R5, R7, p);
    mpc_fp2_mod(xo, R1, p);
    mpc_fp2_mod(yo, R2, p);
    mpc_fp2_mod(zo, R3, p);
}

/*Point addition operation of curve E(Fp^2)
    x:=(y2*z^3-y)^2-(x2*z^2-x)^2*(x+x2*z^2);
    y:=(y2*z^3-y)*(x*(x2*z^2-x)^2-X)-y*(x2*z^2-x)^3;
    z:=(x2*z^2-x)*z;*/
static void PA_fp2(mpc_t x, mpc_t y, mpc_t z, const mpc_t x2, const mpc_t y2, const uint32_t p[8])
{
    mpc_t A, B, C, D, E;

    mpc_fp2_mul(A, z, z, p);
    mpc_fp2_mul(B, x2, A, p);
    mpc_fp2_mul(A, A, z, p);
    /*x2*z^2-x*/
    mpc_fp2_sub(D, B, x, p);
    mpc_fp2_mul(z, D, z, p);
    /*x2*z^2+x*/
    mpc_fp2_add(E, B, x, p);
    /*y2*z^3*/
    mpc_fp2_mul(B, A, y2, p);
    /*(x2*z^2-x)^2*/
    mpc_fp2_mul(C, D, D, p);
    mpc_fp2_mul(A, C, E, p);
    mpc_fp2_mul(E, x, C, p);
    mpc_fp2_mul(C, C, D, p);
    mpc_fp2_mul(C, C, y, p);
    /*y2*z^3-y*/
    mpc_fp2_sub(D, B, y, p);
    mpc_fp2_mul(B, D, D, p);
    mpc_fp2_sub(x, B, A, p);
    /*(x*(x2*z^2-x)^2-X)*/
    mpc_fp2_sub(E, E, x, p);
    mpc_fp2_mul(E, E, D, p);
    mpc_fp2_sub(y, E, C, p);

    mpc_fp2_mod(x, x, p);
    mpc_fp2_mod(z, z, p);
    mpc_fp2_mod(y, y, p);
}

/*Fp2 multiplication*/
static void Mp2(const mpc_t a, const mpc_t b, mpc_t k, const uint32_t p[8])
{
    uint32_t A[8];
    uint32_t B[8];
    uint32_t C[8];
    pke_modadd(p, a->re, a->im, C, 8);
    pke_modadd(p, b->re, b->im, B, 8);

    (void)pke_modmul(p, C, B, C, 8);
    (void)pke_modmul(p, a->re, b->re, A, 8);
    (void)pke_modmul(p, a->im, b->im, B, 8);

    pke_modsub(p, C, A, C, 8);
    pke_modsub(p, C, B, k->im, 8);
    pke_modadd(p, B, B, B, 8);
    pke_modsub(p, A, B, k->re, 8);
}

static void sp6(mpc_t ft2, mpc_t ft, mpc_t f, const uint32_t p[8])
{
    mpc_t a1, a2, a3, A, B, D;

    KECI_init();

    mpc_fp2_set(a1, f);
    mpc_fp2_set(a2, ft);
    mpc_fp2_set(a3, ft2);

    mpc_fp2_mul(A, a1, a1, p);
    mpc_fp2_mul(B, a1, a2, p);
    mpc_fp2_add(B, B, B, p);
    mpc_fp2_sub(a1, a1, a2, p);
    mpc_fp2_add(a1, a1, a3, p);
    mpc_fp2_mul(a1, a1, a1, p);
    mpc_fp2_mul(D, a2, a3, p);
    mpc_fp2_add(D, D, D, p);
    mpc_fp2_mul(a2, a3, a3, p);
    mpc_fp2_sub(a1, a1, a2, p);
    mpc_fp2_add(a1, a1, D, p);
    mpc_fp2_mul(D, D, KECI, p);
    mpc_fp2_mul(a2, a2, KECI, p);
    mpc_fp2_sub(a1, a1, A, p);
    mpc_fp2_add(A, A, D, p);
    mpc_fp2_add(a1, a1, B, p);
    mpc_fp2_add(B, B, a2, p);

    mpc_fp2_mod(f, A, p);
    mpc_fp2_mod(ft, B, p);
    mpc_fp2_mod(ft2, a1, p);
}

static void sp12(mpc12_t f, const uint32_t p[8])
{
    mpc_t Bt2, Bt, B, Ct2, Ct, C, Dt2, Dt, D;

    KECI_init();

    mpc_fp2_set(Ct2, f->fW->ft2);
    mpc_fp2_set(Ct, f->fW->ft);
    mpc_fp2_set(C, f->fW->f);
    sp6(Ct2, Ct, C, p);

    mpc_fp2_set(Bt2, f->f->ft2);
    mpc_fp2_set(Bt, f->f->ft);
    mpc_fp2_set(B, f->f->f);
    sp6(Bt2, Bt, B, p);

    mpc_fp2_add(Dt2, f->fW->ft2, f->f->ft2, p);
    mpc_fp2_add(Dt, f->fW->ft, f->f->ft, p);
    mpc_fp2_add(D, f->fW->f, f->f->f, p);
    sp6(Dt2, Dt, D, p);

    mpc_fp2_sub(Dt2, Dt2, Bt2, p);
    mpc_fp2_sub(Dt, Dt, Bt, p);
    mpc_fp2_sub(D, D, B, p);

    mpc_fp2_sub(Dt2, Dt2, Ct2, p);
    mpc_fp2_sub(Dt, Dt, Ct, p);
    mpc_fp2_sub(D, D, C, p);

    mpc_fp2_add(Bt2, Bt2, Ct, p);
    mpc_fp2_add(Bt, Bt, C, p);
    mpc_fp2_mul(Ct2, Ct2, KECI, p);
    mpc_fp2_add(B, B, Ct2, p);

    mpc_fp2_set(f->f->ft2, Bt2);
    mpc_fp2_set(f->f->ft, Bt);
    mpc_fp2_set(f->f->f, B);
    mpc_fp2_set(f->fW->ft2, Dt2);
    mpc_fp2_set(f->fW->ft, Dt);
    mpc_fp2_set(f->fW->f, D);
}

static void Mp6(mpc6_t a, mpc6_t b, mpc6_t g, const uint32_t p[8])
{
    mpc_t a1, a2, a3, b1, b2, b3, A, B, C, D, E, F, G;

    KECI_init();

    mpc_fp2_set(a1, a->f);
    mpc_fp2_set(a2, a->ft);
    mpc_fp2_set(a3, a->ft2);

    mpc_fp2_set(b1, b->f);
    mpc_fp2_set(b2, b->ft);
    mpc_fp2_set(b3, b->ft2);

    mpc_fp2_mul(A, a1, b1, p);
    mpc_fp2_mul(B, a2, b2, p);
    mpc_fp2_add(C, a1, a2, p);
    mpc_fp2_add(D, b1, b2, p);
    mpc_fp2_mul(E, C, D, p);
    mpc_fp2_sub(E, E, A, p);
    mpc_fp2_sub(E, E, B, p);
    mpc_fp2_add(C, a1, a3, p);
    mpc_fp2_add(D, b1, b3, p);
    mpc_fp2_mul(F, a3, b3, p);
    mpc_fp2_mul(C, C, D, p);
    mpc_fp2_sub(C, C, A, p);
    mpc_fp2_sub(C, C, F, p);
    mpc_fp2_add(D, B, C, p);
    mpc_fp2_add(C, a2, a3, p);
    mpc_fp2_add(G, b2, b3, p);
    mpc_fp2_mul(C, C, G, p);
    mpc_fp2_sub(C, C, B, p);
    mpc_fp2_sub(C, C, F, p);

    mpc_fp2_mul(C, C, KECI, p);
    mpc_fp2_add(C, C, A, p);

    mpc_fp2_set(g->f, C);

    mpc_fp2_mul(F, F, KECI, p);
    mpc_fp2_add(F, F, E, p);

    mpc_fp2_set(g->ft, F);
    mpc_fp2_set(g->ft2, D);
}

/*calculate Fp12 multiplication by down-dimension to Fp6*/
static void fl(mpc12_t F, mpc12_t L, const uint32_t p[8])
{
    mpc_t m;
    mpc6_t C1, C2, A1, A2;

    mpc_fp6_add(C1, F->f, F->fW, p);
    mpc_fp6_add(C2, L->f, L->fW, p);
    Mp6(C1, C2, C1, p);
    Mp6(F->f, L->f, A1, p);
    Mp6(F->fW, L->fW, A2, p);
    mpc_fp6_sub(C1, C1, A1, p);
    mpc_fp6_sub(C1, C1, A2, p);

    /*A2:=A2*t*/
    KECI_init();
    /*ft2->f*/
    mpc_fp2_mul(m, A2->ft2, KECI, p);
    mpc_fp2_set(A2->ft2, A2->ft);
    mpc_fp2_set(A2->ft, A2->f);
    mpc_fp2_set(A2->f, m);

    mpc_fp6_add(A2, A2, A1, p);
    mpc_fp6_set(F->f, A2);
    mpc_fp6_set(F->fW, C1);
}

/*Two-element modular multiplication in Fq^12, g = a * b % p*/
static void Mp12(mpc12_t a, mpc12_t b, mpc12_t g, const uint32_t p[8])
{
    mpc6_t a1, a2, b1, b2, C, D;
    mpc_t m;

    mpc_fp6_set(a1, a->f);
    mpc_fp6_set(a2, a->fW);

    mpc_fp6_set(b1, b->f);
    mpc_fp6_set(b2, b->fW);

    Mp6(a1, b1, C, p);
    Mp6(a2, b2, D, p);

    mpc_fp6_add(a1, a1, a2, p);
    mpc_fp6_add(a2, b1, b2, p);
    Mp6(a1, a2, a1, p);
    mpc_fp6_sub(a1, a1, D, p);
    mpc_fp6_sub(a1, a1, C, p);

    KECI_init();
    mpc_fp2_mul(m, D->ft2, KECI, p);
    mpc_fp2_set(D->ft2, D->ft);
    mpc_fp2_set(D->ft, D->f);
    mpc_fp2_set(D->f, m);
    mpc_fp6_add(D, D, C, p);
    mpc_fp6_set(g->f, D);
    mpc_fp6_set(g->fW, a1);
}

static void mpc_fp12_set(mpc12_t dst, const mpc12_t src)
{
    mpc_fp2_set(dst->f->f, src->f->f);
    mpc_fp2_set(dst->f->ft, src->f->ft);
    mpc_fp2_set(dst->f->ft2, src->f->ft2);
    mpc_fp2_set(dst->fW->f, src->fW->f);
    mpc_fp2_set(dst->fW->ft, src->fW->ft);
    mpc_fp2_set(dst->fW->ft2, src->fW->ft2);
}

/*please make sure a != c*/
void fp12_modexp(mpc12_t a, uint32_t b[], uint32_t bWordLen, mpc12_t c)
{
    uint32_t i;

    mpc_fp12_set(c, a);
    i = get_valid_bits(b, bWordLen);

    while (0u != (--i)) {
        /*squaring*/
        Mp12(c, c, c, sm9p256v1_p);

        if (1u == Get_BitValue(b, i)) {
            Mp12(c, a, c, sm9p256v1_p);
        }
    }
}

static void line1(const mpc_t Tx, const mpc_t Ty, const mpc_t Tz, const mpc_t P1x, const mpc_t P1y, const mpc_t Gz, mpc12_t ret, const uint32_t p[8])
{
    mpc_t A, B, D, E, tmp;

    Mp2(Tz, Tz, A, p);
    Mp2(Gz, A, B, p);

    mpc_fp2_mul(A, A, P1x, p);
    mpc_fp2_mul(B, B, P1y, p);
    mpc_fp2_mul(D, Ty, Ty, p);
    mpc_fp2_add(D, D, D, p);
    mpc_fp2_mul(E, Tx, Tx, p);
    mpc_fp2_add(E, E, E, p);
    mpc_fp2_mul(tmp, Tx, Tx, p);
    mpc_fp2_add(E, E, tmp, p);

    Mp2(A, E, A, p);
    Mp2(E, Tx, E, p);

    mpc_fp2_sub(D, D, E, p);

    pke_sub(p, D->im, D->im, 8);
    pke_sub(p, D->re, D->re, 8);
    pke_sub(p, A->im, A->im, 8);
    pke_sub(p, A->re, A->re, 8);

    mpc_fp2_set(ret->f->ft2, B);
    mpc_fp2_set(ret->fW->f, D);
    mpc_fp2_set(ret->fW->ft, A);
}

static void line2(const mpc_t Tx, const mpc_t Ty, const mpc_t Tz, const mpc_t Q1x, const mpc_t Q1y, const mpc_t P1x, const mpc_t P1y, const mpc_t Gz, mpc12_t ret, const uint32_t p[8])
{
    mpc_t A, B, C, G;

    mpc_fp2_mul(A, Gz, P1y, p);
    mpc_fp2_mul(C, Tz, Tz, p);
    mpc_fp2_mul(C, C, Tz, p);
    mpc_fp2_mul(C, C, Q1y, p);
    mpc_fp2_sub(C, C, Ty, p);
    Mp2(C, Q1x, G, p);
    mpc_fp2_mul(C, C, P1x, p);
    Mp2(Q1y, Gz, B, p);
    mpc_fp2_sub(B, B, G, p);

    pke_sub(p, B->im, B->im, 8);
    pke_sub(p, B->re, B->re, 8);
    pke_sub(p, C->im, C->im, 8);
    pke_sub(p, C->re, C->re, 8);
    /*new combine 2015/11/6,fit M type*/
    mpc_fp2_set(ret->f->ft2, A);
    mpc_fp2_set(ret->fW->f, B);
    mpc_fp2_set(ret->fW->ft, C);
}

static void to_p(mpc12_t f, mpc12_t g, const uint32_t p[8])
{
    mpc_t a1, a2, a3, a4, a5, a6;

    mpc_fp2_set(a1, f->f->f);
    mpc_fp2_set(a2, f->f->ft);
    mpc_fp2_set(a3, f->f->ft2);
    mpc_fp2_set(a4, f->fW->f);
    mpc_fp2_set(a5, f->fW->ft);
    mpc_fp2_set(a6, f->fW->ft2);

    pke_sub(p, a1->im, a1->im, 8);
    pke_sub(p, a2->im, a2->im, 8);
    pke_sub(p, a3->im, a3->im, 8);
    pke_sub(p, a4->im, a4->im, 8);
    pke_sub(p, a5->im, a5->im, 8);
    pke_sub(p, a6->im, a6->im, 8);

    kn_init();
    mpc_fp2_mod(a1, a1, p);
    mpc_fp2_mul(a2, a2, k2, p);
    mpc_fp2_mul(a3, a3, k4, p);
    mpc_fp2_mul(a4, a4, k1, p);
    mpc_fp2_mul(a5, a5, k3, p);
    mpc_fp2_mul(a6, a6, k5, p);

    mpc_fp2_set(g->f->f, a1);
    mpc_fp2_set(g->f->ft, a2);
    mpc_fp2_set(g->f->ft2, a3);
    mpc_fp2_set(g->fW->f, a4);
    mpc_fp2_set(g->fW->ft, a5);
    mpc_fp2_set(g->fW->ft2, a6);

    kn_clear();
}

static void to_p2(mpc12_t f, mpc12_t g, const uint32_t p[8])
{
    mpc_t a1, a2, a3, a4, a5, a6;

    mpc_fp2_set(a1, f->f->f);
    mpc_fp2_set(a2, f->f->ft);
    mpc_fp2_set(a3, f->f->ft2);
    mpc_fp2_set(a4, f->fW->f);
    mpc_fp2_set(a5, f->fW->ft);
    mpc_fp2_set(a6, f->fW->ft2);

    gn_init();
    mpc_fp2_mod(a1, a1, p);
    mpc_fp2_mul(a2, a2, g2, p);
    mpc_fp2_mul(a3, a3, g4, p);
    mpc_fp2_mul(a4, a4, g1, p);
    mpc_fp2_mul(a5, a5, g3, p);
    mpc_fp2_mul(a6, a6, g5, p);

    mpc_fp2_set(g->f->f, a1);
    mpc_fp2_set(g->f->ft, a2);
    mpc_fp2_set(g->f->ft2, a3);
    mpc_fp2_set(g->fW->f, a4);
    mpc_fp2_set(g->fW->ft, a5);
    mpc_fp2_set(g->fW->ft2, a6);

    gn_clear();
}

static void to_p3(mpc12_t f, mpc12_t g, const uint32_t p[8])
{
    mpc_t a1, a2, a3, a4, a5, a6;

    mpc_fp2_set(a1, f->f->f);
    mpc_fp2_set(a2, f->f->ft);
    mpc_fp2_set(a3, f->f->ft2);
    mpc_fp2_set(a4, f->fW->f);
    mpc_fp2_set(a5, f->fW->ft);
    mpc_fp2_set(a6, f->fW->ft2);

    pke_sub(p, a1->im, a1->im, 8);
    pke_sub(p, a2->im, a2->im, 8);
    pke_sub(p, a3->im, a3->im, 8);
    pke_sub(p, a4->im, a4->im, 8);
    pke_sub(p, a5->im, a5->im, 8);
    pke_sub(p, a6->im, a6->im, 8);

    en_init();
    mpc_fp2_mod(a1, a1, p);
    mpc_fp2_mul(a2, a2, e2, p);
    mpc_fp2_mul(a3, a3, e4, p);
    mpc_fp2_mul(a4, a4, e1, p);
    mpc_fp2_mul(a5, a5, e3, p);
    mpc_fp2_mul(a6, a6, e5, p);

    mpc_fp2_set(g->f->f, a1);
    mpc_fp2_set(g->f->ft, a2);
    mpc_fp2_set(g->f->ft2, a3);
    mpc_fp2_set(g->fW->f, a4);
    mpc_fp2_set(g->fW->ft, a5);
    mpc_fp2_set(g->fW->ft2, a6);

    en_clear();
}

/*Invert the elements in Fp^2, g = f^(-1) mod p*/
static void inv_p2(mpc_t f, mpc_t g, const uint32_t p[8])
{
    uint32_t c1[8];
    uint32_t c2[8];

    pke_sub(p, f->im, g->im, 8);

    (void)pke_modmul(p, f->re, f->re, c1, 8);
    (void)pke_modmul(p, f->im, f->im, c2, 8);
    (void)pke_modadd(p, c2, c2, c2, 8);
    (void)pke_modadd(p, c1, c2, c1, 8);

    (void)pke_modinv(p, c1, c1, 8, 8);
    (void)pke_modmul(p, f->re, c1, g->re, 8);
    (void)pke_modmul(p, g->im, c1, g->im, 8);
}


/*Invert the elements in Fp^6, g = f^(-1) mod p*/
static void inv_p6(mpc6_t f, mpc6_t g, const uint32_t p[8])
{
    mpc_t a, b, c, A, B, C, D, E, F;

    mpc_fp2_set(a, f->f);
    mpc_fp2_set(b, f->ft);
    mpc_fp2_set(c, f->ft2);

    mpc_fp2_mul(A, a, a, p);
    mpc_fp2_mul(B, b, c, p);
    KECI_init();
    mpc_fp2_mul(B, KECI, B, p);
    mpc_fp2_sub(A, A, B, p);
    mpc_fp2_mul(B, c, c, p);
    mpc_fp2_mul(B, KECI, B, p);
    mpc_fp2_mul(C, a, b, p);
    mpc_fp2_sub(B, B, C, p);
    mpc_fp2_mul(C, b, b, p);
    mpc_fp2_mul(D, a, c, p);
    mpc_fp2_sub(C, C, D, p);
    mpc_fp2_mul(F, KECI, b, p);
    mpc_fp2_mul(F, F, C, p);
    mpc_fp2_mul(E, a, A, p);
    mpc_fp2_add(F, F, E, p);
    mpc_fp2_mul(E, KECI, c, p);
    mpc_fp2_mul(E, E, B, p);
    mpc_fp2_add(F, F, E, p);

    inv_p2(F, F, p);
    mpc_fp2_mul(A, A, F, p);
    mpc_fp2_mul(B, B, F, p);
    mpc_fp2_mul(C, C, F, p);
    mpc_fp2_set(g->f, A);
    mpc_fp2_set(g->ft, B);
    mpc_fp2_set(g->ft2, C);
}

static void finalexp(mpc12_t f, const uint32_t p[8])
{
    uint32_t i;
    const uint32_t t_6_5[3] = {0x0215D941, 0x40000000, 0x00000002};
    const uint32_t t2_6_1[4] = {0x0CB27659, 0x0000B98B, 0x019062ED, 0xD8000000};

    mpc12_t A, S, D, E, G, a, b, f1, f2, f3, f4, ff1, bf, ab, tmp12;
    mpc6_t c6, zero6, tmp1, tmp2, invc;
    mpc_t m;

    mpc_fp6_clears((mpc6_ptr)&zero6);
    mpc_fp12_clears((mpc12_ptr)&D);
    mpc_fp12_clears((mpc12_ptr)&E);

    to_p2(f, A, p);
    Mp12(A, f, A, p);

    mpc_fp6_set(S->f, A->f);
    /*S:=B[1] - B[2]*W;*/
    mpc_fp6_sub(S->fW, zero6, A->fW, p);

    Mp6(A->f, A->f, tmp1, p);

    Mp6(A->fW, A->fW, tmp2, p);
    /*-B[2]^2*/
    mpc_fp6_sub(tmp2, zero6, tmp2, p);

    KECI_init();
    mpc_fp2_mul(m, tmp2->ft2, KECI, p);
    mpc_fp2_set(tmp2->ft2, tmp2->ft);
    mpc_fp2_set(tmp2->ft, tmp2->f);
    /*-B[2]^2*t*/
    mpc_fp2_set(tmp2->f, m);
    /*finish C:=B[1]^2 - t*B[2]^2;*/
    mpc_fp6_add(c6, tmp1, tmp2, p);
    inv_p6(c6, invc, p);
    mpc_fp6_set(D->f, invc);

    Mp12(D, S, D, p);

    mpc_fp6_set(E->f, invc);
    Mp12(E, A, E, p);
    Mp12(A, E, G, p);
    Mp12(S, D, f, p);

    fp12_modexp(G, (uint32_t *)t_6_5, 3, a);

    to_p(a, b, p);
    Mp12(a, b, b, p);

    to_p(f, f1, p);
    to_p2(f, f2, p);
    to_p3(f, f3, p);
    Mp12(f, f, f4, p);
    Mp12(f, f4, f4, p);
    /*make f^4 = f4*/
    Mp12(f, f4, f4, p);

    Mp12(f4, f3, f4, p);
    Mp12(f1, f, ff1, p);
    /*f1=f1^2*/
    Mp12(f1, f1, f1, p);
    Mp12(b, f1, bf, p);
    Mp12(bf, f2, bf, p);
    Mp12(a, b, ab, p);
    /*ff1=ff1^9, tmp12=ff1*/
    mpc_fp6_set(tmp12->f, ff1->f);
    mpc_fp6_set(tmp12->fW, ff1->fW);

    for (i = 0; i < 3; i++) {
        /*doubling ff1*/
        Mp12(ff1, ff1, ff1, p);
    }

    Mp12(ff1, tmp12, ff1, p);
    Mp12(ff1, ab, ff1, p);
    Mp12(f4, ff1, f4, p);

    fp12_modexp(bf, (uint32_t *)t2_6_1, 4, tmp12);

    Mp12(f4, tmp12, f, p);
}

static void FrobeniusTwist(const mpc_t Qx, const mpc_t Qy, mpc_t Fx, mpc_t Fy)
{
    pke_sub(sm9p256v1_p, Qx->im, Fx->im, 8);
    pke_sub(sm9p256v1_p, Qy->im, Fy->im, 8);
    uint32_copy((uint32_t *)Fx->re, (uint32_t *)Qx->re, 8);
    uint32_copy((uint32_t *)Fy->re, (uint32_t *)Qy->re, 8);

    scal_init();
    mpc_fp2_mul(Fx, Fx, scal1, sm9p256v1_p);
    mpc_fp2_mul(Fy, Fy, scal2, sm9p256v1_p);
}

static void f_gen(mpc12_t f, const mpc_t Q1x, const mpc_t Q1y, const mpc_t Q1z, const mpc_t P1x, const mpc_t P1y)
{
    uint32_t i;
    const uint32_t t_6_2[3] = {0x0215D93E, 0x40000000, 0x00000002};

    mpc_t Tx, Ty, Tz, Gx, Gy, Gz, QFrobx, QFroby, QFrobmx, QFrobmy;
    mpc12_t L1, tmp12;

    mpc_fp2_set(Tx, Q1x);
    mpc_fp2_set(Ty, Q1y);
    mpc_fp2_set(Tz, Q1z);

    mpc_fp12_clears(f);
    f->f->f->re[0] = 0x00000001;

    mpc_fp12_clears(L1);
    mpc_fp12_clears(tmp12);

    i = 66;

    while (0u != (--i)) {
        PD_fp2(Tx, Ty, Tz, Gx, Gy, Gz, sm9p256v1_p);
        sp12(f, sm9p256v1_p);

        line1(Tx, Ty, Tz, P1x, P1y, Gz, L1, sm9p256v1_p);

        fl(f, L1, sm9p256v1_p);

        mpc_fp2_set(Tx, Gx);
        mpc_fp2_set(Ty, Gy);
        mpc_fp2_set(Tz, Gz);

        if (1u == Get_BitValue(t_6_2, i)) {
            PA_fp2(Tx, Ty, Tz, Q1x, Q1y, sm9p256v1_p);
            line2(Gx, Gy, Gz, Q1x, Q1y, P1x, P1y, Tz, tmp12, sm9p256v1_p);
            Mp12(f, tmp12, f, sm9p256v1_p);
        }
    }

    mpc_fp2_set(Tx, Gx);
    mpc_fp2_set(Ty, Gy);
    mpc_fp2_set(Tz, Gz);
    FrobeniusTwist(Q1x, Q1y, QFrobx, QFroby);
    PA_fp2(Tx, Ty, Tz, QFrobx, QFroby, sm9p256v1_p);
    line2(Gx, Gy, Gz, QFrobx, QFroby, P1x, P1y, Tz, tmp12, sm9p256v1_p);
    Mp12(f, tmp12, f, sm9p256v1_p);

    mpc_fp2_set(Gx, Tx);
    mpc_fp2_set(Gy, Ty);
    mpc_fp2_set(Gz, Tz);
    FrobeniusTwist(QFrobx, QFroby, QFrobmx, QFrobmy);

    pke_sub(sm9p256v1_p, QFrobmy->re, QFrobmy->re, 8);
    pke_sub(sm9p256v1_p, QFrobmy->im, QFrobmy->im, 8);

    PA_fp2(Tx, Ty, Tz, QFrobmx, QFrobmy, sm9p256v1_p);
    line2(Gx, Gy, Gz, QFrobmx, QFrobmy, P1x, P1y, Tz, tmp12, sm9p256v1_p);
    Mp12(f, tmp12, f, sm9p256v1_p);
}

/*Calculation of bilinear pair transformation e*/
static void pairing_raw(mpc12_t f, const mpc_t Q1x, const mpc_t Q1y, const mpc_t Q1z, const mpc_t P1x, const mpc_t P1y)
{
    f_gen(f, Q1x, Q1y, Q1z, P1x, P1y);
    finalexp(f, sm9p256v1_p);
}

void G2_pointMul_P2(uint32_t *k, uint32_t kWordLen, fp2_pt_t Q)
{
    uint32_t i;

    i = get_valid_bits(k, 8);

    mpc_fp2_set(Q->x, fp2ptP2->x);
    mpc_fp2_set(Q->y, fp2ptP2->y);
    mpc_fp2_set(Q->z, fp2ptP2->z);

    while (0u != (--i)) {
        PD_fp2(Q->x, Q->y, Q->z, Q->x, Q->y, Q->z, sm9p256v1_p);

        if (1u == Get_BitValue(k, i)) {
            PA_fp2(Q->x, Q->y, Q->z, fp2ptP2->x, fp2ptP2->y, sm9p256v1_p);
        }
    }
}

void set_G1_point_buffer_2_G2_point(uint8_t g1_point_buffer[64], fp2_pt_t g2_point)
{
    U8Big_to_U32Small_8(g2_point->x->re, (const uint8_t *)(g1_point_buffer));
    U8Big_to_U32Small_8(g2_point->y->re, (const uint8_t *)(g1_point_buffer + 32));
    uint32_clear(g2_point->x->im, 8);
    uint32_clear(g2_point->y->im, 8);
    mpc_fp2_clears(g2_point->z);
    g2_point->z->re[0] = 0x00000001;
}

void set_G2_point_buffer_2_G2_point(uint8_t g1_point_buffer[128], fp2_pt_t g2_point)
{
    U8Big_to_U32Small_8(g2_point->x->im, (const uint8_t *)g1_point_buffer);
    U8Big_to_U32Small_8(g2_point->x->re, (const uint8_t *)(g1_point_buffer + 32));
    U8Big_to_U32Small_8(g2_point->y->im, (const uint8_t *)(g1_point_buffer + 64));
    U8Big_to_U32Small_8(g2_point->y->re, (const uint8_t *)(g1_point_buffer + 96));
    mpc_fp2_clears(g2_point->z);
    g2_point->z->re[0] = 0x00000001;
}


uint32_t sm9_sign_gen_mastPubKey_from_mastPriKey(uint8_t ks[32], uint8_t Ppub_s[128])
{
    uint32_t tmp_ks[8];

    fp2_pt_t MastPubKey;

    if (NULL == ks || NULL == Ppub_s) {
        return SM9_BUFFER_NULL;
    }
    else
    {;}

    U8Big_to_U32Small_8(tmp_ks, ks);

    /*make sure sysPriKey in [1, n-1]*/
    if (uint32_bignum_check_zero(tmp_ks, 8)) {
        return SM9_ZERO_ALL;
    }
    else if (uint32_bignumcmp(tmp_ks, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }
    else
    {;}

    G2_pointMul_P2(tmp_ks, 8, MastPubKey);

    coordinate_convert(MastPubKey->x, MastPubKey->y, MastPubKey->z, MastPubKey->x, MastPubKey->y);

    U32Small_to_U8Big(Ppub_s, MastPubKey->x->im);
    U32Small_to_U8Big((uint8_t *)(Ppub_s + 32), MastPubKey->x->re);
    U32Small_to_U8Big((uint8_t *)(Ppub_s + 64), MastPubKey->y->im);
    U32Small_to_U8Big((uint8_t *)(Ppub_s + 96), MastPubKey->y->re);

    return SM9_SUCCESS;
}

uint32_t sm9_sign_gen_mastKeyPair(uint8_t ks[32], uint8_t Ppub_s[128])
{
    uint32_t ret;

    if (NULL == ks || NULL == Ppub_s) {
        return SM9_BUFFER_NULL;
    }
    else
    {;}

SM9_GETKEY_LOOP:

    get_rand(ks, 32);

    ret = sm9_sign_gen_mastPubKey_from_mastPriKey(ks, Ppub_s);

    /*make sure priKey in [1, n-1]*/
    if ((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret)) {
        goto SM9_GETKEY_LOOP;
    }
    else
    {;}

    return ret;
}

uint32_t sm9_sign_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid, uint8_t ks[32], uint8_t dsA[64])
{
    uint32_t tmp_ks[8];
    uint32_t tmp[8];

    if (NULL == IDA || NULL == ks || NULL == dsA) {
        return SM9_BUFFER_NULL;
    }
    else if ((0 == IDA_byteLen) || (IDA_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }
    else
    {;}

    U8Big_to_U32Small_8(tmp_ks, ks);

    /*make sure sysPriKey in [1, n-1]*/
    if (uint32_bignum_check_zero(tmp_ks, 8)) {
        return SM9_ZERO_ALL;
    }
    else if (uint32_bignumcmp(tmp_ks, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }
    else
    {;}

    ibe_H1_H2(1, IDA, IDA_byteLen, (uint8_t *)(&hid), 1, tmp);

    pke_modadd(sm9p256v1_n, tmp, tmp_ks, tmp, 8);

    if (uint32_bignum_check_zero(tmp, 8)) {
        return SM9_ZERO_ALL;
    }
    else
    {;}

    pke_modinv(sm9p256v1_n, tmp, tmp, 8, 8);
    pke_modmul(sm9p256v1_n, tmp, tmp_ks, tmp, 8);
    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp, (uint32_t *)fp2ptP1->x->re, (uint32_t *)fp2ptP1->y->re, tmp_ks, tmp);

    U32Small_to_U8Big(dsA, tmp_ks);
    U32Small_to_U8Big((uint8_t *)(dsA + 32), tmp);

    return SM9_SUCCESS;
}

/*random r*/
static uint8_t sm9_sign_with_r(mpc12_t fg, uint32_t *r, const uint8_t *M, uint32_t MByteLen, uint32_t tmp[8], uint8_t h[32])
{
    uint8_t h2rf_para[32 * 12];
    mpc12_t omega;

    fp12_modexp(fg, r, 8, omega);

    FE2OSP(omega, h2rf_para);

    ibe_H1_H2(2, M, MByteLen, h2rf_para, 32 * 12, tmp);
    U32Small_to_U8Big(h, tmp);

    pke_modsub(sm9p256v1_n, r, tmp, tmp, 8);

    if (uint32_bignum_check_zero(tmp, 8)) {
        return SM9_ZERO_ALL;
    }

    return 0;
}

uint32_t sm9_sign(const uint8_t *M, uint32_t MByteLen, const uint8_t Ppub_s[128], const uint8_t dsA[64], uint8_t r[32],
                  uint8_t h[32], uint8_t S[65])
{
    uint32_t tmp_r[8];
    uint32_t tmp[8];
    mpc12_t fp12g;
    fp2_pt_t MastPubKey, SigPriKey;

    set_G2_point_buffer_2_G2_point((uint8_t *)Ppub_s, MastPubKey);
    set_G1_point_buffer_2_G2_point((uint8_t *)dsA, SigPriKey);

    pairing_raw(fp12g, MastPubKey->x, MastPubKey->y, MastPubKey->z, fp2ptP1->x, fp2ptP1->y);

    if (NULL == r) {
SM9_SIGN_LOOP:
        get_rand((uint8_t *)tmp_r, 32);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            goto SM9_SIGN_LOOP;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            goto SM9_SIGN_LOOP;
        }
        else
        {;}

        if (sm9_sign_with_r(fp12g, tmp_r, M, MByteLen, tmp, h)) {
            goto SM9_SIGN_LOOP;
        }
        else
        {;}
    }
    else {
        U8Big_to_U32Small_8(tmp_r, r);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            return SM9_ZERO_ALL;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            return SM9_INTEGER_TOO_BIG;
        }
        else
        {;}

        if (sm9_sign_with_r(fp12g, tmp_r, M, MByteLen, tmp, h)) {
            return SM9_ZERO_ALL;
        }
        else
        {;}
    }

    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp, SigPriKey->x->re, SigPriKey->y->re, tmp, tmp_r);

    S[0] = 0x04;
    U32Small_to_U8Big((uint8_t *)(S + 1), tmp);
    U32Small_to_U8Big((uint8_t *)(S + 33), tmp_r);

    return SM9_SUCCESS;
}

static uint8_t sm9_pointVerify(fp2_pt_t S)
{
    uint32_t A[8], B[8], C[8];

    uint32_clear(C, 8);
    C[0] = 0x00000005;

    pke_modmul(sm9p256v1_p, S->x->re, S->x->re, A, 8);
    pke_modmul(sm9p256v1_p, A, S->x->re, A, 8);
    pke_modmul(sm9p256v1_p, S->y->re, S->y->re, B, 8);
    pke_modadd(sm9p256v1_p, A, C, A, 8);

    return uint32_cmp(A, B);
}

uint32_t sm9_verify(const uint8_t *M, uint32_t MByteLen, const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid,
                    const uint8_t Ppub_s[128], const uint8_t h[32], const uint8_t S[65])
{
    uint8_t h2rf_para[32 * 12];
    uint32_t SigH[8];
    uint32_t tmp[8];
    fp2_pt_t MastPubKey, SigS, TP2;
    mpc12_t fp12g, omega;

    if (NULL == M || NULL == IDA || NULL == Ppub_s || NULL == h || NULL == S) {
        return SM9_BUFFER_NULL;
    }

    if ((0 == IDA_byteLen) || (IDA_byteLen >= SM9_MAX_ID_BYTE_LEN) || (SM9_POINT_NOT_COMPRESSED != S[0])) {
        return SM9_INPUT_INVALID;
    }

    U8Big_to_U32Small_8(SigH, h);

    /*check h in [1, n-1]*/
    if (uint32_bignum_check_zero(SigH, 8)) {
        return SM9_ZERO_ALL;
    }

    if (uint32_bignumcmp(SigH, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)(S + 1), SigS);

    if (sm9_pointVerify(SigS)) {
        return SM9_NOT_ON_CURVE;
    }

    set_G2_point_buffer_2_G2_point((uint8_t *)Ppub_s, MastPubKey);

    pairing_raw(fp12g, MastPubKey->x, MastPubKey->y, MastPubKey->z, fp2ptP1->x, fp2ptP1->y);

    fp12_modexp(fp12g, SigH, 8, omega);

    ibe_H1_H2(1, IDA, IDA_byteLen, (uint8_t *)(&hid), 1, tmp);

    G2_pointMul_P2(tmp, 8, TP2);

    PA_fp2(TP2->x, TP2->y, TP2->z, MastPubKey->x, MastPubKey->y, sm9p256v1_p);

    coordinate_convert(TP2->x, TP2->y, TP2->z, TP2->x, TP2->y);
    pairing_raw(fp12g, TP2->x, TP2->y, TP2->z, SigS->x, SigS->y);

    Mp12(omega, fp12g, omega, sm9p256v1_p);

    FE2OSP(omega, h2rf_para);

    ibe_H1_H2(2, M, MByteLen, h2rf_para, 32 * 12, tmp);

    if (uint32_cmp(tmp, SigH)) {
        return SM9_VERIFY_FAILED;
    }

    return SM9_SUCCESS;
}


uint32_t sm9_enc_gen_mastPubKey_from_mastPriKey(uint8_t ke[32], uint8_t Ppub_e[64])
{
    uint32_t tmp_ke[8];
    uint32_t tmp1[8];
    uint32_t tmp2[8];

    if (NULL == ke || NULL == Ppub_e) {
        return SM9_BUFFER_NULL;
    }
    else
    {;}

    U8Big_to_U32Small_8(tmp_ke, ke);

    /*make sure priKey in [1, n-1]*/
    if (uint32_bignum_check_zero(tmp_ke, 8)) {
        return SM9_ZERO_ALL;
    }

    if (uint32_bignumcmp(tmp_ke, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }

    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp_ke, (uint32_t *)fp2ptP1->x->re, (uint32_t *)fp2ptP1->y->re, tmp1, tmp2);
    U32Small_to_U8Big(Ppub_e, tmp1);
    U32Small_to_U8Big((uint8_t *)(Ppub_e + 32), tmp2);

    return SM9_SUCCESS;
}


uint32_t sm9_enc_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64])
{
    uint32_t ret;

    if (NULL == ke || NULL == Ppub_e) {
        return SM9_BUFFER_NULL;
    }
    else
    {;}

SM9_GETKEY_LOOP:

    get_rand(ke, 32);

    ret = sm9_enc_gen_mastPubKey_from_mastPriKey(ke, Ppub_e);

    /*make sure priKey in [1, n-1]*/
    if ((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret)) {
        goto SM9_GETKEY_LOOP;
    }
    else
    {;}

    return ret;
}


uint32_t sm9_enc_gen_userPriKey(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, uint8_t ke[32], uint8_t deB[128])
{
    uint32_t tmp_ke[8];
    uint32_t tmp1[8];
    uint32_t tmp2[8];

    fp2_pt_t UserPrivKey;

    if (NULL == IDB || NULL == ke || NULL == deB) {
        return SM9_BUFFER_NULL;
    }
    else if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }
    else
    {;}

    U8Big_to_U32Small_8(tmp_ke, ke);

    /*make sure sysPriKey in [1, n-1]*/
    if (uint32_bignum_check_zero(tmp_ke, 8)) {
        return SM9_ZERO_ALL;
    }
    else if (uint32_bignumcmp(tmp_ke, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }
    else
    {;}

    ibe_H1_H2(1, IDB, IDB_byteLen, (uint8_t *)(&hid), 1, tmp1);

    pke_modadd(sm9p256v1_n, tmp1, tmp_ke, tmp2, 8);

    if (uint32_bignum_check_zero(tmp2, 8)) {
        return SM9_ZERO_ALL;
    }
    else
    {;}

    pke_modinv(sm9p256v1_n, tmp2, tmp1, 8, 8);
    pke_modmul(sm9p256v1_n, tmp1, tmp_ke, tmp2, 8);

    G2_pointMul_P2(tmp2, 8, UserPrivKey);

    coordinate_convert(UserPrivKey->x, UserPrivKey->y, UserPrivKey->z, UserPrivKey->x, UserPrivKey->y);

    U32Small_to_U8Big(deB, UserPrivKey->x->im);
    U32Small_to_U8Big((uint8_t *)(deB + 32), UserPrivKey->x->re);
    U32Small_to_U8Big((uint8_t *)(deB + 64), UserPrivKey->y->im);
    U32Small_to_U8Big((uint8_t *)(deB + 96), UserPrivKey->y->re);

    return SM9_SUCCESS;
}

uint32_t sm9_wrap_key(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64], uint8_t r[32],
                      uint8_t C[64], uint32_t KByteLen, uint8_t *K)
{
    uint32_t tmp_r[8];
    uint32_t tmp[8], Ax[8], Ay[8];
    uint8_t h2rf_para[32 * 12];
    mpc12_t fp12g, omega;
    fp2_pt_t MastPubKey;

    if (NULL == IDB || NULL == Ppub_e || NULL == K || NULL == C) {
        return SM9_BUFFER_NULL;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)Ppub_e, MastPubKey);

    ibe_H1_H2(1, IDB, IDB_byteLen, (uint8_t *)(&hid), 1, tmp);
    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp, (uint32_t *)fp2ptP1->x->re, (uint32_t *)fp2ptP1->y->re, Ax, Ay);
    eccp_pointAdd((eccp_curve_t *)sm9_curve, Ax, Ay, MastPubKey->x->re, MastPubKey->y->re, Ax, Ay);

    if (NULL == r) {
SM9_WRAP_LOOP:
        get_rand((uint8_t *)tmp_r, 32);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            goto SM9_WRAP_LOOP;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            goto SM9_WRAP_LOOP;
        }
        else
        {;}
    }
    else {
        U8Big_to_U32Small_8(tmp_r, r);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            return SM9_ZERO_ALL;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            return SM9_INTEGER_TOO_BIG;
        }
        else
        {;}
    }

    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp_r, Ax, Ay, Ax, Ay);

    pairing_raw(fp12g, fp2ptP2->x, fp2ptP2->y, fp2ptP2->z, MastPubKey->x, MastPubKey->y);

    fp12_modexp(fp12g, tmp_r, 8, omega);

    FE2OSP(omega, h2rf_para);

    U32Small_to_U8Big(C, Ax);
    U32Small_to_U8Big(C + 32, Ay);

    ibe_kdf_wrap_enc(C, h2rf_para, IDB, IDB_byteLen, K,  KByteLen, NULL, 0);

    if (uint8_bignum_check_zero(K,  KByteLen)) {
        if (NULL == r) {
            goto SM9_WRAP_LOOP;
        }
        else {
            return SM9_ZERO_ALL;
        }
    }

    return SM9_SUCCESS;
}

uint32_t sm9_unwrap_key(const uint8_t *IDB, uint32_t IDB_byteLen, const uint8_t deB[128], const uint8_t C[64],
                        uint32_t KByteLen, uint8_t *K)
{
    uint8_t h2rf_para[32 * 12];
    mpc12_t fp12g;
    fp2_pt_t DecPriKey, C1;

    if (NULL == IDB || NULL == deB || NULL == C || NULL == K) {
        return SM9_BUFFER_NULL;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)C, C1);

    if (sm9_pointVerify(C1)) {
        return SM9_NOT_ON_CURVE;
    }

    set_G2_point_buffer_2_G2_point((uint8_t *)deB, DecPriKey);

    pairing_raw(fp12g, DecPriKey->x, DecPriKey->y, DecPriKey->z, C1->x, C1->y);
    FE2OSP(fp12g, h2rf_para);

    ibe_kdf_wrap_enc(C, h2rf_para, IDB, IDB_byteLen, K,  KByteLen, NULL, 0);

    if (uint8_bignum_check_zero(K,  KByteLen)) {
        return SM9_VERIFY_FAILED;
    }

    return SM9_SUCCESS;
}

uint32_t sm9_enc(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t *M, uint32_t MByteLen,
                 const uint8_t Ppub_e[64], uint8_t r[32], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type,
                 uint32_t K2ByteLen, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t tmp_r[8];
    uint32_t tmp[8], Ax[8], Ay[8];
    uint8_t h2rf_para[32 * 12];
    uint8_t tmpc1[64];
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN];
    hash_ctx_t ctx[1];
    mpc12_t fp12g, omega;
    fp2_pt_t MastPubKey;
    uint32_t i, j, k, mlen;
    uint8_t *pC;

    if (enc_type > SM9_ENC_KDF_BLOCK_CIPHER) {
        return SM9_INPUT_INVALID;
    }

    if (SM9_ENC_KDF_BLOCK_CIPHER == enc_type) {
        if (padding_type > SM9_ENC_PKCS7_PADDING) {
            return SM9_INPUT_INVALID;
        }
        else if ((MByteLen & 0x0F) && (SM9_ENC_NO_PADDING == padding_type)) {
            return SM9_INPUT_INVALID;
        }
        else
        {;}
    }

    if (NULL == IDB || NULL == Ppub_e || NULL == M || NULL == C) {
        return SM9_BUFFER_NULL;
    }

    if (M == C) {
        return SM9_IN_OUT_SAME_BUFFER;
    }

    if ((0 == MByteLen) || (MByteLen >= SM9_MAX_MSG_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    if (K2ByteLen > SM9_MAX_ENC_K2_BYTE_LEN) {
        return SM9_INPUT_INVALID;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)Ppub_e, MastPubKey);

    ibe_H1_H2(1, IDB, IDB_byteLen, (uint8_t *)(&hid), 1, tmp);
    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp, (uint32_t *)fp2ptP1->x->re, (uint32_t *)fp2ptP1->y->re, Ax, Ay);
    eccp_pointAdd((eccp_curve_t *)sm9_curve, Ax, Ay, MastPubKey->x->re, MastPubKey->y->re, Ax, Ay);

    if (NULL == r) {
SM9_ENC_LOOP:
        get_rand((uint8_t *)tmp_r, 32);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            goto SM9_ENC_LOOP;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            goto SM9_ENC_LOOP;
        }
        else
        {;}
    }
    else {
        U8Big_to_U32Small_8(tmp_r, r);

        /*make sure r in [1, n-1]*/
        if (uint32_bignum_check_zero(tmp_r, 8)) {
            return SM9_ZERO_ALL;
        }
        else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
            return SM9_INTEGER_TOO_BIG;
        }
        else
        {;}
    }

    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp_r, Ax, Ay, Ax, Ay);

    pairing_raw(fp12g, fp2ptP2->x, fp2ptP2->y, fp2ptP2->z, MastPubKey->x, MastPubKey->y);

    fp12_modexp(fp12g, tmp_r, 8, omega);

    FE2OSP(omega, h2rf_para);

    U32Small_to_U8Big(tmpc1, Ax);
    U32Small_to_U8Big(tmpc1 + 32, Ay);

    pC = C + 96;
    mlen = MByteLen;

    if (SM9_ENC_KDF_STREAM_CIPHER == enc_type) {
        i = ~mlen;

        if (i < K2ByteLen) {
            return SM9_INPUT_INVALID;
        }

        ibe_kdf_wrap_enc(tmpc1, h2rf_para, IDB, IDB_byteLen, pC, mlen, K2, K2ByteLen);

        if (uint8_bignum_check_zero(pC, mlen)) {
            if (NULL == r) {
                goto SM9_ENC_LOOP;
            }
            else {
                return SM9_ZERO_ALL;
            }
        }

        for (i = 0; i < mlen; i++) {
            pC[i] ^= M[i];
        }
    }
    else if (SM9_ENC_KDF_BLOCK_CIPHER == enc_type) {
        ibe_kdf_wrap_enc(tmpc1, h2rf_para, IDB, IDB_byteLen, pC, 16, K2, K2ByteLen);

        if (uint8_bignum_check_zero(pC, 16)) {
            if (NULL == r) {
                goto SM9_ENC_LOOP;
            }
            else {
                return SM9_ZERO_ALL;
            }
        }

        ske_hp_init(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, pC, 0, NULL);

        i = mlen & 0x0F;
        j = mlen - i;
        ske_hp_update_blocks((uint8_t *)M, pC, j);

        if (SM9_ENC_PKCS7_PADDING == padding_type) {
            k = 0x10 - i;
            memset_(h2rf_para, k, 0x10);

            if (i) {
                memcpy_(h2rf_para, (uint8_t *)(M + j), i);
            }

            mlen += k;
            ske_hp_update_blocks(h2rf_para, pC + j, 16);
        }
    }
    else
    {;}

    *CByteLen = 96 + mlen;

    for (i = 0; i < 64; i++) {
        C[i] = tmpc1[i];
    }

    hash_init(ctx, HASH_SM3);

    hash_update(ctx, pC, mlen);
    hash_update(ctx, K2, K2ByteLen);
    hash_final(ctx, C + 64);
    return SM9_SUCCESS;
}

uint32_t sm9_dec(const uint8_t *IDB, uint32_t IDB_byteLen, const uint8_t *C, uint32_t CByteLen, const uint8_t deB[128],
                 sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, uint32_t K2ByteLen, uint8_t *M, uint32_t *MByteLen)
{
    uint8_t K1[16];
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN];
    uint8_t h2rf_para[32 * 12];
    const uint8_t *pC;
    uint32_t i, j, tmplen;
    hash_ctx_t ctx[1];
    mpc12_t fp12g;
    fp2_pt_t DecPriKey, C1;

    if (enc_type > SM9_ENC_KDF_BLOCK_CIPHER) {
        return SM9_INPUT_INVALID;
    }

    if (SM9_ENC_KDF_BLOCK_CIPHER == enc_type) {
        if (padding_type > SM9_ENC_PKCS7_PADDING) {
            return SM9_INPUT_INVALID;
        }
        else if ((CByteLen - 96) & 0x0F) {
            return SM9_INPUT_INVALID;
        }
    }

    if (NULL == IDB || NULL == deB || NULL == M || NULL == C) {
        return SM9_BUFFER_NULL;
    }

    if (M == C) {
        return SM9_IN_OUT_SAME_BUFFER;
    }

    if (CByteLen < (96 + 1)) {
        return SM9_INPUT_INVALID;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    if (K2ByteLen > SM9_MAX_ENC_K2_BYTE_LEN) {
        return SM9_INPUT_INVALID;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)C, C1);

    if (sm9_pointVerify(C1)) {
        return SM9_NOT_ON_CURVE;
    }

    set_G2_point_buffer_2_G2_point((uint8_t *)deB, DecPriKey);

    pairing_raw(fp12g, DecPriKey->x, DecPriKey->y, DecPriKey->z, C1->x, C1->y);
    FE2OSP(fp12g, h2rf_para);

    pC = C + 96;
    tmplen = CByteLen - 96;
    j = 0;

    if (SM9_ENC_KDF_STREAM_CIPHER == enc_type) {
        ibe_kdf_wrap_enc(C, h2rf_para, IDB, IDB_byteLen, M, tmplen, K2, K2ByteLen);

        if (uint8_bignum_check_zero(M, tmplen)) {
            return  SM9_ZERO_ALL;
        }

        for (i = 0; i < tmplen; i++) {
            M[i] ^= pC[i];
        }
    }
    else if (SM9_ENC_KDF_BLOCK_CIPHER == enc_type) {
        if (CByteLen & 0x0F) {
            return SM9_INPUT_INVALID;
        }

        ibe_kdf_wrap_enc(C, h2rf_para, IDB, IDB_byteLen, K1, 16, K2, K2ByteLen);

        if (uint8_bignum_check_zero(K1, 16)) {
            return  SM9_ZERO_ALL;
        }

        ske_hp_init(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, K1, 0, NULL);

        if (SM9_ENC_NO_PADDING == padding_type) {
            ske_hp_update_blocks((uint8_t *)pC, M, tmplen);
        }
        else {
            ske_hp_update_blocks((uint8_t *)pC, M, tmplen - 16);
            ske_hp_update_blocks((uint8_t *)(pC + tmplen - 16), h2rf_para, 16);

            j = h2rf_para[16 - 1];

            if ((j > 0x10) || (j == 0)) {
                return SM9_DECRY_VERIFY_FAILED;
            }

            for (i = 16 - j; i < 16; i++) {
                if (h2rf_para[i] != j) {
                    return SM9_DECRY_VERIFY_FAILED;
                }
            }

            memcpy_(M + (tmplen - 16), h2rf_para, 16 - j);
        }
    }
    else
    {;}

    hash_init(ctx, HASH_SM3);

    hash_update(ctx, pC, tmplen);
    hash_update(ctx, K2, K2ByteLen);
    hash_final(ctx, h2rf_para);

    pC = C + 64;

    for (i = 0; i < 32; i++) {
        if (pC[i] != h2rf_para[i]) {
            return SM9_DECRY_VERIFY_FAILED;
        }
    }

    if ((SM9_ENC_KDF_BLOCK_CIPHER == enc_type) && (SM9_ENC_PKCS7_PADDING == padding_type)) {
        *MByteLen = tmplen - j;
    }
    else {
        *MByteLen = tmplen;
    }

    return SM9_SUCCESS;
}



uint32_t sm9_exckey_gen_mastPubKey_from_mastPriKey(uint8_t ke[32], uint8_t Ppub_e[64])
{
    return sm9_enc_gen_mastPubKey_from_mastPriKey(ke, Ppub_e);
}

uint32_t sm9_exckey_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64])
{
    return sm9_enc_gen_mastKeyPair(ke, Ppub_e);
}


uint32_t sm9_exckey_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_byteLen, uint8_t hid, uint8_t ke[32], uint8_t deA[128])
{
    return sm9_enc_gen_userPriKey(IDA, IDA_byteLen, hid, ke, deA);
}


uint32_t sm9_exckey_gen_tmpPubKey_from_tmpPriKey(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64],
        uint8_t rA[32], uint8_t RA[64])
{
    uint32_t tmp_r[8];
    uint32_t tmp1[8];
    uint32_t tmp2[8];
    uint32_t tmp3[8];

    if ((NULL == IDB) || (NULL == Ppub_e) || (NULL == rA) || (NULL == RA)) {
        return SM9_BUFFER_NULL;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    ibe_H1_H2(1, IDB, IDB_byteLen, (uint8_t *)(&hid), 1, tmp1);
    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp1, (uint32_t *)fp2ptP1->x->re, (uint32_t *)fp2ptP1->y->re, tmp1, tmp2);

    U8Big_to_U32Small_8(tmp_r, (const uint8_t *)(Ppub_e));
    U8Big_to_U32Small_8(tmp3, (const uint8_t *)(Ppub_e + 32));

    eccp_pointAdd((eccp_curve_t *)sm9_curve, tmp1, tmp2, tmp_r, tmp3, tmp1, tmp2);

    U8Big_to_U32Small_8(tmp_r, rA);

    /*make sure priKey in [1, n-1]*/
    if (uint32_bignum_check_zero(tmp_r, 8)) {
        return SM9_ZERO_ALL;
    }
    else if (uint32_bignumcmp(tmp_r, 8, (uint32_t *)sm9p256v1_n, 8) >= 0) {
        return SM9_INTEGER_TOO_BIG;
    }
    else
    {;}

    eccp_pointMul((eccp_curve_t *)sm9_curve, tmp_r, tmp1, tmp2, tmp1, tmp2);
    U32Small_to_U8Big(RA, tmp1);
    U32Small_to_U8Big((uint8_t *)(RA + 32), tmp2);

    return SM9_SUCCESS;
}

uint32_t sm9_exckey_gen_tmpKeyPair(const uint8_t *IDB, uint32_t IDB_byteLen, uint8_t hid, const uint8_t Ppub_e[64],
                                   uint8_t rA[32], uint8_t RA[64])
{
    uint32_t ret;

SM9_GETKEY_LOOP:
    get_rand(rA, 32);

    ret = sm9_exckey_gen_tmpPubKey_from_tmpPriKey(IDB, IDB_byteLen, hid, Ppub_e, rA, RA);

    if ((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret)) {
        goto SM9_GETKEY_LOOP;
    }
    else
    {;}

    return SM9_SUCCESS;
}


static void sm9_exchkey_hash(sm9_exchange_role_e role,
                             const uint8_t *g23,
                             const uint8_t *IDA, uint32_t IDA_byteLen,
                             const uint8_t *IDB, uint32_t IDB_byteLen,
                             const uint8_t *RA, const uint8_t *RB,
                             uint8_t *out)
{
    hash_ctx_t ctx[1];
    hash_init(ctx, HASH_SM3);
    hash_update(ctx, g23, 32 * 12 * 2);

    if (SM9_ROLE_SPONSOR == role) {
        hash_update(ctx, IDA, IDA_byteLen);
        hash_update(ctx, IDB, IDB_byteLen);
        hash_update(ctx, RA, 64);
        hash_update(ctx, RB, 64);
    }
    else if (SM9_ROLE_RESPONSOR == role) {
        hash_update(ctx, IDB, IDB_byteLen);
        hash_update(ctx, IDA, IDA_byteLen);
        hash_update(ctx, RB, 64);
        hash_update(ctx, RA, 64);
    }

    hash_final(ctx, out);
}

uint32_t sm9_exchangekey(sm9_exchange_role_e role,
                         const uint8_t *IDA, uint32_t IDA_byteLen,
                         const uint8_t *IDB, uint32_t IDB_byteLen,
                         const uint8_t Ppub_e[64],
                         const uint8_t deA[128], const uint8_t rA[32],
                         const uint8_t RA[64], const uint8_t RB[64],
                         uint32_t KByteLen,
                         uint8_t *KA, uint8_t S1[32], uint8_t SA[32])
{
    uint32_t tmp[8];
    uint8_t digest[32];
    hash_ctx_t ctx[1];
    uint8_t tag;

    fp2_pt_t MastPubKey, RRA, RRB;
    mpc12_t fp12g1, fp12g2, fp12g3;
    uint8_t h2rf_para[32 * 12 * 3];

    if ((NULL == IDA) || (NULL == IDB) || (NULL == Ppub_e)) {
        return SM9_BUFFER_NULL;
    }

    if ((NULL == deA) || (NULL == rA) || (NULL == RA) || (NULL == RB)) {
        return SM9_BUFFER_NULL;
    }

    if ((NULL == KA) || (NULL == S1) || (NULL == SA)) {
        return SM9_BUFFER_NULL;
    }

    if (role > SM9_ROLE_RESPONSOR) {
        return SM9_EXCHANGE_ROLE_INVALID;
    }

    if ((0 == IDA_byteLen) || (IDA_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    if ((0 == IDB_byteLen) || (IDB_byteLen >= SM9_MAX_ID_BYTE_LEN)) {
        return SM9_INPUT_INVALID;
    }

    if (0 == KByteLen) {
        return SM9_INPUT_INVALID;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)RA, RRA);
    set_G1_point_buffer_2_G2_point((uint8_t *)RB, RRB);

    if (sm9_pointVerify(RRB)) {
        return SM9_NOT_ON_CURVE;
    }

    set_G1_point_buffer_2_G2_point((uint8_t *)Ppub_e, MastPubKey);
    pairing_raw(fp12g2, fp2ptP2->x, fp2ptP2->y, fp2ptP2->z, MastPubKey->x, MastPubKey->y);
    U8Big_to_U32Small_8(tmp, rA);
    fp12_modexp(fp12g2, tmp, 8, fp12g1);

    set_G2_point_buffer_2_G2_point((uint8_t *)deA, MastPubKey);
    pairing_raw(fp12g2, MastPubKey->x, MastPubKey->y, MastPubKey->z, RRB->x, RRB->y);

    fp12_modexp(fp12g2, tmp, 8, fp12g3);

    if (SM9_ROLE_SPONSOR == role) {
        FE2OSP(fp12g1, h2rf_para);
        FE2OSP(fp12g2, h2rf_para + 32 * 12);
    }
    else if (SM9_ROLE_RESPONSOR == role) {
        FE2OSP(fp12g2, h2rf_para);
        FE2OSP(fp12g1, h2rf_para + 32 * 12);
    }

    FE2OSP(fp12g3, h2rf_para + 32 * 12 * 2);

    if (SM9_ROLE_SPONSOR == role) {
        ibe_kdf_exc_key(IDA, IDA_byteLen, IDB, IDB_byteLen, RA, RB, h2rf_para, KA, KByteLen);
    }
    else if (SM9_ROLE_RESPONSOR == role) {
        ibe_kdf_exc_key(IDB, IDB_byteLen, IDA, IDA_byteLen, RB, RA, h2rf_para, KA, KByteLen);
    }
    else
    {;}

    tag = 0x82;
    sm9_exchkey_hash(role, h2rf_para + 32 * 12, IDA, IDA_byteLen, IDB, IDB_byteLen, RA, RB, digest);
    hash_init(ctx, HASH_SM3);
    hash_update(ctx, &tag, 1);
    hash_update(ctx, h2rf_para, 32 * 12);
    hash_update(ctx, digest, 32);

    if (SM9_ROLE_SPONSOR == role) {
        hash_final(ctx, S1);
    }
    else if (SM9_ROLE_RESPONSOR == role) {
        hash_final(ctx, SA);
    }
    else
    {;}

    tag = 0x83;
    hash_init(ctx, HASH_SM3);
    hash_update(ctx, &tag, 1);
    hash_update(ctx, h2rf_para, 32 * 12);
    hash_update(ctx, digest, 32);

    if (SM9_ROLE_SPONSOR == role) {
        hash_final(ctx, SA);
    }
    else if (SM9_ROLE_RESPONSOR == role) {
        hash_final(ctx, S1);
    }
    else
    {;}

    return SM9_SUCCESS;
}
