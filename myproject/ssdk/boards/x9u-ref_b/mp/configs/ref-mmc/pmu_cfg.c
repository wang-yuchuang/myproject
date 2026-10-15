/**
 * @file pmu_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmu config
 *
 * Revision History:
 * -----------------
*/
#include <debug.h>
#include <arch/register.h>
#include <regs_base.h>
#include <types.h>

#include <driver/kunlun/pmu/sdrv_kunlun_pmu_drv.h>

#include "pmu_cfg.h"

/* global pmu device pointer  */
pmu_dev_t* g_pmu_dev_ptr;

/* static sdrv pmu device and config */
static sdrv_pmu_dev_t s_sdrv_pmu_dev;

static pmu_pwrCtrl_cfg_t s_pwrCtrl_cfg = {
    .pwrdown_delay = PMU_CTRL_DELAY_1_MS,
    .pwrup_delay = PMU_CTRL_DELAY_2_MS,
};

static pmu_pwrPinCtrl_cfg_t s_pwrCtlPin0_cfg = {
    .pinId = PMU_PWRCTRL0,
    .io_mode = PMU_PWRPINCTL_OUTPUT_MODE,
    .out_mode = PMU_PWRPIN_OUTCTL_AUTO,
    .pwrdown_delay = PMU_CTRL_DELAY_1_MS,
    .pwrup_delay = PMU_CTRL_DELAY_2_MS,
};

static pmu_pwrPinCtrl_cfg_t s_pwrCtlPin1_cfg = {
    .pinId = PMU_PWRCTRL1,
    .io_mode = PMU_PWRPINCTL_OUTPUT_MODE,
    .out_mode = PMU_PWRPIN_OUTCTL_AUTO,
    .pwrdown_delay = PMU_CTRL_DELAY_1_MS,
    .pwrup_delay = PMU_CTRL_DELAY_2_MS,
};

static pmu_pwrPinCtrl_cfg_t s_pwrCtlPin2_cfg = {
    .pinId = PMU_PWRCTRL2,
    .io_mode = PMU_PWRPINCTL_OUTPUT_MODE,
    .out_mode = PMU_PWRPIN_OUTCTL_AUTO,
    .pwrdown_delay = PMU_CTRL_DELAY_1_MS,
    .pwrup_delay = PMU_CTRL_DELAY_2_MS,
};

static pmu_pwrPinCtrl_cfg_t s_pwrCtlPin3_cfg = {
    .pinId = PMU_PWRCTRL3,
    .io_mode = PMU_PWRPINCTL_INPUT_MODE,
    .out_mode = PMU_PWRPIN_OUTCTL_AUTO,
    .pwrdown_delay = PMU_CTRL_DELAY_1_MS,
    .pwrup_delay = PMU_CTRL_DELAY_2_MS,
};

static pmu_interTrigger_cfg_t s_req_pwrDown0_cfg = {
    .chanId = 0,
    .enable = PMU_FUNC_ENABLE,
    .pol = PMU_POLARITY_LOW_ACTIVE,
};

static pmu_interTrigger_cfg_t s_req_pwrDown1_cfg = {
    .chanId = 1,
    .enable = PMU_FUNC_ENABLE,
    .pol = PMU_POLARITY_LOW_ACTIVE,
};

static pmu_extTrigger_cfg_t s_extWakeup0_cfg = {
    .chanId = 0,
    .enanle = PMU_FUNC_ENABLE,
    .pol = PMU_POLARITY_LOW_ACTIVE,
    .debounce_enable = PMU_FUNC_ENABLE,
    .debounce_delay = PMU_DEBOUNCE_1MS,
};

static pmu_extTrigger_cfg_t s_extWakeup1_cfg = {
    .chanId = 1,
    .enanle = PMU_FUNC_ENABLE,
    .pol = PMU_POLARITY_LOW_ACTIVE,
    .debounce_enable = PMU_FUNC_ENABLE,
    .debounce_delay = PMU_DEBOUNCE_1MS,
};

static pmu_extTrigger_cfg_t s_extReset0_cfg = {
    .chanId = 0,
    .enanle = PMU_FUNC_DISABLE,
    .pol = PMU_POLARITY_HIGH_ACTIVE,
    .debounce_enable = PMU_FUNC_ENABLE,
    .debounce_delay = PMU_DEBOUNCE_1MS,
};

static pmu_extTrigger_cfg_t s_extReset1_cfg = {
    .chanId = 1,
    .enanle = PMU_FUNC_DISABLE,
    .pol = PMU_POLARITY_HIGH_ACTIVE,
    .debounce_enable = PMU_FUNC_ENABLE,
    .debounce_delay = PMU_DEBOUNCE_1MS,
};

static pmu_gf_cfg_t s_pmu_gf_cfg = {
    .enables = PMU_GF_ENABLE_ALL,
    .gf_time = PMU_GF_16_CYCLE,
};

static pmu_por_cfg_t s_pmu_pro_cfg = {
    .porpd_auto = 0,
    .porpd_delay = PMU_POR_PD_DELAY_2_CYCLE,
};

static const sdrv_pmu_config_t s_sdrv_pmu_cfg = {
    .base = APB_PMU_BASE,
    .pwr_ctl = &s_pwrCtrl_cfg,
    .pwrPin_ctl = {
        &s_pwrCtlPin0_cfg,
        &s_pwrCtlPin1_cfg,
        &s_pwrCtlPin2_cfg,
        &s_pwrCtlPin3_cfg,
        NULL
    },
    .req_pwrdown = {
        &s_req_pwrDown0_cfg,
        &s_req_pwrDown1_cfg,
        NULL
    },
    .ext_wakeup = {
        &s_extWakeup0_cfg,
        &s_extWakeup1_cfg,
        NULL
    },
    .ext_reset = {
        &s_extReset0_cfg,
        &s_extReset1_cfg,
        NULL
    },
    .gf_ctl = &s_pmu_gf_cfg,
    .por_ctl = &s_pmu_pro_cfg,
};

int32_t board_pmu_init()
{
    g_pmu_dev_ptr = sdrv_pmu_probe(&s_sdrv_pmu_dev, &s_sdrv_pmu_cfg);

    return 0;
}

/* end of file pmu_cfg.c */