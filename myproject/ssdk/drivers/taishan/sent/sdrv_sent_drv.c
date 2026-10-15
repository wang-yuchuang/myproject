/*
 * sdrv_sent_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv sent driver.
 *
 * Revision History:
 * -----------------
 */
#include <compiler.h>
#include <spinlock.h>
#include <string.h>
#include <types.h>
#include <math.h>

#include "debug.h"
#include "driver/irq.h"
#include "driver/taishan/sent/sdrv_sent_drv.h"
#include "sdrv_etimer.h"
#include <lib/ring_buffer.h>
#include <driver/arch_timer.h>

#define SDRV_SENT_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdrv_sent:"

#if SDRV_SENT_DEBUG_LVL >= SSDK_ERR
#define SDRV_SENT_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_SENT_ERR(format, ...)
#endif

#if SDRV_SENT_DEBUG_LVL >= SSDK_WARNING
#define SDRV_SENT_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_SENT_WARN(format, ...)
#endif

#if SDRV_SENT_DEBUG_LVL >= SSDK_INFO
#define SDRV_SENT_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_SENT_INFO(format, ...)
#endif

static void sdrv_sent_init(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
static void sdrv_sent_deinit(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
static void sdrv_sent_recv_start(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
static void sdrv_sent_recv_stop(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
// static void sdrv_sent_parse_msg(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
static uint32_t sdrv_sent_get_datanum(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
static uint32_t sdrv_sent_get_data(sent_dev_t *dev, uint8_t sdrv_sent_chanid);


static void sdrv_chanid_map(uint8_t sdrv_chan_id, sdrv_etimer_cnt_id_e *etimer_cnt_id)
{
    *etimer_cnt_id = (sdrv_etimer_cnt_id_e)(sdrv_chan_id + 2);
}

static int sdrv_sent_irq_handle(uint32_t irq, void *arg)
{
    sent_dev_t *sent_dev = arg;
    uint32_t sent_irq_sta = sdrv_etimer_lld_get_int_status(sent_dev->base);

    for(uint8_t id = SDRV_ETIMR_CHN_A; id < SDRV_SENT_CHAN_NUM; ++id)
    {
        if(sent_irq_sta & ETMR_BM_INT_STA_CHN_DMA_REQ(id))
        {
            sent_parse_msg(sent_dev, id);
            sdrv_etimer_lld_clear_int_status(sent_dev->base, ETMR_BM_INT_STA_CHN_DMA_REQ(id));
            SDRV_SENT_INFO("sdrv_sent_irq_handle : base = %d, chan_id = %d. \n", sent_dev->base, id);
        }
    }
    return 0;
}

static void sdrv_sent_clk_cfg(sent_dev_t *dev)
{
    sdrv_sent_dev_t *sdrv_sent = dev->priv;
    /* set cmp and disable clk mon  */
    sdrv_etimer_lld_clk_mon_en(dev->base, false);
    /* config clk */
    clk_set_t clk_set = {0};
    clk_set.src_clk_sel = sdrv_sent->clk_sel;
    clk_set.div_num = sdrv_sent->clk_div;
    sdrv_etimer_lld_clk_cfg(dev->base, &clk_set);
    /* enable clk mon  */
    sdrv_etimer_lld_clk_mon_en(dev->base, true);
}

static sent_ops_t g_sdrv_sent_ops = {
    .init = sdrv_sent_init,
    .deinit = sdrv_sent_deinit,
    .recv_start = sdrv_sent_recv_start,
    .recv_stop = sdrv_sent_recv_stop,
    // .msg_parse = sdrv_sent_parse_msg,
    .get_datanum = sdrv_sent_get_datanum,
    .get_data = sdrv_sent_get_data,
};

static void sdrv_sent_init(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    sdrv_sent_dev_t *sdrv_sent = dev->priv;
    sdrv_sent_chan_t *sdrv_sent_chan = &sdrv_sent->sdrv_sent_chan[sdrv_sent_chanid];
    sent_chan_t *sent_chan = &dev->chan[sdrv_sent_chanid];
    sdrv_etimer_cnt_id_e etimer_cnt_id;
    sdrv_chanid_map(sdrv_sent_chanid, &etimer_cnt_id);

    /* set timer counter ovf value 0xfffffff */
    sdrv_etimer_lld_cnt_ovf(dev->base, etimer_cnt_id, 0xFFFFFFF);

    /* cnt will start when the first cpt event happen */
    local_cnt_config_t sdrv_lcnt_cfg = {0};
    sdrv_lcnt_cfg.start_by_first_cpt = true;
    sdrv_etimer_lld_local_cnt_cfg(dev->base, etimer_cnt_id, &sdrv_lcnt_cfg);

    /* reset the counter when the cpt0 event happen */
    sdrv_etimer_lld_lcnt_cpt_clr_en(dev->base, etimer_cnt_id, SDRV_ETIMR_LCNT_CPT_0_CLR, true);

    /* enable timer counter */
    sdrv_etimer_lld_cnt_en(dev->base, etimer_cnt_id, true);

    /* Set the triggered edge as falling edge, set the local timer as capture counter */
    cpt_config_t sdrv_cpt_cfg = {0};
    sdrv_cpt_cfg.cpt0_trig_mode = SDRV_ETIMR_CPT_FALL_EDGE;
    sdrv_cpt_cfg.cnt_sel = SDRV_ETIMR_CPT_LCNT;
    sdrv_etimer_lld_cpt_cfg(dev->base, sdrv_sent_chanid, &sdrv_cpt_cfg);
    sdrv_etimer_lld_cpt_config_set(dev->base, sdrv_sent_chanid);

    /* set water mark level value */
    sdrv_etimer_lld_dma_wml(dev->base, sdrv_sent_chanid, sdrv_sent_chan->fifowml);

    /* set dma block as capture */
    chn_etimer_dma_ctrl_t sdrv_dmactrl_cfg = {0};
    sdrv_dmactrl_cfg.block_sel = SDRV_ETIMR_CPT_BLOCK;
    sdrv_dmactrl_cfg.chn_en = 1;
    sdrv_etimer_lld_dma_chn_cfg(dev->base, sdrv_sent_chanid, &sdrv_dmactrl_cfg);

    /* set channel data info */
    if (sent_chan->fast_chan == SENT_STANDARD_FORMAT) {
        sent_chan->frame_dl = sent_chan->data_bitnum;
    } else if (sent_chan->fast_chan == SENT_SINGLE_SECURE_FORMAT) {
        sent_chan->frame_dl = SENT_DATA_WIDTH_24_BIT;
    } else if (sent_chan->fast_chan == SENT_HIGH_SPEED_FORMAT) {
        sent_chan->frame_dl = SENT_DATA_WIDTH_12_BIT;
    }

    /* calculate the expect scaler value */
    sent_chan->expect_scaler = (sdrv_sent->clk_freq / (sdrv_sent->clk_div + 1)) / (sdrv_sent_chan->tickfreq);

    /* enable sent and attach irq handler */
    if(dev->irq >= 0)
    {
        irq_attach(dev->irq, sdrv_sent_irq_handle, dev);
        irq_enable(dev->irq);
    }

}

static void sdrv_sent_deinit(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    /* disable sent irq handler */
    if(dev->irq >= 0)
    {
        irq_disable(dev->irq);
        irq_detach(dev->irq);
    }

    sent_chan_t *sent_chan = &dev->chan[sdrv_sent_chanid];
    sdrv_etimer_cnt_id_e etimer_cnt_id;
    sdrv_chanid_map(sdrv_sent_chanid, &etimer_cnt_id);
    /* Turn off dma req interrupt */
    sdrv_etimer_lld_int_en(dev->base, ETMR_BM_INT_STA_EN_CHN_DMA_REQ(sdrv_sent_chanid), false);
    /* disable cpt module */
    sdrv_etimer_lld_cpt_en(dev->base, sdrv_sent_chanid, false);

    /* restore timer counter ovf value 0 */
    sdrv_etimer_lld_cnt_ovf(dev->base, etimer_cnt_id, 0);

    /* overflow will be updated to shadow register */
    sdrv_etimer_lld_cnt_upd(dev->base, etimer_cnt_id, false, true);

    /* restore the cnt cfg */
    local_cnt_config_t sdrv_lcnt_cfg = {0};
    sdrv_lcnt_cfg.start_by_first_cpt = false;
    sdrv_etimer_lld_local_cnt_cfg(dev->base, etimer_cnt_id, &sdrv_lcnt_cfg);
    sdrv_etimer_lld_lcnt_cpt_clr_en(dev->base, etimer_cnt_id, SDRV_ETIMR_LCNT_CPT_0_CLR, false);

    /* disable timer counter */
    sdrv_etimer_lld_cnt_en(dev->base, etimer_cnt_id, false);

    /* restore the triggered edge, restore the timer as capture counter */
    cpt_config_t sdrv_cpt_cfg = {0};
    sdrv_cpt_cfg.cpt0_trig_mode = SDRV_ETIMR_CPT_RISE_EDGE;
    sdrv_cpt_cfg.cnt_sel = SDRV_ETIMR_CPT_CNTG0;
    sdrv_etimer_lld_cpt_cfg(dev->base, sdrv_sent_chanid, &sdrv_cpt_cfg);
    sdrv_etimer_lld_cpt_config_set(dev->base, sdrv_sent_chanid);

    /* restore water mark level value */
    sdrv_etimer_lld_dma_wml(dev->base, sdrv_sent_chanid, 0);

    /* set dma block as default block  */
    chn_etimer_dma_ctrl_t sdrv_dmactrl_cfg = {0};
    sdrv_dmactrl_cfg.block_sel = SDRV_ETIMR_CMP_BLOCK;
    sdrv_dmactrl_cfg.chn_en = 0;
    sdrv_etimer_lld_dma_chn_cfg(dev->base, sdrv_sent_chanid, &sdrv_dmactrl_cfg);

    /* restore predivider */
    sent_chan->expect_scaler = 0;
    sent_chan->actual_scaler = 0;
    sent_chan->frame_dl = 0;

}

static void sdrv_sent_recv_start(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    /* Turn on dma req interrupt */
    sdrv_etimer_lld_int_en(dev->base, ETMR_BM_INT_STA_EN_CHN_DMA_REQ(sdrv_sent_chanid), true);

    /* enable cpt module */
    sdrv_etimer_lld_cpt_en(dev->base, sdrv_sent_chanid, true);
}

static void sdrv_sent_recv_stop(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    /* Turn off dma req interrupt */
    sdrv_etimer_lld_int_en(dev->base, ETMR_BM_INT_STA_EN_CHN_DMA_REQ(sdrv_sent_chanid), false);

    /* disable cpt module */
    sdrv_etimer_lld_cpt_en(dev->base, sdrv_sent_chanid, false);
}

static uint32_t sdrv_sent_get_datanum(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    uint32_t fifo_sta = sdrv_etimer_lld_get_fifo_sta(dev->base);
    return ((uint32_t)(fifo_sta >> ((8u * sdrv_sent_chanid) + 2u))) & 0x1fu;
}

static uint32_t sdrv_sent_get_data(sent_dev_t *dev, uint8_t sdrv_sent_chanid)
{
    return sdrv_etimer_lld_get_fifo_val(dev->base, sdrv_sent_chanid);
}

sent_dev_t *sdrv_sent_probe(sdrv_sent_dev_t *sdrv_sent, sdrv_sent_config_t *sent_cfg)
{
    /* sent moudle and clk cfg */
    sdrv_sent->hwid = sent_cfg->hwid;
    sdrv_sent->clk_sel = sent_cfg->clk_sel;
    sdrv_sent->clk_freq = sent_cfg->clk_freq;
    sdrv_sent->clk_div = sent_cfg->clk_div;

    /* sent channel cfg and status init */
    sdrv_sent_chan_t *sdrv_sent_chan = NULL;
    sent_chan_t *sent_chan = NULL;
    uint8_t chan_id = 0;
    uint8_t chan_cfg_num = sent_cfg->chan_cfg_num;
    for(uint8_t i = 0; i < chan_cfg_num; ++i)
    {
        chan_id = sent_cfg->chan_cfg[i].chan_id;
        if((chan_id >= 0) && (chan_id < SDRV_SENT_CHAN_NUM))
        {
            sdrv_sent_chan = &sdrv_sent->sdrv_sent_chan[chan_id];
            sent_chan = &sdrv_sent->sent_chan[chan_id];

            sdrv_sent_chan->chan_id = chan_id;
            sdrv_sent_chan->tickfreq = sent_cfg->chan_cfg[i].tickfreq;
            sdrv_sent_chan->fifowml = sent_cfg->chan_cfg[i].fifowml;

            sent_chan->chan_id = chan_id;
            sent_chan->used = true;
            sent_chan->chan_sta = SENT_STATUS_UNINITIALIZED;
            sent_chan->frameover_cb = NULL;
            sent_chan->arg = NULL;
            sent_chan->expect_scaler = 0;
            sent_chan->actual_scaler = 0;
            sent_chan->lowpulse_width = sent_cfg->chan_cfg[i].lowpulse_width;
            sent_chan->pause_pulse = sent_cfg->chan_cfg[i].pause_pulse;
            sent_chan->data_bitnum = sent_cfg->chan_cfg[i].data_bitnum;
            sent_chan->fast_chan = sent_cfg->chan_cfg[i].fast_chan;
            sent_chan->priv = &sdrv_sent->sdrv_sent_chan[chan_id];
        }
    }

    /* sent dev init */
    sent_dev_t *dev = &sdrv_sent->dev;
    dev->base = sent_cfg->base;
    dev->irq = sent_cfg->irq;
    dev->mutex = NULL;
    dev->dev_sta = SENT_STATUS_UNINIT;
    dev->chan_num = SDRV_SENT_CHAN_NUM;
    dev->ops = &g_sdrv_sent_ops;
    dev->priv = sdrv_sent;
    dev->chan = sdrv_sent->sent_chan;

    sdrv_sent_clk_cfg(dev);

    sent_init(dev);

    return dev;
}


