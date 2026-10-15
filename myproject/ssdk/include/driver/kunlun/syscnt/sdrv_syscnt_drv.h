/**
 * @file sdrv_syscnt_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv syscnt interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_SYSCNT_H
#define INCLUDE_DRV_SDRV_SYSCNT_H

#include <types.h>
#include <compiler.h>

#include "driver/timer.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sdrv syscnt config */
typedef struct sdrv_syscnt_cfg {
    uint32_t                ro_base;
    uint32_t                rw_base;
    uint32_t                clk_frq;
} sdrv_syscnt_cfg_t;

/* sdrv syscnt dev */
typedef struct sdrv_syscnt_dev {
    timer_dev_t             dev;
    uint32_t                ro_base;
    uint32_t                rw_base;
    uint32_t                cnt_per_us;
} sdrv_syscnt_dev_t;

/**
 * @brief sdrv syscnt probe
 *
 * @param[in] syscnt_dev syscnt dev
 * @param[in] cfg syscnt config
 * @return dev timer dev
 */
timer_dev_t *sdrv_syscnt_probe(sdrv_syscnt_dev_t *syscnt_dev,
                               const sdrv_syscnt_cfg_t *cfg);

__END_CDECLS

#endif

#endif
