/**
 * @file dloader_test.c
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
#include <arch/cache.h>
#include <lib/partition/partition_parser.h>
#include <cmsis_os2.h>
#include <malloc.h>
#include "dloader.h"
#include <lib/sd_boot_img/sd_boot_img.h>

#define  DL_TEST_STACK_SIZE 8192
#define  DL_TEST_ARGV_NUM   10
#define  DL_TEST_ARGV_LEN   50
#if CONFIG_DLOADER_BLOCK_IO_MODE
#define  DL_LENGTH_ALIGN    512
#else
#define  DL_LENGTH_ALIGN    128
#endif
#define  DL_ADDR_ALIGN      512
#define  DL_MD5RAM_ALIGN    512
#define  DL_MD5DISK_ALIGN   512

typedef struct dl_test_arg {
    int argc;
    char argv[DL_TEST_ARGV_NUM][DL_TEST_ARGV_LEN];
} dl_test_arg_t;

static dl_test_arg_t      dl_test_arg = {0};
static osEventFlagsId_t   new_msg_event;
static osThreadId_t       dloader_process_thread;
extern uint32_t sfs_get_image_base(DL_STATE_T *ds, uint8_t num);
extern int32_t get_bpt_info(struct bpt *bpt, uint8_t *buffer, uint32_t len);

/**
 * @brief hexdump an buffer. The addresss could be set
 * @param disk_addr
 * @param ptr
 * @param len
 */
void dl_hexdump(uint64_t disk_addr, const void *ptr, size_t len)
{
    addr_t address = (addr_t)ptr;
    size_t count;

    for (count = 0 ; count < len; count += 16) {
        union {
            uint32_t buf[4];
            uint8_t  cbuf[16];
        } u;
        size_t s = ROUNDUP(MIN(len - count, 16), 4);
        size_t i;

        printf("0x%08x: ", (uint32_t)disk_addr);

        for (i = 0; i < s / 4; i++) {
            u.buf[i] = ((const uint32_t *)address)[i];
            printf("%08x ", u.buf[i]);
        }

        for (; i < 4; i++) {
            printf("         ");
        }

        printf("|");

        for (i = 0; i < 16; i++) {
            unsigned char c = u.cbuf[i];

            if (i < s && isprint(c)) {
                printf("%c", c);
            }
            else {
                printf(".");
            }
        }

        printf("|\n");
        address += 16;
        disk_addr += 16;
    }
}

/**
 * @brief Get the inst num of disk
 * @param partiton_type
 * @return uint8_t
 */
static uint8_t get_inst_num(PARTITION_TYPE_E partiton_type)
{
    uint8_t inst_num = 0;

    if (TYPE_BOOT_PACK0 == partiton_type)
        inst_num = 1;

    else if (TYPE_BOOT_PACK1 == partiton_type)
        inst_num = 2;

    DBG("disk_inst= %d\n", inst_num);
    return inst_num;
}

static uint8_t read_bpt(DL_STATE_T *ds, uint8_t bpt_num)
{
    struct bpt *bpt = NULL;
    struct iib *iib = NULL;
    uint64_t addr = 0;
    uint64_t length = 0;
    uint8_t inst_num = 0;
    uint32_t crc32_val = 0;
    uint32_t i = 0;

    if (ds->disk_type == NORFLASH) {
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
        addr = 0x5000;
        inst_num = 0;
    }

    length = 0x1000;
    DBG("read inst %d addr 0x%llx, len %lld for BPT\n", inst_num, addr, length);
    memset((uint8_t *)dl_scratch_base, 0, length);

    if (dl_disk_read(ds->disk_inst[inst_num], addr, (uint8_t *)dl_scratch_base,
                     length)) {
        ERROR("read storage failed\n");
        return 1;
    }

    crc32_val = sfs_crc32(0, (uint8_t *)dl_scratch_base, 0xFEC);

    bpt = (struct bpt *)memalign(ds->block_size, sizeof(struct bpt));

    if (0 != get_bpt_info(bpt, (uint8_t *)dl_scratch_base, length)) {
        free(bpt);
        return 1;
    }
    else {
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

        free(bpt);
        return 0;
    }

}

