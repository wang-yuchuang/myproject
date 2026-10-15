/**
 * @file rtc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: rtc.h interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_RTC_H
#define INCLUDE_RTC_H

#include "cmsis_os2.h"

typedef int (*rtc_cb_t)(uint32_t irq, void *arg);

typedef struct rtc_dev rtc_dev_t;

/**
 * @brief RTC time structure.
 *
 * The RTC time structure uses Gregorian time.
 */
struct rtc_time {
    uint32_t tm_sec;   /* seconds, 0~61 */
    uint32_t tm_min;   /* minutes, 0~59 */
    uint32_t tm_hour;  /* hour, 0~23 */
    uint32_t tm_mday;  /* day, 1~31 */
    uint32_t tm_mon;   /* month, 0~11 */
    uint32_t tm_year;  /* years since 1900 */
    uint32_t tm_wday;  /* day since Sunday, 0~6 */
    uint32_t tm_yday;  /* days since Jan. 1, 0~365 */
    uint32_t tm_isdst; /* daylight saving time flag */
};

/**
 * @brief
 *
 */
struct rtc_wkalrm {
    unsigned char enable;
    unsigned char pending;
    struct rtc_time tm;
};

/**
 * @brief RTC device configuration.
 */
typedef struct rtc_config {
    paddr_t base;
    int irq;
} rtc_config_t;

/**
 * @brief RTC devices operations.
 */
typedef struct rtc_ops {
    int (*get_time)(rtc_dev_t *dev, struct rtc_time *tm);
    int (*set_time)(rtc_dev_t *dev, struct rtc_time *tm);
    int (*get_alarm)(rtc_dev_t *dev, struct rtc_wkalrm *tm);
    int (*set_alarm)(rtc_dev_t *dev, struct rtc_wkalrm *tm);
    int (*enable_alarm)(rtc_dev_t *dev, bool enabled);
    int (*enable)(rtc_dev_t *dev, bool enabled);
    void (*init)(rtc_dev_t *dev);
    void (*deinit)(rtc_dev_t *dev);
    void (*get_time_us)(rtc_dev_t *dev, uint64_t *us);
    int (*set_time_ms)(rtc_dev_t *dev, uint64_t ms);
} rtc_ops_t;

/**
 * @brief RTC device structure.
 */
struct rtc_dev {
    const rtc_config_t *cfg;
    rtc_cb_t rtc_cb; /* rtc alarm callback */
    const rtc_ops_t *ops;
    osMutexId_t lock;
    void *arg;
};

/**
 * @brief check is leap yaer
 *
 * @param year
 * @return true
 * @return false
 */
static inline bool is_leap_year(uint32_t year)
{
    return (!(year % 4) && (year % 100)) || !(year % 400);
}

uint64_t rtc_to_epoch(struct rtc_time *tm);
void epoch_to_rtc(struct rtc_time *tm, uint64_t epoch_sec);
int rtc_get_time(rtc_dev_t *dev, struct rtc_time *tm);
void rtc_get_time_us(rtc_dev_t *dev, uint64_t *us);
void rtc_set_time_ms(rtc_dev_t *dev, uint64_t ms);
int rtc_set_time(rtc_dev_t *dev, struct rtc_time *tm);
int rtc_get_alarm(rtc_dev_t *dev, struct rtc_wkalrm *tm);
int rtc_set_alarm(rtc_dev_t *dev, struct rtc_wkalrm *tm, rtc_cb_t callback);
int rtc_enable_alarm(rtc_dev_t *dev, bool en);
int rtc_enable(rtc_dev_t *dev, bool en);
void rtc_init(rtc_dev_t *dev);
void rtc_deinit(rtc_dev_t *dev);

#endif
