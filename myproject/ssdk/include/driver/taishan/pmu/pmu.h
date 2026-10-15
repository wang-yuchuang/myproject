/**
 * @file pmu.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef PMU_H
#define PMU_H

#include <cmsis_os2.h>
#include <types.h>

#include "sdrv_pmu.h"

typedef struct pmu_dev pmu_dev_t;

typedef struct pmu_bc_ops {
    void (*init)(pmu_dev_t *device, const sdrv_pmu_bc_ctrl_t *cfg);
    void (*deinit)(pmu_dev_t *device);
    void (*filter)(pmu_dev_t *device, const sdrv_pmu_filter_t *filter);
    void (*window)(pmu_dev_t *device, const sdrv_pmu_bc_window_t *window);
} pmu_bc_ops_t;

typedef struct pmu_io_ops {
    uint32_t (*enable)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                       uint8_t enable);
    uint32_t (*delay)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                      uint8_t delay, uint8_t delay_adj);
    uint32_t (*auto_set)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode,
                         uint8_t id, uint8_t mode);
    uint32_t (*polarity)(pmu_dev_t *device, uint8_t id, uint8_t polarity);
    uint32_t (*target)(pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode, uint8_t id,
                       uint8_t target);
} pmu_io_ops_t;

typedef struct _pmu_drv_ops {
    pmu_bc_ops_t bc_button;
    pmu_io_ops_t pwr_ctrl;
    pmu_io_ops_t pwr_on;
} pmu_ops_t;

/**
 * @brief PMU low level device.
 */
typedef struct pmu_dev {
    paddr_t base;
    int irq;
    const pmu_ops_t *ops;
    void *priv;
} pmu_dev_t;

#endif /* PMU_H */
