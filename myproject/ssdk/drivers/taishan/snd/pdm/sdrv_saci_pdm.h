/**
 * @file sdrv_saci_pdm.h
 * @brief sdrv_saci_pdm header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SDRV_SACI_PDM_H_
#define SDRV_SACI_PDM_H_

#include <cmsis_os2.h>
#include <stdbool.h>
#include <types.h>

#include "driver/snd_driver.h"

typedef enum pdm_channel {
    PDM_CHAN_LEFT = 0x00,
    PDM_CHAN_RIGHT = 0x01,
    PDM_CHAN_STEREO = 0x02,
} pdm_channel_t;

typedef enum pdm_gain {
    PDM_GAIN_N3DB = 0x01,
    PDM_GAIN_N6DB = 0x02,
    PDM_GAIN_N9DB = 0x03,
    PDM_GAIN_N12DB = 0x04,
    PDM_GAIN_P0DB = 0x05,
    SPDM_GAIN_P3DB = 0x06,
    PDM_GAIN_P6DB = 0x07,
    PDM_GAIN_P9DB = 0x08,
    PDM_GAIN_P12DB = 0x09,
} pdm_gain_t;

typedef struct saci_pdm_cfg {
    paddr_t base;
    int id;
    uint32_t irq;
    clk_node_t *clk;
    dma_dev_t *dma_dev;
    pdm_gain_t gain;
    pdm_channel_t channel;
} saci_pdm_cfg_t;

typedef struct saci_pdm {
    snd_dev_t dev;
    int id;
    unsigned threshold;
    uint32_t clk_rate;
    bool dma_mode;
    bool pack_mode;
    bool data_lsb;
    pdm_gain_t gain;
    pdm_channel_t channel;
} saci_pdm_t;

snd_dev_t *saci_pdm_probe(saci_pdm_t *saci_pdm, saci_pdm_cfg_t *cfg);

#endif /* SDRV_SACI_PDM_H_ */
