/**
 * @file dw_uart.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: DW UART low level driver. DMA, RS485 and 9 bit mode are
 * not supported.
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <reg.h>
#include <string.h>
#include <types.h>
#include "dw_uart.h"
#ifdef CONFIG_SDRV_VIRT_UART
#include "driver/kunlun/virt_uart/virt_uart.h"
#endif
#ifdef CONFIG_SUPPORT_LOG_TIMESTAMP
#include "driver/kunlun/timestamp/timestamp.h"
#endif

#if defined (CONFIG_SUPPORT_LOG_TIMESTAMP) || defined (CONFIG_SDLOGD) || defined (CONFIG_SDSHELL_BACKEND)
static char last_char = '\n';
#if defined (CONFIG_SDLOGD) || defined (CONFIG_SDSHELL_BACKEND)
__WEAK bool is_sdlogd_running(void) { return false; }
__WEAK bool is_sdshell_backend_running(void) { return false; }
static const char* mp_tag = "[mpc]";
#ifdef CONFIG_SDLOGD
extern bool is_sdlogd_running(void);
#endif
#ifdef CONFIG_SDSHELL_BACKEND
extern bool is_sdshell_backend_running(void);
#endif
#endif
#endif

static bool fifo_enabled(dw_apb_uart_t *uart)
{
    return uart->IIR.FIFOSE == 0x3;
}

static void write_fcr(dw_apb_uart_t *uart, uint32_t fcr)
{
    uart->FCR.v = fcr;
}

static void config_dls(dw_apb_uart_t *uart, dw_uart_dls_t dls)
{
    switch (dls) {
        case DW_UART_CHAR_5_BITS:
        case DW_UART_CHAR_6_BITS:
        case DW_UART_CHAR_7_BITS:
        case DW_UART_CHAR_8_BITS:
            uart->LCR.DLS = (uint32_t)dls;
            break;

        default:
            ASSERT(false);
            break;
    }
}

static void config_stop_bits(dw_apb_uart_t *uart, dw_uart_stop_t stop_bits)
{
    switch (stop_bits) {
        case DW_UART_STOP_1_BIT:
            uart->LCR.STOP = 0;
            break;

        case DW_UART_STOP_1_5_BIT:
            ASSERT(uart->LCR.DLS == 0);
            uart->LCR.STOP = 1;
            break;

        case DW_UART_STOP_2_BIT:
            ASSERT(uart->LCR.DLS != 0);
            uart->LCR.STOP = 1;
            break;

        default:
            ASSERT(false);
            break;
    }
}

static void config_parity(dw_apb_uart_t *uart, dw_uart_parity_t parity)
{
    switch (parity) {
        case DW_UART_NO_PARITY:
            uart->LCR.PEN = 0;
            break;

        case DW_UART_ODD_PARITY:
            uart->LCR.EPS = 0;
            uart->LCR.PEN = 1;
            break;

        case DW_UART_EVEN_PARITY:
            uart->LCR.EPS = 1;
            uart->LCR.PEN = 1;
            break;

        default:
            ASSERT(false);
            break;
    }
}

void dw_uart_lld_enable_txint(paddr_t base, bool enable)
{
    if (0 == base) return;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    uart->IER.ETBEI = (uint32_t)enable;
}

void dw_uart_lld_enable_rxint(paddr_t base, bool enable)
{
    if (0 == base) return;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    uart->IER.ERBFI = (uint32_t)enable;
}

uint32_t dw_uart_lld_handle_irq(paddr_t base)
{
    if (0 == base) return 0;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    return uart->IIR.IID;
}

size_t dw_uart_lld_transmit_async(paddr_t base, const char *data,
                                  size_t size)
{
    if (0 == base) return 0;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    size_t transmitted = 0;

    if (fifo_enabled(uart)) {
        while (transmitted < size && uart->USR.TFNF) {
            writeb(*data++, &uart->THR);
            transmitted++;
        }
    }
    else if (uart->LSR.THRE) {
        writeb(*data++, &uart->THR);
        transmitted++;
    }

    return transmitted;
}

size_t dw_uart_lld_receive_async(paddr_t base, char *data,
                                 size_t size)
{
    if (0 == base) return 0;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    size_t received = 0;

    while ((received < size) && uart->LSR.DR) {
        *data++ = (uint8_t)uart->RBR.RBR8;
        received++;
    }

    return received;
}

void dw_uart_lld_raw_putc(paddr_t base, char c)
{
    if (0 == base) return;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;

    if (fifo_enabled(uart))
        while (!uart->USR.TFNF);
    else
        while (!uart->LSR.THRE);

    writeb(c, &uart->THR);
}

static inline void _dw_uart_lld_putc(paddr_t base, char c)
{
#ifdef CONFIG_SDRV_VIRT_UART
    if (virt_uart_is_available()) {
        if (c == '\n')
            virt_uart_putc('\r');

        virt_uart_putc(c);
    }
#endif

    if (0 == base) return;
    dw_uart_lld_raw_putc(base, c);
}

void dw_uart_lld_putc(paddr_t base, char c)
{
#if defined (CONFIG_SDLOGD) || defined (CONFIG_SDSHELL_BACKEND)
    if (c && ((last_char == '\n') && (c != '>'))) {
        if (is_sdlogd_running() || is_sdshell_backend_running()) {
            for (int i = 0; i < strlen(mp_tag); i++) {
                _dw_uart_lld_putc(base, mp_tag[i]);
            }
        }
    }
#endif

#ifdef CONFIG_SUPPORT_LOG_TIMESTAMP
   int i, len;
   char timestamp[21] = {0};

#ifdef CONFIG_SDSHELL_BACKEND
    if (!is_sdshell_backend_running()) {
#endif
       if (c && ((last_char == '\n') && (c != '>'))) {
           len = snprintf(timestamp, 21, "[%10llu us]", semidrive_get_systime_us());
           for (i = 0; i < len; i++)
                _dw_uart_lld_putc(base, timestamp[i]);
       }
#ifdef CONFIG_SDSHELL_BACKEND
    }
#endif
#endif

    _dw_uart_lld_putc(base, c);

#if defined (CONFIG_SUPPORT_LOG_TIMESTAMP) || defined (CONFIG_SDLOGD)
    last_char = c;
#endif
}

uint8_t dw_uart_lld_getc(paddr_t base)
{
    if (0 == base) return 0;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;

    /* Wait for at least one character in the RBR or Rx FIFO. */
    while (!uart->LSR.DR);
    /* Read one character from RBR or Rx FIFO. */
    return uart->RBR.RBR8;
}

