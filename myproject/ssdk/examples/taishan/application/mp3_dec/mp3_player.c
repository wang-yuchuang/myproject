/**
 * @file mp3_player.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <string.h>

#include "CLI_type.h"
#include "cmsis_os2.h"
#include "ff.h"
#include "mp3.h"

#ifdef CONFIG_CODEC
#include "codec_cfg.h"
#include "driver/taishan/snd/codec_driver.h"
#endif

#include "driver/dma.h"
#include "driver/snd_driver.h"
#include "reg.h"
#include "regs_base.h"

#define DISKD "mmc1:" //"mmc0:"
#define MP3_TEST_FILE DISKD "early_app/cluster/16bit_48K_stereo.mp3"

/* player thead exit event */
#define MP3_PLAYER_THREAD_EXIT 1
#define AUIDO_PCM_XFER_COMPLETE 2

#define CHNNELS_COUNT 2
#define DMA_PERIOD_FRAME_COUNT 1024
#define DMA_PERIOD_SIZE DMA_PERIOD_FRAME_COUNT *CHNNELS_COUNT * 2
#define DMA_PERIOD_COUNT 2
#define DMA_BUF_TOTAL_SIZE DMA_PERIOD_SIZE *DMA_PERIOD_COUNT

extern snd_dev_t *i2s1_dev;
static snd_params_t snd_params = {
    .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_SLAVE,
    .channels = SND_CHANNEL_NUM_STEREO,
    .sample_width = SND_SAMPLE_WIDTH_16BITS,
    .sample_rate = SND_SR_48000,
    .period_size = DMA_PERIOD_SIZE,
    .period_count = DMA_PERIOD_COUNT,
};

static void player_callback(void *context, snd_state_t state)
{

    osEventFlagsId_t audio_event = (osEventFlagsId_t)context;

    if (audio_event) {
        osEventFlagsSet(audio_event, AUIDO_PCM_XFER_COMPLETE);
    }
}

static bool audio_player_init(snd_dev_t *audio_dev, snd_params_t *params,
                              snd_callback callback, void *data)
{
#ifdef CONFIG_CODEC
    bool ret = false;
    snd_params_t *snd_params = params;
    snd_stream_type_t stream = SND_STREAM_PLAYBACK;
    codec_dev_t *codec = NULL;

#ifdef CONFIG_CODEC_AK4619
    codec = codec_ak4619;
#elif defined CONFIG_CODEC_TLV320
    codec = codec_tlv320;
#endif
    if (codec) {
        codec_startup(codec, stream);
        codec_set_params(codec, snd_params, stream);
        codec_trigger(codec, SND_TRIGGER_START, stream);
    }

    ret = snd_open(audio_dev, snd_params, callback, data, stream);
    if (!ret) {
        printf("snd_open err \r\n");
        return ret;
    }
#endif
    return true;
}

static void audio_player_deinit(snd_dev_t *audio_dev)
{
#ifdef CONFIG_CODEC
    snd_stream_type_t stream = SND_STREAM_PLAYBACK;
    codec_dev_t *codec = NULL;

#ifdef CONFIG_CODEC_AK4619
    codec = codec_ak4619;
#elif defined CONFIG_CODEC_TLV320
    codec = codec_tlv320;
#endif

    snd_stop(audio_dev, SND_STREAM_PLAYBACK);
    snd_close(audio_dev, SND_STREAM_PLAYBACK);

    if (codec) {
        codec_trigger(codec, SND_TRIGGER_STOP, stream);
        codec_shutdown(codec, stream);
    }
#endif
}

static void mp3_player_thread_func(void *data)
{
    snd_dev_t *audio_dev = i2s1_dev;

    bool ret = false, audio_started = false;
    size_t readed = 0, total_samples = 0, sample_len = 0, filled = 0;
    mp3d_sample_t *mp3_frame = NULL;
    mp3_dec_context *context = NULL;
    mp3_frame_info frame_info = {0};
    osEventFlagsId_t audio_event = osEventFlagsNew(NULL);
    osEventFlagsId_t player_event = (osEventFlagsId_t)data;

    ret = audio_player_init(audio_dev, &snd_params, player_callback,
                            (void *)audio_event);
    if (!ret) {
        printf("audio_player_init err \r\n");
        return;
    }

    context = mp3_init_dec(MP3_IO_SOURCE_DISK, MP3_TEST_FILE, 0);
    if (!context) {
        printf("mp3_init_dec failed \n");
        return;
    }

    mp3_open_dec(context);
    mp3_dec_get_frame_info(context, &frame_info);
    mp3_frame = osAllocAlign(32, frame_info.samples * sizeof(mp3d_sample_t));

    // mp3_dec_seek_frame(context, (frame_info.duration >> 1));
    while ((readed = mp3_dec_receive_frame(context, mp3_frame,
                                           frame_info.samples)) > 0) {
        sample_len = readed * sizeof(mp3d_sample_t);
        filled = snd_write(audio_dev, (uint8_t *)mp3_frame, sample_len);
        while (filled != sample_len) {
            if (!audio_started) {
                audio_started = true;
                printf("snd_start\n");
                snd_start(audio_dev, SND_STREAM_PLAYBACK);
            }

            osEventFlagsWait(audio_event, AUIDO_PCM_XFER_COMPLETE,
                             osFlagsWaitAll, osWaitForever);
            filled += snd_write(audio_dev, (uint8_t *)mp3_frame + filled,
                                sample_len - filled);
        }

        printf("mp3_dec_get_frames filled with %d, readed with %d \n", filled,
               sample_len);
        total_samples += readed;
    }

    printf("mp3_dec_get_frames total samples %d \n", total_samples);

    if (mp3_frame)
        osFree(mp3_frame);
    mp3_close_dec(context);

    osDelay(10);
    osEventFlagsDelete(audio_event);
    audio_player_deinit(audio_dev);

    osEventFlagsSet(player_event, MP3_PLAYER_THREAD_EXIT);
    osThreadExit();
}

static int mp3_player(int argc, char **argv)
{
    FATFS fatfs = {0};
    osEventFlagsId_t player_event = osEventFlagsNew(NULL);
    osThreadAttr_t attr = {
        .priority = osPriorityRealtime,
        .stack_size = 1024 * 100, /* the mp3 decoder need a big stack */
    };
    f_mount(&fatfs, DISKD, 1);
    osThreadNew(mp3_player_thread_func, player_event, &attr);
    osEventFlagsWait(player_event, MP3_PLAYER_THREAD_EXIT, osFlagsWaitAny,
                     osWaitForever);
    osEventFlagsDelete(player_event);

    f_mount(NULL, DISKD, 1);
    printf("mp3_player exist\n");
    return CLI_TRUE;
}

CLI_CMD("mp3_player", "\r\nmp3_player:\r\n mp3 decode demo\n", mp3_player);
