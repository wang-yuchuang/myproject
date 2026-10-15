/**
 * @file sdrv_irda_drv.c
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
#include <bits.h>
#include <string.h>

#include <driver/irq.h>
#include "driver/taishan/irda/sdrv_irda_drv.h"
#include <taishan/uart/sdrv_uart.h>

#define EXTRA_FMT   "sdrv_irda:"
#define SDRV_IRDA_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_IRDA_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_IRDA_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

#define SDRV_IRDA_STATE_IDLE            0
#define SDRV_IRDA_STATE_TX              1
#define SDRV_IRDA_STATE_TX_ASYNC        2
#define SDRV_IRDA_STATE_RX              3
#define SDRV_IRDA_STATE_RX_ASYNC        4

#define SDRV_IRDA_EVENT_DONE            0x1
#define SDRV_IRDA_EVENT_ABORT           0x2

static void sdrv_irda_init(irda_dev_t *dev, const irda_config_t *config);
static void sdrv_irda_deinit(irda_dev_t *dev);
static uint8_t sdrv_irda_send(irda_dev_t *dev, const char *data,
                              uint8_t len, uint32_t timeout);
static uint8_t sdrv_irda_send_async(irda_dev_t *dev, const char *data,
                                    uint8_t len, irda_cb_t cb, void *arg);
static uint8_t sdrv_irda_receive(irda_dev_t *dev, char *data,
                                 uint8_t len, uint32_t timeout);
uint8_t sdrv_irda_receive_async(irda_dev_t *dev, char *data,
                                uint8_t len, irda_cb_t cb, void *arg);

static irda_ops_t g_sdrv_irda_ops = {
    .init = sdrv_irda_init,
    .deinit = sdrv_irda_deinit,
    .send = sdrv_irda_send,
    .send_async = sdrv_irda_send_async,
    .recv = sdrv_irda_receive,
    .recv_async = sdrv_irda_receive_async,
};

/**
 * @brief sdrv irda irq handler
 *
 * @param[in] dev irda dev
 */
static int sdrv_irda_irq_handler(uint32_t irq, void *arg)
{
    irda_dev_t *dev = (irda_dev_t *)arg;
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;
    uint32_t state;

    state = sdrv_uart_lld_handle_irq(dev->base);

    sdrv_uart_lld_clear_irq(dev->base, state);

    SDRV_IRDA_INFO("irda irq:0x%x\n", state);

    if ((state & SDRV_UART_INTR_APBCMDDONE) != 0) {
        if (sdrv_irda->status == SDRV_IRDA_STATE_TX || \
            sdrv_irda->status == SDRV_IRDA_STATE_RX) {
            osEventFlagsSet(sdrv_irda->complete, SDRV_IRDA_EVENT_DONE);
        }
        else if (sdrv_irda->status == SDRV_IRDA_STATE_TX_ASYNC) {
            sdrv_irda->cb(sdrv_irda->arg, IRDA_OK);
        }
        else if (sdrv_irda->status == SDRV_IRDA_STATE_RX_ASYNC) {
            sdrv_uart_lld_receive(dev->base, sdrv_irda->rx_buffer, sdrv_irda->rx_len);
            sdrv_irda->cb(sdrv_irda->arg, IRDA_OK);
        }
    }
    else if ((state & SDRV_UART_INTR_APBCMDABORT) != 0) {
        if (sdrv_irda->status == SDRV_IRDA_STATE_TX || \
            sdrv_irda->status == SDRV_IRDA_STATE_RX) {
            osEventFlagsSet(sdrv_irda->complete, SDRV_IRDA_EVENT_ABORT);
        }
        else if (sdrv_irda->status == SDRV_IRDA_STATE_TX_ASYNC || \
                 sdrv_irda->status == SDRV_IRDA_STATE_RX_ASYNC) {
            sdrv_irda->cb(sdrv_irda->arg, IRDA_N_OK);
        }
    }

    return 0;
}

/**
 * @brief IrDA init.
 *
 * @param[in] device irda dev
 * @param[in] config irda config
 */
static void sdrv_irda_init(irda_dev_t *dev, const irda_config_t *config)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;
    sdrv_uart_config_t irda_cfg = {
        .baud = dev->carrier_freq,
        .clk_freq = dev->clk_freq,
        .op_mode = SDRV_UART_IRDA_MODE,
        .transfer_mode = SDRV_UART_HALF_DUPLEX,
        .data_bits = SDRV_UART_CHAR_8_BITS,
        .stop_bits = SDRV_UART_STOP_1_BIT,
        .parity = SDRV_UART_NO_PARITY,
    };

    sdrv_irda->complete = osEventFlagsNew(NULL);

    sdrv_uart_lld_init(dev->base, &irda_cfg);
    sdrv_irda->status = SDRV_IRDA_STATE_IDLE;

    if (dev->irq > 0) {
        irq_attach(dev->irq, sdrv_irda_irq_handler, dev);
        irq_enable(dev->irq);
    }
}

