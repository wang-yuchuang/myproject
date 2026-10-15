/**
 * @file animation.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <disp_hal.h>
#include <driver/kunlun/vpu/vpu.h>
#include <sd_property.h>
#include <sdm_display.h>
#include "state_machine.h"
#include "arbitrator.h"
#include "ani_cfg.h"
#include "ani_def.h"
#include "launcher.h"
#include "animation.h"
#include "container.h"
#include "app_log.h"
#ifdef CONFIG_AUDIO_HUB_SVC
#include "ahub_svc.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "animation"

#if defined(CONFIG_AUDIO_MANAGER)
#define ALL_TASK_MASK \
        (ANI_MASK(ANI_DEC_TASK)|ANI_MASK(ANI_PLAYER_TASK_0)|ANI_MASK(ANI_AUDIO_TASK)|ANI_MASK(ANI_RES_LOADER_TASK))
#else
#define ALL_TASK_MASK \
        (ANI_MASK(ANI_DEC_TASK)|ANI_MASK(ANI_PLAYER_TASK_0)|ANI_MASK(ANI_RES_LOADER_TASK))
#endif

static void ani_accessor_start(int magic, int token, void* accessor);
static void ani_accessor_stop(int magic, int token, void* accessor);

extern void ani_decoder_task(void* args);
extern void ani_player_task(void* args);
#if defined(CONFIG_AUDIO_MANAGER)
extern void ani_audio_task(void* args);
#endif
extern void ani_res_loader_task(void* args);

static arb_accessor_t ani_accessor_tbl[] = {
    [CLUSTER] = {
        .magic = arb_magic_video,
        .slave_tk = CLUSTER,
        .prio = arb_prio_low,
        .p_start = ani_accessor_start,
        .p_stop = ani_accessor_stop,
        .p_suspend = ani_accessor_stop,
        .p_resume = ani_accessor_start,
    },
    [INFOTAINMENT] = {
        .magic = arb_magic_video,
        .slave_tk = INFOTAINMENT,
        .prio = arb_prio_low,
        .p_start = ani_accessor_start,
        .p_stop = ani_accessor_stop,
        .p_suspend = ani_accessor_stop,
        .p_resume = ani_accessor_start,
    },
    [ENTERTAINMENT] = {
        .magic = arb_magic_video,
        .slave_tk = ENTERTAINMENT,
        .prio = arb_prio_low,
        .p_start = ani_accessor_start,
        .p_stop = ani_accessor_stop,
        .p_suspend = ani_accessor_stop,
        .p_resume = ani_accessor_start,
    },
};

static ani_task_token_t ani_task_token[ANI_TASK_CNT] = {
    [ANI_DEC_TASK] = {
        .id = ANI_DEC_TASK,
        .name = "decoder",
        .thread_attr = { .name = "ani_dec", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = ani_decoder_task,
        .disp_sts = 0,
        .support_disp = 0,
        .support_res = 0,
        .issue_magic = "supplier",
        .issue_num = 0,
        .issue_size = 0,
    },
    [ANI_PLAYER_TASK_0] = {
        .id = ANI_PLAYER_TASK_0,
        .name = "player0",
        .thread_attr = { .name = "ani_player", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = ani_player_task,
        .disp_sts = 0,
        .support_disp = ANI_DISP_ALL_SUPPORT,
        .support_res = -1,
        .issue_magic = "supplier_consumer",
        .issue_num = 1,
        .issue_size = sizeof(decoder_output_t),
    },
#if defined(CONFIG_AUDIO_MANAGER)
    [ANI_AUDIO_TASK] = {
        .id = ANI_AUDIO_TASK,
        .name = "audio",
        .thread_attr = { .name = "ani_audio", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = ani_audio_task,
        .disp_sts = 0,
        .support_disp = 0,
        .support_res = -1,
        .issue_magic = "non",
        .issue_num = 0,
        .issue_size = 0,
    },
#endif
    [ANI_RES_LOADER_TASK] = {
        .id = ANI_RES_LOADER_TASK,
        .name = "res",
        .thread_attr = { .name = "ani_res", .stack_size = 4096, .priority = osPriorityNormal },
        .thread_task = ani_res_loader_task,
        .disp_sts = 0,
        .support_disp = 0,
        .support_res = 0,
        .issue_magic = "non",
        .issue_num = 0,
        .issue_size = 0,
    },
};

static int ani_properties[] = {
    DMP_ID_AVM_STATUS,
    DMP_ID_BA_STATUS,
    DMP_ID_CLUSTER_STATUS,
    DMP_ID_ANDROID_BOOT_STATUS,
    DMP_ID_CVBS_STATUS,
    -1
};

static ani_ctx_t ani_ctx = {
    .init = 0,
    .property_id = ani_properties,
    .task_tk = ani_task_token,
    .detect_task = 0,
    .task_terminate = NULL,
};

int ani_disp_auto_elapse = 0;
osEventFlagsId_t res_loader_ready = NULL;

/**=================================
 *              FUNC
 **=================================*/
