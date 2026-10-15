/**
 * @file sys_monitor_channel.h
 * @brief Channel status tracking for sysmgr
 *
 * Tracks iccmsg endpoint connection state reported by AP core.
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */
#ifndef SYS_MONITOR_CHANNEL_H
#define SYS_MONITOR_CHANNEL_H

#include <stdbool.h>
#include "sysmgr_protocol.h"  /* SYSMGR_CHANNEL_NAME_MAX */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check whether a named channel (iccmsg endpoint) is connected.
 *
 * @param name  Channel name (max SYSMGR_CHANNEL_NAME_MAX chars)
 * @return      true if connected, false otherwise
 */
bool sysmgr_channel_is_connected(const char *name);

/**
 * Callback invoked when a channel's connection state changes.
 *
 * @param name       Channel name
 * @param connected  true if now connected, false if disconnected
 */
typedef void (*sysmgr_channel_cb_t)(const char *name, bool connected);

/**
 * Register a callback for connection state changes on a channel.
 *
 * @param name  Channel name to monitor
 * @param cb    Callback function
 * @return      0 on success, negative on error
 */
int sysmgr_register_channel_callback(const char *name, sysmgr_channel_cb_t cb);

/**
 * @name Internal functions (used by sys_monitor.c / sys_monitor_msg.c)
 * @{
 */

/** @brief Initialize the channel status table. */
void sys_monitor_channel_init(void);

/** @brief Update connection state for a channel (from CHANNEL_STATUS msg). */
void sys_monitor_channel_update(const char *name, bool connected);

/** @brief Reset all channels to disconnected (called after boot_ap1). */
void sys_monitor_channel_reset_all(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SYS_MONITOR_CHANNEL_H */
