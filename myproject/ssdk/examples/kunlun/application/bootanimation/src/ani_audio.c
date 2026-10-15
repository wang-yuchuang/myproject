/**
 * @file ani_audio.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */
#include <string.h>
#include "ani_cfg.h"
#include "ani_def.h"
#include "launcher.h"
#include "animation.h"
#include "app_log.h"
#include <am_api.h>
#include <malloc.h>
#include <FreeRTOS.h>
#include <heap_ext.h>
#include <res_loader.h>
#include <audio_cfg.h>
#include "audio_res.h"
#include "driver/snd_driver.h"
#include "param.h"
#ifdef CONFIG_AUDIO_HUB_SVC
#include <ahub_svc.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "animation"

#ifndef CONFIG_AUDIO_HUB_SVC
#define ANI_SND_XFER_COMPLETE (0x1u)
#define ANI_CHNNELS_COUNT 2
#define ANI_DMA_PERIOD_FRAME_COUNT 1024
#define ANI_DMA_PERIOD_SIZE ANI_DMA_PERIOD_FRAME_COUNT *ANI_CHNNELS_COUNT * 2
#define ANI_DMA_PERIOD_COUNT 2
#define ANI_DMA_BUF_TOTAL_SIZE ANI_DMA_PERIOD_SIZE *ANI_DMA_PERIOD_COUNT

typedef struct ani_audio_player {
    snd_dev_t *dev;
    osEventFlagsId_t event;
    osThreadId_t thread;
    bool running;
    uint8_t *pcm_data;
    int32_t pcm_size;
    int32_t pcm_ptr;
    snd_params_t *params;
} ani_audio_player_t;

extern snd_dev_t *i2s_sc1_dev;
static ani_audio_player_t app_player;

static snd_params_t pcm_param = {
    .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
    .channels = SND_CHANNEL_NUM_STEREO,
    .sample_width = SND_SAMPLE_WIDTH_16BITS,
    .sample_rate = SND_SR_48000,
    .period_size = ANI_DMA_PERIOD_SIZE,
    .period_count = ANI_DMA_PERIOD_COUNT,
};

static int load_pcm_from_path(const char *path, void **buf, int *size)
{
    int ret = 0;
    *size = ROUNDUP(res_size(path), 32);
    *buf = ExtMallocAligned(*size, 32);

    if (!(*buf)) {
        APP_ERR("Create audio pcm fail.\n");
        return -1;
    }

    ret = res_load(path, *buf, *size, 0);
    if (ret < 0) {
        APP_ERR("can't load audio res!\n");
        ExtFree(*buf);
        buf = NULL;
        return ret;
    }

    APP_INFO("size 0x%x %p.\n", *size, *buf);
    return ret;
}

static void player_callback(void *context, snd_state_t state)
{
    ani_audio_player_t *player = (ani_audio_player_t *)context;

    if (player->event) {
        osEventFlagsSet(player->event, ANI_SND_XFER_COMPLETE);
    }
}

static void ani_audio_playback(ani_task_token_t *task)
{
    ani_audio_player_t *player = (ani_audio_player_t *)&app_player;
    snd_params_t *params = player->params;
    snd_dev_t *dev = player->dev;
    int len;
    int32_t temp = 0;

    while (player->running) {
        osEventFlagsWait(player->event, ANI_SND_XFER_COMPLETE, osFlagsWaitAll,
                         osWaitForever);

        temp = player->pcm_size - player->pcm_ptr;

        if (temp > params->period_size) {
            len = snd_write(dev, player->pcm_data + player->pcm_ptr,
                            params->period_size);
            player->pcm_ptr += len;
        } else {
            if (temp == 0) { /* data transfer over */
                player->running = false;
                player->pcm_ptr = 0;
            } else {
                len = snd_write(dev, player->pcm_data + player->pcm_ptr, temp);
                player->pcm_ptr += len;
            }
        }
    }

    snd_stop(dev, SND_STREAM_PLAYBACK);
    snd_close(dev, SND_STREAM_PLAYBACK);
    am_stop_path("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K");
    osEventFlagsDelete(player->event);
    ExtFree(player->pcm_data);
}

static int ani_audio_init(ani_task_token_t *task)
{
    ani_audio_player_t *player = &app_player;
    snd_dev_t *dev = NULL;
    int len = 0;
    void *audio_pcm_ptr = NULL;
    int audio_pcm_size = 0;

    load_pcm_from_path(FAST_AUDIO_PATH, &audio_pcm_ptr, &audio_pcm_size);

    dev = player->dev = i2s_sc1_dev;
    player->pcm_data = audio_pcm_ptr;
    player->pcm_size = audio_pcm_size;
    player->params = &pcm_param;

    player->running = true;
    player->event = osEventFlagsNew(NULL);
    player->pcm_ptr = 0;

    if (!snd_open(dev, player->params, player_callback, (void *)player,
                  SND_STREAM_PLAYBACK)) {
        goto fail;
    }

    len = snd_write(dev, player->pcm_data + player->pcm_ptr, audio_pcm_size);
    if (len < 0) {
        snd_close(dev, SND_STREAM_PLAYBACK);
        goto fail;
    }

    player->pcm_ptr += len;
    am_start_path("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K", 100);

    if (!snd_start(dev, SND_STREAM_PLAYBACK)) {
        snd_close(dev, SND_STREAM_PLAYBACK);
        am_stop_path("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K");
        goto fail;
    }

    return 0;

fail:
    osEventFlagsDelete(player->event);
    if (player->pcm_data)
        ExtFree(player->pcm_data);

    return -1;
}

static void ani_audio_entry(ani_task_token_t *task)
{
    if (task && task->running) {
        ani_audio_playback(task);
    }
}

static void ani_audio_exit(ani_task_token_t *task)
{
    if (task) {
        osEventFlagsSet(task->terminate_sig, ANI_MASK(task->id));
        msg_t msg = {.msgid = ani_evt_elapsed, .param = task->id};
        ani_set_evt(&msg);
    }
    osThreadExit();
}
#endif /* CONFIG_AUDIO_HUB */

void ani_audio_task(void *args)
{
#ifndef CONFIG_AUDIO_HUB_SVC
    if (args) {
        ani_task_token_t *task = (ani_task_token_t *)args;

        if (!ani_audio_init(task)) {
            ani_audio_entry(task);
        }
        ani_audio_exit(task);
    }
#else
    wait_audio_init_done();
    ahub_ani_audio_task(NULL);
#endif
}
/* EOF */
