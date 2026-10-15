/**
 * @file ab_partition_parse.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <cmsis_os2.h>
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <debug.h>
#include <lib/cksum.h>
#include <compiler.h>
#include <types.h>
#include <lib/partition/partition_parser.h>
#include <lib/partition/ab_partition_parser.h>

//#define AB_DEBUG
#define calc_sect_base_addr(addr, sect_size) (((addr) / (sect_size)) * (sect_size))
/* Slot suffix */
#define SUFFIX_LEN 2
const char *suffix_slot[] = {"_a",
                             "_b"
                            };
const char *suffix_delimiter = "_";

uint64_t att_val_array[ATTR_NUM] = {
    [ATTR_UNBOOTABLE] = PART_ATT_UNBOOTABLE_VAL,
    [ATTR_ACTIVE] = PART_ATT_ACTIVE_VAL | PART_ATT_PRIORITY_VAL,
    [ATTR_SUCCESSFUL] = PART_ATT_SUCCESSFUL_VAL,
    [ATTR_RETRY] = PART_ATT_MAX_RETRY_COUNT_VAL,
};

static void ptdev_switch_slots(partition_device_t *part_dev, int old_slot, int new_slot);      /* Switching slots */
static uint32_t find_char(const uint8_t *buf, uint32_t buf_len, const char c)
{
    for (uint32_t i = 0; i < buf_len; i++) {
        if (buf[i] == (uint8_t)c)
            return 1;
    }

    return 0;
}
/**
 * @brief get the lowest common multiple for "size"
 *
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
 * @brief  get the greatest common divisor for "size"
 *
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
 * @brief print reboot reason
 *
 * @param reason
 */
static void reboot_device(int reason)
{
    printf("reboot reason:%d\n", reason);
}

/**
 * @brief memcpy by size
 *
 * @param dest
 * @param dst_size
 * @param src
 * @param src_size
 * @return size_t
 */
static size_t memscpy(void *dest, size_t dst_size, const void *src,
                      size_t src_size)
{
    size_t copy_size = MIN(dst_size, src_size);
    memcpy(dest, src, copy_size);
    return copy_size;
}

/**
 * @brief strrev
 *
 * @param str
 */
static void strrev(unsigned char *str)
{
    int i;
    int j;
    unsigned char a;
    unsigned len = strlen((const char *)str);

    for (i = 0, j = len - 1; i < j; i++, j--) {
        a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

/**
 * @brief int to asic
 *
 * @param num
 * @param str
 * @param len
 * @param base
 * @return int
 */
static int itoa_s(int num, unsigned char *str, int len, int base)
{
    int sum = num;
    int i = 0;
    int digit;

    if (len == 0)
        return -1;

    do {
        digit = sum % base;

        if (digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;

        sum /= base;
    }
    while (sum && (i < (len - 1)));

    if (i == (len - 1) && sum)
        return -1;

    str[i] = '\0';
    strrev(str);
    return 0;
}

/* local functions. */
static void mark_all_partitions_active(partition_device_t *part_dev,
                                       unsigned slot);
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot);

static void
swap_guid(partition_device_t *part_dev, int new_slot);
static char *ptdev_scan_for_multislot_byname(partition_device_t *part_dev,
        char *name);
/**
 * @brief Read slot attribute of of the partition_entry
 *
 * @param partition_entries
 * @param index
 * @return true
 * @return false
 */
static inline bool slot_is_active(struct partition_entry *partition_entries,
                           unsigned index)
{
    return !!(partition_entries[index].attribute_flag & PART_ATT_ACTIVE_VAL);
}

/**
 * @brief Get a slot's successful bit
 *
 * @param partition_entries
 * @param index
 * @return true
 * @return false
 */
static inline bool slot_is_sucessful(struct partition_entry *partition_entries,
                              unsigned index)
{
    return !!(partition_entries[index].attribute_flag &
              PART_ATT_SUCCESSFUL_VAL);
}

/**
 * @brief Get a slot's retry count bit
 *
 * @param partition_entries
 * @param index
 * @return unsigned
 */
static inline uint8_t slot_retry_count(struct partition_entry
                                       *partition_entries,
                                 unsigned index)
{
    return (((partition_entries[index].attribute_flag
              & PART_ATT_MAX_RETRY_COUNT_VAL) >> PART_ATT_MAX_RETRY_CNT_BIT) & 0xFF);
}

/**
 * @brief Get a slot's priority bit
 *
 * @param partition_entries
 * @param index
 * @return unsigned
 */
static inline uint8_t slot_priority(struct partition_entry *partition_entries,
                              unsigned index)
{
    return (((partition_entries[index].attribute_flag
              & PART_ATT_PRIORITY_VAL) >> PART_ATT_PRIORITY_BIT) & 0xFF);
}

/**
 * @brief Get a slot's bootalbe bit
 * @param partition_entries
 * @param index
 * @return true
 * @return false
 */
static inline bool slot_is_bootable(struct partition_entry *partition_entries,
                             unsigned index)
{
    return !((partition_entries[index].attribute_flag &
              PART_ATT_UNBOOTABLE_VAL) >> PART_ATT_UNBOOTABLE_BIT);
}

/**
 * @brief Get the slot attribute object
 * @param partition_entries
 * @param index
 * @return uint8_t
 */
static inline uint8_t get_slot_attribute(struct partition_entry
        *partition_entries,
                                  unsigned index)
{
    return ((partition_entries[index].attribute_flag >> PART_ATT_PRIORITY_BIT) &
            0xFF);
}

/**
 * @brief Get the inverse slot object
 * @param part_dev
 * @param slot
 * @return int
 */
int get_inverse_slot(partition_device_t *part_dev, int slot)
{
    if (part_dev->multislot_support)
        return (slot == SLOT_A) ? SLOT_B : SLOT_A;

    return INVALID;
}


int get_ab_slot_attribute(partition_device_t *part_dev, uint32_t *attr)
{
    uint8_t attr_a = 0;
    uint8_t attr_b = 0;
    uint16_t attr_a_b = 0;
    uint16_t attr_a_b_inverse = 0;
    struct partition_entry *partition_entries = ptdev_get_partition_entries(part_dev);

    if (!part_dev || !partition_entries || !attr) {
        PT_WARN("Invalide partition dev or attr addr\n");
        return INVALID;
    }

    /* AB slot */
    if (part_dev->multislot_support) {
        attr_a = get_slot_attribute(partition_entries, part_dev->boot_slot_index[SLOT_A]);
        attr_b = get_slot_attribute(partition_entries, part_dev->boot_slot_index[SLOT_B]);
    }
    /* for Non-AB slot , attr value is 0 */
    else {
        attr_a = 0;
        attr_b = 0;
    }

    attr_a_b =  attr_a + ((attr_b << 8) & 0xFF00);
    attr_a_b_inverse = PTDEV_AB_SLOT_MASK - attr_a_b;

    /* attr_a_b_inverse + attr_a_b = PTDEV_AB_SLOT_MASK */
    *attr = attr_a_b + ((attr_a_b_inverse << 16) & 0xFFFF0000);

    return 0;
}

static int gpr_attr_check(uint32_t attr)
{
    uint8_t attr_a = 0;
    uint8_t attr_b = 0;
    uint16_t attr_a_b = 0;
    uint16_t attr_a_b_inverse = 0;
    bool actvie_a = 0;
    bool actvie_b = 0;

    attr_a_b = attr & 0xFFFF;
    attr_a_b_inverse = (attr >> 16) & 0xFFFF;

    if(PTDEV_AB_SLOT_MASK != (attr_a_b + attr_a_b_inverse)) {
        PT_WARN("gpr attr value check error, attr = 0x%x, check value = 0x%x\n", attr_a_b, attr_a_b_inverse);
        return INVALID;
    }

    attr_a = attr_a_b & 0xFF;
    attr_b = (attr_a_b >> 8) & 0xFF;
    actvie_a = !!(((uint64_t)attr_a << PART_ATT_START_BIT) & PART_ATT_ACTIVE_VAL);
    actvie_b = !!(((uint64_t)attr_b << PART_ATT_START_BIT) & PART_ATT_ACTIVE_VAL);

    if((true == actvie_a) && (true == actvie_b)) {
        PT_WARN("gpr attr value check error, both a and b is active\n");
        return INVALID;
    }

    return 0;
}

int set_ab_slot_attribute(partition_device_t *part_dev, uint32_t attr)
{
    char *pname = NULL;
    char *suffix_str = NULL;
    int i = 0;
    uint8_t attr_a = 0;
    uint8_t attr_b = 0;
    int partition_count = ptdev_get_partition_count(part_dev);
    struct partition_entry *partition_entries = ptdev_get_partition_entries(part_dev);

    if (!part_dev || !partition_entries || !partition_count) {
        PT_WARN("Invalide partition dev addr\n");
        return INVALID;
    }

    /* skip if we have a Non-AB partition */
    if (false == part_dev->multislot_support) {
        PT_INFO("skip attributes setting: partition is Non-AB\n");
        return 0;
    }

    if(0 != gpr_attr_check(attr)) {
        PT_WARN("skip attributes setting: gpr attr check error\n");
        return INVALID;
    }

    attr_a = attr & 0xFF;
    attr_b = (attr >> 8) & 0xFF;

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, suffix_slot[SLOT_A])) {
                if(get_slot_attribute(partition_entries, i) != attr_a) {
                    partition_entries[i].attribute_flag &= ~PART_ATT_ALL;
                    partition_entries[i].attribute_flag |= ((uint64_t)attr_a << PART_ATT_START_BIT);
                    part_dev->attributes_updated = true;
                }
            }
            else if (!strcmp(suffix_str, suffix_slot[SLOT_B])) {
                if(get_slot_attribute(partition_entries, i) != attr_b) {
                    partition_entries[i].attribute_flag &= ~PART_ATT_ALL;
                    partition_entries[i].attribute_flag |= ((uint64_t)attr_b << PART_ATT_START_BIT);
                    part_dev->attributes_updated = true;
                }
            }
        }
        else {
            if(get_slot_attribute(partition_entries, i) == 0) {
                CLR_BIT(partition_entries[i].attribute_flag,
                        PART_ATT_UNBOOTABLE_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
                SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
                partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                        PART_ATT_MAX_RETRY_COUNT_VAL);
                part_dev->attributes_updated = true;
            }
        }
    }

    if(true == part_dev->attributes_updated) {
        PT_INFO("slot attributes setting to slot_a = 0x%x, attr_b = 0x%x\n", attr_a, attr_b);
    }
    else {
        PT_INFO("skip attributes setting: do not need update\n");
    }

    /* update the active slot */
    ptdev_find_active_slot(part_dev);

    if (part_dev->attributes_updated) {
        ptdev_attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

    return 0;
}
/**
 * @brief Decrease a slot's retrycnt
 *
 * @param part_dev
 * @param slot
 */
