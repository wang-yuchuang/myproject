/**
 * @file ota_example.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ota example for E3 board
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
// #include <unistd.h>
#include <debug.h>
#include <CLI_type.h>
#include <arch/cache.h>
#include <cmsis_os2.h>
#include <lib/cksum.h>
#include <lib/partition/partition_parser.h>
#include <lib/partition/ab_partition_parser.h>
#include <lib/disk/disk.h>
#include <lib/sd_boot_img/sd_boot_img.h>
#include <reboot.h>
#include "ff.h"
#include "ota.h"

#if CONFIG_OTA_FROM_ETH_TFTP
#include "ota_eth.h"
#endif

#if CONFIG_OTA_FROM_UDISK
#include  "ota_usbh.h"
#endif

#define LBA_SIZE 512
#define MAX_CALLOC_SIZE (32*1024)
#define USE_RW
#define ROM_CTRL_BOOTLOAER_BIT          0
#define ROM_CTRL_DDR_BIT                24
#define ROM_CTRL_FLASH_LINE_NUMBER_BIT  25
#define ROM_CTRL_FLASH_CMD              27
#define ROM_CTRL_FLASH_CTRL_VALID       29

#define XSPI1_MEM_BASE 0x10000000
#define ETH_DISK_NAME "eth0"
#define UPDATE_ONE_PACKAGE 1

static osEventFlagsId_t new_event;
static osThreadId_t cl_process_thread;
static struct disk_dev disk_dev_norflash0     = {0};
static struct disk_dev disk_dev_emmc0         = {0};
static struct disk_dev disk_dev_emmc0_boot1   = {0};
static struct disk_dev disk_dev_emmc0_boot2   = {0};
static struct disk_dev disk_dev_sdcard0       = {0};
static struct disk_dev disk_dev_sdcard1       = {0};
struct sfs sfs_temp = {0};
static uint8_t tempbuf[LOGIC_BLOCK_SIZE] __ALIGNED(512) = {0};
static FATFS fatfs;
static char argv_temp[OTA_ARGV_NUM][OTA_ARGV_LEN];
int argc_temp = 0;
static char file_path_and_name[MAX_FILE_NAME_LIMIT] = {0};
static char file_path[MAX_FILE_NAME_LIMIT] = {0};
static char *file_name = NULL;
extern int partition_update_test(DISK_STATE_T *ds);
uint8_t md5_write[MD5_LEN];

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

void usage()
{
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d [device] -f [device:filename] -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d flash0 -f sd1:xxx -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d flash0 -f eth0:xxx -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d flash0 -f usb0:xxx -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d emmc0 -f sd1:xxx -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d emmc0 -f eth0:xxx -b");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d emmc0 -f usb0:xxx -b");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d [device] -f [device:filename] -p");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d flash0 -p");
    ssdk_printf(SSDK_CRIT,"    example: \n\t ota -d emmc0 -p");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -bl, enter bootloader mode");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -app, enter app mode");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -i boot, get the boot information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i sfs, get the sfs information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i bpt0, get the bpt0 information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i bpt1, get the bpt1 information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i bpt2, get the bpt2 information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i partition, get the partition information");
    ssdk_printf(SSDK_CRIT,"Usage: \n\tota -d flash0 -i sort , sort the bpt to know which boot package to update");
}

void mark_bootloader_flag()
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val |= (0x1 << ROM_CTRL_BOOTLOAER_BIT);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

void clear_bootloader_flag()
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val &= ~(0x1 << ROM_CTRL_BOOTLOAER_BIT);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

static char *boot_package_name[3] = {
    BOOT_PARTITION_NAME0,
    BOOT_PARTITION_NAME1,
    BOOT_PARTITION_NAME2,
};

/* disk information */
static DISK_STATE_T dl_state[] = {
    [0] = {
        .download_name  = OSPI1_STORAGE_NAME,
        .disk_name      = DISK_NOR_FLASH_NAME(0),
        .disk_inst[0]   = &disk_dev_norflash0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .disk_num       = BOOT_DEVICE_NOR_FLASH,
        .ptdev          = NULL,
        .ptb_offset     = NORFLASH_GPT_OFFSET,
        .bpt_info[0]    = {0},
        .bpt_info[1]    = {0},
        .bpt_info[2]    = {0},
    },

    [1] = {
        .download_name   = EMMC1_STORAGE_NAME,
        .disk_name       = DISK_MMC_NAME(0),
        .disk_boot1_name = DISK_MMC_BOOT_NAME(0, 1),
        .disk_boot2_name = DISK_MMC_BOOT_NAME(0, 2),
        .disk_inst[0]    = &disk_dev_emmc0,
        .disk_inst[1]    = &disk_dev_emmc0_boot1,
        .disk_inst[2]    = &disk_dev_emmc0_boot2,
        .disk_num        = BOOT_DEVICE_EMMC_OR_SD1,
        .ptdev           = NULL,
        .ptb_offset      = 0,
        .bpt_info[0]    = {0},
        .bpt_info[1]    = {0},
        .bpt_info[2]    = {0},
    },

    [2] = {
        .download_name  = SD1_STORAGE_NAME,
        .disk_name      = DISK_MMC_NAME(0),
        .disk_inst[0]   = &disk_dev_sdcard0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .disk_num       = BOOT_DEVICE_EMMC_OR_SD1,
        .ptdev          = NULL,
        .ptb_offset     = 0,
        .bpt_info[0]    = {0},
        .bpt_info[1]    = {0},
        .bpt_info[2]    = {0},
    },

    [3] = {
        .download_name  = SD2_STORAGE_NAME,
        .disk_name      = DISK_MMC_NAME(1),
        .disk_inst[0]   = &disk_dev_sdcard1,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .disk_num       = BOOT_DEVICE_EMMC_OR_SD2,
        .ptdev          = NULL,
        .ptb_offset     = 0,
        .bpt_info[0]    = {0},
        .bpt_info[1]    = {0},
        .bpt_info[2]    = {0},
    },
};

uint8_t sort_bpt_for_disk(DISK_STATE_T *ds, uint8_t *max_bpt_num,
                          uint8_t *min_bpt_num);

/**
 * @brief align the "size" to "aligned" boundry;
 * @param size iput number
 * @param aligned  aligned number
 * @return uint64_t lowest common multiple
 */
static uint64_t round_up(uint64_t size, uint64_t aligned)
{
    uint64_t mod = 0;

    if (aligned == 0 || size < aligned)
        return aligned;

    /* Sometimes, 'aligned' is not equal to power of 2 */
    mod = size % aligned;

    size += mod ? aligned - mod : 0;
    return size;
}

/**
 * @brief Get the boot disk number
 * @return int boot disk Number
 */
int get_boot_device()
{
    uint32_t boot_info_addr = BOOT_INFO_ADDR;

    if (*(uint32_t *)boot_info_addr != BOOT_INFO_TAG) {
        ERROR("boot info tag error\n");
        return -1;
    }

    return *(uint8_t *)(boot_info_addr + BOOT_DEVICE_INFO_OFFSET);
}

/**
 * @brief Get the boot package number
 * @return int 0 for Normal package, 1 for Backup package, 2 for Third package
 */
int get_boot_package()
{
    uint32_t boot_info_addr = BOOT_INFO_ADDR;

    if (*(uint32_t *)boot_info_addr != BOOT_INFO_TAG) {
        ERROR("boot info tag error\n");
        return -1;
    }

    return *(uint8_t *)(boot_info_addr + BOOT_PACKAGE_INFO_OFFSET);
}

/**
 * @brief Get the inst num of disk
 * @param ds download disk states
 * @return uint8_t disk inst number
 */
static uint8_t get_inst_num(DISK_STATE_T *ds, uint8_t bpt_num)
{
    uint8_t inst_num = 0;

    if (strstr(ds->disk_name, "flash")) {
        return 0;
    }

    if (0 == bpt_num)
        inst_num = 1;

    else if (1 == bpt_num)
        inst_num = 2;

    DBG("get disk_inst= %d\n", inst_num);
    return inst_num;
}

/**
 * @brief read sfs and get the image base
 * @param ds download disk states
 * @return int sfs's image base
 */
