/**
 * @file sdrv_sem_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv sem driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_DRV_SEM_H
#define INCLUDE_SDRV_DRV_SEM_H

#include <cmsis_os2.h>
#include <compiler.h>
#include <types.h>
#include "taishan/sem/sdrv_sem.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief sem int error type.
 */
typedef enum sdrv_sem_int_err {
    HW_RSTGEN_INT_ERR = 0,
    HW_DC_INT_ERR,
    HW_IOMUX_INT_ERR,
    HW_XTRG_INT_ERR,
    HW_SF_PLL_INT_ERR,
    PMU_INT_ERR,
    CPU_INT_ERR,
    PAD_INT_ERR,
    CORERR_INT_ERR,
    UNCERR_INT_ERR
} sdrv_sem_int_err_e;

/**
 * @brief sem interrupt input type.
 */
typedef enum sdrv_sem_int_input {
    CORERR_INT_IN = 0,
    UNCERR_INT_IN
} sdrv_sem_int_input_e;

/**
 * @brief sem Notification function type.
 */
typedef void (*Sem_NotifyType)(void);

/**
 * @brief SEM1/SEM2 device.
 */
typedef struct sem_dev {
    paddr_t base;
    int irq;
    void *priv;
    Sem_NotifyType  SemNotification;
} sem_dev_t;

/**
 * @brief sem monitor signal configurations.
 */
typedef struct sem_mon_sig_config {
    uint32_t mon_sig;
    bool sig_targ_val;
    sdrv_sem_int_out_e int_out;
} sem_mon_sig_config_t;

/**
 * @brief sem interrupt input configuration.
 */
typedef struct sem_int_input_config {
    sdrv_sem_int_input_e int_type;
    sdrv_sem_int_out_e int_out;
    uint32_t int_in;
} sem_int_input_config_t;

/**
 * @brief Init SEM.
 *
 * @param[in] device   SEM device.
 */
void sdrv_sem_init(sem_dev_t *device);

/**
 * @brief Deinit SEM.
 *
 * @param[in] device   SEM device.
 */
void sdrv_sem_deinit(sem_dev_t *device);

/**
 * @brief SEM internal monitor compares external monitor signals with
 * the target value(0 or 1).
 *
 * @param[in] dev      SEM device.
 * @param[in] cfg      SEM monitor configuration.
 * @param[in] en       Enable or disable sem monitor.
 */
void sdrv_sem_monitor_compare_sig(sem_dev_t *dev, const sem_mon_sig_config_t *cfg, bool en);

/**
 * @brief The Error Injection is globally enabled or disabled by SEM.
 *
 * @param[in] dev      SEM device.
 * @param[in] en       Enable or disable error injection globally.
 */
void sdrv_sem_glb_err_inj_enable(sem_dev_t *dev, bool en);

/**
 * @brief Global error injection enable error injection by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] toggle            Toggle global ej en.
 */
void sdrv_sem_glb_ej_en_error_inj(sem_dev_t *dev, bool toggle);

/**
 * @brief hw(rstgen/dc/iomux/xtrg/sf_pll) interrupt to sem int4~7 selection
 * 2-bit to rstgen/dc/iomux/xtrg/sf_pll,00->sem_int4,01->sem_int5,10->sem_int6
 * 11->sem_int7.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_sel_cfg       HW int to SEM int4~7 selection.
 */
void sdrv_sem_hw_int_sel(sem_dev_t *dev, sdrv_sem_int_sel_cfg_t *int_sel_cfg);

/**
 * @brief sem error configuration for error divider for waveformA/waveform A/B
 * selection/fatal error high/low selection.
 *
 * @param[in] dev           SEM device.
 * @param[in] err_cfg       SEM error configuration.
 */
void sdrv_sem_error_config(sem_dev_t *dev, const sdrv_sem_err_cfg_t *err_cfg);

/**
 * @brief sem int0~7 trigger.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_out           SEM int output.
 * @param[in] en                Enable/disable sem int trigger.
 */
void sdrv_sem_int_trigger(sem_dev_t *dev, sdrv_sem_int_out_e int_out, bool en);

/**
 * @brief sem int output to 0~7 enable.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_out           SEM int output.
 * @param[in] en                Enable/disable sem int output.
 */
void sdrv_sem_int_output_enable(sem_dev_t *dev, sdrv_sem_int_out_e int_out, bool en);

/**
 * @brief Parity error injection is globally enabled or disabled by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] en                Enable/disable parity error injection globally.
 */
void sdrv_sem_glb_parity_error_inj_enable(sem_dev_t *dev, bool en);

/**
 * @brief Parity error injection enable error injection by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] toggle            Toggle parity ej en.
 */
void sdrv_sem_glb_parity_ej_en_error_inj(sem_dev_t *dev, bool toggle);

/**
 * @brief SEM interrupt to io wave control.
 *
 * @param[in] dev               SEM device.
 * @param[in] io_wave_cfg       Io wave config.
 */
void sdrv_sem_int2io_wave_ctl(sem_dev_t *dev, const sdrv_sem_io_wave_cfg_t *io_wave_cfg);

/**
 * @brief SEM interrupt error injection.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_err           SEM int error.
 * @param[in] toggle            Toggle 1 bit.
 */
void sdrv_sem_inj_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err, bool toggle);

/**
 * @brief SEM interrupt error clear.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_err           SEM int error.
 * @param[in] clr               Write 1 bit to clear.
 */
void sdrv_sem_clear_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err, bool clr);

/**
 * @brief SEM interrupt error get.
 *
 * @param[in]  dev               SEM device.
 * @param[in]  int_err           SEM int error.
 * @param[out] status            SEM int error status.
 */
bool sdrv_sem_get_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err);

/**
 * @brief SEM interrupt input enable.
 *
 * @param[in] dev               SEM device.
 * @param[in] cfg               SEM int input config.
 * @param[in] en                SEM int input enable.
 */
void sdrv_sem_int_input_enable(sem_dev_t *dev, const sem_int_input_config_t *cfg, bool en);

/**
 * @brief SEM interrupt input status.
 *
 * @param[in] dev               SEM device.
 * @param[in] cfg               SEM int input config.
 */
bool sdrv_sem_int_input_sta(sem_dev_t *dev, const sem_int_input_config_t *cfg);
__END_CDECLS

#endif

#endif