/**
 * @file au_hw_device.c
 * @brief audio hardware device source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>

#include "au_hw_device.h"

#include "au_hw_pcm.h"

#define AM_AGENT_DBG_PRT SSDK_DEBUG

void hw_device_default(au_hw_device_t *hw_device)
{
    hw_device->status = AU_HWD_ST_IDLE;
    hw_device->hw_ops = au_get_hw_pcm_ops();
}

int hw_device_open(au_hw_device_t *hw_device, hw_device_callback hw_callback,
                   void *cb_param)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_open) {
        ret = hw_device->hw_ops->hw_open(hw_device, hw_callback, cb_param);
    }

    return ret;
}

int hw_device_start(au_hw_device_t *hw_device)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_start) {
        ret = hw_device->hw_ops->hw_start(hw_device);
    }

    return ret;
}

int hw_device_pause(au_hw_device_t *hw_device)
{
    int ret = -1;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s: NO_IMPL\n", __func__);
    return ret;
}

int hw_device_resume(au_hw_device_t *hw_device)
{
    int ret = -1;

    ssdk_printf(AM_AGENT_DBG_PRT, "%s: NO_IMPL\n", __func__);
    return ret;
}

int hw_device_write(au_hw_device_t *hw_device, uint8_t *in, uint16_t len)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_write) {
        ret = hw_device->hw_ops->hw_write(hw_device, in, len);
    }

    return ret;
}

int hw_device_read(au_hw_device_t *hw_device, uint8_t *out, uint16_t len)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_read) {
        ret = hw_device->hw_ops->hw_read(hw_device, out, len);
    }

    return ret;
}

int hw_device_available(au_hw_device_t *hw_device)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_available) {
        ret = hw_device->hw_ops->hw_available(hw_device);
    }

    return ret;
}

int hw_device_stop(au_hw_device_t *hw_device)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_stop) {
        ret = hw_device->hw_ops->hw_stop(hw_device);
    }

    return ret;
}

int hw_device_close(au_hw_device_t *hw_device)
{
    int ret = -1;

    if (!hw_device->hw_ops) {
        ssdk_printf(SSDK_ERR, "%s:hw_device->hw_ops is null \n", __func__);
        return ret;
    }

    if (hw_device->hw_ops->hw_close) {
        ret = hw_device->hw_ops->hw_close(hw_device);
    }

    return ret;
}