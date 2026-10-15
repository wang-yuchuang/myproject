/**
 * @file sec_rtcd.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sec_rtcd.c driver.
 *
 * Revision History:
 * -----------------
 */
#include <cmsis_os2.h>
#include <debug.h>
#include <irq_num.h>
#include <types.h>

#include "driver/taishan/rtc/sec_rtcd.h"

#include "driver/irq.h"
#include "driver/rtc.h"
#include "driver/arch_timer.h"
#include "sec_rtc.h"

#define RTC_CLK_HZ (32 * 1024ul)
#define RTC_CLK_SHIFT __builtin_ctz(RTC_CLK_HZ)
#define RTC_TICK_TO_SECOND(t) ((t) >> RTC_CLK_SHIFT)
#define RTC_SECOND_TO_TICK(s) ((s) << RTC_CLK_SHIFT)

#define DELAY_US    240

typedef void (*rtc_callback_t)(uint32_t value, void *user_arg);
static rtc_callback_t _rtc_callback;
static void *_rtc_callback_arg;

uint32_t err = 0;

/*workaround for rewritting RTC_L/H,extra waitting is 480us*/
static bool bPowerOn = false;
static int _rtc_handler(uint32_t source, void *arg)
{
    rtc_dev_t *dev = arg;
    Rtc *rtc = (Rtc *)dev->cfg->base;
    if (dev->rtc_cb) {
        dev->rtc_cb(source, arg);
    }
    if ((source == RTC1_VIOLATION_INTR_NUM) ||
        (source == RTC2_VIOLATION_INTR_NUM)) { // disable 1c,overflow
        if (rtc->VIOLATION_INT & FV_VIOLATION_INT_MASK(1)) { // overflow
            err |= 4;
            if (rtc->VIOLATION_INT & BM_VIOLATION_INT_OVERFLOW_STATS) {
                err |= 1;
                rtc->VIOLATION_INT = FV_VIOLATION_INT_MASK(1);
            }

        } else { // disable
            err |= 8;
            if (rtc->VIOLATION_INT & BM_VIOLATION_INT_DISABLE_STATUS) {
                err |= 2;
                rtc->VIOLATION_INT = BM_VIOLATION_INT_DISABLE_STATUS;
                rtc->WAKEUP_CTRL |= BM_WAKEUP_CTRL_DISABLE_CLEAR;
            }
        }

        sec_rtc_set_cross_clk_en(rtc, NULL);

        sec_rtc_set_rtc_ctrl(rtc, NULL, 9, 0);

    } else if (source == RTC1_RTC_WAKEUP_INTR_NUM ||
               (source == RTC2_RTC_WAKEUP_INTR_NUM)) {
        sec_rtc_wakeup_enable(rtc, 7, false);
        sec_rtc_clear_alarm_status(rtc);
        sec_rtc_set_cross_clk_en(rtc, NULL);
    } else if (source == RTC1_RTC_PERIODICAL_INTR_NUM ||
               (source == RTC2_RTC_PERIODICAL_INTR_NUM)) {
        /*TBD*/
    }

    return 0;
}
void sec_rtcd_enable_it(uint32_t source, void *arg, rtc_callback_t cb)
{
    _rtc_callback = cb;
    _rtc_callback_arg = arg;
    irq_attach(source, _rtc_handler, arg);
    irq_enable(source);
}
void sec_rtcd_disable_it(uint32_t source)
{
    irq_detach(source);
    irq_disable(source);
}
/**
 * @brief apb bus clock wait for rtc bus clock /ms
 *
 * @param t
 */
static void rtc_sync_apb_clock(const uint32_t t) { osDelay(t); }

/**
 * @brief rtc get time
 *
 * @param dev
 * @param tm
 * @return int
 */
static int sec_rtcd_get_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    uint64_t secs = RTC_TICK_TO_SECOND(sec_rtc_get_tick(rtc));
    epoch_to_rtc(tm, secs);

    return 0;
}

