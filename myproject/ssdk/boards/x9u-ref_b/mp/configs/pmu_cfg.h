/**
 * @file pmu_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmu config header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_PMU_CFG_H
#define INCLUDE_PMU_CFG_H

#include <driver/kunlun/pmu/sdrv_kunlun_pmu_drv.h>
#include <driver/kunlun/pmu/pmu.h>

extern pmu_dev_t* g_pmu_dev_ptr;

int32_t board_pmu_init();

#endif /* INCLUDE_PMU_CFG_H */
/* end of header file pmu_cfg.h */