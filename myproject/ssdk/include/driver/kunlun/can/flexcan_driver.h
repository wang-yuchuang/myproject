/**
 * @file flexcan_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: flexcan dev interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_FLEXCAN_H
#define INCLUDE_DRV_FLEXCAN_H

#include <types.h>

#include "driver/can.h"
#include "kunlun/can/flexcan_lld.h"

#if CONFIG_SDRV_FLEXCAN_ENH
#define MAX_FLEXCAN_CH 24
#else
#define MAX_FLEXCAN_CH 4
#endif

/* Abnormal interrupt configuration. */
#define EN_ERR_INT 0
#define EN_WARNING_INT 0
#define EN_WAKE_UP_INT 1

typedef struct rx_mb_cfg {
    uint32_t frameId;
    uint32_t rxIDFilterMask;
    bool isRemoteFrame;
    bool isExtendedFrame;
} rx_mb_cfg_t;

typedef struct tx_mb_cfg {
    uint8_t paddingVal;
} tx_mb_cfg_t;

typedef struct flexcan_mb_config {
    /* if fifo enabled, MB[0-5] for FIFO.
     * mb_for_fifo = RFFN / 8 * 2 + 6
     * maxMBNum is not allowed to be less than
     * (mb_for_fifo + rx_mb_count + tx_mb_count)
     */
    uint8_t maxMBNum;
    uint8_t rx_mb_count;
    uint8_t tx_mb_count;

    /* config mailboxes for fifo/rx/tx */
    flexcan_rx_fifo_config_t *fifo_cfg;
    rx_mb_cfg_t *rx_mb_cfg;
    tx_mb_cfg_t *tx_mb_cfg;
} flexcan_mb_config_t;

typedef struct flexcan_controller_cfg {
    uint32_t channel;
    addr_t base;
    uint32_t irq;

    /* NOTE: is_rx_fifo_needed and is_canfd_enabled should not be
     * enabled simultaneously.
     */
    bool is_rx_fifo_needed;
    bool is_canfd_enabled;

    bool enableSelfWakeup;
    bool enableIndividMask;
    bool is_loopback_enabled;

    flexcan_timing_config_t nominal_bit_timing;
    /* when fd disabled, data_bit_timing points to NULL */
    flexcan_timing_config_t data_bit_timing;
    /* It's not recommended to set fd_tdc zero if BRS enabled */
    uint8_t fd_tdc;

    /* config can framework sw buffer */
    struct can_buf_cfg buf_cfg;
} flexcan_controller_cfg_t;

/**
 * @brief flexcan probe.
 *
 * @param[in] dev can dev.
 * @param[in] config can config for flexcan controller && framework.
 * @param[in] flexcan_mb_cfg
 */
void flexcan_probe(const flexcan_controller_cfg_t *config,
                    flexcan_mb_config_t *flexcan_mb_cfg);
#endif