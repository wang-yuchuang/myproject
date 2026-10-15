/**
 * @file update_monitor_diskio.
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * update_monitor disk io interface
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <regs_base.h>
#include <compiler.h>
#include <types.h>
#include <debug.h>
#include <lib/disk/disk.h>
#include "update_monitor_diskio.h"
#include "update_monitor.h"

/**
 * @brief open disk for update monitor
 * @param[in] disk_name disk_name to open
 * @param[in] dev disk_device
 * @return int 0 is success, -1 is failed
 */
int um_disk_init(const char* disk_name, struct disk_dev* dev)
{
    if(!dev || !disk_name) {
        ERROR("para error\n");
        return -1;
    }

    disk_close(dev);
    memset(dev, 0, sizeof(struct disk_dev));

    if (disk_open(disk_name, dev)) {
        ERROR("open %s failed \n", disk_name);
        return -1;
    }

    disk_set_block_size(dev, UPDATE_BLOCK_SIZE);

    return 0;
}

/**
 * @brief close disk for update monitor
 * @param[in] pdrv disk number
 * @return int 0 is success, -1 is failed
 */
int um_disk_deinit(struct disk_dev* dev)
{
    if(!dev) {
        ERROR("para error\n");
        return -1;
    }

    return disk_close(dev);
}

/**
 * @brief update monitor read disk
 *
 * @param[in] dev disk_device
 * @param[in] addr address to read
 * @param[out] buf pointer to the read data
 * @param[in] size read size.
 * @return 0:success  other:fail
 */
int um_disk_read(struct disk_dev* dev, uint64_t addr, uint8_t *buf, uint64_t size)
{

    if (!dev || !(dev->info) || (dev->block_size != UPDATE_BLOCK_SIZE)) {
        ERROR("para error\n");
        return -1;
    }

    ASSERT(disk_get_block_size(dev) == UPDATE_BLOCK_SIZE);

    if (IS_ALIGNED(addr, dev->block_size) && IS_ALIGNED(size, dev->block_size) &&
            IS_ALIGNED(buf, dev->info->mem_align_size)) {
        return disk_read_block(dev, addr / (dev->block_size), buf, size / (dev->block_size));
    }
    else {
#if CONFIG_UM_BLOCK_IO_MODE
        ERROR("addr = 0x%llx, size = 0x%llx, block_size = %d\n", addr, size, dev->block_size);
        ERROR("buf addr = 0x%p, buf align addr = 0x%x\n", buf, dev->info->mem_align_size);
        return -1;
#else
        return disk_read(dev, addr, buf,  size);
#endif

    }
}

/**
 * @brief update monitor erase disk
 *
 * @param[in] dev disk_device
 * @param[in] addr address to erase
 * @param[in] size erase size.
 * @return 0:success  other:fail
 */
int um_disk_erase(struct disk_dev* dev, uint64_t addr, uint64_t size)
{
    if (!dev || !(dev->info)) {
        ERROR("para error\n");
        return -1;
    }

    if (strstr(dev->info->disk_name, "flash") && dev->info->erase_size
                                && IS_ALIGNED(addr, dev->info->erase_size)
                                && IS_ALIGNED(size, dev->info->erase_size)) {
        return disk_erase_group(dev, addr / (dev->info->erase_size),
                                size / (dev->info->erase_size), DISK_ERASE_DEFAULT);
    }
    else {
        if(strstr(dev->info->disk_name, "flash")) {
#if CONFIG_UM_BLOCK_IO_MODE
            ERROR("addr = 0x%llx, size = 0x%llx, erase size = %d\n", addr, size, dev->info->erase_size);
            return -1;
#else
            return disk_erase(dev, addr, size, DISK_ERASE_DEFAULT);
#endif
        }
        else {
            return disk_erase(dev, addr, size, DISK_ERASE_DEFAULT);
        }
    }
}

/**
 * @brief update monitor write disk
 *
 * @param[in] dev disk_device
 * @param[in] addr address to read
 * @param[in] buf pointer to the data to be written
 * @param[in] size read size.
 * @return 0:success  other:fail
 */
int um_disk_write(struct disk_dev* dev, uint64_t addr, uint8_t *buf, uint64_t size)
{
    if (!dev || !(dev->info)) {
        ERROR("para error\n");
        return -1;
    }

    ASSERT(disk_get_block_size(dev) == UPDATE_BLOCK_SIZE);

    if (IS_ALIGNED(addr, dev->block_size) && IS_ALIGNED(size, dev->block_size) &&
            IS_ALIGNED(buf, dev->info->mem_align_size)) {
        return disk_write_block(dev, addr / (dev->block_size), buf, size / (dev->block_size));
    }
    else {
#if CONFIG_UM_BLOCK_IO_MODE
        ERROR("addr = 0x%llx, size = 0x%llx, block_size = %d\n", addr, size, dev->block_size);
        ERROR("buf addr = 0x%p, buf align addr = 0x%x\n", buf, dev->info->mem_align_size);
        return -1;
#else
        return disk_write(dev, addr, buf,  size);
#endif
    }
}

/**
 * @brief get the capacity of the storage device
 *
 * @param[in] dev disk_device
 * @return uint64_t reply 0 if failed，reply capacity if successful
 */
uint64_t um_get_capacity(struct disk_dev* dev)
{
    if (!dev || !(dev->info)) {
        ERROR("para error\n");
        return 0;
    }

    return disk_size(dev);
}

/**
 * @brief get the erase size of the storage device
 *
 * @param[in] dev disk_device
 * @return uint32_t reply 0 if failed，reply erase size if successful
 */
uint32_t um_get_erase_size(struct disk_dev* dev)
{
    if (!dev || !(dev->info)) {
        ERROR("para error\n");
        return 0;
    }

    return disk_erase_size(dev);
}

/**
 * @brief get the logic block size of the storage device
 *
 * @param[in] dev disk_device
 * @return uint32_t block size
 */
uint32_t um_get_block_size(struct disk_dev* dev)
{
    if (!dev || !(dev->info)) {
        ERROR("para error\n");
        return 0;
    }

    ASSERT(disk_get_block_size(dev) == UPDATE_BLOCK_SIZE);

    return disk_get_block_size(dev);
}