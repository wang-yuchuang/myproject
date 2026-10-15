/**
 * @file irq_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Interrupt Nesting Tester.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cmsis_os2.h>
#include <CLI_type.h>

#include <board.h>
#include <driver/irq.h>
#include <driver/kunlun/tmr/sdrv_tmr_drv.h>

#ifdef CONFIG_ARCH_WITH_SMP
#include <arch/smp.h>
#endif

#define CNT_NUM 2

#if CONFIG_CODE_DATA_RUN_IN_TCM
/* TCM data section */
__SECTION(".tcmdata")
#endif
static const sdrv_tmr_cfg_t timer1_config = {
    .base = DEVICE_BASE(TIMER3),
    .irq = DEVICE_INTR(TIMER3_CHN),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
};

static const sdrv_tmr_cfg_t timer2_config = {
    .base = DEVICE_BASE(TIMER4),
    .irq = DEVICE_INTR(TIMER4_CHN),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
};

static sdrv_tmr_dev_t sdrv_timer1, sdrv_timer2;
static timer_dev_t *timer1, *timer2;
static int timer1_cnt, timer2_cnt;
#if CONFIG_OS_BAREMETAL
volatile int timer_flag = 0;
#else
static osSemaphoreId_t timer_sem;
#endif

#if CONFIG_CODE_DATA_RUN_IN_TCM
/* TCM code section */
__SECTION(".tcmcode") __NO_INLINE
#endif
static void Timer1Handle(void *arg)
{
#ifdef CONFIG_ARCH_WITH_SMP
    printf("%s, cpuid:%d\n", __func__, arch_curr_cpu_num());
#endif
    printf("Timer1Handle start:%d\n", timer1_cnt);
    printf("Timer1Handle end:%d\n", timer1_cnt);
    timer1_cnt++;
}
#if CONFIG_CODE_DATA_RUN_IN_TCM
__NO_INLINE
#endif
static void Timer2Handle(void *arg)
{
    printf("Timer2Handle start:%d\n", timer2_cnt);
    arch_irq_enable();
    udelay(3000000);
    printf("Timer2Handle end:%d\n", timer2_cnt);
    timer2_cnt++;
    if (timer2_cnt >= CNT_NUM) {
#if CONFIG_OS_BAREMETAL
        timer_flag = 1;
#else
        osSemaphoreRelease(timer_sem);
#endif
    }
}

static void irq_nesting_test(void)
{

#if CONFIG_OS_BAREMETAL
    timer_flag = 0;
#else
    timer_sem = osSemaphoreNew(1, 0, NULL);
#endif
    sdrv_tmr_init(&sdrv_timer1, &timer1_config);
    timer1 = sdrv_tmr_timer_probe(&sdrv_timer1, SDRV_TMR_A);
    irq_set_priority(timer1_config.irq, 20);

#ifdef CONFIG_ARCH_WITH_SMP
    /* cpu 1 */
    gic_set_affinity(0x2, timer1_config.irq);
#endif

    sdrv_tmr_init(&sdrv_timer2, &timer2_config);
    timer2 = sdrv_tmr_timer_probe(&sdrv_timer2, SDRV_TMR_A);
    irq_set_priority(timer2_config.irq, 30);

#ifdef CONFIG_ARCH_WITH_SMP
    /* cpu 2*/
    gic_set_affinity(0x4, timer2_config.irq);
#endif

    timer_set_callback(timer1, Timer1Handle, NULL);
    timer_start(timer1, TMR_TYPE_PERIOD, 1000);

    timer_set_callback(timer2, Timer2Handle, NULL);
    timer_start(timer2, TMR_TYPE_PERIOD, 5000);

#if CONFIG_OS_BAREMETAL
    while(1){
        if(timer_flag){
            break;
        }
    }
#else
    osSemaphoreAcquire(timer_sem, osWaitForever);
    osSemaphoreDelete(timer_sem);
#endif

    timer_stop(timer1);
    timer_stop(timer2);

    printf("irq test success!\n");
}

static int irq_test(int argc, char *argv[])
{
    irq_nesting_test();
    return CLI_FALSE;
}

CLI_CMD("irq_test", "interrupt nesting test", irq_test);
