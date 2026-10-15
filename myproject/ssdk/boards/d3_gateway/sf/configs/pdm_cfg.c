/**
 * @file pdm_cfg.c
 * @brief
 * @version 0.1
 * @date 2021-12-04
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifdef CONFIG_SACI_PDM

#include <board.h>
#include <driver/clk.h>
#include <taishan/snd/pdm/sdrv_saci_pdm.h>
#include <types.h>

#include "clock_cfg.h"

snd_dev_t *pdm1_dev;
snd_dev_t *pdm2_dev;
saci_pdm_t saci1_pdm_dev;
saci_pdm_t saci2_pdm_dev;
extern dma_dev_t dma_ap0;

static saci_pdm_cfg_t saci1_pdm_cfg = {
    .base = DEVICE_BASE(SACI1),
    .irq = DEVICE_INTR(SACI1_PDM),
    .clk = CLK_NODE(g_ckgen_ip_saci1_pdm_clk),
    .channel = PDM_CHAN_LEFT,
    .dma_dev = &dma_ap0,
    .id = 0,
    .gain = SPDM_GAIN_P3DB,
};

static saci_pdm_cfg_t saci2_pdm_cfg = {
    .base = DEVICE_BASE(SACI2),
    .irq = DEVICE_INTR(SACI2_PDM),
    .clk = CLK_NODE(g_ckgen_ip_saci2_pdm_clk),
    .channel = PDM_CHAN_LEFT,
    .dma_dev = &dma_ap0,
    .gain = SPDM_GAIN_P3DB,
    .id = 0,
};

/**
 * @brief
 *
 */
void board_pdm_init(void)
{

    pdm1_dev = saci_pdm_probe(&saci1_pdm_dev, &saci1_pdm_cfg);
    pdm2_dev = saci_pdm_probe(&saci2_pdm_dev, &saci2_pdm_cfg);
}

#endif
