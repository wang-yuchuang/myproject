/**
 * @file pvt.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt common api implement
 *
 * Revision History:
 * -----------------
*/
#include <types.h>
#include <debug.h>
#include <cmsis_os2.h>

#include <driver/kunlun/pvt/pvt.h>
#include <driver/kunlun/pvt/sdrv_pvt_drv.h>


#define PVT_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "pvt:"

#if PVT_DEBUG_LVL >= SSDK_ERR
#define PVT_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PVT_ERR(format, ...)
#endif

#if PVT_DEBUG_LVL >= SSDK_WARNING
#define PVT_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PVT_WARN(format, ...)
#endif

#if PVT_DEBUG_LVL >= SSDK_INFO
#define PVT_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PVT_INFO(format, ...)
#endif

int32_t pvt_init(pvt_dev_t* dev)
{
    if (dev == NULL) {
        PVT_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    if (dev->is_init) {
        PVT_INFO("pvt device is already init\n");
        return 0;
    }

    dev->mutex = osMutexNew(NULL);

    if (dev->ops->init) {
        dev->ops->init(dev);
        dev->is_init = true;
        return 0;
    }
    else {
        // do nothing
        return -1;
    }
}

int32_t pvt_deinit(pvt_dev_t* dev)
{
    if (dev == NULL) {
        PVT_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->deinit) {
        dev->ops->deinit(dev);
    }
    dev->is_init = false;

    return 0;
}

int32_t pvt_set_alarm(pvt_dev_t* dev
                      , pvt_alarm_mode_enum_t mode
                      , pvt_alarm_type_enum_t type
                      , const pvt_alarm_para_t* para)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->set_alarm) {
        ret = dev->ops->set_alarm(dev, mode, type, para);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_clr_alarm(pvt_dev_t* dev, int32_t chanId)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->clr_alarm) {
        ret = dev->ops->clr_alarm(dev, chanId);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_enable_alarm(pvt_dev_t* dev, int32_t chanId)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->enable_alarm) {
        ret = dev->ops->enable_alarm(dev, chanId);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_disable_alarm(pvt_dev_t* dev, int32_t chanId)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->disable_alarm) {
        ret = dev->ops->disable_alarm(dev, chanId);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_get_pvt(pvt_dev_t* dev, pvt_data_t* pvtData)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->get_pvt) {
        ret = dev->ops->get_pvt(dev, pvtData);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_integer_data_get(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t chanId, int32_t* data)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->integer_data_get) {
        ret = dev->ops->integer_data_get(dev, type, chanId, data);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_float_data_get(pvt_dev_t* dev, pvt_float_get_enum_t type, int32_t chanId, float* data)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->float_data_get) {
        ret = dev->ops->float_data_get(dev, type, chanId, data);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pvt_integer_data_set(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t data)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PVT_ERR("%s parameter error or device is not init.\n", __FUNCTION__);
        return ret;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->integer_data_set) {
        ret = dev->ops->integer_data_set(dev, type, data);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

/* end of file pvt.c */