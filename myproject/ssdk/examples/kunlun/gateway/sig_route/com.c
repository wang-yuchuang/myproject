/**
 * @file com.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <compiler.h>
#include <cmsis_os2.h>
#include <string.h>
#include <lib/list.h>
#include <spinlock.h>
#include <reg.h>
#include <debug.h>
#include "com.h"
#include "com_cfg.h"
#include "lin_cfg.h"
#include "vcan.h"
#include "vlin.h"

#define COM_MS2TICK(ms)    ((ms) * osKernelGetTickFreq() / 1000)

static struct list_node g_can_rx_list[CAN_CTRL_CONFIG_CNT],
                        g_lin_rx_list[LIN_IFC_CHN_MAX];
static frame_desc_t **g_tx_frame;
static bool g_com_init;

static void com_sig_route(frame_desc_t *frame)
{
    for (size_t i = 0; i < frame->sig_rule_nr; i++) {
        signal_rule_t *rule = &frame->sig_rule[i];
        uint32_t sig_val = com_read_signal(&rule->src_sig);
        com_write_signal(&rule->tgt_sig, sig_val);
    }
}

static void com_send_frame(frame_desc_t *frame)
{
    if (frame->port_id == PORT_CAN) {
        const Can_PduType pdu = {
            0U,
            frame->len,
            frame->prot_id,
            frame->data
        };
        Can_Write(frame->bus_id, &pdu);
    }
    else if (frame->port_id == PORT_LIN) {
        /**
         * LIN frame is processed by LIN
         * scheduler in lin_sched/lin_sched.c,
         * so do nothing here.
         */
    }
    else {
        ssdk_printf(SSDK_WARNING, "%s: Invalid port\n", __func__);
    }
}

static void com_service_thread(void *arg)
{
    while (true) {
        com_tx_frame();
        osDelay(COM_MS2TICK(CONFIG_TX_THREAD_PERIOD));
    }
}

void com_tx_frame(void)
{
    static uint32_t tx_time_cnt = ~0U -
                CONFIG_TX_THREAD_PERIOD + 1U;

    tx_time_cnt += CONFIG_TX_THREAD_PERIOD;

    if (likely(g_tx_frame)) {
        for (size_t i = 0; g_tx_frame[i]; i++) {
            frame_desc_t *frame = g_tx_frame[i];
            if (!(tx_time_cnt % frame->period)) {
                com_send_frame(frame);
            }
        }
    }
}

void com_rx_frame(uint16_t port_id, uint16_t bus_id,
                  uint32_t prot_id, uint8_t *data)
{
    struct list_node *header;
    frame_desc_t *frame_desc;
    irq_state_t irq_state;

    if (g_com_init) {
        ssdk_printf(SSDK_DEBUG, "%s: port_id %d, bus_id %d, prot_id 0x%x\n",
                    __func__, port_id, bus_id, prot_id);

        if (port_id == PORT_CAN) {
            header = &g_can_rx_list[bus_id];
        }
        else if (port_id == PORT_LIN){
            header = &g_lin_rx_list[bus_id];
        }
        else {
            ssdk_printf(SSDK_WARNING, "%s: Invlaid port\n", __func__);
            return;
        }

        list_for_every_entry(header, frame_desc, frame_desc_t, node) {
            if (frame_desc->prot_id == prot_id) {
                irq_state = spin_lock_irqsave(&frame_desc->lock);
                memcpy(frame_desc->data, data, frame_desc->len);
                spin_unlock_irqrestore(&frame_desc->lock, irq_state);
                com_sig_route(frame_desc);
                break;
            }
        }
    }
}

/**
 * @brief Read signal whose length is less than or
 *        equal to 32 bits.
 */
