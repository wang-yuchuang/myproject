/**
 * @file ak4619.c
 * @version 0.1
 * @date 2022-03-01
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#include <board.h>
#include <debug.h>
#include <driver/irq.h>
#include <taishan/snd/codecs/ak4619.h>
#include <param.h>
#include <string.h>

#include "cmsis_os2.h"
#include "driver/i2c.h"

struct reg_default i2s_stereo_ak4619_reg[] = {
    {0x0, 0x27},  {0x1, 0x08},  {0x2, 0x0A},  {0x3, 0x00},  {0x4, 0x22},
    {0x5, 0x22},  {0x6, 0x30},  {0x7, 0x30},  {0x8, 0x30},  {0x9, 0x30},
    {0xa, 0x00},  {0xb, 0x00},  {0xc, 0x00},  {0xd, 0x00},  {0xe, 0x2c},
    {0xf, 0x2c},  {0x10, 0x2c}, {0x11, 0x2c}, {0x12, 0x00}, {0x13, 0x05},
    {0x14, 0x0A},
};

struct reg_default tdm_128fs_ak4619_reg[] = {
    {0x0, 0x37},  {0x1, 0xFC},  {0x2, 0x1A},  {0x3, 0x00},  {0x4, 0x22},
    {0x5, 0x22},  {0x6, 0x30},  {0x7, 0x30},  {0x8, 0x30},  {0x9, 0x30},
    {0xa, 0x00},  {0xb, 0x00},  {0xc, 0x00},  {0xd, 0x00},  {0xe, 0x2c},
    {0xf, 0x2c},  {0x10, 0x2c}, {0x11, 0x2c}, {0x12, 0x00}, {0x13, 0x05},
    {0x14, 0x0A},
};

static int ak4619_write_reg(ak4619_dev_t *codec, uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;
    ret = i2c_write(codec->i2c_handle, codec->codec_addr, buf, 2);
    if (ret < 0) {
        printf("%s: error:write  codec_addr=0x%x reg=0x%x, val=%x\n", __func__,
               codec->codec_addr, reg, val);
    }

    return ret;
}

static int ak4619_read_reg(ak4619_dev_t *codec, uint8_t reg, uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));
    ret = i2c_write_read(codec->i2c_handle, codec->codec_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        printf("%s: error: read codec_addr=0x%x, reg=0x%x\n", __func__,
               codec->codec_addr, reg);
        return ret;
    }

    printf("%s: codec_addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
           codec->codec_addr, reg, buf[0]);

    *val = buf[0];
    return 0;
}

void ak4619_dump(ak4619_dev_t *codec)
{
    uint8_t val;

    for (int i = 0; i < ARRAY_SIZE(i2s_stereo_ak4619_reg); i++) {

        ak4619_read_reg(codec, i2s_stereo_ak4619_reg[i].reg, &val);
    }
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
bool ak4619_startup(codec_dev_t *dev, snd_stream_type_t stream)
{
    bool ret = false;
    ak4619_dev_t *codec = (ak4619_dev_t *)dev->priv;

    gpio_set(codec->gpio_dev, codec->gpio_pdn, false);
    udelay(100);
    gpio_set(codec->gpio_dev, codec->gpio_pdn, true);

    if (stream == SND_STREAM_PLAYBACK) {

    } else {
    }

    return ret;
}

static bool ak4619_set_format(codec_dev_t *dev, unsigned int fmt,
                              snd_stream_type_t stream)
{
    ak4619_dev_t *codec = (ak4619_dev_t *)dev->priv;
    int try = 3;
    int ret_val;

    switch (fmt & SND_DAIFMT_FORMAT_MASK) {

    case SND_DAIFMT_STANDARD_PHILLIPS:

        for (int i = 0; i < ARRAY_SIZE(i2s_stereo_ak4619_reg); i++) {
            try = 3;
            while (try > 0) {
                ret_val = ak4619_write_reg(codec, i2s_stereo_ak4619_reg[i].reg,
                                           i2s_stereo_ak4619_reg[i].val);
                if (ret_val < 0) {
                    printf("%s: error: write reg=0x%x\n", __func__,
                           i2s_stereo_ak4619_reg[i].reg);
                    try--;
                } else {
                    break;
                }
            }
        }

        break;

    case SND_DAIFMT_DSP_B:

        for (int i = 0; i < ARRAY_SIZE(i2s_stereo_ak4619_reg); i++) {
            try = 3;
            while (try > 0) {
                ret_val = ak4619_write_reg(codec, tdm_128fs_ak4619_reg[i].reg,
                                           tdm_128fs_ak4619_reg[i].val);
                if (ret_val < 0) {
                    printf("%s: error: write reg=0x%x\n", __func__,
                           tdm_128fs_ak4619_reg[i].reg);
                    try--;
                } else {
                    break;
                }
            }
        }
        break;

    default:
        printf("ak4719 %s unsupport fmt !!!\n", __func__);

        return false;
    }

    ak4619_dump(codec);

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
bool ak4619_set_params(codec_dev_t *dev, snd_params_t *params,
                       snd_stream_type_t stream)
{

    bool ret = false;

    ak4619_set_format(dev, params->fmt, stream);

    return ret;
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
bool ak4619_trigger(codec_dev_t *dev, int cmd, snd_stream_type_t stream)
{
    bool ret = false;
    if (stream == SND_STREAM_PLAYBACK) {

    } else {
    }

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
bool ak4619_shutdown(codec_dev_t *dev, snd_stream_type_t stream)
{
    bool ret = false;
    ak4619_dev_t *codec = (ak4619_dev_t *)dev->priv;

    if (stream == SND_STREAM_PLAYBACK) {

    } else {
    }

    gpio_set(codec->gpio_dev, codec->gpio_pdn, false);
    return ret;
}

static codec_ops_t ak4619_ops = {
    .startup = ak4619_startup,
    .set_params = ak4619_set_params,
    .trigger = ak4619_trigger,
    .shutdown = ak4619_shutdown,
};

codec_dev_t *ak4619_probe(ak4619_dev_t *codec, ak4619_cfg_t *cfg)
{
    codec_dev_t *dev = &codec->dev;
    ASSERT(NULL != dev);
    ASSERT(NULL != codec);
    ASSERT(NULL != cfg);

    dev->priv = codec;
    dev->ops = &ak4619_ops;
    codec->clk_rate = cfg->clk_rate;
    codec->i2c_handle = cfg->i2c_handle;
    codec->codec_addr = cfg->codec_addr;
    codec->gpio_pdn = cfg->gpio_pdn;
    codec->gpio_dev = cfg->gpio_dev;
    codec_dev_init(dev);

    return dev;
}