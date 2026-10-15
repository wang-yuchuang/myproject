/**
 * @file hwmutex.h
 * @brief Common hwmutex driver header.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef HW_MUTEX_H_
#define HW_MUTEX_H_

#include <cmsis_os2.h>

#define HWMUTEX_MAX_NUM  32

struct hwmutex;
struct hwmutex_device;

/**
 * struct hwmutex_ops - platform-specific hwmutex handlers
 *
 * @init: init hwmutex
 * @deinit: deinit hwmutex
 * @acquire: acquire hwmutex
 * @release: release hwmutex
 * @get_owner: get hwmutex owner id
 * @subscribe_notify: subscribe notify
 * @get_notified: check notified in isr
 */
struct hwmutex_ops {
    int (*init)(struct hwmutex *mutex);
    int (*deinit)(struct hwmutex *mutex);
    int (*acquire)(struct hwmutex *mutex);
    int (*release)(struct hwmutex *mutex);
    int (*get_owner)(struct hwmutex *mutex);
    int (*subscribe_notify)(struct hwmutex *mutex, bool en);
    bool (*get_notified)(struct hwmutex *mutex);
};

/**
 * struct hwmutex - this struct represents a single hwmutex instance
 * @bank: the hwmutex_device structure which owns this mutex
 * @l_sem: initialized and used by hwmutex core
 * @w_sem: initialized and used by hwmutex core
 * @priv: private data, owned by the underlying platform-specific hwmutex drv
 */
struct hwmutex {
    struct hwmutex_device *bank;
    osSemaphoreId_t l_sem;
    osSemaphoreId_t w_sem;
    void *priv;
};

/**
 * struct hwmutex_device - a device which usually spans numerous hwmutexs
 * @ops: platform-specific hwmutex handlers
 * @irq: irq number to notify other cores
 * @base_id: id index of the first mutex in this device
 * @num_mutexs: number of mutexs in this device
 * @bitmap: indicate used/free hwmutexs
 * @mutex: dynamically allocated array of 'struct hwmutex'
 */
struct hwmutex_device {
    const struct hwmutex_ops *ops;
    int irq;
    int base_id;
    int num_mutexs;
    unsigned int bitmap;
    struct hwmutex *mutex;
};

static inline int hwmutex_to_id(struct hwmutex *hwmutex)
{
    int local_id = hwmutex - &hwmutex->bank->mutex[0];

    return hwmutex->bank->base_id + local_id;
}

int hwmutex_acquire(struct hwmutex *mutex, unsigned int timeout);

int hwmutex_wait_release(struct hwmutex *mutex, unsigned int timeout);

int hwmutex_release(struct hwmutex *mutex);

int hwmutex_get_owner(struct hwmutex *mutex);

int hwmutex_register(struct hwmutex_device *dev);

struct hwmutex *hwmutex_request_specific(struct hwmutex_device *dev, unsigned int id);

int hwmutex_free(struct hwmutex *mutex);

#endif /* HW_MUTEX_H_ */
