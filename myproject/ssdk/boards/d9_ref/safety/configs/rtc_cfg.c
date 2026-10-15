
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_RTC

#include <board.h>
#include <types.h>
#include <reg.h>
#include "driver/kunlun/rtc/dw_rtc_drv.h"
#include "rtc_cfg.h"

rtc_dev_t g_rtc1_dev;

static const rtc_config_t rtc_config = {
    .base = DEVICE_BASE(RTC1),
    .irq = DEVICE_INTR(RTC1_RTC_WAKEUP),
};

static bool rtc_switch_osc(bool wait)
{
    bool done = false;

    do {
        /* xtal mask bit:31 | 1 | 0 - lock | EN |power down */
        if (((readl(APB_XTAL_RTC_BASE)) & 0x80000003) == 0x80000002) {
            /* xtal is stable now. select to xtal as source. */
            if (!(readl(APB_RC_RTC_BASE + 4) & 1ul))
                writel(1ul, APB_RC_RTC_BASE + 4);

            done = true;
            break;
        }
    }
    while (wait);

    return done;
}

static void osc_check(void *argument)
{
    while (!rtc_switch_osc(false))
    {
        osDelay(20);
    }
    osThreadExit();
}

void board_rtc_init(void)
{
    osThreadAttr_t attr = {
        .name = "rtc_osc_check",
        .stack_size = CONFIG_OS_DEFAULT_STACK_SIZE,
        .priority = CONFIG_MAIN_THREAD_PRIORITY,
    };

    dw_rtc_probe(&g_rtc1_dev, &rtc_config);
    rtc_init(&g_rtc1_dev);

    if (!rtc_switch_osc(false))
        osThreadNew(osc_check, NULL, &attr);

    rtc_enable(&g_rtc1_dev,1);
}

#endif
