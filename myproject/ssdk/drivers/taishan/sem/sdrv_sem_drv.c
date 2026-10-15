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
#include <spinlock.h>
#include <string.h>
#include <types.h>

#include "driver/taishan/sem/sdrv_sem_drv.h"

#include "debug.h"
#include "driver/irq.h"

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
    sdrv_sem_lld_glb_mon_en(dev->base, en);
    sdrv_sem_lld_cmp_en(dev->base, cfg->int_out, en);
    sdrv_sem_lld_mon_compare_sig(dev->base, cfg->mon_sig, en,
                                 cfg->sig_targ_val);
}

/**
 * @brief The Error Injection is globally enabled or disabled by SEM.
 *
 * @param[in] dev      SEM device.
 * @param[in] en       Enable or disable error injection globally.
 */
void sdrv_sem_glb_err_inj_enable(sem_dev_t *dev, bool en)
{
    sdrv_sem_lld_glb_ei_lock(dev->base, 0);
    sdrv_sem_lld_error_inj_glb_en(dev->base, en);
}

/**
 * @brief Global error injection enable error injection by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] toggle            Toggle global ej en.
 */
void sdrv_sem_glb_ej_en_error_inj(sem_dev_t *dev, bool toggle)
{
    sdrv_sem_lld_int_ctrl_lock(dev->base, 0);
    sdrv_sem_lld_glb_error_inj(dev->base, toggle);
}

/**
 * @brief hw(rstgen/dc/iomux/xtrg/sf_pll) interrupt to sem int4~7 selection
 * 2-bit to rstgen/dc/iomux/xtrg/sf_pll,00->sem_int4,01->sem_int5,10->sem_int6
 * 11->sem_int7.
 *
 * @param[in] dev           SEM device.
 * @param[in] int_sel       HW int to SEM int4~7 selection.
 */
void sdrv_sem_hw_int_sel(sem_dev_t *dev, sdrv_sem_int_sel_cfg_t *int_sel_cfg)
{
    sdrv_sem_lld_hw_int_sel(dev->base, int_sel_cfg);
}

/**
 * @brief sem error configuration for error divider for waveformA/waveform A/B
 * selection/fatal error high/low selection.
 *
 * @param[in] dev           SEM device.
 * @param[in] err_cfg       SEM error configuration.
 */
void sdrv_sem_error_config(sem_dev_t *dev, const sdrv_sem_err_cfg_t *err_cfg)
{
    sdrv_sem_lld_error_config(dev->base, err_cfg);
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
    sdrv_sem_lld_trigger_en(dev->base, int_out, en);
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
    sdrv_sem_lld_override_en(dev->base, int_out, en);
}

/**
 * @brief Parity error injection is globally enabled or disabled by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] en                Enable/disable parity error injection globally.
 */
void sdrv_sem_glb_parity_error_inj_enable(sem_dev_t *dev, bool en)
{
    sdrv_sem_lld_parity_error_inj_en(dev->base, en);
}

/**
 * @brief Parity error injection enable error injection by SEM.
 *
 * @param[in] dev               SEM device.
 * @param[in] toggle            Toggle parity ej en.
 */
void sdrv_sem_glb_parity_ej_en_error_inj(sem_dev_t *dev, bool toggle)
{
    sdrv_sem_lld_int_ctrl_lock(dev->base, 0);
    sdrv_sem_lld_parity_error_inj(dev->base, toggle);
}

/**
 * @brief SEM interrupt to io wave control.
 *
 * @param[in] dev               SEM device.
 * @param[in] io_wave_cfg       Io wave config.
 */
void sdrv_sem_int2io_wave_ctl(sem_dev_t *dev,
                              const sdrv_sem_io_wave_cfg_t *io_wave_cfg)
{
    sdrv_sem_lld_output_io_ctl(dev->base, io_wave_cfg);
}

