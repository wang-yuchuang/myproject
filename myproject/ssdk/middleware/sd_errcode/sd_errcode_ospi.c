/*
 * sd_errcode_ospi.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * set errcode to ospi
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <lib/partition/partition_parser.h>
#include <lib/disk/disk.h>
#include <lib/fee/fee.h>
#include <lib/fee/fee_eeprom.h>
#include "sd_errcode_ospi.h"

#define ERRCODE_STOR_SIZE 16
#define FEE_ERRCODE_PART_RESERVED_SIZE 16    /*Record current write address*/
#define FEE_ERRCODE_PART_SIZE (4*1024)
#define FEE_ERRCODE_INDEX_MAX ((FEE_ERRCODE_PART_SIZE)/ERRCODE_STOR_SIZE)

#define ERRCODE_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define ERRCODE_NO_DUPLICATE_RECORD 1

#define PART_EEPROM_ERRCODE_SZIE 0x18000
#define CL_BLOCK_SIZE 512

/** errcode item management info  */
typedef enum {
    ERRCODE_ITEM_CURRENT_ID,
    ERRCODE_ITEM_ERROR_CODE,
    ERRCODE_ITEM_INVALID_TYPE,
} errcode_item_type_t;

typedef struct {
    uint32_t id;
    uint32_t addr;
    errcode_item_type_t item_type;
    union {
        uint32_t current_index; /* Record current storage location */
        uint32_t errcode;
    };
} errcode_item_t;

typedef struct {
    errcode_item_t item[FEE_ERRCODE_INDEX_MAX];
    uint32_t item_num;         /* Include index = 0 */
} errcode_item_info_t;

typedef struct {
    uint16_t errcode;
    uint8_t mod_no;
    uint8_t mod_id;
} __attribute__((packed)) errcode_part_t;

typedef struct {
    uint32_t user_data;                 /* When index = 0, Record current storage location */
    uint32_t inter_errcode;
    uint64_t timestamp;
} __attribute__((packed)) errcode_full_t;

#if ERRCODE_NO_DUPLICATE_RECORD
static errcode_item_info_t g_errcode_item_info = {0};
#else
static uint32_t g_fee_write_address = ERRCODE_STOR_SIZE;
#endif

/* fee eeprom 1 */
static struct fee_eeprom fee_eeprom1 = {
    .fee_con = {
        .disk_name = DISK_NOR_FLASH_NAME(0),
        .size = PART_EEPROM_ERRCODE_SZIE,
        .mem_align_size = CONFIG_ARCH_CACHE_LINE,
        .access_align_size = 0x4,
        .pe_cycles = 10,

        .fee_name = FEE_EEPROM_NAME(1),
        .eeprom_size = 4 * 1024,                  /*Must be a multiple of 16*/
        .eeprom_align_size = 4,
        .eeprom_pe_cycles = 100,
    },
};

