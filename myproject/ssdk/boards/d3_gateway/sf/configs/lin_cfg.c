/**
 * @file lin_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_LIN

#include "lin_cfg.h"

#include <types.h>
#include <param.h>
#include <board.h>
#include <clock_cfg.h>

#include <driver/taishan/lin/sdrv_lin_drv.h>

static sdrv_lin_dev_t g_sdrv_lin_dev[LIN_DEV_NUM];
lin_dev_t *g_lin_dev[LIN_DEV_NUM] = {NULL};

const lin_config_t g_lin_cfg[] = {
#ifdef CONFIG_UART2
    {
        .hwid = 1,
        .base = DEVICE_BASE(UART2),
        .irq = DEVICE_INTR(UART2),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_uart_sf_1_to_8),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
#endif
#ifdef CONFIG_UART3
    {
        .hwid = 2,
        .base = DEVICE_BASE(UART3),
        .irq = DEVICE_INTR(UART3),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_uart_sf_1_to_8),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
#endif
};

void board_lin_init(void)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(g_lin_cfg) && i < LIN_DEV_NUM; i++) {
        g_lin_dev[i] = sdrv_lin_probe(&g_sdrv_lin_dev[i]);
        lin_init(g_lin_dev[i], &g_lin_cfg[i]);
    }
}

#endif