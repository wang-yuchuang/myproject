/**
 * @file snd_driver.c
 * @version 0.1
 * @date 2022-03-02
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#include <debug.h>
#include <driver/arch_timer.h>
#include <driver/irq.h>
#include <stdlib.h>
#include <string.h>

#include "driver/snd_driver.h"

#include "arch/cache.h"
#include "cmsis_os2.h"

#define SND_DRIVER_DEBUG_LEVEL 7
#define SND_TRACE_LOG 1

#define ROUNDUP(x, y) ((((x) + (y)-1) / (y)) * (y))
#define SND_PLAYBACK_XFER_COMPLETE (0x1u)
#define SND_CAPTURE_XFER_COMPLETE (0x1u)

#define SND_STOP_THRESHOLD_DEFAULT_COUNT 2

#define roundup_pow_of_two(n) __builtin_ffs(n)
#define min(a, b) (a) < (b) ? a : b

static void playback_dma_transfer_callback(uint32_t status, uint32_t param,
                                           void *context);

static void capture_dma_transfer_callback(uint32_t status, uint32_t param,
                                          void *context);

static bool snd_trigger_start(snd_dev_t *dev, snd_stream_type_t stream);
static bool snd_trigger_stop(snd_dev_t *dev, snd_stream_type_t stream);

static int is_power_of_two(int n) { return (n != 0 && (n & (n - 1)) == 0); }

/* recalcuate the boundary within 32bit */
static uint32_t recalculate_boundary(snd_dma_buf_t *dma_buf)
{
    uint32_t boundary;

    if (!dma_buf->dma_size)
        return 0;
    boundary = dma_buf->dma_size;
    while (boundary * 2 <= 0x7fffffffUL - 2 * dma_buf->dma_size) {
        boundary *= 2;
    }
    return boundary;
}

static bool snd_dma_create(snd_dma_buf_t *dma_buf)
{
    uint32_t length = dma_buf->period_size * dma_buf->period_count;

    dma_buf->dma_addr = (uint8_t *)osAllocAlign(
        CONFIG_ARCH_CACHE_LINE, ROUNDUP(length, CONFIG_ARCH_CACHE_LINE));
    if (!dma_buf->dma_addr) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: error:\n", __func__);
        return false;
    }

    dma_buf->appl_ptr = 0;
    dma_buf->hw_ptr = 0;
    dma_buf->dma_size = length;
    dma_buf->boundary = recalculate_boundary(dma_buf);

    memset(dma_buf->dma_addr, 0x00, length);
    arch_clean_invalidate_cache_range((addr_t)dma_buf->dma_addr, length);
    return true;
}

static void snd_dma_free(snd_dma_buf_t *dma_buf)
{

    if (dma_buf->dma_addr) {
        osFree(dma_buf->dma_addr);
    }

    dma_buf->dma_addr = NULL;
}

static int32_t snd_playback_avail(snd_dma_buf_t *dma_buf)
{
    int32_t avail = dma_buf->dma_size - (dma_buf->appl_ptr - dma_buf->hw_ptr);

    if (avail < 0)
        avail += dma_buf->boundary;
    else if ((uint32_t)avail >= dma_buf->boundary)
        avail -= dma_buf->boundary;

    return avail;
}
static int32_t snd_playback_time_elapsed(snd_dma_buf_t *dma_buf)
{
    int32_t elapsed_span = dma_buf->appl_ptr - dma_buf->hw_ptr;
    if (dma_buf->appl_ptr < dma_buf->hw_ptr) {
        dma_buf->appl_ptr = dma_buf->hw_ptr;
    }
    return elapsed_span;
}

static int32_t snd_playback_hw_avail(snd_dma_buf_t *dma_buf)
{
    return dma_buf->dma_size - snd_playback_avail(dma_buf);
}

static int32_t snd_capture_time_elapsed(snd_dma_buf_t *dma_buf)
{
    int32_t elapsed_span = dma_buf->hw_ptr - dma_buf->appl_ptr;
    if (dma_buf->appl_ptr > dma_buf->hw_ptr) {
        dma_buf->appl_ptr = dma_buf->hw_ptr;
    }
    return elapsed_span;
}

