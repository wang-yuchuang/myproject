/*
 * sdrv-cam-os-def.h
 *
 * Semidrive platform camera driver and hal adaptive to system header file
 *
 * Copyright (C) 2021, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */
#ifndef __SDRV_CAM_OS_DEF_H__
#define __SDRV_CAM_OS_DEF_H__

#ifdef _CONFIG_OS_SAFETY_

#include <arch.h>
#include <assert.h>
#include <bits.h>
#include <compiler.h>
#include <debug.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <hal_port.h>
#include <heap.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <kernel/thread.h>
#include <lib/console.h>
#include <limits.h>
#include <lk_wrapper.h>
#include <malloc.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <spinlock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "__regs_base.h"
#include "chip_res.h"
#include "hal_dio.h"
#include "i2c_hal.h"

#define csi_err(x...) dprintf(ALWAYS, x)
#define csi_warn(x...) dprintf(ALWAYS, x)
#define csi_info(x...) dprintf(ALWAYS, x)
#define csi_debug(x...) dprintf(SPEW, x)

#define cam_err(x...) dprintf(ALWAYS, x)
#define cam_info(x...) dprintf(ALWAYS, x)
#define cam_debug(x...) dprintf(SPEW, x)

#define sensor_err(x...) dprintf(ALWAYS, x)
#define sensor_info(x...) dprintf(ALWAYS, x)
#define sensor_debug(x...) dprintf(SPEW, x)

#define deser_err(x...) dprintf(ALWAYS, x)
#define deser_info(x...) dprintf(ALWAYS, x)
#define deser_debug(x...) dprintf(SPEW, x)

#define HAL_LOGE(x...) dprintf(ALWAYS, x)
#define HAL_LOGI(x...) dprintf(ALWAYS, x)
#define HAL_LOGD(x...) dprintf(SPEW, x)

#ifndef reg_addr_t
typedef void *reg_addr_t;
#endif

#ifndef dma_addr_t
typedef void *dma_addr_t;
#endif

#ifndef atomic_t
typedef volatile int atomic_t;
#endif

typedef struct {
    event_t event;
} completion_t;

typedef spin_lock_t spinlock_t;

static inline uint32_t reg_read(reg_addr_t base, uint32_t addr)
{
    return readl(base + addr);
}

static inline void reg_write(reg_addr_t base, uint32_t addr, uint32_t val)
{
    writel(val, base + addr);
}

inline static void usleep_range(int min, int max)
{
    int avg = (min + max) / 2;
    thread_sleep((avg + 999) / 1000);
}

static inline void *get_i2c_dev(uint32_t i2c_res)
{
    void *i2c_handle = NULL;
    hal_i2c_creat_handle(&i2c_handle, i2c_res);
    return i2c_handle;
}

inline static int i2c_write(void *adap, uint8_t addr, uint8_t *buf, int count)
{
    int reg_count = count - 1;
    int ret =
        hal_i2c_write_reg_data(adap, addr, buf, reg_count, buf + reg_count, 1);
    return ret;
}
inline static int i2c_write_read(void *adap, uint8_t addr, uint8_t *reg,
                                 int regc, uint8_t *buf, int bufc)
{
    int ret = hal_i2c_read_reg_data(adap, addr, reg, regc, buf, bufc);
    return ret;
}

inline static void completion_init(completion_t *com)
{
    event_init(&com->event, false, EVENT_FLAG_AUTOUNSIGNAL);
}

inline static void completion_destroy(completion_t *com)
{
    event_destroy(&com->event);
}

inline static int wait_completion_timeout(completion_t *com,
                                          uint32_t timeout_ms)
{
    if (event_wait_timeout(&com->event, timeout_ms) == ERR_TIMED_OUT)
        return -1;
    return 0;
}

inline static void completion_done(completion_t *com)
{
    event_signal(&com->event, false);
}

typedef mutex_t *osMutexId_t;

