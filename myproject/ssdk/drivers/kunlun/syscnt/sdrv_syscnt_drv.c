/**
 * @file sdrv_syscnt_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: syc cnt driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include "driver/timer.h"
#include "driver/kunlun/syscnt/sdrv_syscnt_drv.h"
#include "sdrv_syscnt.h"

static void sdrv_syscnt_start(timer_dev_t *dev, uint32_t time_out);
static void sdrv_syscnt_stop(timer_dev_t *dev);
static uint32_t sdrv_syscnt_get_current_time(timer_dev_t *dev);
static uint64_t sdrv_syscnt_get_global_time(timer_dev_t *dev);

static timer_ops_t g_sdrv_syscnt_ops = {
    .start = sdrv_syscnt_start,
    .stop = sdrv_syscnt_stop,
    .current = sdrv_syscnt_get_current_time,
    .global = sdrv_syscnt_get_global_time,
};

/**
 * @brief sdrv syscnt start
 *
 * @param[in] dev timer dev
 * @param[in] time_out timerout value, not support by syscnt
 */
static void sdrv_syscnt_start(timer_dev_t *dev, uint32_t time_out)
{
    sdrv_syscnt_dev_t *syscnt_dev = dev->priv;

    sdrv_syscnt_lld_enable(syscnt_dev->rw_base, true);
}

/**
 * @brief sdrv syscnt stop
 *
 * @param[in] dev timer dev
 */
static void sdrv_syscnt_stop(timer_dev_t *dev)
{
    sdrv_syscnt_dev_t *syscnt_dev = dev->priv;

    sdrv_syscnt_lld_enable(syscnt_dev->rw_base, false);
}

/**
 * @brief sdrv syscnt get current cnt
 *
 * @param[in] dev timer dev
 */
static uint32_t sdrv_syscnt_get_current_time(timer_dev_t *dev)
{
    sdrv_syscnt_dev_t *syscnt_dev = dev->priv;
    uint32_t value;

    value = (uint32_t)sdrv_syscnt_lld_get_val(syscnt_dev->ro_base);
    return value / syscnt_dev->cnt_per_us;
}

/**
 * @brief sdrv syscnt get current cnt
 *
 * @param[in] dev timer dev
 */
static uint64_t sdrv_syscnt_get_global_time(timer_dev_t *dev)
{
    sdrv_syscnt_dev_t *syscnt_dev = dev->priv;
    uint64_t value;

    value = sdrv_syscnt_lld_get_val(syscnt_dev->ro_base);
    return value / syscnt_dev->cnt_per_us;
}

/**
 * @brief sdrv syscnt probe
 *
 * @param[in] syscnt_dev syscnt dev
 * @param[in] cfg syscnt config
 * @return dev timer dev
 */
timer_dev_t *sdrv_syscnt_probe(sdrv_syscnt_dev_t *syscnt_dev,
                               const sdrv_syscnt_cfg_t *cfg)
{
    timer_dev_t *dev;

    syscnt_dev->ro_base = cfg->ro_base;
    syscnt_dev->rw_base = cfg->rw_base;
    syscnt_dev->cnt_per_us = cfg->clk_frq / 1000000;

    dev = &syscnt_dev->dev;
    dev->ops = &g_sdrv_syscnt_ops;
    dev->priv = syscnt_dev;
    dev->drv_type = TMR_DRV_CYCLE;

    timer_init(dev);

    return dev;
}
