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
#include <string.h>
#include <lib/list.h>
#include <sd_property.h>
#include <disp_hal.h>
#include <debug.h>
#include <CLI_type.h>
#include "launcher.h"
#include "state_machine.h"
#include "container.h"
#include "app_loader.h"
#include "app_log.h"
#include <image_cfg.h>
#include <arch/cache.h>
#include <reg.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "launcher"

static launcher_ctx_t launcher = { .init = 0 };

#define LAUNCH_NOTIFY_APP(msg, match, app, evt) \
        if ((msg) && (msg)->msgid == (match)) { \
            if ((app) && (app)->p_notifier) { \
                APP_INFO("app name:%s, %d", app->name, evt); \
                (app)->p_notifier(evt); \
            } \
        }

typedef enum _launch_st_em {
    launch_st_invalid = -1,
    launch_st_init = 0,
    launch_st_stby,
    launch_st_run,
    launch_st_remote,
    launch_st_max
} launch_st_em;

typedef enum _launch_evt_em {
    launch_evt_invalid = -1,
    launch_evt_init = 0,
    launch_evt_pwr_on,
    launch_evt_pwr_off,
#ifdef CONFIG_APP_LAUNCHER_DEBUG
    launch_evt_test_start_ba,
    launch_evt_test_stop_ba,
    launch_evt_test_start_avm,
    launch_evt_test_stop_avm,
    launch_evt_test_start_cls,
    launch_evt_test_stop_cls,
    launch_evt_test_start_cp,
    launch_evt_test_stop_cp,
    launch_evt_test_start_em,
    launch_evt_test_stop_em,
#endif
    launch_evt_remote_on,
    launch_evt_remote_off,
    launch_evt_max
} launch_evt_em;

static void launcher_stm_stby_enter(void* param)
{
    APP_INFO("");
    arb_init();

    /* enable display */
    arb_preempt_change(arb_magic_video, CLUSTER, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, INFOTAINMENT, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, ENTERTAINMENT, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, CONTROLPANEL, arb_preempt_always);

    /* enable apps except boot animation */
    // TODO:
}

#define CACHE_LINE 32
bool is_str(void)
{
    uint32_t v;
    uint32_t base = 0;

    base = SAF_AP1_MEMBASE + 0x100000;

    arch_invalidate_cache_range(base, CACHE_LINE);
    v = readl(base);

    if ((v & 0xffff) == 0x8765)
        return true;

    return false;
}

static void launcher_stm_run_enter(void* param)
{
    APP_INFO("");
    msg_t* msg = (msg_t*)param;
    if (!msg) return;
    for (int i = 0; i < launcher.app_num; i++) {
        app_token_t* app = &launcher.apps[i];
        if (app) {
            switch (app->id) {
            case app_id_ba:
                {
                    #ifdef CONFIG_APP_LAUNCHER_DEBUG
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_start_ba, app, app_notify_restart);
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_stop_ba, app, app_notify_stop);
                    }
                    #endif
                    #ifdef CONFIG_AUTO_START_BOOT_ANIMATION
                    {
                        if (!is_str()) {
                            LAUNCH_NOTIFY_APP(msg, launch_evt_pwr_on, app, app_notify_start);
                        }
                    }
                    #endif
                    break;
                }
            case app_id_fastavm:
                {
                    #ifdef CONFIG_APP_LAUNCHER_DEBUG
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_start_avm, app, app_notify_start);
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_stop_avm, app, app_notify_stop);
                    }
                    #endif
                    #ifdef CONFIG_AUTO_START_FASTAVM
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_pwr_on, app, app_notify_start);
                    }
                    #endif
                    break;
                }
            case app_id_cluster:
                {
                    #ifdef CONFIG_APP_LAUNCHER_DEBUG
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_start_cls, app, app_notify_start);
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_stop_cls, app, app_notify_stop);
                    }
                    #endif
                    #ifdef CONFIG_AUTO_START_CLUSTER
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_pwr_on, app, app_notify_start);
                    }
                    #endif
                    break;
                }
            case app_id_controlpanel:
                {
                    #ifdef CONFIG_APP_LAUNCHER_DEBUG
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_start_cp, app, app_notify_start);
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_stop_cp, app, app_notify_stop);
                    }
                    #endif
                    #ifdef CONFIG_AUTO_START_CONTROL_PANEL
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_pwr_on, app, app_notify_start);
                    }
                    #endif
                    break;
                }
            case app_id_emirror:
                {
                    #ifdef CONFIG_APP_LAUNCHER_DEBUG
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_start_em, app, app_notify_start);
                        LAUNCH_NOTIFY_APP(msg, launch_evt_test_stop_em, app, app_notify_stop);
                    }
                    #endif
                    #ifdef CONFIG_AUTO_START_EMIRROR
                    {
                        LAUNCH_NOTIFY_APP(msg, launch_evt_pwr_on, app, app_notify_start);
                    }
                    #endif
                    break;
                }
            default:
                break;
            }
        }
    }
}