static uint32_t sfs_get_image_base(DISK_STATE_T *ds, uint8_t num)
{
    uint8_t *buffer = NULL;
    uint32_t sfs_size = 0;
    uint32_t img_base = 0;

    if (!strstr(ds->disk_name, "flash")) {
        ERROR("ds->disk_type != NORFLASH\n");
        return 0;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, LOGIC_BLOCK_SIZE);
    buffer = tempbuf;

    if (!buffer) {
        ERROR("allocate memory fail\n");
        return 0;
    }

    memset(buffer, 0x0, sfs_size);

    if (disk_read_block(ds->disk_inst[0], 0, buffer, sfs_size/ds->block_size)) {
        ERROR("read nor flash fail\n");
        goto end;
    }

    if (get_sfs_info(&sfs_temp, buffer, LOGIC_BLOCK_SIZE)) {
        ERROR("sfs error\n");
        goto end;
    }

    if (0 == num) {
        DBG("get sfs normal_img_base = 0x%08x\n", sfs_temp.normal_img_base);
        img_base = sfs_temp.normal_img_base;
    }
    else if (1 == num) {
        DBG("get sfs backup_img_base = 0x%08x\n", sfs_temp.backup_img_base);
        img_base = sfs_temp.backup_img_base;
    }
    else if (2 == num) {
        DBG("get sfs third_img_base = 0x%08x\n", sfs_temp.third_img_base);
        img_base = sfs_temp.third_img_base;
    }
    else {
        ERROR("num = %d, must < 3\n", num);
        goto end;
    }

end:
    return img_base;
}

/**
 * @brief read sfs and get the image limit size
 * @param ds download disk states
 * @return int sfs's limited image size
 */
static uint32_t sfs_get_image_size_limit(DISK_STATE_T *ds, uint8_t num)
{
    uint8_t *buffer = NULL;
    uint32_t sfs_size = 0;
    uint32_t img_size = 0;
    uint32_t img_base[3] = {0};
    uint32_t max_img_base = 0;
    uint32_t min_img_base = 0;

    if (!strstr(ds->disk_name, "flash")) {
        ERROR("ds->disk_type != NORFLASH\n");
        return 0;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, LOGIC_BLOCK_SIZE);
    buffer = tempbuf;

    if (!buffer) {
        ERROR("allocate memory fail\n");
        return 0;
    }

    memset(buffer, 0x0, sfs_size);

    if (disk_read_block(ds->disk_inst[0], 0, buffer, sfs_size/ds->block_size)) {
        ERROR("read nor flash fail\n");
        goto end;
    }

    if (get_sfs_info(&sfs_temp, buffer, LOGIC_BLOCK_SIZE)) {
        ERROR("sfs error\n");
        goto end;
    }

    img_base[0] = sfs_temp.normal_img_base;
    img_base[1] = sfs_temp.backup_img_base;
    img_base[2] = sfs_temp.third_img_base;

    if (sfs_temp.normal_img_base == sfs_temp.backup_img_base) {
        img_base[1] = 0;
    }

    if (sfs_temp.normal_img_base == sfs_temp.third_img_base) {
        img_base[2] = 0;
    }

    if (sfs_temp.backup_img_base == sfs_temp.third_img_base) {
        img_base[2] = 0;
    }

    min_img_base = MIN(img_base[0], MIN(img_base[1], img_base[2]));
    max_img_base = MAX(img_base[0], MAX(img_base[1], img_base[2]));

    if (0 == img_base[num])
        img_size = 0;
    else if (img_base[num] == min_img_base) {
        img_size = MIN(img_base[(num + 1) % 3],
                       img_base[(num + 2) % 3]) - img_base[num];
    }
    else if (img_base[num] == max_img_base)
        img_size = ds->capacity - img_base[num];
    else
        img_size = MAX(img_base[(num + 1) % 3],
                       img_base[(num + 2) % 3]) - img_base[num];

end:
    DBG("get image limited size  = 0x%08x\n", img_size);
    return img_size;
}

/**
 * @brief read sfs of nor flash
 * @param ds download disk states
 * @return void
 */
static void read_sfs(DISK_STATE_T *ds)
{
    uint8_t *buffer = NULL;
    uint32_t sfs_size = 0;

    if (!strstr(ds->disk_name, "flash")) {
        return;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, LOGIC_BLOCK_SIZE);
    buffer = tempbuf;

    if (!buffer) {
        ERROR("allocate memory fail\n");
        return;
    }

    memset(buffer, 0x0, sfs_size);

    if (disk_read_block(ds->disk_inst[0], 0, buffer, sfs_size/ds->block_size)) {
        ERROR("read nor flash fail\n");
        return;
    }

    DBG("sfs dump\n");
    hexdump(buffer, ds->block_size);

    if (get_sfs_info(&sfs_temp, buffer, LOGIC_BLOCK_SIZE)) {
        ERROR("sfs error\n");
        return;
    }

    DBG("sfs tag = 0x%08x\n", sfs_temp.tag);
    DBG("sfs freq = 0x%02x\n", sfs_temp.freq);
    DBG("sfs sw_reset_info = 0x%02x\n", sfs_temp.sw_reset_info);
    DBG("sfs normal_img_base = 0x%08x\n", sfs_temp.normal_img_base);
    DBG("sfs backup_img_base = 0x%08x\n", sfs_temp.backup_img_base);
    DBG("sfs third_img_base = 0x%08x\n", sfs_temp.third_img_base);
    DBG("sfs crc32 = 0x%08x\n", sfs_temp.crc32);
}

/**
 * @brief read bpt of disk
 *
 * @param ds download disk states
 * @param bpt_num bpt number of the disk
 * @return uint8_t 0 is read correct bpt
 */
static uint8_t read_bpt(DISK_STATE_T *ds, uint8_t bpt_num)
{
    uint8_t ret = 0;
    struct bpt *bpt = NULL;
    struct iib *iib = NULL;
    uint8_t *buffer = NULL;
    uint64_t addr = 0;
    uint64_t length = 0;
    uint8_t inst_num = 0;
    uint32_t crc32_val = 0;
    uint32_t i = 0;

    if (strstr(ds->disk_name, "flash")) {
        addr = sfs_get_image_base(ds, bpt_num);
        inst_num = 0;
    }
    else if (0 == bpt_num) {
        addr = 0;
        inst_num = 1;
    }
    else if (1 == bpt_num) {
        addr = 0;
        inst_num = 2;
    }
    else if (2 == bpt_num) {
        addr = SD_BOOT_OFFSET;
        inst_num = 0;
    }

    length = BPT_SIZE;
    buffer = (uint8_t *)osAllocAlign(ds->block_size, length);

    if (!buffer) {
        ERROR("buffer calloc failed\n");
        ret = 1;
        goto end;
    }

    memset((uint8_t *)buffer, 0, length);

    DBG("read inst %d addr 0x%llx, len %lld for BPT\n", inst_num, addr, length);

    if (disk_read_block(ds->disk_inst[inst_num], addr/ds->block_size, (uint8_t *)buffer, length/ds->block_size)) {
        ERROR("read storage failed\n");
        ret = 1;
        goto end;
    }

    bpt = (struct bpt *)osAllocAlign(ds->block_size, sizeof(struct bpt));

    if (!bpt) {
        ERROR("bpt calloc failed\n");
        ret = 1;
        goto end;
    }

    memset((uint8_t *)bpt, 0, sizeof(struct bpt));

    if (0 != get_bpt_info(bpt, (uint8_t *)buffer, length)) {
        ERROR("get bpt info failed\n");
        ret = 1;
        goto end;
    }
    else {
        crc32_val = sfs_crc32(0, (uint8_t *)buffer, BPT_CRC32_OFFSET);
        DBG("=================  BPT %d  ================= \n", bpt_num);
        DBG("bpt->tag                  = 0x%08x\n", bpt->tag);
        DBG("bpt->size                 = 0x%x\n",   bpt->size);
        DBG("bpt->sec_version          = 0x%08x\n", bpt->sec_version);
        DBG("bpt->hash_alg             = 0x%x\n", bpt->hash_alg);
        DBG("bpt->crc32                = 0x%08x(crc32_val=0x%08x)\n", bpt->crc32,
            crc32_val);
        DBG("bpt->pac_serial_num       = 0x%08x\n", bpt->pac_serial_num);
        DBG("bpt->inver_pac_serial_num = 0x%08x\n", bpt->inver_pac_serial_num);

        for (i = 0; i < 8; i++) {
            iib = &bpt->iib[i];

            if ((iib->tag != IIB_TAG)) {
                DBG("iib[%d] is none\n", i);
                break;
            }

            DBG("--------\n");
            DBG("iib[%d]->tag            = 0x%x\n", i, iib->tag);
            DBG("iib[%d]->size           = 0x%x\n", i, iib->size);
            DBG("iib[%d]->image_type     = 0x%x\n", i, iib->image_type);
            DBG("iib[%d]->target_core    = 0x%x\n", i, iib->target_core);
            DBG("iib[%d]->decryp_ctl     = 0x%x\n", i, iib->decryp_ctl);
            DBG("iib[%d]->dev_logic_page = 0x%08x\n", i, iib->dev_logic_page);
            DBG("iib[%d]->image_size     = 0x%08x\n", i, iib->image_size);
            DBG("iib[%d]->load_base      = 0x%08x\n", i, iib->load_base);
            DBG("iib[%d]->entry_point    = 0x%08x\n", i, iib->entry_point);
        }

        ret = 0;
    }

end:

    if (buffer)
        osFree(buffer);

    if (bpt)
        osFree(bpt);

    return ret;
}

