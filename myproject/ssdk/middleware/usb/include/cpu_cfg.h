/**
 * @file cpu_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: CPU Configuration file for USB Device & Host.
 *
 * Revision History:
 * -----------------
 */


#ifndef  CPU_CFG_MODULE_PRESENT
#define  CPU_CFG_MODULE_PRESENT

/*
*********************************************************************************************************
*                                     CPU TIMESTAMP CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_TS_xx_EN to enable/disable CPU timestamp features :
*
*               (a) CPU_CFG_TS_32_EN   enable/disable 32-bit CPU timestamp feature
*               (b) CPU_CFG_TS_64_EN   enable/disable 64-bit CPU timestamp feature
*
*           (2) (a) Configure CPU_CFG_TS_TMR_SIZE with the CPU timestamp timer's word size :
*
*                       CPU_WORD_SIZE_08         8-bit word size
*                       CPU_WORD_SIZE_16        16-bit word size
*                       CPU_WORD_SIZE_32        32-bit word size
*                       CPU_WORD_SIZE_64        64-bit word size
*
*               (b) If the size of the CPU timestamp timer is not a binary multiple of 8-bit octets
*                   (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple octet word
*                   size SHOULD be configured (e.g. to 16-bits).  However, the minimum supported word
*                   size for CPU timestamp timers is 8-bits.
*
*                   See also 'cpu_core.h  FUNCTION PROTOTYPES  CPU_TS_TmrRd()  Note #2a'.
*********************************************************************************************************
*/

                                                                /* Configure CPU timestamp features (see Note #1) :     */
#define  CPU_CFG_TS_32_EN                       DEF_DISABLED
#define  CPU_CFG_TS_64_EN                       DEF_DISABLED
                                                                /*   DEF_DISABLED  CPU timestamps DISABLED              */
                                                                /*   DEF_ENABLED   CPU timestamps ENABLED               */

                                                                /* Configure CPU timestamp timer word size ...          */
                                                                /* ... (see Note #2) :                                  */
#define  CPU_CFG_TS_TMR_SIZE                    CPU_WORD_SIZE_32


/*
*********************************************************************************************************
*                                          CACHE MANAGEMENT
*
* Note(s) : (1) Configure CPU_CFG_CACHE_MGMT_EN to enable the cache management API.
*
*           (2) This option only enables the cache management functions.
*               It does not enable any hardware caches, which should be configured in startup code.
*               Caches must be configured and enabled by the time CPU_Init() is called.
*
*           (3) This option is usually required for device drivers which use a DMA engine to transmit
*               buffers that are located in cached memory.
*********************************************************************************************************
*/

                                                                /* Defines CPU data    word-memory order (see Note #1). */
#ifdef CONFIG_ARCH_WITH_CACHE
#define  CPU_CFG_CACHE_MGMT_EN            DEF_ENABLED
#else
#define  CPU_CFG_CACHE_MGMT_EN            DEF_DISABLED
#endif


#endif

