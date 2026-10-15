/**
 * @file sdrv_i2s_mc.c
 * @sdrv i2s mc driver.
 *
 * @Copyright (c) 2022  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <debug.h>
#include <driver/irq.h>
#include <driver/snd/sdrv_i2s_mc.h>
#include <stdio.h>
#include <string.h>

#include "reg.h"

/**
 * @brief sdrv i2s_mc write register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] val  vaulue
 */
static inline void sdrv_i2s_mc_writereg(paddr_t base, uint32_t reg,
                                        uint32_t val)
{
    writel(val, base + reg);
}

/**
 * @brief sdrv i2s_mc read register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @return vaule
 */
static inline uint32_t sdrv_i2s_mc_readreg(paddr_t base, uint32_t reg)
{
    return readl(base + reg);
}

/**
 * @brief sdrv i2s_mc update register bits
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] mask bit mask
 * @param[in] val  value mask
 */
static inline void sdrv_i2s_mc_update_bits(paddr_t base, uint32_t reg,
                                           uint32_t mask, uint32_t val)
{
    uint32_t temp = sdrv_i2s_mc_readreg(base, reg);

    temp &= ~mask;
    temp |= val;

    sdrv_i2s_mc_writereg(base, reg, temp);
}

/**
 * @brief sdrv i2s_mc startup
 *
 * @param[in] dev audio device instance
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_i2s_mc_startup(snd_dev_t *dev, snd_stream_type_t stream)
{
    paddr_t base_addr = dev->base;
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    if (stream == SND_STREAM_PLAYBACK) { /* playback */
        /* Tx FIFO almost empty */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_TFIFO_CTRL,
                                TAEMPTY_THRESHOLD_MASK,
                                VALUE_MASK(I2S_MC_TFIFO_TAEMPTY_THRESHOLD,
                                           TAEMPTY_THRESHOLD_SHIFT));
        /* Tx FIFO almost full */
        sdrv_i2s_mc_update_bits(
            base_addr, MC_I2S_TFIFO_CTRL, TAFULL_THRESHOLD_MASK,
            VALUE_MASK(I2S_MC_TFIFO_TAFULL_THRESHOLD, TAFULL_THRESHOLD_SHIFT));
        /* fifo reset */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TFIFO_RST_MASK,
                                VALUE_MASK(0, CTRL_TFIFO_RST_SHIFT));

        i2s_mc->playback_status = STREAM_STATUS_STOP;
    } else if (stream == SND_STREAM_CAPTURE) { /* capture */
        /* Rx FIFO almost empty */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_RFIFO_CTRL,
                                RAEMPTY_THRESHOLD_MASK,
                                VALUE_MASK(I2S_MC_RFIFO_RAEMPTY_THRESHOLD,
                                           RAEMPTY_THRESHOLD_SHIFT));
        /* Rx FIFO almost full */
        sdrv_i2s_mc_update_bits(
            base_addr, MC_I2S_RFIFO_CTRL, RAFULL_THRESHOLD_MASK,
            VALUE_MASK(I2S_MC_RFIFO_RAFULL_THRESHOLD, RAFULL_THRESHOLD_SHIFT));
        /* fifo reset */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RFIFO_RST_MASK,
                                VALUE_MASK(0, CTRL_RFIFO_RST_SHIFT));

        i2s_mc->capture_status = STREAM_STATUS_STOP;
    }

    return true;
}

/**
 * @brief sdrv i2s_mc shutdown
 *
 * @param[in] dev audio device instance
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_i2s_mc_shutdown(snd_dev_t *dev, snd_stream_type_t stream)
{
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    if ((i2s_mc->playback_status == STREAM_STATUS_STOP) &&
        (i2s_mc->capture_status == STREAM_STATUS_STOP)) {
        /* sfr reset */
        sdrv_i2s_mc_update_bits(dev->base, MC_I2S_CTRL, CTRL_SFR_RST_MASK,
                                VALUE_MASK(0, CTRL_SFR_RST_SHIFT));
    }

    return true;
}

