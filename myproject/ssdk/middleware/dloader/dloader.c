/**
 * @file dloader.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: device downloader program
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <param.h>
#include <board.h>
#include <CLI_type.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <lib/cksum.h>
#include <lib/list.h>
#include <lib/disk/disk.h>
#include <lib/partition/partition_parser.h>
#include <lib/sd_boot_img/sd_boot_img.h>
#include <driver/taishan/fuse/fuse.h>
#include <arch/cache.h>
#include <errno.h>
#include "dloader.h"

#ifdef CONFIG_DLOADER_WITH_USB
#include "dloader_usb_fastboot.h"
static fastboot_t *fb_data;
#endif

#ifndef MAX_RSP_SIZE
#define MAX_RSP_SIZE            64
#endif

#define DLOADER_VERSION                    "V00.00.01"
#define SFS_PT_NAME                        "sfs"
#define RFD_PT_NAME                        "rfd"
#define PARTITION_TABLE_NAME               "partition"
#define ALL_DISK_NAME                      "all"
#define ADD_DIR_FUZZY_NAME                 "0x"

#define OSPI1_STORAGE_NAME                 "ospi1"
#define OSPI2_STORAGE_NAME                 "ospi2"
#define EMMC1_STORAGE_NAME                 "emmc1"
#define EMMC2_STORAGE_NAME                 "emmc2"
#define SD1_STORAGE_NAME                   "sd3"
#define SD2_STORAGE_NAME                   "sd4"

#define SD_SPL_OFFSET                      (0x5000)
#define RFD_OFFSET                         (0x100-16)
#define RFD_SIZE                           (272)
#define SUB_PARTITION                      (1)
#define PRI_PARTITION                      (0)

#define PTB_NEED_FLASH                     (0)
#define PTB_NO_NEED_FLASH                  (1)
#define PTB_CHECK_ERROR                    (2)
#define INVALID_PTB_OFFSET                 (~0llu)
#define DISABLE_GPT_PTB_CHK                (1)
#define NOR_FLASH_PTB_SECTOR_INDEX         (2)
#define PTACH_SFS_WHEN_SFS_DOWNLOADING     (0)
#define PTACH_SFS_CRC_WHEN_SFS_DOWNLOADING (1)
#define DL_ALIGN_SIZE                      (1024)
#define TAISHAN_IRAM_MIN_SIZE              (0x100000)
#define TAISHAN_DLOADER_STANDARD_SIZE      (0x40000)
#define SFS_RFD_RFU_SIZE                   (0x200)
#define IS_ALIGNED(a, b) (!(((uintptr_t)(a)) & (((uintptr_t)(b))-1)))
#define HYPER_FLASH_MODE_FUSE_INDEX        (181)
#define HYPER_FLASH_MODE_FUSE_VAL          (0x00100000)
#define UINT32_HEX_STR_LEN                 (sizeof(uint32_t)*2)
#define FUSE_INDEX_MAX                     (255)

/* globol var */
void *dl_scratch_base = NULL;
uint32_t dl_scratch_sz = 0;
void *dl_data_base = NULL;
uint32_t dl_data_sz = 0;
uint8_t md5_received[MD5_LEN];

#if CONFIG_DLOADER_WITH_TRACE

#define ELF_STATUS_MAGIC (0x6f707273) //"oprs"
#define TRACE_PROCESS_STACK_SIZE         (16384)

typedef struct prog_cmd_args_t {
    uint64_t cur_sz;
    uint64_t offset;
    uint64_t total_sz;
    char storage_name[8];
    char name[72];
    uint64_t dl_status;
    uint32_t elf_status;
    uint32_t data_base;
    uint32_t data_size;
} __PACKED PROG_CMD_ARGS_T;

typedef struct prog_statu_t {
    uint32_t flag;
    uint32_t ret_code;
} __PACKED PROG_STATUS_T;

PROG_STATUS_T *prog_status = NULL;
PROG_CMD_ARGS_T *cmd_args  = NULL;

static osThreadId_t trace_process_thread = NULL;

#endif


#ifdef CONFIG_DLOADER_WITH_USB
#include "rstgen_cfg.h"
#include <reboot.h>
#define ROM_CTRL_BOOT_PIN_OVERRIDE_BIT_OFFSET 0x1
#define ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET 0x9
extern USBD_DEV_CFG  FB_USBD_DevCfg;
#endif

char *boot_package[BOOT_MAX] = {
    BOOT_PARTITION_NAME0,
    BOOT_PARTITION_NAME1,
    BOOT_PARTITION_NAME2,
};

/* static globol var */
static bool do_md5_rb_check = false;
static bool do_md5_rcv_check = true;

static const char *err_info[] = {
    [ERR_UNKNOWN] = "unkown error",
    [ERR_PRI_PTB_NOT_MATCH] = "primary partition table not match",
    [ERR_SUB_PTB_NOT_MATCH] = "sub partition table not match",
    [ERR_PT_NOT_FOUND] = "partition not found",
    [ERR_IMAGE_TOO_LARGE] = "image too large",
    [ERR_IMAGE_FORMAT_ERR] = "image format error",
    [ERR_PRI_PTB_NOT_FLASH] = "primary partition table has not been flashed",
    [ERR_SUB_PTB_NOT_FLASH] = "sub partition table has not been flashed",
    [ERR_PT_READ_FAIL] = "partition read error",
    [ERR_PT_FLASH_FAIL] = "partition flash error",
    [ERR_PT_ERASE_FAIL] = "erase partition error",
    [ERR_PT_OVERLAP] = "partition size not aligned",
    [ERR_PT_FULL_NAME_FORMAT] = "partition full name format error",
    [ERR_INVALID_BLOCK_SIZE] = "invalid block size",
    [ERR_SPARSE_IMAGE_SIZE_TOO_LOW] = "sparse image size too low",
    [ERR_SPARSE_IMAGE_HEADER] = "sparse image header error",
    [ERR_SPARSE_IMAGE_BUFFERED] = "buffered spare image error",
    [ERR_SPARSE_IMAGE_CHUNK_HEADER] = "sparse image chunk header error",
    [ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE] = "sparse image chunk too large",
    [ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH] = "sparse image chunk size not match type",
    [ERR_SPARSE_IMAGE_CHUNK_UNKNOWN] = "sparse image chunk unkown",
    [ERR_SPARSE_IMAGE_MALLOC] = "sparse image malloc error",
    [ERR_SPARSE_IMAGE_DO_NOT_SUPPORT] = "do not support sparse image download",
    [ERR_HASH_FAIL] = "hash check fail",
    [ERR_EFUSE_INDEX] = "efuse index error",
    [ERR_EFUSE_BURN] = "burn efuse fail",
    [ERR_EFUSE_READ] = "read efuse fail",
    [ERR_SWITCH_PART] = "emmc switch part error",
    [ERR_CMD_ERROR]  = "can not find command in cmd_table",
    [ERR_PT_BASE_ERROR] = "target base error, check sfs or parition",
    [ERR_PT_SIZE_ERROR] = "target size error, check sfs or parition",
    [ERR_PTNAME_NOT_EXIT] = "can't get the download partition name",
    [ERR_DISK_NOT_EXIT] = "can't find a matched disk for download",
    [ERR_DISK_OPEN_ERROR] = "open the download disk failed",
    [ERR_PARTITION_READ_ERROR] = "read partition table error",
    [HYPER_FLASH_FUSE_READ_ERROR] = "read hyper flash fuse error",
    [HYPER_FLASH_FUSE_WRITE_ERROR] = "write hyper flash fuse error",
    [HYPER_FLASH_FUSE_VAL_ERROR] = "hyper flash fuse value error",
};

/* flash command */
DL_ERR_CODE_E flash_sparse_img(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t device_inst, uint64_t ptn, uint64_t size);
DL_ERR_CODE_E flash_gpt_table(DL_STATE_T *ds, void *data, unsigned sz,
                              uint8_t sparse_data);
DL_ERR_CODE_E flash_ospi_sfs_areas(DL_STATE_T *ds, void *data, unsigned sz,
                                   uint8_t sparse_data);
DL_ERR_CODE_E flash_ospi_rfd_areas(DL_STATE_T *ds, void *data, unsigned sz,
                                   uint8_t sparse_data);
DL_ERR_CODE_E md5_read_check(DL_STATE_T *ds, uint8_t inst, void *data,
                             unsigned sz, uint64_t read_addr);
DL_ERR_CODE_E flash_normal_partition(DL_STATE_T *ds, void *data, unsigned sz,
                                     uint8_t sparse_data);
