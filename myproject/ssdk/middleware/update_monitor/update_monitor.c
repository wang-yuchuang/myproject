/**
 * @file update_monitor.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * Update monitor is an update service that is for remote processor
 * updating local storage device(such as nor-flash) by rpmsg
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regs_base.h>
#include <irq_num.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <driver/scr.h>
#include <scr_hw.h>
#include <lib/cksum.h>
#include <rpmsg_channel.h>
#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include <rpmsg_service.h>
#include <mbox_cfg.h>
#include <ospi_cfg.h>
#include <rstgen_cfg.h>
#include <rstgen_cfg.h>
#include <scr_cfg.h>
#include <update_monitor_cfg.h>
#include <lib/sd_boot_pin/sd_boot_pin.h>
#include <lib/partition/partition_parser.h>
#include <lib/partition/ab_partition_parser.h>
#include <sd_errcode.h>
#include <driver/watchdog.h>
#include <driver/kunlun/watchdog/sdrv_watchdog_drv.h>
#include <arch/cache.h>
#include "update_monitor.h"
#include "update_monitor_diskio.h"

#define SUPPORT_PARALLEL_BOOT 0
#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif
#define GET_ALIGN_ADDR(addr, erase_size) (((addr) / (erase_size)) * (erase_size))
#define GET_REL_ADDR(addr, sect_size)    ((addr) % (sect_size))
#define STACK_SIZE 4096
#define RECIVE_NEW_EVENT 1

/* local update monitor pointers */
static update_monitor_dev_t *update_mon[UPDATE_MON_NUM] = {NULL};
static char disk_name[UPDATE_MONITOR_PTDEV_NAME_LEN]  = {0};
static char sub_ptbname[UPDATE_MONITOR_PTDEV_NAME_LEN]   = {0};
static char short_ptname[UPDATE_MONITOR_PTDEV_NAME_LEN]  = {0};
static char sub_ptbname_short_ptname[2 * UPDATE_MONITOR_PTDEV_NAME_LEN + 1]   = {0};
static uint8_t __attribute__((aligned(512)))
aligned_read_buffer[ALIGNED_BUFFER_SIZE];
static ptdev_index_t* ptdev_table[NUM_PARTITIONS] = {0};
static int um_ptdev_cnt = 0;
uint8_t secondary_partition_identifier[PARTITION_TYPE_GUID_SIZE] = \
{0x89, 0xa1, 0x06, 0x7d, 0xef, 0x1f, 0x89, 0x4d, 0x89, 0x40, 0xe8, 0xc2, 0xa6, 0x83, 0x2e, 0xcf};
ptdev_index_t base_ptdev_table;
struct disk_dev disk_inst = {0};


#if UPDATE_MONITOR_PRINT_MSG
static const char *ota_cmd_str[OTA_CMD_MAX] = {
    [OTA_CMD_START                      ] =  "OTA_CMD_START",
    [OTA_CMD_START_OK                   ] =  "OTA_CMD_START_OK",
    [OTA_CMD_START_FAIL                 ] =  "OTA_CMD_START_FAIL ",
    [OTA_CMD_SEEK                       ] =  "OTA_CMD_SEEK",
    [OTA_CMD_SEEK_OK                    ] =  "OTA_CMD_SEEK_OK",
    [OTA_CMD_SEEK_FAIL                  ] =  "OTA_CMD_SEEK_FAIL",
    [OTA_CMD_READ                       ] =  "OTA_CMD_READ",
    [OTA_CMD_READ_OK                    ] =  "OTA_CMD_READ_OK",
    [OTA_CMD_READ_FAIL                  ] =  "OTA_CMD_READ_FAIL",
    [OTA_CMD_WRITE                      ] =  "OTA_CMD_WRITE",
    [OTA_CMD_WRITE_OK                   ] =  "OTA_CMD_WRITE_OK",
    [OTA_CMD_WRITE_FAIL                 ] =  "OTA_CMD_WRITE_FAIL",
    [OTA_CMD_WRITE_INFO                 ] =  "OTA_CMD_WRITE_INFO",
    [OTA_CMD_WRITE_INFO_OK              ] =  "OTA_CMD_WRITE_INFO_OK",
    [OTA_CMD_WRITE_INFO_FAIL            ] =  "OTA_CMD_WRITE_INFO_FAIL",
    [OTA_CMD_GET_CAPACITY               ] =  "OTA_CMD_GET_CAPACITY",
    [OTA_CMD_GET_CAPACITY_OK            ] =  "OTA_CMD_GET_CAPACITY_OK",
    [OTA_CMD_GET_CAPACITY_FAIL          ] =  "OTA_CMD_GET_CAPACITY_FAIL ",
    [OTA_CMD_GET_BLOCK                  ] =  "OTA_CMD_GET_BLOCK",
    [OTA_CMD_GET_BLOCK_OK               ] =  "OTA_CMD_GET_BLOCK_OK",
    [OTA_CMD_GET_BLOCK_FAIL             ] =  "OTA_CMD_GET_BLOCK_FAIL",
    [OTA_CMD_GET_ERASESIZE              ] =  "OTA_CMD_GET_ERASESIZE",
    [OTA_CMD_GET_ERASESIZE_OK           ] =  "OTA_CMD_GET_ERASESIZE_OK",
    [OTA_CMD_GET_ERASESIZE_FAIL         ] =  "OTA_CMD_GET_ERASESIZE_FAIL",
    [OTA_CMD_COPY                       ] =  "OTA_CMD_COPY",
    [OTA_CMD_COPY_OK                    ] =  "OTA_CMD_COPY_OK",
    [OTA_CMD_COPY_FAIL                  ] =  "OTA_CMD_COPY_FAIL",
    [OTA_CMD_HANDOVER                   ] =  "OTA_CMD_HANDOVER",
    [OTA_CMD_HANDOVER_OK                ] =  "OTA_CMD_HANDOVER_OK",
    [OTA_CMD_HANDOVER_FAIL              ] =  "OTA_CMD_HANDOVER_FAIL",
    [OTA_CMD_CLOSE                      ] =  "OTA_CMD_CLOSE",
    [OTA_CMD_CLOSE_OK                   ] =  "OTA_CMD_CLOSE_OK",
    [OTA_CMD_CLOSE_FAIL                 ] =  "OTA_CMD_CLOSE_FAIL",
    [OTA_CMD_PRIORITY                   ] =  "OTA_CMD_PRIORITY",
    [OTA_CMD_PRIORITY_OK                ] =  "OTA_CMD_PRIORITY_OK",
    [OTA_CMD_PRIORITY_FAIL              ] =  "OTA_CMD_PRIORITY_FAIL",
    [OTA_CMD_STOP_WDT                   ] =  "OTA_CMD_STOP_WDT",
    [OTA_CMD_STOP_WDT_OK                ] =  "OTA_CMD_STOP_WDT_OK",
    [OTA_CMD_STOP_WDT_FAIL              ] =  "OTA_CMD_STOP_WDT_FAIL",
    [OTA_CMD_SETUP_PTDEV                ] =  "OTA_CMD_SETUP_PTDEV",
    [OTA_CMD_SETUP_PTDEV_OK             ] =  "OTA_CMD_SETUP_PTDEV_OK",
    [OTA_CMD_SETUP_PTDEV_FAIL           ] =  "OTA_CMD_SETUP_PTDEV_FAIL",
    [OTA_CMD_DESTROY_PTDEV              ] =  "OTA_CMD_DESTROY_PTDEV",
    [OTA_CMD_DESTROY_PTDEV_OK           ] =  "OTA_CMD_DESTROY_PTDEV_OK",
    [OTA_CMD_DESTROY_PTDEV_FAIL         ] =  "OTA_CMD_DESTROY_PTDEV_FAIL",
    [OTA_CMD_GET_PARTITION_INDEX        ] =  "OTA_CMD_GET_PARTITION_INDEX",
    [OTA_CMD_GET_PARTITION_INDEX_OK     ] =  "OTA_CMD_GET_PARTITION_INDEX_OK",
    [OTA_CMD_GET_PARTITION_INDEX_FAIL   ] =  "OTA_CMD_GET_PARTITION_INDEX_FAIL",
    [OTA_CMD_GET_PARTITION_SIZE         ] =  "OTA_CMD_GET_PARTITION_SIZE",
    [OTA_CMD_GET_PARTITION_SIZE_OK      ] =  "OTA_CMD_GET_PARTITION_SIZE_OK",
    [OTA_CMD_GET_PARTITION_SIZE_FAIL    ] =  "OTA_CMD_GET_PARTITION_SIZE_FAIL",
    [OTA_CMD_GET_PARTITION_OFFSET       ] =  "OTA_CMD_GET_PARTITION_OFFSET",
    [OTA_CMD_GET_PARTITION_OFFSET_OK    ] =  "OTA_CMD_GET_PARTITION_OFFSET_OK",
    [OTA_CMD_GET_PARTITION_OFFSET_FAIL  ] =  "OTA_CMD_GET_PARTITION_OFFSET_FAIL",
    [OTA_CMD_GET_NUMBER_SLOTS           ] =  "OTA_CMD_GET_NUMBER_SLOTS",
    [OTA_CMD_GET_NUMBER_SLOTS_OK        ] =  "OTA_CMD_GET_NUMBER_SLOTS_OK",
    [OTA_CMD_GET_NUMBER_SLOTS_FAIL      ] =  "OTA_CMD_GET_NUMBER_SLOTS_FAIL",
    [OTA_CMD_GET_CURRENT_SLOTS          ] =  "OTA_CMD_GET_CURRENT_SLOTS",
    [OTA_CMD_GET_CURRENT_SLOTS_OK       ] =  "OTA_CMD_GET_CURRENT_SLOTS_OK",
    [OTA_CMD_GET_CURRENT_SLOTS_FAIL     ] =  "OTA_CMD_GET_CURRENT_SLOTS_FAIL",
    [OTA_CMD_GET_SLOT_ATTR              ] =  "OTA_CMD_GET_SLOT_ATTR",
    [OTA_CMD_GET_SLOT_ATTR_OK           ] =  "OTA_CMD_GET_SLOT_ATTR_OK",
    [OTA_CMD_GET_SLOT_ATTR_FAIL         ] =  "OTA_CMD_GET_SLOT_ATTR_FAIL",
    [OTA_CMD_SET_SLOT_ATTR              ] =  "OTA_CMD_SET_SLOT_ATTR",
    [OTA_CMD_SET_SLOT_ATTR_OK           ] =  "OTA_CMD_SET_SLOT_ATTR_OK",
    [OTA_CMD_SET_SLOT_ATTR_FAIL         ] =  "OTA_CMD_SET_SLOT_ATTR_FAIL",
    [OTA_CMD_UPDATE_SLOT_ATTR           ] =  "OTA_CMD_UPDATE_SLOT_ATTR",
    [OTA_CMD_UPDATE_SLOT_ATTR_OK        ] =  "OTA_CMD_UPDATE_SLOT_ATTR_OK",
    [OTA_CMD_UPDATE_SLOT_ATTR_FAIL      ] =  "OTA_CMD_UPDATE_SLOT_ATTR_FAIL",
    [OTA_CMD_GET_NUMBER_PARTITION       ] =  "OTA_CMD_GET_NUMBER_PARTITION",
    [OTA_CMD_GET_NUMBER_PARTITION_OK    ] =  "OTA_CMD_GET_NUMBER_PARTITION_OK",
    [OTA_CMD_GET_NUMBER_PARTITION_FAIL  ] =  "OTA_CMD_GET_NUMBER_PARTITION_FAIL",
    [OTA_CMD_GET_PARTITION_INFO         ] =  "OTA_CMD_GET_PARTITION_INFO",
    [OTA_CMD_GET_PARTITION_INFO_OK      ] =  "OTA_CMD_GET_PARTITION_INFO_OK",
    [OTA_CMD_GET_PARTITION_INFO_FAIL    ] =  "OTA_CMD_GET_PARTITION_INFO_FAIL",
    [OTA_CMD_GET_BOOT_INFO              ] =  "OTA_CMD_GET_BOOT_INFO",
    [OTA_CMD_GET_BOOT_INFO_OK           ] =  "OTA_CMD_GET_BOOT_INFO_OK",
    [OTA_CMD_GET_BOOT_INFO_FAIL         ] =  "OTA_CMD_GET_BOOT_INFO_FAIL",
    [OTA_CMD_SEEK_MAPPED                ] =  "OTA_CMD_SEEK_MAPPED",
    [OTA_CMD_SEEK_MAPPED_OK             ] =  "OTA_CMD_SEEK_MAPPED_OK",
    [OTA_CMD_SEEK_MAPPED_FAIL           ] =  "OTA_CMD_SEEK_MAPPED_FAIL",
    [OTA_CMD_GET_CAPACITY_MAPPED        ] =  "OTA_CMD_GET_CAPACITY_MAPPED",
    [OTA_CMD_GET_CAPACITY_MAPPED_OK     ] =  "OTA_CMD_GET_CAPACITY_MAPPED_OK",
    [OTA_CMD_GET_CAPACITY_MAPPED_FAIL   ] =  "OTA_CMD_GET_CAPACITY_MAPPED_FAIL",
    [OTA_CMD_COPY_MAPPED                ] =  "OTA_CMD_COPY_MAPPED",
    [OTA_CMD_COPY_MAPPED_OK             ] =  "OTA_CMD_COPY_MAPPED_OK",
    [OTA_CMD_COPY_MAPPED_FAIL           ] =  "OTA_CMD_COPY_MAPPED_FAIL",
    [OTA_CMD_SETUP_MAPPED_STORAGE       ] =  "OTA_CMD_SETUP_MAPPED_STORAGE",
    [OTA_CMD_SETUP_MAPPED_STORAGE_OK    ] =  "OTA_CMD_SETUP_MAPPED_STORAGE_OK ",
    [OTA_CMD_SETUP_MAPPED_STORAGE_FAIL  ] =  "OTA_CMD_SETUP_MAPPED_STORAGE_FAIL",
};
#endif

/*
ptdev_index_t ptdev_table[] = {
    {"mmc1_top", NULL, false,  NULL,       NULL,   NULL, 0, 0},
    {"mmc1_ap1", NULL, true,  "mmc1_top", "ap1",   NULL, 0, 0},
    {"mmc1_ap2", NULL, true,  "mmc1_top", "ap2",   NULL, 0, 0},
};
*/

/**
 * @brief Set the total ptdev cnt
 * @param cnt
 */
