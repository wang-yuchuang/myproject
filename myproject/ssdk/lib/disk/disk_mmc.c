/**
 * @file disk_mmm.c
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
#include <lib/disk/disk_mmc.h>

/* Maximum value of a single transmission */
#define MMC_MAX_XFER_SIZE (0x1000000)

/* MMC physical layer API */
static int mmc_dev_read(struct disk_mmc_info *mmc_info, uint8_t *dst,
                        block_t sector, block_count_t count)
{
    if (mmc_info->config.mmc_read)
        return mmc_info->config.mmc_read(mmc_info->config.mmc_dev, dst, sector, count);
    else
        return -DISK_ERROR_READ_ACCESS;
}
static int mmc_dev_write(struct disk_mmc_info *mmc_info, const uint8_t *src,
                         block_t sector, block_count_t count)
{
    if (mmc_info->config.mmc_write)
        return mmc_info->config.mmc_write(mmc_info->config.mmc_dev, src, sector, count);
    else
        return -DISK_ERROR_WRITE_ACCESS;
}

static int mmc_dev_erase(struct disk_mmc_info *mmc_info, block_t block,
                         disk_size_t size)
{
    if (mmc_info->config.mmc_erase)
        return mmc_info->config.mmc_erase(mmc_info->config.mmc_dev, block, size);
    else
        return -DISK_ERROR_ERASE_ACCESS;
}

/* MMC buff reads */
static int mmc_buff_read(struct disk_mmc_info *mmc_info, uint8_t *buff_dst,
                         block_t sector, block_count_t sector_count,
                         uint8_t *dst, disk_addr_t addr, disk_size_t size)
{
    int ret = 0;
    uint32_t sector_size = mmc_info->config.sector_size;
    /* Protect the buffer */
    osMutexAcquire(mmc_info->disk_mmc_mutex, osWaitForever);
    ret = mmc_dev_read(mmc_info, buff_dst, sector, sector_count);

    if (ret)
        goto error;

    memcpy(dst, buff_dst + addr - ROUNDDOWN(addr, sector_size), size);
    osMutexRelease(mmc_info->disk_mmc_mutex);
    return 0;
error:
    osMutexRelease(mmc_info->disk_mmc_mutex);
    return ret;

}

