/**
 * @file cdns_i2s_sc.c
 * @author yi shao (yi.shao@semidrive.com)
 * @brief
 * @version 0.1
 * @date 2022-04-15
 *
 * @copyright Copyright (c) 2022 Semidrive Semiconductor.
 *
 */
#include <arch/cache.h>
#include <bits.h>
#include <board.h>
#include <debug.h>
#include <driver/irq.h>
#include "kunlun/snd/i2s/cdns_i2s_sc.h"
#include <reg.h>
#include <stdio.h>
#include <string.h>

#define SSDK_DBG_LEVEL SSDK_INFO
/**
 * @brief input slot_width such as 32.
 * 		  return enum CDN_CHN_WIDTH_32BITS
 *
 */
static int check_and_get_slot_width_enum(int slot_width)
{
    int i;
    for (i = 0; i < CDN_CHN_WIDTH_NUMB; i++) {
        if (slot_width == ChnWidthTable[i]) {
            return i;
        }
    }
    ssdk_printf(
        SSDK_ERR,
        "can't support this slot_width %d use 32 CDN_CHN_WIDTH_32BITS\n",
        slot_width);
    return AFE_I2S_CHN_WIDTH;
}

static void cdns_i2s_sc_config(snd_dev_t *dev)
{
    uint32_t val;
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    /*Disable i2s*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (0 << I2S_CTRL_I2S_EN_FIELD_OFFSET));

    /*SFR reset*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_SFR_RST,
                    (0 << I2S_CTRL_SFR_RST_FIELD_OFFSET));

    /*FIFO reset*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_FIFO_RST,
                    (0 << I2S_CTRL_FIFO_RST_FIELD_OFFSET));

    if (true == i2s_sc->is_full_duplex) {

        /*Reset full duplex FIFO reset*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_FIFO_RST,
                        (0 << I2S_CTRL_FDX_FIFO_RST_FIELD_OFFSET));
        /*Enable full-duplex*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_FULL_DUPLEX,
                        (1 << I2S_CTRL_FDX_FULL_DUPLEX_FIELD_OFFSET));
        /*Disable tx sdo*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_I2S_FTX_EN,
                        (0 << I2S_CTRL_FDX_I2S_FTX_EN_FIELD_OFFSET));
        /*Disable rx sdi*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_I2S_FRX_EN,
                        (0 << I2S_CTRL_FDX_I2S_FRX_EN_FIELD_OFFSET));

        /*Unmask full-duplex mode interrupt request.*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_RI2S_MASK,
                        (1 << I2S_CTRL_FDX_RI2S_MASK_FIELD_OFFSET));
        /*Mask full-duplex receive FIFO becomes empty. */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_RFIFO_EMPTY_MASK,
                        (0 << I2S_CTRL_FDX_RFIFO_EMPTY_MASK_FIELD_OFFSET));
        /*Mask full-duplex receive FIFO becomes almost empty.*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_RFIFO_AEMPTY_MASK,
                        (0 << I2S_CTRL_FDX_RFIFO_AEMPTY_MASK_FIELD_OFFSET));
        /*Mask full-duplex receive FIFO becomes full. */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_RFIFO_FULL_MASK,
                        (0 << I2S_CTRL_FDX_RFIFO_FULL_MASK_FIELD_OFFSET));
        /*Mask full-duplex receive FIFO becomes almost full. */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_RFIFO_AFULL_MASK,
                        (0 << I2S_CTRL_FDX_RFIFO_AFULL_MASK_FIELD_OFFSET));
    }
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (0 << I2S_CTRL_I2S_EN_FIELD_OFFSET));

    /*config direction of transmission:Tx*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_DIR_CFG,
                    (1 << I2S_CTRL_DIR_CFG_FIELD_OFFSET));

    /*config total data width*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_CHN_WIDTH,
                    (i2s_sc->slot_width << I2S_CTRL_CHN_WIDTH_FIELD_OFFSET));

    /*config almost empty fifo level:tx fifo level */
    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AEMPTY, 0x30);
    /*config almost full fifo level:defualt value*/
    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AFULL, 0x60);

    /*config resolution,valid number of data bit*/
    if (true == i2s_sc->is_full_duplex) {
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES_FDR, 0xf);
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES, 0xf);
        /*config almost empty fifo level:tx fifo level */
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AEMPTY_FDR, 0x30);
        /*config almost full fifo level:defualt value*/
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AFULL_FDR, 0x60);
    } else {
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES, 0xf);
    }

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_I2S_CTRL 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_I2S_CTRL, val);

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AEMPTY);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_FIFO_AEMPTY 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_FIFO_AEMPTY, val);

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_AFULL);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_FIFO_AFULL 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_FIFO_AFULL, val);

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_I2S_SRES 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_I2S_SRES, val);

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_TDM_FD_DIR);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_TDM_FD_DIR 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_TDM_FD_DIR, val);
}

