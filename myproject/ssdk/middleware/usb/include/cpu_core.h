/*
*********************************************************************************************************
*                                               uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                    Copyright 2004-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           NOTICE
*
*   Semidrive modified this file to adapt it for ssdk platform.
*   The modifications are only intended for use with Semidrive chips.
*   Copyright of all the modifications belongs to Semidrive Semiconductor.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           CORE CPU MODULE
*
* Filename : cpu_core.h
* Version  : V1.32.01
*********************************************************************************************************
* Note(s)  : (1) Assumes the following versions (or more recent) of software modules are included in
*                the project build :
*
*                (a) uC/LIB V1.38.02
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This core CPU header file is protected from multiple pre-processor inclusion through use of
*               the  core CPU module present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  CPU_CORE_MODULE_PRESENT                                /* See Note #1.                                         */
#define  CPU_CORE_MODULE_PRESENT


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*
* Note(s) : (1) CPU-configuration software files are located in the following directories :
*
*               (a) \<Your Product Application>\cpu_cfg.h
*
*               (b) (1) \<CPU-Compiler Directory>\cpu_*.*
*                   (2) \<CPU-Compiler Directory>\<cpu>\<compiler>\cpu*.*
*
*                       where
*                               <Your Product Application>      directory path for Your Product's Application
*                               <CPU-Compiler Directory>        directory path for common CPU-compiler software
*                               <cpu>                           directory name for specific processor (CPU)
*                               <compiler>                      directory name for specific compiler
*
*           (2) NO compiler-supplied standard library functions SHOULD be used.
*
*               (a) Standard library functions are implemented in the custom library module(s) :
*
*                       \<Custom Library Directory>\lib_*.*
*
*                           where
*                                   <Custom Library Directory>      directory path for custom library software
*
*           (3) Compiler MUST be configured to include as additional include path directories :
*
*               (a) '\<Your Product Application>\' directory                            See Note #1a
*
*               (b) (1) '\<CPU-Compiler Directory>\'                  directory         See Note #1b1
*                   (2) '\<CPU-Compiler Directory>\<cpu>\<compiler>\' directory         See Note #1b2
*
*               (c) '\<Custom Library Directory>\' directory                            See Note #2a
*********************************************************************************************************
*/

#include  <debug.h>
#include  <cpu.h>
#include  <lib_def.h>
#include  <cpu_cfg.h>


/*
*********************************************************************************************************
*                                          CPU CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure CPU parameters.  DO NOT MODIFY.
*
*           (2) CPU timestamp timer feature is required for :
*
*               (a) CPU timestamps
*               (b) CPU interrupts disabled time measurement
*
*               See also 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                      & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1'.
*********************************************************************************************************
*/

#ifdef   CPU_CFG_TS_EN
#undef   CPU_CFG_TS_EN
#endif


#if    ((CPU_CFG_TS_32_EN == DEF_ENABLED) || \
        (CPU_CFG_TS_64_EN == DEF_ENABLED))
#define  CPU_CFG_TS_EN                          DEF_ENABLED
#else
#define  CPU_CFG_TS_EN                          DEF_DISABLED
#endif

#if    ((CPU_CFG_TS_EN == DEF_ENABLED) || \
(defined(CPU_CFG_INT_DIS_MEAS_EN)))
#define  CPU_CFG_TS_TMR_EN                      DEF_ENABLED
#else
#define  CPU_CFG_TS_TMR_EN                      DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                          CACHE CONFIGURATION
*
* Note(s) : (1) The following pre-processor directives correctly configure CACHE parameters.  DO NOT MODIFY.
*
**********************************************************************************************************
*/

#ifndef CPU_CFG_CACHE_MGMT_EN
#define CPU_CFG_CACHE_MGMT_EN DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           CPU ERROR CODES
*********************************************************************************************************
*/

typedef enum cpu_err {

    CPU_ERR_NONE                            =         0u,
    CPU_ERR_NULL_PTR                        =        10u,

    CPU_ERR_NAME_SIZE                       =      1000u,

    CPU_ERR_TS_FREQ_INVALID                 =      2000u

} CPU_ERR;


/*
*********************************************************************************************************
*                                      CPU TIMESTAMP DATA TYPES
*
* Note(s) : (1) CPU timestamp timer data type defined to the binary-multiple of 8-bit octets as configured
*               by 'CPU_CFG_TS_TMR_SIZE' (see 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #2').
*********************************************************************************************************
*/

typedef  CPU_INT32U  CPU_TS32;
typedef  CPU_INT64U  CPU_TS64;

typedef  CPU_TS32    CPU_TS;                                    /* Req'd for backwards-compatibility.                   */


/*
*********************************************************************************************************
*                               CPU TIMESTAMP TIMER FREQUENCY DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT32U  CPU_TS_TMR_FREQ;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


#ifndef  CPU_SW_EXCEPTION                                                       /* See Note #2.                         */
#define  CPU_SW_EXCEPTION(err_rtn_val)              do {                        \
                                                        PANIC();                \
                                                    } while (0)
