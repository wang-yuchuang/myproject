/**
 * @file launcher.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_LAUNCHER_H__
#define __SD_LAUNCHER_H__
#include <cmsis_os2.h>
#include <lib/list.h>
#include "state_machine.h"
#include "arbitrator.h"
#include "container.h"

typedef enum _app_id_em {
    app_id_ba = 0,
    app_id_fastavm,
    app_id_cluster,
    app_id_controlpanel,
    app_id_emirror,
} app_id_em;

typedef enum _app_notify_em {
    app_notify_start = 0,
    app_notify_stop,
    app_notify_termiante,
    app_notify_restart,
    app_notify_max
} app_notify_em;

typedef struct _app_token_t {
    int id;
    const char* name;
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr;
    osMessageQueueId_t send_q;
    int accessor_num;
    arb_accessor_t* accessor_array;
    void (*p_init)(void* token);
    void (*p_entry)(void* token);
    void (*p_notifier)(int notify);
} app_token_t;

typedef struct _launcher_ctx_t {
    int init;
    int exit;
    int app_num;
    app_token_t* apps;
    msg_token_t* msg_tk;
} launcher_ctx_t;

typedef struct _msg_t {
    int msgid;
    int param;
} msg_t;

extern void launcher_app();

#endif /*__SD_LAUNCHER_H__*/
/* EOF */
