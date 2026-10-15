/**
 * @file disk_cache.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_CACHE_H_
#define DISK_CACHE_H_

#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <lib/list.h>
#include <lib/disk/disk.h>

struct disk_cache;

struct disk_cache_operations {
    int (*disk_cache_read)(struct disk_cache *cache, uint8_t *dst, disk_addr_t addr,
                           disk_size_t size);
    int (*disk_cache_write)(struct disk_cache *cache, const uint8_t *src,
                            disk_addr_t addr, disk_size_t size);
    int (*disk_cache_erase)(struct disk_cache *cache, disk_addr_t addr,
                            disk_size_t size, uint32_t flag);

    int (*disk_cache_sync)(struct disk_cache *cache);
    int (*disk_cache_sync_addr)(struct disk_cache *cache, disk_addr_t addr,
                                disk_size_t size);
};

struct disk_cache {
    struct disk_info *info;
    struct disk_cache_operations *disk_cache_ops;
    /* Private data */
    void *private_data;
};


/* disk cache info API */
int disk_cache_read(struct disk_cache *cache, uint8_t *dst, disk_addr_t addr,
                    disk_size_t size);
int disk_cache_write(struct disk_cache *cache, const uint8_t *src,
                     disk_addr_t addr, disk_size_t size);
int disk_cache_erase(struct disk_cache *cache, disk_addr_t addr,
                     disk_size_t size, uint32_t flag);
int disk_cache_sync(struct disk_cache *cache);
int disk_cache_sync_addr(struct disk_cache *cache, disk_addr_t addr,
                         disk_size_t size);

/* disk cache dev API */
int disk_cache_dev_read(struct disk_cache *cache, uint8_t *dst,
                        disk_addr_t addr, disk_size_t size);
int disk_cache_dev_write(struct disk_cache *cache, const uint8_t *src,
                         disk_addr_t addr, disk_size_t size);
int disk_cache_dev_erase(struct disk_cache *cache, disk_addr_t addr,
                         disk_size_t size, uint32_t flag);
#endif /* DISK_H_ */
