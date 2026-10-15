/**
 * @file load_image.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
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
#include <CLI_type.h>
#include <debug.h>
#include <lib/cksum.h>
#include <lib/partition/partition_parser.h>
#include <cmsis_os2.h>
#include <lib/disk/disk.h>
#include <arch/cache.h>


#define DL_DEBUG_ON 1
#if DL_DEBUG_ON
#define ERROR(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[ERROR]CLOADER:%s %d "format"", __func__, __LINE__,  ##args);
#define DBG(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[DEBUG]CLOADER:%s %d "format"", __func__, __LINE__,  ##args);
#else
#define ERROR(format, args...)
#define DBG(format, args...)
#endif

#define READ_SIZE 512
#define CL_BLOCK_SIZE 512
#define MAX_DEV_INST_OF_DISK 3
#define MAX_GPT_NAME_SIZE 72
#define OSPI1_STORAGE_NAME    "ospi1"
#define OSPI2_STORAGE_NAME    "ospi2"
#define EMMC1_STORAGE_NAME    "emmc1"
#define EMMC2_STORAGE_NAME    "emmc2"
#define SD1_STORAGE_NAME      "sd3"
#define SD2_STORAGE_NAME      "sd4"
#define SD_SPL_OFFSET         (0x5000)
#define CL_ARGV_NUM           (10)
#define CL_ARGV_LEN           (50)
#define CL_STACK_SIZE         (8192)
#define NORFLASH_GPT_OFFSET   (0x2000)

#define SP0_MEM_BASE 0x700000
#define SP1_MEM_BASE 0x780000
#define SX0_MEM_BASE 0x600000
#define SX1_MEM_BASE 0x680000
#define CORE_SIZE 524288


/* record downloaded state */
typedef struct dl_state {
    const char               download_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot1_name[MAX_GPT_NAME_SIZE + 1];
    const char               disk_boot2_name[MAX_GPT_NAME_SIZE + 1];
    struct disk_dev         *disk_inst[MAX_DEV_INST_OF_DISK];
    struct partition_device *ptdev;
    uint64_t                 ptb_offset;
    char                     ptname[MAX_GPT_NAME_SIZE + 1];
} DL_STATE_T;

typedef struct cl_arg {
    int argc;
    char argv[CL_ARGV_NUM][CL_ARGV_LEN];
} cl_arg_t;

static uint8_t tempbuf[READ_SIZE] = {0};
static cl_arg_t cl_arg = {0};
static osEventFlagsId_t new_event;
static osThreadId_t cl_process_thread;
static struct disk_dev disk_dev_norflash0     = {0};
static struct disk_dev disk_dev_emmc0         = {0};
static struct disk_dev disk_dev_emmc0_boot1   = {0};
static struct disk_dev disk_dev_emmc0_boot2   = {0};
static struct disk_dev disk_dev_sdcard0       = {0};
static struct disk_dev disk_dev_sdcard1       = {0};

/* disk information */
static DL_STATE_T dl_state[] = {
    [0] = {
        .download_name  = OSPI1_STORAGE_NAME,
        .disk_name      = DISK_NOR_FLASH_NAME(0),
        .disk_inst[0]   = &disk_dev_norflash0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .ptdev          = NULL,
        .ptb_offset     = NORFLASH_GPT_OFFSET,
    },

    [1] = {
        .download_name   = EMMC1_STORAGE_NAME,
        .disk_name       = DISK_MMC_NAME(0),
        .disk_boot1_name = DISK_MMC_BOOT_NAME(0, 1),
        .disk_boot2_name = DISK_MMC_BOOT_NAME(0, 2),
        .disk_inst[0]    = &disk_dev_emmc0,
        .disk_inst[1]    = &disk_dev_emmc0_boot1,
        .disk_inst[2]    = &disk_dev_emmc0_boot2,
        .ptdev           = NULL,
        .ptb_offset      = 0,
    },

    [2] = {
        .download_name  = SD1_STORAGE_NAME,
        .disk_name      = DISK_MMC_NAME(0),
        .disk_inst[0]   = &disk_dev_sdcard0,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .ptdev          = NULL,
        .ptb_offset     = 0,
    },

    [3] = {
        .download_name  = SD2_STORAGE_NAME,
        .disk_name      = DISK_MMC_NAME(1),
        .disk_inst[0]   = &disk_dev_sdcard1,
        .disk_inst[1]   = NULL,
        .disk_inst[2]   = NULL,
        .ptdev          = NULL,
        .ptb_offset     = 0,
    },
};

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
 * @brief parse the partition information
 * @param full_ptname
 * @return DL_STATE_T*
 */
