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

#include <board.h>
#include <clock_cfg.h>
#include <driver/taihang/sdrv_lin/sdrv_lin_drv.h>
#include <param.h>
#include <types.h>

#include "lin_cfg.h"

static sdrv_lin_dev_t g_sdrv_lin_dev[LIN_DEV_NUM];
lin_dev_t *g_lin_dev[LIN_DEV_NUM] = {NULL};

const lin_config_t g_lin_cfg[] = {
    {
        .hwid = 1,
        .base = DEVICE_BASE(LIN1),
        .irq = DEVICE_INTR(LIN1),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 2,
        .base = DEVICE_BASE(LIN2),
        .irq = DEVICE_INTR(LIN2),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 3,
        .base = DEVICE_BASE(LIN3),
        .irq = DEVICE_INTR(LIN3),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 4,
        .base = DEVICE_BASE(LIN4),
        .irq = DEVICE_INTR(LIN4),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 5,
        .base = DEVICE_BASE(LIN5),
        .irq = DEVICE_INTR(LIN5),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 6,
        .base = DEVICE_BASE(LIN6),
        .irq = DEVICE_INTR(LIN6),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 7,
        .base = DEVICE_BASE(LIN7),
        .irq = DEVICE_INTR(LIN7),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },
    {
        .hwid = 8,
        .base = DEVICE_BASE(LIN8),
        .irq = DEVICE_INTR(LIN8),
        .baud = 19200,
        .clk = CLK_NODE(g_ckgen_ip_lin),
        .mode = LIN_MASTER,
        .cs_mode = LIN_CLASSIC_CS,
    },

};

void board_lin_init(void)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(g_lin_cfg) && i < LIN_DEV_NUM; i++) {
        g_lin_dev[i] = sdrv_lin_probe(&g_sdrv_lin_dev[i]);
        lin_init(g_lin_dev[i], &g_lin_cfg[i]);
    }
}

#endif