/**
 * @file partition_disk_io.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <cmsis_os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <debug.h>
#include <assert.h>
#include <lib/cksum.h>
#include <compiler.h>
#include <types.h>
#include <param.h>
#include <lib/disk/disk.h>
#include "dloader.h"

/**
 * @brief dl_disk_read
 *
 * @param dev disk device
 * @param addr read addr
 * @param dst data destination
 * @param size read size
 * @return int 0 success other if failed
 */
int dl_disk_read(struct disk_dev *dev, disk_addr_t addr, uint8_t *dst,
                 disk_size_t size)
{
#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (NULL == dev || NULL == dev->info) {
        ERROR("no disk device node\n");
        return -1;
    }

    if (!dev->block_size || !IS_ALIGNED(addr, dev->block_size)
            || !IS_ALIGNED(size, dev->block_size)) {
        ERROR("addr 0x%llx or size 0x%llx not aligned to block_size = %d\n", addr, size,
              dev->block_size);
        return -1;
    }

    if (!IS_ALIGNED(dst, dev->info->mem_align_size)) {
        ERROR("mem addresses 0x%p not aligned to 0x%x\n", dst,
              dev->info->mem_align_size);
        return -1;
    }

    return disk_read_block(dev, addr / (dev->block_size), dst,
                           size / (dev->block_size));
#else
    return disk_read(dev, addr, dst,  size);
#endif
}

/**
 * @brief dl_disk_write
 *
 * @param dev disk device
 * @param addr write addr
 * @param src data source
 * @param size write size
 * @return int 0 success other if failed
 */
int dl_disk_write(struct disk_dev *dev, disk_addr_t addr, uint8_t *src,
                  disk_size_t size)
{
#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (NULL == dev || NULL == dev->info) {
        ERROR("no disk device node\n");
        return -1;
    }

    if (!dev->block_size || !IS_ALIGNED(addr, dev->block_size)
            || !IS_ALIGNED(size, dev->block_size)) {
        ERROR("addr 0x%llx or size 0x%llx not aligned to block_size = %d\n", addr, size,
              dev->block_size);
        return -1;
    }

    if (!IS_ALIGNED(src, dev->info->mem_align_size)) {
        ERROR("mem addresses 0x%p not aligned to 0x%x\n", src,
              dev->info->mem_align_size);
        return -1;
    }

    return disk_write_block(dev, addr / (dev->block_size), src,
                            size / (dev->block_size));
#else
    return disk_write(dev, addr, src,  size);
#endif
}

/**
 * @brief dloader disk erase
 *
 * @param dev disk device
 * @param addr erase addr
 * @param size erase size
 * @return int 0 success
 */
int dl_disk_erase(struct disk_dev *dev, disk_addr_t addr, disk_size_t size)
{
#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (NULL == dev || NULL == dev->info) {
        ERROR("no disk device node\n");
        return -1;
    }

    if (strstr(dev->info->disk_name, "flash")) {
        if (!dev->info->erase_size || !IS_ALIGNED(addr, dev->info->erase_size)
                || !IS_ALIGNED(size, dev->info->erase_size)) {
            ERROR("addr 0x%llx or size 0x%llx not aligned to erase_size = %d\n", addr, size,
                  dev->info->erase_size);
            return -1;
        }

        return disk_erase_group(dev, addr / (dev->info->erase_size),
                                size / (dev->info->erase_size), DISK_ERASE_DEFAULT);
    }
    else {
        return disk_erase(dev, addr, size, DISK_ERASE_DEFAULT);
    }

#else
    return disk_erase(dev, addr, size, DISK_ERASE_DEFAULT);
#endif
}
