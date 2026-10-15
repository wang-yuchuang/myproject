/**
 * @file emirror.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv emirror application.
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
#include "emirror.h"
#include "emirror_cfg.h"
#include "emirror_def.h"
#include "launcher.h"
#include "arbitrator.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "emirror"

static void emirror_accessor_start(int magic, int token, void* accessor);
static void emirror_accessor_stop(int magic, int token, void* accessor);

extern void emirror_camera_task(void* args);
extern void emirror_player_task(void* args);
extern void emirror_car_task(void* args);
#ifdef CONFIG_USE_GUI_LVGL
extern void emirror_lvgl_task(void* args);
#endif

static arb_accessor_t emirror_accessor_tbl[] = {
    [0] = {
        .magic = arb_magic_video,
        .slave_tk = LEFT_MIRROR,
        .prio = arb_prio_high,
        .p_start = emirror_accessor_start,
        .p_stop = emirror_accessor_stop,
        .p_suspend = emirror_accessor_stop,
        .p_resume = emirror_accessor_start,
    },
    [1] = {
        .magic = arb_magic_video,
        .slave_tk = RIGHT_MIRROR,
        .prio = arb_prio_high,
        .p_start = emirror_accessor_start,
        .p_stop = emirror_accessor_stop,
        .p_suspend = emirror_accessor_stop,
        .p_resume = emirror_accessor_start,
    },
};

static emirror_task_token_t emirror_task_token[] = {
    [EMIRROR_CSI_TASK] = {
        .id = EMIRROR_CSI_TASK,
        .name = "emirror_csi",
        .thread_attr = { .name = "emirror_csi", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = emirror_camera_task,
        .disp_sts = 0,
        .support_disp = 0,
        .issue_magic = "supplier",
        .issue_num = 0,
        .issue_size = 0,
    },
    [EMIRROR_PLAYER_0_TASK] = {
        .id = EMIRROR_PLAYER_0_TASK,
        .name = "emirror_player_left",
        .thread_attr = { .name = "emirror_player_left", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = emirror_player_task,
        .disp_sts = 0,
        .support_disp = EMIRROR_MASK(LEFT_MIRROR),
        .issue_magic = "consumer_left",
        .issue_num = 1,
        .issue_size = sizeof(emirror_vstream_t),
    },
    [EMIRROR_PLAYER_1_TASK] = {
        .id = EMIRROR_PLAYER_0_TASK,
        .name = "emirror_player_right",
        .thread_attr = { .name = "emirror_player_right", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = emirror_player_task,
        .disp_sts = 0,
        .support_disp = EMIRROR_MASK(RIGHT_MIRROR),
        .issue_magic = "consumer_right",
        .issue_num = 1,
        .issue_size = sizeof(emirror_vstream_t),
    },
#ifdef CONFIG_USE_GUI_LVGL
    [EMIRROR_LVGL_TASK] = {
        .id = EMIRROR_LVGL_TASK,
        .name = "emirror_lvgl",
        .thread_attr = { .name = "emirror_lvgl", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = emirror_lvgl_task,
        .disp_sts = 0,
        .support_disp = EMIRROR_MASK(LEFT_MIRROR),
        .issue_magic = "non",
        .issue_num = 0,
        .issue_size = 0,
    },
#endif
};

static int emirror_properties[] = {
    DMP_ID_RVM_STATUS,
    -1
};

static emirror_ctx_t emirror_ctx = {
    .init = 0,
    .property_id = emirror_properties,
    .task_tk = emirror_task_token,
    .detect_task = 0,
    .task_terminate = NULL,
};

static void emirror_accessor_start(int magic, int token, void* accessor)
{
    APP_INFO("");
    if (magic == arb_magic_video && (token == LEFT_MIRROR || token == RIGHT_MIRROR)) {
        for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
            emirror_task_token_t* task = &emirror_ctx.task_tk[i];
            if (task && (task->support_disp&(EMIRROR_MASK(token)))) {
                task->disp_sts |= EMIRROR_MASK(token);
                APP_INFO("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
            }
        }
    }
}

static void emirror_accessor_stop(int magic, int token, void* accessor)
{
    APP_INFO("");
    if (magic == arb_magic_video && (token == LEFT_MIRROR || token == RIGHT_MIRROR)) {
        for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
            emirror_task_token_t* task = &emirror_ctx.task_tk[i];
            if (task && (task->support_disp&(EMIRROR_MASK(token)))) {
                task->disp_sts &= ~(EMIRROR_MASK(token));
                APP_INFO("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
            }
        }
    }
}

static inline void emirror_run()
{
    APP_INFO("");
    arb_request_gain_slave(arb_magic_video, LEFT_MIRROR, &emirror_accessor_tbl[0]);
    arb_request_gain_slave(arb_magic_video, RIGHT_MIRROR, &emirror_accessor_tbl[1]);
    for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
        emirror_task_token_t* task = (emirror_task_token_t*)&emirror_task_token[i];
        if (task) {
            if (task->thread_task && !task->thread_id) {
                APP_INFO("new task:%s", task->name);
                task->running = 1;
                task->thread_id = osThreadNew(task->thread_task, (void*)task, &task->thread_attr);
                emirror_ctx.detect_task |= EMIRROR_MASK(i);
                osEventFlagsClear(emirror_ctx.task_terminate, EMIRROR_MASK(i));
            }
        }
    }
}

static inline void emirror_terminate()
{
    APP_INFO("");
    for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
        emirror_task_token_t* task = (emirror_task_token_t*)&emirror_ctx.task_tk[i];
        if (task) {
            task->running = 0;
        }
    }
    if (emirror_ctx.detect_task) {
        uint32_t terminate = osEventFlagsWait(emirror_ctx.task_terminate, emirror_ctx.detect_task, osFlagsWaitAll|osFlagsNoClear, 1000);
        if ((terminate&emirror_ctx.detect_task) == terminate) {
            emirror_ctx.detect_task = 0;
        }
        else {
            emirror_ctx.detect_task = 0;
        }
    }
    for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
        emirror_task_token_t* task = (emirror_task_token_t*)&emirror_ctx.task_tk[i];
        if (task) {
            task->disp_sts = 0;
            //osThreadTerminate(task->thread_id);
            task->thread_id = NULL;
        }
    }
    arb_request_release_slave(arb_magic_video, LEFT_MIRROR, &emirror_accessor_tbl[0]);
    arb_request_release_slave(arb_magic_video, RIGHT_MIRROR, &emirror_accessor_tbl[1]);
}

static void emirror_stm_idle_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case emirror_evt_stop:
            emirror_terminate();
            break;
        case emirror_evt_init:
        default:
            break;
        }
    }
}

static void emirror_stm_run_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case emirror_evt_start:
            emirror_run();
            break;
        default:
            break;
        }
    }
}

const static stm_func_t emirror_stm_enter_tbl[] = {
    { emirror_st_idle,      emirror_stm_idle_enter },
    { emirror_st_run,       emirror_stm_run_enter },
    { -1 }
};

const static stm_t emirror_stm_matrix[] = {
    { emirror_st_uninit,    emirror_evt_init,       emirror_st_idle },

    { emirror_st_idle,      emirror_evt_start,      emirror_st_run },

    { emirror_st_run,       emirror_evt_stop,       emirror_st_idle },

    { -1 }
};

static stm_ctx_t emirror_stm_ctx = {
    .current = emirror_st_uninit,
    .previous = emirror_st_invalid,
    .next = emirror_st_invalid,
    .event = emirror_evt_invalid,
    .enter_tbl = emirror_stm_enter_tbl,
    .exit_tbl = NULL,
    .refresh_tbl = NULL,
};

static inline void emirror_stm_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < emirror_evt_max) {
            stm_set_event(emirror_stm_matrix, &emirror_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static inline void emirror_stm_init()
{
    APP_INFO("");
    msg_t msg = { .msgid = emirror_evt_init };
    emirror_set_evt(&msg);
}

static void emirror_property_changed(int pid, int old_val, int new_val, void* param)
{
}

void emirror_init(void* token)
{
    APP_INFO("");
    app_token_t* app_tk = (app_token_t*)token;
    emirror_ctx_t* ctx = &emirror_ctx;
    if (app_tk && ctx) {
        ctx->init = 1;
        app_tk->accessor_array = emirror_accessor_tbl;
        app_tk->accessor_num = sizeof(emirror_accessor_tbl)/sizeof(emirror_accessor_tbl[0]);
        ctx->launcher_q = app_tk->send_q;
        ctx->stm_msg = msg_token_new("emirror", 5, sizeof(msg_t));
        ctx->task_terminate = osEventFlagsNew(NULL);
        osEventFlagsClear(ctx->task_terminate, 0x00ffffff); /* low 24bits valid */
        if (ctx->task_tk) {
            for (int i = 0; i < EMIRROR_TASK_CNT; i++) {
                emirror_task_token_t* task = &ctx->task_tk[i];
                if (task) {
                    task->running = 0;
                    task->thread_id = NULL;
                    task->terminate_sig = ctx->task_terminate;
                    task->issue = issue_token_new(task->issue_magic);
                    issue_token_init(task->issue, task->issue_num, task->issue_size, 1);
                    APP_INFO("task:%s, issue:%p, %s", task->name, task->issue, task->issue_magic);
                }
            }
            issue_token_serialization(3, ctx->task_tk[EMIRROR_CSI_TASK].issue, \
                                         ctx->task_tk[EMIRROR_PLAYER_0_TASK].issue, \
                                         ctx->task_tk[EMIRROR_PLAYER_1_TASK].issue);
        }
        emirror_stm_init();
        for (int i = 0; ctx->property_id[i] != -1; i++) {
            system_property_observe(ctx->property_id[i], emirror_property_changed, NULL);
        }
    }
}

void emirror_entry(void* token)
{
    APP_INFO("");
    if (!emirror_ctx.stm_msg) return;
    while (1) {
        msg_t msg;
        if (!msg_token_recv(emirror_ctx.stm_msg, &msg, 1000)) {
            APP_INFO("recv:%d", msg.msgid);
            emirror_stm_dispatch(&msg);
        }
    }
}

void emirror_notifier(int notify)
{
    APP_INFO("notify:%d", notify);
    msg_t msg;
    if (notify == app_notify_start) {
        msg.msgid = emirror_evt_start;
        emirror_set_evt(&msg);
    }
    else if (notify == app_notify_stop) {
        msg.msgid = emirror_evt_stop;
        emirror_set_evt(&msg);
    }
}

int emirror_set_evt(msg_t* msg)
{
    return msg_token_send(emirror_ctx.stm_msg, (void*)msg, 1000);
}
/* EOF */
