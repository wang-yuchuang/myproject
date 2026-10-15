/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef PKE_H
#define PKE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utility.h"
#include "register_base_addr.h"
#include "eccp_curve.h"

#define PKE_SEC

#define SUPPORT_SM2 1
#define SUPPORT_C25519 1

#ifdef SUPPORT_C25519
extern const edward_curve_t ed25519[1];
#endif

/***************** PKE register *******************/
#define PKE_CTRL           (*((volatile uint32_t *)(PKE_BASE_ADDR)))
#define PKE_CFG            (*((volatile uint32_t *)(PKE_BASE_ADDR+0x04)))
#define PKE_MC_PTR         (*((volatile uint32_t *)(PKE_BASE_ADDR+0x08)))
#define PKE_RISR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x0C)))
#define PKE_IMCR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x10)))
#define PKE_MISR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x14)))
#define PKE_RT_CODE        (*((volatile uint32_t *)(PKE_BASE_ADDR+0x24)))
#ifdef PKE_SEC
#define PKE_RAND_SEED      (*((volatile uint32_t *)(PKE_BASE_ADDR+0x40)))
#define PKE_RC_EN          (*((volatile uint32_t *)(PKE_BASE_ADDR+0x60)))
#define PKE_RC_KEY         (*((volatile uint32_t *)(PKE_BASE_ADDR+0x64)))
#define PKE_RC_D_NONCE     (*((volatile uint32_t *)(PKE_BASE_ADDR+0x68)))
#define PKE_RC_A_NONCE     (*((volatile uint32_t *)(PKE_BASE_ADDR+0x6C)))
#endif
#define PKE_VERSION        (*((volatile uint32_t *)(PKE_BASE_ADDR+0xFC)))
#define PKE_A(a, step)     ((volatile uint32_t *)(PKE_BASE_ADDR+0x0400+(a)*(step)))
#define PKE_B(a, step)     ((volatile uint32_t *)(PKE_BASE_ADDR+0x1000+(a)*(step)))

/*********** PKE register action offset ************/
#define PKE_INT_ENABLE_OFFSET                 (8)
#define PKE_START_CALC                        (1)

/***************** PKE microcode ******************/
#define MICROCODE_PDBL                        (0x04)
#define MICROCODE_PADD                        (0x08)
#define MICROCODE_PVER                        (0x0C)
#define MICROCODE_PMUL                        (0x10)
#define MICROCODE_MODEXP                      (0x14)
#define MICROCODE_MODMUL                      (0x18)
#define MICROCODE_MODINV                      (0x1C)
#define MICROCODE_MODADD                      (0x20)
#define MICROCODE_MODSUB                      (0x24)
#define MICROCODE_MGMR_PRE_H                  (0x28)
#define MICROCODE_INTMUL                      (0x2C)
#define MICROCODE_Ed25519_PMUL                (0x30)
#define MICROCODE_Ed25519_PADD                (0x34)
#define MICROCODE_C25519_PMUL                 (0x38)
#define MICROCODE_MODRES                      (0x3C)
#define MICROCODE_INTADD                      (0x40)
#define MICROCODE_INTSUB                      (0x44)
#define MICROCODE_PMULF                       (0x48)
#define MICROCODE_MGMR_PRE_H_MM               (0x4C)
#define MICROCODE_MGMR_PRE_N0                 (0x50)
#ifdef PKE_SEC
#define MICROCODE_PMUL_SEC                    (0x54)
#define MICROCODE_MODEXP_WITH_PUBKEY          (0x58)
#define MICROCODE_MODEXP_MGMR_LADDER          (0x5C)
#endif

/*********** some PKE algorithm operand length ************/
#define OPERAND_MAX_BIT_LEN                   (4096)
#define OPERAND_MAX_WORD_LEN                  (GET_WORD_LEN(OPERAND_MAX_BIT_LEN))

#define ECCP_MAX_BIT_LEN                      (521)
#define ECCP_MAX_BYTE_LEN                     (GET_BYTE_LEN(ECCP_MAX_BIT_LEN))
#define ECCP_MAX_WORD_LEN                     (GET_WORD_LEN(ECCP_MAX_BIT_LEN))

#define C25519_BYTE_LEN                       (256/8)
#define C25519_WORD_LEN                       (256/32)

#define Ed25519_BYTE_LEN                      C25519_BYTE_LEN
#define Ed25519_WORD_LEN                      C25519_WORD_LEN

#define MAX_RSA_WORD_LEN                      OPERAND_MAX_WORD_LEN
#define MAX_RSA_BIT_LEN                       (MAX_RSA_WORD_LEN<<5)
#define MIN_RSA_BIT_LEN                       (512)

