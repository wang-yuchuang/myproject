/**
 * @file sdrv_spdif.c
 * @sdrv spdif driver.
 *
 * @Copyright (c) 2022  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <debug.h>
#include <driver/irq.h>
#include <driver/snd/sdrv_spdif.h>
#include <string.h>

#include "reg.h"

/**
 * @brief sdrv spdif write register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] val  vaulue
 */
static inline void sdrv_spdif_writereg(paddr_t base, uint32_t reg, uint32_t val)
{
    writel(val, base + reg);
}

/**
 * @brief sdrv spdif read register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @return vaule
 */
static inline uint32_t sdrv_spdif_readreg(paddr_t base, uint32_t reg)
{
    return readl(base + reg);
}

/**
 * @brief sdrv spdif update register bits
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] mask bit mask
 * @param[in] val  value mask
 */
static inline void sdrv_spdif_update_bits(paddr_t base, uint32_t reg,
                                          uint32_t mask, uint32_t val)
{
    uint32_t temp = sdrv_spdif_readreg(base, reg);

    temp &= ~mask;
    temp |= val;

    sdrv_spdif_writereg(base, reg, temp);
}

/**
 * @brief sdrv spdif startup
 *
 * @param[in] dev audio device instance
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_spdif_startup(snd_dev_t *dev, snd_stream_type_t stream)
{
    paddr_t base_addr = dev->base;
    sdrv_spdif_t *spdif = (sdrv_spdif_t *)dev->priv;

    /* judge if spdif in progress, or return error code */
    if (spdif->spdif_status != SDRV_SPDIF_STATUS_FREE) {
        ssdk_printf(SSDK_ERR, "device busy! spdif status: %d\n",
                    spdif->spdif_status);
        return false;
    }

    /* reset fifo */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_FIFO_ENABLE,
                           VALUE_MASK(0, SPDIF_CTRL_FIFO_ENABLE_SHIFT));
    /* set spdif transfer preamble B num */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_SET_PREAMB_B,
                           VALUE_MASK(0, SPDIF_CTRL_SET_PREAMB_B_SHIFT));
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_INT_REG,
                           BIT_SPDIF_INT_PREAMBLE_DEL,
                           VALUE_MASK(0, SPDIF_INT_PREAMBLE_DEL_SHIFT));
    /* set parity-gen active */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_PARITY_GEN,
                           VALUE_MASK(1, SPDIF_CTRL_PARITY_GEN_SHIFT));
    /* enalbe spdif rx parity check */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_PARITY_CHECK,
                           VALUE_MASK(1, SPDIF_CTRL_PARITY_CHECK_SHIFT));
    /* enalbe spdif rx validty check */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_VALIDITY_CHECK,
                           VALUE_MASK(1, SPDIF_CTRL_VALIDITY_CHECK_SHIFT));
    /* interrupt config */
    /* set interrupt mask */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_UNDERR_MASK,
                           VALUE_MASK(1, SPDIF_CTRL_UNDERR_MASK_SHIFT));
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_OVERR_MASK,
                           VALUE_MASK(1, SPDIF_CTRL_OVERR_MASK_SHIFT));
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_LOCK_MASK,
                           VALUE_MASK(1, SPDIF_CTRL_LOCK_MASK_SHIFT));
    /* set fifo level */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_FIFO_CTRL,
                           BIT_SPDIF_FIFO_CTRL_AEMPTY_THRESHOLD,
                           VALUE_MASK(SPDIF_FIFO_AEMPTY_THRESHOLD,
                                      SPDIF_FIFO_CTRL_AEMPTY_THRESHOLD_SHIFT));
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_FIFO_CTRL,
                           BIT_SPDIF_FIFO_CTRL_AFULL_THRESHOLD,
                           VALUE_MASK(SPDIF_FIFO_AFULL_THRESHOLD,
                                      SPDIF_FIFO_CTRL_AFULL_THRESHOLD_SHIFT));
    /* enable spdif core clk */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_CLK_ENABLE,
                           VALUE_MASK(0, SPDIF_CTRL_CLK_ENABLE_SHIFT));

    return true;
}

