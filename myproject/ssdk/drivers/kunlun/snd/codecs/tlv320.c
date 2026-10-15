/**
 * @file tlv320.c
 * @version 0.1
 * @date 2022-02-11
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#include <board.h>
#include <debug.h>
#include <driver/irq.h>
#include <kunlun/snd/codecs/tlv320.h>
#include <param.h>
#include <string.h>

#include "cmsis_os2.h"
#include "driver/i2c.h"

#define TLV320AIC23_INSTANCE_I2C_1A 0x1a
#define TLV320AIC23_INSTANCE_I2C_1B 0x1b
#define TLV320AI23_MAX_DEV_NUM 3

/* Codec TLV320AIC23 */
#define TLV320AIC23_LINVOL 0x00
#define TLV320AIC23_RINVOL 0x01
#define TLV320AIC23_LCHNVOL 0x02
#define TLV320AIC23_RCHNVOL 0x03
#define TLV320AIC23_ANLG 0x04
#define TLV320AIC23_DIGT 0x05
#define TLV320AIC23_PWR 0x06
#define TLV320AIC23_DIGT_FMT 0x07
#define TLV320AIC23_SRATE 0x08
#define TLV320AIC23_ACTIVE 0x09
#define TLV320AIC23_RESET 0x0F

/* Left (right) line input volume control register */
#define TLV320AIC23_LRS_ENABLED 0x0100
#define TLV320AIC23_LIM_MUTED 0x0080
#define TLV320AIC23_LIV_DEFAULT 0x0017
#define TLV320AIC23_LIV_MAX 0x001f
#define TLV320AIC23_LIV_MIN 0x0000

/* Left (right) channel headphone volume control register */
#define TLV320AIC23_LZC_ON 0x0080
#define TLV320AIC23_LHV_DEFAULT 0x0079
#define TLV320AIC23_LHV_MAX 0x007f
#define TLV320AIC23_LHV_MIN 0x0030

/* Analog audio path control register */
#define TLV320AIC23_STA_REG(x) ((x) << 6)
#define TLV320AIC23_STE_ENABLED 0x0020
#define TLV320AIC23_DAC_SELECTED 0x0010
#define TLV320AIC23_BYPASS_ON 0x0008
#define TLV320AIC23_INSEL_MIC 0x0004
#define TLV320AIC23_MICM_MUTED 0x0002
#define TLV320AIC23_MICB_20DB 0x0001

/* Digital audio path control register */
#define TLV320AIC23_DACM_MUTE 0x0008
#define TLV320AIC23_DEEMP_32K 0x0002
#define TLV320AIC23_DEEMP_44K 0x0004
#define TLV320AIC23_DEEMP_48K 0x0006
#define TLV320AIC23_ADCHP_ON 0x0001

/* Power control down register */
#define TLV320AIC23_DEVICE_PWR_OFF 0x0080
#define TLV320AIC23_CLK_OFF 0x0040
#define TLV320AIC23_OSC_OFF 0x0020
#define TLV320AIC23_OUT_OFF 0x0010
#define TLV320AIC23_DAC_OFF 0x0008
#define TLV320AIC23_ADC_OFF 0x0004
#define TLV320AIC23_MIC_OFF 0x0002
#define TLV320AIC23_LINE_OFF 0x0001

/* Digital audio interface register */
#define TLV320AIC23_MS_MASTER 0x0040
#define TLV320AIC23_LRSWAP_ON 0x0020
#define TLV320AIC23_LRP_ON 0x0010
#define TLV320AIC23_IWL_16 0x0000
#define TLV320AIC23_IWL_20 0x0004
#define TLV320AIC23_IWL_24 0x0008
#define TLV320AIC23_IWL_32 0x000C
#define TLV320AIC23_FOR_I2S 0x0002
#define TLV320AIC23_FOR_DSP 0x0003
#define TLV320AIC23_FOR_LJUST 0x0001

