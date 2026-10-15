/**
 * @file sdrv_uart.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SDRV UART low level driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>
#include <reg.h>

#include "sdrv_uart.h"

#define SDRV_UART_DOORBELL_TRIGGER          0xA53C
#define SDRV_UART_DOORBELL_ABORT            0xA5C3

static void sdrv_uart_lld_set_databit(sdrv_uart_t *dev, sdrv_uart_dls_t dls)
{
    if (dls == SDRV_UART_CHAR_8_BITS) {
        dev->PCR0.DATABIT = SDRV_UART_PCR_CHARLEN_8_BIT;
    }
    else if (dls == SDRV_UART_CHAR_7_BITS) {
        dev->PCR0.DATABIT = SDRV_UART_PCR_CHARLEN_7_BIT;
    }
    else if (dls == SDRV_UART_CHAR_6_BITS) {
        dev->PCR0.DATABIT = SDRV_UART_PCR_CHARLEN_6_BIT;
    }
    else if (dls == SDRV_UART_CHAR_5_BITS) {
        dev->PCR0.DATABIT = SDRV_UART_PCR_CHARLEN_5_BIT;
    }
    else if (dls == SDRV_UART_CHAR_9_BITS) {
        dev->PCR0.DATABIT = SDRV_UART_PCR_CHARLEN_9_BIT;
    }
}

static void sdrv_uart_lld_set_stopbit(sdrv_uart_t *dev, sdrv_uart_stop_t stop)
{
    if (stop == SDRV_UART_STOP_1_BIT) {
        dev->PCR0.STOPBIT = SDRV_UART_PCR_STOPBIT_1_BIT;
    }
    else if (stop == SDRV_UART_STOP_1_5_BIT) {
        dev->PCR0.STOPBIT = SDRV_UART_PCR_STOPBIT_1_5_BIT;
    }
    else if (stop == SDRV_UART_STOP_2_BIT) {
        dev->PCR0.STOPBIT = SDRV_UART_PCR_STOPBIT_2_BIT;
    }
    else if (stop == SDRV_UART_STOP_0_5_BIT) {
        dev->PCR0.STOPBIT = SDRV_UART_PCR_STOPBIT_0_5_BIT;
    }
}

static void sdrv_uart_lld_set_parity(sdrv_uart_t *dev, sdrv_uart_parity_t parity)
{
    if (parity == SDRV_UART_NO_PARITY) {
        dev->PCR0.PARITYBIT = SDRV_UART_PCR_PARITY_NONE;
    }
    else if (parity == SDRV_UART_ODD_PARITY) {
        dev->PCR0.PARITYBIT = SDRV_UART_PCR_PARITY_ODD;
    }
    else if (parity == SDRV_UART_EVEN_PARITY) {
        dev->PCR0.PARITYBIT = SDRV_UART_PCR_PARITY_EVEN;
    }
}

static void sdrv_uart_lld_set_abr(sdrv_uart_t *dev, sdrv_uart_abr_config_t *abr_cfg)
{
    dev->PCR0.ABRCTL0 = abr_cfg->pattern;
    dev->PCR0.ABRCTL1 = abr_cfg->max_time;

    dev->INTEN0.ABRPASS = 1;
    dev->INTEN0.ABRFAIL = 1;

    dev->PCR0.ABREN = 1;
}

static void sdrv_uart_lld_set_fcm(sdrv_uart_t *dev, sdrv_uart_fcm_config_t *fcm_cfg)
{
    dev->PCR0.FCM = fcm_cfg->fcm_type;
    if (fcm_cfg->fcm_type == SDRV_UART_RS485_FC) {
        dev->PCR3.DEAT = fcm_cfg->rs485_fcm.deat_cycle;
        dev->PCR3.DEDT = fcm_cfg->rs485_fcm.dedt_cycle;
        dev->PCR3.DE2RE = fcm_cfg->rs485_fcm.de2re_cycle;
        dev->PCR3.RE2DE = fcm_cfg->rs485_fcm.re2de_cycle;
    }
}

static void sdrv_uart_lin_init(sdrv_uart_t *dev, const sdrv_uart_config_t *cfg)
{
    dev->PCR4.TXBREAKCTL = SDRV_UART_TXBREAK_BIT(cfg->breakbit);
    dev->PCR1.RXBREAKCTL = SDRV_UART_RXBREAK_BIT(cfg->breakbit);

    dev->INTEN0.APBCMDDONE = 1;
    dev->INTEN0.APBCMDABORT = 1;
    dev->INTEN0.RXCHKSUMERR = 1;
    dev->INTEN0.RXCHKSUMPASS = 1;
    dev->INTEN0.RXPIDERR = 1;
    dev->INTEN0.RXPIDPASS = 1;

    if (cfg->op_mode == SDRV_UART_LIN_SLAVE_MODE) {
        dev->PCR6.CHKSUMTYP = cfg->chksum;
    }
}

static void sdrv_uart_irda_init(sdrv_uart_t *dev, const sdrv_uart_config_t *cfg)
{
    dev->PCR5.SIRTXPUL = cfg->tx_pul;
    dev->PCR5.SIRRXPUL = cfg->rx_pul;

    dev->PCR8.RXPOL = 1;

    dev->INTEN0.APBCMDDONE = 1;
    dev->INTEN0.APBCMDABORT = 1;
}

static void sdrv_uart_lld_disable(sdrv_uart_t *dev)
{
    dev->PCR0.TXEN = 0;
    dev->PCR0.RXEN = 0;
    dev->MCR0.MODEN = 0;
}

static void sdrv_uart_lld_enable(sdrv_uart_t *dev)
{
    dev->PCR0.TXEN = 1;
    dev->PCR0.RXEN = 1;
    dev->MCR0.MODEN = 1;
}

static bool sdrv_uart_lld_has_receive_data(sdrv_uart_t *dev)
{
    return !(dev->FSR1.EMPTY);
}

static bool sdrv_uart_lld_is_transmit_full(sdrv_uart_t *dev)
{
    return !!(dev->FSR0.FULL);
}

void sdrv_uart_lld_enable_txint(paddr_t base, bool enable)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->INTEN0.TC = enable;
}

void sdrv_uart_lld_enable_rxint(paddr_t base, bool enable)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->INTEN0.RXFWF = enable;
}

void sdrv_uart_lld_enable_rxfovf(paddr_t base, bool enable)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->INTEN0.RXFOVF = enable;
}

uint32_t sdrv_uart_lld_handle_irq(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    uint32_t state;

    state = dev->INTR0.v;

    return state;
}

void sdrv_uart_lld_clear_irq(paddr_t base, uint32_t state)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->INTR0.v = state;
}

size_t sdrv_uart_lld_transmit_async(paddr_t base, const char *data,
                                    size_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    size_t transmitted = 0;

    while ((!sdrv_uart_lld_is_transmit_full(dev)) &&
           (transmitted < size)) {

        /* Fill the FIFO from the buffer */
        dev->TXDR = *data++;

        /* Increment the send count. */
        transmitted++;
    }

    return transmitted;
}