/**
 * @brief rtc set time
 *
 * @param dev
 * @param tm
 * @return int
 */
static int sec_rtcd_set_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    uint64_t secs = rtc_to_epoch(tm);
    int ret;

    if (secs == 0) {
        ssdk_printf(SSDK_ERR, "%s sec rtc set sec failed\n", __func__);
        return -1;
    }
    Rtc *rtc = (Rtc *)dev->cfg->base;
    ret = sec_rtc_local_enable(rtc, 0);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s sec rtc lld disable failed\n", __func__);
        return -1;
    }
    if(bPowerOn == true)
        arch_udelay(DELAY_US);
    sec_rtc_set_tick(rtc, RTC_SECOND_TO_TICK(secs));
    sec_rtc_set_cross_clk_en(rtc, NULL);
    ret = sec_rtc_local_enable(rtc, 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s sec rtc lld enable fiald\n", __func__);
        return -1;
    }
    if(bPowerOn == true)
        arch_udelay(DELAY_US);
    bPowerOn = true;

    return 0;
}

/**
 * @brief rtc get alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
static int sec_rtcd_get_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    uint64_t secs = RTC_TICK_TO_SECOND(sec_rtc_get_alarm_tick(rtc));

    epoch_to_rtc(&alrm->tm, secs);
    alrm->enable = sec_rtc_is_alarm_enable(rtc);
    alrm->pending = sec_rtc_get_wakeup_status(rtc);

    return 0;
}

/**
 * @brief rtc set alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
static int sec_rtcd_set_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm)
{
    uint64_t secs = rtc_to_epoch(&alrm->tm);

    if (secs == 0) {
        ssdk_printf(SSDK_ERR, "%s sec rtc set sec fiald\n", __func__);
        return -1;
    }
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_set_alarm_tick(rtc, RTC_SECOND_TO_TICK(secs));
    sec_rtc_set_cross_clk_en(rtc, NULL);
    return 0;
}

/**
 * @brief rtc enable alarm
 *
 * @param dev
 * @param enable
 * @return int
 */
static int sec_rtcd_enable_alarm(rtc_dev_t *dev, bool enable)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_wakeup_enable(rtc, 7, enable);
    if (!enable) {
        sec_rtc_clear_alarm_status(rtc);
    }

    return 0;
}
static void sec_rtcd_local_enable(rtc_dev_t *dev, uint8_t en)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_local_enable(rtc, en);
}
static void sec_rtcd_set_ctrl(rtc_dev_t *dev1, rtc_dev_t *dev2, uint32_t ctrl1,
                              uint32_t ctrl2)
{
    Rtc *rtc1 = (Rtc *)dev1->cfg->base;
    Rtc *rtc2 = NULL;
    if (dev2)
        rtc2 = (Rtc *)dev2->cfg->base;
    sec_rtc_set_rtc_ctrl(rtc1, rtc2, ctrl1, ctrl2);
}

static void sec_rtcd_ms_cfg(rtc_dev_t *master, rtc_dev_t *slave, uint8_t m_ctrl,
                            uint8_t s_ctrl)
{
    Rtc *rtc_s = NULL;
    Rtc *rtc_m = (Rtc *)master->cfg->base;
    if (slave)
        rtc_s = (Rtc *)slave->cfg->base;
    sec_rtc_ms_cfg(rtc_m, rtc_s, m_ctrl, s_ctrl);
}
static void sec_rtcd_cross_clk_en(rtc_dev_t *dev1, rtc_dev_t *dev2)
{
    Rtc *rtc1 = (Rtc *)dev1->cfg->base;
    Rtc *rtc2 = (Rtc *)dev2->cfg->base;
    sec_rtc_set_cross_clk_en(rtc1, rtc2);
}

static uint64_t sec_rtcd_get_hld_tick(rtc_dev_t *dev, uint64_t *tick)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_hld_tick(rtc, tick);
}

static void sec_rtcd_auto_adjust(rtc_dev_t *dev, uint32_t ctrl)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_auto_adjust(rtc, ctrl);
}

