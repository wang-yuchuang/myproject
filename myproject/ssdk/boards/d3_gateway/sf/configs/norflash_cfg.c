
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_TAISHAN_XSPI

#include <stdlib.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include <arch/cache.h>
#include <arch/barriers.h>
#include <debug.h>
#include <driver/clk.h>
#include <driver/scr.h>
#include <driver/uart.h>
#include <types.h>
#include <string.h>
#include <param.h>
#include <board.h>

#include "clock_cfg.h"
#include "scr_cfg.h"
#include "scr_hw.h"
#include "rstgen_cfg.h"
#include "norflash_cfg.h"

#ifdef CONFIG_FLASH_LOADER
#include "flash_loader.h"
#endif

struct spi_nor_host spi_nor_host = {0};
struct xspi_pdata xspi = {0};
struct spi_nor flash = {0};

static struct xspi_config host_config = {
    .id = 0,
    .irq = XSPI1_IRQ0_INTR_NUM,
    .apb_base = DEVICE_BASE(XSPI1PORTA),
    .direct_base = XSPI1_XSPI1PORTA_BASE,
};

static struct spi_nor_config config = {
    .id = 0,
    .cs = 0,
    .baudrate = 20000000,
    .xfer_mode = SPI_NOR_XFER_DMA_MODE,
    .sw_rst = true,
    .async_mode = false,
};

static struct disk_norflash_info norflash_info = {
    .config = {
        .disk_name = DISK_NOR_FLASH_NAME(0),
    },
};

void xspi_lockstep_disable()
{
    scr_signal_t signal = SCR_SF_XSPI1_SRC_CFG_LOCKSTEP_MODE_N;

    reset_ctl_assert(&rstsig_xspi1b);
    reset_ctl_assert(&rstsig_xspi1a);
    ISB;
    udelay(10);

    scr_set(&g_scr_sf, &signal, 1);
    udelay(10);
    ISB;

    reset_ctl_deassert(&rstsig_xspi1a);
    reset_ctl_deassert(&rstsig_xspi1b);
}

void board_norflash_init(void)
{
    clk_node_t *clk = CLK_NODE(g_ckgen_ip_xspi1a);

    xspi_lockstep_disable();

    host_config.ref_clk = clk_get_rate(clk);
    xspi_host_init(&spi_nor_host, &xspi, &host_config);
    spi_nor_host.clk = clk;

    clk_set_rate(clk, config.baudrate);
    ssdk_printf(SSDK_CRIT, "xspi clock rate is %u!\n", clk_get_rate(clk));

    if(spi_nor_init(&flash, &spi_nor_host, &config)) {
       ssdk_printf(SSDK_EMERG, "\nspinor init failed!\n");
       return;
    }

#ifdef CONFIG_DISK_NOR_FLASH
    struct flash_info *flash_info = spi_nor_get_info(&flash);
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
#endif

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
