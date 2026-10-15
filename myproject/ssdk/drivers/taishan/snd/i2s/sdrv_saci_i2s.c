/**
 * @file sdrv_saci_i2s.c
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
#include <taishan/snd/i2s/sdrv_saci_i2s.h>
#include <reg.h>
#include <stdio.h>
#include <string.h>

#include "saci_regs.h"

#define SACI_DEBUG_LEVEL 1
#define SACI_REG_DEBUG_LEVEL 1

#if SACI_REG_DEBUG_LEVEL
#define saci_readl(reg)                                                        \
    readl(reg);                                                                \
    ssdk_printf(SACI_DEBUG_LEVEL, "r(0x%x, r(0x%08x)\n", reg, readl(reg));
#define saci_writel(val, reg)                                                  \
    writel(val, reg);                                                          \
    ssdk_printf(SACI_DEBUG_LEVEL, "w(0x%x, 0x%08x), r(0x%08x)\n", reg, val,    \
                readl(reg));
#else
#define saci_writel(val, reg) writel(val, reg)
#define saci_readl(reg) readl(reg)
#endif

#define CLK_DIVIDER_SOURCE_TX_I2S_CLK 0
#define CLK_DIVIDER_SOURCE_RX_I2S_CLK 1
#define CLK_DIVIDER_SOURCE_I2S_MCLK 2
#define CLK_DIVIDER_SOURCE_EXT_CLK 3
#define IIS_MASTER 0
#define IIS_SLAVE 1

#define RECEIVER_ASYNC 0
#define RECEIVER_SYNCFROM_INTX 1
#define RECEIVER_SYNCFROM_EXTX 2
#define RECEIVER_SYNCFROM_EXRX 3

#define TRANSMITTER_ASYNC 0
#define TRANSMITTER_SYNCFROM_INRX 1
#define TRANSMITTER_SYNCFROM_EXTX 2
#define TRANSMITTER_SYNCFROM_EXRX 3

#define SACI_I2S_STANDARD_PHILLIPS 0x00
#define SACI_I2S_RIGHT_JUSTIFIED 0x01
#define SACI_I2S_LEFT_JUSTIFIED 0x02
#define SACI_I2S_DSP_A 0x03
#define SACI_I2S_TDM 0x04
#define SACI_I2S_AC97 0x05
#define SACI_I2S_DSP_B 0x06

#define SACI_TX_DEPTH 512
#define SACI_RX_DEPTH 512
#define SACI_PDM_DEPTH 64
#define SACI_DMA_BURST_SETERO 32

#define SACI_IIS_XFER_COMPLETE (0x1u)

#define SACI_WS_LEN_CHANNEL_WITH 0x0
#define SACI_WS_LEN_ONE_TSCK 0x1

#define TDM_SELCH(n) ((__builtin_ffs(n) - 1) & 0xf)

#define DATA_WIDTH_8BITS 0
#define DATA_WIDTH_10BITS 1
#define DATA_WIDTH_12BITS 2
#define DATA_WIDTH_14BITS 3
#define DATA_WIDTH_16BITS 4
#define DATA_WIDTH_20BITS 5
#define DATA_WIDTH_24BITS 6
#define DATA_WIDTH_32BITS 7

#define SLOT_WIDTH_8BITS 0
#define SLOT_WIDTH_10BITS 1
#define SLOT_WIDTH_12BITS 2
#define SLOT_WIDTH_14BITS 3
#define SLOT_WIDTH_16BITS 4
#define SLOT_WIDTH_20BITS 5
#define SLOT_WIDTH_24BITS 6
#define SLOT_WIDTH_32BITS 7
#define DIV_ROUND_UP(n, d) (((n) + (d)-1u) / (d))

static uint32_t saci_rd_reg_bits(uint32_t reg_base, uint32_t mask,
                                 uint32_t offset)
{
    uint32_t rd_data = saci_readl(reg_base);

    return ((rd_data & mask) >> offset);
}

static void saci_wr_reg_bits(uint32_t reg_base, uint32_t mask, uint32_t offset,
                             uint32_t val)
{
    uint32_t wr_data = saci_readl(reg_base);

    wr_data = wr_data & (~mask);
    wr_data = wr_data | ((val << offset) & mask);

    saci_writel(wr_data, reg_base);
}

static inline int bit_count(int x)
{
    int n;

    for (n = 0; x != 0; x &= (x - 1)) {
        n++;
    }

    return n;
}

static unsigned int to_saci_data_width(snd_sample_width_t width)
{
    unsigned int datawidth = 0;

    switch (width) {
    case SND_SAMPLE_WIDTH_8BITS:
        datawidth = DATA_WIDTH_8BITS;
        break;
    case SND_SAMPLE_WIDTH_10BITS:
        datawidth = DATA_WIDTH_10BITS;
        break;

    case SND_SAMPLE_WIDTH_12BITS:
        datawidth = DATA_WIDTH_12BITS;
        break;

    case SND_SAMPLE_WIDTH_14BITS:
        datawidth = DATA_WIDTH_14BITS;
        break;
    case SND_SAMPLE_WIDTH_16BITS:
        datawidth = DATA_WIDTH_16BITS;
        break;
    case SND_SAMPLE_WIDTH_20BITS:
        datawidth = DATA_WIDTH_20BITS;
        break;
    case SND_SAMPLE_WIDTH_24BITS:
        datawidth = DATA_WIDTH_24BITS;
        break;
    case SND_SAMPLE_WIDTH_32BITS:
        datawidth = DATA_WIDTH_32BITS;
        break;

    default:
        datawidth = DATA_WIDTH_16BITS;
    }
    return datawidth;
}

static unsigned int to_saci_slot_width(snd_slot_width_t width)
{
    unsigned int slot_width = 0;

    switch (width) {
    case SND_SLOT_WIDTH_8BITS:
        slot_width = SLOT_WIDTH_8BITS;
        break;
    case SND_SLOT_WIDTH_10BITS:
        slot_width = SLOT_WIDTH_10BITS;
        break;

    case SND_SLOT_WIDTH_12BITS:
        slot_width = SLOT_WIDTH_12BITS;
        break;

    case SND_SLOT_WIDTH_14BITS:
        slot_width = SLOT_WIDTH_14BITS;
        break;
    case SND_SLOT_WIDTH_16BITS:
        slot_width = SLOT_WIDTH_16BITS;
        break;
    case SND_SLOT_WIDTH_20BITS:
        slot_width = SLOT_WIDTH_20BITS;
        break;
    case SND_SLOT_WIDTH_24BITS:
        slot_width = SLOT_WIDTH_24BITS;
        break;
    case SND_SLOT_WIDTH_32BITS:
        slot_width = SLOT_WIDTH_32BITS;
        break;

    default:
        slot_width = SLOT_WIDTH_16BITS;
    }
    return slot_width;
}

static void saci_iis_lld_interrpt_init(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;

    if (stream == SND_STREAM_PLAYBACK) {

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_UDR_MASK, SACI_0004_TX_FIFO_UDR_POS,
                         1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_FULL_MASK,
                         SACI_0004_TX_FIFO_FULL_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_EMPTY_MASK,
                         SACI_0004_TX_FIFO_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_WIDTH_MISM_MASK,
                         SACI_0004_TX_WIDTH_MISM_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_OVR_MASK, SACI_0004_TX_FIFO_OVR_POS,
                         1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_POS, 1);

    } else if (stream == SND_STREAM_CAPTURE) {

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_OVR_MASK, SACI_0004_RX_FIFO_OVR_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_TIMEOUT_MASK, SACI_0004_RX_TIMEOUT_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_FULL_MASK,
                         SACI_0004_RX_FIFO_FULL_POS, 1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_EMPTY_MASK,
                         SACI_0004_RX_FIFO_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_WIDTH_MISM_MASK,
                         SACI_0004_RX_WIDTH_MISM_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_POS, 1);
    }
}

static void saci_iis_lld_interrpt_reset(snd_dev_t *dev,
                                        snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;

    if (stream == SND_STREAM_PLAYBACK) {

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_UDR_MASK, SACI_0004_TX_FIFO_UDR_POS,
                         1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_FULL_MASK,
                         SACI_0004_TX_FIFO_FULL_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_EMPTY_MASK,
                         SACI_0004_TX_FIFO_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_WIDTH_MISM_MASK,
                         SACI_0004_TX_WIDTH_MISM_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_MASK,
                         SACI_0004_TX_FIFO_PRE_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_TX_FIFO_OVR_MASK, SACI_0004_TX_FIFO_OVR_POS,
                         1);

    } else if (stream == SND_STREAM_CAPTURE) {

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_OVR_MASK, SACI_0004_RX_FIFO_OVR_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_TIMEOUT_MASK, SACI_0004_RX_TIMEOUT_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_FULL_MASK,
                         SACI_0004_RX_FIFO_FULL_POS, 1);

        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_EMPTY_MASK,
                         SACI_0004_RX_FIFO_EMPTY_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_WIDTH_MISM_MASK,
                         SACI_0004_RX_WIDTH_MISM_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_0004_SACI_IRQ_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_MASK,
                         SACI_0004_RX_FIFO_PRE_FULL_POS, 1);
    }
}

void saci_i2s_lld_clear_tx_fifo_empty_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_TX_FIFO_EMPTY_MASK, SACI_0008_TX_FIFO_EMPTY_POS,
                     1);
}

void saci_i2s_lld_clear_tx_fifo_pre_empty_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_TX_FIFO_PRE_EMPTY_MASK,
                     SACI_0008_TX_FIFO_PRE_EMPTY_POS, 1);
}

void saci_i2s_lld_clear_tx_fifo_udr_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_TX_FIFO_UDR_MASK, SACI_0008_TX_FIFO_UDR_POS, 1);
}
void saci_i2s_lld_clear_rx_fifo_prefull_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_RX_FIFO_PRE_FULL_MASK,
                     SACI_0008_RX_FIFO_PRE_FULL_POS, 1);
}
void saci_i2s_lld_clear_rx_fifo_full_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_RX_FIFO_FULL_MASK, SACI_0008_RX_FIFO_FULL_POS,
                     1);
}
void saci_i2s_lld_clear_rx_fifo_ovr_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_RX_FIFO_OVR_MASK, SACI_0008_RX_FIFO_OVR_POS, 1);
}

void saci_i2s_lld_clear_rx_fifo_timeout_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_RX_TIMEOUT_MASK, SACI_0008_RX_TIMEOUT_POS, 1);
}

void saci_i2s_lld_clear_rx_fifo_empty_irq(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0008_SACI_IRQ_STAT,
                     SACI_0008_RX_FIFO_EMPTY_MASK, SACI_0008_RX_FIFO_EMPTY_POS,
                     1);
}

static void saci_i2s_lld_enable_chx(uint32_t base_addr, uint8_t ch_sel)
{
    uint32_t reg_val, ch_val;

    reg_val = saci_rd_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL,
                               SACI_000C_CHx_EN_MASK, SACI_000C_CHx_EN_POS);
    ch_val = reg_val | ch_sel;
    saci_wr_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL, SACI_000C_CHx_EN_MASK,
                     SACI_000C_CHx_EN_POS, ch_val);
}

static void saci_i2s_lld_disable_chx(uint32_t base_addr, uint8_t ch_sel)
{
    uint32_t reg_val, ch_val;

    reg_val = saci_rd_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL,
                               SACI_000C_CHx_EN_MASK, SACI_000C_CHx_EN_POS);
    ch_val = ~(ch_sel);
    ch_val = reg_val & ch_val;
    saci_wr_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL, SACI_000C_CHx_EN_MASK,
                     SACI_000C_CHx_EN_POS, ch_val);
}

static void saci_i2s_lld_config_tx_data_line(uint32_t base_addr, uint8_t ch_sel)
{
    uint32_t reg_val, ch_val;

    saci_i2s_lld_enable_chx(base_addr, ch_sel);
    reg_val = saci_rd_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL,
                               SACI_000C_CHx_DIR_MASK, SACI_000C_CHx_DIR_POS);
    ch_val = reg_val | ch_sel;
    saci_wr_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL, SACI_000C_CHx_DIR_MASK,
                     SACI_000C_CHx_DIR_POS, ch_val);
}

static void saci_i2s_lld_config_rx_data_line(uint32_t base_addr, uint8_t ch_sel)
{
    uint32_t reg_val, ch_val;

    saci_i2s_lld_enable_chx(base_addr, ch_sel);
    reg_val = saci_rd_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL,
                               SACI_000C_CHx_DIR_MASK, SACI_000C_CHx_DIR_POS);
    ch_val = ~(ch_sel);
    ch_val = reg_val & ch_val;
    saci_wr_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL, SACI_000C_CHx_DIR_MASK,
                     SACI_000C_CHx_DIR_POS, ch_val);
}

static void saci_i2s_lld_reset_chx(uint32_t base_addr, uint8_t che_sel)
{
    uint32_t reg_val, ch_val;

    saci_i2s_lld_disable_chx(base_addr, che_sel);
    reg_val = saci_rd_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL,
                               SACI_000C_CHx_DIR_MASK, SACI_000C_CHx_DIR_POS);
    ch_val = ~(che_sel);
    reg_val = ((reg_val & SACI_000C_CHx_DIR_MASK) >> SACI_000C_CHx_DIR_POS);
    ch_val = reg_val & ch_val;
    saci_wr_reg_bits(base_addr + SACI_000C_SACI_CH_CTRL, SACI_000C_CHx_DIR_MASK,
                     SACI_000C_CHx_DIR_POS, ch_val);
}

uint32_t saci_i2s_lld_get_status_irq(uint32_t base_addr)
{
    return saci_readl(base_addr + SACI_0008_SACI_IRQ_STAT);
}

uint32_t saci_i2s_lld_get_mask_irq(uint32_t base_addr)
{
    return saci_readl(base_addr + SACI_0004_SACI_IRQ_MASK);
}

uint32_t saci_i2s_lld_get_tx_fifo_dpt(uint32_t base_addr)
{
    return saci_rd_reg_bits(base_addr + SACI_100C_SACI_TX_FIFO_STA,
                            SACI_100C_FIFO_DPT_MASK, SACI_100C_FIFO_DPT_POS);
}

void saci_i2s_lld_write_tx_fifo_data(uint32_t base_addr, uint32_t sample)
{
    saci_writel(sample, base_addr + SACI_2000_SACI_TX_FIFO_DATA);
}

uint32_t saci_i2s_lld_get_rx_fifo_dpt(uint32_t base_addr)
{
    return saci_rd_reg_bits(base_addr + SACI_300C_SACI_RX_FIFO_STA,
                            SACI_300C_FIFO_DPT_MASK, SACI_300C_FIFO_DPT_POS);
}

uint32_t saci_i2s_lld_read_rx_fifo_data(uint32_t base_addr)
{
    return saci_readl(base_addr + SACI_4000_SACI_RX_FIFO_DATA);
}

void saci_i2s_lld_tx_fifo_reset(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                     SACI_0000_TX_FIFO_FLUSH_MASK, SACI_0000_TX_FIFO_FLUSH_POS,
                     1);
}

void saci_i2s_lld_rx_fifo_reset(uint32_t base_addr)
{
    saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                     SACI_0000_RX_FIFO_FLUSH_MASK, SACI_0000_RX_FIFO_FLUSH_POS,
                     1);
}

static void saci_i2s_lld_config_clk(snd_dev_t *dev, uint32_t freq,
                                    snd_stream_type_t stream)
{

    uint32_t div = 0;
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    uint32_t src_clk_freq = saci_i2s->clk_rate;
    bool slave = (stream == SND_STREAM_PLAYBACK) ? saci_i2s->tx_slave
                                                 : saci_i2s->rx_slave;
    /* Don't apply to slave mode */
    if (slave) {
        ssdk_printf(0, "no need set div for slave\n");
        return;
    }

    /*  div calc div */
    div = src_clk_freq / freq;

    if (div >= 1) {
        div = div - 1;
    }

    if (stream == SND_STREAM_PLAYBACK) {

        /* sel clk source  CLK_DIVIDER_SOURCE_I2S_MCLK */
        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_SEL_MASK, SACI_1000_CLK_SEL_POS,
                         CLK_DIVIDER_SOURCE_TX_I2S_CLK);

        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_DIV_MASK, SACI_1000_CLK_DIV_POS, div);
    } else if (stream == SND_STREAM_CAPTURE) {
        /* sel clk source  CLK_DIVIDER_SOURCE_I2S_MCLK */
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_SEL_MASK, SACI_3000_CLK_SEL_POS,
                         CLK_DIVIDER_SOURCE_RX_I2S_CLK);
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_DEV_MASK, SACI_3000_CLK_DEV_POS, div);
    }
}

