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

#ifndef DISK_MMC_H_
#define DISK_MMC_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_cache_noop.h>

struct disk_mmc_config {
    /* disk device name */
    char *disk_name;

    /* disk attribute */
    disk_size_t disk_size;
    uint32_t erase_size;
    /* write & read  sector size */
    uint32_t sector_size;

    /* mmc device ops API */
    /* write & read:addr is block addr,count is block num */
    /* erase & read:addr is block addr,size is len */
    void *mmc_dev;
    int (*mmc_read)(void *mmc_dev, uint8_t *dst, block_t block,
                    block_count_t count);
    int (*mmc_write)(void *mmc_dev, const uint8_t *src, block_t block,
                     block_count_t count);
    int (*mmc_erase)(void *mmc_dev, block_t block, disk_size_t size);
};

struct disk_mmc_info {
    uint8_t *buff_sector;
    uint32_t buff_size;
    osMutexId_t disk_mmc_mutex;
    uint32_t mmc_max_xfer_size;
    struct disk_mmc_config config;
    struct disk_info info;
    struct disk_cache_noop cache_noop;
};

int register_mmc_disk(struct disk_mmc_info *mmc_info);
int unregister_mmc_disk(struct disk_mmc_info *mmc_info);

#endif /* DISK_MMC_H_ */
