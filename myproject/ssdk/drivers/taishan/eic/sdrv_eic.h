/*
 *  @File: sdrv_eic.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights RESERVEDd.
 *
 * @Description:
 *
 * @Revision History:
 * ----------------
 */

#ifndef DRV_LLD_SDRV_EIC_H
#define DRV_LLD_SDRV_EIC_H

#include <compiler.h>
#include <types.h>
#include "eic_hw.h"

#define LOCK_OFF	0x0U

#define BM_LOCK_LOCK	(0x01U << 0U)

#define EN_OFF(n)	(0x100U + 4U*(n))

#define FM_EN_ENABLE	(0xffffffffU << 0U)
#define FV_EN_ENABLE(v) \
	(((v) << 0U) & FM_EN_ENABLE)
#define GFV_EN_ENABLE(v) \
	(((v) & FM_EN_ENABLE) >> 0U)

#define BIT_OFF(n)	(0x200U + 4U*(n))

#define FM_BIT_BITS	(0xffffffffU << 0U)
#define FV_BIT_BITS(v) \
	(((v) << 0U) & FM_BIT_BITS)
#define GFV_BIT_BITS(v) \
	(((v) & FM_BIT_BITS) >> 0U)

#define FUSA_COR_ERR_INT_STAT_OFF	0x300U

#define BM_FUSA_COR_ERR_INT_STAT_PWDATA_COR_ERR	(0x01U << 0U)

#define FUSA_COR_ERR_INT_STAT_EN_OFF	0x304U

#define BM_FUSA_COR_ERR_INT_STAT_EN_PWDATA_COR_ERR	(0x01U << 0U)

#define FUSA_COR_ERR_INT_SIG_EN_OFF	0x308U

#define BM_FUSA_COR_ERR_INT_SIG_EN_PWDATA_COR_ERR	(0x01U << 0U)

#define FUSA_UNCOR_ERR_INT_STAT_OFF	0x30cU

#define BM_FUSA_UNCOR_ERR_INT_STAT_INPUT_ERR	(0x01U << 4U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_PCTL_UNCOR_ERR	(0x01U << 3U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_PADDR_UNCOR_ERR	(0x01U << 2U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_PWDATA_FATAL_ERR	(0x01U << 1U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_PWDATA_UNCOR_ERR	(0x01U << 0U)

#define FUSA_UNCOR_ERR_INT_STAT_EN_OFF	0x310U

#define BM_FUSA_UNCOR_ERR_INT_STAT_EN_INPUT_ERR	(0x01U << 4U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_EN_PCTL_UNCOR_ERR	(0x01U << 3U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_EN_PADDR_UNCOR_ERR	(0x01U << 2U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_FATAL_ERR	(0x01U << 1U)

#define BM_FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_UNCOR_ERR	(0x01U << 0U)

#define FUSA_UNCOR_ERR_INT_SIG_EN_OFF	0x314U

#define BM_FUSA_UNCOR_ERR_INT_SIG_EN_INPUT_ERR	(0x01U << 4U)

#define BM_FUSA_UNCOR_ERR_INT_SIG_EN_PCTL_UNCOR_ERR	(0x01U << 3U)

#define BM_FUSA_UNCOR_ERR_INT_SIG_EN_PADDR_UNCOR_ERR	(0x01U << 2U)

#define BM_FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_FATAL_ERR	(0x01U << 1U)

#define BM_FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_UNCOR_ERR	(0x01U << 0U)

#define PWDATA_OFF	0x318U

#define PWCODE_OFF	0x31cU

#define INT_ERR_OFF	0x320U

#define BM_INT_ERR_FUSA_UNCOR_INJ	(0x01U << 1U)

#define BM_INT_ERR_FUSA_COR_INJ	(0x01U << 0U)

#define SELFTEST_MODE_OFF	0x324U

#define BM_SELFTEST_MODE_EN	(0x01U << 0U)

#define ERR_ENABLE_OFF(n)	(0x328U + 4U*(n))

#define EIC_NO_ERR 0
#define EIC_ERR    1

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* EIC modules */
typedef enum {
    EIC_SF = 0,
    EIC_BOOT,
    EIC_SP,
    EIC_MAX
} sdrv_eic_module_e;

void sdrv_eic_lld_enable_lock(paddr_t base, bool en);
uint32_t sdrv_eic_lld_enable_err_inj(enum eic_err_inj_signal signal, bool en);
uint32_t sdrv_eic_lld_err_inj(enum eic_err_inj_signal signal, uint32_t inj_bit, bool toggle);
bool sdrv_eic_lld_fusa_cor_err_int_sta_get(paddr_t base, uint32_t mask);
void sdrv_eic_lld_fusa_cor_err_int_sta_clr(paddr_t base, uint32_t value);
void sdrv_eic_lld_fusa_cor_err_int_sta_en(paddr_t base, uint32_t mask, uint32_t value);
void sdrv_eic_lld_fusa_cor_err_int_sig_en(paddr_t base, uint32_t mask, uint32_t value);
bool sdrv_eic_lld_fusa_uncor_err_int_sta_get(paddr_t base, uint32_t mask);
void sdrv_eic_lld_fusa_uncor_err_int_sta_clr(paddr_t base, uint32_t value);
void sdrv_eic_lld_fusa_uncor_err_int_sta_en(paddr_t base, uint32_t mask, uint32_t value);
void sdrv_eic_lld_fusa_uncor_err_int_sig_en(paddr_t base, uint32_t mask, uint32_t value);
void sdrv_eic_lld_selftst_pwdata_err_inj(paddr_t base, uint32_t err_inj_bit, bool toggle);
void sdrv_eic_lld_selftst_pwdatacode_err_inj(paddr_t base, uint32_t err_inj_bit, bool toggle);
void sdrv_eic_lld_int_err_inj(paddr_t base, uint32_t err_inj_bit, bool toggle);
void sdrv_eic_lld_selftst_mode_en(paddr_t base, bool en);
uint32_t sdrv_eic_lld_en_n_err_inj(enum eic_err_inj_en_n_signal signal, bool toggle);
__END_CDECLS

#endif

#endif