static void set_ptdev_cnt(int cnt) {
    um_ptdev_cnt = cnt;
}

/**
 * @brief Get the ptdev cnt
 * @return int ptdev cnt
 */
static int get_ptdev_cnt(void) {
    return MIN(um_ptdev_cnt, NUM_PARTITIONS);
}

/**
 * @brief Get the ptdev name
 *
 * @param index ptdev index
 * @return const char* return ptdev name if success, return NULL if error.
 */
static const char *get_ptdev_name(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return NULL;
    }
    return ptdev_table[index]->ptdev_name;
}

/**
 * @brief Get the ptdev's pointer
 *
 * @param index ptdev index
 * @return partition_device_t** ptdev's pointer
 */
static partition_device_t **get_ptdev(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return NULL;
    }
    return &ptdev_table[index]->ptdev;
}

/**
 * @brief Determine whether ptdev is a Level 2 partition
 *
 * @param index ptdev index
 * @return true  a Level 2 partition
 * @return false a Level 1 partition
 */
static bool is_sub_ptdev(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return false;
    }
    return ptdev_table[index]->is_sub_ptdev;
}

/**
 * @brief Get the upper level ptdev name
 *
 * @param index ptdev index
 * @return const char* return ptdev name if success, return NULL if error.
 */
static const char *get_upper_level_ptdev_name(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return NULL;
    }
    return ptdev_table[index]->upper_level_ptdev_name;
}

/**
 * @brief Get the upper level partition name
 *
 * @param index partition index
 * @return const char* return partition name if success, return NULL if error.
 */
static const char *get_upper_level_partition_name(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return NULL;
    }
    return ptdev_table[index]->upper_level_partition_name;
}

/**
 * @brief Set the sub partition's offset
 *
 * @param index ptdev index
 * @param ptn sub partition's offset
 */
static void set_sub_partition_ptn(int index, uint64_t ptn)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return;
    }
    ptdev_table[index]->sub_partition_ptn = ptn;
}

/**
 * @brief Get the sub partition ptn object
 *
 * @param index ptdev index
 * @return uint64_t sub partition's offset
 */
static uint64_t get_sub_partition_ptn(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return 0;
    }
    return ptdev_table[index]->sub_partition_ptn;
}

/**
 * @brief Set the sub partition pt size
 *
 * @param index ptdev index
 * @param pt_size sub partition's offset
 */
static void set_sub_partition_pt_size(int index, uint64_t pt_size)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return;
    }
    ptdev_table[index]->sub_partition_pt_size = pt_size;
}

/**
 * @brief Get the sub partition pt size
 *
 * @param index ptdev index
 * @return uint64_t sub partition size
 */
static uint64_t get_sub_partition_pt_size(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return 0;
    }
    return ptdev_table[index]->sub_partition_pt_size;
}

/**
 * @brief Get the mapped storage from ptdev index
 *
 * @param index ptdev index
 * @return struct disk_dev* disk device pointer
 */
static struct disk_dev *get_mapped_storage(int index)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return NULL;
    }
    return ptdev_table[index]->mapped_storage;
}

/**
 * @brief Set the mapped storage for ptdev index
 *
 * @param index ptdev index
 * @param struct disk_dev* disk device pointer
 */
static void set_mapped_storage(int index, struct disk_dev* storage)
{
    if((index < 0) || (index >= get_ptdev_cnt()) \
         || !ptdev_table[index]) {
        return;
    }
    ptdev_table[index]->mapped_storage = storage;
}

/**
 * @brief Query whether ptdev is in the ptdev list
 *
 * @param ptdev_name ptdev name
 * @param index[out] ptdev index
 * @return int if in the list return 0, or return -1
 */
static int ptdev_name_check(const char *ptdev_name, int *index)
{
    int i = 0;

    if (!ptdev_name || !index) {
        return -1;
    }

    for (i = 0; i < get_ptdev_cnt(); i++) {
        if (!strcmp(get_ptdev_name(i), ptdev_name)) {
            *index = i;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief Query whether partition name is in the ptdev list
 *
 * @param ptdev_name partition name
 * @param index[out] ptdev index
 * @return int if in the list return 0, or return -1
 */
static int partition_name_check(const char *partition_name, int *index)
{
    int i = 0;

    if (!partition_name || !index) {
        return -1;
    }

    for (i = 0; i < get_ptdev_cnt(); i++) {
        if (!strcmp(get_upper_level_partition_name(i), partition_name)) {
            *index = i;
            return 0;
        }
    }

    return -1;
}

/**
 * @brief Query whether the upper-level partition table has been established
 *
 * @param index[in] ptdev index
 * @return int if upper-level partition table has been established return 0,
 *         or return -1
 */
static int check_upper_ptdev_ok(int index)
{
    int upper_level_ptdev_index;
    partition_device_t **p_ptdev = NULL;

    if (false == is_sub_ptdev(index)) {
        ERROR("%s is not a sub ptdev\n", get_ptdev_name(index));
        return -1;
    }

    if (!get_upper_level_ptdev_name(index)) {
        ERROR("%s's upper level ptdev name is not exist\n", get_ptdev_name(index));
        return -1;
    }

    if (ptdev_name_check(get_upper_level_ptdev_name(index),
                         &upper_level_ptdev_index)) {
        ERROR("%s's upper level ptdev name %s check error, index = %d\n", get_ptdev_name(index),
              get_upper_level_ptdev_name(index), index);
        return -1;
    }

    p_ptdev = get_ptdev(upper_level_ptdev_index);

    if (*p_ptdev == NULL) {
        return 0;
    }
    else {
        return 1;
    }
}

/**
 * @brief get an short value from a buffer
 * @param[in] data pointer to a data buffer
 * @param[in] offset pointer to the offset
 * @return the unsigned int value
 */
static uint16_t getUint16(uint8_t *data, int *offset)
{
    uint16_t value;
    value = data[(*offset)++];
    value |= data[(*offset)++] << 8;
    return value;
}

/**
 * @brief get an unsigned int value from a buffer
 * @param[in] data pointer to a data buffer
 * @param[in] offset pointer to the offset
 * @return the unsigned int value
 */
static uint32_t getUint32(uint8_t *data, int *offset)
{
    uint32_t value;
    uint32_t tmp = getUint16(data, offset);
    value = getUint16(data, offset);
    value <<= 16;
    value += tmp;
    return value;
}

/**
 * @brief get an unsigned long value from a buffer
 * @param[in] data pointer to a data buffer
 * @param[in] offset pointer to the offset
 * @return the unsigned long value
 */
static uint64_t getUint64(uint8_t *data, int *offset)
{
    uint64_t value;
    uint64_t tmp = getUint32(data, offset);
    value = getUint32(data, offset);
    value <<= 32;
    value += tmp;
    return value;
}

/**
 * @brief print a message for debug
 * @param[in] ptr message buffer
 * @param[in] len length of message buffer
 */
static void msg_printer(const void *ptr, size_t len)
{
    unsigned long address = (unsigned long)ptr;
    size_t count;
    size_t i;

    for (count = 0 ; count < len; count += 16) {
        for (i = 0; i < MIN(len - count, 16); i++) {
            ssdk_printf(SSDK_CRIT, "%02hhx ", *(const uint8_t *)(address + i));
        }

        ssdk_printf(SSDK_CRIT, "\n");
        address += 16;
    }
}

/**
 * @brief a small buffer which is aligned to block size is used to read
 *
 * @param dev disk device
 * @param addr read addr
 * @param dst read buffer
 * @param read_len read size
 * @return int 0 is success, other is failed
 */
static int aligned_read(struct disk_dev* dev, uint64_t addr,
                        uint8_t *dst, uint64_t read_len)
{
    int ret = -1;
    uint64_t cnt;
    uint64_t i = 0;
    uint64_t total_read_size = 0;
    uint64_t read_addr = addr;
    uint64_t align_addr;
    uint64_t unalign_size;
    uint64_t copy_size;

    cnt = ((read_addr + read_len - 1) / ALIGNED_BUFFER_SIZE) -
          (read_addr / ALIGNED_BUFFER_SIZE) + 1;
#if UPDATE_MONITOR_PRINT_MSG
    ERROR("aligned read addr = 0x%llx\n", read_addr);
    ERROR("aligned read read_len = 0x%llx\n", read_len);
    ERROR("aligned read cnt = %lld\n", cnt);
#endif

    ASSERT(cnt >= 1);

    for (i = 0; i < cnt; i++) {
        align_addr = GET_ALIGN_ADDR(read_addr, ALIGNED_BUFFER_SIZE);

#if UPDATE_MONITOR_PRINT_MSG
        ERROR("aligned read cnt = %lld, read align_addr =0x%llx, for length 0x%x \n",
              cnt, align_addr, ALIGNED_BUFFER_SIZE);
#endif
        ret = um_disk_read(dev, align_addr, aligned_read_buffer,
                            ALIGNED_BUFFER_SIZE);

        if (ret) {
            ERROR("read error, ret=%d\n", ret);
            goto end;
        }

        unalign_size = GET_REL_ADDR(read_addr, ALIGNED_BUFFER_SIZE);

        if (1 == cnt) {
            copy_size = read_len;
        }
        else if (cnt == (i + 1)) {
            ASSERT(read_len > total_read_size);
            copy_size = read_len - total_read_size;
        }
        else {
            ASSERT(ALIGNED_BUFFER_SIZE > unalign_size);
            copy_size = (ALIGNED_BUFFER_SIZE - unalign_size);
        }

        memcpy(dst + total_read_size, aligned_read_buffer + unalign_size, copy_size);

#if UPDATE_MONITOR_PRINT_MSG
        ERROR("copy data (size = 0x%llx) from (aligned buffer + 0x%llx) to (dst addr + 0x%llx) \n",
              copy_size, unalign_size, total_read_size);
#endif

        total_read_size += copy_size;
        read_addr += copy_size;

#if UPDATE_MONITOR_PRINT_MSG
        ERROR("aligned read total_read_size = 0x%llx, the new read_addr = 0x%llx\n",
              total_read_size, read_addr);
#endif

    }

    ret = 0;

end:
    return ret;
}

/**
 * @brief a big buffer (shared memory) that is aligned to block size is used to read
 *
 * @param dev disk device
 * @param addr read addr
 * @param dst read buffer
 * @param read_len read size
 * @return int 0 is success, other is failed
 */
static int aligned_read_big_buffer(struct disk_dev* storage,
                                   uint64_t addr, uint8_t *dst, uint64_t read_len)
{
    /* read from emmc */
    int ret = -1;
    uint64_t cnt;
    uint64_t i = 0;
    uint64_t total_read_size = 0;
    uint64_t read_addr = addr;
    uint64_t align_addr;
    uint64_t unalign_size;
    uint64_t copy_size;

    cnt = ((read_addr + read_len - 1) / ALIGNED_BUFFER_SIZE) -
          (read_addr / ALIGNED_BUFFER_SIZE) + 1;
    ASSERT(cnt >= 1);

    if (cnt < 3) {
        /* The amount of data is too small, directly use a small buffer to read */
        ret =  aligned_read(storage, addr, dst, read_len);
        goto end;
    }
    else {
        i = 0;

        while (i < cnt) {
            align_addr = GET_ALIGN_ADDR(read_addr, ALIGNED_BUFFER_SIZE);
            unalign_size = GET_REL_ADDR(read_addr, ALIGNED_BUFFER_SIZE);

            if (0 == i) {
                /* first block size */
                ASSERT(0 == total_read_size);
                ASSERT(ALIGNED_BUFFER_SIZE > unalign_size);
                copy_size = (ALIGNED_BUFFER_SIZE - unalign_size);
                ret = um_disk_read(storage, align_addr, aligned_read_buffer,
                                    ALIGNED_BUFFER_SIZE);

                if (ret) {
                    ERROR("storage read error, ret=%d\n", ret);
                    goto end;
                }

                memcpy(dst + total_read_size, aligned_read_buffer + unalign_size, copy_size);
                i++;
            }
            else if ((cnt - 1) == i) {
                /* last block size */
                copy_size = read_len - total_read_size;
                ASSERT((total_read_size + copy_size) == read_len);
                ret = um_disk_read(storage, align_addr, aligned_read_buffer,
                                    ALIGNED_BUFFER_SIZE);

                if (ret) {
                    ERROR("storage read error, ret=%d\n", ret);
                    goto end;
                }

                memcpy(dst + total_read_size, aligned_read_buffer + unalign_size, copy_size);
                i++;
            }
            else {
                /* data in middle, don't need copy */
                copy_size = (cnt - 2) * ALIGNED_BUFFER_SIZE;
                ASSERT(i == 1);
                ASSERT(unalign_size == 0);
                ASSERT(read_addr == align_addr);
                ASSERT((total_read_size + copy_size) <= read_len);
                ret = um_disk_read(storage, align_addr, dst + total_read_size, copy_size);

                if (ret) {
                    ERROR("storage read error, ret=%d\n", ret);
                    goto end;
                }

                i = i + (cnt - 2);
            }

            total_read_size += copy_size;
            read_addr += copy_size;
        }

        ret = 0;
    }

end:
    return ret;
}

/**
 * @brief calloc the wirte buffer
 * @param dev pointer to an update monitor instance
 */
static void update_dev_calloc(update_monitor_dev_t *dev)
{
    ASSERT(dev->block_size > 0);
    ASSERT((dev->block_size % 512) == 0);

    if (dev->write_buf_flag == 0) {
#if CONFIG_UPDATE_FLASH
        /* For flash, the size of the buffer is erase_size. Since the erased object of
        the flash is a sector, that is, erase_size, it is necessary to perform a readback
        operation before the write operation. The size of the readback is the size of the
        buffer, which is erase_size
        */
        dev->write_buf = (uint8_t *)osAllocAlign(dev->block_size, dev->erase_size);
#elif CONFIG_UPDATE_EMMC
        #error ssdk update monitor could not build for a non-safety config
        /* For emmc, we need to ensure that the write address and write length are block_size
        aligned, and the write buffer should be cache line aligned */
        dev->write_buf = (uint8_t *)osAllocAlign(dev->block_size, ALIGNED_BUFFER_SIZE);
#endif
    }

    if(dev->write_buf) {
        dev->write_buf_flag = 1;
    }
}

/**
 * @brief free the wirte buffer
 * @param dev pointer to an update monitor instance
 */
static void update_dev_free(update_monitor_dev_t *dev)
{
    if (dev->write_buf_flag != 0) {
        free(dev->write_buf);
    }

    dev->write_buf_flag = 0;
}

#if CONFIG_HANDOVER_OSPI1
/**
 * @brief reset ospi1
 * @return 0:success other:fail
 */
static int ospi1_reset(void)
{
    return reset_ctl_reset(&rstsig_ospi1);
}

/**
 * @brief handover ospi1
 */
static void ospi1_handover(void)
{
#ifdef CONFIG_BOARD_X9SP
    scr_signal_t apbmux2_ospi1_select = SCR_SAFETY__scr_saf_apbmux2_ospi1_select;
    scr_signal_t axislvmux_ospi1_select = SCR_SAFETY__scr_saf_axislvmux_ospi1_select;
#else  /* !CONFIG_BOARD_X9SP */
    scr_signal_t apbmux2_ospi1_select = SCR_SAFETY__apbmux2_ospi1_select;
    scr_signal_t axislvmux_ospi1_select = SCR_SAFETY__axislvmux_ospi1_select;
#endif /* end CONFIG_BOARD_X9SP */
    scr_set(&g_scr_safety, &apbmux2_ospi1_select, 1);
    scr_set(&g_scr_safety, &axislvmux_ospi1_select, 1);
}

/**
 * @brief reset ospi1 and handover to remote prossessor
 * @return 0:success other:fail
 */
static int ospi1_reset_and_handover(void)
{
    static bool handover_done = 0;
    int ret;

    if (0 == handover_done) {
        /* reset OSPI1 */
        ret = ospi1_reset();

        if (ret) {
            ERROR("ospi1 reset fail \n");
            return -1;
        }

        /* Handover OSPI */
        ospi1_handover();
        handover_done = 1;
    }

    return 0;
}
#endif

/**
 * @brief free the disk for update monitor
 *
 * @param dev pointer to an update monitor instance
 * @return int return0
 */
static int update_monitor_free_storage(update_monitor_dev_t *dev)
{
    int ret = 0;
    dev->load_storage = 0;
    return ret;
}

/**
 * @brief init a disk for a update monitor
 *
 * @param dev pointer to an update monitor instance
 * @return int 0 is success, other is failed
 */
static int update_monitor_load_storage(update_monitor_dev_t *dev)
{
    if(dev->mode == MODE_NEED_UPDATE) {
        if (0 == dev->load_storage) {
            if (NULL == dev->storage) {
                ERROR("failed to load storage\n");
                return -1;
            }

            dev->capacity = um_get_capacity(dev->storage);
            dev->erase_size = um_get_erase_size(dev->storage);
            dev->block_size = um_get_block_size(dev->storage);

            if ((0 == dev->capacity) || (0 == dev->erase_size) || (0 == dev->block_size)) {
                ERROR("update monitor init: failed to load info from storage\n");
                return -1;
            }

            /*load storage OK*/
            dev->load_storage = 1;
        }
    }
    else {
        dev->capacity = CAPACITY_DON_CARE_VALUE;
        dev->erase_size = SECTOR_DON_CARE_VALUE;
        dev->block_size = BLOCK_DON_CARE_VALUE;
    }

    return 0;
}

/**
 * @brief reply the rpmsg to remote prosessor
 * @param[in] monitor pointer to an update monitor instance
 * @param[in] reply a reply message including command and data length
 * @return 0:success other:fail
 */
static int update_msg_sender(update_monitor_dev_t *dev)
{
    update_msg_head_t   *send_head;
    update_cmd_enum cmd = dev->reply.reply_cmd;
    uint16_t data_len = dev->reply.reply_data_len;
    int ret = -1;
    uint16_t total_len;

    if ( data_len > MAX_DATA_LENGTH) {
        ERROR("length error, data_len =%d\n",
              data_len);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_SEND_INVALID_LEN, 0);
        return -1;
    }

    total_len = data_len + MSG_HEAD_SIZE;
    send_head = dev->send_head;
    /*clean the buffer*/
    memset(dev->send_msg, 0, MSG_HEAD_SIZE);

    /* head */
    send_head->flag1    =   OTA_START_MAGIC;
    send_head->len      =   data_len;
    send_head->cmd      =   cmd;
    send_head->crc      =   0;
    send_head->flag2    =   OTA_END_MAGIC;

    /* data is already put in the send_msg */

    /* crc */
    send_head->crc      =   crc32(0, dev->send_msg, total_len);

#if UPDATE_MONITOR_PRINT_MSG
    ERROR("recv_head->len = %s\n", ota_cmd_str[cmd]);
    ERROR("sendmsg:\n");
    msg_printer(dev->send_msg, total_len);
#endif
    /* send msg */
    ret = rpmsg_channel_send(dev->rpmsg_chan, UPDATE_MONITOR_EPT,
                             (void *)dev->send_msg,
                             total_len, 1000);

    if (ret) {
        ERROR("update monitor msg_sender: send data error, ret=%d\n", ret);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_SEND_DATA_FAILED, 0);
        return -1;
    }

    return 0;
}

/**
 * @brief check the format of recived message
 * @param[in] dev pointer to an update monitor instance
 * @return 0:success other:fail
 */
static int update_msg_checker(update_monitor_dev_t *dev)
{
    uint32_t crc_val;
    update_msg_head_t *recv_head =  dev->recv_head;

    if (recv_head->len > MAX_DATA_LENGTH) {
        ERROR("recv msg len error");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_RECV_INVALID_LEN, 0);
        return -1;
    }

    if (recv_head->flag1 != OTA_START_MAGIC) {
        ERROR("recv msg flag1 is 0x%04x, expect 0x%04x \n",
              recv_head->flag1, OTA_START_MAGIC);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_RECV_INVALID_FLG1, 0);
        return -1;
    }

    if (recv_head->flag2 != OTA_END_MAGIC) {
        ERROR("recv msg flag2 is 0x%04x, expect 0x%04x \n",
              recv_head->flag2, OTA_END_MAGIC);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_RECV_INVALID_FLG2, 0);
        return -1;
    }

    crc_val = recv_head->crc;
    recv_head->crc = 0;
    recv_head->crc = crc32(0, dev->recv_msg, (recv_head->len + MSG_HEAD_SIZE));

    if (crc_val != recv_head->crc) {
        ERROR("recv msg crc = 0x%08x error, expect 0x%08x \n",
              crc_val, recv_head->crc);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_RECV_CRC_FAILED, 0);
        return -1;
    }