static void
decrease_slot_retrycnt(partition_device_t *part_dev, unsigned slot)
{
    int i;
    char *pname = NULL;
    char *suffix_str = NULL;
    uint64_t boot_retry_count;
    struct partition_entry *partition_entries;
    int partition_count = 0;

    if (NULL == part_dev) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("mark_slot_attr:slot =%d unknown\n", slot);
        return;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        PT_INFO("Transversing partition %s\n", pname);
#endif

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                boot_retry_count = slot_retry_count(partition_entries, i);

                if ((boot_retry_count > 0) && (boot_retry_count < MAX_RETRY_COUNT)) {
                    partition_entries[i].attribute_flag &= ~PART_ATT_MAX_RETRY_COUNT_VAL;
                    partition_entries[i].attribute_flag |= ((boot_retry_count - 1) <<
                                                            PART_ATT_MAX_RETRY_CNT_BIT);
                    part_dev->attributes_updated = true;
                }
            }
        }
    }

}

/**
 * @brief Inactive a new slot.
 * @param part_dev
 * @param slot
 */
void
ptdev_deactivate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("ERROR: slot= %d  wrong slot number\n", slot);
        return;
    }

    count = part_dev->count;
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* Set Unbootable bit */
                SET_BIT(partition_entries[i].attribute_flag,
                        PART_ATT_UNBOOTABLE_BIT);

                /* Clear Sucess bit and Active bits */
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Clear Max retry count and priority value */
                partition_entries[i].attribute_flag &= (~PART_ATT_PRIORITY_VAL &
                                                        ~PART_ATT_MAX_RETRY_COUNT_VAL);
                part_dev->attributes_updated = true;
            }
        }
    }

    return;
}

/**
 * @brief Active a new slot.
 * use after fisrt start up or after OTA update
 * @param part_dev
 * @param slot
 */
void
ptdev_activate_slot(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("ERROR: slot= %d  wrong slot number\n", slot);
        return;
    }

    count = part_dev->count;
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* CLR Unbootable bit and Sucess bit*/
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_UNBOOTABLE_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);

                /* Set Active bits */
                SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Set Max retry count and priority value */
                partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                        PART_ATT_MAX_RETRY_COUNT_VAL);
            }
            else {
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
                partition_entries[i].attribute_flag &= (~PART_ATT_PRIORITY_VAL);
            }
        }
        else {
            CLR_BIT(partition_entries[i].attribute_flag,
                    PART_ATT_UNBOOTABLE_BIT);
            CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
            SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
            partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                    PART_ATT_MAX_RETRY_COUNT_VAL);
        }
        part_dev->attributes_updated = true;
    }

    return;
}

/**
 * @brief Scan boot partition to find SLOT_A/SLOT_B suffix.
    If found than make multislot_boot flag true and
    scans another partition.
 * @param part_dev
 * @return true
 * @return false
 */
