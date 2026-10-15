/**
 * @file arbitrator.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv arbitrator library.
 *
 * Revision History:
 * -----------------
 */
#include <lib/list.h>
#include <cmsis_os2.h>
#include <disp_hal.h>
#include "arbitrator.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "arbitrator"

#define ARB_OPERATION(f, args...) \
        if (f) \
            f(args);

extern arb_ops_t arb_disp_ops;

arb_master_t arb_master[] = {
    { .magic = arb_magic_video, .ops = &arb_disp_ops },
};

static arb_ctx_t arb_ctx = {
    .init = 0,
    .num = sizeof(arb_master)/sizeof(arb_master[0]),
    .master = arb_master,
};

/**
 * [RET] 0-success
 */
int arb_init()
{
    arb_ctx_t* ctx = &arb_ctx;
    if (!ctx->init) {
        ctx->init = 1;
        ctx->accessor_cnt = 0;
        ctx->accessor_mtx = osMutexNew(NULL);
        ctx->ap_running = 0;
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->ops) {
                ARB_OPERATION(master->ops->p_init, master);
            }
        }
    }
    return ctx->init ? 0 : -1;
}

/**
 * [IN] magic: master magic
 * [IN] token: slave token
 * [IN] accessor: app's accessor
 * [RET] 0-success
 */
int arb_request_gain_slave(int magic, int token, arb_accessor_t* accessor)
{
    arb_ctx_t* ctx = &arb_ctx;
    APP_INFO("magic:%d, token:%d, init:%d", magic, token, ctx->init);
    if (ctx->init) {
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->magic == magic) {
                if (master->ops) {
                    APP_INFO("call request ops");
                    ARB_OPERATION(master->ops->p_request, master, token, accessor);
                }
                break;
            }
        }
    }
    return 0;
}

/**
 * [IN] magic: master magic
 * [IN] token: slave token
 * [IN] accessor: app's accessor
 * [RET] 0-success
 */
int arb_request_release_slave(int magic, int token, arb_accessor_t* accessor)
{
    APP_INFO("magic:%d, token:%d", magic, token);
    arb_ctx_t* ctx = &arb_ctx;
    if (ctx->init) {
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->magic == magic) {
                if (master->ops) {
                    ARB_OPERATION(master->ops->p_release, master, token, accessor);
                }
                break;
            }
        }
    }
    return 0;
}

/**
 * [IN] accessor: app's accessor
 * [RET] accessor's token or -1 as failure
 */
int arb_accessor_attach(arb_accessor_t* accessor)
{
    if (accessor) {
        osMutexAcquire(arb_ctx.accessor_mtx, 0);
        accessor->token = arb_ctx.accessor_cnt++;
        osMutexRelease(arb_ctx.accessor_mtx);
        return 0;
    }
    return -1;
}

/**
 * [IN] magic: master magic
 * [IN] token: slave token
 * [RET] accessor token, -1 is not gain
 */
int arb_query_slave_peak_accessor(int magic, int token)
{
    arb_ctx_t* ctx = &arb_ctx;
    int tk = -1;
    if (ctx->init) {
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->magic == magic) {
                if (master->ops) {
                    ARB_OPERATION(master->ops->p_query, master, token, &tk);
                }
                break;
            }
        }
    }
    return tk;
}

/**
 * [IN] magic: master magic
 * [IN] token: slave token
 * [RET] slave point or NULL
 */
void* arb_get_slave_handle(int magic, int token)
{
    arb_ctx_t* ctx = &arb_ctx;
    void* handle = NULL;
    if (ctx->init) {
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->magic == magic) {
                if (master->ops) {
                    ARB_OPERATION(master->ops->p_getHandle, master, token, &handle);
                }
                break;
            }
        }
    }
    return handle;
}

/**
 * [IN] magic: master magic
 * [IN] token: slave token
 * [RET] slave point or NULL
 */
int arb_preempt_change(int magic, int token, int preempt)
{
    arb_ctx_t* ctx = &arb_ctx;
    if (ctx->init) {
        for (int i = 0; i < ctx->num; i++) {
            arb_master_t* master = &ctx->master[i];
            if (master && master->magic == magic) {
                if (master->ops) {
                    ARB_OPERATION(master->ops->p_preemptChange, master, token, preempt);
                    ARB_OPERATION(master->ops->p_update, master, token);
                }
                break;
            }
        }
    }
    return 0;
}

int arb_is_ap_running()
{
    return arb_ctx.ap_running;
}

/* EOF */
