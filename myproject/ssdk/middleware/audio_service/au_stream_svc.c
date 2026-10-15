/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <arch/cache.h>
#include <debug.h>
#include <lib/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_stream_svc.h"

#include "au_rpc_svc.h"
#include "driver/memmap.h"

static volatile audio_stream_handler_t g_au_stream[MAX_STREAM_COUNT] = {0};
static audio_stream_service_t g_au_stream_server;
const uint32_t audio_source_vol[MAX_STREAM_COUNT] = {
    100, 100, 80, 80, 60, 60, 70, 80,
};
static int audio_stream_rproc_id[] = {
    AUDIO_RPMSG_RRPOC_ID_AP1,
    AUDIO_RPMSG_RRPOC_ID_AP2,
};

static int am_stream_cb_func(pcm_stream_handle_t pcm_stream, int32_t size,
                             void *param)
{
    audio_stream_handler_t *au_stream = (audio_stream_handler_t *)param;
    am_stream_dev_t *dev = (am_stream_dev_t *)au_stream->dev;
    uint8_t *in;
    uint8_t *out;

    if (pcm_stream->pcm_stream_type == AM_STREAM_TYPE_PLAYBACK) {
        am_stream_do_rpc_message(
            dev, AM_STREAM_REQUEST,
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].stream_id, NULL,
            AM_STREAM_MODE_SERVER);

        in = (uint8_t *)(au_stream->stream[AM_STREAM_TYPE_PLAYBACK]
                             .phy_buffer_addr +
                         au_stream->stream[AM_STREAM_TYPE_PLAYBACK]
                             .dma_buffer_ptr);

        arch_invalidate_cache_range(
            (uint32_t)in,
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);

        au_playback_write(
            pcm_stream, in,
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);

        au_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr +=
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size;
        if (au_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr >=
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].buffer_size) {
            au_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr = 0;
        }

        return au_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size;
    } else {
        out = (uint8_t *)(au_stream->stream[AM_STREAM_TYPE_CAPTURE]
                              .phy_buffer_addr +
                          au_stream->stream[AM_STREAM_TYPE_CAPTURE]
                              .dma_buffer_ptr);

        au_capture_read(
            pcm_stream, out,
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size);

        arch_clean_cache_range(
            (uint32_t)out,
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size);

        au_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr +=
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size;
        if (au_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr >=
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].buffer_size) {
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr = 0;
        }

        am_stream_do_rpc_message(
            dev, AM_STREAM_REQUEST,
            au_stream->stream[AM_STREAM_TYPE_CAPTURE].stream_id, NULL,
            AM_STREAM_MODE_SERVER);

        return au_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size;
    }
}

static int am_stream_init(am_stream_dev_t *dev)
{
    if (!dev) {
        return -1;
    }

    dev->private_data = (void *)g_au_stream;

    audio_stream_handler_t *au_stream =
        (audio_stream_handler_t *)dev->private_data;

    for (int index = 0; index < MAX_STREAM_COUNT; index++) {
        for (int id = 0; id < ID_NUM_FOR_STREAM; id++) {
            au_stream->stream[id].volume = audio_source_vol[index];
            au_stream->stream[id].stream_id =
                ((id == 0) ? (ID_NUM_FOR_STREAM * index)
                           : (ID_NUM_FOR_STREAM * index + 1));

            au_source_set(&au_stream->stream[id].source_ptr,
                          am_stream_cb_func, NULL, AU_SOURCE_STREAM);
            au_stream->stream[id].source_ptr.callback_param =
                (void *)au_stream;
        }
        au_stream->dev = dev;
        au_stream++;
    }

    return 0;
}

static int am_stream_deinit(am_stream_dev_t *dev) { return 0; }

static int am_stream_open(am_stream_dev_t *dev, uint8_t stream_id, void *param)
{
    if (!dev) {
        return -1;
    }

    am_stream_t *stream = (am_stream_t *)param;

    audio_stream_handler_t *au_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        stream_id / ID_NUM_FOR_STREAM;
    int id = stream_id % ID_NUM_FOR_STREAM;

    au_stream->stream[id].phy_buffer_addr =
        ap2p(stream->stream_param.buffer_addr);

    au_stream->stream[id].buffer_size = stream->buffer_size;
    au_stream->stream[id].period_size = stream->stream_param.period_size;
    au_stream->stream[id].dma_buffer_ptr = 0;

    if (AM_STREAM_TYPE_PLAYBACK == id) {
        au_stream->stream[id].pcm_stream_ptr =
            au_playback_open(au_stream->stream[id].volume,
                             &au_stream->stream[id].source_ptr);
    } else {
        au_stream->stream[id].pcm_stream_ptr =
            au_capture_open(au_stream->stream[id].volume,
                            &au_stream->stream[id].source_ptr);
    }

    return 0;
}

static int am_stream_start(am_stream_dev_t *dev, uint8_t stream_id, void *param)
{
    if (!dev) {
        return -1;
    }

    audio_stream_handler_t *au_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        stream_id / ID_NUM_FOR_STREAM;

    int id = stream_id % ID_NUM_FOR_STREAM;
    int data_len = au_stream->stream[id].period_size * 2;

    if (AM_STREAM_TYPE_PLAYBACK == id) {
        arch_invalidate_cache_range(
            (uint32_t)au_stream->stream[id].phy_buffer_addr, data_len);

        au_playback_write(au_stream->stream[id].pcm_stream_ptr,
                          (uint8_t *)(au_stream->stream[id].phy_buffer_addr),
                          data_len);
        au_stream->stream[id].dma_buffer_ptr += data_len;
        if (au_stream->stream[id].dma_buffer_ptr >=
            au_stream->stream[id].buffer_size) {
            au_stream->stream[id].dma_buffer_ptr = 0;
        }

        au_playback_start(au_stream->stream[id].pcm_stream_ptr);
    } else {
        au_capture_start(au_stream->stream[id].pcm_stream_ptr);
    }

    return 0;
}

