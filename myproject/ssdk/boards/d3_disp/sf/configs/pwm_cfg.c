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
sdrv_epwm_controller_t g_sdrv_epwm_ctrl_2;
sdrv_epwm_controller_t g_sdrv_epwm_ctrl_3;
sdrv_epwm_controller_t g_sdrv_epwm_ctrl_4;

pwm_dev_t *g_pwm_1;
pwm_dev_t *g_pwm_2;
pwm_dev_t *g_pwm_3;
pwm_dev_t *g_pwm_4;

pwm_dev_t *g_pwm2_1;
pwm_dev_t *g_pwm2_2;
pwm_dev_t *g_pwm2_3;
pwm_dev_t *g_pwm2_4;

pwm_dev_t *g_pwm3_1;
pwm_dev_t *g_pwm3_2;
pwm_dev_t *g_pwm3_3;
pwm_dev_t *g_pwm3_4;

pwm_dev_t *g_pwm4_1;
pwm_dev_t *g_pwm4_2;
pwm_dev_t *g_pwm4_3;
pwm_dev_t *g_pwm4_4;

static const epwm_cfg_t epwm_cfg_1 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
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
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp10 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp1_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_B,
};

static const epwm_cfg_t epwm_cfg_3 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,//normal use
            //.con_mode = EPWM_ONLY_EXECUTE_ONE_TIME, // oneshot use
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                },

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
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp10 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp11 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                },

        },
    .chnl = SDRV_EPWM_CHANNEL_D,
};

static const epwm_cfg_t epwm2_cfg_1 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_MULTIPLE_COMPARE_MODE,
            .cmp_pulse_wid0 =
                {
                    .cmp00 = 10,
                    .cmp01 = 10,
                    .cmp10 = 10,
                    .cmp11 = 10,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_A,
};

static const epwm_cfg_t epwm2_cfg_2 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_B,
};

static const epwm_cfg_t epwm2_cfg_3 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp10 = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                    .cmp1_ovf = SDRV_EPWM_CMP_EVENT_SIGNAL_TOGGLE,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_C,
};

static const epwm_cfg_t epwm2_cfg_4 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_D,
};

static const epwm_cfg_t epwm3_cfg_1 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_BOTH_COMP0_AND_COMP1_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_DUAL_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp01 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp10 = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                    .cmp11 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_A,
};

static const epwm_cfg_t epwm3_cfg_2 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_MULTIPLE_COMPARE_MODE,
            .cmp_pulse_wid0 =
                {
                    .cmp00 = 10,
                    .cmp01 = 10,
                    .cmp10 = 10,
                    .cmp11 = 10,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_B,
};

static const epwm_cfg_t epwm3_cfg_3 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_MULTIPLE_COMPARE_MODE,
            .refresh_intval = 3,
            .cmp_pulse_wid0 =
                {
                    .cmp00 = 10,
                    .cmp01 = 10,
                    .cmp10 = 10,
                    .cmp11 = 10,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_C,
};

static const epwm_cfg_t epwm3_cfg_4 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G0,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .audio_enable = true,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_D,
};

static const epwm_cfg_t epwm4_cfg_1 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            //            .audio_enable = true,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 7, // normal use
                    //.fifo_wml = 31, // co_ac
                    //.fifo_wml = 63, // co_abcd
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_A,
};

static const epwm_cfg_t epwm4_cfg_2 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 31,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_B,
};

static const epwm_cfg_t epwm4_cfg_3 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            .dma_cfg =
                {
                    .dma_enable = true,
                    .fifo_wml = 31,
                },
        },
    .chnl = SDRV_EPWM_CHANNEL_C,
};