/**
 * @brief close disk
 * @param download disk states
 * @return uint32_t 0 is success, 1 is failed
 */
static bool close_disk(DISK_STATE_T *ds)
{
    if (!ds) {
        ERROR("ds is null\n");
        return 1;
    }

    if (disk_close(ds->disk_inst[0])) {
        return 1;
    }

    if (ds->disk_inst[1] && disk_close(ds->disk_inst[1])) {
        return 1;
    }

    if (ds->disk_inst[2] && disk_close(ds->disk_inst[2])) {
        return 1;
    }

    return 0;
}

/**
 * @brief open disk
 * @param ds download disk states
 * @return bool 0 is success, 1 is failed
 */
static bool open_disk(DISK_STATE_T *ds)
{
    if (!ds) {
        ERROR("ds is null\n");
        return 1;
    }

    close_disk(ds);

    if (disk_open(ds->disk_name, ds->disk_inst[0])) {
        ERROR("open %s for ds->disk_inst[0] failed \n", ds->disk_name);
        return 1;
    }

    if (ds->disk_inst[1] && disk_open(ds->disk_boot1_name, ds->disk_inst[1])) {
        ERROR("open %s for ds->disk_inst[1] failed \n", ds->disk_boot1_name);
        return 1;
    }

    if (ds->disk_inst[2] && disk_open(ds->disk_boot2_name, ds->disk_inst[2])) {
        ERROR("open %s for ds->disk_inst[2] failed \n", ds->disk_boot2_name);
        return 1;
    }

    return 0;
}

/**
 * @brief parse the partition information
 * @param full_ptname
 * @return DISK_STATE_T* download disk states
 */
static DISK_STATE_T *parse_disk_name(const char *download_name)
{
    DISK_STATE_T *ds = NULL;
    bool once = 0;
    DBG("-------------------Parsing name------------------------\n");

    if (!download_name || strlen(download_name) == 0) {
        goto fail;
    }

    DBG("download_name:%s\n", download_name);

    for (uint32_t i = 0; i < sizeof(dl_state) / sizeof(dl_state[0]); i++) {
        if (!strcmp(dl_state[i].download_name, download_name)) {
            DBG("ds = dl_state[%d]\n", i);
            ds = &dl_state[i];
            break;
        }
    }

    if (ds == NULL) {
        ERROR("can not find a matched dloader\n");
        goto fail;
    }

    if (open_disk(ds)) {
        ERROR("open disk failed \n");
        goto fail;
    }

    disk_set_block_size(ds->disk_inst[0], LOGIC_BLOCK_SIZE);
    ds->block_size = disk_get_block_size(ds->disk_inst[0]);
    ds->erase_size = disk_erase_size(ds->disk_inst[0]);
    ds->capacity   = disk_size(ds->disk_inst[0]);

    ASSERT(ds->block_size != 0);
    ASSERT(ds->erase_size != 0);
    ASSERT(ds->block_size == LOGIC_BLOCK_SIZE);
    ASSERT(ds->erase_size % ds->block_size == 0);
    ASSERT(ds->capacity > ds->block_size);
    ASSERT(ds->capacity > ds->erase_size);
    ASSERT(ds->capacity > ds->ptb_offset);
    ASSERT(ds->capacity > (GET_PRI_GPT_SIZE(ds->block_size)));

    if (strstr(ds->disk_name, "flash"))  {
        ds->ptb_offset = ds->erase_size * PTB_SECTOR_INDEX;
    }

    if ((!ds->ptdev) && (once == 0)) {
        once = 1;
        ds->ptdev = ptdev_setup(ds->disk_inst[0], ds->ptb_offset);

        if (ds->ptdev) {
            DBG("read partition table\n");

            if (ptdev_read_table(ds->ptdev)) {
                DBG("ptdev_read_table error, disk_name = %s\n", download_name);
                ptdev_destroy(ds->ptdev);
            }
        }
        else {
            ERROR("get ptdev:%s error\n", download_name);
            goto fail;
        }
    }

    DBG("download_name   = %s\n",     ds->download_name);
    DBG("disk_name       = %s\n",     ds->disk_name);
    DBG("disk_boot1_name = %s\n",     ds->disk_boot1_name);
    DBG("disk_boot2_name = %s\n",     ds->disk_boot2_name);
    DBG("ptb_offset      = %lld\n",   ds->ptb_offset);
    DBG("block_size      = %d\n",     ds->block_size);
    DBG("erase_size      = %d\n",     ds->erase_size);
    DBG("capacity        = %lld\n",   ds->capacity);
    return ds;

fail:
    close_disk(ds);
    return NULL;
}

/**
 * @brief dump the boot information
 * @param argv not used
 */
static void boot_info_dump(char *argv)
{
    int boot_dev = -1;
    int boot_package = -1;

    boot_dev = get_boot_device();
    boot_package = get_boot_package();

    switch (boot_dev) {
        case BOOT_DEVICE_EMMC_OR_SD1:
            DBG("boot from emmc1/sd1\n");
            break;

        case BOOT_DEVICE_EMMC_OR_SD2:
            DBG("boot from emmc2/sd2\n");
            break;

        case BOOT_DEVICE_NOR_FLASH:
            DBG("boot from Nor/Hyper Flash\n");
            break;

        case BOOT_DEVICE_USB:
            DBG("boot from USB\n");
            break;

        case BOOT_DEVICE_UART:
            DBG("boot from UART\n");
            break;

        default:
            DBG("CAN't find boot device, dump boot info addr\n");
            hexdump((uint8_t *)BOOT_INFO_ADDR, 0x20);
            break;
    }

    switch (boot_package) {
        case BOOT_NORMAL_PACKAGE:
            DBG("boot from Normal package\n");
            break;

        case BOOT_BACKUP_PACKAGE:
            DBG("boot from Backup package\n");
            break;

        case BOOT_THIRD_PACKAGE:
            DBG("boot from Third package\n");
            break;

        default:
            DBG("CAN't find boot package, dump boot info addr\n");
            hexdump((uint8_t *)BOOT_INFO_ADDR, 0x20);
            break;
    }
}

/**
 * @brief dump disk's information by arg value
 * @param ds  download disk states
 * @param argv input arg value
 * @return int 0 is dump success
 */
static int disk_dump(DISK_STATE_T *ds, char *argv)
{
    int ret = 0;
    uint8_t max_bpt_num;
    uint8_t min_bpt_num;

    if (!strcmp(argv, "boot")) {
        boot_info_dump(argv);
    }

    else {
        if (!ds) {
            ERROR("disk inst is null\n");
            ret = -1;
            goto end;
        }

        if ((!strcmp(argv, "sfs")) && (strstr(ds->disk_name, "flash"))) {
            read_sfs(ds);
        }

        if (!strcmp(argv, "bpt0")) {
            ret = read_bpt(ds, 0);
        }

        if (!strcmp(argv, "bpt1")) {
            ret = read_bpt(ds, 1);
        }

        if (!strcmp(argv, "bpt2")) {
            ret = read_bpt(ds, 2);
        }

        if (!strcmp(argv, "partition")) {
            ptdev_dump(ds->ptdev);
            ptdev_attr_dump(ds->ptdev);
        }

        if (!strcmp(argv, "sort")) {
            sort_bpt_for_disk(ds, &max_bpt_num, &min_bpt_num);
            ERROR("bpt base sort error\n");
            DBG("max_bpt_num = %d, min_bpt_num(target bpt) = %d\n", max_bpt_num,
                min_bpt_num);
        }
    }

end:
    return ret;
}

