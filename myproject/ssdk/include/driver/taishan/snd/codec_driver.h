
/**
 * @file codec_driver.h
 * @version 0.1
 * @date 2022-03-01
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 */

#ifndef _CODEC_DRIVER_H_
#define _CODEC_DRIVER_H_
#include <cmsis_os2.h>
#include <debug.h>
#include <stdbool.h>
#include <types.h>

#include "driver/snd_driver.h"

typedef struct codec_dev codec_dev_t;
struct reg_default {
    unsigned int reg;
    unsigned int val;
};
typedef struct codec_ops {
    bool (*startup)(codec_dev_t *dev, snd_stream_type_t stream);
    bool (*set_params)(codec_dev_t *dev, snd_params_t *params,
                       snd_stream_type_t stream);
    bool (*set_tdm_slot)(codec_dev_t *dev, unsigned int mask, int slots,
                         snd_slot_width_t slot_width, snd_stream_type_t stream);
    bool (*trigger)(codec_dev_t *dev, int cmd, snd_stream_type_t stream);
    bool (*shutdown)(codec_dev_t *dev, snd_stream_type_t stream);
} codec_ops_t;

struct codec_dev {
    snd_params_t params[2];
    codec_ops_t *ops;
    osMutexId_t dev_lock;
    void *priv;
};

bool codec_startup(codec_dev_t *dev, snd_stream_type_t stream);
bool codec_set_params(codec_dev_t *dev, snd_params_t *params,
                      snd_stream_type_t stream);
bool codec_trigger(codec_dev_t *dev, int cmd, snd_stream_type_t stream);
bool codec_shutdown(codec_dev_t *dev, snd_stream_type_t stream);
void codec_dev_init(codec_dev_t *dev);

#endif /* _CODEC_DRIVER_H_ */