static uint32_t saci_i2s_lld_calculate_freq(snd_dev_t *dev,
                                            snd_stream_type_t stream)
{
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_params_t *params = &snd_stream->params;
    uint32_t channels = params->channels;
    unsigned int srate = params->sample_rate;
    uint32_t word_width = params->sample_width;
    uint32_t slot_width = word_width;
    uint32_t slots = (channels == 1) ? 2 : channels;
    uint32_t freq = 0;

    if (stream == SND_STREAM_PLAYBACK) {
        if (saci_i2s->tx_slots) {
            slots = saci_i2s->tx_slots;
        }
        if (saci_i2s->tx_slot_width) {
            slot_width = saci_i2s->tx_slot_width;
        }
    } else {
        if (saci_i2s->rx_slots) {
            slots = saci_i2s->rx_slots;
        }
        if (saci_i2s->rx_slot_width) {
            slot_width = saci_i2s->rx_slot_width;
        }
    }

    freq = slots * slot_width * srate;

    return freq;
}

static bool saci_i2s_lld_set_tx_master_clk(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[SND_STREAM_PLAYBACK];
    snd_params_t *params = &snd_stream->params;
    unsigned int srate = params->sample_rate;
    uint32_t channels = params->channels;
    uint32_t word_width = params->sample_width;
    uint32_t slot_width = word_width;
    uint32_t freq = 0;
    uint32_t slots = (channels == 1) ? 2 : channels;

    if (saci_i2s->tx_slots) {
        slots = saci_i2s->tx_slots;
    }
    if (saci_i2s->tx_slot_width) {
        slot_width = saci_i2s->tx_slot_width;
    }

    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL, SACI_1004_TX_MODE_MASK,
                     SACI_1004_TX_MODE_POS, IIS_MASTER);
    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                     SACI_1004_TX_SYNC_MODE_MASK, SACI_1004_TX_SYNC_MODE_POS,
                     TRANSMITTER_ASYNC);

    freq = slots * slot_width * srate;

    saci_i2s_lld_config_clk(dev, freq, SND_STREAM_PLAYBACK);
    saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                     SACI_1000_DIV_CLK_EN_MASK, SACI_1000_DIV_CLK_EN_POS, 1);
    saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                     SACI_1000_CLK_O_EN_MASK, SACI_1000_CLK_O_EN_POS, 0);
    return true;
}

