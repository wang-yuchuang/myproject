
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

/* timer device drivers */
sdrv_tmr_dev_t g_sdrv_kernel_timer;
timer_dev_t *g_kernel_timer;


/* timer config structures */
static const sdrv_tmr_cfg_t kernel_timer_config = {
    .base = DEVICE_BASE(TIMER3),
    .irq = DEVICE_INTR(TIMER3_CHN),
    .ovf_irq = DEVICE_INTR(TIMER3_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = true,
};


/* timer initialization */
void board_timer_init(void)
{
    sdrv_tmr_init(&g_sdrv_kernel_timer, &kernel_timer_config);
    g_kernel_timer = sdrv_tmr_timer_probe(&g_sdrv_kernel_timer, SDRV_TMR_A);
    arch_timer_init(g_kernel_timer);

}


#endif
