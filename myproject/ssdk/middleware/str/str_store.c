#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <board.h>
#include "str_store.h"

static struct disk_dev disk_dev_emmc0           = {0};
static struct disk_dev disk_dev_norflash0       = {0};

#define DEFAULT_BLOCK_SIZE                      512
#define PTB_SECTOR_INDEX                        2

#define EMMC_PARTITION_TABLE_OFFSET             0x0
#define NORFLASH_PARTITION_TABLE_OFFSET         0x2000

/* disk information */
static DISK_INFO_T disk_info_array[] = {
    [0] = {
        .disk_name      = DISK_MMC_NAME(0),
        .disk_inst      = &disk_dev_emmc0,
        .ptdev          = NULL,
        .ptb_offset     = EMMC_PARTITION_TABLE_OFFSET,
    },
    [1] = {
        .disk_name      = DISK_NOR_FLASH_NAME(0),
        .disk_inst      = &disk_dev_norflash0,
        .ptdev          = NULL,
        .ptb_offset     = NORFLASH_PARTITION_TABLE_OFFSET,
    }
};

/**
 * @brief close disk
 * @param download disk information
 * @return uint32_t 0 is success, 1 is failed
 */
static bool close_disk(DISK_INFO_T *disk)
{
    if (!disk) {
        ssdk_printf(SSDK_ALERT, "disk is null\n");
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
        ssdk_printf(SSDK_ALERT, "disk is null\n");
        return 1;
    }

    close_disk(disk);

    if (disk_open(disk->disk_name, disk->disk_inst)) {
        ssdk_printf(SSDK_ALERT, "open %s for disk->disk_inst[0] failed \n",
                    disk->disk_name);
        return 1;
    }

    return 0;
}

DISK_INFO_T *disk_dev_init(const char *disk_name)
{
    int i;
    uint32_t block_size;
    uint32_t erase_size;
    uint64_t capacity;
    DISK_INFO_T *disk = NULL;

    for (i = 0; i < sizeof(disk_info_array) / sizeof(disk_info_array[0]); i++) {
        if (!strcmp(disk_info_array[i].disk_name, disk_name)) {
            ssdk_printf(SSDK_ALERT, "disk = disk_info_array[%d]\n", i);
            disk = &disk_info_array[i];
            break;
        }
    }

    if (disk == NULL) {
        ssdk_printf(SSDK_ALERT, "can not find a matched dloader\n");
        return NULL;
    }

    if (open_disk(disk)) {
        ssdk_printf(SSDK_ALERT, "open disk failed \n");
        return NULL;
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

    ssdk_printf(SSDK_ALERT, "disk_name       = %s\n",     disk->disk_name);
    ssdk_printf(SSDK_ALERT, "ptb_offset      = %lld\n",   disk->ptb_offset);
    ssdk_printf(SSDK_ALERT, "block_size      = %d\n",     block_size);
    ssdk_printf(SSDK_ALERT,  "erase_size      = %d\n",     erase_size);
    ssdk_printf(SSDK_ALERT, "capacity        = %lld\n",   capacity);

    if (strstr(disk->disk_name, "flash"))  {
        disk->ptb_offset = erase_size * PTB_SECTOR_INDEX;
    }

    return disk;
}

uint64_t disk_ptdev_get_ptn_and_size(DISK_INFO_T *disk, const char *pt_name,
                                     uint64_t *ptn)
{
    uint64_t size;
    struct partition_device *ptdev;

    ptdev = ptdev_setup(disk->disk_inst, disk->ptb_offset);
    if (ptdev) {
        ssdk_printf(SSDK_ALERT, "read partition table\n");

        if (ptdev_read_table(ptdev)) {
            ssdk_printf(SSDK_ALERT, "ptdev_read_table error, disk_name = %s\n",
                        disk->disk_name);
            return -1;
        }
    }
    else {
        ssdk_printf(SSDK_ALERT, "get ptdev:%s error\n", disk->disk_name);
        return -1;
    }

    *ptn = ptdev_get_offset(ptdev, pt_name);
    size = ptdev_get_size(ptdev, pt_name);
    ssdk_printf(SSDK_ALERT, "ptn 0x%llx, size %lld\n", *ptn, size);
    if ((*ptn) % 512 || !(size) || !(*ptn)) {
        ssdk_printf(SSDK_ALERT, "failed to parse partition: %s\n", pt_name);
        return -1;
    }
    ptdev_destroy(ptdev);

    return size;
}

void disk_dev_deinit(DISK_INFO_T *disk)
{
    close_disk(disk);
}
