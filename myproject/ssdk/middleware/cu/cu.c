/**
 * @file cu.c
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
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <param.h>
#include <uart_cfg.h>
#include <mbox_cfg.h>
#include <rstgen_cfg.h>
#include <hwmutex_cfg.h>
#include <types.h>
#include <board.h>
#include <driver/uart.h>
#include <driver/taishan/uart/sdrv_uart_drv.h>
#include <driver/taishan/mbox/sdrv_taishan_mbox_drv.h>
#include <driver/hwmutex.h>
#include <reg.h>
#include <driver/irq.h>
#include <core_shm_cfg.h>

#ifdef CONFIG_PM
#include <driver/pm.h>
#endif

#define SET_CONSOLE_COREID(id)  g_coreshm->coreid = (id)
#define GET_CONSOLE_COREID()    g_coreshm->coreid
#define GET_RUNNING_CORE()      g_coreshm->running_core

const static char *g_core_name[] = {
    "SF", "SP0", "SP1", "SX0", "SX1",
};

static bool g_cu_inited;
static struct hwmutex *g_hwmutex;
static osSemaphoreId_t g_sem;
static osSemaphoreId_t g_print_buffer_sem;
static char g_print_buffer[CONFIG_PRINT_BUFFER_SIZE];
static uint32_t g_print_write;
static uint32_t g_print_read;

#ifdef CONFIG_PM
static void cu_pm_resume(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
        break;

        case PM_DEEP_SLEEP:
        if (board_mbox_get_coreid() == GET_CONSOLE_COREID()) {
            hwmutex_acquire(g_hwmutex, 0);
        }
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }
}

static struct pm_callback_s cu_pm_cb = {
    .resume = cu_pm_resume,
};
#endif

static inline uint32_t cu_print_buffer_avail(void)
{
    uint32_t used;

    if (g_print_read <= g_print_write) {
        used = g_print_write - g_print_read;
    }
    else {
        used = CONFIG_PRINT_BUFFER_SIZE - g_print_read + g_print_write;
    }

    return CONFIG_PRINT_BUFFER_SIZE - 1 - used;
}

static void cu_buffer_print_log_safe(const char *data, uint32_t size)
{
    uint32_t avail;
    uint32_t write;

    avail = cu_print_buffer_avail();
    write = MIN(avail, size);
    size -= write;

    if (write) {
        if (g_print_write >= g_print_read) {
            uint32_t first_write = CONFIG_PRINT_BUFFER_SIZE - g_print_write;
            first_write = MIN(first_write, write);
            memcpy(&g_print_buffer[g_print_write], data, first_write);
            g_print_write = (g_print_write + first_write) & (CONFIG_PRINT_BUFFER_SIZE - 1);
            data += first_write;
            write -= first_write;
        }

        if (write) {
            memcpy(&g_print_buffer[g_print_write], data, write);
            g_print_write = (g_print_write + write) & (CONFIG_PRINT_BUFFER_SIZE - 1);
            data += write;
        }
    }

    while (size) {
        uint32_t loop_w = CONFIG_PRINT_BUFFER_SIZE - g_print_write;
        loop_w = MIN(loop_w, size);
        memcpy(&g_print_buffer[g_print_write], data, loop_w);
        g_print_write = (g_print_write + loop_w) & (CONFIG_PRINT_BUFFER_SIZE - 1);
        g_print_read = (g_print_write + 1) & (CONFIG_PRINT_BUFFER_SIZE - 1);
        data += loop_w;
        size -= loop_w;
    }
}

void cu_transmit_print_log(void)
{
    if (g_print_buffer[g_print_read] == '\r' &&
        g_print_buffer[(g_print_read + 1) & (CONFIG_PRINT_BUFFER_SIZE - 1)] == '\n' &&
        g_print_buffer[(g_print_read + 2) & (CONFIG_PRINT_BUFFER_SIZE - 1)] == '>') {
        g_print_read = (g_print_read + 3) & (CONFIG_PRINT_BUFFER_SIZE - 1);
    }

    if (g_print_read == g_print_write) {
        return;
    }

    printf("Buffered printf start...\n");

    if (g_print_read < g_print_write) {
        uart_transmit(g_console_uart, &g_print_buffer[g_print_read], g_print_write - g_print_read);
        g_print_read = g_print_write;
    }
    else {
        uart_transmit(g_console_uart, &g_print_buffer[g_print_read], CONFIG_PRINT_BUFFER_SIZE - g_print_read);
        if (g_print_write) {
            uart_transmit(g_console_uart, &g_print_buffer[0], g_print_write);
        }
        g_print_read = g_print_write;
    }

    printf("\nBuffered printf end...\n");
}

void cu_buffer_print_log(const char *data, uint32_t size)
{
    osStatus_t ret;

    if (g_cu_inited) {
        ret = osSemaphoreAcquire(g_print_buffer_sem, 0);
        if (ret != osOK) {
            return;
        }

        cu_buffer_print_log_safe(data, size);
        osSemaphoreRelease(g_print_buffer_sem);
    }
    else {
        cu_buffer_print_log_safe(data, size);
    }
}

static void cu_help(void)
{
    printf("cu sf/sp/sx/sp0/sp1/sx0/sx1\n");
}

static void cu_console_assign_to_core(uint32_t coreid)
{
    g_console_occupid = false;
    irq_disable(g_console_uart->irq);
    SET_CONSOLE_COREID(coreid);
    hwmutex_release(g_hwmutex);

    while (hwmutex_get_owner(g_hwmutex) != coreid) {
    }

    osSemaphoreRelease(g_sem);
}

static void cu_console_deamon(void *arg)
{
    uint32_t occupid;
    uint32_t coreid = board_mbox_get_coreid();

    ssdk_printf(SSDK_DEBUG, "%s: core-id is %d\n", __func__, coreid);

    while (true) {
        osSemaphoreAcquire(g_sem, osWaitForever);
        hwmutex_wait_release(g_hwmutex, osWaitForever);
        occupid = GET_CONSOLE_COREID();

        ssdk_printf(SSDK_DEBUG, "%s: self %d, occupid %d\n", __func__, coreid, occupid);

        if (occupid == coreid) {
            hwmutex_acquire(g_hwmutex, osWaitForever);
            g_console_occupid = true;
            irq_enable(g_console_uart->irq);

            printf("Console for %s\n", g_core_name[coreid]);
            cu_transmit_print_log();
            printf("\r\n>");
        }
        else {
            while (hwmutex_get_owner(g_hwmutex) != occupid) {
            }

            ssdk_printf(SSDK_DEBUG, "%s: not me , continue wait\n", __func__);

            osSemaphoreRelease(g_sem);
        }
    }
}

void cu_console_init(void)
{
    osStatus_t ret;
    uint32_t occupid;

    g_hwmutex = hwmutex_request_specific(&g_hwmutex_dev, 0);
    g_sem = osSemaphoreNew(1, 1, NULL);
    g_print_buffer_sem = osSemaphoreNew(1, 1, NULL);
    g_cu_inited = true;

    if (board_mbox_get_coreid() == MBOX_RPROC_SF) {
        SET_CONSOLE_COREID(MBOX_RPROC_SF);
        ret = hwmutex_acquire(g_hwmutex, osWaitForever);
        if (ret != osOK) {
            ssdk_printf(SSDK_DEBUG, "%s:hwmutex_acquire failed.\n", __func__);
        }

        ret = osSemaphoreAcquire(g_sem, osWaitForever);
        if (ret != osOK) {
            ssdk_printf(SSDK_DEBUG, "%s:osSemaphoreAcquire failed.\n", __func__);
        }
    }
    else {
        occupid = GET_CONSOLE_COREID();
        irq_disable(g_console_uart->irq);
        while (hwmutex_get_owner(g_hwmutex) != occupid) {
        }
    }

#ifdef CONFIG_PM
    pm_register(&cu_pm_cb);
#endif

    osThreadNew(cu_console_deamon, NULL, NULL);
}

static int cu_main(int argc, char *argv[])
{
    uint32_t coreid;

    if (argc != 1) {
        cu_help();
        goto exit;
    }

    if (!strcmp(argv[0], "sf")) {
        coreid = MBOX_RPROC_SF;
    }
    else if (!strcmp(argv[0], "sp")) {
        coreid = MBOX_RPROC_SP0;
    }
    else if (!strcmp(argv[0], "sp0")) {
        coreid = MBOX_RPROC_SP0;
    }
    else if (!strcmp(argv[0], "sp1")) {
        coreid = MBOX_RPROC_SP1;
    }
    else if (!strcmp(argv[0], "sx")) {
        coreid = MBOX_RPROC_SX0;
    }
    else if (!strcmp(argv[0], "sx0")) {
        coreid = MBOX_RPROC_SX0;
    }
    else if (!strcmp(argv[0], "sx1")) {
        coreid = MBOX_RPROC_SX1;
    }
    else {
        cu_help();
        goto exit;
    }

    if ((coreid != MBOX_RPROC_SF) &&
        (!(GET_RUNNING_CORE() & (0x1u << (coreid - 1))))) {
        printf("%s not running!\n", g_core_name[coreid]);
        goto exit;
    }

    if (coreid != GET_CONSOLE_COREID()) {
        cu_console_assign_to_core(coreid);
    }

exit:
    return CLI_FALSE;
}

CLI_CMD("cu", "change console to core", cu_main);