static bool saci_i2s_lld_set_format_tx(snd_dev_t *dev, unsigned int fmt)
{

    uint32_t base_addr = dev->base;

    switch (fmt & SND_DAIFMT_FORMAT_MASK) {

    case SND_DAIFMT_STANDARD_PHILLIPS:

        /* config mode */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TIMING_MODE_MASK, SACI_1004_TIMING_MODE_POS,
                         SACI_I2S_STANDARD_PHILLIPS);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_POL_MASK, SACI_1004_TX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TSCK_POL_MASK, SACI_1004_TSCK_POL_POS, 0);
        /* config WS 1 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_OFFSET_MASK,
                         SACI_1004_TX_WS_OFFSET_POS, 1);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_DATA_ALIGN_MASK, SACI_1004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_ALIGN_MASK, SACI_1008_DATA_ALIGN_POS,
                         0);

        break;

    case SND_DAIFMT_RIGHT_JUSTIFIED:
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TIMING_MODE_MASK, SACI_1004_TIMING_MODE_POS,
                         SACI_I2S_RIGHT_JUSTIFIED);

        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_POL_MASK, SACI_1004_TX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TSCK_POL_MASK, SACI_1004_TSCK_POL_POS, 0);
        /* config WS 0 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_OFFSET_MASK,
                         SACI_1004_TX_WS_OFFSET_POS, 0);
        /* RIGHT_JUSTIFIED config dala algin must Be lsb */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_DATA_ALIGN_MASK, SACI_1004_DATA_ALIGN_POS,
                         0);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_ALIGN_MASK, SACI_1008_DATA_ALIGN_POS,
                         0);
        break;

    case SND_DAIFMT_LEFT_JUSTIFIED:

        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TIMING_MODE_MASK, SACI_1004_TIMING_MODE_POS,
                         SACI_I2S_LEFT_JUSTIFIED);

        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_POL_MASK, SACI_1004_TX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TSCK_POL_MASK, SACI_1004_TSCK_POL_POS, 0);
        /* config WS 0 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_OFFSET_MASK,
                         SACI_1004_TX_WS_OFFSET_POS, 0);
        /* LEFT_JUSTIFIED config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_DATA_ALIGN_MASK, SACI_1004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_ALIGN_MASK, SACI_1008_DATA_ALIGN_POS,
                         0);

        break;

    /* DSP_A */
    /* Data on rising edge of bclk, frame high, 1clk before data */
    case SND_DAIFMT_DSP_A:
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TIMING_MODE_MASK, SACI_1004_TIMING_MODE_POS,
                         SACI_I2S_TDM);
        /* config ws  length */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_LEN_MASK, SACI_1004_TX_WS_LEN_POS,
                         SACI_WS_LEN_ONE_TSCK);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_POL_MASK, SACI_1004_TX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TSCK_POL_MASK, SACI_1004_TSCK_POL_POS, 0);
        /* config WS 1 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_OFFSET_MASK,
                         SACI_1004_TX_WS_OFFSET_POS, 1);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_DATA_ALIGN_MASK, SACI_1004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_ALIGN_MASK, SACI_1008_DATA_ALIGN_POS,
                         0);

        break;
    /*
      DSP_B
    * Frame high, one bit for frame sync,
    * frame sync asserts with the first bit of the frame.
    * Data on rising edge of bclk, frame high
    */
    case SND_DAIFMT_DSP_B:
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TIMING_MODE_MASK, SACI_1004_TIMING_MODE_POS,
                         SACI_I2S_TDM);
        /* config ws len chnnal length */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_LEN_MASK, SACI_1004_TX_WS_LEN_POS,
                         SACI_WS_LEN_ONE_TSCK);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_POL_MASK, SACI_1004_TX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TSCK_POL_MASK, SACI_1004_TSCK_POL_POS, 0);
        /* config WS 0 tsck udelay polar diff form DSP_A */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_WS_OFFSET_MASK,
                         SACI_1004_TX_WS_OFFSET_POS, 0);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_DATA_ALIGN_MASK, SACI_1004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_ALIGN_MASK, SACI_1008_DATA_ALIGN_POS,
                         0);

        break;
    default:
        ssdk_printf(SACI_REG_DEBUG_LEVEL, "%s unsupport fmt !!!\n", __func__);
        return false;
    }
    ssdk_printf(SACI_REG_DEBUG_LEVEL, "%s \n", __func__);

    return true;
}

static bool saci_i2s_lld_set_format_rx(snd_dev_t *dev, unsigned int fmt)
{

    uint32_t base_addr = dev->base;

    switch (fmt & SND_DAIFMT_FORMAT_MASK) {

    case SND_DAIFMT_STANDARD_PHILLIPS:

        /* config mode */
        saci_wr_reg_bits(
            base_addr + SACI_3004_SACI_RX_CTRL, SACI_3004_TINMING_MODE_MASK,
            SACI_3004_TINMING_MODE_POS, SACI_I2S_STANDARD_PHILLIPS);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_POL_MASK, SACI_3004_RX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RSCK_POL_MASK, SACI_3004_RSCK_POL_POS, 0);
        /* must WS 1 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_OFFSET_MASK,
                         SACI_3004_RX_WS_OFFSET_POS, 1);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_DATA_ALIGN_MASK, SACI_3004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_ALIGN_MASK, SACI_3008_DATA_ALIGN_POS,
                         0);
        break;

    case SND_DAIFMT_RIGHT_JUSTIFIED:

        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_TINMING_MODE_MASK,
                         SACI_3004_TINMING_MODE_POS, SACI_I2S_RIGHT_JUSTIFIED);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_POL_MASK, SACI_3004_RX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RSCK_POL_MASK, SACI_3004_RSCK_POL_POS, 0);
        /* must WS 0 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_OFFSET_MASK,
                         SACI_3004_RX_WS_OFFSET_POS, 0);
        /* RIGHT_JUSTIFIED config dala algin must Be lsb */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_DATA_ALIGN_MASK, SACI_3004_DATA_ALIGN_POS,
                         0);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_ALIGN_MASK, SACI_3008_DATA_ALIGN_POS,
                         0);
        break;

    case SND_DAIFMT_LEFT_JUSTIFIED:

        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_TINMING_MODE_MASK,
                         SACI_3004_TINMING_MODE_POS, SACI_I2S_LEFT_JUSTIFIED);

        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_POL_MASK, SACI_3004_RX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RSCK_POL_MASK, SACI_3004_RSCK_POL_POS, 0);
        /* must WS 0 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_OFFSET_MASK,
                         SACI_3004_RX_WS_OFFSET_POS, 0);
        /* LEFT_JUSTIFIED config dala alginconfig dala algin must be msb */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_DATA_ALIGN_MASK, SACI_3004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_ALIGN_MASK, SACI_3008_DATA_ALIGN_POS,
                         0);
        break;

    /* DSP_A */

    /* Data on rising edge of bclk, frame high, 1clk before data */
    case SND_DAIFMT_DSP_A:

        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_TINMING_MODE_MASK,
                         SACI_3004_TINMING_MODE_POS, SACI_I2S_TDM);
        /* config ws len chnnal length */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_LEN_MASK, SACI_3004_RX_WS_LEN_POS,
                         SACI_WS_LEN_ONE_TSCK);

        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_POL_MASK, SACI_3004_RX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RSCK_POL_MASK, SACI_3004_RSCK_POL_POS, 0);
        /* config WS 1 tsck udelay polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_OFFSET_MASK,
                         SACI_3004_RX_WS_OFFSET_POS, 1);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_DATA_ALIGN_MASK, SACI_3004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_ALIGN_MASK, SACI_3008_DATA_ALIGN_POS,
                         0);

        break;

    /*
      DSP_B
    * Frame high, one bit for frame sync,
    * frame sync asserts with the first bit of the frame.
    * Data on rising edge of bclk, frame high
    */
    case SND_DAIFMT_DSP_B:

        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_TINMING_MODE_MASK,
                         SACI_3004_TINMING_MODE_POS, SACI_I2S_TDM);
        /* config ws len chnnal length */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_LEN_MASK, SACI_3004_RX_WS_LEN_POS,
                         SACI_WS_LEN_ONE_TSCK);
        /* config ws polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_POL_MASK, SACI_3004_RX_WS_POL_POS, 0);
        /* config sck polar */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RSCK_POL_MASK, SACI_3004_RSCK_POL_POS, 0);
        /* config WS 0 tsck udelay polar diff form DSP_A */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_WS_OFFSET_MASK,
                         SACI_3004_RX_WS_OFFSET_POS, 0);
        /* config dala algin MSB */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_DATA_ALIGN_MASK, SACI_3004_DATA_ALIGN_POS,
                         1);

        /* config fifo algin lsb */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_ALIGN_MASK, SACI_3008_DATA_ALIGN_POS,
                         0);

        break;

    default:
        ssdk_printf(SACI_REG_DEBUG_LEVEL, "rx %s unsupport fmt !!!\n",
                    __func__);
        return false;
    }

    ssdk_printf(SACI_REG_DEBUG_LEVEL, "%s \n", __func__);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_startup(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    uint32_t reset_stat = 0;

    reset_stat =
        saci_rd_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS);

    if (!reset_stat) {
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS, 0);
        udelay(1);
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_SACI_RST_MASK, SACI_0000_SACI_RST_POS, 1);
    }

    if (saci_i2s->tx_chsel & saci_i2s->rx_chsel) {
        ssdk_printf(SSDK_ERR, "%s:%d : TX RX DATA LINE select err \n", __func__,
                    __LINE__);
        return false;
    }

    if (dev->clk) {
        saci_i2s->clk_rate = clk_get_rate(dev->clk);
    } else {
        ssdk_printf(SSDK_ERR, "err saci clk is null !\n");
        return false;
    }

    saci_i2s->pack_mode = true;
    saci_i2s->tx_dma = true;
    saci_i2s->tx_threshold = SACI_TX_DEPTH / 2;

    saci_i2s->rx_dma = true;
    saci_i2s->rx_threshold = SACI_RX_DEPTH / 2;

    return true;
}

