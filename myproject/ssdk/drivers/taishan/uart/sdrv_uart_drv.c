/**
 * @file sdrv_uart_drv.c
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
#include <types.h>
#include <string.h>

#include "driver/irq.h"
#include "driver/taishan/uart/sdrv_uart_drv.h"
#include "sdrv_uart.h"

#define EXTRA_FMT   "sdrv_uart:"
#define SDRV_UART_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_UART_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_UART_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

#define SDRV_UART_STATE_NULL            0
#define SDRV_UART_STATE_ABR             1
#define SDRV_UART_STATE_READY           2

#define SDRV_UART_ABR_PATTERN           0
#define SDRV_UART_ABR_PATTERN_CHAR      0x55
#define SDRV_UART_MAX_ABR_NUM           8

static void sdrv_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg);
static void sdrv_uart_init(uart_dev_t *device);
static void sdrv_uart_deinit(uart_dev_t *device);
static void sdrv_uart_set_baudrate(uart_dev_t *device, uint32_t baud);
static void sdrv_uart_putc(uart_dev_t *device, uint8_t c);
static uint8_t sdrv_uart_getc(uart_dev_t *device);
static void sdrv_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size);
static void sdrv_uart_receive(uart_dev_t *device, char *data, size_t size);
static size_t sdrv_uart_transmit_async(uart_dev_t *device, const char *data,
                                     size_t size);
static size_t sdrv_uart_receive_async(uart_dev_t *device, char *data,
                                    size_t size);
static void sdrv_uart_enable_txint(uart_dev_t *device, bool enable);
static void sdrv_uart_enable_rxint(uart_dev_t *device, bool enable);
static void sdrv_uart_enable_rxfovf(uart_dev_t *device, bool enable);
static void sdrv_uart_abr_pass(uart_dev_t *device);
static void sdrv_uart_abr_fail(uart_dev_t *device);

static uart_ops_t g_sdrv_uart_ops = {
    .earlyinit = sdrv_uart_earlyinit,
    .init = sdrv_uart_init,
    .deinit = sdrv_uart_deinit,
    .set_baudrate = sdrv_uart_set_baudrate,
    .putc = sdrv_uart_putc,
    .getc = sdrv_uart_getc,
    .transmit = sdrv_uart_transmit,
    .receive = sdrv_uart_receive,
    .transmit_async = sdrv_uart_transmit_async,
    .receive_async = sdrv_uart_receive_async,
    .enable_txint = sdrv_uart_enable_txint,
    .enable_rxint = sdrv_uart_enable_rxint,
    .enable_rxfovf = sdrv_uart_enable_rxfovf,
};

static int sdrv_uart_irq_handler(uint32_t irq, void *arg)
{
    uart_dev_t *device = (uart_dev_t *)arg;
    sdrv_uart_dev_t *sdrv_uart = (sdrv_uart_dev_t *)device->priv;
    uint32_t state;

    state = sdrv_uart_lld_handle_irq(device->base);

    /* Dispatch an appropriate handler. */
    if (sdrv_uart->state == SDRV_UART_STATE_ABR) {
        if ((state & SDRV_UART_INTR_ABRPASS) != 0) {
            sdrv_uart_abr_pass(device);
        }
        else if ((state & SDRV_UART_INTR_ABRFAIL) != 0) {
            sdrv_uart_abr_fail(device);
        }
    }
    else if (sdrv_uart->state == SDRV_UART_STATE_READY) {
        if((state & SDRV_UART_INTR_RX_FWF) != 0) {
            /* Received data interrupt */
            uart_receive_chars(device);
        }

        if((state & SDRV_UART_INTR_RX_FOVF) != 0) {
            /* Received fifo overflow interrupt */
            uart_receive_fovf(device);
        }

        if((state & SDRV_UART_INTR_TX_CMP) != 0) {
            /* Transmit data interrupt */
            uart_xmit_chars(device);
        }
    }

    sdrv_uart_lld_clear_irq(device->base, state);

    return 0;
}

/**
 * @brief uart abr ack.
 */
static void sdrv_uart_abr_pass(uart_dev_t *device)
{
    sdrv_uart_dev_t *sdrv_uart = (sdrv_uart_dev_t *)device->priv;

    device->baud = device->clk_freq / sdrv_uart_lld_get_baudrate(device->base);

    SDRV_UART_INFO("ABR PASS, baud:%d\n", device->baud);

    sdrv_uart_lld_stop_abr(device->base);

    sdrv_uart_lld_putc(device->base, SDRV_UART_ABR_PATTERN_CHAR);
    sdrv_uart->state = SDRV_UART_STATE_READY;
}

/**
 * @brief uart abr fail.
 */
static void sdrv_uart_abr_fail(uart_dev_t *device)
{
    sdrv_uart_dev_t *sdrv_uart = (sdrv_uart_dev_t *)device->priv;

    SDRV_UART_INFO("ABR FAIL\n");

    sdrv_uart_lld_stop_abr(device->base);

    sdrv_uart->abr_cnt++;
    if (sdrv_uart->abr_cnt >= SDRV_UART_MAX_ABR_NUM) {
        SDRV_UART_WARN("ABR FAIL Exceeded Maximum Attempts\n");
    }
    else {
        sdrv_uart_lld_start_abr(device->base);
    }
}

