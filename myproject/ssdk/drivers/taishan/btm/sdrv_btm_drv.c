/**
 * @file sdrv_btm_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx btm driver.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <string.h>

#include "driver/timer.h"
#include "driver/taishan/btm/sdrv_btm_drv.h"
#include "driver/irq.h"
#include "sdrv_btm.h"
#include "bits.h"
#include "debug.h"
#if 0
#include <driver/clk.h>
#include <clock_cfg.h>
#endif

#define EXTRA_FMT "sdrv_btm:"
#define SDRV_BTM_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_BTM_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_BTM_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

static void sdrv_btm_timer_init(timer_dev_t *dev);
static void sdrv_btm_timer_start(timer_dev_t *dev, uint32_t time_out);
static void sdrv_btm_timer_start_ns(timer_dev_t *dev, uint32_t time_out);
static void sdrv_btm_timer_stop(timer_dev_t *dev);
static uint32_t sdrv_btm_get_current_time(timer_dev_t *dev);
#ifdef CONFIG_USE_TICKLESS_IDLE
static void sdrv_btm_timer_frozen(timer_dev_t *dev);
static void sdrv_btm_timer_continue(timer_dev_t *dev);
static uint32_t sdrv_btm_timer_get_counter(timer_dev_t *dev);
static void sdrv_btm_timer_set_overflow(timer_dev_t *dev, uint32_t val);
static void sdrv_btm_timer_force_reload(timer_dev_t *dev);
static bool sdrv_btm_timer_overflowed(timer_dev_t *dev);
static void sdrv_btm_timer_clear_int(timer_dev_t *dev);
static uint32_t sdrv_btm_timer_count_to_us(timer_dev_t *dev, uint32_t cnt);
#endif /* CONFIG_USE_TICKLESS_IDLE */

static timer_ops_t g_sdrv_btm_ops = {
    .init = sdrv_btm_timer_init,
    .start = sdrv_btm_timer_start,
    .start_ns = sdrv_btm_timer_start_ns,
    .stop = sdrv_btm_timer_stop,
    .current = sdrv_btm_get_current_time,
#ifdef CONFIG_USE_TICKLESS_IDLE
    ._frozen = sdrv_btm_timer_frozen,
    ._continue = sdrv_btm_timer_continue,
    ._get_counter = sdrv_btm_timer_get_counter,
    ._set_overflow = sdrv_btm_timer_set_overflow,
    ._force_reload = sdrv_btm_timer_force_reload,
    ._overflowed = sdrv_btm_timer_overflowed,
    ._clear_int = sdrv_btm_timer_clear_int,
    ._count_to_us = sdrv_btm_timer_count_to_us,
#endif /* CONFIG_USE_TICKLESS_IDLE */
};

/**
 * @brief sdrv tmr func irq handle.
 *
 * @param[in] irq irq id
 * @param[in] arg irq arg
 */
static int sdrv_btm_irq_handle(uint32_t irq, void *arg)
{
    sdrv_btm_dev_t *sdrv_btm = arg;

    uint32_t int_sta = sdrv_btm_lld_get_int_sta(sdrv_btm->base);
    sdrv_btm_lld_set_int_sta(sdrv_btm->base, int_sta);
    if ((int_sta & BTM_BM_INT_STAT_OE_G1) || (int_sta & BTM_BM_INT_STAT_OE_G0))
    {
        timer_expiry(&sdrv_btm->dev);
    }

    return 0;
}

/**
 * @brief sdrv tmr init.
 *
 * @param[in] dev tmr dev
 */
static void sdrv_btm_timer_init(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_init(timer->base, timer->tmr_id, &(timer->tmr_cfg));
}

/**
 * @brief sdrv tmr start.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 * @param[in] time_out time out value(ms)
 * @param[in] type timer type
 */
static void sdrv_btm_timer_start(timer_dev_t *dev, uint32_t time_out)
{
    sdrv_btm_dev_t *timer = dev->priv;
    uint32_t count;

    count = time_out * timer->cnt_per_us * 1000;
    sdrv_btm_lld_timer_start(timer->base, timer->tmr_id, count);
    sdrv_btm_lld_compare_int_en(timer->base, timer->tmr_id, false);
}

/**
 * @brief sdrv tmr start ns.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 * @param[in] time_out time out value(ns)
 * @param[in] type timer type
 */
static void sdrv_btm_timer_start_ns(timer_dev_t *dev, uint32_t time_out)
{
    sdrv_btm_dev_t *timer = dev->priv;
    uint32_t count;

    if (timer->ns_per_tick == 0) {
        return;
    }
    count = time_out / (timer->ns_per_tick);
    sdrv_btm_lld_timer_start(timer->base, timer->tmr_id, count);
    sdrv_btm_lld_compare_int_en(timer->base, timer->tmr_id, false);
}

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 */
static void sdrv_btm_timer_stop(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_timer_stop(timer->base, timer->tmr_id);
}

/**
 * @brief sdrv tmr get current cnt value.
 *
 * @param[in] lower tmr dev
 * @param[in] tmr_id tmr id
 * @return the value of spec tmr_id(us)
 */
