/**
 * @file app_loader.c
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
#include "launcher.h"
#ifdef CONFIG_BOOT_ANIMATION
#include "animation.h"
#endif
#ifdef CONFIG_FASTAVM
#include "fastavm.h"
#endif
#ifdef CONFIG_CLUSTER
#include "cluster.h"
#endif
#ifdef CONFIG_CONTROL_PANEL
#include "controlpanel.h"
#endif
#ifdef CONFIG_EMIRROR
#include "emirror.h"
#endif
#include "app_log.h"
    
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "launcher"

#define APP_TOKEN_ATTACH(i, n, pi, pe, pn) \
    { \
        .id = i, \
        .name = n, \
        .thread_id = NULL, \
        .thread_attr = { .name = n, .stack_size = 4096, .priority = osPriorityNormal }, \
        .send_q = NULL, \
        .accessor_num = 0, \
        .accessor_array = NULL, \
        .p_init = pi, \
        .p_entry = pe, \
        .p_notifier = pn, \
    }

app_token_t app_tbl[] = {
#ifdef CONFIG_BOOT_ANIMATION
    APP_TOKEN_ATTACH(app_id_ba,             "animation",    ani_init,           ani_entry,          ani_notifier),
#endif
#ifdef CONFIG_FASTAVM
    APP_TOKEN_ATTACH(app_id_fastavm,        "fastavm",      fastavm_init,       fastavm_entry,      fastavm_notifier),
#endif
#if CONFIG_CLUSTER
    APP_TOKEN_ATTACH(app_id_cluster,        "cluster",      cluster_init,       cluster_entry,      cluster_notifier),
#endif
#ifdef CONFIG_CONTROL_PANEL
    APP_TOKEN_ATTACH(app_id_controlpanel,   "controlpanel", cp_init,            cp_entry,           cp_notifier),
#endif
#ifdef CONFIG_EMIRROR
    APP_TOKEN_ATTACH(app_id_emirror,        "emirror",      emirror_init,       emirror_entry,      emirror_notifier),
#endif
};
#define SIZE_APP_TBL (sizeof(app_tbl)/sizeof(app_tbl[0]))

static inline void app_loader_schedule(app_token_t* app)
{
    if (app) {
        if (app->p_init) {
            app->p_init((void*)app);
        }
        for (int i = 0; i < app->accessor_num; i++) {
            arb_accessor_t* accessor = (arb_accessor_t*)&app->accessor_array[i];
            if (accessor) {
                list_initialize(&accessor->node);
                arb_accessor_attach(accessor);
                APP_INFO("app:%s, token:%d", app->name, accessor->token);
            }
        }
        if (app->p_entry) {
            app->thread_id = osThreadNew(app->p_entry, (void*)app, &app->thread_attr);
        }
    }
}

void app_loader_init(launcher_ctx_t* launcher)
{
    if (launcher) {
        for (int i = 0; i < SIZE_APP_TBL; i++) {
            app_token_t* app = &app_tbl[i];
            if (app) {
                if (launcher->msg_tk) app->send_q = launcher->msg_tk->msg_q;
                app_loader_schedule(app);
            }
        }
        launcher->apps = app_tbl;
        launcher->app_num = SIZE_APP_TBL;
    }
}

void app_loader_deinit(launcher_ctx_t* launcher)
{
    if (launcher) {
        for (int i = 0; i< SIZE_APP_TBL; i++) {
            app_token_t* app = &app_tbl[i];
            if (app && app->p_notifier) {
                app->p_notifier(app_notify_termiante);
            }
        }
    }
}
/* EOF */