/**
 * @brief sdrv spdif shutdown
 *
 * @param[in] dev audio device instance
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_spdif_shutdown(snd_dev_t *dev, snd_stream_type_t stream)
{
    paddr_t base_addr = dev->base;

    /* disable spdif core */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_SPDIF_ENABLE,
                           VALUE_MASK(0, SPDIF_CTRL_SPDIF_ENABLE_SHIFT));
    /* disable spdif core clock */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_CLK_ENABLE,
                           VALUE_MASK(1, SPDIF_CTRL_CLK_ENABLE_SHIFT));
    /* disable fifo interface */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_USE_FIFO_IF,
                           VALUE_MASK(0, SPDIF_CTRL_USE_FIFO_IF_SHIFT));
    /* reset sfr */
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_SFR_ENABLE,
                           VALUE_MASK(0, SPDIF_CTRL_SFR_ENABLE_SHIFT));

    return true;
}

/**
 * @brief sdrv spdif set params
 *
 * @param[in] dev audio device instance
 * @param[in] params audio param
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_spdif_set_params(snd_dev_t *dev, snd_params_t *params,
                                  snd_stream_type_t stream)
{
    paddr_t base_addr = dev->base;
    sdrv_spdif_t *spdif = (sdrv_spdif_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;
    clk_rate_t clk_rate = clk_get_rate(dev->clk);
    uint32_t sample_srate = params->sample_rate;
    uint32_t spdif_srate = SPDIF_SAMPLE_RATE_CALC(clk_rate, sample_srate);
    uint32_t value = 0;

    if (stream == SND_STREAM_PLAYBACK) {
        switch (sample_srate) {
        case SND_SR_16000:
            spdif->spdif_status = SDRV_SPDIF_STATUS_16_OUT;
            break;
        case SND_SR_44100:
            spdif->spdif_status = SDRV_SPDIF_STATUS_44_1_OUT;
            break;
        case SND_SR_48000:
            spdif->spdif_status = SDRV_SPDIF_STATUS_48_OUT;
            break;
        default:
            spdif->spdif_status = SDRV_SPDFI_STATUS_UNDEFINED;
            break;
        }
    }

    dma_data->dma_addr = base_addr + SPDIF_FIFO_ADDR_OFFSET;
    dma_data->maxburst = 16;
    dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;

    value = (params->channels == 2) ? 0 : 1;
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_CHANNEL_MODE,
                           VALUE_MASK(value, SPDIF_CTRL_CHANNEL_MODE_SHIFT));

    value = (stream == SND_STREAM_PLAYBACK) ? 1 : 0;
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_TR_MODE,
                           VALUE_MASK(value, SPDIF_CTRL_TR_MODE_SHIFT));

    /* cfg sample rate */
    sdrv_spdif_update_bits(
        base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL, BIT_SPDIF_CTRL_TSAMPLE_RATE,
        VALUE_MASK(spdif_srate, SPDIF_CTRL_TSAMPLE_RATE_SHIFT));

    return true;
}

/**
 * @brief sdrv spdif enable
 *
 * @param[in] dev audio device instance
 * @param[in] flag true or false
 */
static void sdrv_spdif_enable(snd_dev_t *dev, bool flag)
{
    paddr_t base_addr = dev->base;

    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_SPDIF_ENABLE,
                           VALUE_MASK(flag, SPDIF_CTRL_SPDIF_ENABLE_SHIFT));
    sdrv_spdif_update_bits(base_addr, REG_CDN_SPDIF_REGS_SPDIF_CTRL,
                           BIT_SPDIF_CTRL_INTREQ_MASK,
                           VALUE_MASK(flag, SPDIF_CTRL_INTREQ_MASK_SHIFT));
}

/**
 * @brief sdrv spdif trigger
 *
 * @param[in] dev audio device instance
 * @param[in] snd_cmd command
 * @param[in] stream stream type (playback & capture)
 * @return true or false
 */
static bool sdrv_spdif_trigger(snd_dev_t *dev, snd_trigger_op_t snd_cmd,
                               snd_stream_type_t stream)
{
    switch (snd_cmd) {
    case SND_TRIGGER_START:
        sdrv_spdif_enable(dev, true);
        break;
    case SND_TRIGGER_STOP:
        sdrv_spdif_enable(dev, false);
        break;
    default:
        break;
    }

    return true;
}

