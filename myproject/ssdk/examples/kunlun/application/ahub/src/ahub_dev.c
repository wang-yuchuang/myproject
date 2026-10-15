/* This file is setup audio hub device such as I2S SC I2S MC. */
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits.h>
#include <assert.h>

#ifdef CONFIG_AUDIO_MANAGER
#include <am_api.h>
#endif
#include <malloc.h>
#include <res_loader.h>
#include <driver/arch_timer.h>

#include "audio_res.h"

#include "param.h"
#include "ahub.h"
#include "ahub_cfg.h"
#include "ahub_dev.h"
#define AHUB_DEV_DBG_PRT SSDK_CRIT

static snd_dev_t *safety_snd_dev;
static snd_dev_t *ap_main_snd_dev;
snd_params_t *p_ahub_snd_params[AHUB_SND_DEV_PARAM_NUMB];

static size_t dev_write(u_int32_t stream_id, const void *buffer, size_t bytes)
{
    int len = 0;
    if (stream_id == AHUB_PLAYBACK_STREAM_ID0) {
        if (safety_snd_dev)
            len = snd_write(safety_snd_dev, buffer, bytes);
        if (len < bytes) {
            ssdk_printf(SSDK_ERR, "%s: len(%d) < bytes(%d), please check it\n",
                        __func__, len, bytes);
        }
        return len;
    } else if (stream_id == AHUB_PLAYBACK_STREAM_ID1) {
        if (ap_main_snd_dev)
            len = snd_write(ap_main_snd_dev, buffer, bytes);
        if (len < bytes) {
            ssdk_printf(SSDK_ERR, "%s: len(%d) < bytes(%d), please check it\n",
                        __func__, len, bytes);
        }
        return len;
    } else {
        osDelay(2000);
    }
    return 0;
}

/* static uint64_t time_span = 0; */
static size_t dev_read(u_int32_t stream_id, void *buffer, size_t bytes)
{
    int len = 0;

    if (stream_id == AHUB_CAPTURE_STREAM_ID0) {
        if (ap_main_snd_dev)
            len = snd_read(ap_main_snd_dev, buffer, bytes);

        if (len < bytes) {
            ssdk_printf(SSDK_ERR, "%s: len < bytes, please check it\n",
                        __func__);
        }
        return len;
    }
    return 0;
}

static int dev_open_playback_stream(u_int32_t stream_id)
{
    snd_dev_t *dev = NULL;
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter!\n", __func__);
    if (stream_id == AHUB_PLAYBACK_STREAM_ID0) {
        dev = safety_snd_dev;
        // snd_params[0]should align with AHUB_PLAYBACK_STREAM_ID0
        if (!snd_open(dev, p_ahub_snd_params[SAF_SND_DEV_TX_PARAM], NULL, NULL,
                      SND_STREAM_PLAYBACK)) {
            ssdk_printf(SSDK_ERR, "%s: snd_open failed \n", __func__);
            return AHUB_RET_ERR;
        }
#ifdef CONFIG_AUDIO_MANAGER
        am_start_path("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K", 100);
#endif
    } else if (stream_id == AHUB_PLAYBACK_STREAM_ID1) {
        dev = ap_main_snd_dev;
        // snd_params[0]should align with AHUB_PLAYBACK_STREAM_ID0
        if (!snd_open(dev, p_ahub_snd_params[AP_MAIN_DEV_TX_PARAM], NULL, NULL,
                      SND_STREAM_PLAYBACK)) {
            ssdk_printf(SSDK_ERR, "%s: snd_open failed \n", __func__);
            return AHUB_RET_ERR;
        }
#ifdef CONFIG_AUDIO_MANAGER
        am_start_path("PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K", 100);
#endif
    } else {
        ssdk_printf(SSDK_ERR, "%s: stream_id error, please check it\n",
                    __func__);
        return AHUB_RET_ERR;
    }


    return AHUB_RET_OK;
}

