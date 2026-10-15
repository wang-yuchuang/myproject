/********************************************************
 *   @file    Eic_Ip.h
 *
 *   @brief   error injection header file.
 *   @details
 ********************************************************/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#ifndef _EIC_IP_H_
#define _EIC_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CompilerEx.h"
#include "eic_hw.h"
#include "Std_Types.h"
#include "cdefs.h"

#define EIC_REG_BIT_NUM        (32U)    /*Bits number of one EIC register*/
#define INJECT_EIC_1BITERR     (1U)     /*Inject 1 bit error for EIC*/
#define INJECT_EIC_2BITERR     (2U)     /*Inject 2 bitS error for EIC*/


typedef enum {
    EIC_SAFE_MODULE = 0x0u,
    EIC_SEC_MODULE,
    EIC_HPI_MODULE,
    EIC_VSN_MODULE,
    EIC_MAX_MODULE
} Eic_Id;

/*Error bit injection type: 1/2bits*/
typedef enum {
    ONE_BIT_EIC_ERR = 0U,
    TWO_BIT_EIC_ERR
} Eic_Error_BitType;

/* Get EIC address*/
FUNC(void, EIC_CODE) EIC_Ip_GetBaseAddr(void);

FUNC(Eic_Id, EIC_CODE) Eic_Get_Module
(
    enum eic_signal signal_id
)_SECTION(.tcm_func);

FUNC(uint32, EIC_CODE) Eic_Get_Bit
(
    enum eic_signal signal_id
)_SECTION(.tcm_func);

FUNC(uint32, EIC_CODE) Eic_Get_Point
(
    enum eic_signal signal_id
)_SECTION(.tcm_func);

FUNC(void, EIC_CODE) Eic_Enable_Injectpoint
(
    Eic_Error_BitType error_bit,
    Eic_Id eicid,
    uint32 inject_point,
    boolean enable
) _SECTION(.tcm_func);

/*
 * Enable/disable inject point. Inject point must be enabled, before
 * a signal can be toggled by Eic_Enable_Injectpoint_BySignal().
 */
FUNC(void, EIC_CODE) Eic_Enable_Injectpoint_BySignal
(
    Eic_Error_BitType error_bit,
    enum eic_signal signal_id,
    boolean enable
)_SECTION(.tcm_func);

/*
 * Toggle error on specified EIC signal. Note that all injection
 * points share the same injection signal bits. That means if multiple
 * EN bits are enabled within one EIC module, one injection
 * bit will toggle multiple injection signals.
 */
FUNC(void, EIC_CODE) Eic_Inject_Bit
(
    Eic_Error_BitType error_bit,
    enum eic_signal signal_id,
    boolean enable
)_SECTION(.tcm_func);


#ifdef __cplusplus
}
#endif

#endif