bool ptdev_scan_for_multislot(partition_device_t *part_dev)
{
    uint32_t j, len, count;
    char *tmp1, *tmp2;
    uint32_t partition_count;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return false;
    }

    partition_count = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);

    /* Intialize all slot specific variables */
    part_dev->multislot_support = false;
    part_dev->active_slot = INVALID;
    part_dev->attributes_updated = false;
    part_dev->boot_slot_index[0] = 0;
    part_dev->boot_slot_index[1] = 0;

    if (partition_count > NUM_PARTITIONS) {
        PT_WARN("ERROR: partition_count more than supported.\n");
        return part_dev->multislot_support;
    }

    int scan_nr = partition_count > MAX_NR_SCAN_FOR_SLOT ? MAX_NR_SCAN_FOR_SLOT
                  : partition_count;

    for (int m = 0; m < scan_nr; m++) {
        tmp1 = (char *)partition_entries[m].name;
        len = strlen(tmp1);

        if (len < 3)
            continue; /* too few, ignore */

        for (int x = m + 1; x < scan_nr; x++) {
            if (!strncmp((const char *)tmp1, (char *)partition_entries[x].name,
                         len - SUFFIX_LEN) && (len == strlen((char *)partition_entries[x].name))) {
                tmp1 = tmp1 + len - SUFFIX_LEN;
                tmp2 = (char *)(partition_entries[x].name + len - SUFFIX_LEN);
                count = 0;

                for (j = 0; j < AB_SUPPORTED_SLOTS; j++) {
                    if (!strcmp(tmp1, suffix_slot[j]) || !strcmp(tmp2, suffix_slot[j]))
                        count++;
                }

                /* Break out of loop if all slot index are found */
                if (count == AB_SUPPORTED_SLOTS) {
                    part_dev->multislot_support = true;
                    part_dev->boot_slot_index[0] = m;
                    part_dev->boot_slot_index[1] = x;
                    break;
                }
            }
        }

        if (part_dev->multislot_support)
            break;
    }

    return part_dev->multislot_support;
}

/**
 * @brief Scan a partition name to find SLOT_A/SLOT_B suffix.
 *
 * @param part_dev
 * @param name
 * @return char*
 */
char *ptdev_scan_for_multislot_byname(partition_device_t *part_dev, char *name)
{
    uint32_t x, j, len, count;
    char *tmp1, *tmp2;
    uint32_t partition_count;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries) || (!name)) {
        PT_WARN("Invalide partition dev\n");
        return NULL;
    }

    partition_count = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);

    len = strlen(name);

    if (len < 3)
        return NULL;

    for (x = 0; x < partition_count; x++) {
        tmp1 = name;

        if (!strncmp((const char *)tmp1, (char *)partition_entries[x].name,
                     len - SUFFIX_LEN) && (len == strlen((char *)partition_entries[x].name))) {
            tmp1 = tmp1 + len - SUFFIX_LEN;
            tmp2 = (char *)(partition_entries[x].name + len - SUFFIX_LEN);
            count = 0;

            for (j = 0; j < AB_SUPPORTED_SLOTS; j++) {
                if (!strcmp(tmp1, suffix_slot[j]) || !strcmp(tmp2, suffix_slot[j]))
                    count++;
            }

            if (count == AB_SUPPORTED_SLOTS) {
                return tmp1;
            }
        }
    }

    return NULL;
}

/*
    Function: To reset partition attributes
    This function reset partition_priority, retry_count
    and clear successful and bootable bits.
 */
void ptdev_reset_attributes(partition_device_t *part_dev, unsigned index)
{
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    partition_entries[index].attribute_flag |= (PART_ATT_PRIORITY_VAL |
            PART_ATT_MAX_RETRY_COUNT_VAL);

    partition_entries[index].attribute_flag &= ((~PART_ATT_SUCCESSFUL_VAL) &
            (~PART_ATT_UNBOOTABLE_VAL));

    if (!part_dev->attributes_updated)
        part_dev->attributes_updated = true;

    /* Make attributes persistant */
    ptdev_mark_active_slot(part_dev, part_dev->active_slot);
}




/**
 * @brief Function: Switch active partitions.
 * @param part_dev
 * @param old_slot
 * @param new_slot
 */
static void ptdev_switch_slots(partition_device_t *part_dev, int old_slot,
                        int new_slot)
{
    struct partition_entry *partition_entries;
    int count;
    char *pname;
    char *suffix_str = NULL;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if ( (old_slot != SLOT_A) && (old_slot != SLOT_B)) {
        PT_WARN("ERROR: old_slot= %d  wrong slot number\n", old_slot);
        return;
    }

    if ( (new_slot != SLOT_A) && (new_slot != SLOT_B)) {
        PT_WARN("ERROR: new_slot= %d  wrong slot number\n", new_slot);
        return;
    }

    if (new_slot == old_slot) {
        return;
    }

    count = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);

    for (int i = 0; i < count; i++) {
        pname = (char *)partition_entries[i].name;

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            /*old active slot */
            if (!strcmp(suffix_str, SUFFIX_SLOT(old_slot))) {
                /* Set Unbootable bit */
                SET_BIT(partition_entries[i].attribute_flag,
                        PART_ATT_UNBOOTABLE_BIT);
                /* Clear Sucess bit and Active bits */
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Clear Max retry count and priority value */
                partition_entries[i].attribute_flag &= (~PART_ATT_PRIORITY_VAL &
                                                        ~PART_ATT_MAX_RETRY_COUNT_VAL);
            }
            /*new active slot */
            else if (!strcmp(suffix_str, SUFFIX_SLOT(new_slot))) {
                /* CLR Unbootable bit*/
                CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_UNBOOTABLE_BIT);
                //CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);

                /* Set Active bits */
                SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);

                /* Set Max retry count and priority value */
                partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                        PART_ATT_MAX_RETRY_COUNT_VAL);
            }
        }
        else {
            CLR_BIT(partition_entries[i].attribute_flag,
                    PART_ATT_UNBOOTABLE_BIT);
            //CLR_BIT(partition_entries[i].attribute_flag, PART_ATT_SUCCESS_BIT);
            SET_BIT(partition_entries[i].attribute_flag, PART_ATT_ACTIVE_BIT);
            partition_entries[i].attribute_flag |= (PART_ATT_PRIORITY_VAL |
                                                    PART_ATT_MAX_RETRY_COUNT_VAL);
        }
        part_dev->attributes_updated = true;
    }

    swap_guid(part_dev, new_slot);
    part_dev->active_slot = new_slot;
    return;
}

/**
 * @brief This function returns the most priority and active slot,
 * also you need to update the global state seperately.
 * @param part_dev
 * @return int
 */
int ptdev_find_active_slot(partition_device_t *part_dev)
{
    uint8_t current_priority;
    int i, count = 0;
    bool current_active_bit;
    unsigned boot_priority;
    struct partition_entry *partition_entries;
    bool current_bootable_bit;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        goto out;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
#ifdef AB_DEBUG
    PT_INFO( "ptdev_find_active_slot() called\n");
#endif

    /* Return current active slot if already found */
    if (part_dev->active_slot != INVALID)
        goto out;

    for (boot_priority = (MAX_PRIORITY - 1);
            boot_priority >= 0; boot_priority--) {
        /* Search valid boot slot with highest priority */
        for (i = 0; i < AB_SUPPORTED_SLOTS; i++) {
            current_priority = slot_priority(partition_entries,
                                             part_dev->boot_slot_index[i]);
            current_active_bit = slot_is_active(partition_entries,
                                                part_dev->boot_slot_index[i]);
            current_bootable_bit = slot_is_bootable(partition_entries,
                                                    part_dev->boot_slot_index[i]);

            /* Count number of slots with all attributes as zero */
            if ( !current_priority &&
                    !current_active_bit &&
                    current_bootable_bit) {
                count ++;
                continue;
            }

#ifdef AB_DEBUG
            PT_INFO("Slot:Priority:Active:Bootable %s:%d:%d:%d\n",
                    partition_entries[part_dev->boot_slot_index[i]].name,
                    current_priority,
                    current_active_bit,
                    current_bootable_bit);
#endif

            if (boot_priority == current_priority) {
                if (current_active_bit) {
#ifdef AB_DEBUG
                    PT_INFO( "Slot (%s) is Valid High Priority Slot\n", SUFFIX_SLOT(i));
#endif
                    part_dev->active_slot = i;
                    goto out;
                }
            }
        }

        /* All slots are zeroed, this is first bootup */
        /* Marking and trying SLOT 0 as default */
        if (count == AB_SUPPORTED_SLOTS) {
            /* Update the priority of the boot slot */
            ptdev_activate_slot(part_dev, SLOT_A);

            part_dev->active_slot = SLOT_A;

            /* This is required to mark all bits as active,
            for fresh boot post fresh flash */
            part_dev->attributes_updated = true;
            goto out;
        }
    }

out:
    return part_dev->active_slot;
}

