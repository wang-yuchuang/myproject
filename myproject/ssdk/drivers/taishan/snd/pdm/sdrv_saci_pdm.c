/**
 * @file sdrv_saci_pdm.c
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#include <arch/cache.h>
#include <bits.h>
#include <board.h>
#include <debug.h>
#include <driver/irq.h>
#include <taishan/snd/pdm/sdrv_saci_pdm.h>
#include <reg.h>
#include <stdio.h>
#include <string.h>

#include "saci_pdm_regs.h"

#define PDM_DEBUG_LEVEL 2
#define PDM_REG_DEBUG_LEVEL 0

#define PDM_MID_VALUE_16 1 << (16 - 1)

#define SACI_PDM_DEPTH 64
#define PDM_FIFO_PACK_WIDTH_32 7
#define PDM_FIFO_PACK_WIDTH_16 4

static void pdm_delay(int ms) { udelay(ms * 1000); }

#if PDM_REG_DEBUG_LEVEL

#define pdm_readl(reg)                                                         \
    readl(reg);                                                                \
    ssdk_printf(PDM_DEBUG_LEVEL, "r(0x%x, r(0x%08x)\n", reg, readl(reg));
#define pdm_writel(val, reg)                                                   \
    writel(val, reg);                                                          \
    ssdk_printf(PDM_DEBUG_LEVEL, "w(0x%x, 0x%08x), r(0x%08x)\n", reg, val,     \
                readl(reg));
#else

#define pdm_writel(val, reg) writel(val, reg)
#define pdm_readl(reg) readl(reg)

#endif

#define PDM_CHMODE_DISABLE 0x00
#define PDM_CHMODE_LEFT 0x01
#define PDM_CHMODE_RIGHT 0x02
#define PDM_CHMODE_STEREO 0x03

#define PDM_FIFO_LOOSE 0x00
#define PDM_FIFO_PACK 0x01

static uint32_t pdm_rd_reg_bits(uint32_t reg_base, uint32_t mask,
                                uint32_t offset)
{
    uint32_t rd_data = pdm_readl(reg_base);

    return ((rd_data & mask) >> offset);
}

static void pdm_wr_reg_bits(uint32_t reg_base, uint32_t mask, uint32_t offset,
                            uint32_t val)
{
    uint32_t wr_data = pdm_readl(reg_base);

    wr_data = wr_data & (~mask);
    wr_data = wr_data | ((val << offset) & mask);

    pdm_writel(wr_data, reg_base);
}

uint32_t saci_pdm_lld_get_status_irq(uint32_t base_addr)
{
    return pdm_readl(base_addr + PDM_0008_IRQ_STAT);
}

uint32_t saci_pdm_lld_get_mask_irq(uint32_t base_addr)
{
    return pdm_readl(base_addr + PDM_0004_IRQ_MASK);
}

void saci_pdm_lld_clr_rx_fifo_pre_full_irq(uint32_t base_addr, int id)
{

    pdm_wr_reg_bits(base_addr + PDM_0008_IRQ_STAT,
                    PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_MASK(id),
                    PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_POS(id), 1);
}

void saci_pdm_lld_clr_rx_fifo_full_irq(uint32_t base_addr, int id)
{

    pdm_wr_reg_bits(base_addr + PDM_0008_IRQ_STAT,
                    PDM_IRQ_STAT_CHX_FIFO_FULL_MASK(id),
                    PDM_IRQ_STAT_CHX_FIFO_FULL_POS(id), 1);
}

void saci_pdm_lld_clr_rx_fifo_ovr_irq(uint32_t base_addr, int id)
{
    pdm_wr_reg_bits(base_addr + PDM_0008_IRQ_STAT,
                    PDM_IRQ_STAT_CHX_FIFO_OVR_MASK(id),
                    PDM_IRQ_STAT_CHX_FIFO_OVR_POS(id), 1);
}

uint32_t saci_pdm_lld_get_rx_fifo_dpt(uint32_t base_addr, int id)
{
    return pdm_rd_reg_bits(base_addr + PDM_FIFO_STA_X(id), PDM_FIFO_DPT_MASK,
                           PDM_FIFO_DPT_POS);
}

uint32_t saci_pdm_lld_get_rx_fifo_data(uint32_t base_addr, int id)
{
    return pdm_readl(base_addr + PDM_CHX_FIFO_DATA(id));
}

static void saci_pdm_lld_dma_default_int(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;

    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK, PDM_CHX_WID_MISM_MASK(id),
                    PDM_CHX_WID_MISM_POS(id), 1);
    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK, PDM_CHX_FIFO_OVR_MASK(id),
                    PDM_CHX_FIFO_OVR_POS(id), 0);
    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK, PDM_CHX_FIFO_FULL_MASK(id),
                    PDM_CHX_FIFO_FULL_POS(id), 1);
    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK, PDM_CHX_FIFO_EMPTY_MASK(id),
                    PDM_CHX_FIFO_EMPTY_POS(id), 1);
    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK,
                    PDM_CHX_FIFO_PRE_FULL_MASK(id),
                    PDM_CHX_FIFO_PRE_FULL_POS(id), 1);
}

static void saci_pdm_lld_interrpt_init(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;

    saci_pdm_lld_dma_default_int(dev);

    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK,
                    PDM_CHX_FIFO_PRE_FULL_MASK(id),
                    PDM_CHX_FIFO_PRE_FULL_POS(id), 1);
    pdm_wr_reg_bits(base_addr + PDM_0004_IRQ_MASK, PDM_CHX_FIFO_FULL_MASK(id),
                    PDM_CHX_FIFO_FULL_POS(id), 0);
}

/* clk Influence elements : order  dec_rate

case 0 16K
PDMCLK 1024K
16K  filter_order 5  dec_rate 64  width 31
16K  filter_order 4  dec_rate 64  width 25
16K  filter_order 3  dec_rate 64  width 19

PDMCLK 2048K
16K  filter_order 4  dec_rate 128  width 29
16K  filter_order 3  dec_rate 128  width 22



case  1 8K
PDMCLK 1024K

8K  filter_order 4  dec_rate  128  width 29
8K  filter_order 3  dec_rate  128  width 25
8K  filter_order 2  dec_rate  128  width 15   NG

PDMCLK 2048K

8K  filter_order 3  dec_rate  256  width 25
8K  filter_order 2  dec_rate  256  width 17

case 0 32K

PDMCLK 1024K
32K  filter_order 5  dec_rate 32  width 31
32K  filter_order 4  dec_rate 32  width 21
32K  filter_order 3  dec_rate 32  width 16


PDMCLK 2048K

32K  filter_order 5  dec_rate 64  width 31
32K  filter_order 4  dec_rate 64  width 25
32K  filter_order 3  dec_rate 64  width 19


case 0 48K
PDMCLK 3072K
48K  filter_order 5  dec_rate 64  width 31
48K  filter_order 4  dec_rate 64  width 25
48K  filter_order 3  dec_rate 64  width 19

*/