static DL_STATE_T *parse_partition_name(const char *full_ptname)
{
    char *token1 = NULL;
    char *token2 = NULL;
    uint32_t token_num   = 0;
    uint32_t full_len    = 0;
    uint32_t pt_len      = 0;
    char disk_name[MAX_GPT_NAME_SIZE + 1]  = {0};
    char short_ptname[MAX_GPT_NAME_SIZE + 1]  = {0};
    uint32_t block_size;
    uint32_t erase_size;
    uint64_t capacity;

    DL_STATE_T *ds = NULL;
    DBG("-------------------Parsing name------------------------\n");

    if (!full_ptname || (full_len = strlen(full_ptname)) == 0) {
        goto fail;
    }

    for (uint32_t i = 0; i < full_len; i++) {
        if (full_ptname[i] == '$') {
            token_num++;
        }
    }

    /* full name:xxx$yyy$zzz */
    if (token_num != 2) {
        goto fail;
    }

    token1 = strchr(full_ptname, '$');
    token2 = strrchr(full_ptname, '$');

    strncpy(disk_name, full_ptname, token1 - full_ptname);
    strncpy(short_ptname, token2 + 1, full_len - (token2 + 1 - full_ptname));

    DBG("disk_name:%s, sub_pt:%s\n", disk_name, short_ptname);

    for (uint32_t i = 0; i < sizeof(dl_state) / sizeof(dl_state[0]); i++) {
        if (!strcmp(dl_state[i].download_name, disk_name)) {
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

    disk_set_block_size(ds->disk_inst[0], CL_BLOCK_SIZE);
    block_size = disk_get_block_size(ds->disk_inst[0]);
    erase_size = disk_erase_size(ds->disk_inst[0]);
    capacity   = disk_size(ds->disk_inst[0]);

    ASSERT(block_size != 0);
    ASSERT(erase_size != 0);
    ASSERT(block_size == CL_BLOCK_SIZE);
    ASSERT(erase_size % block_size == 0);
    ASSERT(capacity > block_size);
    ASSERT(capacity > erase_size);
    ASSERT(capacity > ds->ptb_offset);
    ASSERT(capacity > (GET_PRI_GPT_SIZE(block_size)));

    if (!ds->ptdev) {
        ds->ptdev = ptdev_setup(ds->disk_inst[0], ds->ptb_offset);

        if (ds->ptdev) {
            DBG("read partition table\n");

            if (ptdev_read_table(ds->ptdev)) {
                ERROR("ptdev_read_table error, disk_name = %s\n", disk_name);
                goto fail;
            }
        }
        else {
            ERROR("get ptdev:%s error\n", disk_name);
            goto fail;
        }
    }

    pt_len = strlen(short_ptname);
    memset(ds->ptname, 0x0, sizeof(ds->ptname));
    strncpy(ds->ptname, short_ptname, pt_len);

    DBG("download_name   = %s\n",     ds->download_name);
    DBG("disk_name       = %s\n",     ds->disk_name);
    DBG("disk_boot1_name = %s\n",     ds->disk_boot1_name);
    DBG("disk_boot2_name = %s\n",     ds->disk_boot2_name);
    DBG("ptb_offset      = %lld\n",   ds->ptb_offset);
    DBG("ptname          = %s\n",     ds->ptname);
    DBG("block_size      = %d\n",     block_size);
    DBG("erase_size      = %d\n",     erase_size);
    DBG("capacity        = %lld\n",   capacity);
    return ds;

fail:
    close_disk(ds);
    return NULL;
}

/**
 * @brief core loader process
 * @return int
 */
static int core_loader_process(void)
{
    int ret = 0;
    uint32_t addr = 0;
    uint32_t membase = 0;
    uint32_t length = 0;
    uint64_t ptn = 0;
    uint64_t size = 0;
    uint32_t total_sec = 0;
    uint32_t i = 0;
    DL_STATE_T *ds = NULL;
    uint8_t md5_calc[MD5_LEN] = {0};
    struct MD5Context context;
    char *addrsuffix;

    ds = parse_partition_name((const char *)cl_arg.argv[0]);

    if (!ds) {
        ERROR("partition name error\n");
        ret = -1;
        goto end;
    }

    ASSERT(ds);

    if (!ds->ptdev) {
        ERROR("partition is null\n");
        ret = -1;
        goto end;
    }

    if(cl_arg.argc < 3) {
        if(strstr((const char *)cl_arg.argv[0],"sp0")) {
            addr =   SP0_MEM_BASE;
            length = CORE_SIZE;
        }

        if(strstr((const char *)cl_arg.argv[0],"sp1")) {
            addr =   SP1_MEM_BASE;
            length = CORE_SIZE;
        }

        if(strstr((const char *)cl_arg.argv[0],"sx0")) {
            addr =   SX0_MEM_BASE;
            length = CORE_SIZE;
        }

        if(strstr((const char *)cl_arg.argv[0],"sx1")) {
            addr =   SX1_MEM_BASE;
            length = CORE_SIZE;
        }
    }
    else {
        addr =   (uint32_t)strtoull(cl_arg.argv[1], &addrsuffix, 16);
        length = (uint32_t)atoi(cl_arg.argv[2]);
    }

    ptn = ptdev_get_offset(ds->ptdev, ds->ptname);
    DBG("addr 0x%08x, len %d , ptn 0x%llx\n", addr, length, ptn);

    size = ptdev_get_size(ds->ptdev, ds->ptname);

    if (addr % 512 || ptn % 512
            || size % READ_SIZE || length % READ_SIZE
            || !size || !length || !ptn) {
        ERROR("para error\n");
        ret = -1;
        goto end;
    }

    MD5Init(&context);
    membase = addr;

    if (size <= length)
        total_sec = size / READ_SIZE;
    else
        total_sec = length / READ_SIZE;

    DBG("total_sec is %d, READ_SIZE is %d \n", total_sec, READ_SIZE)
    DBG("load partition cmd ptn 0x%llx, len %lld--->addr 0x%08x length =%d\n", ptn,
        size, addr, length);

    for (i = 0; i < total_sec; i++) {
        /* clear temp buffer */
        memset((uint8_t *)tempbuf, 0, READ_SIZE);

        /* read from buffer */
        if (disk_read(ds->disk_inst[0], ptn, (uint8_t *)tempbuf, READ_SIZE)) {
            ERROR("read storage failed\n");
            ret = -1;
            goto end;
        }

        /* copy to the dest ram addr */
        memcpy((void *)(uint32_t)addr, (uint8_t *)tempbuf, READ_SIZE);

        /* update md5 for disk's readbuffer and  ram addr */
        MD5Update(&context, (uint8_t *)(uint32_t)addr, READ_SIZE);

        /* update address */
        ptn += READ_SIZE;
        addr += READ_SIZE;
    }

    arch_clean_cache_range(membase, total_sec * READ_SIZE);
    DBG("load partition finish,dump md5 from ram\n");
    MD5Final(md5_calc, &context);
    hexdump((void *)md5_calc, MD5_LEN);
    ret = 0;

end:
    close_disk(ds);
    return ret;
}

/**
 * @brief core loader thread
 * @param arg
 */
static void core_loader_thread(void *arg)
{
    while (true) {
        /* wait recv event happen */
        if (0 == osEventFlagsWait(new_event, 1, osFlagsWaitAny, osWaitForever)) {
            break;
        }

        DBG("dloader thread active\n");
        core_loader_process();

    }
}

/**
 * @brief core loader app
 * @param argc
 * @param argv
 * @return int
 */
static int core_loader_test(int argc, char *argv[])
{
    static bool foronce = 0;
    uint32_t i = 0;
    osThreadAttr_t core_img_loader_attr = {0};
    core_img_loader_attr.name       = (const char *)"core_img_loader";
    core_img_loader_attr.stack_size = CL_STACK_SIZE;
    core_img_loader_attr.priority   = osPriorityNormal;

    if (0 == foronce) {
        new_event = osEventFlagsNew(NULL);
        cl_process_thread = osThreadNew(core_loader_thread, (void *)&cl_arg,
                                        &core_img_loader_attr);

        if (!cl_process_thread) {
            ERROR("failed to dloader_test process thread\n");
            return CLI_FALSE;
        }

        foronce = 1;
    }

    cl_arg.argc = argc;
    DBG("core loader argc = %d\n", cl_arg.argc);

    if (CL_ARGV_NUM < cl_arg.argc) {
        ERROR("cl_arg.argc is too much\n")
    }

    for (i = 0; i < cl_arg.argc; i++) {
        memset(cl_arg.argv[i], 0, CL_ARGV_LEN);
        strncpy(cl_arg.argv[i], argv[i], CL_ARGV_LEN - 1);
        DBG("cl_arg.argc[%d] = %s\n", i, cl_arg.argv[i]);
    }

    osEventFlagsSet(new_event, 1);
    DBG("event set!");

    return CLI_FALSE;
}

CLI_CMD("loadcore", "load_core_image", core_loader_test);
