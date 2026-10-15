#ifndef __STR_STORE_H__
#define __STR_STORE_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <types.h>
#include <lib/partition/partition_parser.h>
#include <lib/disk/disk.h>

/* disk device */
typedef struct disk_information {
    const char               disk_name[MAX_GPT_NAME_SIZE + 1];
    struct disk_dev         *disk_inst;
    struct partition_device *ptdev;
    uint64_t                 ptb_offset;
} DISK_INFO_T;

DISK_INFO_T *disk_dev_init(const char *disk_name);
uint64_t disk_ptdev_get_ptn_and_size(DISK_INFO_T *disk, const char *pt_name,
                                     uint64_t *ptn);
void disk_dev_deinit(DISK_INFO_T *disk);

#endif