/**
 * @brief
 *
 * @param dev
 * @param pcm_sample_rate
 */
static void saci_pdm_lld_config_clk(snd_dev_t *dev,
                                    snd_sample_rate_t sample_rate)
{

    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    uint32_t base_addr = dev->base;
    uint32_t src_clk_freq = pdm->clk_rate;
    uint32_t dec_rat = 0;
    uint32_t filter_order = 0;
    uint32_t pdm_clk = 0;
    uint32_t pdm_data_with = 0;
    uint32_t div = 0;

    switch (sample_rate) {

    case SND_SR_8000:
        filter_order = PDM_FILTER_3ORDER;
        dec_rat = PDM_DEC_RATE_256;
        pdm_data_with = PDM_DATA_WITH_25BITS;

    case SND_SR_16000:
        filter_order = PDM_FILTER_3ORDER;
        dec_rat = PDM_DEC_RATE_128;
        pdm_data_with = PDM_DATA_WITH_22BITS;
        break;

    case SND_SR_32000:
        filter_order = PDM_FILTER_5ORDER;
        dec_rat = PDM_DEC_RATE_64;
        pdm_data_with = PDM_DATA_WITH_31BITS;
        break;

    case SND_SR_48000:
        filter_order = PDM_FILTER_5ORDER;
        dec_rat = PDM_DEC_RATE_64;
        pdm_data_with = PDM_DATA_WITH_31BITS;
        break;

    default:
        ssdk_printf(PDM_DEBUG_LEVEL,
                    "func<%s>: unsupport sr mode default PDM_SR_16000 ? ?\n",
                    __func__);

        filter_order = PDM_FILTER_4ORDER;
        dec_rat = PDM_DEC_RATE_128;
        pdm_data_with = PDM_DATA_WITH_29BITS;
        sample_rate = SND_SR_16000;
    }

    pdm_clk = sample_rate * (dec_rat);
    div = src_clk_freq / (pdm_clk);
    ssdk_printf(PDM_DEBUG_LEVEL, "func<%s>: pdm_clk=%d .div=%d\n", __func__,
                pdm_clk, div);

    pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_DATA_WITH_MASK,
                    PDM_DATA_WITH_POS, pdm_data_with);

    /* src clk sel */
    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_CLK_SEL_MASK,
                    PDM_CLK_SEL_POS, CLK_DIVIDER_SOURCE_PDM_PER_CLK);
    /* clk pol*/
    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_CLK_POL_MASK,
                    PDM_CLK_POL_POS, 0);
    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_CLK_DIV_MASK,
                    PDM_CLK_DIV_POS, div - 1);

    /* dec_rat Affect pcm_width clock */
    pdm_wr_reg_bits(base_addr + PDM_FILTER_CTRL_X(id), PDM_DEC_RATE_MASK,
                    PDM_DEC_RATE_POS, dec_rat - 1);
    pdm_wr_reg_bits(base_addr + PDM_FILTER_CTRL_X(id), PDM_FLT_ORDER_MASK,
                    PDM_FLT_ORDER_POS, filter_order);
}

