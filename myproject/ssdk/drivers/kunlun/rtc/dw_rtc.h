/**
 * @file dw_rtc.h
 * @brief semidrive rtc driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef DRV_LLD_DW_RTC_H
#define DRV_LLD_DW_RTC_H

#include <types.h>

void dw_rtc_lld_xtal_stable(paddr_t base);
void dw_rtc_lld_clear_alarm_status(paddr_t base);
int dw_rtc_lld_set_alarm_enable(addr_t base, bool en);
bool dw_rtc_lld_get_alarm_enable(paddr_t base);
bool dw_rtc_lld_get_alarm_status(paddr_t base);
uint64_t dw_rtc_lld_get_alarm_tick(paddr_t base);
int dw_rtc_lld_enable(paddr_t base);
int dw_rtc_lld_disable(paddr_t base);
void dw_rtc_lld_lock(paddr_t base);
void dw_rtc_lld_unlock(paddr_t base);
int dw_rtc_lld_alarm_compare_value(paddr_t base, uint64_t v);
uint64_t dw_rtc_lld_get_tick(paddr_t base);
void dw_rtc_lld_set_tick(paddr_t base, uint64_t tick);

#endif