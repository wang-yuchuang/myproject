/************************************************************************
 * Copyright(c) 2026  Semidrive
 * All rights reserved.
 *
 * coremark_app.c — CoreMark (EEMBC 1.0) benchmark CLI for D9360 R5
 *
 * Purpose:
 *   Measure Cortex-R5 (SEC domain) CPU performance in the two memory
 *   configurations of the bmu05 board:
 *     - xip project : code executes in place from OSPI2 NOR
 *     - ref project : code executes from OCRAM (RAM)
 *   Cache state can be toggled at runtime from the CLI, so a single image
 *   covers the {cache-on, cache-off} x {xip, ref} matrix.
 *
 * Usage (from R-core FreeRTOS CLI):
 *   coremark                    -> run with default ITERATIONS, cache as-is
 *   coremark <iterations>       -> run N iterations (e.g. 2000 for cache-off)
 *   coremark <iterations> nocache -> disable I-cache during the run
 *   (D-cache kept on: arch_disable_cache(UCACHE) disables D-cache before
 *   flushing, dropping dirty OCRAM stack/data -> crash; I-cache off is the
 *   meaningful XIP fetch-cost axis, D-cache impact on CoreMark is minor)
 *
 * Timing:
 *   Official CoreMark result uses the ARMv7-R PMU cycle counter (prescaler
 *   /64) with CONFIG_COREMARK_CPU_FREQ from defconfig. An independent 1 kHz
 *   FreeRTOS tick measurement is printed after the run for cross-checking:
 *   if PMU seconds != OS tick seconds, adjust CONFIG_COREMARK_CPU_FREQ.
 *
 * Note: the run blocks the CLI thread for >=10s (by design, CoreMark rule).
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>

#include <CLI.h>

#include <arch/cache.h>

#include "coremark.h"
#include "core_portme.h"

// seed4_volatile defined in core_portme.c (SEED_VOLATILE method)
extern volatile ee_s32 seed4_volatile;

/* official core_main.c main(), renamed at compile time (-Dmain=coremark_main) */
extern MAIN_RETURN_TYPE coremark_main(void);

#define COREMARK_MIN_ITERS 1u

static int coremark_run_cmd(int argc, char *argv[])
{
    unsigned int iterations = 0;
    int nocache = 0;
    unsigned int tick_before, tick_after, tick_hz;
    unsigned int cache_state_before;
    int i;

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "nocache") == 0) {
            nocache = 1;
        } else {
            iterations = (unsigned int)strtoul(argv[i], NULL, 0);
        }
    }

    if (iterations == 0) {
        iterations = (unsigned int)seed4_volatile; /* ITERATIONS default */
    }
    if (iterations < COREMARK_MIN_ITERS) {
        printf("[CoreMark] invalid iterations %u\n", iterations);
        printf("usage: coremark [iterations] [nocache]\n");
        return 0; /* CLI: MUST return 0 (non-zero = repeat, infinite loop) */
    }
    if (nocache) {
        iterations = iterations > 100000u ? 100000u : iterations;
    }

    seed4_volatile = (volatile ee_s32)iterations;

    printf("[CoreMark] start: iter=%u cache=%s mode=%s\n",
           iterations, nocache ? "OFF" : "unchanged", MEM_LOCATION);
    cache_state_before = coremark_port_cache_state();

    if (nocache) {
        arch_disable_cache(ICACHE);
        printf("[CoreMark] I-cache disabled (SCTLR cache bits=0x%x)\n",
               coremark_port_cache_state());
    }

    tick_before = (unsigned int)osKernelGetTickCount();
    tick_hz = (unsigned int)osKernelGetTickFreq();

    coremark_main();

    tick_after = (unsigned int)osKernelGetTickCount();

    if (nocache) {
        arch_enable_cache(ICACHE);
        printf("[CoreMark] I-cache re-enabled (SCTLR cache bits=0x%x)\n",
               coremark_port_cache_state());
    }

    /* independent OS tick cross-check (1 kHz): compare against the
     * PMU-based "Total time (secs)" printed by coremark_main() */
    printf("[CoreMark] OS tick check: %u ticks @ %u Hz = %.3f s "
           "(PMU secs above; mismatch => fix CONFIG_COREMARK_CPU_FREQ)\n",
           tick_after - tick_before, tick_hz,
           (double)(tick_after - tick_before) / (double)tick_hz);
    printf("[CoreMark] done (cache bits before=0x%x now=0x%x)\n",
           cache_state_before, coremark_port_cache_state());

    return 0;
}

CLI_CMD("coremark", "coremark [iterations] [nocache]: EEMBC CoreMark 1.0 benchmark",
        coremark_run_cmd);