static int fee_eeprom_errcode_prepare(void *fee_eeprom)
{
    #define BOOT_SECTOR_IDX 2
    #define FEE_PARTITION_NAME   "fee_errcode"

    int ret = 0;
    struct fee_eeprom *fee_dev = (struct fee_eeprom *)fee_eeprom;
    struct partition_device *ptdev = NULL;
    struct disk_dev flash_dev = {0};

    disk_open(fee_dev->fee_con.disk_name, &flash_dev);
    disk_set_block_size(&flash_dev, CL_BLOCK_SIZE);
    ptdev = ptdev_setup(&flash_dev,disk_erase_size(&flash_dev) * BOOT_SECTOR_IDX);
    ptdev_read_table(ptdev);

    uint64_t ptn, pt_size;
    ptn = ptdev_get_offset(ptdev, FEE_PARTITION_NAME);
    pt_size = ptdev_get_size(ptdev, FEE_PARTITION_NAME);
    uint32_t sector_size = disk_erase_size(&flash_dev);
    disk_close(&flash_dev);
    fee_dev->fee_con.sector_size = sector_size;
    fee_dev->fee_con.size = pt_size;
    fee_dev->fee_con.size = ROUNDDOWN(fee_dev->fee_con.size, sector_size * 2);
    disk_addr_t addr = ptn;
    fee_dev->fee_con.addr = ROUNDUP(addr, sector_size);

    printf("\n--------------------------------------------------\n");
    printf("sector_size = %d(%dk)\n", sector_size, sector_size/1024);
    printf("ptn = 0x%llx, pt_size = 0x%llx(%lluk)\n", ptn, pt_size, pt_size/1024);
    printf("size = %llu(0x%llx)=%lluk\n", fee_dev->fee_con.size, fee_dev->fee_con.size, fee_dev->fee_con.size/1024);
    printf("addr = %llu [0x%llx]\n", fee_dev->fee_con.addr, fee_dev->fee_con.addr);
    printf("mem_align_size = %d\n", fee_dev->fee_con.mem_align_size);
    printf("access_align_size = %d\n", fee_dev->fee_con.access_align_size);
    printf("pe_cycles = %d\n", fee_dev->fee_con.pe_cycles);

    printf("fee_name = %s\n", fee_dev->fee_con.fee_name);
    printf("eeprom_size = %d\n", fee_dev->fee_con.eeprom_size);
    printf("eeprom_align_size = %d\n", fee_dev->fee_con.eeprom_align_size);
    printf("eeprom_pe_cycles = %d\n", fee_dev->fee_con.eeprom_pe_cycles);
    printf("\n--------------------------------------------------\n");

    //ptdev_dump(ptdev);
    ptdev_destroy(ptdev);

    ret = fee_register_eeprom(fee_dev);
    if (ret) {
        printf("fee_register_eeprom error\n");
        return -1;
    }

    return 0;
}

static uint32_t errcode_change_id_to_addr(uint32_t id)
{
    return (ERRCODE_STOR_SIZE * id);
}
#if ERRCODE_NO_DUPLICATE_RECORD
#else
static int fee_write(uint32_t addr, uint8_t *write_buff, uint32_t size)
{
    int ret = 0;
    if (!write_buff) {
        printf("%s : write_buff == NULL\n", __func__);
        return -1;
    }
    ERRCODE_DEBUG_PRINT("fee_write addr:0x%x size:0x%x\n", addr, size);

    ret = fee_write_eeprom(FEE_EEPROM_NAME(1), addr, write_buff, size);

    if (ret) {
        printf("%s : fee_write_eeprom errcode error\n", __func__);
    }
    g_fee_write_address += size;

    if (g_fee_write_address == FEE_ERRCODE_PART_SIZE) {
        g_fee_write_address = 16;
    }
    //Record current write address
    ret = fee_write_eeprom(FEE_EEPROM_NAME(1), 0, (uint8_t*)&g_fee_write_address, sizeof(g_fee_write_address));
    if (ret) {
        printf("%s : fee_write_eeprom current_address error\n", __func__);
    }

    return ret ;
}
#endif

static int fee_read(uint32_t addr, uint8_t *read_buff, uint32_t size)
{
    int ret = 0;
    if (!read_buff) {
        printf("%s : read_buff == NULL\n", __func__);
        return -1;
    }
    ERRCODE_DEBUG_PRINT("fee_read addr:0x%x size:0x%x \n", addr, size);

    ret = fee_read_eeprom(FEE_EEPROM_NAME(1), addr, read_buff, size);

    if (ret) {
        printf("%s : fee_read_eeprom error\n", __func__);
    }

    return ret ;
}