/**
 * @brief sdrv i2s_mc set params
 *
 * @param[in] dev  audio device instance
 * @param[in] params audio param
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_i2s_mc_set_params(snd_dev_t *dev, snd_params_t *params,
                                   snd_stream_type_t stream)
{
    paddr_t base_addr = dev->base;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;
    clk_rate_t clk_rate = clk_get_rate(dev->clk);
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;
    uint32_t sample_srate = params->sample_rate;
    uint32_t mc_srate =
        I2S_MC_SAMPLE_RATE_CALC(clk_rate, sample_srate, I2S_MC_DATA_WIDTH);
    uint32_t mc_sample_size =
        I2S_MC_SAMPLE_RESOLUTION_TO_CONFIG(params->sample_width);
    uint32_t sample_fmt = 0;
    uint32_t value = 0;

    i2s_mc->is_slave =
        ((params->fmt & SND_DAIFMT_MASTER_MASK) == SND_DAIFMT_CODEC_SLAVE)
            ? false
            : true;

    if ((i2s_mc->is_slave == true) &&
        (i2s_mc->clk_sync_loop_mode != CLK_SYNC_NO_LOOP)) {
        ssdk_printf(SSDK_ERR,
                    "i2s mc no support use clk_sync_loop in slave mode \n");
        return false;
    }

    switch (params->fmt & SND_DAIFMT_FORMAT_MASK) {
    case SND_DAIFMT_STANDARD_PHILLIPS:
        sample_fmt = FMT_VAL_STANDARD_PHILLIPS;
        break;
    case SND_DAIFMT_LEFT_JUSTIFIED:
        sample_fmt = FMT_VAL_LEFT_JUSTIFIED;
        break;
    case SND_DAIFMT_RIGHT_JUSTIFIED:
        sample_fmt = FMT_VAL_RIGHT_JUSTIFIED;
        break;
    case SND_DAIFMT_DSP_A:
    case SND_DAIFMT_DSP_B:
    default:
        ssdk_printf(SSDK_ERR, "i2s mc no support this format: %d \n",
                    params->fmt & SND_DAIFMT_FORMAT_MASK);
        return false;
    }

    if (i2s_mc->clk_sample_edge) {
        sample_fmt |= TRAN_SCK_POLAR_MASK | REC_SCK_POLAR_MASK;
    }

    sdrv_i2s_mc_writereg(base_addr, MC_I2S_DEV_CONF, sample_fmt);

    value = (i2s_mc->is_slave == true) ? 0 : 1;

    if (stream == SND_STREAM_PLAYBACK) { /* playback */

        dma_data->dma_addr = base_addr + I2S_MC_FIFO_OFFSET;
        dma_data->maxburst = 16;
        dma_data->addr_width = DMA_DEV_BUSWIDTH_2_BYTES;

        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, TSAMPLE_RATE_MASK,
                                VALUE_MASK(mc_srate, TSAMPLE_RATE_SHIFT));
        /* fill sample resolution */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, TRESOLUTION_MASK,
                                VALUE_MASK(mc_sample_size, TRESOLUTION_SHIFT));

        if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_NO_LOOP) { /* normal */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_T_MS_MASK,
                                    VALUE_MASK(value, CTRL_T_MS_SHIFT));
        } else {
            /* clk_sync loop need to set receive */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, RSAMPLE_RATE_MASK,
                                    VALUE_MASK(mc_srate, RSAMPLE_RATE_SHIFT));
            sdrv_i2s_mc_update_bits(
                base_addr, MC_I2S_SRR, RRESOLUTION_MASK,
                VALUE_MASK(mc_sample_size, RRESOLUTION_SHIFT));
        }
    } else if (stream == SND_STREAM_CAPTURE) { /* capture */

        dma_data->dma_addr = base_addr + I2S_MC_FIFO_OFFSET;
        dma_data->maxburst = 16;
        dma_data->addr_width = DMA_DEV_BUSWIDTH_2_BYTES;

        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, RSAMPLE_RATE_MASK,
                                VALUE_MASK(mc_srate, RSAMPLE_RATE_SHIFT));
        /* fill sample resolution */
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, RRESOLUTION_MASK,
                                VALUE_MASK(mc_sample_size, RRESOLUTION_SHIFT));

        if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_NO_LOOP) { /* normal */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_R_MS_MASK,
                                    VALUE_MASK(value, CTRL_R_MS_SHIFT));
        } else {
            /* clk_sync loop need to set tramsmit */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_SRR, TSAMPLE_RATE_MASK,
                                    VALUE_MASK(mc_srate, TSAMPLE_RATE_SHIFT));
            sdrv_i2s_mc_update_bits(
                base_addr, MC_I2S_SRR, TRESOLUTION_MASK,
                VALUE_MASK(mc_sample_size, TRESOLUTION_SHIFT));
        }
    }

    if (i2s_mc->clk_sync_loop_mode != CLK_SYNC_NO_LOOP) {
        value = (i2s_mc->clk_sync_loop_mode == CLK_SYNC_TX_LOOP) ? 0 : 1;
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_T_MS_MASK,
                                VALUE_MASK(value, CTRL_T_MS_SHIFT));
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_R_MS_MASK,
                                VALUE_MASK(!value, CTRL_R_MS_SHIFT));
    }

    /* set tx/rx direction */
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->tx_channels_mask, CTRL_TR_CFG_SHIFT),
        VALUE_MASK(i2s_mc->tx_channels_mask, CTRL_TR_CFG_SHIFT));
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->rx_channels_mask, CTRL_TR_CFG_SHIFT),
        VALUE_MASK(~i2s_mc->rx_channels_mask, CTRL_TR_CFG_SHIFT));

    return true;
}

