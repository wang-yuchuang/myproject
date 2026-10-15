/**
*   @file    Mcu_Soc.c
*
*   @brief   Soc wrapper where G9 specific code sits.
*/
/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                           *
 ********************************************************/

#ifndef MCU_SOC_H
#define MCU_SOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "Mcu_Modules.h"
#include "Mcu_ScrBits.h"
#include "Mcu_Clkgate.h"
#include "Mcu_RstIdx.h"
#include "Memory_Map.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_SOC_H_VENDOR_ID    0x8C
#define MCU_SOC_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_SOC_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_SOC_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_SOC_H_SW_MAJOR_VERSION    1
#define MCU_SOC_H_SW_MINOR_VERSION    0
#define MCU_SOC_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MCU_MODULES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MCU_MODULES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MCU_MODULES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Modules.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MCU_MODULES_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MCU_MODULES_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MCU_MODULES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Modules.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MCU_SOC_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MCU_SOC_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MCU_SOC_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Soc.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MCU_SOC_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MCU_SOC_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MCU_SOC_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Soc.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MCU_SCRBITS_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MCU_SCRBITS_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MCU_SCRBITS_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_ScrBits.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MCU_SCRBITS_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MCU_SCRBITS_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MCU_SCRBITS_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_ScrBits.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MCU_CLKGATE_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MCU_CLKGATE_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MCU_CLKGATE_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Clkgate.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MCU_CLKGATE_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MCU_CLKGATE_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MCU_CLKGATE_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_Clkgate.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MCU_RSTIDX_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MCU_RSTIDX_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MCU_RSTIDX_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_RstIdx.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MCU_RSTIDX_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MCU_RSTIDX_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MCU_RSTIDX_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Mcu_RstIdx.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SOC_H_AR_RELEASE_MAJOR_VERSION != MEMORY_MAP_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_MINOR_VERSION != MEMORY_MAP_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SOC_H_AR_RELEASE_REVISION_VERSION != MEMORY_MAP_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Soc.h and Memory_Map.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SOC_H_SW_MAJOR_VERSION != MEMORY_MAP_H_SW_MAJOR_VERSION)\
    || (MCU_SOC_H_SW_MINOR_VERSION != MEMORY_MAP_H_SW_MINOR_VERSION)\
    || (MCU_SOC_H_SW_PATCH_VERSION != MEMORY_MAP_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Soc.h and Memory_Map.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */
#if defined(CFG_MULTI_CORE_OS)
extern uint32 scr_rbase[];
extern uint32 romc_rbase[];
extern uint32 iram_base[];
extern const uint32 rstgen_rbase[];
extern const uint32 cr5_core_rst_idx[];
extern const uint32 cr5_remap_ar_off_start_bit[];
extern const uint32 cr5_remap_ovrd_en[];
#define IRAM_BASE   iram_base[soc_get_cpu_id()]
#define SCR_RBASE   scr_rbase[soc_get_cpu_id()]
#define ROMC_RBASE  romc_rbase[soc_get_cpu_id()]
#define RSTGEN_RBASE rstgen_rbase[soc_get_cpu_id()]
#define CORE_RESET_B_INDEX  cr5_core_rst_idx[soc_get_cpu_id()]
#define SCR_REMAP_CR5_AR_ADDR_OFF_L32_START_BIT \
            cr5_remap_ar_off_start_bit[soc_get_cpu_id()]
#define SCR_REMAP_CR5_REMAP_OVRD_EN_L31_START_BIT \
            cr5_remap_ovrd_en[soc_get_cpu_id()]
#endif /* defined(CFG_MULTI_CORE_OS) */

#define PMU_RBASE       APB_PMU_BASE
#define RTC1_RBASE      APB_RTC1_BASE

#define ROMC_STICKY_REG_OFF  0x34U

#define FM_ROM_FLAG_ROM_SELFTEST    (0x03U << 0)

#define RG_GPR_REMAP_FLAG           1   /* gpr used to store 'remap' flag */
#define REMAP_DONE                  (0x52454d50u)     /* 'REMP' */

#define RG_GPR_MCAL                 2
#define BM_SAFETY_HANDOVERED        (0x01U << 0u)

#define RG_GPR_SECBOOT_FALG         7
#define SECBOOT_DONE                (0x424F4F54)       /* 'BOOT' */

#define BP_ROMC_STICKY_REMAP_EN     0u

#if !defined(SOC_RPC_REG_ADDR_MAP)
#define SOC_RPC_REG_ADDR_MAP(a)     ((uint32)(a) << 10)
#define SOC_IOMUX_REG_MAP(a)        SOC_RPC_REG_ADDR_MAP(a)
#define SOC_CKGEN_REG_MAP(a)        SOC_RPC_REG_ADDR_MAP(a)
#define SOC_RSTGEN_REG_MAP(a)       SOC_RPC_REG_ADDR_MAP(a)
#define SOC_SCR_REG_MAP(a)          SOC_RPC_REG_ADDR_MAP(a)
#endif

#define SOC_PLL_REFCLK_FREQ     (24UL*1000UL*1000UL)

enum {
    MB_CPU_CR5_SAF,
    MB_CPU_CR5_SEC,
    MB_CPU_CR5_MP,
    MB_CPU_AP1,
    MB_CPU_AP2,
};

/* declared in linker script, at the begining of the image, and shall be 8 bytes aligned.*/
extern unsigned long VECTOR_TBL_BASE[];

/**
 * @brief To set ROMC sticky bit.
 * @param[in] pos   Bit position of the sticky bit.
 */
void ROMC_SetStickyBit(uint32 pos);

/**
 * @brief To remap cpu address zero (from CR5's view) to physical address 'addr'.
 * @brief Usually used to remap exception vector table. When Safety Image get running, the
 *        exception vector table sits at address 0x0 where is part of Boot ROM which had
 *        been locked already. Unfortunately, Cortex-R5 has no register to override exception
 *        vector table. To address this, G9 SOC provides a HW mechanism to remap some RAM
 *        address (say IRAM/TCM) to 'zero' address. To enable this feature, some SCR register
 *        need to be manipulated and a Cortex core reset need to be triggered.
 * @param[in]   addr    The physical address where vector table will be remapped to.
 */
void Mcu_RemapZeroTo(uint32 addr);

/**
 * @brief De-assert(release) moudle's reset signal.
 * @param[in]   m   the module which reset to be de-asserted.
 */
void Mcu_DeassertModuleReset(Mcu_ModuleType m);

/**
 * @brief Setup moudle's reset signal.
 * @detial Configure module's root clock per recommended setting from Semidrive.
 * @param[in]   m   the module which root clock to be setup.
 */
void Mcu_SetupModuleClk(Mcu_ModuleType m, Mcu_ModuleFreqType freq);

/**
 * @brief Assert moudle's reset signal.
 * @param[in]   m   the module which reset to be asserted.
 */
void Mcu_AssertModuleReset(Mcu_ModuleType m);

/**
 * @brief   Handover modules in Safety domain to AP.
 * @details Modules in Safety domain accessible to Safety only by default. Safety image
 *          can assign (handover) some of its module to AP domain. After the handover,
 *          AP can access the module.
 * @param[in]   m   the module which reset to be handovered.
 */
void Mcu_HandoverModule(Mcu_ModuleType m);
void Mcu_UnHandoverModule(Mcu_ModuleType m);

/**
 * @brief Set a flag to AP after modules had been handovered.
 * @details Safety image set a bit in AP rstgen GPR. AP can u8Polling this bit then access
 *          Safety resource (the resources listed in MCU configuration data).
 */
void Mcu_SetHandoverFlag(void);

/**
 * @brief To check if Handover flag being set.
 * @return  TRUE if set, false if not set.
 */
boolean Mcu_IsHandoverFlagSet(void);

/**
 * @brief To check if AP domain powered and accessible.
 * @return  TRUE if powered and accessible, false if not set.
 */
boolean Mcu_IsApOnline(void);

uint32 soc_get_cpu_id(void);

uint32 soc_to_dma_address(uint32 cpu_addr);

/**
 * @brief To setup OSPI flash remapping. Shall never call it when
 *        caller is XIP-running on the flash to be remapped.
 *        A example: ospi2 , remap address1:0x400000 to address2:0x300000 with length 0x1000.
 *          That is original address2 data was overlapped.
 * @param[in]  m        shall be OSPI1 or OSPI2.
 * @param[in]  address1      remap address1 to address2
 * @param[in]  address2      remap address1 to address2
 * @param[in]  size          the size.
 */
void Mcu_Flash_Remap(Mcu_ModuleType m, uint32 address1, uint32 address2, uint32 size);

#ifdef __cplusplus
}
#endif

#endif  /* MCU_SOC_H */