void saci_i2s_lld_reset(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    if (stream == SND_STREAM_PLAYBACK) {

        /* reset tx  */
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_TX_RST_MASK,
                         SACI_0000_TX_RST_POS, 0);
        udelay(1);
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_TX_RST_MASK,
                         SACI_0000_TX_RST_POS, 1);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_TX_FIFO_FLUSH_MASK,
                         SACI_0000_TX_FIFO_FLUSH_POS, 1);
    } else {

        /* reset rx  */
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_RX_RST_MASK,
                         SACI_0000_RX_RST_POS, 0);
        udelay(1);
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_RX_RST_MASK,
                         SACI_0000_RX_RST_POS, 1);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_RX_FIFO_FLUSH_MASK,
                         SACI_0000_RX_FIFO_FLUSH_POS, 1);
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);
}

static bool saci_i2s_lld_config_clk_sync_mode(snd_dev_t *dev,
                                              clk_sync_loop_t sync_mode,
                                              bool i2s_slave,
                                              snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;

    bool active =
        (saci_i2s->playback_active || saci_i2s->capture_active) ? true : false;

    if (i2s_slave) {
        saci_i2s_lld_reset(dev, stream);
    }

    switch (sync_mode) {
    case CLK_TX_SYNC:
        if (!i2s_slave) {

            if (!active) {
                saci_i2s_lld_reset(dev, SND_STREAM_PLAYBACK);

                /* config tx i2s  master mode */
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_MODE_MASK, SACI_1004_TX_MODE_POS,
                                 IIS_MASTER);
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_SYNC_MODE_MASK,
                                 SACI_1004_TX_SYNC_MODE_POS, TRANSMITTER_ASYNC);

                if (stream == SND_STREAM_CAPTURE) {
                    saci_i2s_lld_reset(dev, SND_STREAM_CAPTURE);
                    /* config rx i2s  slave  mode  ,shre tx clk */
                    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                     SACI_3004_RX_MODE_MASK,
                                     SACI_3004_RX_MODE_POS, IIS_SLAVE);
                    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                     SACI_3004_RX_SYNC_MODE_MASK,
                                     SACI_3004_RX_SYNC_MODE_POS,
                                     RECEIVER_SYNCFROM_INTX);
                }

            } else {
                if (stream == SND_STREAM_CAPTURE) {
                    saci_i2s_lld_reset(dev, SND_STREAM_CAPTURE);

                    /* config rx i2s  slave  mode  option */
                    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                     SACI_3004_RX_MODE_MASK,
                                     SACI_3004_RX_MODE_POS, IIS_SLAVE);
                    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                     SACI_3004_RX_SYNC_MODE_MASK,
                                     SACI_3004_RX_SYNC_MODE_POS,
                                     RECEIVER_SYNCFROM_INTX);
                }
            }

        } else {
            if (stream == SND_STREAM_PLAYBACK) {
                /* config tx i2s slave mode ,tx */
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_MODE_MASK, SACI_1004_TX_MODE_POS,
                                 IIS_SLAVE);
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_SYNC_MODE_MASK,
                                 SACI_1004_TX_SYNC_MODE_POS, TRANSMITTER_ASYNC);
            } else {
                /* config rx i2s slave  mode  ,share tx clk */
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_MODE_MASK, SACI_3004_RX_MODE_POS,
                                 IIS_SLAVE);
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_SYNC_MODE_MASK,
                                 SACI_3004_RX_SYNC_MODE_POS,
                                 RECEIVER_SYNCFROM_INTX);
            }
        }

        break;
    case CLK_RX_SYNC:
        if (!i2s_slave) {
            /* config rx i2s  master mode */
            if (!active) {
                saci_i2s_lld_reset(dev, SND_STREAM_CAPTURE);

                /* config rx i2s  master mode,rx clk */
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_MODE_MASK, SACI_3004_RX_MODE_POS,
                                 IIS_MASTER);
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_SYNC_MODE_MASK,
                                 SACI_3004_RX_SYNC_MODE_POS, RECEIVER_ASYNC);

                if (stream == SND_STREAM_PLAYBACK) {
                    /* reset tx  */
                    saci_i2s_lld_reset(dev, SND_STREAM_PLAYBACK);

                    /* config tx i2s  slave mode ,share rx clk*/
                    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                     SACI_1004_TX_MODE_MASK,
                                     SACI_1004_TX_MODE_POS, IIS_SLAVE);
                    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                     SACI_1004_TX_SYNC_MODE_MASK,
                                     SACI_1004_TX_SYNC_MODE_POS,
                                     TRANSMITTER_SYNCFROM_INRX);
                }

            } else {

                if (stream == SND_STREAM_PLAYBACK) {
                    /* reset tx  */
                    saci_i2s_lld_reset(dev, SND_STREAM_PLAYBACK);

                    /* config tx i2s  slave mode ,share rx clk*/
                    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                     SACI_1004_TX_MODE_MASK,
                                     SACI_1004_TX_MODE_POS, IIS_SLAVE);
                    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                     SACI_1004_TX_SYNC_MODE_MASK,
                                     SACI_1004_TX_SYNC_MODE_POS,
                                     TRANSMITTER_SYNCFROM_INRX);
                }
            }
        } else {
            if (stream == SND_STREAM_PLAYBACK) {
                /* config tx i2s slave mode  share rx clk*/
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_MODE_MASK, SACI_1004_TX_MODE_POS,
                                 IIS_SLAVE);
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_SYNC_MODE_MASK,
                                 SACI_1004_TX_SYNC_MODE_POS,
                                 TRANSMITTER_SYNCFROM_INRX);
            } else {
                /* config rx i2s slave  mode  ,rx clk */
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_MODE_MASK, SACI_3004_RX_MODE_POS,
                                 IIS_SLAVE);
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_SYNC_MODE_MASK,
                                 SACI_3004_RX_SYNC_MODE_POS, RECEIVER_ASYNC);
            }
        }

        break;
    case CLK_TX_RX_ASYNC:
        if (!i2s_slave) {

            /* reset   */
            saci_i2s_lld_reset(dev, stream);

            if (stream == SND_STREAM_PLAYBACK) {
                /* config tx i2s  master mode */
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_MODE_MASK, SACI_1004_TX_MODE_POS,
                                 IIS_MASTER);
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_SYNC_MODE_MASK,
                                 SACI_1004_TX_SYNC_MODE_POS, TRANSMITTER_ASYNC);

            } else {
                /* config rx i2s  master  mode  option */
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_MODE_MASK, SACI_3004_RX_MODE_POS,
                                 IIS_MASTER);
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_SYNC_MODE_MASK,
                                 SACI_3004_RX_SYNC_MODE_POS, RECEIVER_ASYNC);
            }
        } else {

            if (stream == SND_STREAM_PLAYBACK) {
                /* config tx i2s  slave mode ,async*/
                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_MODE_MASK, SACI_1004_TX_MODE_POS,
                                 IIS_SLAVE);

                saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                                 SACI_1004_TX_SYNC_MODE_MASK,
                                 SACI_1004_TX_SYNC_MODE_POS, TRANSMITTER_ASYNC);

            } else {

                /* config rx i2s slave  mode  , async */
                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_MODE_MASK, SACI_3004_RX_MODE_POS,
                                 IIS_SLAVE);

                saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                                 SACI_3004_RX_SYNC_MODE_MASK,
                                 SACI_3004_RX_SYNC_MODE_POS, RECEIVER_ASYNC);
            }
        }

        break;
    default:
        ssdk_printf(SSDK_ERR,
                    "<%s>:tx  unsupport clk sync mode sync_mode(%d)\n",
                    __func__, sync_mode);
        return false;
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param params
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_set_params_tx(snd_dev_t *dev, snd_params_t *params)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[SND_STREAM_PLAYBACK];
    snd_params_t *snd_params = &snd_stream->params;
    uint32_t channels = params->channels;
    bool mono = (channels == 1) ? true : false;
    bool pack_mode = saci_i2s->pack_mode;
    snd_slot_width_t slot_width = params->sample_width;

    if (saci_i2s->tx_slot_width) {
        slot_width = saci_i2s->tx_slot_width;
    }
    /* config slot_width mode */
    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                     SACI_1004_TX_CH_WIDTH_MASK, SACI_1004_TX_CH_WIDTH_POS,
                     to_saci_slot_width(slot_width));

    /* config data_with mode */
    saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                     SACI_1004_TX_DATA_WIDTH_MASK, SACI_1004_TX_DATA_WIDTH_POS,
                     to_saci_data_width(params->sample_width));

    if (mono) {
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_MONO_MASK, SACI_1004_TX_MONO_POS, 1);
    } else {
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_MONO_MASK, SACI_1004_TX_MONO_POS, 0);
    }

    if (pack_mode) {
        /* config data pack mode */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_PACK_MODE_MASK,
                         SACI_1008_DATA_PACK_MODE_POS, 1);
    } else {
        /* config data loose mode */
        saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                         SACI_1008_DATA_PACK_MODE_MASK,
                         SACI_1008_DATA_PACK_MODE_POS, 0);
    }

    /* config dma threshold */
    saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                     SACI_1008_THRD_MASK, SACI_1008_THRD_POS,
                     saci_i2s->tx_threshold);
    /* config tx data line */
    saci_i2s_lld_config_tx_data_line(base_addr, saci_i2s->tx_chsel);

    memcpy(snd_params, params, sizeof(snd_params_t));

    ssdk_printf(SACI_REG_DEBUG_LEVEL, "%s \n", __func__);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param params
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_set_params_rx(snd_dev_t *dev, snd_params_t *params)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[SND_STREAM_CAPTURE];
    snd_params_t *snd_params = &snd_stream->params;
    uint32_t channels = params->channels;
    bool mono = (channels == 1) ? true : false;
    bool pack_mode = saci_i2s->pack_mode;
    snd_slot_width_t slot_width = params->sample_width;

    if (saci_i2s->rx_slot_width) {
        slot_width = saci_i2s->rx_slot_width;
    }
    /* config slot_width mode */
    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                     SACI_3004_RX_CH_WIDTH_MASK, SACI_3004_RX_CH_WIDTH_POS,
                     to_saci_slot_width(slot_width));

    /* config data_with mode */
    saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                     SACI_3004_RX_DATA_WIDTH_MASK, SACI_3004_RX_DATA_WIDTH_POS,
                     to_saci_data_width(params->sample_width));

    if (mono) {
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_MONO_MASK, SACI_3004_RX_MONO_POS, 1);
    }
    if (pack_mode) {
        /* config data pack mode */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_PACK_MODE_MASK,
                         SACI_3008_DATA_PACK_MODE_POS, 1);
    } else {
        /* config data loose mode */
        saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                         SACI_3008_DATA_PACK_MODE_MASK,
                         SACI_3008_DATA_PACK_MODE_POS, 0);
    }

    /* config dma threshold */
    saci_wr_reg_bits(base_addr + SACI_3008_SACI_RX_FIFO_CTRL,
                     SACI_3008_THRD_MASK, SACI_3008_THRD_POS,
                     saci_i2s->rx_threshold);
    /* config rx data line */
    saci_i2s_lld_config_rx_data_line(base_addr, saci_i2s->rx_chsel);

    memcpy(snd_params, params, sizeof(snd_params_t));

    ssdk_printf(SACI_REG_DEBUG_LEVEL, "%s \n", __func__);

    return true;
}

