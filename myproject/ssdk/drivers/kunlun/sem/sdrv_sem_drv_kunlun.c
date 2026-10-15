/*
 * sdrv_sem_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv sem driver.
 *
 * Revision History:
 * -----------------
 */
#include <compiler.h>
#include <regs_base.h>
#include <spinlock.h>
#include <string.h>
#include <types.h>

#include "debug.h"
#include "driver/irq.h"
#include "kunlun_sem_lld.h"
#include "driver/kunlun/sem/sdrv_sem_drv.h"

#define EXTRA_FMT "sdrv_sem:"
#define SDRV_SEM_ERR(format, ...)                                              \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_SEM_WARN(format, ...)                                             \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_SEM_INFO(format, ...)                                             \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/**
 * @brief  SEM irq handler.
 *
 * @param[in] irq  SEM irq.
 */
static int sdrv_sem_irq_handler(uint32_t irq, void *arg)
{
    sem_dev_t *device = arg;

    if (device->SemNotification != NULL) {
        device->SemNotification();
    }

    return 0;
}

/**
 * @brief Init SEM.
 *
 * @param[in] device   SEM device.
 */
void sdrv_sem_init(sem_dev_t *device)
{
    if (device->irq >= 0) {
        irq_attach(device->irq, sdrv_sem_irq_handler, device);
        irq_enable(device->irq);
    }
}

/**
 * @brief Deinit SEM.
 *
 * @param[in] device   SEM device.
 */
void sdrv_sem_deinit(sem_dev_t *device)
{
    if (device->irq >= 0) {
        irq_disable(device->irq);
        irq_detach(device->irq);
    }
}

/**
 * @brief SEM internal monitor compares external monitor signals with
 * the target value(0 or 1).
 *
 * @param[in] dev      SEM device.
 * @param[in] cfg      SEM monitor configuration.
 * @param[in] en       Enable or disable sem monitor.
 */
void sdrv_sem_monitor_compare_sig(sem_dev_t *dev,
                                  const sem_mon_sig_config_t *cfg, bool en)
{
    sem_mon_enable(en);
    sem_mon_enable_intr(cfg->int_out, en);
    sem_mon_compare_sig(cfg->mon_sig, en, cfg->sig_targ_val);
}

/**
 * @brief The Error Injection is globally enabled or disabled by SEM.
 *
 * @param[in] dev      SEM device.
 * @param[in] en       Enable or disable error injection globally.
 */
void sdrv_sem_glb_err_inj_enable(sem_dev_t *dev, bool en)
{
    sem_enable_eic(en);
}

/**
 * @brief Global error injection enable error injection by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] toggle            Toggle global ej en.
 */
void sdrv_sem_glb_ej_en_error_inj(sem_dev_t *dev, bool toggle)
{
    sem_enable_eic(toggle);
}

/**
 * @brief sem int0~7 trigger.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_out           SEM int output.
 * @param[in] en                Enable/disable sem int trigger.
 */
void sdrv_sem_int_trigger(sem_dev_t *dev, sdrv_sem_int_out_e int_out, bool en)
{
    if (en) {
        if (dev->base == APB_SEM1_BASE) {
            sem_trigger_intr(0, int_out);
        }
        if (dev->base == APB_SEM2_BASE) {
            sem_trigger_intr(1, int_out);
        }
    }
}

/**
 * @brief sem int output to 0~7 enable.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_out           SEM int output.
 * @param[in] en                Enable/disable sem int output.
 */
void sdrv_sem_int_output_enable(sem_dev_t *dev, sdrv_sem_int_out_e int_out,
                                bool en)
{
    if (dev->base == APB_SEM1_BASE) {
        sem_enable_intr(0, int_out, en);
    }
    if (dev->base == APB_SEM2_BASE) {
        sem_enable_intr(1, int_out, en);
    }
}

/**
 * @brief SEM interrupt input enable.
 *
 * @param[in] dev               SEM device.
 * @param[in] cfg               SEM int input config.
 * @param[in] en                SEM int input enable.
 */
void sdrv_sem_int_input_enable(sem_dev_t *dev,
                               const sem_int_input_config_t *cfg, bool en)
{
    if (en) {
        if (dev->base == APB_SEM1_BASE) {
            sem_map_signal(0, cfg->int_in, cfg->int_out, en);
        }
        if (dev->base == APB_SEM2_BASE) {
            sem_map_signal(1, cfg->int_in, cfg->int_out, en);
        }
    }
}

/**
 * @brief SEM interrupt input status.
 *
 * @param[in] dev               SEM device.
 * @param[in] cfg               SEM int input config.
 */
bool sdrv_sem_int_input_sta(sem_dev_t *dev, const sem_int_input_config_t *cfg)
{

    bool status = false;

    if (dev->base == APB_SEM1_BASE) {
        status = sem_signal_status(0, cfg->int_in);
    }
    if (dev->base == APB_SEM2_BASE) {
        status = sem_signal_status(1, cfg->int_in);
    }
    return status;
}
