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
#include <lib/disk/disk.h>
#include <lib/disk/disk_norflash.h>
#include <driver/clk.h>

/* norflash init functions */
extern void board_norflash_init(void);

extern void board_norflash_deinit(void);

#endif /* BOARDS_NORFLASH_CFG_H */

#endif
