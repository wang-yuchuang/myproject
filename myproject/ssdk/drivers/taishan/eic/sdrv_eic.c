/**
 * @file: sdrv_eic.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description
 *
 * @Revision History:
 * ----------------
 */

#include <reg.h>
#include <regs_base.h>
#include <string.h>
#include <types.h>

#include "sdrv_eic.h"

/* EIC base address */
static uint32_t eic_addr[EIC_MAX] = {[EIC_SF] = APB_EIC_SF_BASE,
                                     [EIC_BOOT] = APB_EIC_BOOT_BASE,
                                     [EIC_SP] = APB_EIC_SP_BASE};

/**
 * @brief sdrv eic error injection bit.
 *
 * @param[in]  signal         eic error injection signal
 * @param[out] return       error injection bit
 */
static inline uint32_t sdrv_eic_lld_get_eic_id(enum eic_err_inj_signal signal)
{
    return (((uint32_t)signal >> 28) & 0x0f);
}

/**
 * @brief sdrv eic error injection enable bit.
 *
 * @param[in]  signal       eic error injection signal
 * @param[out] return       error injection enable bit
 */
static inline uint32_t
sdrv_eic_lld_get_err_inj_enable_bit(enum eic_err_inj_signal signal)
{
    return (((uint32_t)signal >> 20) & 0xff);
}

/**
 * @brief sdrv eic error injection start bit.
 *
 * @param[in]  signal       eic error injection signal
 * @param[out] return       error injection bit
 */
static inline uint32_t
sdrv_eic_lld_get_err_inj_start_bit(enum eic_err_inj_signal signal)
{
    return (((uint32_t)signal >> 8) & 0xfff);
}

/**
 * @brief sdrv eic error injection signal length.
 *
 * @param[in]  signal       eic error injection signal
 * @param[out] return       error injection signal length
 */
static inline uint32_t
sdrv_eic_lld_get_err_inj_sig_len(enum eic_err_inj_signal signal)
{
    return (((uint32_t)signal) & 0xff);
}

/**
 * @brief sdrv eic lock enable.
 *
 * @param[in] base        reg base
 * @param[in] en          enable/disable
 */
void sdrv_eic_lld_enable_lock(paddr_t base, bool en)
{
    uint32_t val = 0;
    uint32_t addr = base + LOCK_OFF;

    val = readl(addr);
    en ? (val |= BM_LOCK_LOCK) : (val &= ~BM_LOCK_LOCK);
    writel(val, addr);
}

/**
 * @brief sdrv eic error inject enable.
 *
 * @param[in]  signal        eic error injection signal
 * @param[in]  en            reg val
 * @param[out] return        eic error
 */
uint32_t sdrv_eic_lld_enable_err_inj(enum eic_err_inj_signal signal, bool en)
{
    uint32_t eic_id = sdrv_eic_lld_get_eic_id(signal);
    uint32_t err_inj_en_bit = 0;
    uint32_t val = 0;

    /* return error if eic id exceeds EIC_MAX */
    if (eic_id >= EIC_MAX) {
        return EIC_ERR;
    }
    err_inj_en_bit = sdrv_eic_lld_get_err_inj_enable_bit(signal);
    val = readl(eic_addr[eic_id] + EN_OFF(err_inj_en_bit / 32));
    en ? (val |= (1 << (err_inj_en_bit % 32)))
       : (val &= ~(1 << (err_inj_en_bit % 32)));
    writel(val, eic_addr[eic_id] + EN_OFF(err_inj_en_bit / 32));
    return EIC_NO_ERR;
}

/**
 * @brief sdrv eic error inject .
 *
 * @param[in]  signal        eic error injection signal
 * @param[in]  inj_bit       error injection bit
 * @param[in]  toggle        toggle 1 or 0
 * @param[out] return        eic error
 */