static void cdns_i2s_sc_start_capture(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    uint32_t cnt, val;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;

    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT, 0);
    /*Enable rx sdi*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_I2S_FRX_EN,
                    (1 << I2S_CTRL_FDX_I2S_FRX_EN_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (1 << I2S_CTRL_I2S_EN_FIELD_OFFSET));

    /*Set interrupt request i2s fdx */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_RI2S_MASK,
                    (1 << I2S_CTRL_FDX_RI2S_MASK_FIELD_OFFSET));

    /*Set all interrupt request generation */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_MASK,
                    (1 << I2S_CTRL_I2S_MASK_FIELD_OFFSET));

    /*Clear fifo almost full mask */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                    BIT_CTRL_FIFO_AFULL_MASK,
                    (0 << I2S_CTRL_FIFO_AFULL_MASK_FIELD_OFFSET));

    /*Clear fifo afull mask */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_RFIFO_AFULL_MASK,
                    (0 << I2S_CTRL_FDX_RFIFO_AFULL_MASK_FIELD_OFFSET));

    /*Clear I2S Overrun status*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT,
                    BIT_STAT_RDATA_OVERR,
                    (0 << I2S_STAT_RDATA_OVERR_FIELD_OFFSET));

    /*Clear I2S UNDERR status*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT,
                    BIT_STAT_TDATA_UNDERR,
                    (0 << I2S_STAT_TDATA_UNDERR_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                    BIT_CTRL_INTREQ_MASK,
                    (1 << I2S_CTRL_INTREQ_MASK_FIELD_OFFSET));

    if ((i2s_sc->tdm_initialized == true) &&
        (i2s_sc->is_tdm_use_rx_last_slot == true)) {

        val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL);
        cnt = 0;
        if ((i2s_sc->playing) && (i2s_sc->is_slave == true) &&
            (BIT_(i2s_sc->slots - 1) & i2s_sc->rx_slot_mask)) {
            while (cnt < 500) {
                cnt++;
                val =
                    snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_LEVEL_FDR);
                if (val > 0) {
                    /* Only read one time */
                    snd_read_reg(base_addr, I2S_SC_FIFO_OFFSET);
                    break;
                }
            }
        }
        if (cnt > 300) {
            ssdk_printf(
                SSDK_DBG_LEVEL,
                "DUMP %d REG_CDN_I2SSC_REGS_FIFO_LEVEL_FDR(0x%x) times(0x%x)\n",
                __LINE__, val, cnt);
        }
    }
    return;
}

static void cdns_i2s_sc_stop_capture(snd_dev_t *dev)
{
    /*Enable rx sdi*/
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;

    if (true == i2s_sc->is_full_duplex){
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_I2S_FRX_EN,
                    (0 << I2S_CTRL_FDX_I2S_FRX_EN_FIELD_OFFSET));
    } else {
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (0 << I2S_CTRL_I2S_EN_FIELD_OFFSET));
    }
    udelay(30);

