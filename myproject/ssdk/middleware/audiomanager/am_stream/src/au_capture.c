/**
 * @file au_capture.c
 * @brief audio capture source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <cmsis_os2.h>
#include <debug.h>
#include <lib/list.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "au_capture.h"

#include "au_hw_device.h"

typedef struct {
    osEventFlagsId_t capture_event;
    osMutexId_t capture_mutex;
    osThreadId_t *thread;
    struct list_node pcm_streams;
    uint32_t hw_opened;
    uint32_t hw_started;
    uint32_t hw_paused;
    uint8_t *capture_data;
    au_hw_device_t hw_device;
} au_capture_dev_t;

static au_capture_dev_t capture_dev_instance;

static bool au_capture_reset(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;

    if (!handle) {
        return false;
    }

    clear_bit(dev->hw_opened, pcm_stream_get_id(handle));
    clear_bit(dev->hw_started, pcm_stream_get_id(handle));
    clear_bit(dev->hw_paused, pcm_stream_get_id(handle));

    return true;
}

static int au_capture_res_create(au_capture_dev_t *dev)
{
    if (!dev) {
        return -1;
    }

    dev->capture_data = (uint8_t *)memalign(32, DMA_PERIOD_BUFLEN);
    if (!dev->capture_data) {
        return -1;
    }

    memset((void *)dev->capture_data, 0x00, DMA_PERIOD_BUFLEN);

    return 0;
}

static void au_capture_res_destroy(au_capture_dev_t *dev)
{
    if (dev && dev->capture_data) {
        free(dev->capture_data);
        dev->capture_data = NULL;
    }
}

static void au_hw_device_capture_callback(void *arg)
{
    au_capture_dev_t *dev = (au_capture_dev_t *)arg;

    osEventFlagsSet(dev->capture_event, 1);
}

static int au_capture_handle(au_capture_dev_t *dev)
{
    int avail_cnt = 0;
    pcm_stream_handle_t pcm_stream;

    if (!dev) {
        return -1;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);

    avail_cnt = hw_device_available(&dev->hw_device);
    if (avail_cnt >= DMA_PERIOD_BUFLEN) {
        hw_device_read(&dev->hw_device, dev->capture_data, DMA_PERIOD_BUFLEN);
        list_for_every_entry (&dev->pcm_streams, pcm_stream,
                              struct au_pcm_stream, capture_node) {
            if (!pcm_stream_is_idle(pcm_stream)) {
                if ((pcm_stream_get_available(pcm_stream) >=
                     DMA_PERIOD_BUFLEN)) {
                    pcm_stream_write(pcm_stream, dev->capture_data, avail_cnt);
                    pcm_stream_post(pcm_stream);
                }
            }
        }
    }

    osMutexRelease(dev->capture_mutex);

    return 0;
}

static void au_capture_task(void *token)
{
    au_capture_dev_t *dev = (au_capture_dev_t *)token;

    au_capture_res_create(dev);

    while (1) {
        osEventFlagsWait(dev->capture_event, 1, osFlagsWaitAny, osWaitForever);
        au_capture_handle(dev);
    }

    au_capture_res_destroy(dev);
}

bool au_capture_init(void)
{
    bool ret = true;
    au_capture_dev_t *dev = &capture_dev_instance;
    osThreadAttr_t attr = {.name = "capture_handler",
                           .stack_size = 4096,
                           .priority = osPriorityNormal};

    memset(dev, 0, sizeof(au_capture_dev_t));
    dev->hw_opened = 0;
    dev->hw_started = 0;
    dev->hw_paused = 0;
    dev->capture_mutex = osMutexNew(NULL);
    dev->capture_event = osEventFlagsNew(NULL);

    list_initialize(&dev->pcm_streams);

    dev->thread = osThreadNew(au_capture_task, dev, &attr);

    if (dev->thread == NULL) {
        osEventFlagsDelete(dev->capture_event);
        osMutexDelete(dev->capture_mutex);
        ret = false;
    }

    hw_device_default(&dev->hw_device);

    return ret;
}

pcm_stream_handle_t au_capture_open(uint16_t vol, au_source_t *source)
{
    pcm_stream_handle_t pcm_stream;
    uint32_t flags;
    au_capture_dev_t *dev = &capture_dev_instance;

    osMutexAcquire(dev->capture_mutex, osWaitForever);

    pcm_stream = pcm_stream_open(source, DMA_BUF_TOTAL_LEN, vol);

    if (!pcm_stream) {
        return NULL;
    }

    pcm_stream->pcm_stream_type = AU_HWD_TYPE_CAPTURE; /* capture */
    dev->hw_device.type = AU_HWD_TYPE_CAPTURE;

    list_add_tail(&dev->pcm_streams, &pcm_stream->capture_node);

    au_capture_reset(pcm_stream);
    flags = dev->hw_opened;
    set_bit(dev->hw_opened, pcm_stream_get_id(pcm_stream));

    if (!flags) {
        hw_device_open(&dev->hw_device, au_hw_device_capture_callback, dev);
    }

    osMutexRelease(dev->capture_mutex);

    return pcm_stream;
}

int au_capture_close(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    uint32_t flags;
    int ret = -1;

    if (!handle) {
        return ret;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    pcm_stream_close(handle);
    list_delete(&handle->capture_node);

    clear_bit(dev->hw_opened, pcm_stream_get_id(handle));
    flags = dev->hw_opened;

    if (!flags) {
        ret = hw_device_close(&dev->hw_device);
    }

    au_capture_reset(handle);
    osMutexRelease(dev->capture_mutex);

    return ret;
}

int au_capture_start(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    uint32_t flags;
    int ret = -1;

    if (!handle) {
        return ret;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    pcm_stream_start(handle);

    flags = dev->hw_started;
    set_bit(dev->hw_started, pcm_stream_get_id(handle));

    if (!flags) {
        ret = hw_device_start(&dev->hw_device);
    }

    osMutexRelease(dev->capture_mutex);

    return ret;
}

int au_capture_stop(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    uint32_t flags;
    int ret = -1;

    if (!handle) {
        return ret;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    pcm_stream_stop(handle);

    clear_bit(dev->hw_started, pcm_stream_get_id(handle));
    flags = dev->hw_started;

    if (!flags) {
        ret = hw_device_stop(&dev->hw_device);
    }

    osMutexRelease(dev->capture_mutex);

    return ret;
}

int au_capture_read(pcm_stream_handle_t handle, uint8_t *out, int32_t size)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    int count = 0;

    if (!handle) {
        return -1;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    count = pcm_stream_read(handle, out, size);
    osMutexRelease(dev->capture_mutex);

    return count;
}

int au_capture_pause(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    uint32_t flags;
    int ret = -1;

    if (!handle) {
        return ret;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    pcm_stream_pause(handle);
    set_bit(dev->hw_paused, pcm_stream_get_id(handle));

    if (dev->hw_started == dev->hw_paused) {
        ret = hw_device_pause(&dev->hw_device);
    }

    osMutexRelease(dev->capture_mutex);

    return ret;
}

int au_capture_resume(pcm_stream_handle_t handle)
{
    au_capture_dev_t *dev = &capture_dev_instance;
    uint8_t flags;
    int ret = -1;

    if (!handle) {
        return ret;
    }

    osMutexAcquire(dev->capture_mutex, osWaitForever);
    clear_bit(dev->hw_paused, pcm_stream_get_id(handle));
    pcm_stream_resume(handle);

    ret = hw_device_resume(&dev->hw_device);
    osMutexRelease(dev->capture_mutex);

    return ret;
}