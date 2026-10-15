/**
 * @file cdns_ospi_drv.c
 * @brief Cadence ospi host high level driver C code.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <string.h>
#include <param.h>
#include <cmsis_os2.h>
#include <arch/cache.h>
#include <driver/irq.h>

#include "driver/kunlun/spi_nor/cdns_ospi_drv.h"
#include "cdns_ospi.h"

#define SPI_NOR_XFER_COMPLETE (0x1u)

static int cospi_irq_handler(uint32_t irq, void *arg)
{
    uint32_t status;
    uint32_t xfer_size;

    struct spi_nor_host *host = arg;
    struct cospi_pdata *cospi = host->priv_data;
    struct spi_nor *nor = host->dev;

    status = cospi_lld_irq_handler(cospi);

    if ((status & COSPI_REG_IRQ_ECC_ERR) != 0u) {
        nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
        ssdk_printf(SSDK_ERR, "ospi ecc error detected!\n");
    }

    if ((status & (COSPI_REG_IRQ_IND_COMP)) != 0u) {
        cospi_lld_tx_complete(cospi);
        cospi_lld_rx_complete(cospi);
        nor->xfer_info_bottom.opt_result = FLASH_OPT_COMPLETE;

        if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
            cospi_lld_dma_enable(cospi, false);
            nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;

            if (nor->async_mode) {
                if (nor->event_handler)
                    nor->event_handler(nor->xfer_info.opt_type,
                                       nor->xfer_info.opt_result);
            }
            else {
                osEventFlagsSet(cospi->bus_event, SPI_NOR_XFER_COMPLETE);
            }
        }
    }

    if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
        return 0;
    }

    if ((status & (COSPI_REG_IRQ_WATERMARK | COSPI_REG_IRQ_IND_SRAM_FULL)) != 0u) {
        xfer_size =
            cospi_lld_read_sram(cospi, nor->xfer_info_bottom.buf,
                                nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += xfer_size;
        nor->xfer_info_bottom.size -= xfer_size;

        xfer_size = cospi_lld_write_sram(cospi, nor->xfer_info_bottom.buf,
                                         nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += xfer_size;
        nor->xfer_info_bottom.size -= xfer_size;
    }

    if (nor->xfer_info_bottom.opt_result != FLASH_OPT_PENDING) {
        nor->xfer_info.size = nor->xfer_info_bottom.size;
        nor->xfer_info.opt_result = nor->xfer_info_bottom.opt_result;

        if (nor->async_mode) {
            if (nor->event_handler)
                nor->event_handler(nor->xfer_info.opt_type,
                                   nor->xfer_info.opt_result);
        }
        else {
            osEventFlagsSet(cospi->bus_event, SPI_NOR_XFER_COMPLETE);
        }
    }

    return 0;
}

static int cospi_rx_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint32_t rx_size;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->xfer_info_bottom.size > 0u) {
        rx_size =
            cospi_lld_read_sram(cospi, nor->xfer_info_bottom.buf,
                                nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += rx_size;
        nor->xfer_info_bottom.size -= rx_size;

        if (nor->xfer_info_bottom.size == 0u) {
            ret = cospi_lld_rx_complete(cospi);
        }
    }

    return ret;
}

static int cospi_tx_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint32_t tx_size;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->xfer_info_bottom.size > 0u) {
        tx_size = cospi_lld_write_sram(cospi, nor->xfer_info_bottom.buf,
                                       nor->xfer_info_bottom.size);

        nor->xfer_info_bottom.buf += tx_size;
        nor->xfer_info_bottom.size -= tx_size;

        if (nor->xfer_info_bottom.size == 0u) {
            ret = cospi_lld_tx_complete(cospi);
        }
    }

    return ret;
}

static void cospi_polling_handler(void *arg)
{
    int ret = 0;
    struct spi_nor *nor = arg;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->xfer_info_bottom.opt_result == FLASH_OPT_PENDING) {
        if (nor->xfer_info_bottom.opt_type == FLASH_OPT_READ) {
            ret = cospi_rx_polling(nor);
        }
        else if (nor->xfer_info_bottom.opt_type == FLASH_OPT_WRITE) {
            ret = cospi_tx_polling(nor);
        }
    }

    nor->xfer_info_bottom.opt_result = ret ? FLASH_OPT_FAILED : FLASH_OPT_PENDING;

    if (nor->xfer_info_bottom.size == 0 &&
            nor->xfer_info_bottom.opt_result == FLASH_OPT_PENDING) {
        nor->xfer_info_bottom.opt_result = FLASH_OPT_COMPLETE;
        nor->xfer_info.opt_result = nor->xfer_info_bottom.opt_result;
        nor->xfer_info.size = nor->xfer_info_bottom.size;

        if (nor->async_mode) {
            if (nor->event_handler)
                nor->event_handler(nor->xfer_info_bottom.opt_type,
                                   nor->xfer_info_bottom.opt_result);
        }
        else {
            osEventFlagsSet(cospi->bus_event, SPI_NOR_XFER_COMPLETE);
        }
    }
}

static int cospi_cancel(struct spi_nor *nor)
{
    // TODO: DMA cancel
    cospi_lld_cancel(nor);
    return 0;
}

static int cospi_lock(struct spi_nor *nor, enum spi_nor_ops ops)
{
    struct cospi_pdata *cospi = nor->host->priv_data;

    ssdk_printf(SSDK_INFO, "cospi_lock!\n");

    if (cospi->bus_mutex)
        osMutexAcquire(cospi->bus_mutex, osWaitForever);

    if (NULL == nor->host->dev) {
        nor->host->dev = nor;
        cospi_lld_nor_setup(nor, ops);

        if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {
            irq_enable(cospi->irq);
        }

        return 0;
    }
    else {
        return -1;
    }
}

static void cospi_unlock(struct spi_nor *nor, enum spi_nor_ops ops)
{
    struct cospi_pdata *cospi = nor->host->priv_data;

    ssdk_printf(SSDK_INFO, "cospi_unlock!\n");

    if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {
        irq_disable(cospi->irq);
    }

    nor->host->dev = NULL;

    if (cospi->bus_mutex)
        osMutexRelease(cospi->bus_mutex);
}

static void cospi_wait_xfer_done(struct spi_nor *nor)
{
    uint32_t event_flags;
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {
        osEventFlagsWait(cospi->bus_event, SPI_NOR_XFER_COMPLETE,
                         osFlagsWaitAny, osWaitForever);
    }
    else {
        while (1) {
            cospi_polling_handler(nor);
            event_flags = osEventFlagsGet(cospi->bus_event);

            if ((event_flags & SPI_NOR_XFER_COMPLETE) != 0u) {
                osEventFlagsClear(cospi->bus_event, SPI_NOR_XFER_COMPLETE);
                break;
            }
        }
    }
}

uint32_t cospi_lld_set_mst_slv_read_address(struct spi_nor *nor,
        flash_addr_t addr)
{
    uint32_t addr_tmp = addr;

    if (nor->cmd_bytes > 1 && SNOR_ADDR_LANS(nor->info.write_proto) != 3) {
        struct cospi_pdata *cospi = nor->host->priv_data;
        uint8_t mode = 0 ;
        uint32_t opcode = (nor->info.read_proto >> SNOR_OPCODE_PROTO_LSB);
        addr_tmp = (addr >> 8u) | ((opcode << 16u) & 0xff000000);
        mode = (addr & 0xff);

        if (SNOR_ADDR_LANS(nor->info.read_proto) != 3) {
            RMWREG32(cospi->apb_base + COSPI_REG_RD_INSTR, COSPI_REG_RD_INSTR_MODE_EN_LSB,
                     1u, 1);//the mode bit needs to be sent after the address byte
            RMWREG32(cospi->apb_base + COSPI_REG_MODE_BIT, COSPI_REG_MODE_BIT_LSB, 8u,
                     mode);//the 8-bit mode data sent to the device followed by the address byte
        }

        //RMWREG32(cospi->apb_base + 0x44, 6, 1, 1);// set interrupt
        RMWREG32(cospi->apb_base + COSPI_REG_SIZE, COSPI_REG_SIZE_ADDRESS_LSB, 4u,
                 3);//4byte addr master-slaver must be write back!!!!!!
    }

    return addr_tmp;
}

static uint32_t cospi_lld_set_mst_slv_write_address(struct spi_nor *nor,
        flash_addr_t addr, const uint8_t *buf, uint32_t write_size)
{

    struct cospi_pdata *cospi = nor->host->priv_data;
    uint32_t addr_tmp = addr;
    uint32_t opcode = (nor->info.write_proto >> SNOR_OPCODE_PROTO_LSB);

    if (nor->cmd_bytes > 1 && SNOR_ADDR_LANS(nor->info.write_proto) != 3) {
        memset(nor->record_wr_buff, 0, sizeof(nor->record_wr_buff));
        memmove((uint8_t *)(nor->record_wr_buff + 4u), buf,
                write_size);//put mode in buff when writing non 8line mode
        nor->record_wr_buff[3] = addr_tmp & 0xff;
        nor->record_wr_buff[2] = (addr_tmp >> 8u) & 0xff;
        nor->record_wr_buff[1] = (addr_tmp >> 16u) & 0xff;
        nor->record_wr_buff[0] = (addr_tmp >> 24u) & 0xff;
        write_size += 4u;
        //this avoids page alignment checks
        RMWREG32(cospi->apb_base + COSPI_REG_SIZE, COSPI_REG_SIZE_ADDRESS_LSB, 4u,
                 0);//1byte addr set
        //writel((opcode >> 8) & 0xff, cospi->apb_base + 0x78);//cmd fill to addr
        addr_tmp = (opcode >> 8u) & 0xff;
        cospi_lld_setup_xfer(nor, FLASH_OPT_WRITE, addr_tmp,
                             (uint8_t *)(nor->record_wr_buff), write_size);
    }
    else {
        cospi_lld_setup_xfer(nor, FLASH_OPT_WRITE, addr_tmp, (uint8_t *)buf,
                             write_size);
    }

    cospi_lld_indirect_trigger(cospi, addr_tmp, write_size, INDIRECT_WRITE_FLAG);
    return write_size;
}

static int cospi_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                      flash_size_t size)
{
    struct cospi_pdata *cospi = nor->host->priv_data;

    if (addr < cospi->trigger_address && size <= 512 && !nor->async_mode
            && !nor->mst_slv_mode) {
        /* direct access */
        arch_invalidate_cache_range((addr_t)cospi->ahb_base + addr,
                                    ROUNDUP(size, 32));
        memcpy(buf, (uint8_t *)cospi->ahb_base + addr, size);

        nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;
        nor->xfer_info.size = 0u;
        return nor->xfer_info.opt_result;
    }
    else {
        if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
            arch_invalidate_cache_range((addr_t)buf, ROUNDUP(size, 32));
            cospi_lld_dma_config(cospi, SPI_NOR_OPS_READ, buf, size);

            if (nor->dma_xfer_config) {
                nor->dma_xfer_config(nor, addr, buf, true, size,
                                     cospi->dma_bus_width, cospi->dma_burst_size);
            }
        }

        if (nor->mst_slv_mode) {
            addr = cospi_lld_set_mst_slv_read_address(nor, addr);
        }

        cospi_lld_setup_xfer(nor, FLASH_OPT_READ, addr, buf, size);
        cospi_lld_indirect_trigger(cospi, addr, size, INDIRECT_READ_FLAG);

        if (nor->async_mode) {
            return 0;
        }
        else {
            cospi_wait_xfer_done(nor);

            if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                /* stop channel transfer */
                if (nor->dma_xfer_stop) {
                    nor->dma_xfer_stop();
                }
                else if (nor->dma_stop) {
                    nor->dma_stop(nor);
                }
            }
        }

        return nor->xfer_info.opt_result;
    }


}

