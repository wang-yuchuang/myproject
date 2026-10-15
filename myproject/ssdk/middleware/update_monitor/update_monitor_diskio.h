/**
 * @file update_monitor_diskio.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <lib/disk/disk.h>

#ifndef UPDATE_MONITOR_DISKIO_H_
#define UPDATE_MONITOR_DISKIO_H_

#define UPDATE_BLOCK_SIZE 512

enum disk_type_enum {
    DISK_TYPE_OSPI1,
    DISK_TYPE_MMC1,
    DISK_TYPE_MMC1_BOOT1,
    DISK_TYPE_MMC1_BOOT2,
    DISK_NUM_MAX,
};

extern int um_disk_init(const char* disk_name, struct disk_dev* dev);
extern int um_disk_deinit(struct disk_dev* dev);
extern int um_disk_read(struct disk_dev* dev, uint64_t addr, uint8_t *buf, uint64_t size);
extern int um_disk_erase(struct disk_dev* dev, uint64_t addr, uint64_t size);
extern int um_disk_write(struct disk_dev* dev, uint64_t addr, uint8_t *buf, uint64_t size);
extern uint64_t um_get_capacity(struct disk_dev* dev);
extern uint32_t um_get_erase_size(struct disk_dev* dev);
extern uint32_t um_get_block_size(struct disk_dev* dev);

#endif /* UPDATE_MONITOR_DISKIO_H_ */


