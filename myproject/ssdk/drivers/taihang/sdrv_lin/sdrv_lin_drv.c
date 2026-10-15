/**
 * @file sdrv_lin_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 *
 * Description:
 * Revision History:
 * -----------------
 */

#include <bits.h>
#include <debug.h>
#include <driver/irq.h>
#include <string.h>
#include <taihang/sdrv_uart/sdrv_uart.h>
#include <types.h>

#include "driver/taihang/sdrv_lin/sdrv_lin_drv.h"

#define EXTRA_FMT "sdrv_lin:"
#define SDRV_LIN_ERR(format, ...)                                              \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_LIN_WARN(format, ...)                                             \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_LIN_INFO(format, ...)                                             \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/* sdrv lin state */
#define SDRV_LIN_STATE_IDLE 0
#define SDRV_LIN_STATE_TX 1
#define SDRV_LIN_STATE_TX_PENDING 2
#define SDRV_LIN_STATE_TX_ERR 3
#define SDRV_LIN_STATE_RX 4
#define SDRV_LIN_STATE_RX_PENDING 5
#define SDRV_LIN_STATE_CHKSUM_ERR 6

/* sdrv lin event */
#define SDRV_LIN_EVENT_TX_DONE 0x1
#define SDRV_LIN_EVENT_RX_DONE 0x2
#define SDRV_LIN_EVENT_ABORT 0x4

#define SDRV_LIN_BREAK_LEN 13

#define LIN_WAKEUP_PID 0x3e

static void sdrv_lin_init(lin_dev_t *dev);
static void sdrv_lin_deinit(lin_dev_t *dev);
static uint8_t sdrv_lin_wake_up(lin_dev_t *dev);
static uint8_t sdrv_lin_send_frame(lin_dev_t *dev, const lin_pdu_t *pdu);
static lin_status_t sdrv_lin_get_status(lin_dev_t *dev, uint8_t **sdu_ptr);
static uint8_t sdrv_lin_abort(lin_dev_t *dev);

static lin_ops_t g_sdrv_lin_ops = {
    .init = sdrv_lin_init,
    .deinit = sdrv_lin_deinit,
    .wake_up = sdrv_lin_wake_up,
    .send_frame = sdrv_lin_send_frame,
    .get_status = sdrv_lin_get_status,
    .abort = sdrv_lin_abort,
};

/**
 * @brief sdrv lin calc parity
 *
 * @param[in] id frame pid
 */
static uint8_t sdrv_lin_calc_parity(uint8_t id)
{
    uint8_t p0 =
        BIT_SET(id, 0) ^ BIT_SET(id, 1) ^ BIT_SET(id, 2) ^ BIT_SET(id, 4);
    uint8_t p1 =
        !(BIT_SET(id, 1) ^ BIT_SET(id, 3) ^ BIT_SET(id, 4) ^ BIT_SET(id, 5));

    return (id & BIT_MASK(6)) | (p0 << 6) | (p1 << 7);
}

/**
 * @brief convert sdrv lin checksum
 *
 * @param[in] id frame pid
 */
static uint8_t sdrv_lin_chksum_convert(lin_cs_mode_t cs_mode)
{
    if (cs_mode == LIN_ENHANCED_CS) {
        return SDRV_UART_ENHANCED_CHECKSUM;
    } else {
        return SDRV_UART_CLASSIC_CHECKSUM;
    }
}

/**
 * @brief sdrv lin init pdu info
 *
 * @param[in] dev lin dev
 */
static void sdrv_lin_pdu_init(sdrv_lin_dev_t *sdrv_lin, const lin_pdu_t *pdu)
{
    sdrv_lin->pdu.pid = sdrv_lin_calc_parity(pdu->pid);
    sdrv_lin->pdu.cs = sdrv_lin_chksum_convert(pdu->cs);
    sdrv_lin->pdu.drc = pdu->drc;
    sdrv_lin->pdu.dl = pdu->dl;
    if (pdu->dl > 0 && pdu->sdu != NULL) {
        memcpy(sdrv_lin->pdu.tx_sdu, pdu->sdu, pdu->dl);
    }
}

