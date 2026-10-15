/**
 * @file ab_partition_parse.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __AB_PART_H__
#define __AB_PART_H__
#include <stdio.h>
#include <lib/partition/partition_parser.h>

extern const char *suffix_slot[];
extern const char *suffix_delimiter;

#define SUFFIX_SLOT(part_slot) suffix_slot[(part_slot)]
#define MAX_SLOT_SUFFIX_SZ      3
#define BOOT_DEV_NAME_SIZE_MAX  10
#define MAX_RSP_SIZE            64
#define MAX_GET_VAR_NAME_SIZE   256
#define MAX_NR_SCAN_FOR_SLOT    16
#define PTDEV_AB_SLOT_MASK      0xFFFF

typedef struct {
    char Suffix[MAX_SLOT_SUFFIX_SZ];
} Slot;

#define SET_BIT(p,n) ((p) |= ((uint64_t)0x1 << (n)))
#define CLR_BIT(p,n) ((p) &= (~(((uint64_t)0x1) << (n))))

typedef enum part_attr {
    ATTR_UNBOOTABLE,
    ATTR_ACTIVE,
    ATTR_SUCCESSFUL,
    ATTR_RETRY,
    ATTR_NUM
} part_attr_t;

/* Structure to print get var info */
struct ab_slot_info {
    char slot_is_unbootable[MAX_GET_VAR_NAME_SIZE];
    char slot_is_unbootable_rsp[MAX_RSP_SIZE];
    char slot_is_active[MAX_GET_VAR_NAME_SIZE];
    char slot_is_active_rsp[MAX_RSP_SIZE];
    char slot_is_succesful[MAX_GET_VAR_NAME_SIZE];
    char slot_is_succesful_rsp[MAX_RSP_SIZE];
    char slot_retry_count[MAX_GET_VAR_NAME_SIZE];
    char slot_retry_count_rsp[MAX_RSP_SIZE];
};

/* A/B support API(s) */
/* Check Multislot is supported */
bool ptdev_multislot_is_supported(partition_device_t
                                  *part_dev);
/* Calling to scan part. table. */
bool ptdev_scan_for_multislot(partition_device_t
                              *part_dev);
/* Marking slot active */
void ptdev_mark_active_slot(partition_device_t *part_dev,
                            int slot);
/* Resetting slot attr. */
void ptdev_reset_attributes(partition_device_t *part_dev,
                            unsigned index);

/* Fill slot meta infomation */
void ptdev_fill_slot_meta(partition_device_t *part_dev,
                          struct ab_slot_info *slot_info);

/* Switching slots */

/* Mark slot unbootable and reset other attributes*/
void ptdev_deactivate_slot(partition_device_t *part_dev,
                           int slot);

/* Mark slot bootable and set other attributes*/
void ptdev_activate_slot(partition_device_t *part_dev,
                         int slot);

/* Find bootable partition */
int ptdev_find_boot_slot(partition_device_t
                         *part_dev);
/* Find bootable partition */
int ptdev_find_bootable_slot(partition_device_t
                             *part_dev);
/* Find current active partition*/
int ptdev_find_active_slot(partition_device_t
                           *part_dev);
int get_inverse_slot(partition_device_t *part_dev, int slot);
/* Fill partition slot info meta*/
int ptdev_find_successfull_slot(partition_device_t *part_dev);
int ptdev_fill_partition_meta(partition_device_t *part_dev,
                              char has_slot_pname[][MAX_GET_VAR_NAME_SIZE],
                              char has_slot_reply[][MAX_RSP_SIZE],
                              int array_size);

/* partition ota roll back check*/
void ptdev_roll_back_check(partition_device_t *part_dev);
/* check if a slot have bootable attr */
int ptdev_check_slot_bootable_attr(partition_device_t *ptdev,
                                   unsigned slot);
int ptdev_mark_slot_attr_noupdate(partition_device_t *part_dev, int slot,
                                   int attr);
int ptdev_mark_slot_attr(partition_device_t *part_dev, int slot, int attr);
int ptdev_clean_slot_attr_noupdate(partition_device_t *part_dev, int slot,
                                    int attr);
int ptdev_clean_slot_attr(partition_device_t *part_dev, int slot, int attr);
int set_slot_retrycnt(partition_device_t *part_dev, int slot,
                      uint8_t boot_retry_count);
int ptdev_get_slot_retrycnt(partition_device_t *part_dev, int slot);
void ptdev_mark_slot_temp(partition_device_t *part_dev, unsigned slot,
                          int attr);
 /* get slot attributes */
int get_ab_slot_attribute(partition_device_t *part_dev, uint32_t *attr);
int set_ab_slot_attribute(partition_device_t *part_dev, uint32_t attr); /* set slot attributes */
int ptdev_attributes_update(partition_device_t *part_dev);
void ptdev_reset_all_attributes(partition_device_t *part_dev);
#endif /* __AB_PART_H__ */