static void saci_pdm_lld_start_capture(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    uint32_t dma_mode = pdm->dma_mode;

    if (dma_mode) {
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_DMA_EN_MASK,
                        PDM_DMA_EN_POS, 1);

    } else {
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_DMA_EN_MASK,
                        PDM_DMA_EN_POS, 0);
    }

    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_DIV_CLK_EN_MASK,
                    PDM_DIV_CLK_EN_POS, 1);

    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_CLK_O_EN_MASK,
                    PDM_CLK_O_EN_POS, 1);
    pdm_delay(45);
    pdm_wr_reg_bits(base_addr + PDM_EN_X(id), PDM_EN_MASK, PDM_EN_POS, 1);
    udelay(1);
    pdm_wr_reg_bits(base_addr + PDM_EN_X(id), PDM_FIFO_EN_MASK, PDM_FIFO_EN_POS,
                    1);
}

static void saci_pdm_lld_stop_capture(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    uint32_t dma_mode = pdm->dma_mode;

    pdm_wr_reg_bits(base_addr + PDM_EN_X(id), PDM_EN_MASK, PDM_EN_POS, 0);

    if (dma_mode) {
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_DMA_EN_MASK,
                        PDM_DMA_EN_POS, 0);
    }
}

static uint32_t to_pdm_chan_mode(pdm_channel_t channel)
{
    uint32_t chan_mode = PDM_CHMODE_DISABLE;

    switch (channel) {

    case PDM_CHAN_LEFT:
        chan_mode = PDM_CHMODE_LEFT;
        break;

    case PDM_CHAN_RIGHT:
        chan_mode = PDM_CHMODE_RIGHT;
        break;

    case PDM_CHAN_STEREO:
        chan_mode = PDM_CHMODE_STEREO;
        break;

    default:
        ssdk_printf(SSDK_ERR, "func<%s>: unsupport channel mode? ?\n",
                    __func__);
    }
    return chan_mode;
}

