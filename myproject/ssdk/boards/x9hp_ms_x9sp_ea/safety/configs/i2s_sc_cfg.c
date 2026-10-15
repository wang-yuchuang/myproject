#ifdef CONFIG_CDNS_I2S_SC
#include <board.h>
#include <driver/clk.h>
#include "kunlun/snd/i2s/cdns_i2s_sc.h"
#include <types.h>
#include <driver/scr.h>
#include <scr_hw.h>

#include "i2s_sc_cfg.h"

#include "clock_cfg.h"

snd_dev_t *i2s_sc1_dev;
snd_dev_t *i2s_sc3_dev;
snd_dev_t *i2s_sc6_dev;

cdns_i2s_sc_drv_t cdns_i2s_sc1_dev;
cdns_i2s_sc_drv_t cdns_i2s_sc3_dev;
cdns_i2s_sc_drv_t cdns_i2s_sc6_dev;
extern dma_dev_t dma1;
extern dma_dev_t dma2;
extern scr_dev_t g_scr_safety;

static cdns_i2s_sc_cfg_t cdns_i2s_sc1_cfg = {
    .base = DEVICE_BASE(I2S_SC1),
    .irq = DEVICE_INTR(I2S_SC1),
    .clk = CLK_NODE(g_ckgen_ip_i2s_sc1),
    .dma_dev = &dma1,
    .is_master_falling_sck_polar = 1,
    .is_slave_falling_sck_polar = 1,
    .is_full_duplex = 1,
    .is_dma_mode = 1,
    .is_pack_en = 1,
    .is_tdm_use_rx_last_slot = 0,
};

static cdns_i2s_sc_cfg_t cdns_i2s_sc3_cfg = {
    .base = DEVICE_BASE(I2S_SC3),
    .irq = DEVICE_INTR(I2S_SC3),
    .clk = CLK_NODE(g_ckgen_ip_i2s_sc3),
    .dma_dev = &dma2,
    .is_master_falling_sck_polar = 1,
    .is_slave_falling_sck_polar = 1,
    .is_full_duplex = 1,
    .is_dma_mode = 1,
    .is_pack_en = 1,
    .is_tdm_use_rx_last_slot = 0,
};

void board_i2s_sc_init(void)
{
    const scr_signal_t i2s_sc12_signal = SCR_SAFETY__pio_i2s_sc12_oectrl_1_0;

    scr_set(&g_scr_safety, &i2s_sc12_signal,0x03);

    i2s_sc1_dev = cdns_i2s_sc_probe(&cdns_i2s_sc1_dev, &cdns_i2s_sc1_cfg);
    i2s_sc3_dev = cdns_i2s_sc_probe(&cdns_i2s_sc3_dev, &cdns_i2s_sc3_cfg);
}
#endif