/* Sample rate control register */
#define TLV320AIC23_CLKOUT_HALF 0x0080
#define TLV320AIC23_CLKIN_HALF 0x0040
#define TLV320AIC23_BOSR_384fs 0x0002 /* BOSR_272fs in USB mode */
#define TLV320AIC23_USB_CLK_ON 0x0001
#define TLV320AIC23_SR_MASK 0xf
#define TLV320AIC23_CLKOUT_SHIFT 7
#define TLV320AIC23_CLKIN_SHIFT 6
#define TLV320AIC23_SR_SHIFT 2
#define TLV320AIC23_BOSR_SHIFT 1

/* Digital interface register */
#define TLV320AIC23_ACT_ON 0x0001

#define AUDIO_CODEC_DEBUG 2
#define TLV320AIC23B_Q1_REG_MAX_ADDR (0xf + 1)

/* functions declaration */
static bool sdrv_tlv320_initialize(codec_dev_t *dev, snd_stream_type_t stream);
static bool sdrv_tlv320_start_up(codec_dev_t *dev, snd_stream_type_t stream);
static bool sdrv_tlv320_set_volume(tlv320_dev_t *codec, int volume_percent,
                                   enum audio_volume_type vol_type);
static bool sdrv_tlv320_set_format(codec_dev_t *dev, unsigned int fmt,
                                   snd_stream_type_t stream);
static bool sdrv_tlv320_set_hw_params(codec_dev_t *dev, snd_params_t *params,
                                      snd_stream_type_t stream);
static bool sdrv_tlv320_trigger(codec_dev_t *dev, int cmd,
                                snd_stream_type_t stream);
static bool sdrv_tlv320_shutdown(codec_dev_t *dev, snd_stream_type_t stream);
static bool sdrv_tlv320_set_input_path(tlv320_dev_t *codec,
                                       uint32_t input_path);
static bool sdrv_tlv320_set_output_path(tlv320_dev_t *codec,
                                        uint32_t output_path);

/*save register value */

struct reg_default tlv320_reg[] = {
    {0x0, 0x17}, {0x1, 0x17}, {0x2, 0x70}, {0x3, 0x70},
    {0x4, 0x14}, {0x5, 0x04}, {0x6, 0x47}, {0x7, 0x02},
    {0x8, 0xfc}, {0x9, 0x01}, {0xa, 0x00}, {0xb, 0x00},
    {0xc, 0x00}, {0xd, 0x00}, {0xe, 0x00}, {0xf, 0x00},
};

/* read register */

static unsigned int tlv320_read_reg(tlv320_dev_t *codec, unsigned int reg)
{

    return tlv320_reg[reg].val;
}

void tlv320_dump(tlv320_dev_t *codec)
{
    unsigned int val;

    for (int i = 0; i < ARRAY_SIZE(tlv320_reg); i++) {

        val = tlv320_read_reg(codec, tlv320_reg[i].reg);
        printf("%s: addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
               codec->codec_addr, tlv320_reg[i].reg, val);
    }
}

/* tlv320aic23b_q1 is a write-only device */
static int tlv320_write_reg(tlv320_dev_t *codec, unsigned int reg,
                            unsigned int val)
{
    uint8_t buf[2], addr;
    int ret = 0;

    addr = (reg << 1) | (val >> 8 & 0x01);
    buf[0] = addr;
    buf[1] = (val & 0xff);

    ret = i2c_write(codec->i2c_handle, codec->codec_addr, buf, 2);
    if (ret < 0) {
        printf("%s: error:write codec_addr=0x%x, reg=0x%x, val=%x\n", __func__,
               codec->codec_addr, reg, val);
    }
    /* local register update */
    tlv320_reg[reg].val = val;
    return ret;
}

