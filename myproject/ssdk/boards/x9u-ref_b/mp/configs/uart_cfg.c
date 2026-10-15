
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_UART

#include "uart_cfg.h"

#include <types.h>
#include <board.h>

/* uart device driver headers */
#include <driver/kunlun/uart/dw_uart_drv.h>

/* uart device drivers */
dw_uart_dev_t g_dw_console_uart;
uart_dev_t *g_console_uart;


/* uart config structures */
static const uart_config_t console_uart_config = {
    .base = DEVICE_BASE(UART10),
    .irq = DEVICE_INTR(UART10),
    .baud = 115200,
    .clk_freq = 60000000,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
};

extern uart_ops_t g_dw_uart_ops;

/* uart early initialization */
void board_uart_early_init(void)
{
    dw_uart_dev_t *dw_uart = &g_dw_console_uart;
	const uart_config_t *cfg = &console_uart_config;
    uart_dev_t *dev = &dw_uart->dev;

    dev->ops = &g_dw_uart_ops;
    dev->priv = dw_uart;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->baud = cfg->baud;
    dev->clk_freq = cfg->clk_freq;

    dev->tx_cb = NULL;
    dev->tx_ptr = NULL;
    dev->tx_remain = 0;

    dev->rx_cb = NULL;
    dev->rx_ptr = NULL;
    dev->rx_remain = 0;

    dev->rxfovf_cb = NULL;
    dev->rxfovf_arg = NULL;

    g_console_uart = dev;
    //g_console_uart = dw_uart_probe(&g_dw_console_uart, &console_uart_config);
    board_set_console(g_console_uart);

}

/* uart initialization */
void board_uart_init(void)
{
    // uart_init(g_console_uart);
}


#endif