/**
 * @brief parse disk's bpt information
 * @param ds download disk states
 * @param bpt_num bpt number
 * @return uint8_t 0 is empty space or got a correct bpt
 */
static uint8_t parse_bpt_for_disk(DISK_STATE_T *ds, uint8_t bpt_num)
{
    uint8_t ret = 1;
    struct bpt *bpt = NULL;
    struct iib *iib = NULL;
    uint8_t *buffer = NULL;
    uint64_t addr = 0;
    uint64_t size = 0;
    uint64_t length = 0;
    uint8_t inst_num = 0;
    uint32_t crc32_val = 0;
    uint32_t i = 0;
    uint8_t iib_cnt = 0;

    ds->bpt_info[bpt_num].valid = 0;
    ds->bpt_info[bpt_num].xip = 0;
    ds->bpt_info[bpt_num].sec_version = 0;
    ds->bpt_info[bpt_num].pac_serial_num = 0;
    ds->bpt_info[bpt_num].inver_pac_serial_num = 0;

    if (strstr(ds->disk_name, "flash")) {
        addr = ptdev_get_offset(ds->ptdev, boot_package_name[bpt_num]);

        if (addr != 0) {
            size = ptdev_get_size(ds->ptdev, boot_package_name[bpt_num]);
        }
        else {
            addr = sfs_get_image_base(ds, bpt_num);

            if (addr != 0)
                size = sfs_get_image_size_limit(ds, bpt_num);
            else {
                ERROR("boot%d image base error, addr = %lld!\n", bpt_num, addr);
                goto no_space;
            }
        }

        inst_num = 0;
    }
    else if (0 == bpt_num) {
        addr = 0;
        size = EMMC_BOOT_PT_SIZE;
        inst_num = 1;
    }
    else if (1 == bpt_num) {
        addr = 0;
        size = EMMC_BOOT_PT_SIZE;
        inst_num = 2;
    }
    else if (2 == bpt_num) {
        addr = ptdev_get_offset(ds->ptdev, boot_package_name[2]);

        if (addr == SD_BOOT_OFFSET) {
            size = ptdev_get_size(ds->ptdev, boot_package_name[2]);
        }
        else {
            addr = SD_BOOT_OFFSET;
            size = ds->capacity - SD_BOOT_OFFSET;
        }

        inst_num = 0;
    }

    if (!size) {
        ERROR("error,boot partition size is 0\n");
        goto no_space;
    }

    length = BPT_SIZE;
    buffer = (uint8_t *)osAllocAlign(ds->block_size, length);

    if (!buffer) {
        ERROR("buffer calloc failed\n");
        goto no_space;
    }

    memset((uint8_t *)buffer, 0, length);

    DBG("read inst %d addr 0x%llx, len %lld for BPT\n", inst_num, addr, length);

    if (disk_read_block(ds->disk_inst[inst_num], addr/ds->block_size, (uint8_t *)buffer, length/ds->block_size)) {
        ERROR("read storage failed\n");
        goto no_space;
    }

    bpt = (struct bpt *)osAllocAlign(ds->block_size, sizeof(struct bpt));

    if (!bpt) {
        ERROR("bpt calloc failed\n");
        goto no_space;
    }

    memset((uint8_t *)bpt, 0, sizeof(struct bpt));

    if (0 != get_bpt_info(bpt, (uint8_t *)buffer, length)) {
        ERROR("get bpt info failed\n");
        goto no_bpt;
    }
    else {
        if ((bpt->pac_serial_num + bpt->inver_pac_serial_num != 0xFFFFFFFF)) {
            if ((bpt->pac_serial_num != 0) || (bpt->inver_pac_serial_num != 0)) {
                ERROR("PSN error,pac_serial_num=0x%08x, inver_pac_serial_num=0x%08x\n",
                      bpt->pac_serial_num, bpt->inver_pac_serial_num);
                goto no_bpt;
            }
        }

        crc32_val = sfs_crc32(0, (uint8_t *)buffer, BPT_CRC32_OFFSET);

        if (crc32_val != bpt->crc32) {
            ERROR("CRC error, bpt->crc32=0x%08x, crc32_val=0x%08x\n", bpt->crc32,
                  crc32_val);
            goto no_bpt;
        }

        for (i = 0; i < 8; i++) {
            iib = &bpt->iib[i];

            if ((iib->tag != IIB_TAG)) {
                continue;
            }
/*
            ds->bpt_info[bpt_num].iib[i].image_type = iib->image_type;
            ds->bpt_info[bpt_num].iib[i].target_core = iib->target_core;
            ds->bpt_info[bpt_num].iib[i].dev_logic_page = iib->dev_logic_page;
            ds->bpt_info[bpt_num].iib[i].image_size = iib->image_size;
            ds->bpt_info[bpt_num].iib[i].load_base = iib->load_base;
            ds->bpt_info[bpt_num].iib[i].entry_point = iib->entry_point;

            if (ds->bpt_info[bpt_num].iib[i].load_base > XSPI1_MEM_BASE) {
                ds->bpt_info[bpt_num].xip = 1;
            }
*/
        }

        iib_cnt = i;

        if (iib_cnt == 0) {
            ERROR("iib count = %d\n", iib_cnt);
            goto no_bpt;
        }
    }

    ds->bpt_info[bpt_num].sec_version          = bpt->sec_version;
    ds->bpt_info[bpt_num].pac_serial_num       = bpt->pac_serial_num;
    ds->bpt_info[bpt_num].inver_pac_serial_num = bpt->inver_pac_serial_num;

no_bpt:
    ds->bpt_info[bpt_num].image_num            = iib_cnt;
    ds->bpt_info[bpt_num].desk_inst            = inst_num;
    ds->bpt_info[bpt_num].bpt_addr             = addr;
    ds->bpt_info[bpt_num].max_size             = size;
    ds->bpt_info[bpt_num].valid = 1;
    ret = 0;

no_space:

    if (buffer)
        osFree(buffer);

    if (bpt)
        osFree(bpt);

    return ret;
}

/**
 * @brief get the bpt with max PSN and the bpt with minimum PSN
 * @param ds download disk states
 * @param max_bpt_num bpt number with max PSN
 * @param min_bpt_num bpt number with minimum PSN
 * @return uint8_t 0 is success
 */
uint8_t sort_bpt_for_disk(DISK_STATE_T *ds, uint8_t *max_bpt_num,
                          uint8_t *min_bpt_num)
{
    int boot_dev = -1;
    int boot_package = -1;
    uint8_t i = 0;
    int64_t min_version = 0xFFFFFFFF;
    int64_t max_version = -1;

    if (!ds) {
        ERROR("disk inst is null\n");
        return 1;
    }

    *min_bpt_num = INVALID_BPT_NUMBER;
    *max_bpt_num = INVALID_BPT_NUMBER;
    boot_dev = get_boot_device();
    boot_package = get_boot_package();

    if ((-1 == boot_dev) || (-1 == boot_package) || (boot_package >= MAX_BPT_NUM)) {
        ERROR("boot info error\n");
        return 1;
    }

    for (i = 0; i < MAX_BPT_NUM; i++) {
        if (parse_bpt_for_disk(ds, i)) {
            DBG("there is no space for bpt %d\n", i);
            continue;
        }

        if (!ds->bpt_info[i].valid) {
            DBG("ds->bpt_info[%d] is passed\n", i);
            continue;
        }

        if ((int64_t)ds->bpt_info[i].pac_serial_num > max_version) {
            *max_bpt_num = i;
            max_version = ds->bpt_info[i].pac_serial_num;
        }

        if (((int64_t)ds->bpt_info[i].pac_serial_num <= min_version)
                && ((boot_package != i) || (boot_dev != ds->disk_num))) {
            *min_bpt_num = i;
            min_version = ds->bpt_info[i].pac_serial_num;
        }
    }

    if ((*min_bpt_num == INVALID_BPT_NUMBER) || (*min_bpt_num >= MAX_BPT_NUM)) {
        ERROR("can't min_bpt_num\n");
        return 1;
    }

    if ((*max_bpt_num == INVALID_BPT_NUMBER) || (*max_bpt_num >= MAX_BPT_NUM)) {
        ERROR("can't max_bpt_num\n");
        return 1;
    }

    if (max_version < min_version) {
        ERROR("max_version < min_version\n");
        return 1;
    }

#if UPDATE_ONE_PACKAGE

    if ((boot_dev == ds->disk_num) && (*max_bpt_num != boot_package)) {
        DBG("warning:boot disk's max_bpt_num is not the now boot package!\n");
        /* ota target is changed to max_bpt_num */
        *min_bpt_num = *max_bpt_num;
        DBG("warning:min_bpt_num(ota target) is changed to %d\n", *min_bpt_num);
        /* the package now running is the biggest */
        *max_bpt_num = boot_package;

        DBG("warning:max_bpt_num(ota's PSN for check) is changed to now running boot package = %d\n",
            *max_bpt_num);
        max_version = ds->bpt_info[*max_bpt_num].pac_serial_num;
        min_version = ds->bpt_info[*min_bpt_num].pac_serial_num;
    }

#endif

    DBG("\n========the boot package with biggest PSN information============\n")
    DBG("ds->bpt_info[%d] max verion = 0x%llx,addr = 0x%llx,max size = 0x%llx\n", \
        *max_bpt_num, max_version, ds->bpt_info[*max_bpt_num].bpt_addr,
        ds->bpt_info[*max_bpt_num].max_size);
    DBG("\n========the boot package to be OTA updated information============\n")
    DBG("ds->bpt_info[%d] min verion = 0x%llx,addr = 0x%llx,max size = 0x%llx\n", \
        *min_bpt_num, min_version, ds->bpt_info[*min_bpt_num].bpt_addr,
        ds->bpt_info[*min_bpt_num].max_size);

    return 0;
}

