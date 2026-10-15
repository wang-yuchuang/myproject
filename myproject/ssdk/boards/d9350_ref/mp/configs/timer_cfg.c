
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_TIMER

#include "timer_cfg.h"

#include <types.h>
#include <board.h>

#include <driver/arch_timer.h>

/* timer device driver headers */
#include <driver/kunlun/tmr/sdrv_tmr_drv.h>
#include <driver/kunlun/syscnt/sdrv_syscnt_drv.h>

/* timer device drivers */
sdrv_tmr_dev_t g_sdrv_kernel_timer;
timer_dev_t *g_kernel_timer;

/* sys cnt drivers */
sdrv_syscnt_dev_t g_sdrv_syscnt_dev;
timer_dev_t *g_syscnt_timer;

/* timer config structures */
static const sdrv_tmr_cfg_t kernel_timer_config = {
    .base = DEVICE_BASE(TIMER1),
    .irq = DEVICE_INTR(TIMER1_CHN),
    .ovf_irq = DEVICE_INTR(TIMER1_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = true,
};

/* timer config structures */
static const sdrv_syscnt_cfg_t syscnt_config = {
    .rw_base = DEVICE_BASE(SYS_CNT_RW),
    .ro_base = DEVICE_BASE(SYS_CNT_RO),
    .clk_frq = 3000000,
};


/* timer initialization */
void board_timer_init(void)
{
    sdrv_tmr_init(&g_sdrv_kernel_timer, &kernel_timer_config);
    g_kernel_timer = sdrv_tmr_timer_probe(&g_sdrv_kernel_timer, SDRV_TMR_A);
    arch_timer_init(g_kernel_timer);

    g_syscnt_timer = sdrv_syscnt_probe(&g_sdrv_syscnt_dev, &syscnt_config);
    timer_start(g_syscnt_timer, TMR_TYPE_PERIOD, 0);
}


#endif
