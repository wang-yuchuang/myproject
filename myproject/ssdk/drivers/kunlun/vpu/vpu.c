/**
 * @file vpu.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: vpu common api implement file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ========================= */
#include <driver/kunlun/vpu/vpu.h>
#include <debug.h>
/* =========================== marco define session ========================= */
/* =========================== enumeration define session =================== */
/* =========================== structure define session ===================== */
/* =========================== function declare session ===================== */
/* =========================== global value define session ================== */
/* =========================== static value define session ================== */
/* =========================== function implementation session ============== */
int32_t vpu_init(vpu_dev_t* dev)
{
    if (dev == NULL
        || dev->ops == NULL
        || dev->ops->init == NULL) {
        ssdk_printf(SSDK_ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    int32_t ret = 0;
    if (!dev->is_init &&
        (ret = dev->ops->init(dev)) == 0) {
        dev->is_init = true;
    }

    return ret;
}

int32_t vpu_acquire_chan(vpu_dev_t* dev, int32_t chanType, void* optData)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->acquire_chan == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }

    return dev->ops->acquire_chan(dev, chanType, optData);
}

int32_t vpu_release_chan(vpu_dev_t* dev, uint16_t chanId)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->release_chan == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }
    return dev->ops->release_chan(dev, chanId);
}

int32_t vpu_chan_codec_sync(vpu_dev_t* dev, uint16_t chanId)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->chan_codec_sync == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }

    return dev->ops->chan_codec_sync(dev, chanId);
}

int32_t vpu_chan_codec_async(vpu_dev_t* dev, uint16_t chanId, void (cb)(uint16_t chanId, int32_t codecRet, void* args), void* usrCtx)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->chan_codec_async == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }

    return dev->ops->chan_codec_async(dev, chanId, cb, usrCtx);
}

int32_t vpu_device_ctrl(vpu_dev_t* dev, uint16_t chanId, uint16_t ctlCmd, void* ctlData)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->device_ctrl == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }

    return dev->ops->device_ctrl(dev, chanId, ctlCmd, ctlData);
}

int32_t vpu_deinit(vpu_dev_t* dev)
{
    if (dev == NULL ||
        dev->ops == NULL ||
        dev->ops->deinit == NULL ||
        !dev->is_init) {
        ssdk_printf(SSDK_ERR, "parameter error or dev %s init error for %s\n", dev->dev_name, __func__);
        return -1;
    }

    int32_t ret = -1;
    if ((ret = dev->ops->deinit(dev)) == 0) {
        dev->is_init = false;
    }

    return ret;
}

/* end of file vpu.c */