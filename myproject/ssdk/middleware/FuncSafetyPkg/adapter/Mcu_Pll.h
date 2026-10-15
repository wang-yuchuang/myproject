/**
*   @file    Mcu_Pll.h
*
*   @brief   Mcu PLL driver.
*   @details Use to program PLL. Given Safety Boot ROM had setup PLL before safety
*            image get running, calling of this function is not a Must.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MCU_PLL_H
#define MCU_PLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_PLL_H_VENDOR_ID    0x8C
#define MCU_PLL_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_PLL_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_PLL_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_PLL_H_SW_MAJOR_VERSION    1
#define MCU_PLL_H_SW_MINOR_VERSION    0
#define MCU_PLL_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_PLL_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_PLL_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_PLL_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Pll.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_PLL_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_PLL_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_PLL_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Pll.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

#define PLL_FBDIV(x)    (x)
#define PLL_REFDIV(x)   (x)
#define PLL_POSTDIV1(x) (x)
#define PLL_FRAC(x)     (x)

#define DIV_A_V(v)  (uint8)(v)
#define DIV_B_V(v)  (uint8)((uint32)(v) >> 8)
#define DIV_C_V(v)  (uint8)((uint32)(v) >> 16)
#define DIV_D_V(v)  (uint8)((uint32)(v) >> 24)

#define DIV_ABCD(a, b, c, d)    \
        (((a) & 0xf) \
         | (((b) & 0xf) << 8) \
         | (((c) & 0xf) << 16)\
         | (((d) & 0xf) << 24))

/**
 * @brief   Program PLL in the chip.
 * @detail  Boot ROM had setup PLLs in Safety domain properly. This function provided here
 *          just in case SW need reprogram PLLs. The formula of the output is as follow:
 *          Fout = (Fref/RefDiv)*(FbDiv+(Frac/2^24))/PostDiv
 *              PostDiv = PostDiv1 * PostDiv2
 *          Some rules shall be followed:
 *              1) PostDiv1 >= PostDiv2
 *              2) 800MHz <= VCO_FREQ <= 3200MHz
 *              3) 16 <= FBDIV <= 640 for INT mode
 *              4) 20 <= FBDIV <= 320 for Frac mode
 *              5) pfd frequency (ref_clk/refdiv) shall be higher than 10MHz for FRAC
 *                 mode, and 5MHz for INT mode.
 *          There are four divs (A/B/C/D, in pll wrapper). Their default values had been
 *          properly set per integration requirement. Usually, there is no need to change them.
 *
 * @param[in]   base        The base address of the PLL.
 * @param[in]   fbdiv       feedback divider, 'FbDiv' in the formula.
 * @param[in]   refdiv      divider to reference clock, 'RefDiv' in the formula.
 * @param[in]   frac        Fraction part of the feedback divider, 'Frac' in the formula.
 * @param[in]   postdiv1    Post Divider1
 * @param[in]   div_abcd    Dividers to A/B/C/D divs.
 */
FUNC(uint32, MCU_CODE) Mcu_PllProgram(
    uint32 base, uint32 fbdiv,
    uint32 refdiv, uint32 frac,
    uint32 postdiv1, uint32 div_abcd);

/**
 * @brief To check if the PLL locked.
 * @param[in] base  The base address of the PLL to be checked.
 * @return  TRUE for locked and FALSE is not.
 */
FUNC(boolean, MCU_CODE) Mcu_IsPllLock(uint32 base);

/**
 * @brief To get the PLL's frequency in KHz.
 * @param[in] base  The base address of the PLL.
 * @return  PLL's frequency in KHz. 0 if PLL not locked or not enabled.
 */
FUNC(uint32, MCU_CODE) Mcu_PllGetFreq(uint32 base);

/**
 * @brief To check if the PLL enabled or not.
 * @param[in] base  The base address of the PLL.
 * @return  TRUE if enabled, FALSE if not.
 */
boolean Mcu_IsPllEnabled(uint32 base);

#ifdef __cplusplus
}
#endif

#endif /* !defined(MCU_PLL_H) */