/**
 * @brief sdrv i2s_mc set interrupt
 *
 * @param[in] dev  audio device instance
 */
static void sdrv_i2s_mc_set_intr(snd_dev_t *dev)
{
    paddr_t base_addr = dev->base;

    /* CID setting */
    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, TFIFO_EMPTY_MASK_MASK,
                            VALUE_MASK(0, TFIFO_EMPTY_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, TFIFO_AEMPTY_MASK_MASK,
                            VALUE_MASK(0, TFIFO_AEMPTY_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, TFIFO_FULL_MASK_MASK,
                            VALUE_MASK(0, TFIFO_FULL_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, TFIFO_AFULL_MASK_MASK,
                            VALUE_MASK(0, TFIFO_AFULL_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, RFIFO_EMPTY_MASK_MASK,
                            VALUE_MASK(0, RFIFO_EMPTY_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, RFIFO_AEMPTY_MASK_MASK,
                            VALUE_MASK(0, RFIFO_AEMPTY_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, RFIFO_FULL_MASK_MASK,
                            VALUE_MASK(0, RFIFO_FULL_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, RFIFO_AFULL_MASK_MASK,
                            VALUE_MASK(0, RFIFO_AFULL_MASK_SHIFT));

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CID_CTRL, INTREQ_MASK_MASK,
                            VALUE_MASK(1, INTREQ_MASK_SHIFT));
}

/**
 * @brief sdrv i2s_mc start playback
 *
 * @param[in] dev  audio device instance
 */
static void sdrv_i2s_mc_start_playback(snd_dev_t *dev)
{
    paddr_t base_addr = dev->base;
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->tx_channels_mask, CTRL_EN_SHIFT),
        VALUE_MASK(i2s_mc->tx_channels_mask, CTRL_EN_SHIFT));

    i2s_mc->playback_status = STREAM_STATUS_START;

    /* clk sync */
    if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_TX_LOOP) {
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RSYNC_RST_MASK,
                                VALUE_MASK(1, CTRL_RSYNC_RST_SHIFT));
    }

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TSYNC_RST_MASK,
                            VALUE_MASK(1, CTRL_TSYNC_RST_SHIFT));

    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CID_CTRL,
        VALUE_MASK(i2s_mc->tx_channels_mask, I2S_MASK_SHIFT),
        VALUE_MASK(i2s_mc->tx_channels_mask, I2S_MASK_SHIFT));

    sdrv_i2s_mc_set_intr(dev);
}

/**
 * @brief sdrv i2s_mc start capture
 *
 * @param[in] dev  audio device instance
 */
static void sdrv_i2s_mc_start_capture(snd_dev_t *dev)
{
    paddr_t base_addr = dev->base;
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->rx_channels_mask, CTRL_EN_SHIFT),
        VALUE_MASK(i2s_mc->rx_channels_mask, CTRL_EN_SHIFT));

    i2s_mc->capture_status = STREAM_STATUS_START;

    /* clk sync */
    if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_RX_LOOP) {
        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TSYNC_RST_MASK,
                                VALUE_MASK(1, CTRL_TSYNC_RST_SHIFT));
    }

    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RSYNC_RST_MASK,
                            VALUE_MASK(1, CTRL_RSYNC_RST_SHIFT));

    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CID_CTRL,
        VALUE_MASK(i2s_mc->rx_channels_mask, I2S_MASK_SHIFT),
        VALUE_MASK(i2s_mc->rx_channels_mask, I2S_MASK_SHIFT));

    sdrv_i2s_mc_set_intr(dev);
}