static bool saci_i2s_lld_enable_dma(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;

    if (stream == SND_STREAM_PLAYBACK) {
        if (saci_i2s->tx_dma) {
            saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                             SACI_1004_TX_DMA_EN_MASK, SACI_1004_TX_DMA_EN_POS,
                             1);
            udelay(5);
        }

    } else {

        if (saci_i2s->rx_dma) {
            saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                             SACI_3004_RX_DMA_EN_MASK, SACI_3004_RX_DMA_EN_POS,
                             1);

            saci_wr_reg_bits(base_addr + SACI_0010_SACI_LBK_CTRL,
                             SACI_0010_RX_TIMEOUT_DMA_EN_MASK,
                             SACI_0010_RX_TIMEOUT_DMA_EN_POS, 0);
        }
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);

    return true;
}
static bool saci_i2s_lld_disable_dma(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;

    if (stream == SND_STREAM_PLAYBACK) {
        if (saci_i2s->tx_dma) {
            saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                             SACI_1004_TX_DMA_EN_MASK, SACI_1004_TX_DMA_EN_POS,
                             0);
            udelay(5);
        }

    } else {

        if (saci_i2s->rx_dma) {
            saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                             SACI_3004_RX_DMA_EN_MASK, SACI_3004_RX_DMA_EN_POS,
                             0);

            saci_wr_reg_bits(base_addr + SACI_0010_SACI_LBK_CTRL,
                             SACI_0010_RX_TIMEOUT_DMA_EN_MASK,
                             SACI_0010_RX_TIMEOUT_DMA_EN_POS, 0);
        }
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);
    return true;
}

static bool saci_i2s_lld_enable_clk(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    uint32_t freq = saci_i2s_lld_calculate_freq(dev, stream);
    uint32_t src_clk_freq = saci_i2s->clk_rate;
    uint32_t div = 0;

    /*  div calc div */
    div = DIV_ROUND_UP(src_clk_freq, freq);
    ssdk_printf(SACI_DEBUG_LEVEL, "freq div=%d \r\n", div);
    if (div >= 1) {
        div = div - 1;
    }

    if (stream == SND_STREAM_PLAYBACK) {
        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_SEL_MASK, SACI_1000_CLK_SEL_POS,
                         CLK_DIVIDER_SOURCE_I2S_MCLK);

        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_DIV_MASK, SACI_1000_CLK_DIV_POS, div);
        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_DIV_CLK_EN_MASK, SACI_1000_DIV_CLK_EN_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_O_EN_MASK, SACI_1000_CLK_O_EN_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN, SACI_1014_EN_MASK,
                         SACI_1014_EN_POS, 1);

    } else {
        /* sel clk source  CLK_DIVIDER_SOURCE_I2S_MCLK */
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_SEL_MASK, SACI_3000_CLK_SEL_POS,
                         CLK_DIVIDER_SOURCE_I2S_MCLK);
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_DEV_MASK, SACI_3000_CLK_DEV_POS, div);
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_DIV_CLK_EN_MASK, SACI_3000_DIV_CLK_EN_POS,
                         1);

        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_O_EN_MASK, SACI_3000_CLK_O_EN_POS, 1);
        saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN, SACI_3014_EN_MASK,
                         SACI_3014_EN_POS, 1);
    }
    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);

    return true;
}

static bool saci_i2s_lld_disable_clk(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;

    if (stream == SND_STREAM_PLAYBACK) {

        saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN, SACI_1014_EN_MASK,
                         SACI_1014_EN_POS, 0);

        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_O_EN_MASK, SACI_1000_CLK_O_EN_POS, 0);
        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_DIV_CLK_EN_MASK, SACI_1000_DIV_CLK_EN_POS,
                         0);

        saci_wr_reg_bits(base_addr + SACI_1000_SACI_TX_CLK_CTRL,
                         SACI_1000_CLK_DIV_MASK, SACI_1000_CLK_DIV_POS, 0);

    } else {
        saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN, SACI_3014_EN_MASK,
                         SACI_3014_EN_POS, 0);

        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_DEV_MASK, SACI_3000_CLK_DEV_POS, 0);
        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_DIV_CLK_EN_MASK, SACI_3000_DIV_CLK_EN_POS,
                         0);

        saci_wr_reg_bits(base_addr + SACI_3000_SACI_RX_CLK_CTRL,
                         SACI_3000_CLK_O_EN_MASK, SACI_3000_CLK_O_EN_POS, 0);
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);

    return true;
}

static void saci_i2s_lld_disable_stream(snd_dev_t *dev,
                                        snd_stream_type_t stream)
{

    uint32_t base_addr = dev->base;

    if (stream == SND_STREAM_PLAYBACK) {

        saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN, SACI_1014_EN_MASK,
                         SACI_1014_EN_POS, 0);

    } else {

        saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN, SACI_3014_EN_MASK,
                         SACI_3014_EN_POS, 0);
    }

}

static bool saci_i2s_lld_slave_mode_reset(snd_dev_t *dev,
                                          snd_stream_type_t stream)
{

    uint32_t base_addr = dev->base;
#ifndef CONFIG_SACI_SLAVE
    int try = 10;
    uint32_t en_stata = 0;
#endif

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d)\n", __func__, stream);

    if (stream == SND_STREAM_PLAYBACK) {

        saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN, SACI_1014_EN_MASK,
                         SACI_1014_EN_POS, 0);

        udelay(5);
#ifdef CONFIG_SACI_SLAVE
        saci_i2s_lld_set_tx_master_clk(dev);

#else
        saci_wr_reg_bits(base_addr + SACI_1018_SACI_TX_OFF_WAIT_TIME,
                         SACI_1018_CNT_MASK, SACI_1018_CNT_POS, 0x1000);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_TX_ASYNC_RST_MASK,
                         SACI_0000_TX_ASYNC_RST_POS, 0x0);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_TX_FIFO_FLUSH_MASK,
                         SACI_0000_TX_FIFO_FLUSH_POS, 1);

        while (try > 0) {
            udelay(1);
            en_stata =
                saci_rd_reg_bits(base_addr + SACI_1014_SACI_TX_EN,
                                 SACI_1014_EN_STAT_MASK, SACI_1014_EN_STAT_POS);
            if ((!en_stata)) {
                break;
            }
            try--;
        }

        ssdk_printf(SACI_DEBUG_LEVEL, "<%s>:en_stata %d ? ?\n", __func__,
                    en_stata);
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_TX_ASYNC_RST_MASK,
                         SACI_0000_TX_ASYNC_RST_POS, 0x1);