size_t sdrv_uart_lld_receive_async(paddr_t base, char *data,
                                   size_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    size_t received = 0;

    /* Keep reading when there's character in RBR or Rx FIFO. */
    while ((received < size) && sdrv_uart_lld_has_receive_data(dev)) {
        *data++ = dev->RXDR;
        received++;
    }

    return received;
}

void sdrv_uart_lld_putc(paddr_t base, char c)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    while (sdrv_uart_lld_is_transmit_full(dev)) {
        ;
    }

    dev->TXDR = c;
}

uint8_t sdrv_uart_lld_getc(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    while (!sdrv_uart_lld_has_receive_data(dev)) {
        ;
    }
    return dev->RXDR;
}

void sdrv_uart_lld_transmit(paddr_t base, const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        sdrv_uart_lld_putc(base, data[i]);
}

void sdrv_uart_lld_receive(paddr_t base, char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        data[i] = sdrv_uart_lld_getc(base);
}

void sdrv_uart_lld_set_baudrate(paddr_t base, uint32_t baud, uint32_t clk_freq)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    dev->PCR1.BAUDRATE = clk_freq / baud;
}

uint32_t sdrv_uart_lld_get_baudrate(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    return dev->PSR1.AUTOBAUDRATE;
}

void sdrv_uart_lld_start_abr(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->PCR0.ABREN = 1;
}

void sdrv_uart_lld_stop_abr(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->PCR0.ABREN = 0;
}

void sdrv_uart_lld_enable_tx(paddr_t base, bool enable)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->PCR0.TXEN = enable;
}

void sdrv_uart_lld_enable_rx(paddr_t base, bool enable)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->PCR0.RXEN = enable;
}

