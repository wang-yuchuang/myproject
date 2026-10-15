/**
 * @file reboot_taishan.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <param.h>
#include <board.h>
#include <lib/partition/partition_parser.h>
#include <cmsis_os2.h>
#include <lib/disk/disk.h>
#include <arch/cache.h>
#include <regs_base.h>
#include <lib/cksum.h>
#include "rstgen_cfg.h"
#include "scr_hw.h"
#include "scr_cfg.h"
#include "reboot.h"

#define DL_DEBUG_ON 1
#if DL_DEBUG_ON
#define ERROR(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[ERROR]REBOOT:%s %d "format"", __func__, __LINE__,  ##args);
#define DBG(format, args...) ssdk_printf(SSDK_INFO, \
                               "[DEBUG]REBOOT:%s %d "format"", __func__, __LINE__,  ##args);
#else
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

static struct disk_dev disk_dev_norflash0     = {0};
static struct disk_dev disk_dev_emmc0         = {0};
static struct disk_dev disk_dev_sdcard0       = {0};


#define DEFAULT_BLOCK_SIZE 512
#define NORFLASH_PARTITION_TABLE_OFFSET 0x2000
#define PTB_SECTOR_INDEX 2

/* disk device */
typedef struct disk_information {
    const char               disk_name[MAX_GPT_NAME_SIZE + 1];
    struct disk_dev         *disk_inst;
    struct partition_device *ptdev;
    uint64_t                 ptb_offset;
} DISK_INFO_T;

/* disk information */
static DISK_INFO_T disk_info_array[] = {
    [0] = {
        .disk_name      = DISK_NOR_FLASH_NAME(0),
        .disk_inst      = &disk_dev_norflash0,
        .ptdev          = NULL,
        .ptb_offset     = NORFLASH_PARTITION_TABLE_OFFSET,
    },

    [1] = {
        .disk_name       = DISK_MMC_NAME(0),
        .disk_inst       = &disk_dev_emmc0,
        .ptdev           = NULL,
        .ptb_offset      = 0,
    },

    [2] = {
        .disk_name      = DISK_MMC_NAME(1),
        .disk_inst      = &disk_dev_sdcard0,
        .ptdev          = NULL,
        .ptb_offset     = 0,
    },
};

/**
 * @brief close disk
 * @param download disk information
 * @return uint32_t 0 is success, 1 is failed
 */
static bool close_disk(DISK_INFO_T *disk)
{
    if (!disk) {
        ERROR("disk is null\n");
        return 1;
    }

    if (disk_close(disk->disk_inst)) {
        return 1;
    }

    return 0;
}

/**
 * @brief open disk
 * @param disk disk information
 * @return bool 0 is success, 1 is failed
 */
static bool open_disk(DISK_INFO_T *disk)
{
    if (!disk) {
        ERROR("disk is null\n");
        return 1;
    }

    close_disk(disk);

    if (disk_open(disk->disk_name, disk->disk_inst)) {
        ERROR("open %s for disk->disk_inst[0] failed \n", disk->disk_name);
        return 1;
    }

    return 0;
}

/**
 * @brief load core image
 * @param reboot_array boot config
 * @return int
 */
