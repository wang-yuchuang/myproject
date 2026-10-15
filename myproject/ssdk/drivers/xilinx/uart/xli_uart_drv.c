/**
 * @file xli_uart_drv.c
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

#include "driver/irq.h"
#include "driver/xilinx/uart/xli_uart_drv.h"
#include "xli_uart.h"

static void xli_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg);
static void xli_uart_init(uart_dev_t *device);
static void xli_uart_deinit(uart_dev_t *device);
static void xli_uart_set_baudrate(uart_dev_t *device, uint32_t baud);
static void xli_uart_putc(uart_dev_t *device, uint8_t c);
static uint8_t xli_uart_getc(uart_dev_t *device);
static void xli_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size);
static void xli_uart_receive(uart_dev_t *device, char *data, size_t size);
static size_t xli_uart_transmit_async(uart_dev_t *device, const char *data,
                                     size_t size);
static size_t xli_uart_receive_async(uart_dev_t *device, char *data,
                                    size_t size);
static void xli_uart_enable_txint(uart_dev_t *device, bool enable);
static void xli_uart_enable_rxint(uart_dev_t *device, bool enable);

static uart_ops_t g_xli_uart_ops = {
    .earlyinit = xli_uart_earlyinit,
    .init = xli_uart_init,
    .deinit = xli_uart_deinit,
    .set_baudrate = xli_uart_set_baudrate,
    .putc = xli_uart_putc,
    .getc = xli_uart_getc,
    .transmit = xli_uart_transmit,
    .receive = xli_uart_receive,
    .transmit_async = xli_uart_transmit_async,
    .receive_async = xli_uart_receive_async,
    .enable_txint = xli_uart_enable_txint,
    .enable_rxint = xli_uart_enable_rxint,
};

static int xli_uart_irq_handler(uint32_t irq, void *arg)
{
    uart_dev_t *device = arg;
    uint32_t state;

    state = xli_uart_lld_handle_irq(device->base);

    xli_uart_lld_clear_irq(device->base, state);

    /* Dispatch an appropriate handler. */
    if((state & (XLI_UART_IXR_RXFULL |
        XLI_UART_IXR_RXEMPTY | XLI_UART_IXR_RXOVR)) != 0) {
        /* Received data interrupt */
        uart_receive_chars(device);
    }

    if((state & XLI_UART_IXR_TXEMPTY) != 0) {
        /* Transmit data interrupt */
        uart_xmit_chars(device);
    }

    return 0;
}

/**
 * @brief enable tx int.
 */
static void xli_uart_enable_txint(uart_dev_t *device, bool enable)
{
    xli_uart_lld_enable_txint(device->base, enable);
}

/**
 * @brief enable rx int.
 */
static void xli_uart_enable_rxint(uart_dev_t *device, bool enable)
{
    xli_uart_lld_enable_rxint(device->base, enable);
}

/**
 * @brief Asynchronous UART Rx.
 */
static size_t xli_uart_receive_async(uart_dev_t *device, char *data,
                                    size_t size)
{
    return xli_uart_lld_receive_async(device->base, data, size);
}

/**
 * @brief Asynchronous UART Tx.
 */
static size_t xli_uart_transmit_async(uart_dev_t *device, const char *data,
                                     size_t size)
{
    return xli_uart_lld_transmit_async(device->base, data, size);
}

/**
 * @brief Configure baud rate.
 */
static void xli_uart_set_baudrate(uart_dev_t *device, uint32_t baud)
{
    xli_uart_lld_set_baudrate(device->base, baud, device->clk_freq);
}

/**
 * @brief Synchronous UART Rx.
 */
static void xli_uart_receive(uart_dev_t *device, char *data, size_t size)
{
    xli_uart_lld_receive(device->base, data, size);
}

/**
 * @brief Synchronous UART Tx.
 */
static void xli_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size)
{
    xli_uart_lld_transmit(device->base, data, size);
}

static uint8_t xli_uart_getc(uart_dev_t *device)
{
    return xli_uart_lld_getc(device->base);
}

static void xli_uart_putc(uart_dev_t *device, uint8_t c)
{
    xli_uart_lld_putc(device->base, c);
}

/**
 * @brief Early UART hardware initialization.
 */
static void xli_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg)
{
    xli_uart_config_t xli_uart_cfg;

    if (!cfg->hw_inited) {
        xli_uart_cfg.baud = cfg->baud;
        xli_uart_cfg.clk_freq = cfg->clk_freq;
        xli_uart_cfg.data_bits = cfg->data_bits;
        xli_uart_cfg.parity = cfg->parity;
        xli_uart_cfg.stop_bits = cfg->stop_bits;
        xli_uart_lld_init(device->base, &xli_uart_cfg);
    }
}

/**
 * @brief Initialize UART.
 */
static void xli_uart_init(uart_dev_t *device)
{
    /*
     * Request and enable IRQ. SOC level UART IRQ is always enabled, but LLD
     * shall enable or disable controller interrupt sources according to
     * current transaction type (sync or async).
     */
    if (device->irq >= 0) {
        irq_attach(device->irq, xli_uart_irq_handler, device);
        irq_enable(device->irq);
    }
}

static void xli_uart_deinit(uart_dev_t *device)
{
    if (device->irq >= 0) {
        irq_disable(device->irq);
        irq_detach(device->irq);
    }
}

/**
 * @brief xli uart probe
 *
 * @param[in] xli_uart xli uart dev
 * @param[in] cfg uart config
 * @return dev uart dev
 */
uart_dev_t *xli_uart_probe(xli_uart_dev_t *xli_uart, const uart_config_t *cfg)
{
    uart_dev_t *dev = &xli_uart->dev;

    dev->ops = &g_xli_uart_ops;
    dev->priv = xli_uart;

    uart_earlyinit(dev, cfg);

    return dev;
}