/**
 * @brief sdrv spdif get status
 *
 * @param[in] dev  audio device instance
 * @return status
 */
static uint32_t sdrv_spdif_get_status(snd_dev_t *dev)
{
    return sdrv_spdif_readreg(dev->base, REG_CDN_SPDIF_REGS_SPDIF_INT_REG);
}

/**
 * @brief sdrv spdif clear status
 *
 * @param[in] dev  audio device instance
 * @param[in] status status
 */
static void sdrv_spdif_clear_status(snd_dev_t *dev, uint32_t status)
{
    sdrv_spdif_writereg(dev->base, REG_CDN_SPDIF_REGS_SPDIF_INT_REG, status);
}

/**
 * @brief sdrv spdif irq handler
 *
 * @param[in] irq irq number
 * @param[in] arg audio device instance
 */
static int sdrv_spdif_handler(uint32_t irq, void *arg)
{
    snd_dev_t *dev = (snd_dev_t *)arg;
    sdrv_spdif_t *spdif = (sdrv_spdif_t *)dev->priv;
    uint32_t status = sdrv_spdif_get_status(dev);
    uint32_t rx_rate = 0;
    clk_rate_t spdif_clk;
    sdrv_spdif_clear_status(dev, status & (~SPDIF_INT_STATUS_REG_BITS));

    if (status & BIT_SPDIF_INT_TDATA_UNDERR) {
        ssdk_printf(SSDK_ERR, "spdif underrun (%d).\n", irq);
    }

    if (status & BIT_SPDIF_INT_RDATA_OVERR) {
        ssdk_printf(SSDK_ERR, "spdif overrun (%d).\n", irq);
    }

    if (status & BIT_SPDIF_INT_LOCK) {
        rx_rate = status & BIT_SPDIF_INT_RSAMPLE_RATE;
        spdif_clk = clk_get_rate(dev->clk);
        if (rx_rate == SPDIF_SAMPLE_RATE_CALC(spdif_clk, SND_SR_16000)) {
            spdif->spdif_status = SDRV_SPDIF_STATUS_16_IN;
        } else if (rx_rate == SPDIF_SAMPLE_RATE_CALC(spdif_clk, SND_SR_44100)) {
            spdif->spdif_status = SDRV_SPDIF_STATUS_44_1_IN;
        } else if (rx_rate == SPDIF_SAMPLE_RATE_CALC(spdif_clk, SND_SR_48000)) {
            spdif->spdif_status = SDRV_SPDIF_STATUS_48_IN;
        } else {
            spdif->spdif_status = SDRV_SPDFI_STATUS_UNDEFINED;
        }
        ssdk_printf(SSDK_INFO, "spdif rx sample rate: %d.\n",
                    spdif->spdif_status);
    }

    return 0;
}

/**
 * @brief spdif ops
 *
 */
static snd_ops_t sdrv_spdif_ops = {
    .startup = sdrv_spdif_startup,
    .set_params = sdrv_spdif_set_params,
    .trigger = sdrv_spdif_trigger,
    .shutdown = sdrv_spdif_shutdown,
};

/**
 * @brief sdrv spdif init
 *
 * @param[in] spdif spdif instance
 * @param[in] cfg spdif config
 */
snd_dev_t *sdrv_spdif_init(sdrv_spdif_t *spdif, sdrv_spdif_cfg_t *cfg)
{
    ASSERT(NULL != spdif);
    ASSERT(NULL != &spdif->dev);
    ASSERT(NULL != cfg);

    snd_dev_t *dev = &spdif->dev;
    dev->priv = spdif;
    dev->ops = &sdrv_spdif_ops;
    dev->dma_dev = cfg->dma_dev;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->clk = cfg->clk;
    spdif->spdif_status = SDRV_SPDIF_STATUS_FREE;

    snd_dev_init(dev);

    if (dev->irq > 0) {
        irq_attach(dev->irq, sdrv_spdif_handler, dev);
        irq_enable(dev->irq);
    }

    return dev;
}