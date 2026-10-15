/**
 * @file hwmutex.c
 * @brief Common hwmutex driver source
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdlib.h>
#include <driver/irq.h>
#include <driver/hwmutex.h>

static osSemaphoreId_t g_protect;

static int hwmutex_isr_handler(uint32_t irq, void *arg)
{
    struct hwmutex_device *dev = (struct hwmutex_device *)arg;

    for (int i = 0; i < dev->num_mutexs; i++) {
        if (dev->bitmap & (0x1u << i)) {
            struct hwmutex *mutex = &dev->mutex[i];

            if (dev->ops->get_notified(mutex)) {
                osSemaphoreRelease(mutex->w_sem);
            }
        }
    }

    return 0;
}

int hwmutex_acquire(struct hwmutex *mutex, unsigned int timeout)
{
    struct hwmutex_device *dev;
    osStatus_t status;

    status = osSemaphoreAcquire(mutex->l_sem, 0);
    if (status != osOK) {
        return osError;
    }

    dev = mutex->bank;
    dev->ops->subscribe_notify(mutex, true);

    while (dev->ops->acquire(mutex)) {
        status = osSemaphoreAcquire(mutex->w_sem, timeout);
        if (status != osOK) {
            break;
        }
    }

    dev->ops->subscribe_notify(mutex, false);
    osSemaphoreRelease(mutex->l_sem);
    return status;
}

int hwmutex_wait_release(struct hwmutex *mutex, unsigned int timeout)
{
    struct hwmutex_device *dev;
    osStatus_t status;

    status = osSemaphoreAcquire(mutex->l_sem, 0);
    if (status != osOK) {
        return osError;
    }

    dev = mutex->bank;
    dev->ops->subscribe_notify(mutex, true);

    if (dev->ops->acquire(mutex)) {
        status = osSemaphoreAcquire(mutex->w_sem, timeout);
    }
    else {
        dev->ops->release(mutex);
    }

    dev->ops->subscribe_notify(mutex, false);
    osSemaphoreRelease(mutex->l_sem);
    return status;
}

int hwmutex_release(struct hwmutex *mutex)
{
    struct hwmutex_device *dev;
    osStatus_t status;

    status = osSemaphoreAcquire(mutex->l_sem, 0);
    if (status != osOK) {
        return osError;
    }

    dev = mutex->bank;
    dev->ops->release(mutex);
    osSemaphoreRelease(mutex->l_sem);
    return status;
}

int hwmutex_get_owner(struct hwmutex *mutex)
{
    struct hwmutex_device *dev;
    osStatus_t status;
    int ret;

    status = osSemaphoreAcquire(mutex->l_sem, 0);
    if (status != osOK) {
        return osError;
    }

    dev = mutex->bank;
    ret = dev->ops->get_owner(mutex);
    osSemaphoreRelease(mutex->l_sem);
    return ret;
}

int hwmutex_register(struct hwmutex_device *dev)
{
    int ret = osOK;

    if (!dev || !dev->ops) {
        return osError;
    }

    if (dev->num_mutexs > HWMUTEX_MAX_NUM) {
        return osErrorParameter;
    }

    dev->bitmap = 0;

    for (int i = 0; i < dev->num_mutexs; i++) {
        struct hwmutex *hwmutex = &dev->mutex[i];

        hwmutex->bank = dev;
        hwmutex->l_sem = osSemaphoreNew(1, 1, NULL);
        hwmutex->w_sem = osSemaphoreNew(1, 0, NULL);
    }

    if (dev->irq > 0) {
        irq_attach(dev->irq, hwmutex_isr_handler, dev);
        irq_enable(dev->irq);
    }

    g_protect = osSemaphoreNew(1, 1, NULL);

    return ret;
}

struct hwmutex *hwmutex_request_specific(struct hwmutex_device *dev, unsigned int id)
{
    struct hwmutex *hwmutex = NULL;
    unsigned int local_id;
    osStatus_t status;

    status = osSemaphoreAcquire(g_protect, osWaitForever);
    if (status != osOK) {
        return NULL;
    }

    local_id = id - dev->base_id;

    if ((local_id < dev->num_mutexs) &&
        (!(dev->bitmap & (0x1u << local_id)))) {
        hwmutex = &dev->mutex[local_id];
        dev->bitmap |= (0x1u << local_id);
    }

    osSemaphoreRelease(g_protect);

    if (hwmutex && dev->ops->init) {
        dev->ops->init(hwmutex);
    }

    return hwmutex;
}

int hwmutex_free(struct hwmutex *mutex)
{
    struct hwmutex_device *dev;
    unsigned int local_id;
    osStatus_t status;

    status = osSemaphoreAcquire(g_protect, osWaitForever);
    if (status != osOK) {
        return status;
    }

    dev = mutex->bank;
    local_id = hwmutex_to_id(mutex);

    if ((local_id < dev->num_mutexs) &&
        (dev->bitmap & (0x1u << local_id))) {
        dev->bitmap &= ~(0x1u << local_id);
    }

    osSemaphoreRelease(g_protect);

    if (dev->ops->deinit) {
        dev->ops->deinit(mutex);
    }

    return osOK;
}
