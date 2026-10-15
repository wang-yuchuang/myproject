/*
 * core_shm_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef CORE_SHM_CFG_H
#define CORE_SHM_CFG_H

#ifdef CONFIG_CORE_SHM

#include <types.h>

struct core_shm_param;

extern struct core_shm_param *g_coreshm;

struct core_shm_param {
/*  #define CORE_SP0_RUNNING  0x1U
    #define CORE_SP1_RUNNING  0x2U
    #define CORE_SX0_RUNNING  0x4U
    #define CORE_SX1_RUNNING  0x8U  */
    uint32_t running_core;

    /* Service CU */
    uint32_t coreid;
    uint32_t uart_base;
    uint32_t uart_irq;

    /* PM */
    uint32_t pm_sp0;
    uint32_t pm_sp1;
    uint32_t pm_sx0;
    uint32_t pm_sx1;
};

extern void board_coreshm_init(void);

#endif

#endif