static const epwm_cfg_t epwm4_cfg_4 = {
    .cmp_cfg =
        {
            .cmp_cnt_sel = SDRV_EPWM_CNT_G1,
            .chnl_out_mode = EPWM_ONLY_COMP0_OUTPUT,
            .con_mode = EPWM_CONSECUTIVE_MODE,
            .cmp_mode = SDRV_EPWM_SINGLE_COMPARE_MODE,
            .refresh_intval = 99, // use in upload twice
            .out_mode =
                {
                    .cmp00 = SDRV_EPWM_CMP_EVENT_LEVEL_LOW,
                    .cmp0_ovf = SDRV_EPWM_CMP_EVENT_LEVEL_HIGH,
                },
            //            .dma_cfg =//use in co_work bd
            //                {
            //                    .dma_enable = true,
            //                    .fifo_wml = 31,
            //                },
        },
    .chnl = SDRV_EPWM_CHANNEL_D,
};

static const sdrv_epwm_cfg_t g_sdrv_epwm_cfg_1 = {
    .base = DEVICE_BASE(EPWM1),
    // .irq = DEVICE_INTR(EPWM1_CNT_OVF), // not set to disable irq
    .irq_id = EPWM_BM_INT_EN_CNT_G0_OVF_SHIFT,

    .clk_src = SDRV_EPWM_HIGH_FREQUENCY_CLOCK,
    .clk_div = 0,

};

static const sdrv_epwm_cfg_t g_sdrv_epwm_cfg_2 = {
    .base = DEVICE_BASE(EPWM2),
    .irq = DEVICE_INTR(EPWM2_CHN_A),

    .clk_src = SDRV_EPWM_HIGH_FREQUENCY_CLOCK,
    .clk_div = 0,

};

static const sdrv_epwm_cfg_t g_sdrv_epwm_cfg_3 = {
    .base = DEVICE_BASE(EPWM3),
    .irq = DEVICE_INTR(EPWM3_CHN_A),

    .clk_src = SDRV_EPWM_HIGH_FREQUENCY_CLOCK,
    .clk_div = 0,

};

static const sdrv_epwm_cfg_t g_sdrv_epwm_cfg_4 = {
    .base = DEVICE_BASE(EPWM4),
    .irq = DEVICE_INTR(EPWM4_CHN_A),

    .clk_src = SDRV_EPWM_HIGH_FREQUENCY_CLOCK,
    .clk_div = 0,

};

/* pwm initialization */
void board_pwm_init(void)
{
    sdrv_epwm_controller_init(&g_sdrv_epwm_ctrl_1, &g_sdrv_epwm_cfg_1);
    sdrv_epwm_controller_init(&g_sdrv_epwm_ctrl_2, &g_sdrv_epwm_cfg_2);
    sdrv_epwm_controller_init(&g_sdrv_epwm_ctrl_3, &g_sdrv_epwm_cfg_3);
    sdrv_epwm_controller_init(&g_sdrv_epwm_ctrl_4, &g_sdrv_epwm_cfg_4);

    g_pwm_1 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_1);
    g_pwm_2 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_2);
    g_pwm_3 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_3);
    g_pwm_4 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_1, &epwm_cfg_4);

    g_pwm2_1 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_2, &epwm2_cfg_1);
    g_pwm2_2 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_2, &epwm2_cfg_2);
    g_pwm2_3 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_2, &epwm2_cfg_3);
    g_pwm2_4 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_2, &epwm2_cfg_4);

    g_pwm3_1 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_3, &epwm3_cfg_1);
    g_pwm3_2 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_3, &epwm3_cfg_2);
    g_pwm3_3 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_3, &epwm3_cfg_3);
    g_pwm3_4 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_3, &epwm3_cfg_4);

    g_pwm4_1 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_4, &epwm4_cfg_1);
    // use in co-work bd
    g_pwm4_2 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_4, &epwm4_cfg_2);
    // use in co-work ac
    g_pwm4_3 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_4, &epwm4_cfg_3);
    // use in co-work bd
    g_pwm4_4 = sdrv_epwm_probe(&g_sdrv_epwm_ctrl_4, &epwm4_cfg_4);
}

#endif