static uint32_t to_pdm_data_gain(pdm_gain_t p_gain)
{
    uint32_t gain = 0x10;

    switch (p_gain) {

    case PDM_GAIN_N3DB:
        gain = PDM_GAIN_VALUE_N3DB;
        break;

    case PDM_GAIN_N6DB:
        gain = PDM_GAIN_VALUE_N6DB;
        break;

    case PDM_GAIN_N9DB:
        gain = PDM_GAIN_VALUE_N9DB;
        break;
    case PDM_GAIN_N12DB:
        gain = PDM_GAIN_VALUE_N12DB;
        break;
    case PDM_GAIN_P0DB:
        gain = PDM_GAIN_VALUE_0DB;
        break;
    case SPDM_GAIN_P3DB:
        gain = PDM_GAIN_VALUE_P3DB;
        break;

    case PDM_GAIN_P6DB:
        gain = PDM_GAIN_VALUE_P6DB;
        break;
    case PDM_GAIN_P9DB:
        gain = PDM_GAIN_VALUE_P9DB;
        break;

    case PDM_GAIN_P12DB:
        gain = PDM_GAIN_VALUE_P12DB;
        break;
    default:
        ssdk_printf(SSDK_ERR, "func<%s>: unsupport gain default 0db \n",
                    __func__);
    }
    return gain;
}

static uint32_t to_pdm_pack_width(snd_sample_width_t sample_width)
{
    uint32_t pack_width = PDM_FIFO_PACK_WIDTH_16;

    switch (sample_width) {

    case SND_SAMPLE_WIDTH_16BITS:
        pack_width = PDM_FIFO_PACK_WIDTH_16;
        break;
    default: {
        ssdk_printf(SSDK_ERR,
                    "func<%s>: unsupport width:%d ,only support  16bit\n",
                    __func__, sample_width);
        pack_width = PDM_FIFO_PACK_WIDTH_16;
    }
    }
    return pack_width;
}

/**
 * @brief
 *
 * @param dev
 * @return true
 * @return false
 */
bool saci_pdm_lld_startup(snd_dev_t *dev, snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    uint32_t reset_stat = 0;
    clk_node_t *clk = dev->clk;

    reset_stat =
        pdm_rd_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                        SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS);

    if (reset_stat) {
        ssdk_printf(PDM_DEBUG_LEVEL, "func<%s>: SACI have reset,no need\n",
                    __func__);
    } else {
        pdm_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                        SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS, 0);
        udelay(1);
        pdm_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                        SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS, 1);
    }

    pdm_wr_reg_bits(base_addr + PDM_0000_SACI_CTRL, PDM_CHX_RST_MASK(id),
                    PDM_CHX_RST_POS(id), 0);
    udelay(1);
    pdm_wr_reg_bits(base_addr + PDM_0000_SACI_CTRL, PDM_CHX_RST_MASK(id),
                    PDM_CHX_RST_POS(id), 1);
    pdm_wr_reg_bits(base_addr + PDM_0000_SACI_CTRL, PDM_CHX_FIFO_FLUSH_MASK(id),
                    PDM_CHX_FIFO_FLUSH_POS(id), 1);

    pdm->dma_mode = true;
    pdm->pack_mode = true;
    pdm->data_lsb = true;
    pdm->threshold = (SACI_PDM_DEPTH / 2);
    dma_data->dma_addr = base_addr + PDM_CHX_FIFO_DATA(id);
    dma_data->fifo_size = SACI_PDM_DEPTH;
    dma_data->maxburst = 32;
    dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;

    pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_THRD_MASK,
                    PDM_THRD_POS, pdm->threshold);

    if (clk) {
        pdm->clk_rate = clk_get_rate(clk);
    } else {
        ssdk_printf(0, "err pdm clk is null !\n");
        return false;
    }

    ssdk_printf(PDM_DEBUG_LEVEL, "pdm threshold=%d,pdm_clk(%d)\n",
                pdm->threshold, pdm->clk_rate);
    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param params
 * @return true
 * @return false
 */
