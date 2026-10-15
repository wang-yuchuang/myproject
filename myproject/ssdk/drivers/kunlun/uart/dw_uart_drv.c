/**
 * @file dw_uart_drv.c
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
#include "driver/kunlun/uart/dw_uart_drv.h"
#include "dw_uart.h"
#include <sd_errcode.h>

static void dw_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg);
static void dw_uart_init(uart_dev_t *device);
static void dw_uart_deinit(uart_dev_t *device);
static void dw_uart_set_baudrate(uart_dev_t *device, uint32_t baud);
static void dw_uart_putc(uart_dev_t *device, uint8_t c);
static void dw_uart_raw_putc(uart_dev_t *device, uint8_t c);
static uint8_t dw_uart_getc(uart_dev_t *device);
static void dw_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size);
static void dw_uart_raw_transmit(uart_dev_t *device, const char *data,
                             size_t size);
static void dw_uart_receive(uart_dev_t *device, char *data, size_t size);
static size_t dw_uart_transmit_async(uart_dev_t *device, const char *data,
                                     size_t size);
static size_t dw_uart_receive_async(uart_dev_t *device, char *data,
                                    size_t size);
static void dw_uart_enable_txint(uart_dev_t *device, bool enable);
static void dw_uart_enable_rxint(uart_dev_t *device, bool enable);

uart_ops_t g_dw_uart_ops = {
    .earlyinit = dw_uart_earlyinit,
    .init = dw_uart_init,
    .deinit = dw_uart_deinit,
    .set_baudrate = dw_uart_set_baudrate,
    .putc = dw_uart_putc,
    .raw_putc = dw_uart_raw_putc,
    .getc = dw_uart_getc,
    .transmit = dw_uart_transmit,
    .raw_transmit = dw_uart_raw_transmit,
    .receive = dw_uart_receive,
    .transmit_async = dw_uart_transmit_async,
    .receive_async = dw_uart_receive_async,
    .enable_txint = dw_uart_enable_txint,
    .enable_rxint = dw_uart_enable_rxint,
};

static int dw_uart_irq_handler(uint32_t irq, void *arg)
{
    uart_dev_t *device = arg;
    uint32_t state;

    state = dw_uart_lld_handle_irq(device->base);
    switch (state) {
        case IID_TX_EMPTY:
            uart_xmit_chars(device);
            break;

        case IID_RX_AVAIL:
            uart_receive_chars(device);
            break;
        case IID_RX_TMO:
            dw_uart_lld_get_rbr(device->base);
            break;
        default:
            break;
    }

    return 0;
}

/**
 * @brief enable tx int.
 */
static void dw_uart_enable_txint(uart_dev_t *device, bool enable)
{
    dw_uart_lld_enable_txint(device->base, enable);
}

/**
 * @brief enable rx int.
 */
static void dw_uart_enable_rxint(uart_dev_t *device, bool enable)
{
    dw_uart_lld_enable_rxint(device->base, enable);
}

/**
 * @brief Asynchronous UART Rx.
 */
static size_t dw_uart_receive_async(uart_dev_t *device, char *data,
                                    size_t size)
{
    return dw_uart_lld_receive_async(device->base, data, size);
}

/**
 * @brief Asynchronous UART Tx.
 */
static size_t dw_uart_transmit_async(uart_dev_t *device, const char *data,
                                     size_t size)
{
    return dw_uart_lld_transmit_async(device->base, data, size);
}

/**
 * @brief Configure baud rate.
 */
static void dw_uart_set_baudrate(uart_dev_t *device, uint32_t baud)
{
    dw_uart_lld_set_baudrate(device->base, baud, device->clk_freq);
}

/**
 * @brief Synchronous UART Rx.
 */
static void dw_uart_receive(uart_dev_t *device, char *data, size_t size)
{
    dw_uart_lld_receive(device->base, data, size);
}

/**
 * @brief Synchronous UART Tx.
 */
static void dw_uart_transmit(uart_dev_t *device, const char *data,
                             size_t size)
{
    dw_uart_lld_transmit(device->base, data, size);
}

static void dw_uart_raw_transmit(uart_dev_t *device, const char *data,
                             size_t size)
{
    dw_uart_lld_raw_transmit(device->base, data, size);
}

static uint8_t dw_uart_getc(uart_dev_t *device)
{
    return dw_uart_lld_getc(device->base);
}

static void dw_uart_putc(uart_dev_t *device, uint8_t c)
{
    dw_uart_lld_putc(device->base, c);
}

static void dw_uart_raw_putc(uart_dev_t *device, uint8_t c)
{
    dw_uart_lld_raw_putc(device->base, c);
}
/**
 * @brief Early UART hardware initialization.
 */
static void dw_uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg)
{
    dw_uart_config_t dw_uart_cfg;
    dw_uart_dev_t *dw_uart = device->priv;

    if (!cfg->hw_inited) {
        dw_uart_cfg.baud = cfg->baud;
        dw_uart_cfg.clk_freq = cfg->clk_freq;
        dw_uart_cfg.data_bits = cfg->data_bits;
        dw_uart_cfg.parity = cfg->parity;
        dw_uart_cfg.stop_bits = cfg->stop_bits;
        dw_uart_lld_init(device->base, &dw_uart_cfg);
        dw_uart_lld_enable_fifo(device->base, &dw_uart->fcr, true);
    }
}

/**
 * @brief Initialize UART.
 */
static void dw_uart_init(uart_dev_t *device)
{
    /*
     * Request and enable IRQ. SOC level UART IRQ is always enabled, but LLD
     * shall enable or disable controller interrupt sources according to
     * current transaction type (sync or async).
     */
    if (device->irq >= 0) {
        irq_attach(device->irq, dw_uart_irq_handler, device);
        irq_enable(device->irq);
    }
}

static void dw_uart_deinit(uart_dev_t *device)
{
    if (device->irq >= 0) {
        irq_disable(device->irq);
        irq_detach(device->irq);
    }
}

/**
 * @brief dw uart probe
 *
 * @param[in] dw_uart dw uart dev
 * @param[in] cfg uart config
 * @return dev uart dev
 */
uart_dev_t *dw_uart_probe(dw_uart_dev_t *dw_uart, const uart_config_t *cfg)
{
    if(NULL == cfg)
    {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_CONFIGERR, 0U);
        return NULL;
    }
    if(NULL == dw_uart) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, cfg->base);
        return NULL;
    }

    uart_dev_t *dev = &dw_uart->dev;

    dev->ops = &g_dw_uart_ops;
    dev->priv = dw_uart;

    uart_earlyinit(dev, cfg);

    return dev;
}