#endif
        /* reset tx  */
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_TX_RST_MASK,
                         SACI_0000_TX_RST_POS, 0);
    } else {

        saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN, SACI_3014_EN_MASK,
                         SACI_3014_EN_POS, 0);

#ifndef CONFIG_SACI_SLAVE

        saci_wr_reg_bits(base_addr + SACI_3024_SACI_RX_OFF_WAIT_TIME,
                         SACI_3024_CNT_MASK, SACI_3024_CNT_POS, 0x2000);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_RX_ASYNC_RST_MASK,
                         SACI_0000_RX_ASYNC_RST_POS, 0x0);

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_RX_FIFO_FLUSH_MASK,
                         SACI_0000_RX_FIFO_FLUSH_POS, 1);

        while (try > 0) {

            udelay(1);
            en_stata =
                saci_rd_reg_bits(base_addr + SACI_3014_SACI_RX_EN,
                                 SACI_3014_EN_STAT_MASK, SACI_3014_EN_STAT_POS);
            if (!en_stata) {
                break;
            }
            try--;
        }

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_RX_ASYNC_RST_MASK,
                         SACI_0000_RX_ASYNC_RST_POS, 0x1);

        ssdk_printf(SACI_DEBUG_LEVEL, "func<%s>: en_stata (%d) try(%d)\n",
                    __func__, en_stata, try);
#endif
        /* reset rx  */
        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL, SACI_0000_RX_RST_MASK,
                         SACI_0000_RX_RST_POS, 0);
    }

    return true;
}


static bool saci_i2s_lld_enable_sync_mode(snd_dev_t *dev,
                                          clk_sync_loop_t sync_mode,
                                          snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_params_t *params = &snd_stream->params;
    uint32_t channels = params->channels;
    bool i2s_slave = (stream == SND_STREAM_PLAYBACK) ? saci_i2s->tx_slave
                                                     : saci_i2s->rx_slave;

    saci_i2s_lld_enable_dma(dev, stream);
    if (!i2s_slave) {
        switch (sync_mode) {
        case CLK_TX_SYNC:

            if (!saci_i2s->tx_clk_enable) {
                saci_i2s->tx_clk_enable = true;

                if (stream == SND_STREAM_CAPTURE) {
                    saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN,
                                     SACI_3014_EN_MASK, SACI_3014_EN_POS, 1);

                    /* config dma threshold */
                    saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                                     SACI_1008_THRD_MASK, SACI_1008_THRD_POS,
                                     channels);

                    for (int i = 0; i < channels; i++) {

                        writel(0x00, base_addr + SACI_2000_SACI_TX_FIFO_DATA);
                    }
                }

                saci_i2s_lld_enable_clk(dev, SND_STREAM_PLAYBACK);
                /* config dma threshold */
                saci_wr_reg_bits(base_addr + SACI_1008_SACI_TX_FIFO_CTRL,
                                 SACI_1008_THRD_MASK, SACI_1008_THRD_POS,
                                 saci_i2s->tx_threshold);

                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:enable tx clk first "
                            "sync_mode(%d),i2s_slave(%d)$$\n",
                            __func__, sync_mode, i2s_slave);
            } else {

                if (stream == SND_STREAM_CAPTURE) {

                    saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN,
                                     SACI_3014_EN_MASK, SACI_3014_EN_POS, 1);
                }
                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:tx clk is ready  "
                            "sync_mode(%d),i2s_slave(%d)$$\n",
                            __func__, sync_mode, i2s_slave);
            }

            break;
        case CLK_RX_SYNC:

            if (!saci_i2s->rx_clk_enable) {
                saci_i2s->rx_clk_enable = true;

                if (stream == SND_STREAM_PLAYBACK) {
                    saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN,
                                     SACI_1014_EN_MASK, SACI_1014_EN_POS, 1);
                }
                saci_i2s_lld_enable_clk(dev, SND_STREAM_CAPTURE);

                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:enable rx clk first "
                            "sync_mode(%d),i2s_slave(%d)\n",
                            __func__, sync_mode, i2s_slave);

            } else {

                if (stream == SND_STREAM_PLAYBACK) {

                    saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN,
                                     SACI_1014_EN_MASK, SACI_1014_EN_POS, 1);
                }
                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:rx clk is ready first "
                            "sync_mode(%d),i2s_slave(%d)$$\n",
                            __func__, sync_mode, i2s_slave);
            }

            break;
        case CLK_TX_RX_ASYNC:

            saci_i2s_lld_enable_clk(dev, stream);

            if (stream == SND_STREAM_PLAYBACK) {
                saci_i2s->tx_clk_enable = true;
                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:tx clk is ready first "
                            "sync_mode(%d),i2s_slave(%d)$$\n",
                            __func__, sync_mode, i2s_slave);

            } else {
                saci_i2s->rx_clk_enable = true;
                ssdk_printf(SACI_DEBUG_LEVEL,
                            "<%s>:rx clk is ready first "
                            "sync_mode(%d),i2s_slave(%d)$$\n",
                            __func__, sync_mode, i2s_slave);
            }

            break;
        default:
            ssdk_printf(SSDK_ERR,
                        "<%s>:tx  unsupport clk sync mode sync_mode(%d)\n",
                        __func__, sync_mode);
            return false;
        }
    }

    if (i2s_slave) {

        if (stream == SND_STREAM_PLAYBACK) {

            saci_wr_reg_bits(base_addr + SACI_1014_SACI_TX_EN,
                             SACI_1014_EN_MASK, SACI_1014_EN_POS, 1);
        } else {

            saci_wr_reg_bits(base_addr + SACI_3014_SACI_RX_EN,
                             SACI_3014_EN_MASK, SACI_3014_EN_POS, 1);
        }
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>:sync_mode(%d),i2s_slave(%d)\n",
                __func__, sync_mode, i2s_slave);

    return true;
}

static bool saci_i2s_lld_disable_sync_mode(snd_dev_t *dev,
                                           clk_sync_loop_t sync_mode,
                                           snd_stream_type_t stream)
{

    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    bool i2s_slave = (stream == SND_STREAM_PLAYBACK) ? saci_i2s->tx_slave
                                                     : saci_i2s->rx_slave;
    bool active =
        (saci_i2s->playback_active || saci_i2s->capture_active) ? true : false;

    ssdk_printf(SACI_DEBUG_LEVEL, " active (%d)\r\n", active);

    saci_i2s_lld_disable_dma(dev, stream);

    if (!i2s_slave) {
        switch (sync_mode) {
        case CLK_TX_SYNC:

            if (!active) {
                saci_i2s->tx_clk_enable = false;

                if (stream == SND_STREAM_CAPTURE) {
                    /* add async reset for rx */
                    saci_i2s_lld_disable_stream(dev, SND_STREAM_CAPTURE);
                }
                saci_i2s_lld_disable_clk(dev, SND_STREAM_PLAYBACK);
            } else {
                if (stream == SND_STREAM_CAPTURE) {
                    /* add async reset for rx */
                    saci_i2s_lld_disable_stream(dev, SND_STREAM_CAPTURE);
                }
            }

            break;

        case CLK_RX_SYNC:

            if (!active) {
                saci_i2s->rx_clk_enable = false;

                if (stream == SND_STREAM_PLAYBACK) {

                    /* add async reset for tx */
                    saci_i2s_lld_disable_stream(dev, SND_STREAM_PLAYBACK);
                }

                saci_i2s_lld_disable_clk(dev, SND_STREAM_CAPTURE);

            } else {

                if (stream == SND_STREAM_PLAYBACK) {

                    /*add async reset for tx */
                    saci_i2s_lld_disable_stream(dev, SND_STREAM_PLAYBACK);
                }
            }

            break;

        case CLK_TX_RX_ASYNC:

            if (stream == SND_STREAM_PLAYBACK) {
                saci_i2s->tx_clk_enable = false;
                saci_i2s_lld_disable_clk(dev, stream);

            } else {
                saci_i2s->rx_clk_enable = false;
                saci_i2s_lld_disable_clk(dev, stream);
            }

            break;

        default:
            ssdk_printf(SSDK_ERR,
                        "<%s>:tx  unsupport clk sync mode sync_mode(%d)\n",
                        __func__, sync_mode);
            return false;
        }
    }

    if (i2s_slave) {
        saci_i2s_lld_slave_mode_reset(dev, stream);
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>:sync_mode(%d),i2s_slave(%d)\n",
                __func__, sync_mode, i2s_slave);

    return true;
}