uint32_t sdrv_eic_lld_err_inj(enum eic_err_inj_signal signal, uint32_t inj_bit,
                              bool toggle)
{
    uint32_t eic_id = sdrv_eic_lld_get_eic_id(signal);
    uint32_t err_inj_bit, inj_start_bit = 0;
    uint32_t val = 0;

    /* return error if eic id exceeds EIC_MAX or error injection bit > length */
    if ((eic_id >= EIC_MAX) ||
        (inj_bit >= sdrv_eic_lld_get_err_inj_sig_len(signal))) {
        return EIC_ERR;
    }

    inj_start_bit = sdrv_eic_lld_get_err_inj_start_bit(signal);
    err_inj_bit = inj_start_bit + inj_bit;
    val = readl(eic_addr[eic_id] + BIT_OFF(err_inj_bit / 32));
    toggle ? (val |= (1 << (err_inj_bit % 32)))
           : (val &= ~(1 << (err_inj_bit % 32)));
    writel(val, eic_addr[eic_id] + BIT_OFF(err_inj_bit / 32));
    return EIC_NO_ERR;
}

/**
 * @brief sdrv eic gets self cor error interrupt status.
 *
 * @param[in]  eic_id        eic id
 * @param[in]  mask          mask
 * @param[out] return        status
 */
bool sdrv_eic_lld_fusa_cor_err_int_sta_get(paddr_t base, uint32_t mask)
{
    uint32_t addr = base + FUSA_COR_ERR_INT_STAT_OFF;
    uint32_t val = readl(addr);

    return ((val & mask) ? 1 : 0);
}

/**
 * @brief sdrv eic clear self cor error interrupt status.
 *
 * @param[in]  base        reg base
 * @param[in]  value       write 1 clear
 */