static void launcher_stm_remote_enter(void* param)
{
    APP_INFO("");
    /* disable display */
    arb_preempt_change(arb_magic_video, CLUSTER, arb_preempt_never);
    arb_preempt_change(arb_magic_video, INFOTAINMENT, arb_preempt_never);
    arb_preempt_change(arb_magic_video, ENTERTAINMENT, arb_preempt_never);
    arb_preempt_change(arb_magic_video, CONTROLPANEL, arb_preempt_never);
}

const static stm_func_t launcher_stm_enter_tbl[] = {
    { launch_st_stby,    launcher_stm_stby_enter },
    { launch_st_run,     launcher_stm_run_enter },
    { launch_st_remote,  launcher_stm_remote_enter },
    { -1 }
};

static void launcher_stm_run_exit(void* param)
{
    APP_INFO("");
    // stop boot animation
    for (int i = 0; i < launcher.app_num; i++) {
        app_token_t* app = &launcher.apps[i];
        if (app) {
            if (app->id == app_id_ba) {
                app->p_notifier(app_notify_stop);
                break;
            }
        }
    }
}

static void launcher_stm_remote_exit(void* param)
{
    APP_INFO("");
    /* enable display */
    arb_preempt_change(arb_magic_video, CLUSTER, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, INFOTAINMENT, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, ENTERTAINMENT, arb_preempt_bfap);
    arb_preempt_change(arb_magic_video, CONTROLPANEL, arb_preempt_always);
}

const static stm_func_t launcher_stm_exit_tbl[] = {
    { launch_st_run,     launcher_stm_run_exit },
    { launch_st_remote,  launcher_stm_remote_exit },
    { -1 }
};

const static stm_t launcher_stm_matrix[] = {
    { launch_st_init,    launch_evt_init,         launch_st_stby },

    { launch_st_stby,    launch_evt_pwr_on,       launch_st_run },
    { launch_st_stby,    launch_evt_remote_on,    launch_st_remote },

    { launch_st_run,     launch_evt_pwr_off,      launch_st_stby },
#ifdef CONFIG_APP_LAUNCHER_DEBUG
    { launch_st_run,     launch_evt_test_start_ba, launch_st_run },
    { launch_st_run,     launch_evt_test_stop_ba,  launch_st_run },
    { launch_st_run,     launch_evt_test_start_avm,launch_st_run },
    { launch_st_run,     launch_evt_test_stop_avm, launch_st_run },
    { launch_st_run,     launch_evt_test_start_cls,launch_st_run },
    { launch_st_run,     launch_evt_test_stop_cls, launch_st_run },
    { launch_st_run,     launch_evt_test_start_cp, launch_st_run },
    { launch_st_run,     launch_evt_test_stop_cp,  launch_st_run },
    { launch_st_run,     launch_evt_test_start_em, launch_st_run },
    { launch_st_run,     launch_evt_test_stop_em,  launch_st_run },
#endif

    { launch_st_remote,  launch_evt_pwr_on,       launch_st_run },
    { launch_st_remote,  launch_evt_remote_off,   launch_st_stby },

    { -1 }
};

