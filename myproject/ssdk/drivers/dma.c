/**
 * @file  dma.c
 * @brief DMA common driver.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 */
#include <debug.h>
#include <string.h>
#include <sd_errcode.h>

#include "driver/dma.h"

#include "arch/cache.h"
#include "driver/irq.h"

/**
 * @brief init dma control
 *
 * @param[in] dev dma control instance,for details, see dma_dev_t
 * @return 0 if OK, or a negative error code.
 */
int dma_init(dma_dev_t *dev)
{
    int ret = 0;
    ASSERT(NULL != dev);

    dev->dev_lock = osMutexNew(NULL);
    ASSERT(NULL != dev->dev_lock);

    ret = osMutexAcquire(dev->dev_lock, osWaitForever);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex acquire fail(%d)\n", __func__,
                    __LINE__, ret);
        goto exit;
    }

    list_initialize(&dev->channels);
    if ((NULL != dev->ops) && (NULL != dev->ops->init))
        ret = dev->ops->init(dev);
    else
        ERRCODE_PRINT(MOD_ID_DMA, dev->dma_base, DMA_ERR_NOT_SUPPORT, 0);

    ret = osMutexRelease(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex release fail(%d)\n", __func__,
                    __LINE__, ret);
    }

exit:
    return ret;
}

/**
 * @brief deinit dma control
 *
 * @param[in] dev dma control instance,for details, see dma_dev_t
 */
void dma_deinit(dma_dev_t *dev)
{
    ASSERT(NULL != dev);
    int32_t ret;

    ret = osMutexAcquire(dev->dev_lock, osWaitForever);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex acquire fail(%d)\n", __func__,
                    __LINE__, ret);
        return;
    }

    if ((NULL != dev->ops) && (NULL != dev->ops->deinit))
        dev->ops->deinit(dev);
    else
        ERRCODE_PRINT(MOD_ID_DMA, dev->dma_base, DMA_ERR_NOT_SUPPORT, 0);

    ret = osMutexRelease(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex release fail(%d)\n", __func__,
                    __LINE__, ret);
        return;
    }
    ret = osMutexDelete(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex delete fail(%d)\n", __func__,
                    __LINE__, ret);
        return;
    }
}

/**
 * @brief request channel random
 *
 * @param[in] dev dma control instance,for details,see dma_dev_t
 * @return channel info
 */
dma_chan_t *dma_req_chan(dma_dev_t *dev)
{
    dma_chan_t *chan = NULL;
    ASSERT(NULL != dev);
    int32_t ret;

    ret = osMutexAcquire(dev->dev_lock, osWaitForever);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex acquire fail(%d)\n", __func__,
                    __LINE__, ret);
        return NULL;
    }

    if ((NULL != dev->ops) && (NULL != dev->ops->request_chan))
        chan = dev->ops->request_chan(dev, NULL);
    else
        ERRCODE_PRINT(MOD_ID_DMA, dev->dma_base, DMA_ERR_NOT_SUPPORT, 0);

    ret = osMutexRelease(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex release fail(%d)\n", __func__,
                    __LINE__, ret);
    }

    return chan;
}

/**
 * @brief request channel by id
 *
 * @param[in] dev dma control instance,for details,see dma_dev_t
 * @param[in] id request specified channel
 * @return channel info
 */
dma_chan_t *dma_req_chan_with_id(dma_dev_t *dev, uint32_t id)
{
    dma_chan_t *chan = NULL;
    ASSERT(NULL != dev);
    int32_t ret;

    ret = osMutexAcquire(dev->dev_lock, osWaitForever);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex acquire fail(%d)\n", __func__,
                    __LINE__, ret);
        return NULL;
    }

    if ((NULL != dev->ops) && (NULL != dev->ops->request_chan))
        chan = dev->ops->request_chan(dev, &id);
    else
        ERRCODE_PRINT(MOD_ID_DMA, dev->dma_base, DMA_ERR_NOT_SUPPORT, 0);

    ret = osMutexRelease(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex release fail(%d)\n", __func__,
                    __LINE__, ret);
    }

    return chan;
}

/**
 * @brief free channel
 *
 * @param[in] chan requested channel info
 */
void dma_free_chan(dma_chan_t *chan)
{
    ASSERT(NULL != chan);
    dma_dev_t *dev = chan->device;
    ASSERT(NULL != dev);
    int32_t ret;

    ret = osMutexAcquire(dev->dev_lock, osWaitForever);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex acquire fail(%d)\n", __func__,
                    __LINE__, ret);
        return;
    }

    if ((NULL != chan->device->ops) &&
        (NULL != chan->device->ops->release_chan))
        chan->device->ops->release_chan(chan);
    else
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
    ret = osMutexRelease(dev->dev_lock);
    if (osOK != ret) {
        ssdk_printf(SSDK_ERR, "%s %d, mutex release fail(%d)\n", __func__,
                    __LINE__, ret);
    }
}