int cospi_write(struct spi_nor *nor, flash_addr_t addr,
                const uint8_t *buf, flash_size_t size)
{
    uint32_t size_tmp = size;
    uint32_t addr_tmp = addr;
    uint32_t write_size = 0;
    uint32_t buf_index = 0;
    struct cospi_pdata *cospi = nor->host->priv_data;
    struct hyperbus_ca ca_t[3] = {{0x555, 0xAA}, {0x2AA, 0x55}, {0x555, 0xA0}};
    struct hyperbus_ca *ca = ca_t;

#ifdef CONFIG_CDNS_OSPI_WR_DIRECT_EN

    if (addr < cospi->trigger_address && size <= 512 && !nor->async_mode
            && !nor->mst_slv_mode) {
        if (nor->hyperbus_mode) {
            cospi_hyper_set_pre_transaction(nor, false, true, 3, &ca);
        }

        /* direct access */
        memcpy((uint8_t *)cospi->ahb_base + addr, buf, size);
        arch_clean_invalidate_cache_range((addr_t)cospi->ahb_base + addr,
                                          ROUNDUP(size, 32));

        nor->xfer_info.opt_result = FLASH_OPT_COMPLETE;
        return nor->xfer_info.opt_result;
    }
    else {
#else
    {
#endif

        if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE)
        {
            arch_clean_cache_range((addr_t)buf, ROUNDUP(size, 32));
        }

        while (size_tmp > 0)
        {
            write_size = MIN((nor->info.page_size - addr_tmp % nor->info.page_size),
                             size_tmp);

            if (nor->hyperbus_mode) {
                cospi_hyper_set_pre_transaction(nor, false, true, 3, &ca);
            }

            if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                cospi_lld_dma_config(cospi, SPI_NOR_OPS_WRITE, (uint8_t *)buf + buf_index,
                                     write_size);

                if (nor->dma_xfer_config) {
                    nor->dma_xfer_config(nor, addr_tmp, (uint8_t *)buf + buf_index, false,
                                         write_size, cospi->dma_bus_width, cospi->dma_burst_size);
                }
            }

            if (nor->mst_slv_mode) {
                write_size = cospi_lld_set_mst_slv_write_address(nor, addr_tmp, buf,
                             write_size);
            }
            else {
                cospi_lld_setup_xfer(nor, FLASH_OPT_WRITE, addr_tmp, (uint8_t *)buf + buf_index,
                                     write_size);
                cospi_lld_indirect_trigger(cospi, addr_tmp, write_size, INDIRECT_WRITE_FLAG);
            }

            // arch_invalidate_cache_range(cospi->ahb_base + cospi->trigger_address, cospi->trigger_range_size);

            if (nor->async_mode) {
                return 0;
            }
            else {
                cospi_wait_xfer_done(nor);

                if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                    /* stop channel transfer */
                    if (nor->dma_xfer_stop) {
                        nor->dma_xfer_stop();
                    }
                    else if (nor->dma_stop) {
                        nor->dma_stop(nor);
                    }
                }

                if (nor->mst_slv_mode && nor->cmd_bytes > 1
                        && SNOR_ADDR_LANS(nor->info.write_proto) != 3) {
                    write_size -= 4;
                }
                else {
                    /* wait for flash idle */
                    if (spi_nor_wait_idle(nor)) {
                        nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
                    }
                }
            }

            addr_tmp += write_size;
            size_tmp -= write_size;
            buf_index += write_size;
        }

        return nor->xfer_info.opt_result;
    }
}

