/**
 * @file disk_mmc.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_NORFLASH_H_
#define DISK_NORFLASH_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <cmsis_os2.h>
#include <lib/list.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_cache_noop.h>

typedef int (*norflash_read_t)(void *, disk_addr_t, uint8_t *, disk_size_t);
typedef int (*norflash_write_t)(void *, disk_addr_t, const uint8_t *, disk_size_t);
typedef int (*norflash_erase_t)(void *, disk_addr_t, disk_size_t);

struct disk_norflash_config {
    /* disk device name */
    char *disk_name;

    /* disk attribute */
    uint64_t disk_size;
    /* write & read  sector size */
    uint32_t sector_size;

    void *norflash_dev;
    norflash_read_t norflash_read;
    norflash_write_t norflash_write;
    norflash_erase_t norflash_erase;
};

struct disk_norflash_info {
    uint8_t *buff_sector;
    uint32_t buff_size;
    osMutexId_t disk_norflash_mutex;
    struct disk_norflash_config config;
    struct disk_info info;
    struct disk_cache_noop cache_noop;
};

int register_norflash_disk(struct disk_norflash_info *norflash_info);
int unregister_norflash_disk(struct disk_norflash_info *norflash_info);

#endif /* DISK_NORFLASH_H_ */

