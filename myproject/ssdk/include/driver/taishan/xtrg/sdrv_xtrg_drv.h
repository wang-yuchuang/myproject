/**
 * @file ssdrv_xtrg_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv xtrg driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_DRV_XTRG_H
#define INCLUDE_SDRV_DRV_XTRG_H

#include <cmsis_os2.h>
#include <compiler.h>
#include <sdrv_xtrg.h>
#include <types.h>

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief xTRG interrupt event.
 */
typedef enum sdrv_xtrg_int_event {
    FAULT_EVENT = 0,
    TMUX0_DONE_EVENT = 16,
    TMUX1_DONE_EVENT,
    TMUX2_DONE_EVENT,
    TMUX3_DONE_EVENT,
    TMUX4_DONE_EVENT,
    TMUX5_DONE_EVENT,
    TMUX6_DONE_EVENT,
    TMUX7_DONE_EVENT,
} sdrv_xtrg_int_event_e;

typedef void (*xtrg_callback_t)(void *arg, sdrv_xtrg_int_event_e event);

/**
 * @brief XTRG configurations.
 */
typedef struct xtrg_config {
    uint32_t sig;
    uint32_t sig_i;
    uint32_t sig_o;
} xtrg_config_t;

/**
 * @brief XTRG low level device.
 */
typedef struct xtrg_dev {
    paddr_t base;
    int irq;
    xtrg_callback_t cb;
    void *arg;
} xtrg_dev_t;

/**
 * @brief  XTRG irp handler.
 *
 * @param[in] irq  xTRG irq.
 */
int sdrv_xtrg_irq_handler(uint32_t irq, void *arg);

/**
 * @brief xtrg callback
 *
 * @param[in] dev   device xtrg dev
 * @param[in] cb    xtrg callback
 * @param[in] arg   callback input parameter
 *
 */
void xtrg_set_cb(xtrg_dev_t *dev, xtrg_callback_t cb, void *arg);

/**
 * @brief xtrg expiry
 *
 * @param[in] dev   device xtrg dev
 * @param[in] event xtrg interrupt enum
 */
void xtrg_expiry(xtrg_dev_t *dev, sdrv_xtrg_int_event_e event);

/**
 * @brief sdrv xtrg init
 *
 * @param[in] device xtrg device
 */
void sdrv_xtrg_init(xtrg_dev_t *device);

/**
 * @brief sdrv xtrg deinit
 *
 * @param[in] device xtrg device
 */
void sdrv_xtrg_deinit(xtrg_dev_t *device);

/**
 * @brief sdrv xtrg sync enable
 *
 * @param[in] device xtrg device
 * @param[in] cfg    xtrg config
 * @param[in] enable sync enable
 */
void sdrv_xtrg_sync_enable(xtrg_dev_t *device, const xtrg_config_t *cfg,
                           bool enable);

/**
 * @brief sdrv xtrg io filter config
 *
 * @param[in] device        xtrg device
 * @param[in] cfg           xtrg config
 * @param[in] io_filter_cfg xtrg io filter config
 */
void sdrv_xtrg_io_filter_config(xtrg_dev_t *device, const xtrg_config_t *cfg,
                                const xtrg_io_filter_config_t *io_filter_cfg,
                                bool enable);

/**
 * @brief sdrv xtrg swt config
 *
 * @param[in] device  xtrg device
 * @param[in] swt_cfg xtrg swt config
 */
void sdrv_xtrg_swt_config(xtrg_dev_t *device, const xtrg_swt_config_t *swt_cfg);

/**
 * @brief sdrv xtrg sse5 config
 *
 * @param[in] device   xtrg device
 * @param[in] sse5_cfg xtrg sse5 config
 * @param[in] enable   sse5 enable
 */
void sdrv_xtrg_sse5_enable(xtrg_dev_t *device,
                           const xtrg_sse5_config_t *sse5_cfg, bool enable);

/**
 * @brief sdrv xtrg smux config
 *
 * @param[in] device xtrg device
 * @param[in] cfg xtrg config
 */
void sdrv_xtrg_smux_config(xtrg_dev_t *device, const xtrg_config_t *cfg);

/**
 * @brief sdrv xtrg io out config
 *
 * @param[in] device xtrg device
 * @param[in] cfg    xtrg config
 * @param[in] enable io out enable
 */
void sdrv_xtrg_io_out_config(xtrg_dev_t *device, const xtrg_config_t *cfg,
                             bool enable);

/**
 * @brief sdrv xtrg syncmux config
 *
 * @param[in] device xtrg device
 * @param[in] cfg    xtrg config
 */
void sdrv_xtrg_syncmux_config(xtrg_dev_t *device, const xtrg_config_t *cfg);

/**
 * @brief sdrv xtrg tmux drt config
 *
 * @param[in] device        xtrg device
 * @param[in] tmux_drt_cfg  xtrg tmux drt config
 */
void sdrv_xtrg_tmux0_drt_config(xtrg_dev_t *device,
                                const xtrg_tmux_drt_config_t *tmux_drt_cfg);

/**
 * @brief sdrv xtrg tmux indrt config
 *
 * @param[in] device        xtrg device
 * @param[in] tmux_indrt_cfg xtrg indrt config
 */
void sdrv_xtrg_tmux0_indrt_config(
    xtrg_dev_t *device, const xtrg_tmux_indrt_config_t *tmux_indrt_cfg);

/**
 * @brief sdrv xtrg smon config
 *
 * @param[in] device xtrg device
 * @param[in] smon_cfg xtrg smon config
 * @param[in] enable smon enable
 */
void sdrv_xtrg_smon_config(xtrg_dev_t *device,
                           const xtrg_smon_config_t *smon_cfg, bool enable);

/**
 * @brief sdrv xtrg swdt config
 *
 * @param[in] device xtrg device
 * @param[in] swdt_cfg xtrg swdt config
 * @param[in] enable swdt enable
 */
void sdrv_xtrg_swdt_config(xtrg_dev_t *device,
                           const xtrg_swdt_config_t *swdt_cfg, bool enable);

/**
 * @brief sdrv xtrg int enable.
 *
 * @param[in] device xtrg device
 * @param[in] int_id int id
 */
void sdrv_xtrg_int_enable(xtrg_dev_t *device, uint32_t int_id);

/**
 * @brief sdrv xtrg int disable.
 *
 * @param[in] device   xtrg device
 * @param[in] int_id int id
 */
void sdrv_xtrg_int_disable(xtrg_dev_t *device, uint32_t int_id);

/**
 * @brief Enable or disable DMA0/1.
 *
 * @param[in] device   xTRG device.
 * @param[in] config   xTRG dma configuration.
 * @param[in] enable   Enable or disable dma.
 */
void sdrv_xtrg_dma_enable(xtrg_dev_t *device, const xtrg_dma_config_t *config,
                          bool enable);

/**
 * @brief Read tmux adc result.
 *
 * @param[in] device   xTRG device.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] adc_id   xTRG adc index.
 */
uint32_t sdrv_xtrg_adc_rslt(xtrg_dev_t *device, xtrg_tmux_sel_e tmux_id,
                            xtrg_adc_sel_e adc_id);

/**
 * @brief Read tmux acmp result.
 *
 * @param[in] device   xTRG device.
 * @param[in] tmux_id  xTRG tmux index.
 * @param[in] acmp_id  xTRG acmp index.
 */
uint32_t sdrv_xtrg_acmp_rslt(xtrg_dev_t *device, xtrg_tmux_sel_e tmux_id,
                             xtrg_acmp_sel_e acmp_id);

__END_CDECLS

#endif

#endif