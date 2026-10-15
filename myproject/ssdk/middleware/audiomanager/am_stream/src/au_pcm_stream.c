/**
 * @file au_pcm_stream.c
 * @brief audio pcm stream source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "au_pcm_stream.h"

#include "au_hw_device.h"

#define AM_PCM_STREAM_DBG_PRT SSDK_DEBUG
#define PCM_STREAM_TIMEOUT (2000)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static int pcm_stream_find_free_minor();
static void pcm_stream_realse_minor(int minor);
static au_pcm_stream_t *find_free_pcm_stream();
static int realse_pcm_stream(int minor);
void pcm_stream_res_destroy(pcm_stream_handle_t pcm_stream);
void pcm_stream_thread(void *token);
bool pcm_stream_init(au_pcm_stream_t *pcm_stream, au_source_t *source,
                     int32_t size, uint16_t vol);

static au_pcm_stream_mgr_t pcm_streams_mgr;

au_pcm_stream_t *pcm_stream_open(au_source_t *tem_source, int32_t size,
                                 uint16_t vol)
{
    au_pcm_stream_t *pcm_stream = find_free_pcm_stream();
    int minor = 0;
    osThreadAttr_t attr = {.name = "pcm_streams_id",
                           .stack_size = 4096,
                           .priority = osPriorityNormal};

    if (!pcm_stream) {
        ssdk_printf(SSDK_ERR, "%s:no free pcm_stream \n", __func__);
        return NULL;
    }

    pcm_stream_init(pcm_stream, tem_source, size, vol);

    pcm_stream->running = true;
    pcm_stream->thread = osThreadNew(pcm_stream_thread, pcm_stream, &attr);

    if (pcm_stream->thread == NULL) {
        ssdk_printf(SSDK_ERR, "%s:pcm_streams faild\n", __func__);
        goto ERR_POS1;
    }

    osEventFlagsWait(pcm_stream->pcm_stream_sync, 1, osFlagsWaitAny,
                     PCM_STREAM_TIMEOUT);
    if (!pcm_stream->result_sync) {
        pcm_stream->status = AU_PCM_STREAM_ST_OPENED;
    } else {
        pcm_stream->status = AU_PCM_STREAM_ST_IDLE;
        goto ERR_POS1;
    }

    ssdk_printf(AM_PCM_STREAM_DBG_PRT,
                "%s: create_pcm_stream pcm_stream id(%d) n", __func__,
                pcm_stream->id);
    return pcm_stream;
ERR_POS1:
    ssdk_printf(SSDK_ERR,
                "%s: ####create_pcm_stream faild pcm_stream id(%d) #####\n",
                __func__, pcm_stream->id);
    return NULL;
}

int pcm_stream_close(pcm_stream_handle_t pcm_stream)
{
    int ret = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    pcm_stream->status = AU_PCM_STREAM_ST_IDLE;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return ret;
}

int pcm_stream_res_create(pcm_stream_handle_t pcm_stream)
{
    int ret = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    ret = au_source_open(&pcm_stream->source);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: au_source_open faild \n", __func__);
        goto ERR_POS1;
    }

    pcm_stream->out_temp = (uint8_t *)memalign(32, DMA_PERIOD_BUFLEN);

    if (!pcm_stream->out_temp) {
        ssdk_printf(SSDK_ERR, "%s: err pcm_streammgr buf cant get \n",
                    __func__);
        goto ERR_POS1;
    }

    ringbuf_create(&pcm_stream->rb,pcm_stream->size);

    osMutexRelease(pcm_stream->pcm_stream_mutex);

    ssdk_printf(AM_PCM_STREAM_DBG_PRT,
                "%s: ####create_pcm_stream sucess #####\n", __func__);

    return ret;

ERR_POS1:
    ssdk_printf(SSDK_ERR, "%s: ####create_pcm_stream faild #####\n", __func__);
    au_source_close(&pcm_stream->source);
    if (pcm_stream->out_temp) {
        free(pcm_stream->out_temp);
        pcm_stream->out_temp = NULL;
    }


    osMutexRelease(pcm_stream->pcm_stream_mutex);
    ret = -1;

    return ret;
}

void pcm_stream_res_destroy(pcm_stream_handle_t pcm_stream)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    au_source_close(&pcm_stream->source);

    if (pcm_stream->out_temp) {
        free(pcm_stream->out_temp);
        pcm_stream->out_temp = NULL;
    }

    ringbuf_destroy(&pcm_stream->rb);

    realse_pcm_stream(pcm_stream_get_id(pcm_stream));
    osMutexRelease(pcm_stream->pcm_stream_mutex);
}

void pcm_stream_start(pcm_stream_handle_t pcm_stream)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    if (AU_PCM_STREAM_ST_OPENED == pcm_stream->status) {
        pcm_stream->status = AU_PCM_STREAM_ST_RUNNING;
    }
    osMutexRelease(pcm_stream->pcm_stream_mutex);
}

void pcm_stream_stop(pcm_stream_handle_t pcm_stream)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    if (AU_PCM_STREAM_ST_RUNNING == pcm_stream->status) {
        pcm_stream->status = AU_PCM_STREAM_ST_STOPPED;
    }
    pcm_stream->running = false;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
    if (AU_PCM_STREAM_ST_STOPPED == pcm_stream->status) {
        pcm_stream_post(pcm_stream);
    }
    osEventFlagsWait(pcm_stream->pcm_stream_sync, 1, osFlagsWaitAny,
                     PCM_STREAM_TIMEOUT);
}

void pcm_stream_set_vol(pcm_stream_handle_t pcm_stream, uint16_t vol)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    if (vol > 100) {
        vol = 100;
    }

    if (vol < 0) {
        vol = 0;
    }

    pcm_stream->vol = vol;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
}

uint16_t pcm_stream_get_vol(pcm_stream_handle_t pcm_stream)
{
    uint16_t vol = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    vol = pcm_stream->vol;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return vol;
}

void pcm_stream_pause(pcm_stream_handle_t pcm_stream)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    pcm_stream->status = AU_PCM_STREAM_ST_PAUSED;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
}

void pcm_stream_resume(pcm_stream_handle_t pcm_stream)
{
    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    pcm_stream->status = AU_PCM_STREAM_ST_RUNNING;
    osMutexRelease(pcm_stream->pcm_stream_mutex);
}

void pcm_stream_post(pcm_stream_handle_t pcm_stream)
{
    osSemaphoreRelease(pcm_stream->sem);
}

void pcm_stream_wait(pcm_stream_handle_t pcm_stream)
{
    osSemaphoreAcquire(pcm_stream->sem, osWaitForever);
}

int pcm_stream_read(pcm_stream_handle_t pcm_stream, uint8_t *out, int32_t size)
{
    int count = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    count = ringbuf_read(&pcm_stream->rb, out, size);

    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return count;
}

int pcm_stream_write(pcm_stream_handle_t pcm_stream, uint8_t *in, int32_t size)
{
    int count = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    count = ringbuf_write(&pcm_stream->rb, in, size);

    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return count;
}

au_pcm_stream_status_t pcm_stream_get_status(pcm_stream_handle_t pcm_stream)
{
    au_pcm_stream_status_t status;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);
    status = pcm_stream->status;
    osMutexRelease(pcm_stream->pcm_stream_mutex);

    return status;
}

int pcm_stream_get_id(pcm_stream_handle_t pcm_stream)
{
    int id;

    id = pcm_stream->id;
    return id;
}

bool pcm_stream_is_active(pcm_stream_handle_t pcm_stream)
{
    au_pcm_stream_status_t status = pcm_stream_get_status(pcm_stream);

    if (AU_PCM_STREAM_ST_RUNNING == status) {
        return true;
    } else {
        return false;
    }
}

bool pcm_stream_is_idle(pcm_stream_handle_t pcm_stream)
{
    au_pcm_stream_status_t status = pcm_stream_get_status(pcm_stream);

    if (AU_PCM_STREAM_ST_IDLE == status || AU_PCM_STREAM_ST_STOPPED == status) {
        return true;
    } else {
        return false;
    }
}

int pcm_stream_get_used(pcm_stream_handle_t pcm_stream)
{
    int count = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    count = ringbuf_get_used(&pcm_stream->rb);

    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return count;
}

int pcm_stream_get_available(pcm_stream_handle_t pcm_stream)
{
    int count = 0;

    osMutexAcquire(pcm_stream->pcm_stream_mutex, osWaitForever);

    count = ringbuf_get_available(&pcm_stream->rb);

    osMutexRelease(pcm_stream->pcm_stream_mutex);
    return count;
}

static au_pcm_stream_t *find_free_pcm_stream()
{
    au_pcm_stream_t *pcm_stream = NULL;
    au_pcm_stream_mgr_t *pcm_streamsmgr = &pcm_streams_mgr;
    au_pcm_stream_minor_t *pcm_stream_minors =
        pcm_streamsmgr->pcm_stream_minors;

    osMutexAcquire(pcm_streamsmgr->mgr_mutex, osWaitForever);

    for (int minor = 0;
         minor < (int)ARRAY_SIZE(pcm_streamsmgr->pcm_stream_minors); ++minor) {
        if (!pcm_stream_minors[minor].allocated) {
            pcm_stream_minors[minor].allocated = true;
            pcm_stream = &pcm_stream_minors[minor].pcm_stream;
            break;
        }
    }

    osMutexRelease(pcm_streamsmgr->mgr_mutex);
    return pcm_stream;
}

static int realse_pcm_stream(int minor)
{
    au_pcm_stream_mgr_t *pcm_streamsmgr = &pcm_streams_mgr;
    au_pcm_stream_minor_t *pcm_stream_minors =
        pcm_streamsmgr->pcm_stream_minors;

    if (minor >= (int)ARRAY_SIZE(pcm_streamsmgr->pcm_stream_minors)) {
        ssdk_printf(SSDK_ERR, " %s:err can't find minor(%d)\n", __func__,
                    minor);
        return -1;
    }

    osMutexAcquire(pcm_streamsmgr->mgr_mutex, osWaitForever);
    pcm_stream_minors[minor].allocated = false;
    osMutexRelease(pcm_streamsmgr->mgr_mutex);
    return 0;
}

static uint32_t request_pcm_stream_data(au_pcm_stream_t *pcm_stream,
                                        int32_t out_len)
{
    int count = 0;
    au_source_t *source = &pcm_stream->source;
    bool have_mix = false;

    if (!pcm_stream->out_temp) {
        ssdk_printf(SSDK_ERR, "err out_temp null .can't handle %s:\n",
                    __func__);
        return 0;
    }

    count = au_source_read(source, pcm_stream->out_temp, out_len);

    if (count > 0) {
        count = pcm_stream_write(pcm_stream, pcm_stream->out_temp, count);
    }

    return count;
}

void prepare_pcm_stream_data(pcm_stream_handle_t pcm_stream)
{
    while (pcm_stream_get_available(pcm_stream) >= DMA_PERIOD_BUFLEN) {
        request_pcm_stream_data(pcm_stream, DMA_PERIOD_BUFLEN);
    }
}

void pcm_stream_request_data_from_source(pcm_stream_handle_t pcm_stream)
{
    au_source_t *source = &pcm_stream->source;

    if (pcm_stream->pcm_stream_type == AU_HWD_TYPE_CAPTURE) { /* capture */
        while (pcm_stream_get_used(pcm_stream) >= DMA_PERIOD_BUFLEN &&
               !pcm_stream_is_idle(pcm_stream)) {
            if (source->request_data_call_back) {

                source->request_data_call_back(pcm_stream, DMA_PERIOD_BUFLEN,
                                               source->callback_param);
            }
        }
    } else {
        while (pcm_stream_get_available(pcm_stream) >= DMA_PERIOD_BUFLEN &&
               !pcm_stream_is_idle(pcm_stream)) {
            if (source->request_data_call_back) {

                source->request_data_call_back(pcm_stream, DMA_PERIOD_BUFLEN,
                                               source->callback_param);
            }
        }
    }
}