/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
static bool sdrv_tlv320_start_up(codec_dev_t *dev, snd_stream_type_t stream)
{
    tlv320_dev_t *codec = (tlv320_dev_t *)dev->priv;

    uint32_t power = 0, val = 0;
    /* reset */
    tlv320_write_reg(codec, TLV320AIC23_RESET, 0x0);
    codec->hphone_out_vol = 50;
    codec->line_in_vol = 50;

    /* power down */
    power =
        tlv320_read_reg(codec, TLV320AIC23_PWR) | TLV320AIC23_DEVICE_PWR_OFF;
    tlv320_write_reg(codec, TLV320AIC23_PWR, power);

    /* set common register */
    tlv320_write_reg(codec, TLV320AIC23_DIGT, 0x4);

    val = TLV320AIC23_DAC_SELECTED | TLV320AIC23_INSEL_MIC;

    /* set line in as default */
    tlv320_write_reg(codec, TLV320AIC23_ANLG, val);

    /* set default volume, both input and output */
    sdrv_tlv320_set_volume(codec, codec->hphone_out_vol, AUDIO_VOL_HEADPHONE);
    sdrv_tlv320_set_volume(codec, codec->line_in_vol, AUDIO_VOL_LINEOUT);

    sdrv_tlv320_set_output_path(codec, AUDIO_CODEC_SET_OUTPUT_AS_PHONE_OUT);
    sdrv_tlv320_set_input_path(codec, AUDIO_CODEC_SET_INPUT_AS_MIC_IN);

    return true;
}

/**
 * @brief
 *
 * @param codec
 * @param volume_percent
 * @param vol_type
 * @return true
 * @return false
 */
static bool sdrv_tlv320_set_volume(tlv320_dev_t *codec, int volume_percent,
                                   enum audio_volume_type vol_type)
{
    int step;
    uint32_t vol;

    if (vol_type == AUDIO_VOL_HEADPHONE) {
        codec->hphone_out_vol = volume_percent;
        /* set phone out volume; left/right volume equal */
        step = (TLV320AIC23_LHV_MAX - TLV320AIC23_LHV_MIN) / 10;
        vol = (volume_percent / 10 * step) & TLV320AIC23_LHV_MAX;
        vol += TLV320AIC23_LHV_MIN;
        tlv320_write_reg(codec, TLV320AIC23_LCHNVOL,
                         TLV320AIC23_LRS_ENABLED | vol);
        tlv320_write_reg(codec, TLV320AIC23_RCHNVOL,
                         TLV320AIC23_LRS_ENABLED | vol);
    } else if (vol_type == AUDIO_VOL_LINEOUT) {
        codec->line_in_vol = volume_percent;
        /* set line input volume; left/right volume equal */
        step = (TLV320AIC23_LIV_MAX - TLV320AIC23_LIV_MIN) / 10;
        vol = (volume_percent / 10 * step) & TLV320AIC23_LIV_MAX;
        tlv320_write_reg(codec, TLV320AIC23_LINVOL,
                         TLV320AIC23_LRS_ENABLED | vol);
        tlv320_write_reg(codec, TLV320AIC23_RINVOL,
                         TLV320AIC23_LRS_ENABLED | vol);
    }

    return true;
}

/**
 * @brief
 *
 * @param dev
 * @param fmt
 * @param stream
 * @return true
 * @return false
 */
