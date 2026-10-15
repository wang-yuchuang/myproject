/**
 * @file disk_ram.c
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

#include <lib/disk/disk.h>
#include <lib/disk/disk_ram.h>

static int memcpy_ram(struct disk_ram_info *ram_info, void *dst,
                      const void *src, disk_size_t size)
{
    osMutexAcquire(ram_info->disk_ram_mutex, osWaitForever);
    memcpy(dst, (void *)src, size);
    osMutexRelease(ram_info->disk_ram_mutex);
    return 0;
}

static int memset_ram(struct disk_ram_info *ram_info, void *dst, uint8_t c,
                      disk_size_t size)
{
    osMutexAcquire(ram_info->disk_ram_mutex, osWaitForever);
    memset(dst, c, size);
    osMutexRelease(ram_info->disk_ram_mutex);
    return 0;
}

static int disk_ram_read(struct disk_info *info, uint8_t *dst, disk_addr_t addr,
                         disk_size_t size)
{
    struct disk_ram_info *ram_info = info->private_data;
    void *raddr = (uint8_t *)ram_info->config.mem_base + info->disk_offset + addr;
    return memcpy_ram(ram_info, dst, (void *)raddr, size);
}

static int disk_ram_write(struct disk_info *info, const uint8_t *src,
                          disk_addr_t addr, disk_size_t size)
{
    struct disk_ram_info *ram_info = info->private_data;
    void *raddr = (uint8_t *)ram_info->config.mem_base + info->disk_offset + addr;
    return memcpy_ram(ram_info, (void *)raddr, src, size);;
}

static int disk_ram_read_block(struct disk_info *info, uint8_t *dst,
                               block_t sector, block_count_t count, block_size_t blk_sz)
{
    disk_size_t rlen = 0;
    struct disk_ram_info *ram_info = info->private_data;
    void *raddr = (uint8_t *)ram_info->config.mem_base + info->disk_offset +
                  (disk_size_t)sector * blk_sz;
    rlen = (disk_size_t)count * blk_sz;
    return memcpy_ram(ram_info, dst, (void *)raddr, rlen);;
}

static int disk_ram_write_block(struct disk_info *info, const uint8_t *src,
                                block_t sector, block_count_t count, block_size_t blk_sz)
{
    disk_size_t rlen = 0;
    struct disk_ram_info *ram_info = info->private_data;
    void *raddr = (uint8_t *)ram_info->config.mem_base + info->disk_offset +
                  (disk_size_t)sector * blk_sz;
    rlen = (disk_size_t)count * blk_sz;
    return memcpy_ram(ram_info, (void *)raddr, src, rlen);;
}

static int disk_ram_erase(struct disk_info *info, disk_addr_t addr,
                          disk_size_t size, uint32_t flag)
{
    struct disk_ram_info *ram_info = info->private_data;
    void *raddr = (uint8_t *)ram_info->config.mem_base + info->disk_offset + addr;

    switch (flag) {
        case DISK_ERASE_DEFAULT:
        case DISK_ERASE_ZERO:
            return memset_ram(ram_info, (void *)raddr, 0x0, size);

        case DISK_ERASE_ONE:
            return memset_ram(ram_info, (void *)raddr, 0xff, size);

        default:
            ssdk_printf(SSDK_ERR, "disk ram erase flag error\n");
            return -1;
    }
}

static int disk_ram_status(struct disk_info *info)
{
    int ret = 0;
    ret = DISK_STATUS_OK;
    return ret;
}

static int disk_ram_ioctl(struct disk_info *info, uint32_t cmd, void *buff)
{
    int ret = 0;
    return ret;
}

struct disk_operations disk_ram_ops = {
    .disk_read = disk_ram_read,
    .disk_write = disk_ram_write,
    .disk_read_block = disk_ram_read_block,
    .disk_write_block = disk_ram_write_block,
    .disk_erase = disk_ram_erase,
    .disk_status = disk_ram_status,
    .disk_ioctl = disk_ram_ioctl,
};

int register_ram_disk(struct disk_ram_info *ram_info)
{
    int ret = 0;

    struct disk_info *info = &ram_info->info;
    struct disk_ram_config *ram_config = &ram_info->config;

    /* init ram_info */
    ram_info->disk_ram_mutex = osMutexNew(NULL);
    /* base on config init info */
    info->disk_name = ram_config->disk_name;
    info->disk_size = ram_config->disk_size;
    info->disk_offset = 0;
    info->erase_size = CONFIG_ARCH_CACHE_LINE;
    info->mem_align_size = CONFIG_ARCH_CACHE_LINE;
    info->block_align_size = CONFIG_ARCH_CACHE_LINE;
    info->private_data = ram_info;
    info->disk_ops = &disk_ram_ops;

    ssdk_printf(SSDK_INFO, "register ram disk:%s disk_size:%lld ram_addr:%p\n",
                info->disk_name, info->disk_size, ram_config->mem_base);
    /* register ram disk */
    ret = register_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "ram disk register:%d\n", ret);
        return ret;
    }

    return ret;
}

int unregister_ram_disk(struct disk_ram_info *ram_info)
{
    int ret = 0;
    struct disk_info *info = &ram_info->info;

    ssdk_printf(SSDK_INFO, "unregister ram disk:%s\n", info->disk_name);
    /* free ram_info */
    osMutexDelete(ram_info->disk_ram_mutex);

    /* unregister ram disk */
    ret = unregister_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "ram disk unregister:%d\n", ret);
        return ret;
    }

    return ret;
}