static int load_core_image(struct reboot_cfg *reboot_array)
{
    int ret = -1;
    uint32_t addr = 0;
    uint64_t ptn = 0;
    uint64_t size = 0;
    uint32_t i = 0;
    uint32_t block_size;
    uint32_t erase_size;
    uint64_t capacity;
    DISK_INFO_T *disk = NULL;

    for (i = 0; i < sizeof(disk_info_array) / sizeof(disk_info_array[0]); i++) {
        if (!strcmp(disk_info_array[i].disk_name, reboot_array->disk_name)) {
            DBG("disk = disk_info_array[%d]\n", i);
            disk = &disk_info_array[i];
            break;
        }
    }

    if (disk == NULL) {
        ERROR("can not find a matched dloader\n");
        ret = -1;
        goto end;
    }

    if (open_disk(disk)) {
        ERROR("open disk failed \n");
        ret = -1;
        goto end;
    }

    disk_set_block_size(disk->disk_inst, DEFAULT_BLOCK_SIZE);
    block_size = disk_get_block_size(disk->disk_inst);
    erase_size = disk_erase_size(disk->disk_inst);
    capacity   = disk_size(disk->disk_inst);

    ASSERT(block_size != 0);
    ASSERT(erase_size != 0);
    ASSERT(block_size == DEFAULT_BLOCK_SIZE);
    ASSERT(erase_size % block_size == 0);
    ASSERT(capacity > block_size);
    ASSERT(capacity > erase_size);
    ASSERT(capacity > disk->ptb_offset);
    ASSERT(capacity > (GET_PRI_GPT_SIZE(block_size)));

    DBG("disk_name       = %s\n",     disk->disk_name);
    DBG("pt_name         = %s\n",     reboot_array->pt_name);
    DBG("ptb_offset      = %lld\n",   disk->ptb_offset);
    DBG("block_size      = %d\n",     block_size);
    DBG("erase_size      = %d\n",     erase_size);
    DBG("capacity        = %lld\n",   capacity);

    if (strstr(disk->disk_name, "flash"))  {
        disk->ptb_offset = erase_size * PTB_SECTOR_INDEX;
    }

    if (!disk->ptdev) {
        disk->ptdev = ptdev_setup(disk->disk_inst, disk->ptb_offset);

        if (disk->ptdev) {
            DBG("read partition table\n");

            if (ptdev_read_table(disk->ptdev)) {
                ERROR("ptdev_read_table error, disk_name = %s\n", disk->disk_name);
                ret = -1;
                goto end;
            }
        }
        else {
            ERROR("get ptdev:%s error\n", disk->disk_name);
            ret = -1;
            goto end;
        }
    }

    addr =   (uint32_t)reboot_array->target_addr;
    ptn = ptdev_get_offset(disk->ptdev, reboot_array->pt_name);
    size = ptdev_get_size(disk->ptdev, reboot_array->pt_name);
    DBG("target_addr 0x%08x, ptn 0x%llx, size %lld\n", addr, ptn, size);

    if (addr % 512 || ptn % 512 || !size || !ptn) {
        ERROR("para error\n");
        ret = -1;
        goto end;
    }

    if ((uint32_t)size > (reboot_array->target_len) * 1024) {
        size = (reboot_array->target_len) * 1024;
    }

    DBG("load partition cmd ptn 0x%llx, len %lld--->addr 0x%08x\n", ptn, size,
        addr);

    /* read from buffer */
    if (disk_read(disk->disk_inst, ptn, (uint8_t *)addr, size)) {
        ERROR("read storage failed\n");
        ret = -1;
        goto end;
    }

    arch_clean_cache_range(addr, (uint32_t)size);

#if 0
    uint8_t  md5_calc[MD5_LEN] = {0};
    md5((const unsigned char *)addr, (const int)size, (unsigned char *)md5_calc);
    hexdump(md5_calc, MD5_LEN);
#endif

    ret = 0;
end:
    ptdev_destroy(disk->ptdev);
    disk->ptdev = NULL;
    close_disk(disk);
    return ret;
}

/**
 * @brief reboot control
 * @param reboot_array boot config
 * @return void
 */
void reboot_ctrl(struct reboot_cfg *reboot_array)
{
    uint32_t ret;
    ASSERT(reboot_array);

    if (reboot_array->id >= MAX_CORE_ID) {
        ERROR("cpu id:%d error\n", reboot_array->id);
        return;
    }

    ret = load_core_image(reboot_array);

    if (ret) {
        ERROR("load_core_image for id:%d error\n", reboot_array->id);
        return;
    }

    if (reboot_array->scr_entry_signal.num) {
        for (int i = 0; i < reboot_array->scr_entry_signal.num; i++) {
            scr_set(reboot_array->scr_entry_signal.scr_dev, \
                    &reboot_array->scr_entry_signal.scr_sig[i], \
                    reboot_array->scr_entry_signal.scr_value[i]);
        }
    }

    if (reboot_array->rst_cpu) {
        reset_ctl_reset(reboot_array->rst_cpu);
    }

    return;
}