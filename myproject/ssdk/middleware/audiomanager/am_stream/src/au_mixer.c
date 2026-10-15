/**
 * @file au_mixer.c
 * @brief audio mixer source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <cmsis_os2.h>
#include <debug.h>
#include <lib/list.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_mixer.h"

#include "au_hw_device.h"
#include "au_swvol.h"
#include "sd_audio.h"

typedef enum {
    AU_MIX_OP_NONE = 0,
    AU_MIX_OP_OPEN = 1,
    AU_MIX_OP_CLOSE = 2,
    AU_MIX_OP_START = 3,
    AU_MIX_OP_STOP = 4,
    AU_MIX_OP_PAUSE = 5,
    AU_MIX_OP_RESUME = 6,
    AU_MIX_OP_MIX = 7,
} au_mix_op_type;

typedef struct {
    osEventFlagsId_t mix_start_event;
    osThreadId_t *thread;
    au_mix_op_type op_code;
    osMutexId_t mix_mutex;
    uint32_t hw_opened;
    uint32_t hw_started;
    uint32_t hw_paused;
    struct list_node pcm_streams;
    au_hw_device_t hw_device;
    uint8_t *out_temp;
    uint8_t *sink;
} au_mix_t;

static au_mix_t mix_manager;
static int au_mixer_handle(au_mix_t *mix);
int mix_res_create(au_mix_t *mix);
void mix_res_destroy(au_mix_t *mix);
void mix_data_prepare(au_mix_t *mix);
bool au_playback_reset(pcm_stream_handle_t handle);

#define AM_MIX_DBG_PRT SSDK_DEBUG

void audio_mix_task(void *token)
{
    au_mix_t *mix = (au_mix_t *)token;

    mix_res_create(mix);

    while (1) {

        ssdk_printf(AM_MIX_DBG_PRT, "mix_start_event:=%p\n",
                    mix->mix_start_event);
        osEventFlagsWait(mix->mix_start_event, 1, osFlagsWaitAny,
                         osWaitForever);

        au_mixer_handle(mix);
    }

    mix_res_destroy(mix);
    ssdk_printf(AM_MIX_DBG_PRT, "%s:@@@ mix task exit @@@ \n", __func__);
}

bool au_mixer_init(void)
{
    bool ret = true;
    au_mix_t *mix = &mix_manager;
    osThreadAttr_t attr = {.name = "mixer_server",
                           .stack_size = 4096,
                           .priority = osPriorityNormal};

    memset(mix, 0, sizeof(au_mix_t));
    mix->hw_opened = 0;
    mix->hw_started = 0;
    mix->hw_paused = 0;
    mix->mix_mutex = osMutexNew(NULL);
    mix->mix_start_event = osEventFlagsNew(NULL);

    list_initialize(&mix->pcm_streams);

    ssdk_printf(AM_MIX_DBG_PRT, "mix_start_event:=%p\n", mix->mix_start_event);
    mix->thread = osThreadNew(audio_mix_task, mix, &attr);

    if (mix->thread == NULL) {
        osEventFlagsDelete(mix->mix_start_event);
        osMutexDelete(mix->mix_mutex);
        ssdk_printf(SSDK_ERR, "%s: ####mix task creat faild #####\n", __func__);
        ret = false;
    }

    hw_device_default(&mix->hw_device);
    au_pcm_streams_init();

    return ret;
}

void mixer_hw_device_callback(void *arg)
{
    au_mix_t *mix = (au_mix_t *)arg;

    mix->op_code = AU_MIX_OP_MIX;
    osEventFlagsSet(mix->mix_start_event, 1);
}

pcm_stream_handle_t au_playback_open(uint16_t vol, au_source_t *source)
{
    int handle = 0;
    int ret = 0;
    uint32_t flags;
    au_mix_t *mix = &mix_manager;
    pcm_stream_handle_t pcm_stream;

    osMutexAcquire(mix->mix_mutex, osWaitForever);

    pcm_stream = pcm_stream_open(source, DMA_BUF_TOTAL_LEN, vol);

    if (!pcm_stream) {
        ssdk_printf(SSDK_ERR, "%s: ####mix open faild #####\n", __func__);
        osMutexRelease(mix->mix_mutex);
        return NULL;
    }

    pcm_stream->pcm_stream_type = AU_HWD_TYPE_PLAYBACK; /* playback */
    mix->hw_device.type = AU_HWD_TYPE_PLAYBACK;

    list_add_tail(&mix->pcm_streams, &pcm_stream->mixer_node);

    au_playback_reset(pcm_stream);
    flags = mix->hw_opened;
    set_bit(mix->hw_opened, pcm_stream_get_id(pcm_stream));

    if (!flags) {
        /* first open ,need prepare data for hw_device,otherwise hw nodata to
         * consume */
        hw_device_open(&mix->hw_device, mixer_hw_device_callback, mix);
        /* todo mix data if file type */
        // mix_data_prepare(mix);
    }

    osMutexRelease(mix->mix_mutex);

    ssdk_printf(AM_MIX_DBG_PRT, "%s: mix open sucess \n", __func__);

    return pcm_stream;
}