/*
   This function check if a slot is bootable
   1 is bootable, 0 is unbootable, -1 is error
*/
int ptdev_check_slot_bootable_attr(partition_device_t *part_dev, unsigned slot)
{
    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev!\n");
        return -1;
    }
    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("an INVALID active_slot slot!\n");
        return -1;
    }
    return slot_is_bootable(part_dev->partition_entries,
                            part_dev->boot_slot_index[slot]);
}
/**
 * @brief ptdev_roll_back_check
 *
 * @param part_dev
 */
void ptdev_roll_back_check(partition_device_t *part_dev)
{
    int active_slot = INVALID;
    int other_slot;
    uint8_t boot_retry_count = 0;
    struct partition_entry *partition_entries;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev!\n");
        goto out;
    }

    active_slot = ptdev_find_active_slot(part_dev);
    other_slot = get_inverse_slot(part_dev, active_slot);

    if ((active_slot != SLOT_A) && (active_slot != SLOT_B)) {
        PT_WARN("an INVALID active_slot slot!\n");
        goto out;
    }

    if ((other_slot != SLOT_A) && (other_slot != SLOT_B)) {
        PT_WARN("an INVALID inactive_slot slot!\n");
        goto out;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

#ifndef ALWAYS_ROLL_BACK_CHECK
    /*Do not need fallback, if a active slot's partition flag is set to sucessful and bootable*/
    if ( slot_is_bootable(partition_entries,
                          part_dev->boot_slot_index[active_slot]) &&
            slot_is_sucessful(partition_entries, part_dev->boot_slot_index[active_slot])) {
        PT_INFO("Rollback skip! Active slot%s is Successful and Bootable\n",
                 SUFFIX_SLOT(active_slot));
        goto check_update;
    }

    /*Do not need fallback, if a inactive slot's partition flag is not set to sucessful and bootable */
    if ( (!slot_is_bootable(partition_entries,
                            part_dev->boot_slot_index[other_slot])) ||
            (!slot_is_sucessful(partition_entries,
                                part_dev->boot_slot_index[other_slot]))) {
        PT_INFO("Rollback skip! Inactive slot%s is not Successful or not Bootable\n",
                 SUFFIX_SLOT(other_slot));
        goto check_update;
    }

    boot_retry_count = slot_retry_count(partition_entries,
                                        part_dev->boot_slot_index[active_slot]);

    if ((boot_retry_count != 0) && (boot_retry_count < MAX_RETRY_COUNT)) {
        /* Decrement retry count, boot from active slot */
        PT_INFO("Rollback decrement Retrycount from %d to %d, boot from slot%s\n",
                 boot_retry_count, (boot_retry_count - 1), SUFFIX_SLOT(active_slot));
        decrease_slot_retrycnt(part_dev, active_slot);
    }
    else {
        PT_INFO("Rollback from slot%s to slot%s!\n", SUFFIX_SLOT(active_slot),
                 SUFFIX_SLOT(other_slot));
        ptdev_switch_slots(part_dev, active_slot, other_slot);
    }

#else
    if (slot_is_bootable(partition_entries,
                         part_dev->boot_slot_index[active_slot])) {
        boot_retry_count = slot_retry_count(partition_entries,
                                            part_dev->boot_slot_index[active_slot]);
        if ((boot_retry_count != 0) && (boot_retry_count < MAX_RETRY_COUNT)) {
            PT_INFO("Rollback decrement Retrycount from %d to %d, boot from slot%s\n",
                    boot_retry_count, (boot_retry_count - 1), SUFFIX_SLOT(active_slot));
            decrease_slot_retrycnt(part_dev, active_slot);
            goto check_update;
        }
        else if (slot_is_bootable(partition_entries,
                                  part_dev->boot_slot_index[other_slot])) {
            PT_INFO("Rollback from slot%s to slot%s, decrease retry cnt from 3 to 2!\n",
                    SUFFIX_SLOT(active_slot),
                    SUFFIX_SLOT(other_slot));
            ptdev_switch_slots(part_dev, active_slot, other_slot);
            decrease_slot_retrycnt(part_dev, other_slot);
            goto check_update;
        }
        else {
            PT_INFO("Both slots are unbootable, boot from slot%s!\n", SUFFIX_SLOT(SLOT_A));
            part_dev->attributes_updated = true;
            ptdev_deactivate_slot(part_dev, active_slot);
            part_dev->active_slot = INVALID;
            ptdev_mark_active_slot(part_dev, SLOT_A);
            goto check_update;
        }
    }
    else if (slot_is_bootable(partition_entries,
                              part_dev->boot_slot_index[other_slot])) {
        PT_INFO("slot%s is active but not bootable, Rollback to slot%s, decrease retry cnt from 3 to 2!\n",
                SUFFIX_SLOT(active_slot),
                SUFFIX_SLOT(other_slot));
        ptdev_switch_slots(part_dev, active_slot, other_slot);
        decrease_slot_retrycnt(part_dev, other_slot);
        goto check_update;
    }
    else {
        PT_INFO("slot%s is active and both slots are unbootable, boot from slot_a!\n",
                SUFFIX_SLOT(active_slot));
        ptdev_mark_active_slot(part_dev, SLOT_A);
        goto check_update;
    }
#endif

check_update:

    if (part_dev->attributes_updated) {
        ptdev_attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

out:
    return;
}

/**
 * @brief ptdev_find_boot_slot
    Active + Bootable + Successful
    Active + Bootable ---------------->check Retry_count>0
    Bootable + Successful
    Bootable
 * @param part_dev
 * @return int
 */
int ptdev_find_boot_slot(partition_device_t *part_dev)
{
    int boot_slot = INVALID;

    if (part_dev == NULL) {
        PT_WARN("Invalide partition dev!\n");
        return INVALID;
    }

    boot_slot = ptdev_find_active_slot(part_dev);

    if (boot_slot == INVALID) {
        PT_WARN("can't find a bootable slot!\n");
        return INVALID;
    }

    return boot_slot;
}

/**
 * @brief find the successful slot
 * @param part_dev
 * @return int
 */
int ptdev_find_bootable_slot(partition_device_t *part_dev)
{
    int boot_slot = NOT_FIND, unboot_slot = NOT_FIND;
    int slt_index;
    struct partition_entry *partition_entries;

    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    if (!part_dev->multislot_support) {
        PT_WARN("can't find boot slot, multislot not support.\n");
        return INVALID;
    }

    for (int i = 0; i < AB_SUPPORTED_SLOTS; i++) {
        slt_index = part_dev->boot_slot_index[i];

        if (partition_entries[slt_index].attribute_flag & PART_ATT_UNBOOTABLE_VAL) {
            unboot_slot = i;
            continue;
        }

        boot_slot = i;
    }

    if (unboot_slot == NOT_FIND) {
        PT_DBG("both slots mark as bootable\n");
        return AB_SUPPORTED_SLOTS;
    }

    return boot_slot;
}

int ptdev_find_successfull_slot(partition_device_t *part_dev)
{
    int valid = NOT_FIND, invalid = NOT_FIND;
    int slt_index;
    struct partition_entry *partition_entries;

    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    if (!part_dev->multislot_support) {
        PT_WARN("can't find boot slot, multislot not support.\n");
        return INVALID;
    }

    for (int i = 0; i < AB_SUPPORTED_SLOTS; i++) {
        slt_index = part_dev->boot_slot_index[i];

        if (partition_entries[slt_index].attribute_flag & PART_ATT_SUCCESSFUL_VAL) {
            valid = i;
            continue;
        }

        invalid = i;
    }

    if (invalid == NOT_FIND) {
        PT_DBG("both slots marked as successfull\n");
        return AB_SUPPORTED_SLOTS;
    }

    return valid;

}

int ptdev_get_slot_retrycnt(partition_device_t *part_dev, int slot)
{
    struct partition_entry *partition_entries;
    int boot_retry_count = -1;

    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("an INVALID slot!\n");
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);

    if (!part_dev->multislot_support || !partition_entries) {
        PT_WARN("can't find boot slot, multislot not support.\n");
        return INVALID;
    }

    boot_retry_count = slot_retry_count(partition_entries,
                                        part_dev->boot_slot_index[slot]);

    if ((boot_retry_count >= MAX_RETRY_COUNT) || (boot_retry_count < 0)) {
        PT_WARN("boot_retry_count = %d, error\n", boot_retry_count);
        return INVALID;
    }

    return boot_retry_count;
}