static void ani_accessor_start(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video) {
        if (token == CLUSTER || token == INFOTAINMENT || token == ENTERTAINMENT) {
            for (int i = 0; i < ANI_TASK_CNT; i++) {
                ani_task_token_t* task = &ani_ctx.task_tk[i];
                if (task && (task->support_disp&(ANI_MASK(token)))) {
                    osStatus_t ret = ANI_DISP_BUSY(task);
                    task->disp_sts |= ANI_MASK(token);
                    ANI_DISP_IDLE(task, ret);
                    APP_CRIT("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
                    break;
                }
            }
        }
    }
}

static void ani_accessor_stop(int magic, int token, void* accessor)
{
    if (magic == arb_magic_video) {
        if (token == CLUSTER || token == INFOTAINMENT || token == ENTERTAINMENT) {
            for (int i = 0; i < ANI_TASK_CNT; i++) {
                ani_task_token_t* task = &ani_ctx.task_tk[i];
                if (task && (task->support_disp&(ANI_MASK(token)))) {
                    osStatus_t ret = ANI_DISP_BUSY(task);
                    int clear = (task->disp_sts&(ANI_MASK(token))) ? 1 : 0;
                    task->disp_sts &= ~(ANI_MASK(token));
                    if (clear) {
                        sdm_display_t* disp = arb_get_slave_handle(magic, token);
                        if (disp && disp->handle) sdm_clear_display(disp->handle);
                    }
                    ANI_DISP_IDLE(task, ret);
                    APP_CRIT("magic:%d, token:%d, task:%s, disp_sts:%x", magic, token, task->name, task->disp_sts);
                    break;
                }
            }
        }
    }
}

static void ani_property_changed(int pid, int old_val, int new_val, void* param)
{
    int sig = new_val&0xffff;
    int remote = (new_val>>16)&0xffff;
    APP_CRIT("pid:%d, new_val:%x, sig:%d, remote:%d", pid, new_val, sig, remote);
    switch (pid) {
    case DMP_ID_AVM_STATUS:
        {
            if (remote == 1) {
                msg_t msg = { .msgid = ani_evt_hide, .param = ANI_MASK(INFOTAINMENT) };
                ani_set_evt(&msg);
            }
            break;
        }
    case DMP_ID_BA_STATUS:
        {
            if (remote == 2) {
                msg_t msg = { .msgid = ani_evt_playback, .param = ani_evt_param_replay };
                ani_set_evt(&msg);
            }
            break;
        }
    case DMP_ID_CLUSTER_STATUS:
    case DMP_ID_CVBS_STATUS:
        {
            if (remote == 1) {
                msg_t msg = { .msgid = ani_evt_hide, .param = ANI_MASK(CLUSTER) };
                ani_set_evt(&msg);
            }
            break;
        }
    case DMP_ID_ANDROID_BOOT_STATUS:
        {
            if (remote == 1) {
                msg_t msg = { .msgid = ani_evt_hide, .param = (ANI_MASK(INFOTAINMENT)|ANI_MASK(ENTERTAINMENT)) };
                ani_set_evt(&msg);
            }else if (remote == 2) {
                msg_t msg = { .msgid = ani_evt_hide, .param = (ANI_MASK(INFOTAINMENT)) };
                ani_set_evt(&msg);
            }
            break;
        }
    default:
        break;
    }
}

