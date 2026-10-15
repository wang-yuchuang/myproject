/**
 * @file xli_uart.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx UART low level driver. DMA, RS485 and 9 bit mode are
 * not supported.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>
#include <reg.h>

#include "xli_uart.h"

static inline uint32_t xli_uart_lld_readl(paddr_t base, uint32_t offset)
{
    return readl(base + offset);
}

static inline void xli_uart_lld_writel(paddr_t base, uint32_t offset,
                                       uint32_t val)
{
    writel(val, base + offset);
}

static void xli_uart_lld_disable(paddr_t base)
{
    uint32_t cr_val;

    cr_val = xli_uart_lld_readl(base, XLI_UART_CR_OFFSET);
    cr_val &= (~XLI_UART_CR_EN_DIS_MASK);
    cr_val |= (XLI_UART_CR_RX_DIS | XLI_UART_CR_TX_DIS);
    xli_uart_lld_writel(base, XLI_UART_CR_OFFSET, cr_val);
}

static void xli_uart_lld_enable(paddr_t base)
{
    uint32_t cr_val;

    cr_val = xli_uart_lld_readl(base, XLI_UART_CR_OFFSET);
    cr_val &= (~XLI_UART_CR_EN_DIS_MASK);
    cr_val |= (XLI_UART_CR_RX_EN | XLI_UART_CR_TX_EN);
    xli_uart_lld_writel(base, XLI_UART_CR_OFFSET, cr_val);
}

static bool xli_uart_lld_has_receive_data(paddr_t base)
{
    uint32_t sr_val = xli_uart_lld_readl(base, XLI_UART_SR_OFFSET);
    return ((sr_val & XLI_UART_SR_RXEMPTY) != XLI_UART_SR_RXEMPTY);
}

static bool xli_uart_lld_is_transmit_full(paddr_t base)
{
    uint32_t sr_val = xli_uart_lld_readl(base, XLI_UART_SR_OFFSET);
    return ((sr_val & XLI_UART_SR_TXFULL) == XLI_UART_SR_TXFULL);
}

void xli_uart_lld_enable_txint(paddr_t base, bool enable)
{
    if (enable) {
        uint32_t reg_val = xli_uart_lld_readl(base, XLI_UART_IER_OFFSET);
        xli_uart_lld_writel(base, XLI_UART_IER_OFFSET,
                            reg_val | XLI_UART_IXR_TXEMPTY);
    }
    else {
        xli_uart_lld_writel(base, XLI_UART_IDR_OFFSET,
                            XLI_UART_IXR_TXEMPTY);
    }
}

void xli_uart_lld_enable_rxint(paddr_t base, bool enable)
{
    if (enable) {
        uint32_t reg_val = xli_uart_lld_readl(base, XLI_UART_IER_OFFSET);
        xli_uart_lld_writel(base, XLI_UART_IER_OFFSET,
                            (reg_val | XLI_UART_IXR_RXFULL |
                            XLI_UART_IXR_RXEMPTY | XLI_UART_IXR_RXOVR));
    }
    else {
        xli_uart_lld_writel(base, XLI_UART_IDR_OFFSET,
                            (XLI_UART_IXR_RXFULL |
                            XLI_UART_IXR_RXEMPTY |
                            XLI_UART_IXR_RXOVR));
    }
}

uint32_t xli_uart_lld_handle_irq(paddr_t base)
{
    uint32_t isr_status;

    isr_status = xli_uart_lld_readl(base, XLI_UART_IMR_OFFSET);
    isr_status &= xli_uart_lld_readl(base, XLI_UART_ISR_OFFSET);

    return isr_status;
}

void xli_uart_lld_clear_irq(paddr_t base, uint32_t state)
{
    /* Clear the interrupt status. */
    xli_uart_lld_writel(base, XLI_UART_ISR_OFFSET, state);
}

size_t xli_uart_lld_transmit_async(paddr_t base, const char *data,
                                   size_t size)
{
    size_t transmitted = 0;

    while ((!xli_uart_lld_is_transmit_full(base)) &&
           (transmitted < size)) {

        /* Fill the FIFO from the buffer */
        xli_uart_lld_writel(base, XLI_UART_FIFO_OFFSET,
                            *data++);

        /* Increment the send count. */
        transmitted++;
    }

    return transmitted;
}

size_t xli_uart_lld_receive_async(paddr_t base, char *data,
                                  size_t size)
{
    size_t received = 0;

    /* Keep reading when there's character in RBR or Rx FIFO. */
    while ((received < size) && xli_uart_lld_has_receive_data(base)) {
        *data++ = xli_uart_lld_readl(base, XLI_UART_FIFO_OFFSET);
        received++;
    }

    return received;
}

void xli_uart_lld_putc(paddr_t base, char c)
{
    while (xli_uart_lld_is_transmit_full(base)) {
        ;
    }

    xli_uart_lld_writel(base, XLI_UART_FIFO_OFFSET, c);
}

uint8_t xli_uart_lld_getc(paddr_t base)
{
    while (!xli_uart_lld_has_receive_data(base)) {
        ;
    }
    return xli_uart_lld_readl(base, XLI_UART_FIFO_OFFSET);
}

