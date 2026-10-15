//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_PWM

#include <board.h>
#include <types.h>

#include "pwm_cfg.h"

/* pwm device driver headers */
#include <driver/taishan/epwm/sdrv_epwm_drv.h>
sdrv_epwm_controller_t g_sdrv_epwm_ctrl_1;

static pwm_dev_t *g_pwm_1;
static pwm_dev_t *g_pwm_2;
static pwm_dev_t *g_pwm_3;
static pwm_dev_t *g_pwm_4;

pwm_dev_t *mc_chnl1;
pwm_dev_t *mc_chnl2;
pwm_dev_t *mc_chnl3;
pwm_dev_t *mc_chnl4;

static const epwm_cfg_t epwm_cfg_1 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .cmp_pulse_wid0.cmp00 = 100,
            // cmp_pulse_wid0.cmp00 SHOULD LESS THAN OR EQUAL TO 1/EPWM_SRC_CLK
            // * 0xFF
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_POSITIVE_PULSE,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_KEEP,
                },

        },
    .chnl = SDRV_EPWM_CHANNEL_A,
};

static const epwm_cfg_t epwm_cfg_2 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .sw_rld_mode = 1,
        },
    .chnl = SDRV_EPWM_CHANNEL_B,
};

static const epwm_cfg_t epwm_cfg_3 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .sw_rld_mode = 1,
        },
    .chnl = SDRV_EPWM_CHANNEL_C,
};

static const epwm_cfg_t epwm_cfg_4 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .sw_rld_mode = 1,
        },
    .chnl = SDRV_EPWM_CHANNEL_D,
};

static const sdrv_epwm_cfg_t g_sdrv_epwm_cfg_1 = {
    .base = DEVICE_BASE(EPWM1),
    .irq = DEVICE_INTR(EPWM1_CNT_OVF),
    .irq_id = EPWM_BM_CNT_G0_OVF,

    .clk_src = SDRV_EPWM_HIGH_FREQUENCY_CLOCK,
    .clk_div = 0,

};

/* pwm initialization */
void board_pwm_init(void)
{
    sdrv_epwm_controller_init(&g_sdrv_epwm_ctrl_1, &g_sdrv_epwm_cfg_1);

    g_pwm_1 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_1);
    g_pwm_2 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_2);
    g_pwm_3 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_3);
    g_pwm_4 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_4);

    // configure mc_chan1/2/3/4
    mc_chnl1 = g_pwm_1;
    mc_chnl2 = g_pwm_2;
    mc_chnl3 = g_pwm_3;
    mc_chnl4 = g_pwm_4;
}

#endif