/*
    Function: set the slot retry cnt
*/
int set_slot_retrycnt(partition_device_t *part_dev, int slot,
                      uint8_t boot_retry_count)
{
    int i;
    char *pname = NULL;
    char *suffix_str = NULL;
    struct partition_entry *partition_entries;
    int partition_count = 0;
    uint64_t retry_count = boot_retry_count;

    if (NULL == part_dev) {
        PT_WARN("Invalide partition dev\n");
        return INVALID;
    }

    if ( (slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("mark_slot_attr:slot =%d unknown\n", slot);
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                if ((retry_count >= 0) && (retry_count < MAX_RETRY_COUNT)) {
                    partition_entries[i].attribute_flag &= ~PART_ATT_MAX_RETRY_COUNT_VAL;
                    partition_entries[i].attribute_flag |= (retry_count <<
                                                            PART_ATT_MAX_RETRY_CNT_BIT);
                    part_dev->attributes_updated = true;
                }
            }
        }
    }

    return 0;
}
/**
 * @brief update guid for parition
 *
 * @param partition_entries
 * @param old_index
 * @param new_index
 */
static
void guid_update(struct partition_entry *partition_entries,
                 unsigned old_index,
                 unsigned new_index)
{
    unsigned char tmp_guid[PARTITION_TYPE_GUID_SIZE];

#ifdef AB_DEBUG
    PT_INFO( "Swapping GUID (%s) --> (%s) \n",
           partition_entries[old_index].name,
           partition_entries[new_index].name);
#endif
    memcpy(tmp_guid, partition_entries[old_index].type_guid,
           PARTITION_TYPE_GUID_SIZE);
    memcpy(partition_entries[old_index].type_guid,
           partition_entries[new_index].type_guid,
           PARTITION_TYPE_GUID_SIZE);
    memcpy(partition_entries[new_index].type_guid, tmp_guid,
           PARTITION_TYPE_GUID_SIZE);
    return;
}

/**
 * @brief Function to swap guids of slots
 *
 * @param part_dev
 * @param new_slot
 */
static void
swap_guid(partition_device_t *part_dev, int new_slot)
{
    unsigned i, j, tmp_strlen;
    unsigned partition_cnt;
    struct partition_entry *partition_entries;
    const char *ptr_pname, *ptr_suffix;
    int old_slot;

    if (NULL == part_dev) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if ((new_slot != SLOT_A) && (new_slot != SLOT_B)) {
        PT_WARN("ERROR: slot= %d  wrong slot number\n", new_slot);
        return;
    }

    if ((part_dev->active_slot != SLOT_B) && (part_dev->active_slot != SLOT_A)) {
        PT_DBG("first set Active, do not need swap guid\n");
        return;
    }

    partition_cnt = ptdev_get_partition_count(part_dev);
    partition_entries = ptdev_get_partition_entries(part_dev);
    old_slot = part_dev->active_slot;

    if ( old_slot == new_slot)
        return;

    for (i = 0; i < partition_cnt; i++) {
        ptr_pname = (const char *)partition_entries[i].name;
        ptr_suffix = ptdev_scan_for_multislot_byname(part_dev, (char *)ptr_pname);

        if (ptr_suffix) {
            /* Search for suffix in partition name */
            if (!strcmp(ptr_suffix, SUFFIX_SLOT(new_slot))) {
                for (j = i + 1; j < partition_cnt; j++) {
                    tmp_strlen = strlen(ptr_pname) - strlen(SUFFIX_SLOT(new_slot));

                    if (!strncmp((const char *)partition_entries[j].name, ptr_pname,
                                 tmp_strlen) &&
                            strstr((const char *)partition_entries[j].name, SUFFIX_SLOT(old_slot))
                            && strlen(ptr_pname) == strlen((char *)partition_entries[j].name))
                        guid_update(partition_entries, j, i);
                }
            }
            else if (!strcmp(ptr_suffix, SUFFIX_SLOT(old_slot))) {
                for (j = i + 1; j < partition_cnt; j++) {
                    tmp_strlen = strlen(ptr_pname) - strlen(SUFFIX_SLOT(old_slot));

                    if (!strncmp((const char *)partition_entries[j].name, ptr_pname,
                                 tmp_strlen) &&
                            strstr((const char *)partition_entries[j].name, SUFFIX_SLOT(new_slot))
                            && strlen(ptr_pname) == strlen((char *)partition_entries[j].name))
                        guid_update(partition_entries, i, j);
                }
            }
        }
    }
}

/**
 * @brief To set active bit of all partitions of actve slot.
 * also, unset active bits of all other slot
 * @param part_dev
 * @param slot
 */
