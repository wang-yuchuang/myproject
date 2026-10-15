/**
 * @file sys_monitor.h
 * @brief System Monitor — R-core sysmgr endpoint main header（完整版）
 *
 * Called from SSDK main. 完整版: iccmsg_add("sysmgr") + monitor_task。
 * 保留 v3 兼容入口 sys_monitor_init_deferred()（内部转 sys_monitor_init()）。
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 * 移植记录: openspec/changes/fix-virtio-rpmsg-ns-deadlock (Phase 2 启用)
 */
#ifndef SYS_MONITOR_H
#define SYS_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

#include "iccmsg.h"

#include "sysmgr_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Global flag: AP core connected via iccmsg heartbeat.
 * Set true when first heartbeat/ping arrives, false on timeout.
 */
extern bool g_ap_connected;

/**
 * Initialize the system monitor subsystem.
 * Called from SSDK main as the first endpoint.
 * Phase 2: creates iccmsg endpoint, starts monitor task, etc.
 */
void sys_monitor_init(void);

/** @brief v3 兼容入口（内部转 sys_monitor_init()） */
void sys_monitor_init_deferred(void);

/**
 * @brief iccmsg rx callback for the "sysmgr" endpoint.
 * Signature must match iccmsg_rx_cb_t.
 */
int sys_monitor_rx_cb(void *user_ctx, const void *data, uint32_t len);

/** @brief Called when a PING is received — updates heartbeat, sets g_ap_connected. */
extern void sys_monitor_on_ping(void);

/** @brief Enter grace mode (PLANNED_REBOOT received). */
extern void sys_monitor_enter_grace(void);

/** @brief Exit grace mode (AP reconnected after grace/boot_ap1). */
extern void sys_monitor_exit_grace(void);

/** @brief Get the sysmgr iccmsg endpoint for sending responses. */
extern iccmsg_t *sys_monitor_get_ept(void);

/** @brief Get watchdog time-left in seconds. */
extern uint32_t sys_monitor_get_wdt_timeleft(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MONITOR_H */
