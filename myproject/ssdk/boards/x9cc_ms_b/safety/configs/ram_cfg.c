/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_DISK_RAM

#include <stdlib.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include <arch/cache.h>
#include <debug.h>
#include <types.h>
#include <string.h>
#include <param.h>
#include <board.h>

#include "ram_cfg.h"
#include "image_cfg.h"

#define PRELOAD_RES_SIZE  0x1800000     /**< 24M */

struct disk_ram_info ram_info0 = {
    .config = {
        .disk_name = DISK_RAM_NAME(0),
        .mem_base = (void *)(SAF_MEMBASE + SAF_MEMSIZE - PRELOAD_RES_SIZE),
        .disk_size = PRELOAD_RES_SIZE,
    },
};

const struct disk_ram_info *board_ram[] = {
    &ram_info0,
};

int ram_prepare(struct disk_ram_info *ram_info)
{
    register_ram_disk(ram_info);
    return 0;
}

void board_ram_init(void)
{
    /* Prepare mmc & disk tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_ram); i++) {
        ram_prepare((struct disk_ram_info *)board_ram[i]);
    }

}
#endif