uint32_t com_read_signal(signal_desc_t *signal)
{
    uint32_t sig_value = 0U;
    frame_desc_t *frame = signal->frame;
    irq_state_t irq_state;
    uint8_t msb_byte, lsb_byte;
    uint16_t lsb_bit;
    uint16_t len = signal->width;

    switch (signal->order) {
        case BYTE_ORDER_INTEL:
            lsb_byte = signal->start_bit >> 3;
            lsb_bit = signal->start_bit & 7U;
            msb_byte = lsb_byte + ((lsb_bit + len - 1U) >> 3);

            ssdk_printf(SSDK_DEBUG, "%s: lsb byte %d, lsb bit %d, msb byte %d, len %d\n",
                        __func__, lsb_byte, lsb_bit, msb_byte, len);

            irq_state = spin_lock_irqsave(&frame->lock);
            for (size_t i = lsb_byte; i <= msb_byte; i++) {
                sig_value += readb(&frame->data[i]) <<
                             ((i - lsb_byte) << 3);
            }
            spin_unlock_irqrestore(&frame->lock, irq_state);

            /* Remove needless head and tail. */
            sig_value >>= lsb_bit;
            sig_value &= (1U << len) - 1U;

            ssdk_printf(SSDK_DEBUG, "%s: Read signal value 0x%x\n", __func__, sig_value);
            break;

        default:
            break;
    }

    return sig_value;
}

/**
 * @brief Write signal whose length is less than or
 *        equal to 32 bits.
 */
void com_write_signal(signal_desc_t *signal, uint32_t value)
{
    irq_state_t irq_state;
    frame_desc_t *frame = signal->frame;
    uint8_t msb_byte, lsb_byte;
    uint16_t lsb_bit;
    uint16_t len = signal->width;

    switch (signal->order) {
        case BYTE_ORDER_INTEL:
            lsb_byte = signal->start_bit >> 3;
            lsb_bit = signal->start_bit & 7U;
            msb_byte = lsb_byte + ((lsb_bit + len - 1U) >> 3);

            ssdk_printf(SSDK_DEBUG, "%s: lsb byte %d, lsb bit %d, msb byte %d, len %d\n",
                        __func__, lsb_byte, lsb_bit, msb_byte, len);

            uint32_t old_value = 0U;

            irq_state = spin_lock_irqsave(&frame->lock);

            for (size_t i = lsb_byte; i <= msb_byte; i++) {
                old_value += readb(&frame->data[i]) <<
                             ((i - lsb_byte) << 3);
            }

            ssdk_printf(SSDK_DEBUG, "%s: old value 0x%x\n", __func__, old_value);

            old_value &= (~0U << (lsb_bit + len)) | ((1U << lsb_bit) - 1U);

            ssdk_printf(SSDK_DEBUG, "%s: after clear bits to be modified old value 0x%x\n",
                        __func__, old_value);

            value &= (1U << len) - 1U;
            value <<= lsb_bit;
            value |= old_value;

            ssdk_printf(SSDK_DEBUG, "%s: write value 0x%x\n", __func__, value);

            for (size_t i = lsb_byte; i <= msb_byte; i++) {
                writeb((value >> ((i - lsb_byte) << 3)) & 0xFFU, &frame->data[i]);
            }

            spin_unlock_irqrestore(&frame->lock, irq_state);
            break;

        default:
            break;
    }
}

void com_init(frame_desc_t *rx_frame[], frame_desc_t *tx_frame[])
{
    for (size_t i = 0; i < CAN_CTRL_CONFIG_CNT; i++) {
        list_initialize(&g_can_rx_list[i]);
    }

    for (size_t i = 0; i < LIN_IFC_CHN_MAX; i++) {
        list_initialize(&g_lin_rx_list[i]);
    }

    for (size_t i = 0; rx_frame[i]; i++) {
        uint8_t bus_id = rx_frame[i]->bus_id;
        if (rx_frame[i]->port_id == PORT_CAN) {
            list_add_tail(&g_can_rx_list[bus_id], &rx_frame[i]->node);
        }
        else if (rx_frame[i]->port_id == PORT_LIN) {
            list_add_tail(&g_lin_rx_list[bus_id], &rx_frame[i]->node);
        }
        else {
            ssdk_printf(SSDK_WARNING, "%s: Invalid port\n", __func__);
        }

        spin_lock_init(&rx_frame[i]->lock);
    }

    for (size_t i = 0; tx_frame[i]; i++) {
        spin_lock_init(&tx_frame[i]->lock);
    }

    g_tx_frame = tx_frame;
    g_com_init = true;
}

void start_sig_route_service(void)
{
    com_init(g_rx_frame_cfg, g_tx_frame_cfg);
    osThreadNew(com_service_thread, NULL, NULL);
}
