/**
 * @file fee_eeprom.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */


#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <param.h>
#include <types.h>
#include <cmsis_os2.h>
#include <arch/atomic.h>
#include <stdarg.h>

#include <lib/list.h>
#include <lib/fee/fee_eeprom.h>

static struct list_node g_fee_list;
static osMutexId_t g_fee_mutex;

static struct fee_eeprom *fee_get_dev(const char *fee_name)
{
    struct fee_eeprom *fee_eeprom;

    osMutexAcquire(g_fee_mutex, osWaitForever);
    /* matches nodes based on name */
    list_for_every_entry(&g_fee_list, fee_eeprom,
                         struct fee_eeprom, node) {
        if (!strcmp(fee_name, fee_eeprom->fee_name)) {
            osMutexRelease(g_fee_mutex);
            return fee_eeprom;
        }
    }
    osMutexRelease(g_fee_mutex);
    return NULL;
}

static int fee_eeprom_check(struct fee_config_eeprom *fee_con)
{
    /* check whether the Flash capacity is supported */
    uint16_t pe_ratio = ROUNDUP(fee_con->eeprom_pe_cycles,
                                fee_con->pe_cycles) / fee_con->pe_cycles;
    uint16_t size_avg = fee_con->eeprom_align_size * FEE_EEPROM_SIZE_AVG;
    disk_size_t target_size = fee_con->eeprom_size * pe_ratio +
                              (fee_con->eeprom_size / size_avg) * pe_ratio * sizeof(struct fee_record_info);

    if (target_size > fee_con->size) {
        ssdk_printf(SSDK_CRIT,
                    "fee eeprom target_size %llx actual size %llx,space size does not meet the requirements\n",
                    target_size, fee_con->size);
        return -1;
    }

    ssdk_printf(SSDK_CRIT, "fee eeprom target_size %llx actual size %llx\n",
                target_size, fee_con->size);
    return 0;
}

int fee_register_eeprom(struct fee_eeprom *fee)
{
    int ret = 0;
    struct fee_dev *fee_dev = &fee->fee_dev;
    struct fee_config_eeprom *fee_con = &fee->fee_con;
    fee->fee_name = fee_con->fee_name;
    ssdk_printf(SSDK_INFO, "register fee eeprom device %s\n", fee->fee_name);

    if (NULL != fee_get_dev(fee->fee_name)) {
        ssdk_printf(SSDK_CRIT, "fee devicve %s already registered\n", fee->fee_name);
        return -1;
    }

    /* check the configuration */
    ret = fee_eeprom_check(fee_con);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "register fee eeprom device %s\n error", fee->fee_name);
        goto error;
    }

    osMutexAcquire(g_fee_mutex, osWaitForever);
    /* insert node */
    list_add_tail(&g_fee_list, &fee->node);
    osMutexRelease(g_fee_mutex);

    fee->fee_mutex = osMutexNew(NULL);
    /* Disk device configuration, underlying block based operation, size access_align_size */
    struct disk_dev *disk_dev = &fee_dev->disk;
    ret = disk_open(fee_con->disk_name, disk_dev);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee eeprom %s open disk dev %s faild\n", fee->fee_name,
                    fee_con->disk_name);
        goto error;
    }

    ret = disk_set_block_size(disk_dev, fee_con->access_align_size);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee eeprom %s set disk dev %s block size %d faild\n",
                    fee->fee_name, fee_con->disk_name, fee_con->access_align_size);
        goto disk_error;
    }

    fee_dev->disk_addr = fee_con->addr;
    fee_dev->disk_size = fee_con->size;
    fee_dev->disk_mem_align_size = fee_con->mem_align_size;
    fee_dev->disk_access_size = fee_con->access_align_size;
    fee_dev->disk_sector_size = disk_erase_size(disk_dev);

    fee_dev->block_length = fee_con->eeprom_align_size;
    fee_dev->block_number = fee_con->eeprom_size / fee_dev->block_length;

    fee_dev->page_mode = FEE_PAGE_EEPROM;

    ret = fee_init(fee_dev);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee eeprom %s init faild\n", fee->fee_name);
        goto disk_error;
    }

    return ret;

disk_error:
    disk_close(disk_dev);
