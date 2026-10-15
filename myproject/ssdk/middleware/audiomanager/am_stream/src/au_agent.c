/**
 * @file au_agent.c
 * @brief audio agent source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "au_agent.h"

#include "driver/snd_driver.h"
#include "param.h"
#include "res_loader.h"

#define DMA_MAX (65536)
#define DMA_PERIOD_LEN (4096)
#define DMA_PERIOD_CNT (4)
#define AGENT_TIMEOUT 1000
#define AM_AGENT_DBG_PRT SSDK_DEBUG

#ifdef CONFIG_CDNS_I2S_SC

#define SND_AGENT_XFER_COMPLETE (0x1u)

typedef struct snd_agent {
    snd_dev_t *dev;
    osEventFlagsId_t event;
    osThreadId_t thread;
    bool running;
    uint8_t *pcm_data;
    int32_t pcm_size;
    int32_t pcm_ptr;
    snd_params_t *params;
} snd_agent_t;

extern snd_dev_t *i2s_sc1_dev;
static snd_agent_t agent_player;

static snd_params_t pcm_param = {
    .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
    .channels = SND_CHANNEL_NUM_STEREO,
    .sample_width = SND_SAMPLE_WIDTH_16BITS,
    .sample_rate = SND_SR_48000,
    .period_size = DMA_PERIOD_LEN,
    .period_count = DMA_PERIOD_CNT,
};
#endif

static au_agent_t au_agent;

static int au_agent_do_action(au_agent_t *agent);

void audio_agent_task(void *token)
{
    au_agent_t *agent = (au_agent_t *)token;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s: ####audio agent running #####\n",
                __func__);

    for (;;) {
        osEventFlagsWait(agent->agent_start_event, 1, osFlagsWaitAny,
                         osWaitForever);
        ssdk_printf(AM_AGENT_DBG_PRT, "%s: audioagent rec op_code =%x\n",
                    __func__, agent->op_code);
        au_agent_do_action(agent);
        osEventFlagsSet(agent->agent_done_event, 1);
    }
}

bool au_agent_init(void)
{
    bool ret = true;
    au_agent_t *agent = &au_agent;
    osThreadAttr_t attr = {.name = "audio_agent",
                           .stack_size = 4096,
                           .priority = osPriorityNormal};

    memset(agent, 0, sizeof(au_agent_t));
    agent->agent_start_event = osEventFlagsNew(NULL);
    agent->agent_done_event = osEventFlagsNew(NULL);

    agent->status = AU_AGENT_ST_IDLE;
    agent->pcm_data = NULL;
    agent->i2s_handle = NULL;
    agent->thread = osThreadNew(audio_agent_task, agent, &attr);

    if (agent->thread == NULL) {
        osEventFlagsDelete(agent->agent_done_event);
        osEventFlagsDelete(agent->agent_start_event);
        ret = false;
        ssdk_printf(SSDK_ERR, "%s: ####agent task creat faild #####\n",
                    __func__);
    }

    return ret;
}

bool au_agent_open(au_agent_handle_t *handle, uint32_t res_id)
{
    au_agent_t *agent = &au_agent;
    *handle = (au_agent_handle_t)agent;

    return true;
}

bool au_agent_close(au_agent_handle_t handle)
{
    handle = NULL;

    return true;
}

bool au_agent_operation(au_agent_handle_t handle,
                        au_agent_params_t *agent_params)
{
    int ret = 0;
    bool result = true;

    if ((!handle) || (!agent_params)) {
        ssdk_printf(SSDK_ERR, "%s: error handle or agent_params is null\n",
                    __func__);
        result = false;
        return result;
    }

    au_agent_t *agent = (au_agent_t *)handle;
    agent->op_code = agent_params->op_code;
    agent->params = agent_params;

    osEventFlagsSet(agent->agent_start_event, 1);
    ret = osEventFlagsWait(agent->agent_done_event, 1, osFlagsWaitAny,
                           AGENT_TIMEOUT);

    if (osErrorTimeout == ret) {
        ssdk_printf(SSDK_ERR, "%s: agent timeout error\n", __func__);
        result = false;
    }

    if (agent->result_error) {
        ssdk_printf(SSDK_ERR, "%s: error:result_error=%d\n", __func__,
                    agent->result_error);
        result = false;
    }

    return result;
}

static int pcm_prepare(au_agent_t *agent)
{
    int ret = 0;
    int size = 0;
    au_agent_params_t *agent_params;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s:\n", __func__);

    if (AU_AGENT_ST_IDLE != agent->status) {
        ssdk_printf(SSDK_ERR,
                    "%s: error:agent need be IDLE,shuold do stop it  now=%d\n",
                    __func__, agent->status);
        ret = -1;
        return ret;
    }

    agent_params = agent->params;
    strncpy(agent->path, agent_params->path, AU_AGENT_PATH_SIZE);
    agent->priority = agent_params->priority;
    size = res_size(agent->path);

    if (size < 0) {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s: error:agent->pcm_data is NULL \n", __func__);
        return ret;
    }

    agent->pcm_size = size;
    agent->pcm_data = (uint8_t *)memalign(32, ROUNDUP(size, 32));

    ssdk_printf(AM_AGENT_DBG_PRT, "%s: agent->pcm_size %d \n", __func__,
                agent->pcm_size);

    if (!agent->pcm_data) {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s: error:agent->pcm_data is NULL \n", __func__);
        return ret;
    }

    ret = res_load(agent->path, agent->pcm_data, ROUNDUP(size, 32), 0);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "can't load audio res!\n");
        free(agent->pcm_data);
        agent->pcm_data = NULL;
        ret = -1;
        return ret;
    }

    agent->status = AU_AGENT_ST_PREPARE;

    return ret;
}

static int pcm_realse(au_agent_t *agent)
{
    int ret = 0;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s:\n", __func__);

    if (AU_AGENT_ST_STOP != agent->status) {
        ssdk_printf(SSDK_ERR,
                    "%s: error:agent need be in AGENT_ST_STOP  now=%d\n",
                    __func__, agent->status);
        ret = -1;
        return ret;
    }

    if (agent->pcm_data) {
        free(agent->pcm_data);
    }

    agent->pcm_data = NULL;
    agent->status = AU_AGENT_ST_IDLE;

    return ret;
}

static int pcm_reset(au_agent_t *agent)
{
    int ret = 0;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s:\n", __func__);

    if (agent->pcm_data) {
        free(agent->pcm_data);
    }

    agent->pcm_data = NULL;
    agent->status = AU_AGENT_ST_IDLE;

    return ret;
}

au_agent_status pcm_getstatus(void)
{
    au_agent_t *agent = &au_agent;

    return agent->status;
}

#ifdef CONFIG_CDNS_I2S_SC
static void player_callback(void *context, snd_state_t state)
{
    snd_agent_t *player = (snd_agent_t *)context;

    if (player->event) {
        osEventFlagsSet(player->event, SND_AGENT_XFER_COMPLETE);
    }
}

static void agent_player_thread(void *pcArg)
{
    snd_agent_t *player = (snd_agent_t *)pcArg;
    snd_params_t *params = player->params;
    snd_dev_t *dev = player->dev;
    int len;
    int32_t temp = 0;

    while (player->running) {
        osEventFlagsWait(player->event, SND_AGENT_XFER_COMPLETE, osFlagsWaitAll,
                         osWaitForever);

        temp = player->pcm_size - player->pcm_ptr;

        if (temp >= params->period_size) {
            len = snd_write(dev, player->pcm_data + player->pcm_ptr,
                            params->period_size);
            player->pcm_ptr += len;
        } else {
            len = snd_write(dev, player->pcm_data + player->pcm_ptr, temp);
            player->pcm_ptr = 0;
            len = snd_write(dev, player->pcm_data + player->pcm_ptr,
                            params->period_size - temp);
            player->pcm_ptr += params->period_size;
        }
    }

    osEventFlagsDelete(player->event);
    osThreadExit();
    player->event = NULL;
    player->thread = NULL;
}

static void player_init(snd_agent_t *player)
{
    osThreadAttr_t attr = {
        .name = "agent_play_thread",
        .stack_size = 4096,
        .priority = osPriorityNormal1,
    };

    player->running = true;
    player->event = osEventFlagsNew(NULL);
    player->pcm_ptr = 0;
    player->thread = osThreadNew(agent_player_thread, player, &attr);
}
#endif

static int pcm_start(au_agent_t *agent)
{
    int ret = 0;
#ifdef CONFIG_CDNS_I2S_SC
    snd_agent_t *player = &agent_player;
    snd_dev_t *dev = NULL;
    int len = 0;

    dev = player->dev = i2s_sc1_dev;
    player->pcm_data = agent->pcm_data;
    player->pcm_size = agent->pcm_size;
    player->params = &pcm_param;
    player_init(player);

    if (!snd_open(dev, &pcm_param, player_callback, (void *)player,
                  SND_STREAM_PLAYBACK)) {
        ret = -1;
        goto ERR_HANDLE2;
    }

    len = snd_write(dev, player->pcm_data + player->pcm_ptr, agent->pcm_size);
    if (len < 0) {
        ret = -1;
        goto ERR_HANDLE1;
    }

    agent->i2s_handle = dev;
    player->pcm_ptr += len;

    if (!snd_start(dev, SND_STREAM_PLAYBACK)) {
        ret = -1;
        goto ERR_HANDLE1;
    }

    agent->status = AU_AGENT_ST_RUNNING;

    return ret;

ERR_HANDLE1:
    snd_close(dev, SND_STREAM_PLAYBACK);
ERR_HANDLE2:
    agent->i2s_handle = NULL;
    pcm_reset(agent);
#endif
    return ret;
}

static int pcm_stop(au_agent_t *agent)
{
    int ret = 0;

    agent_player.running = false;

    if (agent->i2s_handle) {
        snd_stop((snd_dev_t *)agent->i2s_handle, SND_STREAM_PLAYBACK);
        snd_close((snd_dev_t *)agent->i2s_handle, SND_STREAM_PLAYBACK);
    }

    agent->i2s_handle = NULL;
    agent->status = AU_AGENT_ST_STOP;

    return ret;
}

static int au_agent_do_action(au_agent_t *agent)
{
    int ret = 0;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s:\n", __func__);

    switch (agent->op_code) {
    case AU_AGENT_OP_START:
        ret = pcm_prepare(agent);
        ret = pcm_start(agent);
        break;
    case AU_AGENT_OP_STOP:
        ret = pcm_stop(agent);
        ret = pcm_realse(agent);
        break;
    default:
        ssdk_printf(SSDK_ERR, "%s: error: not support op_code=%d\n", __func__,
                    agent->op_code);
        ret = -1;
    }

    agent->params = NULL;
    agent->result_error = ret;

    return ret;
}