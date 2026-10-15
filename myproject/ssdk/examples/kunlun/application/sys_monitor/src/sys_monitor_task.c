/**
 * @file sys_monitor_task.c
 * @brief System Monitor — R-core task supervision (rapp)
 *
 * Each supervised task registers itself and periodically "kicks"
 * its heartbeat.  If a task misses its timeout, the configured
 * recovery action (restart or SoC reset) is triggered.
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */

#include "sys_monitor_task.h"

#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>

#include "rstgen_cfg.h"
#include "driver/reset.h"

#define RAPP_MAX  16

typedef struct {
    bool                   active;
    char                   name[16];
    osThreadId_t           handle;
    uint32_t               last_kick_tick;
    uint32_t               timeout_ms;
    rapp_recovery_action_t recovery;
    uint32_t               max_restarts;
    uint32_t               restart_count;
    rapp_restart_fn        restart_fn;
    void                  *restart_arg;
    uint32_t               crash_count;
} rapp_entry_t;

static rapp_entry_t g_rapp[RAPP_MAX];
static uint32_t     g_total_crash = 0;

void sys_monitor_task_init(void)
{
    memset(g_rapp, 0, sizeof(g_rapp));
}

int rapp_register(const rapp_reg_t *reg)
{
    if (!reg || !reg->task_name)
        return -1;

    /* Reject duplicate names */
    for (int i = 0; i < RAPP_MAX; i++) {
        if (g_rapp[i].active &&
            strcmp(g_rapp[i].name, reg->task_name) == 0)
            return -1;
    }

    /* Find a free slot */
    for (int i = 0; i < RAPP_MAX; i++) {
        if (!g_rapp[i].active) {
            memset(&g_rapp[i], 0, sizeof(g_rapp[i]));
            g_rapp[i].active         = true;
            strncpy(g_rapp[i].name, reg->task_name, sizeof(g_rapp[i].name) - 1);
            g_rapp[i].handle         = (osThreadId_t)reg->task_handle;
            g_rapp[i].last_kick_tick = osKernelGetTickCount();
            g_rapp[i].timeout_ms     = reg->timeout_ms;
            g_rapp[i].recovery       = reg->recovery;
            g_rapp[i].max_restarts   = reg->max_restarts;
            g_rapp[i].restart_count  = 0;
            g_rapp[i].restart_fn     = reg->restart_fn;
            g_rapp[i].restart_arg    = reg->restart_arg;
            g_rapp[i].crash_count    = 0;
            return 0;
        }
    }

    return -1;   /* table full */
}

int rapp_unregister(const char *task_name)
{
    for (int i = 0; i < RAPP_MAX; i++) {
        if (g_rapp[i].active &&
            strcmp(g_rapp[i].name, task_name) == 0) {
            memset(&g_rapp[i], 0, sizeof(g_rapp[i]));
            return 0;
        }
    }
    return -1;
}

void rapp_kick(const char *task_name)
{
    for (int i = 0; i < RAPP_MAX; i++) {
        if (g_rapp[i].active &&
            strcmp(g_rapp[i].name, task_name) == 0) {
            g_rapp[i].last_kick_tick = osKernelGetTickCount();
            return;
        }
    }
}

/**
 * Called from monitor_task loop.
 * Checks all supervised tasks; restarts at most ONE timed-out task per call.
 */
void sys_monitor_task_check(uint32_t now, uint32_t tick_freq)
{
    uint32_t tick_per_ms = tick_freq / 1000;
    if (tick_per_ms == 0)
        return;

    for (int i = 0; i < RAPP_MAX; i++) {
        if (!g_rapp[i].active)
            continue;

        uint32_t elapsed_ms = (now - g_rapp[i].last_kick_tick) / tick_per_ms;
        if (elapsed_ms <= g_rapp[i].timeout_ms)
            continue;

        /* --- Task timed out --- */
        printf("[sys_monitor] Task '%s' timeout (%ums), recovery\n",
               g_rapp[i].name, elapsed_ms);

        g_rapp[i].crash_count++;
        g_total_crash++;

        /* Decide recovery action */
        bool need_soc_reset =
            (g_rapp[i].recovery == RAPP_REBOOT_SOC) ||
            (g_rapp[i].max_restarts > 0 &&
             g_rapp[i].restart_count >= g_rapp[i].max_restarts);

        if (need_soc_reset) {
            printf("[sys_monitor] Task '%s' max_restarts exceeded "
                   "(%u/%u), SoC reset\n",
                   g_rapp[i].name,
                   g_rapp[i].restart_count,
                   g_rapp[i].max_restarts);
            /* Full SoC reset — should not return */
            reset_ctl_reset(&rstsig_sw_self_rst);
            /* If we somehow survive, spin to let WDT finish the job */
            for (;;) { /* watchdog will reset */ }
        }

        /* Restart the task */
        if (g_rapp[i].handle) {
            osThreadTerminate(g_rapp[i].handle);
            g_rapp[i].handle = NULL;
        }
        if (g_rapp[i].restart_fn) {
            g_rapp[i].restart_fn(g_rapp[i].restart_arg);
        }
        g_rapp[i].restart_count++;
        g_rapp[i].last_kick_tick = osKernelGetTickCount();

        /* Only handle one timeout per iteration (G13 constraint) */
        break;
    }
}

uint32_t sys_monitor_task_get_crash_total(void)
{
    return g_total_crash;
}

uint8_t sys_monitor_task_get_monitored_count(void)
{
    uint8_t count = 0;
    for (int i = 0; i < RAPP_MAX; i++) {
        if (g_rapp[i].active)
            count++;
    }
    return count;
}