static int disk_mmc_read(struct disk_info *info, uint8_t *dst, disk_addr_t addr,
                         disk_size_t size)
{
    int ret = 0;
    struct disk_mmc_info *mmc_info = info->private_data;

    uint8_t *buf = NULL;
    disk_size_t remaining = size;
    uint32_t sector_size = mmc_info->config.sector_size;
    uint32_t buff_size = mmc_info->buff_size;

    disk_addr_t sector_addr = 0;
    disk_size_t sector_count = 0;
    disk_size_t read_len = 0;
    disk_size_t max_read_length = 0;

    while (remaining) {
        /* Process the unaligned portion of the address */
        if (!IS_ALIGNED(addr, sector_size)) {
            ssdk_printf(SSDK_DEBUG, "mmc disk read %s address sector unaligned portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            sector_count = 1;
            read_len = MIN(ROUNDUP(addr, sector_size) - addr, remaining);
            ssdk_printf(SSDK_DEBUG, "mmc disk read addr:%llx read_len:%lld\n", addr, read_len);
        }
        /* Address & size aligned portion of sector,Memory CACHE_LINE alignment */
        else if ((remaining > sector_size)
                 && IS_ALIGNED(dst, CONFIG_ARCH_CACHE_LINE)) {
            ssdk_printf(SSDK_DEBUG,
                        "mmc disk read %s address sector aligned & dst CACHE_LINE aligned portion\n",
                        info->disk_name);
            buf = dst;
            sector_addr = addr / sector_size;
            max_read_length = MIN(mmc_info->mmc_max_xfer_size, remaining);
            sector_count = max_read_length / sector_size;
            read_len = sector_count * sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk read addr:%llx read_len:%lld\n", addr, read_len);
        }
        /* Address & size aligned portion of sector,Memory CACHE_LINE is not aligned */
        else {
            ssdk_printf(SSDK_DEBUG, "mmc disk read %s remaining portion\n", info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            read_len = MIN(buff_size, remaining);
            sector_count = ROUNDUP(read_len, sector_size) / sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk read addr:%llx read_len:%lld\n", addr, read_len);
        }

        if (buf == dst) {
            ret = mmc_dev_read(mmc_info, buf, sector_addr, sector_count);

            if (ret)
                return ret;
        }
        else {
            ret = mmc_buff_read(mmc_info, buf, sector_addr, sector_count, dst, addr,
                                read_len);

            if (ret)
                return ret;
        }

        /* Address & size offset */
        dst += read_len;
        addr += read_len;
        remaining -= read_len;
    }

    return 0;
}

static int mmc_buff_write(struct disk_mmc_info *mmc_info, uint8_t *buff_src,
                          block_t sector, block_count_t sector_count,
                          uint8_t *src, disk_addr_t addr, disk_size_t size)
{
    int ret = 0;
    uint32_t sector_size = mmc_info->config.sector_size;
    /* Protect the buffer */
    osMutexAcquire(mmc_info->disk_mmc_mutex, osWaitForever);

    if (IS_ALIGNED(addr, sector_size) && IS_ALIGNED(size, sector_size)) {
        memcpy(buff_src, src, size);
    }
    else {
        ret = mmc_dev_read(mmc_info, buff_src, sector, sector_count);

        if (ret)
            goto error;

        memcpy(buff_src + addr - ROUNDDOWN(addr, sector_size), src, size);
    }

    ret = mmc_dev_write(mmc_info, buff_src, sector, sector_count);

    if (ret)
        goto error;

    osMutexRelease(mmc_info->disk_mmc_mutex);
    return 0;
error:
    osMutexRelease(mmc_info->disk_mmc_mutex);
    return ret;
}

static int disk_mmc_write(struct disk_info *info, const uint8_t *src,
                          disk_addr_t addr, disk_size_t size)
{
    int ret = 0;
    struct disk_mmc_info *mmc_info = info->private_data;

    uint8_t *buf = NULL;
    disk_size_t remaining = size;
    uint32_t sector_size = mmc_info->config.sector_size;
    uint32_t buff_size = mmc_info->buff_size;

    disk_addr_t sector_addr = 0;
    disk_size_t sector_count = 0;
    disk_size_t write_len = 0;
    disk_size_t max_read_length = 0;

    while (remaining) {
        /* Process the unaligned portion of the address */
        if (!IS_ALIGNED(addr, sector_size)) {
            ssdk_printf(SSDK_DEBUG, "mmc disk write %s address sector unaligned portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            sector_count = 1;
            write_len = MIN(ROUNDUP(addr, sector_size) - addr, remaining);
            ssdk_printf(SSDK_DEBUG, "mmc disk write addr:%llx write_len:%lld\n", addr, write_len);
        }
        /* Address & size aligned portion of sector,Memory CACHE_LINE alignment */
        else if ((remaining > sector_size)
                 && IS_ALIGNED(src, CONFIG_ARCH_CACHE_LINE)) {
            ssdk_printf(SSDK_DEBUG,
                        "mmc disk write %s address sector aligned & dst CACHE_LINE aligned portion\n",
                        info->disk_name);
            buf = (uint8_t *)src;
            sector_addr = addr / sector_size;
            max_read_length = MIN(mmc_info->mmc_max_xfer_size, remaining);
            sector_count = max_read_length / sector_size;
            write_len = sector_count * sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk write addr:%llx write_len:%lld\n", addr, write_len);
        }
        else {
            ssdk_printf(SSDK_DEBUG, "mmc disk write %s remaining portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            write_len = MIN(buff_size, remaining);

            if (!IS_ALIGNED(write_len, sector_size) && (write_len > sector_size))
                write_len = ROUNDDOWN(write_len, sector_size);

            sector_count = ROUNDUP(write_len, sector_size) / sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk write addr:%llx write_len:%lld\n", addr, write_len);
        }

        if (buf == src) {
            ret = mmc_dev_write(mmc_info, buf, sector_addr, sector_count);

            if (ret)
                return ret;
        }
        else {
            ret = mmc_buff_write(mmc_info, buf, sector_addr, sector_count,
                                 (uint8_t *)src, addr, write_len);

            if (ret)
                return ret;
        }

        /* Address & size offset */
        src += write_len;
        addr += write_len;
        remaining -= write_len;
    }

    return 0;
}

static int disk_mmc_read_block(struct disk_info *info, uint8_t *dst,
                               block_t block, block_count_t count, block_size_t blk_sz)
{
    struct disk_mmc_info *mmc_info = info->private_data;
    uint32_t sector_size = mmc_info->config.sector_size;
    uint32_t blk_to_sector = blk_sz / sector_size;
    return mmc_dev_read(mmc_info, dst, block * blk_to_sector,
                        count * blk_to_sector);
}

static int disk_mmc_write_block(struct disk_info *info, const uint8_t *src,
                                block_t block, block_count_t count, block_size_t blk_sz)
{
    struct disk_mmc_info *mmc_info = info->private_data;
    uint32_t sector_size = mmc_info->config.sector_size;
    uint32_t blk_to_sector = blk_sz / sector_size;
    return mmc_dev_write(mmc_info, src, block * blk_to_sector,
                         count * blk_to_sector);
}

static int mmc_buff_erase(struct disk_mmc_info *mmc_info, uint8_t *buff,
                          block_t sector, block_count_t sector_count,
                          disk_addr_t addr, disk_size_t size, char flag)
{
    int ret = 0;
    uint32_t sector_size = mmc_info->config.sector_size;
    /* Protect the buff_sector buffer */
    osMutexAcquire(mmc_info->disk_mmc_mutex, osWaitForever);

    if (IS_ALIGNED(addr, sector_size) && IS_ALIGNED(size, sector_size)) {
        memset(buff, flag, size);
    }
    else {
        ret = mmc_dev_read(mmc_info, buff, sector, sector_count);

        if (ret)
            goto error;

        memset(buff + addr - ROUNDDOWN(addr, sector_size), flag, size);
    }

    ret = mmc_dev_write(mmc_info, buff, sector, sector_count);

    if (ret)
        goto error;

    osMutexRelease(mmc_info->disk_mmc_mutex);
    return 0;
error:
    osMutexRelease(mmc_info->disk_mmc_mutex);
    return ret;
}

#define MMC_DEFAULT_FLAG(flag) ((flag == DISK_ERASE_DEFAULT) || (flag == DISK_ERASE_ZERO))
static int disk_mmc_erase(struct disk_info *info, disk_addr_t addr,
                          disk_size_t size, uint32_t flag)
{
    int ret = 0;
    struct disk_mmc_info *mmc_info = info->private_data;

    uint8_t *buf = NULL;
    disk_size_t remaining = size;
    uint32_t sector_size = mmc_info->config.sector_size;
    uint32_t buff_size = mmc_info->buff_size;
    uint32_t erase_size = info->erase_size;

    disk_addr_t sector_addr = 0;
    disk_size_t sector_count = 0;
    disk_size_t erase_count = 0;
    disk_size_t erase_len = 0;

    /* configure c_flag */
    char c_flag = 0;

    if (MMC_DEFAULT_FLAG(flag))
        c_flag = 0x0;
    else
        c_flag = 0xff;

    while (remaining) {
        /* Process the sector unaligned portion of the address */
        if (!IS_ALIGNED(addr, sector_size)) {
            ssdk_printf(SSDK_DEBUG, "mmc disk erase %s address sector unaligned portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            sector_count = 1;
            erase_len = MIN(ROUNDUP(addr, sector_size) - addr, remaining);
            ssdk_printf(SSDK_DEBUG, "mmc disk erase addr:%llx erase_len:%lld\n", addr, erase_len);
        }
        /* Process the erase_size unaligned portion of the address */
        else if (!IS_ALIGNED(addr, erase_size) && (remaining > erase_size)) {
            ssdk_printf(SSDK_DEBUG, "mmc disk erase %s address erase_size unaligned portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            erase_len = MIN(buff_size, remaining);
            erase_len = MIN(ROUNDUP(addr, erase_size) - addr, erase_len);
            sector_count = erase_len / sector_size;
            erase_len = sector_count * sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk erase addr:%llx erase_len:%lld\n", addr, erase_len);
        }
        /* Address & size aligned portion of sector,Memory CACHE_LINE alignment */
        else if ((remaining > erase_size)  && MMC_DEFAULT_FLAG(flag)) {
            ssdk_printf(SSDK_DEBUG,
                        "mmc disk erase %s address erase_size aligned & size erase_size aligned portion\n",
                        info->disk_name);
            buf = NULL;
            sector_addr = addr / sector_size;
            erase_count = remaining / erase_size;
            erase_len = erase_count * erase_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk erase addr:%llx erase_len:%lld\n", addr, erase_len);
        }
        /* Address & size aligned portion of sector,Memory CACHE_LINE is not aligned */
        else {
            ssdk_printf(SSDK_DEBUG, "mmc disk erase %s remaining portion\n",
                        info->disk_name);
            buf = mmc_info->buff_sector;
            sector_addr = addr / sector_size;
            erase_len = MIN(buff_size, remaining);

            if (!IS_ALIGNED(erase_len, sector_size) && (erase_len > sector_size))
                erase_len = ROUNDDOWN(erase_len, sector_size);

            sector_count = ROUNDUP(erase_len, sector_size) / sector_size;
            ssdk_printf(SSDK_DEBUG, "mmc disk erase addr:%llx erase_len:%lld\n", addr, erase_len);
        }

        if (NULL == buf) {
            ret = mmc_dev_erase(mmc_info, sector_addr, erase_len);

            if (ret)
                return ret;
        }
        else {
            ret = mmc_buff_erase(mmc_info, buf, sector_addr, sector_count, addr, erase_len,
                                 c_flag);

            if (ret)
                return ret;
        }

        /* Address & size offset */
        addr += erase_len;
        remaining -= erase_len;
    }

    return 0;
}

static int disk_mmc_erase_group(struct disk_info *info, uint32_t erase_block,
                                uint32_t count, uint32_t erase_sz, uint32_t flag)
{
    struct disk_mmc_info *mmc_info = info->private_data;
    uint32_t sector_size = mmc_info->config.sector_size;

    if (erase_sz != info->erase_size) {
        ssdk_printf(SSDK_DEBUG,
                    "mmc disk %s erase group erase_sz error,erase_sz:0x%x erase_size:0x%x \n",
                    info->disk_name, erase_sz, info->erase_size);
        return -DISK_ERROR_INVALID_PARAMETER;
    }

    if (!IS_ALIGNED(erase_sz, sector_size)) {
        ssdk_printf(SSDK_DEBUG,
                    "mmc disk %s erase group erase_sz error,erase_sz:0x%x sector_size:0x%x \n",
                    info->disk_name, erase_sz, sector_size);
        return -DISK_ERROR_INVALID_PARAMETER;
    }

    uint32_t sector_addr = erase_block * erase_sz / sector_size;

    switch (flag) {
        case DISK_ERASE_DEFAULT:
        case DISK_ERASE_ZERO:
            return mmc_dev_erase(mmc_info, sector_addr, count * erase_sz);

        case DISK_ERASE_ONE:
            return disk_mmc_erase(info, (disk_addr_t)erase_block * erase_sz,
                                  (disk_size_t)count * erase_sz, flag);

        default:
            ssdk_printf(SSDK_ERR, "disk mmc erase flag error\n");
            return -1;
    }
}

static int disk_mmc_status(struct disk_info *info)
{
    return DISK_STATUS_OK;
}

static int disk_mmc_ioctl(struct disk_info *info, uint32_t cmd, void *buff)
{
    return 0;
}

struct disk_operations disk_mmc_ops = {
    .disk_read = disk_mmc_read,
    .disk_write = disk_mmc_write,
    .disk_read_block = disk_mmc_read_block,
    .disk_write_block = disk_mmc_write_block,
    .disk_erase = disk_mmc_erase,
    .disk_erase_group = disk_mmc_erase_group,
    .disk_status = disk_mmc_status,
    .disk_ioctl = disk_mmc_ioctl,
};

int register_mmc_disk(struct disk_mmc_info *mmc_info)
{
    int ret = 0;

    struct disk_info *info = &mmc_info->info;
    struct disk_mmc_config *mmc_config = &mmc_info->config;

    /* init mmc_info  */
    mmc_info->buff_size = 4 * 1024;

    if (!IS_ALIGNED(mmc_info->buff_size, mmc_config->sector_size)) {
        ssdk_printf(SSDK_ERR,
                    "mmc disk %s mmc_max_xfer_size not aligned to sector_size\n",
                    mmc_config->disk_name);
        return -1;
    }

    mmc_info->buff_sector = osAllocAlign(CONFIG_ARCH_CACHE_LINE,
                                         mmc_info->buff_size);

    if (NULL == mmc_info->buff_sector) {
        ssdk_printf(SSDK_ERR, "mmc disk %s buff malloc error\n", mmc_config->disk_name);
        return -1;
    }

    mmc_info->disk_mmc_mutex = osMutexNew(NULL);
    mmc_info->mmc_max_xfer_size = MMC_MAX_XFER_SIZE;

    if (!IS_ALIGNED(mmc_info->mmc_max_xfer_size, mmc_config->sector_size)) {
        ssdk_printf(SSDK_ERR,
                    "mmc disk %s mmc_max_xfer_size not aligned to sector_size\n",
                    mmc_config->disk_name);
        return -1;
    }

    /* base on config init info */
    info->disk_name = mmc_config->disk_name;
    info->disk_size = mmc_config->disk_size;
    info->disk_offset = 0;
    info->erase_size = mmc_config->erase_size;
    info->mem_align_size = CONFIG_ARCH_CACHE_LINE;
    info->block_align_size = mmc_config->sector_size;
    info->private_data = mmc_info;
    info->disk_ops = &disk_mmc_ops;

#ifdef CONFIG_DISK_CACHE_NOOP
    struct disk_cache_noop *cache_noop = &mmc_info->cache_noop;
    /* base on config cache,cache in disk dev layer??? */
    cache_noop->cache_align_size = 4u * 1024u;
    cache_noop->cache.private_data = cache_noop;
    ret = register_disk_cache_noop(cache_noop);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mmc disk register cache noop:%d\n", ret);
        return ret;
    }

#endif

    ssdk_printf(SSDK_INFO,
                "register mmc disk:%s disk_size:%lld erase_size:0x%x sector_size:0x%x\n",
                info->disk_name, info->disk_size, info->erase_size, mmc_config->sector_size);
    /* register mmc disk */
    ret = register_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mmc disk register:%d\n", ret);
        return ret;
    }

    return 0;
}
int unregister_mmc_disk(struct disk_mmc_info *mmc_info)
{
    int ret = 0;
    struct disk_info *info = &mmc_info->info;

    ssdk_printf(SSDK_INFO, "unregister mmc disk:%s\n", info->disk_name);

#ifdef CONFIG_DISK_CACHE_NOOP
    struct disk_cache_noop *cache_noop = &mmc_info->cache_noop;
    /* unregister disk cache noop */
    ret = unregister_disk_cache_noop(cache_noop);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mmc disk unregister cache noop:%d\n", ret);
        return ret;
    }

#endif

    free(mmc_info->buff_sector);
    osMutexDelete(mmc_info->disk_mmc_mutex);

    /* unregister mmc disk */
    ret = unregister_disk(info);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "mmc disk unregister:%d\n", ret);
        return ret;
    }

    return ret;
}