void xli_uart_lld_transmit(paddr_t base, const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        xli_uart_lld_putc(base, data[i]);
}

void xli_uart_lld_receive(paddr_t base, char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        data[i] = xli_uart_lld_getc(base);
}

void xli_uart_lld_set_baudrate(paddr_t base, uint32_t baud, uint32_t clk_freq)
{
    uint32_t mode_reg;
    uint32_t baud_div;
    uint32_t brgr;
    uint32_t best_baud_div = 0;
    uint32_t best_brgr = 0;
    uint32_t calc_baud;
    uint32_t baud_err;
    uint32_t best_baud_err = 0xffffffff;

    mode_reg = xli_uart_lld_readl(base, XLI_UART_MR_OFFSET);

    if(mode_reg & XLI_UART_MR_CLKSEL) {
        clk_freq = clk_freq / 8;
    }

    /*
     * Determine the Baud divider. It can be 4to 254.
     * Loop through all possible combinations
     */
    for (baud_div = 4; baud_div < 255; baud_div++) {

        /* Calculate the value for BRGR register */
        brgr = clk_freq / (baud * (baud_div + 1));

        /* Calculate the baud rate from the BRGR value */
        calc_baud = clk_freq / (brgr * (baud_div + 1));

        /* Avoid unsigned integer underflow */
        if (baud > calc_baud) {
            baud_err = baud - calc_baud;
        }
        else {
            baud_err = calc_baud - baud;
        }

        /* Find the calculated baud rate closest to requested baud rate. */
        if (best_baud_err > baud_err) {

            best_brgr = brgr;
            best_baud_div = baud_div;
            best_baud_err = baud_err;
        }
    }

    /* Disable TX and RX to avoid glitches when setting the baud rate. */
    xli_uart_lld_disable(base);

    xli_uart_lld_writel(base, XLI_UART_BAUDGEN_OFFSET, best_brgr);
    xli_uart_lld_writel(base, XLI_UART_BAUDDIV_OFFSET, best_baud_div);

    /* RX and TX SW reset */
    xli_uart_lld_writel(base, XLI_UART_CR_OFFSET,
                        XLI_UART_CR_TXRST | XLI_UART_CR_RXRST);

    /* Enable device */
    xli_uart_lld_enable(base);
}

void xli_uart_lld_init(paddr_t base, const xli_uart_config_t *cfg)
{
    uint32_t mode_val;
    uint32_t data_bit_val = XLI_UART_MR_CHARLEN_8_BIT;
    uint32_t stop_bit_val = XLI_UART_MR_STOPMODE_1_BIT;
    uint32_t parity_val = XLI_UART_MR_PARITY_NONE;

    /* Set the register value to the desired data format */

    mode_val = xli_uart_lld_readl(base, XLI_UART_MR_OFFSET);

    mode_val &= (~(XLI_UART_MR_CHARLEN_MASK |
                   XLI_UART_MR_STOPMODE_MASK |
                   XLI_UART_MR_PARITY_MASK));

    if (cfg->data_bits == XLI_UART_CHAR_8_BITS) {
        data_bit_val = XLI_UART_MR_CHARLEN_8_BIT;
    }
    else if (cfg->data_bits == XLI_UART_CHAR_7_BITS) {
        data_bit_val =  XLI_UART_MR_CHARLEN_7_BIT;
    }
    else if (cfg->data_bits == XLI_UART_CHAR_6_BITS) {
        data_bit_val = XLI_UART_MR_CHARLEN_6_BIT;
    }

    if (cfg->stop_bits == XLI_UART_STOP_1_BIT) {
        stop_bit_val = XLI_UART_MR_STOPMODE_1_BIT;
    }
    else if (cfg->stop_bits == XLI_UART_STOP_1_5_BIT) {
        stop_bit_val = XLI_UART_MR_STOPMODE_1_5_BIT;
    }
    else if (cfg->stop_bits == XLI_UART_STOP_2_BIT) {
        stop_bit_val = XLI_UART_MR_STOPMODE_2_BIT;
    }

    if (cfg->parity == XLI_UART_NO_PARITY) {
        parity_val = XLI_UART_MR_PARITY_NONE;
    }
    else if (cfg->parity == XLI_UART_ODD_PARITY) {
        parity_val = XLI_UART_MR_PARITY_ODD;
    }
    else if (cfg->parity == XLI_UART_EVEN_PARITY) {
        parity_val = XLI_UART_MR_PARITY_EVEN;
    }

    mode_val |= (data_bit_val | stop_bit_val | parity_val);

    xli_uart_lld_writel(base, XLI_UART_MR_OFFSET, mode_val);

    /* Set baudrate. */
    xli_uart_lld_set_baudrate(base, cfg->baud, cfg->clk_freq);

    /* Set the RX FIFO trigger at 1 data bytes. */
    xli_uart_lld_writel(base, XLI_UART_RXWM_OFFSET, 0x01);

    /* Disable all interrupts, polled mode is the default */
    xli_uart_lld_writel(base, XLI_UART_IDR_OFFSET,
                        XLI_UART_IXR_MASK);
}