int au_playback_close(pcm_stream_handle_t handle)
{
    au_mix_t *mix = &mix_manager;
    int ret = 0;
    uint32_t flags;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    osMutexAcquire(mix->mix_mutex, osWaitForever);
    pcm_stream_close(handle);
    list_delete(&handle->mixer_node);

    clear_bit(mix->hw_opened, pcm_stream_get_id(handle));
    flags = mix->hw_opened;

    if (!flags) {
        ret = hw_device_close(&mix->hw_device);
    }

    au_playback_reset(handle);
    osMutexRelease(mix->mix_mutex);
    ssdk_printf(AM_MIX_DBG_PRT, "%s: mix close sucess \n", __func__);

    return ret;
}

int au_playback_start(pcm_stream_handle_t handle)
{
    int ret = 0;
    uint32_t flags;
    au_mix_t *mix = &mix_manager;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    osMutexAcquire(mix->mix_mutex, osWaitForever);

    pcm_stream_start(handle);

    flags = mix->hw_started;
    set_bit(mix->hw_started, pcm_stream_get_id(handle));

    if (!flags) {
        /* fisrt mix data */
        mix_data_prepare(mix);
        ret = hw_device_start(&mix->hw_device);
    }

    osMutexRelease(mix->mix_mutex);
    ssdk_printf(AM_MIX_DBG_PRT, "%s: mix start sucess \n", __func__);

    return ret;
}

int au_playback_stop(pcm_stream_handle_t handle)
{
    uint32_t flags;
    int ret = 0;
    au_mix_t *mix = &mix_manager;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    osMutexAcquire(mix->mix_mutex, osWaitForever);
    pcm_stream_stop(handle);
    clear_bit(mix->hw_started, pcm_stream_get_id(handle));
    flags = mix->hw_started;

    if (!flags) {
        ret = hw_device_stop(&mix->hw_device);
    }

    osMutexRelease(mix->mix_mutex);

    return true;
}

int au_playback_write(pcm_stream_handle_t handle, uint8_t *in, int32_t size)
{
    int count = 0;
    int ret = 0;
    au_mix_t *mix = &mix_manager;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    osMutexAcquire(mix->mix_mutex, osWaitForever);
    count = pcm_stream_write(handle, in, size);
    osMutexRelease(mix->mix_mutex);

    return count;
}

int au_playback_pause(pcm_stream_handle_t handle)
{
    uint32_t flags;
    int ret = 0;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    au_mix_t *mix = &mix_manager;
    osMutexAcquire(mix->mix_mutex, osWaitForever);
    pcm_stream_pause(handle);
    set_bit(mix->hw_paused, pcm_stream_get_id(handle));

    if (mix->hw_started == mix->hw_paused) {
        ret = hw_device_pause(&mix->hw_device);
    }

    osMutexRelease(mix->mix_mutex);

    return ret;
}

int au_playback_resume(pcm_stream_handle_t handle)
{
    uint8_t flags;
    int ret = 0;
    au_mix_t *mix = &mix_manager;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    osMutexAcquire(mix->mix_mutex, osWaitForever);
    clear_bit(mix->hw_paused, pcm_stream_get_id(handle));
    pcm_stream_resume(handle);
    ret = hw_device_resume(&mix->hw_device);
    osMutexRelease(mix->mix_mutex);

    return ret;
}

