/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_RTC_CFG_H
#define BOARDS_RTC_CFG_H

#ifdef CONFIG_RTC

#include <driver/rtc.h>

extern rtc_dev_t g_rtc1_dev;
void board_rtc_init(void);

#endif

#endif /* BOARDS_RTC_CFG_H */
