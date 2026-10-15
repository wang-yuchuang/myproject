/**
 * @file taihang_xspi_drv.h
 * @brief taihang xspi driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef TAIHANG_XSPI_DRV_H_
#define TAIHANG_XSPI_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <reg.h>
#include <cmsis_os2.h>
#include "driver/spi_nor.h"

enum xspi_xfer_mode {
    XSPI_XFER_NONE = 0,
    XSPI_XFER_READ,
    XSPI_XFER_WRITE,
};

struct xspi_config {
    uint8_t id;
    addr_t apb_base;
    addr_t direct_base;
    unsigned int irq;
    unsigned long ref_clk;
    bool no_cache;          /**< xspi prefeth whether it is not necessary */
    bool crc_en;            /**< enable xspi crc check */
    bool xip_mode;          /**< device is xip mode */
};

struct xspi_pdata {
    uint8_t id;
    addr_t apb_base;
    addr_t direct_base;
    unsigned int irq;
    unsigned long ref_clk_hz;

    unsigned int sclk;
    uint8_t dma_bus_width;
    uint32_t dma_burst_size;
    bool cs_decoded;
    uint8_t current_cs;
    bool no_cache;
    bool crc_en;
    bool dqs_en;
    uint8_t fifo_depth;
    uint8_t fifo_width;
    uint32_t direct_range_size;

    osMutexId_t bus_mutex;
    osEventFlagsId_t bus_event;
};

/**
 * @brief xspi host init
 *
 * @param[in] host spi nor host
 * @param[in] xspi xspi private info
 * @param[in] config xspi config info
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int xspi_host_init(struct spi_nor_host *host, struct xspi_pdata *xspi,
                   struct xspi_config *config);
// int xspi_protect_clear(struct xspi_pdata *xspi);
// int xspi_protect_setup(struct xspi_pdata *xspi,
//     protect_config_t *config, uint8_t num);
// uint32_t spi_nor_get_proto(struct spi_nor *nor, enum spi_nor_rw_para ops);
// uint32_t spi_nor_read_dummy_para(struct spi_nor *nor);
// flash_size_t spi_nor_read_dev_size_para(struct spi_nor *nor);
// bool xspi_read_dqs_en(struct xspi_pdata *xspi);

#ifdef __cplusplus
}
#endif
#endif /* TAIHANG_XSPI_DRV_H_ */

