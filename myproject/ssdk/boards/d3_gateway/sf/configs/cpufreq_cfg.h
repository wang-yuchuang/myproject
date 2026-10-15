/*
 * cpufreq_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef CPUFREQ_CFG_H_
#define CPUFREQ_CFG_H_

#ifdef CONFIG_CPU_FREQ

#include <cpufreq.h>

extern struct cpufreq g_cpufreq_high;
extern struct cpufreq g_cpufreq_medium;
extern struct cpufreq g_cpufreq_low;

#endif /* CONFIG_CPU_FREQ */

#endif /* CPUFREQ_CFG_H_ */
