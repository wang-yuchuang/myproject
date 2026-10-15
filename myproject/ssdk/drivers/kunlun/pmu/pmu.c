/**
 * @file pmu.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmu common api implement
 *
 * Revision History:
 * -----------------
*/
#include <types.h>
#include <debug.h>
#include <cmsis_os2.h>

#include <driver/kunlun/pmu/pmu.h>


#define PMU_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "pmu:"

#if PMU_DEBUG_LVL >= SSDK_ERR
#define PMU_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PMU_ERR(format, ...)
#endif

#if PMU_DEBUG_LVL >= SSDK_WARNING
#define PMU_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PMU_WARN(format, ...)
#endif

#if PMU_DEBUG_LVL >= SSDK_INFO
#define PMU_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define PMU_INFO(format, ...)
#endif


int32_t pmu_init(pmu_dev_t* dev)
{
    if (dev == NULL) {
        PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    if (dev->is_init) {
        PMU_INFO("pmu device is already init\n");
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

int32_t pmu_deinit(pmu_dev_t* dev)
{
    if (dev == NULL) {
        PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    dev->is_init = false;

    return 0;
}

int32_t pmu_get_status(pmu_dev_t* dev, int32_t* sta)
{
    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->get_status) {
        return dev->ops->get_status(dev, sta);
    }

    return -1;
}

int32_t pmu_change_state(pmu_dev_t* dev, int32_t sta)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->change_state) {
        ret = dev->ops->change_state(dev, sta);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pmu_get_event_source(pmu_dev_t* dev, uint32_t* allEvtSrc)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->get_event_source) {
        ret = dev->ops->get_event_source(dev, allEvtSrc);
    }

    return ret;
}

int32_t pmu_clear_event_source(pmu_dev_t* dev, uint32_t multiEvt)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->clear_event_source) {
        ret = dev->ops->clear_event_source(dev, multiEvt);
    }

    return ret;
}

int32_t pmu_get_triger_status(pmu_dev_t* dev, int32_t type, int32_t* sta)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->get_triger_status) {
        ret = dev->ops->get_triger_status(dev, type, sta);
    }

    return ret;
}

int32_t pmu_set_pwr_out_status(pmu_dev_t* dev, int32_t pinId, int32_t lev)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->set_pwr_out_status) {
        ret = dev->ops->set_pwr_out_status(dev, pinId, lev);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

int32_t pmu_get_pwr_input_status(pmu_dev_t* dev, int32_t pinId, int32_t* lev)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    if (dev->ops->get_pwr_input_status) {
        ret = dev->ops->get_pwr_input_status(dev, pinId, lev);
    }

    return ret;
}

int32_t pmu_set_por_force_pwrdown(pmu_dev_t* dev, int32_t enable)
{
    int32_t ret = -1;

    if (dev == NULL || !dev->is_init) {
        PMU_ERR("%s error by invalid device.\n", __FUNCTION__);
        return -1;
    }

    osMutexAcquire(dev->mutex, osWaitForever);
    if (dev->ops->set_por_force_pwrdown) {
        ret = dev->ops->set_por_force_pwrdown(dev, enable);
    }
    osMutexRelease(dev->mutex);

    return ret;
}

/* end of file pmu.c */