static int am_stream_stop(am_stream_dev_t *dev, uint8_t stream_id, void *param)
{
    if (!dev) {
        return -1;
    }

    audio_stream_handler_t *au_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        stream_id / ID_NUM_FOR_STREAM;
    int id = stream_id % ID_NUM_FOR_STREAM;

    if (AM_STREAM_TYPE_PLAYBACK == id) {
        au_playback_stop(au_stream->stream[id].pcm_stream_ptr);
        au_playback_close(au_stream->stream[id].pcm_stream_ptr);
    } else {
        au_capture_stop(au_stream->stream[id].pcm_stream_ptr);
        au_capture_close(au_stream->stream[id].pcm_stream_ptr);
    }
    au_stream->stream[id].pcm_stream_ptr = NULL;

    return 0;
}

static int am_stream_send_msg(int stream_index, char *msg_buf, uint16_t msg_len)
{
#define AUDIO_STREAM_RPC_TIMEOUT 500
    int ret = 0;
    audio_stream_service_t *service = &g_au_stream_server;
    uint8_t index = g_au_stream[stream_index / ID_NUM_FOR_STREAM].dcf_id;

    ret =
        rpmsg_channel_send(service->device[index].chan, AUDIO_STREAM_SERV_EPT,
                           (void *)msg_buf, msg_len, AUDIO_STREAM_RPC_TIMEOUT);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

am_stream_dev_t am_stream_dev = {
    .stream_cnt = MAX_STREAM_COUNT,
    .ops =
        {
            .init = am_stream_init,
            .open = am_stream_open,
            .start = am_stream_start,
            .stop = am_stream_stop,
            .pause = NULL,
            .suspend = NULL,
            .resume = NULL,
            .deinit = am_stream_deinit,
            .send_msg = am_stream_send_msg,
        },
};

static void audio_stream_rpc_task(void *arg)
{
    struct am_stream_ipcc_data pack;
    am_stream_ctrl_t *am_ctrl = NULL;
    audio_stream_service_t *service = (audio_stream_service_t *)arg;
    osStatus_t ret;

    au_capture_init();
    au_mixer_init();

    am_stream_register_platform(&am_stream_dev);

    while (1) {
        ret = osMessageQueueGet(service->audio_stream_queue, &pack, NULL,
                                osWaitForever);

        if (ret == osOK) {
            if (pack.cmd == OP_PCM_STREAM_REQ) {
                am_ctrl = (am_stream_ctrl_t *)&pack.data[0];
                g_au_stream[am_ctrl->stream_id / ID_NUM_FOR_STREAM].dcf_id =
                    pack.dcf_id;
                am_stream_do_rpc_message(&am_stream_dev, am_ctrl->stream_cmd,
                                         am_ctrl->stream_id, &pack.data[0],
                                         AM_STREAM_MODE_SERVER);
            }
        }
    }
}

static int audio_stream_event_handle(void *data, int len, unsigned long src,
                                     void *arg)
{
    osStatus_t ret;
    struct audio_stream_rpc_dev *rpc_dev = (struct audio_stream_rpc_dev *)arg;
    struct am_stream_ipcc_data *rpc_data = (struct am_stream_ipcc_data *)data;
    audio_stream_service_t *service = containerof(
        rpc_dev, audio_stream_service_t, device[rpc_dev->device_index]);

    rpc_data->dcf_id = rpc_dev->device_index;

    ret = osMessageQueuePut(service->audio_stream_queue, rpc_data, 0, 0);

    if (ret == osOK) {
        return 0;
    } else {
        ssdk_printf(SSDK_ERR, "%s: message queue put failed.\r\n", __func__);
        return -1;
    }
}

void audio_stream_service_init(void)
{
    audio_stream_service_t *service = &g_au_stream_server;
    struct rpmsg_instance *dev;
    struct rpmsg_channe *chan;
    osThreadAttr_t audio_stream_task_attr = {
        .name = "audio_stream_task",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal,
    };

    if (service->initialized) {
        return;
    }

    service->initialized = true;

    for (int index = 0; index < ARRAY_SIZE(audio_stream_rproc_id); index++) {
        dev = rpmsg_get_instance(AUDIO_STREAM_RPMSG_TYPE,
                                 audio_stream_rproc_id[index]);
        if (!dev) {
            ssdk_printf(SSDK_ALERT, "audio stream create instance failed\r\n");
            return;
        }

        chan = rpmsg_channel_create(dev, AUDIO_STREAM_RPMSG_NAME,
                                    AUDIO_STREAM_SERV_EPT);
        if (!chan) {
            ssdk_printf(SSDK_ALERT, "audio rpmsg create chan failed\r\n");
            return;
        }

        service->device[index].dev = dev;
        service->device[index].chan = (struct rpmsg_channel *)chan;
        service->device[index].device_index = index;

        rpmsg_channel_start(chan, audio_stream_event_handle,
                            (void *)&service->device[index]);
    }

    service->audio_stream_queue = osMessageQueueNew(
        AUDIO_STREAM_MAX_QUEUE_COUNT, sizeof(struct am_stream_ipcc_data), NULL);
    if (service->audio_stream_queue == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create queue failed.\r\n", __func__);
        return;
    }

    service->stream_handler = osThreadNew(
        audio_stream_rpc_task, (void *)service, &audio_stream_task_attr);

    if (service->stream_handler == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        osMessageQueueDelete(service->audio_stream_queue);
        return;
    }
}