/**
 * @brief Get the file name from input buffer
 * @param argv agr value
 * @return uint8_t 0 is success
 */
uint8_t get_file_name(char *argv)
{
    uint8_t ret = 0;

    if ((strlen(argv) > MAX_FILE_NAME_LIMIT) || (strlen(argv) < 6)) {
        ERROR("bpt name %s length\n", argv);
        return 1;
    }

    file_name = strstr(argv, ":");

    DBG("argv = %s, file_name = %s\n", argv, file_name);

    if (NULL == file_name) {
        ERROR("file_name error\n");
        return 1;
    }

    argv[file_name - argv] = 0;
    file_name += 1;

    DBG("argv = %s, file_name = %s\n", argv, file_name);

    if (!strcmp(argv, SD1_STORAGE_NAME) || strstr(argv, DISK_MMC_NAME(0))) {
        sprintf(file_path, "%s:", DISK_MMC_NAME(0));
        sprintf(file_path_and_name, "%s:%s", DISK_MMC_NAME(0), file_name);
    }

    else if (!strcmp(argv, SD2_STORAGE_NAME) || strstr(argv, DISK_MMC_NAME(1))) {
        sprintf(file_path, "%s:", DISK_MMC_NAME(1));
        sprintf(file_path_and_name, "%s:%s", DISK_MMC_NAME(1), file_name);
    }

    else if (strstr(argv, DISK_NOR_FLASH_NAME(0))) {
        sprintf(file_path, "%s:", DISK_NOR_FLASH_NAME(0));
        sprintf(file_path_and_name, "%s:%s", DISK_NOR_FLASH_NAME(0), file_name);
    }

    else if (strstr(argv, DISK_NOR_FLASH_NAME(1))) {
        sprintf(file_path, "%s:", DISK_NOR_FLASH_NAME(1));
        sprintf(file_path_and_name, "%s:%s", DISK_NOR_FLASH_NAME(1), file_name);
    }

    else if (strstr(argv, DISK_USB_NAME(0))) {
#if CONFIG_OTA_FROM_UDISK
        sprintf(file_path, "%s:", DISK_USB_NAME(0));
        sprintf(file_path_and_name, "%s:%s", DISK_USB_NAME(0), file_name);

        /* wait recv event happen */
        if (0 == osEventFlagsWait(udisk_ready, 1, osFlagsWaitAny | osFlagsNoClear,
                                  1000)) {
            ret = 1;
        }

#else
        ERROR("do not support ota from Udisk , need to config\n")
#endif
    }

    else if (strstr(argv, ETH_DISK_NAME)) {
#if CONFIG_OTA_FROM_ETH_TFTP
        sprintf(file_path, "%s:", ETH_DISK_NAME);
        sprintf(file_path_and_name, "%s:%s", ETH_DISK_NAME, file_name);
#else
        ERROR("do not support ota from eth0 vid tftp , need to config\n")
#endif
    }

    else
        ret = 1;

    DBG("file_name = %s file_path = %s, file_path_and_name = %s\n", file_name,
        file_path, file_path_and_name);
    return ret;
}

/**
 * @brief read back data from disk and do md5 check
 * @param ds download disk states
 * @param inst disk inst
 * @param read_addr read back address
 * @param size size of data
 * @return DL_ERR_CODE_E error code
 */
uint8_t md5_read_check(DISK_STATE_T *ds, uint8_t inst,
                       uint64_t read_addr, uint32_t size)
{
    uint8_t  md5_calc[MD5_LEN] = {0};
    struct MD5Context context;
    uint32_t i = 0;
    uint32_t cnt = 0;
    uint32_t left = 0;
    uint64_t length = 0;
    uint32_t data_size = 4096;
    uint8_t ret = 1;
    uint8_t *buf = NULL;

    MD5Init(&context);
    cnt = size / data_size;
    left = size % data_size;

    buf = osAllocAlign(512, data_size);
    ASSERT(data_size > ds->block_size);
    ASSERT(data_size % ds->block_size == 0);

    if (!buf) {
        ERROR("buf calloc failed\n");
        goto end;
    }

    for (i = 0; i <= cnt; i++) {
        if (i == cnt) {
            length = left;
        }
        else {
            length = data_size;
        }

        if (0 == length)
            break;

        if (disk_read_block(ds->disk_inst[inst], read_addr / ds->block_size,
                            (uint8_t *)buf, data_size / ds->block_size)) {
            ERROR("read back error!\n");
            goto end;
        }

        MD5Update(&context, (uint8_t *)buf, length);
        read_addr += length;
    }

    MD5Final(md5_calc, &context);

    if (memcmp(md5_write, md5_calc, MD5_LEN)) {
        ERROR("md5 check fail!\n");
        hexdump(md5_write, MD5_LEN);
        hexdump(md5_calc, MD5_LEN);
        goto end;
    }

    hexdump(md5_calc, MD5_LEN);
    ret = 0;
    DBG("read back md5 check success!\n");

end:

    if (buf)
        osFree(buf);

    return ret;
}

uint8_t check_bpt_for_update(DISK_STATE_T *ds, struct bpt *bpt,
                             uint8_t max_bpt_num, uint8_t min_bpt_num)
{
    uint32_t xip_addr_base;
    uint64_t addr = INVALID_VERSION;
    uint32_t i = 0;

    if (!ds) {
        ERROR("disk inst is null\n");
        return 1;
    }

    if (!bpt) {
        ERROR("bpt is null\n");
        return 1;
    }

    if (bpt->pac_serial_num < ds->bpt_info[max_bpt_num].pac_serial_num) {
        ERROR("bpt's PSN from file = 0x%08x less than ds->bpt_info[%d].pac_serial_num = 0x%08x\n",
              bpt->pac_serial_num, max_bpt_num, ds->bpt_info[max_bpt_num].pac_serial_num);
        return 1;
    }

    for (i = 0; i < 8; i++) {
        if (bpt->iib[i].tag != IIB_TAG) {
            if (0 == i) {
                ERROR("bpt->iib[0]'s tag error\n");
                return 1;
            }

            break;
        }

        if ((bpt->iib[i].entry_point > XSPI1_MEM_BASE)
                || (bpt->iib[i].load_base > XSPI1_MEM_BASE)) {

            xip_addr_base = XSPI1_MEM_BASE + ds->bpt_info[min_bpt_num].bpt_addr +
                            bpt->iib[i].dev_logic_page * 512;

            if (bpt->iib[i].entry_point != xip_addr_base) {
                ERROR("file %s bpt->iib[%d]'s entry_point is 0x%x(XIP), expected entry_point is 0x%x \n",
                      file_name, i, bpt->iib[i].entry_point, xip_addr_base);
                return 1;
            }

            if (bpt->iib[i].load_base != xip_addr_base) {
                ERROR("file %s bpt->iib[%d]'s load_base is 0x%x(XIP), expected load_base is 0x%x \n",
                      file_name, i, bpt->iib[i].load_base, xip_addr_base);
                return 1;
            }
        }
    }

    addr = ds->bpt_info[min_bpt_num].bpt_addr;

    if (strstr(ds->disk_name, "flash") && (addr % ds->erase_size)) {
        ERROR("disk type = flash, bpt addr = 0x%llx must be aligned to sector size 0x%x\n",
              addr, ds->erase_size);
        return 1;
    }
    else if (addr % ds->block_size) {
        ERROR("bpt addr = 0x%llx must be aligned to block size 0x%x\n", addr,
              ds->block_size);
        return 1;
    }

    return 0;
}

