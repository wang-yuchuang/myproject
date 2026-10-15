/**
 * @file codec_driver.c
 * @version 0.1
 * @date 2022-03-01
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#include <debug.h>
#include <driver/irq.h>
#include <string.h>

#include "cmsis_os2.h"
#include <driver/taishan/snd/codec_driver.h>
/**
 * @brief
 *
 * @param dev
 * @param params
 * @param stream
 * @return true
 * @return false
 */
bool codec_startup(codec_dev_t *dev, snd_stream_type_t stream)
{
    bool ret = false;

    osMutexAcquire(dev->dev_lock, osWaitForever);

    if ((NULL != dev->ops) && (NULL != dev->ops->startup)) {
        ret = dev->ops->startup(dev, stream);
    }

    osMutexRelease(dev->dev_lock);
    return ret;
}

/**
 * @brief
 *
 * @param dev
 * @param params
 * @param stream
 * @return true
 * @return false
 */
bool codec_set_params(codec_dev_t *dev, snd_params_t *params,
                      snd_stream_type_t stream)
{

    bool ret = false;

    osMutexAcquire(dev->dev_lock, osWaitForever);
    if ((NULL != dev->ops) && (NULL != dev->ops->set_params)) {
        ret = dev->ops->set_params(dev, params, stream);
    }
    osMutexRelease(dev->dev_lock);
    return ret;
}

/**
 * @brief
 *
 * @param dev
 * @param cmd
 * @param stream
 * @return true
 * @return false
 */
bool codec_trigger(codec_dev_t *dev, int cmd, snd_stream_type_t stream)
{
    bool ret = false;

    osMutexAcquire(dev->dev_lock, osWaitForever);
    if ((NULL != dev->ops) && (NULL != dev->ops->trigger)) {
        ret = dev->ops->trigger(dev, cmd, stream);
    }
    osMutexRelease(dev->dev_lock);

    return ret;
}

/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool codec_shutdown(codec_dev_t *dev, snd_stream_type_t stream)
{
    bool ret = false;

    osMutexAcquire(dev->dev_lock, osWaitForever);
    if ((NULL != dev->ops) && (NULL != dev->ops->shutdown)) {
        ret = dev->ops->shutdown(dev, stream);
    }
    osMutexRelease(dev->dev_lock);

    return ret;
}

/**
 * @brief
 *
 * @param dev
 */
void codec_dev_init(codec_dev_t *dev) { dev->dev_lock = osMutexNew(NULL); }