static uint64_t sec_rtcd_get_tick(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_tick(rtc);
}

static void sec_rtcd_set_tick(rtc_dev_t *dev, uint64_t tick)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_set_tick(rtc, tick);
}
static void sec_rtcd_set_periodical(rtc_dev_t *dev, uint8_t freq, uint8_t en)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_set_periodical(rtc, freq, en);
}
static uint8_t sec_rtcd_get_periodical_freq(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_periodical_freq(rtc);
}

static uint8_t sec_rtcd_get_violation_status(rtc_dev_t *dev, uint8_t vio)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_violation_status(rtc, vio);
}
static void sec_rtcd_set_voilation(rtc_dev_t *dev, uint32_t ctrl)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_set_violation(rtc, ctrl);
}

static uint8_t sec_rtcd_get_wakeup_status(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_wakeup_status(rtc);
}
static void sec_rtcd_clear_alarm(rtc_dev_t *dev, uint8_t type)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_clear_alarm_status(rtc);
}
static void sec_rtcd_wakeup_enable(rtc_dev_t *dev, uint8_t type, uint8_t en)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_wakeup_enable(rtc, type, en);
}
static void sec_rtcd_fusa_enable(rtc_dev_t *dev1, rtc_dev_t *dev2, uint32_t en)
{
    Rtc *rtc2 = NULL;
    Rtc *rtc1 = (Rtc *)dev1->cfg->base;
    if (dev2)
        rtc2 = (Rtc *)dev2->cfg->base;
    sec_rtc_fusa_enable(rtc1, rtc2, en);
}
static void sec_rtcd_err_inj(rtc_dev_t *dev, rtc_ej_t ej, uint32_t val)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_ej(rtc, ej, val);
}
static uint32_t sec_rtcd_get_parity_status(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_parity_status(rtc);
}
static void sec_rtcd_clear_parity_status(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_clear_parity_status(rtc);
}
static void sec_rtcd_apb_monitor_ej_clear(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_apb_monitor_ej_clear(rtc);
}
static uint32_t sec_rtcd_get_errirq_status(rtc_dev_t *dev, uint8_t err_type)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    return sec_rtc_get_errirq_status(rtc, err_type);
}
static void sec_rtcd_clear_errirq_status(rtc_dev_t *dev)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_clear_errirq_status(rtc);
}

static void sec_rtcd_err_sig_enable(rtc_dev_t *dev, uint8_t sig, uint32_t en)
{
    Rtc *rtc = (Rtc *)dev->cfg->base;
    sec_rtc_err_sig_enable(rtc, sig, en);
}

/**
 * @brief rtc init
 *
 * @param dev
 */
static void sec_rtcd_init(rtc_dev_t *dev)
{
    int source = dev->cfg->irq;
    if (source >= 0) {
        irq_attach(source, _rtc_handler, dev);
        irq_enable(source);
    }
    return;
}

/**
 * @brief rtc deinit
 *
 * @param dev
 */
static void sec_rtcd_deinit(rtc_dev_t *dev)
{
    int source = dev->cfg->irq;

    if (source >= 0) {
        irq_disable(source);
        irq_detach(source);
    }
    return;
}

static const rtc_ops_t sec_rtc_ops = {
    .get_time = sec_rtcd_get_time,
    .set_time = sec_rtcd_set_time,
    .get_alarm = sec_rtcd_get_alarm,
    .set_alarm = sec_rtcd_set_alarm,
    .enable_alarm = sec_rtcd_enable_alarm,
    .init = sec_rtcd_init,
    .deinit = sec_rtcd_deinit,
};

/**
 * @brief get rtc config
 *
 * @param dev
 * @param cfg
 * @return rtc_dev_t*
 */
rtc_dev_t *sec_rtc_probe(rtc_dev_t *dev, const rtc_config_t *cfg)
{
    dev->ops = &sec_rtc_ops;
    dev->cfg = cfg;

    return dev;
}