/**
 * @brief sdrv i2s_mc stop playback
 *
 * @param[in] dev  audio device instance
 */
static void sdrv_i2s_mc_stop_playback(snd_dev_t *dev)
{
    paddr_t base_addr = dev->base;
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    /* Disable tx_channels_mask */
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->tx_channels_mask, CTRL_EN_SHIFT),
        VALUE_MASK(~i2s_mc->tx_channels_mask, CTRL_EN_SHIFT));
    /* Then disable interrupt */
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CID_CTRL,
        VALUE_MASK(i2s_mc->tx_channels_mask, INTREQ_MASK_SHIFT),
        VALUE_MASK(~i2s_mc->tx_channels_mask, INTREQ_MASK_SHIFT));

    i2s_mc->playback_status = STREAM_STATUS_STOP;

    if (i2s_mc->capture_status == STREAM_STATUS_STOP) { /* capture no runnig */
        if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_TX_LOOP) {
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RSYNC_RST_MASK,
                                    VALUE_MASK(0, CTRL_RSYNC_RST_SHIFT));
        }

        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TSYNC_RST_MASK,
                                VALUE_MASK(0, CTRL_TSYNC_RST_SHIFT));
    } else { /* capture is running */
        if (i2s_mc->clk_sync_loop_mode !=
            CLK_SYNC_RX_LOOP) { /* no need transmit provid clk */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TSYNC_RST_MASK,
                                    VALUE_MASK(0, CTRL_TSYNC_RST_SHIFT));
        }
    }

    /* tfifo reset */
    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TFIFO_RST_MASK,
                            VALUE_MASK(0, CTRL_TFIFO_RST_SHIFT));
}

/**
 * @brief sdrv i2s_mc stop capture
 *
 * @param[in] dev  audio device instance
 */
static void sdrv_i2s_mc_stop_capture(snd_dev_t *dev)
{
    paddr_t base_addr = dev->base;
    sdrv_i2s_mc_t *i2s_mc = (sdrv_i2s_mc_t *)dev->priv;

    /* Disable rx_channels_mask */
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CTRL,
        VALUE_MASK(i2s_mc->rx_channels_mask, CTRL_EN_SHIFT),
        VALUE_MASK(~i2s_mc->rx_channels_mask, CTRL_EN_SHIFT));
    /* Then disable interrupt */
    sdrv_i2s_mc_update_bits(
        base_addr, MC_I2S_CID_CTRL,
        VALUE_MASK(i2s_mc->rx_channels_mask, INTREQ_MASK_SHIFT),
        VALUE_MASK(~i2s_mc->rx_channels_mask, INTREQ_MASK_SHIFT));

    i2s_mc->capture_status = STREAM_STATUS_STOP;

    if (i2s_mc->playback_status == STREAM_STATUS_STOP) { /* capture no runnig */
        if (i2s_mc->clk_sync_loop_mode == CLK_SYNC_RX_LOOP) {
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_TSYNC_RST_MASK,
                                    VALUE_MASK(0, CTRL_TSYNC_RST_SHIFT));
        }

        sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RSYNC_RST_MASK,
                                VALUE_MASK(0, CTRL_RSYNC_RST_SHIFT));
    } else { /* capture is running */
        if (i2s_mc->clk_sync_loop_mode !=
            CLK_SYNC_TX_LOOP) { /* no need receive provid clk */
            sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RSYNC_RST_MASK,
                                    VALUE_MASK(0, CTRL_RSYNC_RST_SHIFT));
        }
    }

    /* rfifo reset */
    sdrv_i2s_mc_update_bits(base_addr, MC_I2S_CTRL, CTRL_RFIFO_RST_MASK,
                            VALUE_MASK(0, CTRL_RFIFO_RST_SHIFT));
}