int au_playback_set_vol(pcm_stream_handle_t handle, uint16_t vol)
{
    uint8_t flags;
    int ret = 0;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    au_mix_t *mix = &mix_manager;
    osMutexAcquire(mix->mix_mutex, osWaitForever);
    pcm_stream_set_vol(handle, vol);
    osMutexRelease(mix->mix_mutex);

    return ret;
}

int au_playback_get_vol(pcm_stream_handle_t handle)
{
    uint8_t flags;
    int ret = 0;
    uint16_t vol = 0;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        ret = -1;
        return ret;
    }

    au_mix_t *mix = &mix_manager;
    osMutexAcquire(mix->mix_mutex, osWaitForever);
    vol = pcm_stream_get_vol(handle);
    osMutexRelease(mix->mix_mutex);

    return vol;
}

bool au_playback_reset(pcm_stream_handle_t handle)
{
    au_mix_t *mix = &mix_manager;

    if (!handle) {
        ssdk_printf(SSDK_ERR, "%s: handle is null\n", __func__);
        return false;
    }

    clear_bit(mix->hw_opened, pcm_stream_get_id(handle));
    clear_bit(mix->hw_started, pcm_stream_get_id(handle));
    clear_bit(mix->hw_paused, pcm_stream_get_id(handle));

    return true;
}

static int32_t do_mix_s16(uint8_t *out, uint8_t *in, int32_t sample_count,
                          int16_t vol)
{
    int16_t input;
    int32_t sample;
    int16_t *pIn = (int16_t *)in;
    int16_t *pOut = (int16_t *)out;
    int16_t mixed = 0;
    int i = 0;

    if (vol < MAX_VOL_MAXVALUE) {
        au_swvol_apply_s16(out, in, sample_count, vol);
    } else {
        /* handle 0db (100 vol) */
        for (; i < sample_count; i++) {
            *pOut++ += *pIn++;
            ;
        }
    }

    return sample_count;
}

static int32_t do_mix_period(uint8_t *out, uint8_t *in, int32_t bytes,
                             int16_t vol, enum sd_audioflow_sample_width_t with)
{
    int32_t ret = 0;
    int32_t sample_count = 0;
    int32_t (*mix_hook)(uint8_t *, uint8_t *, int32_t, int16_t) = NULL;

    if (!in || !out) {
        ssdk_printf(SSDK_ERR, "err in or out is null %s:\n", __func__);
        return -1;
    }

    switch (with) {
    case SD_AUDIO_SAMPLE_WIDTH_16BITS:
        mix_hook = do_mix_s16;
        sample_count = bytes / 2;
        break;
    case SD_AUDIO_SAMPLE_WIDTH_32BITS:
        break;
    case SD_AUDIO_SAMPLE_WIDTH_24BITS:
        break;
    default:
        break;
    }

    if (mix_hook) {
        ret = mix_hook(out, in, sample_count, vol);
    }

    return ret;
}

/* while pcm_stream doing,may pcm_stream be removed from list  */
static uint32_t do_mix(au_mix_t *mix, int32_t out_len)
{
    pcm_stream_handle_t pcm_stream;
    int count = 0;
    bool have_mix = false;

    if ((!mix->sink) || (!mix->out_temp)) {
        ssdk_printf(SSDK_ERR, "err sink or out_temp NULL%s:\n", __func__);
        return 0;
    }

    memset((void *)mix->sink, 0x00, DMA_PERIOD_BUFLEN);
    list_for_every_entry (&mix->pcm_streams, pcm_stream, struct au_pcm_stream,
                          mixer_node) {
        if (pcm_stream_is_active(pcm_stream)) {
            if ((pcm_stream_get_used(pcm_stream) >= DMA_PERIOD_BUFLEN)) {
                count = pcm_stream_read(pcm_stream, mix->out_temp,
                                        DMA_PERIOD_BUFLEN);
                do_mix_period(mix->sink, mix->out_temp, count, pcm_stream->vol,
                              SD_AUDIO_SAMPLE_WIDTH_16BITS);
                pcm_stream_post(pcm_stream);
            } else {
                ssdk_printf(SSDK_ERR,
                            " %s:pcm_stream underrun pcm_stream_id=%d\n",
                            __func__, pcm_stream->id);
                pcm_stream_post(pcm_stream);
            }
        }
    }
    return count;
}