/**
 * @brief update bpt from fat32 file
 * @param ds download disk states
 */
void bpt_update_test(DISK_STATE_T *ds)
{
    FRESULT rc;
    FIL fil;
    UINT br;
    struct bpt *bpt = NULL;
    uint32_t offset = 0;
    uint8_t *buf = NULL;
    FILINFO fno;
    uint32_t crc32_val = 0;
    uint64_t addr = INVALID_VERSION;
    uint32_t data_size = BPT_SIZE;
    uint32_t i = 0;
    uint32_t total_sec = 0;
    uint32_t left_sec = 0;
    uint8_t min_bpt_num = INVALID_BPT_NUMBER;
    uint8_t max_bpt_num = INVALID_BPT_NUMBER;
    int boot_dev = -1;
    int boot_package = -1;
    uint8_t inst_num = 0;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint64_t length = 0;
    struct MD5Context context;

    if (!ds) {
        ERROR("disk inst is null\n");
        goto end;
    }

    sector_size = ds->erase_size;
    block_size = ds->block_size;
    ASSERT(sector_size);
    ASSERT(block_size);
    ASSERT(data_size % block_size == 0);
    boot_dev = get_boot_device();
    boot_package = get_boot_package();

    if ((-1 == boot_dev) || (-1 == boot_package) || (boot_package >= MAX_BPT_NUM)) {
        ERROR("boot info error\n");
        goto end;
    }

    rc = f_mount(&fatfs, file_path, 1);

    if (rc) {
        ERROR("f_mount returned %d\n", rc);
        goto end;
    }

    DBG("f_mount success\r\n");

    /* Get file status */
    rc = f_stat(file_path_and_name, &fno);

    if (rc) {
        ERROR("f_stat %s returned %d\n", file_path_and_name, rc);
        goto unmount;
    }

    DBG("fno.fsize = %lld", fno.fsize);

    rc = f_open(&fil, file_path_and_name, FA_READ);

    if (rc) {
        ERROR("f_open %s returned %d\n", file_path_and_name, rc);
        goto unmount;
    }

    rc = f_lseek(&fil, offset);

    if (rc) {
        ERROR("f_lseek returned %d\n", rc);
        goto unmount;
    }

    buf = osAllocAlign(512, data_size);

    if (!buf) {
        ERROR("buf calloc failed\n");
        goto unmount;
    }

    bpt = (struct bpt *)osAllocAlign(512, sizeof(struct bpt));

    if (!bpt) {
        ERROR("bpt calloc failed\n");
        goto unmount;
    }

    memset((uint8_t *)buf, 0, data_size);
    memset((uint8_t *)bpt, 0, sizeof(struct bpt));

    rc = f_read(&fil, (void *)buf, BPT_SIZE, &br);

    if (rc) {
        ERROR("f_read returned %d\n", rc);
        goto unmount;
    }

    if (0 != get_bpt_info(bpt, (uint8_t *)buf, BPT_SIZE)) {
        ERROR("get bpt info from file failed\n");
        goto unmount;
    }

    crc32_val = sfs_crc32(0, (uint8_t *)buf, BPT_CRC32_OFFSET);

    DBG("update file bpt->tag                  = 0x%08x\n", bpt->tag);
    DBG("update file bpt->size                 = 0x%x\n",   bpt->size);
    DBG("update file bpt->sec_version          = 0x%08x\n", bpt->sec_version);
    DBG("update file bpt->hash_alg             = 0x%x\n",   bpt->hash_alg);
    DBG("update file bpt->crc32                = 0x%08x(crc32_val=0x%08x)\n",
        bpt->crc32, crc32_val);
    DBG("update file bpt->pac_serial_num       = 0x%08x\n", bpt->pac_serial_num);
    DBG("update file bpt->inver_pac_serial_num = 0x%08x\n",
        bpt->inver_pac_serial_num);

    if (crc32_val != bpt->crc32) {
        ERROR("CRC error, bpt->crc32=0x%08x, crc32_val=0x%08x\n", bpt->crc32,
              crc32_val);
        goto unmount;
    }

    if (sort_bpt_for_disk(ds, &max_bpt_num, &min_bpt_num)) {
        ERROR("bpt base sort error\n");
        goto unmount;
    }

    DBG("max_bpt_num = %d, min_bpt_num = %d\n", max_bpt_num, min_bpt_num);

    if (check_bpt_for_update(ds, bpt, max_bpt_num, min_bpt_num)) {
        ERROR("ota check error\n");
        goto unmount;
    }

    if (fno.fsize > ds->bpt_info[min_bpt_num].max_size) {
        DBG("file size = %lld is bigger than ds->bpt_info[%d].max_size = %lld\n",
            fno.fsize, min_bpt_num, ds->bpt_info[min_bpt_num].max_size);
        goto unmount;
    }

    if (fno.fsize < BPT_SIZE) {
        ERROR("length_total error\n");
        goto unmount;
    }

    addr = ds->bpt_info[min_bpt_num].bpt_addr;
    DBG("addr 0x%llx, len %lld\n", addr, fno.fsize);

    total_sec = (uint32_t)(fno.fsize / data_size);
    left_sec = (uint32_t)(fno.fsize % data_size);

    DBG("total_sec = %d, left_sec = %d \n", total_sec, left_sec);

    rc = f_lseek(&fil, offset);

    if (rc) {
        ERROR("f_lseek returned %d\r\n", rc);
        goto unmount;
    }

    inst_num = get_inst_num(ds, min_bpt_num);

    if (strstr(ds->disk_name, "flash")) {
        DBG("waiting erase addr 0x%llx, len 0x%llx......\n", addr, fno.fsize);

        if (disk_erase_group(ds->disk_inst[inst_num], addr / sector_size,
                             round_up(fno.fsize, sector_size) / sector_size, DISK_ERASE_DEFAULT)) {
            ERROR("disk_erase_group addr = 0x%llx error\n", addr);
            goto unmount;
        }
    }

    memset(md5_write, 0, MD5_LEN);
    MD5Init(&context);

    for (i = 0; i <= total_sec; i++) {
        if (i == total_sec) {
            length = left_sec;
            memset(buf, 0, data_size);
        }
        else {
            length = data_size;
        }

        if (0 == (uint32_t)length)
            break;

        DBG("flash data from addr = %d, length = %d \n", (uint32_t)addr,
            (uint32_t)length);

        rc = f_read(&fil, (void *)buf, length, &br);

        if (rc) {
            ERROR("f_read returned %d\r\n", rc);
            goto unmount;
        }

        if (disk_write_block(ds->disk_inst[inst_num], addr / block_size, (uint8_t *)buf,
                             data_size / block_size)) {
            ERROR("write data error!\n");
            goto unmount;
        }

        MD5Update(&context, (uint8_t *)buf, length);
        addr += length;
    }

    MD5Final(md5_write, &context);

    rc = f_close(&fil);

    if (rc) {
        ERROR("f_open returned %d\r\n", rc);
    }

    md5_read_check(ds, inst_num, ds->bpt_info[min_bpt_num].bpt_addr, fno.fsize);

unmount:
    f_unmount(file_path);

end:

    if (bpt)
        osFree(bpt);

    if (buf)
        osFree(buf);

    return;
}
#if CONFIG_OTA_FROM_ETH_TFTP

/**
 * @brief update bpt from ethernet
 * @param ds download disk states
 */