#ifndef CONFIG_BOARD_X9SP
    uint32_t val, cnt;
    /*Clear FIFO for X9HP etc*/
    cnt = 0;
    while (cnt < I2S_SC_RX_DEPTH * 2) {
        val = snd_read_reg(base_addr, I2S_SC_FIFO_OFFSET);
        val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_LEVEL_FDR);
        if (val == 0) {
            break;
        }
        cnt++;
    }
#endif

    if (true == i2s_sc->is_full_duplex) {
        /*Reset full duplex FIFO reset*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                        BIT_CTRL_FDX_FIFO_RST,
                        (0 << I2S_CTRL_FDX_FIFO_RST_FIELD_OFFSET));
    } else {
        /*Reset FIFO reset*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_FIFO_RST,
                        (0 << I2S_CTRL_FIFO_RST_FIELD_OFFSET));
    }
    return;
}

static void cdns_i2s_sc_start_playback(snd_dev_t *dev)
{
    uint32_t val;
    uint32_t base_addr = dev->base;
    // cdns_i2s_sc_drv_t * i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    /*Set interrupt request i2s fdx */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_RI2S_MASK,
                    (1 << I2S_CTRL_FDX_RI2S_MASK_FIELD_OFFSET));

    /*Set all interrupt request generation */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_MASK,
                    (1 << I2S_CTRL_I2S_MASK_FIELD_OFFSET));

    /*Set fifo almost empty mask */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                    BIT_CTRL_FIFO_AEMPTY_MASK,
                    (0 << I2S_CTRL_FIFO_AEMPTY_MASK_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT, BIT_STAT_FIFO_EMPTY,
                    (0 << I2S_STAT_TDATA_UNDERR_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT,
                    BIT_STAT_FIFO_AEMPTY,
                    (0 << I2S_STAT_TDATA_UNDERR_FIELD_OFFSET));
    /*Clear I2S UNDERR status*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT,
                    BIT_STAT_TDATA_UNDERR,
                    (0 << I2S_STAT_TDATA_UNDERR_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                    BIT_CTRL_INTREQ_MASK,
                    (1 << I2S_CTRL_INTREQ_MASK_FIELD_OFFSET));

    /*Enable tx sdo*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_I2S_FTX_EN,
                    (1 << I2S_CTRL_FDX_I2S_FTX_EN_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (1 << I2S_CTRL_I2S_EN_FIELD_OFFSET));

    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL);
    ssdk_printf(SSDK_DBG_LEVEL,
                "DUMP REG_CDN_I2SSC_REGS_I2S_CTRL 0x%x:(0x%x)\n",
                REG_CDN_I2SSC_REGS_I2S_CTRL, val);
}

static void cdns_i2s_sc_stop_playback(snd_dev_t *dev)
{
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;

#ifndef CONFIG_BOARD_X9SP
    uint32_t ret, val, cnt, delay_cnt;

    cnt = 0;
    // cdns_i2s_sc_drv_t * i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    /* (1000000/10)  * I2S_SC_TX_DEPTH / (afe->srate * afe->tx_channels) + 1*/
    delay_cnt = 100000 * I2S_SC_TX_DEPTH;
    while (cnt < delay_cnt) {
        val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT);
        if (ret >= 0) {
            if (BIT_STAT_FIFO_EMPTY & val) {
                break;
            }
        }
        udelay(10);
        cnt++;
    }
    val = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_FIFO_LEVEL);
    if (val > 0) {
        ssdk_printf(SSDK_ERR, "delay %d , level %d \n", delay_cnt, val);
    }