static int32_t snd_capture_avail(snd_dma_buf_t *dma_buf)
{
    int32_t avail = dma_buf->hw_ptr - dma_buf->appl_ptr;

    if (avail < 0)
        avail += dma_buf->boundary;

    return avail;
}

static int32_t snd_capture_hw_avail(snd_dma_buf_t *dma_buf)
{
    return dma_buf->dma_size - snd_capture_avail(dma_buf);
}

static int snd_dma_write(snd_dma_buf_t *dma_buf, uint8_t *buffer, uint32_t len)
{

    uint32_t offset;
    uint32_t size;
    uint32_t avail, appl_ptr;

    if (!dma_buf->dma_addr) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s:dma_buf is null ,cannot write \r\n", __func__);
        return -1;
    }

    avail = snd_playback_avail(dma_buf);
    appl_ptr = dma_buf->appl_ptr;
    offset = appl_ptr % dma_buf->dma_size;
    len = min(len, avail);
    size = min(len, dma_buf->dma_size - offset);
    memcpy(dma_buf->dma_addr + offset, buffer, size);
    memcpy(dma_buf->dma_addr, buffer + size, len - size);

    appl_ptr += len;
    if (appl_ptr >= dma_buf->boundary) {
        appl_ptr -= dma_buf->boundary;
    }

    dma_buf->appl_ptr = appl_ptr;

    arch_clean_cache_range((addr_t)(dma_buf->dma_addr + offset), size);

    if ((len - size) > 0) {
        arch_clean_cache_range((addr_t)dma_buf->dma_addr, len - size);
    }

    return len;
}

static int snd_dma_read(snd_dma_buf_t *dma_buf, uint8_t *buffer, uint32_t len)
{
    uint32_t offset;
    uint32_t size;
    uint32_t avail, appl_ptr;

    if (!dma_buf->dma_addr) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s:dma_buf is null ,cannot write \r\n", __func__);
        return -1;
    }

    avail = snd_capture_avail(dma_buf);
    appl_ptr = dma_buf->appl_ptr;
    offset = appl_ptr % dma_buf->dma_size;
    len = min(len, avail);
    size = min(len, dma_buf->dma_size - offset);

    arch_invalidate_cache_range((addr_t)(dma_buf->dma_addr + offset), size);

    if ((len - size) > 0) {
        arch_invalidate_cache_range((addr_t)dma_buf->dma_addr, len - size);
    }

    memcpy(buffer, dma_buf->dma_addr + offset, size);
    memcpy(buffer + size, dma_buf->dma_addr, len - size);

    appl_ptr += len;
    if (appl_ptr >= dma_buf->boundary) {
        appl_ptr -= dma_buf->boundary;
    }

    dma_buf->appl_ptr = appl_ptr;

    return len;
}

static void snd_trace(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    char *stream_type = stream == SND_STREAM_PLAYBACK ? "playback" : "capture";

    ssdk_printf(SND_TRACE_LOG,
                ".stream_type(%s) .hw_ptr(%d) .appl_ptr(%d) .period_size(%d) "
                ".state(%d)\r\n",
                stream_type, dma_buf->hw_ptr, dma_buf->appl_ptr,
                dma_buf->period_size, snd_stream->state);
}

static bool snd_recover_xrun(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    bool ret = true;
    char *stream_type = stream == SND_STREAM_PLAYBACK ? "playback" : "capture";

    ssdk_printf(SSDK_ALERT, "%s: warning stream_type(%s) xrun recover \r\n",
                __func__, stream_type);

    ret = snd_trigger_start(dev, stream);
    snd_stream->state = SND_STATE_RUNNING;
    return ret;
}

static bool snd_xrun(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    char *stream_type = stream == SND_STREAM_PLAYBACK ? "playback" : "capture";

    ssdk_printf(SSDK_ALERT, "%s: warning stream_type(%s) xrun occur \r\n",
                __func__, stream_type);

    snd_trigger_stop(dev, stream);

    snd_trace(dev, stream);

    dma_buf->appl_ptr = 0;
    dma_buf->hw_ptr = 0;

    if (SND_STREAM_PLAYBACK == stream) {
        snd_stream->state = SND_STATE_UNDERRUN;
        if (dev->wr_event) {
            osEventFlagsClear(dev->wr_event, SND_PLAYBACK_XFER_COMPLETE);
        }
    } else {
        snd_stream->state = SND_STATE_OVERRUN;
        if (dev->rd_event) {
            osEventFlagsClear(dev->rd_event, SND_PLAYBACK_XFER_COMPLETE);
        }
    }

    snd_trace(dev, stream);
    return true;
}