void bpt_update_test_eth(DISK_STATE_T *ds)
{
    uint8_t min_bpt_num = INVALID_BPT_NUMBER;
    uint8_t max_bpt_num = INVALID_BPT_NUMBER;
    int boot_dev = -1;
    int boot_package = -1;
    uint8_t inst_num = 0;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    void *handle = NULL;
    sector_size = ds->erase_size;
    block_size = ds->block_size;
    ASSERT(sector_size);
    ASSERT(block_size);
    ASSERT(BPT_SIZE % block_size == 0);
    boot_dev = get_boot_device();
    boot_package = get_boot_package();

    if ((-1 == boot_dev) || (-1 == boot_package) || (boot_package >= MAX_BPT_NUM)) {
        ERROR("boot info error\n");
        return;
    }

    if (sort_bpt_for_disk(ds, &max_bpt_num, &min_bpt_num)) {
        ERROR("bpt base sort error\n");
        return;
    }

    DBG("max_bpt_num = %d, min_bpt_num = %d\n", max_bpt_num, min_bpt_num);

    inst_num = get_inst_num(ds, min_bpt_num);

    DBG("remote file_name is %s\n", file_name);

    handle = create_tftp_handle(ds, file_name, max_bpt_num, min_bpt_num, inst_num);

    if (handle == NULL) {
        ERROR("failed to create file\n");
        return;
    }

    if (ota_tftp_init_client(handle)) {
        ERROR("ota over tftp failed\n");
        return;
    }

    return;

}
#endif

/**
 * @brief update the parition attributes
 * @param ptdev parition device
 * @return unsigned 0 is success
 */
unsigned ptdev_attr_update(partition_device_t *ptdev)
{
    int ret = -1;

    if (ptdev->attributes_updated) {
        ret = ptdev_attributes_update(ptdev);
        ptdev->attributes_updated = false;
    }

    return ret;
}

/**
 * @brief Get the number of parition slots
 * @param ptdev parition device
 * @return unsigned 2 is A/B parition, 1 is not
 */
unsigned get_number_slots(partition_device_t *ptdev)
{
    if (ptdev_multislot_is_supported(ptdev))
        return AB_SUPPORTED_SLOTS;

    return 1;
}

/**
 * @brief Get the current slot
 * @param ptdev parition device
 * @return unsigned 0 is for slot A , 1 is for slot B
 */
unsigned get_current_slot(partition_device_t *ptdev)
{
    signed active_slot = ptdev_find_active_slot(ptdev);
    return active_slot;
}

/**
 * @brief Mark the active slot as Successful
 * @param ptdev parition device
 * @return int 0 is success
 */
int mark_boot_successful(partition_device_t *ptdev)
{
    signed active_slot = ptdev_find_active_slot(ptdev);
    ptdev_mark_slot_attr_noupdate(ptdev, active_slot, ATTR_SUCCESSFUL);
    return 0;
}

/**
 * @brief Set a boot as active and boot slot
 * @param ptdev parition device
 * @param slot the target slot
 * @return int 0 is success
 */
int set_active_boot_slot(partition_device_t *ptdev, unsigned slot)
{
    ptdev_clean_slot_attr_noupdate(ptdev, slot, ATTR_SUCCESSFUL);
    ptdev_mark_slot_attr_noupdate(ptdev, slot, ATTR_ACTIVE);
    ptdev_mark_slot_attr_noupdate(ptdev, slot, ATTR_RETRY);
    ptdev_clean_slot_attr_noupdate(ptdev, slot, ATTR_UNBOOTABLE);

    if (get_current_slot(ptdev) == slot)
        return 0;

    return -1;
}

/**
 * @brief check if a slot is marked bootable
 * @param ptdev partition device
 * @param slot slot number
 * @return int 1 is bootable, 0 is not bootable
 */
int is_slot_bootable(partition_device_t *ptdev, unsigned slot)
{
    int ret = ptdev_find_boot_slot(ptdev);

    if (ret == AB_SUPPORTED_SLOTS)
        return 1;
    else
        return (ret == slot) ? 1 : 0;
}

/**
 * @brief get the suffix of a slot
 * @param ptdev partition device
 * @param slot slot number
 * @return const char* "_a" for slot A, "_b" for slot B
 */
const char *get_suffix(partition_device_t *ptdev, unsigned slot)
{
    if (slot < AB_SUPPORTED_SLOTS)
        return suffix_slot[slot];

    return NULL;
}

/**
 * @brief check if a slot is marked successful
 * @param ptdev partition device
 * @param slot slot number
 * @return int 1 is successful, 0 is not successful
 */
int is_slot_marked_successful(partition_device_t *ptdev, unsigned slot)
{
    int ret = ptdev_find_successfull_slot(ptdev);

    if (ret == AB_SUPPORTED_SLOTS)
        return 1;
    else
        return (ret == slot) ? 1 : 0;
}

/**
 * @brief slot A/B programming
 * @param ds download disk states
 * @return int 0 is success
 */
int slot_programming(DISK_STATE_T *ds)
{
    int slot_active[64] = {0};
    int slot_inactive[64] = {0};
    int active_slot = 0;
    int inactive_slot = 0;
    int len = 0;
    int ret = -1;
    unsigned long long gpt_offset = ds->ptb_offset;
    partition_device_t *ptdev = ds->ptdev;
    unsigned long long in = 0;
    unsigned long long out = 0;
    unsigned long long size = 0;
    unsigned long long left = 0;
    unsigned long long cnt = 0;
    char *pname = NULL;
    const char *suffix = NULL;
    int index = 0;
    int file_index = -1;
    unsigned char *data = NULL;
    unsigned partition_count = ptdev->count;
    struct partition_entry *partition_entries = ptdev->partition_entries;
    FRESULT rc;
    FIL fil;
    UINT br;
    uint32_t offset = 0;
    FILINFO fno;
    char *temp = NULL;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;

    if (!ds) {
        ERROR("disk inst is null\n");
        goto end;
    }

    sector_size = ds->erase_size;
    block_size = ds->block_size;
    ASSERT(sector_size);
    ASSERT(block_size);
    ASSERT(MAX_CALLOC_SIZE % block_size == 0);

    if (!strlen(file_name) || !strlen(file_path_and_name) || !strlen(file_path)) {
        DBG("copy test between partition A/B\n");
    }
    else {
        rc = f_mount(&fatfs, file_path, 1);

        if (rc) {
            ERROR("f_mount returned %d\n", rc);
            goto end;
        }

        DBG("f_mount success\r\n");

        /* Get file status */
        rc = f_stat (file_path_and_name, &fno);

        if (rc) {
            ERROR("f_stat %s returned %d\n", file_path_and_name, rc);
            goto unmount;
        }

        DBG("fno.fsize = %lld\n", fno.fsize);

        rc = f_open(&fil, file_path_and_name, FA_READ);

        if (rc) {
            ERROR("f_open %s returned %d\n\n", file_path_and_name, rc);
            goto unmount;
        }

        rc = f_lseek(&fil, offset);

        if (rc) {
            ERROR("f_lseek returned %d\n", rc);
            goto unmount;
        }

        temp = strstr(file_name, ".");

        if (temp) {
            file_name[temp - file_name] = 0;
        }

        DBG("find partition name = %s\n", file_name);
    }

    active_slot = get_current_slot(ptdev);
    suffix = get_suffix(ptdev, active_slot);

    for (int i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        len = strlen(pname);

        if (len < 4)
            continue; /* too few, ignore */

        if (strcmp((char *)(pname + len - 2), suffix_slot[0])
                && strcmp((char *)(pname + len - 2), suffix_slot[1])) {
            continue;
        }

        for (int j = i + 1; j < partition_count; j++) {
            if ((!strncmp(pname, (char *)partition_entries[j].name, len - 2)) &&
                    (len == strlen((char *)partition_entries[j].name))) {
                if (strstr(pname, suffix)) {
                    slot_active[index] = i;
                    slot_inactive[index] = j;
                }
                else {
                    slot_active[index] = j;
                    slot_inactive[index] = i;
                }

                if ((!strncmp(file_name, (char *)partition_entries[i].name, len - 2)) &&
                        (strlen(file_name) == (len - 2))) {
                    file_index = index;
                    DBG("find file_index = %d\n", file_index);
                }

                index++;
            }
        }
    }

    data = (unsigned char *)osAllocAlign(512, MAX_CALLOC_SIZE);

    if (!data) {
        ERROR("calloc failed\n");
        goto unmount;
    }

    for (int i = 0; i < index; i++) {
        active_slot = slot_active[i];
        inactive_slot = slot_inactive[i];
        in = (partition_entries[active_slot].first_lba) * LBA_SIZE;
        out = (partition_entries[inactive_slot].first_lba) * LBA_SIZE;
        in += gpt_offset;
        out += gpt_offset;
        size = (partition_entries[inactive_slot].last_lba -
                partition_entries[inactive_slot].first_lba + 1) * LBA_SIZE;

        DBG("i=%d\n", i);
        DBG("active_slot=%s\n", partition_entries[active_slot].name);
        DBG("inactive_slot=%s\n", partition_entries[inactive_slot].name);

        if (strstr(ds->disk_name, "flash")) {
            if (out % sector_size || in % sector_size || size % sector_size) {
                ERROR("partition addr must be aligned to sector_size 0x%x\n", sector_size);
                goto unmount;
            }

            DBG("erase inactive slot addr 0x%llx, len 0x%llx......\n", out, size);

            if (disk_erase_group(ds->disk_inst[0], out / sector_size, size / sector_size,
                                 DISK_ERASE_DEFAULT)) {
                ERROR("disk_erase_group addr = 0x%llx error\n", out);
                goto unmount;
            }
        }

        if (i != file_index) {
            DBG("in=0x%08x out=0x%08x size=%lld\n", (uint32_t)in, (uint32_t)out, size);
        }
        else {
            if (fno.fsize > size) {
                ERROR("file size = %lld > parition size = %lld\n", fno.fsize, size);
                goto unmount;
            }
            else {
                size = fno.fsize;
            }

            DBG("in=%s out=0x%08x size=%lld\n", file_path_and_name, (uint32_t)out, size);
        }

        cnt =  (size / MAX_CALLOC_SIZE);
        left = (size % MAX_CALLOC_SIZE);

        for (unsigned long long n = 0; n < cnt; n++) {
            if (i != file_index) {
                ret = disk_read_block(ds->disk_inst[0], in / block_size, data,
                                      MAX_CALLOC_SIZE / block_size);

                if (ret < 0) {
                    ERROR("read failed\n");
                    goto unmount;
                }
            }
            else {
                rc = f_read(&fil, (void *)data, MAX_CALLOC_SIZE, &br);

                if (rc) {
                    ERROR("f_read returned %d\r\n", rc);
                    goto unmount;
                }
            }

            ret = disk_write_block(ds->disk_inst[0], out / block_size, data,
                                   MAX_CALLOC_SIZE / block_size);

            if (ret < 0) {
                ERROR("write failed\n");
                goto unmount;
            }

            in  += MAX_CALLOC_SIZE;
            out += MAX_CALLOC_SIZE;
        }

        if (0 != left) {
            memset(data, 0, MAX_CALLOC_SIZE);

            if (i != file_index) {
                ret = disk_read_block(ds->disk_inst[0], in / block_size, data,
                                      left / block_size);

                if (ret < 0) {
                    ERROR("read failed\n");
                    goto unmount;
                }
            }
            else {
                rc = f_read(&fil, (void *)data, left, &br);

                if (rc) {
                    ERROR("f_read returned %d\r\n", rc);
                    goto unmount;
                }
            }

            ret = disk_write_block(ds->disk_inst[0], out / block_size, data,
                                   MAX_CALLOC_SIZE / block_size);

            if (ret < 0) {
                ERROR("write failed\n");
                goto unmount;
            }
        }
    }

    ret = 0;

unmount:
    f_unmount(file_path);

end:

    if (data)
        osFree(data);

    return ret;
}