#if UPDATE_MONITOR_PRINT_MSG
    ERROR("recv_head->len = %d\n", recv_head->len);

    if (recv_head->cmd < OTA_CMD_MAX) {
        ERROR("recv_head->cmd = %s\n", ota_cmd_str[recv_head->cmd]);
    }
    else {
        ERROR("recv_head->cmd error\n");
    }

#endif

    return 0;
}

/**
 * @brief handle the "handover" cammand from remote processer
 * @param[in] monitor pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_handover(update_monitor_dev_t *dev)
{
#if !CONFIG_HANDOVER_OSPI1
    dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_FAIL, 0};
    DBG("do not support ospi handover\n");
    return OTA_ERR_NO_ERR;
#else
    int ret = 0;
    ret = ospi1_reset_and_handover();

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_FAIL, 0};
        ERROR("update_msg_handover error\n");
        return OTA_ERR_HANDOVER_OSPI_FAILED;
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_HANDOVER_OK, 0};
        DBG("update_msg_handover done\n");
        return OTA_ERR_NO_ERR;
    }
#endif
}

/**
 * @brief handle the "close" cammand from remote processer
 * @param[in] monitor pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_close(update_monitor_dev_t *dev)
{
    int ret = -1;
    ret =  update_monitor_free_storage(dev);

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_CLOSE_FAIL, 0};
        ERROR("load storage failed, start error\n");
        return OTA_ERR_STORAGE_FREE_FAILED;
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_CLOSE_OK, 0};
        return OTA_ERR_NO_ERR;
    }
}

/**
 * @brief setup partition table
 *
 * @param index ptdev_index
 * @param storage disk device
 * @param force_read force read regardless of whether it has already been read
 * @return int 0 is success, other is failed
 */
static int update_monitor_ptdev_setup(int index, struct disk_dev* storage,
                                      bool force_read)
{
    int ret = -1;
    partition_device_t **p_ptdev = get_ptdev(index);

    if ((index >= get_ptdev_cnt()) || (index < 0) || !p_ptdev) {
        ERROR("index error\n");
        ret = OTA_ERR_INVALID_PART_INDEX;
        goto end;
    }

    if (!storage) {
        ERROR("storage is null\n");
        ret = OTA_ERR_STORAGE_DEV_NULL;
        goto end;
    }

    DBG("read partition %s\n", get_ptdev_name(index));

    if (NULL != *p_ptdev) {
        DBG("re-read partition %s\n", get_ptdev_name(index));

        if (!force_read || !ptdev_read_table(*p_ptdev)) {
            ret = OTA_ERR_NO_ERR;
            goto end;
        }

        DBG("re-read partition error\n");
        ptdev_destroy(*p_ptdev);
        *p_ptdev = NULL;
    }

#if CONFIG_UPDATE_FLASH

    if (is_sub_ptdev(index)) {
        *p_ptdev = ptdev_setup(storage, 0);
    }
    else {
        *p_ptdev = ptdev_setup(storage, um_get_erase_size(storage) * 2);
    }

#elif CONFIG_UPDATE_EMMC
    #error ssdk update monitor could not build for a non-safety config
    *p_ptdev = ptdev_setup(storage, 0);
#endif

    if (!*p_ptdev) {
        ERROR("ptdev setup error\n");
        ret = OTA_ERR_PART_DEV_NULL;
        goto end;
    }

    if (ptdev_read_table(*p_ptdev)) {
        ERROR("read partition error!\n");
        ret = OTA_ERR_PART_READ_TABLE_FAILED;
        goto end;
    }

#if UPDATE_MONITOR_PRINT_MSG
    ptdev_dump(*p_ptdev);
#endif
    ret = OTA_ERR_NO_ERR;

end:

    if (ret && *p_ptdev) {
        ptdev_destroy(*p_ptdev);
        *p_ptdev = NULL;
    }

    return ret;
}

/**
 * @brief Set the up mapped disk object
 *
 * @param config input para
 * @param mapped_info output mapped disk info
 * @param mapped_storage output disk device
 * @return int
 */
static int setup_mapped_disk(struct disk_mapped_config* config, struct disk_mapped_info** mapped_info_out, struct disk_dev** mapped_storage_out)
{
    struct disk_mapped_info * mapped_info = NULL;
    struct disk_dev *mapped_storage = NULL;

    mapped_info = (struct disk_mapped_info *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, sizeof(struct disk_mapped_info));
    if(!mapped_info) {
        ERROR("calloc for %s mapped info failed \n", config->disk_name);
        return -1;
    }
    memset(mapped_info, 0 , sizeof(struct disk_mapped_info));

    mapped_storage = (struct disk_dev *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, sizeof(struct disk_dev));
    if(!mapped_storage) {
        ERROR("calloc for %s mapped dev failed \n", config->disk_name);
        free(mapped_info);
        return -1;
    }

    memset(mapped_storage, 0 , sizeof(struct disk_dev));
    mapped_info->config.disk_name = config->disk_name;
    mapped_info->config.disk_offset = config->disk_offset;
    mapped_info->config.disk_size = config->disk_size;
    mapped_info->config.base_disk_info = config->base_disk_info;

    if(register_mapped_disk(mapped_info)) {
        ERROR("register for %s mapped dev failed \n", config->disk_name);
        free(mapped_info);
        free(mapped_storage);
        return -1;
    }

    if(um_disk_init(config->disk_name, mapped_storage)) {
        ERROR("open %s mapped dev failed \n", config->disk_name);
        free(mapped_info);
        free(mapped_storage);
        return -1;
    }

    *mapped_info_out = mapped_info;
    *mapped_storage_out = mapped_storage;

    return 0;
}

/**
 * @brief destroy a mapped disk device
 *
 * @param mapped_info mapped information
 * @param mapped_storage mapped disk device
 */
static void destroy_mapped_disk(struct disk_mapped_info* mapped_info, struct disk_dev* mapped_storage)
{
    if(mapped_storage) {
        um_disk_deinit(mapped_storage);
        free(mapped_storage);
    }

    if(mapped_info) {
        unregister_mapped_disk(mapped_info);
        free(mapped_info);
    }
}

/**
 * @brief setup partition table for mapped storage
 *
 * @param index ptdev index
 * @param base_storage level-one partition table
 * @return int 0 is success, other is failed
 */
static int update_monitor_mapped_storage_setup(int index,
        struct disk_dev *base_storage)
{
    int ret = -1;
    int upper_level_ptdev_exits = -1;
    uint64_t ptn = 0;
    uint64_t pt_size = 0;
    struct disk_dev *mapped_storage = NULL;
    struct disk_mapped_info * mapped_info = NULL;
    struct disk_mapped_config config = {0};

    partition_device_t **p_upper_level_ptdev = NULL;
    const char *parent = NULL;
    int upper_level_ptdev_index = 0;
    upper_level_ptdev_exits = check_upper_ptdev_ok(index);

    if (!base_storage) {
        ERROR("base storage is NULL\n");
        ret = OTA_ERR_STORAGE_DEV_NULL;
        goto out;
    }

    if (get_mapped_storage(index)) {
        DBG("get mapped storage\n");
        ret = OTA_ERR_NO_ERR;
        goto out;
        /*
            DBG("destroy mapped storage\n");
            storage_dev_destroy(get_mapped_storage(index));
            set_mapped_storage(index, NULL);
        */
    }
    DBG("setup new mapped storage\n");
    if (upper_level_ptdev_exits < 0) {
        ERROR("check upper ptdev error\n");
    }
    else if (0 == upper_level_ptdev_exits) {
        if (ptdev_name_check(get_upper_level_ptdev_name(index),
                             &upper_level_ptdev_index)) {
            ERROR("%s's upper level ptdev name %s check error\n", get_ptdev_name(index),
                  get_upper_level_ptdev_name(index));
            ret = OTA_ERR_PART_CHK_FAILED;
            goto out;
        }

        ret = update_monitor_ptdev_setup(upper_level_ptdev_index, base_storage,
                                         DO_NOT_UPDATE);

        if (ret != OTA_ERR_NO_ERR) {
            ERROR("setup upper level ptdev %s error\n", get_upper_level_ptdev_name(index));
            goto out;
        }
        DBG("update_monitor_ptdev_setup OK\n");
    }

