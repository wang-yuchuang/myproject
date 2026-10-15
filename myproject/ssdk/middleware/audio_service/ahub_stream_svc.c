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

#include "ahub.h"
#include "au_rpc_svc.h"
#include "au_stream_svc.h"
#include "driver/memmap.h"

typedef struct ahub_config {
    uint8_t vaild_flag;      /* whether this configuration is valid */
    uint16_t ahub_stream_id; /* see struct AHUB_STREAMS */
    int16_t ahub_endpoint;   /* see struct AHUB_ENDPOINTS */
    void *policy;            /* policy for multi endpoint */
} ahub_config_t;

static eps_mix_policy_t eps_mix_policy = {
    .eps_count = 2,
    .archor_endpoint = AHUB_ENDPOINT_ID8,
    .sample_bytes = AHUB_SAMPLE_BYTES, /* fix to 16 bits sample bytes */
    .mix_policy =
        {
            {AHUB_ENDPOINT_ID8, 0, 2, 0},
            {AHUB_ENDPOINT_ID9, 2, 2, 0},
        },
};

static eps_fetch_policy_t eps_fetch_policy = {
    .eps_count = 2,
    .archor_endpoint = AHUB_ENDPOINT_ID2,
    .sample_bytes = AHUB_SAMPLE_BYTES, /* fix to 16 bits sample bytes */
    .fetch_policy =
        {
            {AHUB_ENDPOINT_ID2, 0, 2, 0},
            {AHUB_ENDPOINT_ID3, 2, 2, 0},
        },
};

/* MAX_STREAM_COUNT -- how many virtual i2s supported
   ID_NUM_FOR_STREAM -- each virtual i2s contains playback & capture stream
   odd number for playback , even number for capture
   For example:
0 ->playback,1 -> capture, corresponds to dts [semidrive,stream-id =<0x01>]
2 ->playback,3 -> capture, corresponds to dts [semidrive,stream-id =<0x02>]
4 ->playback,5 -> capture, corresponds to dts [semidrive,stream-id =<0x04>]
6 ->playback,7 -> capture, corresponds to dts [semidrive,stream-id =<0x08>]
semidrive,stream-id = (1 << n),n*2 for playback ,n*2+1 for capture */
static ahub_config_t
    g_ahub_stream_config[MAX_STREAM_COUNT * ID_NUM_FOR_STREAM] = {
        [0] = /* media -> slot1~4 */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_PLAYBACK_STREAM_ID1,
                .ahub_endpoint = -1,
                .policy = (void *)&eps_fetch_policy,
            },
        [1] = /* capture -> slot1~4 */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_CAPTURE_STREAM_ID0,
                .ahub_endpoint = -1,
                .policy = (void *)&eps_mix_policy,
            },
        [2] = /* phone ring -> slot1~2 */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_PLAYBACK_STREAM_ID1,
                .ahub_endpoint = AHUB_ENDPOINT_ID4,
                .policy = NULL,
            },
        [4] = /* phone -> slot1~2 */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_PLAYBACK_STREAM_ID1,
                .ahub_endpoint = AHUB_ENDPOINT_ID5,
                .policy = NULL,
            },
        [6] = /* Navi -> slot1~2 */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_PLAYBACK_STREAM_ID1,
                .ahub_endpoint = AHUB_ENDPOINT_ID6,
                .policy = NULL,
            },
        [8] = /* for QNX/linux Cluster */
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_PLAYBACK_STREAM_ID1,
                .ahub_endpoint = AHUB_ENDPOINT_ID1,
                .policy = NULL,
            },
        [9] =
            {
                .vaild_flag = AUIDO_STREAM_VAILD_VALUE,
                .ahub_stream_id = AHUB_CAPTURE_STREAM_ID0,
                .ahub_endpoint = AHUB_ENDPOINT_ID10,
                .policy = NULL,
            },
};

static volatile audio_stream_handler_t g_ahub_stream[MAX_STREAM_COUNT] = {0};
static audio_stream_service_t g_ahub_stream_server;

static int audio_stream_rproc_id[] = {
    AUDIO_RPMSG_RRPOC_ID_AP1,
    AUDIO_RPMSG_RRPOC_ID_AP2,
};

