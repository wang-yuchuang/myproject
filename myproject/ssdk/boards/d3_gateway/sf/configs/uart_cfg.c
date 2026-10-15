
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
#include <clock_cfg.h>
#ifdef CONFIG_CU
#include <mbox_cfg.h>
#include <rstgen_cfg.h>
#include <driver/reset.h>
#include <core_shm_cfg.h>
#include <arch/cache.h>
#endif

/* uart device driver headers */
#include <driver/taishan/uart/sdrv_uart_drv.h>

/* uart device drivers */
sdrv_uart_dev_t g_sdrv_console_uart;
uart_dev_t *g_console_uart;

sdrv_uart_dev_t g_sdrv_dma_uart;
uart_dev_t *g_dma_uart;

/* uart config structures */
static uart_config_t console_uart_config = {
    .base = DEVICE_BASE(UART7),
    .irq = DEVICE_INTR(UART7),
    .baud = 115200,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
};

static uart_config_t dma_uart_config = {
    .base = DEVICE_BASE(UART1),
    .irq = DEVICE_INTR(UART1),
    .baud = 115200,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
    .dma_en = true,
    .txwmlvl = 0xf,
    .rxmwlvl = 0x0,
};

/* uart early initialization */
void board_uart_early_init(void)
{
#ifdef CONFIG_CU
    if (board_mbox_get_coreid() == MBOX_RPROC_SF) {
        g_console_occupid = true;
        g_coreshm->uart_base = console_uart_config.base;
        g_coreshm->uart_irq = console_uart_config.irq;
        arch_clean_invalidate_cache_range((addr_t)g_coreshm, sizeof(struct core_shm_param));
    }
    else {
        arch_invalidate_cache_range((addr_t)g_coreshm, sizeof(struct core_shm_param));
        console_uart_config.base = g_coreshm->uart_base;
        console_uart_config.irq = g_coreshm->uart_irq;
        console_uart_config.hw_inited = true;
    }
#endif

    console_uart_config.clk_freq = clk_get_rate(CLK_NODE(g_ckgen_ip_uart_sf_1_to_8));
    g_console_uart = sdrv_uart_probe(&g_sdrv_console_uart, &console_uart_config);
    board_set_console(g_console_uart);

    dma_uart_config.clk_freq = clk_get_rate(CLK_NODE(g_ckgen_ip_uart_sf_1_to_8));
    g_dma_uart = sdrv_uart_probe(&g_sdrv_dma_uart, &dma_uart_config);
}

/* uart initialization */
void board_uart_init(void)
{
    uart_init(g_console_uart);
    uart_init(g_dma_uart);
}


#endif
