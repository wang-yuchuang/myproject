/**
 * @file ak4619.h
 * @version 0.1
 * @date 2022-03-01
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#ifndef AK4619_H_
#define AK4619_H_

#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/snd/codec_driver.h>

typedef struct ak4619_dev ak4619_dev_t;

struct ak4619_dev {
    codec_dev_t dev;
    uint32_t clk_rate;
    void *i2c_handle;
    uint16_t codec_addr;
    gpio_dev_t *gpio_dev;
    unsigned int gpio_pdn;
};

typedef struct ak4619_cfg {
    void *i2c_handle;
    uint16_t codec_addr;
    gpio_dev_t *gpio_dev;
    unsigned int gpio_pdn;
    uint32_t clk_rate;
} ak4619_cfg_t;

codec_dev_t *ak4619_probe(ak4619_dev_t *codec, ak4619_cfg_t *cfg);

#endif /* AK4619_H_ */
