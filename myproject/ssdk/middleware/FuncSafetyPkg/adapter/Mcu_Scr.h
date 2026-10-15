/**
*   @file    Mcu_Scr.h
*
*   @brief   Abstract layer of System Control Registers.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef SCR_H
#define SCR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_SCR_H_VENDOR_ID    0x8C
#define MCU_SCR_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_SCR_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_SCR_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_SCR_H_SW_MAJOR_VERSION    1
#define MCU_SCR_H_SW_MINOR_VERSION    0
#define MCU_SCR_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_SCR_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_SCR_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_SCR_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Scr.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_SCR_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_SCR_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_SCR_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Scr.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

#define SCR_L31_LCK_BIT(bit)    ((bit)/32*32 + 31)
#define SCR_L16_LCK_BIT(bit)    ((bit) + 16)

typedef enum {
    RW,         /**< @brief Read/Write register group within SCR */
    RO,         /**< @brief Read-Only register group */
    L16,        /**< @brief Lockable register group. Upper 16 bits
                            are lock bits of lower 16 bits */
    L31,        /**< @brief Lockable register group. Bit31 to lock bit30-0 */
    R16W16_RW,     /**< @brief Upper 16bits Read-Only and lower 16bit Read/Write */
    R16W16_RO,     /**< @brief Upper 16bits Read-Only and lower 16bit Read/Write */
} scr_reg_type_e;

/**
 * @brief To get one bit of SCR register
 * @param[in] b     Base address of the SCR.
 * @param[in] type  The type of SCR register group. Refer scr_reg_type_e.
 * @param[in] pos   The bit position. Not within the word but the overall group.
 */
uint8 Mcu_ScrGetBit(uint32 b, scr_reg_type_e type, uint32 pos);

/**
 * @brief To set one bit of SCR register
 * @param[in] b     Base address of the SCR.
 * @param[in] type  The type of SCR register group. Refer scr_reg_type_e.
 * @param[in] pos   The bit position. Not within the word but the overall group.
 */
void Mcu_ScrSetBit(uint32 b, scr_reg_type_e type, uint32 pos);

/**
 * @brief To clear one bit of SCR register
 * @param[in] b     Base address of the SCR.
 * @param[in] type  The type of SCR register group. Refer scr_reg_type_e.
 * @param[in] pos   The bit position. Not within the word but the overall group.
 */
void Mcu_ScrClrBit(uint32 b, scr_reg_type_e type, uint32 pos);

/**
 * @brief To read continous bits (no more then 32 bits) of SCR register
 * @param[in] b     Base address of the SCR.
 * @param[in] type  The type of SCR register group. Refer scr_reg_type_e.
 * @param[in] start The start bit position to be read. Not within the word but the overall group.
 * @param[in] width The number of bits to be read, shall not bigger than 32.
 * @return  The value been read.
 */
uint32 Mcu_ScrReadBits(uint32 b, scr_reg_type_e type, uint32 start, uint32 width);

/**
 * @brief To write continous bits (no more then 32 bits) of SCR register
 * @param[in] b     Base address of the SCR.
 * @param[in] type  The type of SCR register group. Refer scr_reg_type_e.
 * @param[in] start The start bit position to be read. Not within the word but the overall group.
 * @param[in] width The number of bits to be read, shall not bigger than 32.
 * @param[in] val  The value to be written.
 */
void Mcu_ScrWriteBits(uint32 b, scr_reg_type_e type, uint32 start, uint32 width, uint32 val);

#ifdef __cplusplus
}
#endif

#endif  /* SCR_H */