static inline void sdrv_lin_lld_receive(lin_dev_t *dev, char *data, size_t size)
{
    if ((NULL == dev) || (NULL == data)) {
        return;
    }
    uint32_t timeout = dev->timeout;
    sdrv_uart_t *lin_base = (sdrv_uart_t *)(dev->base);

    for (size_t i = 0; i < size; i++) {
        while (!!(lin_base->FSR1.EMPTY)) {
            if (0 == timeout)
                return;
            --timeout;
        }
        data[i] = lin_base->RXDR;
    }
}

/**
 * @brief sdrv lin irq handler
 *
 * @param[in] dev lin dev
 */
static int sdrv_lin_irq_handler(uint32_t irq, void *arg)
{
    lin_dev_t *dev = (lin_dev_t *)arg;
    sdrv_lin_dev_t *sdrv_lin = (sdrv_lin_dev_t *)dev->priv;
    uint32_t state;

    state = sdrv_uart_lld_handle_irq(dev->base);

    sdrv_uart_lld_clear_irq(dev->base, state);

    if ((state & SDRV_UART_INTR_ABRPASS) != 0) {
        dev->baud = dev->clk_freq / sdrv_uart_lld_get_baudrate(dev->base);
        SDRV_LIN_INFO("ABR PASS, baud:%d\n", dev->baud);
    } else if ((state & SDRV_UART_INTR_ABRFAIL) != 0) {
        SDRV_LIN_INFO("ABR FAIL\n");
    }

    if (sdrv_lin->sub_state == SDRV_LIN_STATE_RX) {
        if ((state & SDRV_UART_INTR_RXCHKSUMPASS) != 0) {
            sdrv_lin_lld_receive(dev, (char *)sdrv_lin->pdu.rx_pdu,
                                 sdrv_lin->pdu.dl);
            sdrv_lin->sub_state = SDRV_LIN_STATE_IDLE;
        } else if ((state & SDRV_UART_INTR_RXCHKSUMERR) != 0) {
            sdrv_lin->sub_state = SDRV_LIN_STATE_CHKSUM_ERR;
        }
        SDRV_LIN_INFO("checksum is:0x%x\n",
                      sdrv_uart_lld_get_lin_chksum(dev->base));
    } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_TX) {
        if ((state & SDRV_UART_INTR_APBCMDDONE) != 0) {
            sdrv_lin->sub_state = SDRV_LIN_STATE_IDLE;
        } else if ((state & SDRV_UART_INTR_APBCMDABORT) != 0) {
            sdrv_lin->sub_state = SDRV_LIN_STATE_TX_ERR;
        }
    } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_TX_PENDING) {
        if ((state & SDRV_UART_INTR_PIDPASS) != 0) {
            sdrv_uart_lld_lin_transmit(dev->base, sdrv_lin->pdu.cs,
                                       (char *)sdrv_lin->pdu.tx_sdu,
                                       sdrv_lin->pdu.dl);
            sdrv_lin->sub_state = SDRV_LIN_STATE_TX;
        }
    } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_RX_PENDING) {
        if ((state & SDRV_UART_INTR_PIDPASS) != 0) {
            sdrv_uart_lld_lin_receive(dev->base, sdrv_lin->pdu.cs,
                                      sdrv_lin->pdu.dl);
            sdrv_lin->sub_state = SDRV_LIN_STATE_RX;
        }
    }

    SDRV_LIN_INFO("lin irq:0x%x\n", state);

    return 0;
}

/**
 * @brief sdrv lin init
 *
 * @param[in] dev lin dev
 */