uint8_t dw_uart_lld_get_rbr(paddr_t base)
{
    if (0 == base) return 0;

    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;

    return uart->RBR.RBR8;
}

void dw_uart_lld_transmit(paddr_t base, const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        dw_uart_lld_putc(base, data[i]);
}

void dw_uart_lld_raw_transmit(paddr_t base, const char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        dw_uart_lld_raw_putc(base, data[i]);
}

void dw_uart_lld_receive(paddr_t base, char *data, size_t size)
{
    if (0 == base) return;
    for (size_t i = 0; i < size; i++)
        data[i] = dw_uart_lld_getc(base);
}

#define DIV_ROUND_CLOSEST(x, divisor)                                          \
    ({                                                                         \
        (((x - 1) > 0) || ((divisor)-1) > 0 || ((x > 0) == (divisor > 0)))     \
            ? (((x) + ((divisor) / 2)) / (divisor))                            \
            : (((x) - ((divisor) / 2)) / (divisor));                           \
    })

void dw_uart_lld_set_baudrate(paddr_t base, uint32_t baud, uint32_t clk_freq)
{
    if (0 == base)
        return;
    dw_apb_uart_t *uart = (dw_apb_uart_t *)base;
    uint32_t divisor = (clk_freq * 1.0) / (16 * baud);
    uint32_t frac = ((clk_freq * 1.0) / (16 * baud) - divisor) * 10000;
    frac = DIV_ROUND_CLOSEST(frac * 16, 10000);
    uart->LCR.DLAB = 1;
    uart->DLL.DLL = divisor & 0xff;
    uart->DLH.DLH = (divisor >> 8) & 0xff;
    uart->DLF.DLF = (frac & 0xf);
    uart->LCR.DLAB = 0;
}

void dw_uart_lld_enable_fifo(paddr_t base, uint32_t *fcs_shadow, bool enable)
{
    if (0 == base) return;
    FCR_Type *fcr = (FCR_Type *)fcs_shadow;

    if (enable) {
        /* Reset Rx & Tx FIFOs. */
        fcr->XFIFOR = 1;
        fcr->RFIFOR = 1;
        write_fcr((dw_apb_uart_t *)base, *fcs_shadow);

        /* The FIFO reset bits are self cleared. */
        fcr->XFIFOR = 0;
        fcr->RFIFOR = 0;

        /* Trigger Rx interrupt when there's one character in Rx FIFO. */
        fcr->RT = 0;
        /* Trigger Tx interrupt when Tx FIFO is empty. */
        fcr->TET = 0;
        /* Enable Rx/Tx FIFOs. */
        fcr->FIFOE = 1;
    }
    else {
        /* Disable Rx/Tx FIFOs. */
        fcr->FIFOE = 0;
    }

    write_fcr((dw_apb_uart_t *)base, *fcs_shadow);
}

void dw_uart_lld_init(paddr_t base, const dw_uart_config_t *cfg)
{
    if (0 == base) return;
    write_fcr((dw_apb_uart_t *)base, 0);     /* FCR = 0 on reset */

    config_dls((dw_apb_uart_t *)base, cfg->data_bits);
    config_stop_bits((dw_apb_uart_t *)base, cfg->stop_bits);
    config_parity((dw_apb_uart_t *)base, cfg->parity);

    dw_uart_lld_set_baudrate(base, cfg->baud, cfg->clk_freq);
}
