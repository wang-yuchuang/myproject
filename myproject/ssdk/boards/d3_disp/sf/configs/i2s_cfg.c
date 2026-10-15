/**
 * @file i2s_cfg.c
 * @brief
 * @version 0.1
 * @date 2021-12-04
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifdef CONFIG_SACI_I2S

#include <board.h>
#include <driver/clk.h>
#include <taishan/snd/i2s/sdrv_saci_i2s.h>
#include <types.h>

#include "i2s_cfg.h"

#include "clock_cfg.h"

snd_dev_t *i2s1_dev;
snd_dev_t *i2s2_dev;
saci_i2s_t saci1_i2s_dev;
saci_i2s_t saci2_i2s_dev;

extern dma_dev_t dma_ap0;

static saci_i2s_cfg_t saci1_i2s_cfg = {
    .base = DEVICE_BASE(SACI1),
    .sync_mode = CLK_TX_SYNC,
    .mclk_sel=SACI_MCLK_1,
    .mclk_dir=SACI_MCLK_OUT,
    .irq = DEVICE_INTR(SACI1_I2S),
    .clk = CLK_NODE(g_ckgen_ip_saci1_clk),
    .id = 0,
    .tx_chsel = SACI_SEL_DATALINE0,
    .rx_chsel = SACI_SEL_DATALINE1,
    .dma_dev = &dma_ap0, 
};

static saci_i2s_cfg_t saci2_i2s_cfg = {
    .base = DEVICE_BASE(SACI2),
    .sync_mode = CLK_TX_SYNC,
    .irq = DEVICE_INTR(SACI2_I2S),
    .clk = CLK_NODE(g_ckgen_ip_saci2_clk),
    .id = 0,
    .tx_chsel = SACI_SEL_DATALINE1,
    .rx_chsel = SACI_SEL_DATALINE0,
    .dma_dev = &dma_ap0,
};

/**
 * @brief
 *
 */
void board_i2s_init(void)
{
    i2s1_dev = saci_i2s_probe(&saci1_i2s_dev, &saci1_i2s_cfg);
    i2s2_dev = saci_i2s_probe(&saci2_i2s_dev, &saci2_i2s_cfg);
}

#endif