/**
 * @brief sdrv i2s_mc trigger
 *
 * @param[in] dev  audio device instance
 * @param[in] snd_cmd command
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_i2s_mc_trigger(snd_dev_t *dev, snd_trigger_op_t snd_cmd,
                                snd_stream_type_t stream)
{
    switch (snd_cmd) {
    case SND_TRIGGER_START:
        if (stream == SND_STREAM_PLAYBACK) {
            sdrv_i2s_mc_start_playback(dev);
        } else {
            sdrv_i2s_mc_start_capture(dev);
        }
        break;
    case SND_TRIGGER_STOP:
        if (stream == SND_STREAM_PLAYBACK) {
            sdrv_i2s_mc_stop_playback(dev);
        } else {
            sdrv_i2s_mc_stop_capture(dev);
        }
        break;
    default:
        break;
    }

    return true;
}

/**
 * @brief sdrv i2s_mc get status
 *
 * @param[in] dev  audio device instance
 * @return status
 */
static uint32_t sdrv_i2s_mc_get_status(snd_dev_t *dev)
{
    return sdrv_i2s_mc_readreg(dev->base, MC_I2S_INTR_STAT);
}

/**
 * @brief sdrv i2s_mc clear status
 *
 * @param[in] dev  audio device instance
 * @param[in] status status
 */
static void sdrv_i2s_mc_clear_status(snd_dev_t *dev, uint32_t status)
{
    sdrv_i2s_mc_writereg(dev->base, MC_I2S_INTR_STAT, status);
}

/**
 * @brief sdrv i2s_mc irq handler
 *
 * @param[in] irq irq number
 * @param[in] arg audio device instance
 */
static int sdrv_i2s_mc_irq_handler(uint32_t irq, void *arg)
{
    snd_dev_t *dev = (snd_dev_t *)arg;
    uint32_t status = sdrv_i2s_mc_get_status(dev);
    uint32_t mask = INTR_OVRERR_CODE_MASK | INTR_RDATA_OVRERR_MASK;
    int chan_code = 0;

    if (status & INTR_RDATA_OVRERR_MASK) {
        sdrv_i2s_mc_clear_status(dev, 0);
        chan_code = (status & INTR_OVRERR_CODE_MASK) >> INTR_OVRERR_CODE_SHIFT;
        ssdk_printf(SSDK_ERR, "i2s_mc chan(0x%x) overrun\n", chan_code);
    }

    if (status & (~mask)) {
        if (status & INTR_TDATA_UNDERR_MASK) {
            chan_code =
                (status & INTR_UNDERR_CODE_MASK) >> INTR_UNDERR_CODE_SHIFT;
            ssdk_printf(SSDK_ERR, "i2s_mc chan(0x%x) underrun\n", chan_code);
        }
        sdrv_i2s_mc_clear_status(dev, INTR_RDATA_OVRERR_MASK);
    }

    return 0;
}

/**
 * @brief i2s_mc ops
 *
 */
static snd_ops_t sdrv_i2s_mc_ops = {
    .startup = sdrv_i2s_mc_startup,
    .set_params = sdrv_i2s_mc_set_params,
    .trigger = sdrv_i2s_mc_trigger,
    .shutdown = sdrv_i2s_mc_shutdown,
};

/**
 * @brief sdrv i2s_mc init
 *
 * @param[in] i2s_mc i2s_mc instance
 * @param[in] cfg i2s_mc config
 */
snd_dev_t *sdrv_i2s_mc_init(sdrv_i2s_mc_t *i2s_mc, sdrv_i2s_mc_cfg_t *cfg)
{
    ASSERT(NULL != i2s_mc);
    ASSERT(NULL != &i2s_mc->dev);
    ASSERT(NULL != cfg);

    snd_dev_t *dev = &i2s_mc->dev;
    dev->priv = i2s_mc;
    dev->ops = &sdrv_i2s_mc_ops;
    dev->dma_dev = cfg->dma_dev;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->clk = cfg->clk;

    i2s_mc->clk_sync_loop_mode = cfg->loop_mode;
    i2s_mc->clk_sample_edge = cfg->clk_polar;
    i2s_mc->tx_channels_mask = cfg->tx_ch_mask;
    i2s_mc->rx_channels_mask = cfg->rx_ch_mask;

    snd_dev_init(dev);

    if (dev->irq > 0) {
        irq_attach(dev->irq, sdrv_i2s_mc_irq_handler, dev);
        irq_enable(dev->irq);
    }

    return dev;
}