/**
 * @file touch.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: touch common api implement file
 *
 * Revision History:
 * -----------------
*/
#include <types.h>
#include <debug.h>

#include <driver/kunlun/touch/touch.h>

/** function implement section */
int32_t touch_init(touch_dev_t* dev)
{
    int32_t ret = -1;
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init) {
        return 0;
    }

    if (dev->ops && dev->ops->init) {
        ret = dev->ops->init(dev);
        dev->is_init = true;
    }

    return ret;
}

int32_t touch_deinit(touch_dev_t* dev)
{
    int32_t ret = -1;

    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->ops != NULL && dev->ops->deinit != NULL) {
        ret = dev->ops->deinit(dev);
        dev->is_init = false;
        return ret;
    }

    return -1;
}

int32_t touch_online_check(touch_dev_t* dev)
{
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init &&
        dev->ops != NULL &&
        dev->ops->online_check != NULL) {
        return dev->ops->online_check(dev);
    }

    return -1;
}

int32_t touch_register_callback(touch_dev_t* dev, touch_event_notice cb, void* usrCtx)
{
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init &&
        dev->ops != NULL &&
        dev->ops->register_callback != NULL) {
        return dev->ops->register_callback(dev, cb, usrCtx);
    }

    return -1;
}

int32_t touch_device_control(touch_dev_t* dev, touch_ctrl_type_enum_t type, void* data, int32_t* len)
{
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init &&
        dev->ops != NULL &&
        dev->ops->device_control != NULL) {
        return dev->ops->device_control(dev, type, data, len);
    }

    return -1;
}

int32_t touch_reg_read(touch_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init &&
        dev->ops != NULL &&
        dev->ops->reg_read != NULL) {
        return dev->ops->reg_read(dev, reg, regVal);
    }

    return -1;
}

int32_t touch_reg_write(touch_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    if (dev->is_init &&
        dev->ops != NULL &&
        dev->ops->reg_write != NULL) {
        return dev->ops->reg_write(dev, reg, regVal);
    }

    return -1;
}

/* end of file touch.c */