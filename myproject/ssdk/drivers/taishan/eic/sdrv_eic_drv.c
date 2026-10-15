/*
 * sdrv_eic_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv eic driver.
 *
 * Revision History:
 * -----------------
 */
#include <compiler.h>
#include <spinlock.h>
#include <string.h>
#include <types.h>

#include "driver/taishan/eic/sdrv_eic_drv.h"

#include "debug.h"

#define EXTRA_FMT "sdrv_eic:"
#define SDRV_EIC_ERR(format, ...)                                              \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_EIC_WARN(format, ...)                                             \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_EIC_INFO(format, ...)                                             \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/**
 * @brief EIC error injection for others IP.
 *
 * @param[in] dev      EIC device.
 * @param[in] cfg      EIC error injection signal configuration.
 * @param[in] en       Enable or disable error injection.
 */
uint32_t sdrv_eic_err_injection(eic_dev_t *dev,
                                const eic_err_inj_sig_config_t *cfg, bool en)
{
    uint32_t ret = EIC_NO_ERR;

    sdrv_eic_lld_enable_lock(dev->base, 0);
    /* enable error injection */
    if (sdrv_eic_lld_enable_err_inj(cfg->err_inj_sig, en) == EIC_ERR) {
        return EIC_ERR;
    }
    /* toggle 1 bit to inject error */
    ret = sdrv_eic_lld_err_inj(cfg->err_inj_sig, cfg->err_inj_bit, en);

    return ret;
}

/**
 * @brief EIC clear error interrupt.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC error int.
 * @param[in] clr               Write 1 bit to clear.
 */
void sdrv_eic_clear_err_int(eic_dev_t *dev, sdrv_eic_err_sta_e err_int,
                            bool clr)
{
    switch (err_int) {
    case EIC_PWDATA_COR_ERR:
        sdrv_eic_lld_fusa_cor_err_int_sta_clr(dev->base, (uint32_t)clr);
        break;
    case EIC_PWDATA_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_clr(dev->base,
                                                (uint32_t)(clr << 0));
        break;
    case EIC_PWDATA_FATAL_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_clr(dev->base,
                                                (uint32_t)(clr << 1));
        break;
    case EIC_PADDR_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_clr(dev->base,
                                                (uint32_t)(clr << 2));
        break;
    case EIC_PCTL_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_clr(dev->base,
                                                (uint32_t)(clr << 3));
        break;
    case EIC_INPUT_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_clr(dev->base,
                                                (uint32_t)(clr << 4));
        break;
    default:
        break;
    }
}

/**
 * @brief EIC get error interrupt.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC error int.
 */
bool sdrv_eic_get_err_int_sta(eic_dev_t *dev, sdrv_eic_err_sta_e err_int)
{
    bool status = false;

    switch (err_int) {
    case EIC_PWDATA_COR_ERR:
        status = sdrv_eic_lld_fusa_cor_err_int_sta_get(
            dev->base, BM_FUSA_COR_ERR_INT_STAT_PWDATA_COR_ERR);
        break;
    case EIC_PWDATA_UNCOR_ERR:
        status = sdrv_eic_lld_fusa_uncor_err_int_sta_get(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_PWDATA_UNCOR_ERR);
        break;
    case EIC_PWDATA_FATAL_ERR:
        status = sdrv_eic_lld_fusa_uncor_err_int_sta_get(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_PWDATA_FATAL_ERR);
        break;
    case EIC_PADDR_UNCOR_ERR:
        status = sdrv_eic_lld_fusa_uncor_err_int_sta_get(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_PADDR_UNCOR_ERR);
        break;
    case EIC_PCTL_UNCOR_ERR:
        status = sdrv_eic_lld_fusa_uncor_err_int_sta_get(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_PCTL_UNCOR_ERR);
        break;
    case EIC_INPUT_ERR:
        status = sdrv_eic_lld_fusa_uncor_err_int_sta_get(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_INPUT_ERR);
        break;
    default:
        break;
    }

    return status;
}

/**
 * @brief EIC enable error interrupt status.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC int error.
 * @param[in] en                EIC enable error int status.
 */
