/*
 * spdif_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level i2s mc configuration.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_SDRV_SPDIF

#include <board.h>
#include <driver/clk.h>
#include <driver/snd/sdrv_spdif.h>
#include <types.h>

#include "spdif_cfg.h"

#include "clock_cfg.h"

snd_dev_t *spdif1_dev;
snd_dev_t *spdif2_dev;
sdrv_spdif_t sdrv_spdif1_dev;
sdrv_spdif_t sdrv_spdif2_dev;

extern dma_dev_t dma2;

static sdrv_spdif_cfg_t sdrv_spdif1_cfg = {
    .base = DEVICE_BASE(SPDIF1),
    .irq = DEVICE_INTR(SPDIF1),
    .clk = CLK_NODE(g_ckgen_ip_spdif1),
    .dma_dev = &dma2,
};

static sdrv_spdif_cfg_t sdrv_spdif2_cfg = {
    .base = DEVICE_BASE(SPDIF2),
    .irq = DEVICE_INTR(SPDIF2),
    .clk = CLK_NODE(g_ckgen_ip_spdif2),
    .dma_dev = &dma2,
};

void board_spdif_init(void)
{
    spdif1_dev = sdrv_spdif_init(&sdrv_spdif1_dev, &sdrv_spdif1_cfg);
    spdif2_dev = sdrv_spdif_init(&sdrv_spdif2_dev, &sdrv_spdif2_cfg);
}

#endif