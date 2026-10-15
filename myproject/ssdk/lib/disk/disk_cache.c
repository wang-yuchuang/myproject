/**
 * @file disk_cache.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <param.h>
#include <types.h>

#include <lib/disk/disk.h>
#include <lib/disk/disk_cache.h>

int disk_cache_read(struct disk_cache *cache, uint8_t *dst, disk_addr_t addr,
                    disk_size_t size)
{
    if (cache->disk_cache_ops->disk_cache_read)
        return cache->disk_cache_ops->disk_cache_read(cache, dst, addr, size);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_write(struct disk_cache *cache, const uint8_t *src,
                     disk_addr_t addr, disk_size_t size)
{
    if (cache->disk_cache_ops->disk_cache_write)
        return cache->disk_cache_ops->disk_cache_write(cache, src, addr, size);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_erase(struct disk_cache *cache, disk_addr_t addr,
                     disk_size_t size, uint32_t flag)
{
    if (cache->disk_cache_ops->disk_cache_erase)
        return cache->disk_cache_ops->disk_cache_erase(cache, addr, size, flag);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_sync(struct disk_cache *cache)
{
    if (cache->disk_cache_ops->disk_cache_sync)
        return cache->disk_cache_ops->disk_cache_sync(cache);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_sync_addr(struct disk_cache *cache, disk_addr_t addr,
                         disk_size_t size)
{
    if (cache->disk_cache_ops->disk_cache_sync_addr)
        return cache->disk_cache_ops->disk_cache_sync_addr(cache, addr, size);
    else
        return -DISK_ERROR_NO_FUN;
}


int disk_cache_dev_read(struct disk_cache *cache, uint8_t *dst,
                        disk_addr_t addr, disk_size_t size)
{
    struct disk_info *info = cache->info;

    if (info->disk_ops->disk_read)
        return info->disk_ops->disk_read(info, dst, addr, size);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_dev_write(struct disk_cache *cache, const uint8_t *src,
                         disk_addr_t addr, disk_size_t size)
{
    struct disk_info *info = cache->info;

    if (info->disk_ops->disk_write)
        return info->disk_ops->disk_write(info, src, addr, size);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_cache_dev_erase(struct disk_cache *cache, disk_addr_t addr,
                         disk_size_t size, uint32_t flag)
{
    struct disk_info *info = cache->info;

    if (info->disk_ops->disk_erase)
        return info->disk_ops->disk_erase(info, addr, size, flag);
    else
        return -DISK_ERROR_NO_FUN;
}
