/**
 * @file sdrv_tmr_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv timer interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_TMR_H
#define INCLUDE_DRV_SDRV_TMR_H

#include <types.h>
#include <compiler.h>
#include <spinlock.h>

#include "driver/timer.h"

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* sdrv tmr id */
typedef enum {
    SDRV_TMR_G0,
    SDRV_TMR_G1,
    SDRV_TMR_A,
    SDRV_TMR_B,
    SDRV_TMR_C,
    SDRV_TMR_D,
    SDRV_TMR_MAX_NUM
} sdrv_tmr_id_e;

/* sdrv tmr source */
typedef enum {
    SDRV_TMR_HF_CLK = 0,
    SDRV_TMR_AHF_CLK,
    SDRV_TMR_LF_CLK,
    SDRV_TMR_LP_CLK,
    SDRV_TMR_ERR_CLK
} sdrv_tmr_clk_src_e;

/* sdrv tmr config */
typedef struct sdrv_tmr_cfg {
    uint32_t                base;
    int                     irq;
    int                     ovf_irq;
    sdrv_tmr_clk_src_e      clk_sel;
    uint32_t                clk_frq;
    uint32_t                clk_div;
    bool                    cascase_mode;
    uint32_t                ptp_base;
} sdrv_tmr_cfg_t;

/* sdrv tmr func type */
typedef enum {
    SDRV_TMR_FUNC_TYPE_NONE = 0,
    SDRV_TMR_FUNC_TYPE_CMP,
    SDRV_TMR_FUNC_TYPE_CPT,
    SDRV_TMR_FUNC_TYPE_MAX
} sdrv_tmr_func_type_e;

/* sdrv tmr cmp cnt mode */
typedef enum {
    SDRV_TMR_CMP_CNT_G0 = 0,
    SDRV_TMR_CMP_CNT_LOCAL,
    SDRV_TMR_CMP_CNT_MAX
} sdrv_tmr_cmp_cntr_e;

/* sdrv tmr cmp out mode */
typedef enum {
    SDRV_TMR_CMP_POS_PULSE = 0,
    SDRV_TMR_CMP_NEG_PULSE,
    SDRV_TMR_CMP_SIG_TOGGLE,
    SDRV_TMR_CMP_LEVEL_HIGH,
    SDRV_TMR_CMP_LEVEL_LOW,
    SDRV_TMR_CMP_OUT_MODE_MAX
} sdrv_tmr_cmp_out_mode_e;

/* sdrv tmr cpt cnt mode */
typedef enum {
    SDRV_TMR_CPT_CNT_G0 = 0,
    SDRV_TMR_CPT_CNT_G0G1,
    SDRV_TMR_CPT_CNT_LOCAL,
    SDRV_TMR_CPT_CNT_MAX
} sdrv_tmr_cpt_cntr_e;

/* sdrv tmr cpt trigger mode */
typedef enum {
    SDRV_TMR_CPT_RISING_EDGE = 0,
    SDRV_TMR_CPT_FALLING_EDGE,
    SDRV_TMR_CPT_TOGGLE_EDGE,
    SDRV_TMR_CPT_EDGE_MAX
} sdrv_tmr_cpt_edge_e;

/* sdrv tmr timer priv */
typedef struct sdrv_tmr_timer_priv {
    timer_dev_t             dev;
    uint32_t                base;
    sdrv_tmr_id_e           id;
    uint32_t                cnt_per_us;
    void                    *priv;
} sdrv_tmr_timer_priv_t;

/* sdrv tmr dev */
typedef struct sdrv_tmr_dev {
    uint32_t                base;
    int                     irq;
    int                     ovf_irq;
    bool                    cascase_mode;
    uint32_t                cnt_per_us;
    spin_lock_t             spinlock;
    sdrv_tmr_timer_priv_t   timer[SDRV_TMR_MAX_NUM];
    uint32_t                ptp_base;
} sdrv_tmr_dev_t;

/**
 * @brief sdrv timer dev init
 *
 * @param[in] sdrv_tmr sdrv tmr dev
 * @param[in] cfg sdrv tmr early config
 */
void sdrv_tmr_init(sdrv_tmr_dev_t *sdrv_tmr, const sdrv_tmr_cfg_t *cfg);

/**
 * @brief sdrv timer probe
 *
 * @param[in] sdrv_tmr sdrv timer dev
 * @param[in] id tmr id in one sdrv tmr
 * @return dev timer dev
 */
timer_dev_t *sdrv_tmr_timer_probe(sdrv_tmr_dev_t *sdrv_tmr, sdrv_tmr_id_e id);

__END_CDECLS

#endif

#endif
