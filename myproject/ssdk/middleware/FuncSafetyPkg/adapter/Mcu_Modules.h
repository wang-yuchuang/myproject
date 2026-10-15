/**
*   @file    Mcu_Modules.h
*
*   @brief   To list all the modules in G9 SOC.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MCU_MODULES_H
#define MCU_MODULES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_MODULES_H_VENDOR_ID    0x8C
#define MCU_MODULES_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_MODULES_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_MODULES_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_MODULES_H_SW_MAJOR_VERSION    1
#define MCU_MODULES_H_SW_MINOR_VERSION    0
#define MCU_MODULES_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_MODULES_H_AR_RELEASE_MAJOR_VERSION != STD_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_MODULES_H_AR_RELEASE_MINOR_VERSION != STD_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_MODULES_H_AR_RELEASE_REVISION_VERSION != STD_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu_Modules.h and Std_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_MODULES_H_SW_MAJOR_VERSION != STD_TYPES_H_SW_MAJOR_VERSION)\
    || (MCU_MODULES_H_SW_MINOR_VERSION != STD_TYPES_H_SW_MINOR_VERSION)\
    || (MCU_MODULES_H_SW_PATCH_VERSION != STD_TYPES_H_SW_PATCH_VERSION))
#error "Opps, Mcu_Modules.h and Std_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

typedef enum {
    FREQ_LOW = 0,
    FREQ_MID = 1,
    FREQ_HIGH = 2,
    FREQ_ULTRA_HIGH = 3,
    OSPI_FREQ_SAFE = 4,
    OSPI_FREQ0 = 5,
    OSPI_FREQ1 = 6,
    OSPI_FREQ2 = 7,
    OSPI_FREQ3 = 8,
    OSPI_FREQ0_DIV = 9,
    OSPI_FREQ1_DIV = 10,

} Mcu_ModuleFreqType;

typedef enum {
    UART1 = 0x1u,
    UART2,
    UART3,
    UART4,
    UART5,
    UART6,
    UART7,
    UART8,
    UART9,
    UART10,
    UART11,
    UART12,
    UART13,
    UART14,
    UART15,
    UART16,

    TIMER1,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER6,
    TIMER7,
    TIMER8,

    WDT1,
    WDT2,
    WDT3,
    WDT4,

    CANFD1,
    CANFD2,
    CANFD3,
    CANFD4,
    CANFD5,
    CANFD6,
    CANFD7,
    CANFD8,
    CANFD9,
    CANFD10,
    CANFD11,
    CANFD12,
    CANFD13,
    CANFD14,
    CANFD15,
    CANFD16,
    CANFD17,
    CANFD18,
    CANFD19,
    CANFD20,

    OSPI1,
    OSPI2,

    SPI1,
    SPI2,
    SPI3,
    SPI4,
    SPI5,
    SPI6,
    SPI7,
    SPI8,

    I2C1,
    I2C2,
    I2C3,
    I2C4,
    I2C5,
    I2C6,
    I2C7,
    I2C8,
    I2C9,
    I2C10,
    I2C11,
    I2C12,
    I2C13,
    I2C14,
    I2C15,
    I2C16,

    DMA1,
    DMA2,
    DMA3,
    DMA4,
    DMA5,
    DMA6,
    DMA7,
    DMA8,

    PWM1,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM6,
    PWM7,
    PWM8,

    CE1,
    CE2,

    MAILBOX,

    GPIO1,
    GPIO2,
    GPIO3,
    GPIO4,
    GPIO5,

    ENET1,
    ENET2,

    GIC1,
    GIC2,
    GIC3,

    IRAMC1,
    IRAMC2,
    IRAMC3,
    IRAMC4,

    SEM1,
    SEM2,

    EIC_SAFE,
    EIC_SEC,
    EIC_HPI,
    EIC_VSN,

    ADC,

    HPI,
    MODULE_END,
} Mcu_ModuleType;

uint32 Mcu_GetModuleBase(Mcu_ModuleType m);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef MCU_MODULES_H */