/**
 * @brief enable tx int.
 */
static void sdrv_uart_enable_txint(uart_dev_t *device, bool enable)
{
    sdrv_uart_lld_enable_txint(device->base, enable);
}

/**
 * @brief enable rx int.
 */
static void sdrv_uart_enable_rxint(uart_dev_t *device, bool enable)
{
    sdrv_uart_lld_enable_rxint(device->base, enable);
}

/**
 * @brief enable rx fifo overflow int.
 */
static void sdrv_uart_enable_rxfovf(uart_dev_t *device, bool enable)
{
    sdrv_uart_lld_enable_rxfovf(device->base, enable);
}

/**
 * @brief Asynchronous UART Rx.
 */
static size_t sdrv_uart_receive_async(uart_dev_t *device, char *data,
                                      size_t size)
{
    return sdrv_uart_lld_receive_async(device->base, data, size);
}

/**
 * @brief Asynchronous UART Tx.
 */
static size_t sdrv_uart_transmit_async(uart_dev_t *device, const char *data,
                                       size_t size)
{
    return sdrv_uart_lld_transmit_async(device->base, data, size);
}

/**
 * @brief Configure baud rate.
 */
static void sdrv_uart_set_baudrate(uart_dev_t *device, uint32_t baud)
{
    sdrv_uart_lld_set_baudrate(device->base, baud, device->clk_freq);
}

/**
 * @brief Synchronous UART Rx.
 */
static void sdrv_uart_receive(uart_dev_t *device, char *data, size_t size)
{
    sdrv_uart_lld_receive(device->base, data, size);
}

/**
 * @brief Synchronous UART Tx.
 */
static void sdrv_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size)
{
    sdrv_uart_lld_transmit(device->base, data, size);
}

static uint8_t sdrv_uart_getc(uart_dev_t *device)
{
    return sdrv_uart_lld_getc(device->base);
}

static void sdrv_uart_putc(uart_dev_t *device, uint8_t c)
{
    sdrv_uart_lld_putc(device->base, c);
}

/**
 * @brief Early UART hardware initialization.
 */
static void sdrv_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg)
{
    sdrv_uart_dev_t *sdrv_uart = (sdrv_uart_dev_t *)device->priv;
    sdrv_uart_config_t sdrv_uart_cfg;

    memset(&sdrv_uart_cfg, 0x00, sizeof(sdrv_uart_config_t));
    sdrv_uart_cfg.op_mode = SDRV_UART_SERIAL_MODE;
    sdrv_uart_cfg.clk_freq = cfg->clk_freq;
    sdrv_uart_cfg.data_bits = cfg->data_bits;
    sdrv_uart_cfg.parity = cfg->parity;
    sdrv_uart_cfg.stop_bits = cfg->stop_bits;
    sdrv_uart_cfg.fcm.fcm_type = SDRV_UART_NO_FC;
    sdrv_uart_cfg.transfer_mode = SDRV_UART_FULL_DUPLEX;
    sdrv_uart_cfg.dma_en = cfg->dma_en;
    sdrv_uart_cfg.txwmlvl = cfg->txwmlvl;
    sdrv_uart_cfg.rxmwlvl = cfg->rxmwlvl;

    sdrv_uart_cfg.baud = cfg->baud;
    if (cfg->baud == UART_AUTO_BAUD) {
        sdrv_uart_cfg.abr.abr_en = 1;
        sdrv_uart_cfg.abr.pattern = SDRV_UART_ABR_PATTERN;
    }

    if (!cfg->hw_inited) {
        sdrv_uart_lld_init(device->base, &sdrv_uart_cfg);
    }

    if (cfg->baud == UART_AUTO_BAUD) {
        sdrv_uart->abr_cnt = 0;
        sdrv_uart->state = SDRV_UART_STATE_ABR;
    }
    else {
        sdrv_uart->state = SDRV_UART_STATE_READY;
    }
}

/**
 * @brief Initialize UART.
 */
static void sdrv_uart_init(uart_dev_t *device)
{
    /*
     * Request and enable IRQ. SOC level UART IRQ is always enabled, but LLD
     * shall enable or disable controller interrupt sources according to
     * current transaction type (sync or async).
     */
    if (device->irq >= 0) {
        irq_attach(device->irq, sdrv_uart_irq_handler, device);
        irq_enable(device->irq);
    }
}

static void sdrv_uart_deinit(uart_dev_t *device)
{
    if (device->irq >= 0) {
        irq_disable(device->irq);
        irq_detach(device->irq);
    }
}

/**
 * @brief sdrv uart probe
 *
 * @param[in] sdrv_uart sdrv uart dev
 * @param[in] cfg uart config
 * @return uart dev
 */
uart_dev_t *sdrv_uart_probe(sdrv_uart_dev_t *sdrv_uart, const uart_config_t *cfg)
{
    uart_dev_t *dev = &sdrv_uart->dev;

    dev->ops = &g_sdrv_uart_ops;
    dev->priv = sdrv_uart;

    uart_earlyinit(dev, cfg);

    return dev;
}
