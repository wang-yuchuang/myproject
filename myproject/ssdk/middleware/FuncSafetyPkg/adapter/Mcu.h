/**
*   @file    Mcu.h
*
*   @brief   MCU abstraction layer.
*   @details Implement of Specification of MCU Driver AUTOSAR CP Release 4.3.1
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MCU_H
#define MCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Mcu_Cfg.h"


#define MCU_MODULE_ID   101

#include "Mcal.h"
/* Version and Check Begin */
/* Version Info Begin */
#define MCU_H_VENDOR_ID    0x8C
#define MCU_H_AR_RELEASE_MAJOR_VERSION    4
#define MCU_H_AR_RELEASE_MINOR_VERSION    3
#define MCU_H_AR_RELEASE_REVISION_VERSION 1
#define MCU_H_SW_MAJOR_VERSION    1
#define MCU_H_SW_MINOR_VERSION    0
#define MCU_H_SW_PATCH_VERSION    0
/* Version Info End */

/* Version Check Begin */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_H_AR_RELEASE_MAJOR_VERSION != MCU_CFG_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_H_AR_RELEASE_MINOR_VERSION != MCU_CFG_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_H_AR_RELEASE_REVISION_VERSION != MCU_CFG_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu.h and Mcu_Cfg.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_H_SW_MAJOR_VERSION != MCU_CFG_H_SW_MAJOR_VERSION)\
    || (MCU_H_SW_MINOR_VERSION != MCU_CFG_H_SW_MINOR_VERSION)\
    || (MCU_H_SW_PATCH_VERSION != MCU_CFG_H_SW_PATCH_VERSION))
#error "Opps, Mcu.h and Mcu_Cfg.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
#if !defined(DISABLE_MCAL_ASR_VER_CHECK)
#if ((MCU_H_AR_RELEASE_MAJOR_VERSION != MCAL_H_AR_RELEASE_MAJOR_VERSION)\
    || (MCU_H_AR_RELEASE_MINOR_VERSION != MCAL_H_AR_RELEASE_MINOR_VERSION)\
    || (MCU_H_AR_RELEASE_REVISION_VERSION != MCAL_H_AR_RELEASE_REVISION_VERSION))
#error "Opps, Mcu.h and Mcal.h AutoSAR version not match!"
#endif
#endif  /* !DISABLE_MCAL_ASR_VER_CHECK */
#if !defined(DISABLE_MCAL_SW_VER_CHECK)
#if ((MCU_H_SW_MAJOR_VERSION != MCAL_H_SW_MAJOR_VERSION)\
    || (MCU_H_SW_MINOR_VERSION != MCAL_H_SW_MINOR_VERSION)\
    || (MCU_H_SW_PATCH_VERSION != MCAL_H_SW_PATCH_VERSION))
#error "Opps, Mcu.h and Mcal.h Software version not match!"
#endif
#endif  /* !DISABLE_MCAL_SW_VER_CHECK */
/* Version Check End */
/* Version and Check End */

/* Error Code */
#define MCU_E_PARAM_CONFIG      0x0Au
#define MCU_E_PARAM_CLOCK       0x0Bu
#define MCU_E_PARAM_MODE        0x0Cu
#define MCU_E_PARAM_RAMSECTION  0x0Du
#define MCU_E_PLL_NOT_LOCKED    0x0Eu
#define MCU_E_UNINIT            0x0Fu
#define MCU_E_PARAM_POINTER     0x10u
#define MCU_E_INIT_FAILED       0x11u

typedef uint32 Mcu_ClockType;

typedef uint32 Mcu_RamSectionType;

typedef enum {
    MCU_POWER_ON_RESET,     /**< @brief Power On Reset (default) */
    MCU_WATCHDOG_RESET,     /**< @brief Internal Watchdog Timer Reset */
    MCU_SW_RESET,           /**< @brief Software Reset */
    MCU_RESET_UNDEFINED,    /**< @brief Reset is undefined */
    MCU_SEM_RESET,          /**< @brief SEM (function safety related) reset */
    MCU_DBG_RESET,          /**< @brief Reset by Debugger */
} Mcu_ResetType;

typedef uint32 Mcu_RawResetType;

/* SWS_Mcu_00250 */
typedef enum {
    MCU_PLL_LOCKED = 0x5Au,             /**< @brief PLL is locked. */
    MCU_PLL_UNLOCKED = 0xA5u,           /**< @brief PLL is unlocked. */
    MCU_PLL_STATUS_UNDEFINED = 0xF0u,   /**< @brief PLL Status is unknown. */
} Mcu_PllStatusType;

typedef uint32 Mcu_RamStateType;

typedef enum {
    MCU_MODE_NORMAL,        /**< @brief MCU Normal running mode */
    MCU_MODE_DEEPSLEEP,     /**< @brief All powers excepts RTC been disabled */
} Mcu_ModeType;

typedef uint32 Mcu_ModeConfigType;

typedef uint32 Mcu_ClockConfigType;

/**
 * @brief   Mcu_ConfigType [SWS_Mcu_0024]
 *
 * @details A pointer to such a structure is provided to the MCU initialization routines for
 *          configuration.
 */
