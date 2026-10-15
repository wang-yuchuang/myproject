/**
 * @file cpufreq.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef CPU_FREQ_H_
#define CPU_FREQ_H_

#include <types.h>

typedef struct cpufreq {
#ifdef CONFIG_PART_SF
    uint32_t sf_freq;
#endif

#ifdef CONFIG_PART_SP
    uint32_t sp_freq;
#endif

#ifdef CONFIG_PART_SX
    uint32_t sx_freq;
#endif

    uint32_t ap_bus;
    uint32_t disp_bus;
    uint32_t seip_bus;
} cpufreq_t;

int cpufreq_set(struct cpufreq *cpu_f);

int cpufreq_get(struct cpufreq *cpu_f);

#endif /* CPU_FREQ_H_ */