static inline void ani_display_hide(int mask)
{
    APP_CRIT("mask:%x", mask);
    for (int i = 0; i < ANI_TASK_CNT; i++) {
        ani_task_token_t* task = (ani_task_token_t*)&ani_task_token[i];
        if (task) {
            if (task->support_disp&mask) {
                ani_disp_auto_elapse |= mask;
                int sts = mask;
                for (int j = 0; sts; sts>>=1, j++) {
                    if (sts&1) {
                        arb_request_release_slave(arb_magic_video, j, &ani_accessor_tbl[j]);
                    }
                }
            }
            osStatus_t ret = ANI_DISP_BUSY(task);
            if (task->support_disp != 0 && task->disp_sts == 0) {
                /* terminate */
                msg_t msg = { .msgid = ani_evt_stop };
                ani_set_evt(&msg);
            }
            ANI_DISP_IDLE(task, ret);
        }
    }
}

static inline void ani_display_show(int disp_id)
{
}

static inline void ani_playback(int param)
{
    APP_DBG("param:%d", param);
    switch (param) {
    case ani_evt_param_openning:
        {
            int cls_sts = 0, ivi_sts = 0;
            system_property_get(DMP_ID_CLUSTER_STATUS, &cls_sts);
            system_property_get(DMP_ID_ANDROID_BOOT_STATUS, &ivi_sts);
            cls_sts = ((cls_sts>>16)&0xffff);
            ivi_sts = ((ivi_sts>>16)&0xffff);
            APP_CRIT("cls_sts:%d, ivi_sts:%d", cls_sts, ivi_sts);
            if (cls_sts != 1) {
                arb_request_gain_slave(arb_magic_video, CLUSTER, &ani_accessor_tbl[CLUSTER]);
            }
            if (ivi_sts != 1) {
                arb_request_gain_slave(arb_magic_video, INFOTAINMENT, &ani_accessor_tbl[INFOTAINMENT]);
                arb_request_gain_slave(arb_magic_video, ENTERTAINMENT, &ani_accessor_tbl[ENTERTAINMENT]);
            }
            break;
        }
    case ani_evt_param_replay:
    default:
        {
            arb_request_gain_slave(arb_magic_video, CLUSTER, &ani_accessor_tbl[CLUSTER]);
            arb_request_gain_slave(arb_magic_video, INFOTAINMENT, &ani_accessor_tbl[INFOTAINMENT]);
            arb_request_gain_slave(arb_magic_video, ENTERTAINMENT, &ani_accessor_tbl[ENTERTAINMENT]);
            break;
        }
    }
    for (int i = 0; i < ANI_TASK_CNT; i++) {
        ani_task_token_t* task = (ani_task_token_t*)&ani_task_token[i];
        if (task) {
            APP_CRIT("new start:%s", task->name);
            if (task->thread_task && !task->thread_id) {
                issue_token_clear(task->issue);
                task->running = 1;
                task->thread_id = osThreadNew(task->thread_task, (void*)task, &task->thread_attr);
#if defined(CONFIG_AUDIO_MANAGER) && defined(CONFIG_AUDIO_HUB)
                if (ANI_AUDIO_TASK != task->id)
#endif
                ani_ctx.detect_task |= ANI_MASK(i);
                osEventFlagsClear(ani_ctx.task_terminate, ANI_MASK(i));
            }
        }
    }
}