    p_upper_level_ptdev = get_ptdev(upper_level_ptdev_index);
    parent  = get_upper_level_partition_name(index);

    //read level 1 partition
    ptn = ptdev_get_offset(*p_upper_level_ptdev, parent);
    pt_size = ptdev_get_size(*p_upper_level_ptdev, parent);

    if (!ptn || !pt_size || !parent) {
        ERROR("Fail to get partition: %s info!\n", parent);
        ret = OTA_ERR_INVALID_PART_INFO;
        goto out;
    }

    config.disk_name = parent;
    config.disk_offset = ptn;
    config.disk_size = pt_size;
    config.base_disk_info = base_storage->info;
    if(setup_mapped_disk(&config, &mapped_info, &mapped_storage)) {
        ERROR("Fail to setup mapped storage\n");
        ret = OTA_ERR_STORAGE_DEV_MAPPED_NULL;
        goto out;
    }

    set_sub_partition_ptn(index, ptn);
    set_sub_partition_pt_size(index, pt_size);
    set_mapped_storage(index, mapped_storage);
    DBG("mapped_dev index = %d\n", index);
    DBG("mapped_storage=%p\n", mapped_storage);
    ret = OTA_ERR_NO_ERR;

out:

    if (ret) {
        destroy_mapped_disk(mapped_info, mapped_storage);
        set_sub_partition_ptn(index, 0);
        set_sub_partition_pt_size(index, 0);
    }

    return ret;
}

/**
 * @brief update monitor msg to setup mpped disk device
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_setup_mapped_storage(update_monitor_dev_t *dev)
{
    int ret = -1;
    int index = -1;

    const char *partition_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN)) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    DBG("setup mapped storage for %s\n", partition_name);

    if (!partition_name_check(partition_name, &index)) {
        if ((index >= get_ptdev_cnt()) || (index < 0)) {
            ERROR("index error\n");
            ret = OTA_ERR_INVALID_PART_INDEX;
            goto end;
        }
        DBG("index =%d\n", index);
        ret = update_monitor_mapped_storage_setup(index, dev->storage);

        if (ret != OTA_ERR_NO_ERR) {
            ERROR("setup mapped storage %s error\n", partition_name);
        }
    }
    else {
        ERROR("setup mapped storage for %s error \n", partition_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_SETUP_MAPPED_STORAGE_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_SETUP_MAPPED_STORAGE_OK, 0};
    }

    return ret;
}

/**
 * @brief update monitor msg to get boot information
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR if success, other is failed
 */
static inline int update_msg_get_boot_info(update_monitor_dev_t *dev)
{
    if(strstr(dev->boot_info, BOOT_INFO_PREFIX)) {
        DBG("boot info = %s\n", dev->boot_info);
        memcpy((uint8_t *)(dev->send_msg + MSG_HEAD_SIZE), dev->boot_info, MAX_DATA_LENGTH);
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BOOT_INFO_OK, MAX_DATA_LENGTH};
        return OTA_ERR_NO_ERR;
    }
    else {
        ERROR("boot info = %s\n", dev->boot_info);
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BOOT_INFO_FAIL, 0};
        return OTA_ERR_INVALID_BOOT_INFO;
    }
}

/**
 * @brief update monitor msg to setup ptdev
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR if success, other is failed
 */
static inline int update_msg_ptdev_setup(update_monitor_dev_t *dev)
{
    int ret = OTA_ERR_UNKNOWN_ERROR;
    int index = -1;
    struct disk_dev* storage = NULL;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    DBG("setup ptdev for %s\n", ptdev_name);

    if (!ptdev_name_check(ptdev_name, &index)) {
        if ((index >= get_ptdev_cnt()) || (index < 0)) {
            ERROR("index error\n");
            ret = OTA_ERR_INVALID_PART_INDEX;
            goto end;
        }

        if (is_sub_ptdev(index)) {
            DBG("%s is sub ptdev\n", ptdev_name);
            update_monitor_mapped_storage_setup(index, dev->storage);
            storage = get_mapped_storage(index);
        }
        else {
            storage = dev->storage;
        }

        ret = update_monitor_ptdev_setup(index, storage, DO_UPDATE);

        if (ret != OTA_ERR_NO_ERR) {
            ERROR("ptdev %s setup error\n", get_ptdev_name(index));
        }
    }
    else {
        ERROR("ptdev name error %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_SETUP_PTDEV_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_SETUP_PTDEV_OK, 0};
    }

    return ret;
}

/**
 * @brief update monitor msg to destroy ptdev
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR if success, other is failed
 */
static inline int update_msg_ptdev_destroy(update_monitor_dev_t *dev)
{
    int ret = -1;
    int index = -1;
    partition_device_t **p_ptdev;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            ptdev_destroy(*p_ptdev);
            *p_ptdev = NULL;
            ret = OTA_ERR_NO_ERR;
            DBG("ptdev destroy ok\n");
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev name error %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_DESTROY_PTDEV_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_DESTROY_PTDEV_OK, 0};
    }

    return ret;
}

/**
 * @brief parse partition name
 *
 * @param ptdev_name pointer to an ptdev_name
 * @return int 0 is success, other is failed
 */
static int get_partition_name(const char *ptdev_name)
{
    int token_num = 0;
    char *token1 = NULL;
    char *token2 = NULL;
    ASSERT(MAX_DATA_LENGTH > strlen(ptdev_name));

    for (uint32_t i = 0; i < strlen(ptdev_name); i++) {
        if (ptdev_name[i] == '$') {
            token_num++;
        }
    }

    /* full name:xxx$yyy$zzz */
    if (token_num != 2) {
        ERROR("partition name = [%s] format error, should be xxx$$zzz\n", ptdev_name);
        return -1;
    }

    token1 = strchr(ptdev_name, '$');
    token2 = strrchr(ptdev_name, '$');

    if ((token1 - ptdev_name) >= UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("disk_name too long\n");
    }

    memset(disk_name, 0, (UPDATE_MONITOR_PTDEV_NAME_LEN));
    strncpy(disk_name, ptdev_name, token1 - ptdev_name);

    if ((token2 - token1 - 1) >= UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("sub_ptbname too long\n");
        return -1;
    }

    memset(sub_ptbname, 0, (UPDATE_MONITOR_PTDEV_NAME_LEN));
    strncpy(sub_ptbname, token1 + 1, token2 - token1 - 1);

    if ((strlen(ptdev_name) - (token2 + 1 - ptdev_name)) >=
            UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("sub_ptbname too long\n");
        return -1;
    }

    memset(short_ptname, 0, (UPDATE_MONITOR_PTDEV_NAME_LEN));
    strncpy(short_ptname, token2 + 1,
            strlen(ptdev_name) - (token2 + 1 - ptdev_name));

    DBG("disk_name:%s sub_ptbname:%s sub_pt:%s\n", disk_name, sub_ptbname,
        short_ptname);

    if (0 != strlen(sub_ptbname)) {
        snprintf(sub_ptbname_short_ptname, 2 * UPDATE_MONITOR_PTDEV_NAME_LEN + 1,
                 "%s$%s", sub_ptbname, short_ptname);
    }

    return 0;
}

