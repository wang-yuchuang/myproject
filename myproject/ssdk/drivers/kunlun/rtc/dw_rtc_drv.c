/**
 * @file dw_rtc_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dw_rtc_drv.c driver.
 *
 * Revision History:
 * -----------------
 */
#include <cmsis_os2.h>
#include <debug.h>
#include <types.h>
#include "driver/irq.h"
#include "driver/kunlun/rtc/dw_rtc_drv.h"
#include "dw_rtc.h"
#include <board.h>


#define RTC_CLK_HZ (32 * 1024ul)
#define RTC_CLK_SHIFT __builtin_ctz(RTC_CLK_HZ)
#define RTC_TICK_TO_SECOND(t) (((uint64_t)t) >> RTC_CLK_SHIFT)
#define RTC_SECOND_TO_TICK(s) (((uint64_t)s) << RTC_CLK_SHIFT)
#define RTC_MSECOND_TO_TICK(s) (((uint64_t)s/1000) << RTC_CLK_SHIFT)
#define RTC_TICK_TO_MSECOND(t) ((((uint64_t)t) * 1000U) >> RTC_CLK_SHIFT)
#define RTC_TICK_TO_USECOND(t) ((((uint64_t)t) * 1000000U) >> RTC_CLK_SHIFT)

/**
 * @brief apb bus clock wait for rtc bus clock /ms
 *
 * @param t
 */
static void rtc_sync_apb_clock(const uint32_t t)
{
    // osDelay(t);
    udelay(t);
}

/**
 * @brief rtc get time
 *
 * @param dev
 * @param tm
 * @return int
 */
static int dw_rtc_get_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    uint64_t secs = RTC_TICK_TO_SECOND(dw_rtc_lld_get_tick(dev->cfg->base));
    epoch_to_rtc(tm, secs);

    return 0;
}

static void dw_rtc_get_time_us(rtc_dev_t *dev, uint64_t *us) {
    *us = RTC_TICK_TO_USECOND(dw_rtc_lld_get_tick(dev->cfg->base));
}

static int dw_rtc_set_time_ms(rtc_dev_t *dev, uint64_t ms) {
    int ret;

    if (ms == 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc set sec fiald\n", __func__);
        return -1;
    }

    ret = dw_rtc_lld_disable(dev->cfg->base);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc lld disable fiald\n", __func__);
        return -1;
    }

    rtc_sync_apb_clock(300);
    dw_rtc_lld_set_tick(dev->cfg->base, RTC_MSECOND_TO_TICK(ms));
    ret = dw_rtc_lld_enable(dev->cfg->base);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc lld enable fiald\n", __func__);
        return -1;
    }

    return 0;

}

/**
 * @brief rtc set time
 *
 * @param dev
 * @param tm
 * @return int
 */
static int dw_rtc_set_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    uint64_t secs = rtc_to_epoch(tm);
    int ret;

    if (secs == 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc set sec fiald\n", __func__);
        return -1;
    }

    ret = dw_rtc_lld_disable(dev->cfg->base);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc lld disable fiald\n", __func__);
        return -1;
    }

    rtc_sync_apb_clock(300);
    dw_rtc_lld_set_tick(dev->cfg->base, RTC_SECOND_TO_TICK(secs));
    ret = dw_rtc_lld_enable(dev->cfg->base);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc lld enable fiald\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * @brief rtc get alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
static int dw_rtc_get_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm)
{
    uint64_t secs =
        RTC_TICK_TO_SECOND(dw_rtc_lld_get_alarm_tick(dev->cfg->base));

    epoch_to_rtc(&alrm->tm, secs);
    alrm->enable = dw_rtc_lld_get_alarm_enable(dev->cfg->base);
    alrm->pending = dw_rtc_lld_get_alarm_status(dev->cfg->base);

    return 0;
}

/**
 * @brief rtc set alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
static int dw_rtc_set_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm)
{
    uint64_t secs = rtc_to_epoch(&alrm->tm);
    int ret;

    if (secs == 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc set sec fiald\n", __func__);
        return -1;
    }

    ret = dw_rtc_lld_alarm_compare_value(dev->cfg->base,
                                         RTC_SECOND_TO_TICK(secs));

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc set compare value fiald\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * @brief rtc enable alarm
 *
 * @param dev
 * @param enable
 * @return int
 */
static int dw_rtc_enable_alarm(rtc_dev_t *dev, bool enable)
{
    int ret = dw_rtc_lld_set_alarm_enable(dev->cfg->base, enable);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s dw rtc set alarm enable fiald\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * @brief rtc enable
 *
 * @param dev
 * @param enable
 * @return int
 */
static int dw_rtc_enable(rtc_dev_t *dev, bool enable)
{
    if (enable) {
        if (dw_rtc_lld_enable(dev->cfg->base) < 0)
            return -1;
    } else {
        if (dw_rtc_lld_disable(dev->cfg->base) < 0)
            return -1;
    }

    return 0;
}

/**
 * @brief dw rtc irq handler
 *
 * @param irq
 * @param arg
 * @return int
 */
static int dw_rtc_irq_handler(uint32_t irq, void *arg)
{
    rtc_dev_t *dev = arg;

    if (dev->rtc_cb) {
        return dev->rtc_cb(irq, arg);
    }

    return -1;
}

/**
 * @brief rtc init
 *
 * @param dev
 */
static void dw_rtc_init(rtc_dev_t *dev)
{
    int rtc_irq = dev->cfg->irq;

    if(true == dw_rtc_lld_get_alarm_status(dev->cfg->base)) {
        dw_rtc_lld_set_alarm_enable(dev->cfg->base, false);
    }

    if (rtc_irq >= 0) {
        irq_attach(rtc_irq, dw_rtc_irq_handler, dev);
        irq_enable(rtc_irq);
    }
}

/**
 * @brief rtc deinit
 *
 * @param dev
 */
static void dw_rtc_deinit(rtc_dev_t *dev)
{
    int rtc_irq = dev->cfg->irq;

    if (rtc_irq >= 0) {
        irq_disable(rtc_irq);
        irq_detach(rtc_irq);
    }
}

static const rtc_ops_t dw_rtc_ops = {
    .get_time = dw_rtc_get_time,
    .set_time = dw_rtc_set_time,
    .get_alarm = dw_rtc_get_alarm,
    .set_alarm = dw_rtc_set_alarm,
    .enable_alarm = dw_rtc_enable_alarm,
    .enable = dw_rtc_enable,
    .init = dw_rtc_init,
    .deinit = dw_rtc_deinit,
    .get_time_us = dw_rtc_get_time_us,
    .set_time_ms = dw_rtc_set_time_ms,
};

/**
 * @brief get rtc config
 *
 * @param dev
 * @param cfg
 * @return rtc_dev_t*
 */
rtc_dev_t *dw_rtc_probe(rtc_dev_t *dev, const rtc_config_t *cfg)
{
    dev->ops = &dw_rtc_ops;
    dev->cfg = cfg;

    return dev;
}
