//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_SUPPORT_STR_MODE

#include <lp875xx.h>
#include "can_str.h"
#include "pmu_cfg.h"
#include <../drivers/kunlun/pmu/sdrv_kunlun_pmu.h>

void power_config_before_str(void)
{
    pmu_dev_t* dev_ptr = g_pmu_dev_ptr;

    pmic_config_before_str();
    canfd2_trcv_sleep();
    pmu_set_pwr_out_status(dev_ptr, PMU_PWRCTRL2, HIGH_LEVEL);
}

void power_config_before_shutdown(void)
{
    canfd2_trcv_sleep();
}
#endif
