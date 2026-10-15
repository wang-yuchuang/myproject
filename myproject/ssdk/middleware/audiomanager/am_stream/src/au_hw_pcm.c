/**
 * @file au_hw_pcm.c
 * @brief audio hardware pcm source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <container.h>
#include <debug.h>
#include <malloc.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_hw_pcm.h"

#include "driver/dma.h"
#include "driver/snd_driver.h"
#include "sd_audio.h"

#define AM_MIX_DBG_PRT SSDK_DEBUG
#define AM_MIX_ERR_PRT SSDK_ERR

typedef struct {
    void *hw_pcm_handler;
    uint8_t pcm_status[2];
} au_hw_pcm_instance_t;

static au_hw_pcm_instance_t pcm_intance = {0};
extern snd_dev_t *i2s_sc1_dev;

static snd_params_t pcm_param = {
    .fmt = SND_DAIFMT_STANDARD_PHILLIPS | SND_DAIFMT_CODEC_MASTER,
    .channels = SND_CHANNEL_NUM_STEREO,
    .sample_width = SND_SAMPLE_WIDTH_16BITS,
    .sample_rate = SND_SR_48000,
    .period_size = DMA_PERIOD_BUFLEN,
    .period_count = DMA_PERIOD_COUNT,
};

static void hw_pcm_callback(void *context, snd_state_t state)
{
    au_hw_pcm_instance_t *instance = (au_hw_pcm_instance_t *)context;
    au_hw_pcm_t *hw_pcm = instance->hw_pcm_handler;

    hw_pcm->hw_callback(hw_pcm->cb_param);
}

int hw_pcm_open(au_hw_device_t *hw_device, hw_device_callback hw_callback,
                void *cb_param)
{
    int ret = 0;
    snd_dev_t *dev = i2s_sc1_dev;
    au_hw_pcm_t *hw_pcm;

    ssdk_printf(AM_MIX_DBG_PRT, "%s:\n", __func__);

    if (hw_device->status != AU_HWD_ST_IDLE) {
        ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__,
                    __LINE__, hw_device->status);
        return ret;
    }

    hw_pcm = (au_hw_pcm_t *)malloc(sizeof(au_hw_pcm_t));

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s: error:hw_pcmis null \n", __func__);
        hw_device->private_data = NULL;
        return -1;
    }

    memset(hw_pcm, 0x00, sizeof(au_hw_pcm_t));
    hw_device->private_data = hw_pcm;

    hw_pcm->hw_callback = hw_callback;
    hw_pcm->cb_param = cb_param;
    pcm_intance.hw_pcm_handler = hw_pcm;

    if (!snd_open(dev, &pcm_param, hw_pcm_callback, (void *)&pcm_intance,
                  hw_device->type)) {
        ssdk_printf(AM_MIX_ERR_PRT, "error:i2s resource get failed.\n");
        ret = -1;
        goto ERR_HANDLE1;
    }

    hw_pcm->i2s_handle = dev;

    ssdk_printf(AM_MIX_DBG_PRT, "%s:au_hw_pm.........\n", __func__);

    pcm_intance.pcm_status[hw_device->type] = AU_HW_PCM_STATUS_OPENED;

    hw_device->status = AU_HWD_ST_OPENED;
    ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__, __LINE__,
                hw_device->status);

    return ret;

ERR_HANDLE1:
    free(hw_pcm);
    hw_pcm = NULL;
    hw_device->private_data = NULL;

    return ret;
}

int hw_pcm_start(au_hw_device_t *hw_device)
{
    int ret = -1;
    uint8_t cmd;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_DBG_PRT, "%s hw_pcm is null \n", __func__);
        return ret;
    }

    if (hw_device->status != AU_HWD_ST_OPENED) {

        ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__,
                    __LINE__, hw_device->status);
        return ret;
    }

    if (hw_pcm->i2s_handle) {
        snd_start(hw_pcm->i2s_handle, hw_device->type);
    }

    pcm_intance.pcm_status[hw_device->type] = AU_HW_PCM_STATUS_STARTED;

    hw_device->status = AU_HWD_ST_RUNNING;

    return ret;
}

int hw_pcm_pause(au_hw_device_t *hw_device)
{
    int ret = -1;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_DBG_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    return ret;
}

int hw_pcm_resume(au_hw_device_t *hw_device)
{
    int ret = -1;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_DBG_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    ret = 0;

    return ret;
}

int hw_pcm_write(au_hw_device_t *hw_device, uint8_t *in, uint16_t len)
{
    int ret = -1;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    if (hw_device->status == AU_HWD_ST_IDLE) {

        ssdk_printf(AM_MIX_ERR_PRT, "%s:line %d.status%d\n", __func__, __LINE__,
                    hw_device->status);
        return ret;
    }

    return snd_write(hw_pcm->i2s_handle, in, len);
}

int hw_pcm_read(au_hw_device_t *hw_device, uint8_t *out, uint16_t len)
{
    int ret = -1;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    if (hw_device->status == AU_HWD_ST_IDLE) {

        ssdk_printf(AM_MIX_ERR_PRT, "%s:line %d.status%d\n", __func__, __LINE__,
                    hw_device->status);
        return ret;
    }

    return snd_read(hw_pcm->i2s_handle, out, len);
}

int hw_pcm_available(au_hw_device_t *hw_device)
{
    int ret = 0;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    if (hw_device->status == AU_HWD_ST_IDLE) {

        ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__,
                    __LINE__, hw_device->status);
        return ret;
    }

    return snd_get_hw_avail(hw_pcm->i2s_handle, hw_device->type);
}

int hw_pcm_stop(au_hw_device_t *hw_device)
{
    int ret = -1;
    uint8_t cmd;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    ssdk_printf(AM_MIX_DBG_PRT, "%s:\n", __func__);

    if (hw_device->status != AU_HWD_ST_RUNNING) {
        ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__,
                    __LINE__, hw_device->status);
        return ret;
    }

    if (hw_pcm->i2s_handle) {
        snd_stop(hw_pcm->i2s_handle, hw_device->type);
    }

    pcm_intance.pcm_status[hw_device->type] = AU_HW_PCM_STATUS_STOPED;
    hw_device->status = AU_HWD_ST_STOP;
    ret = 0;

    return ret;
}

int hw_pcm_close(au_hw_device_t *hw_device)
{
    int ret = -1;
    au_hw_pcm_t *hw_pcm = (au_hw_pcm_t *)hw_device->private_data;

    if (!hw_pcm) {
        ssdk_printf(AM_MIX_ERR_PRT, "%s hw_pcm is null\n", __func__);
        return ret;
    }

    if (hw_device->status == AU_HWD_ST_OPENED ||
        hw_device->status == AU_HWD_ST_STOP) {
    } else {

        ssdk_printf(AM_MIX_DBG_PRT, "%s:line %d.status %d\n", __func__,
                    __LINE__, hw_device->status);
        return ret;
    }

    ssdk_printf(AM_MIX_DBG_PRT, "%s:\n", __func__);

    if (hw_pcm->i2s_handle) {
        snd_close(hw_pcm->i2s_handle, hw_device->type);
    }

    if (hw_pcm->i2s_handle) {
        if (pcm_intance.pcm_status[(hw_device->type + 1) % 2] ==
            AU_HW_PCM_STATUS_CLOSED) { /* if playback & capture closed */
            hw_pcm->i2s_handle = NULL;
        }
    }

    pcm_intance.pcm_status[hw_device->type] = AU_HW_PCM_STATUS_CLOSED;
    hw_device->status = AU_HWD_ST_IDLE;

    if (hw_device->private_data)
        free(hw_device->private_data);

    hw_device->private_data = NULL;
    ret = 0;

    return ret;
}

static au_hw_device_ops_t hw_pcm_ops = {
    .hw_open = hw_pcm_open,
    .hw_close = hw_pcm_close,
    .hw_start = hw_pcm_start,
    .hw_stop = hw_pcm_stop,
    .hw_write = hw_pcm_write,
    .hw_read = hw_pcm_read,
    .hw_resume = hw_pcm_resume,
    .hw_pause = hw_pcm_pause,
    .hw_available = hw_pcm_available,

};

au_hw_device_ops_t *au_get_hw_pcm_ops(void) { return &hw_pcm_ops; }
