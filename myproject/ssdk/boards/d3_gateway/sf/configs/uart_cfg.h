
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_UART_CFG_H
#define BOARDS_UART_CFG_H

#ifdef CONFIG_UART

#include <driver/uart.h>

/* exported uart devices */
extern uart_dev_t *g_console_uart;
extern uart_dev_t *g_dma_uart;

/* uart init functions */
extern void board_uart_early_init(void);
extern void board_uart_init(void);

#endif

#endif /* BOARDS_UART_CFG_H */
