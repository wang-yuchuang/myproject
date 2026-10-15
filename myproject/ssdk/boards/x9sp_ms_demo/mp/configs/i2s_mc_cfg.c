/*
 * i2s_mc_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level i2s mc configuration.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_SDRV_I2S_MC

#include <board.h>
#include <driver/clk.h>
#include <driver/snd/sdrv_i2s_mc.h>
#include <types.h>

#include "i2s_mc_cfg.h"

#include "clock_cfg.h"

snd_dev_t *i2s_mc1_dev;
snd_dev_t *i2s_mc2_dev;
sdrv_i2s_mc_t sdrv_i2s_mc1_dev;
sdrv_i2s_mc_t sdrv_i2s_mc2_dev;

extern dma_dev_t dma2;

static sdrv_i2s_mc_cfg_t sdrv_i2s_mc1_cfg = {
    .base = DEVICE_BASE(I2S_MC1),
    .irq = DEVICE_INTR(I2S_MC1),
    .clk = CLK_NODE(g_ckgen_ip_i2s_mc1),
    .dma_dev = &dma2,
    .loop_mode = 0,
    .clk_polar = 1,
    .tx_ch_mask = 0x02,
    .rx_ch_mask = 0x01,
};

static sdrv_i2s_mc_cfg_t sdrv_i2s_mc2_cfg = {
    .base = DEVICE_BASE(I2S_MC2),
    .irq = DEVICE_INTR(I2S_MC2),
    .clk = CLK_NODE(g_ckgen_ip_i2s_mc2),
    .dma_dev = &dma2,
    .loop_mode = 1,
    .clk_polar = 1,
    .tx_ch_mask = 0x01,
    .rx_ch_mask = 0x30,
};

void board_i2s_mc_init(void)
{
    i2s_mc1_dev = sdrv_i2s_mc_init(&sdrv_i2s_mc1_dev, &sdrv_i2s_mc1_cfg);
    i2s_mc2_dev = sdrv_i2s_mc_init(&sdrv_i2s_mc2_dev, &sdrv_i2s_mc2_cfg);
}

#endif