static void sdrv_lin_init(lin_dev_t *dev)
{
    sdrv_lin_dev_t *sdrv_lin = (sdrv_lin_dev_t *)dev->priv;
    sdrv_uart_config_t lin_cfg = {
        .baud = dev->baud,
        .clk_freq = dev->clk_freq,
        .data_bits = SDRV_UART_CHAR_8_BITS,
        .stop_bits = SDRV_UART_STOP_1_BIT,
        .parity = SDRV_UART_NO_PARITY,
        .breakbit = SDRV_LIN_BREAK_LEN,
    };

    if (dev->mode == LIN_MASTER) {
        lin_cfg.op_mode = SDRV_UART_LIN_MASTER_MODE;
    } else {
        lin_cfg.op_mode = SDRV_UART_LIN_SLAVE_MODE;
        lin_cfg.chksum = sdrv_lin_chksum_convert(dev->cs_mode);
    }

    if (dev->baud == LIN_AUTO_BAUD) {
        lin_cfg.abr.abr_en = 1;
        lin_cfg.abr.pattern = 0;
        lin_cfg.abr.max_time = 0;
    }

    sdrv_uart_lld_init(dev->base, &lin_cfg);

    if (dev->irq > 0) {
        irq_attach(dev->irq, sdrv_lin_irq_handler, dev);
        irq_enable(dev->irq);
    }

    sdrv_lin->sub_state = SDRV_LIN_STATE_IDLE;
}

/**
 * @brief sdrv lin deinit
 *
 * @param[in] dev lin dev
 */
static void sdrv_lin_deinit(lin_dev_t *dev)
{
    if (dev->irq > 0) {
        irq_disable(dev->irq);
        irq_detach(dev->irq);
    }

    sdrv_uart_lld_deinit(dev->base);
}

/**
 * @brief sdrv lin wake up
 *
 * @param[in] dev lin dev
 */
static uint8_t sdrv_lin_wake_up(lin_dev_t *dev)
{
    sdrv_uart_lld_set_lin_pid(dev->base, sdrv_lin_calc_parity(LIN_WAKEUP_PID));

    sdrv_uart_lld_lin_transmit(dev->base, SDRV_UART_CLASSIC_CHECKSUM, NULL, 0);

    return LIN_OK;
}

/**
 * @brief sdrv lin send abort
 *
 * @param[in] dev lin dev
 */
static uint8_t sdrv_lin_abort(lin_dev_t *dev)
{
    if (NULL == dev) {
        return LIN_N_OK;
    }
    uint32_t timeout = dev->timeout;
    sdrv_lin_dev_t *sdrv_lin = (sdrv_lin_dev_t *)dev->priv;
    sdrv_uart_t *lin_base = (sdrv_uart_t *)(dev->base);

    lin_base->FCR0.CLR = 1;
    lin_base->FCR1.CLR = 1;

    lin_base->MCR0.MODRST = 1;
    while (1 == lin_base->MCR0.MODRST) {
        if (0 == timeout) {
            return LIN_N_OK;
        }
        --timeout;
    }

    sdrv_lin->sub_state = SDRV_LIN_STATE_IDLE;
    memset(&sdrv_lin->pdu, 0, sizeof(sdrv_lin_pdu_t));

    return LIN_OK;
}

/**
 * @brief sdrv lin send frame
 *
 * @param[in] dev lin dev
 */
static uint8_t sdrv_lin_send_frame(lin_dev_t *dev, const lin_pdu_t *pdu)
{
    sdrv_lin_dev_t *sdrv_lin = (sdrv_lin_dev_t *)dev->priv;

    if (sdrv_lin->sub_state != SDRV_LIN_STATE_IDLE) {
        SDRV_LIN_WARN("state error:%d\n", sdrv_lin->sub_state);
        return LIN_N_OK;
    }

    if (dev->mode == LIN_MASTER) {

        sdrv_lin_pdu_init(sdrv_lin, pdu);

        sdrv_lin->sub_state = SDRV_LIN_STATE_TX;

        sdrv_uart_lld_set_lin_pid(dev->base, pdu->pid);

        if (pdu->drc == LIN_MASTER_RESPONSE) {
            sdrv_uart_lld_lin_transmit(dev->base,
                                       sdrv_lin_chksum_convert(pdu->cs),
                                       (char *)pdu->sdu, pdu->dl);
        } else if (pdu->drc == LIN_SLAVE_RESPONSE) {
            sdrv_uart_lld_lin_receive(
                dev->base, sdrv_lin_chksum_convert(pdu->cs), pdu->dl);
            sdrv_lin->sub_state = SDRV_LIN_STATE_RX;
        } else {
            sdrv_uart_lld_lin_transmit(dev->base, 0, NULL, 0);
        }
    } else {
        /* slave mode only for test */

        sdrv_lin_pdu_init(sdrv_lin, pdu);

        if (pdu->drc == LIN_MASTER_RESPONSE) {
            sdrv_lin->sub_state = SDRV_LIN_STATE_RX_PENDING;
        } else if (pdu->drc == LIN_SLAVE_RESPONSE) {
            sdrv_lin->sub_state = SDRV_LIN_STATE_TX_PENDING;
        }
    }

    return LIN_OK;
}

