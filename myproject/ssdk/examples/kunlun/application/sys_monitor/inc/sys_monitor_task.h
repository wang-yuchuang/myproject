/**
 * @file sys_monitor_task.h
 * @brief Task supervision (rapp) — heartbeat-based watchdog for R-core tasks
 *
 * Each monitored task registers itself and periodically "kicks" (feeds)
 * its heartbeat. If a task misses its timeout, the configured recovery
 * action is triggered.
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */
#ifndef SYS_MONITOR_TASK_H
#define SYS_MONITOR_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Recovery action when a monitored task times out. */
typedef enum {
    RAPP_RESTART_TASK = 0,  /**< Restart the task via restart_fn */
    RAPP_REBOOT_SOC   = 1,  /**< Reboot the entire SoC */
} rapp_recovery_action_t;

/** Function pointer to restart a task (Phase 2 provides implementations). */
typedef void (*rapp_restart_fn)(void *arg);

/** Registration descriptor for a supervised task. */
typedef struct {
    const char            *task_name;          /**< Human-readable task name */
    void                  *task_handle;        /**< FreeRTOS task handle */
    uint32_t               heartbeat_period_ms; /**< Expected kick interval */
    uint32_t               timeout_ms;         /**< Max time without kick */
    rapp_recovery_action_t recovery;           /**< What to do on timeout */
    uint32_t               max_restarts;       /**< Max restarts before giving up */
    rapp_restart_fn        restart_fn;         /**< Restart callback */
    void                  *restart_arg;        /**< Argument for restart_fn */
} rapp_reg_t;

/**
 * Register a task for supervision.
 *
 * @param reg  Pointer to a filled rapp_reg_t (contents are copied)
 * @return     0 on success, negative on error (e.g. table full)
 */
int rapp_register(const rapp_reg_t *reg);

/**
 * Unregister a supervised task.
 *
 * @param task_name  Task name to remove
 * @return           0 on success, negative if not found
 */
int rapp_unregister(const char *task_name);

/**
 * Feed (kick) the heartbeat for a registered task.
 * Must be called periodically within timeout_ms.
 *
 * @param task_name  Task name to kick
 */
void rapp_kick(const char *task_name);

/**
 * @name Internal functions (used by sys_monitor.c)
 * @{
 */

/** @brief Initialize the task supervision table. */
void sys_monitor_task_init(void);

/** @brief Check all supervised tasks for timeout (called from monitor_task). */
void sys_monitor_task_check(uint32_t now, uint32_t tick_freq);

/** @brief Get total crash count across all tasks. */
uint32_t sys_monitor_task_get_crash_total(void);

/** @brief Get count of currently monitored tasks. */
uint8_t sys_monitor_task_get_monitored_count(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SYS_MONITOR_TASK_H */
