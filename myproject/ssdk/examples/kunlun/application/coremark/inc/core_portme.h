/*
 * core_portme.h — CoreMark porting layer for SemiDrive D9360 R5 (ssdk / FreeRTOS)
 *
 * Target : Cortex-R5 (SEC domain), newlib libc, CMSIS-RTOS2 FreeRTOS
 * Timing : ARMv7-R PMU cycle counter (CONFIG_ARM_WITH_PMU), prescaler /64
 * Memory : MEM_STATIC (no malloc), data block in .bss
 *
 * Derived from EEMBC CoreMark barebones port (Apache-2.0, (c) EEMBC).
 */

#ifndef CORE_PORTME_H
#define CORE_PORTME_H

#include <stddef.h>  /* size_t */

/************************/
/* Data types and settings */
/************************/

/* Platform supports floating point (FPU enabled, newlib printf %f) */
#ifndef HAS_FLOAT
#define HAS_FLOAT 1
#endif

/* No POSIX time.h */
#ifndef HAS_TIME_H
#define HAS_TIME_H 0
#endif
#ifndef USE_CLOCK
#define USE_CLOCK 0
#endif
/* stdio.h needed because coremark.h maps ee_printf -> printf (HAS_PRINTF=1) */
#ifndef HAS_STDIO
#define HAS_STDIO 1
#endif
/* Use target printf for ee_printf() */
#ifndef HAS_PRINTF
#define HAS_PRINTF 1
#endif

/* Definitions : COMPILER_VERSION, COMPILER_FLAGS, MEM_LOCATION */
#ifndef COMPILER_VERSION
#ifdef __GNUC__
#define COMPILER_VERSION "GCC"__VERSION__
#else
#define COMPILER_VERSION "Please put compiler version here (e.g. gcc 4.1)"
#endif
#endif

/* Keep in sync with rules.mk MODULE_OPTFLAGS / MODULE_COMPILEFLAGS */
#ifndef COMPILER_FLAGS
#define COMPILER_FLAGS "-O2 -fno-builtin"
#endif

#ifndef MEM_LOCATION
#if defined(CONFIG_XIP_MODE)
#define MEM_LOCATION "XIP_NOR_code_OCRAM_data"
#else
#define MEM_LOCATION "OCRAM_code_data"
#endif
#endif

/* Data Types */
typedef signed short   ee_s16;
typedef unsigned short ee_u16;
typedef signed int     ee_s32;
typedef double         ee_f32;
typedef unsigned char  ee_u8;
typedef unsigned int   ee_u32;
typedef ee_u32         ee_ptr_int;
typedef size_t         ee_size_t;
#ifndef NULL
#define NULL ((void *)0)
#endif

/* align_mem : align offset to 32b value */
#define align_mem(x) (void *)(4 + (((ee_ptr_int)(x)-1) & ~3))

/* Configuration : CORE_TICKS — PMU cycle counter /64 */
#define CORETIMETYPE ee_u32
typedef ee_u32 CORE_TICKS;

/* Configuration : SEED_METHOD */
#ifndef SEED_METHOD
#define SEED_METHOD SEED_VOLATILE
#endif

/* Configuration : MEM_METHOD — static block, no heap dependency */
#ifndef MEM_METHOD
#define MEM_METHOD MEM_STATIC
#endif

/* Configuration : MULTITHREAD — single context */
#ifndef MULTITHREAD
#define MULTITHREAD 1
#define USE_PTHREAD 0
#define USE_FORK    0
#define USE_SOCKET  0
#endif

/* Configuration : MAIN_HAS_NOARGC / NORETURN
 * The official core_main.c main() is renamed to coremark_main() at compile
 * time (-Dmain=coremark_main) so the app can call it from a CLI command.
 */
#ifndef MAIN_HAS_NOARGC
#define MAIN_HAS_NOARGC 1
#endif
#ifndef MAIN_HAS_NORETURN
#define MAIN_HAS_NORETURN 0
#endif

/* Default number of iterations.
 * cache-on  (~400MHz R5, ~1000+ iter/s): 20000 iters ~= 15-20s (>=10s rule)
 * cache-off (XIP NOR, slow): pass a smaller number on the CLI, e.g.
 *   "coremark 2000 nocache"
 * Override on the command line: "coremark <iterations> [nocache]"
 */
#ifndef ITERATIONS
#define ITERATIONS 20000
#endif

/* Run type: performance run (seeds 0,0,0x66) */
#ifndef PERFORMANCE_RUN
#define PERFORMANCE_RUN 1
#endif

/* EE_TICKS_PER_SEC: PMU cycle counter runs at CPU freq / 64 (prescaler D bit).
 * CONFIG_COREMARK_CPU_FREQ comes from the project defconfig (int type).
 * NOTE: verify on target — coremark output prints both the PMU-derived
 * seconds and the independent 1kHz OS tick seconds; if they diverge, adjust
 * CONFIG_COREMARK_CPU_FREQ to match (real_freq = configured * tick_s / pmu_s).
 */
#ifndef EE_TICKS_PER_SEC
#if defined(CONFIG_COREMARK_CPU_FREQ) && (CONFIG_COREMARK_CPU_FREQ > 0)
#define EE_TICKS_PER_SEC ((CONFIG_COREMARK_CPU_FREQ) / 64)
#else
#define EE_TICKS_PER_SEC (400000000 / 64) /* default: assume 400 MHz */
#endif
#endif

/* Variable : default_num_contexts (single context port, must be 1) */
extern ee_u32 default_num_contexts;

typedef struct CORE_PORTABLE_S
{
    ee_u8 portable_id;
} core_portable;

/* target specific init/fini */
void portable_init(core_portable *p, int *argc, char *argv[]);
void portable_fini(core_portable *p);

/* app helper: current I/D cache enable state (bit1=icache, bit0=dcache) */
unsigned int coremark_port_cache_state(void);

#endif /* CORE_PORTME_H */
