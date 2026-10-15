/**
*   @file    Mcu_Rstgen.c
*
*   @brief   Rstgen IP driver.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                 *
 *        All rights reserved.                           *
 ********************************************************/

#ifndef MCU_RSTGEN_H
#define MCU_RSTGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_RSTGEN_H_VENDOR_ID    0x8C
#define MCU_RSTGEN_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_RSTGEN_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_RSTGEN_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_RSTGEN_H_SW_MAJOR_VERSION    1
#define MCU_RSTGEN_H_SW_MINOR_VERSION    0
#define MCU_RSTGEN_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_RSTGEN_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_RSTGEN_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_RSTGEN_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Rstgen.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_RSTGEN_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_RSTGEN_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_RSTGEN_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Rstgen.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

uint32 rg_module_reset(uint32 base, uint32 id, uint8 val);
uint32 rg_rd_gpr(uint32 base, uint32 id);
void rg_wr_gpr(uint32 base, uint32 id, uint32 v);
uint32 rg_get_reset_source(uint32 base);
uint32 rg_glb_reset_en(uint32 base, uint32 msk);
uint32 rg_glb_reset_dis(uint32 base, uint32 msk);
void rg_glb_self_reset(uint32 base, uint8 v);
void rg_glb_other_reset(uint32 base, uint8 v);
void rg_core_reset(uint32 base, uint32 id);
void rg_en_isolation(uint32 base, uint8 id);
void rg_dis_isolation(uint32 base, uint8 id);
void rg_core_ctl(uint32 base, uint32 id, boolean release);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef MCU_RSTGEN_H */
