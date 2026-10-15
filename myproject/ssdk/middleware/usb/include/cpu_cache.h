/**
 * @file cpu_cache.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: CPU Cache header file for USB Device & Host.
 *
 * Revision History:
 * -----------------
 */


#ifndef  CPU_CACHE_MODULE_PRESENT
#define  CPU_CACHE_MODULE_PRESENT


#include  <cpu.h>
#include  <cpu_cfg.h>
#include  <arch/cache.h>


/*
*********************************************************************************************************
*                                         CACHE CONFIGURATION
*********************************************************************************************************
*/

#ifndef CPU_CFG_CACHE_MGMT_EN
#define CPU_CFG_CACHE_MGMT_EN  DEF_DISABLED
#endif


/*
*********************************************************************************************************
*                                 CACHE OPERATIONS FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static inline
void  CPU_DCACHE_RANGE_FLUSH(void      *addr_start,
                             CPU_ADDR   len)
{
    arch_clean_cache_range((addr_t)addr_start, len);
}

static inline
void  CPU_DCACHE_RANGE_FLUSHINV(void       *addr_start,
                                CPU_ADDR    len)
{
    arch_clean_invalidate_cache_range((addr_t)addr_start, len);
}

static inline
void  CPU_DCACHE_RANGE_INV  (void      *addr_start,
                             CPU_ADDR   len)
{
    arch_invalidate_cache_range((addr_t)addr_start, len);
}


#endif

