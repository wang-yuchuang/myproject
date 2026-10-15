
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#if defined(CONFIG_CDNS_OSPI) && defined(CONFIG_DISK_NOR_FLASH)

#include <arch/barriers.h>
#include <arch/cache.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/clk.h>
#include <driver/scr.h>
#include <driver/uart.h>
#include <param.h>
#include <regs_base.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "clock_cfg.h"
#include "ospi_cfg.h"
#include "rstgen_cfg.h"
#include "scr_cfg.h"
#include "scr_hw.h"

#include <driver/kunlun/spi_nor/cdns_ospi_drv.h>
#include <lib/disk/disk_norflash.h>
#ifdef CONFIG_FLASH_LOADER
#include "flash_loader.h"
extern void FlashLoaderRegister(struct spi_nor *);
#endif

struct spi_nor_host spi_nor_host = {0};
// struct xspi_pdata xspi = {0};
struct cospi_pdata ospi = {0};
struct spi_nor flash = {0};

static struct cospi_config host_config = {
    .id = 0,
    .apb_base = APB_OSPI1_BASE,
    .ahb_base = OSPI1_BASE,
    .irq = OSPI1_INTR_NUM,
};

static struct spi_nor_config config = {
    .id = 0,
    .cs = 0,
    .baudrate = 25000000,
    .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
    .sw_rst = true,
    .async_mode = false,
};

static struct disk_norflash_info norflash_info = {
    .config =
        {
            .disk_name = DISK_NOR_FLASH_NAME(0),
        },
};


void board_norflash_init(void)
{
    struct flash_info *flash_info;
    clk_node_t *clk = CLK_NODE(g_ckgen_ip_ospi1);

    host_config.ref_clk = clk_get_rate(clk);
    cospi_host_init(&spi_nor_host, &ospi, &host_config);

    spi_nor_host.clk = clk;

    // clk_set_rate(clk, config.baudrate);
    ssdk_printf(SSDK_CRIT, "ospi clock rate is %u!\n", clk_get_rate(clk));

    if (spi_nor_init(&flash, &spi_nor_host, &config)) {
        ssdk_printf(SSDK_EMERG, "\nspinor init failed!\n");
        return;
    }

    flash_info = spi_nor_get_info(&flash);
    norflash_info.config.disk_size = flash_info->size;
    norflash_info.config.sector_size = flash_info->sector_size;
    norflash_info.config.norflash_dev = &flash;
    norflash_info.config.norflash_erase = (norflash_erase_t)spi_nor_erase;
    norflash_info.config.norflash_read = (norflash_read_t)spi_nor_read;
    norflash_info.config.norflash_write = (norflash_write_t)spi_nor_write;
    if (register_norflash_disk(&norflash_info)) {
        ssdk_printf(SSDK_EMERG, "\nregister_norflash_disk failed!\n");
        return;
    }

#ifdef CONFIG_FLASH_LOADER
    FlashLoaderRegister(&flash);
#endif
}

void board_norflash_deinit(void)
{
    spi_nor_deinit(&flash);
    ssdk_printf(SSDK_CRIT, "\nspinor deinit!\n");
    return;
}

#endif
