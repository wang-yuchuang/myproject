/**
 * @file rtc_test.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: rtc_test.c application.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <types.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include "driver/irq.h"
#include <driver/kunlun/rtc/dw_rtc_drv.h>
#include <board.h>
#include <rtc_cfg.h>

#define TIMER_TEST_SET    0
#define TIMER_TEST_GET    1
#define ALARM_TEST_SET    2
#define ALARM_TEST_GET    3

/**
 * @brief time week info
 *
 */
static const char week[][5] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};

/**
 * @brief time info meet the requirement of rtc lib-c
 *
 * @param tm
 * @return true
 * @return false
 */
static bool check_set_timer_args(struct rtc_time *tm)
{
    uint32_t tm_sec = tm->tm_sec;
    uint32_t tm_min = tm->tm_min;
    uint32_t tm_hour = tm->tm_hour;
    uint32_t tm_mday = tm->tm_mday;
    uint32_t tm_mon = tm->tm_mon;
    uint32_t tm_year = tm->tm_year;

    epoch_to_rtc(tm, rtc_to_epoch(tm));
    return (tm_sec == tm->tm_sec && tm_min == tm->tm_min && tm_hour == tm->tm_hour
            && \
            tm_mday == tm->tm_mday && tm_mon == tm->tm_mon && tm_year == tm->tm_year);
}

/**
 * @brief
 *
 * @param irq
 * @param arg
 * @return int
 */
