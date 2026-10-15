/**
 * @file cluster.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv cluster application.
 *
 * Revision History:
 * -----------------
 */
#include <string.h>
#include <cmsis_os2.h>
#include <lib/list.h>
#include <sd_property.h>
#include <disp_hal.h>
#include "container.h"
#include "cluster.h"
#include "cluster_cfg.h"
#include "cluster_def.h"
#include "launcher.h"
#include "arbitrator.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cluster"

static void cluster_accessor_start(int magic, int token, void* accessor);
static void cluster_accessor_stop(int magic, int token, void* accessor);

extern void cluster_camera_task(void* args);
extern void cluster_player_task(void* args);
extern void cluster_car_task(void* args);
#ifdef CONFIG_USE_GUI_LVGL
extern void cluster_lvgl_task(void* args);
#endif

static arb_accessor_t cluster_accessor_tbl[] = {
    [0] = {
        .magic = arb_magic_video,
        .slave_tk = CLUSTER,
        .prio = arb_prio_mid,
        .p_start = cluster_accessor_start,
        .p_stop = cluster_accessor_stop,
        .p_suspend = cluster_accessor_stop,
        .p_resume = cluster_accessor_start,
    },
};

static cluster_task_token_t cluster_task_token[] = {
#ifdef CONFIG_USE_GUI_LVGL
    [CLUSTER_LVGL_TASK] = {
        .id = CLUSTER_LVGL_TASK,
        .name = "cls_lvgl",
        .thread_attr = { .name = "cls_lvgl", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = cluster_lvgl_task,
        .disp_sts = 0,
        .support_disp = CLUSTER_MASK(CLUSTER),
    },
#endif
};

static int cluster_properties[] = {
    DMP_ID_CLUSTER_STATUS,
    -1
};

static cluster_ctx_t cluster_ctx = {
    .init = 0,
    .property_id = cluster_properties,
    .task_tk = cluster_task_token,
    .detect_task = 0,
    .task_terminate = NULL,
};

static void cluster_accessor_start(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == CLUSTER) {
        for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
            cluster_task_token_t* task = &cluster_ctx.task_tk[i];
            if (task && (task->support_disp&(CLUSTER_MASK(token)))) {
                task->disp_sts |= CLUSTER_MASK(token);
                break;
            }
        }
    }
}

static void cluster_accessor_stop(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == CLUSTER) {
        for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
            cluster_task_token_t* task = &cluster_ctx.task_tk[i];
            if (task && (task->support_disp&(CLUSTER_MASK(token)))) {
                task->disp_sts &= ~(CLUSTER_MASK(token));
                break;
            }
        }
    }
}

static inline void cluster_run()
{
    APP_INFO("");
    arb_request_gain_slave(arb_magic_video, CLUSTER, &cluster_accessor_tbl[0]);
    for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
        cluster_task_token_t* task = (cluster_task_token_t*)&cluster_task_token[i];
        if (task) {
            if (task->thread_task && !task->thread_id) {
                APP_INFO("new task:%s", task->name);
                task->running = 1;
                task->thread_id = osThreadNew(task->thread_task, (void*)task, &task->thread_attr);
                cluster_ctx.detect_task |= CLUSTER_MASK(i);
                osEventFlagsClear(cluster_ctx.task_terminate, CLUSTER_MASK(i));
            }
        }
    }
}

static inline void cluster_terminate()
{
    APP_INFO("");
    if (cluster_ctx.detect_task) {
        uint32_t terminate = osEventFlagsWait(cluster_ctx.task_terminate, cluster_ctx.detect_task, osFlagsWaitAll|osFlagsNoClear, 1000);
        if ((terminate&cluster_ctx.detect_task) == terminate) {
            cluster_ctx.detect_task = 0;
        }
        else {
            cluster_ctx.detect_task = 0;
        }
    }
    for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
        cluster_task_token_t* task = (cluster_task_token_t*)&cluster_ctx.task_tk[i];
        if (task) {
            task->running = 0;
            if (!(cluster_ctx.detect_task&CLUSTER_MASK(i)) && task->thread_id) {
                osThreadTerminate(task->thread_id);
                task->thread_id = NULL;
            }
        }
    }
    arb_request_release_slave(arb_magic_video, CLUSTER, &cluster_accessor_tbl[0]);
}

static inline void cluster_suspend(int disp_id)
{
    APP_INFO("");
    for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
        cluster_task_token_t* task = (cluster_task_token_t*)&cluster_task_token[i];
        if (task) {
            if (strstr(task->name, "cls_lvgl")) {
                if ((task->support_disp>>disp_id)&1) {
                    task->disp_sts |= CLUSTER_MASK(disp_id);
                }
            }
        }
    }
}

static void cluster_stm_idle_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case cluster_evt_stop:
            cluster_terminate();
            break;
        case cluster_evt_init:
            break;
        default:
            break;
        }
    }
}