/**
 * @brief update monitor msg to get partition index
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_partition_index(update_monitor_dev_t *dev)
{
    int ret = -1;
    int32_t index = -1;
    int ptdev_index = -1;
    partition_device_t **p_ptdev = NULL;

    const char *full_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);
    int max_len = 3 * UPDATE_MONITOR_PTDEV_NAME_LEN;

    if (dev->recv_head->len > max_len) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (get_partition_name(full_name)) {
        ERROR("update get partition index error get_partition_name error\n");
        ret = OTA_ERR_PART_NAME_NULL;
        goto end;
    }

    if (!ptdev_name_check(disk_name, &ptdev_index)) {
        p_ptdev = get_ptdev(ptdev_index);

        if (p_ptdev && *p_ptdev) {
            if (0 == strlen(sub_ptbname)) {
                index = ptdev_get_index(*p_ptdev, short_ptname);
                DBG("ptdev %s : partition index for %s is %d \n", get_ptdev_name(ptdev_index),
                    short_ptname, index);
            }
            else {
                index = ptdev_get_index(*p_ptdev, sub_ptbname_short_ptname);
                DBG("ptdev %s : partition sub index for %s is %d \n",
                    get_ptdev_name(ptdev_index), short_ptname, index);
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("update get partition index error, ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("get partition index for %s error, table name is %s\n", full_name,
              disk_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_INDEX_FAIL, 0};
    }
    else {
        *(int32_t *)(dev->send_msg + MSG_HEAD_SIZE) = index;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_INDEX_OK, sizeof(int32_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get partition size
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_partition_size(update_monitor_dev_t *dev)
{
    int ret = -1;
    uint64_t size = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;

    const char *full_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);
    int max_len = 3 * UPDATE_MONITOR_PTDEV_NAME_LEN;

    if (dev->recv_head->len > max_len) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (get_partition_name(full_name)) {
        ERROR("get_partition_name error\n");
        ret = OTA_ERR_PART_NAME_NULL;
        goto end;
    }

    if (!ptdev_name_check(disk_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            if (0 == strlen(sub_ptbname)) {
                size = ptdev_get_size(*p_ptdev, short_ptname);
            }
            else {
                size = ptdev_get_size(*p_ptdev, sub_ptbname_short_ptname);
            }

            if (0 == size) {
                ERROR("get partition size is 0, error\n");
                ret = OTA_ERR_PART_GET_SZ_FAILED;
                goto end;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("get partition size for %s error, table name is %s\n", full_name,
              disk_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_SIZE_FAIL, 0};
    }
    else {
        *(uint64_t *)(dev->send_msg + MSG_HEAD_SIZE) = size;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_SIZE_OK, sizeof(uint64_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get partition offset
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_partition_offset(update_monitor_dev_t *dev)
{
    int ret = -1;
    uint64_t offset = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;

    const char *full_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);
    int max_len = 3 * UPDATE_MONITOR_PTDEV_NAME_LEN;

    if (dev->recv_head->len > max_len) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (get_partition_name(full_name)) {
        ERROR("get_partition_name error\n");
        ret = OTA_ERR_PART_NAME_NULL;
        goto end;
    }

    if (!ptdev_name_check(disk_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            if (0 == strlen(sub_ptbname)) {
                offset = ptdev_get_offset(*p_ptdev, short_ptname);
            }
            else {
                offset = ptdev_get_offset(*p_ptdev, sub_ptbname_short_ptname);
            }

            if (0 == offset) {
                ERROR("update get partition offset is 0, error\n");
                ret = OTA_ERR_PART_GET_OFF_FAILED;
                goto end;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("get partition offet for %s error, table name is %s\n", full_name,
              disk_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_OFFSET_FAIL, 0};
    }
    else {
        *(uint64_t *)(dev->send_msg + MSG_HEAD_SIZE) = offset;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_OFFSET_OK, sizeof(uint64_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get partition slot number
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_get_slot_number(update_monitor_dev_t *dev)
{
    int ret = -1;
    uint8_t value = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            if (ptdev_multislot_is_supported(*p_ptdev)) {
                value = AB_SUPPORTED_SLOTS;
            }
            else {
                value = 1;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_NUMBER_SLOTS_FAIL, 0};
    }
    else {
        *(uint8_t *)(dev->send_msg + MSG_HEAD_SIZE) = value;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_NUMBER_SLOTS_OK, sizeof(uint8_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get partition number
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_get_partition_number(
    update_monitor_dev_t *dev)
{
    int ret = -1;
    uint8_t value = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            value = (*p_ptdev)->count;
            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_NUMBER_PARTITION_FAIL, 0};
    }
    else {
        *(uint8_t *)(dev->send_msg + MSG_HEAD_SIZE) = value;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_NUMBER_PARTITION_OK, sizeof(uint8_t)};
    }

    return ret;
}

struct um_partition_info {
    uint8_t type_guid[PARTITION_TYPE_GUID_SIZE];
    uint8_t unique_partition_guid[UNIQUE_PARTITION_GUID_SIZE];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t attribute_flag;
    unsigned char name[MAX_GPT_NAME_SIZE];
} __attribute__((packed));

typedef struct um_partition_info um_partition_info_t;

/**
 * @brief update monitor msg to get partition information
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_get_partition_info(update_monitor_dev_t
        *dev)
{
    int ret = -1;
    um_partition_info_t info;
    partition_device_t *ptdev = NULL;
    struct partition_entry *entry = NULL;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    uint8_t partition_index = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE);
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE + sizeof(
            uint8_t));

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN + sizeof(uint8_t))) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            ptdev = *p_ptdev;

            if (partition_index >= ptdev->count) {
                ERROR("partition_index %d too large, limited is %d\n", partition_index,
                      ptdev->count);
                ret = OTA_ERR_INVALID_PART_INDEX;
                goto end;
            }

            if (!ptdev->partition_entries) {
                ERROR("ptdev_name %s partition_entries is null\n", ptdev_name);
                ret = OTA_ERR_PART_ENTRY_NULL;
                goto end;
            }

            entry = ptdev->partition_entries;
            memcpy((uint8_t *)(info.type_guid),
                   (uint8_t *)(entry[partition_index].type_guid), PARTITION_TYPE_GUID_SIZE);
            memcpy((uint8_t *)(info.unique_partition_guid),
                   (uint8_t *)(entry[partition_index].unique_partition_guid),
                   UNIQUE_PARTITION_GUID_SIZE);
            memset((uint8_t *)(info.name), 0, MAX_GPT_NAME_SIZE);
            memcpy((uint8_t *)(info.name), (uint8_t *)(entry[partition_index].name),
                   MAX_GPT_NAME_SIZE);
            info.first_lba = entry[partition_index].first_lba;
            info.last_lba  = entry[partition_index].last_lba;
            info.attribute_flag  = entry[partition_index].attribute_flag;
            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_INFO_FAIL, 0};
    }
    else {
        memcpy((uint8_t *)(dev->send_msg + MSG_HEAD_SIZE), (uint8_t *)&info,
               sizeof(um_partition_info_t));
        dev->reply = (msg_reply_t) {OTA_CMD_GET_PARTITION_INFO_OK, sizeof(um_partition_info_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get current slot
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_get_current_slot(update_monitor_dev_t *dev)
{
    int ret = -1;
    int8_t value = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            value = (int8_t)ptdev_find_active_slot(*p_ptdev);

            if (value == INVALID) {
                ERROR("can't find a active slot!\n");
                ret = OTA_ERR_PART_ATTR_ACTIVE_FAILED;
                goto end;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CURRENT_SLOTS_FAIL, 0};
    }
    else {
        *(int8_t *)(dev->send_msg + MSG_HEAD_SIZE) = value;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CURRENT_SLOTS_OK, sizeof(int8_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to get ptdev slot attr
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_get_slot_attr(update_monitor_dev_t *dev)
{
    int ret = -1;
    int8_t value = 0;
    int result = 0;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    uint8_t slot = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE);
    uint8_t attr = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE + sizeof(
                                          uint8_t));
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE + 2 *
                                            sizeof(uint8_t));

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN + 2 * sizeof(
                                   uint8_t))) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            switch (attr) {
                case ATTR_UNBOOTABLE :
                    result = ptdev_find_bootable_slot(*p_ptdev);

                    if (result == INVALID) {
                        ERROR("find a bootable slot error\n");
                        ret = OTA_ERR_PART_ATTR_UNBOOT_FAILED;
                        goto end;
                    }

                    value = ((result == slot) || (result == AB_SUPPORTED_SLOTS)) ? 0 : 1;
                    DBG("ATTR_UNBOOTABLE value for slot %d is %d\n", slot, value);
                    break;

                case ATTR_ACTIVE :
                    result = ptdev_find_active_slot(*p_ptdev);

                    if (result == INVALID) {
                        ERROR("find a active slot error\n");
                        ret = OTA_ERR_PART_ATTR_ACTIVE_FAILED;
                        goto end;
                    }

                    value = (result == slot) ? 1 : 0;
                    DBG("ATTR_ACTIVE value for slot %d is %d\n", slot, value);
                    break;

                case ATTR_SUCCESSFUL :
                    result = ptdev_find_successfull_slot(*p_ptdev);

                    if (result == INVALID) {
                        ERROR("find a successful slot error\n");
                        ret = OTA_ERR_PART_ATTR_SUCC_FAILED;
                        goto end;
                    }

                    value = ((result == slot) || (result == AB_SUPPORTED_SLOTS)) ? 1 : 0;
                    DBG("ATTR_ACTIVE value for slot %d is %d\n", slot, value);
                    break;

                case ATTR_RETRY :
                    result = ptdev_get_slot_retrycnt(*p_ptdev, slot);

                    if (result == INVALID) {
                        ERROR("find a active slot error\n");
                        ret = OTA_ERR_PART_ATTR_RETRY_FAILED;
                        goto end;
                    }

                    value = (int8_t)result;
                    DBG("ATTR_ACTIVE value for slot %d is %d\n", slot, value);
                    break;

                default :
                    ERROR("attr error\n");
                    ret = OTA_ERR_PART_ATTR_UNKNOWN;
                    goto end;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_SLOT_ATTR_FAIL, 0};
    }
    else {
        *(int8_t *)(dev->send_msg + MSG_HEAD_SIZE) = value;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_SLOT_ATTR_OK, sizeof(int8_t)};
    }

    return ret;
}

/**
 * @brief update monitor msg to set ptdev slot attr
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_set_slot_attr(update_monitor_dev_t *dev)
{
    int ret = -1;
    int result = -1;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    uint8_t slot = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE);
    uint8_t attr = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE + sizeof(
                                          uint8_t));
    uint8_t value = *(const uint8_t *)(dev->recv_msg + MSG_HEAD_SIZE + 2 * sizeof(
                                           uint8_t));
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE + 3 *
                                            sizeof(uint8_t));

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN + 3 * sizeof(
                                   uint8_t))) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            switch (attr) {
                case ATTR_UNBOOTABLE :
                case ATTR_ACTIVE :
                case ATTR_SUCCESSFUL :
                    if (1 == value)
                        result = ptdev_mark_slot_attr_noupdate(*p_ptdev, slot, attr);
                    else if (0 == value)
                        result = ptdev_clean_slot_attr_noupdate(*p_ptdev, slot, attr);
                    else {
                        ERROR("find set attr = %d value = %d error\n", attr, value);
                        ret = OTA_ERR_PART_ATTR_SUCC_INVALID;
                        goto end;
                    }

                    if (result != 0) {
                        ERROR("find set attr %d value %d, for slot %d error\n", attr, value, slot);
                        ret = OTA_ERR_PART_ATTR_SUCC_FAILED;
                        goto end;
                    }

                    break;

                case ATTR_RETRY :
                    if ((value < MAX_RETRY_COUNT) && (value >= 0))
                        result = set_slot_retrycnt(*p_ptdev, slot, value);
                    else {
                        ERROR("find set attr = %d value = %d error\n", attr, value);
                        ret = OTA_ERR_PART_ATTR_RETRY_INVALID;
                        goto end;
                    }

                    if (result != 0) {
                        ERROR("find set attr %d value %d, for slot %d error\n", attr, value, slot);
                        ret = OTA_ERR_PART_ATTR_RETRY_FAILED;
                        goto end;
                    }

                    break;

                default :
                    ERROR("attr error\n");
                    ret = OTA_ERR_PART_ATTR_UNKNOWN;
                    goto end;
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_SET_SLOT_ATTR_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_SET_SLOT_ATTR_OK, 0};
    }

    return ret;
}

/**
 * @brief update monitor msg to update ptdev slot attr
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_ptdev_update_slot_attr(update_monitor_dev_t *dev)
{
    int ret = -1;
    int index = -1;
    partition_device_t **p_ptdev = NULL;
    const char *ptdev_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > UPDATE_MONITOR_PTDEV_NAME_LEN) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!ptdev_name_check(ptdev_name, &index)) {
        p_ptdev = get_ptdev(index);

        if (p_ptdev && *p_ptdev) {
            if ((*p_ptdev)->attributes_updated) {
                if (0 != ptdev_attributes_update(*p_ptdev)) {
                    ERROR("ptdev attributes update error\n");
                    ret = OTA_ERR_PART_ATTR_UPDATE_FAILED;
                    goto end;
                }

                (*p_ptdev)->attributes_updated = false;
            }
            else {
                DBG("do not need update ptdev %s\n", ptdev_name);
            }

            ret = OTA_ERR_NO_ERR;
        }
        else {
            ERROR("ptdev is null\n");
            ret = OTA_ERR_PART_DEV_NULL;
        }
    }
    else {
        ERROR("ptdev not exist, ptdev_name is %s\n", ptdev_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_UPDATE_SLOT_ATTR_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_UPDATE_SLOT_ATTR_OK, 0};
    }

    return ret;
}

/**
 * @brief update monitor msg to start communication with ota-lib
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_start(update_monitor_dev_t *dev)
{
    int ret = -1;
    ret =  update_monitor_load_storage(dev);

    if (ret < 0) {
        dev->reply = (msg_reply_t) {OTA_CMD_START_FAIL, 0};
        ERROR("load storage failed ,start error\n");
        return OTA_ERR_STORAGE_LOAD_FAILED;
    }
    else {
#if USE_SHARED_MEMORY
        uint32_t *p = (uint32_t *)(dev->send_msg + MSG_HEAD_SIZE);
        *p++ = SHARED_MEMORY_BASE;
        *p++ = MAX_SHARED_MEMORY_READ_DATA_LENGTH;
        *p++ = MAX_SHARED_MEMORY_WRITE_DATA_LENGTH;
        *p++ = MAX_SHARED_MEMORY_CRC_LENGTH;
        dev->reply = (msg_reply_t) {OTA_CMD_START_OK, 4 * sizeof(uint32_t)};
#else
        dev->reply = (msg_reply_t) {OTA_CMD_START_OK, 0};
#endif
        DBG("start ok\n");
        return OTA_ERR_NO_ERR;
    }

}
/**
 * @brief handle the "set_priority" cammand from remote processer
 * @param[in] monitor pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_set_priority(update_monitor_dev_t *dev)
{
    uint32_t priority = 0;
    int offset = MSG_HEAD_SIZE;

    priority = getUint32(dev->recv_msg, &offset);

    if (priority) {
        DBG("set high priorty\n");
        osThreadSetPriority(dev->msg_process_thread, osPriorityNormal7);
    }
    else {
        DBG("set default priorty\n");
        osThreadSetPriority(dev->msg_process_thread, osPriorityNormal);
    }

    dev->reply = (msg_reply_t) {OTA_CMD_PRIORITY_OK, 0};
    return OTA_ERR_NO_ERR;
}

/**
 * @brief handle the "seek" cammand from remote processer
 * @param[in] monitor pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_seek(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    uint64_t seek_addr;

    seek_addr = getUint64(dev->recv_msg, &offset);
    if (seek_addr > (dev->capacity)) {
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_FAIL, 0};
        ERROR("msg_seek: seek=%lld bigger than capacity=%lld \n",
                    seek_addr, dev->capacity);
        return OTA_ERR_INVALID_SEEK_ADDR;
    }
    else {
        dev->seek_addr = seek_addr;
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_OK, 0};
        DBG("msg_seek:%lld\n", dev->seek_addr);
        return OTA_ERR_NO_ERR;
    }
}

/**
 * @brief update monitor msg to seek a addr for mapped disk dev
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_seek_mapped(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    uint64_t seek_addr = 0;
    int ret = -1;
    int index = -1;
    struct disk_dev *mapped_storage = NULL;
    uint64_t ptn = 0;
    uint64_t pt_size = 0;

    seek_addr = getUint64(dev->recv_msg, &offset);
    const char *partition_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE +
                                 sizeof(uint64_t));

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN + sizeof(uint64_t))) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!partition_name_check(partition_name, &index)) {
        if ((index >= get_ptdev_cnt()) || (index < 0)) {
            ERROR("index error\n");
            ret = OTA_ERR_INVALID_PART_INDEX;
            goto end;
        }

        ptn = get_sub_partition_ptn(index);
        pt_size = get_sub_partition_pt_size(index);
        mapped_storage = get_mapped_storage(index);

        if (!mapped_storage || !ptn || !pt_size) {
            ERROR("mapped storage para error mapped_storage = %p, ptn = 0x%llx, pt_size = 0x%llx\n",
                  mapped_storage, ptn, pt_size);
            ret = OTA_ERR_INVALID_PART_INFO;
            goto end;
        }

        if (seek_addr > pt_size) {
            ERROR("mapped seek=%lld bigger than capacity=%lld \n", seek_addr, pt_size);
            ret = OTA_ERR_INVALID_SEEK_ADDR;
            goto end;
        }

        dev->seek_addr = seek_addr + ptn;
        DBG("mapped seek:%lld, real seek:%lld \n", seek_addr, dev->seek_addr);
        ret = OTA_ERR_NO_ERR;
    }
    else {
        ERROR("seek for %s error \n", partition_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_MAPPED_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_SEEK_MAPPED_OK, 0};
    }

    return ret;
}
/**
 * @brief handle the "read" cammand from remote processer
 * @param[in] dev pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_read(update_monitor_dev_t *dev)
{
    int ret = -1;
    uint32_t read_len;
    int offset = MSG_HEAD_SIZE;
    struct disk_dev* storage = dev->storage;

    /* storage check */
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        ERROR("msg_read: storage is NULL\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }

    /* get read length */
    read_len = getUint32(dev->recv_msg, &offset);

#if USE_SHARED_MEMORY
    uint8_t *data = (uint8_t *)(SHARED_MEMORY_BASE +
                                SHARED_MEMORY_READ_DATA_OFFSET);
    uint32_t *data_crc = (uint32_t *)(SHARED_MEMORY_BASE +
                                      SHARED_MEMORY_READ_CRC_OFFSET);

    uint32_t rand_data = getUint32(dev->recv_msg, &offset);

    if ((read_len > MAX_SHARED_MEMORY_READ_DATA_LENGTH) ) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        ERROR("length error\n");
        return OTA_ERR_INVALID_READ_LEN;
    }

    /*
        if(dev->seek_addr % 512) {
            ERROR("seek addr shold aligned to 512\n");
            return;
        }
    */
    arch_invalidate_cache_range(SHARED_MEMORY_BASE + SHARED_MEMORY_READ_DATA_OFFSET,
                                ROUNDUP(read_len, CONFIG_ARCH_CACHE_LINE));

    /*read from storage*/
    ret = aligned_read_big_buffer(storage, dev->seek_addr, data, read_len);

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        ERROR("storage read error, ret=%d\n", ret);
        return OTA_ERR_STORAGE_READ_FAILED;
    }

    arch_clean_cache_range(SHARED_MEMORY_BASE + SHARED_MEMORY_READ_DATA_OFFSET,
                           ROUNDUP(read_len, CONFIG_ARCH_CACHE_LINE));

    *data_crc = crc32(rand_data, data, read_len);
    arch_clean_cache_range(SHARED_MEMORY_BASE + SHARED_MEMORY_READ_CRC_OFFSET,
                           ROUNDUP(sizeof(uint32_t), CONFIG_ARCH_CACHE_LINE));
    DBG("rand_data = 0x%08x, *data_crc  = 0x%08x\n", rand_data, *data_crc);

    *(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE) = rand_data;
    dev->reply = (msg_reply_t) {OTA_CMD_READ_OK, sizeof(rand_data)};

    /*seek_addr increase*/
    dev->seek_addr = dev->seek_addr + read_len;
#else

    if ((read_len > MAX_DATA_LENGTH) ) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        ERROR("length error\n");
        return OTA_ERR_INVALID_READ_LEN;
    }

    ret = aligned_read(storage, dev->seek_addr,
                       (uint8_t *)(dev->send_msg + MSG_HEAD_SIZE), read_len);

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_READ_FAIL, 0};
        ERROR("storage read error, ret=%d\n", ret);
        return OTA_ERR_STORAGE_READ_FAILED;
    }

    /*seek_addr increase*/
    dev->seek_addr = dev->seek_addr + read_len;
    dev->reply = (msg_reply_t) {OTA_CMD_READ_OK, read_len};
#endif

    return OTA_ERR_NO_ERR;
}


/**
 * @brief handle the "write info" cammand from remote processer
 * @param[in] dev pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_write_info(update_monitor_dev_t *dev)
{
    int ret = -1;
    struct disk_dev* storage = dev->storage;
    int offset = MSG_HEAD_SIZE;
    uint64_t align_addr = 0;
    uint64_t write_buffer_size = 0;

    /* storage check */
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        ERROR("storage is NULL\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }

    /* get write total length */
    dev->write_len_total = getUint64(dev->recv_msg, &offset);
    ASSERT(dev->write_len_total > 0);
    /*
        ASSERT(0 == (dev->write_len_total % 32));
    */

    if (!dev->write_buf_flag) {
        update_dev_calloc(dev);
    }

    if (!dev->write_buf) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        ERROR("calloc fail\n");
        return OTA_ERR_WRITE_BUF_ALLOC_FAILED;
    }