bool saci_pdm_lld_set_params(snd_dev_t *dev, snd_params_t *params,
                             snd_stream_type_t stream)
{
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_params_t *snd_params = &snd_stream->params;
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    unsigned sample_rate = params->sample_rate;
    uint32_t chan_mode = to_pdm_chan_mode(pdm->channel);
    snd_channel_t channels = params->channels;
    uint32_t gain = to_pdm_data_gain(pdm->gain);
    uint32_t pack_with = to_pdm_pack_width(params->sample_width);
    bool pack_mode = pdm->pack_mode;
    bool data_lsb = pdm->data_lsb;
    bool mono = (channels == SND_CHANNEL_NUM_MONO) ? true : false;

    if (!mono) {
        chan_mode = PDM_CHMODE_STEREO;
    } else {
        if (chan_mode == PDM_CHMODE_STEREO) {
            ssdk_printf(PDM_DEBUG_LEVEL,
                        "func<%s>: board config channel mode is diffrent ,use "
                        "left default\n",
                        __func__);
            chan_mode = PDM_CHMODE_LEFT;
        }
    }

    ssdk_printf(PDM_DEBUG_LEVEL, "func<%s>: board config channel mode (%d) \n",
                __func__, chan_mode);

    if (pack_mode) {
        pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id),
                        PDM_DATA_PACK_MODE_MASK, PDM_DATA_PACK_MODE_POS,
                        PDM_FIFO_PACK);
    } else {
        pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id),
                        PDM_DATA_PACK_MODE_MASK, PDM_DATA_PACK_MODE_POS,
                        PDM_FIFO_LOOSE);
    }

    pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_MODE_MASK, PDM_MODE_POS,
                    chan_mode);
    pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_FIFO_PACK_WIDTH_MASK,
                    PDM_FIFO_PACK_WIDTH_POS, pack_with);

    if (PDM_GAIN_P0DB != pdm->gain) {
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_GAIN_EN_MASK,
                        PDM_GAIN_EN_POS, 1);
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_GAIN_VALUE_MASK,
                        PDM_GAIN_VALUE_POS, gain);
    } else {
        pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_GAIN_EN_MASK,
                        PDM_GAIN_EN_POS, 0);
    }

    pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_FIFO16BIT_VLD_MASK,
                    PDM_FIFO16BIT_VLD_POS, 1);

    pdm_wr_reg_bits(base_addr + PDM_MID_VALUE, PDM_MID_VALUE_MASK,
                    PDM_MID_VALUE_POS, PDM_MID_VALUE_16);
    pdm_wr_reg_bits(base_addr + PDM_CTRL_X(id), PDM_MINUS_MID_EN_MASK,
                    PDM_MINUS_MID_EN_POS, 1);

    if (data_lsb) {
        pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_DATA_ALIGN_MASK,
                        PDM_DATA_ALIGN_POS, 0);
    } else {
        pdm_wr_reg_bits(base_addr + PDM_FIFO_CTRL_X(id), PDM_DATA_ALIGN_MASK,
                        PDM_DATA_ALIGN_POS, 1);
    }

    memcpy(snd_params, params, sizeof(snd_params_t));
    saci_pdm_lld_config_clk(dev, sample_rate);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param cmd
 * @return true
 * @return false
 */
bool saci_pdm_lld_trigger(snd_dev_t *dev, snd_trigger_op_t snd_cmd,
                          snd_stream_type_t stream)
{
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;

    switch (snd_cmd) {
    case SND_TRIGGER_START:
        saci_pdm_lld_start_capture(dev);
        break;

    case SND_TRIGGER_STOP:
        saci_pdm_lld_stop_capture(dev);
        break;

    default:
        ssdk_printf(PDM_DEBUG_LEVEL, "func<%s>: pdm snd_cmd not support ? ?\n",
                    __func__);
    }

    if (pdm->dma_mode) {
        saci_pdm_lld_dma_default_int(dev);
    } else {
        saci_pdm_lld_interrpt_init(dev);
    }

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @return true
 * @return false
 */
bool saci_pdm_lld_shutdown(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;

    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_DIV_CLK_EN_MASK,
                    PDM_DIV_CLK_EN_POS, 0);

    pdm_wr_reg_bits(base_addr + PDM_CLK_X_CTRL(id), PDM_CLK_O_EN_MASK,
                    PDM_CLK_O_EN_POS, 0);

    pdm_wr_reg_bits(base_addr + PDM_EN_X(id), PDM_FIFO_EN_MASK, PDM_FIFO_EN_POS,
                    0);

    return true;
}

