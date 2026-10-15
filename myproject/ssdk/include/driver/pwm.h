/**
 * @file: epwm.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 *
 * @Revision History:
 * ----------------
 */

#ifndef INCLUDE_DRV_PWM_H
#define INCLUDE_DRV_PWM_H

#include <cmsis_os2.h>
#include <compiler.h>
#include <spinlock.h>
#include <types.h>

#ifndef ASSEMBLY

/**
 * @brief pwm_polarity.
 *
 */
typedef enum pwm_polarity {
    PWM_POLARITY_NORMAL,
    PWM_POLARITY_INVERSED,
} pwm_polarity_e;

/**
 * @brief pwm_status.
 *
 */
typedef struct pwm_status {
    uint32_t period; /* PWM period in nano seconds. */
    uint32_t duty;   /* PWM duty cycle in nano seconds. */
} pwm_state_t;

typedef void (*pwm_callback_t)(void *arg);

typedef struct pwm_dev pwm_dev_t;

typedef struct pwm_ops {
    /* Start PWM output. */
    void (*start)(pwm_dev_t *dev);

    /* Stop PWM output. */
    void (*stop)(pwm_dev_t *dev);

    /* Configure PWM output state. This function configures
     * PWM output parameters, but doesn't start or stop it.
     */
    void (*config)(pwm_dev_t *dev, const pwm_state_t *state);

    /* Capture PWM input state. */
    int (*capture)(pwm_dev_t *dev, pwm_state_t *state);

} pwm_ops_t;

typedef struct pwm_dev {
    pwm_polarity_e polarity;
    pwm_state_t state;
    pwm_ops_t *ops;
    pwm_callback_t cb;
    void *arg;

    /* Device specific data. */
    void *priv;
    uint16_t clk_div;
} pwm_dev_t;

/**
 * @brief pwm configure
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm duty and period
 *
 */
void pwm_config(pwm_dev_t *dev, const pwm_state_t *state);

/**
 * @brief pwm start
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_start(pwm_dev_t *dev);

/**
 * @brief pwm stop
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_stop(pwm_dev_t *dev);

/**
 * @brief pwm callback
 *
 * @param[in] dev pwm common instance
 * @param[in] cb pwm callback
 * @param[in] arg  callback input parameter
 *
 */
void pwm_set_cb(pwm_dev_t *dev, pwm_callback_t cb, void *arg);

/**
 * @brief pwm expiry
 *
 * @param[in] dev pwm common instance
 *
 */
void pwm_expiry(pwm_dev_t *dev);

/**
 * @brief pwm pwm_capture
 *
 * @param[in] dev pwm common instance
 * @param[in] state pwm period
 * @param[in] timeout over time
 * @return  0
 *
 */
int pwm_capture(pwm_dev_t *dev, pwm_state_t *state);

#endif

#endif