#if USE_SHARED_MEMORY
    arch_invalidate_cache_range(SHARED_MEMORY_BASE +
                                SHARED_MEMORY_WRITE_DATA_OFFSET, ROUNDUP(dev->write_len_total, CONFIG_ARCH_CACHE_LINE));
    arch_invalidate_cache_range(SHARED_MEMORY_BASE + SHARED_MEMORY_WRITE_CRC_OFFSET,
                                ROUNDUP(sizeof(uint32_t), CONFIG_ARCH_CACHE_LINE));
#endif

#if CONFIG_UPDATE_FLASH
    write_buffer_size = dev->erase_size;
#elif CONFIG_UPDATE_EMMC
    #error ssdk update monitor could not build for a non-safety config
    write_buffer_size = ALIGNED_BUFFER_SIZE;
#endif

    ASSERT(write_buffer_size > 0);

    align_addr = GET_ALIGN_ADDR(dev->seek_addr, write_buffer_size);

    /* read back the buffer */
    ret = aligned_read(storage, align_addr, dev->write_buf, write_buffer_size);

    if (ret) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_FAIL, 0};
        ERROR("storage read error1, ret=%d\n", ret);
        return OTA_ERR_STORAGE_READ_FAILED;
    }

    DBG("seek_addr=%lld\n", dev->seek_addr);
    DBG("write_len_total=%lld\n", dev->write_len_total);
    dev->reply = (msg_reply_t) {OTA_CMD_WRITE_INFO_OK, 0};
    return OTA_ERR_NO_ERR;
}

/**
 * @brief aligned write
 *
 * @param storage disk dev
 * @param align_addr addr need to align
 * @param write_buf write data buffer
 * @param size write data size
 * @param aligned_size size nedd to align
 * @return int 0 is successful, other is failed
 */
int write_aligned_buffer(struct disk_dev *storage, uint64_t align_addr,
                         uint8_t *write_buf, uint64_t size, uint64_t aligned_size)
{
    int ret = -1;

    if (align_addr % 512 || size % 512 || aligned_size % 512 ) {
        ERROR("addr 0x%llx or buffer size 0x%llx is not aligned to 512\n", align_addr,
              size);
        goto end;
    }

    if (align_addr % aligned_size || size % aligned_size) {
        ERROR("addr 0x%llx is not aligned to aligned size 0x%llx\n", align_addr,
              aligned_size);
        goto end;
    }

#if CONFIG_UPDATE_FLASH

    /* erase now sector */
    if (0 != um_disk_erase(storage, align_addr, size)) {
        ERROR("storage erase addr 0x%llx fail\n", align_addr);
        goto end;;
    }

#endif

    /* write to flash */
    if (0 != um_disk_write(storage, align_addr, write_buf, size)) {
        ERROR("msg_write:storage write addr 0x%llx fail\n", align_addr);
        goto end;;
    }

    ret = 0;
end:
    return ret;
}

/**
 * @brief handle the "write" cammand from remote processer
 *  To make sure the full use of ospi nor flash , write one erase size each time
 * @param[in] dev pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_write(update_monitor_dev_t *dev)
{
    int ret = -1;
    struct disk_dev* storage = dev->storage;
    uint64_t write_len = 0;    /* length of write data this time */
    uint64_t rel_addr = 0;     /* relative addr in one erase sector */
    uint64_t align_addr = 0;   /* addr align to erase size */
    int64_t msg_part1 = 0;
    int64_t msg_part2 = 0;
    int64_t write_cnt = 0;
    uint8_t *data = NULL;
    uint64_t write_buffer_size = 0;
    bool skip_once = false;

#if USE_SHARED_MEMORY
    int offset = MSG_HEAD_SIZE;
    uint32_t rand_data = 0;
    int write_large_one_time = 0;
    uint32_t data_crc = *(uint32_t *)(SHARED_MEMORY_BASE +
                                      SHARED_MEMORY_WRITE_CRC_OFFSET);
    uint32_t crc_val = 0xFFFFFFFF;
    data = (uint8_t *)(SHARED_MEMORY_BASE + SHARED_MEMORY_WRITE_DATA_OFFSET);
    write_len = (uint64_t)getUint32(dev->recv_msg, &offset);
    rand_data = getUint32(dev->recv_msg, &offset);

    if (write_len == 0 || write_len > MAX_SHARED_MEMORY_WRITE_DATA_LENGTH) {
        ERROR("msg write len = 0x%llx, error\n", write_len);
        ERROR("rand_data = 0x%x\n", rand_data);
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        update_dev_free(dev);
        return OTA_ERR_INVALID_WRITE_PARA;
    }

    ASSERT(write_len);
    ASSERT(write_len <= MAX_SHARED_MEMORY_WRITE_DATA_LENGTH);

    arch_invalidate_cache_range(SHARED_MEMORY_BASE +
                                SHARED_MEMORY_WRITE_DATA_OFFSET, ROUNDUP(write_len, CONFIG_ARCH_CACHE_LINE));
    arch_invalidate_cache_range(SHARED_MEMORY_BASE + SHARED_MEMORY_WRITE_CRC_OFFSET,
                                ROUNDUP(sizeof(uint32_t), CONFIG_ARCH_CACHE_LINE));

#else
    data = (dev->recv_msg) + MSG_HEAD_SIZE;
    write_len = dev->recv_head->len;

    if (write_len == 0 || write_len > MAX_DATA_LENGTH) {
        ERROR("msg write len = 0x%llx, error\n", write_len);
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        update_dev_free(dev);
        return OTA_ERR_INVALID_WRITE_PARA;
    }

    ASSERT(write_len);
    ASSERT(write_len <= MAX_DATA_LENGTH);
#endif

    /*storage check*/
    if (dev->storage == NULL) {
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        ERROR("storage is NULL\n");
        update_dev_free(dev);
        return OTA_ERR_STORAGE_DEV_NULL;
    }

    /* check write buf */
    if (!dev->write_buf_flag) {
        ERROR("write_buf is null\n");
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        return OTA_ERR_WRITE_BUF_ALLOC_FAILED;
    }

#if CONFIG_UPDATE_FLASH
    write_buffer_size = dev->erase_size;
    ASSERT(write_buffer_size > 0);
#elif CONFIG_UPDATE_EMMC
    #error ssdk update monitor could not build for a non-safety config
    write_buffer_size = ALIGNED_BUFFER_SIZE;
    ASSERT(write_buffer_size > 0);
#endif


#if USE_SHARED_MEMORY
    crc_val = crc32(rand_data, data, write_len);
    DBG("final crc_val = 0x%x\n", crc_val);

    if (data_crc != crc_val) {
        ERROR("rand_data = 0x%x\n", rand_data);
        ERROR("crc read = 0x%08x, crc calc = 0x%08x error\n", data_crc, crc_val);
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        update_dev_free(dev);
        return OTA_ERR_DATA_CRC_FAILED;
    }

    ASSERT((uint64_t)(SHARED_MEMORY_BASE + SHARED_MEMORY_WRITE_DATA_OFFSET) %
           write_buffer_size == 0);
#endif

    rel_addr =     GET_REL_ADDR(dev->seek_addr, write_buffer_size);
    align_addr = GET_ALIGN_ADDR(dev->seek_addr, write_buffer_size);

    /* copy data from recv msg to buf */
    if ((rel_addr + write_len) < write_buffer_size) {
        memcpy(dev->write_buf + rel_addr, data, write_len);
        DBG("msg_write:not full\n");
    }
    else {
        msg_part1 = write_buffer_size - rel_addr;
        msg_part2 = write_len - msg_part1;
        memcpy((dev->write_buf) + rel_addr, data, msg_part1);
        write_cnt = write_len / write_buffer_size + 2;

        do {
            if (false == skip_once) {
                ret = write_aligned_buffer(storage, align_addr, dev->write_buf,
                                           write_buffer_size, write_buffer_size);

                if (ret < 0) {
                    ERROR("write 0x%llx fail\n", align_addr);
                    dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                    update_dev_free(dev);
                    return OTA_ERR_STORAGE_WRITE_FAILED;
                }
            }

            skip_once = false;

            /* clear buf */
            memset(dev->write_buf, 0, write_buffer_size);

            if (msg_part2 >= (int64_t)write_buffer_size) {
#if USE_SHARED_MEMORY
                /* just for emmc shared memory mode, write large amounts of data here */
                int64_t part2_cnt = msg_part2 / write_buffer_size;

                if (msg_part1 % 512 == 0) {
                    ASSERT(write_large_one_time == 0);
                    ASSERT(part2_cnt >= 1);
                    ASSERT(msg_part2 >= (int64_t)(part2_cnt * write_buffer_size));
                    ASSERT(part2_cnt <= (write_cnt - 1));
                    align_addr += write_buffer_size;
                    ASSERT(msg_part1 % 512 == 0);
                    /* write to flash */
                    DBG("msg write:storage write big data 0x%llx\n", part2_cnt * write_buffer_size);
                    ret = write_aligned_buffer(storage, align_addr, data + msg_part1,
                                               part2_cnt * write_buffer_size, write_buffer_size);

                    //ret =     um_disk_write(storage, align_addr, data + msg_part1, part2_cnt * write_buffer_size);
                    if (ret < 0) {
                        ERROR("msg write:storage write addr 0x%llx fail\n", align_addr);
                        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                        update_dev_free(dev);
                        return OTA_ERR_STORAGE_WRITE1_FAILED;
                    }

                    msg_part2 -= (part2_cnt * write_buffer_size);
                    msg_part1 += (part2_cnt * write_buffer_size);
                    align_addr += (part2_cnt * write_buffer_size);
                    write_cnt = write_cnt - part2_cnt;
                    skip_once = true;
                    write_large_one_time++;
                }
                else {
                    memcpy((dev->write_buf), data + msg_part1, write_buffer_size);
                    msg_part2 -= write_buffer_size;
                    msg_part1 += write_buffer_size;
                    align_addr += write_buffer_size;
                }

#else
                memcpy((dev->write_buf), data + msg_part1, write_buffer_size);
                msg_part2 -= write_buffer_size;
                msg_part1 += write_buffer_size;
                align_addr += write_buffer_size;
#endif
                continue;
            }
            else {
                if (msg_part2 > 0)  {
                    memcpy((dev->write_buf), data + msg_part1, msg_part2);
                    msg_part1 += msg_part2;
                    msg_part2 = 0;
                }

                break;
            }

            if (write_cnt-- < 0) {
                ERROR("write cnt error\n");
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                update_dev_free(dev);
                return OTA_ERR_INVALID_WRITE_CNT;
            }

        }
        while (1);

        ASSERT(msg_part1 == (int64_t)write_len);
        DBG("full\n");
    }

    /*seek_addr increase*/
    dev->seek_addr += write_len;

    /*update relative addr adn align addr */
    rel_addr = GET_REL_ADDR(dev->seek_addr, write_buffer_size);
    align_addr = GET_ALIGN_ADDR(dev->seek_addr, write_buffer_size);

    /* calc the msg total length left */
    if (dev->write_len_total >= write_len)
        dev->write_len_total = dev->write_len_total - write_len;
    else {
        ERROR("len_total=0x%llx left is shorter than write_len=0x%llx, fail\n",
              dev->write_len_total, write_len);
        dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
        update_dev_free(dev);
        return OTA_ERR_INVALID_TOTAL_LEN;
    }

    DBG("ptate_monitor msg_write: this write_len=0x%llx\n", write_len);
    DBG("ptate_monitor msg_write: left write_len_total=0x%llx\n",
        dev->write_len_total);

    /*last msg */
    if (dev->write_len_total == 0) {
        DBG("ptate_monitor msg_write: last msg\n");

        if (rel_addr != 0) {
            DBG("last copy\n");
            //copy the rest of data
            ret = aligned_read(storage, dev->seek_addr, (dev->write_buf) + rel_addr,
                               write_buffer_size - rel_addr);

            if (ret) {
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                ERROR("storage read error2, ret=%d\n", ret);
                update_dev_free(dev);
                return OTA_ERR_STORAGE_READ_FAILED;
            }

            ret = write_aligned_buffer(storage, align_addr, dev->write_buf,
                                       write_buffer_size, write_buffer_size);

            if (ret < 0) {
                ERROR("write 0x%llx fail2\n", align_addr);
                dev->reply = (msg_reply_t) {OTA_CMD_WRITE_FAIL, 0};
                update_dev_free(dev);
                return OTA_ERR_STORAGE_WRITE2_FAILED;
            }
        }

        update_dev_free(dev);
    }

    dev->reply = (msg_reply_t) {OTA_CMD_WRITE_OK, 0};
    return OTA_ERR_NO_ERR;
}


