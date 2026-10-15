/**
 * @file xli_ttc_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx ttc driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include "driver/timer.h"
#include "driver/xilinx/ttc/xli_ttc_drv.h"
#include "driver/irq.h"
#include "xli_ttc.h"
#include "bits.h"
#include "debug.h"

#define EXTRA_FMT   "xli_ttc:"
#define XLI_TTC_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define XLI_TTC_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define XLI_TTC_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

static void xli_ttc_timer_init(timer_dev_t *dev);
static void xli_ttc_timer_start(timer_dev_t *dev, uint32_t time_out);
static void xli_ttc_timer_stop(timer_dev_t *dev);
static uint32_t xli_ttc_get_current_time(timer_dev_t *dev);

static timer_ops_t g_xli_ttc_ops = {
    .init = xli_ttc_timer_init,
    .start = xli_ttc_timer_start,
    .stop = xli_ttc_timer_stop,
    .current = xli_ttc_get_current_time,
};

/**
 * @brief sdrv tmr func irq handle.
 *
 * @param[in] irq irq id
 * @param[in] arg irq arg
 */
static int xli_ttc_irq_handle(uint32_t irq, void *arg)
{
    xli_ttc_dev_t *xli_ttc = arg;

    xli_ttc_lld_clear_int(xli_ttc->base);

    timer_expiry(&xli_ttc->dev);

    return 0;
}

/**
 * @brief sdrv tmr init.
 *
 * @param[in] dev tmr dev
 */
static void xli_ttc_timer_init(timer_dev_t *dev)
{
    xli_ttc_dev_t *timer = dev->priv;
    xli_ttc_lld_init(timer->base);
}

/**
 * @brief sdrv tmr start.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 * @param[in] time_out time out value(ms)
 * @param[in] type timer type
 */
static void xli_ttc_timer_start(timer_dev_t *dev, uint32_t time_out)
{
    xli_ttc_dev_t *timer = dev->priv;
    uint32_t count;

    count = time_out * timer->cnt_per_us * 1000;
    xli_ttc_lld_timer_start(timer->base, count);
}

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 */
static void xli_ttc_timer_stop(timer_dev_t *dev)
{
    xli_ttc_dev_t *timer = dev->priv;
    xli_ttc_lld_timer_stop(timer->base);
}

/**
 * @brief sdrv tmr get current cnt value.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 * @return the value of spec tmr_id(us)
 */
static uint32_t xli_ttc_get_current_time(timer_dev_t *dev)
{
    xli_ttc_dev_t *timer = dev->priv;
    uint32_t current;

    current = xli_ttc_lld_get_cur_cnt(timer->base);
    return (current / timer->cnt_per_us);
}

/**
 * @brief xli ttc dev probe
 *
 * @param[in] xli_ttc xli ttc dev
 * @param[in] cfg xli ttc config
 * @return dev timer dev
 */
timer_dev_t *xli_ttc_probe(xli_ttc_dev_t *xli_ttc, const xli_ttc_cfg_t *cfg)
{
    timer_dev_t *dev = &xli_ttc->dev;

    xli_ttc->base = cfg->base;
    xli_ttc->irq = cfg->irq;
    xli_ttc->clk_frq = cfg->clk_frq;
    xli_ttc->cnt_per_us = cfg->clk_frq / 1000000;

    dev->ops = &g_xli_ttc_ops;
    dev->priv = xli_ttc;
    dev->drv_type = TMR_DRV_CYCLE;

    timer_init(dev);

    if(cfg->irq >= 0) {
        irq_attach(cfg->irq, xli_ttc_irq_handle, xli_ttc);
        irq_enable(cfg->irq);
    }

    return dev;
}
