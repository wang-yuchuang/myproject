/**
 * @file disk_cache_noop.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_CACHE_NOOP_H_
#define DISK_CACHE_NOOP_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_cache.h>

struct disk_cache_noop {
    struct list_node addr_list;     /* cache node base on addr list */
    struct list_node create_list;   /* cache node base on create list */
    struct list_node write_list;    /* cache node base on write list */

    uint32_t cache_align_size;      /* buff node align the size */
    uint32_t cache_rsize;           /* buffer data real size */
    uint32_t cache_size;            /* buffer data size max */

    struct disk_cache cache;
};

/* cache API */
int register_disk_cache_noop(struct disk_cache_noop *cache_noop);
int unregister_disk_cache_noop(struct disk_cache_noop *cache_noop);
#endif /* DISK_CACHE_H_ */