/**
 * @brief handle the "get capacity size" cammand from remote processer
 * @param[in] dev pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static inline int update_msg_get_capacity(update_monitor_dev_t *dev)
{
    if (0 == dev->capacity) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_FAIL, 0};
        ERROR("get_capacity fail\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }
    else {
        *(uint64_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->capacity;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_OK, sizeof(uint64_t)};
        DBG("get_capacity ok\n");
        return OTA_ERR_NO_ERR;
    }
}

/**
 * @brief update monitor msg to get capacity for a mapped disk device
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_capacity_mapped(update_monitor_dev_t *dev)
{
    int ret = -1;
    int index = -1;
    struct disk_dev *mapped_storage = NULL;
    uint64_t ptn = 0;
    uint64_t pt_size = 0;
    uint64_t capacity;

    const char *partition_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE);

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN)) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!partition_name_check(partition_name, &index)) {
        if ((index >= get_ptdev_cnt()) || (index < 0)) {
            ERROR("index error\n");
            ret = OTA_ERR_INVALID_PART_INDEX;
            goto end;
        }
        DBG("partition_name = %s index = %d\n", partition_name, index);
        ptn = get_sub_partition_ptn(index);
        pt_size = get_sub_partition_pt_size(index);
        mapped_storage = get_mapped_storage(index);

        if (!mapped_storage || !ptn || !pt_size) {
            ERROR("index = %d\n", index);
            ERROR("mapped storage para error mapped_storage = %p, ptn = 0x%llx, pt_size = 0x%llx\n",
                  mapped_storage, ptn, pt_size);
            ret = OTA_ERR_INVALID_PART_INFO;
            goto end;
        }

        DBG("mapped_storage = %p\n", mapped_storage);
        capacity = um_get_capacity(mapped_storage);

        if (pt_size != capacity) {
            ERROR("pt_size=%lld not equal to  get capacity=%lld \n", pt_size, capacity);
            ret = OTA_ERR_INVALID_CAPACITY;
            goto end;
        }

        DBG("mapped partition_name %s capacity 0x%llx \n", partition_name, capacity);
        ret = OTA_ERR_NO_ERR;
    }
    else {
        ERROR("get capacity for %s error \n", partition_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret != OTA_ERR_NO_ERR) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_MAPPED_FAIL, 0};
    }
    else {
        *(uint64_t *)(dev->send_msg + MSG_HEAD_SIZE) = capacity;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_CAPACITY_MAPPED_OK, sizeof(uint64_t)};
    }

    return ret;

}

/**
 * @brief update monitor msg to get erase size of disk device
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_erasesize(update_monitor_dev_t *dev)
{
    if (0 == dev->erase_size) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_ERASESIZE_FAIL, 0};
        ERROR("get erase size fail\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }
    else {
        *(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->erase_size;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_ERASESIZE_OK, sizeof(uint32_t)};
        DBG("get erase size ok\n");
        return OTA_ERR_NO_ERR;
    }
}

/**
 * @brief update monitor msg to get block size of disk device
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_get_blocksize(update_monitor_dev_t *dev)
{
    if (0 == dev->block_size) {
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BLOCK_FAIL, 0};
        ERROR("get block_size fail\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }
    else {
        *(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE) = dev->block_size;
        dev->reply = (msg_reply_t) {OTA_CMD_GET_BLOCK_OK, sizeof(uint32_t)};
        DBG("get block_size ok\n");
        return OTA_ERR_NO_ERR;
    }
}

/**
 * @brief do memory copy from a disk addr to another disk addr
 *
 * @param dev pointer to an update monitor instance
 * @param storage  disk device
 * @param src_cp_addr source addr
 * @param dst_cp_addr destination addr
 * @param size copy size
 * @return int
 */
static inline int memory_copy(update_monitor_dev_t *dev,
                              struct disk_dev* storage, uint64_t src_cp_addr, uint64_t dst_cp_addr,
                              uint64_t size)
{
    int ret = -1;
    uint32_t write_buffer_size = 0;;
    uint64_t cp_cnt = 0;

#if CONFIG_UPDATE_FLASH
    write_buffer_size = dev->erase_size;
#elif CONFIG_UPDATE_EMMC
    #error ssdk update monitor could not build for a non-safety config
    write_buffer_size = ALIGNED_BUFFER_SIZE;
#endif
    ASSERT(0 == (src_cp_addr % write_buffer_size));
    ASSERT(0 == (dst_cp_addr % write_buffer_size));
    ASSERT(0 == (size % write_buffer_size));
    cp_cnt = size / write_buffer_size;

    /*storage check*/
    if (storage == NULL) {
        ERROR("msg_copy storage is NULL\n");
        return OTA_ERR_STORAGE_DEV_NULL;
    }

    DBG("msg_copy: src_addr=%d\n", (uint32_t)src_cp_addr);
    DBG("msg_copy: dst_addr=%d\n", (uint32_t)dst_cp_addr);
    DBG("msg_copy: size=%d\n", (uint32_t)size);

    if ((src_cp_addr > dst_cp_addr)
            || ((src_cp_addr + cp_cnt * write_buffer_size) <= dst_cp_addr)) {
        update_dev_calloc(dev);

        for (uint64_t n = 0; n < cp_cnt; n++) {
            /*read*/
            ret = um_disk_read(storage, src_cp_addr, dev->write_buf, write_buffer_size);

            if (ret) {
                ERROR("msg_copy: storage read error, addr 0x%08x\n",
                      (uint32_t)src_cp_addr);
                update_dev_free(dev);
                return OTA_ERR_STORAGE_READ_FAILED;
            }

#if CONFIG_UPDATE_FLASH
            /*erase*/
            ret = um_disk_erase(storage, dst_cp_addr, write_buffer_size);

            if (ret < 0) {
                ERROR("msg_copy: storage erase addr 0x%08x fail\n",
                      (uint32_t)dst_cp_addr);
                update_dev_free(dev);
                return OTA_ERR_STORAGE_ERASE_FAILED;
            }

#endif
            /*write to flash*/
            ret = um_disk_write(storage, dst_cp_addr, dev->write_buf, write_buffer_size);

            if (ret < 0) {
                ERROR("msg_copy: 0x%08x fail\n", (uint32_t)dst_cp_addr);
                update_dev_free(dev);
                return OTA_ERR_STORAGE_WRITE_FAILED;
            }

            /*increase addr*/
            src_cp_addr += write_buffer_size;
            dst_cp_addr += write_buffer_size;
        }

        update_dev_free(dev);
        return OTA_ERR_NO_ERR;
        DBG("msg_copy: ok\n");
    }
    else {
        return OTA_ERR_INVALID_COPY_PARA;
        ERROR("msg_copy: para error\n");
    }
}

/**
 * @brief update monitor msg to do memory copy
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_copy(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    uint64_t src_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t dst_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t size        = getUint64(dev->recv_msg, &offset);
    int ret = memory_copy(dev, dev->storage, src_cp_addr, dst_cp_addr, size);

    if (ret != OTA_ERR_NO_ERR) {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_OK, 0};
    }

    return ret;
}

/**
 * @brief update monitor msg to do memory copy for a mapped disk
 *
 * @param dev pointer to an update monitor instance
 * @return int OTA_ERR_NO_ERR is success, other is failed
 */
static inline int update_msg_copy_mapped(update_monitor_dev_t *dev)
{
    int offset = MSG_HEAD_SIZE;
    int ret = -1;
    int index = -1;
    struct disk_dev *mapped_storage = NULL;
    uint64_t ptn = 0;
    uint64_t pt_size = 0;
    uint64_t src_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t dst_cp_addr = getUint64(dev->recv_msg, &offset);
    uint64_t size        = getUint64(dev->recv_msg, &offset);

    const char *partition_name = (const char *)(dev->recv_msg + MSG_HEAD_SIZE + 3 *
                                 sizeof(uint64_t));

    if (dev->recv_head->len > (UPDATE_MONITOR_PTDEV_NAME_LEN + 3 * sizeof(
                                   uint64_t))) {
        ERROR("recv msg len error\n");
        ret = OTA_ERR_INVALID_DATA_LEN;
        goto end;
    }

    if (!partition_name_check(partition_name, &index)) {
        if ((index >= get_ptdev_cnt()) || (index < 0)) {
            ERROR("index error\n");
            ret = OTA_ERR_INVALID_PART_INDEX;
            goto end;
        }

        ptn = get_sub_partition_ptn(index);
        pt_size = get_sub_partition_pt_size(index);
        mapped_storage = get_mapped_storage(index);

        if (!mapped_storage || !ptn || !pt_size) {
            ERROR("mapped storage para error mapped_storage = %p, ptn = 0x%llx, pt_size = 0x%llx\n",
                  mapped_storage, ptn, pt_size);
            ret = OTA_ERR_INVALID_PART_INFO;
            goto end;
        }

        ret = memory_copy(dev, mapped_storage, src_cp_addr, dst_cp_addr, size);

        if (ret) {
            ERROR("mapped memory copy error\n");
            goto end;
        }

        ret = OTA_ERR_NO_ERR;
    }
    else {
        ERROR("mapped copy for %s error\n", partition_name);
        ret = OTA_ERR_PART_CHK_FAILED;
    }

end:

    if (ret != OTA_ERR_NO_ERR) {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_MAPPED_FAIL, 0};
    }
    else {
        dev->reply = (msg_reply_t) {OTA_CMD_COPY_MAPPED_OK, 0};
    }

    return ret;
}

/**
 * @brief stop watch dog
 */
void stop_wdt()
{
    static watchdog_device_t g_wdt;
    static sdrv_watchdog_device_t g_sdrv_wdt = {
        .base  =  APB_WDT1_BASE,
    };

    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        ERROR("sdrv watchdog init failed\n");
        return;
    }

    if (watchdog_stop(&g_wdt)) {
        ERROR("stop watchdog failed\n");
        return;
    }

}

/**
 * @brief stop wdt dog msg
 *
 * @param dev pointer to an update monitor instance
 * @return int return OTA_ERR_NO_ERR
 */
static inline int update_msg_stop_wdt(update_monitor_dev_t *dev)
{
    ERROR("update monitor stop wdt\n");
    stop_wdt();
    dev->reply = (msg_reply_t) {OTA_CMD_STOP_WDT_OK, 0};
    return OTA_ERR_NO_ERR;
}

/**
 * all msgs that update monitor supported
 */
update_msg_cmd_t update_msg_processer_table[] = {
    {OTA_CMD_START,                 update_msg_start,                      "update_msg_start",                      true},
    {OTA_CMD_PRIORITY,              update_msg_set_priority,               "update_msg_set_priority",               true},
    {OTA_CMD_SEEK,                  update_msg_seek,                       "update_msg_seek",                       true},
    {OTA_CMD_READ,                  update_msg_read,                       "update_msg_read",                       true},
    {OTA_CMD_WRITE_INFO,            update_msg_write_info,                 "update_msg_write_info",                 true},
    {OTA_CMD_WRITE,                 update_msg_write,                      "update_msg_write",                      true},
    {OTA_CMD_GET_CAPACITY,          update_msg_get_capacity,               "update_msg_get_capacity",               true},
    {OTA_CMD_GET_ERASESIZE,         update_msg_get_erasesize,              "update_msg_get_erasesize",              true},
    {OTA_CMD_GET_BLOCK,             update_msg_get_blocksize,              "update_msg_get_blocksize",              true},
    {OTA_CMD_COPY,                  update_msg_copy,                       "update_msg_copy",                       true},
    {OTA_CMD_HANDOVER,              update_msg_handover,                   "update_msg_handover",                   true},
    {OTA_CMD_CLOSE,                 update_msg_close,                      "update_msg_close",                      true},
    {OTA_CMD_STOP_WDT,              update_msg_stop_wdt,                   "update_msg_stop_wdt",                   true},
#if CONFIG_UPDATE_EMMC
    {OTA_CMD_SETUP_PTDEV,           update_msg_ptdev_setup,                "update_msg_ptdev_setup",                true},
    {OTA_CMD_DESTROY_PTDEV,         update_msg_ptdev_destroy,              "update_msg_ptdev_destroy",              true},
    {OTA_CMD_GET_PARTITION_INDEX,   update_msg_get_partition_index,        "update_msg_get_partition_index",        true},
    {OTA_CMD_GET_PARTITION_SIZE,    update_msg_get_partition_size,         "update_msg_get_partition_size",         true},
    {OTA_CMD_GET_PARTITION_OFFSET,  update_msg_get_partition_offset,       "update_msg_get_partition_offset",       true},
    {OTA_CMD_GET_NUMBER_SLOTS,      update_msg_ptdev_get_slot_number,      "update_msg_ptdev_get_slot_number",      true},
    {OTA_CMD_GET_CURRENT_SLOTS,     update_msg_ptdev_get_current_slot,     "update_msg_ptdev_get_current_slot",     true},
    {OTA_CMD_GET_SLOT_ATTR,         update_msg_ptdev_get_slot_attr,        "update_msg_ptdev_get_slot_attr",        true},
    {OTA_CMD_SET_SLOT_ATTR,         update_msg_ptdev_set_slot_attr,        "update_msg_ptdev_set_slot_attr",        true},
    {OTA_CMD_UPDATE_SLOT_ATTR,      update_msg_ptdev_update_slot_attr,     "update_msg_ptdev_update_slot_attr",     true},
    {OTA_CMD_GET_NUMBER_PARTITION,  update_msg_ptdev_get_partition_number, "update_msg_ptdev_get_partition_number", true},
    {OTA_CMD_GET_PARTITION_INFO,    update_msg_ptdev_get_partition_info,   "update_msg_ptdev_get_partition_info",   true},
#endif
    {OTA_CMD_GET_BOOT_INFO,         update_msg_get_boot_info,              "update_msg_get_boot_info",              true},
#if CONFIG_UPDATE_EMMC
    {OTA_CMD_SEEK_MAPPED,           update_msg_seek_mapped,                "update_msg_seek_mapped",                true},
    {OTA_CMD_GET_CAPACITY_MAPPED,   update_msg_get_capacity_mapped,        "update_msg_get_capacity_mapped",        true},
    {OTA_CMD_COPY_MAPPED,           update_msg_copy_mapped,                "update_msg_copy_mapped",                true},
    {OTA_CMD_SETUP_MAPPED_STORAGE,  update_msg_setup_mapped_storage,       "update_msg_setup_mapped_storage",       true},
#endif
};

/**
 * @brief process the rpmsg according to the command type
 * @param[in] monitor pointer to an update monitor instance
 * @return a reply message including command and data length
 */
static int update_msg_processer(update_monitor_dev_t *dev)
{
    /* clear send data buffer */
    update_msg_cmd_t *processer = NULL;
    int i = 0;
    int ret = -1;
    int num = sizeof(update_msg_processer_table) / sizeof(update_msg_cmd_t);
    ASSERT(num < OTA_CMD_MAX);
    memset(dev->send_msg, 0, MAX_RECV_LENGTH);


    /*chose action from cmd*/
    for (i = 0; i < num; i++) {
        if (dev->recv_head->cmd == update_msg_processer_table[i].cmd) {
            processer = &update_msg_processer_table[i];
            DBG("find processer cmd, i = %d, %s\n", i,
                update_msg_processer_table[i].cmd_name);
            break;
        }
    }

    if (!processer || !processer->func) {
        ERROR("can not processer cmd\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_MSG_UNKNOWN_CMD, 0);
        goto fail;
    }

    ret = processer->func(dev);

    if (ret != OTA_ERR_NO_ERR && processer->is_set_errcode) {
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, ret, dev->reply.reply_cmd);
    }

fail:
    return ret;
}

/**
 * @brief The a thread handle the recived rpmsg message and send reply
 * @param[in] arg pointer to an update_monitor
 * @return 0:success other:fail
 */
