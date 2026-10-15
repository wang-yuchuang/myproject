/**
*   @file    Mcu_Ckgen.c
*
*   @brief   Ckgen IP driver.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                 *
 *        All rights reserved.                           *
 ********************************************************/

#ifndef MCU_CKGEN_H
#define MCU_CKGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_CKGEN_H_VENDOR_ID    0x8C
#define MCU_CKGEN_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_CKGEN_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_CKGEN_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_CKGEN_H_SW_MAJOR_VERSION    1
#define MCU_CKGEN_H_SW_MINOR_VERSION    0
#define MCU_CKGEN_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_CKGEN_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_CKGEN_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_CKGEN_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Ckgen.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_CKGEN_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_CKGEN_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_CKGEN_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Ckgen.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

#define SLICE_ID(x) (x)
#define PREDIV(x)   (x)
#define POSTDIV(x)  (x)
#define SLICE_SRC(x)    (x)
#define UUU_ID(x)   (x)

#define DIV_MNPQ(m, n, p, q)    \
    (FV_CKGEN_UUU_SLICE_M_DIV_NUM(m) \
      | FV_CKGEN_UUU_SLICE_N_DIV_NUM(n)\
      | FV_CKGEN_UUU_SLICE_P_DIV_NUM(p)\
      | FV_CKGEN_UUU_SLICE_Q_DIV_NUM(q))

typedef enum {
    PATH_A = 1,
    PATH_B,
} slice_path_e;

typedef enum {
    UUU_SEL_CKGEN_SOC = 0,
    UUU_SEL_PLL = 3,
} uuu_src_sel_e;

/* clkgen slice monitor clk type */
typedef enum {
    SLICE_MON_IP_CLK = 0,
    SLICE_MON_BUS_CLK,
    SLICE_MON_CORE_CLK,
} ckgen_slice_mon_type;

uint32 ckgen_ip_slice_cfg(uint32 base, uint32 slice_id,
                          uint32 src_sel, uint32 pre_div, uint32 post_div);
uint32 ckgen_bus_slice_cfg(uint32 base, uint32 slice_id,
                           uint32 path, uint32 src_sel, uint32 pre_div);
uint32 ckgen_bus_slice_postdiv_update(uint32 base, uint32 slice_id, uint32 post_div);
void ckgen_bus_slice_switch(uint32 base, uint32 slice_id);
uint32 ckgen_core_slice_cfg(uint32 base, uint32 slice_id, uint32 path, uint32 src_sel);
uint32 ckgen_core_slice_postdiv_update(uint32 base, uint32 slice_id, uint32 post_div);
void ckgen_core_slice_switch(uint32 base, uint32 slice_id);
void ckgen_cg_en(uint32 base, uint32 id);
void ckgen_cg_dis(uint32 base, uint32 id);
uint32 ckgen_uuu_slice_cfg(uint32 base, uint32 slice_id, uint32 src_sel, uint32 div_mnpq);
uint32 ckgen_ip_slice_gate(uint32 base, uint32 slice_id);
uint32 ckgen_ip_slice_ungate(uint32 base, uint32 slice_id);
uint32 ckgen_mon_ip_slice(uint32 base, uint16 ip_slice_idx, uint8 ref_clk_div);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef _MCU_CKGEN_H_ */