void
mark_all_partitions_active(partition_device_t *part_dev, unsigned slot)
{
    int i;
    char *pname = NULL;
    char *suffix_str = NULL;
    struct partition_entry *partition_entries =
        ptdev_get_partition_entries(part_dev);
    int partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        PT_INFO( "Transversing partition %s\n", pname);
#endif

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot)))
                /* 2a. Mark matching partition as active. */
                partition_entries[i].attribute_flag |= PART_ATT_ACTIVE_VAL;
            else
                /* 2b. Unset active bit for all other partitions. */
                partition_entries[i].attribute_flag &= ~PART_ATT_ACTIVE_VAL;
        }
        else {
            /* 3. If it isn't A/B enabled, Mark partition as active as well */
            partition_entries[i].attribute_flag |= PART_ATT_ACTIVE_VAL;
        }

    part_dev->attributes_updated = true;
    }
}

/**
 * @brief Mark the slot to be active and also conditionally
 *        update the slot parameters if there is a change.
 * @param part_dev
 * @param slot
 */
void ptdev_mark_active_slot(partition_device_t *part_dev, int slot)
{
    if (!part_dev) {
        PT_WARN("Invalide partition dev\n");
        goto out;
    }

    if (part_dev->active_slot == slot)
        goto out;

    if (slot != INVALID) {
        PT_WARN( "Marking (%s) as active\n", SUFFIX_SLOT(slot));

        /* 1. Swap GUID's to new slot */
        swap_guid(part_dev, slot);

        /* 2. Set Active bit for all partitions of active slot */
        mark_all_partitions_active(part_dev, slot);
    }

    part_dev->active_slot = slot;
out:

    if (part_dev->attributes_updated) {
        ptdev_attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

    return;
}

static int mark_slot_attr(partition_device_t *part_dev, int slot,
                           int attr)
{
    int i;
    char *pname = NULL;
    char *suffix_str = NULL;
    uint64_t part_att_val;
    struct partition_entry *partition_entries;
    int partition_count;
    bool attribute_flag_changed = false;

    if (!part_dev || !(part_dev->partition_entries)) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    if (attr >= ATTR_NUM) {
        PT_WARN("mark_slot_attr:bad part attribute\n");
        return INVALID;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("mark_slot_attr:slot =%d unknown\n", slot);
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    part_att_val = att_val_array[attr];

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        PT_INFO("Transversing partition %s\n", pname);
#endif

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* 2. Mark matching partition as active. */
                if ((partition_entries[i].attribute_flag & part_att_val) != part_att_val) {
                partition_entries[i].attribute_flag |= part_att_val;
                    attribute_flag_changed = true;
                }
            }
        }
        else if (attr == ATTR_SUCCESSFUL) {
            /* 3. Allow marking none A/B partition ATTR_SUCCESSFUL attr*/
            if ((partition_entries[i].attribute_flag & part_att_val) != part_att_val) {
            partition_entries[i].attribute_flag |= part_att_val;
                attribute_flag_changed = true;
            }
        }
    }

    if (attribute_flag_changed)
    part_dev->attributes_updated = true;

    return 0;
}

/*
Function: To mark bit of all partitions of specific slot.
 */
static int clean_slot_attr(partition_device_t *part_dev, int slot,
                            int attr)
{
    int i;
    char *pname = NULL;
    char *suffix_str = NULL;
    uint64_t part_att_val;
    struct partition_entry *partition_entries;
    int partition_count;
    bool attribute_flag_changed = false;

    if (!part_dev || !(part_dev->partition_entries)) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    if (attr >= ATTR_NUM) {
        PT_WARN("clean_slot_attr:bad part attribute\n");
        return INVALID;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("clean_slot_attr:slot =%d unknown\n", slot);
        return INVALID;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);
    part_att_val = att_val_array[attr];

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
#ifdef AB_DEBUG
        PT_INFO("Transversing partition %s\n", pname);
#endif

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(slot))) {
                /* Clean matching partition. */
                if ((partition_entries[i].attribute_flag & (~part_att_val)) !=
                        partition_entries[i].attribute_flag) {
                partition_entries[i].attribute_flag &= ~part_att_val;
                    attribute_flag_changed = true;
                }
            }
        }
    }

    if (attribute_flag_changed)
    part_dev->attributes_updated = true;
    return 0;
}

int
ptdev_mark_slot_attr_noupdate(partition_device_t *part_dev, int slot,
                              int attr)
{
    int inverse_slot;
    int ret = INVALID;

    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        goto end;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("slot is error\n");
        goto end;
    }

    if (attr >= ATTR_NUM) {
        PT_WARN("bad part attribute\n");
        goto end;
    }

    if (attr == ATTR_ACTIVE) {
        /* do not need mark */
        if (part_dev->active_slot == slot) {
            ret = 0;
            goto end;
        }

        inverse_slot = get_inverse_slot(part_dev, slot);
        /* 1. Swap GUID's to new slot */
        swap_guid(part_dev, slot);
        /* 2. Set slot as Active */
        if (0 != mark_slot_attr(part_dev, slot, attr))
            goto end;
        /* 3. Set inverse_slot as Inactive */
        if (0 != clean_slot_attr(part_dev, inverse_slot, attr))
            goto end;
        part_dev->active_slot = slot;
    }
    else {
        /* Set attribute for all partitions */
        if (0 != mark_slot_attr(part_dev, slot, attr))
            goto end;
    }
    ret = 0;
end:
    return ret;
}

/*
Function: To mark bit of all partitions of specific slot.
 */
int
ptdev_clean_slot_attr_noupdate(partition_device_t *part_dev, int slot,
                               int attr)
{
    int inverse_slot;
    int ret = INVALID;

    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        goto end;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("slot is error\n");
        goto end;
    }

    if (attr >= ATTR_NUM) {
        PT_WARN("bad part attribute\n");
        goto end;
    }

    if (attr == ATTR_ACTIVE) {
        inverse_slot = get_inverse_slot(part_dev, slot);

        /* do not need mark */
        if ((part_dev->active_slot != slot)
                && (part_dev->active_slot == inverse_slot)) {
            ret = 0;
            goto end;
        }

        /* 1. Swap GUID's to new slot */
        swap_guid(part_dev, inverse_slot);

        /* 2. slot mark Active */
        if (0 != clean_slot_attr(part_dev, slot, attr))
            goto end;

        /* 3. inverse_slot mark Inactive */
        if (0 != mark_slot_attr(part_dev, inverse_slot, attr))
            goto end;

        part_dev->active_slot = inverse_slot;
    }
    else {
        /* Set attribute for all partitions */
        if (0 != clean_slot_attr(part_dev, slot, attr))
            goto end;
    }

    ret = 0;
end:
    return ret;
}

/*
    Function: Mark the slot attribute and update to the
    storage device if there is a change.
 */
int ptdev_mark_slot_attr(partition_device_t *part_dev, int slot, int attr)
{
    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("slot is error\n");
        return INVALID;
    }

    if (attr > ATTR_NUM) {
        PT_WARN("bad part attribute\n");
        return INVALID;
    }

    if (0 != ptdev_mark_slot_attr_noupdate(part_dev, slot, attr)) {
        return INVALID;
    }

    if (part_dev->attributes_updated) {
        ptdev_attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

    return 0;
}

/*
    Function: Clean the slot attribute and update to the
    storage device if there is a change.
 */
