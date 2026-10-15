/**
 * @file sdrv_tmr_drv.c
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
#include <string.h>
#include <sd_errcode.h>

#include "driver/timer.h"
#include "driver/kunlun/tmr/sdrv_tmr_drv.h"
#include "driver/irq.h"
#include "sdrv_tmr.h"
#include "bits.h"
#include "debug.h"

#define EXTRA_FMT   "sdrv_tmr:"
#define SDRV_TMR_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_TMR_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_TMR_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

static void sdrv_tmr_timer_start(timer_dev_t *dev, uint32_t time_out);
static void sdrv_tmr_timer_stop(timer_dev_t *dev);
static uint32_t sdrv_tmr_get_current_time(timer_dev_t *dev);
static uint64_t sdrv_tmr_get_global_time(timer_dev_t *dev);
static void sdrv_tmr_get_ptp_time(timer_dev_t *dev,timer_ptp_timespec_t *ptp_time);

static timer_ops_t g_sdrv_tmr_timer_ops = {
    .start = sdrv_tmr_timer_start,
    .stop = sdrv_tmr_timer_stop,
    .current = sdrv_tmr_get_current_time,
    .global = sdrv_tmr_get_global_time,
    .ptp = sdrv_tmr_get_ptp_time,
};

/**
 * @brief sdrv tmr func irq handle.
 *
 * @param[in] irq irq id
 * @param[in] arg irq arg
 */
static int sdrv_tmr_irq_handle(uint32_t irq, void *arg)
{
    sdrv_tmr_dev_t *sdrv_tmr = arg;
    uint32_t int_sta;
    uint8_t id;

    int_sta = sdrv_tmr_lld_get_int_status(sdrv_tmr->base);

    /* cnt ovf irq handler */
    for (id = SDRV_TMR_G0; id < SDRV_TMR_MAX_NUM; id++) {
        if (BIT_SET(int_sta, SDRV_TMR_STA_CNT_OVF_SHIFT(id))) {
            sdrv_tmr_timer_priv_t *timer = &sdrv_tmr->timer[id];
            timer_expiry(&timer->dev);
        }
    }

    /* func irq handler */
    for (id = SDRV_TMR_A; id < SDRV_TMR_MAX_NUM; id++) {
        uint8_t chn_id = id - 2;
        if (BIT_SET(int_sta, SDRV_TMR_STA_CMP_SHIFT(chn_id)) || \
            BIT_SET(int_sta, SDRV_TMR_STA_CPT_SHIFT(chn_id))) {
            /* TODO */
        }

        if (BIT_SET(int_sta, SDRV_TMR_STA_FIFO_OVERRUN_SHIFT(chn_id)) || \
            BIT_SET(int_sta, SDRV_TMR_STA_FIFO_UNDERRUN_SHIFT(chn_id))) {
            /* TODO */
        }
    }

    return 0;
}

/**
 * @brief sdrv tmr start.
 *
 * @param[in] lower tmr dev
 */
static void sdrv_tmr_timer_start(timer_dev_t *dev, uint32_t time_out)
{
    sdrv_tmr_timer_priv_t *timer = dev->priv;
    uint32_t count;

    count = time_out * timer->cnt_per_us * 1000 - 1;
    sdrv_tmr_lld_timer_start(timer->base, timer->id, count);
}

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] lower tmr dev
 */
static void sdrv_tmr_timer_stop(timer_dev_t *dev)
{
    sdrv_tmr_timer_priv_t *timer = dev->priv;
    sdrv_tmr_lld_timer_stop(timer->base, timer->id);
}

/**
 * @brief sdrv tmr get current cnt value.
 *
 * @param[in] lower tmr dev
 * @return the value of spec tmr_id(us)
 */
static uint32_t sdrv_tmr_get_current_time(timer_dev_t *dev)
{
    sdrv_tmr_timer_priv_t *timer = dev->priv;
    uint32_t current;

    current = sdrv_tmr_lld_get_cur_cnt(timer->base, timer->id);
    return ((current + timer->cnt_per_us / 2) / timer->cnt_per_us);
}

/**
 * @brief sdrv tmr get glb cnt value.
 *
 * @param[in] dev tmr dev
 * @return glb cnt value(us)
 */
