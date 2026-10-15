/**
 *   @file    RegHelper.h
 *
 *   @brief   HW register access layer.
 */

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef REGHELPER_H
#define REGHELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"
#include "CompilerEx.h"
#include "Std_Types.h"
#include "arch.h"
/* Version and Check Begin */
/* Version Info Begin */
#define REGHELPER_H_VENDOR_ID    0x8C
#define REGHELPER_H_AR_RELEASE_MAJOR_VERSION    4
#define REGHELPER_H_AR_RELEASE_MINOR_VERSION    3
#define REGHELPER_H_AR_RELEASE_REVISION_VERSION 1
#define REGHELPER_H_SW_MAJOR_VERSION    1
#define REGHELPER_H_SW_MINOR_VERSION    0
#define REGHELPER_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((REGHELPER_H_AR_RELEASE_MAJOR_VERSION != PLATFORM_TYPES_H_AR_RELEASE_MAJOR_VERSION)\
    || (REGHELPER_H_AR_RELEASE_MINOR_VERSION != PLATFORM_TYPES_H_AR_RELEASE_MINOR_VERSION)\
    || (REGHELPER_H_AR_RELEASE_REVISION_VERSION != PLATFORM_TYPES_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, RegHelper.h and Platform_Types.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((REGHELPER_H_SW_MAJOR_VERSION != PLATFORM_TYPES_H_SW_MAJOR_VERSION)\
    || (REGHELPER_H_SW_MINOR_VERSION != PLATFORM_TYPES_H_SW_MINOR_VERSION)\
    || (REGHELPER_H_SW_PATCH_VERSION != PLATFORM_TYPES_H_SW_PATCH_VERSION))
#error "Opps, RegHelper.h and Platform_Types.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

#define writell(v, a)   (*(volatile uint64*)(uintptr_t)(a)) = (uint64) (v)
#define readll(a)       (*(volatile uint64*)(uintptr_t)(a))
#define readl(a)        (*(volatile uint32*)(uintptr_t)(a))
#define writel(v, a)    (*(volatile uint32*)(uintptr_t)(a)) = (uint32) (v)
#define readh(a)        (*(volatile uint16*)(uintptr_t)(a))
#define writeh(v, a)    (*(volatile uint16*)(uintptr_t)(a)) = (uint16) (v)
#define readb(a)        (*(volatile uint8*)(uintptr_t)(a))
#define writeb(v, a)    (*(volatile uint8*)(uintptr_t)(a))  = (uint8) (v)

#define reg32(addr)     (*(volatile uint32 *)(addr))
#define reg16(addr)     (*(volatile uint16 *)(addr))
#define reg8(addr)      (*(volatile uint8 *)(addr))

#define rmw32(addr, startbit, width, val) \
            reg32(addr) = ((reg32(addr) & (uint32)(~(uint32)((((uint32)1u<<(width)) - 1u) << (startbit)))) | ((uint32)(val) << (startbit)))
#define rmw16(addr, startbit, width, val) \
            reg16(addr) = ((reg16(addr) & ~(((1u<<(width)) - 1u) << (startbit))) | ((val) << (startbit)))
#define rmw8(addr, startbit, width, val) \
            reg8(addr) = ((reg8(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit)))


#define REG64(addr) ((volatile uint64_t *)(uintptr_t)(addr))
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG16(addr) ((volatile uint16_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~((((uint64_t)1u<<(width)) - 1u) << (startbit))) | ((uint64_t)(val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~((((uint32_t)1u<<(width)) - 1u) << (startbit))) | ((uint32_t)(val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~((((uint16_t)1u<<(width)) - 1u) << (startbit))) | ((uint16_t)(val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~((((uint8_t)1u<<(width)) - 1u) << (startbit))) | ((uint8_t)(val) << (startbit))




#define REG_READ32(a)   readl(a)
#define REG_READ16(a)   readh(a)
#define REG_READ8(a)    readb(a)

#define REG_WRITE32(v, a)   writel(v, a)
#define REG_WRITE16(v, a)   writeh(v, a)
#define REG_WRITE8(v, a)    writeb(v, a)
#define REG_RMW32(addr, startbit, width, val)   rmw32(addr, startbit, width, val)
#define REG_RMW16(addr, startbit, width, val)   rmw16(addr, startbit, width, val)
#define REG_RMW8(addr, startbit, width, val)    rmw8(addr, startbit, width, val)

#define BIT(nr)       (1U << (nr))
#define BITS_WIDTH(x) ((1U << (x)) - 1)
#define SET_BITS32(x, start, width) (x = ((uint32)x | ((BITS_WIDTH(width)) << start)))
#define CLEAR_BITS32(x, start, width) (x = ((uint32)x & (~((BITS_WIDTH(width)) << start))))
#define GET_ONE_BYTE(v, start_bit) (((uint32)v & (0xFF << start_bit)) >> start_bit)

LOCAL_INLINE Std_ReturnType Reg_Write_ReadBack32(uint32 v, uint32 a, uint32 mask)
{
    writel(v, a);
    if((v & mask) == (readl(a) & mask)) {
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

LOCAL_INLINE Std_ReturnType Reg_RMW_ReadBack32(uint32 addr, uint32 startbit, uint32 width, uint32 val, uint32 mask)
{
    rmw32(addr, startbit, width, val);
    if((val & mask) == (((readl(addr)>>startbit)&((1u<<(width)) - 1u)) & mask)) {
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

#ifdef __cplusplus
}
#endif
#endif  /* REGHELPER_H */
