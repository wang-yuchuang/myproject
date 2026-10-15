
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
    .base = DEVICE_BASE(UART3),
    .irq = DEVICE_INTR(UART3),
    .baud = 115200,
    .clk_freq = 80000000,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
};


/* uart early initialization */
void board_uart_early_init(void)
{
    g_console_uart = dw_uart_probe(&g_dw_console_uart, &console_uart_config);
    board_set_console(g_console_uart);

}

/* uart initialization */
void board_uart_init(void)
{
    uart_init(g_console_uart);
}


#endif
