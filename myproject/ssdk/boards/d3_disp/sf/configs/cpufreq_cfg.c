/*
 * cpufreq_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_CPU_FREQ

#include "cpufreq_cfg.h"

struct cpufreq g_cpufreq_high = {
#ifdef CONFIG_PART_SF
    .sf_freq  = 600000000,
#endif

#ifdef CONFIG_PART_SP
    .sp_freq  = 600000000,
#endif

#ifdef CONFIG_PART_SX
    .sx_freq  = 600000000,
#endif

    .ap_bus   = 600000000,
    .disp_bus = 600000000,
    .seip_bus = 600000000,
};

struct cpufreq g_cpufreq_medium = {
#ifdef CONFIG_PART_SF
    .sf_freq  = 300000000,
#endif

#ifdef CONFIG_PART_SP
    .sp_freq  = 300000000,
#endif

#ifdef CONFIG_PART_SX
    .sx_freq  = 300000000,
#endif

    .ap_bus   = 300000000,
    .disp_bus = 300000000,
    .seip_bus = 300000000,
};

struct cpufreq g_cpufreq_low = {
#ifdef CONFIG_PART_SF
    .sf_freq  = 24000000,
#endif

#ifdef CONFIG_PART_SP
    .sp_freq  = 24000000,
#endif

#ifdef CONFIG_PART_SX
    .sx_freq  = 24000000,
#endif

    .ap_bus   = 24000000,
    .disp_bus = 24000000,
    .seip_bus = 24000000,
};

#endif /* CONFIG_CPU_FREQ */