static uint32_t do_prepare_mix(au_mix_t *mix, int32_t out_len)
{

    pcm_stream_handle_t pcm_stream;
    int count = 0;
    bool have_mix = false;

    if ((!mix->sink) || (!mix->out_temp)) {
        ssdk_printf(SSDK_ERR, "err sink or out_temp NULL%s:\n", __func__);
        return 0;
    }

    memset((void *)mix->sink, 0x00, DMA_PERIOD_BUFLEN);
    list_for_every_entry (&mix->pcm_streams, pcm_stream, struct au_pcm_stream,
                          mixer_node) {
        if (!pcm_stream_is_idle(pcm_stream)) {
            if ((pcm_stream_get_used(pcm_stream) >= DMA_PERIOD_BUFLEN)) {
                count = pcm_stream_read(pcm_stream, mix->out_temp,
                                        DMA_PERIOD_BUFLEN);
                do_mix_period(mix->sink, mix->out_temp, count, pcm_stream->vol,
                              SD_AUDIO_SAMPLE_WIDTH_16BITS);
                pcm_stream_post(pcm_stream);
            } else {
                pcm_stream_post(pcm_stream);
            }
        }
    }
    return count;
}

static int au_mixer_handle(au_mix_t *mix)
{
    int ret = 0;
    int count = 0;

    ssdk_printf(AM_MIX_DBG_PRT, "%s:\n", __func__);

    switch (mix->op_code) {
    case AU_MIX_OP_MIX: {
        osMutexAcquire(mix->mix_mutex, osWaitForever);
        count = do_mix(mix, DMA_PERIOD_BUFLEN);
        if (count) {
            hw_device_write(&mix->hw_device, mix->sink, count);
        }

        osMutexRelease(mix->mix_mutex);
        break;
    }

    default:
        ssdk_printf(SSDK_ERR, "%s: error: not support op_code=%d\n", __func__,
                    mix->op_code);
        ret = -1;
    }

    return ret;
}

int mix_res_create(au_mix_t *mix)
{
    int ret = 0;

    mix->sink = (uint8_t *)memalign(32, DMA_PERIOD_BUFLEN);
    if (!mix->sink) {
        ssdk_printf(SSDK_ERR, "%s: err sink is null\n", __func__);
        goto ERR_POS1;
    }

    memset((void *)mix->sink, 0x00, DMA_PERIOD_BUFLEN);
    mix->out_temp = (uint8_t *)memalign(32, DMA_PERIOD_BUFLEN);

    if (!mix->out_temp) {
        ssdk_printf(SSDK_ERR, "%s: err out_temp is null\n", __func__);
        goto ERR_POS1;
    }

    memset((void *)mix->out_temp, 0x00, DMA_PERIOD_BUFLEN);
    ssdk_printf(AM_MIX_DBG_PRT, "%s: ####mix res sucess #####\n", __func__);
    return ret;

ERR_POS1:
    ssdk_printf(SSDK_ERR, "%s: ####mix res faild #####\n", __func__);

    if (mix->sink) {
        free(mix->sink);
        mix->sink = NULL;
    }

    if (mix->out_temp) {
        free(mix->out_temp);
        mix->out_temp = NULL;
    }

    ret = -1;
    return ret;
}

void mix_res_destroy(au_mix_t *mix)
{
    if (mix->sink) {
        free(mix->sink);
        mix->sink = NULL;
    }

    if (mix->out_temp) {
        free(mix->out_temp);
        mix->out_temp = NULL;
    }
}

void mix_data_prepare(au_mix_t *mix)
{
    int count = 0;

    /* two period_size data must be filled before startup */
    while (hw_device_available(&mix->hw_device) >= 2 * DMA_PERIOD_BUFLEN) {

        count = do_prepare_mix(mix, DMA_PERIOD_BUFLEN);

        if (count > 0) {
            hw_device_write(&mix->hw_device, mix->sink, count);
        }
    }
}