static int dev_close_playback_stream(u_int32_t stream_id)
{
    snd_dev_t *dev = NULL;
    // 1 do output standby
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter! \n", __func__);
    if (stream_id == AHUB_PLAYBACK_STREAM_ID0) {
        dev = safety_snd_dev;
        snd_stop(dev, SND_STREAM_PLAYBACK);
        snd_close(dev, SND_STREAM_PLAYBACK);
#ifdef CONFIG_AUDIO_MANAGER
        am_stop_path("PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K");
#endif
    } else if (stream_id == AHUB_PLAYBACK_STREAM_ID1) {

        dev = ap_main_snd_dev;
        snd_stop(dev, SND_STREAM_PLAYBACK);
        snd_close(dev, SND_STREAM_PLAYBACK);
#ifdef CONFIG_AUDIO_MANAGER
        am_stop_path("PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K");
#endif
    } else {
        ssdk_printf(SSDK_ERR, "%s: stream_id error, please check it\n",
                    __func__);
        return AHUB_RET_ERR;
    }

    return AHUB_RET_OK;
}

static int dev_open_capture_stream(u_int32_t stream_id)
{
    // 1 print config
    // 2 get stream id
    // 3 open real audio device

    snd_dev_t *dev = NULL;
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter!\n", __func__);

    if (stream_id == AHUB_CAPTURE_STREAM_ID0) {
        dev = ap_main_snd_dev;
        // snd_params[0]should align with AHUB_PLAYBACK_STREAM_ID0
        if (!snd_open(dev, p_ahub_snd_params[AP_MAIN_DEV_RX_PARAM], NULL, NULL,
                      SND_STREAM_CAPTURE)) {
            ssdk_printf(SSDK_ERR, "%s: snd_open failed \n", __func__);
            return AHUB_RET_ERR;
        }
#ifdef CONFIG_AUDIO_MANAGER
        am_start_path("PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K", 100);
#endif
    } else {
        ssdk_printf(SSDK_ERR, "%s: stream_id error, please check it\n",
                    __func__);
        return AHUB_RET_ERR;
    }

    return AHUB_RET_OK;
}
static int dev_close_capture_stream(u_int32_t stream_id)
{
    snd_dev_t *dev = NULL;
    // 1 do output standby
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter!\n", __func__);
    if (stream_id == AHUB_CAPTURE_STREAM_ID0) {

        dev = ap_main_snd_dev;
        snd_stop(dev, SND_STREAM_CAPTURE);
        snd_close(dev, SND_STREAM_CAPTURE); // Can not close this device
#ifdef CONFIG_AUDIO_MANAGER
        am_stop_path("PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K");
#endif
    } else {
        ssdk_printf(SSDK_ERR, "%s: stream_id error, please check it\n",
                    __func__);
        return AHUB_RET_ERR;
    }

    return AHUB_RET_OK;
}

static int init_dev(u_int32_t stream_id)
{
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter!\n", __func__);
    return AHUB_RET_OK;
}

static int release_dev(u_int32_t stream_id)
{
    ssdk_printf(AHUB_DEV_DBG_PRT, "%s enter!\n", __func__);
    return AHUB_RET_OK;
}

/* audio hub hardware device entry */
ahub_hw_dev_t AHUB_HW_DEV = {
    .init = init_dev,
    .release = release_dev,

    .open_playback_stream = dev_open_playback_stream,
    .close_playback_stream = dev_close_playback_stream,
    .open_capture_stream = dev_open_capture_stream,
    .close_capture_stream = dev_close_capture_stream,

    .read = dev_read,
    .write = dev_write,
};

int ahub_safety_dev_init(snd_dev_t *dev, snd_params_t *tx_param,
                         snd_params_t *rx_param)
{
    safety_snd_dev = dev;
    p_ahub_snd_params[SAF_SND_DEV_TX_PARAM] = tx_param;
    p_ahub_snd_params[SAF_SND_DEV_RX_PARAM] = rx_param;
    return AHUB_RET_OK;
}

int ahub_ap_main_dev_init(snd_dev_t *dev, snd_params_t *tx_param,
                          snd_params_t *rx_param)
{
    ap_main_snd_dev = dev;
    p_ahub_snd_params[AP_MAIN_DEV_TX_PARAM] = tx_param;
    p_ahub_snd_params[AP_MAIN_DEV_RX_PARAM] = rx_param;
    return AHUB_RET_OK;
}