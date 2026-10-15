//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_CDNS_OSPI
#include <arch/barriers.h>
#include <arch/cache.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
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

struct spi_nor_host spi_nor_host = {0};
struct cospi_pdata ospi = {0};
struct spi_nor flash = {0};

struct cospi_config host_config = {
    .id = 2,
    .apb_base = DEVICE_BASE(OSPI2),
    .ahb_base = OSPI2_BASE,
    .xip_mode = true,   /* firmware XIPs on OSPI2: skip hw reinit */
};

struct spi_nor_config config = {
    .id = 2,
    .cs = 0,
    .baudrate = 50000000,
    .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
    .sw_rst = true,
    .async_mode = false,
    .force_size = 0x400000,   /* 4MB gd25q */
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

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi2);
    cospi_host_init(&spi_nor_host, &ospi, &host_config);
    ssdk_printf(SSDK_CRIT, "ospi clock rate is %u!\n", clk_get_rate(&g_ckgen_ip_ospi2));

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
}

void board_norflash_deinit(void)
{
    spi_nor_deinit(&flash);
    ssdk_printf(SSDK_CRIT, "\nspinor deinit!\n");

    return;
}

#endif