static bool saci_i2s_lld_shutdown_reset(snd_dev_t *dev,
                                        clk_sync_loop_t sync_mode,
                                        snd_stream_type_t stream)
{

    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    bool i2s_slave = (stream == SND_STREAM_PLAYBACK) ? saci_i2s->tx_slave
                                                     : saci_i2s->rx_slave;
    bool active =
        (saci_i2s->playback_active || saci_i2s->capture_active) ? true : false;

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>:sync_mode(%d),i2s_slave(%d)\n",
                __func__, sync_mode, i2s_slave);

    if (!i2s_slave) {
        switch (sync_mode) {
        case CLK_TX_SYNC:

            if (!active) {

                if (stream == SND_STREAM_CAPTURE) {
                    /* add async reset for rx */
                    saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_CAPTURE);
                }
                saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_PLAYBACK);
            } else {
                if (stream == SND_STREAM_CAPTURE) {
                    /* add async reset for rx */
                    saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_CAPTURE);
                }
            }

            break;

        case CLK_RX_SYNC:

            if (!active) {

                if (stream == SND_STREAM_PLAYBACK) {

                    /* add async reset for tx */
                    saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_PLAYBACK);
                }

                saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_CAPTURE);

            } else {

                if (stream == SND_STREAM_PLAYBACK) {

                    /*add async reset for tx */
                    saci_i2s_lld_slave_mode_reset(dev, SND_STREAM_PLAYBACK);
                }
            }

            break;

        case CLK_TX_RX_ASYNC:

            saci_i2s_lld_slave_mode_reset(dev, stream);

            break;

        default:
            ssdk_printf(SSDK_ERR,
                        "<%s>:tx  unsupport clk sync mode sync_mode(%d)\n",
                        __func__, sync_mode);
            return false;
        }
    }

    if (i2s_slave) {
        saci_i2s_lld_slave_mode_reset(dev, stream);
    }

    return true;
}

bool saci_i2s_lld_config_slave_master(snd_dev_t *dev, uint32_t fmt,
                                      snd_stream_type_t stream)
{

    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    bool i2s_slave = false;

    switch (fmt & SND_DAIFMT_MASTER_MASK) {

    case SND_DAIFMT_CODEC_SLAVE:

        i2s_slave = false;
        break;

    case SND_DAIFMT_CODEC_MASTER:

        i2s_slave = true;
        break;
    default:
        ssdk_printf(SACI_REG_DEBUG_LEVEL,
                    "tx func<%s>: i2s mastet or slave ? ?\n", __func__);
    }

    if (stream == SND_STREAM_PLAYBACK) {
        saci_i2s->tx_slave = i2s_slave;
    } else {
        saci_i2s->rx_slave = i2s_slave;
    }

    return i2s_slave;
}

unsigned int saci_i2s_lld_set_dma_burst(snd_dev_t *dev, snd_params_t *params,
                                        snd_stream_type_t stream)
{
    unsigned int maxburst;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    unsigned int slot_mask = (stream == SND_STREAM_PLAYBACK)
                                 ? params->tx_slot_mask
                                 : saci_i2s->rx_slot_mask;

    switch (params->fmt & SND_DAIFMT_FORMAT_MASK) {

    case SND_DAIFMT_STANDARD_PHILLIPS:
    case SND_DAIFMT_RIGHT_JUSTIFIED:
    case SND_DAIFMT_LEFT_JUSTIFIED:
        maxburst = SACI_DMA_BURST_SETERO;
        break;
    case SND_DAIFMT_DSP_A:
    case SND_DAIFMT_DSP_B:
        maxburst = bit_count(slot_mask);
        break;

    default:
        maxburst = SACI_DMA_BURST_SETERO;
    }

    ssdk_printf(SACI_REG_DEBUG_LEVEL, "func<%s> dma maxburst(%d):\n", __func__,
                maxburst);
    return maxburst;
}

/**
 * @brief
 *
 * @param dev
 * @param params
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_set_params(snd_dev_t *dev, snd_params_t *params,
                             snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    clk_sync_loop_t sync_mode = saci_i2s->sync_mode;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;
    bool i2s_slave;
    unsigned int maxburst;

    bool active =
        (saci_i2s->playback_active || saci_i2s->capture_active) ? true : false;

    i2s_slave = saci_i2s_lld_config_slave_master(dev, params->fmt, stream);

    saci_i2s_lld_config_clk_sync_mode(dev, sync_mode, i2s_slave, stream);

    if (i2s_slave) {
        if (stream == SND_STREAM_PLAYBACK) {
            saci_i2s_lld_set_format_tx(dev, params->fmt);
            saci_i2s_lld_set_params_tx(dev, params);
        } else {
            saci_i2s_lld_set_format_rx(dev, params->fmt);
            saci_i2s_lld_set_params_rx(dev, params);
        }
    } else {

        switch (sync_mode) {
        case CLK_TX_SYNC:

            if (!active) {
                if (stream == SND_STREAM_PLAYBACK) {
                    saci_i2s_lld_set_format_tx(dev, params->fmt);
                    saci_i2s_lld_set_params_tx(dev, params);

                } else {

                    saci_i2s_lld_set_format_tx(dev, params->fmt);
                    saci_i2s_lld_set_params_tx(dev, params);
                    saci_i2s_lld_set_format_rx(dev, params->fmt);
                    saci_i2s_lld_set_params_rx(dev, params);
                }
            } else {
                if (stream == SND_STREAM_PLAYBACK) {

                    ssdk_printf(
                        SACI_REG_DEBUG_LEVEL,
                        "func<%s>: tx no need config format params ? ?\n",
                        __func__);

                } else {

                    saci_i2s_lld_set_format_rx(dev, params->fmt);
                    saci_i2s_lld_set_params_rx(dev, params);
                }
            }

            break;
        case CLK_RX_SYNC: {

            if (!active) {
                if (stream == SND_STREAM_CAPTURE) {
                    saci_i2s_lld_set_format_rx(dev, params->fmt);
                    saci_i2s_lld_set_params_rx(dev, params);

                } else {
                    saci_i2s_lld_set_format_tx(dev, params->fmt);
                    saci_i2s_lld_set_params_tx(dev, params);
                    saci_i2s_lld_set_format_rx(dev, params->fmt);
                    saci_i2s_lld_set_params_rx(dev, params);
                }
            } else {

                if (stream == SND_STREAM_PLAYBACK) {
                    saci_i2s_lld_set_format_tx(dev, params->fmt);
                    saci_i2s_lld_set_params_tx(dev, params);
                } else {

                    ssdk_printf(
                        SACI_REG_DEBUG_LEVEL,
                        " func<%s>: rx no need config format params ? ?\n",
                        __func__);
                }
            }
        } break;
        case CLK_TX_RX_ASYNC:

            if (stream == SND_STREAM_PLAYBACK) {
                saci_i2s_lld_set_format_tx(dev, params->fmt);
                saci_i2s_lld_set_params_tx(dev, params);
            } else {
                saci_i2s_lld_set_format_rx(dev, params->fmt);
                saci_i2s_lld_set_params_rx(dev, params);
            }

            break;
        default:
            ssdk_printf(SSDK_ERR,
                        "<%s>:tx  unsupport clk sync mode sync_mode(%d) "
                        "i2s_slave(%d)\n",
                        __func__, sync_mode, i2s_slave);
            return false;
        }
    }

    maxburst = saci_i2s_lld_set_dma_burst(dev, params, stream);

    if (stream == SND_STREAM_PLAYBACK) {

        dma_data->dma_addr = base_addr + SACI_2000_SACI_TX_FIFO_DATA;
        dma_data->fifo_size = SACI_TX_DEPTH;
        dma_data->maxburst = maxburst;
        dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }

    if (stream == SND_STREAM_CAPTURE) {

        dma_data->dma_addr = base_addr + SACI_4000_SACI_RX_FIFO_DATA;
        dma_data->fifo_size = SACI_RX_DEPTH;
        dma_data->maxburst = maxburst;
        dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }

    ssdk_printf(SACI_DEBUG_LEVEL,
                "<%s>:clk sync mode sync_mode(%d) i2s_slave(%d)\n", __func__,
                sync_mode, i2s_slave);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param tx_mask
 * @param rx_mask
 * @param slots
 * @param slot_width
 * @return true
 * @return false
 */
