/*
 * @File: motor_app.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 *
 * @Revision History:
 * ----------------
 */

#include <clock_cfg.h>
#include <driver/clk.h>
#include <driver/gpio.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/uart.h>
#include <pinmux_cfg.h>
#include <scr_cfg.h>
#include <scr_hw.h>

#include "dma_cfg.h"
#include "driver/clk.h"
#include "driver/dma.h"
#include "driver/irq.h"
#include "driver/taishan/adc/taishan_adc.h"
#include "driver/taishan/epwm/sdrv_epwm_drv.h"
#include "driver/taishan/sem/sdrv_sem_drv.h"
#include "driver/taishan/xtrg/sdrv_xtrg_drv.h"
#include "driver/timer.h"
#include "pwm_cfg.h"
#include "taishan/adc/taishan_adc_lld.h"

/*#define TEST_PERIOD_CHANGE*/
/*#define TEST_DUTY_CHANGE_CYCLE_BY_CYCLE*/
/*#define TEST_CHANGED_PERIOD_ADC_TRIG*/
/*#define TEST_SW_TRIG_ADC*/

#undef BIT
#define BIT(nr) ((uint32_t)1u << (nr))

#define SADC_REF_CLK_MAX (32 * 1000 * 1000)
#define SADC_RCLK_DIV_MAX 32
#define SADC_RCLK_DIV_MIN 2

#define M1_TMUX_SEL 0
#define M1_TMUX1_SEL 1
#define TMUX_ADC1_RST 0
#define TMUX_ADC2_RST 1
#define TMUX_ADC3_RST 2

static const uint8_t samctrl_to_clk_cnt[8] = {16, 18, 20, 24, 32, 48, 80, 143};

// PWM IDLE POLARITY
#define PWM_IDLE_FAULT_POL_LOW 1
#define PWM_IDLE_FAULT_POL_HIGH 0

// PWM EVENT ID
#define PWM_EVENT_ID 0x03
#define PWM_EVENT_ID_2 0x04

// ADC ENTRY CHANNEL CONFIGURE
#define SADC_ENTRY_SEL (PWM_EVENT_ID & 0x1F)
#define SADC_ENTRY_SEL_2 (PWM_EVENT_ID_2 & 0x1F)

// PWM IDLE FAULT_EVENT SOURCE
#define PWM_IDLE_FAULT_SOURCE SDRV_EPWM_SUB_CHANNEL_A
#define PWM_OVRCUR_FAULT_SOURCE SDRV_EPWM_SUB_CHANNEL_B
#define PWM_OVRCUR_FAULT_SOURCE_BIT EPWM_BM_CMP_A_FAULT_EVENT

// XTRG EPWM SEL
#define XTRG_EPWM_SEL 0  // 0:EPWM1 1:EPWM2 2:EPWM3 3:EPWM4
#define XTRG_EPWM2_SEL 1 // 0:EPWM1 1:EPWM2 2:EPWM3 3:EPWM4

// XTRG SSM SEL
#define XTRG_IDLE_SSM_SEL XTRG_SSM1

// XTRG SWT_TRIG SEL
#define XTRG_MOTOR1_TURN_ON_OFF XTRG_SWT1
#define XTRG_TWO_MOTOR_HOLD_SW_TRIG_SEL XTRG_SWT2

#define IDLE_HANG 0

#define M1_CHNLB_0 9      // GPIO_A9
#define M1_CHNLB_1 10     // GPIO_A10
#define M1_CHNLC_0 4      // GPIO_A4
#define M1_CHNLC_1 6      // GPIO_A6
#define M1_CHNLD_0 5      // GPIO_A5
#define M1_CHNLD_1 7      // GPIO_A7
#define M1_TMUX_TRIG 11   // GPIO_A11
#define M1_TMUX_TRIG_2 19 // GPIO_S1
#define M1_FAULT_INPUT 8  // GPIO_A8

#define DTI_NS_VAL 1000 // 1us

#define TIM_CLK_DIV 1
#define EPWM_ETIMER_SRC_CLK_MHZ 250
#define ADV_TIM_CLK_MHz EPWM_ETIMER_SRC_CLK_MHZ / TIM_CLK_DIV

#define PWM_FREQ 2000 //  2k
#define PWM_PERIOD_CYCLES                                                      \
    (uint32_t)(                                                                \
        (ADV_TIM_CLK_MHz * (uint32_t)1000000u / ((uint32_t)(PWM_FREQ))) &      \
        0xFFFFFFFF)
#define PWM_HALF_PERIOD_CYCLES (PWM_PERIOD_CYCLES / 2)
#define PWM_PERIOD_CYCLES_NS (EPWM_CNTVAL_TO_NS(PWM_PERIOD_CYCLES, 0))
#define PWM_HALF_PERIOD_CYCLES_NS (EPWM_CNTVAL_TO_NS(PWM_HALF_PERIOD_CYCLES, 0))

#define PWM_FREQ_8K 8000 //  8k
#define PWM_PERIOD_CYCLES_8K                                                   \
    (uint32_t)(                                                                \
        (ADV_TIM_CLK_MHz * (uint32_t)1000000u / ((uint32_t)(PWM_FREQ_8K))) &   \
        0xFFFFFFFF)
#define PWM_HALF_PERIOD_CYCLES_8K (PWM_PERIOD_CYCLES_8K / 2)
#define PWM_PERIOD_CYCLES_8K_NS (EPWM_CNTVAL_TO_NS(PWM_PERIOD_CYCLES_8K, 0))
#define PWM_HALF_PERIOD_CYCLES_8K_NS                                           \
    (EPWM_CNTVAL_TO_NS(PWM_HALF_PERIOD_CYCLES_8K, 0))