void pcm_stream_thread(void *token)
{
    int ret = 0;
    /* must open source before creat thread */
    au_pcm_stream_t *pcm_stream = (au_pcm_stream_t *)token;
    au_source_t *source = &pcm_stream->source;

    ret = pcm_stream_res_create(pcm_stream);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR,
                    "%s: pcm_stream_res_create pcm_stream id(%d) faild \n",
                    __func__, pcm_stream->id);
        pcm_stream->running = false;
        pcm_stream->result_sync = 1;
    } else {
        /* TODO pcm_stream request data if file type */
        if (source->source_type == AU_SOURCE_FILE) {
            prepare_pcm_stream_data(pcm_stream);
            ssdk_printf(AM_PCM_STREAM_DBG_PRT, "%s: prepare data ready(%d) \n",
                        __func__, pcm_stream->id);
        }

        pcm_stream->result_sync = 0;
    }

    osEventFlagsSet(pcm_stream->pcm_stream_sync, 1);

    while (pcm_stream->running) {
        pcm_stream_wait(pcm_stream);

        if (pcm_stream_is_idle(pcm_stream)) {

            ssdk_printf(AM_PCM_STREAM_DBG_PRT, "%s: idle pcm_stream id =%d\n",
                        __func__, pcm_stream->id);

        } else {
            pcm_stream_request_data_from_source(pcm_stream);
        }
    }

    pcm_stream_res_destroy(pcm_stream);
    osEventFlagsSet(pcm_stream->pcm_stream_sync, 1);
    osThreadExit();
    ssdk_printf(AM_PCM_STREAM_DBG_PRT, "%s: pcm_stream exit id =%d\n", __func__,
                pcm_stream->id);
}

