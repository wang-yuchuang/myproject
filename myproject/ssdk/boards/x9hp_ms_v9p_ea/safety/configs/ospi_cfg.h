
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#if defined(CONFIG_CDNS_OSPI) && defined(CONFIG_DISK_NOR_FLASH)
#ifndef BOARDS_NORFLASH_CFG_H
#define BOARDS_NORFLASH_CFG_H

#include <driver/spi_nor.h>
// #include <driver/spi_nor/taishan_xspi_drv.h>
#include <driver/kunlun/spi_nor/cdns_ospi_drv.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_norflash.h>
#define g_spi_nor1 flash

extern struct spi_nor_host spi_nor_host;
extern struct xspi_pdata xspi;
extern struct spi_nor flash;

/* norflash init functions */
extern void board_norflash_init(void);

extern void board_norflash_deinit(void);

#endif /* BOARDS_NORFLASH_CFG_H */

#endif