#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*
* Note(s) : (1) CPU interrupts disabled time measurement functions prototyped/defined only if
*               CPU_CFG_INT_DIS_MEAS_EN  #define'd in 'cpu_cfg.h'.
*
*           (2) (a) CPU_CntLeadZeros()  defined in :
*
*                   (1) 'cpu_a.asm',  if CPU_CFG_LEAD_ZEROS_ASM_PRESENT       #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable assembly-optimized function(s)
*
*                   (2) 'cpu_core.c', if CPU_CFG_LEAD_ZEROS_ASM_PRESENT   NOT #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable C-source-optimized function(s)
*
*               (b) CPU_CntTrailZeros() defined in :
*
*                   (1) 'cpu_a.asm',  if CPU_CFG_TRAIL_ZEROS_ASM_PRESENT      #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable assembly-optimized function(s)
*
*                   (2) 'cpu_core.c', if CPU_CFG_TRAIL_ZEROS_ASM_PRESENT  NOT #define'd in 'cpu.h'/
*                                         'cpu_cfg.h' to enable C-source-optimized function(s)
*********************************************************************************************************
*/

                                                                        /* --------------- CPU TS FNCTS --------------- */
#if (CPU_CFG_TS_32_EN == DEF_ENABLED)
CPU_TS32         CPU_TS_Get32             (void);
#endif

#if (CPU_CFG_TS_64_EN == DEF_ENABLED)
CPU_TS64         CPU_TS_Get64             (void);
#endif


#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)                                  /* ------------- CPU TS TMR FNCTS ------------- */
CPU_TS_TMR_FREQ  CPU_TS_TmrFreqGet        (CPU_ERR          *p_err);

void             CPU_TS_TmrFreqSet        (CPU_TS_TMR_FREQ   freq_hz);
#endif


                                                                        /* ----------- CPU CNT ZEROS FNCTS ------------ */
static inline
CPU_DATA         CPU_CntLeadZeros32       (CPU_INT32U  val)
{
    return __builtin_clz(val);
}

static inline
CPU_DATA         CPU_CntTrailZeros        (CPU_DATA    val)
{
    return __builtin_ctz(val);
}

static inline
CPU_DATA         CPU_CntTrailZeros08      (CPU_INT08U  val)
{
    return val ? __builtin_ctz(val) : 8;
}

static inline
CPU_DATA         CPU_CntTrailZeros32      (CPU_INT32U  val)
{
    return __builtin_ctz(val);
}


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  CPU_CFG_TS_32_EN
#error  "CPU_CFG_TS_32_EN                      not #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#elif  ((CPU_CFG_TS_32_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_32_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_32_EN                illegally #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#endif


#ifndef  CPU_CFG_TS_64_EN
#error  "CPU_CFG_TS_64_EN                      not #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#elif  ((CPU_CFG_TS_64_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_64_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_64_EN                illegally #define'd in 'cpu_cfg.h'"
#error  "                                [MUST be  DEF_DISABLED]           "
#error  "                                [     ||  DEF_ENABLED ]           "

#endif

                                                                /* Correctly configured in 'cpu_core.h'; DO NOT MODIFY. */
#ifndef  CPU_CFG_TS_EN
#error  "CPU_CFG_TS_EN                         not #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "

#elif  ((CPU_CFG_TS_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_EN                   illegally #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "

#endif


                                                                /* Correctly configured in 'cpu_core.h'; DO NOT MODIFY. */
#ifndef  CPU_CFG_TS_TMR_EN
#error  "CPU_CFG_TS_TMR_EN                     not #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "

#elif  ((CPU_CFG_TS_TMR_EN != DEF_DISABLED) && \
        (CPU_CFG_TS_TMR_EN != DEF_ENABLED ))
#error  "CPU_CFG_TS_TMR_EN               illegally #define'd in 'cpu_core.h'"
#error  "                                [MUST be  DEF_DISABLED]            "
#error  "                                [     ||  DEF_ENABLED ]            "


#elif   (CPU_CFG_TS_TMR_EN == DEF_ENABLED)

#ifndef  CPU_CFG_TS_TMR_SIZE
#error  "CPU_CFG_TS_TMR_SIZE                   not #define'd in 'cpu_cfg.h'       "
#error  "                                [MUST be  CPU_WORD_SIZE_08   8-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_16  16-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_32  32-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_64  64-bit timer]"

#elif  ((CPU_CFG_TS_TMR_SIZE != CPU_WORD_SIZE_08) && \
        (CPU_CFG_TS_TMR_SIZE != CPU_WORD_SIZE_16) && \
        (CPU_CFG_TS_TMR_SIZE != CPU_WORD_SIZE_32) && \
        (CPU_CFG_TS_TMR_SIZE != CPU_WORD_SIZE_64))
#error  "CPU_CFG_TS_TMR_SIZE             illegally #define'd in 'cpu_cfg.h'       "
#error  "                                [MUST be  CPU_WORD_SIZE_08   8-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_16  16-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_32  32-bit timer]"
#error  "                                [     ||  CPU_WORD_SIZE_64  64-bit timer]"
#endif

#endif


/*
*********************************************************************************************************
*                                    CPU PORT CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  CPU_CFG_ADDR_SIZE
#error  "CPU_CFG_ADDR_SIZE      not #define'd in 'cpu.h'"
#endif

#ifndef  CPU_CFG_DATA_SIZE
#error  "CPU_CFG_DATA_SIZE      not #define'd in 'cpu.h'"
#endif

#ifndef  CPU_CFG_DATA_SIZE_MAX
#error  "CPU_CFG_DATA_SIZE_MAX  not #define'd in 'cpu.h'"
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'cpu_core.h  MODULE'.
*********************************************************************************************************
*/

#endif                                                          /* End of CPU core module include.                      */

