/**
 * @file uart_lin_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
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
#include <driver/kunlun/lin/uart_lin_drv.h>

#define EXTRA_FMT   "uart_lin:"
#define UART_LIN_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define UART_LIN_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define UART_LIN_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/* uart lin state */
#define UART_LIN_STATE_IDLE             0
#define UART_LIN_STATE_TX               1
#define UART_LIN_STATE_TX_PENDING       2
#define UART_LIN_STATE_TX_ERR           3
#define UART_LIN_STATE_RX               4
#define UART_LIN_STATE_RX_PENDING       5
#define UART_LIN_STATE_CHKSUM_ERR       6
#define UART_LIN_STATE_ABORT            7

/* uart lin event */
#define UART_LIN_EVENT_DONE             1

static void uart_lin_init(lin_dev_t *dev);
static void uart_lin_deinit(lin_dev_t *dev);
static uint8_t uart_lin_wake_up(lin_dev_t *dev);
static uint8_t uart_lin_send_frame(lin_dev_t *dev, const lin_pdu_t *pdu);
static lin_status_t uart_lin_get_status(lin_dev_t *dev, uint8_t **sdu_ptr);
static uint8_t uart_lin_abort(lin_dev_t *dev);
static void uart_lin_rx_callback(void *arg);

static lin_ops_t g_uart_lin_ops = {
    .init = uart_lin_init,
    .deinit = uart_lin_deinit,
    .wake_up = uart_lin_wake_up,
    .send_frame = uart_lin_send_frame,
    .get_status = uart_lin_get_status,
    .abort = uart_lin_abort,
};

/**
 * @brief uart lin calc parity
 *
 * @param[in] id frame pid
 */
static uint8_t uart_lin_calc_parity(uint8_t id)
{
    uint8_t p0 = BIT_SET(id, 0) ^ BIT_SET(id, 1)
                 ^ BIT_SET(id, 2) ^ BIT_SET(id, 4);
    uint8_t p1 = !(BIT_SET(id, 1) ^ BIT_SET(id, 3)
                   ^ BIT_SET(id, 4) ^ BIT_SET(id, 5));

    return (id & BIT_MASK(6)) | (p0 << 6) | (p1 << 7);
}

/**
 * @brief uart lin init pdu info
 *
 * @param[in] dev lin dev
 */
static void uart_lin_pdu_init(uart_lin_dev_t *uart_lin, const lin_pdu_t *pdu)
{
    uart_lin->pdu.pid = uart_lin_calc_parity(pdu->pid);
    uart_lin->pdu.cs = pdu->cs;
    uart_lin->pdu.drc = pdu->drc;
    uart_lin->pdu.dl = pdu->dl;

    if (pdu->dl > 0 && pdu->sdu != NULL) {
        memcpy(uart_lin->pdu.sdu_buf, pdu->sdu, pdu->dl);
    }
}

/**
 * @brief uart lin send abort
 *
 * @param[in] dev lin dev
 */
static uint8_t uart_lin_abort(lin_dev_t *dev)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    if (uart_lin->sub_state == UART_LIN_STATE_TX
            || uart_lin->sub_state == UART_LIN_STATE_RX) {
        uart_lin->sub_state = UART_LIN_STATE_ABORT;
    }

    uart_lin->sub_state = UART_LIN_STATE_IDLE;

    return LIN_OK;
}

static uint8_t uart_lin_checksum(lin_dev_t *dev, volatile uint8_t *data,
                                 uint8_t len)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    uint16_t carry = 0x00;
    uint16_t checkSum = uart_lin->pdu.cs == LIN_CLASSIC_CS ? 0 : uart_lin->pdu.pid;

    /* 0x3c/0x7d classic checksum */
    if ((uart_lin->pdu.pid == 0x3c) || (uart_lin->pdu.pid == 0x7d)) {
        checkSum = 0;
    }

    for (int i = 0; i < len; i++, data++) {
        checkSum += *data;
    }

    carry = checkSum >> 8;
    checkSum = (checkSum & 0xFF) + carry;

    return (uint8_t)(~checkSum);
}

/**
 * @brief uart lin get status
 *
 * @param[in] dev lin dev
 */