static int ahub_stream_get_stream_support_channels(uint16_t stream_id)
{
    ahub_config_t *config = NULL;
    eps_fetch_policy_t *playback_policy = NULL;
    eps_mix_policy_t *capture_policy = NULL;
    ep_fetch_params_t *playback_params = NULL;
    ep_mix_params_t *capture_params = NULL;
    int index = 0;
    int channels = 0,temp = 0;

    if (stream_id >= (MAX_STREAM_COUNT * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    config = &g_ahub_stream_config[stream_id];
    if (config->vaild_flag != AUIDO_STREAM_VAILD_VALUE) {
        return -1;
    }

    if (config->ahub_endpoint == -1) {
        if (stream_id % ID_NUM_FOR_STREAM ==
            AM_STREAM_TYPE_PLAYBACK) { /* playback */
            playback_policy = (eps_fetch_policy_t *)config->policy;
            for (index = 0;index < playback_policy->eps_count;index++) {
                playback_params = &playback_policy->fetch_policy[index];
                if (playback_params->start_slot +
                    playback_params->slot_len > channels) {
                    channels = playback_params->start_slot +
                               playback_params->slot_len;
                }
            }
        } else {/* capture */
            capture_policy = (eps_mix_policy_t *)config->policy;
            for (index = 0;index < capture_policy->eps_count;index++) {
                capture_params = &capture_policy->mix_policy[index];
                if (capture_params->start_slot +
                    capture_params->slot_len > channels) {
                    channels = capture_params->start_slot +
                               capture_params->slot_len;
                }
            }
        }
        return channels;
    } else {
        return SND_CHANNEL_NUM_STEREO;
    }
}

static int ahub_stream_init(am_stream_dev_t *dev)
{
    if (!dev) {
        return -1;
    }

    dev->private_data = (void *)g_ahub_stream;

    audio_stream_handler_t *ahub_stream =
        (audio_stream_handler_t *)dev->private_data;

    for (int index = 0; index < MAX_STREAM_COUNT; index++) {
        for (int type = 0; type < ID_NUM_FOR_STREAM; type++) {
            ahub_stream->stream[type].stream_id =
                ((type == AM_STREAM_TYPE_PLAYBACK)
                     ? (ID_NUM_FOR_STREAM * index)
                     : (ID_NUM_FOR_STREAM * index + 1));
            ahub_stream->stream[type].running = 0;
            ahub_stream->stream[type].private_data =
                (void *)&g_ahub_stream_config[ahub_stream->stream[type]
                                                  .stream_id];
        }
        ahub_stream->dev = dev;
        ahub_stream++;
    }

    return 0;
}

static int ahub_stream_deinit(am_stream_dev_t *dev) { return 0; }

static int ahub_stream_open(am_stream_dev_t *dev, uint8_t stream_id,
                            void *param)
{
    if (!dev) {
        return -1;
    }

    am_stream_t *stream = (am_stream_t *)param;
    int type = stream_id % ID_NUM_FOR_STREAM;
    int channels = 0;

    audio_stream_handler_t *ahub_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        (stream_id / ID_NUM_FOR_STREAM);

    ahub_config_t *config =
        (ahub_config_t *)ahub_stream->stream[type].private_data;

    ahub_stream->stream[type].phy_buffer_addr =
        ap2p(stream->stream_param.buffer_addr);

    ahub_stream->stream[type].stream_fmt = stream->stream_param.stream_fmt;
    ahub_stream->stream[type].stream_channels =
        stream->stream_param.stream_channels;
    ahub_stream->stream[type].stream_rate = stream->stream_param.stream_rate;

    ahub_stream->stream[type].buffer_size = stream->buffer_size;
    ahub_stream->stream[type].period_size = stream->stream_param.period_size;
    ahub_stream->stream[type].dma_buffer_ptr = 0;

    channels = ahub_stream_get_stream_support_channels(stream_id);

    if (channels <= 0 || stream->stream_param.stream_channels < channels ) {
        ssdk_printf(SSDK_CRIT, "ahub_stream_open stream:%d ahub stream %d failed ! \
        cannot support the channels (%d) or the stream,please check it!\r\n",
        stream_id,config->ahub_stream_id,
        stream->stream_param.stream_channels);
        return -1;
    } else {
        if (AM_STREAM_TYPE_PLAYBACK == type) {
            ahub_open_playback_stream(config->ahub_stream_id);
        } else if (AM_STREAM_TYPE_CAPTURE == type) {
            ahub_open_capture_stream(config->ahub_stream_id);
        }
        ssdk_printf(SSDK_CRIT, "ahub_stream_open stream:%d ahub stream %d\r\n",
            stream_id,config->ahub_stream_id);
    }

    return 0;
}

static int ahub_stream_start(am_stream_dev_t *dev, uint8_t stream_id,
                             void *param)
{
    if (!dev) {
        return -1;
    }

    int type = stream_id % ID_NUM_FOR_STREAM;
    endpoint_params_t ep_params;
    int ret = AHUB_RET_ERR_UNKNOWN;
    audio_stream_handler_t *ahub_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        stream_id / ID_NUM_FOR_STREAM;
    ahub_config_t *config =
        (ahub_config_t *)ahub_stream->stream[type].private_data;
    eps_fetch_policy_t *playback_policy = NULL;
    eps_mix_policy_t *capture_policy = NULL;

    if (config == NULL || config->vaild_flag != AUIDO_STREAM_VAILD_VALUE) {
        return -1;
    }

    if (AM_STREAM_TYPE_PLAYBACK == type) {
        /* Need set endpoint_id as audio hub. */
        if (config->ahub_endpoint == -1) { /* multi endpoint */

            playback_policy = (eps_fetch_policy_t *)config->policy;

            for (int index = 0; index < playback_policy->eps_count; index++) {
                ep_params.endpoint_id =
                    playback_policy->fetch_policy[index].endpoint_id;
                ep_params.channels = SND_CHANNEL_NUM_STEREO;
                  /* ep input should fixed to stereo
                                             */
                ep_params.sample_rate = ahub_stream->stream[type].stream_rate;

                ep_params.sample_bytes = AHUB_SAMPLE_BYTES; /* 16bits */

                ret = ahub_register_ep_to_playback_stream(
                    config->ahub_stream_id, ep_params);
                if (ret != AHUB_RET_OK) {
                    ssdk_printf(
                        SSDK_CRIT,
                        "Error in %s: ahub_register_ep_to_playback_stream "
                        "failed!\n",
                        __func__);
                }
            }
        } else {
            ep_params.endpoint_id = config->ahub_endpoint;
            ep_params.channels = ahub_stream->stream[type].stream_channels;
            ep_params.sample_rate = ahub_stream->stream[type].stream_rate;

            ep_params.sample_bytes = AHUB_SAMPLE_BYTES; /* 16bits */

            ret = ahub_register_ep_to_playback_stream(config->ahub_stream_id,
                                                      ep_params);
            if (ret != AHUB_RET_OK) {
                ssdk_printf(SSDK_CRIT,
                            "Error in %s: ahub_register_ep_to_playback_stream "
                            "failed!\n",
                            __func__);
            }
        }

        ahub_stream->stream[type].running = 1;
        osEventFlagsSet(ahub_stream->stream[type].event,
                        AUDIO_STREAM_EVENT_START);
        ssdk_printf(SSDK_CRIT, " playback ahub_stream_start %d \r\n",
                    stream_id);
    } else if (AM_STREAM_TYPE_CAPTURE == type) {
        /* capture type */
        if (config->ahub_endpoint == -1) { /* multi endpoint */

            capture_policy = (eps_mix_policy_t *)config->policy;

            for (int index = 0; index < capture_policy->eps_count; index++) {
                ep_params.endpoint_id =
                    capture_policy->mix_policy[index].endpoint_id;
                ep_params.channels =
                    SND_CHANNEL_NUM_STEREO; /* ep input should fixed to stereo
                                             */
                ep_params.sample_rate = ahub_stream->stream[type].stream_rate;

                ep_params.sample_bytes = AHUB_SAMPLE_BYTES; /* 16bits */

                ret = ahub_register_ep_to_capture_stream(config->ahub_stream_id,
                                                         ep_params);
                if (ret != AHUB_RET_OK) {
                    ssdk_printf(
                        SSDK_CRIT,
                        "Error in %s: ahub_register_ep_to_playback_stream "
                        "failed!\n",
                        __func__);
                }
            }
        } else {
            ep_params.endpoint_id = config->ahub_endpoint;
            ep_params.channels =
                SND_CHANNEL_NUM_STEREO; // ep input should fixed to stereo
            ep_params.sample_rate = ahub_stream->stream[type].stream_rate;

            ep_params.sample_bytes = AHUB_SAMPLE_BYTES; // 16bits

            ret = ahub_register_ep_to_capture_stream(config->ahub_stream_id,
                                                     ep_params);
            if (ret != AHUB_RET_OK) {
                ssdk_printf(SSDK_CRIT,
                            "Error in %s: ahub_register_ep_to_playback_stream "
                            "failed!\n",
                            __func__);
            }
        }

        ahub_stream->stream[type].running = 1;
        osEventFlagsSet(ahub_stream->stream[type].event,
                        AUDIO_STREAM_EVENT_START);
        ssdk_printf(SSDK_CRIT, " capture ahub_stream_start %d \r\n", stream_id);
    }

    return 0;
}

static int ahub_stream_stop(am_stream_dev_t *dev, uint8_t stream_id,
                            void *param)
{
    if (!dev) {
        return -1;
    }

    int type = stream_id % ID_NUM_FOR_STREAM;
    audio_stream_handler_t *ahub_stream =
        ((audio_stream_handler_t *)(dev->private_data)) +
        stream_id / ID_NUM_FOR_STREAM;
    ahub_config_t *config =
        (ahub_config_t *)ahub_stream->stream[type].private_data;
    eps_fetch_policy_t *playback_policy = NULL;
    eps_mix_policy_t *capture_policy = NULL;

    if (AM_STREAM_TYPE_PLAYBACK == type) {
        if (config->ahub_endpoint == -1) { /* multi endpoint */

            playback_policy = (eps_fetch_policy_t *)config->policy;

            for (int index = 0; index < playback_policy->eps_count; index++) {
                ahub_unregister_ep_from_playback_stream(
                    config->ahub_stream_id,
                    playback_policy->fetch_policy[index].endpoint_id);
            }
        } else {
            ahub_unregister_ep_from_playback_stream(config->ahub_stream_id,
                                                    config->ahub_endpoint);
        }
    } else {
        if (config->ahub_endpoint == -1) { /* multi endpoint */

            playback_policy = (eps_fetch_policy_t *)config->policy;

            for (int index = 0; index < playback_policy->eps_count; index++) {
                ahub_unregister_ep_from_capture_stream(
                    config->ahub_stream_id,
                    playback_policy->fetch_policy[index].endpoint_id);
            }
        } else {
            ahub_unregister_ep_from_capture_stream(config->ahub_stream_id,
                                                   config->ahub_endpoint);
        }
    }

    ahub_stream->stream[type].running = 0;

    return 0;
}

static int ahub_stream_send_msg(int stream_index, char *msg_buf,
                                uint16_t msg_len)
{
#define AUDIO_STREAM_RPC_TIMEOUT 500
    int ret = 0;
    audio_stream_service_t *service = &g_ahub_stream_server;
    uint8_t index = g_ahub_stream[stream_index / ID_NUM_FOR_STREAM].dcf_id;

    ret =
        rpmsg_channel_send(service->device[index].chan, AUDIO_STREAM_SERV_EPT,
                           (void *)msg_buf, msg_len, AUDIO_STREAM_RPC_TIMEOUT);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

am_stream_dev_t ahub_stream_dev = {
    .stream_cnt = MAX_STREAM_COUNT,
    .ops =
        {
            .init = ahub_stream_init,
            .open = ahub_stream_open,
            .start = ahub_stream_start,
            .stop = ahub_stream_stop,
            .pause = NULL,
            .suspend = NULL,
            .resume = NULL,
            .deinit = ahub_stream_deinit,
            .send_msg = ahub_stream_send_msg,
        },
};

static void ahub_eps_rx_task(void *arg)
{
    uint32_t stream_id = (uint32_t)arg;

    audio_stream_handler_t *ahub_stream =
        &g_ahub_stream[stream_id / ID_NUM_FOR_STREAM];
    am_stream_dev_t *dev = (am_stream_dev_t *)ahub_stream->dev;
    uint8_t *out;
    ahub_config_t *config =
        (ahub_config_t *)ahub_stream->stream[AM_STREAM_TYPE_CAPTURE]
            .private_data;
    eps_mix_policy_t *capture_policy = NULL;

    while (1) {
        if (ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].running == 0) {
            osEventFlagsWait(ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].event,
                             AUDIO_STREAM_EVENT_START, osFlagsWaitAll,
                             osWaitForever);

            continue;
        }

        out = (uint8_t *)(ahub_stream->stream[AM_STREAM_TYPE_CAPTURE]
                              .phy_buffer_addr +
                          ahub_stream->stream[AM_STREAM_TYPE_CAPTURE]
                              .dma_buffer_ptr);
        if (config->ahub_endpoint == -1) { /* multi endpoint */
            capture_policy = (eps_mix_policy_t *)config->policy;
            if (ahub_read_from_eps(
                    *capture_policy, out,
                    ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size,
                    ahub_stream->stream[AM_STREAM_TYPE_CAPTURE]
                        .stream_channels) != AHUB_RET_OK) {
                ssdk_printf(
                    SSDK_CRIT, "ahub_read_from_eps--error %d \n",
                    ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size);
                osDelay(100);
            }
        } else {
            if (ahub_read_from_ep(
                    config->ahub_endpoint, out,
                    ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size) !=
                AHUB_RET_OK) {
                ssdk_printf(
                    SSDK_CRIT, "ahub_read_from_ep--error %d\n",
                    ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size);
                osDelay(100);
            }
        }

        arch_clean_cache_range(
            (uint32_t)out,
            ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size);

        ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr +=
            ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].period_size;
        if (ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr >=
            ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].buffer_size) {
            ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].dma_buffer_ptr = 0;
        }
        if (ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].running != 0) {
            am_stream_do_rpc_message(
                dev, AM_STREAM_REQUEST,
                ahub_stream->stream[AM_STREAM_TYPE_CAPTURE].stream_id, NULL,
                AM_STREAM_MODE_SERVER);
        }
            osDelay(1);

    }
}

