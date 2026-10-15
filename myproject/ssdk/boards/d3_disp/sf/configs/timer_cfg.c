
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
#include <driver/taishan/btm/sdrv_btm_drv.h>

/* timer device drivers */
sdrv_btm_dev_t g_sdrv_btm_kernel_timer;
timer_dev_t *g_kernel_timer;


/* timer config structures */
static const sdrv_btm_cfg_t kernel_timer_config = {
    .base = DEVICE_BASE(BTM1),
    .irq = DEVICE_INTR(BTM1_O_BTM),
    .tmr_id = SDRV_BTM_G1,
    .tmr_cfg = {
        .si_val = 23,
        .inc_val = 1,
        .frc_rld_rst_cnt_en = true,
        .term_use_mode = SDRV_BTM_SHADOW,
        .cmp_use_mode = SDRV_BTM_SHADOW,
        .cnt_dir = SDRV_BTM_CNT_UP,
        .cnt_mode = SDRV_BTM_CONTINOUS_MODE,
    }
};


/* timer initialization */
void board_timer_init(void)
{
    g_kernel_timer = sdrv_btm_probe(&g_sdrv_btm_kernel_timer, &kernel_timer_config);
    arch_timer_init(g_kernel_timer);

}


#endif
