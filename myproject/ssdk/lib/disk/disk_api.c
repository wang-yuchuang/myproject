/**
 * @file disk_api.c
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
#include <cmsis_os2.h>
#include <arch/atomic.h>
#include <stdarg.h>

#include <lib/list.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_cache.h>

static struct list_node g_disk_list;
static osMutexId_t g_disk_mutex;

struct disk_info *disk_get_info(const char *disk_name)
{
    struct disk_info *info;

    osMutexAcquire(g_disk_mutex, osWaitForever);
    /* matches nodes based on name */
    list_for_every_entry(&g_disk_list, info,
                         struct disk_info, node) {
        if (!strcmp(disk_name, info->disk_name)) {
            osMutexRelease(g_disk_mutex);
            return info;
        }
    }
    osMutexRelease(g_disk_mutex);
    return NULL;
}

int disk_open(const char *disk_name, struct disk_dev *dev)
{
    struct disk_info *info;
    info = disk_get_info(disk_name);

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk devicve node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    if (NULL != dev->info) {
        ssdk_printf(SSDK_ERR, "disk device is open.\n");
        return -DISK_ERROR_BUSY;
    }

    ssdk_printf(SSDK_DEBUG, "disk open:%s\n", info->disk_name);
    dev->info = info;
    dev->block_size = info->block_align_size;
    dev->flags = DISK_FLAGS_REWR;
    arch_atomic_add(&info->use_count, 1);
    return 0;
}

int disk_close(struct disk_dev *dev)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk close:%s\n", info->disk_name);
    dev->info = NULL;
    arch_atomic_add(&info->use_count, -1);
    return 0;
}