/**
 * @brief sdrv lin get status
 *
 * @param[in] dev lin dev
 */
static lin_status_t sdrv_lin_get_status(lin_dev_t *dev, uint8_t **sdu_ptr)
{
    sdrv_lin_dev_t *sdrv_lin = (sdrv_lin_dev_t *)dev->priv;
    lin_status_t ret_state = LIN_NOT_OK;

    if (dev->mode == LIN_MASTER) {

        if (sdrv_lin->pdu.drc == LIN_MASTER_RESPONSE) {
            if (sdrv_lin->sub_state == SDRV_LIN_STATE_IDLE) {
                ret_state = LIN_TX_OK;
            } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_TX) {
                ret_state = LIN_TX_BUSY;
            } else {
                ret_state = LIN_TX_ERROR;
            }
            *sdu_ptr = sdrv_lin->pdu.tx_sdu;
        } else if (sdrv_lin->pdu.drc == LIN_SLAVE_RESPONSE) {
            if (sdrv_lin->sub_state == SDRV_LIN_STATE_IDLE) {
                ret_state = LIN_RX_OK;
            } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_RX) {
                ret_state = LIN_RX_NO_RESPONSE;
            } else {
                ret_state = LIN_RX_ERROR;
            }
            *sdu_ptr = sdrv_lin->pdu.rx_pdu;
        } else {
            if (sdrv_lin->sub_state == SDRV_LIN_STATE_IDLE) {
                ret_state = LIN_TX_OK;
            } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_TX) {
                ret_state = LIN_TX_BUSY;
            } else {
                ret_state = LIN_TX_HEADER_ERROR;
            }
            *sdu_ptr = sdrv_lin->pdu.tx_sdu;
        }
    } else {
        /* slave mode only for test */
        if (sdrv_lin->pdu.drc == LIN_SLAVE_RESPONSE) {

            if (sdrv_lin->sub_state == SDRV_LIN_STATE_IDLE) {
                ret_state = LIN_TX_OK;
            } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_TX ||
                       sdrv_lin->sub_state == SDRV_LIN_STATE_TX_PENDING) {
                ret_state = LIN_TX_BUSY;
            } else {
                ret_state = LIN_TX_ERROR;
            }
            *sdu_ptr = sdrv_lin->pdu.tx_sdu;
        } else if (sdrv_lin->pdu.drc == LIN_MASTER_RESPONSE) {

            if (sdrv_lin->sub_state == SDRV_LIN_STATE_IDLE) {
                ret_state = LIN_RX_OK;
            } else if (sdrv_lin->sub_state == SDRV_LIN_STATE_RX ||
                       sdrv_lin->sub_state == SDRV_LIN_STATE_RX_PENDING) {
                ret_state = LIN_RX_BUSY;
            } else {
                ret_state = LIN_RX_ERROR;
            }
            *sdu_ptr = sdrv_lin->pdu.rx_pdu;
        }
    }

    return ret_state;
}

/**
 * @brief sdrv lin probe
 *
 * @param[in] dev lin dev
 */
lin_dev_t *sdrv_lin_probe(sdrv_lin_dev_t *sdrv_lin)
{
    lin_dev_t *dev = &sdrv_lin->dev;
    dev->ops = &g_sdrv_lin_ops;
    dev->priv = sdrv_lin;
    return dev;
}