/**
 * @brief IrDA deinit.
 *
 * @param[in] device irda dev
 */
static void sdrv_irda_deinit(irda_dev_t *dev)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;

    if (dev->irq > 0) {
        irq_disable(dev->irq);
        irq_detach(dev->irq);
    }

    sdrv_uart_lld_deinit(dev->base);

    osEventFlagsDelete(sdrv_irda->complete);
}

/**
 * @brief IrDA send data.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @param[in] timeout send timeout value
 * @return send result
 */
static uint8_t sdrv_irda_send(irda_dev_t *dev, const char *data,
                              uint8_t len, uint32_t timeout)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;
    uint32_t flags;
    uint8_t ret = IRDA_OK;

    if (sdrv_irda->status != SDRV_IRDA_STATE_IDLE) {
        SDRV_IRDA_WARN("status error: %d\n", sdrv_irda->status);
        return IRDA_N_OK;
    }

    sdrv_irda->status = SDRV_IRDA_STATE_TX;

    sdrv_uart_lld_sir_transmit(dev->base, data, len);
    flags = osEventFlagsWait(sdrv_irda->complete,
        SDRV_IRDA_EVENT_DONE | SDRV_IRDA_EVENT_ABORT,
        osFlagsWaitAny, timeout);

    if ((flags & SDRV_IRDA_EVENT_DONE) != SDRV_IRDA_EVENT_DONE) {
        ret = IRDA_N_OK;
    }

    sdrv_irda->status = SDRV_IRDA_STATE_IDLE;

    return ret;
}

/**
 * @brief IrDA send data in async mode.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @return send result
 */
static uint8_t sdrv_irda_send_async(irda_dev_t *dev, const char *data,
                                    uint8_t len, irda_cb_t cb, void *arg)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;

    if (sdrv_irda->status != SDRV_IRDA_STATE_IDLE) {
        SDRV_IRDA_WARN("status error: %d\n", sdrv_irda->status);
        return IRDA_N_OK;
    }

    sdrv_irda->cb = cb;
    sdrv_irda->arg = arg;
    sdrv_irda->status = SDRV_IRDA_STATE_TX_ASYNC;

    sdrv_uart_lld_sir_transmit(dev->base, data, len);

    return IRDA_OK;
}

/**
 * @brief IrDA receive data.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @param[in] timeout receive timeout value
 * @return receive result
 */
uint8_t sdrv_irda_receive(irda_dev_t *dev, char *data,
                          uint8_t len, uint32_t timeout)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;
    uint32_t flags;
    uint8_t ret = IRDA_OK;

    if (sdrv_irda->status != SDRV_IRDA_STATE_IDLE) {
        SDRV_IRDA_WARN("status error: %d\n", sdrv_irda->status);
        return IRDA_N_OK;
    }

    sdrv_irda->status = SDRV_IRDA_STATE_RX;

    sdrv_uart_lld_sir_receive(dev->base, len);
    flags = osEventFlagsWait(sdrv_irda->complete,
        SDRV_IRDA_EVENT_DONE | SDRV_IRDA_EVENT_ABORT,
        osFlagsWaitAny, timeout);

    if ((flags & SDRV_IRDA_EVENT_DONE) == SDRV_IRDA_EVENT_DONE) {
        sdrv_uart_lld_receive(dev->base, data, len);
    }
    else {
        ret = IRDA_N_OK;
    }

    return ret;
}

/**
 * @brief IrDA receive data in async mode.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @return receive result
 */
uint8_t sdrv_irda_receive_async(irda_dev_t *dev, char *data,
                                uint8_t len, irda_cb_t cb, void *arg)
{
    sdrv_irda_dev_t *sdrv_irda = (sdrv_irda_dev_t *)dev;

    if (sdrv_irda->status != SDRV_IRDA_STATE_IDLE) {
      SDRV_IRDA_WARN("status error: %d\n", sdrv_irda->status);
      return IRDA_N_OK;
    }

    sdrv_irda->cb = cb;
    sdrv_irda->arg = arg;
    sdrv_irda->rx_buffer = data;
    sdrv_irda->rx_len = len;
    sdrv_irda->status = SDRV_IRDA_STATE_RX_ASYNC;

    sdrv_uart_lld_sir_receive(dev->base, len);

    return IRDA_OK;
}

/**
 * @brief sdrv irda probe
 *
 * @param[in] dev irda dev
 */
irda_dev_t *sdrv_irda_probe(sdrv_irda_dev_t *sdrv_irda)
{
    irda_dev_t *dev = &sdrv_irda->dev;
    dev->ops = &g_sdrv_irda_ops;
    return dev;
}