bool saci_i2s_lld_set_tdm_slot(snd_dev_t *dev, unsigned int tx_mask,
                               unsigned int rx_mask, unsigned int slots,
                               snd_slot_width_t slot_width)
{
    uint32_t base_addr = dev->base;
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;

    if ((slots < 1) || (slots > 16)) {
        return false;
    }

    if (saci_i2s->tx_chsel & saci_i2s->rx_chsel) {
        ssdk_printf(SSDK_ERR, "%s:%i : TX RX DATA LINE select err \n", __func__,
                    __LINE__);
    }

    if (tx_mask) {
        saci_i2s->tx_slot_mask = tx_mask;
        saci_i2s->tx_slots = slots;
        saci_i2s->tx_slot_width = slot_width;

        /* enable tdm channels to tx */
        saci_wr_reg_bits(base_addr + SACI_1010_SACI_TX_TDM_CTRL,
                         SACI_1010_OUT_SEL_MASK, SACI_1010_OUT_SEL_POS,
                         TDM_SELCH(saci_i2s->tx_chsel));
        saci_wr_reg_bits(base_addr + SACI_1010_SACI_TX_TDM_CTRL,
                         SACI_1010_CH_NUM_MASK, SACI_1010_CH_NUM_POS,
                         slots - 1);
        saci_wr_reg_bits(base_addr + SACI_1010_SACI_TX_TDM_CTRL,
                         SACI_1010_CH_EN_MASK, SACI_1010_CH_EN_POS, tx_mask);
        /* config slot_width mode */
        saci_wr_reg_bits(base_addr + SACI_1004_SACI_TX_CTRL,
                         SACI_1004_TX_CH_WIDTH_MASK, SACI_1004_TX_CH_WIDTH_POS,
                         to_saci_slot_width(slot_width));

        ssdk_printf(SACI_DEBUG_LEVEL,
                    "func<%s>: TX TDM setitng tx line is: "
                    "%d,slots=%d,tx_mask:0x%x\n",
                    __func__, TDM_SELCH(saci_i2s->tx_chsel), slots, tx_mask);
    } else {

        ssdk_printf(SSDK_ERR,
                    "func<%s>: TX TDM setitng tx line is: "
                    "%d,slots=%d,tx_mask:0x%x\n",
                    __func__, TDM_SELCH(saci_i2s->tx_chsel), slots, tx_mask);
    }

    if (rx_mask) {
        saci_i2s->rx_slot_mask = rx_mask;
        saci_i2s->rx_slots = slots;
        saci_i2s->rx_slot_width = slot_width;

        /* enable tdm channels to rx */
        saci_wr_reg_bits(base_addr + SACI_3010_SACI_RX_TDM_CTRL,
                         SACI_3010_IN_SEL_MASK, SACI_3010_IN_SEL_POS,
                         TDM_SELCH(saci_i2s->rx_chsel));
        saci_wr_reg_bits(base_addr + SACI_3010_SACI_RX_TDM_CTRL,
                         SACI_3010_CH_NUM_MASK, SACI_3010_CH_NUM_POS,
                         slots - 1);
        saci_wr_reg_bits(base_addr + SACI_3010_SACI_RX_TDM_CTRL,
                         SACI_3010_CH_EN_MASK, SACI_3010_CH_EN_POS, rx_mask);
        /* config slot_width mode */
        saci_wr_reg_bits(base_addr + SACI_3004_SACI_RX_CTRL,
                         SACI_3004_RX_CH_WIDTH_MASK, SACI_3004_RX_CH_WIDTH_POS,
                         to_saci_slot_width(slot_width));

        ssdk_printf(SACI_DEBUG_LEVEL,
                    "func<%s>: RX TDM setitng rx line is : "
                    "%d,slots=%d,rx_mask:0x%x\n",
                    __func__, TDM_SELCH(saci_i2s->rx_chsel), slots, rx_mask);
    } else {

        ssdk_printf(SACI_DEBUG_LEVEL,
                    "func<%s>: RX TDM setitng rx line is : "
                    "%d,slots=%d,rx_mask:0x%x\n",
                    __func__, TDM_SELCH(saci_i2s->rx_chsel), slots, rx_mask);
    }

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param cmd
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_trigger(snd_dev_t *dev, snd_trigger_op_t cmd,
                          snd_stream_type_t stream)
{

    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    clk_sync_loop_t sync_mode = saci_i2s->sync_mode;

    switch (cmd) {
    case SND_TRIGGER_START:
        if (stream == SND_STREAM_PLAYBACK) {

            saci_i2s->playback_active = true;

        } else {
            saci_i2s->capture_active = true;
        }

        saci_i2s_lld_enable_sync_mode(dev, sync_mode, stream);
        saci_iis_lld_interrpt_init(dev, stream);

        break;

    case SND_TRIGGER_STOP:
        if (stream == SND_STREAM_PLAYBACK) {

            saci_i2s->playback_active = false;

        } else {
            saci_i2s->capture_active = false;
        }

        saci_i2s_lld_disable_sync_mode(dev, sync_mode, stream);
        saci_iis_lld_interrpt_reset(dev, stream);

        break;

    default:
        ssdk_printf(SACI_REG_DEBUG_LEVEL, "func<%s>: cmd not support ? ?\n",
                    __func__);
    }

    ssdk_printf(SACI_DEBUG_LEVEL, "<%s>: stream(%d) cmd(%d)\n", __func__,
                stream, cmd);

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool saci_i2s_lld_shutdown(snd_dev_t *dev, snd_stream_type_t stream)
{
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    clk_sync_loop_t sync_mode = saci_i2s->sync_mode;

    saci_i2s_lld_shutdown_reset(dev, sync_mode, stream);

    return true;
}

static int saci_i2s_irq_handler(uint32_t irq, void *arg)
{
    snd_dev_t *dev = (snd_dev_t *)arg;
    uint32_t base_addr = dev->base;
    uint32_t irq_stat = saci_i2s_lld_get_status_irq(base_addr);
    uint32_t irq_mask = saci_i2s_lld_get_mask_irq(base_addr);
    uint32_t mask_value, irq_value;

    mask_value = !((irq_mask & SACI_0008_TX_FIFO_PRE_EMPTY_MASK) >>
                   SACI_0008_TX_FIFO_PRE_EMPTY_POS);
    irq_value = (irq_stat & SACI_0008_TX_FIFO_PRE_EMPTY_MASK) >>
                SACI_0008_TX_FIFO_PRE_EMPTY_POS;

    if (irq_value & mask_value) {
        saci_i2s_lld_clear_tx_fifo_pre_empty_irq(base_addr);
    }

    mask_value =
        !((irq_mask & SACI_0004_TX_FIFO_UDR_MASK) >> SACI_0004_TX_FIFO_UDR_POS);
    irq_value =
        (irq_stat & SACI_0008_TX_FIFO_UDR_MASK) >> SACI_0008_TX_FIFO_UDR_POS;
    if (irq_value & mask_value) {
        saci_i2s_lld_clear_tx_fifo_udr_irq(base_addr);
    }

    mask_value = !((irq_mask & SACI_0004_RX_FIFO_PRE_FULL_MASK) >>
                   SACI_0004_RX_FIFO_PRE_FULL_POS);
    irq_value = (irq_stat & SACI_0008_RX_FIFO_PRE_FULL_MASK) >>
                SACI_0008_RX_FIFO_PRE_FULL_POS;
    if (irq_value & mask_value) {
        saci_i2s_lld_clear_rx_fifo_prefull_irq(base_addr);
    }

    mask_value =
        !((irq_mask & SACI_0008_RX_FIFO_OVR_MASK) >> SACI_0008_RX_FIFO_OVR_POS);
    irq_value =
        (irq_stat & SACI_0008_RX_FIFO_OVR_MASK) >> SACI_0008_RX_FIFO_OVR_POS;
    if (irq_value & mask_value) {

        saci_i2s_lld_clear_rx_fifo_ovr_irq(base_addr);
    }

    mask_value =
        !((irq_mask & SACI_0004_RX_TIMEOUT_MASK) >> SACI_0004_RX_TIMEOUT_POS);
    irq_value =
        (irq_stat & SACI_0008_RX_TIMEOUT_MASK) >> SACI_0008_RX_TIMEOUT_POS;
    if (irq_value & mask_value) {
        saci_i2s_lld_clear_rx_fifo_timeout_irq(base_addr);
    }

    return 0;
}

static void saci_i2s_config_mclk_dir(snd_dev_t *dev)
{
    saci_i2s_t *saci_i2s = (saci_i2s_t *)dev->priv;
    uint32_t base_addr;
    uint32_t mclk_dir;

    if (SACI_MCLK_NULL != saci_i2s->mclk_sel) {

        base_addr = (saci_i2s->mclk_sel == SACI_MCLK_0) ? DEVICE_BASE(SACI1)
                                                        : DEVICE_BASE(SACI2);
        mclk_dir = (saci_i2s->mclk_dir == SACI_MCLK_OUT) ? 1 : 0;

        saci_wr_reg_bits(base_addr + SACI_0000_SACI_CTRL,
                         SACI_0000_MCLK_DIR_MASK, SACI_0000_MCLK_DIR_POS,
                         mclk_dir);

        ssdk_printf(SACI_DEBUG_LEVEL, "saci: mclk_sel(%d), mclk_dir(%d)\n",
                    saci_i2s->mclk_sel, saci_i2s->mclk_dir);
    }
}

static snd_ops_t saci_i2s_ops = {
    .startup = saci_i2s_lld_startup,
    .set_params = saci_i2s_lld_set_params,
    .set_tdm_slot = saci_i2s_lld_set_tdm_slot,
    .trigger = saci_i2s_lld_trigger,
    .shutdown = saci_i2s_lld_shutdown,
};

static void saci_i2s_init(snd_dev_t *dev)
{
    ASSERT(NULL != dev);

    ssdk_printf(SSDK_INFO, "saci i2s inited, dev: %p\n", dev);
    snd_dev_init(dev);
    saci_i2s_config_mclk_dir(dev);

    if (dev->irq > 0) {
        irq_attach(dev->irq, saci_i2s_irq_handler, dev);
        irq_enable(dev->irq);
    }
}

snd_dev_t *saci_i2s_probe(saci_i2s_t *saci_i2s, saci_i2s_cfg_t *cfg)
{
    snd_dev_t *dev = &saci_i2s->dev;

    ASSERT(NULL != dev);
    ASSERT(NULL != cfg);
    ASSERT(NULL != saci_i2s);

    saci_i2s->tx_chsel = cfg->tx_chsel;
    saci_i2s->rx_chsel = cfg->rx_chsel;
    saci_i2s->sync_mode = cfg->sync_mode;
    saci_i2s->mclk_sel = cfg->mclk_sel;
    saci_i2s->mclk_dir = cfg->mclk_dir;

    dev->priv = saci_i2s;
    dev->ops = &saci_i2s_ops;
    dev->dma_dev = cfg->dma_dev;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->clk = cfg->clk;
    saci_i2s_init(dev);

    return dev;
}