int ptdev_clean_slot_attr(partition_device_t *part_dev, int slot, int attr)
{
    if (NULL == part_dev) {
        PT_WARN("part_dev is NULL\n");
        return INVALID;
    }

    if ((slot != SLOT_A) && (slot != SLOT_B)) {
        PT_WARN("slot is error\n");
        return INVALID;
    }

    if (attr > ATTR_NUM) {
        PT_WARN("bad part attribute\n");
        return INVALID;
    }

    if (0 != ptdev_clean_slot_attr_noupdate(part_dev, slot, attr)) {
        return INVALID;
    }

    if (part_dev->attributes_updated) {
        ptdev_attributes_update(part_dev);
        part_dev->attributes_updated = false;
    }

    return 0;
}
/**
 * @brief Function to find if multislot is supported
 * @param part_dev
 * @return true
 * @return false
 */
bool ptdev_multislot_is_supported(partition_device_t *part_dev)
{
    if (!part_dev) {
        PT_WARN("Invalide partition dev\n");
        return false;
    }

    return part_dev->multislot_support;
}

/**
 * @brief Fill partition meta used for fastboot get var info publication.
    Input partition_entries, partition_count and
    buffer to fill information.
 * @param part_dev
 * @param has_slot_pname
 * @param has_slot_reply
 * @param array_size
 * @return int
 */
int ptdev_fill_partition_meta(partition_device_t *part_dev,
                              char has_slot_pname[][MAX_GET_VAR_NAME_SIZE],
                              char has_slot_reply[][MAX_RSP_SIZE],
                              int array_size)
{
    int i, tmp;
    int count = 0;
    char *pname = NULL;
    int pname_size;
    struct partition_entry *partition_entries;
    int partition_count;
    char *suffix_str;

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return 0;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    for (i = 0; i < partition_count; i++) {
        pname = (char *)partition_entries[i].name;
        pname_size = strlen(pname);
        suffix_str = NULL;
#ifdef AB_DEBUG
        PT_INFO( "Transversing partition %s\n", pname);
#endif

        /* Find partition, if it is A/B enabled */
        suffix_str = ptdev_scan_for_multislot_byname(part_dev, pname);

        if (suffix_str) {
            if (!strcmp(suffix_str, SUFFIX_SLOT(SLOT_A))) {
                /* 2. put the partition name in array */
                tmp = pname_size - strlen(suffix_str);
                memset(has_slot_pname[count], 0x0, MAX_GET_VAR_NAME_SIZE);
                strncpy(has_slot_pname[count], pname, tmp + 1);
                strncpy(has_slot_reply[count], " Yes", MAX_RSP_SIZE);
                count++;
            }
        }
        else {
            memset(has_slot_pname[count], 0x0, MAX_GET_VAR_NAME_SIZE);
            strncpy(has_slot_pname[count], pname, MAX_GET_VAR_NAME_SIZE);
            strncpy(has_slot_reply[count], " No", MAX_RSP_SIZE);
            count++;
        }

        /* Avoid over population of array provided */
        if (count >= array_size) {
            PT_WARN("ERROR: Not able to parse all partitions\n");
            return count;
        }
    }

#ifdef AB_DEBUG

    for (i = 0; i < count; i++)
        PT_INFO( "has-slot:%s:%s\n", has_slot_pname[i], has_slot_reply[i]);

#endif
    return count;
}

/**
 * @brief Fill the slot meta used for fastboot get var info publication.
 * @param part_dev
 * @param slot_info
 */
void ptdev_fill_slot_meta(partition_device_t *part_dev,
                          struct ab_slot_info *slot_info)
{
    int i, current_slot_index;
    struct partition_entry *ptn_entries;
    char buff[3];

    if ((!part_dev) || (!part_dev->partition_entries)) {
        PT_WARN("Invalide partition dev\n");
        return;
    }

    if (NULL == slot_info) {
        PT_WARN("slot_info is NULL\n");
        return;
    }

    ptn_entries = ptdev_get_partition_entries(part_dev);

    /* Update slot info */
    for (i = 0; i < AB_SUPPORTED_SLOTS; i++) {
        current_slot_index = part_dev->boot_slot_index[i];
        strncpy(slot_info[i].slot_is_unbootable_rsp,
                slot_is_bootable(ptn_entries, current_slot_index) ? "No" : "Yes",
                MAX_RSP_SIZE);
        strncpy(slot_info[i].slot_is_active_rsp,
                slot_is_active(ptn_entries, current_slot_index) ? "Yes" : "No",
                MAX_RSP_SIZE);
        strncpy(slot_info[i].slot_is_succesful_rsp,
                slot_is_sucessful(ptn_entries, current_slot_index) ? "Yes" : "No",
                MAX_RSP_SIZE);
        itoa_s(slot_retry_count(ptn_entries, current_slot_index),
               (unsigned char *)buff, 2, 10);
        strncpy(slot_info[i].slot_retry_count_rsp, buff, MAX_RSP_SIZE);
    }
}

/**
 * @brief Read and update the attributes of GPT
 *
 * @param part_dev
 * @param gpt_start_addr
 * @param gpt_hdr_offset
 * @param gpt_entries_offset
 * @return int
 */
