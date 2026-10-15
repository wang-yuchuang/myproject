/**
 * @file irda.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>
#include <types.h>

#include <driver/irq.h>
#include <driver/irda.h>

#define EXTRA_FMT   "irda:"
#define IRDA_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define IRDA_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define IRDA_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/**
 * @brief IrDA init.
 *
 * @param[in] device irda dev
 * @param[in] config irda config
 */
void irda_init(irda_dev_t *device, const irda_config_t *config)
{
    device->lock = osMutexNew(NULL);
    ASSERT(device->lock);

    device->base = config->base;
    device->irq = config->irq;
    device->carrier_freq = config->carrier_freq;
    device->clk_freq = config->clk_freq;

    device->ops->init(device, config);
}

/**
 * @brief IrDA deinit.
 *
 * @param[in] device irda dev
 */
void irda_deinit(irda_dev_t *device)
{
    device->ops->deinit(device);
    osMutexDelete(device->lock);
}

/**
 * @brief IrDA send data.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @param[in] timeout send timeout value
 * @return send result
 */
uint8_t irda_send(irda_dev_t *device, const char *data,
                  uint8_t len, uint32_t timeout)
{
    uint8_t ret = IRDA_N_OK;

    osMutexAcquire(device->lock, osWaitForever);
    ret = device->ops->send(device, data, len, timeout);
    osMutexRelease(device->lock);
    return ret;
}

/**
 * @brief IrDA send data in aync mode.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @param[in] cb callback function
 * @param[in] arg callback arg
 * @return send result
 */
uint8_t irda_send_async(irda_dev_t *device, const char *data,
                        uint8_t len, irda_cb_t cb, void *arg)
{
    uint8_t ret = IRDA_N_OK;

    osMutexAcquire(device->lock, osWaitForever);
    ret = device->ops->send_async(device, data, len, cb, arg);
    osMutexRelease(device->lock);
    return ret;
}

/**
 * @brief IrDA receive data.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @param[in] timeout receive timeout value
 * @return receive result
 */
uint8_t irda_receive(irda_dev_t *device, char *data,
                     uint8_t len, uint32_t timeout)
{
    uint8_t ret = IRDA_N_OK;

    osMutexAcquire(device->lock, osWaitForever);
    ret = device->ops->recv(device, data, len, timeout);
    osMutexRelease(device->lock);
    return ret;
}

/**
 * @brief IrDA receive data in aync mode.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @param[in] cb callback function
 * @param[in] arg callback arg
 * @return receive result
 */
uint8_t irda_receive_async(irda_dev_t *device, char *data,
                           uint8_t len, irda_cb_t cb, void *arg)
{
    uint8_t ret = IRDA_N_OK;

    osMutexAcquire(device->lock, osWaitForever);
    ret = device->ops->recv_async(device, data, len, cb, arg);
    osMutexRelease(device->lock);
    return ret;
}