static lin_status_t uart_lin_get_status(lin_dev_t *dev, uint8_t **sdu_ptr)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;
    lin_status_t ret_state = LIN_NOT_OK;

    if (dev->mode == LIN_MASTER) {

        if (uart_lin->pdu.drc == LIN_MASTER_RESPONSE) {
            if (uart_lin->sub_state == UART_LIN_STATE_IDLE) {
                ret_state = LIN_TX_OK;
            }
            else if (uart_lin->sub_state == UART_LIN_STATE_TX) {
                ret_state = LIN_TX_BUSY;
            }
            else {
                ret_state = LIN_TX_ERROR;
            }

            *sdu_ptr = uart_lin->pdu.sdu_buf;
        }
        else if (uart_lin->pdu.drc == LIN_SLAVE_RESPONSE) {
            if (uart_lin->sub_state == UART_LIN_STATE_IDLE) {
                ret_state = LIN_RX_OK;
            }
            else if (uart_lin->sub_state == UART_LIN_STATE_RX) {
                ret_state = LIN_RX_NO_RESPONSE;
            }
            else {
                ret_state = LIN_RX_ERROR;
            }

            *sdu_ptr = uart_lin->rx_buf;
        }
        else {
            if (uart_lin->sub_state == UART_LIN_STATE_IDLE) {
                ret_state = LIN_TX_OK;
            }
            else if (uart_lin->sub_state == UART_LIN_STATE_TX) {
                ret_state = LIN_TX_BUSY;
            }
            else {
                ret_state = LIN_TX_HEADER_ERROR;
            }

            *sdu_ptr = uart_lin->pdu.sdu_buf;
        }
    }
    else {
        /* do not support slave mode */
        return LIN_NOT_OK;

    }

    return ret_state;
}

/**
 * @brief uart lin send frame
 *
 * @param[in] dev lin dev
 * @param[in] pdu lin pdu
 */
static uint8_t uart_lin_send_frame(lin_dev_t *dev, const lin_pdu_t *pdu)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    if (dev->mode == LIN_MASTER) {
        /* send frame */
        if (LIN_IDLE == uart_lin->frame_state ) {
            uart_lin_pdu_init(uart_lin, pdu);

            uart_lin->sub_state = UART_LIN_STATE_TX;

            /* only master response caculate checksum */
            if (uart_lin->pdu.drc == LIN_MASTER_RESPONSE) {
                /* calculate parity checksum */
                uart_lin->pdu.check_sum = uart_lin_checksum(dev, uart_lin->pdu.sdu_buf,
                                          uart_lin->pdu.dl);
                /* add checksum compose one frame */
                uart_lin->pdu.sdu_buf[pdu->dl] = uart_lin->pdu.check_sum;
            }

            uart_lin->frame_state = LIN_SEND_SYNC;

            uart_set_baudrate(uart_lin->uart_dev, dev->baud / 2);
            uart_putc(uart_lin->uart_dev, BREAK);
            uart_receive_async(uart_lin->uart_dev, &uart_lin->rxdata, 1,
                               uart_lin_rx_callback);

            osEventFlagsWait(uart_lin->complete, UART_LIN_EVENT_DONE,
                             osFlagsWaitAny, dev->timeout);
        }
    }
    else {
        /* do not support slave mode */
        return LIN_N_OK;
    }

    return LIN_OK;
}

/**
 * @brief uart lin master response
 *
 * @param[in] dev lin dev
 * @param[in] rxdata rx data
 */
static void uart_lin_master_response(lin_dev_t *dev, uint8_t rxdata)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;
    uint8_t checksum = 0u;

    /* verify the first byte is parityID and send data */
    if (uart_lin->master_response == 1) {
        /* verify the first byte is parityID */
        if (uart_lin->pdu.pid != rxdata) {
            uart_lin->frame_state = LIN_ID_ERROR;
        }

        /* send frame & add checksum */
        uart_transmit_async(uart_lin->uart_dev, (char *)uart_lin->pdu.sdu_buf,
                            uart_lin->pdu.dl + 1, NULL);
        uart_lin->master_response = 0;
        uart_receive_async_isr(uart_lin->uart_dev, (char *)uart_lin->rx_buf,
                               uart_lin->pdu.dl + 1, uart_lin_rx_callback);
    }
    /* receive and verify master send data here */
    else {
        /* received all data include checksum */
        uart_lin->master_length += uart_lin->pdu.dl + 1;

        checksum = uart_lin_checksum(dev, uart_lin->pdu.sdu_buf, uart_lin->pdu.dl);

        if (checksum == uart_lin->rx_buf[uart_lin->master_length - 1]) {
            uart_lin->frame_state = LIN_IDLE;
        }
        else {
            uart_lin->frame_state = LIN_MASTER_CHECKSUM_ERROR;
        }

        /* ready send next frame */
        uart_lin->sub_state = UART_LIN_STATE_IDLE;
        uart_lin->master_length = 0;
        uart_lin->master_response = 1;
        osEventFlagsSet(uart_lin->complete, UART_LIN_EVENT_DONE);
    }
}

/**
 * @brief uart lin slave response
 *
 * @param[in] dev lin dev
 */
