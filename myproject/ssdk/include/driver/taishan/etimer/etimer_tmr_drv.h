/**
 * @file sdrv_etiemr_tmr_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv timer interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_ETIMER_TMR_H
#define INCLUDE_DRV_ETIMER_TMR_H

#include <compiler.h>
#include <spinlock.h>
#include <types.h>
#include <sdrv_etimer.h>
#include "driver/timer.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

typedef enum
{
    ETIMR_LTMR_A,
    ETIMR_LTMR_B,
    ETIMR_LTMR_C,
    ETIMR_LTMR_D,
    ETIMR_LTMR_NUM
} etimer_local_tmr_id_e;

/* sdrv tmr config */
typedef struct sdrv_tmr_cfg
{
    uint32_t base;
    sdrv_etimer_irq_e irq;
    sdrv_etimer_clk_src_e clk_sel;
    uint32_t clk_frq;
    uint32_t clk_div;
} etimer_tmr_cfg_t;

/* sdrv tmr timer priv */
typedef struct sdrv_tmr_timer_priv
{
    timer_dev_t dev;
    uint32_t base;
    sdrv_etimer_cnt_id_e id;
    uint32_t cnt_per_us;
    void *priv;
} etimer_tmr_priv_t;

/* sdrv tmr dev */
typedef struct sdrv_tmr_dev
{
    timer_dev_t dev;
    uint32_t base;
    uint32_t irq;
    uint32_t cnt_per_us;
    spin_lock_t spinlock;
    etimer_tmr_priv_t etiemr_tmr[ETIMR_LTMR_NUM];
} etimer_tmr_dev_t;

/**
 * @brief etimer timer dev init
 *
 * @param[in] sdrv_tmr sdrv tmr dev
 * @param[in] cfg sdrv etimer tmr early config
 */

void etimer_tmr_init(etimer_tmr_dev_t *sdrv_tmr,
                     const etimer_tmr_cfg_t *cfg);

/**
 * @brief etimer_tmr probe
 *
 * @param[in] sdrv_tmr sdrv etimer_tmr dev
 * @param[in] id etmr id in one etmr
 * @return dev timer dev
 */
timer_dev_t *etimer_tmr_probe(etimer_tmr_dev_t *sdrv_tmr, etimer_local_tmr_id_e id);

__END_CDECLS

#endif

#endif
