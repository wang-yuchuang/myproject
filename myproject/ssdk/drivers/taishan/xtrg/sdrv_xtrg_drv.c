/**
 * @file sdrv_xtrg_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>
#include <string.h>
#include <types.h>

#include "driver/taishan/xtrg/sdrv_xtrg_drv.h"

#include "driver/irq.h"
#include "sdrv_xtrg.h"

/**
 * @brief xtrg callback
 *
 * @param[in] dev   device xtrg dev
 * @param[in] cb    xtrg callback
 * @param[in] arg   callback input parameter
 *
 */
void xtrg_set_cb(xtrg_dev_t *dev, xtrg_callback_t cb, void *arg)
{
    ASSERT(dev != NULL);
    dev->cb = cb;
    dev->arg = arg;
}

/**
 * @brief xtrg expiry
 *
 * @param[in] dev device xtrg dev
 *
 */
void xtrg_expiry(xtrg_dev_t *dev, sdrv_xtrg_int_event_e event)
{
    ASSERT(dev != NULL);
    if (dev->cb) {
        dev->cb(dev->arg, event);
    }
}

/**
 * @brief  XTRG irp handler.
 *
 * @param[in] irq  xTRG irq.
 */
int sdrv_xtrg_irq_handler(uint32_t irq, void *arg)
{
    xtrg_dev_t *device = arg;

    uint32_t state = sdrv_xtrg_lld_handle_irq(device->base);
    sdrv_xtrg_lld_clear_irq(device->base, state);

    xtrg_expiry(device, (sdrv_xtrg_int_event_e)__builtin_ctz(state));

    return 0;
}

/**
 * @brief sdrv xtrg int enable.
 *
 * @param[in] device xtrg dev
 * @param[in] int_id int id
 */
void sdrv_xtrg_int_enable(xtrg_dev_t *device, uint32_t int_id)
{
    sdrv_xtrg_lld_int_enable(device->base, int_id);
}

/**
 * @brief sdrv xtrg int disable.
 *
 * @param[in] xtrg   xtrg dev
 * @param[in] int_id int id
 */
void sdrv_xtrg_int_disable(xtrg_dev_t *device, uint32_t int_id)
{
    sdrv_xtrg_lld_int_disable(device->base, int_id);
}

/**
 * @brief Init XTRG.
 *
 * @param[in] device   xTRG device.
 */
void sdrv_xtrg_init(xtrg_dev_t *device)
{
    if (device->irq >= 0) {
        irq_attach(device->irq, sdrv_xtrg_irq_handler, device);
        irq_enable(device->irq);
    }
}

/**
 * @brief Deinit XTRG.
 *
 * @param[in] device   xTRG device.
 */
void sdrv_xtrg_deinit(xtrg_dev_t *device)
{
    if (device->irq >= 0) {
        irq_disable(device->irq);
        irq_detach(device->irq);
    }
}

/**
 * @brief Enable or disable input signal synchronization.
 *
 * SIG_i, TRG_i, SYNC_i and IO_i signals are synchronized to timer clock domain
 * by default. This function enable or disable signal synchronizations.
 *
 * @param[in] device   xTRG device.
 * @param[in] cfg      xTRG configuration.
 * @param[in] enable   Enable or disable synchronization.
 */
void sdrv_xtrg_sync_enable(xtrg_dev_t *device, const xtrg_config_t *cfg,
                           bool enable)
{
    sdrv_xtrg_lld_sync_enable(device->base, cfg->sig, enable);
}

/**
 * @brief Configure I/O filters.
 *
 * @param[in] base          xTRG base.
 * @param[in] cfg           The input signal to enable or disable debouncing
 * @param[in] io_filter_cfg I/O filter configuration.
 */
void sdrv_xtrg_io_filter_config(xtrg_dev_t *device, const xtrg_config_t *cfg,
                                const xtrg_io_filter_config_t *io_filter_cfg,
                                bool enable)
{
    sdrv_xtrg_lld_io_filter_config(device->base, cfg->sig, io_filter_cfg,
                                   enable);
}

/**
 * @brief Config SWT (software trigger).
 *
 * @param[in] device     xTRG device.
 * @param[in] swt_cfg    swt configuration.
 */
void sdrv_xtrg_swt_config(xtrg_dev_t *device, const xtrg_swt_config_t *swt_cfg)
{
    sdrv_xtrg_lld_swt_set_pulse_width(device->base, swt_cfg->swt,
                                      swt_cfg->width);
    sdrv_xtrg_lld_swt_trigger(device->base, swt_cfg->swt, swt_cfg->event);
}

/**
 * @brief Enable or disable SSE5.
 *
 * @param[in] device        xTRG device.
 * @param[in] sse5_cfg      xTRG sse configuration.
 * @param[in] enable        Enable or disable sse5.
 */
void sdrv_xtrg_sse5_enable(xtrg_dev_t *device,
                           const xtrg_sse5_config_t *sse5_cfg, bool enable)
{
    sdrv_xtrg_lld_sse5_enable(device->base, enable, sse5_cfg);
}

/**
 * @brief Configure signal mux, muxing input signals to SIG_o or IO_o.
 *
 * The smux module muxes SSIG, TRG_i, SIG_i, and I/O inputs to SIG_o
 * and I/O outputs.
 *
 * @param[in] device    xTRG device.
 * @param[in] cfg       xTRG configuration.
 */
void sdrv_xtrg_smux_config(xtrg_dev_t *device, const xtrg_config_t *cfg)
{
    sdrv_xtrg_lld_smux_config(device->base, cfg->sig_i, cfg->sig_o);
}