static inline void ani_elapse(uint32_t task_id)
{
    APP_CRIT("task_id:%d, ani_ctx.detect_task:%x", task_id, ani_ctx.detect_task);
    uint32_t task_mask = ANI_MASK(task_id);
    if (task_id < ANI_TASK_CNT && task_mask && !(0xff000000&task_mask)) {
        ani_task_token_t* task = (ani_task_token_t*)&ani_ctx.task_tk[task_id];
        if (task) {
            task->disp_sts = 0;
            task->running = 0;
            uint32_t termiante = osEventFlagsWait(task->terminate_sig, task_mask, osFlagsNoClear, 1000);
            if (termiante&task_mask) {
                osThreadTerminate(task->thread_id);
                task->thread_id = NULL;
                ani_ctx.detect_task &= ~task_mask;
                APP_CRIT("%s task terminate, ani_ctx.detect_task:%x", task->name, ani_ctx.detect_task);
            }
        }
    }
    if (!ani_ctx.detect_task) {
        msg_t msg = { .msgid = ani_evt_stop };
        ani_set_evt(&msg);
    }
}

static inline void ani_stop()
{
    APP_CRIT("");
    for (int i = 0; i < ANI_TASK_CNT; i++) {
        ani_task_token_t* task = (ani_task_token_t*)&ani_ctx.task_tk[i];
        if (task) {
            task->running = 0;
        }
    }
    if (ani_ctx.detect_task) {
        uint32_t termiante = osEventFlagsWait(ani_ctx.task_terminate, ani_ctx.detect_task, osFlagsWaitAll|osFlagsNoClear, 1000);
        if ((termiante&ani_ctx.detect_task) == ani_ctx.detect_task) {
            ani_ctx.detect_task = 0;
        }
        else {
            ani_ctx.detect_task = 0;
        }
    }

    arb_request_release_slave(arb_magic_video, CLUSTER, &ani_accessor_tbl[CLUSTER]);
    arb_request_release_slave(arb_magic_video, INFOTAINMENT, &ani_accessor_tbl[INFOTAINMENT]);
    arb_request_release_slave(arb_magic_video, ENTERTAINMENT, &ani_accessor_tbl[ENTERTAINMENT]);

    for (int i = 0; i < ANI_TASK_CNT; i++) {
        ani_task_token_t* task = (ani_task_token_t*)&ani_ctx.task_tk[i];
        if (task) {
            task->disp_sts = 0;
            //osThreadTerminate(task->thread_id);
            task->thread_id = NULL;
        }
    }
}

/**=================================
 *              STM
 **=================================*/
static void ani_stm_idle_enter(void* param)
{
    APP_DBG("");
    if (param) {
        msg_t* msg = (msg_t*)param;
        switch (msg->msgid) {
        case ani_evt_stop:
            ani_stop();
            break;
        case ani_evt_hide:
            ani_display_hide(msg->param);
            break;
        case ani_evt_show:
            ani_display_show(msg->param);
            break;
        default:
            break;
        }
    }
}

static void ani_stm_run_enter(void* param)
{
    msg_t* msg = (msg_t*)param;
    if (msg) {
        APP_DBG("msgid:%d", msg->msgid);
        switch (msg->msgid) {
        case ani_evt_playback:
            ani_playback(msg->param);
            break;
        case ani_evt_hide:
            ani_display_hide(msg->param);
            break;
        case ani_evt_show:
            ani_display_show(msg->param);
            break;
        case ani_evt_elapsed:
            ani_elapse(msg->param);
            break;
        default:
            break;
        }
    }
}

const static stm_func_t ani_stm_enter_tbl[] = {
    { ani_st_idle,      ani_stm_idle_enter },
    { ani_st_run,       ani_stm_run_enter },
    { -1 }
};

const static stm_t ani_stm_matrix[] = {
    { ani_st_uninit,    ani_evt_init,       ani_st_idle },

    { ani_st_idle,      ani_evt_playback,   ani_st_run },
    { ani_st_idle,      ani_evt_hide,       ani_st_idle },
    { ani_st_idle,      ani_evt_show,       ani_st_idle },

    { ani_st_run,       ani_evt_stop,       ani_st_idle },
    { ani_st_run,       ani_evt_elapsed,    ani_st_run },
    { ani_st_run,       ani_evt_hide,       ani_st_run },
    { ani_st_run,       ani_evt_show,       ani_st_run },

    { -1 }
};

