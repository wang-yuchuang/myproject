/**
 * @file  sdrv_dma.h
 * @brief dma hardware api header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 */
#ifndef __SDRV_DMA_H__
#define __SDRV_DMA_H__

#include <lib/list.h>
#include <types.h>

#include "driver/dma.h"

/**
 * @brief dmac mux direction
 */
typedef enum {
    DMA_MUX_RD = 0x1, /* RD PERi to MEM -> RX */
    DMA_MUX_WR = 0x2, /* MEM to PERI -> TX */
    DMA_MUX_BOTH = (DMA_MUX_RD | DMA_MUX_WR),
} DMA_MUX_DIR;

/**
 * @brief dmac mux port
 */
typedef struct sdrv_dma_mux_param {
    uint32_t port;
    uint64_t start_addr;
    uint64_t len;
    DMA_MUX_DIR direct;
} sdrv_dma_mux_param_t;

/**
 * @brief dmac transfer addr capability
 */
typedef struct sdrv_dma_addr_range {
    uint64_t base_addr;
    uint64_t len;
} sdrv_dma_addr_range_t;

/**
 * @brief dmac hardware config
 */
typedef struct sdrv_dma_cfg {
    sdrv_dma_addr_range_t *addr_table;
    uint32_t addr_table_len;
    sdrv_dma_mux_param_t *mux_table;
    uint32_t mux_table_len;
    uint32_t *irq_id_table;
    uint32_t irq_num;
} sdrv_dma_cfg_t;

/**
 * @brief sdrv_dma init
 *
 * @param[in] dev dma control instance
 * @param[in] cfg dma control config,see sdrv_dma_cfg_t
 */
void sdrv_dma_init(dma_dev_t *dev, sdrv_dma_cfg_t *cfg);

#endif