void sdrv_eic_lld_fusa_cor_err_int_sta_clr(paddr_t base, uint32_t value)
{
    uint32_t addr = base + FUSA_COR_ERR_INT_STAT_OFF;
    uint32_t val = readl(addr);

    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic self cor error interrupt status enable.
 *
 * @param[in] eic_id        eic id
 * @param[in] mask          mask
 * @param[in] value         value
 */
void sdrv_eic_lld_fusa_cor_err_int_sta_en(paddr_t base, uint32_t mask,
                                          uint32_t value)
{
    uint32_t addr = base + FUSA_COR_ERR_INT_STAT_EN_OFF;
    uint32_t val = readl(addr);

    val &= ~mask;
    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic self cor error interrupt signal enable.
 *
 * @param[in] base        reg base
 * @param[in] mask        mask
 * @param[in] value       value
 */
void sdrv_eic_lld_fusa_cor_err_int_sig_en(paddr_t base, uint32_t mask,
                                          uint32_t value)
{
    uint32_t addr = base + FUSA_COR_ERR_INT_SIG_EN_OFF;
    uint32_t val = readl(addr);

    val &= ~mask;
    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic gets self uncor error interrupt status.
 *
 * @param[in]  base          reg base
 * @param[in]  mask          mask
 * @param[out] return        status
 */
bool sdrv_eic_lld_fusa_uncor_err_int_sta_get(paddr_t base, uint32_t mask)
{
    uint32_t addr = base + FUSA_UNCOR_ERR_INT_STAT_OFF;
    uint32_t val = readl(addr);

    return (bool)((val & mask) ? 1 : 0);
}

/**
 * @brief sdrv eic clear self uncorr error interrupt status.
 *
 * @param[in]  base          reg base
 * @param[in]  value         write 1 clear
 */
void sdrv_eic_lld_fusa_uncor_err_int_sta_clr(paddr_t base, uint32_t value)
{
    uint32_t addr = base + FUSA_UNCOR_ERR_INT_STAT_OFF;
    uint32_t val = readl(addr);

    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic self uncor error interrupt status enable.
 *
 * @param[in] base          reg base
 * @param[in] mask          mask
 * @param[in] value         value
 */
void sdrv_eic_lld_fusa_uncor_err_int_sta_en(paddr_t base, uint32_t mask,
                                            uint32_t value)
{
    uint32_t addr = base + FUSA_UNCOR_ERR_INT_STAT_EN_OFF;
    uint32_t val = readl(addr);

    val &= ~mask;
    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic self uncor error interrupt signal enable.
 *
 * @param[in] base          reg base
 * @param[in] mask          mask
 * @param[in] value         value
 */
void sdrv_eic_lld_fusa_uncor_err_int_sig_en(paddr_t base, uint32_t mask,
                                            uint32_t value)
{
    uint32_t addr = base + FUSA_UNCOR_ERR_INT_SIG_EN_OFF;
    uint32_t val = readl(addr);

    val &= ~mask;
    val |= value;
    writel(val, addr);
}

/**
 * @brief sdrv eic self pwdata error injection.
 *
 * @param[in] base          reg base
 * @param[in] err_inj_bit   error injected bit
 * @param[in] toggle        toggle bit
 */
void sdrv_eic_lld_selftst_pwdata_err_inj(paddr_t base, uint32_t err_inj_bit,
                                         bool toggle)
{
    uint32_t val = 0;
    uint32_t addr = base + PWDATA_OFF;

    val = readl(addr);
    toggle ? (val |= (1 << err_inj_bit)) : (val &= ~(1 << err_inj_bit));
    writel(val, addr);
}

/**
 * @brief sdrv eic self pwdata code error injection.
 *
 * @param[in] base          reg base
 * @param[in] err_inj_bit   error injected bit
 * @param[in] toggle        toggle bit
 */
void sdrv_eic_lld_selftst_pwdatacode_err_inj(paddr_t base, uint32_t err_inj_bit,
                                             bool toggle)
{
    uint32_t val = 0;
    uint32_t addr = base + PWCODE_OFF;

    val = readl(addr);
    toggle ? (val |= (1 << err_inj_bit)) : (val &= ~(1 << err_inj_bit));
    writel(val, addr);
}

/**
 * @brief sdrv eic interrupt error injection.
 *
 * @param[in] base          reg base
 * @param[in] err_inj_bit   error injected bit
 * @param[in] toggle        toggle bit
 */
void sdrv_eic_lld_int_err_inj(paddr_t base, uint32_t err_inj_bit, bool toggle)
{
    uint32_t val = 0;
    uint32_t addr = base + INT_ERR_OFF;

    val = readl(addr);
    toggle ? (val |= (1 << err_inj_bit)) : (val &= ~(1 << err_inj_bit));
    writel(val, addr);
}

/**
 * @brief sdrv eic self test mode enable.
 *
 * @param[in] base        reg base
 * @param[in] en          enable
 */
void sdrv_eic_lld_selftst_mode_en(paddr_t base, bool en)
{
    uint32_t val = 0;
    uint32_t addr = base + SELFTEST_MODE_OFF;

    val = readl(addr);
    en ? (val |= BM_SELFTEST_MODE_EN) : (val &= ~BM_SELFTEST_MODE_EN);
    writel(val, addr);
}

/**
 * @brief sdrv eic error injection for negative enable pin .
 *
 * @param[in]  signal        eic error injection negative enable pin signal
 * @param[in]  toggle        toggle bit
 * @param[out] return        eic error
 */
uint32_t sdrv_eic_lld_en_n_err_inj(enum eic_err_inj_en_n_signal signal,
                                   bool toggle)
{
    uint32_t eic_id = ((signal >> 16) & 0xff);
    uint32_t err_en_inj_bit = 0;
    uint32_t val = 0;

    /* return error if eic id exceeds EIC_MAX */
    if (eic_id >= EIC_MAX) {
        return EIC_ERR;
    }
    err_en_inj_bit = (signal & 0xffff);
    val = readl(eic_addr[eic_id] + ERR_ENABLE_OFF(err_en_inj_bit / 32));
    toggle ? (val |= (1 << (err_en_inj_bit % 32)))
           : (val &= ~(1 << (err_en_inj_bit % 32)));
    writel(val, eic_addr[eic_id] + ERR_ENABLE_OFF(err_en_inj_bit / 32));
    return EIC_NO_ERR;
}
