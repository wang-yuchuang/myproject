/**
 * @file: epwm.c

 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion: pwm hld common driver
 *
 * @Revision History:
 * ----------------
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include "driver/pwm.h"

#include "cmsis_os2.h"
#include "debug.h"
// #include "driver/taishan/epwm/sdrv_epwm_drv.h"
#include "reg.h"

/**
 * @brief pwm configure
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 *
 */
void pwm_config(pwm_dev_t *dev, const pwm_state_t *state)
{
    ASSERT(dev != NULL || state != NULL);

    if (dev->ops->config)
        dev->ops->config(dev, state);
}

/**
 * @brief pwm start
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_start(pwm_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->start)
        dev->ops->start(dev);
}

/**
 * @brief pwm stop
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_stop(pwm_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->stop)
        dev->ops->stop(dev);
}

/**
 * @brief pwm callback
 *
 * @param[in] dev pwm common instance
 * @param[in] cb pwm callback
 * @param[in] arg  callback input parameter
 *
 */
void pwm_set_cb(pwm_dev_t *dev, pwm_callback_t cb, void *arg)
{
    ASSERT(dev != NULL);
    dev->cb = cb;
    dev->arg = arg;
}

/**
 * @brief pwm expiry
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_expiry(pwm_dev_t *dev)
{
    ASSERT(dev != NULL);
    if (dev->cb) {
        dev->cb(dev->arg);
    }
}

/**
 * @brief pwm pwm_capture
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm period
 * @param[in] timeout over time
 * @return  0
 *
 */
int pwm_capture(pwm_dev_t *dev, pwm_state_t *state)
{

    ASSERT(dev != NULL || state != NULL);

    if (dev->ops->capture) {
        dev->ops->capture(dev, state);
    }
    return 0;
}


