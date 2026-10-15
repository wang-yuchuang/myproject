#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "CLI_type.h"

#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "irq_num.h"
#include "regs_base.h"
#include "param.h"

static int wdt_overflow_isr(uint32_t irq, void *arg);

static osSemaphoreId_t g_sem;
static watchdog_device_t g_wdt;
static sdrv_watchdog_device_t g_sdrv_wdt = {
#ifdef CONFIG_BOARD_D9_AP2
    .base  =  APB_WDT6_BASE,
    .irq   =  WDT6_OVFLOW_INTR_NUM,
#else
    .base  =  APB_WDT1_BASE,
    .irq   =  WDT1_OVFLOW_INTR_NUM,
#endif
    .isr   =  wdt_overflow_isr,
};

static int wdt_overflow_isr(uint32_t irq, void *arg)
{
    printf("wdt_overflow_isr\n");

    osSemaphoreRelease(g_sem);

    return 0;
}

static int watchdog_test_main(int argc, char *argv[])
{
    int refresh_cnt = 5;

    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        printf("sdrv_watchdog_init fail.\n");
        goto ret;
    }

    /* set timeout 2second */
    if (watchdog_set_timeout(&g_wdt, 2)) {
        printf("watchdog_set_timeout fail.\n");
        goto ret;
    }

    if (watchdog_start(&g_wdt)) {
        printf("watchdog_start fail.\n");
        goto ret;
    }

    while (refresh_cnt > 0) {
        osDelay(1 * osKernelGetTickFreq());
        printf("ping dog at time: %d\n", refresh_cnt);
        if(watchdog_ping(&g_wdt)) {
            printf("watchdog_ping fail.\n");
            goto ret;
        }
        refresh_cnt--;
    }

    g_sem = osSemaphoreNew(1, 0, NULL);

    watchdog_ping(&g_wdt);

    printf("Test watchdog timeout begin.\n");

    if (osSemaphoreAcquire(g_sem, 3 * osKernelGetTickFreq()) == osOK)
        printf("Watchdog timeout success\n");
    else
        printf("osSemaphoreAcquire fail.\n");

    osDelay(5 * osKernelGetTickFreq());

    osSemaphoreDelete(g_sem);
ret:
    return CLI_FALSE;
}

CLI_CMD("wdg_test", "Watchdog test", watchdog_test_main);