/**
 * @brief dloader test thread
 * @param arg
 */
static void dloader_test_thread(void *arg)
{
    uint64_t length_total = 0;
    uint32_t total_sec = 0;
    uint32_t left_sec = 0;
    uint32_t i = 0;
    uint8_t inst_num = 0;
    char *addrsuffix;
    uint32_t crc32_val = 0;
    uint8_t md5_calc[MD5_LEN] = {0};
    struct MD5Context context;
    DL_STATE_T *ds = NULL;
    uint64_t addr = 0;
    uint64_t length = 0;
    DL_ERR_CODE_E err;

    while (true) {

        if (0 == osEventFlagsWait(new_msg_event, 1, osFlagsWaitAny, osWaitForever)) {
            break;
        }

        DBG("dloader thread active\n");
        ds = NULL;
        inst_num = 0;
        crc32_val = 0;
        memset(md5_calc, 0, MD5_LEN);

        if (!strcmp(dl_test_arg.argv[0], "discache")) {
            arch_invalidate_cache_range((uint32_t)0x30000000, 1048576 * 2);
        }

        /* disk read test */
        else if (!strcmp(dl_test_arg.argv[0], "read")) {

            if (dl_test_arg.argc < 2) {
                ERROR("dl read need at least 2 arg\n");
                continue;
            }

            ds = parse_partition_name((const char *)dl_test_arg.argv[1], &err);

            if (!ds) {
                ERROR("partition name error\n");
                continue;
            }

            if (ds->disk_type == MMC)
                inst_num = get_inst_num(ds->partiton_type);

            /* read sfs */
            if (!strcmp(ds->ptname, "sfs") && (ds->disk_type == NORFLASH)) {
                DBG("read sfs\n");
                read_sfs(ds);
                continue;
            }

            /* read partition table */
            if (!strcmp(ds->ptname, "partition")) {
                DBG("read partition\n");
                ptdev_read_table(ds->ptdev);
                ptdev_dump(ds->ptdev);
                continue;
            }

            /* read a partition address in storage device */
            else if (!strcmp(ds->ptname, "bpt0")) {
                read_bpt(ds, 0);
                continue;
            }

            /* read a partition address in storage device */
            else if (!strcmp(ds->ptname, "bpt1")) {
                read_bpt(ds, 1);
                continue;
            }

            /* read a partition address in storage device */
            else if (!strcmp(ds->ptname, "bpt2")) {
                read_bpt(ds, 2);
                continue;
            }

            /* read a partition address in storage device */
            else if ( dl_test_arg.argc <= 3 && 0 == inst_num) {
                if (!ds->ptdev) {
                    ERROR("partition is null\n");
                    continue;
                }

                addr = ptdev_get_offset(ds->ptdev, ds->ptname);
                length = ptdev_get_size(ds->ptdev, ds->ptname);
                DBG("read partition cmd addr 0x%llx, len %lld\n", addr, length);

                if (dl_test_arg.argc == 3) {
                    DBG("argv[2] = %s(0x%08x)\n", dl_test_arg.argv[2], atoi(dl_test_arg.argv[2]));
                    length = atoi(dl_test_arg.argv[2]);
                    DBG("length is changed to  %lld\n", length);
                }

                if (!length || addr % DL_ADDR_ALIGN || length % DL_LENGTH_ALIGN) {
                    ERROR("do not align\n");
                    continue;
                }

                total_sec = length / DL_LENGTH_ALIGN;

                for (i = 0; i < total_sec; i++) {
                    memset((uint8_t *)dl_scratch_base, 0, DL_LENGTH_ALIGN);

                    if (dl_disk_read(ds->disk_inst[0], addr, (uint8_t *)dl_scratch_base,
                                     DL_LENGTH_ALIGN)) {
                        ERROR("read storage failed\n");
                        continue;
                    }

                    dl_hexdump(addr, (void *)dl_scratch_base, DL_LENGTH_ALIGN);
                    addr += DL_LENGTH_ALIGN;
                }

                continue;
            }

            /* read any address in storage device */
            else if (dl_test_arg.argc == 4) {

                addr = (addr_t)strtoull(dl_test_arg.argv[2], &addrsuffix, 16);
                length = atoi(dl_test_arg.argv[3]);

                DBG("read addr cmd addr 0x%llx, len %lld\n", addr, length);

                if (addr % DL_ADDR_ALIGN || length % DL_LENGTH_ALIGN || !length) {
                    ERROR("do not align\n");
                    continue;
                }

                if (dl_disk_read(ds->disk_inst[inst_num], addr, (uint8_t *)dl_scratch_base,
                                 MIN(length, dl_scratch_sz))) {
                    ERROR("read storage failed\n");
                    continue;
                }

                dl_hexdump((uint64_t)atoi(dl_test_arg.argv[2]), (void *)dl_scratch_base,
                           atoi(dl_test_arg.argv[3]));
                crc32_val = crc32(0, (uint8_t *)dl_scratch_base, MIN(length, dl_scratch_sz));
                DBG("crc32_val = 0x%08x\n", crc32_val);
            }
        }

        /* md5 calc for disk */
        else if (!strcmp(dl_test_arg.argv[0], "md5disk")) {
            if (dl_test_arg.argc != 4) {
                ERROR("md5disk need 4 arg\n");
                continue;
            }

            ds = parse_partition_name((const char *)dl_test_arg.argv[1], &err);

            if (!ds) {
                ERROR("partition name error\n");
                continue;
            }

            if (ds->disk_type == MMC)
                inst_num = get_inst_num(ds->partiton_type);

            addr = (addr_t)strtoull(dl_test_arg.argv[2], &addrsuffix, 16);
            length = atoi(dl_test_arg.argv[3]);
            printf("disk md5 cmd addr 0x%llx, len %lld\n", addr, length);
            MD5Init(&context);

            if (addr % DL_ADDR_ALIGN || length % DL_LENGTH_ALIGN) {
                ERROR("do not align\n");
                continue;
            }

            total_sec = length / DL_LENGTH_ALIGN;

            for (i = 0; i < total_sec; i++) {
                memset((uint8_t *)dl_scratch_base, 0, DL_LENGTH_ALIGN);

                if (dl_disk_read(ds->disk_inst[inst_num], addr, (uint8_t *)dl_scratch_base,
                                 DL_LENGTH_ALIGN)) {
                    ERROR("read storage failed\n");
                    continue;
                }

                crc32_val = crc32(crc32_val, (uint8_t *)dl_scratch_base, DL_LENGTH_ALIGN);
                MD5Update(&context, (uint8_t *)dl_scratch_base, DL_LENGTH_ALIGN);
                addr += DL_LENGTH_ALIGN;
            }

            DBG("crc32_val = 0x%08x\n", crc32_val);
            MD5Final(md5_calc, &context);
            dl_hexdump(0, (void *)md5_calc, MD5_LEN);
            continue;
        }

        /* md5 calc for ram */
        else if (!strcmp(dl_test_arg.argv[0], "md5ram")) {

            if (dl_test_arg.argc != 3) {
                ERROR("md5ram need 3 arg\n");
                continue;
            }

            addr = (addr_t)strtoull(dl_test_arg.argv[1], &addrsuffix, 16);
            length = atoi(dl_test_arg.argv[2]);

            printf("ram md5 cmd addr 0x%llx, len %lld\n", addr, length);
            MD5Init(&context);

            if (addr % DL_ADDR_ALIGN || length % DL_LENGTH_ALIGN) {
                ERROR("do not align\n");
                continue;
            }

            total_sec = length / DL_MD5RAM_ALIGN;

            for (i = 0; i < total_sec; i++) {
                memset((uint8_t *)dl_scratch_base, 0, DL_MD5RAM_ALIGN);
                memcpy((uint8_t *)dl_scratch_base, (void *)(uint32_t)addr, DL_MD5RAM_ALIGN);
                crc32_val = crc32(crc32_val, (uint8_t *)dl_scratch_base, DL_MD5RAM_ALIGN);
                MD5Update(&context, (uint8_t *)dl_scratch_base, DL_MD5RAM_ALIGN);
                addr += DL_MD5RAM_ALIGN;
            }

            DBG("crc32_val = 0x%08x\n", crc32_val);
            MD5Final(md5_calc, &context);
            dl_hexdump(0, (void *)md5_calc, MD5_LEN);
            continue;
        }

        /* flash command test */
        else if (!strcmp(dl_test_arg.argv[0], "flash")) {

            if (dl_test_arg.argc != 4) {
                ERROR("dl flash need at least 4 arg\n");
                continue;
            }

            addr = (addr_t)strtoull(dl_test_arg.argv[2], &addrsuffix, 16);
            length_total = (uint32_t)atoi(dl_test_arg.argv[3]);

            DBG("flash cmd = %s, addr 0x%llx, len %lld\n",
                (const char *)dl_test_arg.argv[1], addr, length_total);

            total_sec = ((uint32_t)length_total) / dl_data_sz;
            left_sec = ((uint32_t)length_total) % dl_data_sz;
            DBG("total_sec = %d, left_sec = %d \n", total_sec, left_sec);

            for (i = 0; i <= total_sec; i++) {
                if (i == total_sec) {
                    length = left_sec;
                }
                else {
                    length = dl_data_sz;
                }

                if (0 == (uint32_t)length)
                    break;

                DBG("flash data from addr = %d, length = %d \n", (uint32_t)addr,
                    (uint32_t)length);
                md5((const uint8_t *)(uint32_t)addr, (const int)length, md5_received);

                if (dl_cmd_flash((const char *)dl_test_arg.argv[1], (void *)(uint32_t)addr,
                                 (uint32_t)length)) {
                    ERROR("dl_cmd_flash error\n");
                    continue;
                }

                addr += length;
            }

            DBG("md5_received :\n");
            dl_hexdump(0, (void *)md5_received, MD5_LEN);
            DBG("dl_cmd_flash finish\n");
            continue;
        }

        /* erase command test*/
        else if (!strcmp(dl_test_arg.argv[0], "erase")) {
            if (dl_test_arg.argc != 2) {
                ERROR("dl flash need at least 2 arg\n");
                continue;
            }

            DBG("cmd = %s\n", (const char *)dl_test_arg.argv[1]);
            dl_cmd_erase((const char *)dl_test_arg.argv[1], dl_data_base, 0);
            continue;
        }
    }
}