ERR_HANDLE errcode_set_to_ospi(int strid, uint8_t *data_buffer, uint16_t length)
{
    if (!data_buffer) {
        printf("%s : Invalid data_buffer == NULL!\n", __func__);
        return -1;
    }
    ERR_HANDLE handle = ERRCODE_OK;
    int no = 1;
    no = no;

    ERRCODE_DEBUG_PRINT("errcode_set_to_ospi start OSPI id = %d\n", strid);
    ERRCODE_DEBUG_PRINT("errcode_set_to_ospi length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    if (strid != 0/*STR_ID_OSPI1*/) {
        printf("%s : Currently, Only support OSPI1!\n", __func__);
        goto Label;
    }

#if ERRCODE_NO_DUPLICATE_RECORD
    errcode_item_info_t *errcode_info = &g_errcode_item_info;
    int ret = -1;
    int i = 0;
    uint32_t inter_errcode = 0;
    errcode_full_t *full_errcode = (errcode_full_t *)data_buffer;
    inter_errcode = full_errcode->inter_errcode;
    int addr = 0;

    for (i = 0; i < (int)ERRCODE_ARRAY_SIZE(errcode_info->item); i++) {
        if (i == 0) {
            addr = errcode_change_id_to_addr(errcode_info->item[i].current_index);
        }
        if ((errcode_info->item[i].item_type == ERRCODE_ITEM_ERROR_CODE) \
            && (errcode_info->item[i].errcode == inter_errcode)) {
            addr = errcode_info->item[i].addr;
            ERRCODE_DEBUG_PRINT("%s: The errcode  0x%x has been recorded in fee.\n", __func__, inter_errcode);
            break;
        }
    }

    if (i >= (int)ERRCODE_ARRAY_SIZE(errcode_info->item)) {
        //Update index = 0
        errcode_full_t errcode_tmp = {0};
        int index = errcode_info->item[0].current_index;
        errcode_info->item[0].current_index++;
        ERRCODE_DEBUG_PRINT("%s: Recorded a new errcode 0x%x to fee.\n", __func__, inter_errcode);

        //If reach to MAX, Circular coverage.
        if (errcode_info->item[0].current_index >= (int)ERRCODE_ARRAY_SIZE(errcode_info->item)) {
            errcode_info->item[0].current_index = 1;
            printf("%s: The fee storage reaches the maximum, Circular coverage.\n", __func__);
        }

        errcode_tmp.user_data = errcode_info->item[0].current_index;
        ret = fee_write_eeprom(FEE_EEPROM_NAME(1), 0, (uint8_t*)&errcode_tmp, sizeof(errcode_tmp));
        if (ret) {
            printf("%s : fee_write_eeprom current_address error\n", __func__);
        }

        //Update errcode_info->item
        errcode_info->item[index].id = index;
        errcode_info->item[index].addr = errcode_change_id_to_addr(index);
        errcode_info->item[index].item_type = ERRCODE_ITEM_ERROR_CODE;
        errcode_info->item[index].errcode = inter_errcode;
    }

    ret = fee_write_eeprom(FEE_EEPROM_NAME(1), addr, data_buffer, length);
    if (ret) {
        printf("%s : fee_write_eeprom errcode error\n", __func__);
    }
#else
    fee_write(g_fee_write_address, data_buffer, length);
#endif

Label:

    return handle;
}

int errcode_dump_from_ospi(int strid, char* buf, int bufsize)
{
    int ret = ERRCODE_OK;
    int total_size = 0;
    bool dump_flag = true;

    uint8_t data_buffer[ERRCODE_STOR_SIZE] = {0};
    ERRCODE_DEBUG_PRINT("errcode_dump_from_ospi start OSPI id = %d\n", strid);

    if (strid != 0/*STR_ID_OSPI1*/) {
        printf("%s : Currently, Only support OSPI1!\n", __func__);
        return ERRCODE_NG;
    }

    for (int i = 0; i < FEE_ERRCODE_INDEX_MAX; i++) {
        memset(data_buffer, 0, ERRCODE_STOR_SIZE);
        fee_read(ERRCODE_STOR_SIZE*(i), data_buffer, ERRCODE_STOR_SIZE);
        total_size += ERRCODE_STOR_SIZE;

        if (buf && dump_flag) {
            int size = 0;
            size = (bufsize >= total_size) ? ERRCODE_STOR_SIZE : (ERRCODE_STOR_SIZE+bufsize-total_size);
            memcpy(buf, data_buffer, size);
            buf += size;
            if (total_size > bufsize) {
                dump_flag = false;
            }
        }

        printf("%08x [", i*16);
        for (int j = 0; j < ERRCODE_STOR_SIZE; j++) {
            printf(" %02x", data_buffer[j]);
        }
        printf("]\n");
    }

    return total_size;
}