inline static osMutexId_t osMutexNew(void *p)
{
    mutex_t *lock = (mutex_t *)malloc(sizeof(mutex_t));
    mutex_init(lock);
    return (osMutexId_t)lock;
}
inline static void osMutexDelete(osMutexId_t lock) { free(lock); }
inline static void osMutexAcquire(osMutexId_t lock, int en)
{
    mutex_acquire(lock);
}
inline static void osMutexRelease(osMutexId_t lock) { mutex_release(lock); }

#define mdelay(ms) thread_sleep(ms)
#define udelay(us) thread_sleep((us + 999) / 1000)
#define usleep_range(min, max) thread_sleep((((min + max) / 2) + 999) / 1000)

#else /* _CONFIG_OS_SSDK_ */

#include <cmsis_os2.h>
#include <arch.h>
#include <arch/atomic.h>
#include <arch/cache.h>
#include <arch/irq.h>
#include <arch/spinlock.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <errno.h>
#include <lib/list.h>
#include <stddef.h>
#include <stdint.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <debug.h>
#include <board.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/irq.h"
#include "i2c_cfg.h"
#include "irq_num.h"
#include "regs_base.h"


#define csi_err(x...) ssdk_printf(SSDK_ERR, x)
#define csi_warn(x...) ssdk_printf(SSDK_WARNING, x)
#define csi_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define csi_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#define cam_err(x...) ssdk_printf(SSDK_ERR, x)
#define cam_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define cam_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#define sensor_err(x...) ssdk_printf(SSDK_ERR, x)
#define sensor_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define sensor_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#define deser_crit(x...) ssdk_printf(SSDK_CRIT, x)
#define deser_err(x...) ssdk_printf(SSDK_ERR, x)
#define deser_info(x...) ssdk_printf(SSDK_NOTICE, x)
#define deser_debug(x...) ssdk_printf(SSDK_DEBUG, x)

#define HAL_LOGE(x...) ssdk_printf(SSDK_ERR, x)
#define HAL_LOGI(x...) ssdk_printf(SSDK_NOTICE, x)
#define HAL_LOGD(x...) ssdk_printf(SSDK_DEBUG, x)

#ifndef reg_addr_t
typedef paddr_t reg_addr_t;
#endif

#ifndef dma_addr_t
typedef paddr_t dma_addr_t;
#endif

#ifndef atomic_t
typedef volatile int atomic_t;
#endif

#define CSI_COMPLETE_EVENT_FLAG 1u
typedef struct {
    osEventFlagsId_t event;
} completion_t;

typedef spin_lock_t spinlock_t;

static inline uint32_t reg_read(reg_addr_t base, uint32_t addr)
{
    return readl(base + addr);
}

static inline void reg_write(reg_addr_t base, uint32_t addr, uint32_t val)
{
    writel(val, base + addr);
}

#define usleep_range(min, max) udelay(min >> 1)
#define mdelay(ms) usleep_range(ms * 1000, ms * 1000)

static inline int atomic_add(atomic_t *ptr, int val)
{
    return arch_atomic_add((int *)ptr, val);
}

inline static void completion_init(completion_t *com)
{
    com->event = osEventFlagsNew(NULL);
}

inline static void completion_destroy(completion_t *com)
{
    osEventFlagsDelete(com->event);
}

inline static int wait_completion_timeout(completion_t *com, uint32_t timeout)
{
    uint32_t ret;

    ret = osEventFlagsWait(com->event, CSI_COMPLETE_EVENT_FLAG, osFlagsWaitAny,
                           timeout);
    return (int)ret;
}

inline static void completion_done(completion_t *com)
{
    osEventFlagsSet(com->event, CSI_COMPLETE_EVENT_FLAG);
}

#define spin_lock_irqsave(spinlock, flags)                                     \
    do {                                                                       \
        irq_state_t state = arch_irq_save();                                   \
        flags = state;                                                         \
        arch_spin_lock(spinlock);                                              \
    } while (0)

#define spin_unlock_irqrestore(spinlock, flags)                                \
    do {                                                                       \
        irq_state_t state = (irq_state_t)flags;                                \
        arch_spin_unlock(spinlock);                                            \
        arch_irq_restore(state);                                               \
    } while (0)

#endif

#endif /* __SDRV_CAM_OS_DEF_H__ */
