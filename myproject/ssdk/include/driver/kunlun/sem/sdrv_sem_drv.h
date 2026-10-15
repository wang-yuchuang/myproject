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