static void playback_dma_transfer_callback(uint32_t status, uint32_t param,
                                           void *context)
{

    snd_dev_t *dev = (snd_dev_t *)context;
    snd_stream_type_t stream = SND_STREAM_PLAYBACK;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    snd_params_t *params = &snd_stream->params;
    int32_t hw_avail, app_avail;
    uint32_t hw_ptr = dma_buf->hw_ptr;
    snd_state_t state;

    if (params->xrun_enable) {
        if (snd_stream->state != SND_STATE_UNDERRUN) {
            app_avail = snd_playback_avail(dma_buf);
            if (app_avail >= params->stop_threshold) {
                snd_xrun(dev, stream);
                return;
            }
        } else {
            return;
        }
    }

    if (DMA_COMPLETED == status) {

        hw_ptr += dma_buf->period_size;
        if (hw_ptr >= dma_buf->boundary) {
            hw_ptr -= dma_buf->boundary;
        }
        dma_buf->hw_ptr = hw_ptr;
        if (dev->wr_event) {
            osEventFlagsSet(dev->wr_event, SND_PLAYBACK_XFER_COMPLETE);
        }

        hw_avail = snd_playback_hw_avail(dma_buf);
        if (hw_avail <= 0) {

            state = SND_STATE_UNDERRUN;
            /* underrun case */

        } else {
            state = SND_STATE_RUNNING;
        }

        if (snd_stream->callback) {
            snd_stream->callback(snd_stream->context, state);
        }
    }
}

static void capture_dma_transfer_callback(uint32_t status, uint32_t param,
                                          void *context)
{
    snd_dev_t *dev = (snd_dev_t *)context;
    snd_stream_type_t stream = SND_STREAM_CAPTURE;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_params_t *params = &snd_stream->params;
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    int32_t hw_avail, app_avail;
    uint32_t hw_ptr = dma_buf->hw_ptr;
    snd_state_t state;

    if (params->xrun_enable) {
        if (snd_stream->state != SND_STATE_OVERRUN) {
            app_avail = snd_capture_avail(dma_buf);
            if (app_avail >= params->stop_threshold) {
                snd_xrun(dev, stream);
                return;
            }
        } else {
            return;
        }
    }

    if (DMA_COMPLETED == status) {
        hw_ptr += dma_buf->period_size;
        if (hw_ptr >= dma_buf->boundary) {
            hw_ptr -= dma_buf->boundary;
        }
        dma_buf->hw_ptr = hw_ptr;
    }

    if (dev->rd_event) {
        osEventFlagsSet(dev->rd_event, SND_PLAYBACK_XFER_COMPLETE);
    }
    hw_avail = snd_capture_hw_avail(dma_buf);
    if (hw_avail <= 0) {

        state = SND_STATE_OVERRUN;
        /*overrun case */

    } else {
        state = SND_STATE_RUNNING;
    }

    if (snd_stream->callback) {
        snd_stream->callback(snd_stream->context, state);
    }
}

int32_t snd_get_hw_avail(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;

    int32_t avail = (stream == SND_STREAM_PLAYBACK)
                        ? snd_playback_hw_avail(dma_buf)
                        : snd_capture_hw_avail(dma_buf);

    return avail;
}

