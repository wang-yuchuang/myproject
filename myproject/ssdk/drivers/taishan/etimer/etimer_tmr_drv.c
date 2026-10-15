/*
 * @FilePath: \ssdk\drivers\timer\etimer_tmr_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: ping.yu
 * @Date: 2021-07-08 15:59:55
 * @Descripttion: taishan timer hld driver
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "etimer_tmr_drv.c "
#include <driver/clk.h>
#include <errno.h>
#include <string.h>
#include <types.h>
#include "driver/taishan/etimer/etimer_tmr_drv.h"
#include "bits.h"
#include "debug.h"
#include "driver/irq.h"
#include "driver/timer.h"
#include "reg.h"
#include "sdrv_etimer.h"

static void etimer_tmr_start(timer_dev_t *dev, uint32_t time_out);
static void etimer_tmr_stop(timer_dev_t *dev);
static uint32_t etimer_tmr_get_current_time(timer_dev_t *dev);
static uint64_t etimer_tmr_get_global_time(timer_dev_t *dev);

static timer_ops_t g_etimer_tmr_ops = {
    .start = etimer_tmr_start,
    .stop = etimer_tmr_stop,
    .current = etimer_tmr_get_current_time,
    .global = etimer_tmr_get_global_time,
};

static void etimer_tmr_start(timer_dev_t *dev, uint32_t time_out)
{
    etimer_tmr_priv_t *etimer = dev->priv;
    uint32_t ovf_val = time_out * etimer->cnt_per_us * 1000 - 1;
    uint32_t int_sig = ETMR_BM_INT_STA_CNT_OVF(etimer->id);
    sdrv_etimer_lld_cnt_ovf(etimer->base, etimer->id, ovf_val);
    sdrv_etimer_lld_int_enable(etimer->base, int_sig);
    sdrv_etimer_lld_cnt_en(etimer->base, etimer->id, 1);
}

static void etimer_tmr_stop(timer_dev_t *dev)
{
    etimer_tmr_priv_t *etimer = dev->priv;
    sdrv_etimer_lld_int_enable(etimer->base, 0);
    sdrv_etimer_lld_cnt_en(etimer->base, etimer->id, 0);
    uint32_t sw_rst = 0x3F;
    sdrv_etimer_lld_sw_rst(etimer->base, sw_rst);
}

static uint32_t etimer_tmr_get_current_time(timer_dev_t *dev)
{
    etimer_tmr_priv_t *etimer = dev->priv;
    uint32_t current;
    current = sdrv_etimer_lld_get_cnt_value(etimer->base, etimer->id);
    return ((current + etimer->cnt_per_us / 2) / etimer->cnt_per_us);
}

void etimer_tmr_cascade_init(uint32_t base, uint32_t clk, uint32_t div)
{
    clk_set_t clock_set;
    clock_set.div_num = div;
    clock_set.src_clk_sel = clk;
    sdrv_etimer_lld_clk_mon_en(base, false);
    sdrv_etimer_lld_clk_cfg(base, &clock_set);
    sdrv_etimer_lld_clk_mon_en(base, true);
    sdrv_etimer_lld_cnt_ovf(base, SDRV_ETIMR_CNT_G0, 0xFFFFFFFF);
    sdrv_etimer_lld_cnt_ovf(base, SDRV_ETIMR_CNT_G1, 0xFFFFFFFF);
    basic_cnt_config_t cnt_config;
    memset(&cnt_config, 0, sizeof(cnt_config));
    cnt_config.cascade_mode = 1;
    sdrv_etimer_lld_basic_cnt_cfg(base, SDRV_ETIMR_CNT_G0, &cnt_config);
    sdrv_etimer_lld_cnt_en(base, SDRV_ETIMR_CNT_G0, 1);
}

uint64_t etimer_tmr_get_cascade_cnt(uint32_t base)
{
    uint32_t g0_cnt, g0_cnt_pre, g1_cnt;

    g0_cnt = sdrv_etimer_lld_get_cnt_value(base, SDRV_ETIMR_CNT_G0);
    do
    {
        g0_cnt_pre = g0_cnt;
        g1_cnt = sdrv_etimer_lld_get_cnt_value(base, SDRV_ETIMR_CNT_G1);
        g0_cnt = sdrv_etimer_lld_get_cnt_value(base, SDRV_ETIMR_CNT_G0);
    } while (g0_cnt_pre > g0_cnt);

    return (((uint64_t)g1_cnt) << 32) + g0_cnt;
}

static uint64_t etimer_tmr_get_global_time(timer_dev_t *dev)
{
    etimer_tmr_priv_t *etimer = dev->priv;
    etimer_tmr_dev_t *sdrv_dev = etimer->priv;
    uint64_t value;
    irq_state_t state;

    state = spin_lock_irqsave(&sdrv_dev->spinlock);
    value = etimer_tmr_get_cascade_cnt(etimer->base);
    spin_unlock_irqrestore(&sdrv_dev->spinlock, state);
    return ((value + etimer->cnt_per_us / 2) / etimer->cnt_per_us);
}

timer_dev_t *etimer_tmr_probe(etimer_tmr_dev_t *etimer_tmr, etimer_local_tmr_id_e id)
{
    etimer_tmr_priv_t *timer_priv;
    timer_dev_t *dev;

    timer_priv = &etimer_tmr->etiemr_tmr[id];
    timer_priv->base = etimer_tmr->base;
    timer_priv->id = id + 2;
    timer_priv->cnt_per_us = etimer_tmr->cnt_per_us;
    timer_priv->priv = etimer_tmr;
    dev = &timer_priv->dev;
    dev->ops = &g_etimer_tmr_ops;
    dev->priv = timer_priv;
    dev->drv_type = TMR_DRV_CYCLE;
    timer_init(dev);
    return dev;
}

static int etimer_tmr_irq_handle(uint32_t irq, void *arg)
{
    etimer_tmr_dev_t *sdrv_etimer_tmr = arg;
    uint32_t state;
    state = sdrv_etimer_lld_get_int_status(sdrv_etimer_tmr->base);
    sdrv_etimer_lld_clear_int_status(sdrv_etimer_tmr->base, state);
    for (uint8_t id = ETIMR_LTMR_A; id < ETIMR_LTMR_NUM; id++)
    {
        if (state & ETMR_BM_INT_STA_CNT_OVF(id + 2))
        {
            etimer_tmr_priv_t *etimer_priv = &sdrv_etimer_tmr->etiemr_tmr[id];
            timer_expiry(&etimer_priv->dev);
        }
    }
    return 0;
}

void etimer_tmr_init(etimer_tmr_dev_t *etimer_tmr,
                     const etimer_tmr_cfg_t *cfg)
{
    etimer_tmr->base = cfg->base;
    etimer_tmr->cnt_per_us = (cfg->clk_frq / (cfg->clk_div + 1)) / 1000000;
    etimer_tmr_cascade_init(cfg->base, cfg->clk_sel, cfg->clk_div);

    etimer_tmr->irq = cfg->irq;
    sdrv_etimer_irq_e ovf_irq = cfg->irq;
    if (ovf_irq >= 0)
    {
        irq_attach(ovf_irq, etimer_tmr_irq_handle, etimer_tmr);
        irq_enable(ovf_irq);
    }

    spin_lock_init(&etimer_tmr->spinlock);
}