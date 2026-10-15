
//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#if defined(CONFIG_FEE_BLOCK) || defined(CONFIG_FEE_EEPROM)

#include <stdlib.h>
#include <regs_base.h>
#include <cmsis_os2.h>
#include <arch/cache.h>
#include <debug.h>
#include <driver/clk/clk.h>
#include <driver/uart.h>
#include <types.h>
#include <string.h>
#include <param.h>
#include <board.h>

#include "fee_cfg.h"

#define PART_TABLE_SZIE 0x10000
#define PART_EEPROM_SZIE 0x30000

/* fee eeprom 0 */
struct fee_eeprom fee_eeprom0 = {
    .fee_con = {
        .disk_name = DISK_NOR_FLASH_NAME(0),
        .size = PART_EEPROM_SZIE,
        .mem_align_size = CONFIG_ARCH_CACHE_LINE,
        .access_align_size = 0x4,
        .pe_cycles = 10,

        .fee_name = FEE_EEPROM_NAME(0),
        .eeprom_size = 8 * 1024,
        .eeprom_align_size = 4,
        .eeprom_pe_cycles = 100,
    },
};

const struct fee_eeprom *board_fee_eeprom[] = {
    &fee_eeprom0,
};

const struct fee_block *board_fee_block[] = {
    0
};

int fee_eeprom_prepare(struct fee_eeprom *fee_dev)
{
    int ret = 0;
    struct disk_dev flash_dev = {0};

    disk_open(fee_dev->fee_con.disk_name, &flash_dev);
    disk_size_t flash_size = disk_size(&flash_dev);
    uint32_t sector_size = disk_erase_size(&flash_dev);
    disk_close(&flash_dev);
    fee_dev->fee_con.size = ROUNDUP(fee_dev->fee_con.size, sector_size * 2);
    disk_addr_t addr = flash_size - fee_dev->fee_con.size - PART_TABLE_SZIE;
    fee_dev->fee_con.addr = ROUNDDOWN(addr, sector_size);

    ret = fee_register_eeprom(fee_dev);

    if (ret) {
        printf("fee_register_eeprom error\n");
        return -1;
    }

    return 0;
}

int fee_block_prepare(struct fee_block *fee_dev)
{
    return 0;
}

void board_fee_init(void)
{

#if defined(CONFIG_FEE_EEPROM)

    if (fee_init_eeprom())
        return;

    /* Prepare fee eeprom tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_fee_eeprom); i++) {
        fee_eeprom_prepare((struct fee_eeprom *)board_fee_eeprom[i]);
    }

#endif

#if defined(CONFIG_FEE_BLOCK)

    if (fee_init_block())
        return;

    for (uint32_t i = 0; i < ARRAY_SIZE(board_fee_block); i++) {
        fee_block_prepare((struct fee_block *)board_fee_block[i]);
    }

#endif
}
#endif

