/**
 * @file cdns_ospi_drv.h
 * @brief Cadence ospi driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef CDNS_OSPI_DRV_H_
#define CDNS_OSPI_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <cmsis_os2.h>
#include <driver/spi_nor.h>
#include <driver/ospi_mst_slv.h>

enum cospi_flash_phy_mode {
    COSPI_PHY_NONE = 0,
    COSPI_PHY_BYPASS_DLL,
    COSPI_PHY_MASTER_DLL,
};

enum cospi_xfer_mode {
    COSPI_XFER_NONE = 0,
    COSPI_XFER_READ,
    COSPI_XFER_WRITE,
};

struct cospi_config {
    uint8_t id;
    addr_t apb_base;
    addr_t ahb_base;
    unsigned int irq;
    unsigned long ref_clk;
    bool xip_mode;                 /**< firmware XIPs on this controller (skip hw reinit) */
};

struct cospi_pdata {
#if 1
    uint8_t id;
    addr_t apb_base;
    addr_t ahb_base;
    unsigned int irq;
    unsigned long ref_clk_hz;
#endif

    unsigned int sclk;
    uint8_t dma_bus_width;
    uint32_t dma_burst_size;
    uint32_t dma_max_burst;
    bool cs_decoded;
    uint8_t current_cs;
    uint8_t block_power_index;
    uint32_t page_size;
    bool dqs_en;
    bool rclk_loopback;
    uint8_t master_delay;
    uint8_t capture_delay;

    uint8_t *xfer_buf;
    uint32_t remaining_size;

    uint8_t inst_type;
    uint8_t addr_bytes;
    uint8_t sram_size_nbit;
    uint8_t fifo_depth;
    uint8_t fifo_width;
    uint32_t trigger_address;
    uint32_t trigger_range_size;
    enum cospi_flash_phy_mode phy_mode;
    bool phy_training_succ;

    osMutexId_t bus_mutex;
    osEventFlagsId_t bus_event;
};

int cospi_host_init(struct spi_nor_host *host, struct cospi_pdata *cospi,
                    struct cospi_config *config);

#ifdef __cplusplus
}
#endif
#endif /* CDNS_OSPI_DRV_H_ */