int request_file_data_callback(au_pcm_stream_t *pcm_stream, int32_t size,
                               void *param)
{
    int count = 0;
    au_source_t *source = &pcm_stream->source;
    bool have_mix = false;

    if (!pcm_stream->out_temp) {
        ssdk_printf(SSDK_ERR, "err out_temp null .can't handle %s:\n",
                    __func__);
        return 0;
    }

    count = au_source_read(source, pcm_stream->out_temp, DMA_PERIOD_BUFLEN);

    if (count > 0) {
        count = pcm_stream_write(pcm_stream, pcm_stream->out_temp, count);
    }

    return count;
}

bool pcm_stream_init(au_pcm_stream_t *pcm_stream, au_source_t *a_source,
                     int32_t size, uint16_t vol)
{
    bool ret = true;
    au_source_t *source = &pcm_stream->source;
    source->source_type = a_source->source_type;
    source->pcm_stream = pcm_stream;

    if (a_source->request_data_call_back) {
        source->request_data_call_back = a_source->request_data_call_back;
        source->callback_param = a_source->callback_param;
    }

    if (a_source->source_type == AU_SOURCE_FILE) {
        strncpy(source->path, a_source->path, AU_SOURCE_PATH_SIZE);
        source->request_data_call_back = request_file_data_callback;
        source->callback_param = pcm_stream;
    }
    pcm_stream->size = size;
    pcm_stream->vol = vol;
    return ret;
}

void au_pcm_streams_init(void)
{
    int minor;
    au_pcm_stream_t *pcm_stream;
    au_pcm_stream_mgr_t *pcm_streamsmgr = &pcm_streams_mgr;
    au_pcm_stream_minor_t *pcm_stream_minors =
        pcm_streamsmgr->pcm_stream_minors;
    pcm_streamsmgr->mgr_mutex = osMutexNew(NULL);

    for (minor = 0; minor < (int)ARRAY_SIZE(pcm_streamsmgr->pcm_stream_minors);
         ++minor) {

        pcm_stream = &pcm_stream_minors[minor].pcm_stream;
        memset(pcm_stream, 0, sizeof(au_pcm_stream_t));
        pcm_stream_minors[minor].allocated = false;
        pcm_stream->id = minor;
        pcm_stream->pcm_stream_mutex = osMutexNew(NULL);
        pcm_stream->sem = osSemaphoreNew(1, 0, NULL);
        pcm_stream->pcm_stream_sync = osEventFlagsNew(NULL);
        pcm_stream->status = AU_PCM_STREAM_ST_IDLE;
    }
}
