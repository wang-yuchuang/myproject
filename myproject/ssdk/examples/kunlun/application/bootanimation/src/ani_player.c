/**
 * @file ani_player.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */
#include <vpu_cfg.h>
#include <disp_hal.h>
#include <disp_data_type.h>
#include <sdm_display.h>
#include "ani_def.h"
#include "ani_cfg.h"
#include "launcher.h"
#include "animation.h"
#include "disp_utility.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "animation"

extern int ani_disp_auto_elapse;

static ani_frame_ele_t ani_frame[] = {
    { CLUSTER,      0, {0, 0, 800, 480},    {0, 0, 800, 480} },  /* BMU05 single screen 800x480 */
    { -1 }
};

#define ANI_SDMBUF_TEMPLATE { \
    .layer = 1, \
    .layer_dirty = 0, \
    .layer_en = 1, \
    .fmt = COLOR_YUV420P, \
    .src = { 0, 0, 0, 0 }, \
    .addr = { 0, 0, 0, 0 }, \
    .src_stride = { 0, 0, 0, 0 }, \
    .start = { 0, 0, 0, 0 }, \
    .dst = { 0, 0, 0, 0 }, \
    .ckey_en = 0, \
    .ckey = 0, \
    .alpha_en = 1, \
    .alpha = 0xff, \
    .z_order = 1, \
    .security = 0 \
    }

static int ani_player_frame_process(sdm_display_t* disp, int disp_id, int res_id, decoder_output_t* dec_data)
{
    if (disp && disp->handle && dec_data && res_id < ANI_RES_CNT) {
        struct sdm_buffer sdmbuf = ANI_SDMBUF_TEMPLATE;
        struct sdm_post_config pcfg = { .bufs = &sdmbuf, .n_bufs = 1, .custom_data = NULL, .custom_data_size = 0 };
        utils_fill_sdm_disp_buff_yuv(&sdmbuf, dec_data->y_ptr, dec_data->u_ptr, dec_data->v_ptr, dec_data->output_width);
        for (int i = 0; ani_frame[i].disp_id != -1; i++) {
            if (disp_id == ani_frame[i].disp_id) {
                utils_crop_sdm_disp_buff(&sdmbuf, ani_frame[i].src_rect.x, ani_frame[i].src_rect.y, ani_frame[i].src_rect.w, ani_frame[i].src_rect.h);
                utils_map_sdm_disp_buff(&sdmbuf, ani_frame[i].dest_rect.x, ani_frame[i].dest_rect.y, ani_frame[i].dest_rect.w, ani_frame[i].dest_rect.h);
                int ret = sdm_post(disp->handle, &pcfg);
                APP_DBG("sdm_post %s", ret ? "fail" : "success");
                return ret;
            }
        }
    }
    return -1;
}

static int ani_player_clear_disp(int disp_sts)
{
    if (disp_sts) {
        msg_t msg = { .msgid = ani_evt_hide, .param = disp_sts };
        ani_set_evt(&msg);
    }
    return 0;
}

static void ani_player_fps_callback (void *argument)
{
    if (argument) {
        osEventFlagsId_t sig = (osEventFlagsId_t)argument;
        osEventFlagsSet(sig, 1);
    }
}

static int ani_player_init(ani_task_token_t* task)
{
    return 0;
}

static void ani_player_entry(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task && task->issue) {
        int wait_fps = 0;
        osEventFlagsId_t fps_sig = osEventFlagsNew(NULL);
        osTimerId_t fps_timer = osTimerNew(ani_player_fps_callback, osTimerPeriodic, (void*)fps_sig, NULL);
        if (fps_timer && fps_sig) {
            osStatus_t ret = osEventFlagsClear(fps_sig, 1);
            if (ret == osOK) {
                ret = osTimerStart(fps_timer, 1000/ANI_FPS/ANI_RES_CNT);
                wait_fps = (ret == osOK) ? 1 : 0;
            }
        }
        int disp_sts = 0;
        int quit = 0;
        while (task->running) {
            container_t* container = issue_token_receive(task->issue, 10);
            if (container) {
                osStatus_t ret = ANI_DISP_BUSY(task);
                disp_sts = task->disp_sts;
                decoder_output_t output;
                container_carry_down(container, &output);
                if (output.is_valid) {
                    APP_DBG("recv one frame, disp_sts:%x", disp_sts);
                    if (output.y_ptr && output.u_ptr && output.v_ptr && disp_sts) {
                        for (int i = 0; disp_sts; disp_sts>>=1, i++) {
                            if (disp_sts&1) {
                                sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, i);
                                //APP_DBG("play one frame in no.%d disp, disp:%p", i, disp);
                                if (disp) {
                                    ani_player_frame_process(disp, i, task->support_res, &output);
                                }
                            }
                        }
                    }
#ifdef CONFIG_LOOP_LOGO
                    else {
                        /* looping logo */
                        ani_player_clear_disp(ani_disp_auto_elapse);
                        APP_CRIT("looping logo, disp_sts:0x%02x", disp_sts);
                        if (disp_sts == 0) {
                            /* terminate */
                            msg_t msg = { .msgid = ani_evt_stop };
                            ani_set_evt(&msg);
                        }
                    }
#endif
                    ANI_DISP_IDLE(task, ret);
                    issue_token_dereference(task->issue, container);
                    if (wait_fps) {
                        uint32_t sig = osEventFlagsWait(fps_sig, 1, osFlagsWaitAny, 1000/ANI_FPS/ANI_RES_CNT);
                        if (sig&1) {
                            osEventFlagsClear(fps_sig, 1);
                        }
                    }
                }
                else {
                    APP_CRIT("recv playbck done");
                    ANI_DISP_IDLE(task, ret);
                    issue_token_dereference(task->issue, container);
                    quit = 1;

                }
                if (quit) break;
            }
        }
        osStatus_t ret = ANI_DISP_BUSY(task);
        ani_player_clear_disp(task->disp_sts);
        ANI_DISP_IDLE(task, ret);
        if (fps_timer) {
            osTimerDelete(fps_timer);
            fps_timer = NULL;
        }
    }
}

static void ani_player_exit(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task) {
        osEventFlagsSet(task->terminate_sig, ANI_MASK(task->id));
        msg_t msg = { .msgid = ani_evt_elapsed, .param = task->id };
        ani_set_evt(&msg);
    }
    osThreadExit();
}

void ani_player_task(void* args)
{
    APP_CRIT("");
    if (args) {
        ani_task_token_t* task = (ani_task_token_t*)args;
        if (!ani_player_init(task)) {
            ani_player_entry(task);
        }
        ani_player_exit(task);
    }
}

//#define ANI_FRAME_TESING
#ifdef ANI_FRAME_TESING
#include <CLI_type.h>

static int ani_player_test(int argc, char* argv[])
{
    if (argc > 1) {
        void* addr = (void *)strtoul(argv[0], NULL, 0);
        int disp_id = atoi(argv[1]);
        if (addr && disp_id < SCREEN_MAX) {
            sdm_display_t* disp = arb_get_slave_handle(arb_magic_video, disp_id);
            ani_player_frame_process(disp, disp_id, 0, (decoder_output_t*)addr);
        }
    }
    return 0;
}

CLI_CMD("aniframe", "animation frame player", ani_player_test);
#endif /* ANI_FRAME_TESING */
/* EOF */