#endif
    /*Disable tx sdi*/
    if (true == i2s_sc->is_full_duplex) {
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL_FDX,
                    BIT_CTRL_FDX_I2S_FTX_EN,
                    (0 << I2S_CTRL_FDX_I2S_FTX_EN_FIELD_OFFSET));
    } else {
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_I2S_EN,
                        (0 << I2S_CTRL_I2S_EN_FIELD_OFFSET));
    }

    udelay(30);
    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT, 0);

    udelay(100);
    /*FIFO Reset*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_FIFO_RST,
                    (0 << I2S_CTRL_FIFO_RST_FIELD_OFFSET));
}

static void cdns_i2s_sc_stop(snd_dev_t *dev)
{
    // uint32_t val;
    uint32_t base_addr = dev->base;
    /*Clear I2S status*/
    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT, 0);
    /*Disable i2s*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_I2S_EN,
                    (0 << I2S_CTRL_I2S_EN_FIELD_OFFSET));
    ssdk_printf(SSDK_DBG_LEVEL, "cdns_i2s_sc_stop!\n");
}

bool cdns_i2s_sc_set_params(snd_dev_t *dev, snd_params_t *params,
                            snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    clk_rate_t clk_rate = clk_get_rate(dev->clk);
    uint32_t srate = params->sample_rate;
    uint32_t channels = params->channels;
    uint32_t sample_size = params->sample_width;
    bool sck_polar;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;

    i2s_sc->is_slave =
        ((params->fmt & SND_DAIFMT_MASTER_MASK) == SND_DAIFMT_CODEC_SLAVE)
            ? false
            : true;

    dma_data->dma_addr = base_addr + I2S_SC_FIFO_OFFSET;
    dma_data->maxburst = channels;

    ssdk_printf(SSDK_DBG_LEVEL,
                "cdns_i2s_sc_set_params: is_slave(%d),clk_rate(%d),srate(%d), "
                "chan(%d),sample_size(%d)\n",
                i2s_sc->is_slave, clk_rate, srate, channels, sample_size);
    /*Filter TDM setting*/
    if (i2s_sc->tdm_initialized == false) {
        if (channels == 2) {
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_CTRL,
                            BIT_TDM_CTRL_TDM_EN,
                            (0 << TDM_CTRL_TDM_EN_FIELD_OFFSET));

            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_AUDIO_MODE,
                            (0 << I2S_CTRL_AUDIO_MODE_FIELD_OFFSET));
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_MONO_MODE,
                            (0 << I2S_CTRL_MONO_MODE_FIELD_OFFSET));
            /*set to pack mode*/
            if (i2s_sc->is_pack_en == true) {
                snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                                BIT_CTRL_LR_PACK,
                                (1 << I2S_CTRL_LR_PACK_FIELD_OFFSET));
            } else {
                snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                                BIT_CTRL_LR_PACK,
                                (0 << I2S_CTRL_LR_PACK_FIELD_OFFSET));
            }
        } else if (channels == 1) {

            /* Disable tdm mode */
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_CTRL,
                            BIT_TDM_CTRL_TDM_EN,
                            (0 << TDM_CTRL_TDM_EN_FIELD_OFFSET));

            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_AUDIO_MODE,
                            (1 << I2S_CTRL_AUDIO_MODE_FIELD_OFFSET));
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_MONO_MODE,
                            (0 << I2S_CTRL_MONO_MODE_FIELD_OFFSET));
            /*clear pack mode*/
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_LR_PACK,
                            (0 << I2S_CTRL_LR_PACK_FIELD_OFFSET));
        } else {
            /* Already set to tdm by snd_afe_set_dai_tdm_slot */
            /*clear pack mode*/
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_LR_PACK,
                            (0 << I2S_CTRL_LR_PACK_FIELD_OFFSET));
        }
    }
    if (stream == SND_STREAM_PLAYBACK) {
        if (false == i2s_sc->is_full_duplex) {
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_DIR_CFG,
                            (1 << I2S_CTRL_DIR_CFG_FIELD_OFFSET));
        }
        /*set tx substream*/
        // i2s_sc->tx_channels = channels;
    } else if (stream == SND_STREAM_CAPTURE) {

        if (false == i2s_sc->is_full_duplex) {
            snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                            BIT_CTRL_DIR_CFG,
                            (0 << I2S_CTRL_DIR_CFG_FIELD_OFFSET));
        }
        /*set rx substream */
        // i2s_sc->rx_channels = channels;
    } else {
        return false;
    }
    switch (sample_size) {
    case SND_SAMPLE_WIDTH_8BITS:
        ssdk_printf(
            SSDK_ERR,
            "%s(%d) Don't support this function for S8 is low dynamic range.\n",
            __func__, __LINE__);
        return false;
    case SND_SAMPLE_WIDTH_16BITS:
        /* Don't support in LR Pack mode */
        if ((channels == 2) && (i2s_sc->is_pack_en == true) &&
            (i2s_sc->tdm_initialized == false)) {
            dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        } else {
            dma_data->addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        }
        break;
    case SND_SAMPLE_WIDTH_24BITS:
    case SND_SAMPLE_WIDTH_32BITS:
        dma_data->addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        break;
    default:
        return false;
    }

    /* Here calculate and fill sample rate.  */
    if (true == i2s_sc->is_full_duplex) {

        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES_FDR,
                      sample_size - 1);

        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES, sample_size - 1);

    } else {
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRES, sample_size - 1);
    }
    if (i2s_sc->tdm_initialized == false) {
        snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_SRATE,
                      I2S_SC_SAMPLE_RATE_CALC(
                          clk_rate, srate, ((channels < 2) ? 2 : channels),
                          ChnWidthTable[i2s_sc->slot_width]));
    } else {
        snd_write_reg(
            base_addr, REG_CDN_I2SSC_REGS_I2S_SRATE,
            I2S_SC_SAMPLE_RATE_CALC(clk_rate, srate, i2s_sc->slots,
                                    ChnWidthTable[i2s_sc->slot_width]));
    }

    if (i2s_sc->is_slave == false) {
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_MS_CFG,
                        (1 << I2S_CTRL_MS_CFG_FIELD_OFFSET));
        sck_polar = (i2s_sc->is_master_falling_sck_polar == true) ? 1 : 0;
    } else {
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_MS_CFG,
                        (0 << I2S_CTRL_MS_CFG_FIELD_OFFSET));
        sck_polar = (i2s_sc->is_slave_falling_sck_polar == true) ? 1 : 0;
    }
    // set dai format
    switch (params->fmt & SND_DAIFMT_FORMAT_MASK) {
    case SND_DAIFMT_STANDARD_PHILLIPS:
        /*config sck polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_SCK_POLAR,
                        (sck_polar << I2S_CTRL_SCK_POLAR_FIELD_OFFSET));
        /*config ws polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_POLAR,
                        (0 << I2S_CTRL_WS_POLAR_FIELD_OFFSET));
        /*config word select mode*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_MODE, (1 << I2S_CTRL_WS_MODE_FIELD_OFFSET));
        /*config ws singal delay*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_WS_DEL,
                        (1 << I2S_CTRL_DATA_WS_DEL_FIELD_OFFSET));
        /*config data align:MSB*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ALIGN,
                        (0 << I2S_CTRL_DATA_ALIGN_FIELD_OFFSET));
        /*config data order:fisrt send MSB */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ORDER,
                        (0 << I2S_CTRL_DATA_ORDER_FIELD_OFFSET));
        break;
    case SND_DAIFMT_LEFT_JUSTIFIED:
        /*config sck polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_SCK_POLAR,
                        (sck_polar << I2S_CTRL_SCK_POLAR_FIELD_OFFSET));
        /*config ws polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_POLAR,
                        (1 << I2S_CTRL_WS_POLAR_FIELD_OFFSET));
        /*config word select mode*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_MODE, (1 << I2S_CTRL_WS_MODE_FIELD_OFFSET));
        /*config ws singal delay*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_WS_DEL,
                        (0 << I2S_CTRL_DATA_WS_DEL_FIELD_OFFSET));
        /*config data align:MSB*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ALIGN,
                        (0 << I2S_CTRL_DATA_ALIGN_FIELD_OFFSET));
        /*config data order:first send MSB */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ORDER,
                        (0 << I2S_CTRL_DATA_ORDER_FIELD_OFFSET));
        break;
    case SND_DAIFMT_RIGHT_JUSTIFIED:
        /*config sck polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_SCK_POLAR,
                        (sck_polar << I2S_CTRL_SCK_POLAR_FIELD_OFFSET));
        /*config ws polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_POLAR,
                        (1 << I2S_CTRL_WS_POLAR_FIELD_OFFSET));
        /*config word select mode*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_MODE, (1 << I2S_CTRL_WS_MODE_FIELD_OFFSET));
        /*config ws singal delay*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_WS_DEL,
                        (0 << I2S_CTRL_DATA_WS_DEL_FIELD_OFFSET));
        /*config data align:MSB*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ALIGN,
                        (1 << I2S_CTRL_DATA_ALIGN_FIELD_OFFSET));
        /*config data order:first send MSB */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ORDER,
                        (0 << I2S_CTRL_DATA_ORDER_FIELD_OFFSET));
        break;
    case SND_DAIFMT_DSP_A:
        /*config sck polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_SCK_POLAR,
                        (sck_polar << I2S_CTRL_SCK_POLAR_FIELD_OFFSET));

        /*config ws polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_POLAR,
                        (0 << I2S_CTRL_WS_POLAR_FIELD_OFFSET));

        /*config word select mode*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_MODE, (0 << I2S_CTRL_WS_MODE_FIELD_OFFSET));

        /*config ws singal delay*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_WS_DEL,
                        (1 << I2S_CTRL_DATA_WS_DEL_FIELD_OFFSET));

        /*config data align:MSB*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ALIGN,
                        (0 << I2S_CTRL_DATA_ALIGN_FIELD_OFFSET));

        /*config data order:first send MSB */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ORDER,
                        (0 << I2S_CTRL_DATA_ORDER_FIELD_OFFSET));
        break;
    case SND_DAIFMT_DSP_B:

        /*config sck polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_SCK_POLAR,
                        (sck_polar << I2S_CTRL_SCK_POLAR_FIELD_OFFSET));

        /*config ws polar*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_POLAR,
                        (0 << I2S_CTRL_WS_POLAR_FIELD_OFFSET));

        /*config word select mode*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_WS_MODE, (0 << I2S_CTRL_WS_MODE_FIELD_OFFSET));

        /*config ws singal delay*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_WS_DEL,
                        (0 << I2S_CTRL_DATA_WS_DEL_FIELD_OFFSET));

        /*config data align:MSB*/
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ALIGN,
                        (0 << I2S_CTRL_DATA_ALIGN_FIELD_OFFSET));

        /*config data order:first send MSB */
        snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL,
                        BIT_CTRL_DATA_ORDER,
                        (0 << I2S_CTRL_DATA_ORDER_FIELD_OFFSET));
        break;
    default:
        ssdk_printf(SSDK_ERR, "i2s mc no support this format: %d \n",
                    params->fmt & SND_DAIFMT_FORMAT_MASK);
        return false;
    }
    return true;
}
bool cdns_i2s_sc_set_tdm_slot(snd_dev_t *dev, unsigned int tx_mask,
                              unsigned int rx_mask, unsigned int slots,
                              snd_slot_width_t slot_width)
{
    // uint32_t val;
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;

    // int ret, val;
    uint32_t slots_mask = 0;

    /* Enable tdm mode */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_CTRL, BIT_TDM_CTRL_TDM_EN,
                    (1 << TDM_CTRL_TDM_EN_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_FD_DIR,
                    BIT_TDM_FD_DIR_CH_TX_EN,
                    (tx_mask << TDM_FD_DIR_CH0_TXEN_FIELD_OFFSET));

    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_FD_DIR,
                    BIT_TDM_FD_DIR_CH_RX_EN,
                    (rx_mask << TDM_FD_DIR_CH0_RXEN_FIELD_OFFSET));
    if ((slots < 1) || (slots > 16)) {
        return false;
    }
    // bitmap_fill(&slots_mask,slots);

    for (int i = 0; i < slots; i++) {
        slots_mask |= (1 << i);
    }
    ssdk_printf(SSDK_DBG_LEVEL,
                "%s:%i : tx_mask(0x%x) rx_mask(0x%x) slots(%d) "
                "slots_mask(0x%x) slot_width(%d) \n",
                __func__, __LINE__, tx_mask, rx_mask, slots, slots_mask,
                slot_width);
    i2s_sc->slot_width = check_and_get_slot_width_enum(slot_width);
    i2s_sc->slots = slots;
    i2s_sc->slots_mask = slots_mask;
    i2s_sc->tx_slot_mask = tx_mask;
    i2s_sc->rx_slot_mask = rx_mask;
    /*Audio will use tdm */
    i2s_sc->tdm_initialized = true;

    /* Enable tdm channel */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_CTRL,
                    BIT_TDM_CTRL_CHN_NUMB,
                    ((i2s_sc->slots - 1) << TDM_CTRL_CHN_NO_FIELD_OFFSET));
    /* Set enabled number  */
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_TDM_CTRL, BIT_TDM_CTRL_CH_EN,
                    (i2s_sc->slots_mask << TDM_CTRL_CH0_EN_FIELD_OFFSET));
    /*config total data width*/
    snd_update_bits(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL, BIT_CTRL_CHN_WIDTH,
                    (i2s_sc->slot_width << I2S_CTRL_CHN_WIDTH_FIELD_OFFSET));
    ssdk_printf(SSDK_DBG_LEVEL, "%d:0x%x \n", i2s_sc->slots,
                i2s_sc->slots_mask);
    /* 	ret = regmap_read(afe->regmap, REG_CDN_I2SSC_REGS_TDM_CTRL, &val);
            dev_info(afe->dev, "DUMP REG_CDN_I2SSC_REGS_TDM_CTRL(0x%x)\n", val);
            ret = regmap_read(afe->regmap, REG_CDN_I2SSC_REGS_TDM_FD_DIR, &val);
            dev_info(afe->dev, "DUMP REG_CDN_I2SSC_REGS_TDM_FD_DIR(0x%x)\n",
       val); */
    return true;
}
bool cdns_i2s_sc_trigger(snd_dev_t *dev, snd_trigger_op_t cmd,
                         snd_stream_type_t stream)
{
    // uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    ssdk_printf(SSDK_DBG_LEVEL, "cdns_i2s_sc_trigger stream(0x%x) cmd(0x%x).\n",
                stream, cmd);

    switch (cmd) {
    case SND_TRIGGER_START:
        if (stream == SND_STREAM_PLAYBACK) {
            cdns_i2s_sc_start_playback(dev);
            i2s_sc->playing = 1;
        } else {
            cdns_i2s_sc_start_capture(dev);
            i2s_sc->capturing = 1;
        }
        break;
    case SND_TRIGGER_STOP:
        if (stream == SND_STREAM_PLAYBACK) {
            cdns_i2s_sc_stop_playback(dev);
            i2s_sc->playing = 0;
        } else {
            cdns_i2s_sc_stop_capture(dev);
            i2s_sc->capturing = 0;
        }
        if (!i2s_sc->playing && !i2s_sc->capturing) {
            cdns_i2s_sc_stop(dev);
        }

        break;
    default:
        break;
    }
    return true;
}