typedef struct {
    VAR(Mcu_RamSectionType, MCU_VAR) u32NRamSections;
    VAR(Mcu_ModeType, MCU_VAR) u32NModes;
    VAR(Mcu_ClockType, MCU_VAR) u32NClkCfgs;
    CONST(Mcu_RamConfigType, MCU_CONST) (*pRamCfgs)[MCU_MAX_RAMCFGS];
    CONST(Mcu_ModeConfigType, MCU_CONST) (*pModeCfgs)[MCU_MAX_MODECFGS];
    CONST(Mcu_ClockConfigType, MCU_CONST) (*pClkCfgs)[MCU_MAX_CLKCFGS];
    P2CONST(Mcu_HwIpsConfigType, MCU_VAR, MCU_APPL_CONST) pHwIpsCfg;
} Mcu_ConfigType;

#if defined(CFG_MULTI_CORE_OS)
extern const Mcu_ConfigType Mcu_Config[];
#else
extern const Mcu_ConfigType Mcu_Config;
#endif

/**
 * @brief   Mcu init routine.
 * @details This function does some soc specific jobs, including #1. vector table remap,
 *          #2. release resets of modules which been provisioned in Mcu configuration data.
 *          #3. some other actions.
 * @param[in]   pConfigPtr  pointing to configuration data of Mcu.
 * @return  None.
 */
FUNC(void, MCU_CODE) Mcu_Init(P2CONST(Mcu_ConfigType, AUTOMATIC, MCU_APPL_CONST) pConfigPtr);

/**
 * @brief Initialize clocks which configuration been provided in Mcu configuration structure.
 * @details Clock configurations recommended by Semidrive will be setup in this function.
 *          Customer just needs to spefic the module list, and this function will setup
 *          the root clock of the modules.
 * @param[in]   selection of clock settings. Will be ignored in this implementation.
 * @return E_OK for success and others for failure.
 */
FUNC(Std_ReturnType, MCU_CODE) Mcu_InitClock(VAR(Mcu_ClockType, AUTOMATIC) ClockSetting);

/**
 * @brief Initialize Ram Section.
 * @details On G9, there is HW mechnism to Zeroize IRAMs, thus by default, all IRAMs are cleared.
 *          There is ECC support in TCM thus SW shall not read it unless it had been written. So
 *          we can not noinit sections.
 * @param[in]   RamSection  idx of ram section to be initialized.
 * @return      E_OK for success and E_NOT_OK for failure.
 */
FUNC(Std_ReturnType, MCU_CODE) Mcu_InitRamSection(VAR(Mcu_RamSectionType, AUTOMATIC) RamSection);

/**
 * @brief Distribute PLL clock to clock system which feed peripherals.
 * @details On G9, thanks to Boot ROM, PLL had been enabled and clocks had distrubted
            to ckgen already. Thus this function do nothing on G9.
 * @return  E_OK.
 */
FUNC(Std_ReturnType, MCU_CODE) Mcu_DistributePllClock(VAR(void, AUTOMATIC));

/**
 * @brief   Get the PLL status.
 * @return  The status of the PLL. On G9, there are two PLLs on safety domain. This function
 *          returns the status of the two PLLs.
 */
FUNC(Mcu_PllStatusType, MCU_CODE) Mcu_GetPllStatus(VAR(void, AUTOMATIC));

/**
 * @brief   Get the reason of last reset.
 * @detials Besides the reasons defined by AutoSAR spec,  MCU_SEM_RESET/MCU_DBG_RESET
 *          also supported here.
 * @return  The type of last reset.
 */
FUNC(Mcu_ResetType, MCU_CODE) Mcu_GetResetReason(VAR(void, AUTOMATIC));

/**
 * @brief   Get the raw value of reset status register.
 * @return  Value of raw reset status register.
 */
FUNC(Mcu_RawResetType, MCU_CODE) Mcu_GetResetRawValue(VAR(void, AUTOMATIC));

/**
 * @brief   Perform reset to Mcu.
 * @details On G9, it will reset safety domain (ap domain will be reset also).
 *          No impact to rtc domain.
 */
FUNC(void, MCU_CODE) Mcu_PerformReset(VAR(void, AUTOMATIC));

/**
 * @brief   Set the power mode of MCU.
 * @details So far, only Normal and Deepsleep (RTC power only) mode supported.
 *          Once been set into Deepsleep mode, only wakeup (rtc1/2 and
 *          wake_pin0/1) can wakeup the Mcu.

 * @param[in]   McuMode     The power mode to switch to.
 */
FUNC(void, MCU_CODE) Mcu_SetMode(VAR(Mcu_ModeType, AUTOMATIC) McuMode);

#if (MCU_GET_RAM_STATE_API == STD_ON)
FUNC(Mcu_RamStateType, MCU_CODE) Mcu_GetRamState(VAR(void, AUTOMATIC));
#endif /* (MCU_GET_RAM_STATE_API == STD_ON) */

/**
 * @brief   Get version information of this driver.
 */
FUNC(void, MCU_CODE) Mcu_GetVersionInfo(
    P2VAR(Std_VersionInfoType, AUTOMATIC, MCU_APPL_DATA) pVersionInfo
);

/* Non-AutoSAR APIs */
#if (MCU_PERFORM_AP_DOMAIN_RESET_API == STD_ON)
/**
 * @brief   Reset Ap domain, triggered by Safety image.
 */
FUNC(void, MCU_CODE) Mcu_PerformApDomainReset(VAR(void, AUTOMATIC));
#endif /* (MCU_PERFORM_AP_DOMAIN_RESET_API == STD_ON) */

#ifdef __cplusplus
}
#endif

#endif /* !defined(_MCU_H_) */