static uint64_t sdrv_tmr_get_global_time(timer_dev_t *dev)
{
    sdrv_tmr_timer_priv_t *timer = dev->priv;
    sdrv_tmr_dev_t *sdrv_dev = timer->priv;
    uint64_t value;
    irq_state_t state;

    if (!sdrv_dev->cascase_mode) {
        SDRV_TMR_ERR("get global time only support in cascase mode\n");
        ERRCODE_PRINT(MOD_ID_TMR, timer->id, TMR_GLOBAL_CASCASE_MODE_ERR, 0);
        return 0;
    }

    state = spin_lock_irqsave(&sdrv_dev->spinlock);
    value = sdrv_tmr_lld_get_glb_cnt(timer->base);
    spin_unlock_irqrestore(&sdrv_dev->spinlock, state);
    return ((value + timer->cnt_per_us / 2) / timer->cnt_per_us);
}

/**
 * @brief sdrv tmr get glb cnt value.
 *
 * @param[in] dev tmr dev
 * @param[in] ptp_time struct
 */

static void sdrv_tmr_get_ptp_time(timer_dev_t *dev,timer_ptp_timespec_t *ptp_time)
{
    sdrv_tmr_timer_priv_t *timer = dev->priv;
    sdrv_tmr_dev_t *sdrv_dev = timer->priv;
    irq_state_t state;

    state = spin_lock_irqsave(&sdrv_dev->spinlock);
    if(sdrv_tmr_lld_get_ptp_enable(sdrv_dev->ptp_base,0))
    {
          ptp_time->sec = sdrv_tmr_lld_get_ptp_time(sdrv_dev->ptp_base,0x08);
          ptp_time->nsec = sdrv_tmr_lld_get_ptp_time(sdrv_dev->ptp_base,0x0C);
          ptp_time->upd_sec = sdrv_tmr_lld_get_ptp_time(sdrv_dev->ptp_base,0x10);
          ptp_time->upd_nsec = sdrv_tmr_lld_get_ptp_time(sdrv_dev->ptp_base,0x14);
    }
    else
    {
          ptp_time->sec = 0;
          ptp_time->nsec = 0;
          ptp_time->upd_sec = 0;
          ptp_time->upd_nsec = 0;
    }
    spin_unlock_irqrestore(&sdrv_dev->spinlock, state);
}

/**
 * @brief sdrv timer probe
 *
 * @param[in] sdrv_tmr sdrv timer dev
 * @param[in] id tmr id in one sdrv tmr
 * @return dev timer dev
 */
timer_dev_t *sdrv_tmr_timer_probe(sdrv_tmr_dev_t *sdrv_tmr, sdrv_tmr_id_e id)
{
    sdrv_tmr_timer_priv_t *timer_priv;
    timer_dev_t *dev;

    ASSERT(id < SDRV_TMR_MAX_NUM);

    timer_priv = &sdrv_tmr->timer[id];
    timer_priv->base = sdrv_tmr->base;
    timer_priv->id = id;
    timer_priv->cnt_per_us = sdrv_tmr->cnt_per_us;
    timer_priv->priv = sdrv_tmr;

    dev = &timer_priv->dev;
    dev->ops = &g_sdrv_tmr_timer_ops;
    dev->priv = timer_priv;
    dev->drv_type = TMR_DRV_CYCLE;

    timer_init(dev);

    return dev;
}

/**
 * @brief sdrv timer dev init
 *
 * @param[in] sdrv_tmr sdrv tmr dev
 * @param[in] cfg sdrv tmr early config
 */
void sdrv_tmr_init(sdrv_tmr_dev_t *sdrv_tmr, const sdrv_tmr_cfg_t *cfg)
{
    sdrv_tmr->base = cfg->base;
    sdrv_tmr->cnt_per_us = (cfg->clk_frq / (cfg->clk_div + 1)) / 1000000;
    sdrv_tmr_lld_init(cfg->base, cfg->clk_sel, cfg->clk_div);

    sdrv_tmr->cascase_mode = cfg->cascase_mode;
    sdrv_tmr_lld_set_cascase_mode(cfg->base, cfg->cascase_mode);

    sdrv_tmr->irq = cfg->irq;
    if (cfg->irq >= 0) {
        irq_attach(cfg->irq, sdrv_tmr_irq_handle, sdrv_tmr);
        irq_enable(cfg->irq);
    }

    sdrv_tmr->ovf_irq = cfg->ovf_irq;
    if (cfg->ovf_irq >= 0) {
        irq_attach(cfg->ovf_irq, sdrv_tmr_irq_handle, sdrv_tmr);
        irq_enable(cfg->ovf_irq);
    }
    sdrv_tmr->ptp_base = cfg->ptp_base;

    spin_lock_init(&sdrv_tmr->spinlock);
}