/**
 * @brief test app for dloader
 * @param argc
 * @param argv
 * @return int
 */
static int dloader_test(int argc, char *argv[])
{
    int i = 0;
    static bool dl_test_thread_on = 0;

    if (0 == dl_test_thread_on) {
        if (!strcmp(argv[0], "init")) {
            dl_test_thread_on = 1;
            /* test thread */
            new_msg_event = osEventFlagsNew(NULL);
            osThreadAttr_t dloader_test_attr = {0};
            dloader_test_attr.name       = (const char *)"dloader_test";
            dloader_test_attr.stack_size = DL_TEST_STACK_SIZE;
            dloader_test_attr.priority   = osPriorityNormal;
            dloader_process_thread = osThreadNew(dloader_test_thread, (void *)&dl_test_arg,
                                                 &dloader_test_attr);

            if (!dloader_process_thread) {
                ERROR("failed to dloader_test process thread\n");
                return CLI_FALSE;
            }

            DBG("download test init success\n");
            return CLI_FALSE;
        }
    }

    if (argc == 0) {
        ERROR("need in put arg\n");
        return CLI_FALSE;
    }

    if (argc > DL_TEST_ARGV_NUM) {
        ERROR("too many arg\n");
        return CLI_FALSE;
    }

    DBG("dloader argc = %d\n", argc);
    dl_test_arg.argc = argc;

    for (i = 0; i < dl_test_arg.argc; i++) {
        memset(dl_test_arg.argv[i], 0, DL_TEST_ARGV_LEN);
        strncpy(dl_test_arg.argv[i], argv[i], DL_TEST_ARGV_LEN - 1);
        DBG("dl_test_arg.argc[%d] = %s\n", i, dl_test_arg.argv[i]);
    }

    osEventFlagsSet(new_msg_event, 1);
    DBG("event set!");
    return CLI_FALSE;
}

CLI_CMD("dl", "download test", dloader_test);