static void spi_nor_drv_main_function(struct spi_nor *nor)
{
    if (nor->xfer_mode == SPI_NOR_XFER_POLLING_MODE) {
        cospi_polling_handler(nor);
    }
}

static int cospi_enter_xip_mode(struct spi_nor *nor)
{
    struct cospi_pdata *cospi = nor->host->priv_data;
    cospi_lld_enter_xip(cospi);
    return 0;
}

struct spi_nor_host_ops cospi_host_ops = {
    .reg_read = cospi_lld_command_read,
    .reg_write = cospi_lld_command_write,
    .read = cospi_read,
    .write = cospi_write,
    .erase = cospi_lld_stig_erase,
    .complex_erase = cospi_lld_stig_complex_erase,
    .training = cospi_lld_phy_training,
    .cancel = cospi_cancel,
    .prepare = cospi_lock,
    .unprepare = cospi_unlock,
    .cache_flush = cospi_cache_flush,
    .enter_xip_mode = cospi_enter_xip_mode,
    .xip_proto_setup = cospi_xip_proto_setup,
    .drv_main_function = spi_nor_drv_main_function,
};

struct hyperbus_host_ops cospi_hyperbus_ops = {
    .read16 = cospi_hyper_read16,
    .write16 = cospi_hyper_write16,
    .read = cospi_read,
    .write = cospi_write,
    .hyperram_en = cospi_hyperram_enable,
    .set_pre_transaction = cospi_hyper_set_pre_transaction,
    .cache_flush = cospi_cache_flush,
};

