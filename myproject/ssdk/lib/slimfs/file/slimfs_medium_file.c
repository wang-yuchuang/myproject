/**
 * @file slimfs_medium_file.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs file medium operation interface
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "sdrv_slimfs_impl.h"
#include "sdrv_slimfs_log.h"
#include "common_utility.h"
/* =========================== marco define session ======================== */
#define SLIMFS_FILE_MEDIUM_SECTOR_NUM   (2 * 1024 * 10)
#define SLIMFS_FILE_MEDIUM_SIZE         (CONFIG_FLASH_SECTOR_SIZE * SLIMFS_FILE_MEDIUM_SECTOR_NUM)  /**< 512, 10M */

/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
static void* file_medium_init(void* arg);
static int32_t file_medium_read(void* hdl, uint8_t* rx_buf, uint64_t blk_addr, uint32_t blk_number);
static int32_t file_medium_write(void* hdl, const uint8_t* tx_buf, uint64_t blk_addr, uint32_t blk_number);
static void file_medium_deinit(void* hdl);

/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
static slimfs_medium_ops_t s_slimfs_file_ops = {
    .init = file_medium_init,
    .read = file_medium_read,
    .write = file_medium_write,
    .erase = NULL,
    .deinit = file_medium_deinit,
};
/* =========================== function implementation session ============= */

static void* file_medium_init(void* arg)
{
    FILE* fd = NULL;

    if (arg == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.", __func__);
        return NULL;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.\n", __func__);
    char *path = (char *)arg;
    struct stat sta;

    if (access(path, F_OK) == 0 &&
        stat(path, &sta) == 0) {
        fd = fopen(path, "r+");
        if (fd == NULL) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "open file %s failed.", path);
        }
        return fd;
    }
    else {
        fd = fopen(path, "w+");
        if (fd == NULL) {
            SLIMFS_PRINT(SLIMFS_LOG_ERR, "open file %s failed.", path);
            return NULL;
        }
        // uint8_t buf[CONFIG_FLASH_SECTOR_SIZE];

        // for (int32_t i = 0; i < SLIMFS_FILE_MEDIUM_SECTOR_NUM; i++) {
        //     fwrite(buf, CONFIG_FLASH_SECTOR_SIZE, 1, fd);
        // }

        return fd;
    }

    return NULL;
}

static int32_t file_medium_read(void* hdl, uint8_t* rx_buf, uint64_t blk_addr, uint32_t blk_number)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.", __func__);
        return -1;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.\n", __func__);
    FILE* fd = (FILE *)hdl;

    fseek(fd, blk_addr * CONFIG_FLASH_SECTOR_SIZE, SEEK_SET);
    fread(rx_buf, CONFIG_FLASH_SECTOR_SIZE, blk_number, fd);

    return 0;
}

static int32_t file_medium_write(void* hdl, const uint8_t* tx_buf, uint64_t blk_addr, uint32_t blk_number)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.", __func__);
        return -1;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s, blk addr: %ld, blk_number: %d\n", __func__, blk_addr, blk_number);

    FILE* fd = (FILE *)hdl;
    fseek(fd, blk_addr * CONFIG_FLASH_SECTOR_SIZE, SEEK_SET);
    fwrite(tx_buf, CONFIG_FLASH_SECTOR_SIZE, blk_number, fd);

    return 0;
}


static void file_medium_deinit(void* hdl)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "parameter error for %s.", __func__);
        return;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s.\n", __func__);
    FILE* fd = (FILE *)hdl;

    fclose(fd);

    return;
}

slimfs_medium_ops_t* slimfs_get_medium_file_ops()
{
    return &s_slimfs_file_ops;
}
/* end of file slimfs_medium_file.c */