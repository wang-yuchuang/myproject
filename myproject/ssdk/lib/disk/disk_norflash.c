/**
 * @file disk_norflash.c
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
#include <arch/cache.h>

#include <lib/disk/disk.h>
#include <lib/disk/disk_norflash.h>

static inline int norflash_dev_read(struct disk_norflash_info *norflash_info,
                      disk_addr_t addr, uint8_t *buf, disk_size_t length)
{
    if(norflash_info->config.norflash_read)
        return norflash_info->config.norflash_read(
                        norflash_info->config.norflash_dev, addr, buf, length);
    else
        return -DISK_ERROR_READ_ACCESS;
}

static inline int norflash_dev_write(struct disk_norflash_info *norflash_info,
                       disk_addr_t addr, const uint8_t *buf,
                       disk_size_t length)
{
    if(norflash_info->config.norflash_write)
        return norflash_info->config.norflash_write(
                        norflash_info->config.norflash_dev, addr, buf, length);
    else
        return -DISK_ERROR_WRITE_ACCESS;
}

static inline int norflash_dev_erase(struct disk_norflash_info *norflash_info,
                       disk_addr_t addr, disk_size_t length)
{
    if(norflash_info->config.norflash_erase)
        return norflash_info->config.norflash_erase(
                             norflash_info->config.norflash_dev, addr, length);
    else
        return -DISK_ERROR_ERASE_ACCESS;
}

static int disk_norflash_read(struct disk_info *info, uint8_t *dst, disk_addr_t addr,
                       disk_size_t size)
{
    int ret = 0;
    struct disk_norflash_info *norflash_info = info->private_data;

    ssdk_printf(SSDK_DEBUG, "norflash disk read:%s mem:%p addr:%lld size:%lld\n",
            info->disk_name, dst, addr, size);

    disk_size_t tmp_len = 0;

    while (size) {
        tmp_len = size;
        if (!IS_ALIGNED(dst, CONFIG_ARCH_CACHE_LINE)) {
            tmp_len = MIN(ROUNDUP((addr_t)dst, CONFIG_ARCH_CACHE_LINE) - (addr_t)dst, size);
        }

        tmp_len = tmp_len > CONFIG_ARCH_CACHE_LINE ?
                  (tmp_len - tmp_len % CONFIG_ARCH_CACHE_LINE) : tmp_len;

        if (tmp_len < CONFIG_ARCH_CACHE_LINE) {
            osMutexAcquire(norflash_info->disk_norflash_mutex, osWaitForever);
            if (norflash_dev_read(norflash_info, addr, norflash_info->buff_sector,
                                  ROUNDUP(tmp_len, 4))) {
                ret= -1;
                goto error;
            }
            memcpy(dst, norflash_info->buff_sector, tmp_len);
            osMutexRelease(norflash_info->disk_norflash_mutex);
        } else {
            if (norflash_dev_read(norflash_info, addr, dst, tmp_len)) {
                return -1;
            }
        }

        addr += tmp_len;
        size -= tmp_len;
        dst += tmp_len;
    }

    return 0;

error:
    osMutexRelease(norflash_info->disk_norflash_mutex);
    return ret;
}

static int disk_norflash_write(struct disk_info *info, const uint8_t *src, disk_addr_t addr, disk_size_t size)
{
    int ret = 0;
    struct disk_norflash_info *norflash_info = info->private_data;
    ssdk_printf(SSDK_DEBUG, "norflash disk write:%s mem:%p addr:%lld size:%lld\n",
            info->disk_name, src, addr, size);

    uint32_t sector_size = norflash_info->config.sector_size;
    uint32_t tmp_len;
    uint8_t *buf;

    while (size) {
        buf = norflash_info->buff_sector;
        tmp_len = size;
        if (!IS_ALIGNED(addr, sector_size)) {
            tmp_len = MIN(ROUNDUP(addr, sector_size) - addr, size);
        } else if (IS_ALIGNED(src, CONFIG_ARCH_CACHE_LINE) &&
                   size >= sector_size) {
            buf = (uint8_t *)src;
        }

        tmp_len = tmp_len > sector_size ? (tmp_len - tmp_len % sector_size) : tmp_len;
        if (src == buf) {
            if (norflash_dev_erase(norflash_info, addr, tmp_len)) {
                return -1;
            }

            if (norflash_dev_write(norflash_info, addr, buf, tmp_len)) {
                return -1;
            }
        } else {
            if (tmp_len < sector_size) {
                osMutexAcquire(norflash_info->disk_norflash_mutex, osWaitForever);
                if (norflash_dev_read(norflash_info, ROUNDDOWN(addr, sector_size),
                                      buf, sector_size)) {
                    ret = -1;
                    goto error;
                }

                if (norflash_dev_erase(norflash_info,
                                       ROUNDDOWN(addr, sector_size),
                                       sector_size)) {
                    return -1;
                }
            } else {
                if (norflash_dev_erase(norflash_info, addr, tmp_len)) {
                    return -1;
                }
                osMutexAcquire(norflash_info->disk_norflash_mutex, osWaitForever);
            }

            for (disk_size_t size_tmp = 0;
                 size_tmp < ROUNDUP(tmp_len, sector_size);
                 size_tmp += sector_size) {
                 memcpy(buf + ((addr + size_tmp)  % sector_size), src,
                        MIN(sector_size, tmp_len));
                if (norflash_dev_write(norflash_info,
                                        ROUNDDOWN(addr + size_tmp, sector_size),
                                        buf, sector_size)) {
                    ret = -1;
                    goto error;
                }
            }
            osMutexRelease(norflash_info->disk_norflash_mutex);
        }

        addr += tmp_len;
        src += tmp_len;
        size -= tmp_len;
    }

    return 0;

error:
    osMutexRelease(norflash_info->disk_norflash_mutex);
    return ret;
}

static int disk_norflash_read_block(struct disk_info *info, uint8_t *dst,
                             block_t block, block_count_t count,
                             block_size_t blk_sz)
{
    ssdk_printf(SSDK_DEBUG, "disk norflash read block:%s mem:%p block:%lld"
                "count:%d blk_sz:%d addr:%lld size:%d\n", info->disk_name, dst,
                block, count, blk_sz, block * blk_sz, count * blk_sz);

    return disk_norflash_read(info, dst, (disk_addr_t)block * blk_sz,
                              (disk_size_t)count * blk_sz);
}

static int disk_norflash_write_block(struct disk_info *info, const uint8_t *src,
                              block_t block, block_count_t count,
                              block_size_t blk_sz)
{
    disk_size_t write_len;
    disk_size_t remain_len = count * blk_sz;
    disk_addr_t addr = block * blk_sz;
    struct disk_norflash_info *norflash_info = info->private_data;

    ssdk_printf(SSDK_DEBUG, "disk norflash write block:%s mem:%p block:%lld"
                "count:%d blk_sz:%d addr:%lld size:%d\n",info->disk_name, src,
                block, count, blk_sz, block * blk_sz, count * blk_sz);

    while (remain_len) {
        write_len = remain_len;
        if (!IS_ALIGNED(src, CONFIG_ARCH_CACHE_LINE)) {
            write_len = MIN(ROUNDUP((addr_t)src, CONFIG_ARCH_CACHE_LINE) - (addr_t)src, write_len);
        }

        write_len = write_len > CONFIG_ARCH_CACHE_LINE ?
                  ROUNDDOWN(write_len, CONFIG_ARCH_CACHE_LINE): write_len;

        if (write_len < CONFIG_ARCH_CACHE_LINE) {
            osMutexAcquire(norflash_info->disk_norflash_mutex, osWaitForever);
            memcpy(norflash_info->buff_sector, src, write_len);
            if (norflash_dev_write(norflash_info, addr, norflash_info->buff_sector,
                                   ROUNDUP(write_len, 4))) {
                osMutexRelease(norflash_info->disk_norflash_mutex);
                return -1;
            }
            osMutexRelease(norflash_info->disk_norflash_mutex);
        } else {
            if (norflash_dev_write(norflash_info, addr, src, write_len)) {
                return -1;
            }
        }

        addr += write_len;
        src += write_len;
        remain_len -= write_len;
    }

    return 0;
}

static int disk_norflash_erase(struct disk_info *info, disk_addr_t addr,
                               disk_size_t size, uint32_t flag)
{
    int ret = 0;
    struct disk_norflash_info *norflash_info = info->private_data;
    ssdk_printf(SSDK_DEBUG, "norflash disk %s erase,addr:%lld size:%lld:"
                "flag:%d\n", info->disk_name, addr, size, flag);

    uint32_t sector_size = norflash_info->config.sector_size;
    uint32_t tmp_len;
    uint8_t *buf;

    while (size) {
        buf = norflash_info->buff_sector;
        tmp_len = size;
        if (!IS_ALIGNED(addr, sector_size)) {
            tmp_len = MIN(ROUNDUP(addr, sector_size) - addr, size);
        }

        tmp_len = tmp_len > sector_size ?
                  (tmp_len - tmp_len % sector_size) : tmp_len;

        if (tmp_len < sector_size) {
            osMutexAcquire(norflash_info->disk_norflash_mutex, osWaitForever);
            if (norflash_dev_read(norflash_info, ROUNDDOWN(addr, sector_size),
                                  buf, sector_size)) {
                ret = -1;
                goto error;
            }

            if (norflash_dev_erase(norflash_info, ROUNDDOWN(addr, sector_size),
                                   sector_size) < 0) {
                ret = -1;
                goto error;
            }

            memset(buf + addr % sector_size, 0xff, tmp_len);

            if (norflash_dev_write(norflash_info, ROUNDDOWN(addr, sector_size),
                                    buf, sector_size)) {
                ret = -1;
                goto error;
            }
            osMutexRelease(norflash_info->disk_norflash_mutex);
        } else {
            if (norflash_dev_erase(norflash_info, addr, tmp_len)) {
                return -1;
            }
        }

        addr += tmp_len;
        size -= tmp_len;
    }

    return 0;

error:
    osMutexRelease(norflash_info->disk_norflash_mutex);
    return ret;
}

static int disk_norflash_erase_group(struct disk_info *info,
                                     uint32_t erase_block, uint32_t count,
                                     uint32_t erase_sz, uint32_t flag)
{
    return norflash_dev_erase(info->private_data,
                             (disk_addr_t)erase_block * erase_sz,
                             (disk_size_t)count * erase_sz);
}

static int disk_norflash_status(struct disk_info *info)
{
    return DISK_STATUS_OK;
}

static int disk_norflash_ioctl(struct disk_info *info, uint32_t cmd, void *buff)
{
    return 0;
}

static struct disk_operations disk_norflash_ops = {
#ifdef CONFIG_DISK_NOR_FLASH_BYTES_API_SUPPORT
    .disk_read = disk_norflash_read,
    .disk_write = disk_norflash_write,
    .disk_erase = disk_norflash_erase,
#else
    .disk_read = NULL,
    .disk_write = NULL,
    .disk_erase = NULL,
#endif
    .disk_read_block = disk_norflash_read_block,
    .disk_write_block = disk_norflash_write_block,

    .disk_erase_group = disk_norflash_erase_group,
    .disk_status = disk_norflash_status,
    .disk_ioctl = disk_norflash_ioctl,
};

int register_norflash_disk(struct disk_norflash_info *norflash_info)
{
    int ret = 0;

    struct disk_info *info = &norflash_info->info;
    struct disk_norflash_config *norflash_config = &norflash_info->config;
#ifdef CONFIG_DISK_NOR_FLASH_BYTES_API_SUPPORT
    norflash_info->buff_size = norflash_config->sector_size;
#else
    norflash_info->buff_size = CONFIG_ARCH_CACHE_LINE;
#endif
    norflash_info->buff_sector = osAllocAlign(CONFIG_ARCH_CACHE_LINE, norflash_info->buff_size);

    if(NULL == norflash_info->buff_sector) {
        ssdk_printf(SSDK_ERR, "norflash disk %s buff malloc error\n",
                    norflash_config->disk_name);
        return -1;
    }

    norflash_info->disk_norflash_mutex = osMutexNew(NULL);

    /* base on config init info */
    info->disk_name = norflash_config->disk_name;
    info->disk_size = norflash_config->disk_size;
    info->disk_offset = 0;
    info->erase_size = norflash_config->sector_size;
    info->mem_align_size = CONFIG_ARCH_CACHE_LINE;
    info->block_align_size = 4;
    info->private_data = norflash_info;
    info->disk_ops = &disk_norflash_ops;

