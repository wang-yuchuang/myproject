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

struct spi_nor_host spi_nor_host = {0};
struct cospi_pdata ospi = {0};
struct spi_nor flash = {0};

struct cospi_config host_config = {
    .id = 1,
    .apb_base = DEVICE_BASE(OSPI1),
    .ahb_base = OSPI1_BASE,
};

struct spi_nor_config config = {
    .id = 1,
    .cs = 0,
    .baudrate = 50000000,
    .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
    .sw_rst = true,
    .async_mode = false,
};

void board_norflash_init(void)
{
    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi1);
    cospi_host_init(&spi_nor_host, &ospi, &host_config);
    ssdk_printf(SSDK_CRIT, "ospi clock rate is %u!\n", clk_get_rate(&g_ckgen_ip_ospi1));

    if (spi_nor_init(&flash, &spi_nor_host, &config)) {
        ssdk_printf(SSDK_EMERG, "\nspinor init failed!\n");
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
