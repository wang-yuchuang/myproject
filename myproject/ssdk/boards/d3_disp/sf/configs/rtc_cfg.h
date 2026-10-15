/*
 * rtc_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARD_RTC_CFG_H
#define BOARD_RTC_CFG_H

#ifdef CONFIG_RTC

#include <driver/taishan/rtc/sec_rtcd.h>

extern rtc_dev_t g_sec_rtc_dev;

void board_rtc_init(void);

#endif

#endif