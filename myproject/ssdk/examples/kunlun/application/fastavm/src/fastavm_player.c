/**
 * @file fastavm_player.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "fastavm"

#include <disp_data_type.h>
#include <sdm_display.h>
#include <param.h>
#include "fastavm_cfg.h"
#include "fastavm_def.h"
#include "fastavm.h"
#include "disp_utility.h"
#include "app_log.h"

#define FASTAVM_SDMBUF_TEMPLATE { \
        .layer = 0, \
        .layer_dirty = 0, \
        .layer_en = 1, \
        .fmt = COLOR_RGB888, \
        .src = { 0, 0, 0, 0 }, \
        .addr = { 0, 0, 0, 0 }, \
        .src_stride = { 0, 0, 0, 0 }, \
        .start = { 0, 0, 0, 0 }, \
        .dst = { 0, 0, 0, 0 }, \
        .ckey_en = 0, \
        .ckey = 0, \
        .alpha_en = 1, \
        .alpha = 0xff, \
        .z_order = 0, \
        .security = 0 \
        }

static uint32_t disp_w;
static uint32_t disp_h;

static inline int fastavm_player_init(fastavm_task_token_t* task)
{
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, INFOTAINMENT);
    if (disp) {
        utils_get_sdm_disp_rect(disp, &disp_w, &disp_h);
    }
    return 0;
}

static inline void fastavm_player_entry(fastavm_task_token_t* task)
{
    APP_INFO("");
    sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, INFOTAINMENT);
    while (task && task->running) {
        container_t* container = issue_token_receive(task->issue, 10);
        if (container) {
            int disp_sts = task->disp_sts;
            fastavm_vstream_t vstream;
            container_carry_down(container, &vstream);
            if (task->disp_sts&FASTAVM_MASK(INFOTAINMENT)) {
                if (disp && disp->handle) {
                    struct sdm_buffer sdmbuf = FASTAVM_SDMBUF_TEMPLATE;
                    struct sdm_post_config pcfg = { .bufs = &sdmbuf, .n_bufs = 1, .custom_data = NULL, .custom_data_size = 0 };
                    utils_fill_sdm_disp_buff(&sdmbuf, vstream.yuyv_addr, vstream.yuyv_stride);
                    utils_crop_sdm_disp_buff(&sdmbuf, 0, 0, vstream.disp_w,  vstream.disp_h);
                    utils_map_sdm_disp_buff(&sdmbuf, 0, 0, vstream.disp_w,  vstream.disp_h);
                    sdm_post(disp->handle, &pcfg);
#if 0
                    static int once = 0;
                    if (!once) {
                        once = 1;
                        APP_LOG("1st frame, %d", current_time());
                    }
#endif
                }
            }
            issue_token_dereference(task->issue, container);
        }
    }
    if (task && task->disp_sts&FASTAVM_MASK(INFOTAINMENT)) {
        if (disp && disp->handle) {
            sdm_clear_display(disp->handle);
        }
    }
}

static inline void fastavm_player_exit(fastavm_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        osEventFlagsSet(task->terminate_sig, FASTAVM_MASK(task->id));
        msg_t msg = { .msgid = fastavm_evt_stop, .param = task->id };
        fastavm_set_evt(&msg);
    }
    osThreadExit();
}

void fastavm_player_task(void* args)
{
    if (args) {
        fastavm_task_token_t* task = (fastavm_task_token_t*)args;
        if (!fastavm_player_init(task)) {
            fastavm_player_entry(task);
        }
        fastavm_player_exit(task);
    }
}
/* EOF */