bool snd_open(snd_dev_t *dev, snd_params_t *params, snd_callback callback,
              void *context, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    snd_dma_data_t *dma_data = &snd_stream->dma_data;
    dma_chan_t *dma_ch;
    struct dma_dev_cfg dma_cfg = {0};

    if (NULL == dev) {
        return false;
    }

    ASSERT(NULL != params);
    ASSERT(NULL != dev->ops);
    ASSERT(0 != params->period_size);
    ASSERT(0 != params->period_count);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);

    if (SND_STATE_NOT_READY != snd_stream->state) {
        ssdk_printf(
            SSDK_WARNING,
            "%s: dev is  already ,do not reopen ,current state(%d) \r\n",
            __func__, snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    dma_buf->period_size = params->period_size;
    dma_buf->period_count = params->period_count;

    if (callback) {
        snd_stream->callback = callback;
        snd_stream->context = context;
    } else {

        snd_stream->callback = NULL;
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: init callback is null :\n",
                    __func__);
    }

    if (!snd_dma_create(dma_buf)) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: failed create dma buf :\n",
                    __func__);
        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    if (NULL != dev->ops->startup) {
        if (!dev->ops->startup(dev, stream)) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: startup failed \r\n",
                        __func__);
            goto OPEN_ERR1;
        }
    }

    if (params->slots > 2) {
        if (!params->slot_width) {
            ssdk_printf(
                SND_DRIVER_DEBUG_LEVEL,
                "%s: err TDM mode need set slots slot_width slot_mask \r\n",
                __func__);
            goto OPEN_ERR1;
        } else {
            if (dev->ops->set_tdm_slot) {
                dev->ops->set_tdm_slot(dev, params->tx_slot_mask,
                                       params->rx_slot_mask, params->slots,
                                       params->slot_width);
            }
        }
    }

    if (NULL != dev->ops->set_params) {
        if (!dev->ops->set_params(dev, params, stream)) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: set_params failed \r\n",
                        __func__);
            goto OPEN_ERR1;
        }
    }

    dma_ch = dma_req_chan(dev->dma_dev);
    if (!dma_ch) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: request dma ch  failed \r\n",
                    __func__);
        goto OPEN_ERR2;
    }
    /*TODO: */
    if (SND_STREAM_PLAYBACK == stream) {
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.dst_addr = (addr_t)dma_data->dma_addr;
        dma_cfg.src_addr = (addr_t)dma_buf->dma_addr;
        /*this part need overwrite by sound device driver as sound parameters
        in previous set_params function*/
        dma_cfg.src_addr_width = dma_data->addr_width;
        dma_cfg.dst_addr_width = dma_data->addr_width;
        dma_cfg.src_maxburst = dma_data->maxburst;
        dma_cfg.dst_maxburst = dma_data->maxburst;
        dma_ch->irq_cb = playback_dma_transfer_callback;

    } else {
        dma_cfg.direction = DMA_DEV2MEM;
        dma_cfg.dst_addr = (addr_t)dma_buf->dma_addr;
        dma_cfg.src_addr = (addr_t)dma_data->dma_addr;
        /*this part need overwrite by sound device driver as sound parameters
        in previous set_params function*/
        dma_cfg.src_addr_width = dma_data->addr_width;
        dma_cfg.dst_addr_width = dma_data->addr_width;
        dma_cfg.src_maxburst = dma_data->maxburst;
        dma_cfg.dst_maxburst = dma_data->maxburst;
        dma_ch->irq_cb = capture_dma_transfer_callback;
    }

    dma_ch->context = (void *)dev;
    dma_config_chan(dma_ch, &dma_cfg);

    if (!dma_prep_cyclic(dma_ch, dma_buf->dma_addr, dma_buf->dma_size,
                         dma_buf->period_size, DMA_INTERRUPT)) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: dma_prep_cyclic failed \r\n",
                    __func__);
        goto OPEN_ERR2;
    }

    snd_stream->dma_ch = dma_ch;

    if (params->xrun_enable) {
        if (SND_STREAM_PLAYBACK == stream) {
            if (!params->start_threshold) {
                params->start_threshold =
                    dma_buf->period_size * dma_buf->period_count / 2;
            }
            if (!params->stop_threshold) {
                params->stop_threshold = dma_buf->period_size *
                                         dma_buf->period_count *
                                         SND_STOP_THRESHOLD_DEFAULT_COUNT;
            }

        } else {
            if (!params->start_threshold) {
                params->start_threshold = 1;
            }
            if (!params->stop_threshold) {
                params->stop_threshold = dma_buf->period_size *
                                         dma_buf->period_count *
                                         SND_STOP_THRESHOLD_DEFAULT_COUNT;
            }
        }
    }
    memcpy(&snd_stream->params, params, sizeof(snd_params_t));

    snd_stream->state = SND_STATE_READY;

    osMutexRelease(dev->dev_lock[stream]);

    return true;

OPEN_ERR2:
    dma_free_chan(dma_ch);