int disk_read(struct disk_dev *dev, disk_addr_t addr, uint8_t *dst,
              disk_size_t size)
{
    if (!(dev->flags & DISK_FLAGS_READ)) {
        ssdk_printf(SSDK_ERR, "disk device does not support read operations \n");
        return -DISK_ERROR_NO_PERMISSION;
    }

    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (addr + size > info->disk_size) {
        ssdk_printf(SSDK_ERR, "out of disk range, addr:%lld size:%lld disk_size:%lld\n",
                    addr, size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    ssdk_printf(SSDK_DEBUG, "disk read:%s addr:%lld size:%lld\n",
                info->disk_name, addr, size);
#ifdef CONFIG_DISK_CACHE

    if ((info->cache) && !(dev->flags & DISK_FLAGS_DIRECT_IO)) {
        return disk_cache_read(info->cache, dst, addr, size);
    }
    else
#endif
        if (info->disk_ops->disk_read)
            return info->disk_ops->disk_read(info, dst, addr, size);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_write(struct disk_dev *dev, disk_addr_t addr, const uint8_t *src,
               disk_size_t size)
{
    if (!(dev->flags & DISK_FLAGS_WRITE)) {
        ssdk_printf(SSDK_ERR, "disk device does not support write operations \n");
        return -DISK_ERROR_NO_PERMISSION;
    }

    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (addr + size > info->disk_size) {
        ssdk_printf(SSDK_ERR, "out of disk range, addr:%lld size:%lld disk_size:%lld\n",
                    addr, size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    ssdk_printf(SSDK_DEBUG, "disk write:%s addr:%lld size:%lld\n", info->disk_name,
                addr, size);
#ifdef CONFIG_DISK_CACHE

    if (info->cache) {
        int ret = 0;
        ret |= disk_cache_write(info->cache, src, addr, size);

        /* DIRECT_IO situation,TODO */
        if (dev->flags & DISK_FLAGS_DIRECT_IO)
            ret |= disk_cache_sync_addr(info->cache, addr, size);

        return ret;
    }
    else
#endif
        if (info->disk_ops->disk_write)
            return info->disk_ops->disk_write(info, src, addr, size);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_read_block(struct disk_dev *dev, block_t block, uint8_t *dst,
                    block_count_t count)
{
    if (!(dev->flags & DISK_FLAGS_READ)) {
        ssdk_printf(SSDK_ERR, "disk device does not support read operations \n");
        return -DISK_ERROR_NO_PERMISSION;
    }

    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (block + count > info->disk_size / dev->block_size) {
        ssdk_printf(SSDK_ERR,
                    "out of disk range, block:%lld count:%d block_size:0x%x addr:%lld size:%lld disk_size:%lld\n",
                    block, count, dev->block_size, (disk_addr_t)block * dev->block_size,
                    (disk_addr_t)count * dev->block_size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    /* mem alignment judgment */
    if (!IS_ALIGNED(dst, info->mem_align_size)) {
        ssdk_printf(SSDK_ERR, "mem addresses 0x%p not aligned to 0x%x\n", dst,
                    info->mem_align_size);
        return -DISK_ERROR_NO_ALIGNED;
    }

    ssdk_printf(SSDK_DEBUG,
                "disk read block:%s block:%lld count:%d block_size:0x%x addr:%lld size:%lld\n",
                info->disk_name, block, count, dev->block_size,
                (disk_addr_t)block * dev->block_size, (disk_addr_t)count * dev->block_size);
#ifdef CONFIG_DISK_CACHE

    if ((info->cache) && !(dev->flags & DISK_FLAGS_DIRECT_IO)) {
        return disk_cache_read(info->cache, dst, block * dev->block_size,
                               count * dev->block_size);
    }
    else
#endif
        if (info->disk_ops->disk_read_block)
            return info->disk_ops->disk_read_block(info, dst, block, count,
                                                   dev->block_size);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_write_block(struct disk_dev *dev, block_t block, const uint8_t *src,
                     block_count_t count)
{
    if (!(dev->flags & DISK_FLAGS_WRITE)) {
        ssdk_printf(SSDK_ERR, "disk device does not support write operations \n");
        return -DISK_ERROR_NO_PERMISSION;
    }

    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (block + count > info->disk_size / dev->block_size) {
        ssdk_printf(SSDK_ERR,
                    "out of disk range, block:%lld count:%d block_size:0x%x addr:%lld size:%lld disk_size:%lld\n",
                    block, count, dev->block_size, (disk_addr_t)block * dev->block_size,
                    (disk_addr_t)count * dev->block_size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    /* mem alignment judgment */
    if (!IS_ALIGNED(src, info->mem_align_size)) {
        ssdk_printf(SSDK_ERR, "mem addresses %p not aligned to %d\n", src,
                    info->mem_align_size);
        return -DISK_ERROR_NO_ALIGNED;
    }

    ssdk_printf(SSDK_DEBUG,
                "disk write block:%s block:%lld count:%d block_size:0x%x addr:%lld size:%lld\n",
                info->disk_name, block, count, dev->block_size,
                (disk_addr_t)block * dev->block_size, (disk_addr_t)count * dev->block_size);
#ifdef CONFIG_DISK_CACHE

    if (info->cache) {
        int ret = 0;
        ret |= disk_cache_write(info->cache, src, block * dev->block_size,
                                count * dev->block_size);

        /* DIRECT_IO situation,TODO */
        if (dev->flags & DISK_FLAGS_DIRECT_IO)
            ret |= disk_cache_sync_addr(info->cache, block * dev->block_size,
                                        count * dev->block_size);

        return ret;
    }
    else
#endif
        if (info->disk_ops->disk_write_block)
            return info->disk_ops->disk_write_block(info, src, block, count,
                                                    dev->block_size);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_erase(struct disk_dev *dev, disk_addr_t addr, disk_size_t size,
               disk_erase_flags flag)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (addr + size > info->disk_size) {
        ssdk_printf(SSDK_ERR, "out of disk range, addr:%lld size:%lld disk_size:%lld\n",
                    addr, size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    ssdk_printf(SSDK_DEBUG, "disk erase:%s addr:%lld size:%lld flag:%d\n",
                info->disk_name, addr, size, flag);
#ifdef CONFIG_DISK_CACHE

    if (info->cache) {
        int ret = 0;
        ret |= disk_cache_erase(info->cache, addr, size, flag);

        /* DIRECT_IO situation,TODO */
        if (dev->flags & DISK_FLAGS_DIRECT_IO)
            ret |= disk_cache_sync_addr(info->cache, addr, size);

        return ret;
    }
    else
#endif
        if (info->disk_ops->disk_erase)
            return info->disk_ops->disk_erase(info, addr, size, flag);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_erase_group(struct disk_dev *dev, uint32_t erase_block, uint32_t count,
                     disk_erase_flags flag)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (erase_block + count > info->disk_size / info->erase_size) {

        ssdk_printf(SSDK_ERR,
                    "out of disk range, erase_block:%d count:%d erase_size:0x%x addr:%lld size:%lld disk_size:%lld\n",
                    erase_block, count, info->erase_size,
                    (disk_addr_t)erase_block * info->erase_size,
                    (disk_addr_t)count * info->erase_size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    ssdk_printf(SSDK_DEBUG,
                "disk erase group:%s erase_block:%d count:%d erase_size:0x%x addr:%lld size:%lld flag:%d\n",
                info->disk_name, erase_block, count, info->erase_size,
                (disk_addr_t)erase_block * info->erase_size,
                (disk_addr_t)count * info->erase_size, flag);
#ifdef CONFIG_DISK_CACHE

    if (info->cache) {
        int ret = 0;
        ret |= disk_cache_erase(info->cache, erase_block * info->erase_size,
                                count * info->erase_size, flag);

        /* DIRECT_IO situation,TODO */
        if (dev->flags & DISK_FLAGS_DIRECT_IO)
            ret |= disk_cache_sync_addr(info->cache, erase_block * info->erase_size,
                                        count * info->erase_size);

        return ret;
    }
    else
#endif
        if (info->disk_ops->disk_erase_group)
            return info->disk_ops->disk_erase_group(info, erase_block, count,
                                                    info->erase_size, flag);
        else
            return -DISK_ERROR_NO_FUN;
}

int disk_sync(struct disk_dev *dev)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk sync:%s\n", info->disk_name);
#ifdef CONFIG_DISK_CACHE

    if (info->cache)
        return disk_cache_sync(info->cache);
    else
#endif
        return -DISK_ERROR_NO_FUN;
}

int disk_sync_addr(struct disk_dev *dev, disk_addr_t addr, disk_size_t size)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    /* size range judgment */
    if (addr + size > info->disk_size) {
        ssdk_printf(SSDK_ERR, "out of disk range, addr:%lld size:%lld disk_size:%lld\n",
                    addr, size, info->disk_size);
        return -DISK_ERROR_OUT_RANGE;
    }

    ssdk_printf(SSDK_DEBUG, "disk sync addr:%s addr:%lld size:%lld\n",
                info->disk_name, addr, size);
#ifdef CONFIG_DISK_CACHE

    if (info->cache)
        return disk_cache_sync_addr(info->cache, addr, size);
    else
#endif
        return -DISK_ERROR_NO_FUN;
}

disk_addr_t disk_size(struct disk_dev *dev)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk size:%s size:%lld\n", info->disk_name,
                info->disk_size);
    return info->disk_size;
}

uint32_t disk_erase_size(struct disk_dev *dev)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk erase size:%s erase_size:0x%x\n", info->disk_name,
                info->erase_size);
    return info->erase_size;
}

uint32_t disk_get_block_size(struct disk_dev *dev)
{
    ssdk_printf(SSDK_DEBUG, "disk get block size:%s block_size:0x%x\n",
                dev->info->disk_name, dev->block_size);
    return dev->block_size;
}

int disk_set_block_size(struct disk_dev *dev, uint32_t blk_sz)
{
    struct disk_info *info = dev->info;
    ssdk_printf(SSDK_DEBUG, "disk set block size:%s block_size:0x%x\n",
                dev->info->disk_name, blk_sz);

    /* blk_sz alignment judgment */
    if (!IS_ALIGNED(blk_sz, info->block_align_size)) {
        ssdk_printf(SSDK_ERR, "blk_sz %d not aligned to 0x%x\n", blk_sz,
                    info->block_align_size);
        return -DISK_ERROR_NO_ALIGNED;
    }

    dev->block_size = blk_sz;
    return 0;
}

int disk_set_flags(struct disk_dev *dev, bool mode, uint32_t flags)
{
    if (mode)
        dev->flags &= flags;
    else
        dev->flags |= ~flags;

    ssdk_printf(SSDK_DEBUG, "disk set flags:%s flags:0x%x\n", dev->info->disk_name,
                dev->flags);
    return 0;
}

uint32_t disk_get_flags(struct disk_dev *dev)
{
    ssdk_printf(SSDK_DEBUG, "disk get flags:%s flags:0x%x\n", dev->info->disk_name,
                dev->flags);
    return dev->flags;
}

int disk_status(struct disk_dev *dev)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk status:%s\n", info->disk_name);

    if (info->disk_ops->disk_status)
        return info->disk_ops->disk_status(info);
    else
        return -DISK_ERROR_NO_FUN;
}

int disk_ioctl(struct disk_dev *dev, uint32_t cmd, void *buff)
{
    struct disk_info *info = dev->info;

    if (NULL == info) {
        ssdk_printf(SSDK_ERR, "no disk device node\n");
        return -DISK_ERROR_NO_DEVICE;
    }

    ssdk_printf(SSDK_DEBUG, "disk ioctl:%s\n", info->disk_name);

    if (info->disk_ops->disk_ioctl)
        return info->disk_ops->disk_ioctl(info, cmd, buff);
    else
        return -DISK_ERROR_NO_FUN;
}

int register_disk(struct disk_info *disk)
{
    int ret = 0;
    ssdk_printf(SSDK_DEBUG, "register disk :%s disk_size:%lld erase_size:0x%x\n",
                disk->disk_name, disk->disk_size, disk->erase_size);
    osMutexAcquire(g_disk_mutex, osWaitForever);
    /* insert node */
    list_add_tail(&g_disk_list, &disk->node);
    disk->use_count = 0;
    osMutexRelease(g_disk_mutex);
    return ret;
}

int unregister_disk(struct disk_info *disk)
{
    int ret = 0;
    ssdk_printf(SSDK_DEBUG, "unregiste disk :%s\n", disk->disk_name);
    osMutexAcquire(g_disk_mutex, osWaitForever);

    if (disk->use_count) {
        osMutexRelease(g_disk_mutex);
        return -DISK_ERROR_BUSY;
    }

    /* remove nodes */
    if (list_in_list(&disk->node))
        list_delete(&disk->node);

    osMutexRelease(g_disk_mutex);
    return ret;
}

int disk_init(void)
{
    ssdk_printf(SSDK_DEBUG, "disk init\n");
    /* disk list node init */
    list_initialize(&g_disk_list);
    g_disk_mutex = osMutexNew(NULL);
    return 0;
}

int disk_exit(void)
{
    ssdk_printf(SSDK_DEBUG, "disk exit\n");
    /* disk list node init */
    osMutexRelease(g_disk_mutex);
    return 0;
}