int errcode_clear_ospi(int strid)
{
    int ret = ERRCODE_OK;
    uint8_t data_buffer[ERRCODE_STOR_SIZE] = {0};
    ERRCODE_DEBUG_PRINT("errcode_clear_ospi start OSPI id = %d\n", strid);

    if (strid != 0/*STR_ID_OSPI1*/) {
        printf("%s : Currently, Only support OSPI1!\n", __func__);
        goto Label;
    }

    memset(data_buffer, 0, ERRCODE_STOR_SIZE);
    for (int i = 0; i < FEE_ERRCODE_INDEX_MAX; i++) {
#if ERRCODE_NO_DUPLICATE_RECORD
        ret = fee_write_eeprom(FEE_EEPROM_NAME(1), ERRCODE_STOR_SIZE*i, data_buffer, ERRCODE_STOR_SIZE);
        if (ret) {
            printf("%s : fee_write_eeprom errcode error\n", __func__);
            goto Label;
        }
#else
        fee_write(ERRCODE_STOR_SIZE*i, data_buffer, ERRCODE_STOR_SIZE);
#endif
    }

#if ERRCODE_NO_DUPLICATE_RECORD
    errcode_item_info_t *errcode_info = &g_errcode_item_info;
    memset(errcode_info, 0, sizeof(errcode_item_info_t));
    errcode_info->item_num = FEE_ERRCODE_INDEX_MAX;
    errcode_info->item[0].id = 0;
    errcode_info->item[0].addr = errcode_change_id_to_addr(0);
    errcode_info->item[0].item_type = ERRCODE_ITEM_CURRENT_ID;
    errcode_info->item[0].current_index = 1;

    errcode_full_t errcode_tmp = {0};
    errcode_tmp.user_data = errcode_info->item[0].current_index;
    ret = fee_write_eeprom(FEE_EEPROM_NAME(1), 0, (uint8_t*)&errcode_tmp, sizeof(errcode_tmp));
    if (ret) {
        printf("%s : fee_write_eeprom current_address error\n", __func__);
        goto Label;
    }
#else
    g_fee_write_address = 16;

    ret = fee_write_eeprom(FEE_EEPROM_NAME(1), 0, (uint8_t*)&g_fee_write_address, sizeof(g_fee_write_address));
    if (ret) {
        printf("%s : fee_write_eeprom current_address error\n", __func__);
    }
#endif

Label:

    return ret;
}

int errcode_ospi_init(void)
{
    int ret = 0;

    /* Prepare fee eeprom tree. */
    ret = fee_eeprom_errcode_prepare(&fee_eeprom1);
    if (ret != 0) {
        printf("%s : fee_eeprom_errcode_prepare failed.\n", __func__);
        goto Label;
    }

#if ERRCODE_NO_DUPLICATE_RECORD
    errcode_item_info_t *errcode_info = &g_errcode_item_info;
    int i = 0;
    int len = sizeof(errcode_full_t)*FEE_ERRCODE_INDEX_MAX;
    errcode_full_t *errcode_buf = (errcode_full_t *)malloc(len);
    if (!errcode_buf) {
        printf("%s : not enough memory for errcode.\n", __func__);
        goto Label;
    }

    errcode_full_t *errcode = errcode_buf;
    ret = fee_read(0, (uint8_t*)errcode, len);
    if (ret != 0) {
        printf("%s : fee_read failed.\n", __func__);
        goto Label;
    }

    errcode_info->item_num = FEE_ERRCODE_INDEX_MAX;
    for (i = 0; i < (int)ERRCODE_ARRAY_SIZE(errcode_info->item); i++) {
        errcode_info->item[i].id = i;
        errcode_info->item[i].addr = errcode_change_id_to_addr(i);
        if (i == 0) {
            errcode_info->item[i].item_type = ERRCODE_ITEM_CURRENT_ID;
            errcode_info->item[i].current_index = errcode->user_data;
        }
        else {
            errcode_info->item[i].item_type = ERRCODE_ITEM_ERROR_CODE;
            errcode_info->item[i].errcode = errcode->inter_errcode;
        }
        errcode++;
    }

    if (errcode_buf) {
        free(errcode_buf);
        errcode_buf = NULL;
    }

    printf("%s : Current errcode write addr = 0x%x\n", __func__, \
        errcode_change_id_to_addr(errcode_info->item[0].current_index));
#else
    uint32_t addr = 0;
    fee_read(0, (uint8_t*)&addr, sizeof(addr));
    if (ret != 0) {
        printf("%s : fee_read failed.\n", __func__);
        goto Label;
    }

    g_fee_write_address = addr;
    printf("%s : Current errcode write addr = 0x%x\n", __func__, addr);
#endif

Label:

    return ret;
}