static void ahub_eps_tx_task(void *arg)
{
    uint32_t stream_id = (uint32_t)arg;
    audio_stream_handler_t *ahub_stream =
        &g_ahub_stream[stream_id / ID_NUM_FOR_STREAM];
    am_stream_dev_t *dev = (am_stream_dev_t *)ahub_stream->dev;
    uint8_t *in;

    ahub_config_t *config =
        (ahub_config_t *)ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK]
            .private_data;
    eps_fetch_policy_t *playback_policy = NULL;

    while (1) {
        if (ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].running == 0) {
            osEventFlagsWait(ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].event,
                             AUDIO_STREAM_EVENT_START, osFlagsWaitAll,
                             osWaitForever);
            continue;
        }

        in = (uint8_t *)(ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK]
                             .phy_buffer_addr +
                         ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK]
                             .dma_buffer_ptr);

        arch_invalidate_cache_range(
            (uint32_t)in,
            ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);

        if (config->ahub_endpoint == -1) { /* multi endpoint */
            playback_policy = (eps_fetch_policy_t *)config->policy;
            if (ahub_write_to_eps(
                    *playback_policy, in,
                    ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size,
                    ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK]
                        .stream_channels) != AHUB_RET_OK) {
                ssdk_printf(
                    SSDK_CRIT, "ahub_write_to_eps--error %d\n",
                    ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);
                osDelay(100);
            }
        } else {
            if (ahub_write_to_ep(
                    config->ahub_endpoint, in,
                    ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size) !=
                AHUB_RET_OK) {
                ssdk_printf(
                    SSDK_CRIT, "ahub_write_to_ep--error %d\n",
                    ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);
                osDelay(100);
            }
        }

        memset(in, 0, ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size);

        ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr +=
            ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].period_size;
        if (ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr >=
            ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].buffer_size) {
            ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].dma_buffer_ptr = 0;
        }
        if (ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].running != 0) {
            am_stream_do_rpc_message(
                dev, AM_STREAM_REQUEST,
                ahub_stream->stream[AM_STREAM_TYPE_PLAYBACK].stream_id, NULL,
                AM_STREAM_MODE_SERVER);
        }
        osDelay(1);
    }
}

