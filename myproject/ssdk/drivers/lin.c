/**
 * @file lin.c
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
#include <driver/irq.h>
#include <types.h>

#include "driver/lin.h"

#define EXTRA_FMT "lin:"
#define LIN_ERR(format, ...)                                                   \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define LIN_WARN(format, ...)                                                  \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define LIN_INFO(format, ...)                                                  \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

#define LIN_SLEEP_PID 0x3c

#define LIN_MAX_TIMEOUT 10000000

static inline void lin_lock(lin_dev_t *device)
{
    if ((NULL != device) && (NULL != device->lock)) {
        osMutexAcquire(device->lock, osWaitForever);
    }
}

static inline void lin_unlock(lin_dev_t *device)
{
    if ((NULL != device) && (NULL != device->lock)) {
        osMutexRelease(device->lock);
    }
}

/**
 * @brief LIN send frame.
 */
uint8_t lin_send_frame(lin_dev_t *device, const lin_pdu_t *pdu)
{
    uint8_t ret = LIN_N_OK;

    if ((NULL == device) || (NULL == device->lock) || (NULL == pdu)) {
        return ret;
    }

    lin_lock(device);

    if (device->ch_status != LIN_CH_OPERATIONAL) {
        LIN_WARN("ch_state error:%d\n", device->ch_status);
        goto send_out;
    }

    ret = device->ops->send_frame(device, pdu);

send_out:
    lin_unlock(device);
    return ret;
}

/**
 * @brief LIN abort send frame.
 */
uint8_t lin_abort(lin_dev_t *device)
{
    uint8_t ret = LIN_N_OK;

    if ((NULL == device) || (NULL == device->lock)) {
        return ret;
    }

    lin_lock(device);

    if (device->ch_status != LIN_CH_OPERATIONAL) {
        LIN_WARN("ch_state error:%d\n", device->ch_status);
        goto abort_out;
    }

    ret = device->ops->abort(device);

abort_out:
    lin_unlock(device);
    return ret;
}

/**
 * @brief LIN check wakeup.
 */
uint8_t lin_check_wakeup(lin_dev_t *device) { return LIN_OK; }

/**
 * @brief LIN goto sleep.
 */
uint8_t lin_goto_sleep(lin_dev_t *device)
{
    if ((NULL == device) || (NULL == device->lock)) {
        return LIN_N_OK;
    }

    uint8_t data[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    lin_pdu_t pdu = {
        .pid = LIN_SLEEP_PID,
        .cs = LIN_CLASSIC_CS,
        .drc = LIN_MASTER_RESPONSE,
        .dl = 8,
        .sdu = data,
    };
    uint8_t ret = LIN_OK;

    lin_lock(device);

    if (device->ch_status == LIN_CH_SLEEP_STATE) {
        goto sleep_out;
    }

    ret = device->ops->send_frame(device, &pdu);
    if (ret == LIN_OK) {
        device->ch_status = LIN_CH_SLEEP_STATE;
    }

sleep_out:
    lin_unlock(device);
    return ret;
}

/**
 * @brief LIN goto sleep internal.
 */
uint8_t lin_goto_sleep_internal(lin_dev_t *device)
{
    if ((NULL == device) || (NULL == device->lock)) {
        return LIN_N_OK;
    }
    lin_lock(device);

    if (device->ch_status != LIN_CH_SLEEP_STATE) {
        device->ch_status = LIN_CH_SLEEP_STATE;
    }

    lin_unlock(device);

    return LIN_OK;
}

/**
 * @brief LIN wakeup.
 */
uint8_t lin_wakeup(lin_dev_t *device)
{
    uint8_t ret = LIN_N_OK;

    if ((NULL == device) || (NULL == device->lock)) {
        return ret;
    }

    lin_lock(device);

    if (device->ch_status != LIN_CH_SLEEP_STATE) {
        goto wakeup_out;
    }

    ret = device->ops->wake_up(device);
    if (ret == LIN_OK) {
        device->ch_status = LIN_CH_OPERATIONAL;
    }

wakeup_out:
    lin_unlock(device);
    return ret;
}

/**
 * @brief LIN wakeup internal.
 */
uint8_t lin_wakeup_internal(lin_dev_t *device)
{
    uint8_t ret = LIN_N_OK;

    if ((NULL == device) || (NULL == device->lock)) {
        return ret;
    }

    lin_lock(device);

    if (device->ch_status != LIN_CH_SLEEP_STATE) {
        goto wakeup_inter_out;
    }

    device->ch_status = LIN_CH_OPERATIONAL;
    ret = LIN_OK;

wakeup_inter_out:
    lin_unlock(device);
    return ret;
}

/**
 * @brief LIN get status.
 */
lin_status_t lin_get_status(lin_dev_t *device, uint8_t **sdu_ptr)
{
    lin_status_t ret;

    if ((NULL == device) || (NULL == device->lock)) {
        return LIN_N_OK;
    }

    lin_lock(device);

    if (device->ch_status == LIN_CH_SLEEP_STATE) {
        ret = LIN_CH_SLEEP;
    } else if (device->ch_status == LIN_CH_SLEEP_PENDING) {
        device->ch_status = LIN_CH_SLEEP_STATE;
        ret = LIN_CH_SLEEP;
    } else {
        ret = device->ops->get_status(device, sdu_ptr);
    }

    lin_unlock(device);

    return ret;
}

/**
 * @brief LIN init.
 */
void lin_init(lin_dev_t *device, const lin_config_t *config)
{
    if (NULL == device) {
        return;
    }
    device->lock = osMutexNew(NULL);
    ASSERT(device->lock);

    device->hwid = config->hwid;
    device->base = config->base;
    device->irq = config->irq;
    device->baud = config->baud;
    device->mode = config->mode;
    device->cs_mode = config->cs_mode;

    device->clk = config->clk;
    device->clk_freq = clk_get_rate(config->clk);
    device->timeout = LIN_MAX_TIMEOUT;

    device->ops->init(device);

    device->ch_status = LIN_CH_SLEEP_STATE;
}

/**
 * @brief LIN deinit.
 */
void lin_deinit(lin_dev_t *device)
{
    if (NULL != device) {
        device->ops->deinit(device);
    }

    if (NULL != device->lock) {
        osMutexDelete(device->lock);
        device->lock = NULL;
    }
}
