/**
 * @file sdrv_eic_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv eic driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_DRV_EIC_H
#define INCLUDE_SDRV_DRV_EIC_H

#include <cmsis_os2.h>
#include <compiler.h>
#include <types.h>
#ifdef CONFIG_TAISHAN_SEM
#include "sdrv_eic.h"
#endif
#ifdef CONFIG_KUNLUN_SEM
#include <../drivers/kunlun/eic/kunlun_eic.h>
#endif
#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief EIC error sta type.
 */
typedef enum {
    EIC_PWDATA_COR_ERR = 0,
    EIC_PWDATA_UNCOR_ERR,
    EIC_PWDATA_FATAL_ERR,
    EIC_PADDR_UNCOR_ERR,
    EIC_PCTL_UNCOR_ERR,
    EIC_INPUT_ERR
} sdrv_eic_err_sta_e;

/**
 * @brief EIC int error type.
 */
typedef enum {
    EIC_FUSA_COR_ERR = 0,
    EIC_FUSA_UNCOR_ERR
} sdrv_eic_int_err_e;

/**
 * @brief EIC device:EIC_BOOT/EIC_SF/EIC_SP.
 */
typedef struct eic_dev {
    paddr_t base;
    int irq;
    void *priv;
} eic_dev_t;

/**
 * @brief eic error injection signal configurations.
 */
typedef struct eic_err_inj_sig_config {
    #ifdef CONFIG_TAISHAN_SEM
    enum eic_err_inj_signal * err_inj_sig;
    #endif
    #ifdef CONFIG_KUNLUN_SEM
    enum eic_signal * err_inj_sig;
    #endif
    uint32_t err_inj_bit;
} eic_err_inj_sig_config_t;

/**
 * @brief EIC error injection for others IP.
 *
 * @param[in] dev      EIC device.
 * @param[in] cfg      EIC error injection signal configuration.
 * @param[in] en       Enable or disable error injection.
 */
uint32_t sdrv_eic_err_injection(eic_dev_t *dev, const eic_err_inj_sig_config_t *cfg, bool en);

__END_CDECLS

#endif

#endif