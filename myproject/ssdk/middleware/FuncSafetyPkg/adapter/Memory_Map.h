/**
*   @file    Memory_Map.h
*
*   @brief   The base address and size of memories in the chip.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "__regs_base.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MEMORY_MAP_H_VENDOR_ID    0x8C
#define MEMORY_MAP_H_AR_RELEASE_MAJOR_VERSION    4
#define MEMORY_MAP_H_AR_RELEASE_MINOR_VERSION    3
#define MEMORY_MAP_H_AR_RELEASE_REVISION_VERSION 1
#define MEMORY_MAP_H_SW_MAJOR_VERSION    1
#define MEMORY_MAP_H_SW_MINOR_VERSION    0
#define MEMORY_MAP_H_SW_PATCH_VERSION    0
/* Version Info End */
/* Version and Check End */
#define ATCM_BASE   0x10000
#define BTCM_BASE   0x0
#define B0TCM_BASE  BTCM_BASE
#define B1TCM_BASE  (B0TCM_BASE + 0x8000)

#define TCM_BASE    BTCM_BASE
#define TCM_SZ      0x20000
#define TCM_END     (BTCM_BASE + TCM_SZ - 1)

#define APB_BASE                0xF0000000
#define APB_SYSTEM_ADDR         0x30000000


#define SEC_TCM_SYSTEM_ADDR     0x3B0000
#define SAF_TCM_SYSTEM_ADDR     0x330000

#if defined(CORE_sec)
#define SYS_ICACHE_BASE  R5_SEC_CACHE_BASE
#else
#define SYS_ICACHE_BASE  R5_SAF_CACHE_BASE
#endif
#define SYS_DCACHE_BASE  (SYS_ICACHE_BASE + 0x8000)

#if defined(CORE_safe)
#define OSPI_MEM_BASE       OSPI2_BASE
#else
#define OSPI_MEM_BASE       OSPI1_BASE
#endif
#define OSPI_MEM_SZ         0x4000000
#define OSPI_MEM_END       (OSPI_MEM_BASE + OSPI_MEM_SZ - 1)
#define OSPI_ADDR_WIDTH    26

#define TB_TRIG_CMD_ADDR    0x4CFFC0
#define TB_TRIG_ARG1_ADDR   0x4CFFD0
#define TB_TRIG_ARG2_ADDR   0x4CFFE0

#if !defined(CFG_MULTI_CORE_OS)
#if defined(CORE_sec)
#define IRAM_BASE       IRAM2_BASE
#define SCR_RBASE       APB_SCR_SEC_BASE
#define ROMC_RBASE      APB_ROMC2_BASE
#define RSTGEN_RBASE    APB_RSTGEN_SEC_BASE
#define CORE_RESET_B_INDEX  RSTGEN_SEC_CORE_RST_B_CR5_SEC_INDEX
#define SCR_REMAP_CR5_AR_ADDR_OFF_L32_START_BIT \
            SCR_SEC_REMAP_CR5_SEC_AR_ADDR_OFFSET_19_0_L31_START_BIT
#define SCR_REMAP_CR5_REMAP_OVRD_EN_L31_START_BIT \
            SCR_SEC_REMAP_CR5_SEC_AR_REMAP_OVRD_EN_L31_START_BIT
#else
#define IRAM_BASE       IRAM1_BASE
#define SCR_RBASE       APB_SCR_SAF_BASE
#define ROMC_RBASE      APB_ROMC1_BASE
#define RSTGEN_RBASE    APB_RSTGEN_SAF_BASE
#define CORE_RESET_B_INDEX  RSTGEN_SAF_CORE_RST_B_CR5_SAF_INDEX
#define SCR_REMAP_CR5_AR_ADDR_OFF_L32_START_BIT \
            SCR_SAF_REMAP_CR5_SAF_AR_ADDR_OFFSET_19_0_L31_START_BIT
#define SCR_REMAP_CR5_REMAP_OVRD_EN_L31_START_BIT \
            SCR_SAF_REMAP_CR5_SAF_AR_REMAP_OVRD_EN_L31_START_BIT
#endif  /* defined(CORE_sec) */
#endif  /* !defined(CFG_MULTI_CORE_OS */


#ifdef __cplusplus
}
#endif

#endif /* MEMORY_MAP_H */