/******************* PKE return code ********************/
enum PKE_RET_CODE {
    PKE_SUCCESS = 0,
    PKE_STOP,
    PKE_NO_MODINV,
    PKE_NOT_ON_CURVE,
    PKE_INVALID_MC,
    PKE_ZERO_ALL,
    PKE_INTEGER_TOO_BIG,
    PKE_INVALID_INPUT,
    PKE_ERROR,
};

typedef void *(*KDF_FUNC)(const void *input, uint32_t bytelen, uint8_t *key, uint32_t keybytelen);

uint32_t pke_get_version(void);

uint32_t pke_get_operand_bytes(void);

uint32_t pke_modinv(const uint32_t *modulus, const uint32_t *a, uint32_t *ainv, uint32_t modwordlen,
                    uint32_t awordlen);

uint32_t pke_modadd(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
                    uint32_t *out, uint32_t wordlen);

uint32_t pke_modsub(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
                    uint32_t *out, uint32_t wordlen);

uint32_t pke_add(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen);

uint32_t pke_sub(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen);

uint32_t pke_mul(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t ab_wordLen);

uint32_t pke_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t a_wordlen,
                          uint32_t b_wordlen, uint32_t out_wordlen);

uint32_t pke_pre_calc_mont(const uint32_t *modulus, uint32_t bitlen, uint32_t *H);

uint32_t pke_pre_calc_mont_no_output(const uint32_t *modulus, uint32_t wordlen);

void pke_load_pre_calc_mont(uint32_t *H,  uint32_t wordlen);

uint32_t pke_modmul_internal(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, uint32_t *out,
                             uint32_t wordlen);

uint32_t pke_modmul(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordlen);

uint32_t pke_modexp(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
                    uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen);

uint32_t pke_mod(uint32_t *a, uint32_t awordlen, uint32_t *b, uint32_t *b_h, uint32_t bwordlen, uint32_t *c);

uint32_t eccp_pointMul(eccp_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                       uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir(eccp_curve_t *curve,
                              uint32_t *k1, uint32_t *P1x, uint32_t *P1y,
                              uint32_t *k2, uint32_t *P2x, uint32_t *P2y,
                              uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir_safe(eccp_curve_t *curve,
                                   uint32_t *k1, uint32_t *P1x, uint32_t *P1y,
                                   uint32_t *k2, uint32_t *P2x, uint32_t *P2y,
                                   uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_base(eccp_curve_t *curve, uint32_t *k, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointAdd(eccp_curve_t *curve, uint32_t *P1x, uint32_t *P1y, uint32_t *P2x, uint32_t *P2y,
                       uint32_t *Qx, uint32_t *Qy);

void pke_set_operand_width(uint32_t bitlen);

void pke_load_operand(uint32_t *baseaddr, uint32_t *data, uint32_t wordlen);

void pke_clear_interrupt(void);

void pke_set_microcode(uint32_t addr);

void pke_start(void);

uint32_t pke_check_rt_code(void);

void pke_wait_till_done(void);

/* recommended not to define */
#define ECCP_POINT_DOUBLE
#ifdef ECCP_POINT_DOUBLE
uint32_t eccp_pointDouble(eccp_curve_t *curve, uint32_t *Px, uint32_t *Py, uint32_t *Qx, uint32_t *Qy);
#endif

uint32_t eccp_pointVerify(eccp_curve_t *curve, uint32_t *Px, uint32_t *Py);

uint32_t eccp_get_pubkey_from_prikey(eccp_curve_t *curve, uint8_t *prikey, uint8_t *pubkey);

uint32_t eccp_getkey(eccp_curve_t *curve, uint8_t *prikey, uint8_t *pubkey);

#ifdef SUPPORT_C25519
uint32_t x25519_pointMul(mont_curve_t *curve, uint32_t *k, uint32_t *Pu, uint32_t *Qu);

uint32_t ed25519_pointMul(edward_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py,
                          uint32_t *Qx, uint32_t *Qy);

uint32_t ed25519_pointAdd(edward_curve_t *curve, uint32_t *P1x, uint32_t *P1y, uint32_t *P2x, uint32_t *P2y,
                          uint32_t *Qx, uint32_t *Qy);

uint32_t ed25519_decode_point(edward_curve_t *curve, uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32]);
#endif

#ifdef PKE_SEC

uint32_t pke_sec_init(void);

uint32_t pke_sec_uninit(void);

uint32_t pke_modexp_ladder(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
                           uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen);

uint32_t pke_modexp_with_pub(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *pub, const uint32_t *base,
                             uint32_t *out, uint32_t mod_wordlen, uint32_t exp_wordlen, uint32_t pub_wordlen);

uint32_t eccp_pointMul_sec(eccp_curve_t *curve, uint32_t *k, uint32_t *Px, uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

#endif

#ifdef __cplusplus
}
#endif

#endif