DL_ERR_CODE_E flash_emmc_pack0(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_emmc_pack1(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_emmc_pack2(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_ospi_pack0(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_ospi_pack1(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_ospi_pack2(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data);
DL_ERR_CODE_E flash_addr_direct(DL_STATE_T *ds, void *data, unsigned sz,
                                uint8_t sparse_data);
/* erase command */
DL_ERR_CODE_E erase_gpt_table(DL_STATE_T *ds);
DL_ERR_CODE_E erase_normal_partition(DL_STATE_T *ds);
DL_ERR_CODE_E erase_all(DL_STATE_T *ds);
DL_ERR_CODE_E erase_ospi_sfs_areas(DL_STATE_T *ds);
DL_ERR_CODE_E erase_ospi_rfd_areas(DL_STATE_T *ds);
DL_ERR_CODE_E erase_emmc_pack0(DL_STATE_T *ds);
DL_ERR_CODE_E erase_emmc_pack1(DL_STATE_T *ds);
DL_ERR_CODE_E erase_emmc_pack2(DL_STATE_T *ds);
DL_ERR_CODE_E erase_ospi_pack0(DL_STATE_T *ds);
DL_ERR_CODE_E erase_ospi_pack1(DL_STATE_T *ds);
DL_ERR_CODE_E erase_ospi_pack2(DL_STATE_T *ds);
DL_ERR_CODE_E erase_addr_direct(DL_STATE_T *ds);

/* dloader command table */
static COMMAND_TABLE_T dl_cmd_table[DL_CMD_TB_LEN] = {
    {TYPE_PT_UNKNOWN,    ALLDISK,  NULL,                   NULL,                   NULL,                      NULL,                     },
    {TYPE_PRI_PTB,       ALLDISK,  flash_gpt_table,        erase_gpt_table,        "flash_gpt_table",         "erase_gpt_table",        },
    {TYPE_PRI_PT,        ALLDISK,  flash_normal_partition, erase_normal_partition, "flash_normal_partition",  "erase_normal_partition", },
    {TYPE_SUB_PTB,       ALLDISK,  flash_gpt_table,        erase_gpt_table,        "flash_gpt_table",         "erase_gpt_table",        },
    {TYPE_SUB_PT,        ALLDISK,  flash_normal_partition, erase_normal_partition, "flash_normal_partition",  "erase_normal_partition", },
    {TYPE_SUB_PT_WHOLE,  ALLDISK,  flash_normal_partition, erase_normal_partition, "flash_normal_partition",  "erase_normal_partition", },
    {TYPE_ALL_PT,        ALLDISK,  NULL,                   erase_all,               NULL,                      "erase_all",             },
    {TYPE_NOT_IN_GPT,    ALLDISK,  NULL,                   NULL,                    NULL,                     NULL,                     },
    {TYPE_BOOT_PACK0,    MMC,      flash_emmc_pack0,       erase_emmc_pack0,       "flash_emmc_pack0",        "erase_emmc_pack0",       },
    {TYPE_BOOT_PACK1,    MMC,      flash_emmc_pack1,       erase_emmc_pack1,       "flash_emmc_pack1",        "erase_emmc_pack1",       },
    {TYPE_BOOT_PACK2,    MMC,      flash_emmc_pack2,       erase_emmc_pack2,       "flash_emmc_pack2",        "erase_emmc_pack2",       },
    {TYPE_BOOT_PACK0,    NORFLASH, flash_ospi_pack0,       erase_ospi_pack0,       "flash_ospi_pack0",        "erase_ospi_pack0",       },
    {TYPE_BOOT_PACK1,    NORFLASH, flash_ospi_pack1,       erase_ospi_pack1,       "flash_ospi_pack1",        "erase_ospi_pack1",       },
    {TYPE_BOOT_PACK2,    NORFLASH, flash_ospi_pack2,       erase_ospi_pack2,       "flash_ospi_pack2",        "erase_ospi_pack2",       },
    {TYPE_SAFETY_SFS_PT, NORFLASH, flash_ospi_sfs_areas,   erase_ospi_sfs_areas,   "flash_ospi_sfs_areas",    "erase_ospi_sfs_areas",   },
    {TYPE_SAFETY_RFD_PT, NORFLASH, flash_ospi_rfd_areas,   erase_ospi_rfd_areas,   "flash_ospi_rfd_areas",    "erase_ospi_rfd_areas",   },
    {TYPE_ADD_DIRECT,    ALLDISK,  flash_addr_direct,      erase_addr_direct,      "flash_addr_direct",       "erase_addr_direct",      },
};

/* download type table */
static DOWNLOAD_TYPE_TABLE_T dl_type_table[DL_TYPE_TB_LEN] = {
    {PARTITION_TABLE_NAME,  PRI_PARTITION,     "primary parition table",  TYPE_PRI_PTB},
    {PARTITION_TABLE_NAME,  SUB_PARTITION,     "sub partition table",     TYPE_SUB_PTB},
    {ALL_DISK_NAME,         PRI_PARTITION,     "all disk",                TYPE_ALL_PT},
    {BOOT_PARTITION_NAME0,  PRI_PARTITION,     "boot package0",           TYPE_BOOT_PACK0},
    {BOOT_PARTITION_NAME1,  PRI_PARTITION,     "boot package1",           TYPE_BOOT_PACK1},
    {BOOT_PARTITION_NAME2,  PRI_PARTITION,     "boot package2",           TYPE_BOOT_PACK2},
    {SFS_PT_NAME,           PRI_PARTITION,     "sfs",                     TYPE_SAFETY_SFS_PT},
    {RFD_PT_NAME,           PRI_PARTITION,     "rfd",                     TYPE_SAFETY_RFD_PT},
};

/* disk device for dloader, will be init in open_disk function */
static struct disk_dev disk_dev_norflash0     = {0};
static struct disk_dev disk_dev_emmc0         = {0};
static struct disk_dev disk_dev_emmc0_boot1   = {0};
static struct disk_dev disk_dev_emmc0_boot2   = {0};
static struct disk_dev disk_dev_sdcard0       = {0};

/* all download disk state is listed here*/
static DL_STATE_T dl_state[] = {

    /* Nor Flash 0 */
    [0] = {
        .download_name  = OSPI1_STORAGE_NAME,
#if CONFIG_DLODER_USE_RAM_AS_DISK
        .disk_name      = DISK_RAM_NAME(0),
#else
        .disk_name      = DISK_NOR_FLASH_NAME(0),
#endif
        .disk_type      = NORFLASH,
        .disk_inst[0]   = &disk_dev_norflash0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .ptb_offset     = 0x2000,
        .boot_offset    = 0x7000,
    },

    /* mmc0 */
    [1] = {
        .disk_type       = MMC,
        .download_name   = EMMC1_STORAGE_NAME,
#if CONFIG_DLODER_USE_RAM_AS_DISK
        .disk_name       = DISK_RAM_NAME(1),
        .disk_boot1_name = DISK_RAM_NAME(2),
        .disk_boot2_name = DISK_RAM_NAME(3),
#else
        .disk_name       = DISK_MMC_NAME(0),
        .disk_boot1_name = DISK_MMC_BOOT_NAME(0, 1),
        .disk_boot2_name = DISK_MMC_BOOT_NAME(0, 2),
#endif
        .disk_inst[0]    = &disk_dev_emmc0,
        .disk_inst[1]    = &disk_dev_emmc0_boot1,
        .disk_inst[2]    = &disk_dev_emmc0_boot2,
        .ptb_offset      = 0,
        .boot_offset     = 0,
    },

    /* SD CARD0 */
    [2] = {
        .disk_type      = MMC,
        .download_name  = SD1_STORAGE_NAME,
#if CONFIG_DLODER_USE_RAM_AS_DISK
        .disk_name      = DISK_RAM_NAME(0),
#elif CONFIG_SDCARD_SLOT_NUMBER
        .disk_name      = DISK_MMC_NAME(1),
#else
        .disk_name      = DISK_MMC_NAME(0),
#endif
        .disk_inst[0]   = &disk_dev_sdcard0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .ptb_offset     = 0,
        .boot_offset    = SD_SPL_OFFSET,
    },
};

/**
 * @brief get the lowest common multiple for "size" and "aligned"
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
 * @brief  get the greatest common divisor for "size" and "aligned"
 * @param size iput number
 * @param aligned aligned number
 * @return uint64_t the greatest common divisor
 */
static uint64_t round_down(uint64_t size, uint64_t aligned)
{
    uint64_t mod = 0;

    if (aligned == 0 || size < aligned)
        return 0;

    /* Sometimes, 'aligned' is not equal to power of 2 */
    mod = size % aligned;

    size -= mod;
    return size;
}

/**
 * @brief char to hex
 * @param c char
 * @return uint8_t hex value
 */
static uint8_t char2hex(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return 0;
}

/**
 * @brief string to hex
 * @param str
 * @param str_len
 * @param hex
 * @param hex_len
 */
static void str2hex(const char *str, uint32_t str_len, uint8_t *hex,
                    uint32_t hex_len)
{
    for (uint32_t i = 0; i < str_len / 2 && i < hex_len; i++) {
        hex[i] = 0;
        hex[i] = (char2hex(str[i * 2]) & 0xF) << 4;
        hex[i] |= char2hex(str[i * 2 + 1]) & 0xF;
    }
}

/**
 * @brief read sfs of nor flash
 * @param ds download disk states
 * @return int
 */
void read_sfs(DL_STATE_T *ds)
{
    uint8_t *buffer = (uint8_t *)dl_scratch_base;
    uint32_t sfs_size = 0;
    struct sfs *sfs = NULL;

    if (ds->disk_type != NORFLASH) {
        ERROR("disk type must be flash type\n");
        return;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, ds->block_size);
    sfs = (struct sfs *)osAllocAlign(ds->block_size, sfs_size);

    if (!sfs) {
        ERROR("allocate memory fail\n");
        return;
    }

    memset(buffer, 0x0, sfs_size);

    if (dl_disk_read(ds->disk_inst[0], 0, buffer, sfs_size)) {
        ERROR("read nor flash fail\n");
        goto end;
    }

    DBG("sfs dump\n");
    hexdump(buffer, ds->block_size);

    if (get_sfs_info(sfs, buffer, ds->block_size)) {
        ERROR("sfs error\n");
        goto end;
    }

    DBG("sfs tag = 0x%08x\n", sfs->tag);
    DBG("sfs freq = 0x%02x\n", sfs->freq);
    DBG("sfs sw_reset_info = 0x%02x\n", sfs->sw_reset_info);
    DBG("sfs normal_img_base = 0x%08x\n", sfs->normal_img_base);
    DBG("sfs backup_img_base = 0x%08x\n", sfs->backup_img_base);
    DBG("sfs third_img_base = 0x%08x\n", sfs->third_img_base);
    DBG("sfs crc32 = 0x%08x\n", sfs->crc32);

end:

    if (sfs)
        osFree(sfs);

    return;
}

/**
 * @brief read sfs and get the image base
 * @param ds download disk states
 * @param num package number
 * @return int
 */
uint32_t sfs_get_image_base(DL_STATE_T *ds, uint8_t num)
{
    uint8_t *buffer = (uint8_t *)dl_scratch_base;
    uint32_t sfs_size = 0;
    struct sfs *sfs = NULL;
    uint32_t img_base = 0;

    if (ds->disk_type != NORFLASH) {
        ERROR("ds->disk_type != NORFLASH\n");
        return 0;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, ds->block_size);
    sfs = (struct sfs *)osAllocAlign(ds->block_size, sfs_size);

    if (!sfs) {
        ERROR("allocate memory fail\n");
        return 0;
    }

    memset(buffer, 0x0, sfs_size);

    if (dl_disk_read(ds->disk_inst[0], 0, buffer, sfs_size)) {
        ERROR("read nor flash fail\n");
        goto end;
    }

    if (get_sfs_info(sfs, buffer, ds->block_size)) {
        ERROR("sfs error\n");
        goto end;
    }

    if (BOOT0_NUM == num) {
        DBG("get sfs normal_img_base = 0x%08x\n", sfs->normal_img_base);
        img_base = sfs->normal_img_base;
    }
    else if (BOOT1_NUM == num) {
        DBG("get sfs backup_img_base = 0x%08x\n", sfs->backup_img_base);
        img_base = sfs->backup_img_base;
    }
    else if (BOOT2_NUM == num) {
        DBG("get sfs third_img_base = 0x%08x\n", sfs->third_img_base);
        img_base = sfs->third_img_base;
    }
    else {
        ERROR("num = %d, must < 3\n", num);
        goto end;
    }

end:

    if (sfs)
        osFree(sfs);

    return img_base;
}

/**
 * @brief read sfs and get the image limit size
 * @param ds download disk states
 * @param num inst number
 * @return int
 */
uint32_t sfs_get_image_size_limit(DL_STATE_T *ds, uint8_t num)
{
    uint8_t *buffer = (uint8_t *)dl_scratch_base;
    uint32_t sfs_size = 0;
    struct sfs *sfs = NULL;
    uint32_t img_size = 0;
    uint32_t img_base[BOOT_MAX] = {0};
    uint32_t max_img_base = 0;
    uint32_t min_img_base = 0;

    if (ds->disk_type != NORFLASH) {
        ERROR("ds->disk_type != NORFLASH\n");
        return 0;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, ds->block_size);
    sfs = (struct sfs *)osAllocAlign(ds->block_size, sfs_size);

    if (!sfs) {
        ERROR("allocate memory fail\n");
        return 0;
    }

    memset(buffer, 0x0, sfs_size);

    if (dl_disk_read(ds->disk_inst[0], 0, buffer, sfs_size)) {
        ERROR("read nor flash fail\n");
        goto end;
    }

    if (get_sfs_info(sfs, buffer, ds->block_size)) {
        ERROR("sfs error\n");
        goto end;
    }

    img_base[BOOT0_NUM] = sfs->normal_img_base;
    img_base[BOOT1_NUM] = sfs->backup_img_base;
    img_base[BOOT2_NUM] = sfs->third_img_base;

    if (sfs->normal_img_base == sfs->backup_img_base) {
        img_base[BOOT0_NUM] = 0;
    }

    if (sfs->normal_img_base == sfs->third_img_base) {
        img_base[BOOT1_NUM] = 0;
    }

    if (sfs->backup_img_base == sfs->third_img_base) {
        img_base[BOOT2_NUM] = 0;
    }

    min_img_base = MIN(img_base[BOOT0_NUM], MIN(img_base[BOOT1_NUM],
                       img_base[BOOT2_NUM]));
    max_img_base = MAX(img_base[BOOT0_NUM], MAX(img_base[BOOT1_NUM],
                       img_base[BOOT2_NUM]));

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

    if (sfs)
        osFree(sfs);

    DBG("get image limited size  = 0x%08x\n", img_size);
    return img_size;
}

/**
 * @brief get the error code of this operation
 * @param err
 * @param pname
 * @return char*
 */
static char *response_error(enum dl_err_code err, const char *pname)
{
    static char err_response[MAX_RSP_SIZE];
    const char *info = "unknown error";

    if (err < ERR_UNKNOWN || err >= ERR_MAX) {
        ERROR("unknown error code:%d\n", err);
        return NULL;
    }

    info = err_info[err];
    snprintf(err_response, sizeof(err_response), "%04x:%s - %s", err, info,
             pname);
    return err_response;
}

/**
 * @brief compare gpt's entries
 * @param new GPT_header
 * @param old GPT_header
 * @param count partition_entry's number
 * @return int
 */
static int compare_gpt_entries(struct partition_entry *new,
                               struct partition_entry *old, uint32_t count)
{
    int ret = 0;

    for (uint32_t i = 0; i < count; i++) {

        ret |= strncmp((char *)new[i].name, (char *)(old[i].name),
                       MAX_GPT_NAME_SIZE);
        //ret |= memcmp(new[i].type_guid, old[i].type_guid,
        //              sizeof(new[i].type_guid));
        //ret |= memcmp(new[i].unique_partition_guid, old[i].unique_partition_guid,
        //              sizeof(new[i].unique_partition_guid));
        ret |= (new[i].first_lba != old[i].first_lba);
        ret |= (new[i].attribute_flag != old[i].attribute_flag);

        /* The last partition's last_lba may be extended to
         * storage capacity - gpt_header_block - partition_entries_blocks.
         * Skip it.
         * */
        if (i != count - 1) {
            ret |= (new[i].last_lba != old[i].last_lba);
        }

        if (ret)
            break;
    }

    return ret;
}

/**
 * @brief compare gpt's head
 * @param new GPT_header
 * @param old GPT_header
 * @return int
 */
static int compare_gpt_header(GPT_header *new, GPT_header *old)
{
    int ret = 0;

    ret |= memcmp(new->sign, old->sign, sizeof(new->sign));
    ret |= memcmp(new->version, old->version, sizeof(new->version));
    //ret |= memcmp(new->guid, old->guid, sizeof(new->guid));
    ret |= (new->header_sz != old->header_sz);
    ret |= (new->current_lba != old->current_lba);
    //ret |= (new->backup_lba  != old->backup_lba);//this field will be re-calculated before written to emmc
    ret |= (new->first_usable_lba != old->first_usable_lba);
    //ret |= (new->last_usable_lba  != old->last_usable_lba);//this field will be re-calculated before written to emmc
    ret |= (new->partition_entry_lba != old->partition_entry_lba);
    ret |= (new->partition_entry_count != old->partition_entry_count);
    ret |= (new->partition_entry_sz != old->partition_entry_sz);
    ret |= (new->actual_entries_count != old->actual_entries_count);

    if (ret) {
        return ret;
    }

    ret |= compare_gpt_entries(new->partition_entries, old->partition_entries,
                               new->actual_entries_count);
    return ret;
}

/**
 * @brief check if partition table need to be flash
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return int
 */
static int check_partition_table(DL_STATE_T *ds, void *data, unsigned sz)
{
    int index           = 0;
    uint32_t ret        = PTB_CHECK_ERROR;
    uint64_t offset     = 0;
    uint64_t gpt_sz     = 0;
    uint8_t  *buffer    = NULL;
    uint32_t block_size = 0;
    const char *ptname      = ds->ptname;
    const char *sub_ptbname = ds->sub_ptbname;
    uint32_t blocks_for_entries    = 0;
    struct partition_device *ptdev = ds->ptdev;
    GPT_header buffered_gpt_header = {0};
    GPT_header storage_gpt_header  = {0};
    block_size = ds->block_size;

    if (!block_size || block_size % 4 != 0) {
        goto end;
    }

    blocks_for_entries = (NUM_PARTITIONS * PARTITION_ENTRY_SIZE) / block_size;
    /* MBR is in the first LBA.
     * GPT header is in the second LBA.
     * GPT entries are in the following blocks.
     * */
    gpt_sz = (GPT_HEADER_BLOCKS + blocks_for_entries + 1) * block_size;


    if (sz < gpt_sz) {
        ERROR("download gpt image error");
        goto end;
    }

    if (ds->disk_type == NORFLASH) {
        ret = gpt_partition_round(((uint8_t *)data + block_size),
                                  (gpt_sz - block_size) * 2, block_size, ds->erase_size,
                                  ds->capacity - ds->ptb_offset);

        if (ret) {
            ERROR("patch nor flash ptb error! ret:%d\n", ret);
            ret = PTB_CHECK_ERROR;
            goto end;
        }
    }

    /*
    * LBA0 is for MBR, skip it
    */
    ret = parse_gpt_table_from_buffer(((uint8_t *)data + block_size),
                                      gpt_sz - block_size,
                                      &buffered_gpt_header, block_size, false);

    if (ret) {
        ERROR("parse gpt header error");
        ret = PTB_NEED_FLASH;
        goto end;
    }

#if 0 //DISABLE_GPT_PTB_CHK
    ret = PTB_NEED_FLASH;
    goto end;
#endif
    buffer = osAllocAlign(block_size, round_up(gpt_sz, block_size));

    if (!buffer) {
        ERROR("mem allocate buffer error");
        goto end;
    }

    /* If it is primary partition table, LBA0 is for MBR,skipping */
    if (ds->partiton_type == TYPE_PRI_PTB) {
        offset = ds->ptb_offset;
    }
    else {
        index = ptdev_get_index(ptdev, sub_ptbname);

        if (index == INVALID_PTN) {
            goto end;
        }

        offset = ptdev_get_offset(ptdev, sub_ptbname);
    }

    DBG("ptb_offset:%lld\n", offset);

    if (offset % ds->block_size) {
        ERROR("offset is not aligned to block size");
        goto end;
    }

    ret = dl_disk_read(ds->disk_inst[0], offset, (uint8_t *)buffer, round_up(gpt_sz,
                       ds->block_size));

    if (ret) {
        ERROR("read gpt table error:%s$%s$%s", ds->disk_name, sub_ptbname, ptname);
        hexdump(buffer, 2 * ds->block_size);
        ret = PTB_CHECK_ERROR;
        goto end;
    }

    ret = parse_gpt_table_from_buffer((buffer + block_size),
                                      gpt_sz - block_size,
                                      &storage_gpt_header, block_size, false);

    if (ret) {
        DBG("no available gpt in device, need flash partition table\n");


        /* There is no correct gpt header in emmc,
         * so needs flash gpt image by returing PTB_NEED_FLASH
         * */
        ret = PTB_NEED_FLASH;
        goto end;
    }

    ret = compare_gpt_header(&buffered_gpt_header, &storage_gpt_header);

    if (!ret) {
        ret = PTB_NO_NEED_FLASH;
    }
    else {
        ret = PTB_NEED_FLASH;
    }

end:

    if (buffered_gpt_header.partition_entries != NULL) {
        osFree(buffered_gpt_header.partition_entries);
    }

    if (storage_gpt_header.partition_entries != NULL) {
        osFree(storage_gpt_header.partition_entries);
    }

    if (buffer) {
        osFree(buffer);
    }

    return ret;
}

/**
 * @brief erase nor flash areas
 *
 * @param ds download disk states
 * @param full_ptname parition name
 * @param img_sz parition s
 * @return true
 * @return false
 */
static bool erase_nor_flash_partition(DL_STATE_T *ds, uint64_t ptn,
                                      uint64_t size,
                                      uint64_t img_sz)
{
    uint64_t erase_size;

    if (!img_sz || !ptn || !size) {
        ERROR("img_sz or ptn or size is 0\n");
        return false;
    }

    erase_size = round_up(img_sz, ds->erase_size);
    erase_size = erase_size > size ? size : erase_size;
    erase_size = (size - erase_size > ds->erase_size) ? erase_size : size;

    if (dl_disk_erase(ds->disk_inst[0], ptn, erase_size)) {
        ERROR("erase storage error\n");
        return false;
    }

    /* erase the last sector for footer */
    if (( size - erase_size > ds->erase_size)
            && dl_disk_erase(ds->disk_inst[0], ptn + size - ds->erase_size,
                             ds->erase_size)) {
        ERROR("erase storage error\n");
        return false;
    }

    DBG("erase ptn = 0x%llx erase_size:0x%llx size:0x%llx group:0x%x\n",
        ptn, erase_size, size, ds->erase_size);
    return true;
}

/**
 * @brief flash one area using sparsing format
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_sparse_img(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t device_inst, uint64_t ptn, uint64_t size)
{
    uint32_t i         = 0;
    uint32_t fill_val  = 0;
    uintptr_t data_end = (uintptr_t)data + sz;
    unsigned int chunk = 0;
    uint32_t *fill_buf = NULL;
    bool fill_buf_init = false;
    void *data_ptr_temp = NULL;
    void *data_back = data;
    uint32_t total_blocks   = 0;
    uint64_t pos  = 0;
    uint32_t blk_sz_actual  = 0;
    uint64_t chunk_data_sz  = 0;
    uint8_t *block_wrapper  = NULL;
    uint64_t count_aligned  = 0;
    chunk_header_t *chunk_header   = NULL;
    sparse_header_t *sparse_header = NULL;
    uint64_t chunk_data_sz_remain  = 0;
    block_wrapper = (uint8_t *)(dl_scratch_base);
    uint8_t j = device_inst;
    uint8_t  md5_calc_bef[MD5_LEN] = {0};
    uint8_t  md5_calc_aft[MD5_LEN] = {0};

#if CONFIG_DLOADER_BLOCK_IO_MODE
    uint32_t sector_size = ds->erase_size;
#endif

    DBG("Flashing sparce img\n");

    if (!block_wrapper
            || ((addr_t)block_wrapper % ds->block_size) != 0
            || (dl_scratch_sz % ds->block_size) != 0
            || dl_scratch_sz == 0) {
        ERROR("aligned  memory allocate fail!\n");
        return ERR_SPARSE_IMAGE_MALLOC;
    }

#if CONFIG_DLOADER_WITH_TRACE
    return ERR_SPARSE_IMAGE_DO_NOT_SUPPORT;
#endif

    DBG("inst number=%d\n", j);
    DBG("size limit=%lld, ds->boot_offset(ptn)=%lld\n", size, ptn);
    total_blocks = 0;
    data = data_back;
    /* Read and skip over sparse image header */
    sparse_header = (sparse_header_t *) data;

    if (!sparse_header->blk_sz || (sparse_header->blk_sz % 4)) {
        ERROR("block size error:%u\n", sparse_header->blk_sz);
        return ERR_INVALID_BLOCK_SIZE;
    }

    if (((uint64_t)sparse_header->total_blks * (uint64_t)sparse_header->blk_sz) >
            size) {
        ERROR("image too large :%llu\n", size);
        return ERR_IMAGE_TOO_LARGE;
    }

    data = (uint8_t *)data + sizeof(sparse_header_t);

    if (data_end < (uintptr_t)data) {
        ERROR("data end:%u  header size:%zu\n", data_end, sizeof(sparse_header_t));
        return ERR_SPARSE_IMAGE_BUFFERED;
    }

    if (sparse_header->file_hdr_sz != sizeof(sparse_header_t)) {
        ERROR("image header error!\n");
        return ERR_SPARSE_IMAGE_HEADER;
    }

    DBG("=== Sparse Image Header ===\n");
    DBG("magic: 0x%x\n", sparse_header->magic);
    DBG("major_version: 0x%x\n", sparse_header->major_version);
    DBG("minor_version: 0x%x\n", sparse_header->minor_version);
    DBG("file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
    DBG("chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
    DBG("blk_sz: %d\n", sparse_header->blk_sz);
    DBG("total_blks: %d\n", sparse_header->total_blks);
    DBG("total_chunks: %d\n", sparse_header->total_chunks);

    /* Start processing chunks */
    for (chunk = 0; chunk < sparse_header->total_chunks; chunk++) {
        /* Make sure the total image size does not exceed the partition size */
        if (((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz) >= size) {
            ERROR("image too large:%llu!\n", size);
            return ERR_IMAGE_TOO_LARGE;
        }

        /* Read and skip over chunk header */
        chunk_header = (chunk_header_t *) data;
        data = (uint8_t *)data + sizeof(chunk_header_t);

        if (data_end < (uintptr_t)data) {
            ERROR("data end:%u  data:%p chunk header size:%zu\n", data_end, data,
                  sizeof(chunk_header_t));
            return ERR_IMAGE_TOO_LARGE;
        }

        DBG("=== Chunk Header ===\n");
        DBG("chunk_type: 0x%x\n", chunk_header->chunk_type);
        DBG("chunk_data_sz/sparse_blk_sz: 0x%x\n", chunk_header->chunk_sz);
        DBG("total_size: 0x%x\n", chunk_header->total_sz);

        if (sparse_header->chunk_hdr_sz != sizeof(chunk_header_t)) {
            ERROR("chunk header error:%u!\n", sparse_header->chunk_hdr_sz);
            return ERR_SPARSE_IMAGE_CHUNK_HEADER;
        }

        chunk_data_sz = (uint64_t)sparse_header->blk_sz * chunk_header->chunk_sz;

        /* Make sure that the chunk size calculated from sparse image does not exceed partition size */
        if ((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz + chunk_data_sz >
                size) {
            ERROR("chunk data too large:%llu!\n", size);
            return ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE;
        }

        switch (chunk_header->chunk_type) {
            case CHUNK_TYPE_RAW:
                if ((uint64_t)chunk_header->total_sz != ((uint64_t) sparse_header->chunk_hdr_sz
                        + chunk_data_sz)) {
                    ERROR("chunk size:%llu error!\n", chunk_data_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                if (data_end < (uintptr_t)data + chunk_data_sz) {
                    ERROR("data end:%u  data:%p chunk_data_sz:%llu\n", data_end, data,
                          chunk_data_sz);
                    return ERR_SPARSE_IMAGE_BUFFERED;
                }

                /* chunk_header->total_sz is uint32,So chunk_data_sz is now less than 2^32
                otherwise it will return in the line above
                */
                pos = ptn + ((uint64_t)total_blocks * sparse_header->blk_sz);
                count_aligned = 0;
                data_ptr_temp = data;
                chunk_data_sz_remain  = chunk_data_sz;

#if CONFIG_DLOADER_BLOCK_IO_MODE

                /* for ospi nor flash, erase the partition before writting */
                if (pos == ptn && (ds->disk_type == NORFLASH)) {
                    ASSERT(pos % sector_size == 0);

                    if (dl_disk_erase(ds->disk_inst[j], pos,
                                      round_up((sparse_header->blk_sz * sparse_header->total_blks), sector_size))) {
                        ERROR("disk_erase_group addr = 0x%llx error\n", pos);
                        return ERR_PT_ERASE_FAIL;
                    }
                }

#endif

                while (chunk_data_sz_remain) {
                    count_aligned = MIN(dl_scratch_sz, chunk_data_sz_remain);
                    memset(block_wrapper, 0x0, dl_scratch_sz);
                    memcpy(block_wrapper, data_ptr_temp, count_aligned);

                    if (do_md5_rb_check) {
                        md5((unsigned char *)(block_wrapper), count_aligned,
                            (unsigned char *)md5_calc_bef);
                    }

                    if (dl_disk_write(ds->disk_inst[j], pos, block_wrapper,
                                      round_up(count_aligned, (uint64_t)ds->block_size))) {
                        ERROR("flash storage error\n");
                        return ERR_PT_FLASH_FAIL;
                    }

                    if (do_md5_rb_check) {
                        memset(block_wrapper, 0x0, dl_scratch_sz);

                        if (dl_disk_read(ds->disk_inst[j], pos, block_wrapper,
                                         round_up(count_aligned, (uint64_t)ds->block_size))) {
                            ERROR("flash storage error\n");
                            return ERR_PT_READ_FAIL;
                        }

                        md5((unsigned char *)(block_wrapper), count_aligned,
                            (unsigned char *)md5_calc_aft);

                        if (memcmp(md5_calc_aft, md5_calc_bef, MD5_LEN)) {
                            ERROR("md5 check fail!\n");
                            hexdump(md5_calc_bef, MD5_LEN);
                            hexdump(md5_calc_aft, MD5_LEN);
                            return ERR_HASH_FAIL;
                        }

                        DBG("md5 check ok!\n");
                    }

                    pos += count_aligned;
                    data_ptr_temp = (uint8_t *)data_ptr_temp + count_aligned;
                    chunk_data_sz_remain -= count_aligned;
                }

                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR("chunk size error:%u!\n", chunk_header->chunk_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                total_blocks += chunk_header->chunk_sz;
                data = (uint8_t *)data + (uint32_t)chunk_data_sz;
                break;

            case CHUNK_TYPE_FILL:
                if (chunk_header->total_sz != (sparse_header->chunk_hdr_sz + sizeof(
                                                   uint32_t))) {
                    ERROR("fill type error, size:%u!\n", chunk_header->total_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                blk_sz_actual = round_up(sparse_header->blk_sz, 32);

                /* Integer overflow detected */
                if (blk_sz_actual < sparse_header->blk_sz) {
                    ERROR("blk_sz_actual:%u error!\n", blk_sz_actual);
                    return ERR_INVALID_BLOCK_SIZE;
                }

                fill_buf = (uint32_t *)(dl_scratch_base);

                if (!fill_buf) {
                    ERROR(" allocat memory fail!\n");
                    return ERR_SPARSE_IMAGE_MALLOC;
                }

                if (data_end < (uintptr_t)data + sizeof(uint32_t)) {
                    ERROR("data end:%u  data:%p \n", data_end, data);
                    return ERR_SPARSE_IMAGE_BUFFERED;
                }

                fill_val = *(uint32_t *)data;
                data = (char *) data + sizeof(uint32_t);

                chunk_data_sz_remain  = chunk_header->chunk_sz * sparse_header->blk_sz;
                fill_buf_init = false;

                while (chunk_data_sz_remain) {

                    if (dl_scratch_sz > chunk_data_sz_remain) {
                        count_aligned = chunk_data_sz_remain;
                    }
                    else {
                        count_aligned = dl_scratch_sz;
                    }

                    if (!fill_buf_init) {
                        for (i = 0; !fill_buf_init && i < (count_aligned / sizeof(fill_val)); i++) {
                            fill_buf[i] = fill_val;
                        }

                        fill_buf_init = true;
                    }

                    if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                        ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                        return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                    }

                    /* Make sure that the data written to partition does not exceed partition size */
                    if ((uint64_t)total_blocks * (uint64_t)sparse_header->blk_sz +
                            sparse_header->blk_sz > size) {
                        ERROR(" size:%llu error!\n", size);
                        return ERR_SPARSE_IMAGE_CHUNK_TOO_LARGE;
                    }

                    pos = ptn + ((uint64_t)total_blocks * sparse_header->blk_sz);

#if CONFIG_DLOADER_BLOCK_IO_MODE

                    /* for ospi nor flash, erase the partition before writting */
                    if (pos == ptn && (ds->disk_type == NORFLASH)) {
                        ASSERT(pos % sector_size == 0);

                        if (dl_disk_erase(ds->disk_inst[j], pos,
                                          round_up((sparse_header->blk_sz * sparse_header->total_blks), sector_size))) {
                            ERROR("disk_erase_group addr = 0x%llx error\n", pos);
                            return ERR_PT_ERASE_FAIL;
                        }
                    }

#endif

                    if (do_md5_rb_check) {
                        md5((unsigned char *)(fill_buf), count_aligned, (unsigned char *)md5_calc_bef);
                    }

                    if (dl_disk_write(ds->disk_inst[j], pos, (uint8_t *)fill_buf,
                                      round_up(count_aligned, (uint64_t)ds->block_size))) {
                        ERROR(" write data error!\n");
                        return ERR_PT_FLASH_FAIL;
                    }

                    if (do_md5_rb_check) {
                        memset(block_wrapper, 0x0, dl_scratch_sz);

                        if (dl_disk_read(ds->disk_inst[j], pos, block_wrapper,
                                         round_up(count_aligned, (uint64_t)ds->block_size))) {
                            ERROR("flash storage error\n");
                            return ERR_PT_READ_FAIL;
                        }

                        md5((unsigned char *)(fill_buf), count_aligned, (unsigned char *)md5_calc_aft);

                        if (memcmp(md5_calc_aft, md5_calc_bef, MD5_LEN)) {
                            ERROR("md5 check fail!\n");
                            hexdump(md5_calc_bef, MD5_LEN);
                            hexdump(md5_calc_aft, MD5_LEN);
                            return ERR_HASH_FAIL;
                        }

                        DBG("md5 check ok!\n");
                    }

                    chunk_data_sz_remain -= count_aligned;
                    total_blocks += count_aligned / sparse_header->blk_sz;
                }

                break;

            case CHUNK_TYPE_DONT_CARE:
                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                total_blocks += chunk_header->chunk_sz;
                break;

            case CHUNK_TYPE_CRC:
                if (chunk_header->total_sz != sparse_header->chunk_hdr_sz) {
                    ERROR(" chunk total size:%u error!\n", chunk_header->total_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                if (total_blocks > (UINT_MAX - chunk_header->chunk_sz)) {
                    ERROR(" chunk size:%u error!\n", chunk_header->chunk_sz);
                    return ERR_SPARSE_IMAGE_CHUNK_NOT_MATCH;
                }

                total_blocks += chunk_header->chunk_sz;

                if ((uintptr_t)data > UINT_MAX - chunk_data_sz) {
                    ERROR(" chunk data size:%llu error!\n", chunk_data_sz);
                    return ERR_IMAGE_TOO_LARGE;
                }

                data = (uint8_t *)data + (uint32_t)chunk_data_sz;

                if (data_end < (uintptr_t)data) {
                    ERROR("data end:%u  data:%p \n", data_end, data);
                    return ERR_SPARSE_IMAGE_BUFFERED;
                }

                break;

            default:
                ERROR("Unkown chunk type: %x\n", chunk_header->chunk_type);
                return ERR_SPARSE_IMAGE_CHUNK_UNKNOWN;
        }
    }

    DBG("Wrote %d blocks, expected to write %d blocks\n",
        total_blocks, sparse_header->total_blks);

    if (total_blocks != sparse_header->total_blks) {
        ERROR(" total block:%u error!\n", total_blocks);
        return ERR_PT_FLASH_FAIL;
    }

    return ERR_NONE;
}

/**
 * @brief read back data from disk and do md5 check
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @param read_addr read back address
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E md5_read_check(DL_STATE_T *ds, uint8_t inst, void *data,
                             unsigned sz, uint64_t read_addr)
{
    uint8_t  md5_calc[MD5_LEN] = {0};
    unsigned size = 0;
    struct MD5Context context;
    uint32_t i = 0;
    uint32_t cnt = 0;
    uint32_t left = 0;
    uint32_t length = 0;

    ASSERT(inst < EMMC_DEV_INST_MAX);
    size = sz;
    MD5Init(&context);
    cnt = size / dl_scratch_sz;
    left = size % dl_scratch_sz;

    for (i = 0; i <= cnt; i++) {

        if (i == cnt) {
            length = left;
        }
        else {
            length = dl_scratch_sz;
        }

        if (0 == length)
            break;

        memset((uint8_t *)(dl_scratch_base), 0x0, length);

#if CONFIG_DLOADER_BLOCK_IO_MODE

        if (dl_disk_read(ds->disk_inst[inst], round_down(read_addr, ds->block_size),
                         (uint8_t *)(dl_scratch_base), round_up(length, ds->block_size))) {
            ERROR("read back error!\n");
            return ERR_HASH_FAIL;
        }

#else

        if (dl_disk_read(ds->disk_inst[inst], read_addr,
                         (uint8_t *)(dl_scratch_base), length)) {
            ERROR("read back error!\n");
            return ERR_HASH_FAIL;
        }

#endif

        MD5Update(&context, (uint8_t *)(dl_scratch_base), length);
        read_addr += length;
    }

    MD5Final(md5_calc, &context);

#if CONFIG_DLOADER_WITH_TRACE
    /*when downloading with trace, we do not have md5 received from PC, so calc here*/
    md5(data, sz, md5_received);
#endif

    if (memcmp(md5_received, md5_calc, MD5_LEN)) {
        ERROR("md5 check fail!\n");
        hexdump(md5_received, MD5_LEN);
        hexdump(md5_calc, MD5_LEN);
        return ERR_HASH_FAIL;
    }

    hexdump(md5_calc, MD5_LEN);
    DBG("read back md5 check success!\n");
    return ERR_NONE;
}

/**
 * @brief flash an addr directly
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @param sparse_data use parse format or not
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_addr_direct(DL_STATE_T *ds, void *data, unsigned sz,
                                uint8_t sparse_data)
{
    char *ptname = ds->ptname;
    uint64_t ptn  = 0;
    uint64_t size = 0;
    char *addrsuffix;

    DBG("Flashing an addr directly\n");

    ptn = strtoull(ptname, &addrsuffix, 16);

    if ((ptn % ds->block_size) || (ptn > ds->capacity)) {
        ERROR("ptn alignment error  = %llx\n", ptn);
        return ERR_PT_NOT_FOUND;
    }

#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (ds->disk_type == NORFLASH) {
        if ((ptn % ds->erase_size)) {
            ERROR("ptn alignment to erase size error  = %llx\n", ptn);
            return ERR_PT_NOT_FOUND;
        }
    }

#endif

    size = ds->capacity - ptn;

    DBG("ptn:%lld size:%lld\n", ptn, size);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, 0, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->erase_size) > size) {
            ERROR(" image too large:%llu!\n", round_up(sz, ds->erase_size));
            return ERR_IMAGE_TOO_LARGE;
        }

#if CONFIG_DLOADER_WITH_TRACE
        ptn += cmd_args->offset;
        DBG("ptn + cmd_args->offset is 0x%llx\n", ptn);

        if (cmd_args->total_sz > size) {
            ERROR("trace image total size too large:%llu!\n", cmd_args->total_sz);
            return ERR_IMAGE_TOO_LARGE;
        }

        if (ptn % ds->block_size) {
            ERROR("ptn alignment error in trace mode  = %llx\n", ptn);
            return ERR_PT_BASE_ERROR;
        }

#endif

#if CONFIG_DLOADER_BLOCK_IO_MODE
#if CONFIG_DLOADER_WITH_TRACE

        /* e3 do not have enough ram in block io mode, so make sure ptn is aligned to
            erase size and block size */
        if ((ds->disk_type == NORFLASH) && (cmd_args->offset == 0)) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(cmd_args->total_sz,
                              ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }

#else

        if (ds->disk_type == NORFLASH) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(sz, ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }

#endif
#endif

        if (dl_disk_write(ds->disk_inst[0], ptn, (uint8_t *)data, round_up(sz,
                          ds->block_size))) {
            ERROR(" write data error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check)
            return md5_read_check(ds, 0, data, sz, ptn);

        return ERR_NONE;
    }
}

/**
 * @brief flash a normal partition
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_normal_partition(DL_STATE_T *ds, void *data, unsigned sz,
                                     uint8_t sparse_data)
{
    char *ptname                                = ds->ptname;
    char *sub_ptbname                           = ds->sub_ptbname;
    PARTITION_TYPE_E pttype                     = ds->partiton_type;
    struct partition_device *ptdev              = ds->ptdev;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};
    uint64_t ptn  = 0;
    uint64_t size = 0;

    DBG("Flashing normal partition img\n");

    if (pttype == TYPE_SUB_PT) {
        /* If the partition is sub partition, we should pass the full name */
        snprintf(full_ptname, sizeof(full_ptname), "%s$%s", sub_ptbname, ptname);
    }
    else if (pttype == TYPE_SUB_PT_WHOLE) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
    }
    else {
        /* This is a primary partition */
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
    }

    ptn = ptdev_get_offset(ptdev, full_ptname);

    if (ptn == 0 || ptn > ds->capacity) {
        ERROR("ptn size alignment error  = %llx\n", ptn);
        return ERR_PT_NOT_FOUND;
    }

    size = ptdev_get_size(ptdev, full_ptname);

    if (size < ds->block_size  || (size > (ds->capacity - ds->ptb_offset))) {
        ERROR("size error!\n");
        return ERR_PT_NOT_FOUND;
    }

    if (ptn % ds->block_size || size % ds->block_size) {
        ERROR("ptn:%lld is not aligned to block size\n", ptn);
        return ERR_PT_OVERLAP;
    }

    if (ds->disk_type == NORFLASH) {
        if (ptn % ds->erase_size != 0 || size % ds->erase_size != 0) {
            ERROR("the size of the partition in nor flash is not aligned to erase group size\n");
            return ERR_PT_OVERLAP;
        }
    }

    DBG("ptn:%lld size:%lld\n", ptn, size);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, 0, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->erase_size) > size) {
            ERROR(" image too large:%llu!\n", round_up(sz, ds->block_size));
            return ERR_IMAGE_TOO_LARGE;
        }

#if CONFIG_DLOADER_WITH_TRACE
        ptn += cmd_args->offset;
        DBG("ptn + cmd_args->offset is 0x%llx\n", ptn);

        if (cmd_args->total_sz > size) {
            ERROR("trace image total size too large:%llu!\n", cmd_args->total_sz);
            return ERR_IMAGE_TOO_LARGE;
        }

        if ((ptn % ds->block_size)) {
            ERROR("ptn alignment error in trace mode  = %llx\n", ptn);
            return ERR_PT_BASE_ERROR;
        }

#endif

#if CONFIG_DLOADER_BLOCK_IO_MODE
#if CONFIG_DLOADER_WITH_TRACE

        /* e3 do not have enough ram in block io mode, so make sure ptn is aligned to
            erase size and block size */
        if ((ds->disk_type == NORFLASH) && (cmd_args->offset == 0)) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(cmd_args->total_sz,
                              ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }

#else

        if (ds->disk_type == NORFLASH) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(sz, ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }

#endif
#endif

        if (dl_disk_write(ds->disk_inst[0], ptn, (uint8_t *)data, round_up(sz,
                          ds->block_size))) {
            ERROR(" write data error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check)
            return md5_read_check(ds, 0, data, sz, ptn);

        return ERR_NONE;
    }
}

/**
 * @brief flash emmc boot1 areas
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_emmc_pack0(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    uint8_t i   = EMMC_BOOT1_DEV_INST;
    uint64_t ptn = 0;
    uint64_t size = 0;
    enum dl_err_code err = ERR_UNKNOWN;

    size = EMMC_BOOT_PT_SIZE;
    ptn  = 0;

    DBG("Flashing emmc's boot0 package img, inst = %d\n", i);
    DBG("ptn:%lld size:%lld sz:%d\n", ptn, size, sz);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, i, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->block_size) > size) {
            ERROR("boot image too large!\n");
            return ERR_IMAGE_TOO_LARGE;
        }

#if CONFIG_DLOADER_WITH_TRACE
        ptn += cmd_args->offset;
        DBG("ptn + cmd_args->offset is 0x%llx\n", ptn);

        if (cmd_args->total_sz > size) {
            ERROR("trace image total size too large:%llu!\n", cmd_args->total_sz);
            return ERR_IMAGE_TOO_LARGE;
        }

        if ((ptn % ds->block_size)) {
            ERROR("ptn alignment error in trace mode  = %llx\n", ptn);
            return ERR_PT_BASE_ERROR;
        }

#endif

        else if (dl_disk_write(ds->disk_inst[i], ptn, (uint8_t *)data, round_up(sz,
                               ds->block_size) )) {
            ERROR("write emmc error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check) {
            err = md5_read_check(ds, i, data, sz, ptn);

            if (ERR_NONE != err)
                return err;
        }

        return ERR_NONE;
    }
}

/**
 * @brief flash emmc boot2 areas
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_emmc_pack1(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    uint8_t i   = EMMC_BOOT2_DEV_INST;
    uint64_t ptn = 0;
    uint64_t size = 0;
    enum dl_err_code err = ERR_UNKNOWN;

    size = EMMC_BOOT_PT_SIZE;
    ptn  = 0;
    DBG("Flashing emmc's boot1 package img, inst = %d\n", i);
    DBG("ptn:%lld size:%lld sz:%d\n", ptn, size, sz);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, i, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->block_size) > size) {
            ERROR("boot image too large!\n");
            return ERR_IMAGE_TOO_LARGE;
        }

        else if (dl_disk_write(ds->disk_inst[i], ptn, (uint8_t *)data, round_up(sz,
                               ds->block_size) )) {
            ERROR("write emmc error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check) {
            err = md5_read_check(ds, i, data, sz, ptn);

            if (ERR_NONE != err)
                return err;
        }

        return ERR_NONE;
    }
}

/**
 * @brief flash emmc user areas with an offset 20k
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_emmc_pack2(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    uint8_t i   = EMMC_USER_SPACE_DEV_INST;
    uint64_t ptn = 0;
    uint64_t size = 0;
    enum dl_err_code err = ERR_UNKNOWN;

    DBG("Flashing emmc's boot2 package img, inst = %d\n", i);

    ptn = ptdev_get_offset(ds->ptdev, boot_package[BOOT2_NUM]);

    if (ptn == SD_SPL_OFFSET) {
        size = ptdev_get_size(ds->ptdev, boot_package[BOOT2_NUM]);
    }
    else {
        ptn = SD_SPL_OFFSET;
        size = ds->capacity - SD_SPL_OFFSET;
    }

    if (ptn == 0 || ptn > ds->capacity) {
        ERROR("ptn size alignment error  = %llx\n", ptn);
        return ERR_PT_NOT_FOUND;
    }

    if (size < ds->block_size  || (size > (ds->capacity - ds->ptb_offset))) {
        ERROR("size error!\n");
        return ERR_PT_NOT_FOUND;
    }

    DBG("ptn:%lld size:%lld sz:%d\n", ptn, size, sz);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, i, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->block_size) > size) {
            ERROR("boot image too large!\n");
            return ERR_IMAGE_TOO_LARGE;
        }

        else if (dl_disk_write(ds->disk_inst[i], ptn, (uint8_t *)data, round_up(sz,
                               ds->block_size) )) {
            ERROR("write emmc error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check) {
            err = md5_read_check(ds, i, data, sz, ptn);

            if (ERR_NONE != err)
                return err;
        }

        return ERR_NONE;
    }
}

/**
 * @brief patch sfs information
 * @param ds download disk states
 * @param buffer data buffer
 * @param len size of data
 * @return true
 * @return false
 */
static void patch_sfs_crc(DL_STATE_T *ds, uint8_t *buffer, uint32_t len)
{
    struct sfs sfs = {0};
    uint32_t crc_val = 0;

    if (get_sfs_info(&sfs, buffer, len)) {
        ERROR("there is no available sfs in nor flash\n");
    }

    crc_val = sfs_crc32(0, buffer, SFS_SIZE - 4);
    PUT_LONG(&buffer[SFS_CRC32_OFFSET], crc_val);
}

/**
 * @brief patch sfs information
 * @param ds download disk states
 * @param buffer data buffer
 * @param len size of data
 * @return true
 * @return false
 */
static bool patch_sfs(DL_STATE_T *ds, uint8_t *buffer, uint32_t len)
{
    struct   partition_device *ptdev    =  ds->ptdev;
    int i = 0;
    bool patched = false;
    struct sfs sfs = {0};
    uint32_t crc_val = 0;
    uint64_t boot_1st_ptn = 0;
    uint64_t boot_2nd_ptn = 0;
    uint64_t boot_3rd_ptn = 0;
    uint32_t boot_1st_sfs_base = 0;
    uint32_t boot_2nd_sfs_base = 0;
    uint32_t boot_3rd_sfs_base = 0;
    const char *partition1[2] = {BOOT_PARTITION_NAME0, "boot"};
    const char *partition2[2] = {BOOT_PARTITION_NAME1, "boot_back"};
    const char *partition3[2] = {BOOT_PARTITION_NAME2, "boot_third"};

    if (len < SFS_SIZE || !ptdev) {
        return false;
    }

    for (i = 0; i < 2; i++) {
        boot_1st_ptn = ptdev_get_offset(ptdev, partition1[i]);
        boot_2nd_ptn = ptdev_get_offset(ptdev, partition2[i]);
        boot_3rd_ptn = ptdev_get_offset(ptdev, partition3[i]);

        if ((!boot_1st_ptn || boot_1st_ptn >= UINT32_MAX)
                && (!boot_2nd_ptn || boot_2nd_ptn >= UINT32_MAX)
                && (!boot_3rd_ptn || boot_3rd_ptn >= UINT32_MAX)) {
            if (1 == i) {
                DBG("there is no boot partiton in nor flash's partition\n");
                return false;
            }

            continue;
        }
        else
            break;
    }

    if (boot_1st_ptn >= UINT32_MAX)
        boot_1st_ptn = 0;

    if (boot_2nd_ptn >= UINT32_MAX)
        boot_2nd_ptn = 0;

    if (boot_3rd_ptn >= UINT32_MAX)
        boot_3rd_ptn = 0;

    DBG("boot_1st_ptn = %llx\n", boot_1st_ptn);
    DBG("boot_2nd_ptn = %llx\n", boot_2nd_ptn);
    DBG("boot_3rd_ptn = %llx\n", boot_3rd_ptn);

    if (get_sfs_info(&sfs, buffer, len)) {
        ERROR("there is no available sfs in nor flash\n");
        return false;
    }

    boot_1st_sfs_base = GET_LWORD_FROM_BYTE(&buffer[SFS_NIA_OFFSET]);
    boot_2nd_sfs_base = GET_LWORD_FROM_BYTE(&buffer[SFS_BIA_OFFSET]);
    boot_3rd_sfs_base = GET_LWORD_FROM_BYTE(&buffer[SFS_TIA_OFFSET]);

    DBG("1st boot base sfs:0x%08x, partition:0x%08x\n", boot_1st_sfs_base,
        (uint32_t)boot_1st_ptn);
    DBG("2nd boot base sfs:0x%08x, partition:0x%08x\n", boot_2nd_sfs_base,
        (uint32_t)boot_2nd_ptn);
    DBG("3rd boot base sfs:0x%08x, partition:0x%08x\n", boot_3rd_sfs_base,
        (uint32_t)boot_3rd_ptn);

    if (boot_1st_sfs_base != (uint32_t)boot_1st_ptn) {
        PUT_LONG(&buffer[SFS_NIA_OFFSET], (uint32_t)boot_1st_ptn);
        DBG("sfs 1st image base update,original base:0x%08x -> base:0x%08x\n",
            boot_1st_sfs_base, (uint32_t)boot_1st_ptn);
        patched = true;
    }

    if (boot_2nd_sfs_base != (uint32_t)boot_2nd_ptn) {
        PUT_LONG(&buffer[SFS_BIA_OFFSET], (uint32_t)boot_2nd_ptn);
        DBG("sfs 2nd image base update, original base:0x%08x -> base:0x%08x\n",
            boot_2nd_sfs_base, (uint32_t) boot_2nd_ptn);
        patched = true;
    }

    if (boot_3rd_sfs_base != (uint32_t)boot_3rd_ptn) {
        PUT_LONG(&buffer[SFS_TIA_OFFSET], (uint32_t)boot_3rd_ptn);
        DBG("sfs 3rd try image base update, original base:0x%08x -> base:0x%08x\n",
            boot_3rd_sfs_base, (uint32_t) boot_3rd_ptn);
        patched = true;
    }

    if (patched) {
        crc_val = sfs_crc32(0, buffer, SFS_SIZE - 4);
        PUT_LONG(&buffer[SFS_CRC32_OFFSET], crc_val);
    }

    return patched;
}

/**
 * @brief flash Nor flash sfs areas
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_sfs_areas(DL_STATE_T *ds, void *data, unsigned sz,
                                   uint8_t sparse_data)
{
    uint64_t ptn    = 0;

    DBG("Flashing ospi's sfs img\n");

    if (ds->partiton_type != TYPE_SAFETY_SFS_PT) {
        ERROR("pttype:%d error!\n", ds->partiton_type);
        return ERR_PT_NOT_FOUND;
    }

#if PTACH_SFS_CRC_WHEN_SFS_DOWNLOADING
    patch_sfs_crc(ds, data, sz);
    md5(data, sz, md5_received);
#endif

#if PTACH_SFS_WHEN_SFS_DOWNLOADING
    bool     patched = false;
    patched = patch_sfs(ds, data, sz);
    md5(data, sz, md5_received);
#endif

    DBG("ptn:%lld size:%d erase grp:%d\n", ptn, sz, ds->erase_size);

    if (SFS_RFD_RFU_SIZE > CONFIG_DL_SCRATCH_SIZE || (ptn + sz) > SFS_RFD_RFU_SIZE
            || ptn > SFS_RFD_RFU_SIZE || sz > SFS_RFD_RFU_SIZE) {
        ERROR("rfd size error\n");
        return ERR_PT_FLASH_FAIL;
    }

#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (dl_disk_read(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                     round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    if (dl_disk_erase(ds->disk_inst[0], 0, round_up(SFS_RFD_RFU_SIZE,
                      ds->erase_size))) {
        ERROR("erase partition fail\n");
        return ERR_PT_ERASE_FAIL;
    }

    memcpy((uint8_t *)dl_scratch_base + ptn, data, sz);

    if (dl_disk_write(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                      round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

#else

    if (dl_disk_write(ds->disk_inst[0], ptn, data, sz)) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

#endif
    read_sfs(ds);

    if (do_md5_rb_check)
        return md5_read_check(ds, 0, data, sz, ptn);

    return ERR_NONE;
}

/**
 * @brief flash Nor flash rfd areas
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_rfd_areas(DL_STATE_T *ds, void *data, unsigned sz,
                                   uint8_t sparse_data)
{
    uint64_t ptn    = RFD_OFFSET;

    DBG("Flashing ospi's sfs img\n");

    if (ds->partiton_type != TYPE_SAFETY_RFD_PT) {
        ERROR("pttype:%d error!\n", ds->partiton_type);
        return ERR_PT_NOT_FOUND;
    }

    DBG("ptn:%lld size:%d erase grp:%d\n", ptn, sz, ds->erase_size);

    if (SFS_RFD_RFU_SIZE > CONFIG_DL_SCRATCH_SIZE || (ptn + sz) > SFS_RFD_RFU_SIZE
            || ptn > SFS_RFD_RFU_SIZE || sz > SFS_RFD_RFU_SIZE) {
        ERROR("rfd size error\n");
        return ERR_PT_FLASH_FAIL;
    }

#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (dl_disk_read(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                     round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    if (dl_disk_erase(ds->disk_inst[0], 0, round_up(SFS_RFD_RFU_SIZE,
                      ds->erase_size))) {
        ERROR("erase partition fail\n");
        return ERR_PT_ERASE_FAIL;
    }

    memcpy((uint8_t *)dl_scratch_base + ptn, data, sz);

    if (dl_disk_write(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                      round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    md5((uint8_t *)dl_scratch_base, SFS_RFD_RFU_SIZE, md5_received);

    if (do_md5_rb_check)
        return md5_read_check(ds, 0, (uint8_t *)dl_scratch_base, SFS_RFD_RFU_SIZE, 0);

#else

    if (dl_disk_write(ds->disk_inst[0], ptn, data, sz)) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    if (do_md5_rb_check)
        return md5_read_check(ds, 0, data, sz, ptn);

#endif

    return ERR_NONE;
}

/**
 * @brief update sfs information according by parititon
 * @param ds download disk states
 * @return uint32_t 0 is success, 1 is fail
 */
static uint32_t update_sfs(DL_STATE_T *ds)
{
    bool need_update = false;
    uint8_t *buffer = NULL;
    uint32_t ret = 0;
    uint32_t sfs_size = 0;

    if (ds->disk_type != NORFLASH) {
        return 1;
    }

    /* sfs must have an exclusive sector */
    sfs_size = round_up(SFS_SIZE, ds->block_size);
    buffer = (uint8_t *)osAllocAlign(ds->block_size, sfs_size);

    if (!buffer) {
        ERROR("allocate memory fail\n");
        return 1;
    }

    memset(buffer, 0x0, sfs_size);

    if (dl_disk_read(ds->disk_inst[0], 0, buffer, sfs_size)) {
        ERROR("read nor flash fail\n");
        ret = 1;
        goto end;
    }

    need_update = patch_sfs(ds, buffer, sfs_size);

    if (!need_update)
        goto end;

    if (dl_disk_erase(ds->disk_inst[0], 0, round_up(sfs_size, ds->erase_size))) {
        ERROR("erase sfs fail\n");
        ret = 1;
        goto end;
    }

    if (dl_disk_write(ds->disk_inst[0], 0, buffer, sfs_size)) {
        ERROR("write sfs fail\n");
        ret = 1;
        goto end;
    }

    ret = 0;
end:

    if (buffer)
        osFree(buffer);

    return ret;
}

/**
 * @brief flash Nor flash Boot Package
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @param sz pack num
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_pack(DL_STATE_T *ds, void *data, unsigned sz,
                              uint8_t sparse_data, uint8_t pack_num)
{
    uint64_t ptn    = 0;
    uint64_t size    = 0;

    DBG("Flashing norflash's boot%d package img\n", pack_num);

    ptn = ptdev_get_offset(ds->ptdev, boot_package[pack_num]);

    if (ptn != 0) {
        size = ptdev_get_size(ds->ptdev, boot_package[pack_num]);
    }
    else {
        ptn = sfs_get_image_base(ds, pack_num);

        if (ptn != 0)
            size = sfs_get_image_size_limit(ds, pack_num);
        else {
            ERROR("boot%d image base error, ptn = %lld!\n", pack_num, ptn);
            return ERR_PT_BASE_ERROR;
        }
    }

    if (ptn == 0 || ptn > ds->capacity) {
        ERROR("ptn size alignment error  = %llx\n", ptn);
        return ERR_PT_NOT_FOUND;
    }

    if (size < ds->block_size  || (size > ds->capacity)) {
        ERROR("size error!\n");
        return ERR_PT_NOT_FOUND;
    }

    if ((ptn % ds->block_size) || (ptn % ds->erase_size)) {
        ERROR("ptn alignment error  = %llx\n", ptn);
        return ERR_PT_BASE_ERROR;
    }

    if ((size % ds->block_size) || (size % ds->erase_size)) {
        ERROR("size alignment error  = %llx\n", size);
        return ERR_PT_BASE_ERROR;
    }

    DBG("ptn:%lld size:%lld erase grp:%d\n", ptn, size, ds->erase_size);

    if (sparse_data) {
        DBG("sparce donwload mode\n");
        return flash_sparse_img(ds, data, sz, 0, ptn, size);
    }
    else {
        DBG("normal donwload mode\n");

        if (round_up(sz, ds->erase_size) > size) {
            ERROR(" image too large:%llu!\n", round_up(sz, ds->block_size));
            return ERR_IMAGE_TOO_LARGE;
        }

#if CONFIG_DLOADER_WITH_TRACE
        ptn += cmd_args->offset;
        DBG("ptn + cmd_args->offset is 0x%llx\n", ptn);

        if (cmd_args->total_sz > size) {
            ERROR("trace image total size too large:%llu!\n", cmd_args->total_sz);
            return ERR_IMAGE_TOO_LARGE;
        }

        if (ptn % ds->block_size) {
            ERROR("ptn alignment error in trace mode  = %llx\n", ptn);
            return ERR_PT_BASE_ERROR;
        }
#endif

#if CONFIG_DLOADER_BLOCK_IO_MODE
#if CONFIG_DLOADER_WITH_TRACE
        if ((ds->disk_type == NORFLASH) && (cmd_args->offset == 0)) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(cmd_args->total_sz,
                              ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }
#else
        if (ds->disk_type == NORFLASH) {
            if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(sz, ds->erase_size))) {
                ERROR("erase partition fail\n");
                return ERR_PT_ERASE_FAIL;
            }
        }
#endif
#endif

        if (dl_disk_write(ds->disk_inst[0], ptn, data, round_up(sz, ds->block_size))) {
            ERROR(" write data error!\n");
            return ERR_PT_FLASH_FAIL;
        }

        if (do_md5_rb_check)
            return md5_read_check(ds, 0, data, sz, ptn);

        return ERR_NONE;
    }
}

/**
 * @brief flash Nor flash Boot Package Normal
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_pack0(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    return flash_ospi_pack(ds, data, sz, sparse_data, BOOT0_NUM);
}

/**
 * @brief flash Nor flash Boot Package Backup
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_pack1(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    return flash_ospi_pack(ds, data, sz, sparse_data, BOOT1_NUM);
}

/**
 * @brief flash Nor flash Boot Package Third
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_ospi_pack2(DL_STATE_T *ds, void *data, unsigned sz,
                               uint8_t sparse_data)
{
    return flash_ospi_pack(ds, data, sz, sparse_data, BOOT2_NUM);
}

/**
 * @brief flash the GUID parition table for disk
 * @param ds download disk states
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E flash_gpt_table(DL_STATE_T *ds, void *data, unsigned sz,
                              uint8_t sparse_data)
{
    uint32_t ret                      = PTB_CHECK_ERROR;
    char *ptname                      = ds->ptname;
    char *sub_ptbname                 = ds->sub_ptbname;
    PARTITION_TYPE_E pttype           = ds->partiton_type;
    struct partition_device *ptdev    = ds->ptdev;
    bool last_part_extend             = true;
    uint64_t gpt_sz                   = 0;

    DBG("Flashing gpt table img\n");

    if (ds->disk_type == NORFLASH) {
        last_part_extend = false;
    }

    ret = check_partition_table(ds, data, sz);

    DBG("check_partition_table ret:%d\n", ret);

    if (ret == PTB_CHECK_ERROR) {
        ERROR("check error ret:%u !\n", ret);

        if (strlen(sub_ptbname))
            return ERR_SUB_PTB_NOT_MATCH;
        else
            return ERR_PRI_PTB_NOT_MATCH;
    }
    else if (ret == PTB_NO_NEED_FLASH) {
        DBG("partition table doesn't need to flash\n");
        goto end;
    }

    /* This is primary GPT table. To write primary ptb, set sub_ptbname = NULL */
    if (pttype == TYPE_PRI_PTB)
        sub_ptbname = NULL;

    DBG("Attempt to write primary/sub partition:%s \n", ptname);
    DBG("ptb_offset:0x%llx\n", ds->ptb_offset);

    /* If flashing ospi nor flash, it may re-calculate ptb offset and reset ptdev, so get ptdev pointer here */
    ptdev = ds->ptdev;

    if (ptdev && ds->disk_type == NORFLASH ) {
        gpt_sz = GET_PRI_GPT_SIZE(ds->block_size);

        /*
         * For nor flash, it needs to erase storage before writting data to it.
         * To reduce time consumption, it only erases primary gpt header and backup gpt header.
         * Note, the addr and the length are must be aligned to erase group size.
         * */
        dl_disk_erase(ds->disk_inst[0], ds->ptb_offset, round_up(gpt_sz,
                      ds->erase_size));
        dl_disk_erase(ds->disk_inst[0],
                      round_down(ds->capacity - gpt_sz + ds->block_size, ds->erase_size),
                      round_up(gpt_sz - ds->block_size, ds->erase_size));

        DBG("ptb_offset:0x%0llx gpt_sz:%llu block size:%d capacity:0x%0llx \n",
            ds->ptb_offset, gpt_sz, ds->block_size, ds->capacity);
    }

    if (!ptdev
            || ptdev_write_table(ptdev, sub_ptbname, sz, (unsigned char *) data,
                                 last_part_extend)) {
        ERROR("write gpt table error pt:%s ptb_offset:0x%llx\n", ptname,
              ds->ptb_offset);
        return ERR_PT_FLASH_FAIL;
    }

end:

    if (ds->disk_type == NORFLASH) {
        DBG("update sfs\n");
        ret = update_sfs(ds);

        if (ret) {
            ERROR("updata sfs fail\n");
            return ERR_PT_FLASH_FAIL;
        }
    }

    return ERR_NONE;
}

static bool parse_direct_add_name(const char *sub_ptbname, const char *ptname)
{
    if ((strlen(ptname) <= strlen(ADD_DIR_FUZZY_NAME)) || (strlen(sub_ptbname)))
        return 0;

    return !strncmp(ptname, ADD_DIR_FUZZY_NAME, strlen(ADD_DIR_FUZZY_NAME));
}

/**
 * @brief get the partition type
 * @param sub_ptbname
 * @param ptname
 * @return PARTITION_TYPE_E partition type
 */
static PARTITION_TYPE_E parse_pt_type(const char *sub_ptbname,
                                      const char *ptname)
{
    uint8_t i = 0;
    bool parse_sub_pt  = strlen(sub_ptbname) ? 1 : 0;
    bool parse_pt      = strlen(ptname) ?  1 : 0;

    /* sub partition table name and ptname can not be null simultaneously */
    if (!parse_sub_pt && !parse_pt) {
        ERROR("partition is TYPE_PT_UNKNOWN\n");
        return TYPE_PT_UNKNOWN;
    }

    /* get the partition type */
    for (i = 0; i < DL_TYPE_TB_LEN; i++) {
        if (parse_sub_pt ^ dl_type_table[i].is_sub_pt)
            continue;

        if (!strcmp(ptname, dl_type_table[i].ptname)) {
            DBG("get partiton type = %s\n", dl_type_table[i].type_name);
            return dl_type_table[i].partiton_type;
        }
    }

    if (parse_direct_add_name(sub_ptbname, ptname)) {
        DBG("get partiton type = adrress direct\n");
        return TYPE_ADD_DIRECT;
    }
    else if (!parse_sub_pt) {
        DBG("get partiton type = primary parition\n");
        return TYPE_PRI_PT;
    }
    else if (!parse_pt) {
        DBG("get partiton type = sub parition all\n");
        return TYPE_SUB_PT_WHOLE;
    }
    else {
        DBG("get partiton type = sub parition\n");
        return TYPE_SUB_PT;
    }

    ERROR("partition type is unknow\n");
    return TYPE_PT_UNKNOWN;
}

/**
 * @brief close disk
 * @param download disk states
 * @return uint32_t 0 is success, 1 is failed
 */
static bool close_disk(DL_STATE_T *ds)
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
 * @param download disk states
 * @return bool 0 is success, 1 is failed
 */
static bool open_disk(DL_STATE_T *ds)
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
 * @brief parse the target information that is from fastboot command
 * @param full_ptname target information
 * @return DL_STATE_T* download disk states
 */
DL_STATE_T *parse_partition_name(const char *full_ptname, DL_ERR_CODE_E *err)
{
    char *token1 = NULL;
    char *token2 = NULL;
    uint32_t token_num   = 0;
    uint32_t full_len    = 0;
    uint32_t sub_ptb_len = 0;
    uint32_t pt_len      = 0;
    char disk_name[MAX_GPT_NAME_SIZE + 1]  = {0};
    char sub_ptbname[MAX_GPT_NAME_SIZE + 1]   = {0};
    char short_ptname[MAX_GPT_NAME_SIZE + 1]  = {0};
    *err = ERR_NONE;

    DL_STATE_T *ds = NULL;
    DBG("-------------------Parsing name------------------------\n");

    if (!full_ptname || (full_len = strlen(full_ptname)) == 0) {
        ERROR("can not get the partition name for download\n");
        *err = ERR_PTNAME_NOT_EXIT;
        goto fail;
    }

    for (uint32_t i = 0; i < full_len; i++) {
        if (full_ptname[i] == '$') {
            token_num++;
        }
    }

    /* full name:xxx$yyy$zzz */
    if (token_num != 2) {
        ERROR("partition name format error, should be xxx$$zzz\n");
        *err = ERR_PT_FULL_NAME_FORMAT;
        goto fail;
    }

    token1 = strchr(full_ptname, '$');
    token2 = strrchr(full_ptname, '$');

    strncpy(disk_name, full_ptname, token1 - full_ptname);
    strncpy(sub_ptbname, token1 + 1, token2 - token1 - 1);
    strncpy(short_ptname, token2 + 1, full_len - (token2 + 1 - full_ptname));

    DBG("disk_name:%s sub_ptb:%s sub_pt:%s\n", disk_name, sub_ptbname,
        short_ptname);

    for (uint32_t i = 0; i < sizeof(dl_state) / sizeof(dl_state[0]); i++) {
        if (!strcmp(dl_state[i].download_name, disk_name)) {
            DBG("ds = dl_state[%d]\n", i);
            ds = &dl_state[i];
            break;
        }
    }

    if (ds == NULL) {
        ERROR("can't find a matched disk for download\n");
        *err = ERR_DISK_NOT_EXIT;
        goto fail;
    }

    if (open_disk(ds)) {
        ERROR("open the download disk failed\n");
        *err = ERR_DISK_OPEN_ERROR;
        goto fail;
    }

    disk_set_block_size(ds->disk_inst[0], CONFIG_DLOADER_DISK_BLOCK_SIZE);
    ds->block_size = disk_get_block_size(ds->disk_inst[0]);
    ds->erase_size = disk_erase_size(ds->disk_inst[0]);
    ds->capacity   = disk_size(ds->disk_inst[0]);

#if CONFIG_DLODER_USE_RAM_AS_DISK

    if (!strcmp(ds->disk_name, DISK_RAM_NAME(0))) {
        ds->erase_size = 4096;
    }

    if (!strcmp(ds->disk_name, DISK_RAM_NAME(1))) {
        ds->erase_size = 524288;
    }

#endif

    ASSERT(ds->block_size != 0);
    ASSERT(ds->erase_size != 0);
    ASSERT(ds->block_size == CONFIG_DLOADER_DISK_BLOCK_SIZE);
    ASSERT(ds->erase_size % ds->block_size == 0);
    ASSERT(ds->capacity % ds->block_size == 0);
    ASSERT(ds->capacity > ds->block_size);
    ASSERT(ds->capacity > ds->erase_size);
    ASSERT(ds->capacity > ds->ptb_offset);
    ASSERT(ds->capacity > (GET_PRI_GPT_SIZE(ds->block_size)));

    if (ds->disk_type == NORFLASH) {
        ds->ptb_offset = ds->erase_size * NOR_FLASH_PTB_SECTOR_INDEX;
    }
    else {
        ds->ptb_offset = 0;
    }

    if (!(ds->ptdev) && ds->ptb_offset != INVALID_PTB_OFFSET) {
        ds->ptdev = ptdev_setup(ds->disk_inst[0], ds->ptb_offset);

        if (ds->ptdev) {
            DBG("read partition table\n");
            ptdev_read_table(ds->ptdev);
        }
        else {
            ERROR("get ptdev:%s error\n", disk_name);
            *err = ERR_PARTITION_READ_ERROR;
            goto closedisk;
        }
    }

    ds->partiton_type = parse_pt_type(sub_ptbname, short_ptname);

    if (ds->partiton_type == TYPE_PT_UNKNOWN) {
        goto closedisk;
    }

    pt_len = strlen(short_ptname);
    sub_ptb_len = strlen(sub_ptbname);
    memset(ds->sub_ptbname, 0x0, sizeof(ds->sub_ptbname));
    memset(ds->ptname, 0x0, sizeof(ds->ptname));
    strncpy(ds->sub_ptbname, sub_ptbname,  sub_ptb_len);
    strncpy(ds->ptname, short_ptname, pt_len);

    DBG("download_name   = %s\n",     ds->download_name);
    DBG("disk_name       = %s\n",     ds->disk_name);
    DBG("disk_boot1_name = %s\n",     ds->disk_boot1_name);
    DBG("disk_boot2_name = %s\n",     ds->disk_boot2_name);
    DBG("disk_type       = %d\n",     ds->disk_type);
    DBG("block_size      = %d\n",     ds->block_size);
    DBG("erase_size      = %d\n",     ds->erase_size);
    DBG("capacity        = %lld\n",   ds->capacity);
    DBG("ptb_offset      = %lld\n",   ds->ptb_offset);
    DBG("partiton_type   = %d\n",     ds->partiton_type);
    DBG("ptname          = %s\n",     ds->ptname);
    DBG("sub_ptbname     = %s\n",     ds->sub_ptbname);
    DBG("boot_offset     = %lld\n",   ds->boot_offset);

    return ds;

closedisk:
    close_disk(ds);

fail:
    return NULL;
}

/**
 * @brief dloader flash comman prosess
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E dl_cmd_flash(const char *arg, void *data, unsigned sz)
{
    DL_ERR_CODE_E err = ERR_NONE;
    static uint32_t flash_cnt = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    uint8_t i = 0;
    PARTITION_TYPE_E pttype = TYPE_PT_UNKNOWN;
    struct command_table *downloader = NULL;
    sparse_header_t *sparse_header = NULL;
    sparse_header = (sparse_header_t *) data;
    DL_ERR_CODE_E (*flash_cmd)(DL_STATE_T * ds, void *data, unsigned sz,
                               uint8_t sparse_data) = NULL;
    DL_STATE_T *ds = NULL;
#if CONFIG_DLOADER_PROG_HYPER_FLASH_FUSE
    static uint32_t fuse_once = 0;
    uint32_t val = 0;
#endif

    /* get flash name */
    ds = parse_partition_name(arg, &err);

    if (!ds || err != ERR_NONE) {
        ERROR("partition name error\n");
        goto end;
    }

    ASSERT(ds);
    DBG("------------------flash_cnt = %d-------------------\n", flash_cnt++);

    /* check recived data's md5 */
    if (do_md5_rcv_check) {
        md5(data, sz, md5_calc);

        if (memcmp(md5_received, md5_calc, MD5_LEN)) {
            ERROR("md5 check fail!\n");
            hexdump(md5_received, MD5_LEN);
            hexdump(md5_calc, MD5_LEN);
            goto closedisk;
        }

        DBG("calc md5 check ok!\n");
    }

    /* do flashing work */
    for (i = 0; i < DL_CMD_TB_LEN; i++) {
        if (ds->partiton_type == dl_cmd_table[i].partiton_type) {
            if (ds->disk_type == dl_cmd_table[i].disk_type
                    || dl_cmd_table[i].disk_type == ALLDISK) {
                downloader = &dl_cmd_table[i];
                DBG("find download cmd, i = %d\n", i);
                break;
            }
        }
    }

    if (!downloader) {
        ERROR("can not find command in cmd_table\n");
        goto closedisk;
    }

    flash_cmd = downloader->flash_callback;

    if (flash_cmd) {
        DBG("flash_cmd function = %s\n", downloader->download_cmd_name);

        if (sparse_header->magic != SPARSE_HEADER_MAGIC)
            err = flash_cmd(ds, data, sz, 0);
        else
            err = flash_cmd(ds, data, sz, 1);

        if (err) {
            ERROR("flash error\n");
            goto closedisk;
        }
    }
    else {
        ERROR("flash_cmd is NULL\n");
        goto closedisk;
    }

    /* If the image is whole sub partition, need update partition table here */
    if  (pttype == TYPE_SUB_PT_WHOLE) {
        ptdev_read_table(ds->ptdev);
    }

#if CONFIG_DLOADER_PROG_HYPER_FLASH_FUSE

    /* If the disk is hyper flash, program the hyper flash fuse bit */
    if (ds->disk_type == NORFLASH && fuse_once == 0 && err == ERR_NONE) {
        fuse_once = 1;

        if (0 != fuse_sense(HYPER_FLASH_MODE_FUSE_INDEX, &val)) {
            ERROR("Failed to read hyper flash fuse %d\n", HYPER_FLASH_MODE_FUSE_INDEX);
            err = HYPER_FLASH_FUSE_READ_ERROR;
            goto closedisk;
        }

        DBG("read hyper flash fuse %d val 0x%08x\n", HYPER_FLASH_MODE_FUSE_INDEX, val);

        if ((val & HYPER_FLASH_MODE_FUSE_VAL) == 0) {
            val |= HYPER_FLASH_MODE_FUSE_VAL;
            DBG("write hyper flash fuse %d val 0x%08x\n", HYPER_FLASH_MODE_FUSE_INDEX, val);

            if (0 != fuse_program(HYPER_FLASH_MODE_FUSE_INDEX, val)) {
                ERROR("Failed to write hyper flash fuse %d val 0x%08x\n",
                      HYPER_FLASH_MODE_FUSE_INDEX, val);
                err = HYPER_FLASH_FUSE_WRITE_ERROR;
                goto closedisk;
            }

            if (0 != fuse_sense(HYPER_FLASH_MODE_FUSE_INDEX, &val)) {
                ERROR("Failed to read hyper flash fuse %d\n", HYPER_FLASH_MODE_FUSE_INDEX);
                err = HYPER_FLASH_FUSE_READ_ERROR;
                goto closedisk;
            }

            DBG("read back hyper flash fuse %d val 0x%08x\n", HYPER_FLASH_MODE_FUSE_INDEX,
                val);

            if ((val & HYPER_FLASH_MODE_FUSE_VAL) != HYPER_FLASH_MODE_FUSE_VAL) {
                ERROR("read back error, hyper flash fuse %d val 0x%08x\n",
                      HYPER_FLASH_MODE_FUSE_INDEX, val);
                err = HYPER_FLASH_FUSE_VAL_ERROR;
                goto closedisk;
            }
        }
        else {
            DBG("no need to fuse\n");
        }
    }

#endif

closedisk:
    close_disk(ds);

end:
    return err;
}

/**
 * @brief erase emmc's ont boot area by the part number
 * @param ds download disk states
 * @param inst_num emmc dev number
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_emmc_one_boot_area(DL_STATE_T *ds, uint8_t inst_num)
{
    uint64_t ptn  = 0;
    uint64_t size = 0;
    struct disk_dev *dev = NULL;

    ASSERT(inst_num < EMMC_DEV_INST_MAX);
    ASSERT(ds->disk_inst[inst_num]);
    dev  = ds->disk_inst[inst_num];
    size = EMMC_BOOT_PT_SIZE;
    ptn  = ds->boot_offset;

    /* for sdcard and emmc'user areas, the boot offset is 20k */
    if (inst_num == EMMC_USER_SPACE_DEV_INST) {
        ptn = ptdev_get_offset(ds->ptdev, boot_package[BOOT2_NUM]);

        if (ptn == SD_SPL_OFFSET) {
            size = ptdev_get_size(ds->ptdev, boot_package[BOOT2_NUM]);
        }
        else {
            ptn = SD_SPL_OFFSET;
            size = BPT_SIZE;
        }

        if (size < ds->block_size) {
            ERROR("image size error, size = %lld!\n", size);
            return ERR_PT_SIZE_ERROR;
        }
    }

    DBG("erase emmc inst_num = %d\n", inst_num);
    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (dl_disk_erase(dev, ptn, round_up(size, ds->block_size))) {
        ERROR("erase data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    return ERR_NONE;
}

/**
 * @brief erase emmc's boot1 space
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_emmc_pack0(DL_STATE_T *ds)
{
    enum dl_err_code err = ERR_UNKNOWN;

    DBG("erase_emmc_pack0\n");

    /* The current storage is emmc card */
    if (!ds->boot_offset) {

        err = erase_emmc_one_boot_area(ds, EMMC_BOOT1_DEV_INST);

        if (err) {
            ERROR("erase_emmc_pack0 error!\n");
            return err;
        }
    }

    /* The current storage is sd card */
    else {
        DBG("sd card do not have pack0\n");
    }

    return ERR_NONE;
}

/**
 * @brief erase emmc's boot2 space
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_emmc_pack1(DL_STATE_T *ds)
{
    enum dl_err_code err = ERR_UNKNOWN;

    DBG("erase_emmc_pack1\n");

    /* The current storage is emmc card */
    if (!ds->boot_offset) {
        err = erase_emmc_one_boot_area(ds, EMMC_BOOT2_DEV_INST);

        if (err) {
            ERROR("erase_emmc_pack1 error!\n");
            return err;
        }
    }

    /* The current storage is sd card */
    else {
        DBG("sd card do not have pack1\n");
    }

    return ERR_NONE;
}

/**
 * @brief erase the 20k byte offset of emmc's user space
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_emmc_pack2(DL_STATE_T *ds)
{
    enum dl_err_code err = ERR_UNKNOWN;

    DBG("erase_emmc_pack2\n");

    /* The current storage is emmc card */
    err = erase_emmc_one_boot_area(ds, EMMC_USER_SPACE_DEV_INST);

    if (err) {
        ERROR("erase_emmc_pack2 error!\n");
        return err;
    }

    return ERR_NONE;
}

/**
 * @brief erase the Nor flash sfs areas
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_sfs_areas(DL_STATE_T *ds)
{
    uint64_t ptn     = 0;
    uint64_t size    = 0;
    PARTITION_TYPE_E  pttype = ds->partiton_type;

    DBG("erase sfs\n");

    if (pttype != TYPE_SAFETY_SFS_PT) {
        ERROR("pttype:%d error!\n", pttype);
        return ERR_PT_NOT_FOUND;
    }

    ptn = 0;
    size = SFS_SIZE;

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (dl_disk_read(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                     round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    if (dl_disk_erase(ds->disk_inst[0], 0, round_up(SFS_RFD_RFU_SIZE,
                      ds->erase_size))) {
        ERROR("erase partition fail\n");
        return ERR_PT_ERASE_FAIL;
    }

    memset((uint8_t *)dl_scratch_base + ptn, 0xFF, SFS_SIZE);

    if (dl_disk_write(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                      round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

#else

    if (dl_disk_erase(ds->disk_inst[0], ptn, size)) {
        ERROR(" erase data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

#endif
    return 0;
}

/**
 * @brief erase the Nor flash rfd areas
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_rfd_areas(DL_STATE_T *ds)
{
    uint64_t ptn     = RFD_OFFSET;
    uint64_t size    = RFD_SIZE;
    PARTITION_TYPE_E  pttype = ds->partiton_type;

    DBG("erase rdf\n");

    if (pttype != TYPE_SAFETY_RFD_PT) {
        ERROR("pttype:%d error!\n", pttype);
        return ERR_PT_NOT_FOUND;
    }

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

#if CONFIG_DLOADER_BLOCK_IO_MODE

    if (dl_disk_read(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                     round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

    if (dl_disk_erase(ds->disk_inst[0], 0, round_up(SFS_RFD_RFU_SIZE,
                      ds->erase_size))) {
        ERROR("erase partition fail\n");
        return ERR_PT_ERASE_FAIL;
    }

    memset((uint8_t *)dl_scratch_base + ptn, 0xFF, RFD_SIZE);

    if (dl_disk_write(ds->disk_inst[0], 0, (uint8_t *)dl_scratch_base,
                      round_up(SFS_RFD_RFU_SIZE, ds->block_size))) {
        ERROR(" write data error!\n");
        return ERR_PT_FLASH_FAIL;
    }

#else

    if (dl_disk_erase(ds->disk_inst[0], ptn, size)) {
        ERROR(" erase data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

#endif
    return 0;
}

/**
 * @brief erase the Nor flash Boot package areas
 * @param ds download disk states
 * @param pack_num package number
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_pack(DL_STATE_T *ds, uint8_t pack_num)
{
    uint64_t ptn     = 0;
    uint64_t size    = 0;

    DBG("erase ospi_pack %d\n", pack_num);

    ptn = ptdev_get_offset(ds->ptdev, boot_package[pack_num]);

    if (ptn != 0) {
        size = ptdev_get_size(ds->ptdev, boot_package[pack_num]);
    }
    else {
        ptn = sfs_get_image_base(ds, pack_num);

        if (ptn != 0)
            size = BPT_SIZE;
        else {
            ERROR("image base error, ptn = %lld!\n", ptn);
            return ERR_PT_BASE_ERROR;
        }
    }

    if (!size) {
        ERROR("image size error, size = %lld!\n", size);
        return ERR_PT_SIZE_ERROR;
    }

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (ptn % ds->erase_size || size % ds->erase_size ) {
        ERROR("the size of the partition in nor flash is not aligned to erase group size\n");
        return ERR_PT_OVERLAP;
    }

    if (dl_disk_erase(ds->disk_inst[0], ptn, round_up(size, ds->erase_size))) {
        ERROR(" erase data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    return 0;
}

/**
 * @brief erase the Nor flash Firtst Boot package areas
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_pack0(DL_STATE_T *ds)
{
    return erase_ospi_pack(ds, BOOT0_NUM);
}

/**
 * @brief erase the Nor flash Second Boot package areas
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_pack1(DL_STATE_T *ds)
{
    return erase_ospi_pack(ds, BOOT1_NUM);
}

/**
 * @brief erase the Nor flash Third Boot package areas
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_ospi_pack2(DL_STATE_T *ds)
{
    return erase_ospi_pack(ds, BOOT2_NUM);
}

/**
 * @brief erase the GUID parition table
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_gpt_table(DL_STATE_T *ds)
{
    uint64_t gpt_sz = 0;
    uint64_t capacity = 0;
    const char *ptname = NULL;
    PARTITION_TYPE_E pttype = 0;
    uint64_t ptn_pri = 0;
    uint64_t ptn_bak = 0;
    const char *sub_ptbname = NULL;
    uint64_t size_pri = 0;
    uint64_t size_bak = 0;
    struct partition_device *ptdev = NULL;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};

    pttype  = ds->partiton_type;
    ptname = ds->ptname;
    sub_ptbname = ds->sub_ptbname;
    ptdev = ds->ptdev;
    capacity = ds->capacity;

    DBG("ptb_offset:0x%llx\n", ds->ptb_offset);

    /* MBR is in the first LBA. GPT header is in the second LBA. GPT entries are in the following blocks */
    gpt_sz = GET_PRI_GPT_SIZE(ds->block_size);
    size_pri = gpt_sz;

    if (pttype == TYPE_SUB_PTB) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
        ptn_pri  = ptdev_get_offset(ptdev, full_ptname);
        size_bak = ptdev_get_size(ptdev, full_ptname);
        ptn_bak  = ptn_pri + size_bak - gpt_sz + ds->block_size;
    }
    else {
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
        ptn_pri = ds->ptb_offset;
        ptn_bak = ds->capacity - gpt_sz + ds->block_size;
    }

    /* back up gpt header no mbr block */
    size_bak = gpt_sz - ds->block_size;

    DBG("ptn_pri:0x%llx size_pri:0x%llx  ptn_bak:0x%llx  size_bak:0x%llx cap:0x%llx!\n",
        ptn_pri, size_pri, ptn_bak, size_bak, capacity);

    if (ds->disk_type == NORFLASH) {
        size_pri = round_up(size_pri, ds->erase_size);
        size_bak = round_up(size_bak, ds->erase_size);
        ptn_bak = round_down(ptn_bak, ds->erase_size);

        if (ptn_pri % ds->erase_size != 0) {
            ERROR("nor flash ptn must be aligned to erase group size!\n");
            return ERR_PT_ERASE_FAIL;
        }
    }

    /* erase primary gpt header */
    if (dl_disk_erase(ds->disk_inst[0], ptn_pri, size_pri)) {
        ERROR("erase primary gpt header error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    /* erase back up gpt header */
    if (dl_disk_erase(ds->disk_inst[0], ptn_bak, size_bak)) {
        ERROR(" erase backup gpt header error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    ptdev_read_table(ds->ptdev);

    return ERR_NONE;
}

/**
 * @brief erase a normal partition in PTB
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_addr_direct(DL_STATE_T *ds)
{
    return ERR_NONE;
}

/**
 * @brief erase a normal partition in PTB
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_normal_partition(DL_STATE_T *ds)
{
    const char *ptname  = NULL;
    uint32_t block_size = 0;
    PARTITION_TYPE_E pttype = ds->partiton_type;
    uint64_t ptn  = 0;
    uint64_t size = 0;
    const char *sub_ptbname = NULL;
    struct partition_device *ptdev = NULL;
    char full_ptname[MAX_GPT_NAME_SIZE * 2 + 2] = {0};

    ptdev = ds->ptdev;
    block_size = ds->block_size;

    DBG("ptb_offset:0x%llx\n", ds->ptb_offset);

    /* For ospi nor flash, if ptdev is null,
     * it indicates there is no gpt in nor flash.
     * */
    if (!ptdev) {
        ERROR("ospi flash has no gpt\n");
        return 0;
    }

    ptname = ds->ptname;
    sub_ptbname = ds->sub_ptbname;

    if (pttype == TYPE_SUB_PT_WHOLE) {
        snprintf(full_ptname, sizeof(full_ptname), "%s", sub_ptbname);
    }
    else if (pttype == TYPE_SUB_PT) {
        snprintf(full_ptname, sizeof(full_ptname), "%s$%s", sub_ptbname, ptname);
    }
    else {
        snprintf(full_ptname, sizeof(full_ptname), "%s", ptname);
    }

    ptn = ptdev_get_offset(ptdev, full_ptname);
    size = ptdev_get_size(ptdev, full_ptname);

    if (size < block_size || ptn == 0 || ptn > ds->capacity
            || (size > (ds->capacity - ds->ptb_offset))) {
        ERROR("partition not found ptn:%lld size:%lld! name:%s\n", ptn, size,
              full_ptname);
        return ERR_PT_NOT_FOUND;
    }

    if (ptn % block_size || size % block_size) {
        ERROR("ptn:%lld is not aligned to block size\n", ptn);
        return ERR_PT_OVERLAP;
    }

    if (ds->disk_type == NORFLASH) {
        if (ptn % ds->erase_size != 0 || size % ds->erase_size != 0) {
            ERROR("the size of the partition in nor flash is not aligned to erase group size\n");
            return ERR_PT_OVERLAP;
        }
    }

    DBG("erase ptn:%lld size:%lld!\n", ptn, size);

    if (dl_disk_erase(ds->disk_inst[0], ptn, size)) {
        ERROR(" erase data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    return ERR_NONE;
}

/**
 * @brief erass all the disk
 * @param ds download disk states
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E erase_all(DL_STATE_T *ds)
{
    enum dl_err_code err = ERR_UNKNOWN;

    DBG("all erase\n");

    if (!ds->boot_offset && ds->disk_type == MMC) {

        /* The current storage is emmc */
        err = erase_emmc_one_boot_area(ds, EMMC_BOOT1_DEV_INST);

        if (err) {
            ERROR("erase boot1 error!\n");
            return err;
        }

        err = erase_emmc_one_boot_area(ds, EMMC_BOOT2_DEV_INST);

        if (err) {
            ERROR("erase boot2 error!\n");
            return err;
        }
    }

    if (dl_disk_erase(ds->disk_inst[0], 0, ds->capacity)) {
        ERROR("erase all data error!\n");
        return ERR_PT_ERASE_FAIL;
    }

    return ERR_NONE;
}

/**
 * @brief dloader erase process action
 * @param arg input fastboot argument
 * @param data data buffer
 * @param sz data buffer size
 * @return DL_ERR_CODE_E error code
 */
DL_ERR_CODE_E dl_cmd_erase(const char *arg, void *data, unsigned sz)
{
    static uint32_t erase_cnt = 0;
    uint8_t i = 0;
    DL_ERR_CODE_E err = ERR_NONE;
    struct command_table *downloader = NULL;
    DL_ERR_CODE_E (*erase_cmd)(DL_STATE_T * ds) = NULL;
    DL_STATE_T *ds = NULL;

    ds = parse_partition_name(arg, &err);

    if (!ds || err != ERR_NONE) {
        ERROR("parse pt name error\n");
        err = ERR_PT_FULL_NAME_FORMAT;
        goto end;
    }

    ASSERT(ds);
    DBG("------------------erase_cnt = %d-------------------\n", erase_cnt++);

    /* do flashing work */
    for (i = 0; i < DL_CMD_TB_LEN; i++) {
        if (ds->partiton_type == dl_cmd_table[i].partiton_type) {
            if (ds->disk_type == dl_cmd_table[i].disk_type
                    || dl_cmd_table[i].disk_type == ALLDISK) {
                downloader = &dl_cmd_table[i];
                DBG("find erase cmd, i = %d\n", i);
                break;
            }
        }
    }

    if (!downloader) {
        ERROR("can not find command in cmd_table\n");
        err = ERR_CMD_ERROR;
        goto closedisk;
    }

    erase_cmd = downloader->erase_cmd_callback;

    if (erase_cmd) {
        DBG("erase_cmd function = %s\n", downloader->erase_cmd_name);
        err = erase_cmd(ds);

        if (err) {
            ERROR("flash error\n");
        }
    }
    else {
        ERROR("erase_cmd is NULL\n");
        err =  ERR_CMD_ERROR;
    }

closedisk:
    close_disk(ds);

end:
    return err;
}

#ifdef CONFIG_DLOADER_WITH_USB

/**
 * @brief fastboot flash command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_flash(fastboot_t *fb, const char *arg, void *data,
                               unsigned sz)
{
    DL_ERR_CODE_E err = ERR_UNKNOWN;
    err = dl_cmd_flash(arg, data, sz);

    if (err) {
        ERROR("flash error\n");
        fastboot_common_fail(fb, response_error(err, arg));
    }

    fastboot_common_okay(fb, "");
}

/**
 * @brief fastboot erase command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_erase(fastboot_t *fb, const char *arg, void *data,
                               unsigned sz)
{
    DL_ERR_CODE_E err = ERR_UNKNOWN;
    err = dl_cmd_erase(arg, data, sz);

    if (err) {
        ERROR("erase error\n");
        fastboot_common_fail(fb, response_error(err, arg));
    }

    fastboot_common_okay(fb, "");
}

static void mark_boot_pin_flag(uint8_t boot_pin)
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val |= ((boot_pin << ROM_CTRL_BOOT_PIN_OVERRIDE_BIT_OFFSET) & 0x1E);
    val |= (0x1 << ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

static void clear_boot_pin_flag()
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val &= ~0x1E;
    val &= ~(0x1 << ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

/**
 * @brief fastboot reboot command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_reboot(fastboot_t *fb, const char *arg, void *data,
                                unsigned sz)
{
    DBG("fastboot reboot device arg = %s\n", arg);
    if(strstr(arg, "bootloader")) {
        mark_boot_pin_flag(8);
    }
    else {
        clear_boot_pin_flag();
    }
    reboot_global_ctrl();
    fastboot_common_okay(fb, "");

}

/**
 * @brief fastboot recv md5 command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_md5(fastboot_t *fb, const char *arg, void *data,
                             unsigned sz)
{
    char md5_received_str[MD5_LEN * 2 + 1] = {0};
    memset(md5_received, 0x0, MD5_LEN);
    memcpy(md5_received_str, arg, MD5_LEN * 2);
    str2hex(md5_received_str, MD5_LEN * 2, md5_received, MD5_LEN);
    do_md5_rcv_check = true;
    fastboot_common_okay(fb, "");
}

/**
 * @brief fastboot read fuse command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_read_fuse(fastboot_t *fb, const char *arg, void *data,
                                   unsigned sz)
{
    uint32_t fuse_val   = 0;
    uint32_t fuse_index = 0;
    char fuse_response[MAX_RSP_SIZE] = {0};

    fuse_index = strtoul(arg, NULL, 16);
    DBG("arg:%s fuse_index = 0x%x!\n", arg, fuse_index);

    if (errno != 0 || fuse_index > FUSE_INDEX_MAX) {
        ERROR("index error:%d!\n", errno);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_INDEX, ""));
        return;
    }

    if (0 != fuse_sense(fuse_index, &fuse_val)) {
        ERROR("failed to sense fuse 0x%x\n", fuse_index);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_READ, ""));
        return;
    }

    DBG("fuse_0x%x = 0x%08x\n", fuse_index, fuse_val);
    snprintf(fuse_response, sizeof(fuse_response), "eFuseRead:%08x:%08x",
             fuse_index, fuse_val);
    fastboot_common_okay(fb, fuse_response);
    return;
}

/**
 * @brief Get the efuse args object
 * @param arg input arguement
 * @param index fuse index
 * @param val fuseval
 * @return 0 is success
 */
static uint32_t get_efuse_args(const char *arg, uint32_t *index, uint32_t *val)
{
    uint32_t arg_len = 0;
    const char token = ':';
    uint32_t val_str_len = 0;
    const char *val_pos  = NULL;
    const char *md5_pos  = NULL;
    uint32_t md5_str_len   = 0;
    uint8_t md5_r[MD5_LEN] = {0};
    uint32_t index_str_len    = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    char val_str[UINT32_HEX_STR_LEN + 1]   = {0};
    char index_str[UINT32_HEX_STR_LEN + 1] = {0};

    val_pos = strchr(arg, token);
    md5_pos = strrchr(arg, token);

    if (!val_pos || val_pos == md5_pos) {
        ERROR("arg error:%s!\n", arg);
        return 1;
    }

    arg_len = strlen(arg);
    index_str_len = val_pos - arg;
    val_str_len = md5_pos - val_pos - 1;
    md5_str_len = arg_len - index_str_len - val_str_len - 2;

    if ( index_str_len > UINT32_HEX_STR_LEN
            || val_str_len > UINT32_HEX_STR_LEN
            || md5_str_len != MD5_LEN * 2) {
        ERROR("arg len error:%s!\n", arg);
        return 2;
    }

    str2hex(md5_pos + 1, md5_str_len, md5_r, MD5_LEN);
    md5((const unsigned char *)arg, val_str_len + index_str_len + 1, md5_calc);

    if (memcmp(md5_calc, md5_r, MD5_LEN)) {
        ERROR("md5 check fail!\n");
        hexdump(md5_r, MD5_LEN);
        hexdump(md5_calc, MD5_LEN);
        return 3;
    }

    strncpy(index_str, arg, index_str_len);
    strncpy(val_str, val_pos + 1, val_str_len);

    *index = strtoul(index_str, NULL, 16);

    if (errno || *index > FUSE_INDEX_MAX) {
        ERROR("strtoul error:%d!\n", errno);
        return 4;
    }

    *val = strtoul(val_str, NULL, 16);

    if (errno) {
        ERROR("strtoul error:%d!\n", errno);
        return 5;
    }

    return 0;
}

/**
 * @brief fastboot flash fuse command
 * @param fb fastboot hanlder
 * @param arg input argument
 * @param data data buffer
 * @param sz size of data
 */
static void fastboot_cmd_flash_fuse(fastboot_t *fb, const char *arg,void *data,
                                   unsigned sz)
{
    uint32_t ret = 0;
    uint32_t fuse_val   = 0;
    uint32_t fuse_index = 0;

    ret = get_efuse_args(arg, &fuse_index, &fuse_val);

    if (ret) {
        ERROR("index ret:%d!\n", ret);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_INDEX, ""));
        return;
    }

    DBG("fuse index:0x%08x  val:0x%08x\n",
        fuse_index, fuse_val);

    ret = fuse_program(fuse_index, fuse_val);

    if (ret) {
        ERROR("burn efuse fail:%d!\n", ret);
        fastboot_common_fail(fb, response_error(ERR_EFUSE_BURN, ""));
        return;
    }

    fastboot_common_okay(fb, "");
    return;
}

/**
 * @brief Get the serialno number
 * @return const char*
 */
static const char *get_serialno(void)
{
    return FB_USBD_DevCfg.SerialNbrStrPtr;
}

/**
 * @brief Get the product name
 * @return const char*
 */
static const char *get_product(void)
{
    return FB_USBD_DevCfg.ProductStrPtr;
}

/**
 * @brief register fastboot commands
 * @param dl_buf_sz data buffer size
 */
static void register_commands(uint32_t dl_buf_sz)
{
    static char max_download_size[MAX_RSP_SIZE] = {0};

    /* Publish fastboot cmd */
    fastboot_register_cmd("flash:",        fastboot_cmd_flash);
    fastboot_register_cmd("erase:",        fastboot_cmd_erase);
    fastboot_register_cmd("reboot",        fastboot_cmd_reboot);
    fastboot_register_cmd("md5:",          fastboot_cmd_md5);
    fastboot_register_cmd("eFuseRead:",    fastboot_cmd_read_fuse);
    fastboot_register_cmd("eFuseProgram:", fastboot_cmd_flash_fuse);

    /* publish variables and their values */
    fastboot_register_var("product",  get_product());
    fastboot_register_var("serialno", get_serialno());
    fastboot_register_var("dloader-version", DLOADER_VERSION);
    fastboot_register_var("dev-stage", "dloader");

    /* Max download size supported */
    snprintf(max_download_size, MAX_RSP_SIZE, "\t0x%x", dl_buf_sz);
    fastboot_register_var("max-download-size", (const char *) max_download_size);
}

#endif //DLOADER_WITH_USB

#if CONFIG_DLOADER_WITH_TRACE

/**
 * @brief dump trace command
 */
static void dump_cmd(void)
{
    DBG("cmd:%p \n", cmd_args);
    DBG("storage      :%10s\n", cmd_args->storage_name);
    DBG("partition    :%10s\n", cmd_args->name);
    DBG("total size   :%10llu\n", cmd_args->total_sz);
    DBG("offset       :%10llu\n", cmd_args->offset);
    DBG("current size :%10llu\n", cmd_args->cur_sz);
    DBG("download flag:%10llu\n\n", cmd_args->dl_status);
}

/**
 * @brief Trace process thread
 * @param arg not used
 */
static void trace_process(void *arg)
{
    DL_ERR_CODE_E err = ERR_UNKNOWN;
    char pt_name[256] = {0};

    while (true) {
        /* wait recv event happen */
        while (!(cmd_args->dl_status)) {
            //DBG("trace process waiting\n");
            osDelay(osKernelGetTickFreq() / 10);
        }

        cmd_args->dl_status = 0;
        prog_status->ret_code = 1;
        prog_status->flag = 0;
        dump_cmd();

        memset(pt_name, 0, 256);
        sprintf(pt_name, "%s$$%s", cmd_args->storage_name, cmd_args->name);
        err = dl_cmd_flash(pt_name, dl_data_base, cmd_args->cur_sz);

        if (!err) {
            prog_status->ret_code = 0;
            DBG("trace flash success %p\n", &prog_status->ret_code);
        }
        else {
            ERROR("trace flash error, %s", err_info[err]);
        }

        prog_status->flag = 1;
    }
}
#endif

/**
 * @brief dloader init
 * @return 0 is success , other is fail
 */
int dloader_init(void)
{

#if CONFIG_DLOADER_BLOCK_IO_MODE
    DBG("Block Io Mode\n");
#else
    DBG("Cached Io Mode\n");
#endif

    dl_scratch_sz   = CONFIG_DL_SCRATCH_SIZE;
    dl_scratch_base = (void *)osAllocAlign(DL_ALIGN_SIZE, dl_scratch_sz);

    if (!dl_scratch_base) {
        ERROR("dl_scratch_base align failed\n");
        return -1;
    }

    if (CONFIG_MEM_SIZE <= TAISHAN_IRAM_MIN_SIZE) {
        dl_data_sz = MIN(TAISHAN_DLOADER_STANDARD_SIZE, CONFIG_DL_DATA_BUF_SIZE);
    }
    else {
        dl_data_sz = CONFIG_DL_DATA_BUF_SIZE;
    }

    dl_data_base  = (void *)osAllocAlign(DL_ALIGN_SIZE, dl_data_sz);

    if (!dl_data_base) {
        ERROR("dl_data_base align failed\n");
        return -1;
    }

    DBG("dl_scratch_base:0x%p dl_scratch_sz: 0x%08x\n", dl_scratch_base,
        dl_scratch_sz);
    DBG("dl_data_base:   0x%p dl_data_sz:    0x%08x\n", dl_data_base, dl_data_sz);

    do_md5_rb_check = true;
    do_md5_rcv_check = false;

#ifdef CONFIG_DLOADER_WITH_USB
    DBG("USB init\n");
    register_commands(dl_data_sz);
    fb_data = fastboot_common_init((void *)dl_data_base, dl_data_sz);

    if (!fb_data) {
        ERROR("fastboot_common_init failed\n");
        return -1;
    }

#endif

#if CONFIG_DLOADER_WITH_TRACE
    prog_status = (PROG_STATUS_T *)CONFIG_TRACE_PROG_STATUS_BASE;

    if (!prog_status) {
        ERROR("prog_status osAllocAlign failed\n");
        return -1;
    }

    cmd_args = (PROG_CMD_ARGS_T *)CONFIG_TRACE_CMD_BASE;

    if (!cmd_args) {
        ERROR("cmd_args osAllocAlign failed\n");
        return -1;
    }

    DBG("Trace Program Status Memory Base = 0x%x\n", CONFIG_TRACE_PROG_STATUS_BASE);
    DBG("Trace Command Memory Base = 0x%x\n", CONFIG_TRACE_CMD_BASE);

    arch_disable_cache(UCACHE);
    memset(prog_status, 0x0, sizeof(PROG_STATUS_T));
    memset(cmd_args, 0x0, sizeof(PROG_CMD_ARGS_T));
    cmd_args->elf_status = ELF_STATUS_MAGIC;
    cmd_args->data_base = (uint32_t)dl_data_base;
    cmd_args->data_size = dl_data_sz;
    ASSERT(dl_data_sz >= TAISHAN_DLOADER_STANDARD_SIZE);

    if (!trace_process_thread) {
        osThreadAttr_t attr = {0};
        attr.name       = (const char *)"trace_process";
        attr.stack_size = TRACE_PROCESS_STACK_SIZE;
        attr.priority   = osPriorityNormal;
        trace_process_thread = osThreadNew(trace_process, NULL, &attr);

        if (!trace_process_thread) {
            ERROR("failed to dloader_test process thread\n");
            return -1;
        }
    }

    DBG("Start dloader for trace!\n");
    return 0;
#endif

    return 0;
}