int cospi_host_init(struct spi_nor_host *host, struct cospi_pdata *cospi,
                    struct cospi_config *config)
{
    memset(host, 0, sizeof(struct spi_nor_host));
    memset(cospi, 0, sizeof(struct cospi_pdata));

    /* init the read and write protocal */
    cospi->inst_type = 0xff;

    host->id = config->id;
    host->base = config->apb_base;
    host->irq = config->irq;
    host->ref_clk_hz = config->ref_clk;

    cospi->apb_base = config->apb_base;
    cospi->ahb_base = config->ahb_base;
    cospi->irq = config->irq;
    cospi->ref_clk_hz = config->ref_clk;

    cospi->bus_event = osEventFlagsNew(NULL);
    cospi->bus_mutex = osMutexNew(NULL);

    host->priv_data = cospi;
    host->ops = &cospi_host_ops;
    host->xip_mode = config->xip_mode;
    host->hyper_ops = &cospi_hyperbus_ops;

    if (cospi->irq) {
        irq_attach(cospi->irq, cospi_irq_handler, (void *)host);
    }

    if (config->xip_mode) {
        cospi_lld_soft_init(cospi);
    } else {
        cospi_lld_init(cospi);
        cospi_lld_reset_flash(cospi);
    }

    return 0;
}

static void cospi_drv_deinit(struct spi_nor *nor)
{
    // TODO: DMA deinit
    return;
}