OPEN_ERR1:
    if (dev->ops->shutdown) {
        if (!dev->ops->shutdown(dev, stream)) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: startup failed \r\n",
                        __func__);
        }
    }
    snd_dma_free(dma_buf);
    snd_stream->state = SND_STATE_NOT_READY;
    osMutexRelease(dev->dev_lock[stream]);
    ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: snd_open failed \r\n", __func__);
    return false;
}

bool snd_close(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    dma_chan_t *dma_ch = snd_stream->dma_ch;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);

    if (SND_STATE_NOT_READY == snd_stream->state) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s: dev is not ready ,current state(%d) \r\n", __func__,
                    snd_stream->state);

        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    if (dev->ops->shutdown) {
        if (!dev->ops->shutdown(dev, stream)) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: startup failed \r\n",
                        __func__);
        }
    }

    dma_free_chan(dma_ch);
    snd_dma_free(dma_buf);
    snd_stream->state = SND_STATE_NOT_READY;

    osMutexRelease(dev->dev_lock[stream]);
    return true;
}

static bool snd_trigger_start(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    dma_chan_t *dma_ch = snd_stream->dma_ch;
    bool ret = true;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    if (dma_ch) {
        dma_start(dma_ch);
    }

    if (dev->ops->trigger) {
        ret = dev->ops->trigger(dev, SND_TRIGGER_START, stream);
        if (!ret) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: trigger start failed\r\n",
                        __func__);
        }
    }

    return ret;
}
static bool snd_trigger_stop(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    dma_chan_t *dma_ch = snd_stream->dma_ch;
    bool ret = true;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    if (dma_ch) {
        dma_stop(dma_ch);
    }

    if (dev->ops->trigger) {
        ret = dev->ops->trigger(dev, SND_TRIGGER_STOP, stream);
        if (!ret) {
            ssdk_printf(SND_DRIVER_DEBUG_LEVEL, "%s: trigger start failed\r\n",
                        __func__);
        }
    }

    return ret;
}

bool snd_start(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    bool ret = true;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);
    if (SND_STATE_READY != snd_stream->state) {
        osMutexRelease(dev->dev_lock[stream]);
        return true;
    }

    ret = snd_trigger_start(dev, stream);
    snd_stream->state = SND_STATE_RUNNING;
    osMutexRelease(dev->dev_lock[stream]);
    return ret;
}

bool snd_stop(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    bool ret = true;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);
    if (SND_STATE_NOT_READY == snd_stream->state) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s: dev is not ready.current state(%d) \r\n", __func__,
                    snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    ret = snd_trigger_stop(dev, stream);
    snd_stream->state = SND_STATE_STOPPING;
    osMutexRelease(dev->dev_lock[stream]);
    return ret;
}

bool snd_pause(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    bool ret = false;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);
    if (SND_STATE_RUNNING != snd_stream->state) {
        ssdk_printf(
            SND_DRIVER_DEBUG_LEVEL,
            "%s: start failed,snd_dev is not in running,current state(%d) \r\n",
            __func__, snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    if (NULL != dev->ops->trigger) {
        ret = dev->ops->trigger(dev, SND_TRIGGER_PAUSE, stream);
    }
    if (ret) {
        snd_stream->state = SND_STATE_PAUSING;
    }
    osMutexRelease(dev->dev_lock[stream]);

    return ret;
}

bool snd_resume(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    bool ret = false;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);
    if (SND_STATE_PAUSING != snd_stream->state) {
        ssdk_printf(
            SND_DRIVER_DEBUG_LEVEL,
            "%s: start failed,snd_dev is not in pausing,current state(%d) \r\n",
            __func__, snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return false;
    }

    if (NULL != dev->ops->trigger) {
        ret = dev->ops->trigger(dev, SND_TRIGGER_RESUME, stream);
    }
    if (ret) {
        snd_stream->state = SND_STATE_RUNNING;
    }
    osMutexRelease(dev->dev_lock[stream]);

    return ret;
}

int snd_write(snd_dev_t *dev, uint8_t *buffer, uint32_t len)
{
    snd_stream_type_t stream = SND_STREAM_PLAYBACK;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    snd_params_t *params = &dev->stream[stream].params;
    uint32_t avail;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);
    ASSERT(NULL != buffer);

    if (!dma_buf->dma_addr) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s:dma_buf is null ,cannot write \r\n", __func__);
        return -1;
    }

