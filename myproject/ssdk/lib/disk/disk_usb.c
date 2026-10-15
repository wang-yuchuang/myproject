/**
 * @file disk_usb.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <errno.h>
#include <param.h>
#include <debug.h>

#include <Class/MSC/usbh_msc.h>
#include <lib/disk/disk_usb.h>


/* Maximum blocks of a single transmission */
#define USB_MAX_XFER_BLKS       (240u)

#define disk_usb_err(...)       ssdk_printf(SSDK_ERR, __VA_ARGS__)
#define disk_usb_warn(...)      ssdk_printf(SSDK_WARNING, __VA_ARGS__)


static int disk_usb_read(struct disk_info  *info,
                         uint8_t           *dst,
                         disk_addr_t        addr,
                         disk_size_t        size)
{
    USBH_MSC_DEV   *p_msc_dev;
    USBH_ERR        err_usbh;
    CPU_INT32U      blk_off;
    CPU_INT32U      blk_cnt;
    CPU_INT32U      rd_blks;

    p_msc_dev = (USBH_MSC_DEV *)info->private_data;
    if (!p_msc_dev || !dst || !size) {
        return -EINVAL;
    }

    blk_off = addr / info->block_align_size;
    blk_cnt = size / info->block_align_size;
    if ((addr != (disk_addr_t)blk_off * info->block_align_size) ||
        (size != (disk_size_t)blk_cnt * info->block_align_size)) {
        return -EINVAL;
    }

    if (!IS_ALIGNED(dst, CONFIG_ARCH_CACHE_LINE) ||
        !IS_ALIGNED(dst + size, CONFIG_ARCH_CACHE_LINE)) {
        disk_usb_warn("unaligned rx buffer %p ++ %llx may cause incorrect data\n",
                      dst, size);
    }

    while (blk_cnt > 0) {
        rd_blks = blk_cnt > USB_MAX_XFER_BLKS ? USB_MAX_XFER_BLKS : blk_cnt;

        (void)USBH_MSC_Rd(             p_msc_dev,
                                       0u,
                          (CPU_INT32U) blk_off,
                          (CPU_INT16U) rd_blks,
                          (CPU_INT32U) info->block_align_size,
                                       dst,
                                      &err_usbh);

        if (err_usbh != USBH_ERR_NONE) {
            disk_usb_err("%s: Could not rd MSC dev, USB err: %d.\n", __func__, err_usbh);
            return -EIO;
        }

        blk_cnt -= rd_blks;
        blk_off += rd_blks;
        dst += rd_blks * info->block_align_size;
    }

    return 0;
}

static int disk_usb_write(struct disk_info *info,
                          const uint8_t    *src,
                          disk_addr_t       addr,
                          disk_size_t       size)
{
    USBH_MSC_DEV   *p_msc_dev;
    USBH_ERR        err_usbh;
    CPU_INT32U      blk_off;
    CPU_INT32U      blk_cnt;
    CPU_INT32U      wr_blks;

    p_msc_dev = (USBH_MSC_DEV *)info->private_data;
    if (!p_msc_dev || !src || !size) {
        return -EINVAL;
    }

    blk_off = addr / info->block_align_size;
    blk_cnt = size / info->block_align_size;
    if ((addr != (disk_addr_t)blk_off * info->block_align_size) ||
        (size != (disk_size_t)blk_cnt * info->block_align_size)) {
        return -EINVAL;
    }

    if (!IS_ALIGNED(src, CONFIG_ARCH_CACHE_LINE) ||
        !IS_ALIGNED(src + size, CONFIG_ARCH_CACHE_LINE)) {
        disk_usb_warn("unaligned tx buffer %p ++ %llx may cause incorrect data\n",
                      src, size);
    }

    while (blk_cnt > 0) {
        wr_blks = blk_cnt > USB_MAX_XFER_BLKS ? USB_MAX_XFER_BLKS : blk_cnt;

        (void)USBH_MSC_Wr(             p_msc_dev,
                                       0u,
                          (CPU_INT32U) blk_off,
                          (CPU_INT16U) wr_blks,
                          (CPU_INT32U) info->block_align_size,
                                       src,
                                      &err_usbh);

        if (err_usbh != USBH_ERR_NONE) {
            printf("%s: Could not wr MSC dev, USB err: %d.\n", __func__, err_usbh);
            return -EIO;
        }

        blk_cnt -= wr_blks;
        blk_off += wr_blks;
        src += wr_blks * info->block_align_size;
    }

    return 0;
}

static int disk_usb_status(struct disk_info *info)
{
    return DISK_STATUS_OK;
}

static int disk_usb_ioctl(struct disk_info *info, uint32_t cmd, void *buff)
{
    return 0;
}

static struct disk_operations disk_usb_ops = {
    .disk_read = disk_usb_read,
    .disk_write = disk_usb_write,

    .disk_status = disk_usb_status,
    .disk_ioctl = disk_usb_ioctl,
};

int register_usb_disk(struct disk_info *info)
{
    USBH_MSC_DEV   *p_msc_dev;
    USBH_ERR        err_usbh;
    CPU_INT32U      blk_nr, blk_sz;
    int             ret = 0;

    if (!info->disk_name || !info->private_data) {
        return -1;
    }

    p_msc_dev = (USBH_MSC_DEV *)info->private_data;

    err_usbh  = USBH_MSC_Init(p_msc_dev, 0u);
    if (err_usbh != USBH_ERR_NONE) {
        disk_usb_err("%s: Could not init MSC dev, USB err: %d.\n", __func__, err_usbh);
        return -err_usbh;
    }

    err_usbh = USBH_MSC_CapacityRd( p_msc_dev,
                                    0u,
                                   &blk_nr,
                                   &blk_sz);

    if (err_usbh != USBH_ERR_NONE) {
        disk_usb_err("%s: Could not get MSC dev capacity, USB err: %d.\n",
                     __func__, err_usbh);
        return -err_usbh;
    }

    info->cache = NULL;
    info->disk_size = (uint64_t)blk_nr * blk_sz;
    info->disk_offset = 0;
    info->erase_size = blk_sz;
    info->mem_align_size = CONFIG_ARCH_CACHE_LINE;
    info->block_align_size = blk_sz;
    info->disk_ops = &disk_usb_ops;

    ret = register_disk(info);
    if (ret < 0) {
        disk_usb_err("usb disk register err:%d\n", ret);
        return ret;
    }

    return 0;
}

int unregister_usb_disk(struct disk_info *info)
{
    int ret = 0;

    ret = unregister_disk(info);
    if (ret < 0) {
        disk_usb_err("usb disk unregister err:%d\n", ret);
        return ret;
    }

    return ret;
}
