/**
 * @file ospi_mst_slv.h
 * @brief ospi master api header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef OSPI_MST_SLV_H_
#define OSPI_MST_SLV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <arch/irq.h>
#include <spinlock.h>
#include <types.h>
#include "driver/clk.h"
#include "driver/flash.h"


/**
 * @brief ospi master instance initialization interface.
 * @details This function must used after spi_nor_host_init and before other APIs.
 * @param[in, out] flash_handle ospi master instance contex handle, the data memory need alloc by user.
 * @param[in] flash_config ospi master device config data, will not used after function return, so this param can be in stack.
 * @return int
 * @retval 0: sucess
 * @retval other: failed
 */
int ospi_master_init(struct spi_nor *flash_handle, struct spi_nor_host *host,
                 const struct spi_nor_config *flash_config);

/**
 * @brief ospi master instance destruction interface.
 * @param[in] flash_handle ospi master instance contex handle.
 */
void spi_nor_deinit(struct spi_nor *flash_handle);

/**
 * @brief ospi master read interface.
 * @details When the buffer and read size aligned with cacheline size will provide best performance.
 * @param[in] flash_handle ospi master instance contex handle.
 * @param[in] addr ospi master addr.
 * @param[in] buf Read buffer, alloc by user.
 * @param[in] size Read size, need aligned with 4bytes for dma limit.
 * @return int
 * @retval 0: sucess
 * @retval other: failed
 */
int ospi_master_read(struct spi_nor *flash_handle, flash_addr_t addr, uint8_t *buf,
                 flash_size_t size);

/**
 * @brief ospi master write interface.
 * @details When the buffer and write size aligned with cacheline size will provide best performance.
 * @param[in] flash_handle ospi master instance contex handle.
 * @param[in] addr ospi master addr.
 * @param[in] buf Write buffer, alloc by user.
 * @param[in] size Read size, need aligned with 4bytes for dma limit.
 * @return int
 * @retval 0: sucess
 * @retval other: failed
 */
int ospi_master_write(struct spi_nor *flash_handle, flash_addr_t addr,
                  const uint8_t *buf, flash_size_t size);

#ifdef __cplusplus
}
#endif
#endif /* SPI_NOR_H_ */
