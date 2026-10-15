/**
 * @file adc.c
 * @brief adc subsystem core source.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <string.h>
#include <debug.h>
#include <driver/adc.h>
#include <driver/irq.h>
#include <types.h>

#include "cmsis_os2.h"

int32_t adc_start(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                  adc_buff_t *buf, uint16_t len)
{
    int32_t ret = 0;

    ASSERT(device && device->ops);

    osMutexAcquire(device->mutex, osWaitForever);

    ret = device->ops->adc_start(device, channel_chain, buf, len);

    if (ADC_XFER_POLLING_MODE_E != device->adc_conf.xfer_mode) {
        ret = irq_attach(device->adc_conf.irq, device->ops->adc_irq_handler, (void *)device);
        if (0 != ret) {
          osMutexRelease(device->mutex);
          return ret;
        }

        irq_enable(device->adc_conf.irq);
    }

    osMutexRelease(device->mutex);

    return ret;
}

int32_t adc_stop(adc_dev_t *device)
{
    int32_t ret = 0;

    ASSERT(device);

    osMutexAcquire(device->mutex, osWaitForever);

    ret = device->ops->adc_stop(device);

    if (ADC_XFER_POLLING_MODE_E != device->adc_conf.xfer_mode) {
        irq_detach(device->adc_conf.irq);
        irq_disable(device->adc_conf.irq);
    }

    osMutexRelease(device->mutex);
    return ret;
}

int32_t adc_read_sample(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                        adc_buff_t *buff, uint16_t len)
{
    int32_t ret = 0;

    ASSERT(device);
    ASSERT(channel_chain);
    ASSERT(buff);

    osMutexAcquire(device->mutex, osWaitForever);

    ret = device->ops->adc_read_sample(device, channel_chain, buff, len);

    osMutexRelease(device->mutex);

    return ret;
}

int32_t adc_init(adc_dev_t *device, const adc_config_t *cfg)
{
    ASSERT(device);
    ASSERT(cfg);

    memset(device, 0, sizeof(adc_dev_t));
    memcpy(&device->adc_conf, cfg, sizeof(adc_config_t));

    device->mutex = osMutexNew(NULL);

    return 0;
}

int32_t adc_deinit(adc_dev_t *device)
{
    ASSERT(device);

    osMutexDelete(device->mutex);

    return 0;
}

void adc_main_func(adc_dev_t *device)
{
    ASSERT(device);

    if (ADC_XFER_POLLING_MODE_E == device->adc_conf.xfer_mode) {
        device->ops->adc_main_func(device);
    }
}

void adc_add_channel_mask(adc_channel_chain_t *name, adc_ch_size_t num, ...)
{
    va_list argptr;
    adc_ch_size_t tmp;

    ASSERT(name);

    va_start(argptr, num);
    while (num--) {
        tmp = va_arg(argptr, uint32_t);
        name->chain_mask[tmp / 8] |= 1u << (tmp % 8);
    }
    va_end(argptr);
}

void adc_clear_channel_mask(adc_channel_chain_t *name, adc_ch_size_t num, ...)
{
    va_list argptr;
    adc_ch_size_t tmp;

    ASSERT(name);

    va_start(argptr, num);
    while (num--) {
        tmp = va_arg(argptr, uint32_t);
        name->chain_mask[tmp / 8] &= ~(1u << (tmp % 8));
    }
    va_end(argptr);
}

void adc_clear_all_channel_mask(adc_channel_chain_t *name)
{
    ASSERT(name);

    for (adc_ch_size_t i = 0; i < name->chain_mask_len; i++) {
        name->chain_mask[i] = 0;
    }
}

