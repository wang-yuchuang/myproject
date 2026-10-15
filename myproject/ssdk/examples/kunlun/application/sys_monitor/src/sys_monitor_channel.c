/**
 * @file sys_monitor_channel.c
 * @brief System Monitor — iccmsg channel connection-state tracking
 *
 * Maintains a name→connected lookup table updated by CHANNEL_STATUS
 * messages from the AP core.  Other R-core modules can query state
 * and register callbacks for connection changes.
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */

#include "sys_monitor_channel.h"

#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>

#define CHANNEL_MAX  16

typedef struct {
    char                name[SYSMGR_CHANNEL_NAME_MAX];
    bool                connected;
    sysmgr_channel_cb_t cb;
} channel_entry_t;

static channel_entry_t g_channels[CHANNEL_MAX];
static int             g_channel_count = 0;
static osMutexId_t     g_channel_lock  = NULL;

void sys_monitor_channel_init(void)
{
    memset(g_channels, 0, sizeof(g_channels));
    g_channel_count = 0;

    g_channel_lock = osMutexNew(NULL);
    if (g_channel_lock == NULL) {
        printf("[sys_monitor] WARNING: channel mutex creation failed\n");
    }
}

/* ---- Internal helpers (no locking — caller must hold mutex) ---- */

static channel_entry_t *find_entry_nolock(const char *name)
{
    for (int i = 0; i < g_channel_count; i++) {
        if (strcmp(g_channels[i].name, name) == 0)
            return &g_channels[i];
    }
    return NULL;
}

/* ---- Public API ---- */

bool sysmgr_channel_is_connected(const char *name)
{
    bool ret = false;
    if (g_channel_lock) osMutexAcquire(g_channel_lock, osWaitForever);
    channel_entry_t *e = find_entry_nolock(name);
    if (e)
        ret = e->connected;
    if (g_channel_lock) osMutexRelease(g_channel_lock);
    return ret;
}

int sysmgr_register_channel_callback(const char *name, sysmgr_channel_cb_t cb)
{
    int ret = 0;
    if (g_channel_lock) osMutexAcquire(g_channel_lock, osWaitForever);

    /* Find existing entry */
    channel_entry_t *e = find_entry_nolock(name);
    if (e) {
        e->cb = cb;
    } else {
        if (g_channel_count >= CHANNEL_MAX) {
            ret = -1;          /* table full */
        } else {
            e = &g_channels[g_channel_count++];
            strncpy(e->name, name, SYSMGR_CHANNEL_NAME_MAX - 1);
            e->connected = false;
            e->cb        = cb;
        }
    }

    if (g_channel_lock) osMutexRelease(g_channel_lock);
    return ret;
}

/* ---- Internal API ---- */

void sys_monitor_channel_update(const char *name, bool connected)
{
    if (g_channel_lock) osMutexAcquire(g_channel_lock, osWaitForever);

    channel_entry_t *e = find_entry_nolock(name);
    if (!e) {
        if (g_channel_count >= CHANNEL_MAX) {
            if (g_channel_lock) osMutexRelease(g_channel_lock);
            printf("[sys_monitor] channel table full, cannot add '%s'\n", name);
            return;
        }
        e = &g_channels[g_channel_count++];
        strncpy(e->name, name, SYSMGR_CHANNEL_NAME_MAX - 1);
        e->connected = false;
        e->cb        = NULL;
    }

    bool old = e->connected;
    e->connected = connected;

    if (g_channel_lock) osMutexRelease(g_channel_lock);

    /* Notify outside the lock to avoid deadlock in callback */
    if (old != connected) {
        printf("[sys_monitor] Channel '%s' %s\n",
               name, connected ? "connected" : "disconnected");
        if (e->cb)
            e->cb(name, connected);
    }
}

void sys_monitor_channel_reset_all(void)
{
    if (g_channel_lock) osMutexAcquire(g_channel_lock, osWaitForever);

    for (int i = 0; i < g_channel_count; i++) {
        if (g_channels[i].connected) {
            g_channels[i].connected = false;
            printf("[sys_monitor] Channel '%s' reset to disconnected\n",
                   g_channels[i].name);
            /* Callback outside lock (see note in _update) */
            if (g_channels[i].cb) {
                sysmgr_channel_cb_t cb = g_channels[i].cb;
                char name_buf[SYSMGR_CHANNEL_NAME_MAX];
                strncpy(name_buf, g_channels[i].name, sizeof(name_buf) - 1);
                name_buf[sizeof(name_buf) - 1] = '\0';
                if (g_channel_lock) osMutexRelease(g_channel_lock);
                cb(name_buf, false);
                if (g_channel_lock) osMutexAcquire(g_channel_lock, osWaitForever);
            }
        }
    }

    if (g_channel_lock) osMutexRelease(g_channel_lock);
}