/**
 * @brief dma device config (only for dev2mem or mem2dev)
 *
 * @param[in] chan requested channel info
 * @param[in] cfg  devcie config,for details, see dma_dev_cfg_t
 */
void dma_config_chan(dma_chan_t *chan, const dma_dev_cfg_t *cfg)
{
    ASSERT((NULL != chan) && (NULL != cfg));

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->config_chan))
        chan->device->ops->config_chan(chan, cfg);
    else
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
}

/**
 * @brief get mem2mem copy transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] dst  dst address
 * @param[in] src  src address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memcpy(dma_chan_t *chan, void *dst, void *src, size_t buf_len,
                     uint32_t flags)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->prep_memcpy))
        return chan->device->ops->prep_memcpy(chan, dst, src, buf_len, flags);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}

/**
 * @brief get mem2mem copy transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] dst  dst address
 * @param[in] src  src address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memcpy64(dma_chan_t *chan, uint64_t dst, uint64_t src, size_t buf_len,
                     uint32_t flags)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->prep_memcpy64))
        return chan->device->ops->prep_memcpy64(chan, dst, src, buf_len, flags);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}



/**
 * @brief get mem2mem set transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] val  set value
 * @param[in] buf_addr  dst address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memset(dma_chan_t *chan, uint8_t val, void *buf_addr,
                     size_t buf_len, uint32_t flags)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->prep_memset))
        return chan->device->ops->prep_memset(chan, val, buf_addr, buf_len,
                                              flags);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}

/**
 * @brief get mem2dev or dev2mem transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] buf_addr  dst address or src address (with dma_dev_config api)
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_dev(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                  uint32_t flags)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->prep_dev))
        return chan->device->ops->prep_dev(chan, buf_addr, buf_len, flags);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}

/**
 * @brief get mem2dev or dev2mem transfer config (loop)
 *
 * @param[in] chan requested channel info
 * @param[in] buf_addr  dst address or src address (with dma_dev_config api)
 * @param[in] buf_len transfer length
 * @param[in] period_len package data length in a time
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_cyclic(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                     size_t period_len, uint32_t flags)
{
    ASSERT(NULL != chan);
    ASSERT((buf_len % period_len) == 0);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->prep_cyclic))
        return chan->device->ops->prep_cyclic(chan, buf_addr, buf_len,
                                              period_len, flags);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}

/**
 * @brief start transfer
 *
 * @param[in] chan requested channel info
 */
void dma_start(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) && (NULL != chan->device->ops->enable_chan))
        chan->device->ops->enable_chan(chan);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
    }
}

/**
 * @brief terminate transfer
 *
 * @param[in] chan requested channel info
 * @return true/false
 */
bool dma_stop(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) &&
        (NULL != chan->device->ops->disable_chan))
        return chan->device->ops->disable_chan(chan);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return false;
    }
}

/**
 * @brief get transfer status
 *
 * @param[in] chan arequested channel info
 * @param[in] timeout wait time out
 * @return dma_status_e (completed in_progress err paused)
 */
dma_status_e dma_sync_wait(dma_chan_t *chan, const uint32_t timeout)
{
    ASSERT(NULL != chan);

    if ((NULL != chan->device->ops) &&
        (NULL != chan->device->ops->sync_wait))
        return chan->device->ops->sync_wait(chan, timeout);
    else {
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_NOT_SUPPORT, 0);
        return DMA_ERR;
    }
}

/**
 * @brief alloc no-cache buffer for transfer
 *
 * @param[in] size alloc length
 * @return alloced buffer address
 */
void *dma_alloc_memory(size_t size) { return NULL; }

/**
 * @brief free alloced buffer
 *
 * @param[in] dma_addr alloced buffer address
 */
void dma_free_memory(void *dma_addr) {}

/**
 * @brief flush cache for prevent data inconsistency
 *
 * @param[in] dma_addr buffer address
 * @param[in] size  buffer length
 */
void dma_flush_cache(addr_t dma_addr, size_t size)
{
    /* must align cache_line */
    ASSERT((dma_addr % CONFIG_ARCH_CACHE_LINE == 0) && (size % CONFIG_ARCH_CACHE_LINE == 0));

    arch_clean_cache_range(dma_addr, size);
}

/**
 * @brief flush invalidate for prevent data inconsistency
 *
 * @param[in] dma_addr  buffer address
 * @param[in] size  buffer length
 */
void dma_invalidate_cache(addr_t dma_addr, size_t size)
{
    /* must align cache_line */
    ASSERT((dma_addr % CONFIG_ARCH_CACHE_LINE == 0) && (size % CONFIG_ARCH_CACHE_LINE == 0));

    arch_invalidate_cache_range(dma_addr, size);
}
