/**
 * @file au_rpc_svc.c
 * @brief audio service file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <lib/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_rpc_svc.h"

static int audio_rproc_id[] = {
    AUDIO_RPMSG_RRPOC_ID_AP1,
    AUDIO_RPMSG_RRPOC_ID_AP2,
};

static au_rpc_service_t g_au_rpc_serv = {0};

static int audio_rpc_send_msg(struct rpmsg_channel *chan, char *buf, int len)
{
#define AUDIO_RPC_TIMEOUT 500
    int ret = 0;

    ret = rpmsg_channel_send(chan, AUDIO_RPMSG_SERV_EPT, (void *)buf, len,
                             AUDIO_RPC_TIMEOUT);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static int audio_event_handle(void *data, int len, unsigned long src, void *arg)
{
    osStatus_t ret;
    struct audio_rpc_dev *rpc_dev = (struct audio_rpc_dev *)arg;
    struct audio_rpc_data *rpc_data = (struct audio_rpc_data *)data;
    au_rpc_service_t *service =
        containerof(rpc_dev, au_rpc_service_t, device[rpc_dev->device_index]);

    rpc_data->dcf_id = rpc_dev->device_index;

    ret = osMessageQueuePut(service->queue, rpc_data, 0, 0);

    if (ret == osOK) {
        return 0;
    } else {
        ssdk_printf(SSDK_ERR, "%s: message queue put failed.\r\n", __func__);
        return -1;
    }
}

static void audio_rpc_task(void *arg)
{
    osStatus_t ret;
    struct audio_rpc_data rpc_data;
    struct audio_rpc_ret rpc_ret;
    bool mute;
    au_agent_params_t agent_params;
    au_rpc_service_t *service = (au_rpc_service_t *)arg;

    while (1) {
        ret = osMessageQueueGet(service->queue, &rpc_data, NULL, osWaitForever);

        osMutexAcquire(service->mutex, osWaitForever);

        if (ret == osOK) {
            switch (rpc_data.cmd) {
            case OP_START:
                rpc_ret.code = am_start_path(rpc_data.data, rpc_data.param);
                ssdk_printf(SSDK_ERR, "audio start : (%s),vol : %d\r\n",
                            rpc_data.data, rpc_data.param);
                break;
            case OP_STOP:
                rpc_ret.code = am_stop_path(rpc_data.data);
                ssdk_printf(SSDK_ERR, "audio stop : (%s)\r\n", rpc_data.data);
                break;
            case OP_SETVOL:
                rpc_ret.code = am_set_path_vol(rpc_data.data, rpc_data.param);
                ssdk_printf(SSDK_ERR, "audio setvol : (%s)\r\n", rpc_data.data);
                break;
            case OP_SET_MUTE:
                am_mute_path(rpc_data.data, rpc_data.param);
                ssdk_printf(SSDK_ERR, "audio setmute : (%s)\r\n",
                            rpc_data.data);
                break;
            case OP_GET_MUTE:
                am_get_path_mute_stat(rpc_data.data, &mute);
                rpc_ret.code = (mute == true) ? 1 : 0;
                ssdk_printf(SSDK_ERR, "audio getmute : (%s) - %d\r\n",
                            rpc_data.data, rpc_ret.code);
                break;
            case OP_PLAY_AGENT_START:
                agent_params.op_code = AU_AGENT_OP_START;
                strncpy(agent_params.path, rpc_data.data, AU_AGENT_PATH_SIZE);
                agent_params.priority = 1;
                au_agent_operation(service->audio_handle, &agent_params);
                ssdk_printf(SSDK_ERR, "audio agent start (%s)\r\n",
                            agent_params.path);
                break;
            case OP_PLAY_AGENT_STOP:
                agent_params.op_code = AU_AGENT_OP_STOP;
                au_agent_operation(service->audio_handle, &agent_params);
                ssdk_printf(SSDK_ERR, "audio agent stop\r\n");
                break;
            case OP_SWITCH:
                rpc_ret.code = am_switch_path(rpc_data.data, rpc_data.data1);
                ssdk_printf(SSDK_ERR, "audio switch %s -> %s \r\n",
                            rpc_data.data, rpc_data.data1);
                break;
            case OP_PARAM:
                rpc_ret.code = am_send_path_param(
                    rpc_data.data, rpc_data.param, rpc_data.data1_len,
                    (unsigned char *)rpc_data.data1);
                ssdk_printf(SSDK_ERR, "audio set param\r\n");
                break;
            default:
                rpc_ret.code = -1;
                ssdk_printf(SSDK_ERR, "audio unknow cmd\r\n");
                break;
            }

            audio_rpc_send_msg(service->device[rpc_data.dcf_id].chan,
                               (char *)&rpc_ret, sizeof(struct audio_rpc_ret));
        }

        osMutexRelease(service->mutex);
    }
}

void au_rpc_service_init(void)
{
    struct rpmsg_instance *dev;
    struct rpmsg_channe *chan;
    au_rpc_service_t *service = &g_au_rpc_serv;
    osThreadAttr_t audio_task_attr = {
        .name = "audio_sound_task",
        .stack_size = 4096,
        .priority = osPriorityNormal2,
    };

    if (service->initialized) {
        return;
    }

    service->initialized = true;

    for (int index = 0; index < ARRAY_SIZE(audio_rproc_id); index++) {
        dev = rpmsg_get_instance(AUDIO_RPMSG_TYPE, audio_rproc_id[index]);
        if (!dev) {
            ssdk_printf(SSDK_ALERT, "audio rpmsg create instance failed\r\n");
            return;
        }

        chan =
            rpmsg_channel_create(dev, AUDIO_RPMSG_NAME, AUDIO_RPMSG_SERV_EPT);
        if (!chan) {
            ssdk_printf(SSDK_ALERT, "audio rpmsg create chan failed\r\n");
            return;
        }

        service->device[index].dev = dev;
        service->device[index].chan = (struct rpmsg_channel *)chan;
        service->device[index].device_index = index;

        rpmsg_channel_start(chan, audio_event_handle,
                            (void *)&service->device[index]);
    }

    au_agent_init();
    au_agent_open(&service->audio_handle, 0);

    service->queue = osMessageQueueNew(AUDIO_MAX_QUEUE_COUNT,
                                       sizeof(struct audio_rpc_data), NULL);
    if (service->queue == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create queue failed.\r\n", __func__);
        return;
    }

    service->mutex = osMutexNew(NULL);

    if (service->mutex == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create mutex failed.\r\n", __func__);
        osMessageQueueDelete(service->queue);
        return;
    }

    service->thread =
        osThreadNew(audio_rpc_task, (void *)service, &audio_task_attr);
    if (service->thread == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        osMutexDelete(service->mutex);
        osMessageQueueDelete(service->queue);
        return;
    }
}