error:
    return ret;
}
int fee_unregister_eeprom(struct fee_eeprom *fee)
{
    int ret = 0;
    struct fee_dev *fee_dev = &fee->fee_dev;
    ssdk_printf(SSDK_INFO, "unregiste fee eeprom device %s\n", fee->fee_name);
    fee->fee_name = NULL;
    osMutexAcquire(g_fee_mutex, osWaitForever);

    /* remove nodes */
    if (list_in_list(&fee->node))
        list_delete(&fee->node);

    osMutexRelease(g_fee_mutex);

    fee_exit(fee_dev);
    disk_close(&fee_dev->disk);
    return ret;
}

int fee_read_eeprom(const char *fee_name, uint32_t addr, uint8_t *data_buffer,
                    uint16_t length)
{
    int ret = 0;
    struct fee_eeprom *fee = fee_get_dev(fee_name);

    if (NULL == fee) {
        ssdk_printf(SSDK_CRIT, "no fee devicve %s\n", fee_name);
        return -1;
    }

    ssdk_printf(SSDK_INFO, "read fee eeprom device %s\n", fee->fee_name);
    struct fee_dev *fee_dev = &fee->fee_dev;
    struct fee_config_eeprom *fee_con = &fee->fee_con;

    if (!IS_ALIGNED(addr, fee_dev->block_length)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_read eeprom addr %d not aligned to record_length %d\n",
                    addr, fee_dev->block_length);
        return -1;
    }

    uint16_t block_number = addr / fee_dev->block_length;

    if (!IS_ALIGNED(length, fee_dev->block_length)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_read eeprom length %d not aligned to record_length %d\n",
                    length, fee_dev->block_length);
        return -1;
    }

    if ((addr + length) > fee_con->eeprom_size) {
        ssdk_printf(SSDK_CRIT,
                    "fee_read eeprom addr %d length %d more than eeprom capacity %d\n",
                    addr, length, fee_con->eeprom_size);
        return -1;
    }

    osMutexAcquire(fee->fee_mutex, osWaitForever);

    ret = fee_read_record_multiple(fee_dev, block_number, data_buffer, length);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee_read_record_multiple fee_name %s addr %d error\n",
                    fee_name, addr);
        osMutexRelease(fee->fee_mutex);
        return -1;
    }

    osMutexRelease(fee->fee_mutex);

    return ret;

}

int fee_write_eeprom(const char *fee_name, uint32_t addr,
                     const uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;
    struct fee_eeprom *fee = fee_get_dev(fee_name);

    if (NULL == fee) {
        ssdk_printf(SSDK_CRIT, "no fee devicve %s\n", fee_name);
        return -1;
    }

    struct fee_dev *fee_dev = &fee->fee_dev;

    struct fee_config_eeprom *fee_con = &fee->fee_con;

    ssdk_printf(SSDK_INFO, "write fee eeprom device %s\n", fee->fee_name);

    if (!IS_ALIGNED(addr, fee_dev->block_length)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_write eeprom addr %d not aligned to record_length %d\n",
                    addr, fee_dev->block_length);
        return -1;
    }

    uint16_t block_number = addr / fee_dev->block_length;

    if (!IS_ALIGNED(length, fee_dev->block_length)) {
        ssdk_printf(SSDK_CRIT,
                    "fee_write eeprom length %d not aligned to record_length %d\n",
                    length, fee_dev->block_length);
        return -1;
    }

    if ((addr + length) > fee_con->eeprom_size) {
        ssdk_printf(SSDK_CRIT,
                    "fee_write eeprom addr %d length %d more than eeprom capacity %d\n",
                    addr, length, fee_con->eeprom_size);
        return -1;
    }

    osMutexAcquire(fee->fee_mutex, osWaitForever);

    ret = fee_write_record_multiple(fee_dev, block_number, (uint8_t *)data_buffer,
                                    length);

    if (ret) {
        ssdk_printf(SSDK_CRIT, "fee_write_record_multiple fee_name %s addr %d error\n",
                    fee_name, addr);
        osMutexRelease(fee->fee_mutex);
        return -1;
    }

    osMutexRelease(fee->fee_mutex);
    return ret;
}

int fee_init_eeprom(void)
{
    ssdk_printf(SSDK_INFO, "fee eeprom init\n");
    /* disk list node init */
    list_initialize(&g_fee_list);
    g_fee_mutex = osMutexNew(NULL);
    return 0;
}

int fee_exit_eeprom(void)
{
    ssdk_printf(SSDK_INFO, "fee eeprom exit\n");
    /* disk list node init */
    osMutexRelease(g_fee_mutex);
    return 0;
}
