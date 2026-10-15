/**
 * @file tlv320.h
 * @version 0.1
 * @date 2022-02-11
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#ifndef __TLV320_H__
#define __TLV320_H__
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/snd/codec_driver.h>

typedef struct tlv320_dev tlv320_dev_t;

enum audio_codec_input {
    AUDIO_CODEC_SET_INPUT_AS_DEFAULT = 0,
    AUDIO_CODEC_SET_INPUT_AS_LINE_IN,
    AUDIO_CODEC_SET_INPUT_AS_MIC_IN,
};

enum audio_codec_output {
    AUDIO_CODEC_SET_OUTPUT_AS_DEFAULT = 0,
    AUDIO_CODEC_SET_OUTPUT_AS_LINE_OUT,
    AUDIO_CODEC_SET_OUTPUT_AS_PHONE_OUT,
};

enum audio_volume_type {
    AUDIO_VOL_INPUT = 0,
    AUDIO_VOL_LINEIN = AUDIO_VOL_INPUT,
    AUDIO_VOL_OUTPUT,
    AUDIO_VOL_LINEOUT = AUDIO_VOL_OUTPUT,
    AUDIO_VOL_HEADPHONE,
    AUDIO_VOL_HEADSET_MIC,

};

struct tlv320_dev {
    codec_dev_t dev;
    uint32_t clk_rate;
    void *i2c_handle;
    uint16_t codec_addr;
    gpio_dev_t *gpio_dev;
    unsigned int gpio_pdn;
    enum audio_codec_input input_path;
    enum audio_codec_output output_path;
    int hphone_out_vol;
    int line_in_vol;
};

typedef struct tlv320_cfg {
    void *i2c_handle;
    uint16_t codec_addr;
    gpio_dev_t *gpio_dev;
    unsigned int gpio_pdn;
    uint32_t clk_rate;
} tlv320_cfg_t;

codec_dev_t *tlv320_probe(tlv320_dev_t *codec, tlv320_cfg_t *cfg);

#endif