#ifdef CONFIG_DISK_CACHE_NOOP
    struct disk_cache_noop *cache_noop = &norflash_info->cache_noop;
    /* base on config cache,cache in disk dev layer??? */
    cache_noop->cache_align_size = 4u * 1024u;
    cache_noop->cache.private_data = cache_noop;
    ret = register_disk_cache_noop(cache_noop);
    if(ret < 0) {
        ssdk_printf(SSDK_ERR, "norflash disk register cache noop:%d\n",ret);
        return ret;
    }
#endif

    ssdk_printf(SSDK_INFO, "register norflash disk:%s disk_size:%lld"
                "erase_size:%d sector_size:%d\n", info->disk_name,
                info->disk_size, info->erase_size,
                norflash_config->sector_size);

    ret = register_disk(info);
    if(ret < 0) {
        ssdk_printf(SSDK_ERR, "norflash disk register:%d\n",ret);
        return ret;
    }

    return 0;
}
int unregister_norflash_disk(struct disk_norflash_info *norflash_info)
{
    int ret = 0;
    struct disk_info *info = &norflash_info->info;

    ssdk_printf(SSDK_INFO, "unregister norflash disk:%s\n", info->disk_name);

#ifdef CONFIG_DISK_CACHE_NOOP
    struct disk_cache_noop *cache_noop = &norflash_info->cache_noop;
    /* unregister disk cache noop */
    ret = unregister_disk_cache_noop(cache_noop);
    if(ret < 0) {
        ssdk_printf(SSDK_ERR, "norflash disk unregister cache noop:%d\n",ret);
        return ret;
    }
#endif

    free(norflash_info->buff_sector);
    osMutexDelete(norflash_info->disk_norflash_mutex);

    ret = unregister_disk(info);
    if(ret < 0) {
        ssdk_printf(SSDK_ERR, "norflash disk unregister:%d\n",ret);
        return ret;
    }
    return ret;
}

