/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/

#include <stdio.h>
#include <stdlib.h>

#include "CLI_type.h"
#include "coremark.h"
#include "driver/arch_timer.h"

#ifdef CONFIG_ENABLE_CYCLE_COUNTER
#include "arch/pmu.h"
#endif

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
volatile ee_s32 seed4_volatile = CONFIG_ITERATIONS;
volatile ee_s32 seed5_volatile = 0;
/* Porting : Timing functions
        How to capture time and convert to seconds must be ported to whatever is
   supported by the platform. e.g. Read value from on board RTC, read value from
   cpu clock cycles performance counter etc. Sample implementation for standard
   time.h and windows.h definitions included.
*/
/* Define : TIMER_RES_DIVIDER
        Divider to trade off timer resolution and total time that can be
   measured.

        Use lower values to increase resolution, but make sure that overflow
   does not occur. If there are issues with the return value overflowing,
   increase this value.
        */
#define CORETIMETYPE               uint64_t
#define MYTIMEDIFF(fin, ini)       ((fin) - (ini))

#ifdef CONFIG_ENABLE_CYCLE_COUNTER
  /* Use ARM cycle counter. 800 MHz */
  #define CORE_FREQ                  (800 * 1024 * 1024)
  #define TIMER_RES_DIVIDER          64
  #define EE_TICKS_PER_SEC           (CORE_FREQ / TIMER_RES_DIVIDER) /* 800MHz / 64 */
#else
  /* Use global timer count (us). */
  #define EE_TICKS_PER_SEC           (1000 * 1000)
#endif

/** Define Host specific (POSIX), or target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;

/* Function : start_time
        This function will be called right before starting the timed portion of
   the benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or zeroing some system parameters - e.g. setting the cpu clocks
   cycles to 0.
*/
void
start_time(void)
{
#ifdef CONFIG_ENABLE_CYCLE_COUNTER
    // arch_irq_disable();

    pmu_stop_clear_cycle_cntr();
    pmu_stop_clear_evt_cntr(PMU_EVT_COUNTER1);
    pmu_stop_clear_evt_cntr(PMU_EVT_COUNTER2);
    pmu_stop_clear_evt_cntr(PMU_EVT_COUNTER3);

    start_time_val = 0ull;

    pmu_start_cycle_cntr(true);
    pmu_start_evt_cntr(PMU_EVT_COUNTER1, PMU_EVT_INSTRUCTION_EXECUTED);
    pmu_start_evt_cntr(PMU_EVT_COUNTER2, PMU_EVT_ICACHE_MISS);
    pmu_start_evt_cntr(PMU_EVT_COUNTER3, PMU_EVT_DCACHE_MISS);
#else
    start_time_val = arch_timer_get_current();
#endif
}
/* Function : stop_time
        This function will be called right after ending the timed portion of the
   benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or other system parameters - e.g. reading the current value of
   cpu cycles counter.
*/
void
stop_time(void)
{
#ifdef CONFIG_ENABLE_CYCLE_COUNTER
    pmu_stop_cycle_cntr();
    pmu_stop_evt_cntr(PMU_EVT_COUNTER1);
    pmu_stop_evt_cntr(PMU_EVT_COUNTER2);
    pmu_stop_evt_cntr(PMU_EVT_COUNTER3);

    stop_time_val = (CORETIMETYPE)pmu_get_cycle_cntr();
    uint32_t inst_nr = pmu_get_evt_cntr(PMU_EVT_COUNTER1);
    uint32_t icache_miss_nr = pmu_get_evt_cntr(PMU_EVT_COUNTER2);
    uint32_t dcache_miss_nr = pmu_get_evt_cntr(PMU_EVT_COUNTER3);

    // arch_irq_enable();

    printf("CoreMark statistics(counter might overflow for long iterations!): "
        "%lld cycles, %d instructions, %d i-cache miss, %d d-cache miss\n",
        stop_time_val, inst_nr, icache_miss_nr, dcache_miss_nr);
#else
    stop_time_val = arch_timer_get_current();
#endif
}
/* Function : get_time
        Return an abstract "ticks" number that signifies time on the system.

        Actual value returned may be cpu cycles, milliseconds or any other
   value, as long as it can be converted to seconds by <time_in_secs>. This
   methodology is taken to accommodate any hardware or simulated platform. The
   sample implementation returns millisecs by default, and the resolution is
   controlled by <TIMER_RES_DIVIDER>
*/
CORE_TICKS
get_time(void)
{
    CORE_TICKS elapsed
        = (CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
    return elapsed;
}
/* Function : time_in_secs
        Convert the value returned by get_time to seconds.

        The <secs_ret> type is used to accommodate systems with no support for
   floating point. Default implementation implemented by the EE_TICKS_PER_SEC
   macro above.
*/
secs_ret
time_in_secs(CORE_TICKS ticks)
{
    secs_ret retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
    return retval;
}

ee_u32 default_num_contexts = 1;

/* Function : portable_init
        Target specific initialization code
        Test for some common mistakes.
*/
void
portable_init(core_portable *p, int *argc, char *argv[])
{
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
    p->portable_id = 1;
}
/* Function : portable_fini
        Target specific final code
*/
void
portable_fini(core_portable *p)
{
    p->portable_id = 0;
}

/* Function : portable_fini
        Target specific final code
*/
extern int coremark_main(void);

static int coremark_test(int argc, char *argv[])
{
    coremark_main();
    return 0;
}

CLI_CMD("coremark", "start coremark test", coremark_test);