check_tx_buf:
    avail = snd_playback_avail(dma_buf);
    if (avail < len) {

        osEventFlagsWait(dev->wr_event, SND_PLAYBACK_XFER_COMPLETE,
                         osFlagsWaitAll, 10000);
    }

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);

    if (SND_STATE_NOT_READY == snd_stream->state) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s: dev is not ready.current state(%d) \r\n", __func__,
                    snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return -1;
    }

    len = snd_dma_write(dma_buf, buffer, len);

    if (params->xrun_enable) {

        if (SND_STATE_READY == snd_stream->state) {
            avail = snd_playback_hw_avail(dma_buf);
            if (avail >= params->start_threshold) {
                snd_trigger_start(dev, stream);
                snd_stream->state = SND_STATE_RUNNING;
            }
        }

        if (SND_STATE_UNDERRUN == snd_stream->state) {
            avail = snd_playback_hw_avail(dma_buf);
            if (avail >= params->start_threshold) {
                snd_recover_xrun(dev, stream);
            }
        }
    }

    osMutexRelease(dev->dev_lock[stream]);

    return len;
}

int snd_read(snd_dev_t *dev, uint8_t *buffer, uint32_t len)
{
    snd_stream_type_t stream = SND_STREAM_CAPTURE;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_buf_t *dma_buf = &snd_stream->dma_buf;
    snd_params_t *params = &dev->stream[stream].params;
    uint32_t used;

    ASSERT(NULL != dev);
    ASSERT(NULL != dev->ops);
    ASSERT(NULL != buffer);

    if (!dma_buf->dma_addr) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s:dma_buf is null ,cannot read \r\n", __func__);
        return -1;
    }

    osMutexAcquire(dev->dev_lock[stream], osWaitForever);
    if (SND_STATE_NOT_READY == snd_stream->state) {
        ssdk_printf(SND_DRIVER_DEBUG_LEVEL,
                    "%s: dev is not ready.current state(%d) \r\n", __func__,
                    snd_stream->state);
        osMutexRelease(dev->dev_lock[stream]);
        return -1;
    }

    if (params->xrun_enable) {
        if (SND_STATE_READY == snd_stream->state) {
            snd_trigger_start(dev, stream);
            snd_stream->state = SND_STATE_RUNNING;
        }
        if (snd_stream->state == SND_STATE_OVERRUN) {
            snd_recover_xrun(dev, stream);
        }
    }

check_rx_buf:
    used = snd_capture_avail(dma_buf);
    if (used < len) {
        osEventFlagsWait(dev->rd_event, SND_CAPTURE_XFER_COMPLETE,
                         osFlagsWaitAll, 10000);
    }

    len = snd_dma_read(dma_buf, buffer, len);

    osMutexRelease(dev->dev_lock[stream]);
    return len;
}

void snd_dev_init(snd_dev_t *dev)
{
    dev->dev_lock[SND_STREAM_PLAYBACK] = osMutexNew(NULL);
    dev->dev_lock[SND_STREAM_CAPTURE] = osMutexNew(NULL);
    dev->stream[SND_STREAM_PLAYBACK].state = SND_STATE_NOT_READY;
    dev->stream[SND_STREAM_CAPTURE].state = SND_STATE_NOT_READY;
    dev->wr_event = osEventFlagsNew(NULL);
    dev->rd_event = osEventFlagsNew(NULL);
}

void snd_dev_deinit(snd_dev_t *dev)
{
    if (dev->dev_lock[SND_STREAM_PLAYBACK] != NULL)
        osMutexDelete(dev->dev_lock[SND_STREAM_PLAYBACK]);
    if (dev->dev_lock[SND_STREAM_CAPTURE] != NULL)
        osMutexDelete(dev->dev_lock[SND_STREAM_CAPTURE]);

    dev->stream[SND_STREAM_PLAYBACK].state = SND_STATE_NOT_READY;
    dev->stream[SND_STREAM_CAPTURE].state = SND_STATE_NOT_READY;

    if (dev->wr_event != NULL)
        osEventFlagsDelete(dev->wr_event);
    if (dev->rd_event != NULL)
        osEventFlagsDelete(dev->rd_event);
}
