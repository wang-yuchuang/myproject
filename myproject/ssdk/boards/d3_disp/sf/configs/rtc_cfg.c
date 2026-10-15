/*
 * rtc_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_RTC

#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "rtc_cfg.h"

rtc_dev_t g_sec_rtc_dev;

rtc_config_t rtc_cfg = {
    .base = DEVICE_BASE(RTC1),
    .irq = DEVICE_INTR(RTC1_RTC_WAKEUP),
};

/* 2022-1-1 00:00:00 */
static struct rtc_time tm0 = {
    .tm_sec  = 0,
    .tm_min  = 0,
    .tm_hour = 0,
    .tm_mday = 1,
    .tm_mon  = 0,
    .tm_year = 122,
};

void board_rtc_init(void)
{
    struct rtc_time tm;

    sec_rtc_probe(&g_sec_rtc_dev, &rtc_cfg);

    rtc_init(&g_sec_rtc_dev);

    rtc_get_time(&g_sec_rtc_dev, &tm);

    if (rtc_to_epoch(&tm) < rtc_to_epoch(&tm0)) {
        rtc_set_time(&g_sec_rtc_dev, &tm0);
    }
}

#endif
