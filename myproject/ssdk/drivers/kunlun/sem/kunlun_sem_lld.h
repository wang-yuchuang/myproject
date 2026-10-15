/*
 * sem.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __SEM_H__
#define __SEM_H__

#include <stdbool.h>

#define IO_LEVEL_CFG_NUM     2U

/*
 * SEM1 and SEM2 are the same IPs with same input signals. The
 * differences are:
 *  1) SEM2 does not have monitor input.
 *  2) SEM2 has only one output interrupt: SEM_INTR_CPU.
 *  3) SEM2 does not control the global enable signal of EIC modules.
 */
enum sem {
    SEM1 = 0,
    SEM2,
};

/* SEM interrupt output targets. */
enum sem_intr {
    SEM_INTR_PAD0 = 0, /* PAD0, recoverable error. */
    SEM_INTR_PAD1,     /* PAD1, fatal error. */
    SEM_INTR_CPU,      /* GIC 1~5. */
    SEM_INTR_SYS,      /* global reset. */
    SEM_INTR_DC0,      /* DC output 0. */
    SEM_INTR_DC1,      /* DC output 1. */
    SEM_INTR_DC2,      /* DC output 2. */
    SEM_INTR_DC3,      /* DC output 3. */
};

typedef enum {
    SEM_OUTPUT_PAD0 = 0,
    SEM_OUTPUT_PAD1,
    SEM_OUTPUT_CPU,
    SEM_OUTPUT_SYS,
    SEM_OUTPUT_DC0,
    SEM_OUTPUT_DC1,
    SEM_OUTPUT_DC2,
    SEM_OUTPUT_DC3,
} sdrv_sem_int_out_e;

/* sem interrupt out selection for(HW0~4:rstgen/dc/iomux/xtrg/sf_pll) */
typedef struct sdrv_sem_int_sel_cfg {
    uint8_t rstgen;
    uint8_t dc;
    uint8_t iomux;
    uint8_t xtrg;
    uint8_t sf_pll;
} sdrv_sem_int_sel_cfg_t;

/* sem error configuration(error divider ration/wave A/B selection /fatal error) */
typedef struct sdrv_sem_err_cfg {
    uint8_t err_divider;
    uint8_t err_wave_sel;
    uint8_t fatal_highlow_sel;
} sdrv_sem_err_cfg_t;

typedef struct sdrv_sem_io_wave_cfg {
    uint8_t level_cfg[IO_LEVEL_CFG_NUM];
    uint8_t level_en[IO_LEVEL_CFG_NUM];
} sdrv_sem_io_wave_cfg_t;

enum sem_signal;
enum sem_monitor_sig;

/* Generated SEM input signals, and monitor signals. */
#if CONFIG_BOARD_X9SP
#include "taihang_sem_lld_hw.h"
#else
#include "kunlun_sem_lld_hw.h"
#endif

void sem_enable_intr(enum sem sem, enum sem_intr intr, bool enable);
void sem_trigger_intr(enum sem sem, enum sem_intr intr);
bool sem_signal_status(enum sem sem, enum sem_signal input);
void sem_map_signal(enum sem sem, enum sem_signal input,
                    enum sem_intr intr, bool enable);

void sem_mon_enable(bool enable);
void sem_mon_enable_intr(enum sem_intr intr, bool enable);
void sem_mon_compare_sig(enum sem_monitor_sig signal,
                         bool compare, bool target);

void sem_enable_eic(bool enable);

#endif