bool cdns_i2s_sc_shutdown(snd_dev_t *dev, snd_stream_type_t stream)
{
    /*     uint32_t base_addr = dev->base;
        cdns_i2s_sc_drv_t * i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv; */
    ssdk_printf(SSDK_DBG_LEVEL, "cdns_i2s_sc_shutdown .\n");
    return true;
}

bool cdns_i2s_sc_startup(snd_dev_t *dev, snd_stream_type_t stream)
{
    uint32_t base_addr = dev->base;
    // cdns_i2s_sc_drv_t * i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    snd_stream_t *snd_stream = &dev->stream[stream];
    snd_dma_data_t *dma_data = &snd_stream->dma_data;

    dma_data->dma_addr = base_addr + I2S_SC_FIFO_OFFSET;
    ssdk_printf(SSDK_DBG_LEVEL, "cdns_i2s_sc_startup .\n");
    return true;
}

static int cdns_i2s_sc_irq_handler(uint32_t irq, void *arg)
{
    snd_dev_t *dev = (snd_dev_t *)arg;
    uint32_t base_addr = dev->base;
    cdns_i2s_sc_drv_t *i2s_sc = (cdns_i2s_sc_drv_t *)dev->priv;
    uint32_t i2s_sc_status;

    i2s_sc_status = snd_read_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_CTRL);
    snd_write_reg(base_addr, REG_CDN_I2SSC_REGS_I2S_STAT, 0);
    if (i2s_sc_status & BIT_STAT_FIFO_AEMPTY) {
        ssdk_printf(SSDK_DBG_LEVEL, "i2s sc almost empty! \n");
    }
    if (i2s_sc_status & BIT_STAT_FIFO_AFULL) {
        ssdk_printf(SSDK_DBG_LEVEL, "i2s sc almost full! \n");
    }
    if (i2s_sc_status & BIT_STAT_RDATA_OVERR) {
        if (i2s_sc->capturing) {
            ssdk_printf(SSDK_DBG_LEVEL, "i2s sc overrun! \n");
        }
    }
    if (i2s_sc_status & BIT_STAT_TDATA_UNDERR) {
        if (i2s_sc->playing) {
            ssdk_printf(SSDK_DBG_LEVEL, "i2s sc underrun! \n");
        }
    }
    return 0;
}