static void update_msg_handler(void *arg)
{
    update_monitor_dev_t *dev = (update_monitor_dev_t *)arg;
    int ret = -1;

    while (true) {
        /* wait recv event happen */
        osEventFlagsWait(dev->new_msg_event, RECIVE_NEW_EVENT, osFlagsWaitAny,
                         osWaitForever);

#if UPDATE_MONITOR_PRINT_MSG
        ERROR("recv msg:\n");
        msg_printer(dev->recv_msg, dev->recv_head->len + MSG_HEAD_SIZE);
#endif

        if (dev->state != STATE_INIT) {
            ERROR("uptate monitor process thread: state error = %d\n",
                        dev->state);
            return;
        }

        /* check msg */
        ret = update_msg_checker(dev);

        if (ret < 0) {
            ERROR("msg_handler: msg format error");
            continue;
        }

        /* process msg */
        ret = update_msg_processer(dev);

        if (ret == OTA_ERR_MSG_UNKNOWN_CMD) {
            ERROR("msg_handler: msg process error\n");
            continue;
        }

        /* send reply msg */
        ret = update_msg_sender(dev);

        if (ret) {
            ERROR("msg_handler: msg send error\n");
            continue;
        }
    }

    return;
}
/**
 * @brief The callback function when a rpmsg message is recieved
 * @param[in] data pointer to recieved message buffer
 * @param[in] len recieved message length
 * @param[in] src source of rpmsg
 * @param[in] arg parameter of this callback function
 * @return 0:success
 */
static int update_msg_callback(void *data, int len, unsigned long src,
                               void *arg)
{
    if ((!arg) || (!data) || (len <= 0)) {
        ERROR("update monitor msg_callback: para error\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_MSG_CALLBACK_INVALID_PARA, 0);
        return 0;
    }

    update_monitor_dev_t *dev = (update_monitor_dev_t *)arg;
    memset(dev->recv_msg, 0, sizeof(MAX_RECV_LENGTH));
    memcpy(dev->recv_msg, (char *)data, MIN((unsigned int)len,
            MAX_RECV_LENGTH));
    osEventFlagsSet(dev->new_msg_event, RECIVE_NEW_EVENT);
    return 0;
}

static void free_ptdev_index_table(void)
{
    int i = 1;
    for(i = 1; i < NUM_PARTITIONS; i++) {
        if(ptdev_table[i]) {
            free(ptdev_table[i]);
            ptdev_table[i] = NULL;
        }
    }
}

/**
 * @brief scan level-one partition for level-two partition
 *
 * @param base_ptdev_table pointer of level-one partition
 * @param storage disk device
 * @param err errorno
 * @return int 0 is success, other is fail
 */
static int scan_for_level2_ptdev(ptdev_index_t* base_ptdev_table, struct disk_dev* storage, uint32_t* err)
{
    int ret = -1;
    partition_device_t *ptdev = NULL;
    struct partition_entry *partition_entries;
    int partition_count = 0;
    int partiton_cnt = 1;
    ptdev_table[0] = base_ptdev_table;
    set_ptdev_cnt(1);

    if(!base_ptdev_table) {
        ERROR("base ptdev error\n");
        free_ptdev_index_table();
        *err = OTA_ERR_BASE_PARTITION_ERROR;
        return -1;
    }
    memset(base_ptdev_table, 0, sizeof(ptdev_index_t));
#if CONFIG_UPDATE_FLASH
    snprintf(base_ptdev_table->ptdev_name, MAX_GPT_NAME_SIZE, "%s", "ospi1");
#elif CONFIG_UPDATE_EMMC
    snprintf(base_ptdev_table->ptdev_name, MAX_GPT_NAME_SIZE, "%s", "mmc1_top");
    #error ssdk update monitor could not build for a non-safety config
#endif
    base_ptdev_table->is_sub_ptdev = false;

    ret = update_monitor_ptdev_setup(0, storage, DO_UPDATE);
    if(ret != OTA_ERR_NO_ERR) {
        ERROR("setup base ptdev error\n");
        *err = ret;
        return -1;
    }

    ptdev = base_ptdev_table->ptdev;
    partition_entries = ptdev_get_partition_entries(ptdev);
    partition_count = ptdev_get_partition_count(ptdev);

    if(!ptdev || !partition_entries || !partition_count) {
        ERROR("para error\n");
        *err = OTA_ERR_PTDEV_PARA_ERROR;
        return -1;
    }

    for (int i = 0; i < partition_count; i++) {
        if(!memcmp(partition_entries[i].type_guid, secondary_partition_identifier, PARTITION_TYPE_GUID_SIZE)) {
            ptdev_table[partiton_cnt] = (ptdev_index_t *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, sizeof(ptdev_index_t));
            if(!ptdev_table[partiton_cnt]) {
                ERROR("osAllocAlign for %d ptdev_index failed\n", i);
                free_ptdev_index_table();
                *err = OTA_ERR_PTDEV_ALLOC_FAILED;
                return -1;
            }
            memset(ptdev_table[partiton_cnt], 0, sizeof(ptdev_index_t));

#if CONFIG_UPDATE_FLASH
            snprintf(ptdev_table[partiton_cnt]->ptdev_name, UPDATE_MONITOR_PTDEV_NAME_LEN, "ospi1_%s", (char *)(partition_entries[i].name));
            snprintf(ptdev_table[partiton_cnt]->upper_level_ptdev_name, UPDATE_MONITOR_PTDEV_NAME_LEN, "%s", "ospi1");
            snprintf(ptdev_table[partiton_cnt]->upper_level_partition_name, MAX_GPT_NAME_SIZE, "%s", (char *)(partition_entries[i].name));
#elif CONFIG_UPDATE_EMMC
            #error update monitor could not build for a non-safety config
            snprintf(ptdev_table[partiton_cnt]->ptdev_name, UPDATE_MONITOR_PTDEV_NAME_LEN, "mmc1_%s", (char *)(partition_entries[i].name));
            snprintf(ptdev_table[partiton_cnt]->upper_level_ptdev_name, UPDATE_MONITOR_PTDEV_NAME_LEN, "%s", "mmc1_top");
            snprintf(ptdev_table[partiton_cnt]->upper_level_partition_name, MAX_GPT_NAME_SIZE, "%s", (char *)(partition_entries[i].name));
#endif
            ptdev_table[partiton_cnt]->is_sub_ptdev = true;

            DBG("partiton_cnt = %d find ptdev_name :[%s], upper_level_ptdev_name:[%s], upper_level_partition_name:[%s]\n", partiton_cnt, ptdev_table[partiton_cnt]->ptdev_name, \
                    ptdev_table[partiton_cnt]->upper_level_ptdev_name, \
                    ptdev_table[partiton_cnt]->upper_level_partition_name);

            partiton_cnt++;
        }
    }

    DBG("partiton cnt = %d\n", partiton_cnt);
    *err = OTA_ERR_NO_ERR;
    return partiton_cnt;
}

/**
 * @brief register a boot informaion for ota-lib(in AP processer)
 *
 * @param dev[in] dev pointer to an update monitor device
 * @param boot_mode a flag for skip update or need update
 * @param ptdev_cnt partition table cnt
 * @return int
 */
static int register_boot_info(update_monitor_dev_t *dev, update_monitor_mode_e boot_mode, int ptdev_cnt)
{
    /* register boot mode  */
    char *cmd = (char *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, ONE_BOOT_INFO_LEN);
    if(!cmd) {
        ERROR("failed to calloc for cmd\n");
        return OTA_ERR_CMD_ALLOC_FAILED;
    }
    snprintf(cmd, ONE_BOOT_INFO_LEN, "%s%d", BOOT_INFO_PREFIX, boot_mode);
    strcat((char *)(dev->boot_info), cmd);

    /* register level 2 ptdev */
    for(int i = 1; i < ptdev_cnt; i++) {
        memset(cmd, 0, ONE_BOOT_INFO_LEN);
        snprintf(cmd, ONE_BOOT_INFO_LEN, "%s%s", LEVEL2_PTDEV_PREFIX, \
            ptdev_table[i]->ptdev_name);

        if((strlen((char *)(dev->boot_info)) +  strlen(cmd)) >= MAX_DATA_LENGTH) {
            ERROR("failed to generate boot info\n");
            free(cmd);
            return OTA_ERR_GENERATE_BOOT_INFO;
        }
        strcat((char *)(dev->boot_info), cmd);
    }

    DBG("update monitor boot info is %s\n", (char *)(dev->boot_info));
    free(cmd);
    return OTA_ERR_NO_ERR;
}

/**
 * @brief init an update monitor
 * @param[in] no the inst number of update monitor
 * @param[in] dev pointer of pointer to an update monitor device
 * @param[in] cfg pointer to the configuration of update monitor
 * @return 0:success  other:fail
 */
static int update_monitor_init(uint8_t no, update_monitor_dev_t **pdev, update_monitor_cfg_t *cfg)
{
    int ret = 0;
    int ptdev_cnt = 0;
    osThreadAttr_t update_msg_thread_attr = {0};
    uint32_t err = OTA_ERR_UNKNOWN_ERROR;
    update_monitor_dev_t *dev = NULL;

    /* check the configuration */
    if (!cfg) {
        ERROR("update monitor init:cfg is NULL\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_CFG_NOT_EXIST, 0);
        goto end;
    }

    /* calloc a update monitor instance */
    dev = (update_monitor_dev_t *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, sizeof(update_monitor_dev_t));

    if (!dev) {
        ERROR("update monitor init: calloc fail.\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_CFG_CALLOC_UM, 0);
        goto end;
    }

    /* init update monotir name  */
    memset(dev, 0, sizeof(update_monitor_dev_t));
    memset(dev->name, 0, UPDATE_MONITOR_NAME_LENGTH);
    sprintf(dev->name, "update_monitor_%d", no);
    dev->recv_head = (update_msg_head_t *)dev->recv_msg;
    dev->send_head = (update_msg_head_t *)dev->send_msg;
    dev->state = STATE_UN_INIT;
    dev->seek_addr = 0;
    dev->write_len_total = 0;
    dev->write_buf = NULL;
    dev->write_buf_flag = 0;
    dev->reply = (msg_reply_t) {0, 0};
    dev->storage = &disk_inst;
    dev->capacity = 0;
    dev->erase_size = 0;
    dev->block_size = 0;
    dev->load_storage = 0;
    dev->mode = MODE_NEED_UPDATE;
    memset(dev->boot_info, 0, sizeof(dev->boot_info));
#if CONFIG_NO_UPDATE
    dev->mode = MODE_SKIP_UPDATE;
#else
    dev->mode = MODE_NEED_UPDATE;
#endif

    if(dev->mode != MODE_SKIP_UPDATE) {
        if(cfg->disk_type != DISK_TYPE_OSPI1) {
            ERROR("disk type %d do not support\n", cfg->disk_type);
            ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_STORAGE_TYPE_ERRPR, 0);
            goto error;
        }

        if((0 != um_disk_init(DISK_NOR_FLASH_NAME(0), dev->storage))) {
            ERROR("disk init error for %s\n", DISK_NOR_FLASH_NAME(0));
            ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_STORAGE_DEV_NULL, 0);
            goto error;
        }
    }

    /* scan for sub ptdev */
    if(dev->mode != MODE_SKIP_UPDATE) {
        ptdev_cnt = scan_for_level2_ptdev(&base_ptdev_table, dev->storage, &err);
        if((ptdev_cnt < 1) || (ptdev_cnt > NUM_PARTITIONS) || (err != OTA_ERR_NO_ERR)) {
            ERROR("failed to scan partition table, ptdev cnt = %d, err = %d\n", ptdev_cnt, err);
            set_ptdev_cnt(0);
            ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, err, SCAN_FOR_LEVEL2_ERROR_DATA);
            goto error;
        }
        set_ptdev_cnt(ptdev_cnt);
    }

    /* get boot information */
    ret = register_boot_info(dev, dev->mode, ptdev_cnt);
    if(OTA_ERR_NO_ERR != ret) {
        ERROR("failed to register boot info, err = %d\n", ret);
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, ret, REGISTER_BOOT_INFO_ERROR_DATA);
        goto error;
    }

    /* get a rpmsg instance */
    dev->rpmsg_dev = rpmsg_get_instance(cfg->rpmsg_type, cfg->remote_proc);

    if (!dev->rpmsg_dev) {
        ERROR("update monitor init: failed to create an rpmsg device\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_CREATE_IPCC_DEV_FAILED, 0);
        goto error;
    }

    /* create a rpmsg channel */
    dev->rpmsg_chan = rpmsg_channel_create(dev->rpmsg_dev, UPDATE_MONITOR,
                          UPDATE_MONITOR_EPT);

    if (dev->rpmsg_chan == NULL) {
        ERROR("update monitor init: failed to create rpmsg channel\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_CREATE_IPCC_CHAN_FAILED, 0);
        goto error;
    }

    /* init a new msg event */
    dev->new_msg_event = osEventFlagsNew(NULL);

    /* wait */
    osDelay(osKernelGetTickFreq());

    /* creat a rpmsg handle thread */
    update_msg_thread_attr.name       = (const char *)dev->name;
    update_msg_thread_attr.stack_size = STACK_SIZE;
    update_msg_thread_attr.priority   = osPriorityNormal;
    dev->msg_process_thread = osThreadNew(update_msg_handler,
                                  (void *)dev,
                                  &update_msg_thread_attr);

    if (!dev->msg_process_thread) {
        ERROR("update monitor init: failed to create update msg process thread\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_CREATE_THREAD_FAILED, 0);
        goto error;
    }

    /* start the rpmsg channel */
    ret = rpmsg_channel_start(dev->rpmsg_chan, update_msg_callback,
                              (void *)dev);

    if (ret != 0) {
        ERROR("update monitor init: start rpmsg channel failed.\n");
        ERRCODE_PRINT(MOD_ID_OTA, DEFAULT_MOD_NO, OTA_ERR_START_IPCC_ERRPR, 0);
        goto error;
    }

    dev->state = STATE_INIT;
    *pdev = dev;
    ERROR("update monitor init ok\n");
    return 0;

error:
    if (dev->msg_process_thread)
        osThreadTerminate(dev->msg_process_thread);

    if (dev)
        free(dev);

end:
    return -1;
}

/**
 * @brief start update monitor service
 * @param[in] cfg pointer to the configuration array for update monitor
 * @return 0:success  other:fail
 */
int update_monitor_start(update_monitor_cfg_t *cfg)
{
    uint8_t i = 0;
    int ret = 0;

    for (i = 0; i < UPDATE_MON_NUM; i++) {
        ret = update_monitor_init(i, &update_mon[i], &cfg[i]);

        if (ret) {
            ERROR("update monitor %d init error\n", i);
            return -1;
        }
    }

    return 0;
}

