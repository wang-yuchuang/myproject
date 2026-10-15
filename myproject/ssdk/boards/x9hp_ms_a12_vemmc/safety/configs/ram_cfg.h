/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_RAM_CFG_H
#define BOARDS_RAM_CFG_H

#ifdef CONFIG_DISK_RAM

#include <lib/disk/disk.h>
#include <lib/disk/disk_ram.h>

extern void board_ram_init(void);
#endif

#endif /* BOARDS_MMC_CFG_H */