static snd_ops_t cdns_i2s_sc_ops = {
    .startup = cdns_i2s_sc_startup,
    .set_params = cdns_i2s_sc_set_params,
    .set_tdm_slot = cdns_i2s_sc_set_tdm_slot,
    .trigger = cdns_i2s_sc_trigger,
    .shutdown = cdns_i2s_sc_shutdown,
};

snd_dev_t *cdns_i2s_sc_probe(cdns_i2s_sc_drv_t *cdns_i2s_sc,
                             cdns_i2s_sc_cfg_t *cfg)
{
    snd_dev_t *dev = &cdns_i2s_sc->dev;

    ASSERT(NULL != dev);
    ASSERT(NULL != cfg);
    ASSERT(NULL != cdns_i2s_sc);

    cdns_i2s_sc->is_master_falling_sck_polar = cfg->is_master_falling_sck_polar;
    cdns_i2s_sc->is_slave_falling_sck_polar = cfg->is_slave_falling_sck_polar;
    cdns_i2s_sc->is_full_duplex = cfg->is_full_duplex;
    cdns_i2s_sc->is_dma_mode = cfg->is_dma_mode;
    cdns_i2s_sc->is_pack_en = cfg->is_pack_en;
    cdns_i2s_sc->tdm_initialized = false;
    cdns_i2s_sc->slot_width = AFE_I2S_CHN_WIDTH;
    cdns_i2s_sc->is_tdm_use_rx_last_slot = cdns_i2s_sc->is_tdm_use_rx_last_slot;
    /* next is common setting */
    dev->priv = cdns_i2s_sc;
    dev->ops = &cdns_i2s_sc_ops;
    dev->dma_dev = cfg->dma_dev;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->clk = cfg->clk;
    // cdns_i2s_sc_init(dev); init device.
    ssdk_printf(SSDK_DBG_LEVEL, "cdns i2s sc initialized, dev: %p\n", dev);
    snd_dev_init(dev);
    // set clock
    if (dev->irq > 0) {
        irq_attach(dev->irq, cdns_i2s_sc_irq_handler, dev);
        irq_enable(dev->irq);
    }
    /*Config i2s sc*/
    cdns_i2s_sc_config(dev);
    return dev;
}
