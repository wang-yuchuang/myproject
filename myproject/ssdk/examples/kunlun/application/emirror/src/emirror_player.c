/**
 * @file emirror_player.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#include <disp_data_type.h>
#include <sdm_display.h>
#include <param.h>
#include <res_loader.h>
#include <malloc.h>
#include "emirror_cfg.h"
#include "emirror_def.h"
#include "emirror.h"
#include "disp_utility.h"
#include "app_log.h"
        
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "emirror"

#ifdef CONFIG_MP
#define SD_LOGO_PATH  "early_app/fastrvm_mp/sdv_logo.rgb"
#else
#define SD_LOGO_PATH  "early_app/fastrvm/sdv_logo.rgb"
#endif

#define EMIRROR_SDMBUF_TEMPLATE { \
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

#define EMIRROR_SDMBUF_LOGO { \
        .layer = 1, \
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
        .alpha_en = 0, \
        .alpha = 0xff, \
        .z_order = 1, \
        .security = 0 \
        }

static uint8_t* sd_logo_buff = NULL;
static bool post_logo = false;

static int emirror_player_post_logo(sdm_display_t* disp)
{
    APP_INFO("");
    if (disp) {
        struct sdm_buffer sdmbuf = EMIRROR_SDMBUF_LOGO;
        struct sdm_post_config pcfg = { .bufs = &sdmbuf, .n_bufs = 1, .custom_data = NULL, .custom_data_size = 0 };
        utils_fill_sdm_disp_buff(&sdmbuf, (unsigned long)sd_logo_buff, 640*3);
        utils_crop_sdm_disp_buff(&sdmbuf, 0, 0, 640, 360);
        utils_map_sdm_disp_buff(&sdmbuf, 1280, 360, 640, 360);
        int ret = sdm_post(disp->handle, &pcfg);
        APP_DBG("sdm_post %s", !ret ? "success" : "fail");
    }
    return 0;
}

static int emirror_player_init(emirror_task_token_t* task)
{
    int size = res_size(SD_LOGO_PATH);
    post_logo = false;
    sd_logo_buff = memalign(0x1000, size);
    if (sd_logo_buff) {
        res_load(SD_LOGO_PATH, sd_logo_buff, size, 0);
    }
    return 0;
}

static void emirror_player_entry(emirror_task_token_t* task)
{
    APP_INFO("");
    if (!task)
        return;

    sdm_display_t* disp = NULL;
    while (task->running) {
        container_t* container = issue_token_receive(task->issue, 10);
        if (container) {
            int disp_sts = task->disp_sts;
            emirror_vstream_t vstream;
            container_carry_down(container, &vstream);
            disp = vstream.disp_handle;
            if (disp && disp->handle) {
                if (task->disp_sts&EMIRROR_MASK(disp->id)) {
                    if (!post_logo && RIGHT_MIRROR == disp->id) {
                        post_logo = true;
                        emirror_player_post_logo(disp);
                    }
                    struct sdm_buffer sdmbuf = EMIRROR_SDMBUF_TEMPLATE;
                    struct sdm_post_config pcfg = { .bufs = &sdmbuf, .n_bufs = 1, .custom_data = NULL, .custom_data_size = 0 };
                    utils_fill_sdm_disp_buff(&sdmbuf, vstream.addr, vstream.stride);
                    utils_crop_sdm_disp_buff(&sdmbuf, 0, 0, vstream.cam_w, vstream.cam_h);
                    utils_map_sdm_disp_buff(&sdmbuf, 0, 0, vstream.cam_w, vstream.cam_h);
                    int ret = sdm_post(disp->handle, &pcfg);
                    APP_DBG("sdm_post %s", !ret ? "success" : "fail");
                }
            }
            issue_token_dereference(task->issue, container);
        }
    }
    if (disp && disp->handle) {
        if (task && task->disp_sts&EMIRROR_MASK(disp->id)) {
            sdm_clear_display(disp->handle);
        }
    }
}

static void emirror_player_exit(emirror_task_token_t* task)
{
    APP_INFO("");
    if (task) {
        if (sd_logo_buff) {
            free(sd_logo_buff);
            sd_logo_buff = NULL;
        }
        osEventFlagsSet(task->terminate_sig, EMIRROR_MASK(task->id));
        msg_t msg = { .msgid = emirror_evt_stop, .param = task->id };
        emirror_set_evt(&msg);
    }
    osThreadExit();
}

void emirror_player_task(void* args)
{
    APP_INFO("");
    if (args) {
        emirror_task_token_t* task = (emirror_task_token_t*)args;
        if (!emirror_player_init(task)) {
            emirror_player_entry(task);
        }
        emirror_player_exit(task);
    }
}
/* EOF */