static int rtc_alarm_callback_handler(uint32_t irq, void *arg)
{
    ASSERT(arg);

    int ret = -1;
    rtc_dev_t *rtc_dev = (rtc_dev_t *)arg;

    ssdk_printf(SSDK_EMERG, "%s rtc_irq_happend: rtc alarm\n", __func__);

    ret = rtc_enable_alarm(rtc_dev, false);

    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "%s rtc enable alarm failed...\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * @brief ret alarm set test
 *
 * @param rtc_dev
 * @return int
 */
static int rtc_alarm_test_set(rtc_dev_t *rtc_dev)
{
    int ret = -1;

    /* 2021-12-16 16:14:01 */
    struct rtc_time tm = {
        .tm_sec  = 1,
        .tm_min  = 14,
        .tm_hour = 16,
        .tm_mday = 16,
        .tm_mon  = 11,
        .tm_year = 121,
    };
    /* 2021-12-16 16:14:11 */
    struct rtc_wkalrm alrm = {
        .tm = {
            .tm_sec  = 11,
            .tm_min  = 14,
            .tm_hour = 16,
            .tm_mday = 16,
            .tm_mon  = 11,
            .tm_year = 121,
        }
    };

    /* set default rtc time */
    ret = rtc_set_time(rtc_dev, &tm);

    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "%s rtc set time failed\n", __func__);
        return ret;
    }

    /* set rtc alarm args */
    ret = rtc_set_alarm(rtc_dev, &alrm, rtc_alarm_callback_handler);

    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "%s rtc set alarm failed\n", __func__);
        return ret;
    }

    /* enable rtc alarm */
    ret = rtc_enable_alarm(rtc_dev, true);

    if (ret < 0) {
        ssdk_printf(SSDK_EMERG, "%s rtc enable alarm failed\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * @brief rtc alarm get test
 *
 * @param rtc_dev
 * @return int
 */
static int rtc_alarm_test_get(rtc_dev_t *rtc_dev)
{
    struct rtc_wkalrm alrm;

    rtc_get_alarm(rtc_dev, &alrm);

    ssdk_printf(SSDK_EMERG, "alarm info %d-%d-%d %d:%d:%d %d %d\n", alrm.tm.tm_year + 1900,
                alrm.tm.tm_mon + 1, alrm.tm.tm_mday, alrm.tm.tm_hour, \
                alrm.tm.tm_min, alrm.tm.tm_sec, alrm.enable, alrm.pending);

    return 0;
}
uint32_t lp = 1;
/**
 * @brief rtc time set test
 *
 * @param rtc_dev
 * @param argv
 * @return int
 */
static int rtc_time_test_set(rtc_dev_t *rtc_dev, char *argv[])
{
    int ret = -1;
    struct rtc_time tm;

    tm.tm_year = atoi(argv[1]) - 1900;
    tm.tm_mon  = atoi(argv[2]) - 1;
    tm.tm_mday = atoi(argv[3]);
    tm.tm_hour = atoi(argv[4]);
    tm.tm_min  = atoi(argv[5]);
    tm.tm_sec  = atoi(argv[6]);

    if (!check_set_timer_args(&tm)) {
        ssdk_printf(SSDK_EMERG, "%s timer args error\n", __func__);
        return ret;
    }

    printf("set time: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    ret = rtc_set_time(rtc_dev, &tm);

    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret set time fialed\n", __func__);
        return ret;
    }

    return 0;
}

/**
 * @brief rtc time get test
 *
 * @param rtc_dev
 * @return int
 */
static int rtc_time_test_get(rtc_dev_t *rtc_dev)
{
    int32_t ret = -1;
    struct rtc_time tm;

    ret = rtc_get_time(rtc_dev, &tm);

    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret get time fialed\n", __func__);
        return ret;
    }

    ssdk_printf(SSDK_EMERG, "time info %d-%d-%d %d:%d:%d %s\n", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, week[tm.tm_wday]);

    osDelay(5000);

    ret = rtc_get_time(rtc_dev, &tm);

    if (ret < 0 ) {
        ssdk_printf(SSDK_EMERG, "%s ret get time fialed\n", __func__);
        return ret;
    }

    ssdk_printf(SSDK_EMERG, "time info %d-%d-%d %d:%d:%d %s\n", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, week[tm.tm_wday]);

    return 0;
}

static void rtc_test_usage(void)
{
    printf("\nRTC test commands:\n");
    printf("\trtc_test 0 year month day hour min sec ;set time to timer\n");
    printf("\trtc_test 1 ;get time from timer\n");
    printf("\trtc_test 2 ;set time to alarm\n");
    printf("\trtc_test 3 ;get time from alarm\n");
}

/**
 * @brief rtc test entry
 *
 * @param argc
 * @param argv
 * @return int
 */
static int rtc_test(int argc, char *argv[])
{
    /* wake_up, set_tick, get_tick .eg.*/
    uint32_t cmd_num = atoi(argv[0]);
    rtc_dev_t *rtc_dev = NULL;

    rtc_dev = &g_rtc1_dev;

    switch (cmd_num) {
        case TIMER_TEST_SET:
            if (argc != 7) {
                ssdk_printf(SSDK_EMERG, "%s time set args err\n", __func__);
                rtc_test_usage();
                return 0;
            }

            rtc_time_test_set(rtc_dev, argv);
            break;

        case TIMER_TEST_GET:
            if (argc != 1) {
                ssdk_printf(SSDK_EMERG, "%s time get args err\n", __func__);
                rtc_test_usage();
                return 0;
            }

            rtc_time_test_get(rtc_dev);
            break;

        case ALARM_TEST_SET:
            if (argc != 1) {
                ssdk_printf(SSDK_EMERG, "%s alarm get args err\n", __func__);
                rtc_test_usage();
                return 0;
            }

            rtc_alarm_test_set(rtc_dev);
            break;

        case ALARM_TEST_GET:
            if (argc != 1) {
                ssdk_printf(SSDK_EMERG, "%s alarm set args err\n", __func__);
                rtc_test_usage();
                return 0;
            }

            rtc_alarm_test_get(rtc_dev);
            break;

        default:
            ssdk_printf(SSDK_EMERG, "%s cmd num err\n", __func__);
            rtc_test_usage();
            break;
    }

    return 0;
}

CLI_CMD("rtc_test", "rtc test", rtc_test);