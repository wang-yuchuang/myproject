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
#include "sdrv_eic.h"

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
    enum eic_err_inj_signal err_inj_sig;
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

/**
 * @brief EIC clear error interrupt.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC error int.
 * @param[in] clr               Write 1 bit to clear.
 */
void sdrv_eic_clear_err_int(eic_dev_t *dev, sdrv_eic_err_sta_e err_int, bool clr);

/**
 * @brief EIC get error interrupt.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC error int.
 */
bool sdrv_eic_get_err_int_sta(eic_dev_t *dev, sdrv_eic_err_sta_e err_int);

/**
 * @brief EIC enable error interrupt status.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC int error.
 * @param[in] en                EIC enable error int status.
 */
void sdrv_eic_enable_err_int_sta(eic_dev_t *dev, sdrv_eic_err_sta_e err_int, bool en);

/**
 * @brief EIC enable error interrupt signal.
 *
 * @param[in] dev               EIC device.
 * @param[in] err_int           EIC int error.
 * @param[in] en                EIC enable error int signal.
 */
void sdrv_eic_enable_err_int_sig(eic_dev_t *dev, sdrv_eic_err_sta_e err_int, bool en);

/**
 * @brief EIC error injection for eic pwdata.
 *
 * @param[in] dev                   EIC device.
 * @param[in] err_inj_bit           error injection bit.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_pwdata_err_inj(eic_dev_t *dev, uint32_t err_inj_bit, bool toggle);

/**
 * @brief EIC error injection for eic pwdatacode.
 *
 * @param[in] dev                   EIC device.
 * @param[in] err_inj_bit           error injection bit.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_pwdatacode_err_inj(eic_dev_t *dev, uint32_t err_inj_bit, bool toggle);

/**
 * @brief EIC error injection for eic int error.
 *
 * @param[in] dev                   EIC device.
 * @param[in] int_err               int error.
 * @param[in] toggle                toggle 1 bit.
 */
void sdrv_eic_int_err_inj(eic_dev_t *dev, sdrv_eic_int_err_e int_err, bool toggle);

/**
 * @brief EIC enable selftest mode.
 *
 * @param[in] dev                   EIC device.
 * @param[in] en                    enable selftest mode.
 */
void sdrv_eic_enable_selftest_mode(eic_dev_t *dev, bool en);

/**
 * @brief EIC error enable error injection.
 *
 * @param[in] signal                error injection signal.
 * @param[in] toggle                toggle 1 bit.
 */
bool sdrv_eic_err_en_err_inj(enum eic_err_inj_en_n_signal signal, bool toggle);

__END_CDECLS

#endif

#endif
