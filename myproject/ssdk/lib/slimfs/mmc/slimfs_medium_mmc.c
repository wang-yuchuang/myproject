/**
 * @file slimfs_meidun_mmc.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs mmc medium operation interface
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <lib/disk/disk.h>
// #include <res.h>
// #include <mmc_hal.h>
// #include <boot.h>
#include <lib/partition/partition_parser.h>

#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs_log.h"
#include "common_utility.h"
/* =========================== marco define session ======================== */
#ifndef BOOT_DEVICE_GPT_START
#define BOOT_DEVICE_GPT_START 0x0
#endif
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
typedef struct {
    uint64_t pt_offset;
    uint64_t pt_size;
    struct disk_dev disk;
    partition_device_t* ptdev;
}mmc_medium_mgr_t;

/* =========================== function declare session ==================== */
static void* mmc_medium_init(void* arg);
static int32_t mmc_medium_read(void* hdl, uint8_t* rx_buf, uint64_t blk_addr, uint32_t blk_number);
static int32_t mmc_medium_write(void* hdl, const uint8_t* tx_buf, uint64_t blk_addr, uint32_t blk_number);
static void mmc_medium_deinit(void* hdl);
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
// static struct mmc_cfg mmc_config = {
//     .voltage = MMC_VOL_1_8,
//     .max_clk_rate = MMC_CLK_200MHZ,
//     .bus_width = MMC_BUS_WIDTH_8BIT,
//     .hs400_support = 1,
// };

static slimfs_medium_ops_t s_slimfs_mmc_ops = {
    .init = mmc_medium_init,
    .read = mmc_medium_read,
    .write = mmc_medium_write,
    .erase = NULL,
    .deinit = mmc_medium_deinit,
};

/* =========================== function implementation session ============= */
void* mmc_medium_init(void* arg)
{
    if (arg == NULL) {
        return NULL;
    }

    char *ptname = (char *)arg;
    int32_t ret = 0;

    mmc_medium_mgr_t* mmc_mgr_ptr = NULL;
    mmc_mgr_ptr = malloc(sizeof(mmc_medium_mgr_t));
    if (mmc_mgr_ptr == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "malloc memory failed for %s.\n", __func__);
        return NULL;
    }

    memset(mmc_mgr_ptr, 0x00, sizeof(mmc_medium_mgr_t));
    // setup storage device
    ret = disk_open(DISK_MMC_NAME(0), &mmc_mgr_ptr->disk);
    if ( ret != -DISK_ERROR_BUSY && ret != 0) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "setup storage failed while %s\n", __func__);
        goto err;
    }

    // setup partion device & read table
    mmc_mgr_ptr->ptdev = ptdev_setup(&mmc_mgr_ptr->disk, BOOT_DEVICE_GPT_START);
    if (mmc_mgr_ptr->ptdev == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "setup ptdev failed while %s\n", __func__);
        goto err;
    }
    ptdev_read_table(mmc_mgr_ptr->ptdev);

    // read offset and size
    mmc_mgr_ptr->pt_offset = ptdev_get_offset(mmc_mgr_ptr->ptdev, ptname);
    mmc_mgr_ptr->pt_size = ptdev_get_size(mmc_mgr_ptr->ptdev, ptname);
    if (!mmc_mgr_ptr->pt_offset &&
        !mmc_mgr_ptr->pt_size) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "can not fount partition: %s.\n", ptname);
        goto err;
    }

    SLIMFS_PRINT(SLIMFS_LOG_INFO, "get partition: %s info: offset = 0x%llx, size: 0x%llx\n"
                , ptname
                , mmc_mgr_ptr->pt_offset
                , mmc_mgr_ptr->pt_size);

    return mmc_mgr_ptr;

err:
    free(mmc_mgr_ptr);
    return NULL;
}

int32_t mmc_medium_read(void* hdl, uint8_t* rx_buf, uint64_t blk_addr, uint32_t blk_number)
{
    if (hdl == NULL || rx_buf == NULL) {
        return -1;
    }

    int32_t ret = 0;
    mmc_medium_mgr_t* mmc_mgr_ptr = (mmc_medium_mgr_t*)hdl;

    ret = disk_read(&mmc_mgr_ptr->disk
                    , mmc_mgr_ptr->pt_offset + blk_addr * CONFIG_FLASH_SECTOR_SIZE
                    , rx_buf
                    , CONFIG_FLASH_SECTOR_SIZE * blk_number);

    return ret;
}

int32_t mmc_medium_write(void* hdl, const uint8_t* tx_buf, uint64_t blk_addr, uint32_t blk_number)
{
    if (hdl == NULL || tx_buf == NULL) {
        return -1;
    }

    int32_t ret = 0;
    mmc_medium_mgr_t* mmc_mgr_ptr = (mmc_medium_mgr_t*)hdl;

    ret = disk_write(&mmc_mgr_ptr->disk
                    , mmc_mgr_ptr->pt_offset + blk_addr * CONFIG_FLASH_SECTOR_SIZE
                    , tx_buf
                    , CONFIG_FLASH_SECTOR_SIZE * blk_number);
    return ret;
}

void mmc_medium_deinit(void* hdl)
{
    if (hdl == NULL) {
        return;
    }

    mmc_medium_mgr_t* mmc_mgr_ptr = (mmc_medium_mgr_t *)hdl;

    disk_close(&mmc_mgr_ptr->disk);

    ptdev_destroy(mmc_mgr_ptr->ptdev);

    free(mmc_mgr_ptr);

    return;
}

slimfs_medium_ops_t* slimfs_get_medium_mmc_ops()
{
    return &s_slimfs_mmc_ops;
}
/* end of file slimfs_meidun_mmc.c */