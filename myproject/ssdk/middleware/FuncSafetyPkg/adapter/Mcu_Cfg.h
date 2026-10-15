/**
 * @file Mcu_Cfg.h
 * @brief MCU_CFG abstraction layer.
 * @details Implement of Specification of MCU_CFG Driver AUTOSAR CP Release 4.3.1
 * @date 2021-07-02 11:05:28
 */

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MCU_CFG_H
#define MCU_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "Mcu_Modules.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_CFG_H_VENDOR_ID    0x8C
#define MCU_CFG_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_CFG_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_CFG_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_CFG_H_SW_MAJOR_VERSION    1
#define MCU_CFG_H_SW_MINOR_VERSION    0
#define MCU_CFG_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_CFG_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_CFG_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_CFG_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Cfg.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_CFG_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_CFG_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_CFG_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Cfg.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_CFG_H_AR_RELEASE_MAJOR_VERSION != MCU_MODULES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_CFG_H_AR_RELEASE_MINOR_VERSION != MCU_MODULES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_CFG_H_AR_RELEASE_REVISION_VERSION != MCU_MODULES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Cfg.h and Mcu_Modules.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_CFG_H_SW_MAJOR_VERSION != MCU_MODULES_H_SW_MAJOR_VERSION)\
    || (MCU_CFG_H_SW_MINOR_VERSION != MCU_MODULES_H_SW_MINOR_VERSION)\
    || (MCU_CFG_H_SW_PATCH_VERSION != MCU_MODULES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Cfg.h and Mcu_Modules.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

#define MCU_MAX_RAMCFGS  4
#define MCU_MAX_MODECFGS 1
#define MCU_MAX_CLKCFGS  1

typedef struct {
    uint32 u32Idx;
    uint8 *pBase;
    uint32 u32Size;
    uint8 u8DefaultValue;
} Mcu_RamConfigType;

typedef struct {
    const Mcu_ModuleType *pModules;
    const Mcu_ModuleType *pHandOvers;
} Mcu_HwIpsConfigType;

#define MCU_DEV_ERROR_DETECT (STD_ON)
#define MCU_VERSION_INFO_API (STD_OFF)
#define MCU_PERFORM_RESET_API (STD_ON)
#define MCU_PERFORM_AP_DOMAIN_RESET_API (STD_ON)
#define MCU_GET_RAM_STATE_API (STD_ON)

#ifdef __cplusplus
}
#endif

#endif /* !defined(MCU_CFG_H) */
