/*
 * core_portme.c — CoreMark porting layer for SemiDrive D9360 R5 (ssdk / FreeRTOS)
 *
 * Timing  : ARMv7-R PMU cycle counter, prescaler /64 (CONFIG_ARM_WITH_PMU).
 *           start_time() zeroes & starts the counter, stop_time() stops it and
 *           latches the elapsed cycles — no u32 wraparound for single runs
 *           under ~687s @400MHz (u32/64-prescale limit).
 * printf  : ee_printf() maps to newlib printf (HAS_PRINTF=1 in core_portme.h).
 * Memory  : MEM_STATIC — no malloc used by the benchmark.
 *
 * Derived from EEMBC CoreMark barebones port (Apache-2.0, (c) EEMBC).
 */
#include "coremark.h"
#include "core_portme.h"

#include <stdio.h>

#include <arch/pmu.h>
#include <arch/cache.h>
#include <arch/register.h> /* arm_read_sctlr */

#if VALIDATION_RUN
volatile ee_s32 seed1_volatile = 0x3415;
volatile ee_s32 seed2_volatile = 0x3415;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PERFORMANCE_RUN
volatile ee_s32 seed1_volatile = 0x0;
volatile ee_s32 seed2_volatile = 0x0;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PROFILE_RUN
volatile ee_s32 seed1_volatile = 0x8;
volatile ee_s32 seed2_volatile = 0x8;
volatile ee_s32 seed3_volatile = 0x8;
#endif
volatile ee_s32 seed4_volatile = ITERATIONS;
volatile ee_s32 seed5_volatile = 0;

/* Porting : Timing functions (PMU cycle counter, prescaler /64) */
CORETIMETYPE
barebones_clock()
{
    return (CORETIMETYPE)pmu_get_cycle_cntr();
}

#define GETMYTIME(_t)              (*_t = barebones_clock())
#define MYTIMEDIFF(fin, ini)       ((fin) - (ini))
#define TIMER_RES_DIVIDER          1
#define SAMPLE_TIME_IMPLEMENTATION 1
/* EE_TICKS_PER_SEC comes from core_portme.h (PMU cycle counter, /64 prescale) */

/** Target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;

/* Function : start_time — reset & start PMU cycle counter */
void
start_time(void)
{
    pmu_stop_clear_cycle_cntr();
    pmu_start_cycle_cntr(true); /* div64 prescale, avoids early u32 overflow */
    GETMYTIME(&start_time_val);
}

/* Function : stop_time — stop PMU cycle counter and latch value */
void
stop_time(void)
{
    pmu_stop_cycle_cntr();
    GETMYTIME(&stop_time_val);
}

/* Function : get_time — elapsed PMU ticks (/64 prescale) */
CORE_TICKS
get_time(void)
{
    CORE_TICKS elapsed
        = (CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
    return elapsed;
}

/* Function : time_in_secs — convert PMU ticks to seconds */
secs_ret
time_in_secs(CORE_TICKS ticks)
{
    secs_ret retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
    return retval;
}

ee_u32 default_num_contexts = 1;


/* raw cycle counter snapshot (PMU, /64) for cross-check prints */
unsigned int
coremark_port_pmu_raw_ticks(void)
{
    return (unsigned int)pmu_get_cycle_cntr();
}

/* current I/D cache enable state (SCTLR.I bit12 / SCTLR.C bit2) */
unsigned int
coremark_port_cache_state(void)
{
    unsigned int sctlr = (unsigned int)arm_read_sctlr();
    unsigned int st = 0;
    if (sctlr & (1u << 12))
        st |= 0x2; /* icache on */
    if (sctlr & (1u << 2))
        st |= 0x1; /* dcache on */
    return st;
}

/* Function : portable_init — enable PMU, print environment, sanity checks */
void
portable_init(core_portable *p, int *argc, char *argv[])
{
    (void)argc;
    (void)argv;

    pmu_enable();

    if (sizeof(ee_ptr_int) != sizeof(ee_u8 *))
    {
        ee_printf(
            "ERROR! Please define ee_ptr_int to a type that holds a "
            "pointer!\n");
    }
    if (sizeof(ee_u32) != 4)
    {
        ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
    }

    ee_printf("CoreMark env: %s | CPU=%s %s | EE_TICKS_PER_SEC=%d (PMU/64)\n",
              MEM_LOCATION, COMPILER_VERSION, COMPILER_FLAGS,
              (int)EE_TICKS_PER_SEC);
#if defined(CONFIG_COREMARK_CPU_FREQ) && (CONFIG_COREMARK_CPU_FREQ > 0)
    ee_printf("CoreMark env: assumed CPU freq = %d Hz "
              "(verify: PMU secs must match OS tick secs)\n",
              (int)CONFIG_COREMARK_CPU_FREQ);
#endif

    p->portable_id = 1;
}

/* Function : portable_fini */
void
portable_fini(core_portable *p)
{
    p->portable_id = 0;
}