void sdrv_eic_enable_err_int_sta(eic_dev_t *dev, sdrv_eic_err_sta_e err_int,
                                 bool en)
{
    switch (err_int) {
    case EIC_PWDATA_COR_ERR:
        sdrv_eic_lld_fusa_cor_err_int_sta_en(
            dev->base, BM_FUSA_COR_ERR_INT_STAT_EN_PWDATA_COR_ERR,
            (uint32_t)en);
        break;
    case EIC_PWDATA_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_UNCOR_ERR,
            (uint32_t)(en << 0));
        break;
    case EIC_PWDATA_FATAL_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_EN_PWDATA_FATAL_ERR,
            (uint32_t)(en << 1));
        break;
    case EIC_PADDR_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_EN_PADDR_UNCOR_ERR,
            (uint32_t)(en << 2));
        break;
    case EIC_PCTL_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_EN_PCTL_UNCOR_ERR,
            (uint32_t)(en << 3));
        break;
    case EIC_INPUT_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sta_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_STAT_EN_INPUT_ERR,
            (uint32_t)(en << 4));
        break;
    default:
        break;
    }
}

/**
 * @brief EIC enable error interrupt signal.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC int error.
 * @param[in] en                EIC enable error int signal.
 */
void sdrv_eic_enable_err_int_sig(eic_dev_t *dev, sdrv_eic_err_sta_e err_int,
                                 bool en)
{
    switch (err_int) {
    case EIC_PWDATA_COR_ERR:
        sdrv_eic_lld_fusa_cor_err_int_sig_en(
            dev->base, BM_FUSA_COR_ERR_INT_SIG_EN_PWDATA_COR_ERR, (uint32_t)en);
        break;
    case EIC_PWDATA_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sig_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_UNCOR_ERR,
            (uint32_t)(en << 0));
        break;
    case EIC_PWDATA_FATAL_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sig_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_SIG_EN_PWDATA_FATAL_ERR,
            (uint32_t)(en << 1));
        break;
    case EIC_PADDR_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sig_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_SIG_EN_PADDR_UNCOR_ERR,
            (uint32_t)(en << 2));
        break;
    case EIC_PCTL_UNCOR_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sig_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_SIG_EN_PCTL_UNCOR_ERR,
            (uint32_t)(en << 3));
        break;
    case EIC_INPUT_ERR:
        sdrv_eic_lld_fusa_uncor_err_int_sig_en(
            dev->base, BM_FUSA_UNCOR_ERR_INT_SIG_EN_INPUT_ERR,
            (uint32_t)(en << 4));
        break;
    default:
        break;
    }
}

/**
 * @brief EIC error injection for eic pwdata.
 *
 * @param[in] dev                   EIC device.
 * @param[in] err_inj_bit           error injection bit.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_pwdata_err_inj(eic_dev_t *dev, uint32_t err_inj_bit, bool toggle)
{
    sdrv_eic_lld_selftst_pwdata_err_inj(dev->base, err_inj_bit, toggle);
}

/**
 * @brief EIC error injection for eic pwdatacode.
 *
 * @param[in] dev                   EIC device.
 * @param[in] err_inj_bit           error injection bit.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_pwdatacode_err_inj(eic_dev_t *dev, uint32_t err_inj_bit,
                                 bool toggle)
{
    sdrv_eic_lld_selftst_pwdatacode_err_inj(dev->base, err_inj_bit, toggle);
}

/**
 * @brief EIC error injection for eic int error.
 *
 * @param[in] dev                   EIC device.
 * @param[in] int_err               int error.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_int_err_inj(eic_dev_t *dev, sdrv_eic_int_err_e int_err,
                          bool toggle)
{
    switch (int_err) {
    case EIC_FUSA_COR_ERR:
        sdrv_eic_lld_int_err_inj(dev->base, 0, toggle);
        break;
    case EIC_FUSA_UNCOR_ERR:
        sdrv_eic_lld_int_err_inj(dev->base, 1, toggle);
        break;
    default:
        break;
    }
}

/**
 * @brief EIC enable selftest mode.
 *
 * @param[in] dev                   EIC device.
 * @param[in] en                    enable selftest mode.
 */
void sdrv_eic_enable_selftest_mode(eic_dev_t *dev, bool en)
{
    sdrv_eic_lld_selftst_mode_en(dev->base, en);
}

/**
 * @brief EIC error enable error injection.
 *
 * @param[in] signal                error injection signal.
 * @param[in] toggle                toggle 1 bit.
 */
bool sdrv_eic_err_en_err_inj(enum eic_err_inj_en_n_signal signal, bool toggle)
{
    bool ret = sdrv_eic_lld_en_n_err_inj(signal, toggle);

    return ret;
}
