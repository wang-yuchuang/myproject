/**
 * @file sdrv_btm_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx btm driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_XLI_BTM_H
#define INCLUDE_DRV_XLI_BTM_H

#include <types.h>
#include <compiler.h>

#include "driver/timer.h"
#include "sdrv_btm.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sdrv btm config */
typedef struct sdrv_btm_cfg
{
    uint32_t base;
    int irq;
    sdrv_btm_cnt_id_e tmr_id;
    sdrv_btm_lld_cnt_cfg_t tmr_cfg;
} sdrv_btm_cfg_t;

/* sdrv btm dev */
typedef struct sdrv_btm_dev
{
    timer_dev_t dev;
    uint32_t base;
    int irq;
    uint32_t cnt_per_us;
    uint32_t ns_per_tick;
    sdrv_btm_cnt_id_e tmr_id;
    sdrv_btm_lld_cnt_cfg_t tmr_cfg;
} sdrv_btm_dev_t;

/**
 * @brief sdrv btm dev probe
 *
 * @param[in] sdrv_btm sdrv btm dev
 * @param[in] cfg sdrv btm config
 * @return dev timer dev
 */
timer_dev_t *sdrv_btm_probe(sdrv_btm_dev_t *sdrv_btm, const sdrv_btm_cfg_t *cfg);

__END_CDECLS

#endif

#endif