/**
 * @brief GPT's A/B slot partition updating test
 * @param download disk states
 * @return int
 */
int partition_update_test(DISK_STATE_T *ds)
{
    int ret;
    int slot;
    int inactive_slot;
    const char *suffix;

    if (!ds) {
        DBG("disk state is null\n");
        return -1;
    }

    partition_device_t *ptdev = ds->ptdev;

    /* dump current part */
    DBG("first bootup dump ptdev.\n");
    ptdev_dump(ptdev);
    ptdev_attr_dump(ptdev);

    /* All slots are zeroed, mark SLOT 0, active & bootable */
    ret = get_number_slots(ptdev);
    DBG("slots count %d\n", ret);
    slot = get_current_slot(ptdev);
    DBG("corrent slot %d\n", slot);

    if (slot == INVALID) {
        DBG("bad slot %d\n", slot);
        return -1;
    }

    suffix = get_suffix(ptdev, slot);
    DBG("slot suffix %s\n", suffix);

    inactive_slot = get_inverse_slot(ptdev, slot);

    /* mark bootup successfully if necessary */
    ret = is_slot_marked_successful(ptdev, slot);

    if (ret != 1) {
        mark_boot_successful(ptdev);
    }

    ptdev_attr_update(ptdev);
    ptdev_attr_dump(ptdev);

    /* A/B partition updating */
    DBG("A/B partition updating ... \n");
    ret = slot_programming(ds);

    if (ret < 0) {
        ERROR("slot program error\n");
        return -1;
    }

    /* active & bootable */
    set_active_boot_slot(ptdev, inactive_slot);
    ptdev_attr_update(ptdev);
    ptdev_attr_dump(ptdev);

    DBG("OTA finish, try reboot to take effect \n");

    return 0;
}

/**
 * @brief ota example test thread
 * @param arg not used
 */
static void ota_thread(void *arg)
{
    DISK_STATE_T *ds = NULL;
    int i = 0;

    while (true) {
        /* wait recv event happen */
        if (0 == osEventFlagsWait(new_event, 1, osFlagsWaitAny, osWaitForever)) {
            break;
        }

        ds = NULL;
        i = 0;
        memset(file_path_and_name, 0, MAX_FILE_NAME_LIMIT);
        memset(file_path, 0, MAX_FILE_NAME_LIMIT);
        file_name = NULL;
        DBG("dloader thread active\n");

        while (i < argc_temp) {
            DBG("i = %d argv_temp = %s\n", i, argv_temp[i]);

            if (!strcmp(argv_temp[i], "-d")) {
                ds = parse_disk_name(argv_temp[i + 1]);

                if (!ds) {
                    ERROR("disk inst is null\n");
                    break;
                }

                i += 2;
            }
            else if (!strcmp(argv_temp[i], "-f")) {
                get_file_name(argv_temp[i + 1]);
                i += 2;
            }
            else if (!strcmp(argv_temp[i], "-p")) {
                partition_update_test(ds);
                i++;
            }
            else if (!strcmp(argv_temp[i], "-b")) {
#if CONFIG_OTA_FROM_ETH_TFTP

                if (strstr(file_path, ETH_DISK_NAME)) {
                    DBG("OTA via eth");
                    bpt_update_test_eth(ds);
                }
                else {
                    DBG("OTA via local storage");
                    bpt_update_test(ds);
                }

#else
                DBG("OTA via local storage");
                bpt_update_test(ds);
#endif
                i++;
            }
            else if (!strcmp(argv_temp[i], "-i")) {
                disk_dump(ds, argv_temp[i + 1]);
                i += 2;
            }
            else if (!strcmp(argv_temp[i], "-bl")) {
                mark_bootloader_flag();
                reboot_global_ctrl();
                i++;
            }
            else if (!strcmp(argv_temp[i], "-app")) {
                clear_bootloader_flag();
                reboot_global_ctrl();
                i++;
            }
            else {
                usage();
                i++;
                break;
            }
        }
    }
}

/**
 * @brief ota example
 * @param argc
 * @param argv
 * @return int
 */
static int ota_update_test(int argc, char *argv[])
{
    uint32_t i = 0;

    if (!cl_process_thread) {
        osThreadAttr_t ota_attr = {0};

        ota_attr.name       = (const char *)"ota_update_test";
        ota_attr.stack_size = OTA_STACK_SIZE;
        ota_attr.priority   = osPriorityNormal;

        cl_process_thread = osThreadNew(ota_thread, NULL, &ota_attr);
        new_event = osEventFlagsNew(NULL);

        if (!cl_process_thread) {
            ERROR("failed to dloader_test process thread\n");
            return CLI_FALSE;
        }

#if CONFIG_OTA_FROM_UDISK

        if (DEF_OK != OTA_USBH_Init()) {
            ERROR("failed to init usb host\n");
            return CLI_FALSE;
        }

#endif

    }

    if (argc > OTA_ARGV_NUM) {
        ERROR("too much arg\n");
    }

    argc_temp = argc;

    for (i = 0; i < argc; i++) {
        if (strlen(argv[i]) >= OTA_ARGV_LEN) {
            ERROR("argv %d length too long\n", i);
            return CLI_FALSE;
        }

        memset(argv_temp[i], 0, OTA_ARGV_LEN);
        memcpy(argv_temp[i], argv[i], strlen(argv[i]));
    }

    osEventFlagsSet(new_event, 1);
    DBG("event set!\n");

    return CLI_FALSE;
}

CLI_CMD("ota", "ota\r\n ota_update_test\n ", ota_update_test);
