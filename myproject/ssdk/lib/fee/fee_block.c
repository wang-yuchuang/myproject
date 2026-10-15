/**
 * @file fee_block.c
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
#include <lib/fee/fee_block.h>

static struct list_node g_fee_list;
static osMutexId_t g_fee_mutex;

static struct fee_block *fee_get_dev(const char *fee_name)
{
    struct fee_block *fee_block;

    osMutexAcquire(g_fee_mutex, osWaitForever);
    /* matches nodes based on name */
    list_for_every_entry(&g_fee_list, fee_block,
                         struct fee_block, node) {
        if (!strcmp(fee_name, fee_block->fee_name)) {
            osMutexRelease(g_fee_mutex);
            return fee_block;
        }
    }
    osMutexRelease(g_fee_mutex);
    return NULL;
}

int fee_register_block(struct fee_block *fee)
{
    int ret = 0;
    return ret;
}
int fee_unregister_block(struct fee_block *fee)
{
    int ret = 0;
    return ret;
}

int fee_read_block(const char *fee_name, uint16_t block_number,
                   uint16_t block_offset, uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;

    return ret;

}

int fee_write_block(const char *fee_name, uint16_t block_number,
                    const uint8_t *data_buffer, uint16_t length)
{
    int ret = 0;
    return ret;

}

int fee_init_block(void)
{
    ssdk_printf(SSDK_INFO, "fee block init\n");
    /* disk list node init */
    list_initialize(&g_fee_list);
    g_fee_mutex = osMutexNew(NULL);
    return 0;
}

int fee_exit_block(void)
{
    ssdk_printf(SSDK_INFO, "fee block exit\n");
    /* disk list node init */
    osMutexRelease(g_fee_mutex);
    return 0;
}


