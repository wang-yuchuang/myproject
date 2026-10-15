/**
 * @file sdrv_tmr.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
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
#include <debug.h>

#include <board.h>
#include <driver/irq.h>
#include <driver/kunlun/tmr/sdrv_tmr_drv.h>

#define SDRV_TMR_TEST_CNT       50

#define SDRV_TMR_DEMO_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, format, ##__VA_ARGS__)
#define SDRV_TMR_DEMO_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, format, ##__VA_ARGS__)
#define SDRV_TMR_DEMO_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, format, ##__VA_ARGS__)
#define SDRV_TMR_DEMO_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, format, ##__VA_ARGS__)

static const sdrv_tmr_cfg_t timer1_config = {
    .base = DEVICE_BASE(TIMER5),
    .irq = DEVICE_INTR(TIMER5_CHN),
    .ovf_irq = DEVICE_INTR(TIMER5_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = false,
};

static const sdrv_tmr_cfg_t timer2_config = {
    .base = DEVICE_BASE(TIMER6),
    .irq = DEVICE_INTR(TIMER6_CHN),
    .ovf_irq = DEVICE_INTR(TIMER6_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = true,
};

static sdrv_tmr_dev_t sdrv_timer1;
static timer_dev_t *timer1;
static sdrv_tmr_dev_t sdrv_timer2;
static timer_dev_t *timer2;
static osSemaphoreId_t timer_sem;
static int timer_cnt;

static void Timer1Handle(void *arg)
{
    printf("Timer1Handle:%d\n", timer_cnt);
    timer_cnt++;
    if (timer_cnt >= SDRV_TMR_TEST_CNT) {
        osSemaphoreRelease(timer_sem);
    }
}

static void sdrv_tmr_glb_cnt_demo(int argc, char *argv[])
{
    timer_cnt = 0;

    timer_sem = osSemaphoreNew(1, 0, NULL);

    sdrv_tmr_init(&sdrv_timer1, &timer1_config);
    timer1 = sdrv_tmr_timer_probe(&sdrv_timer1, SDRV_TMR_G0);

    timer_set_callback(timer1, Timer1Handle, NULL);
    timer_start(timer1, TMR_TYPE_PERIOD, 1000);

    osSemaphoreAcquire(timer_sem, osWaitForever);

    osSemaphoreDelete(timer_sem);
    timer_stop(timer1);

    printf("sdrv_tmr_glb_cnt_demo success!\n");
}

static void Timer2Handle(void *arg)
{
    printf("Timer2Handle:%d\n", timer_cnt);
    timer_cnt++;
    if (timer_cnt >= SDRV_TMR_TEST_CNT) {
        osSemaphoreRelease(timer_sem);
    }
}

static void sdrv_tmr_chn_cnt_demo(int argc, char *argv[])
{
    timer_cnt = 0;

    timer_sem = osSemaphoreNew(1, 0, NULL);

    sdrv_tmr_init(&sdrv_timer2, &timer2_config);
    timer2 = sdrv_tmr_timer_probe(&sdrv_timer2, SDRV_TMR_A);

    timer_set_callback(timer2, Timer2Handle, NULL);
    timer_start(timer2, TMR_TYPE_PERIOD, 1000);

    osSemaphoreAcquire(timer_sem, osWaitForever);

    osSemaphoreDelete(timer_sem);
    timer_stop(timer2);

    printf("sdrv_tmr_chn_cnt_demo success!\n");
}

static void sdrv_tmr_demo_usage(void)
{
    SDRV_TMR_DEMO_ALWAYS("sdrv_tmr_demo glb_cnt");
    SDRV_TMR_DEMO_ALWAYS("\t\t:sdrv tmr global count demo\n");

    SDRV_TMR_DEMO_ALWAYS("sdrv_tmr_demo chn_cnt");
    SDRV_TMR_DEMO_ALWAYS("\t\t:sdrv tmr channel count demo\n");
}

static int sdrv_tmr_demo(int argc, char *argv[])
{
    if (argc == 0) {
        sdrv_tmr_demo_usage();
        return 0;
    }

    if (!strcmp(argv[0], "glb_cnt")) {
        sdrv_tmr_glb_cnt_demo(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "chn_cnt")) {
        sdrv_tmr_chn_cnt_demo(argc - 1, &argv[1]);
    }

    return CLI_FALSE;
}

CLI_CMD("sdrv_tmr_demo", "\r\nsdrv_tmr_demo:\r\n sdrv tmr demo\r\n", sdrv_tmr_demo);
