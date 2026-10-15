/**
 * @file ani_decoder.c
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
#include <res_loader.h>
#include <driver/kunlun/vpu/sdrv_jpu.h>
#include "ani_def.h"
#include "ani_cfg.h"
#include "launcher.h"
#include "animation.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "animation"

/* 0:decode, 1:encode */
const int32_t dec_type = 0;
const int32_t enc_type = 1;

static uint32_t dec_chan[ANI_RES_CNT] = { -1 };
static ani_mjpeg_t mjpeg[ANI_RES_CNT];
static bool islooping = false;

extern uint32_t ani_res_loader_wait_ready(uint32_t timeout);
extern void ani_res_loader_set_path(const char* path);
extern int ani_res_loader_get_buffer(int offset, char* output, int len);

static void ani_decoder_fill_buffer(void *private_ptr, int expect_size, void *input_buffer, int *fill_input_size)
{
    if (private_ptr && input_buffer && fill_input_size) {
        ani_mjpeg_t* mjpeg = (ani_mjpeg_t*)private_ptr;
        if (mjpeg->offset >= mjpeg->size) {
            *fill_input_size = 0;
        }
        else {
            int ret = ani_res_loader_get_buffer(mjpeg->offset, input_buffer, expect_size);
            mjpeg->offset += ret;
            *fill_input_size = ret;
        }
    }
}

static inline int ani_decoder_res_init(int res, bool isloop)
{
    if (res >= 0 && res < ANI_RES_CNT) {
reload:
        mjpeg[res].path = !isloop?ANI_RES_PATH_BA:ANI_RES_PATH_BA_LOOP;
        mjpeg[res].size = res_size(mjpeg[res].path);
        mjpeg[res].offset = 0;
        ani_res_loader_set_path(mjpeg[res].path);
        if (isloop && (int)mjpeg[res].size <= 0) {
            APP_WARN("no logo state file");
            isloop = false;
            goto reload;
        }
        return 0;
    }
    return -1;
}

static inline int ani_decoder_res_deinit(int res)
{
    if (res >= 0 && res < ANI_RES_CNT) {
        //mjpeg[res].size = 0;
        mjpeg[res].offset = 0;
    }
    return 0;
}

static inline int ani_decoder_jpu_init(int res)
{
    if (res >= 0 && res < ANI_RES_CNT) {
        decoder_opt_t opt;
        opt.dec_feeding_method = FEED_METHOD_FIXED_SIZE;
        opt.fill_buffer_callback = ani_decoder_fill_buffer;
        opt.output_cbcrInterleave = CBCR_SEPARATED;
        opt.output_packed_format = PACKED_FORMAT_NONE;
        opt.output_format = FORMAT_420;
        opt.user_private_ptr = &mjpeg[res];
        dec_chan[res] = vpu_acquire_chan(jpu_dev, dec_type, (void*)&opt);
        return dec_chan[res] < 0 ? -1 : 0;
    }
    return -1;
}

static inline int ani_decoder_jpu_deinit(int res)
{
    if (res >= 0 && res < ANI_RES_CNT) {
        vpu_release_chan(jpu_dev, dec_chan[res]);
        dec_chan[res] = -1;
    }
    return 0;
}

static inline int ani_decoder_init(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task) {
        islooping = false;
        if (!ani_res_loader_wait_ready(3000)) goto err;
        if (ani_decoder_res_init(task->support_res, false)) goto err;
        if (ani_decoder_jpu_init(task->support_res)) goto err_jpu;
        return 0;
err_jpu:
        ani_decoder_jpu_deinit(task->support_res);
    }

err:
    return -1;
}

static inline void ani_decoder_entry(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task && task->support_res >= 0 && task->support_res < ANI_RES_CNT) {
        while (task->running) {
            decoder_output_t output;
            int32_t ret = vpu_chan_codec_sync(jpu_dev, dec_chan[task->support_res]);
            APP_DBG("decode one frame, ret:%d", ret);
            if (ret == VPU_CODEC_RET_BITSTREAM_DONE) {
                /* playback done */
#ifdef CONFIG_LOOP_LOGO
                if (!islooping) {
                    output.is_valid = true;
                    output.y_ptr = NULL;
                    output.u_ptr = NULL;
                    output.v_ptr = NULL;
                    ret = issue_token_send_sync(task->issue, (void*)&output, &task->running);
                }
                islooping = true;
                ani_decoder_res_deinit(task->support_res);
                ani_decoder_res_init(task->support_res, true);
                ret = vpu_device_ctrl(jpu_dev, dec_chan[task->support_res], VPU_CODEC_CTLCMD_DEC_RESET, NULL);
#else
                output.is_valid = false;
                ret = issue_token_send_async(task->issue, (void*)&output, 1000);
                APP_CRIT("playback done:%d", ret);
                break;
#endif
            }
            else if (ret == VPU_CODEC_RET_ERROR) {
                /* need re-acquire channel */
                // TODO:
            }
            else if (ret == VPU_CODEC_RET_FRAME_DONE) {
                /* playback one frame */
                ret = vpu_device_ctrl(jpu_dev, dec_chan[task->support_res], VPU_CODEC_CTLCMD_GET_DECOUT_DATA, &output);
                if (ret || !output.is_valid) {
                    continue;
                }
                if (!task->issue) {
                    continue;
                }
                ret = issue_token_send_sync(task->issue, (void*)&output, &task->running);
                APP_DBG("send sync ret:%d", ret);
            }
        }
    }
}

static inline void ani_decoder_exit(ani_task_token_t* task)
{
    APP_CRIT("");
    if (task) {
        islooping = false;
        ani_decoder_jpu_deinit(task->support_res);
        ani_decoder_res_deinit(task->support_res);
        osEventFlagsSet(task->terminate_sig, ANI_MASK(task->id));
        msg_t msg = { .msgid = ani_evt_elapsed, .param = task->id };
        ani_set_evt(&msg);
    }
    osThreadExit();
}

void ani_decoder_task(void* args)
{
    APP_CRIT("");
    if (args) {
        ani_task_token_t* task = (ani_task_token_t*)args;
        if (!ani_decoder_init(task)) {
            ani_decoder_entry(task);
        }
        ani_decoder_exit(task);
    }
}
/* EOF */
