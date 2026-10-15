/*
 * rtc_ip.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: RTC low level driver.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>
#include <stdint.h>
#include <types.h>

#include "sec_rtc.h"

#include "driver/taishan/rtc/sec_rtcd.h"
#include "sec_rtc_reg.h"

#define RTC_WRITE_SYNC_CLK 7

static void _delay(uint32_t count)
{
    uint32_t cnt = count * 3;
    for (int i = 0; i < cnt; i++)
        ;
}
void _rtc_clk_sync(Rtc *rtc, uint32_t clk)
{
    uint64_t toclk = sec_rtc_get_tick(rtc) + clk;
    while (sec_rtc_get_tick(rtc) <= toclk)
        ;
}
uint64_t sec_rtc_get_tick(Rtc *rtc)
{
    uint64_t tick = (uint64_t)(rtc->RTC_H) << 32 | (rtc->RTC_L);
    return tick;
}

void sec_rtc_set_tick(Rtc *rtc, uint64_t v)
{
    uint32_t tick = v & 0xffffffff;
    rtc->RTC_L = tick;
    rtc->RTC_H = (v >> 32) & 0xffff;
}

void sec_rtc_lock(Rtc *rtc, uint32_t lock)
{
    if (lock)
        rtc->SEC_RTC_CTRL |= BM_SEC_RTC_CTRL_LOCK;
    else
        rtc->SEC_RTC_CTRL &= ~BM_SEC_RTC_CTRL_LOCK;
}

bool sec_rtc_is_locked(Rtc *rtc)
{
    bool locked = false;
    if (rtc->SEC_RTC_CTRL & BM_SEC_RTC_CTRL_LOCK)
        locked = true;
    return locked;
}

int sec_rtc_local_enable(Rtc *rtc, uint32_t en)
{
    if (!sec_rtc_is_locked(rtc)) {
        if (en)
            rtc->SEC_RTC_CTRL |= BM_SEC_RTC_CTRL_RTC_LOCAL_ENABLE;
        else
            rtc->SEC_RTC_CTRL &= ~BM_SEC_RTC_CTRL_RTC_LOCAL_ENABLE;
        return 1;
    }
    return 0;
}

void sec_rtc_set_alarm_tick(Rtc *rtc, uint64_t val)
{
    rtc->TIMER_L = (uint32_t)(val & 0xffffffff);
    rtc->TIMER_H = (val >> 32) & 0xffff;
}

uint64_t sec_rtc_get_alarm_tick(Rtc *rtc)
{
    uint64_t tick = (uint64_t)(rtc->TIMER_H) << 32 | (rtc->TIMER_L);
    return tick;
}

bool sec_rtc_get_alarm_status(Rtc *rtc)
{
    bool balarm = false;
    if (rtc->WAKEUP_CTRL & BM_WAKEUP_CTRL_STATUS) {
        balarm = true;
    }
    return balarm;
}
uint8_t sec_rtc_is_alarm_enable(Rtc *rtc)
{
    uint32_t val = rtc->WAKEUP_CTRL;
    return (val | BM_WAKEUP_CTRL_ENABLE | BM_WAKEUP_CTRL_IRQ_ENABLE |
            BM_WAKEUP_CTRL_REQ_ENABLE);
}

void sec_rtc_clear_alarm_status(Rtc *rtc)
{
    rtc->WAKEUP_CTRL |= BM_WAKEUP_CTRL_CLEAR;
    _rtc_clk_sync(rtc, RTC_WRITE_SYNC_CLK);

    // 1c?
    rtc->WAKEUP_CTRL &= ~BM_WAKEUP_CTRL_CLEAR;
    _rtc_clk_sync(rtc, RTC_WRITE_SYNC_CLK);
}

uint64_t sec_rtc_get_hld_tick(Rtc *rtc, uint64_t *tick)
{
    uint32_t ctrl = rtc->SEC_RTC_CTRL;
    if (ctrl &
        (BM_SEC_RTC_CTRL_SECURE_ENABLE | BM_SEC_RTC_CTRL_PRIVILEGE_ENABLE))
        printf("check access permission!");
    if (tick)
        *tick = sec_rtc_get_tick(rtc);
    uint64_t tick_hld =
        (uint64_t)(rtc->RTC_H_HOLD_SHADOW) << 32 | (rtc->RTC_L_HOLD_SHADOW);
    return tick_hld;
}

void sec_rtc_set_rtc_ctrl(Rtc *rtc1, Rtc *rtc2, uint32_t ctrl1, uint32_t ctrl2)
{
    rtc1->SEC_RTC_CTRL = ctrl1;
    if (rtc2)
        rtc2->SEC_RTC_CTRL = ctrl2;
}
void sec_rtc_set_violation(Rtc *rtc1, uint32_t rtc1v)
{
    rtc1->VIOLATION_INT = rtc1v;
}
void sec_rtc_set_cross_clk_en(Rtc *rtc1, Rtc *rtc2)
{
    rtc1->RTC_REGISTER_CROSS_CLOCK |=
        BM_RTC_REGISTER_CROSS_CLOCK_REGISTER_CROSS_CLOCK_EN;

    if (rtc2)
        rtc2->RTC_REGISTER_CROSS_CLOCK |=
            BM_RTC_REGISTER_CROSS_CLOCK_REGISTER_CROSS_CLOCK_EN;
    while (rtc1->RTC_REGISTER_CROSS_CLOCK &
           BM_RTC_REGISTER_CROSS_CLOCK_REGISTER_CROSS_CLOCK_EN)
        _delay(1);
}

void sec_rtc_setup_wakeup_ctrl1(Rtc *rtc1, Rtc *rtc2, uint32_t v1, uint32_t v2)
{
    rtc1->WAKEUP_CTRL = v1;
    if (rtc2)
        rtc2->WAKEUP_CTRL = v2;
    sec_rtc_set_cross_clk_en(rtc1, rtc2);
}
void sec_rtc_wakeup_enable(Rtc *rtc, uint8_t type, uint8_t en)
{
    uint32_t v = rtc->WAKEUP_CTRL;
    uint32_t vtype = 0;
    switch (type) {
    case WKUP_ENABLE:
        vtype = BM_WAKEUP_CTRL_ENABLE;
        break;
    case WKUP_IRQ:
        vtype = BM_WAKEUP_CTRL_IRQ_ENABLE;
        break;
    case WKUP_REQ:
        vtype = BM_WAKEUP_CTRL_REQ_ENABLE;
        break;
    default:
        vtype = type & 0x7;
        break;
    }
    if (en)
        v |= vtype;
    else
        v &= ~vtype;
    rtc->WAKEUP_CTRL = v;
}
void sec_rtc_clear_wakeup(Rtc *rtc, uint8_t type)
{
    uint32_t v = rtc->WAKEUP_CTRL;
    switch (type) {
    case WKUP_CLEAR:
        v |= BM_WAKEUP_CTRL_CLEAR;
        break;
    case WKUP_OV_CLR:
        v |= BM_WAKEUP_CTRL_OVERFLOW_CLEAR;
        break;
    case WKUP_DISABLE_CLR:
        v |= BM_WAKEUP_CTRL_DISABLE_CLEAR;
        break;
    deault:
        v |= (type & 0x7) << 4;
        break;
    }
    rtc->WAKEUP_CTRL |= v;
}
uint8_t sec_rtc_get_wakeup_status(Rtc *rtc)
{
    uint8_t ret = 0;
    if (rtc->WAKEUP_CTRL & BM_WAKEUP_CTRL_STATUS)
        ret = 1;
    return ret;
}

void sec_rtc_ms_cfg(Rtc *master, Rtc *slave, uint8_t ctmode1, uint8_t ctmode2)
{
    master->RTC_WORKING_MODE = BM_RTC_WORKING_MODE_RTC_MASTER_SLAVE;
    if (ctmode1)
        master->RTC_WORKING_MODE |= BM_RTC_WORKING_MODE_RTC_COUNT_MODE;

    slave->RTC_WORKING_MODE &= ~BM_RTC_WORKING_MODE_RTC_MASTER_SLAVE;
    if (ctmode2)
        slave->RTC_WORKING_MODE |= BM_RTC_WORKING_MODE_RTC_COUNT_MODE;
}

void sec_rtc_set_violation_disable(Rtc *rtc1, Rtc *rtc2)
{
    if (rtc1)
        rtc1->VIOLATION_INT = BM_VIOLATION_INT_OVERFLOW_STATS;
    if (rtc2)
        rtc2->VIOLATION_INT = BM_VIOLATION_INT_OVERFLOW_STATS;
}
void sec_rtc_set_violation_overflow(Rtc *rtc1, Rtc *rtc2)
{
    if (rtc1)
        rtc1->VIOLATION_INT = FV_VIOLATION_INT_MASK(1);
    if (rtc2)
        rtc2->VIOLATION_INT = FV_VIOLATION_INT_MASK(1);
}

void sec_rtc_set_periodical(Rtc *rtc, uint8_t freq, uint8_t en)
{

    rtc->PERIODICAL_CTRL = FV_PERIODICAL_CRTL_FREQ(freq);
    if (en)
        rtc->PERIODICAL_CTRL |= BM_PERIODICAL_CRTL_IRQ_ENABLE;
}
uint8_t sec_rtc_get_periodical_freq(Rtc *rtc)
{
    uint32_t v = rtc->PERIODICAL_CTRL;
    return GFV_PERIODICAL_CRTL_FREQ(v);
}

void rtc_set_ms_tick(Rtc *rtc1, Rtc *rtc2, uint64_t tick1, uint64_t tick2)
{
    uint32_t v = tick1 & 0xffffffff;
    rtc1->RTC_H = (tick1 >> 32) & 0xffff;
    rtc1->RTC_L = v;

    if (rtc2) {
        rtc2->RTC_H = (tick2 >> 32) & 0xffff;
        rtc2->RTC_L = tick2 & 0xffffffff;
    }
}
void sec_rtc_set_ms_timer(Rtc *rtc1, Rtc *rtc2, uint64_t tick1, uint64_t tick2)
{
    rtc1->TIMER_H = (tick1 >> 32) & 0xffff;
    rtc1->TIMER_L = tick1 & 0xffffffff;

    if (rtc2) {
        rtc2->TIMER_H = (tick2 >> 32) & 0xffff;
        rtc2->TIMER_L = tick2 & 0xffffffff;
    }
}

uint32_t sec_rtc_get_violation_status(Rtc *rtc, uint8_t vio)
{
    uint32_t val = rtc->VIOLATION_INT;
    if (vio == 0) {
        if (val & BM_VIOLATION_INT_OVERFLOW_STATS)
            val = 1;
        else
            val = 0;
    }
    if (vio == 1) {
        if (val & BM_VIOLATION_INT_DISABLE_STATUS)
            val = 1;
        else
            val = 0;
    }
    return val;
}
void sec_rtc_fusa_enable(Rtc *rtc1, Rtc *rtc2, uint32_t enable)
{
    if (rtc1) {
        rtc1->APB_MONITOR_ERR_INJ_EN = enable;
        rtc1->IRQ_ERR_EJ_EN = enable;
        rtc1->RTC_PARITY_ERR_INT_STAT_EN = enable;
        rtc1->RTC_PARITY_ERR_INT_SIG_EN = enable;
        rtc1->IRQ_ERR_INJ |= 0x10;

        rtc1->SAFETY_ERR_IRQ_STAT_EN |= 0x37f;
        rtc1->SAFETY_ERR_IRQ_SIG_EN |= 0x37f;
    }

    if (rtc2) {
        rtc2->APB_MONITOR_ERR_INJ_EN = enable;
        rtc2->IRQ_ERR_EJ_EN = enable;
        rtc2->RTC_PARITY_ERR_INT_STAT_EN = enable;
        rtc2->RTC_PARITY_ERR_INT_SIG_EN = enable;
        rtc2->IRQ_ERR_INJ |= 0x10;

        rtc2->SAFETY_ERR_IRQ_STAT_EN |= 0x37f;
        rtc2->SAFETY_ERR_IRQ_SIG_EN |= 0x37f;
    }
}

void sec_rtc_ej(Rtc *rtc, rtc_ej_t ej, uint32_t val)
{
    if (ej.wdata) {
        rtc->APB_MONITOR_ERR_INJ_WDATA = val;
    }

    if (ej.ecc) {
        rtc->APB_MONITOR_ERR_INJ_ECC = val;
    }

    if (ej.parity) {
        rtc->PERIODICAL_CTRL = val;
    }
    rtc->RTC_L = 1;
}

void sec_rtc_apb_monitor_ej_clear(Rtc *rtc)
{
    if (rtc->APB_MONITOR_ERR_INJ_WDATA)
        rtc->APB_MONITOR_ERR_INJ_WDATA = 0;
    if (rtc->APB_MONITOR_ERR_INJ_ECC)
        rtc->APB_MONITOR_ERR_INJ_ECC = 0;

    rtc->SAFETY_ERR_IRQ_STAT |= (0x5 << 4);
}
uint32_t sec_rtc_get_parity_status(Rtc *rtc)
{
    return rtc->RTC_PARITY_ERR_INT_STAT;
}
void sec_rtc_clear_parity_status(Rtc *rtc) { rtc->RTC_PARITY_ERR_INT_STAT = 1; }
uint32_t sec_rtc_get_errirq_status(Rtc *rtc, uint8_t type)
{
    uint32_t val = rtc->SAFETY_ERR_IRQ_STAT;
    uint32_t ret = 0;
    switch (type) {
    case ERR_SIG_PADDR_UNC:
        val &= BM_SAFTY_ERR_IRQ_STAT_PADDR_UNCERR_INT_STAT;
        break;
    case ERR_SIG_PUSER_UNC:
        val &= BM_SAFTY_ERR_IRQ_STAT_PUSER_UNCERR_INT_STAT;
        break;
    case ERR_SIG_PCTL0_UNC:
        val &= BM_SAFTY_ERR_IRQ_STAT_PCTL0_UNCERR_INT_STAT;
        break;
    case ERR_SIG_PCTL1_UNC:
        val &= BM_SAFTY_ERR_IRQ_STAT_PCTL1_UNCERR_INT_STAT;
        break;
    case ERR_SIG_PWDATA_UNC:
        val &= BM_SAFTY_ERR_IRQ_STAT_PWDATA_UNCERR_INT_STAT;
        break;
    case ERR_SIG_PWDATA_COR:
        val &= BM_SAFTY_ERR_IRQ_STAT_PWDATA_CORERR_INT_STAT;
        break;
    case ERR_SIG_PWDATA_FATAL:
        val &= BM_SAFTY_ERR_IRQ_STAT_PWDATA_FATAL_INT_STAT;
        break;
    case ERR_SIG_SELFTEST_MODE:
        val &= BM_SAFTY_ERR_IRQ_STAT_SELFTEST_MODE_REDUNDANCY_ERR_INT_STAT;
        break;
    case ERR_SIG_PARITY_EJ:
        val &= BM_SAFTY_ERR_IRQ_STAT_REG_PARITY_EJ_EN_REDUNDANCY_ERR_INT_STAT;
        break;
    default:
        ret = val;
        break;
    }
    if (ret == 0)
        ret = val ? 1 : 0;

    return ret;
}

void sec_rtc_clear_errirq_status(Rtc *rtc)
{
    uint32_t bit = 0;
    if (rtc->SAFETY_ERR_IRQ_STAT &
        BM_SAFTY_ERR_IRQ_STAT_SELFTEST_MODE_REDUNDANCY_ERR_INT_STAT)
        bit = BM_SAFTY_ERR_IRQ_STAT_SELFTEST_MODE_REDUNDANCY_ERR_INT_STAT;
    else if (rtc->SAFETY_ERR_IRQ_STAT &
             BM_SAFTY_ERR_IRQ_STAT_REG_PARITY_EJ_EN_REDUNDANCY_ERR_INT_STAT)
        bit = BM_SAFTY_ERR_IRQ_STAT_REG_PARITY_EJ_EN_REDUNDANCY_ERR_INT_STAT;
    rtc->SAFETY_ERR_IRQ_STAT |= bit;
    rtc->SAFETY_ERR_IRQ_STAT &= ~bit;
}

void sec_rtc_auto_adjust(Rtc *rtc, uint32_t ctrl) { rtc->AUTO_ADJUST = ctrl; }

void sec_trigger_wakeup_req(Rtc *rtc1, Rtc *rtc2, uint32_t wait_cycle)
{
#if 0
    uint32_t rtc1_h = rtc1->RTC_H;
    uint32_t rtc1_l = rtc1->RTC_L;
    uint32_t rtc2_l = rtc2->RTC_L;

    uint32_t timer1_h = rtc1_h;
    uint32_t timer1_l = rtc1_l+wait_cycle;
    uint32_t timer2_h = 0xffff;
    uint32_t timer2_l = 0xffffffff - rtc2_l-wait_cycle;

    rtc1->TIMER_H = timer1_h;
    rtc1->TIMER_L = timer1_l;
    rtc2->TIMER_H = timer2_h;
    rtc2->TIMER_L = timer2_l;

    rtc1->WAKEUP_CTRL &= 0x7;
    rtc2->WAKEUP_CTRL &= 0x7;
    rtc_set_cross_clk_en(rtc1,rtc2);
    rtc_set_rtc_ctrl(rtc1,rtc2,BM_SEC_RTC_CTRL_RTC_LOCAL_ENABLE|BM_SEC_RTC_CTRL_RTC_ROMOTE_ENABLE,0);
#endif
}
void sec_rtc_err_sig_enable(Rtc *rtc, uint8_t sig, uint32_t en)
{
    uint32_t val = rtc->SAFETY_ERR_IRQ_SIG_EN;
    uint32_t sig_v = 0;
    switch (sig) {
    case ERR_SIG_PADDR_UNC:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PADDR_UNCERR_INT_SIG_EN;
        break;
    case ERR_SIG_PUSER_UNC:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PUSER_UNCERR_INT_SIG_EN;
        break;
    case ERR_SIG_PCTL0_UNC:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PCTL0_UNCERR_INT_SIG_EN;
        break;
    case ERR_SIG_PCTL1_UNC:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PCTL1_UNCERR_INT_SIG_EN;
        break;
    case ERR_SIG_PWDATA_UNC:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PWDATA_UNCERR_INT_SIG_EN;
        break;
    case ERR_SIG_PWDATA_COR:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PWDATA_CORERR_INT_SIG_EN;
        break;
    case ERR_SIG_PWDATA_FATAL:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_PWDATA_FATAL_INT_SIG_EN;
        break;
    case ERR_SIG_SELFTEST_MODE:
        sig_v = BM_SAFTY_ERR_IRQ_SIG_EN_SELFTEST_MODE_REDUNDANCY_ERR_INT_SIG_EN;
        break;
    case ERR_SIG_PARITY_EJ:
        sig_v =
            BM_SAFTY_ERR_IRQ_SIG_EN_REG_PARITY_EJ_EN_REDUNDANCY_ERR_INT_SIG_EN;
        break;
    default:
        sig_v = 0x37f;
        break;
    }
    if (en)
        val |= sig_v;
    else
        val &= ~sig;
}

void sec_rtc_out_ej(Rtc *rtc, uint8_t type, uint8_t en)
{
    uint32_t v = rtc->RTC_OUT_ERR_INJ;
    uint32_t wdata = 0;
    wdata = type ? (BM_RTC_OUT_ERR_INJ_RTC_OVERFLOW_DISABLE_ERR_INJ_EN |
                    BM_RTC_OUT_ERR_INJ_RTC_OVERFLOW_DISABLE_ERR_INJ)
                 : (BM_RTC_OUT_ERR_INJ_RTC_WAKEUP_REQ_ERR_INJ_EN |
                    BM_RTC_OUT_ERR_INJ_RTC_WAKEUP_REQ_ERR_INJ);

    if (en)
        v |= wdata;
    else
        v &= ~wdata;
    rtc->RTC_OUT_ERR_INJ = v;
}

void sec_rtc_out_ej_cancel(Rtc *rtc, uint8_t type)
{
    uint32_t v = rtc->RTC_OUT_ERR_INJ;
    if (type > SEC_RTC_OUT_OVDIS)
        v &= ~(BM_RTC_OUT_ERR_INJ_RTC_OVERFLOW_DISABLE_ERR_INJ |
               BM_RTC_OUT_ERR_INJ_RTC_WAKEUP_REQ_ERR_INJ);
    else
        v &= ~(1 << (type * 2));

    rtc->RTC_OUT_ERR_INJ = v;
}

uint8_t sec_rtc_get_out_ej(Rtc *rtc)
{
    uint32_t v = rtc->RTC_OUT_ERR_INJ;
    uint8_t res = 2;
    if (v & BM_RTC_OUT_ERR_INJ_RTC_WAKEUP_REQ_ERR_INJ)
        res = 0;
    if (v & BM_RTC_OUT_ERR_INJ_RTC_OVERFLOW_DISABLE_ERR_INJ)
        res = 1;
    return res;
}
