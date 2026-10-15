/**
 * @file au_rpc_svc.h
 * @brief audio service header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_RPC_SVC_H__
#define __AU_RPC_SVC_H__

#include <rpmsg.h>
#include <rpmsg_channel.h>
#include <rpmsg_echo.h>
#include <rpmsg_service.h>
#include <stdint.h>
#include <stdio.h>

#ifdef CONFIG_AUDIO_MANAGER
#include "am_api.h"
#include "au_agent.h"
#endif

#define AUDIO_RPC_EPT_NUM 2

#define AUDIO_RPMSG_RRPOC_ID_AP1 3
#define AUDIO_RPMSG_RRPOC_ID_AP2 4

#define AUDIO_RPMSG_TYPE RPMSG_TYPE_IPCC
#define AUDIO_RPMSG_NAME "audio_monitor"
#define AUDIO_RPMSG_SERV_EPT (125)
#define AUDIO_RPC_LEN_MAX 64

#define AUDIO_MAX_QUEUE_COUNT 100

typedef enum {
    OP_HANDSHAKE,
    OP_START,
    OP_STOP,
    OP_SET_MUTE,
    OP_GET_MUTE,
    OP_SETVOL,
    OP_SWITCH,
    OP_RESET,
    OP_RESTORE,
    OP_PARAM,
    OP_PLAY_AGENT_START,
    OP_PLAY_AGENT_STOP,
    OP_PCM_PLAYBACK_CTL,
    OP_PCM_CAPTURE_CTL,
    OP_PCM_LOOPBACK_CTL,
    OP_PCM_STREAM_REQ, /* the same to AM_OP_PCM_STREAM_REQ */
    OP_PCM_STREAM_IND,
    OP_NUMB
} AM_OPCODE_TYPE;

struct audio_rpc_data {
    char data[AUDIO_RPC_LEN_MAX];
    int32_t data_len;
    char data1[AUDIO_RPC_LEN_MAX];
    int32_t data1_len;
    uint8_t cmd;
    uint8_t param;
    uint8_t dcf_id;
    uint8_t reserved1;
} __attribute__((packed));

struct audio_rpc_ret {
    int32_t code;
    uint8_t cmd;
    uint8_t type;
    uint8_t reserved0;
    uint8_t reserved1;
} __attribute__((packed));

struct audio_rpc_dev {
    struct rpmsg_instance *dev;
    struct rpmsg_channel *chan;
    uint8_t device_index;
};

typedef struct au_rpc_service {
    struct audio_rpc_dev device[AUDIO_RPC_EPT_NUM];
    osThreadId_t *thread;
    osMutexId_t mutex;
    osMessageQueueId_t queue;
    void* audio_handle;

    bool initialized;
} au_rpc_service_t;

void au_rpc_service_init(void);

#endif