static snd_ops_t saci_pdm_ops = {
    .startup = saci_pdm_lld_startup,
    .set_params = saci_pdm_lld_set_params,
    .trigger = saci_pdm_lld_trigger,
    .shutdown = saci_pdm_lld_shutdown,
};

static int sdrv_pdm_irq_handler(uint32_t irq, void *arg)
{
    snd_dev_t *dev = (snd_dev_t *)arg;
    saci_pdm_t *pdm = (saci_pdm_t *)dev->priv;
    int id = pdm->id;
    uint32_t base_addr = dev->base;
    uint32_t irq_stat = saci_pdm_lld_get_status_irq(base_addr);
    uint32_t irq_mask = saci_pdm_lld_get_mask_irq(base_addr);
    uint32_t mask_value, irq_value;

    mask_value = !((irq_mask & PDM_CHX_FIFO_PRE_FULL_MASK(id)) >>
                   PDM_CHX_FIFO_PRE_FULL_POS(id));

    irq_value = (irq_stat & PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_MASK(id)) >>
                PDM_IRQ_STAT_CHX_FIFO_PRE_FULL_POS(id);
    if (mask_value & irq_value) {
        saci_pdm_lld_clr_rx_fifo_pre_full_irq(base_addr, id);
    }

    mask_value =
        !((irq_mask & PDM_CHX_FIFO_FULL_MASK(id)) >> PDM_CHX_FIFO_FULL_POS(id));

    irq_value = (irq_stat & PDM_IRQ_STAT_CHX_FIFO_FULL_MASK(id)) >>
                PDM_IRQ_STAT_CHX_FIFO_FULL_POS(id);
    if (mask_value & irq_value) {
        saci_pdm_lld_clr_rx_fifo_full_irq(base_addr, id);
    }

    mask_value =
        !((irq_mask & PDM_CHX_FIFO_OVR_MASK(id)) >> PDM_CHX_FIFO_OVR_POS(id));

    irq_value = (irq_stat & PDM_IRQ_STAT_CHX_FIFO_OVR_MASK(id)) >>
                PDM_IRQ_STAT_CHX_FIFO_OVR_POS(id);
    if (mask_value & irq_value) {
        saci_pdm_lld_clr_rx_fifo_ovr_irq(base_addr, id);
    }

    return 0;
}

void saci_pdm_init(snd_dev_t *dev)
{
    ASSERT(NULL != dev);
    ssdk_printf(SSDK_INFO, "snd pdm inited, dev: %p\n", dev);

    if (dev->irq > 0) {
        irq_attach(dev->irq, sdrv_pdm_irq_handler, dev);
        irq_enable(dev->irq);
    }

    snd_dev_init(dev);
}

snd_dev_t *saci_pdm_probe(saci_pdm_t *saci_pdm, saci_pdm_cfg_t *cfg)
{
    snd_dev_t *dev = &saci_pdm->dev;

    ASSERT(NULL != dev);
    ASSERT(NULL != saci_pdm);
    ASSERT(NULL != cfg);

    dev->priv = saci_pdm;
    dev->ops = &saci_pdm_ops;
    dev->dma_dev = cfg->dma_dev;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->clk = cfg->clk;
    saci_pdm->id = cfg->id;
    saci_pdm->gain = cfg->gain;
    saci_pdm->channel = cfg->channel;
    saci_pdm_init(dev);

    return dev;
}
