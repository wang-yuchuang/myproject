/**
 * @file controlpanel.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv controlpanel application.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include <lib/list.h>
#include <sd_property.h>
#include <disp_hal.h>
#include "container.h"
#include "controlpanel.h"
#include "controlpanel_cfg.h"
#include "controlpanel_def.h"
#include "launcher.h"
#include "arbitrator.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "controlpanel"

static void cp_accessor_start(int magic, int token, void* accessor);
static void cp_accessor_stop(int magic, int token, void* accessor);

extern void cp_camera_task(void* args);
extern void cp_player_task(void* args);
extern void cp_car_task(void* args);
#ifdef CONFIG_USE_GUI_LVGL
extern void cp_lvgl_task(void* args);
#endif

static arb_accessor_t cp_accessor_tbl[] = {
    [0] = {
        .magic = arb_magic_video,
        .slave_tk = CONTROLPANEL,
        .prio = arb_prio_mid,
        .p_start = cp_accessor_start,
        .p_stop = cp_accessor_stop,
        .p_suspend = cp_accessor_stop,
        .p_resume = cp_accessor_start,
    },
};

static cp_task_token_t cp_task_token[] = {
#ifdef CONFIG_USE_GUI_LVGL
    [CP_LVGL_TASK] = {
        .id = CP_LVGL_TASK,
        .name = "cp_lvgl",
        .thread_attr = { .name = "cp_lvgl", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = cp_lvgl_task,
        .disp_sts = 0,
        .support_disp = CP_MASK(CONTROLPANEL),
    },
#endif
};

static int cp_properties[] = {
    DMP_ID_CP_STATUS,
    -1
};

static cp_ctx_t cp_ctx = {
    .init = 0,
    .property_id = cp_properties,
    .task_tk = cp_task_token,
    .detect_task = 0,
    .task_terminate = NULL,
};

static void cp_accessor_start(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == CONTROLPANEL) {
        for (int i = 0; i < CP_TASK_CNT; i++) {
            cp_task_token_t* task = &cp_ctx.task_tk[i];
            if (task && (task->support_disp&(CP_MASK(token)))) {
                task->disp_sts |= CP_MASK(token);
                break;
            }
        }
    }
}

static void cp_accessor_stop(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == CONTROLPANEL) {
        for (int i = 0; i < CP_TASK_CNT; i++) {
            cp_task_token_t* task = &cp_ctx.task_tk[i];
            if (task && (task->support_disp&(CP_MASK(token)))) {
                task->disp_sts &= ~(CP_MASK(token));
                break;
            }
        }
    }
}

static inline void cp_run()
{
    APP_INFO("");
    arb_request_gain_slave(arb_magic_video, CONTROLPANEL, &cp_accessor_tbl[0]);
    for (int i = 0; i < CP_TASK_CNT; i++) {
        cp_task_token_t* task = (cp_task_token_t*)&cp_task_token[i];
        if (task) {
            if (task->thread_task && !task->thread_id) {
                APP_INFO("new task:%s", task->name);
                task->running = 1;
                task->thread_id = osThreadNew(task->thread_task, (void*)task, &task->thread_attr);
                cp_ctx.detect_task |= CP_MASK(i);
                osEventFlagsClear(cp_ctx.task_terminate, CP_MASK(i));
            }
        }
    }
}

static inline void cp_terminate()
{
    APP_INFO("");
    for (int i = 0; i < CP_TASK_CNT; i++) {
        cp_task_token_t* task = (cp_task_token_t*)&cp_ctx.task_tk[i];
        if (task) {
            task->running = 0;
        }
    }
    if (cp_ctx.detect_task) {
        uint32_t terminate = osEventFlagsWait(cp_ctx.task_terminate, cp_ctx.detect_task, osFlagsWaitAll|osFlagsNoClear, 1000);
        if ((terminate&cp_ctx.detect_task) == terminate) {
            cp_ctx.detect_task = 0;
        }
        else {
            cp_ctx.detect_task = 0;
        }
    }
    for (int i = 0; i < CP_TASK_CNT; i++) {
        cp_task_token_t* task = (cp_task_token_t*)&cp_ctx.task_tk[i];
        if (task) {
            task->disp_sts = 0;
            //osThreadTerminate(task->thread_id);
            task->thread_id = NULL;
        }
    }
    arb_request_release_slave(arb_magic_video, CONTROLPANEL, &cp_accessor_tbl[0]);
}

static inline void cp_suspend(int disp_id)
{
    APP_INFO("");
    for (int i = 0; i < CP_TASK_CNT; i++) {
        cp_task_token_t* task = (cp_task_token_t*)&cp_task_token[i];
        if (task) {
            if (strnstr(task->name, "lvgl", strlen("lvgl"))) {
                if ((task->support_disp>>disp_id)&1) {
                    task->disp_sts |= CP_MASK(disp_id);
                }
            }
        }
    }
}

static void cp_stm_idle_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case cp_evt_stop:
            cp_terminate();
            break;
        case cp_evt_init:
            break;
        default:
            break;
        }
    }
}

static void cp_stm_run_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case cp_evt_start:
        case cp_evt_resume:
            cp_run();
            break;
        default:
            break;
        }
    }
}

static void cp_stm_suspend_enter(void* param)
{
    APP_INFO("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        cp_suspend(msg->param);
    }
}

const static stm_func_t cp_stm_enter_tbl[] = {
    { cp_st_idle,      cp_stm_idle_enter },
    { cp_st_run,       cp_stm_run_enter },
    { cp_st_suspend,   cp_stm_suspend_enter },
    { -1 }
};

const static stm_t cp_stm_matrix[] = {
    { cp_st_uninit,    cp_evt_init,       cp_st_idle },

    { cp_st_idle,      cp_evt_start,      cp_st_run },
    { cp_st_idle,      cp_evt_resume,     cp_st_run },

    { cp_st_run,       cp_evt_stop,       cp_st_idle },
    { cp_st_run,       cp_evt_suspend,    cp_st_suspend },

    { cp_st_suspend,   cp_evt_stop,       cp_st_idle },
    { cp_st_suspend,   cp_evt_start,      cp_st_run },
    { cp_st_suspend,   cp_evt_resume,     cp_st_run },

    { -1 }
};

static stm_ctx_t cp_stm_ctx = {
    .current = cp_st_uninit,
    .previous = cp_st_invalid,
    .next = cp_st_invalid,
    .event = cp_evt_invalid,
    .enter_tbl = cp_stm_enter_tbl,
    .exit_tbl = NULL,
    .refresh_tbl = NULL,
};

static inline void cp_stm_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < cp_evt_max) {
            stm_set_event(cp_stm_matrix, &cp_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static inline void cp_stm_init()
{
    APP_INFO("");
    msg_t msg = { .msgid = cp_evt_init };
    cp_set_evt(&msg);
}

static void cp_property_changed(int pid, int old_val, int new_val, void* param)
{
    if (cp_ctx.property_id) {
        for (int i = 0; cp_ctx.property_id[i] != -1; i++) {
            if (pid == cp_ctx.property_id[i]) {
                int sig = (new_val&0xffff0000)>>16;
                msg_t msg;
                switch (sig) {
                case cp_sig_hand:
                    msg.msgid = cp_evt_stop;
                    break;
                case cp_sig_take:
                    msg.msgid = cp_evt_start;
                    break;
                default:
                    msg.msgid = cp_evt_invalid;
                    break;
                }
                if (msg.msgid != cp_evt_invalid) {
                    cp_set_evt(&msg);
                }
                break;
            }
        }
    }
}

void cp_init(void* token)
{
    APP_INFO("");
    app_token_t* app_tk = (app_token_t*)token;
    cp_ctx_t* ctx = &cp_ctx;
    if (app_tk && ctx) {
        ctx->init = 1;
        app_tk->accessor_array = cp_accessor_tbl;
        app_tk->accessor_num = sizeof(cp_accessor_tbl)/sizeof(cp_accessor_tbl[0]);
        ctx->launcher_q = app_tk->send_q;
        ctx->stm_msg = msg_token_new("controlpanel", 10, sizeof(msg_t));
        ctx->task_terminate = osEventFlagsNew(NULL);
        osEventFlagsClear(ctx->task_terminate, 0x00ffffff); /* low 24bits valid */
        if (ctx->task_tk) {
            for (int i = 0; i < CP_TASK_CNT; i++) {
                cp_task_token_t* task = &ctx->task_tk[i];
                if (task) {
                    task->running = 0;
                    task->thread_id = NULL;
                    task->terminate_sig = ctx->task_terminate;
                    APP_INFO("task:%s", task->name);
                }
            }
        }
        cp_stm_init();
        for (int i = 0; ctx->property_id[i] != -1; i++) {
            system_property_observe(ctx->property_id[i], cp_property_changed, NULL);
        }
    }
}

void cp_entry(void* token)
{
    APP_INFO("");
    if (!cp_ctx.stm_msg) return;
    while (1) {
        msg_t msg;
        if (!msg_token_recv(cp_ctx.stm_msg, &msg, 1000)) {
            APP_INFO("recv:%d", msg.msgid);
            cp_stm_dispatch(&msg);
        }
    }
    osThreadExit();
}

void cp_notifier(int notify)
{
    APP_INFO("notify:%d", notify);
    msg_t msg;
    if (notify == app_notify_start) {
        msg.msgid = cp_evt_start;
        cp_set_evt(&msg);
    }
    else if (notify == app_notify_stop) {
        msg.msgid = cp_evt_stop;
        cp_set_evt(&msg);
    }
}

int cp_set_evt(msg_t* msg)
{
    return msg_token_send(cp_ctx.stm_msg, (void*)msg, 1000);
}
/* EOF */
