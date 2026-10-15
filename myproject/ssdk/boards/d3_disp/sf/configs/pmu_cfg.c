/*
 * pmu_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_PMU

#include <types.h>
#include <stdio.h>
#include "board.h"
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include "pmu_cfg.h"

extern gpio_dev_t g_gpio_safety;

sdrv_pmu_dev_t g_sdrv_pmu_dev;

sdrv_pmu_config_t sdrv_pmu_config = {
    .base = DEVICE_BASE(PMU_CORE),
    .irq = DEVICE_INTR(PMU_CORE_PMU),
};

static const sdrv_pmu_wakeup_t wakeup_ctrl = {
    .Chn[0] = {
        .enable = PMU_WAKEUP_ENABLE,
        .sensitive = PMU_WAKEUP_EDGE_SENSITIVE,

        .polarity = 0,//0:low or failing edge active， 1:high or rising active
        .filterBypass = 1, //1:bypass
    },

    .Chn[1] = {
        .enable = PMU_WAKEUP_DISABLE,
        .sensitive = PMU_WAKEUP_LEVEL_SENSITIVE,
        .polarity = 1,
        .filterBypass = 1, //1:bypass
    },

    .wakeupAND = 0,
    .latchWakeup = 0,
    .internalWakeup = 1,
};

void board_pmu_init(void)
{
    sdrv_pmu_init(&g_sdrv_pmu_dev, &sdrv_pmu_config);

    // config pmu wakeup
    sdrv_pmu_wakeup_enable(&g_sdrv_pmu_dev, &wakeup_ctrl);
}

#endif