static stm_ctx_t launcher_stm_ctx = {
    .current = launch_st_init,
    .previous = launch_st_invalid,
    .next = launch_st_invalid,
    .event = launch_evt_invalid,
    .enter_tbl = launcher_stm_enter_tbl,
    .exit_tbl = launcher_stm_exit_tbl,
    .refresh_tbl = NULL,
};

static inline void launcher_msg_dispatch(msg_t* msg)
{
    if (msg) {
        if (msg->msgid < launch_evt_max) {
            stm_set_event(launcher_stm_matrix, &launcher_stm_ctx, msg->msgid, (void*)msg);
        }
    }
}

static int launcher_msg_notifier(msg_t* msg)
{
    if (msg) {
        return msg_token_send(launcher.msg_tk, msg, 1000);
    }
    return -1;
}

static inline int launcher_init(launcher_ctx_t* ctx)
{
    if (ctx) {
        if (!ctx->init) {
            ctx->init = 1;
            ctx->exit = 0;
            ctx->msg_tk = msg_token_new("launcher", 10, sizeof(msg_t));
            app_loader_init(ctx);
            msg_t msg = { .msgid = launch_evt_init, .param = 0 };
            launcher_msg_notifier(&msg);
#ifdef CONFIG_APP_LAUNCHER_DEBUG
            msg.msgid = launch_evt_pwr_on;
            launcher_msg_notifier(&msg);
#endif
            // TODO:
            // register external trigger callback...
        }
        return ctx->init ? 0 : -1;
    }
    return -1;
}

static inline void launcher_exit(launcher_ctx_t* ctx)
{
    if (ctx) {
        if (ctx->init) {
            ctx->init = 0;
            ctx->exit = 1;
            msg_token_delete(&ctx->msg_tk);
            app_loader_deinit(ctx);
        }
    }
    osThreadExit();
}

static void launcher_entry(void* args)
{
    int ret = launcher_init(&launcher);
    while (!ret && !launcher.exit) {
        msg_t msg;
        if (!msg_token_recv(launcher.msg_tk, &msg, 1000)) {
            APP_INFO("recv:%d", msg.msgid);
            launcher_msg_dispatch(&msg);
        }
    }
    launcher_exit(&launcher);
}

static osThreadId_t launcher_thread = NULL;
void launcher_app()
{
    if (!launcher_thread) {
        launcher_thread = osThreadNew(launcher_entry, NULL, NULL);
    }
}

#ifdef CONFIG_APP_LAUNCHER_DEBUG
static int launcher_test(int argc, char* argv[])
{
    if (!launcher_thread) {
        launcher_app();
    }
    else if (argc > 0) {
        msg_t msg;
        if (0 == strcmp(argv[0], "-ani")) {
            if (argc > 1 && 0 == strcmp(argv[1], "stop")) msg.msgid = launch_evt_test_stop_ba;
            else msg.msgid = launch_evt_test_start_ba;
        }
        else if (0 == strcmp(argv[0], "-avm")) {
            if (argc > 1 && 0 == strcmp(argv[1], "stop")) msg.msgid = launch_evt_test_stop_avm;
            else msg.msgid = launch_evt_test_start_avm;
        }
        else if (0 == strcmp(argv[0], "-cls")) {
            if (argc > 1 && 0 == strcmp(argv[1], "stop")) msg.msgid = launch_evt_test_stop_cls;
            else msg.msgid = launch_evt_test_start_cls;
        }
        else if (0 == strcmp(argv[0], "-cp")) {
            if (argc > 1 && 0 == strcmp(argv[1], "stop")) msg.msgid = launch_evt_test_stop_cp;
            else msg.msgid = launch_evt_test_start_cp;
        }
        else if (0 == strcmp(argv[0], "-em")) {
            if (argc > 1 && 0 == strcmp(argv[1], "stop")) msg.msgid = launch_evt_test_stop_em;
            else msg.msgid = launch_evt_test_start_em;
        }
        else {
            goto exit;
        }
        APP_INFO("msg:%d", msg.msgid);
        launcher_msg_notifier(&msg);
    }

exit:
    return 0;
}

CLI_CMD("apptest", "start early app launcher", launcher_test);
#endif /* CONFIG_APP_LAUNCHER_DEBUG */
/* EOF */