/**
 * @brief Configure IO output, muxing SSIG_i,TRG_i,SIG_i,IO_i to IO_o.
 *
 * @param[in] device    xTRG device.
 * @param[in] cfg       xTRG configuration.
 * @param[in] enable    Enable or disable io out.
 */
void sdrv_xtrg_io_out_config(xtrg_dev_t *device, const xtrg_config_t *cfg,
                             bool enable)
{
    sdrv_xtrg_lld_io_out_config(device->base, cfg->sig_i, cfg->sig_o);
    sdrv_xtrg_lld_io_out_enable(device->base, cfg->sig_o, enable);
}

/**
 * @brief Configure syncmux, muxing input sync signals to SYNC_o.
 *
 * The syncmux module muxes SWT_i, SSIG and SYNC_i inputs to SYNC_o outputs.
 *
 * @param[in] device    xTRG device.
 * @param[in] cfg       xTRG configuration.
 */
void sdrv_xtrg_syncmux_config(xtrg_dev_t *device, const xtrg_config_t *cfg)
{
    sdrv_xtrg_lld_syncmux_config(device->base, cfg->sig_i, cfg->sig_o);
}

/**
 * @brief Configure syncmux, muxing input sync signals to SYNC_o.
 *
 * The syncmux module muxes SWT_i, SSIG and SYNC_i inputs to SYNC_o outputs.
 *
 * @param[in] device         xTRG device.
 * @param[in] tmux_drt_cfg   xTRG tmux direct configuration.
 */
void sdrv_xtrg_tmux0_drt_config(xtrg_dev_t *device,
                                const xtrg_tmux_drt_config_t *tmux_drt_cfg)
{
    sdrv_xtrg_lld_tmux0_drt_config(device->base, tmux_drt_cfg);
}

/**
 * @brief Configure the Trigger Mux (tmux) indrt modules..
 *
 * The tmux muxes TRG_i, SSIG signals to ADC and ACMP modules, with specified
 * TID. There are 8 "drt" decoders, whose TID comes from TID_i, and 8
 * "indrt" decoders, whose TID comes from xTRG internel TID pool.
 *
 * @param[in] base        xTRG base.
 * @param[in] tmux_indrt  xTRG tmux indirect configuration.
 */
void sdrv_xtrg_tmux0_indrt_config(
    xtrg_dev_t *device, const xtrg_tmux_indrt_config_t *tmux_indrt_cfg)
{
    sdrv_xtrg_lld_tmux0_indrt_config(device->base, tmux_indrt_cfg);
}

/**
 * @brief Configure SMON (Signal Monitor).
 * The SMON monitors the input and output signal groups
 * First level select 32 input signals and 32 output signals.
 * Second level select inputs from output of 1st level
 *
 * @param[in] device     xTRG device.
 * @param[in] smon_cfg   smon configuration.
 * @param[in] enable     Enable or disable smon.
 */
void sdrv_xtrg_smon_config(xtrg_dev_t *device,
                           const xtrg_smon_config_t *smon_cfg, bool enable)
{
    sdrv_xtrg_lld_smon_enable(device->base, enable, smon_cfg);
}

/**
 * @brief Configure SWDT Pulse Check.(Signal Watch Dog Timer).
 *
 * @param[in] device     xTRG device.
 * @param[in] swdt_cfg   swdt configuration.
 * @param[in] enable     Enable or disable swdt.
 */
void sdrv_xtrg_swdt_config(xtrg_dev_t *device,
                           const xtrg_swdt_config_t *swdt_cfg, bool enable)
{
    switch (swdt_cfg->xtrg_swdt_check_mode) {
    case XTRG_SWDT_PULSE_CHECK:
        sdrv_xtrg_lld_swdt_pulse_check_enable(
            device->base, enable, &swdt_cfg->xtrg_swdt_pulse_check_config);
        break;
    case XTRG_SWDT_EVENT_CHECK:
        sdrv_xtrg_lld_swdt_event_check_enable(
            device->base, enable, &swdt_cfg->xtrg_swdt_event_check_config);
        break;
    default:
        break;
    }
}

/**
 * @brief Enable or disable DMA0/1.
 *
 * @param[in] device   xTRG device.
 * @param[in] config   xTRG dma configuration.
 * @param[in] enable   Enable or disable dma.
 */
void sdrv_xtrg_dma_enable(xtrg_dev_t *device, const xtrg_dma_config_t *config,
                          bool enable)
{
    sdrv_xtrg_lld_dma_enable(device->base, config, enable);
}

/**
 * @brief Read tmux adc result.
 *
 * @param[in] device   xTRG device.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] adc_id   xTRG adc index.
 */
uint32_t sdrv_xtrg_adc_rslt(xtrg_dev_t *device, xtrg_tmux_sel_e tmux_id,
                            xtrg_adc_sel_e adc_id)
{
    return ((sdrv_xtrg_lld_adc_rslt(device->base, tmux_id, adc_id) & 0xFFF00) >>
            8);
}

/**
 * @brief Read tmux acmp result.
 *
 * @param[in] device   xTRG device.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] acmp_id  xTRG acmp index.
 */
uint32_t sdrv_xtrg_acmp_rslt(xtrg_dev_t *device, xtrg_tmux_sel_e tmux_id,
                             xtrg_acmp_sel_e acmp_id)
{
    return sdrv_xtrg_lld_acmp_rslt(device->base, tmux_id, acmp_id);
}