static void uart_lin_slave_response(lin_dev_t *dev, uint8_t rxdata)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;
    uint8_t checksum = 0u;

    /* verify the first byte is parityID and send data */
    if (uart_lin->slave_response == 1) {
        /* verify the first byte is parityID */
        if (uart_lin->pdu.pid != rxdata) {
            uart_lin->frame_state = LIN_ID_ERROR;
        }

        uart_lin->slave_response = 0;

        uart_receive_async_isr(uart_lin->uart_dev, &uart_lin->rxdata, 1,
                               uart_lin_rx_callback);
    }
    /* receive and verify master send data here */
    else {
        /* check frame */
        uart_lin->rx_buf[uart_lin->slave_length] = rxdata;

        /* received all data include checksum */
        if (uart_lin->slave_length == uart_lin->pdu.dl) {
            checksum = uart_lin_checksum(dev, uart_lin->rx_buf, uart_lin->slave_length);

            if (checksum == uart_lin->rx_buf[uart_lin->slave_length]) {
                uart_lin->frame_state = LIN_IDLE;
            }
            else {
                uart_lin->frame_state = LIN_SLAVE_CHECKSUM_ERROR;
            }

            uart_lin->sub_state = UART_LIN_STATE_IDLE;
            uart_lin->slave_length = 0;
            /* ready send next frame */
            uart_lin->slave_response = 1;
            osEventFlagsSet(uart_lin->complete, UART_LIN_EVENT_DONE);
        }
        else {
            uart_lin->slave_length ++;
            uart_receive_async_isr(uart_lin->uart_dev, &uart_lin->rxdata, 1,
                                   uart_lin_rx_callback);
        }
    }
}

/**
 * @brief uart lin rx callback
 *
 * @param[in] arg callback arg
 */
static void uart_lin_rx_callback(void *arg)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)arg;
    lin_dev_t *dev = &uart_lin->dev;

    switch (uart_lin->frame_state) {
        case LIN_SEND_SYNC:
            uart_set_baudrate(uart_lin->uart_dev, dev->baud);
            uart_putc(uart_lin->uart_dev, SYNC);
            uart_receive_async_isr(uart_lin->uart_dev, &uart_lin->rxdata, 1,
                                   uart_lin_rx_callback);
            uart_lin->frame_state = LIN_SEND_ID;
            break;

        case LIN_SEND_ID:
            uart_putc(uart_lin->uart_dev, uart_lin->pdu.pid);
            uart_receive_async_isr(uart_lin->uart_dev, &uart_lin->rxdata, 1,
                                   uart_lin_rx_callback);

            if (uart_lin->pdu.drc == LIN_MASTER_RESPONSE) {
                uart_lin->frame_state = LIN_MASTER_RESPONE_DATA;
            }
            else if (uart_lin->pdu.drc == LIN_SLAVE_RESPONSE) {
                uart_lin->frame_state = LIN_SLAVE_RESPONE_DATA;
            }
            else {
                /* slave to slave */
            }

            break;

        case LIN_MASTER_RESPONE_DATA:
            uart_lin_master_response(dev, uart_lin->rxdata);
            break;

        case LIN_SLAVE_RESPONE_DATA:
            uart_lin->sub_state = UART_LIN_STATE_RX;
            uart_lin_slave_response(dev, uart_lin->rxdata);
            break;

        default:
            break;
    }
}

/**
 * @brief uart lin initialize
 *
 * @param[in] dev lin dev
 */
static void uart_lin_init(lin_dev_t *dev)
{

    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    uart_lin->complete = osEventFlagsNew(NULL);
    ASSERT(uart_lin->complete);

    uart_init(uart_lin->uart_dev);

    uart_lin->sub_state = UART_LIN_STATE_IDLE;
    uart_lin->frame_state = LIN_NOT_READY;
    uart_lin->master_response = 1;
    uart_lin->slave_response = 1;
}

/**
 * @brief uart lin deinit
 *
 * @param[in] dev lin dev
 */
static void uart_lin_deinit(lin_dev_t *dev)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    osEventFlagsDelete(uart_lin->complete);
}

/**
 * @brief uart lin wake up
 *
 * @param[in] dev lin dev
 */
static uint8_t uart_lin_wake_up(lin_dev_t *dev)
{
    uart_lin_dev_t *uart_lin = (uart_lin_dev_t *)dev->priv;

    uart_lin->sub_state = UART_LIN_STATE_IDLE;
    uart_lin->frame_state = LIN_IDLE;

    uart_set_baudrate(uart_lin->uart_dev, dev->baud / 2);
    uart_putc(uart_lin->uart_dev, BREAK);
    uart_receive_async(uart_lin->uart_dev, &uart_lin->rxdata, 1, NULL);

    return LIN_OK;
}

/**
 * @brief uart lin probe
 *
 * @param[in] dev lin dev
 * @param[in] uart_dev uart dev
 * @param[in] cfg lin config
 */
lin_dev_t *uart_lin_probe(uart_lin_dev_t *uart_lin, uart_dev_t *uart_dev,
                          lin_config_t *cfg)
{
    lin_dev_t *dev = &uart_lin->dev;

    dev->ops = &g_uart_lin_ops;

    uart_lin->uart_dev = uart_dev;
    uart_lin->uart_dev->rx_arg = uart_lin;

    dev->priv = uart_lin;

    return dev;
}

