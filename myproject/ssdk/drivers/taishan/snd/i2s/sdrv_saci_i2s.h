/**
 * @file sdrv_saci_i2s.h
 * @version 0.1
 * @date 2022-03-02
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */
#ifndef SDRV_SACI_I2SH_
#define SDRV_SACI_I2SH_

#include <cmsis_os2.h>
#include <stdbool.h>
#include <types.h>

#include "driver/snd_driver.h"

#define SACI_SEL_CHNULL 0x00
#define SACI_SEL_DATALINE0 (0x01 << 0)
#define SACI_SEL_DATALINE1 (0x01 << 1)
#define SACI_SEL_DATALINE2 (0x01 << 2)
#define SACI_SEL_DATALINE3 (0x01 << 3)
#define SACI_SEL_DATALINE4 (0x01 << 4)
#define SACI_SEL_DATALINE5 (0x01 << 5)
#define SACI_SEL_DATALINE6 (0x01 << 6)
#define SACI_SEL_DATALINE7 (0x01 << 7)

typedef enum clk_sync_loop {
    CLK_TX_SYNC = 0,
    CLK_RX_SYNC,
    CLK_TX_RX_ASYNC,
} clk_sync_loop_t;

typedef enum saci_mclk_dir {
    SACI_MCLK_IN = 0,
    SACI_MCLK_OUT,
} saci_mclk_dir;

typedef enum saci_mclk_sel {
    SACI_MCLK_NULL = 0,
    SACI_MCLK_0,
    SACI_MCLK_1,
} saci_mclk_sel_t;

typedef struct saci_i2s_cfg {
    paddr_t base;
    int id;
    uint32_t irq;
    uint32_t clk_rate;
    clk_node_t *clk;
    dma_dev_t *dma_dev;
    clk_sync_loop_t sync_mode;
    uint8_t tx_chsel;
    uint8_t rx_chsel;
    saci_mclk_dir mclk_dir;
    saci_mclk_sel_t mclk_sel;
} saci_i2s_cfg_t;

typedef struct saci_i2s {
    snd_dev_t dev;
    clk_sync_loop_t sync_mode;
    uint8_t tx_chsel;
    uint8_t rx_chsel;
    unsigned int tx_slots;
    unsigned int rx_slots;
    unsigned int tx_slot_width;
    unsigned int rx_slot_width;
    bool tx_slave;
    bool rx_slave;
    unsigned tx_threshold;
    unsigned rx_threshold;
    unsigned tx_slot_mask;
    unsigned rx_slot_mask;
    bool tx_dma;
    bool rx_dma;
    bool pack_mode;
    bool capture_active;  /*capture  stream is in use */
    bool playback_active; /*playback stream is in use */
    bool tx_clk_enable;   /*tx clk is enable */
    bool rx_clk_enable;   /*rx clk is in enable */
    uint32_t clk_rate;
    saci_mclk_dir mclk_dir;
    saci_mclk_sel_t mclk_sel;
} saci_i2s_t;

snd_dev_t *saci_i2s_probe(saci_i2s_t *saci_i2s, saci_i2s_cfg_t *cfg);

#endif /* SDRV_SACI_I2SH_ */