static bool sdrv_tlv320_set_format(codec_dev_t *dev, unsigned int fmt,
                                   snd_stream_type_t stream)
{
#if 1
    int interface = 0, ms = 0;
    tlv320_dev_t *codec = (tlv320_dev_t *)dev->priv;

    /* set i2s interface mode */
    switch (fmt & SND_DAIFMT_FORMAT_MASK) {
    case SND_DAIFMT_STANDARD_PHILLIPS:
        interface = TLV320AIC23_FOR_I2S;
        break;

    case SND_DAIFMT_LEFT_JUSTIFIED:
        interface = TLV320AIC23_FOR_LJUST;
        break;

    case SND_DAIFMT_RIGHT_JUSTIFIED:
        ssdk_printf(AUDIO_CODEC_DEBUG,
                    "func<%s>: unsupported i2s right justified mode.\n",
                    __func__);
        break;

    case SND_DAIFMT_DSP_A:
        interface = TLV320AIC23_FOR_DSP | TLV320AIC23_LRP_ON;
        break;

    case SND_DAIFMT_DSP_B:
        interface = TLV320AIC23_FOR_DSP & (~TLV320AIC23_LRP_ON);
        break;

    default:
        ssdk_printf(AUDIO_CODEC_DEBUG,
                    "codec interface mode wrong arg or no init.\n");
    }

    /* set master/slave */
    switch (fmt & SND_DAIFMT_MASTER_MASK) {
    /* codec clk & FRM slave i2s master */
    case SND_DAIFMT_CODEC_SLAVE:
        /* config master mode */
        ms = 0;
        break;

    /* codec clk & FRM master i2s slave */
    case SND_DAIFMT_CODEC_MASTER:
        ms = TLV320AIC23_MS_MASTER;
        break;
    default:
        ssdk_printf(AUDIO_CODEC_DEBUG, "tx func<%s>: i2s mastet or slave ? ?\n",
                    __func__);
        return false;
    }

    tlv320_write_reg(codec, TLV320AIC23_DIGT_FMT, interface | ms);
#endif
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
static bool sdrv_tlv320_set_hw_params(codec_dev_t *dev, snd_params_t *params,
                                      snd_stream_type_t stream)
{
#if 1
    uint32_t sample_rate = 0, slot_width = 0, format = 0;
    tlv320_dev_t *codec = (tlv320_dev_t *)dev->priv;

    /* set sample rate */
    sample_rate = params->sample_rate;
    uint32_t val = 0;

    switch (sample_rate) {
    case SND_SR_8000:
        val = 0x3 << TLV320AIC23_SR_SHIFT;
        break;

    case SND_SR_32000:
        val = 0x6 << TLV320AIC23_SR_SHIFT;
        break;

    case SND_SR_48000:
        val = 0;
        break;

    case SND_SR_96000:
        val = 0x7 << TLV320AIC23_SR_SHIFT;
        break;

    default:
        ssdk_printf(AUDIO_CODEC_DEBUG, "unsupport sample rate: %d.\n",
                    sample_rate);
    }

    tlv320_write_reg(codec, TLV320AIC23_SRATE, val);

    /* set pcm data slot width */
    format = tlv320_read_reg(codec, TLV320AIC23_DIGT_FMT);

    switch (params->sample_width) {
    case SND_SAMPLE_WIDTH_16BITS:
        slot_width = TLV320AIC23_IWL_16;
        break;

    case SND_SAMPLE_WIDTH_20BITS:
        slot_width = TLV320AIC23_IWL_20;
        break;

    case SND_SAMPLE_WIDTH_24BITS:
        slot_width = TLV320AIC23_IWL_24;
        break;

    case SND_SAMPLE_WIDTH_32BITS:
        slot_width = TLV320AIC23_IWL_32;
        break;

    default:
        ssdk_printf(AUDIO_CODEC_DEBUG,
                    "codec slot width wrong arg or no init.\n");
    }

    format |= slot_width;
    tlv320_write_reg(codec, TLV320AIC23_DIGT_FMT, format);
#endif
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
static bool sdrv_tlv320_trigger(codec_dev_t *dev, int cmd,
                                snd_stream_type_t stream)
{
    tlv320_dev_t *codec = (tlv320_dev_t *)dev->priv;

#if 1
    uint32_t power = 0;

    /* inactive digital interface and power off */
    tlv320_write_reg(codec, TLV320AIC23_ACTIVE, 0x0);
    power =
        tlv320_read_reg(codec, TLV320AIC23_PWR) | TLV320AIC23_DEVICE_PWR_OFF;
    tlv320_write_reg(codec, TLV320AIC23_PWR, power);

    switch (cmd) {
    case SND_TRIGGER_START:
        if (stream == SND_STREAM_PLAYBACK) {
            power &= ~(TLV320AIC23_DAC_OFF | TLV320AIC23_OUT_OFF);
            power &= ~(TLV320AIC23_ADC_OFF | TLV320AIC23_MIC_OFF);
        } else {
            if (codec->input_path == AUDIO_CODEC_SET_INPUT_AS_LINE_IN ||
                codec->input_path == AUDIO_CODEC_SET_INPUT_AS_DEFAULT)
                power &= ~(TLV320AIC23_ADC_OFF | TLV320AIC23_LINE_OFF);
            else
                power &= ~(TLV320AIC23_ADC_OFF | TLV320AIC23_MIC_OFF);
        }

        break;

    case SND_TRIGGER_STOP:
        if (stream == SND_STREAM_PLAYBACK) {
            power |= TLV320AIC23_DAC_OFF | TLV320AIC23_OUT_OFF;
        } else {
            power |= TLV320AIC23_ADC_OFF | TLV320AIC23_LINE_OFF |
                     TLV320AIC23_MIC_OFF;
        }
        break;

    default:
        ssdk_printf(AUDIO_CODEC_DEBUG, "func<%s>: cmd not support ? ?\n",
                    __func__);
    }

    tlv320_write_reg(codec, TLV320AIC23_ACTIVE, TLV320AIC23_ACT_ON);
    tlv320_write_reg(codec, TLV320AIC23_PWR,
                     power & ~TLV320AIC23_DEVICE_PWR_OFF);
#endif

    tlv320_dump(codec);
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
static bool sdrv_tlv320_shutdown(codec_dev_t *dev, snd_stream_type_t stream)
{
    tlv320_dev_t *codec = (tlv320_dev_t *)dev->priv;

    /* inactive and power down */
    tlv320_write_reg(codec, TLV320AIC23_ACTIVE, 0x0);
    tlv320_write_reg(codec, TLV320AIC23_PWR, 0xff);

    return true;
}

/**
 * @brief
 *
 * @param codec
 * @param input_path
 * @return true
 * @return false
 */
static bool sdrv_tlv320_set_input_path(tlv320_dev_t *codec, uint32_t input_path)
{
    if (input_path == AUDIO_CODEC_SET_INPUT_AS_MIC_IN) {
        uint32_t analog_au_path;
        analog_au_path =
            tlv320_read_reg(codec, TLV320AIC23_ANLG) | TLV320AIC23_INSEL_MIC;
        tlv320_write_reg(codec, TLV320AIC23_ANLG, analog_au_path);
    }

    codec->input_path = input_path;

    return true;
}

/**
 * @brief
 *
 * @param codec
 * @param output_path
 * @return true
 * @return false
 */
static bool sdrv_tlv320_set_output_path(tlv320_dev_t *codec,
                                        uint32_t output_path)
{
    codec->output_path = output_path;

    return true;
}

static codec_ops_t tlv320_ops = {
    .startup = sdrv_tlv320_start_up,
    .set_params = sdrv_tlv320_set_hw_params,
    .trigger = sdrv_tlv320_trigger,
    .shutdown = sdrv_tlv320_shutdown,
};

/**
 * @brief
 *
 * @param codec
 * @param cfg
 * @return codec_dev_t*
 */
codec_dev_t *tlv320_probe(tlv320_dev_t *codec, tlv320_cfg_t *cfg)
{
    codec_dev_t *dev = &codec->dev;

    ASSERT(NULL != dev);
    ASSERT(NULL != codec);
    ASSERT(NULL != cfg);

    dev->priv = codec;
    dev->ops = &tlv320_ops;
    codec->clk_rate = cfg->clk_rate;
    codec->i2c_handle = cfg->i2c_handle;
    codec->codec_addr = cfg->codec_addr;
    codec->gpio_pdn = cfg->gpio_pdn;
    codec->gpio_dev = cfg->gpio_dev;

    codec_dev_init(dev);

    return dev;
}
