/**
 * @file uart.h
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
#include "driver/uart.h"
#include <sd_errcode.h>

static inline void uart_lock(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    osMutexAcquire(device->lock, osWaitForever);
}

static inline void uart_unlock(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    osMutexRelease(device->lock);
}

/**
 * @brief UART receive chars, called by irq handler.
 */
void uart_receive_chars(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    size_t received;

    received = device->ops->receive_async(device,
            device->rx_ptr, device->rx_remain);
    if (received > 0) {
        device->rx_remain -= received;
        device->rx_ptr += received;
    }

    if (device->rx_remain == 0) {
        device->ops->enable_rxint(device, false);
        if (device->rx_cb) {
            device->rx_cb(device->rx_arg);
        }
    }
    else {
        device->ops->enable_rxint(device, true);
    }
}

/**
 * @brief UART receive fifo overflow, called by irq handler.
 */
void uart_receive_fovf(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    if(device->rxfovf_cb) {
        device->rxfovf_cb(device->rxfovf_arg);
    }
}

/**
 * @brief UART send chars, called by irq handler.
 */
void uart_xmit_chars(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    size_t transmitted;

    if (device->tx_remain == 0) {
        device->ops->enable_txint(device, false);
        if (device->tx_cb) {
            device->tx_cb(device->tx_arg);
        }
    }
    else {
        transmitted = device->ops->transmit_async(device,
                        device->tx_ptr, device->tx_remain);
        if (transmitted > 0) {
            device->tx_remain -= transmitted;
            device->tx_ptr += transmitted;
            device->ops->enable_txint(device, true);
        }
    }
}

/**
 * @brief Asynchronous UART Rx.
 */
bool uart_receive_async_isr(uart_dev_t *device, char *data,
                        size_t size, uart_cb_t rx_cb)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return false;
    }
    bool ret = false;

    if (size > 0 && device->rx_remain == 0) {
        device->rx_ptr = data;
        device->rx_remain = size;
        device->rx_cb = rx_cb;
        device->rx_arg = data;

        uart_receive_chars(device);

        ret = true;
    }

    return ret;
}

/**
 * @brief Asynchronous UART Rx.
 */
bool uart_receive_async(uart_dev_t *device, char *data,
                        size_t size, uart_cb_t rx_cb)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return false;
    }
    bool ret = false;

    uart_lock(device);

    ret = uart_receive_async_isr(device, data, size, rx_cb);

    uart_unlock(device);

    return ret;
}

/**
 * @brief UART receive fifo overflow enable and set callback func.
 */
void uart_receive_fovf_enable(uart_dev_t *device, bool enable, uart_cb_t rxfovf_cb)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    device->rxfovf_cb = enable ? rxfovf_cb : NULL;
    if(device->ops->enable_rxfovf != NULL){
        device->ops->enable_rxfovf(device, enable);
    }
}

/**
 * @brief Asynchronous UART Tx.
 */
bool uart_transmit_async(uart_dev_t *device, const char *data,
                         size_t size, uart_cb_t tx_cb)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return false;
    }
    bool ret = false;
    size_t transmitted;

    uart_lock(device);

    if (size > 0 && device->tx_remain == 0) {
        device->tx_ptr = data;
        device->tx_remain = size;
        device->tx_cb = tx_cb;

        transmitted = device->ops->transmit_async(device,
                device->tx_ptr, device->tx_remain);
        if (transmitted > 0) {
            device->tx_remain -= transmitted;
            device->tx_ptr += transmitted;
            device->ops->enable_txint(device, true);
        }
        ret = true;
    }

    uart_unlock(device);

    return ret;
}

/**
 * @brief Configure baud rate.
 */
void uart_set_baudrate(uart_dev_t *device, uint32_t baud)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    device->ops->set_baudrate(device, baud);
}

/**
 * @brief Synchronous UART Rx.
 */
void uart_receive(uart_dev_t *device, char *data, size_t size)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    if(NULL == data)
    {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_CONFIGERR, device->base);
        return;
    }

    device->ops->receive(device, data, size);
}

/**
 * @brief Synchronous UART Tx.
 */
void uart_transmit(uart_dev_t *device, const char *data,
                   size_t size)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    if(NULL == data)
    {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_CONFIGERR, device->base);
        return;
    }
    device->ops->transmit(device, data, size);
}

/**
 * @brief Synchronous UART Tx.
 */
void uart_raw_transmit(uart_dev_t *device, const char *data,
                   size_t size)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    if(NULL == data)
    {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_CONFIGERR, device->base);
        return;
    }
    device->ops->raw_transmit(device, data, size);
}

uint8_t uart_getc(uart_dev_t *device)
{
    return device->ops->getc(device);
}

void uart_putc(uart_dev_t *device, uint8_t c)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    device->ops->putc(device, c);
}

void uart_raw_putc(uart_dev_t *device, uint8_t c)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }
    device->ops->raw_putc(device, c);
}

/**
 * @brief Early UART hardware initialization.
 */
void uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg)
{
    if(NULL == cfg)
    {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_CONFIGERR, 0U);
        return;
    }
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, cfg->base);
        return;
    }

    device->base = cfg->base;
    device->irq = cfg->irq;
    device->baud = cfg->baud;
    device->clk_freq = cfg->clk_freq;

    device->tx_cb = NULL;
    device->tx_ptr = NULL;
    device->tx_remain = 0;

    device->rx_cb = NULL;
    device->rx_ptr = NULL;
    device->rx_remain = 0;

    device->rxfovf_cb = NULL;
    device->rxfovf_arg = NULL;

    device->ops->earlyinit(device, cfg);
}

/**
 * @brief Initialize UART.
 */
void uart_init(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }

    device->lock = osMutexNew(NULL);
    ASSERT(device->lock);

    device->ops->init(device);
}

/**
 * @brief Deinit UART.
 */
void uart_deinit(uart_dev_t *device)
{
    if(NULL == device) {
        ERRCODE_PRINT(MOD_ID_UART, 0xFF, UART_ERR_NOINSTANCE, 0U);
        return;
    }

    device->ops->deinit(device);
    osMutexDelete(device->lock);
}