static void cluster_stm_run_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case cluster_evt_start:
        case cluster_evt_resume:
            cluster_run();
            break;
        default:
            break;
        }
    }
}

static void cluster_stm_suspend_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        cluster_suspend(msg->param);
    }
}

const static stm_func_t cluster_stm_enter_tbl[] = {
    { cluster_st_idle,      cluster_stm_idle_enter },
    { cluster_st_run,       cluster_stm_run_enter },
    { cluster_st_suspend,   cluster_stm_suspend_enter },
    { -1 }
};

const static stm_t cluster_stm_matrix[] = {
    { cluster_st_uninit,    cluster_evt_init,       cluster_st_idle },

    { cluster_st_idle,      cluster_evt_start,      cluster_st_run },
    { cluster_st_idle,      cluster_evt_resume,     cluster_st_run },

    { cluster_st_run,       cluster_evt_stop,       cluster_st_idle },
    { cluster_st_run,       cluster_evt_suspend,    cluster_st_suspend },

    { cluster_st_suspend,   cluster_evt_stop,       cluster_st_idle },
    { cluster_st_suspend,   cluster_evt_start,      cluster_st_run },
    { cluster_st_suspend,   cluster_evt_resume,     cluster_st_run },

    { -1 }
};

static stm_ctx_t cluster_stm_ctx = {
    .current = cluster_st_uninit,
    .previous = cluster_st_invalid,
    .next = cluster_st_invalid,
    .event = cluster_evt_invalid,
    .enter_tbl = cluster_stm_enter_tbl,
    .exit_tbl = NULL,
    .refresh_tbl = NULL,
};

static inline void cluster_stm_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < cluster_evt_max) {
            stm_set_event(cluster_stm_matrix, &cluster_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static inline void cluster_stm_init()
{
    APP_INFO("");
    msg_t msg = { .msgid = cluster_evt_init };
    cluster_set_evt(&msg);
}

static void cluster_property_changed(int pid, int old_val, int new_val, void* param)
{
    if (cluster_ctx.property_id) {
        for (int i = 0; cluster_ctx.property_id[i] != -1; i++) {
            if (pid == cluster_ctx.property_id[i]) {
                int sig = (new_val&0xffff0000)>>16;
                msg_t msg;
                switch (sig) {
                case cluster_sig_hand:
                    msg.msgid = cluster_evt_stop;
                    break;
                case cluster_sig_take:
                    msg.msgid = cluster_evt_start;
                    break;
                default:
                    msg.msgid = cluster_evt_invalid;
                    break;
                }
                if (msg.msgid != cluster_evt_invalid) {
                    cluster_set_evt(&msg);
                }
                break;
            }
        }
    }
}

void cluster_init(void* token)
{
    APP_INFO("");
    app_token_t* app_tk = (app_token_t*)token;
    cluster_ctx_t* ctx = &cluster_ctx;
    if (app_tk && ctx) {
        ctx->init = 1;
        app_tk->accessor_array = cluster_accessor_tbl;
        app_tk->accessor_num = sizeof(cluster_accessor_tbl)/sizeof(cluster_accessor_tbl[0]);
        ctx->launcher_q = app_tk->send_q;
        ctx->stm_msg = msg_token_new("cluster", 10, sizeof(msg_t));
        ctx->task_terminate = osEventFlagsNew(NULL);
        osEventFlagsClear(ctx->task_terminate, 0x00ffffff); /* low 24bits valid */
        if (ctx->task_tk) {
            for (int i = 0; i < CLUSTER_TASK_CNT; i++) {
                cluster_task_token_t* task = &ctx->task_tk[i];
                if (task) {
                    task->running = 0;
                    task->thread_id = NULL;
                    task->terminate_sig = ctx->task_terminate;
                    APP_INFO("task:%s", task->name);
                }
            }
        }
        cluster_stm_init();
        for (int i = 0; ctx->property_id[i] != -1; i++) {
            //system_property_observe(ctx->property_id[i], cluster_property_changed, NULL);
        }
    }
}

void cluster_entry(void* token)
{
    APP_INFO("");
    if (!cluster_ctx.stm_msg) return;
    while (1) {
        msg_t msg;
        if (!msg_token_recv(cluster_ctx.stm_msg, &msg, 1000)) {
            APP_INFO("recv:%d", msg.msgid);
            cluster_stm_dispatch(&msg);
        }
    }
    osThreadExit();
}

void cluster_notifier(int notify)
{
    APP_INFO("notify:%d", notify);
    msg_t msg;
    if (notify == app_notify_start) {
        msg.msgid = cluster_evt_start;
        cluster_set_evt(&msg);
    }
    else if (notify == app_notify_stop) {
        msg.msgid = cluster_evt_stop;
        cluster_set_evt(&msg);
    }
}

int cluster_set_evt(msg_t* msg)
{
    return msg_token_send(cluster_ctx.stm_msg, (void*)msg, 1000);
}

/* EOF */