/**
 * @brief SEM interrupt error injection.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_err           SEM int error.
 * @param[in] toggle            Toggle 1 bit.
 */
void sdrv_sem_inj_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err,
                          bool toggle)
{
    switch (int_err) {
    case HW_RSTGEN_INT_ERR:
        sdrv_sem_lld_hw_int_error_inj(dev->base, (uint32_t)(toggle << 0));
        break;
    case HW_DC_INT_ERR:
        sdrv_sem_lld_hw_int_error_inj(dev->base, (uint32_t)(toggle << 1));
        break;
    case HW_IOMUX_INT_ERR:
        sdrv_sem_lld_hw_int_error_inj(dev->base, (uint32_t)(toggle << 2));
        break;
    case HW_XTRG_INT_ERR:
        sdrv_sem_lld_hw_int_error_inj(dev->base, (uint32_t)(toggle << 3));
        break;
    case HW_SF_PLL_INT_ERR:
        sdrv_sem_lld_hw_int_error_inj(dev->base, (uint32_t)(toggle << 4));
        break;
    case PMU_INT_ERR:
        sdrv_sem_lld_pmu_int_error_inj(dev->base, toggle);
        break;
    case CPU_INT_ERR:
        sdrv_sem_lld_cpu_int_error_inj(dev->base, toggle);
        break;
    case PAD_INT_ERR:
        sdrv_sem_lld_pad_int_error_inj(dev->base, toggle);
        break;
    case CORERR_INT_ERR:
        sdrv_sem_lld_corerr_int_error_inj(dev->base, toggle);
        break;
    case UNCERR_INT_ERR:
        sdrv_sem_lld_uncorerr_int_error_inj(dev->base, toggle);
        break;
    default:
        break;
    }
}

/**
 * @brief SEM interrupt error clear.
 *
 * @param[in] dev               SEM device.
 * @param[in] int_err           SEM int error.
 * @param[in] clr               Write 1 bit to clear.
 */
void sdrv_sem_clear_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err,
                            bool clr)
{
    switch (int_err) {
    case CORERR_INT_ERR:
        sdrv_sem_lld_corerr_int_error_clr(dev->base, clr);
        break;
    case UNCERR_INT_ERR:
        sdrv_sem_lld_uncorerr_int_error_clr(dev->base, clr);
        break;
    default:
        break;
    }
}

/**
 * @brief SEM gets interrupt error.
 *
 * @param[in]  dev               SEM device.
 * @param[in]  int_err           SEM int error.
 * @param[out] status            SEM int error status.
 */
bool sdrv_sem_get_int_err(sem_dev_t *dev, sdrv_sem_int_err_e int_err)
{
    bool status = false;

    switch (int_err) {
    case CORERR_INT_ERR:
        status = sdrv_sem_lld_get_corerr_int_error_sta(dev->base);
        break;
    case UNCERR_INT_ERR:
        status = sdrv_sem_lld_get_uncorerr_int_error_sta(dev->base);
        break;
    default:
        break;
    }
    return status;
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
    switch (cfg->int_type) {
    case CORERR_INT_IN:
        sdrv_sem_lld_corr_interrupt_en(dev->base, cfg->int_out,
                                       (sem_corr_int_input)(cfg->int_in), en);
        break;
    case UNCERR_INT_IN:
        sdrv_sem_lld_uncorr_interrupt_en(
            dev->base, cfg->int_out, (sem_uncorr_int_input)(cfg->int_in), en);
        break;
    default:
        break;
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

    switch (cfg->int_type) {
    case CORERR_INT_IN:
        status = sdrv_sem_lld_corr_int_status(
            dev->base, (sem_corr_int_input)(cfg->int_in));
        break;
    case UNCERR_INT_IN:
        status = sdrv_sem_lld_uncorr_int_status(
            dev->base, (sem_uncorr_int_input)(cfg->int_in));
        break;
    default:
        break;
    }
    return status;
}