static void ahub_stream_rpc_task(void *arg)
{
    struct am_stream_ipcc_data pack;
    am_stream_ctrl_t *am_ctrl = NULL;
    audio_stream_service_t *service = (audio_stream_service_t *)arg;
    osStatus_t ret;

    am_stream_register_platform(&ahub_stream_dev);

    while (1) {
        ret = osMessageQueueGet(service->audio_stream_queue, &pack, NULL,
                                osWaitForever);

        if (ret == osOK) {

            if (pack.cmd == OP_PCM_STREAM_REQ) {

                am_ctrl = (am_stream_ctrl_t *)&pack.data[0];
                // x9 hp client type
                g_ahub_stream[am_ctrl->stream_id / ID_NUM_FOR_STREAM].dcf_id =
                    pack.dcf_id;
                am_stream_do_rpc_message(&ahub_stream_dev, am_ctrl->stream_cmd,
                                         am_ctrl->stream_id, &pack.data[0],
                                         AM_STREAM_MODE_SERVER);
            }
        }
    }
}

static int ahub_stream_event_handle(void *data, int len, unsigned long src,
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
    audio_stream_service_t *service = &g_ahub_stream_server;
    struct rpmsg_instance *dev;
    struct rpmsg_channe *chan;
    char task_name[AUDIO_TASK_NAME_LEN] = {0};

    osThreadAttr_t audio_stream_task_attr = {
        .name = "ahub_stream_task",
        .stack_size = 4096,
        .priority = osPriorityAboveNormal,
    };

    if (service->initialized) {
        return;
    }

    service->initialized = true;

    for (int index = 0; index < ARRAY_SIZE(audio_stream_rproc_id); index++) {
        /* get different AUDIO_RPMSG_RRPOC_ID_AP 1/2 */
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

        rpmsg_channel_start(chan, ahub_stream_event_handle,
                            (void *)&service->device[index]);
    }

    service->audio_stream_queue = osMessageQueueNew(
        AUDIO_STREAM_MAX_QUEUE_COUNT, sizeof(struct am_stream_ipcc_data), NULL);
    if (service->audio_stream_queue == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create queue failed.\r\n", __func__);
        return;
    }

    service->stream_handler = osThreadNew(ahub_stream_rpc_task, (void *)service,
                                          &audio_stream_task_attr);

    if (service->stream_handler == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        osMessageQueueDelete(service->audio_stream_queue);
        return;
    }
    /* audio hub task create */

    osThreadAttr_t audio_ep_task_attr = {
        .name = "ahub_ep_task",
        .stack_size = 8192 * 2,
        .priority = osPriorityAboveNormal,
    };

    for (int index = 0; index < ARRAY_SIZE(g_ahub_stream_config); index++) {
        if (g_ahub_stream_config[index].vaild_flag ==
            AUIDO_STREAM_VAILD_VALUE) {
            if (g_ahub_stream_config[index].ahub_stream_id <
                AHUB_CAPTURE_STREAM_ID0) { /* playback task */
                snprintf(task_name, sizeof(task_name), "ahub_ep_tx_task_%d",
                         index);
                audio_ep_task_attr.name = task_name;
                g_ahub_stream[index / ID_NUM_FOR_STREAM]
                    .stream[AM_STREAM_TYPE_PLAYBACK]
                    .event = osEventFlagsNew(NULL);
                if (NULL == osThreadNew(ahub_eps_tx_task, (void *)index,
                                        &audio_ep_task_attr)) {
                    osEventFlagsDelete(g_ahub_stream[index / ID_NUM_FOR_STREAM]
                                           .stream[AM_STREAM_TYPE_PLAYBACK]
                                           .event);
                    ssdk_printf(SSDK_ALERT,
                                "%s: create eps tx thread(%d) failed.\r\n",
                                __func__, index);
                }
            } else { /* capture task */
                snprintf(task_name, sizeof(task_name), "ahub_ep_rx_task_%d",
                         index);
                audio_ep_task_attr.name = task_name;
                g_ahub_stream[index / ID_NUM_FOR_STREAM]
                    .stream[AM_STREAM_TYPE_CAPTURE]
                    .event = osEventFlagsNew(NULL);
                if (NULL == osThreadNew(ahub_eps_rx_task, (void *)index,
                                        &audio_ep_task_attr)) {
                    osEventFlagsDelete(g_ahub_stream[index / ID_NUM_FOR_STREAM]
                                           .stream[AM_STREAM_TYPE_CAPTURE]
                                           .event);
                    ssdk_printf(SSDK_ALERT,
                                "%s: create eps rx thread(%d) failed.\r\n",
                                __func__, index);
                }
            }
        }
    }
}
