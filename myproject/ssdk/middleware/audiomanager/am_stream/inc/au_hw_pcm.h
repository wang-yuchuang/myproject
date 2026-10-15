/**
 * @file au_hw_pcm.h
 * @brief audio hardware pcm header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_HW_PCM_H__
#define __AU_HW_PCM_H__

#include <stdint.h>
#include <stdlib.h>

#include "au_hw_device.h"

typedef struct au_hw_pcm *hw_pcm_handle_t;

typedef struct au_hw_pcm {
    void *i2s_handle;
    hw_device_callback hw_callback;
    void *cb_param;
    au_hw_device_status_t status;
} au_hw_pcm_t;

typedef enum {
    AU_HW_PCM_STATUS_CLOSED = 0,
    AU_HW_PCM_STATUS_STOPED = 1,
    AU_HW_PCM_STATUS_OPENED = 2,
    AU_HW_PCM_STATUS_STARTED = 3,
} au_hw_pcm_status_t;

au_hw_device_ops_t *au_get_hw_pcm_ops(void);

#endif
