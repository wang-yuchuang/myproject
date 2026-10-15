/**
 * @file disk_mapped.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_MAPPED_H_
#define DISK_MAPPED_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>
#include <lib/disk/disk.h>

struct disk_mapped_config {
    /* disk device name */
    char *disk_name;

    /* mapped para */
    uint64_t disk_offset;
    uint64_t disk_size;

    /* base disk */
    struct disk_info* base_disk_info;
};

struct disk_mapped_info {
    struct disk_mapped_config config;
    struct disk_info info;
};

int register_mapped_disk(struct disk_mapped_info *mapped_info);
int unregister_mapped_disk(struct disk_mapped_info *mapped_info);

#endif /* DISK_MAPPED_H_ */
