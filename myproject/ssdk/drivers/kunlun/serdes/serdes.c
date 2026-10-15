/**
 * @file serdes.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes common api implement
 *
 * Revision History:
 * -----------------
*/
#include <types.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <driver/kunlun/serdes/serdes.h>
#include <driver/kunlun/serdes/serdes_vendor.h>

/** macro define section */
#define SERDES_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "serdes:"

#if SERDES_DEBUG_LVL >= SSDK_ERR
#define SERDES_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SERDES_ERR(format, ...)
#endif

#if SERDES_DEBUG_LVL >= SSDK_WARNING
#define SERDES_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SERDES_WARN(format, ...)
#endif

#if SERDES_DEBUG_LVL >= SSDK_INFO
#define SERDES_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SERDES_INFO(format, ...)
#endif

int32_t serdes_init(serdes_dev_t* dev)
{
    int32_t ret = -1;

    if (dev == NULL) {
        SERDES_ERR("parameter error for %s\n", __func__);
        return -1;
    }

    if (dev->is_init) {
        return 0;
    }

    if (dev->ops->init) {
        ret = dev->ops->init(dev);
    }

    if (ret == 0) {
        dev->is_init = true;
    }
    else {
        dev->is_init = false;
    }

    return ret;
}

int32_t serdes_deinit(serdes_dev_t* dev)
{
    int32_t ret = -1;

    if (dev != NULL &&
        dev->ops != NULL &&
        dev->ops->deinit != NULL) {
        ret = dev->ops->deinit(dev);
        dev->is_init = false;
        return ret;
    }

    return ret;
}

int32_t serdes_check_serdes_link(serdes_dev_t* dev)
{
    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->check_serdes_link != NULL) {
        return dev->ops->check_serdes_link(dev);
    }

    return -1;
}

int32_t serdes_control(serdes_dev_t* dev, int32_t ctlType, void* data, uint8_t* dataLen)
{
    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->device_control != NULL) {
        return dev->ops->device_control(dev, ctlType, data, dataLen);
    }

    return -1;
}

int32_t serdes_ser_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->ser_reg_read != NULL) {
        return dev->ops->ser_reg_read(dev, reg, regVal);
    }

    return -1;
}

int32_t serdes_ser_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->ser_reg_write != NULL) {
        return dev->ops->ser_reg_write(dev, reg, regVal);
    }

    return -1;
}

int32_t serdes_des_reg_read(serdes_dev_t* dev, uint32_t reg, uint32_t* regVal)
{

    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->des_reg_read != NULL) {
        return dev->ops->des_reg_read(dev, reg, regVal);
    }

    return -1;
}

int32_t serdes_des_reg_write(serdes_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev != NULL &&
        dev->is_init &&
        dev->ops != NULL &&
        dev->ops->des_reg_write != NULL) {
        return dev->ops->des_reg_write(dev, reg, regVal);
    }

    return -1;
}

/* end of file serdes.c */