void sdrv_uart_lld_init(paddr_t base, sdrv_uart_config_t *cfg)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    sdrv_uart_lld_disable(dev);

    dev->MCR0.MODRST = 1;
    while (dev->MCR0.MODRST == 1);

    dev->MCR0.OPMOD = cfg->op_mode;
    dev->PCR0.TRANSFERMODE = cfg->transfer_mode;

    dev->INTEN0.v = 0;
    dev->INTR0.v = 0xFFFFFFFF;

    sdrv_uart_lld_set_fcm(dev, &cfg->fcm);
    sdrv_uart_lld_set_databit(dev, cfg->data_bits);
    sdrv_uart_lld_set_stopbit(dev, cfg->stop_bits);
    sdrv_uart_lld_set_parity(dev, cfg->parity);

    if (cfg->abr.abr_en) {
        sdrv_uart_lld_set_abr(dev, &cfg->abr);
    }
    else {
        sdrv_uart_lld_set_baudrate(base, cfg->baud, cfg->clk_freq);
    }

    dev->FCR0.WMLVL = cfg->txwmlvl;
    dev->FCR1.WMLVL = cfg->rxmwlvl;
    dev->FCR0.CLR = 1;
    dev->FCR1.CLR = 1;

    if (cfg->dma_en) {
        dev->DMACR.TXDMARST = 1;
        while (dev->DMACR.TXDMARST == 1);

        dev->DMACR.RXDMARST = 1;
        while (dev->DMACR.RXDMARST == 1);

        dev->DMACR.TXDMAE = 1;
        dev->DMACR.RXDMAE = 1;
    }
    else {
        dev->DMACR.TXDMAE = 0;
        dev->DMACR.RXDMAE = 0;
    }

    // set rx noise filter
    //dev->PCR8.RXFILTCTL = 1;
    dev->PCR8.RXSYNCEN = 1;

    if (cfg->op_mode == SDRV_UART_LIN_MASTER_MODE ||
        cfg->op_mode == SDRV_UART_LIN_SLAVE_MODE) {
        sdrv_uart_lin_init(dev, cfg);
    }
    else if (cfg->op_mode == SDRV_UART_IRDA_MODE) {
        sdrv_uart_irda_init(dev, cfg);
    }

    if (cfg->fcm.fcm_type != SDRV_UART_RS485_FC) {
        sdrv_uart_lld_enable(dev);
    }
    else {
        dev->MCR0.MODEN = 1;
    }
}

void sdrv_uart_lld_deinit(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    /* make sure pre test case was complete */
    while (dev->PSR0.IOTX_STATE || dev->PSR0.IORX_STATE);

    dev->MCR0.MODRST = 1;
    while (dev->MCR0.MODRST == 1);

    dev->DMACR.TXDMARST = 1;
    while (dev->DMACR.TXDMARST == 1);

    dev->DMACR.RXDMARST = 1;
    while (dev->DMACR.RXDMARST == 1);

    dev->DMACR.TXDMAE = 0;
    dev->DMACR.RXDMAE = 0;

    sdrv_uart_lld_disable(dev);
}

void sdrv_uart_lld_lin_transmit(paddr_t base, uint8_t cs,
                                const char *data, uint8_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    uint8_t cnt;

    dev->CMDCSR1.TXBYTENUM = size;
    dev->CMDCSR1.RXBYTENUM = 0;

    if (dev->MCR0.OPMOD == SDRV_UART_LIN_MASTER_MODE) {
        dev->PCR6.CHKSUMTYP = cs;
    }

    dev->CMDCSR0.DOORBELL = SDRV_UART_DOORBELL_TRIGGER;

    for (cnt = 0; cnt < size; cnt++) {
        dev->TXDR = *data++;
    }
}

void sdrv_uart_lld_lin_receive(paddr_t base,
                               uint8_t cs, uint8_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    dev->CMDCSR1.TXBYTENUM = 0;
    dev->CMDCSR1.RXBYTENUM = size;

    if (dev->MCR0.OPMOD == SDRV_UART_LIN_MASTER_MODE) {
        dev->PCR6.CHKSUMTYP = cs;
    }

    dev->CMDCSR0.DOORBELL = SDRV_UART_DOORBELL_TRIGGER;
}

void sdrv_uart_lld_lin_abort(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->CMDCSR0.DOORBELL = SDRV_UART_DOORBELL_ABORT;
}

void sdrv_uart_lld_set_lin_pid(paddr_t base, uint8_t pid)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    dev->CMDCSR1.PID = pid;
}

uint8_t sdrv_uart_lld_get_lin_pid(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    return (uint8_t)(dev->CMDCSR1.PID);
}

uint8_t sdrv_uart_lld_get_lin_chksum(paddr_t base)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    return dev->PSR0.LIN_CHECKSUM;
}

void sdrv_uart_lld_sir_transmit(paddr_t base, const char *data, uint8_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;
    uint8_t cnt;

    dev->CMDCSR1.TXBYTENUM = size;
    dev->CMDCSR1.RXBYTENUM = 0;

    dev->CMDCSR0.DOORBELL = SDRV_UART_DOORBELL_TRIGGER;

    for (cnt = 0; cnt < size; cnt++) {
        dev->TXDR = *data++;
    }
}

void sdrv_uart_lld_sir_receive(paddr_t base, uint8_t size)
{
    sdrv_uart_t *dev = (sdrv_uart_t *)base;

    dev->CMDCSR1.TXBYTENUM = 0;
    dev->CMDCSR1.RXBYTENUM = size;

    dev->CMDCSR0.DOORBELL = SDRV_UART_DOORBELL_TRIGGER;
}
