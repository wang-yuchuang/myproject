/**
 * @file rtc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: RTC driver.
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>
#include <stdlib.h>

#include "driver/rtc.h"

#define SECONDS_PER_MINUTE (60ul)
#define MINUTES_PER_HOUR (60ul)
#define HOURS_PER_DAY (24ul)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * HOURS_PER_DAY)

#define LEAP_YEAR_DAYS (366ul)
#define COMMON_YEAR_DAYS (365ul)

#define TM_START_YEAR (1900ul)
#define EPOCH_START_YEAR (1970ul)
static const uint32_t month_days[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

/**
 * @brief Simple algorithm to convert RTC date to Epoch seconds, which
 * starts from 1970-1-1, 00:00:00
 *
 * @param tm RTC time.
 * @return uint64_t Number of seconds since Epoch.
 */
uint64_t rtc_to_epoch(struct rtc_time *tm)
{
    uint64_t epoch_sec = 0;
    uint32_t year = tm->tm_year + TM_START_YEAR;

    for (uint32_t num = EPOCH_START_YEAR; num < year; num++) {
        epoch_sec += SECONDS_PER_DAY *
                     (is_leap_year(num) ? LEAP_YEAR_DAYS : COMMON_YEAR_DAYS);
    }

    for (uint32_t num = 0; num < tm->tm_mon; num++) {
        epoch_sec += month_days[num] * SECONDS_PER_DAY;

        if (is_leap_year(year) && (num == 1)) {
            epoch_sec += SECONDS_PER_DAY;
        }
    }

    epoch_sec += (tm->tm_mday - 1) * SECONDS_PER_DAY;
    epoch_sec += tm->tm_hour * SECONDS_PER_HOUR;
    epoch_sec += tm->tm_min * SECONDS_PER_MINUTE;
    epoch_sec += tm->tm_sec;

    return epoch_sec;
}

/**
 * @brief Simple algorithm to Epoch seconds to RTC time.
 *
 * @param tm
 * @param timec_ount
 */
void epoch_to_rtc(struct rtc_time *tm, uint64_t epoch_sec)
{
    uint32_t year = EPOCH_START_YEAR;
    uint32_t month = 0;
    uint32_t days = epoch_sec / SECONDS_PER_DAY;
    uint32_t seconds;

    if (days) {
        tm->tm_wday = (4 + days) % 7;
    }
    else{
        tm->tm_wday = 0;
    }

    while (days >= COMMON_YEAR_DAYS) {
        if (is_leap_year(year)) {
            if (days >= LEAP_YEAR_DAYS) {
                days -= LEAP_YEAR_DAYS;
            } else {
                year++;
                break;
            }
        } else {
            days -= COMMON_YEAR_DAYS;
        }

        year++;
    }

    tm->tm_year = year - TM_START_YEAR;

    while (days >= 28) {
        if (is_leap_year(tm->tm_year) && (month == 1)) {
            if (days >= 29) {
                days -= 29;
            } else {
                break;
            }
        } else {
            if (days >= month_days[month]) {
                days -= month_days[month];
            } else {
                break;
            }
        }

        month++;
    }

    tm->tm_mon = month;
    tm->tm_mday = days + 1;

    seconds = epoch_sec % SECONDS_PER_DAY;
    tm->tm_hour = seconds / 3600;
    tm->tm_min = (seconds % 3600) / 60;
    tm->tm_sec = (seconds % 3600) % 60;
}

/**
 * @brief rtc get time
 *
 * @param dev
 * @param tm
 * @return int
 */
int rtc_get_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    if (dev->ops->get_time) {
        return dev->ops->get_time(dev, tm);
    }

    return -1;
}

/**
 * @brief rtc set time
 *
 * @param dev
 * @param tm
 * @return int
 */
int rtc_set_time(rtc_dev_t *dev, struct rtc_time *tm)
{
    if (dev->ops->set_time) {
        return dev->ops->set_time(dev, tm);
    }

    return -1;
}

/**
 * @brief rtc get alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
int rtc_get_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm)
{
    if (dev->ops->get_alarm) {
        return dev->ops->get_alarm(dev, alrm);
    }

    return -1;
}

/**
 * @brief rtc set alarm
 *
 * @param dev
 * @param alrm
 * @return int
 */
int rtc_set_alarm(rtc_dev_t *dev, struct rtc_wkalrm *alrm, rtc_cb_t callback)
{
    if (dev->ops->set_alarm) {
        dev->arg = dev;
        dev->rtc_cb = callback;
        return dev->ops->set_alarm(dev, alrm);
    }

    return -1;
}

/**
 * @brief rtc enable alarm
 *
 * @param dev
 * @param en
 * @return int
 */
int rtc_enable_alarm(rtc_dev_t *dev, bool en)
{
    if (dev->ops->enable_alarm) {
        return dev->ops->enable_alarm(dev, en);
    }

    return -1;
}

/**
 * @brief rtc enable
 *
 * @param dev
 * @param en
 * @return int
 */
int rtc_enable(rtc_dev_t *dev, bool en)
{
    if (dev->ops->enable) {
        return dev->ops->enable(dev, en);
    }

    return -1;
}

/**
 * @brief rtc init
 *
 * @param dev
 */
void rtc_init(rtc_dev_t *dev)
{
    if (dev->ops->init) {
        dev->ops->init(dev);
    }

    return;
}

/**
 * @brief rtc deinit
 *
 * @param dev
 */
void rtc_deinit(rtc_dev_t *dev)
{
    if (dev->ops->deinit) {
        dev->ops->deinit(dev);
    }

    return;
}

void rtc_get_time_us(rtc_dev_t *dev, uint64_t *us) {
    if (dev->ops->get_time_us) {
        dev->ops->get_time_us(dev, us);
    }

    return;
}

void rtc_set_time_ms(rtc_dev_t *dev, uint64_t ms) {
    if (dev->ops->set_time_ms) {
        dev->ops->set_time_ms(dev, ms);
    }

    return;
}


