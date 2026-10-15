/*
 * pmu_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARD_PMU_CFG_H
#define BOARD_PMU_CFG_H

#ifdef CONFIG_PMU

#include <driver/taishan/pmu/sdrv_pmu_drv.h>

extern sdrv_pmu_dev_t g_sdrv_pmu_dev;

void board_pmu_init(void);

#endif

#endif