static stm_ctx_t ani_stm_ctx = {
    .current = ani_st_invalid,
    .previous = ani_st_invalid,
    .next = ani_st_invalid,
    .event = ani_evt_invalid,
    .enter_tbl = ani_stm_enter_tbl,
    .exit_tbl = NULL,
    .refresh_tbl = NULL,
};

static inline void ani_stm_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < ani_evt_max) {
            stm_set_event(ani_stm_matrix, &ani_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static inline void ani_stm_init()
{
    ani_stm_ctx.current = ani_st_uninit;
    msg_t msg = { .msgid = ani_evt_init };
    ani_set_evt(&msg);
}

/**=================================
 *              ENTRY
 **=================================*/
/**
 * [IN] token: app token from launcher
 */
void ani_init(void* token)
{
    APP_CRIT("");
    app_token_t* app_tk = (app_token_t*)token;
    ani_ctx_t* ctx = (ani_ctx_t*)&ani_ctx;
    if (app_tk && ctx && !ctx->init) {
        ctx->init = 1;
        app_tk->accessor_array = ani_accessor_tbl;
        app_tk->accessor_num = sizeof(ani_accessor_tbl)/sizeof(ani_accessor_tbl[0]);
        ctx->launcher_q = app_tk->send_q;
        ctx->stm_msg = msg_token_new("animation", 10, sizeof(msg_t));
        ctx->task_terminate = osEventFlagsNew(NULL);
        osEventFlagsClear(ctx->task_terminate, 0x00ffffff); /* low 24bits valid */
        if (ctx->task_tk) {
            for (int i = 0; i < ANI_TASK_CNT; i++) {
                ani_task_token_t* task = &ctx->task_tk[i];
                if (task) {
                    task->running = 0;
                    task->thread_id = NULL;
                    task->terminate_sig = ctx->task_terminate;
                    task->issue = issue_token_new(task->issue_magic);
                    issue_token_init(task->issue, task->issue_num, task->issue_size, 1);
                    task->disp_mtx = task->support_disp ? osMutexNew(NULL) : NULL;
                    APP_CRIT("task:%s, issue:%p, %s", task->name, task->issue, task->issue_magic);
                }
            }
            issue_token_serialization(2, ctx->task_tk[ANI_DEC_TASK].issue, ctx->task_tk[ANI_PLAYER_TASK_0].issue);
        }
        ani_stm_init();
        for (int i = 0; ctx->property_id[i] != -1; i++) {
            APP_CRIT("observer %d", ctx->property_id[i]);
            system_property_observe(ctx->property_id[i], ani_property_changed, NULL);
        }
        res_loader_ready = osEventFlagsNew(NULL);
        osEventFlagsClear(res_loader_ready, 0x00ffffff);
    }
}

/**
 * [IN] token: app token from launcher
 */
void ani_entry(void* token)
{
    APP_CRIT("");
    if (!ani_ctx.stm_msg) return;
    while (1) {
        msg_t msg;
        if (!msg_token_recv(ani_ctx.stm_msg, &msg, 1000)) {
            APP_CRIT("recv:%d", msg.msgid);
            ani_stm_dispatch(&msg);
        }
    }
    osThreadExit();
}

void ani_notifier(int notify)
{
    APP_CRIT("notify:%d", notify);
    msg_t msg;
    if (notify == app_notify_start) {
        msg.msgid = ani_evt_playback;
        msg.param = ani_evt_param_openning;
        ani_set_evt(&msg);
    }
    else if (notify == app_notify_restart) {
        msg.msgid = ani_evt_playback;
        msg.param = ani_evt_param_replay;
        ani_set_evt(&msg);
    }
    else if (notify == app_notify_stop) {
        msg.msgid = ani_evt_stop;
        ani_set_evt(&msg);
    }
}

int ani_set_evt(msg_t* msg)
{
    int ret = msg_token_send(ani_ctx.stm_msg, (void*)msg, 1000);
    if (ret)
        APP_ERR("set evt:%d fail", msg->msgid);
    return ret;
}
/* EOF */
