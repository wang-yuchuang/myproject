/**
 * @file disk_ram.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_RAM_H_
#define DISK_RAM_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>
#include <lib/disk/disk.h>

struct disk_ram_config {
    /* disk device name */
    char *disk_name;

    /* disk attribute */
    void *mem_base;
    disk_size_t disk_size;
};

struct disk_ram_info {
    osMutexId_t disk_ram_mutex;
    struct disk_ram_config config;
    struct disk_info info;
};

int register_ram_disk(struct disk_ram_info *ram_info);
int unregister_ram_disk(struct disk_ram_info *ram_info);

#endif /* DISK_RAM_H_ */
