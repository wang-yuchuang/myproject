/*
 * scr.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: System Control Register driver.
 *
 * Revision History:
 * -----------------
 */
#include <types.h>
#include <compiler.h>
#include <spinlock.h>


#include "driver/scr.h"
#if (defined CONFIG_ARCH_CHIP_D9) || (defined CONFIG_ARCH_CHIP_G9) || (defined CONFIG_ARCH_CHIP_X9)
#include "kunlun/scr/scr_lld.h"
#endif

#if (defined CONFIG_ARCH_CHIP_E3) || (defined CONFIG_ARCH_CHIP_D3)
#include "taishan/scr/scr_lld.h"
#endif


#define SCR_LOCK(dev) \
    irq_state_t state = spin_lock_irqsave(&dev->lock)

#define SCR_UNLOCK(dev) \
    spin_unlock_irqrestore(&dev->lock, state)

/**
 * @brief Get SCR signal value.
 */
uint32_t scr_get(scr_dev_t *dev, const scr_signal_t *signal)
{
    SCR_LOCK(dev);
    uint32_t val = scr_lld_get(dev, signal);
    SCR_UNLOCK(dev);

    return val;
}

/**
 * @brief Set SCR signal value.
 */
bool scr_set(scr_dev_t *dev, const scr_signal_t *signal, uint32_t value)
{
    SCR_LOCK(dev);
    bool ret = scr_lld_set(dev, signal, value);
    SCR_UNLOCK(dev);

    return ret;
}

/**
 * @brief Lock the SCR signal.
 */
bool scr_lock(scr_dev_t *dev, const scr_signal_t *signal)
{
    SCR_LOCK(dev);
    bool ret = scr_lld_lock(dev, signal);
    SCR_UNLOCK(dev);

    return ret;
}

/**
 * @brief Check whether the SCR signal is locked.
 *
 * A SCR signal is locked if it's read only or manually locked by SW.
 */
bool scr_is_locked(scr_dev_t *dev, const scr_signal_t *signal)
{
    SCR_LOCK(dev);
    bool locked = scr_lld_is_locked(dev, signal);
    SCR_UNLOCK(dev);

    return locked;
}

