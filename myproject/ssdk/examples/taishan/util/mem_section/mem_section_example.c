/*
 * mem_section_example.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Example to place code and data in memory sections.
 *
 * Revision History:
 * -----------------
 */
#include <arch/cache.h>
#include <arch/pmu.h>
#include <param.h>
#include <stdio.h>
#include <string.h>

#include "CLI_type.h"

/* TCM data section */
__SECTION(".tcmdata")
static volatile uint32_t g_tcmdata[8192];

/* .data section */
static volatile uint32_t g_data[8192];

/* TCM code section */
__SECTION(".tcmcode") __NO_INLINE
static uint32_t tcm_func(volatile uint32_t *data, int count)
{
    volatile uint32_t sum = 0;
    int index = 0;

    memset((void *)data, 0, sizeof(*data) * count);

    for (int i = 0; i < 100000; i++) {
        data[index] = i;
        index = (index + 8) % count;  /* d-cache miss */
        sum += data[index];
    }

    return sum;
}

/* .text section */
__NO_INLINE
static uint32_t func(volatile uint32_t *data, int count)
{
    volatile uint32_t sum = 0;
    int index = 0;

    memset((void *)data, 0, sizeof(*data) * count);

    for (int i = 0; i < 100000; i++) {
        data[index] = i;
        index = (index + 8) % count;  /* d-cache miss */
        sum += data[index];
    }

    return sum;
}

static int mem_section_test(int argc, char *argv[])
{
    struct testcase {
        const char *desc;
        uint32_t (*func)(volatile uint32_t *, int);
        volatile uint32_t *data;
        int count;
    };

    struct testcase cases[] = {
        { "tcm func, tcm data", tcm_func, g_tcmdata, ARRAY_SIZE(g_tcmdata) },
        { "tcm func, ram data", tcm_func, g_data, ARRAY_SIZE(g_data) },
        { "ram func, tcm data", func, g_tcmdata, ARRAY_SIZE(g_tcmdata) },
        { "ram func, ram data", func, g_data, ARRAY_SIZE(g_data) },
    };

    pmu_enable();
    // arch_disable_cache(UCACHE);

    for (size_t i = 0; i < ARRAY_SIZE(cases); i++) {
        struct testcase *c = &cases[i];
        uint32_t cycles, icache_miss, dcache_miss;
        uint32_t sum;

        pmu_stop_clear_cycle_cntr();
        pmu_stop_clear_evt_cntr(PMU_EVT_COUNTER1);
        pmu_stop_clear_evt_cntr(PMU_EVT_COUNTER2);

        pmu_start_cycle_cntr(false);
        pmu_start_evt_cntr(PMU_EVT_COUNTER1, PMU_EVT_ICACHE_MISS);
        pmu_start_evt_cntr(PMU_EVT_COUNTER2, PMU_EVT_DCACHE_MISS);

        sum = c->func(c->data, c->count);

        cycles = pmu_get_cycle_cntr();
        icache_miss = pmu_get_evt_cntr(PMU_EVT_COUNTER1);
        dcache_miss = pmu_get_evt_cntr(PMU_EVT_COUNTER2);

        printf("case %s, sum %d: cycles %d, i-cache-miss %d, d-cache-miss %d\n",
            c->desc, sum, cycles, icache_miss, dcache_miss);
    }

    // arch_enable_cache(UCACHE);

    return 0;
}

CLI_CMD("mem_section_test", "\r\nmem_section_test:\r\n Memory section examples\n",
        mem_section_test);
