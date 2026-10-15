/**
 * @file xli_ttc_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx ttc driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_XLI_TTC_H
#define INCLUDE_DRV_XLI_TTC_H

#include <types.h>
#include <compiler.h>

#include "driver/timer.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* xli ttc config */
typedef struct xli_ttc_cfg {
    uint32_t                base;
    int                     irq;
    uint32_t                clk_frq;
} xli_ttc_cfg_t;

/* xli ttc dev */
typedef struct xli_ttc_dev {
    timer_dev_t             dev;
    uint32_t                base;
    int                     irq;
    uint32_t                clk_frq;
    uint32_t                cnt_per_us;
} xli_ttc_dev_t;

/**
 * @brief xli ttc dev probe
 *
 * @param[in] xli_ttc xli ttc dev
 * @param[in] cfg xli ttc config
 * @return dev timer dev
 */
timer_dev_t *xli_ttc_probe(xli_ttc_dev_t *xli_ttc, const xli_ttc_cfg_t *cfg);

__END_CDECLS

#endif

#endif
