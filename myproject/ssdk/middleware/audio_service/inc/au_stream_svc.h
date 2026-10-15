/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __AUDIO_STREAM_SERVICE_H__
#define __AUDIO_STREAM_SERVICE_H__

#include <stdlib.h>
#include <sys/types.h>

#include "am_stream.h"
#include "au_capture.h"
#include "au_mixer.h"

#define AUDIO_STREAM_RPMSG_NAME "safety,vi2s"
#define AUDIO_STREAM_SERV_EPT (130)
#define AUDIO_STREAM_RPMSG_TYPE RPMSG_TYPE_IPCC
#define AUDIO_STREAM_EPT_NUM 2
#define AUDIO_TASK_NAME_LEN 30
#define AUDIO_STREAM_EVENT_START 0x01
#define AUIDO_STREAM_VAILD_VALUE 0xA5
#define AUDIO_STREAM_MAX_QUEUE_COUNT 400
#define MAX_STREAM_COUNT 16

typedef struct {
    uint32_t stream_id;
    uint8_t stream_fmt;
    uint8_t stream_channels;
    uint16_t stream_rate;
    pcm_stream_handle_t pcm_stream_ptr;
    struct au_source source_ptr;
    osEventFlagsId_t event;
    uint32_t volume;
    uint32_t phy_buffer_addr;
    uint32_t period_size;
    uint32_t buffer_size;
    uint32_t dma_buffer_ptr;
    uint32_t running;
    void *private_data;
} audio_stream_config_t;

typedef struct {
    audio_stream_config_t stream[ID_NUM_FOR_STREAM];
    uint8_t dcf_id;
    am_stream_dev_t *dev;
} audio_stream_handler_t;

struct audio_stream_rpc_dev {
    struct rpmsg_instance *dev;
    struct rpmsg_channel *chan;
    uint8_t device_index;
};

typedef struct audio_stream_service {
    struct audio_stream_rpc_dev device[AUDIO_STREAM_EPT_NUM];
    osMessageQueueId_t audio_stream_queue;
    osThreadId_t *stream_handler;

    bool initialized;
} audio_stream_service_t;

void audio_stream_service_init(void);

#endif
