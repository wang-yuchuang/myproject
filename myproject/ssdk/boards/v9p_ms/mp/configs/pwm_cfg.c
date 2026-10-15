/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_PWM

#include <types.h>
#include <board.h>

#include "pwm_cfg.h"
#include "disp_cfg.h"

pwm_instance_t g_pwm3, g_pwm4, g_pwm5;

static drv_pwm_simple_cfg_t pwm3_cfg = {
    .phy_num = 3,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_A] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI1
    .cmp_cfg[DRV_PWM_CHN_B] = {70, DRV_PWM_PHASE_POLARITY_POS},    // DSI2
};

static drv_pwm_simple_cfg_t pwm4_cfg = {
    .phy_num = 4,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_C_D_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_D] = {70, DRV_PWM_PHASE_POLARITY_POS},    // LVDS34
};

static drv_pwm_simple_cfg_t pwm5_cfg = {
    .phy_num = 5,
    .freq = 25000,
    .grp_num = DRV_PWM_CHN_A_B_WORK,
    .single_mode = DRV_PWM_CONTINUE_CMP,
    .align_mode = DRV_PWM_EDGE_ALIGN_MODE,
    .cmp_cfg[DRV_PWM_CHN_A] = {70, DRV_PWM_PHASE_POLARITY_POS},    // LVDS12
    .cmp_cfg[DRV_PWM_CHN_B] = {70, DRV_PWM_PHASE_POLARITY_POS},
};

static drv_pwm_instance_cfg_t pwm3_ins_cfg = {DEVICE_BASE(PWM3), 3};
static drv_pwm_instance_cfg_t pwm4_ins_cfg = {DEVICE_BASE(PWM4), 4};
static drv_pwm_instance_cfg_t pwm5_ins_cfg = {DEVICE_BASE(PWM5), 5};

/* pwm initialization */
void board_pwm_init(void)
{
    // init res
    drv_pwm_instance_init(&g_pwm3, &pwm3_ins_cfg);
    drv_pwm_instance_init(&g_pwm4, &pwm4_ins_cfg);
    drv_pwm_instance_init(&g_pwm5, &pwm5_ins_cfg);

    // check cfg
    panel_check_phase(&pwm3_cfg);
    panel_check_phase(&pwm4_cfg);
    panel_check_phase(&pwm5_cfg);

    // init cfg
    drv_pwm_simple_init(g_pwm3.pwmc, &pwm3_cfg, &g_pwm3.pwm_simple_ctx);
    drv_pwm_simple_init(g_pwm4.pwmc, &pwm4_cfg, &g_pwm4.pwm_simple_ctx);
    drv_pwm_simple_init(g_pwm5.pwmc, &pwm5_cfg, &g_pwm5.pwm_simple_ctx);

    // start output
    drv_pwm_cmp_out_start(g_pwm3.pwmc);
    drv_pwm_cmp_out_start(g_pwm4.pwmc);
    drv_pwm_cmp_out_start(g_pwm5.pwmc);
}

#endif