static uint32_t sdrv_btm_get_current_time(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    uint32_t current;

    current = sdrv_btm_lld_get_cnt_val(timer->base, timer->tmr_id);
    if (timer->cnt_per_us == 0)
    {
        return 0;
    }
    else
    {
        return (current / timer->cnt_per_us);
    }
}

/**
 * @brief sdrv btm dev probe
 *
 * @param[in] sdrv_btm sdrv btm dev
 * @param[in] cfg sdrv btm config
 * @return dev timer dev
 */
timer_dev_t *sdrv_btm_probe(sdrv_btm_dev_t *sdrv_btm, const sdrv_btm_cfg_t *cfg)
{
    timer_dev_t *dev = &sdrv_btm->dev;
    sdrv_btm->base = cfg->base;
    sdrv_btm->irq = cfg->irq;
    sdrv_btm->tmr_id = cfg->tmr_id;
    sdrv_btm->tmr_cfg = cfg->tmr_cfg;
    if (cfg->tmr_id == SDRV_BTM_G0)
    {
        #if 0
        uint32_t apb_freq = clk_get_rate(CLK_NODE(g_ckgen_bus_cr5_sf_div_p));
        /* btm_g0 fre: get the apb freq from g_ckgen_bus_cr5_sf_div_p_node */
        sdrv_btm->cnt_per_us = (apb_freq * (cfg->tmr_cfg.inc_val)) / ((cfg->tmr_cfg.si_val) + 1) / 1000000;
        #endif
        /* btm_g0 fre: 200M Hz */
        sdrv_btm->cnt_per_us = (200000000 * (cfg->tmr_cfg.inc_val)) / ((cfg->tmr_cfg.si_val) + 1) / 1000000;
        if (cfg->tmr_cfg.inc_val != 0) {
            sdrv_btm->ns_per_tick = (1000) / ((200 * (cfg->tmr_cfg.inc_val)) / ((cfg->tmr_cfg.si_val) + 1));
        }
    }
    else if (cfg->tmr_id == SDRV_BTM_G1)
    {
        /* btm_g1 fre: 24M Hz */
        sdrv_btm->cnt_per_us = (24000000 * (cfg->tmr_cfg.inc_val)) / ((cfg->tmr_cfg.si_val) + 1) / 1000000;
        if (cfg->tmr_cfg.inc_val != 0) {
            sdrv_btm->ns_per_tick = (1000) / ((24 * (cfg->tmr_cfg.inc_val)) / ((cfg->tmr_cfg.si_val) + 1));
        }
    }

    dev->ops = &g_sdrv_btm_ops;
    dev->priv = sdrv_btm;
    if (cfg->tmr_cfg.cnt_mode == SDRV_BTM_CONTINOUS_MODE) {
        dev->drv_type = TMR_DRV_CYCLE;
    }
    else {
        dev->drv_type = TMR_DRV_ONESHOT;
    }

    timer_init(dev);

    if (cfg->irq >= 0)
    {
        irq_attach(cfg->irq, sdrv_btm_irq_handle, sdrv_btm);
        irq_enable(cfg->irq);
    }

    return dev;
}

#ifdef CONFIG_USE_TICKLESS_IDLE
static void sdrv_btm_timer_frozen(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_cnt_stop(timer->base, timer->tmr_id, true);
}

static void sdrv_btm_timer_continue(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_cnt_stop(timer->base, timer->tmr_id, false);
}

static uint32_t sdrv_btm_timer_get_counter(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    return sdrv_btm_lld_get_cnt_val(timer->base, timer->tmr_id);
}

static void sdrv_btm_timer_set_overflow(timer_dev_t *dev, uint32_t val)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_set_ovf_cnt(timer->base, timer->tmr_id, val);
}

static void sdrv_btm_timer_force_reload(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    sdrv_btm_lld_frc_rld(timer->base, timer->tmr_id);
}

static bool sdrv_btm_timer_overflowed(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    uint32_t int_sta;

    int_sta = sdrv_btm_lld_get_int_sta(timer->base);

    if (timer->tmr_id == SDRV_BTM_G1) {
        return (int_sta & BTM_BM_INT_STAT_OE_G1) ? true : false;
    }
    else {
        return (int_sta & BTM_BM_INT_STAT_OE_G0) ? true : false;
    }
}

static void sdrv_btm_timer_clear_int(timer_dev_t *dev)
{
    sdrv_btm_dev_t *timer = dev->priv;
    uint32_t int_sta;

    int_sta = sdrv_btm_lld_get_int_sta(timer->base);
    sdrv_btm_lld_set_int_sta(timer->base, int_sta);
}

static uint32_t sdrv_btm_timer_count_to_us(timer_dev_t *dev, uint32_t cnt)
{
    sdrv_btm_dev_t *timer = dev->priv;

    return timer->cnt_per_us ? (cnt / timer->cnt_per_us) : 0;
}
#endif /* CONFIG_USE_TICKLESS_IDLE */
