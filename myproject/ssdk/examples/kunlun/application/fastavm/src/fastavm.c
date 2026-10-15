/**
 * @file fastavm.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv fastavm application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "fastavm"

#include <string.h>
#include <cmsis_os2.h>
#include <lib/list.h>
#include <sd_property.h>
#include <disp_hal.h>
#include "container.h"
#include "fastavm.h"
#include "fastavm_cfg.h"
#include "fastavm_def.h"
#include "launcher.h"
#include "arbitrator.h"
#include "app_log.h"

static void fastavm_accessor_start(int magic, int token, void* accessor);
static void fastavm_accessor_stop(int magic, int token, void* accessor);

extern void fastavm_camera_task(void* args);
extern void fastavm_player_task(void* args);
extern void fastavm_car_task(void* args);
#ifdef CONFIG_USE_GUI_LVGL
extern void fastavm_lvgl_task(void* args);
#endif

static arb_accessor_t fastavm_accessor_tbl[] = {
    [0] = {
        .magic = arb_magic_video,
        .slave_tk = INFOTAINMENT,
        .prio = arb_prio_high,
        .p_start = fastavm_accessor_start,
        .p_stop = fastavm_accessor_stop,
        .p_suspend = fastavm_accessor_stop,
        .p_resume = fastavm_accessor_start,
    },
};

static fastavm_task_token_t fastavm_task_token[] = {
    [FASTAVM_CSI_TASK] = {
        .id = FASTAVM_CSI_TASK,
        .name = "avm_csi",
        .thread_attr = { .name = "avm_csi", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = fastavm_camera_task,
        .disp_sts = 0,
        .support_disp = 0,
        .issue_magic = "supplier",
        .issue_num = 0,
        .issue_size = 0,
    },
    [FASTAVM_PLAYER_TASK] = {
        .id = FASTAVM_PLAYER_TASK,
        .name = "avm_player",
        .thread_attr = { .name = "avm_player", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = fastavm_player_task,
        .disp_sts = 0,
        .support_disp = FASTAVM_MASK(INFOTAINMENT),
        .issue_magic = "consumer",
        .issue_num = 1,
        .issue_size = sizeof(fastavm_vstream_t),
    },
#ifdef CONFIG_USE_GUI_LVGL
    [FASTAVM_LVGL_TASK] = {
        .id = FASTAVM_LVGL_TASK,
        .name = "avm_lvgl",
        .thread_attr = { .name = "avm_lvgl", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = fastavm_lvgl_task,
        .disp_sts = 0,
        .support_disp = FASTAVM_MASK(INFOTAINMENT),
        .issue_magic = "non",
        .issue_num = 0,
        .issue_size = 0,
    },
#endif
};

static int fastavm_properties[] = {
    DMP_ID_AVM_STATUS,
    -1
};

static fastavm_ctx_t fastavm_ctx = {
    .init = 0,
    .property_id = fastavm_properties,
    .task_tk = fastavm_task_token,
    .detect_task = 0,
    .task_terminate = NULL,
};

static void fastavm_accessor_start(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == INFOTAINMENT) {
        for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
            fastavm_task_token_t* task = &fastavm_ctx.task_tk[i];
            if (task && (task->support_disp&(FASTAVM_MASK(token)))) {
                task->disp_sts |= FASTAVM_MASK(token);
                APP_INFO("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
            }
        }
    }
}

static void fastavm_accessor_stop(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video && token == INFOTAINMENT) {
        for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
            fastavm_task_token_t* task = &fastavm_ctx.task_tk[i];
            if (task && (task->support_disp&(FASTAVM_MASK(token)))) {
                task->disp_sts &= ~(FASTAVM_MASK(token));
                APP_INFO("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
            }
        }
    }
}

static inline void fastavm_run()
{
    arb_request_gain_slave(arb_magic_video, INFOTAINMENT, &fastavm_accessor_tbl[0]);
    for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)&fastavm_task_token[i];
        if (task) {
            if (task->thread_task && !task->thread_id) {
                APP_INFO("new task:%s", task->name);
                task->running = 1;
                task->thread_id = osThreadNew(task->thread_task, (void*)task, &task->thread_attr);
                fastavm_ctx.detect_task |= FASTAVM_MASK(i);
                osEventFlagsClear(fastavm_ctx.task_terminate, FASTAVM_MASK(i));
            }
        }
    }
}

static inline void fastavm_terminate()
{
    APP_INFO("");
    for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)&fastavm_ctx.task_tk[i];
        if (task) {
            task->running = 0;
        }
    }
    if (fastavm_ctx.detect_task) {
        uint32_t terminate = osEventFlagsWait(fastavm_ctx.task_terminate, fastavm_ctx.detect_task, osFlagsWaitAll|osFlagsNoClear, 1000);
        if ((terminate&fastavm_ctx.detect_task) == terminate) {
            fastavm_ctx.detect_task = 0;
        }
        else {
            fastavm_ctx.detect_task = 0;
        }
    }
    for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)&fastavm_ctx.task_tk[i];
        if (task) {
            task->disp_sts = 0;
            //osThreadTerminate(task->thread_id);
            task->thread_id = NULL;
        }
    }
    arb_request_release_slave(arb_magic_video, INFOTAINMENT, &fastavm_accessor_tbl[0]);
}

static void fastavm_stm_idle_enter(void* param)
{
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case fastavm_evt_stop:
            fastavm_terminate();
            break;
        case fastavm_evt_init:
            break;
        default:
            break;
        }
    }
}

static void fastavm_stm_run_enter(void* param)
{
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case fastavm_evt_start:
            fastavm_run();
            break;
        default:
            break;
        }
    }
}

const static stm_func_t fastavm_stm_enter_tbl[] = {
    { fastavm_st_idle,      fastavm_stm_idle_enter },
    { fastavm_st_run,       fastavm_stm_run_enter },
    { -1 }
};

const static stm_t fastavm_stm_matrix[] = {
    { fastavm_st_uninit,    fastavm_evt_init,       fastavm_st_idle },

    { fastavm_st_idle,      fastavm_evt_start,      fastavm_st_run },

    { fastavm_st_run,       fastavm_evt_stop,       fastavm_st_idle },

    { -1 }
};

static stm_ctx_t fastavm_stm_ctx = {
    .current = fastavm_st_uninit,
    .previous = fastavm_st_invalid,
    .next = fastavm_st_invalid,
    .event = fastavm_evt_invalid,
    .enter_tbl = fastavm_stm_enter_tbl,
    .exit_tbl = NULL,
    .refresh_tbl = NULL,
};

static inline void fastavm_stm_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < fastavm_evt_max) {
            stm_set_event(fastavm_stm_matrix, &fastavm_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static inline void fastavm_stm_init()
{
    msg_t msg = { .msgid = fastavm_evt_init };
    fastavm_set_evt(&msg);
}

static void fastavm_property_changed(int pid, int old_val, int new_val, void* param)
{
    if (fastavm_ctx.property_id) {
        for (int i = 0; fastavm_ctx.property_id[i] != -1; i++) {
            if (pid == fastavm_ctx.property_id[i]) {
                int sig = (new_val&0xffff0000)>>16;
                msg_t msg;
                switch (sig) {
                case fastavm_sig_hand:
                    msg.msgid = fastavm_evt_stop;
                    break;
                case fastavm_sig_take:
                    msg.msgid = fastavm_evt_start;
                    break;
                default:
                    msg.msgid = fastavm_evt_invalid;
                    break;
                }
                if (msg.msgid != fastavm_evt_invalid) {
                    fastavm_set_evt(&msg);
                }
                break;
            }
        }
    }
}

void fastavm_init(void* token)
{
    app_token_t* app_tk = (app_token_t*)token;
    fastavm_ctx_t* ctx = &fastavm_ctx;
    if (app_tk && ctx) {
        ctx->init = 1;
        app_tk->accessor_array = fastavm_accessor_tbl;
        app_tk->accessor_num = sizeof(fastavm_accessor_tbl)/sizeof(fastavm_accessor_tbl[0]);
        ctx->launcher_q = app_tk->send_q;
        ctx->stm_msg = msg_token_new("fastavm", 5, sizeof(msg_t));
        ctx->task_terminate = osEventFlagsNew(NULL);
        osEventFlagsClear(ctx->task_terminate, 0x00ffffff); /* low 24bits valid */
        if (ctx->task_tk) {
            for (int i = 0; i < FASTAVM_TASK_CNT; i++) {
                fastavm_task_token_t* task = &ctx->task_tk[i];
                if (task) {
                    task->running = 0;
                    task->thread_id = NULL;
                    task->terminate_sig = ctx->task_terminate;
                    task->issue = issue_token_new(task->issue_magic);
                    issue_token_init(task->issue, task->issue_num, task->issue_size, 1);
                    APP_INFO("task:%s, issue:%p, %s", task->name, task->issue, task->issue_magic);
                }
            }
            issue_token_serialization(2, ctx->task_tk[FASTAVM_CSI_TASK].issue, ctx->task_tk[FASTAVM_PLAYER_TASK].issue);
        }
        fastavm_stm_init();
        for (int i = 0; ctx->property_id[i] != -1; i++) {
            //system_property_observe(ctx->property_id[i], fastavm_property_changed, NULL);
        }
    }
}

void fastavm_entry(void* token)
{
    if (!fastavm_ctx.stm_msg) return;
    while (1) {
        msg_t msg;
        if (!msg_token_recv(fastavm_ctx.stm_msg, &msg, 1000)) {
            APP_INFO("recv:%d", msg.msgid);
            fastavm_stm_dispatch(&msg);
        }
    }
    osThreadExit();
}

void fastavm_notifier(int notify)
{
    APP_INFO("notify:%d", notify);
    msg_t msg;
    if (notify == app_notify_start) {
        msg.msgid = fastavm_evt_start;
        fastavm_set_evt(&msg);
    }
    else if (notify == app_notify_stop) {
        msg.msgid = fastavm_evt_stop;
        fastavm_set_evt(&msg);
    }
}

static int fastavm_primary_cam = 0;
void fastavm_set_primary_cam(int cam)
{
    fastavm_primary_cam = cam >= 0 && cam < 4 ? cam : fastavm_primary_cam;
}

int fastavm_get_primary_cam()
{
    return fastavm_primary_cam;
}

int fastavm_set_evt(msg_t* msg)
{
    return msg_token_send(fastavm_ctx.stm_msg, (void*)msg, 1000);
}
/* EOF */