static int
update_gpt(partition_device_t *part_dev, uint64_t gpt_start_addr,
           uint64_t gpt_hdr_offset,
           uint64_t gpt_entries_offset)
{
    uint8_t *buffer = NULL;
    uint8_t *mbr_buffer = NULL;
    uint8_t *gpt_entries_ptr, *gpt_hdr_ptr, *tmp = NULL;
    struct partition_entry *partition_entries;
    uint32_t partition_count;
    unsigned max_partition_count = 0;
    unsigned partition_entry_size = 0;
    uint32_t block_size;
    uint32_t crc_val = 0;
    int ret = 0;
    uint64_t offset;
    uint64_t max_gpt_size_bytes;
    uint32_t sector_size = 0;

    if (NULL == part_dev) {
        PT_WARN("Invalide partition dev\n");
        goto out;
    }

    partition_entries = ptdev_get_partition_entries(part_dev);
    partition_count = ptdev_get_partition_count(part_dev);

    block_size = pt_disk_get_blocksize(part_dev->diskdev);
    sector_size = pt_disk_get_erasesize(part_dev->diskdev);

    if (!block_size || !sector_size) {
        PT_WARN("get block size %d or rase_grp_sz %derror\n", block_size,
                 sector_size);
        goto out;
    }

    offset = part_dev->gpt_offset;
    max_gpt_size_bytes =
        (PARTITION_ENTRY_SIZE * NUM_PARTITIONS + GPT_HEADER_BLOCKS * block_size);

    /* Get Current LUN for UFS target */

    buffer = osAllocAlign(block_size, ROUNDUP(max_gpt_size_bytes, block_size));

    if (!buffer) {
        PT_WARN( "update_gpt: Failed at memory allocation\n");
        goto out;
    }

    if (gpt_hdr_offset) {
        /* Secondary GPT dont'need offset */
        offset = 0;
    }

    ret = pt_disk_read(part_dev->diskdev, gpt_start_addr + offset,
                        (uint8_t *)buffer,
                        max_gpt_size_bytes);

    if (ret) {
        PT_WARN( "Failed to read GPT\n");
        goto out;
    }

    /* 0. Intialise ptrs for header and entries */
    gpt_entries_ptr = buffer + gpt_entries_offset * block_size;
    gpt_hdr_ptr = buffer + gpt_hdr_offset * block_size;

    /* Update attributes_flag of partition entry */
    tmp = gpt_entries_ptr;

    for (unsigned i = 0; i < partition_count; i++) {
        /* Update the partition attributes */
        PUT_LONG_LONG(&tmp[ATTRIBUTE_FLAG_OFFSET],
                      partition_entries[i].attribute_flag);

        /* Skip sub-partition table entry's guid,
         * because we only write gpt entries to the corresponding gpt table.
         * sub-partition table entries belong to its parent's partition gpt.
         * And there is really no '$' in GPT table.
         * It is not required that partition name is NUL terminated, so it is not appropriate to use strchr here.
         * */
        if (find_char(partition_entries[i].name, MAX_GPT_NAME_SIZE, '$'))
            continue;

        memscpy(tmp, PARTITION_TYPE_GUID_SIZE, partition_entries[i].type_guid,
                PARTITION_TYPE_GUID_SIZE);

        /* point to the next partition entry */
        tmp += PARTITION_ENTRY_SIZE;
    }

    /* Calculate and update CRC of partition entries array */
    max_partition_count =
        GET_LWORD_FROM_BYTE(&gpt_hdr_ptr[PARTITION_COUNT_OFFSET]);
    partition_entry_size =
        GET_LWORD_FROM_BYTE(&gpt_hdr_ptr[PENTRY_SIZE_OFFSET]);

    /* Check for partition entry size */
    if (partition_entry_size != PARTITION_ENTRY_SIZE) {
        PT_WARN( "Invalid parition entry size\n");
        goto out;
    }

    /* Check for maximum partition size */
    if ((max_partition_count) > (MIN_PARTITION_ARRAY_SIZE /
                                 (partition_entry_size))) {
        PT_WARN( "Invalid maximum partition count\n");
        goto out;
    }

    crc_val  = crc32(0U, gpt_entries_ptr, ((max_partition_count) *
                                           (partition_entry_size)));
    PUT_LONG(&gpt_hdr_ptr[PARTITION_CRC_OFFSET], crc_val);

    /* Write CRC to 0 before we calculate the crc of the GPT header */
    crc_val = 0;
    PUT_LONG(&gpt_hdr_ptr[HEADER_CRC_OFFSET], crc_val);
    crc_val  = crc32(0U, gpt_hdr_ptr, GPT_HEADER_SIZE);
    PUT_LONG(&gpt_hdr_ptr[HEADER_CRC_OFFSET], crc_val);

    if (strstr(part_dev->diskdev->info->disk_name, "flash")) {

        /* sector_size is large */
        if ( sector_size > max_gpt_size_bytes) {

            /* For Primary GPT, we need backup the MBR */
            if (gpt_hdr_offset == 0) {
                mbr_buffer = osAllocAlign(block_size, ROUNDUP(block_size, block_size));

                if (!mbr_buffer) {
                    PT_WARN( "update_gpt: Failed at mbr_buffer allocation\n");
                    goto out;
                }

                ret = pt_disk_read(part_dev->diskdev, (gpt_start_addr + offset - block_size),
                                    (uint8_t *)mbr_buffer, block_size);

                if (ret) {
                    PT_WARN( "Failed to read GPT MBR\n");
                    goto out;
                }
            }

            /* erase ospi sector */
            ret = pt_disk_erase(part_dev->diskdev, calc_sect_base_addr((gpt_start_addr + offset),
                                 sector_size), sector_size);

            if (ret) {
                PT_WARN("Failed to erase gpt!\n");
                goto out;
            }

            /* write to ospi */
            ret = pt_disk_write(part_dev->diskdev, gpt_start_addr + offset,
                                 (uint8_t *)buffer, max_gpt_size_bytes);

            if (ret) {
                PT_WARN( "Failed to write gpt\n");
                goto out;
            }

            /* For Primary GPT, we need backup the MBR */
            if (gpt_hdr_offset == 0) {
                /*write to ospi MBR */
                ret = pt_disk_write(part_dev->diskdev, (gpt_start_addr + offset - block_size),
                                     (uint8_t *)mbr_buffer, block_size);

                if (ret) {
                    PT_WARN( "Failed to write gpt MBR\n");
                    goto out;
                }
            }

        }
        else {
            /* cached write to ospi */
            ret = disk_write(part_dev->diskdev, gpt_start_addr + offset,
                                        (uint8_t *)buffer, max_gpt_size_bytes);

            if (ret) {
                PT_WARN( "Failed to cached_write gpt\n");
                goto out;
            }
        }
    }
    else {
        /* write to mmc */
        ret = pt_disk_write(part_dev->diskdev, gpt_start_addr + offset,
                             (uint8_t *)buffer, max_gpt_size_bytes);

        if (ret) {
            PT_WARN( "Failed to write gpt\n");
            goto out;
        }
    }

out:

    if (buffer)
        osFree(buffer);

    if (mbr_buffer)
        osFree(mbr_buffer);


    return ret;
}

/**
 * @brief Update the backup and primary guid parition table
 * @param part_dev
 */
int ptdev_attributes_update(partition_device_t *part_dev)
{
    uint64_t offset;
    uint64_t gpt_entries_offset, gpt_hdr_offset;
    uint64_t gpt_start_addr;
    int ret;
    uint32_t block_size;
    unsigned max_entries_size_bytes;
    unsigned max_entries_blocks;
    unsigned max_gpt_blocks;
    uint64_t device_capacity;

    if (NULL == part_dev) {
        PT_WARN("Invalide partition dev\n");
        return INVALID;
    }

    block_size = pt_disk_get_blocksize(part_dev->diskdev);

    if (!block_size) {
        PT_WARN("get block size = %d error\n", block_size);
        return -1;
    }

    device_capacity = pt_disk_get_capacity(part_dev->diskdev);

    if (!device_capacity) {
        PT_WARN("get capacity %lld error\n", device_capacity);
        return -1;
    }

    max_entries_size_bytes = PARTITION_ENTRY_SIZE * NUM_PARTITIONS;
    max_entries_blocks = max_entries_size_bytes / block_size;
    max_gpt_blocks = GPT_HEADER_BLOCKS + max_entries_blocks;

    /* Update Primary GPT */
    offset = MBR_BLOCKS;  /*  offset is 0x1 for primary GPT */
    gpt_start_addr = offset * block_size;
    /* Take gpt_start_addr as start and calculate offset from that in block sz*/
    gpt_hdr_offset = 0; /* For primary partition offset is zero */
    gpt_entries_offset = GPT_HEADER_BLOCKS;

    ret = update_gpt(part_dev, gpt_start_addr, gpt_hdr_offset,
                     gpt_entries_offset);

    if (ret) {
        PT_WARN( "Failed to update Primary GPT\n");
        return INVALID;
    }

    /* Update Secondary GPT */
    offset = ((device_capacity / block_size) - max_gpt_blocks);
    gpt_start_addr = offset * block_size;
    gpt_hdr_offset = max_entries_blocks;
    gpt_entries_offset = 0; /* For secondary GPT entries offset is zero */

    ret = update_gpt(part_dev, gpt_start_addr, gpt_hdr_offset,
                     gpt_entries_offset);

    if (ret) {
        PT_WARN( "Failed to update Secondary GPT\n");
        return INVALID;
    }

    return 0;
}
