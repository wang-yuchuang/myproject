/**
 * @file timer.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: timer driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <sd_errcode.h>

#include "cmsis_os2.h"
#include "debug.h"
#include "driver/timer.h"
#include "driver/kunlun/tmr/sdrv_tmr_drv.h"

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_init(timer_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->init)
        dev->ops->init(dev);

    dev->status = TMR_STA_STOP;
    spin_lock_init(&dev->lock);
}

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_deinit(timer_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->deinit)
        dev->ops->deinit(dev);

    dev->status = TMR_STA_STOP;
}

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_set_callback(timer_dev_t *dev, timer_callback callback, void *arg)
{
    ASSERT(dev != NULL);
    dev->callback = callback;
    dev->arg = arg;
}

/**
 * @brief timer start.
 *
 * @param[in] dev timer dev
 * @param[in] type timer type
 * @param[in] time_out time out value(ms)
 */
void timer_start(timer_dev_t *dev, timer_type_e type, uint32_t time_out)
{
    irq_state_t flag;

    ASSERT(dev != NULL);

    flag = spin_lock_irqsave(&dev->lock);

    if (dev->status == TMR_STA_START) {
        dev->ops->stop(dev);
    }

    dev->type = type;
    dev->time_out = time_out;
    dev->count_type = TMR_COUNT_MS;
    dev->ops->start(dev, time_out);
    dev->status = TMR_STA_START;

    spin_unlock_irqrestore(&dev->lock, flag);
}

/**
 * @brief timer start ns.
 *
 * @param[in] dev timer dev
 * @param[in] type timer type
 * @param[in] time_out time out value(ns)
 */
void timer_start_ns(timer_dev_t *dev, timer_type_e type, uint32_t time_out)
{
    irq_state_t flag;

    ASSERT(dev != NULL);

    flag = spin_lock_irqsave(&dev->lock);

    if (dev->status == TMR_STA_START) {
        dev->ops->stop(dev);
    }

    dev->type = type;
    dev->time_out = time_out;
    dev->count_type = TMR_COUNT_NS;
    dev->ops->start_ns(dev, time_out);
    dev->status = TMR_STA_START;

    spin_unlock_irqrestore(&dev->lock, flag);
}

/**
 * @brief timer stop.
 *
 * @param[in] dev timer dev
 */
void timer_stop(timer_dev_t *dev)
{
    irq_state_t flag;

    ASSERT(dev != NULL);

    flag = spin_lock_irqsave(&dev->lock);

    if (dev->status == TMR_STA_START) {
        dev->status = TMR_STA_STOP;
        dev->ops->stop(dev);
    }

    spin_unlock_irqrestore(&dev->lock, flag);
}

/**
 * @brief timer expiry.
 *
 * @param[in] dev timer dev
 */
void timer_expiry(timer_dev_t *dev)
{
    irq_state_t flag;
    sdrv_tmr_timer_priv_t *timer = dev->priv;

    ASSERT(dev != NULL);

    flag = spin_lock_irqsave(&dev->lock);

    if (dev->status != TMR_STA_START) {
        spin_unlock_irqrestore(&dev->lock, flag);
        ERRCODE_PRINT(MOD_ID_TMR, timer->id, TMR_DEV_STATUS_ERR, 0);
        return;
    }

    if (dev->type == TMR_TYPE_NORMAL &&
        dev->drv_type == TMR_DRV_CYCLE) {
        dev->ops->stop(dev);
        dev->status = TMR_STA_STOP;
    }
    else if (dev->type == TMR_TYPE_PERIOD &&
             dev->drv_type == TMR_DRV_ONESHOT) {
        if (dev->count_type == TMR_COUNT_MS) {
            dev->ops->start(dev, dev->time_out);
        } else if (dev->count_type == TMR_COUNT_NS) {
            dev->ops->start_ns(dev, dev->time_out);
        }
        dev->status = TMR_STA_START;

    }

    spin_unlock_irqrestore(&dev->lock, flag);

    if (dev->callback) {
        dev->callback(dev->arg);
    }
}

/**
 * @brief timer get current cnt value.
 *
 * @param[in] dev timer dev
 * @return the value of spec tmr_id(us)
 */
uint32_t timer_get_current_time(timer_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->current)
        return dev->ops->current(dev);
    return 0;
}

/**
 * @brief timer get global cnt value.
 *
 * @param[in] dev timer dev
 * @return the value of global tmr_id(us)
 */
uint64_t timer_get_global_time(timer_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->global)
        return dev->ops->global(dev);
    return 0;
}


void timer_get_gptp_time(timer_dev_t *dev,timer_ptp_timespec_t *ptp_time)
{
    ASSERT(dev != NULL);
    ASSERT(ptp_time != NULL);

    if (dev->ops->ptp)
        return dev->ops->ptp(dev,ptp_time);
}
#ifdef CONFIG_USE_TICKLESS_IDLE

void timer_frozen(timer_dev_t *dev)
{
    ASSERT(dev->ops->_frozen);
    dev->ops->_frozen(dev);
}

void timer_continue(timer_dev_t *dev)
{
    ASSERT(dev->ops->_continue);
    dev->ops->_continue(dev);
}

uint32_t timer_get_counter_register(timer_dev_t *dev)
{
    ASSERT(dev->ops->_get_counter);
    return dev->ops->_get_counter(dev);
}

void timer_set_terminal_register(timer_dev_t *dev, uint32_t val)
{
    ASSERT(dev->ops->_set_overflow);
    dev->ops->_set_overflow(dev, val);
}

void timer_force_reload(timer_dev_t *dev)
{
    ASSERT(dev->ops->_force_reload);
    dev->ops->_force_reload(dev);
}

bool timer_overflow_int_occur(timer_dev_t *dev)
{
    ASSERT(dev->ops->_overflowed);
    return dev->ops->_overflowed(dev);
}

void timer_clear_overflow_int(timer_dev_t *dev)
{
    ASSERT(dev->ops->_clear_int);
    dev->ops->_clear_int(dev);
}

uint32_t timer_count_to_us(timer_dev_t *dev, uint32_t val)
{
    ASSERT(dev->ops->_count_to_us);
    return val ? dev->ops->_count_to_us(dev, val) : 0;
}

#endif /* CONFIG_USE_TICKLESS_IDLE */
