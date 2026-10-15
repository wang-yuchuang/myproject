/**
 * @file disk_mapped.c
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
#include <lib/disk/disk_mapped.h>



static int disk_mapped_read(struct disk_info *info, uint8_t *dst, disk_addr_t addr,
                         disk_size_t size)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_read(base_disk_info, dst, addr + disk_offset, size);
}

static int disk_mapped_write(struct disk_info *info, const uint8_t *src,
                          disk_addr_t addr, disk_size_t size)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_write(base_disk_info, src, addr + disk_offset, size);
}

static int disk_mapped_read_block(struct disk_info *info, uint8_t *dst,
                               block_t sector, block_count_t count, block_size_t blk_sz)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_read_block(base_disk_info, dst, sector + disk_offset/blk_sz, count, blk_sz);
}

static int disk_mapped_write_block(struct disk_info *info, const uint8_t *src,
                                block_t sector, block_count_t count, block_size_t blk_sz)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_write_block(base_disk_info, src, sector + disk_offset/blk_sz, count, blk_sz);

}

static int disk_mapped_erase(struct disk_info *info, disk_addr_t addr,
                          disk_size_t size, uint32_t flag)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_erase(base_disk_info, addr + disk_offset, size, flag);
}

static int disk_mapped_erase_group(struct disk_info *info, uint32_t erase_block,
                                uint32_t count, uint32_t erase_sz, uint32_t flag)
{
    struct disk_mapped_config *mapped_config = &(((struct disk_mapped_info *)(info->private_data))->config);
    struct disk_info* base_disk_info = mapped_config->base_disk_info;
    uint64_t disk_offset = mapped_config->disk_offset;

    return base_disk_info->disk_ops->disk_erase_group(base_disk_info, erase_block + disk_offset/erase_sz, count, erase_sz, flag);
}

static int disk_mapped_status(struct disk_info *info)
{
    int ret = 0;
    ret = DISK_STATUS_OK;
    return ret;
}

static int disk_mapped_ioctl(struct disk_info *info, uint32_t cmd, void *buff)
{
    int ret = 0;
    return ret;
}

struct disk_operations disk_mapped_ops = {
    .disk_read = disk_mapped_read,
    .disk_write = disk_mapped_write,
    .disk_read_block = disk_mapped_read_block,
    .disk_write_block = disk_mapped_write_block,
    .disk_erase = disk_mapped_erase,
    .disk_erase_group = disk_mapped_erase_group,
    .disk_status = disk_mapped_status,
    .disk_ioctl = disk_mapped_ioctl,
};

int register_mapped_disk(struct disk_mapped_info *mapped_info)
{
    int ret = 0;
    struct disk_info *info = &mapped_info->info;
    struct disk_mapped_config *mapped_config = &mapped_info->config;
    struct disk_info *base_info = mapped_config->base_disk_info;

    /* base on config init info */
    info->disk_name = mapped_config->disk_name;
    info->disk_size = mapped_config->disk_size;
    info->disk_offset = 0;
    info->erase_size = base_info->erase_size;
    info->mem_align_size = base_info->mem_align_size;
    info->block_align_size = base_info->block_align_size;
    info->private_data = mapped_info;
    info->disk_ops = &disk_mapped_ops;

    ssdk_printf(SSDK_INFO, "register mapped disk:%s disk_size:%lld mapped_addr:%lld\n",
                info->disk_name, info->disk_size, mapped_config->disk_offset);

    /* register mapped disk */
    ret = register_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mapped disk register:%d\n", ret);
        return ret;
    }

    return ret;
}

int unregister_mapped_disk(struct disk_mapped_info *mapped_info)
{
    int ret = 0;
    struct disk_info *info = &mapped_info->info;

    ssdk_printf(SSDK_INFO, "unregister mapped disk:%s\n", info->disk_name);

    /* unregister mapped disk */
    ret = unregister_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mapped disk unregister:%d\n", ret);
        return ret;
    }

    return ret;
}
