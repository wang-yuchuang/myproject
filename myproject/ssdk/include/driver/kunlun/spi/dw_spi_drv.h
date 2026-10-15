/**
 * @file dw_spi_drv.h
 * @brief dw spi driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _DW_SPI_DRV_H_
#define _DW_SPI_DRV_H_

#include <types.h>
#include <driver/spi.h>
#include <driver/scr.h>
#include <driver/clk.h>
#include <driver/dma.h>
#include <driver/gpio.h>

struct dw_spi_controller {
    struct spi_controller *ctrl;
    struct spi_msg *cur_msg;
    osEventFlagsId_t event;
    scr_signal_t scr_sig;
    scr_dev_t *scr_dev;
    clk_node_t *clk;
    uint32_t sclk_hz;
    gpio_dev_t *gpio_dev;
    dma_dev_t **dma_dev;
    dma_chan_t *spi_tx_ch;
    dma_chan_t *spi_rx_ch;
    dma_status_e dma_tx_stat;
    dma_status_e dma_rx_stat;
    void *tx_buf;
    void *rx_buf;
    uint32_t tx_len;
    uint32_t rx_len;
    uint32_t buf_len;
    uint32_t fifo_len;
    uint8_t n_bytes;
    int trans_stat;
};

/**
 * @brief dw spi probe
 *
 * @param[in] ctrl/spi controller
 * @param[in] dw_ctrl/dw spi controller
 * @param[in] cfg/spi controller config
 */
int dw_spi_controller_probe(struct spi_controller *ctrl,
                            struct dw_spi_controller *dw_ctrl, const struct spi_controller_config *cfg);

#endif
