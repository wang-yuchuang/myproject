/******************************************************
 *   @file    FuncSafetyPkg.c
 *
 *   @brief   Function safety package source file.
 *   @details
 *
 *******************************************************/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include"FuncSafetyPkg.h"
#include"__regs_base.h"
#include"RegHelper.h"
#include"Mcu_Scr.h"
#include"archEx.h"
#include "fuse_ctrl_reg.h"
#include "Mcu_Delay.h"
#include "Memory_Map.h"
#include "Wdg_Reg.h"
#include "Mcu_Xtal.h"
#include "Mcu_Soc.h"
#include "FuncSafetyPkg_Cfg.h"
#include "xtal24m_reg.h"
#include "osc24m_reg.h"
#include <CLI.h>
#include <CLI_type.h>
#include <property_cfg.h>
#include <cmsis_os2.h>

extern void udelay(uint32_t usecs);
#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))

typedef enum {
    TESTID0 = 0x0u,
    TESTID1,
    TESTID2,
    TESTID3,
    TESTID4,
    TESTID5,
    TESTID6,
    TESTID7,
    TESTID8,
    TESTID9,
    TESTID10,
    TESTID11,
    TEST_PERIOD_START=253,
    TEST_PERIOD_STOP=254,
    TESTALL=255,
} Test_Id;

/******************************External variable******************************/
__no_init VAR_ALIGN(volatile uint8 Iram_Inj_Err[256], CACHE_LINE) _SECTION(.iram_data);
__no_init VAR_ALIGN(volatile uint8 Tcm_Inj_Err[256], CACHE_LINE) _SECTION(.tcm_data);

/******************************Global variable for configuration******************************/
static bool FuncSafetyPkg_Run_Period_Run_Flag = false;
#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
P2CONST(Iram_ZoneConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_IramZoneCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
P2CONST(TCMErrConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_TCMErrCurConfig = NULL_PTR;
#endif

#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
P2CONST(DCACHEErrConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_DCACHEErrCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
P2CONST(FabricErrConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_FabricErrCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
P2CONST(FuseErrConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_FuseErrCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
P2CONST(SemSignalMonConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_SemSignalMonCurConfig = NULL_PTR;
#endif

#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
P2CONST(BistConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_BistCurConfig = NULL_PTR;
#endif

#if (FUSAPKG_WDT_SELFTEST_POWER_ON == STD_ON)
P2CONST(WdtSelfTestConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_WdtSelfTestCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON) || (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON))
P2CONST(FsrefClkChkConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_FsrefClkChkCurConfig = NULL_PTR;
#endif

P2CONST(SemIntMonConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_SemIntCurConfig = NULL_PTR;

#if ((FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD == STD_ON))
P2CONST(DualCoreLockStepInjectErrConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_DualCoreLockStepCurConfig = NULL_PTR;
#endif

#if (FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON == STD_ON)
P2CONST(SocTempMonitorTstConfigType, FUNCSAFETYPKG_VAR,
        FUNCSAFETYPKG_APPL_CONST) g_SocTempMonitorTstCurConfig = NULL_PTR;
#endif

#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Injecting 2bits TCM error flag */
boolean Inject2bitTcmErrFlg = FALSE;
#endif

/******************************Static global variable ******************************/
/*Iram base address for injecting error in iram1/2/3/4*/
static unsigned long InjErr_IramBaseAddr32[4U] = {0U, 0U, 0U, 0U};
/*Enter sem isr when setting a wrong expected target value to trigger checking*/
static boolean gSemSigMonInjectErrIsrFlg = FALSE;
/*Set a wrong expected target value to trigger checking*/
static boolean gSemSigMonInjectErrEnable = FALSE;
/*Set a expected target value to monitor sem signal*/
static boolean gSemSigMonEnable = FALSE;
/*sem scr information*/
#ifndef CONFIG_BOARD_X9SP
static CONST(Sem_ScrBitsType, FUNCSAFETYPKG_VAR) g_SemScrinfo[SEM_SCR_NUM] = {
    SCR_SEM_INFO(SEC, NOC_SEC, M_0_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_1_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_2_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_3_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_5_MAINSGLARADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_5_MAINSGLAWADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, M_5_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_0_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_1_MAINSGLECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_2_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_3_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_13_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_14_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_SEC, S_17_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_11_MAINSGLARADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_11_MAINSGLAWADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_11_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_12_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_12_MAINSGLAWADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_12_MAINSGLARADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_24_MAINSGLARADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_24_MAINSGLAWADDRECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, M_24_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, S_0_SBP_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, S_3_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, S_4_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SEC, NOC_MAIN, S_11_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_MAINB, M_0_MAINSGLARADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_MAINB, M_0_MAINSGLAWADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_MAINB, M_0_MAINSGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, M_0_MAINDUPL_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, M_0_MAINMAIN_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_1_MAINDUPL_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_1_MAINMAIN_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, M_4_MAINMAIN_SGLARADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, M_4_MAINMAIN_SGLAWADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_4_MAINMAIN_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_4_MAINDUPL_SGLARADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_4_MAINDUPL_SGLAWADDRECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_4_MAINDUPL_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, S_0_MAINDUPL_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF, S_0_MAINMAIN_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_1_MAINMAIN_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_1_MAINDUPL_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_2_MAINMAIN_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_2_MAINDUPL_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_4_MAINDUPL_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_4_MAINMAIN_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPHCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_0),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_1),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_2),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_3),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_4),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISFATAL0_0),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISFATAL0_1),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISFATAL0_2),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISFATAL0_3),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXISFATAL0_4),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_0),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_1),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_2),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_3),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_4),
    SCR_SEM_INFO(SAF, CR5_SAF,  DCCMOUT_0),
    SCR_SEM_INFO(SAF, CR5_SAF,  DCCMOUT2_0),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMCORR0),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXCORR0),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPHCORR0),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMFATAL0_0),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMFATAL0_1),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMFATAL0_2),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMFATAL0_3),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXIMFATAL0_4),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXISFATAL0_0),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXISFATAL0_1),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXISFATAL0_2),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXISFATAL0_3),
    SCR_SEM_INFO(SEC, CR5_SEC,  AXISFATAL0_4),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXFATAL0_0),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXFATAL0_1),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXFATAL0_2),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXFATAL0_3),
    SCR_SEM_INFO(SEC, CR5_SEC,  PPXFATAL0_4),
    SCR_SEM_INFO(SEC, CR5_SEC,  DCCMOUT_0),
    SCR_SEM_INFO(SEC, CR5_SEC,  DCCMOUT2_0)
};
#else /* CONFIG_BOARD_X9SP*/
static CONST(Sem_ScrBitsType, FUNCSAFETYPKG_VAR) g_SemScrinfo[SEM_SCR_NUM] = {
    SCR_SEM_INFO(SAF, NOC_SAF,  S_2_MAINMAIN_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  S_2_MAINDUPL_SGLDATAECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_0_MAINDUPL_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_0_MAINMAIN_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_1_MAINDUPL_SGLECCERR),
    SCR_SEM_INFO(SAF, NOC_SAF,  M_1_MAINMAIN_SGLECCERR),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  AXIMFATAL0_2),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXCORR0),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_0),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_1),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_2),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_3),
    SCR_SEM_INFO(SAF, CR5_SAF,  PPXFATAL0_4),
};
#endif /* CONFIG_BOARD_X9SP*/

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
/*sem scr information for monitoring sem signal*/
static CONST(SemMonSig_ScrBitsType, FUNCSAFETYPKG_VAR) g_SemMonSigScrinfo[SEM_MON_SIG_SCR_NUM] = {
    SCR_SEM_MON_SIG_INFO(APBMUX2_SEM1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_CE1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PWRCTRL_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_OSPI1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_CANFD1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_CANFD2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_CANFD3_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_CANFD4_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2C1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2C2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2C3_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2C4_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART3_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART4_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART5_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART6_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART7_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_UART8_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_ENET1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_TIMER1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_TIMER2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PWM1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PWM2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_WDT1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_DMA1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RC_24M_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PLL1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PLL2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PVT_SNS_SAF_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_BIPC_ENET1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2S_SC1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_I2S_SC2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_IRAM1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SPI1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SPI2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SPI3_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SPI4_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RPC_SAF_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_XTAL_SAF_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_EIC_SAFE_SEL),
    SCR_SEM_MON_SIG_INFO(AXISLVMUX_IRAM1_SEL),
    SCR_SEM_MON_SIG_INFO(AXISLVMUX_OSPI1_SEL),
    SCR_SEM_MON_SIG_INFO(AXISLVMUX_DMA1_SEL),
    SCR_SEM_MON_SIG_INFO(AXISLVMUX_CE1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_TM_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RC_RTC_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_IOMUX_CTRL_RTC_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_XTAL_RTC_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RTC1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RTC2_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_PMU_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_RSTGEN_RTC_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SEC_STORAGE1_SEL),
    SCR_SEM_MON_SIG_INFO(APBMUX2_SEC_STORAGE2_SEL)
};
#endif

#if (FUSAPKG_SELF_TEST_POWER_ON == STD_ON)
/*Information for running function safety task when ecu powers on */
static VAR(PoweronFuncSafetyPkgTaskType,
           FUNCSAFETYPKG_VAR) gPoweronFuncSafetyTaskInfo[FUNCSAFETY_POWERONTASK_NUM] = {
    {
#if (FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON == STD_ON)
        .pFuncSafetyTask = Soc_Temp_Monitor,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
        .pFuncSafetyTask = Iram_Inject_SigMulErr_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
        .pFuncSafetyTask = Fabric_Inject_SigMulErr_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
        .pFuncSafetyTask = Fuse_Inject_Err_Check,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
        .pFuncSafetyTask = TCM_InjSigMulErr_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
        .pFuncSafetyTask = Dcache_1bitErr_Injection,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
        .pFuncSafetyTask = Run_Bist_Noc_SafeCore,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if (FUSAPKG_WDT_SELFTEST_POWER_ON == STD_ON)
        .pFuncSafetyTask = Wdt_SelfTst_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON) || (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON))
        .pFuncSafetyTask = Fsrefclk_Check_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON == STD_ON)
        .pFuncSafetyTask = DualCore_LockStep_InjectError,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    },
    {
#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
        .pFuncSafetyTask = Sem_SigMonitor_Func,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
    }
};
#endif

#if (FUSAPKG_SELF_TEST_PERIOD == STD_ON)
/*Information for running function safety task periodically */
static VAR(PeriodFuncSafetyPkgTaskType,
           FUNCSAFETYPKG_VAR) gPeriodFuncSafetyTaskInfo[FUNCSAFETY_PERIODTASK_NUM] = {
    {
#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
        .pFuncSafetyTask = Fuse_Inject_Err_Check,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
        .u8RunPeriodCnt = FUSE_ERROR_INJECT_PERIOD_CNT,
        .bPerformedFlg = FALSE
    },
    {
#if (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD == STD_ON)
        .pFuncSafetyTask = DualCore_LockStep_InjectError,
#else
        .pFuncSafetyTask = NULL_PTR,
#endif
        .u8RunPeriodCnt = DUALCORE_LOCKSTEP_ERROR_INJECT_PERIOD_CNT,
        .bPerformedFlg = FALSE
    },
};

/*Count for running function safety task periodically */
static uint8 gFuncSafetyTaskPeriodCnt = 0U;
unsigned char gFuncSafetyFinishedInit = 0;
#endif

/*************************************************Static function************************************/
/* Get Iram injection error address*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_GetInjErrBaseAddr(void)
{
    InjErr_IramBaseAddr32[0] = (unsigned long)&Iram_Inj_Err[0];
   // InjErr_IramBaseAddr32[0] = (unsigned long)__inject_err_iram1_start;
    InjErr_IramBaseAddr32[1] = (unsigned long)IRAM2_INJERR_ADDR;
    InjErr_IramBaseAddr32[2] = (unsigned long)IRAM3_INJERR_ADDR;
    InjErr_IramBaseAddr32[3] = (unsigned long)IRAM4_INJERR_ADDR;
}

void print_InjErr_IramBaseAddr32(void)
{
    printf("InjErr_IramBaseAddr32[0]=%ld\n",InjErr_IramBaseAddr32[0]);
    printf("InjErr_IramBaseAddr32[1]=%ld\n",InjErr_IramBaseAddr32[1]);
    printf("InjErr_IramBaseAddr32[2]=%ld\n",InjErr_IramBaseAddr32[2]);
    printf("InjErr_IramBaseAddr32[3]=%ld\n",InjErr_IramBaseAddr32[3]);
}




/*Clear status for NOC Mission Interrupt via BIST mode*/
static FUNC(void, FUNCSAFETYPKG_CODE) NOC_Mission_Interrupt_Clr
(
    VAR(Bist_NocType, AUTOMATIC)   NocType,
    VAR(uint32, AUTOMATIC)         bist_timeout1,
    VAR(uint32, AUTOMATIC)         bist_timeout2
)
{
    uint32 base_addr = 0U;
    uint32 wait_cnt = 0U;

    if (NocType == NOC_SAFE) {
        base_addr = GPV_BASE;
    } else if (NocType == NOC_SEC) {
        base_addr = APB_GPV_SEC_BASE;
    } else if (NocType == NOC_HPIA) {
#ifndef CONFIG_BOARD_X9SP
        base_addr = FAB_GPV_HPIA;
#endif
    } else if (NocType == NOC_HPIB)  {
        base_addr = APB_GPV_HPIB_BASE;
    } else {
        /*for misra*/
    }

    /*set the time-out value of timeout1 mechanism of the BIST sequence*/
    REG_RMW32((base_addr + BISTTO1), 0U, 16U, bist_timeout1);
    /*set the time-out value of timeout2 mechanism of the BIST sequence*/
    REG_RMW32((base_addr + BISTTO2), 0U, 8U, bist_timeout2);
    /*start a bist sequence*/
    REG_RMW32((base_addr + BISTCTRL), 0U, 1U, 0x01U);

    while (((REG_READ32(base_addr + BISTDONE) & 0x01U) == 0U) && (wait_cnt < 500000U)) {
        //Mcu_udelay(1U);
        udelay(1U);
        wait_cnt++;
    }

    /*Clr bist done status*/
    REG_RMW32((base_addr + BISTCTRL), 1U, 1U, 0x01U);
    /*Clr mission fault*/
    REG_RMW32((base_addr + INTCLR), 0U, 2U, 0X3);
}

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Enable IRAM memory/Uncorrectable wdata_fatal signal interrupt*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Sig_Int_En
(
    VAR(IramCtlType, AUTOMATIC) IramCtl,
    VAR(boolean, AUTOMATIC)     Enable
)
{
    uint32 Iram_Mem_Sig = 0U;

    if (Enable == TRUE) {
        Iram_Mem_Sig = (uint32)(BM_MEM_ERR_INT_SIG_EN_SIG_ERR_EN           | \
                                BM_MEM_ERR_INT_SIG_EN_MUL_ERR_EN        | \
                                BM_MEM_ERR_INT_SIG_EN_WR_RDATA_FATAL_EN | \
                                BM_MEM_ERR_INT_SIG_EN_RDATA_FATAL_EN);
    } else {
        Iram_Mem_Sig = 0x0U;
    }

    /*Enable iram signal*/
    Iram_Inject_Err_Int_StaSig_En(MEM_ERR_SIG, IramCtl, 0U, 4U, Iram_Mem_Sig);
}

/*Enable IRAM memory status interrupt*/
static FUNC(void, FUNCSAFETYPKG_CODE) Enable_Iram_Mem_Sta_Int
(
    VAR(IramCtlType, AUTOMATIC) IramCtl,
    VAR(boolean, AUTOMATIC)     Enable
)
{
    uint32 Iram_Mem_Sta = 0U;

    if (Enable == TRUE) {
        Iram_Mem_Sta = (uint32)(BM_MEM_ERR_INT_STA_EN_SIG_ERR_EN           | \
                                BM_MEM_ERR_INT_STA_EN_MUL_ERR_EN        | \
                                BM_MEM_ERR_INT_STA_EN_WR_RDATA_FATAL_EN | \
                                BM_MEM_ERR_INT_STA_EN_RDATA_FATAL_EN);
    } else {
        Iram_Mem_Sta = 0x0U;
    }

    /*Enable iram status*/
    Iram_Inject_Err_Int_StaSig_En(MEM_ERR_STA, IramCtl, 0U, 4U, Iram_Mem_Sta);
}

/*Enable IRAM memory error injection point*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Eic_point_En
(
    VAR(IramCtlType, AUTOMATIC) IramCtl,
    VAR(boolean, AUTOMATIC)     Enable
)
{
    if (IramCtl == IRAMCtrl1) {
        Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, IRAM1_EIC_POINT, Enable);
    } else if (IramCtl == IRAMCtrl2) {
        Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SEC_MODULE, IRAM2_EIC_POINT, Enable);
    } else if (IramCtl == IRAMCtrl3) {
        Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SEC_MODULE, IRAM3_EIC_POINT, Enable);
    } else if (IramCtl == IRAMCtrl4) {
        Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SEC_MODULE, IRAM4_EIC_POINT, Enable);
    } else {
        /*do not anything*/
    }
}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Enable IRAM AXI signal interrupt for correct and uncorrect awaddr wdata*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Axi_Sig_Int_En
(
    VAR(boolean, AUTOMATIC)     Enable
)
{
    if (Enable == TRUE) {
        Iram_Inject_Err_Int_StaSig_En(COR_ERR_SIG, IRAMCtrl1, 0U, 4U, CERR_INT_STA_RST_MASK);
        Iram_Inject_Err_Int_StaSig_En(UNCOR_ERR_SIG, IRAMCtrl1, 0, 32U, UNCERR_INT_STA_RST_MASK);
    } else {
        Iram_Inject_Err_Int_StaSig_En(COR_ERR_SIG, IRAMCtrl1, 0U, 4U, 0U);
        Iram_Inject_Err_Int_StaSig_En(UNCOR_ERR_SIG, IRAMCtrl1, 0, 32U, 0U);
    }
}

/*Enable IRAM AXI status interrupt for correct and uncorrect awaddr wdata*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Axi_Sta_Int_En
(
    VAR(boolean, AUTOMATIC)     Enable
)
{
    if (Enable == TRUE) {
        Iram_Inject_Err_Int_StaSig_En(COR_ERR_STA, IRAMCtrl1, 0U, 4U, CERR_INT_STA_RST_MASK);
        Iram_Inject_Err_Int_StaSig_En(UNCOR_ERR_STA, IRAMCtrl1, 0, 32U, UNCERR_INT_STA_RST_MASK);
    } else {
        Iram_Inject_Err_Int_StaSig_En(COR_ERR_STA, IRAMCtrl1, 0U, 4U, 0U);
        Iram_Inject_Err_Int_StaSig_En(UNCOR_ERR_STA, IRAMCtrl1, 0, 32U, 0U);
    }
}
#endif

/*Clear error status through scr*/
static FUNC(void, FUNCSAFETYPKG_CODE) Scr_Clr_Error_Status(uint32 reg_addr, uint32 setclr_pos,
        uint32 get_pos)
{
    Mcu_ScrSetBit(reg_addr, R16W16_RW, setclr_pos);

    while (!Mcu_ScrGetBit(reg_addr, R16W16_RO, get_pos));

    Mcu_ScrClrBit(reg_addr, R16W16_RW, setclr_pos);
}

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Clear status for iram correction error interrupt status for error injection on fabric*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Corr_Err_Int_Sta_Clr(void)
{
    for (uint8 index = 0U; index < 4U; index++) {
        if (index != 2U) {
            if ((Get_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index))) == TRUE) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index));
            } else {
                /*do not anything*/
            }
        } else {
            continue;
        }
    }

    /*Uncorrectable on AXI bus*/
    for (uint8 index = 0U; index < 4U; index++)
    {
        if (Get_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index)))
        {
            Clr_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index));
        }
    }

}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Clear status for fabric uncorrection error interrupt status */
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Uncerr_Sta_Check_Clr(void)
{
    /*Uncorrectable on AXI bus*/
    for (uint8 index = 0U; index < 32U; index++) {
        if (((1 << index) & UNCERR_INT_STA_RST_MASK) != 0U) {
            if (Get_Iram_Inj_Err_Int_Status(IRAM_UNCORR_INT, IRAMCtrl1, (1U << index))) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_UNCORR_INT, IRAMCtrl1, (1U << index));
            }
        } else {
            continue;
        }
    }

     /*Uncorrectable on AXI bus*/
    for (uint8 index = 0U; index < 4U; index++)
    {
        if (Get_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index)))
        {
            Clr_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IRAMCtrl1, (1U << index));
        }
    }


}
#endif

/*Clear corresponding error status for SEM interrupt input */
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_SigErr_Interrupt_Status_Clr
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
)
{
    /*Error in iram memory*/
    for (uint8 index = 0U; index < 4U; index++) {
        if (index == 1U) {
            continue;
        } else {
            if (Get_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index))) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index));
            }
        }
    }

    /*Correctable on AXI bus*/
    for (uint8 index = 0U; index < 4U; index++) {
        if (index == 2U) {
            continue;
        } else {
            if (Get_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IramCtl, (1U << index))) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IramCtl, (1U << index));
            }
        }
    }
}

/*Clear iram multiple error interrupt status*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_MulErr_Interrupt_Status_Clr
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
)
{
    /*Error in iram memory*/
    for (uint8 index = 0U; index < 4U; index++) {
        if (index == 0U) {
            continue;
        } else {
            if (Get_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index))) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index));
            }
        }
    }

    /*Uncorrectable on AXI bus*/
    for (uint8 index = 0U; index < 32U; index++) {
        if (((1 << index) & UNCERR_INT_STA_RST_MASK) != 0U) {
            if (Get_Iram_Inj_Err_Int_Status(IRAM_UNCORR_INT, IramCtl, (1U << index))) {
                Clr_Iram_Inj_Err_Int_Status(IRAM_UNCORR_INT, IramCtl, (1U << index));
            }
        } else {
            continue;
        }
    }


    /*Uncorrectable on AXI bus*/
    for (uint8 index = 0U; index < 4U; index++)
    {
        if (Get_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IramCtl, (1U << index)))
        {
            Clr_Iram_Inj_Err_Int_Status(IRAM_CORR_INT, IramCtl, (1U << index));
        }
    }

}

/*Check sem interrupt status for iram and clear interrupt status*/
static FUNC(void, FUNCSAFETYPKG_CODE) Iram_Sem_Interrupt_Status_Chk
(
    VAR(Sem_Int_Input, AUTOMATIC) SemIntInputSig
)
{
    if (SemIntInputSig == IRAM1____SIG_ERR_INT) {
        Iram_SigErr_Interrupt_Status_Clr(IRAMCtrl1);
    } else if (SemIntInputSig == IRAM1____MUL_ERR_INT) {
        Iram_MulErr_Interrupt_Status_Clr(IRAMCtrl1);
    } else if (SemIntInputSig == IRAM2____SIG_ERR_INT) {
        Iram_SigErr_Interrupt_Status_Clr(IRAMCtrl2);
    } else if (SemIntInputSig == IRAM2____MUL_ERR_INT) {
        Iram_MulErr_Interrupt_Status_Clr(IRAMCtrl2);
    } else if (SemIntInputSig == IRAM3____SIG_ERR_INT) {
        Iram_SigErr_Interrupt_Status_Clr(IRAMCtrl3);
    } else if (SemIntInputSig == IRAM3____MUL_ERR_INT) {
        Iram_MulErr_Interrupt_Status_Clr(IRAMCtrl3);
    } else if (SemIntInputSig == IRAM4____SIG_ERR_INT) {
        Iram_SigErr_Interrupt_Status_Clr(IRAMCtrl4);
    } else {
        Iram_MulErr_Interrupt_Status_Clr(IRAMCtrl4);
    }
}

/*Clear status:PWR_LAT_SAF____RTC_POWER_LAT PWR_LAT_SAF____AP_POWER_LAT PWR_LAT_RTC____AP_POWER_LAT*/
static FUNC(void, FUNCSAFETYPKG_CODE) Pwr_Lat_Power_Clr(void)
{
    /*Clear status for PWR_LAT_SAF____RTC_POWER_LAT*/
    if (Sem_Interrupt_Status(SEMgr1, PWR_LAT_SAF____RTC_POWER_LAT) == TRUE) {
        Scr_Clr_Error_Status(APB_SCR_SAF_BASE, SCR_SAF_PWR_LAT_SAF_RTC_POWER_LAT_CLR_R16W16_RW_START_BIT,
                             SCR_SAF_PWR_LAT_SAF_RTC_POWER_LAT_CLR_ACK_R16W16_RO_START_BIT);
    }

    /*Clear status for PWR_LAT_SAF____AP_POWER_LAT*/
    if (Sem_Interrupt_Status(SEMgr1, PWR_LAT_SAF____AP_POWER_LAT) == TRUE) {
        Scr_Clr_Error_Status(APB_SCR_SAF_BASE, SCR_SAF_PWR_LAT_SAF_AP_POWER_LAT_CLR_R16W16_RW_START_BIT,
                             SCR_SAF_PWR_LAT_SAF_AP_POWER_LAT_CLR_ACK_R16W16_RO_START_BIT);
    }

    /*Clear status for PWR_LAT_RTC____AP_POWER_LAT*/
    if (Sem_Interrupt_Status(SEMgr1, PWR_LAT_RTC____AP_POWER_LAT) == TRUE) {
        Scr_Clr_Error_Status(APB_SCR_SAF_BASE, SCR_SAF_PWR_LAT_RTC_AP_POWER_LAT_CLR_R16W16_RW_START_BIT,
                             SCR_SAF_PWR_LAT_RTC_AP_POWER_LAT_CLR_ACK_R16W16_RO_START_BIT);
    }
}

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
#ifndef CONFIG_BOARD_X9SP
/*Check and clear sem interrupt status for injecting 1bit error in fabric E2*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E2_1bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;
    boolean sem_int_flg = FALSE;
    /*index_arry for storing scr status index from g_SemScrinfo[]*/
    Sem_Scr_Index e2_index_arry[5U] = {SEC_NOCMAIN_M_12_MAINSGLDATAECCERR, SEC_NOCMAIN_M_12_MAINSGLAWADDRECCERR, SEC_NOCMAIN_M_12_MAINSGLARADDRECCERR, SAF_NOCSAF_S_1_MAINMAIN_SGLDATAECCERR, SAF_NOCSAF_S_1_MAINDUPL_SGLDATAECCERR};

    /*To check and clear sem interrupt input status through SCR*/
    for (uint8 index = 0U; index < 5U; index++) {
        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[e2_index_arry[index]].u32SemIntId)) == TRUE) {
            sem_int_flg = TRUE;
            Scr_Clr_Error_Status(g_SemScrinfo[e2_index_arry[index]].u32DominAddr,
                                 g_SemScrinfo[e2_index_arry[index]].u32StartPos, g_SemScrinfo[e2_index_arry[index]].u32AckPos);
        }
    }

    /*To check and clear sem interrupt input status through Mission mode*/
    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        sem_int_flg = TRUE;
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else if ((Sem_Interrupt_Status(SEMgr1, NOC_MAIN_____SOC_MAINMISSIONINTBAR) == TRUE)
               || (Sem_Interrupt_Status(SEMgr1, NOC_MAIN____SOC_MAINMISSIONINT) == TRUE)) {
        sem_int_flg = TRUE;
#ifndef CONFIG_BOARD_X9SP
        NOC_Mission_Interrupt_Clr(NOC_HPIA, 0X200U, 0X80U);
#endif
    }

    /*Check if sem interrupt set when inject 1 bit error on fabric E2*/
    if (sem_int_flg == FALSE) {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric1bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric1bitUnDetectNotification(FabricErrCode);
        }
    } else {
        sem_int_flg = FALSE;
    }
}
#endif

#if (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 2bits error in fabric E2*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E2_2bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;

    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else if ((Sem_Interrupt_Status(SEMgr1, NOC_MAIN_____SOC_MAINMISSIONINTBAR) == TRUE)
               || (Sem_Interrupt_Status(SEMgr1, NOC_MAIN____SOC_MAINMISSIONINT) == TRUE)) {
#ifndef CONFIG_BOARD_X9SP
        NOC_Mission_Interrupt_Clr(NOC_HPIA, 0X200U, 0X80U);
#endif
    } else {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric2bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric2bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#endif/*CONFIG_BOARD_X9SP*/
#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
#ifndef CONFIG_BOARD_X9SP
/*Check and clear sem interrupt status for injecting 1/2bits error in fabric E2*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E2_InjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index,
    VAR(Eic_Error_BitType, AUTOMATIC)    _err_bit
)
{
    if (_err_bit == ONE_BIT_EIC_ERR) {
        Fabric_E2_1bitInjErr_IntSta_ChkClr(_signal_index);
    } else {
        Fabric_E2_2bitInjErr_IntSta_ChkClr(_signal_index);
    }
}
#endif/*CONFIG_BOARD_X9SP*/
#endif

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 1bit error in fabric E5 and E6*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E5E6_1bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;

    if (Sem_Interrupt_Status(SEMgr1, IRAM1____SIG_ERR_INT) == TRUE) {
        Iram_Corr_Err_Int_Sta_Clr();
    } else if (Sem_Interrupt_Status(SEMgr1, IRAM1____MUL_ERR_INT) == TRUE) {
        Iram_Uncerr_Sta_Check_Clr();
    } else if ((Sem_Interrupt_Status(SEMgr1,
                                     (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32SemIntId)) == TRUE)
               || (Sem_Interrupt_Status(SEMgr1,
                                        (Sem_Int_Input)g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32SemIntId) == TRUE)) {
        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32SemIntId) == TRUE) {
            /*Clear status for NOC_SAF____s_2_mainMain_SglDataEccErr*/
            Scr_Clr_Error_Status(g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32DominAddr,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32StartPos,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32AckPos);
        }

        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32SemIntId)) == TRUE) {
            /*Clear status for NOC_SAF____s_2_mainDupl_SglDataEccErr*/
            Scr_Clr_Error_Status(g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32DominAddr,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32StartPos,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32AckPos);
        }
    } else if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
               || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric1bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric1bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#if (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 2bits error in fabric E5 and E6*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E5E6_2bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;

    if ((Sem_Interrupt_Status(SEMgr1,
                              (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32SemIntId)) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32SemIntId)) == TRUE)) {
        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32SemIntId)) == TRUE) {
            /*Clear status for NOC_SAF____s_2_mainMain_SglDataEccErr*/
            Scr_Clr_Error_Status(g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32DominAddr,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32StartPos,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR].u32AckPos);
        }

        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32SemIntId)) == TRUE) {
            /*Clear status for NOC_SAF____s_2_mainDupl_SglDataEccErr*/
            Scr_Clr_Error_Status(g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32DominAddr,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32StartPos,
                                 g_SemScrinfo[SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR].u32AckPos);
        }
    }

    if ((Sem_Interrupt_Status(SEMgr1, IRAM1____MUL_ERR_INT) == TRUE) || \
        (Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE) || \
        (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        if (Sem_Interrupt_Status(SEMgr1, IRAM1____MUL_ERR_INT) == TRUE) {
            Iram_Uncerr_Sta_Check_Clr();
        }

        if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
            || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
            NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
        }

    } else {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric2bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric2bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Check and clear sem interrupt status for injecting 1/2bits error in fabric E5 and E6*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E5E6_InjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index,
    VAR(Eic_Error_BitType, AUTOMATIC)    _err_bit
)
{
    if (_err_bit == ONE_BIT_EIC_ERR) {
        Fabric_E5E6_1bitInjErr_IntSta_ChkClr(_signal_index);
    } else {
        Fabric_E5E6_2bitInjErr_IntSta_ChkClr(_signal_index);
    }
}
#endif

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 1bit error in fabric E7*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E7_1bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;
    boolean sem_int_flg = FALSE;
    /*index_arry for storing scr status index from g_SemScrinfo[]*/
    Sem_Scr_Index e7_index_arry[4U] = {SAF_NOCSAF_M_1_MAINDUPL_SGLECCERR, SAF_NOCSAF_M_1_MAINMAIN_SGLECCERR, SAF_CR5SAF_AXIMCORR0, SAF_CR5SAF_AXIMFATAL0_2};

    /*To check and clear sem interrupt input status through SCR*/
    for (uint8 index = 0U; index < 4U; index++) {
        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[e7_index_arry[index]].u32SemIntId)) == TRUE) {
            sem_int_flg = TRUE;
            Scr_Clr_Error_Status(g_SemScrinfo[e7_index_arry[index]].u32DominAddr,
                                 g_SemScrinfo[e7_index_arry[index]].u32StartPos, g_SemScrinfo[e7_index_arry[index]].u32AckPos);
        }
    }

    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        sem_int_flg = TRUE;
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    }

    if (sem_int_flg == FALSE) {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric1bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric1bitUnDetectNotification(FabricErrCode);
        }
    } else {
        sem_int_flg = FALSE;
    }
}
#endif

#if (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 2bits error in fabric E7*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E7_2bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;

    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric2bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric2bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Check and clear sem interrupt status for injecting 1/2bits error in fabric E7*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E7_InjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index,
    VAR(Eic_Error_BitType, AUTOMATIC)    _err_bit
)
{
    if (_err_bit == ONE_BIT_EIC_ERR) {
        Fabric_E7_1bitInjErr_IntSta_ChkClr(_signal_index);
    } else {
        Fabric_E7_2bitInjErr_IntSta_ChkClr(_signal_index);
    }
}
#endif

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 1bit error in fabric E8*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E8_1bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;
    boolean sem_int_flg = FALSE;
    /*index_arry for storing scr status index from g_SemScrinfo[]*/
    Sem_Scr_Index e8_index_arry[8U] = {SAF_NOCSAF_M_0_MAINDUPL_SGLECCERR, SAF_NOCSAF_M_0_MAINMAIN_SGLECCERR, SAF_CR5SAF_PPXCORR0, SAF_CR5SAF_PPXFATAL0_0, SAF_CR5SAF_PPXFATAL0_1, SAF_CR5SAF_PPXFATAL0_2, SAF_CR5SAF_PPXFATAL0_3, SAF_CR5SAF_PPXFATAL0_4};

    /*To check and clear sem interrupt input status through SCR*/
    for (uint8 index = 0U; index < 8U; index++) {
        if (Sem_Interrupt_Status(SEMgr1,
                                 (Sem_Int_Input)(g_SemScrinfo[e8_index_arry[index]].u32SemIntId)) == TRUE) {
            sem_int_flg = TRUE;
            Scr_Clr_Error_Status(g_SemScrinfo[e8_index_arry[index]].u32DominAddr,
                                 g_SemScrinfo[e8_index_arry[index]].u32StartPos, g_SemScrinfo[e8_index_arry[index]].u32AckPos);
        }
    }

    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE) || \
        (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        sem_int_flg = TRUE;
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    }

    if (sem_int_flg == FALSE) {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric1bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric1bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#if (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Check and clear sem interrupt status for injecting 2bits error in fabric E8*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E8_2bitInjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index
)
{
    uint32 FabricErrCode = 0U;

    if ((Sem_Interrupt_Status(SEMgr1, NOC_SAF____PD_SAF_MAINMISSIONINT) == TRUE)
        || (Sem_Interrupt_Status(SEMgr1, NOC_SAF_____PD_SAF_MAINMISSIONINTBAR) == TRUE)) {
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else if (Sem_Interrupt_Status(SEMgr1,
                                    (Sem_Int_Input)(g_SemScrinfo[SAF_CR5SAF_PPXFATAL0_4].u32SemIntId)) == TRUE) {
        /*Clear status for CR5_SAF____ppxfatal0_4_*/
        Scr_Clr_Error_Status(g_SemScrinfo[SAF_CR5SAF_PPXFATAL0_4].u32DominAddr,
                             g_SemScrinfo[SAF_CR5SAF_PPXFATAL0_4].u32StartPos, g_SemScrinfo[SAF_CR5SAF_PPXFATAL0_4].u32AckPos);
    } else {
        FabricErrCode = _signal_index;

        if (g_FabricErrCurConfig->pFabric2bitUnDetectNotification != NULL_PTR) {
            g_FabricErrCurConfig->pFabric2bitUnDetectNotification(FabricErrCode);
        }
    }
}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Check and clear sem interrupt status for injecting 1/2bits error in fabric E8*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fabric_E8_InjErr_IntSta_ChkClr
(
    VAR(enum eic_signal, AUTOMATIC)      _signal_index,
    VAR(Eic_Error_BitType, AUTOMATIC)    _err_bit
)
{
    if (_err_bit == ONE_BIT_EIC_ERR) {
        Fabric_E8_1bitInjErr_IntSta_ChkClr(_signal_index);
    } else {
        Fabric_E8_2bitInjErr_IntSta_ChkClr(_signal_index);
    }
}
#endif

/*Enable signal interrupt for fuse*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Interrupt_Sig_En
(
    VAR(boolean, AUTOMATIC) Enable
)
{
    uint32 value = 0U;

    if (Enable == TRUE) {
        value = BM_INT_SIG_EN_FATAL_ERR | \
                BM_INT_SIG_EN_MUL_ERR   | \
                BM_INT_SIG_EN_RED_ERR   | \
                BM_INT_SIG_EN_SIG_ERR;
    } else {
        value = 0U;
    }

    Fuse_Interrupt_SigSta_Enable(FUSE_INT_SIG, 0U, 4U, value);
}

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Trig reload fuse word for sw load*/
static FUNC(void, FUNCSAFETYPKG_CODE) Trig_Reload_Fuse_Word_SwLoad(void)
{
    uint32 waitcnt = 0U;

    while (((Get_Fuse_Ctrl_Value(BM_FUSE_CTRL_BUSY) == TRUE) \
            || (Get_Fuse_Ctrl_Value(BM_FUSE_CTRL_ERROR) == TRUE)) \
           && (waitcnt < 500000U)) {
        //Mcu_udelay(1U);
        udelay(1U);
        waitcnt++;
    }

    /*reload fuse macro into shadow register*/
    (void)Set_Fuse_Trig_Value(TRUE);
    waitcnt = 0U;

    while (((Get_Fuse_Ctrl_Value(BM_FUSE_CTRL_BUSY) == TRUE) \
            || (Get_Fuse_Trig_Value(BM_FUSE_TRIG_LOAD) == TRUE)) \
           && (waitcnt < 500000U)) {
        //Mcu_udelay(1U);
        udelay(1U);
        waitcnt++;
    }
}

/*Enable status interrupt for fuse*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Interrupt_Sta_En
(
    VAR(boolean, AUTOMATIC) Enable
)
{
    uint32 value = 0U;

    if (Enable == TRUE) {
        value = BM_INT_STA_EN_FATAL_ERR | \
                BM_INT_STA_EN_MUL_ERR   | \
                BM_INT_STA_EN_RED_ERR   | \
                BM_INT_STA_EN_SIG_ERR;
    } else {
        value = 0U;
    }

    Fuse_Interrupt_SigSta_Enable(FUSE_INT_STA, 0U, 4U, value);
}

/*Inject 1/2bits error in function fuse for read data and ecc code for ecc mode*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fuse_InjErr_SigMulErr_Chk
(
    VAR(uint32, AUTOMATIC)                  ErrWordBit,
    VAR(FuseErrStaType, AUTOMATIC)          Err_Sta,
    VAR(Fuse_Error_BitType, AUTOMATIC)      Err_Bit
)
{
    uint8 index = 0U;
    uint32 ErrWordCode = 0U;

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

    /*TMV[223:0] function fuse1 161~167*/
    for (index = 153U; index < 160U; index++) {
#else
    index = 153U;
#endif

        if (Get_Fuse_Error_Status(Err_Sta, index) == TRUE) {
            Clr_Fuse_Error_Status(Err_Sta, index);
        } else {
            ErrWordCode = (Err_Sta << 16U) + ((index + 8U) << 8U) + ErrWordBit;

            if (Err_Bit == ONE_BIT_FUSE_ERR) {
                if (g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification != NULL_PTR) {
                    g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification(ErrWordCode);
                }
            } else {
                if (g_FuseErrCurConfig->pFuse2bitErrStaUnDetectNotification != NULL_PTR) {
                    g_FuseErrCurConfig->pFuse2bitErrStaUnDetectNotification(ErrWordCode);
                }
            }
        }

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)
    }

#endif
}
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2bits error in function fuse for reduncancy fuse data for redundancy mode*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fuse_InjErr_RedErr_Chk
(
    VAR(uint32, AUTOMATIC)    ErrWordBit
)
{
    uint8 index = 0U;
    uint32 ErrWordCode = 0U;

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

    for (index = 0U; index < 20U; index++) {
#else
    index = 0U;
#endif

        if (Get_Fuse_Error_Status(REDERR, index) == TRUE) {
            Clr_Fuse_Error_Status(REDERR, index);
        } else {
            ErrWordCode = (REDERR << 16U) + ((index + 168U) << 8U) + ErrWordBit;

            if (g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification != NULL_PTR) {
                g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification(ErrWordCode);
            }
        }

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)
    }

#endif
}

/*Inject 1/2bits error in function fuse for lock reduncancy fuse data for redundancy mode*/
static FUNC(void, FUNCSAFETYPKG_CODE) Fuse_InjErr_LockRedErr_Chk
(
    VAR(uint32, AUTOMATIC)    ErrWordBit
)
{
    uint8 index = 0U;
    uint32 ErrWordCode = 0U;

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

    for (index = 0U; index < 8U; index++) {
#else
    index = 0U;
#endif

        if (Get_Fuse_Error_Status(LOCKREDERR, index) == TRUE) {
            Clr_Fuse_Error_Status(LOCKREDERR, index);
        } else {
            ErrWordCode = (LOCKREDERR << 16U) + (index << 8U) + ErrWordBit;

            if (g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification != NULL_PTR) {
                g_FuseErrCurConfig->pFuse1bitErrStaUnDetectNotification(ErrWordCode);
            }
        }

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)
    }

#endif
}
#endif

/*Initial SEM interrupt for PAD/CPU/SYS_CTRL*/
static FUNC(void, FUNCSAFETYPKG_CODE) SemInterruptInit(void)
{
    uint8 i, j, k, l = 0U;

    Sem_Set_Error_Div(SEMgr1);          //zhong.changjun on 20220822

    /*Recoverable error for PAD interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput0 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_PAD0, TRUE);

        for (i = 0U; i < SEM_INT_OUT0_NUM; i++) {
            Sem_Interrupt_Enable(SEMgr1, g_SemIntCurConfig->pSemIntOutput0[i], SEM_OUTPUT_PAD0, TRUE);
        }
    }

    /*Fatal error for PAD interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput1 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_PAD1, TRUE);

        for (j = 0U; j < SEM_INT_OUT1_NUM; j++) {
            Sem_Interrupt_Enable(SEMgr1, g_SemIntCurConfig->pSemIntOutput1[j], SEM_OUTPUT_PAD1, TRUE);
        }
    }

    /*CPU interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput2 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_CPU, TRUE);

        for (k = 0U; k < SEM_INT_OUT2_NUM; k++) {
            Sem_Interrupt_Enable(SEMgr1, g_SemIntCurConfig->pSemIntOutput2[k], SEM_OUTPUT_CPU, TRUE);
        }
    }

    /*Sys_Ctrl interrupt to reset*/
    if (g_SemIntCurConfig->pSemIntOutput3 != NULL_PTR) {
        /*enable global reset for sem*/
        REG_RMW32(APB_RSTGEN_SAF_BASE, 1U, 1U, 1U);
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_SYS, TRUE);

        for (l = 0U; l < SEM_INT_OUT3_NUM; l++) {
            Sem_Interrupt_Enable(SEMgr1, g_SemIntCurConfig->pSemIntOutput3[l], SEM_OUTPUT_SYS, TRUE);
        }
    }

    /*Enable fuse signal enable */
    (void)Fuse_Interrupt_Sig_En(TRUE);

}
#if ((FUSAPKG_SELF_TEST_POWER_ON == STD_ON) || (FUSAPKG_SELF_TEST_PERIOD == STD_ON))
/*Initial SEM interrupt for PAD/CPU/SYS_CTRL*/
FUNC(void, FUNCSAFETYPKG_CODE) SemOutInterruptEnable(boolean enable)
{
    /*Recoverable error for PAD interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput0 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_PAD0, enable);
    }

    /*Fatal error for PAD interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput1 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_PAD1, enable);
    }

    /*CPU interrupt*/
    if (g_SemIntCurConfig->pSemIntOutput2 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_CPU, enable);
    }

    /*Sys_Ctrl interrupt to reset*/
    if (g_SemIntCurConfig->pSemIntOutput3 != NULL_PTR) {
        Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_SYS, enable);
    }
}
#endif

static FUNC(void, FUNCSAFETYPKG_CODE) Sem_IntSta_ScrClr
(
    VAR(Sem_Int_Input, AUTOMATIC)         sem_intsta_id
)
{
    for (uint16 index = 0U; index < SEM_SCR_NUM; index++) {
        if (sem_intsta_id == g_SemScrinfo[index].u32SemIntId) {
            Scr_Clr_Error_Status(g_SemScrinfo[index].u32DominAddr, g_SemScrinfo[index].u32StartPos,
                                 g_SemScrinfo[index].u32AckPos);
            break;
        }
    }
}

/* Cheak and clear sem interrupt status via BIST mode*/
static FUNC(void, FUNCSAFETYPKG_CODE) Sem_IntSta_BistClr
(
    VAR(Sem_Int_Input, AUTOMATIC)         sem_intsta_id
)
{
    /*To clear sem interrupt input status in Mission mode*/
    if (sem_intsta_id == NOC_SAF____PD_SAF_MAINMISSIONINT) {
        NOC_Mission_Interrupt_Clr(NOC_SAFE, 0X30U, 0X10U);
    } else if (sem_intsta_id == NOC_MAIN____SOC_MAINMISSIONINT) {
#ifndef CONFIG_BOARD_X9SP
        NOC_Mission_Interrupt_Clr(NOC_HPIA, 0X200U, 0X80U);
#endif
    } else if (sem_intsta_id == NOC_MAINB____HPIB_MAINMISSIONINT) {
        NOC_Mission_Interrupt_Clr(NOC_HPIB, 0X200U, 0X80U);
    } else if (sem_intsta_id == NOC_SEC____PD_SEC_MAINMISSIONINT) {
        NOC_Mission_Interrupt_Clr(NOC_SEC, 0X30U, 0X10U);
    } else {
        /*For misra*/
    }
}

/* Cheak and clear sem interrupt status in Iram*/
static FUNC(void, FUNCSAFETYPKG_CODE) Sem_IntSta_IramClr
(
    VAR(Sem_Int_Input, AUTOMATIC)         sem_intsta_id
)
{
    /*To clear sem interrupt input status through Iram memory*/
    for (Sem_Int_Input index = IRAM1____SIG_ERR_INT; index <= IRAM4____MUL_ERR_INT; index++) {
        if (sem_intsta_id == index) {
            Iram_Sem_Interrupt_Status_Chk(index);
            break;
        }
    }
}

static FUNC(void, FUNCSAFETYPKG_CODE) Sem_IntSta_Clr
(
    VAR(Sem_Int_Input, AUTOMATIC)         sem_intsta_id
)
{
    /*To clear sem interrupt input status via SCR*/
    Sem_IntSta_ScrClr(sem_intsta_id);
    /*Cheak and clear sem interrupt status via BIST mode*/
    Sem_IntSta_BistClr(sem_intsta_id);
    /*To clear sem interrupt input status in Iram*/
    //printf("0xf01f0000=0x%x\n", (uint32_t)readl(0xf01f0000));
    //printf("0xf01f000c=0x%x\n", (uint32_t)readl(0xf01f000c));
    //printf("0xf01f0018=0x%x\n", (uint32_t)readl(0xf01f0018));
    //printf("0xf0320080=0x%x\n", (uint32_t)readl(0xf0320080));

    Sem_IntSta_IramClr(sem_intsta_id);
}

/*************************************************Global function************************************/
#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*1/2bits Error injection for fabric signal,then wr/rd value and compare them*/
FUNC(void, FUNCSAFETYPKG_CODE) Eic_Inject_Err_WrRd_Val_Comp_Func
(
    VAR(enum eic_signal, AUTOMATIC)  _signal_index,
    VAR(uint32, AUTOMATIC)           _addr,
    VAR(uint32, AUTOMATIC)           _wr_val,
    VAR(Eic_Error_BitType, AUTOMATIC)    _err_bit
)
{
    uint32  _rd_val_temp = 0U;
    _rd_val_temp = readl(_addr);

    if (_err_bit == ONE_BIT_EIC_ERR) {
        Eic_Enable_Injectpoint_BySignal(ONE_BIT_EIC_ERR, _signal_index, TRUE);
        Eic_Inject_Bit(ONE_BIT_EIC_ERR, _signal_index, TRUE);

    } else {
        Eic_Enable_Injectpoint_BySignal(TWO_BIT_EIC_ERR, _signal_index, TRUE);
        Eic_Inject_Bit(TWO_BIT_EIC_ERR, _signal_index, TRUE);
    }

    dsb();
    isb();
    writel(_wr_val, _addr);
    dsb();
    isb();
    (void)readl(_addr);
    dsb();
    isb();

    if (_err_bit == ONE_BIT_EIC_ERR) {
        Eic_Enable_Injectpoint_BySignal(ONE_BIT_EIC_ERR, _signal_index, FALSE);
        Eic_Inject_Bit(ONE_BIT_EIC_ERR, _signal_index, FALSE);
    } else {
        Eic_Enable_Injectpoint_BySignal(TWO_BIT_EIC_ERR, _signal_index, FALSE);
        Eic_Inject_Bit(TWO_BIT_EIC_ERR, _signal_index, FALSE);
    }

    dsb();
    isb();
    writel(_rd_val_temp, _addr);
}

/*Inject 1/2 bits error on fabric*/
FUNC(void, FUNCSAFETYPKG_CODE) Fabric_Inject_SigMulErr
(
    VAR(Error_PosType, AUTOMATIC)         err_position,
    VAR(enum eic_signal, AUTOMATIC)       signal_first,
    VAR(enum eic_signal, AUTOMATIC)       signal_last,
    VAR(uint32, AUTOMATIC)                _addr,
    VAR(uint32, AUTOMATIC)                _wr_val,
    VAR(Eic_Error_BitType, AUTOMATIC)     _err_bit
)
{
    if (err_position == E2) {
#ifndef CONFIG_BOARD_X9SP
#if (FABRIC_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

        for (enum eic_signal signal_index = signal_first; signal_index <= signal_last; signal_index++) {
            Eic_Inject_Err_WrRd_Val_Comp_Func(signal_index,  _addr, _wr_val, _err_bit);
            Fabric_E2_InjErr_IntSta_ChkClr(signal_index, _err_bit);
        }

#else
        Eic_Inject_Err_WrRd_Val_Comp_Func(signal_first,  _addr, _wr_val, _err_bit);
        Fabric_E2_InjErr_IntSta_ChkClr(signal_first, _err_bit);
#endif
#endif/* CONFIG_BOARD_X9SP*/
    } else if ((err_position == E5) || (err_position == E6)) {
#if (FABRIC_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

        for (enum eic_signal signal_index = signal_first; signal_index <= signal_last; signal_index++) {
            Eic_Inject_Err_WrRd_Val_Comp_Func(signal_index,  _addr, _wr_val, _err_bit);
            Fabric_E5E6_InjErr_IntSta_ChkClr(signal_index, _err_bit);
        }

#else
        Eic_Inject_Err_WrRd_Val_Comp_Func(signal_first,  _addr, _wr_val, _err_bit);
        Fabric_E5E6_InjErr_IntSta_ChkClr(signal_first, _err_bit);
#endif
    } else if (err_position == E7) {
#if (FABRIC_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

        for (enum eic_signal signal_index = signal_first; signal_index <= signal_last; signal_index++) {
            Eic_Inject_Err_WrRd_Val_Comp_Func(signal_index,  _addr, _wr_val, _err_bit);
            Fabric_E7_InjErr_IntSta_ChkClr(signal_index, _err_bit);
        }

#else

        Eic_Inject_Err_WrRd_Val_Comp_Func(signal_first,  _addr, _wr_val, _err_bit);
        Fabric_E7_InjErr_IntSta_ChkClr(signal_first, _err_bit);
#endif
    } else if (err_position == E8) {
#if (FABRIC_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

        for (enum eic_signal signal_index = signal_first; signal_index <= signal_last; signal_index++) {
            Eic_Inject_Err_WrRd_Val_Comp_Func(signal_index,  _addr, _wr_val, _err_bit);
            Fabric_E8_InjErr_IntSta_ChkClr(signal_index, _err_bit);
        }

#else
        Eic_Inject_Err_WrRd_Val_Comp_Func(signal_first,  _addr, _wr_val, _err_bit);
        Fabric_E8_InjErr_IntSta_ChkClr(signal_first, _err_bit);
#endif
    } else {
        /* do not anything*/
    }
}
#endif

/*Function safety package initial*/
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Init(void)
{
#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
    g_IramZoneCurConfig = Iram_ZoneConfig;
#endif
#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
    g_TCMErrCurConfig = &TCMErrConfig;
#endif
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
    g_DCACHEErrCurConfig = &DcacheErrConfig;
#endif
#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
    g_FabricErrCurConfig = &FabricErrConfig;
#endif
#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
    g_FuseErrCurConfig = &FuseErrConfig;
#endif
#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
    g_SemSignalMonCurConfig = &SemSigMonitorConfig;
#endif
#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
    g_BistCurConfig = &BistConfig;
#endif
#if (FUSAPKG_WDT_SELFTEST_POWER_ON == STD_ON)
    g_WdtSelfTestCurConfig = &WdtSelfTestConfig;
#endif
#if ((FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON) || (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON))
    g_FsrefClkChkCurConfig = &FsrefClkChkConfig;
#endif
    g_SemIntCurConfig = &SemIntMonConfig;
#if ((FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD == STD_ON))
    g_DualCoreLockStepCurConfig = &DualCoreLockStepInjectErrConfig;
#endif
#if (FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON == STD_ON)
    g_SocTempMonitorTstCurConfig = &SocTempMonitorTstConfig;
#endif
    /* Get Iram Controller address*/
    IramCtrl_GetBaseAddr();
    /* Get Iram injection error address*/
    Iram_GetInjErrBaseAddr();
    /* Get SEM address*/
    Sem_Ip_GetBaseAddr();
    /* Get EIC address*/
    EIC_Ip_GetBaseAddr();
    /* Clear status:PWR_LAT_SAF____RTC_POWER_LAT PWR_LAT_SAF____AP_POWER_LAT PWR_LAT_RTC____AP_POWER_LAT*/
    Pwr_Lat_Power_Clr();
    /* Initial SEM interrupt for PAD/CPU/SYS_CTRL*/
    SemInterruptInit();
    gFuncSafetyFinishedInit = TRUE;
}

/*Running Function Safety package when ECU power on*/
#if (FUSAPKG_SELF_TEST_POWER_ON == STD_ON)
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Run_PowerOn_Func(void)
{
    uint8 i = 0U;

    printf("FuncSafetyPkg_Run_PowerOn_Func  Enter\n");
    /*Disable SEM input interrupt*/
    SemOutInterruptEnable(FALSE);

    for (i = 0U; i < FUNCSAFETY_POWERONTASK_NUM; i++)
    {
        printf("FuncSafetyPkg_Run_PowerOn_Func  i=%d\n", i);
        #ifdef CONFIG_BOARD_X9SP
        continue;
        #endif
        if (gPoweronFuncSafetyTaskInfo[i].pFuncSafetyTask != NULL_PTR)
        {
            if ((i == 1U) || (i == 2U) ||(i==5u))
            {
                arch_disable_cache(DCACHE);
                gPoweronFuncSafetyTaskInfo[i].pFuncSafetyTask();
                arch_enable_cache(DCACHE);
            }
            else
            {
                gPoweronFuncSafetyTaskInfo[i].pFuncSafetyTask();
            }
        }
    }

    /*Enable SEM input interrupt*/
    SemOutInterruptEnable(TRUE);
    printf("FuncSafetyPkg_Run_PowerOn_Func  Leave\n");
}
#endif

/*Running Function Safety package periodically*/
#if (FUSAPKG_SELF_TEST_PERIOD == STD_ON)
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Run_Period_Func(void)
{
    uint8 i, j, k = 0U;
    /*Disable SEM input interrupt*/
    SemOutInterruptEnable(FALSE);
    gFuncSafetyTaskPeriodCnt  += 1U;

    for (i = 0U; i < FUNCSAFETY_PERIODTASK_NUM; i++) {
        if (gPeriodFuncSafetyTaskInfo[i].pFuncSafetyTask != NULL_PTR) {
            if (gFuncSafetyTaskPeriodCnt == gPeriodFuncSafetyTaskInfo[i].u8RunPeriodCnt) {
                /*Disable cache for injecting 1/2 bits error in Iram*/
                if (i == 0U) {
                    SuspendAllInterrupts();
                    arch_disable_cache(DCACHE);
                    gPeriodFuncSafetyTaskInfo[i].pFuncSafetyTask();
                    gPeriodFuncSafetyTaskInfo[i].bPerformedFlg = TRUE;
                    arch_enable_cache(DCACHE);
                    ResumeAllInterrupts();
                } else {
                    gPeriodFuncSafetyTaskInfo[i].pFuncSafetyTask();
                    gPeriodFuncSafetyTaskInfo[i].bPerformedFlg = TRUE;
                }
            }
        }
    }

    /*Polling if unperformed task */
    for (j = 0U; j < FUNCSAFETY_PERIODTASK_NUM; j++) {
        if (gPeriodFuncSafetyTaskInfo[j].pFuncSafetyTask != NULL_PTR) {
            if (gPeriodFuncSafetyTaskInfo[j].bPerformedFlg == FALSE) {
                break;
            }
        }
    }

    if (j == FUNCSAFETY_PERIODTASK_NUM) {
        for (k = 0U; k < FUNCSAFETY_PERIODTASK_NUM; k++) {
            if (gPeriodFuncSafetyTaskInfo[k].pFuncSafetyTask != NULL_PTR) {
                gPeriodFuncSafetyTaskInfo[k].bPerformedFlg = FALSE;
            }
        }

        gFuncSafetyTaskPeriodCnt = 0U;
    }

    /*Enable SEM input interrupt*/
    SemOutInterruptEnable(TRUE);
}
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1/2 bits error on fabric for all traversal or fixed point*/
FUNC(void, FUNCSAFETYPKG_CODE) Fabric_Inject_SigMulErr_Func(void)
{
    volatile unsigned long addr_Iram1 = InjErr_IramBaseAddr32[0];
#ifndef CONFIG_BOARD_X9SP
    const volatile unsigned long addr_sec_reg = (unsigned long)APB_EIC_SEC_BASE;
#endif
#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
#ifndef CONFIG_BOARD_X9SP
    const volatile unsigned long addr_safe_reg = (unsigned long)APB_EIC_SAF_BASE;
#endif
#endif
    uint32 val_wr = INJECT_ERR_VALUE;

    addr_Iram1 = ROUNDUP(addr_Iram1, 0x40);
    /*SEM enable error injection*/
    Sem_Enable_Eic(SEMgr1, TRUE);
    Iram_Axi_Sig_Int_En(TRUE);
    Iram_Axi_Sta_Int_En(TRUE);

#if (FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
#ifndef CONFIG_BOARD_X9SP
    /* Inject one bit error for E2*/
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____wctlcode_0_, E2____eic_safe____wctlcode_4_, addr_sec_reg,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____wdatacode_0_, E2____eic_safe____wdatacode_15_, addr_sec_reg,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____wdatacode_0_, E2____eic_safe____wdatacode_15_, addr_sec_reg,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____awidcode_0_, E2____eic_safe____awidcode_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____awctl0code_0_, E2____eic_safe____awctl1code_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____arctl0code_0_, E2____eic_safe____arctl1code_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____araddrcode_0_, E2____eic_safe____araddrcode_6_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____aridcode_0_, E2____eic_safe____aridcode_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____awaddrcode_0_, E2____eic_safe____awaddrcode_6_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____ridcode_0_, E2____eic_hpi____ridcode_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____rctlcode_0_, E2____eic_hpi____rctlcode_2_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____rdatacode_0_, E2____eic_hpi____rdatacode_15_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____ridcode_0_, E2____eic_hpi____ridcode_4_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____bidcode_0_, E2____eic_hpi____bidcode_4_, addr_sec_reg,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_hpi____bctlcode_0_, E2____eic_hpi____bctlcode_2_,
                            addr_sec_reg, val_wr, ONE_BIT_EIC_ERR);
#endif/*CONFIG_BOARD_X9SP*/
    /* Inject one bit error for E5*/
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____wvalidcode, E5____eic_safe____wvalidcode, addr_Iram1,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____awvalidcode, E5____eic_safe____wctlcode_4_, addr_Iram1,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____awidcode_0_, E5____eic_safe____awidcode_4_, addr_Iram1,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____awctl1code_0_, E5____eic_safe____awctl1code_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____arvalidcode, E5____eic_safe____arvalidcode,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____awaddrcode_0_, E5____eic_safe____awaddrcode_6_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____aridcode_0_, E5____eic_safe____aridcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____arctl1code_0_, E5____eic_safe____arctl1code_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____ridcode_0_, E5____eic_safe____ridcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____bidcode_0_, E5____eic_safe____bidcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____bvalidcode, E5____eic_safe____rctlcode_2_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
#ifndef CONFIG_BOARD_X9SP
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____rdatacode_0_, E5____eic_safe____rdatacode_15_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
#endif/*CONFIG_BOARD_X9SP*/
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____arreadycode, E5____eic_safe____bctlcode_2_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    /* Inject one bit error for E6*/
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____wvalidcode, E6____eic_safe____wvalidcode,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____awidcode_0_, E6____eic_safe____awidcode_4_, addr_Iram1,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____wdatacode_0_, E6____eic_safe____wdatacode_15_, addr_Iram1,
                            val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____aridcode_0_, E6____eic_safe____aridcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____arvalidcode, E6____eic_safe____arvalidcode,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____awctl1code_0_, E6____eic_safe____awctl1code_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____araddrcode_0_, E6____eic_safe____araddrcode_6_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____arctl1code_0_, E6____eic_safe____arctl1code_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____ridcode_0_, E6____eic_safe____ridcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____bidcode_0_, E6____eic_safe____bidcode_4_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
#ifndef CONFIG_BOARD_X9SP
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____rdatacode_0_, E6____eic_safe____rdatacode_15_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
#endif/*CONFIG_BOARD_X9SP*/
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____arreadycode, E6____eic_safe____bctlcode_2_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
#ifndef CONFIG_BOARD_X9SP
    /* Inject one bit error for E7*/
    Fabric_Inject_SigMulErr(E7, E7____eic_safe____rerrcode_0_, E7____eic_safe____rerrcode_7_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E7, E7____eic_safe____werrcode_0_, E7____eic_safe____werrcode_7_,
                            addr_Iram1, val_wr, ONE_BIT_EIC_ERR);
    /* Inject one bit error for E8*/
    Fabric_Inject_SigMulErr(E8, E8____eic_safe____arrpty, E8____eic_safe____awrpty,
                            addr_safe_reg,val_wr, ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E8, E8____eic_safe____bvpty, E8____eic_safe____bvpty, addr_safe_reg, val_wr,
                            ONE_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E8, E8____eic_safe____rvpty, E8____eic_safe____rvpty,
                            addr_safe_reg, val_wr, ONE_BIT_EIC_ERR);
#endif/*CONFIG_BOARD_X9SP*/

#endif

#if (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
    /*Inject 2bits error for E2*/
#ifndef CONFIG_BOARD_X9SP
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____awaddrcode_0_, E2____eic_safe____awaddrcode_5_,
                            addr_sec_reg, val_wr, TWO_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E2, E2____eic_safe____wdatacode_0_, E2____eic_safe____wdatacode_14_,
                            addr_sec_reg, val_wr, TWO_BIT_EIC_ERR);
#endif
#ifndef CONFIG_BOARD_X9SP
    /*Inject 2bits error for E5*/
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____awaddrcode_0_, E5____eic_safe____awaddrcode_5_,
                            addr_Iram1, val_wr, TWO_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____araddrcode_0_, E5____eic_safe____araddrcode_5_,
                            addr_Iram1, val_wr, TWO_BIT_EIC_ERR);
    Fabric_Inject_SigMulErr(E5, E5____eic_safe____wdatacode_0_, E5____eic_safe____wdatacode_14_,
                            addr_Iram1, val_wr, TWO_BIT_EIC_ERR);
    /*Inject 2bits error for E7*/
    Fabric_Inject_SigMulErr(E7, E7____eic_safe____werrcode_0_, E7____eic_safe____werrcode_6_,
                            addr_Iram1, val_wr, TWO_BIT_EIC_ERR);
#endif/*CONFIG_BOARD_X9SP*/
    /*Inject 2bits error for E6*/
    Fabric_Inject_SigMulErr(E6, E6____eic_safe____rdatacode_0_, E6____eic_safe____rdatacode_14_,
                            addr_Iram1, val_wr, TWO_BIT_EIC_ERR);
#endif

    Sem_Enable_Eic(SEMgr1, FALSE);
}
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_InjErr
(
    VAR(Iram_Error_BitType, AUTOMATIC) err_bit,
    VAR(IramInjErrType, AUTOMATIC)  iram_inj_Err,
    VAR(IramCtlType, AUTOMATIC)     iramctl,
    VAR(uint8, AUTOMATIC)           bank_index,
    VAR(uint8, AUTOMATIC)           bit_index,
    VAR(uint32, AUTOMATIC)          signal_bit
)
{
    volatile unsigned long addr_iram = InjErr_IramBaseAddr32[iramctl];
    uint32 val_wr = INJECT_ERR_VALUE;
    addr_iram = ROUNDUP(addr_iram, 0x40);

    if (err_bit == ONE_BIT_IRAM_ERR) {
        Iram_Error_Injection(ONE_BIT_IRAM_ERR, iram_inj_Err, iramctl, bank_index, bit_index, TRUE);
        dsb();
        isb();
        /*write value to iram */
        writel(val_wr, (addr_iram + bank_index * 16U));
        dsb();
        isb();
        /*read value from iram */
        (void)readl(addr_iram + bank_index * 16U);
        Iram_Error_Injection(ONE_BIT_IRAM_ERR, iram_inj_Err, iramctl, bank_index, bit_index, FALSE);
    } else {
        /*Inject 2 bits error to iram*/
        Iram_Error_Injection(TWO_BIT_IRAM_ERR, iram_inj_Err, iramctl, bank_index, bit_index, TRUE);
        dsb();
        isb();
        /*Write value to iram */
        writel(val_wr, (addr_iram + bank_index * 16U));
        dsb();
        isb();

        /*Do not read from Iram for injecting 2bit error in iram read path decoding0,But only for Iram read path decoding1*/
        if (signal_bit == BM_MEM_ERR_INT_STATUS_RDATA_FATAL) {
            (void)readl(addr_iram + bank_index * 16U);
        }

        /*cancel injecting 2 bits error to iram*/
        Iram_Error_Injection(TWO_BIT_IRAM_ERR, iram_inj_Err, iramctl, bank_index, bit_index, FALSE);

    }
}
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON))
/* The mode of inject 1bit error to iram memory, for two modes:INJECT_ALL_TRAVELL, INJECT_FIXED_POINT*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj1BitErr_Mode
(
    VAR(IramInjErrType, AUTOMATIC)  Iram_Inj_Err,
    VAR(IramStaType, AUTOMATIC)     Iram_Sta,
    VAR(IramCtlType, AUTOMATIC)     IramCtl,
    VAR(uint32, AUTOMATIC)          Signal_bit,
    VAR(uint8, AUTOMATIC)           Bank_Len,
#if (IRAM_1BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
    VAR(uint8, AUTOMATIC)           Bit_Len
#else
    VAR(uint8, AUTOMATIC)           BitIndex
#endif
)
{
    volatile unsigned long addr_iram = InjErr_IramBaseAddr32[IramCtl];

    addr_iram = ROUNDUP(addr_iram, 0x40);
    uint32 val_wr = INJECT_ERR_VALUE;
    uint32 val_rd = 0U;
    uint32 val_temp = 0U;
    uint32 IramErrCode = 0U;
    uint8 bank_index = 0U;
    uint8 bit_index = 0U;

    for (bank_index = 0U; bank_index < Bank_Len; bank_index++) {
#if (IRAM_1BIT_ERROR_MODE == INJECT_ALL_TRAVELL)

        for (bit_index = 0U; bit_index < Bit_Len; bit_index++) {
#else
        bit_index = BitIndex;
#endif
            /*Store the origin data which will be written for an address of iram*/
            val_temp = readl(addr_iram + bank_index * 16U);
            Iram_Mem_InjErr(ONE_BIT_IRAM_ERR, Iram_Inj_Err, IramCtl, bank_index, bit_index, Signal_bit);
            val_rd = readl((addr_iram + bank_index * 16U));

            if (val_wr != val_rd) {
                IramErrCode = (1 << 28U) + (IramCtl << 24U) + (Iram_Inj_Err << 16U) + (bank_index << 8U) +
                              (bit_index);

                if (g_IramZoneCurConfig->pIram1bitUnCorrNotification != NULL_PTR) {
                    g_IramZoneCurConfig->pIram1bitUnCorrNotification(IramErrCode);
                }

            } else {
                /*For misra*/
            }

            /*Check interrup status and clear*/
            if (Get_Iram_Inj_Err_Int_Status(Iram_Sta, IramCtl, Signal_bit) ==  TRUE) {
                (void)Clr_Iram_Inj_Err_Int_Status(Iram_Sta, IramCtl, Signal_bit);
            } else {
                IramErrCode = (1 << 28U) + (IramCtl << 24U) + (Iram_Inj_Err << 16U) + (bank_index << 8U) +
                              (bit_index);

                if (g_IramZoneCurConfig->pIram1bitUnDetectNotification != NULL_PTR) {
                    g_IramZoneCurConfig->pIram1bitUnDetectNotification(IramErrCode);
                }
            }

            for (uint8 index = 0U; index < 4U; index++) {
                if (Get_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index)) ==  TRUE) {
                    (void)Clr_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index));
                }
            }

            /*Restore the origin data which has been written for an address of iram*/
            writel(val_temp, (addr_iram + bank_index * 16U));
#if (IRAM_1BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
        }

#endif
    }
}
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject one bit error in iram memory wr/rd path for all traversal or one fixed point*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj1BitErr_Func
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
)
{
    /*SEM enable error injection*/
    Sem_Enable_Eic(SEMgr1, TRUE);
    Iram_Eic_point_En(IramCtl, TRUE);
    Enable_Iram_Mem_Sta_Int(IramCtl, TRUE);
    Iram_Mem_Sig_Int_En(IramCtl, FALSE);
#if (IRAM_1BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
    /*Inject 1 bit error in Iram for all travel all points*/
    /*Inject 1bit error into iram memory read path decoding0 data:bank0~bank3,bit0~bit127 ecc: bank0~bank3,bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(RDDATADE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_SIG_ERR,
                             BANK_LONG_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj1BitErr_Mode(RDECCDE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_SIG_ERR,
                             BANK_LONG_LEN, BIT_SHORT_LEN);

    /*Inject 1bit error into iram memory read path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(RDDATADE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj1BitErr_Mode(RDECCDE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, BIT_SHORT_LEN);

    /*Inject 1bit error into iram memory write path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(WRDATADE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj1BitErr_Mode(WRECCDE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, BIT_SHORT_LEN);
#else
    /*Inject 1 bit error in Iram for one point of one iram bank*/
    /*Inject 1bit error into iram memory read path decoding0 data:bank0~bank3,bit0~bit127 ecc: bank0~bank3,bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(RDDATADE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_SIG_ERR,
                             BANK_LONG_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj1BitErr_Mode(RDECCDE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_SIG_ERR,
                             BANK_LONG_LEN, IRAM_INJECT_ERR_POINT);

    /*Inject 1bit error into iram memory read path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(RDDATADE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj1BitErr_Mode(RDECCDE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);

    /*Inject 1bit error into iram memory write path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj1BitErr_Mode(WRDATADE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj1BitErr_Mode(WRECCDE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
#endif

    Sem_Enable_Eic(SEMgr1, FALSE);
    Iram_Eic_point_En(IramCtl, FALSE);
    Iram_Mem_Sig_Int_En(IramCtl, TRUE);
}
#endif

#if ((FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/* The mode of inject 2bit error to iram memory*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj2BitErr_Mode
(
    VAR(IramInjErrType, AUTOMATIC)  Iram_Inj_Err,
    VAR(IramStaType, AUTOMATIC)     Iram_Sta,
    VAR(IramCtlType, AUTOMATIC)     IramCtl,
    VAR(uint32, AUTOMATIC)          Signal_bit,
    VAR(uint8, AUTOMATIC)           Bank_Len,
#if (IRAM_2BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
    VAR(uint8, AUTOMATIC)           Bit_Len
#else
    VAR(uint8, AUTOMATIC)           BitIndex
#endif
)
{
    volatile unsigned long addr_iram = InjErr_IramBaseAddr32[IramCtl];

    addr_iram = ROUNDUP(addr_iram, 0x40);
    uint32 val_temp = 0U;
    uint32 IramErrCode = 0U;
    uint8 bank_index = 0U;
    uint8 bit_index = 0U;

    for (bank_index = 0U; bank_index < Bank_Len; bank_index++) {
#if (IRAM_2BIT_ERROR_MODE == INJECT_ALL_TRAVELL)

        for (bit_index = 0U; bit_index < Bit_Len; bit_index++) {
#else
        bit_index = BitIndex;
#endif
            /*Store the origin data which will be written for an address of iram*/
            val_temp = readl(addr_iram + bank_index * 16U);

            if ((bit_index != 7U) && (bit_index != 15U) && (bit_index != 31U) && (bit_index != 63U)
                && (bit_index != 95U) && (bit_index != 127U)) {
                Iram_Mem_InjErr(TWO_BIT_IRAM_ERR, Iram_Inj_Err, IramCtl, bank_index, bit_index, Signal_bit);

                /*Check interrup status and clear*/
                if (Get_Iram_Inj_Err_Int_Status(Iram_Sta, IramCtl, Signal_bit) ==  TRUE) {
                    (void)Clr_Iram_Inj_Err_Int_Status(Iram_Sta, IramCtl, Signal_bit);
                } else {
                    IramErrCode = (1 << 28U) + (IramCtl << 24U) + (Iram_Inj_Err << 16U) + (bank_index << 8U) +
                                  (bit_index);

                    if (g_IramZoneCurConfig->pIram2bitUnDetectNotification != NULL_PTR) {
                        g_IramZoneCurConfig->pIram2bitUnDetectNotification(IramErrCode);
                    }
                }

                for (uint8 index = 0U; index < 4U; index++) {
                    if (Get_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index)) ==  TRUE) {
                        (void)Clr_Iram_Inj_Err_Int_Status(IRAM_MEM_INT, IramCtl, (1U << index));
                    }
                }
            } else {
                continue;
            }

            /*Restore the origin data which has been written for an address of iram*/
            writel(val_temp, (addr_iram + bank_index * 16U));
#if (IRAM_2BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
        }

#endif
    }
}
#endif

#if ((FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject two bits error in iram memory wr/rd path for all traversal or one fixed point in one iram bank*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj2BitErr_Func
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
)
{
    /*SEM enable error injection*/
    Sem_Enable_Eic(SEMgr1, TRUE);
    Iram_Eic_point_En(IramCtl, TRUE);
    Enable_Iram_Mem_Sta_Int(IramCtl, TRUE);
    Iram_Mem_Sig_Int_En(IramCtl, FALSE);

#if (IRAM_2BIT_ERROR_MODE == INJECT_ALL_TRAVELL)
    /*Inject 2 bit error in Iram for all travel all points*/
    /*Inject 2bits error into iram memory read path decoding0 data:bank0~bank3,bit0~bit127 ecc: bank0~bank3,bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(RDDATADE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_MUL_ERR,
                             BANK_LONG_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj2BitErr_Mode(RDECCDE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_MUL_ERR,
                             BANK_LONG_LEN, BIT_SHORT_LEN);
    /*Inject 2bits error into iram memory read path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(RDDATADE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj2BitErr_Mode(RDECCDE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, BIT_SHORT_LEN);

    /*Inject 2bits error into iram memory write path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(WRDATADE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, BIT_LONG_LEN);
    Iram_Mem_Inj2BitErr_Mode(WRECCDE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, BIT_SHORT_LEN);
#else
    /*Inject 2 bit error in Iram for one point of one iram bank*/
    /*Inject 2bits error into iram memory read path decoding0 data:bank0~bank3,bit0~bit127 ecc: bank0~bank3,bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(RDDATADE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_MUL_ERR,
                             BANK_LONG_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj2BitErr_Mode(RDECCDE0, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_MUL_ERR,
                             BANK_LONG_LEN, IRAM_INJECT_ERR_POINT);
    /*Inject 2bits error into iram memory read path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(RDDATADE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj2BitErr_Mode(RDECCDE1, IRAM_MEM_INT, IramCtl, BM_MEM_ERR_INT_STATUS_RDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);

    /*Inject 2bits error into iram memory write path decoding1 data:bit0~bit127 ecc:bit0~bit15*/
    Iram_Mem_Inj2BitErr_Mode(WRDATADE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
    Iram_Mem_Inj2BitErr_Mode(WRECCDE1, IRAM_UNCORR_INT, IramCtl, BM_UNCERR_INT_STA_WDATA_FATAL,
                             BANK_SHORT_LEN, IRAM_INJECT_ERR_POINT);
#endif

    Sem_Enable_Eic(SEMgr1, FALSE);
    Iram_Eic_point_En(IramCtl, FALSE);
    Iram_Mem_Sig_Int_En(IramCtl, TRUE);
}
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2 bit error in Iram WR/RD path,WR path is only for Iram1,RD path can only for Iram1/2/3/4*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Inject_SigMulErr_Func(void)
{
    uint8 index = 0U;

    /*Inject 1/2 bits error in iram memory wr/rd path*/
    for (index = 0U; index < IRAM_ERR_NUM; index++)
    {
        g_IramZoneCurConfig = &Iram_ZoneConfig[index];
#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON))
        Iram_Mem_Inj1BitErr_Func((IramCtlType)(g_IramZoneCurConfig->eIram_Zone));
#endif
#if ((FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
        //Iram_Mem_Inj2BitErr_Func((IramCtlType)(g_IramZoneCurConfig->eIram_Zone));     //zhong.changjun
#endif
    }
}
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2bits error mode for fuse*/
FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Inject_Err_Mode
(
    VAR(FuseInjectErrType, AUTOMATIC)       Inject_Err,
    VAR(FuseErrStaType, AUTOMATIC)          Err_Sta,
    VAR(uint8, AUTOMATIC)                   Bit_Len,
    VAR(Fuse_Error_BitType, AUTOMATIC)      Err_Bit,
    VAR(uint32, AUTOMATIC)                  Sig_Bit
)
{
    uint32 ErrWordCode = 0U;
    uint32 ErrWordBit = 0U;

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)

    for (ErrWordBit = 0U; ErrWordBit < Bit_Len; ErrWordBit++) {
#else
    ErrWordBit = FUSE_INJECT_ERR_BIT;
    (void)Bit_Len;
#endif
        Fuse_Inject_Error_En(Err_Bit, Inject_Err, ErrWordBit, TRUE);
        /*Reload fuse word via sw load*/
        Trig_Reload_Fuse_Word_SwLoad();
        Fuse_Inject_Error_En(Err_Bit, Inject_Err, ErrWordBit, FALSE);

        if ((Err_Sta == SIGERR) || (Err_Sta == MULERR)) {
            Fuse_InjErr_SigMulErr_Chk(ErrWordBit, Err_Sta, Err_Bit);
        } else {
            if (Err_Sta == REDERR) {
                Fuse_InjErr_RedErr_Chk(ErrWordBit);
            } else if (Err_Sta == LOCKREDERR) {
                Fuse_InjErr_LockRedErr_Chk(ErrWordBit);
            }
        }

        if (Get_Fuse_Interrupt_Status(Sig_Bit) == TRUE) {
            Clr_Fuse_Interrupt_Status(Sig_Bit);
        } else {
            ErrWordCode = (Sig_Bit << 16U) + (Err_Bit << 8U) + ErrWordBit;

            if (Err_Bit == ONE_BIT_FUSE_ERR) {
                if (g_FuseErrCurConfig->pFuse1bitIntStaUnDetectNotification != NULL_PTR) {
                    g_FuseErrCurConfig->pFuse1bitIntStaUnDetectNotification(ErrWordCode);
                }
            } else {
                if (g_FuseErrCurConfig->pFuse2bitIntStaUnDetectNotification != NULL_PTR) {
                    g_FuseErrCurConfig->pFuse2bitIntStaUnDetectNotification(ErrWordCode);
                }
            }
        }

        if (Sig_Bit == BM_INT_STA_MUL_ERR) {
            if (Get_Fuse_Interrupt_Status(BM_INT_STA_FATAL_ERR) == TRUE) {
                Clr_Fuse_Interrupt_Status(BM_INT_STA_FATAL_ERR);
            }
        }

#if (FUSE_INJECT_ERROR_MODE == INJECT_ALL_TRAVEL)
    }

#endif
}
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2 bit error in fuse register and check errror*/
FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Inject_Err_Check(void)
{
    (void)Sem_Enable_Eic(SEMgr1, TRUE);
    /*Semaphore lock bit*/
    (void)Set_Fuse_Sema_Lock(TRUE);
    (void)Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, FUSE_EIC_POINT, TRUE);
    (void)Fuse_Interrupt_Sta_En(TRUE);
    (void)Fuse_Interrupt_Sig_En(FALSE);

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON))
    /*Inject one bit error*/
    Fuse_Inject_Err_Mode(RDDATAECC, SIGERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR, BM_INT_STA_SIG_ERR);
    Fuse_Inject_Err_Mode(ECCCODEECC, SIGERR, FUSE_BIT_SHORT_LEN, ONE_BIT_FUSE_ERR, BM_INT_STA_SIG_ERR);
    Fuse_Inject_Err_Mode(RDDATARED, REDERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR, BM_INT_STA_RED_ERR);
    Fuse_Inject_Err_Mode(REDDATARED, REDERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR, BM_INT_STA_RED_ERR);
    Fuse_Inject_Err_Mode(RDDATARED, LOCKREDERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR,
                         BM_INT_STA_RED_ERR);
    Fuse_Inject_Err_Mode(REDDATARED, LOCKREDERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR,
                         BM_INT_STA_RED_ERR);
    Fuse_Inject_Err_Mode(RDCORRDATAECC, NONERR, FUSE_BIT_LONG_LEN, ONE_BIT_FUSE_ERR,
                         BM_INT_STA_FATAL_ERR);
    Fuse_Inject_Err_Mode(ECCCORRCODEECC, NONERR, FUSE_BIT_SHORT_LEN, ONE_BIT_FUSE_ERR,
                         BM_INT_STA_FATAL_ERR);
#endif

#if ((FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
    /*Inject two bits error*/
    Fuse_Inject_Err_Mode(RDDATAECC, MULERR, (FUSE_BIT_LONG_LEN - 1U), TWO_BIT_FUSE_ERR,
                         BM_INT_STA_MUL_ERR);
    Fuse_Inject_Err_Mode(ECCCODEECC, MULERR, (FUSE_BIT_SHORT_LEN - 1U), TWO_BIT_FUSE_ERR,
                         BM_INT_STA_MUL_ERR);
#endif

    (void)Sem_Enable_Eic(SEMgr1, FALSE);
    (void)Set_Fuse_Sema_Lock(FALSE);
    (void)Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, FUSE_EIC_POINT, FALSE);
    (void)Fuse_Interrupt_Sig_En(TRUE);
}
#endif

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
/*SEM signal Monitor and inject error*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_SigMonitor_Func(void)
{
#if (FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON)
    Sem_Signal_Monitor(SEMSIGMON_INJERR);
#endif
#if (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON)
    Sem_Signal_Monitor(SEM_SIG_MON);
#endif
}
#endif

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
/*Set SEM signal wrong expected target value to trigger checking or an expected target value to monitor sem signal*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_Signal_Monitor
(
    SemSigMonTstType semsig_tstmode
)
{
    uint8 signal_value = 0U;
    uint8 signal_wrong_value = 0U;
    uint8 set_sig_value = 0U;

    /*Enable SEM output for CPU interrupt*/
    Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_CPU, TRUE);

    for (enum sem_monitor_sig signal_index = EFUSE_MISC_CFG_0_;
         signal_index <= APBMUX2_SEC_STORAGE2_SEL; signal_index++) {
        if ((signal_index == EFUSE_MISC_CFG_74_) || ((signal_index >= EFUSE_MISC_CFG_85_)
                && (signal_index <= EFUSE_MISC_CFG_88_)) || (signal_index == APBMUX2_RPC_SAF_SEL)) {
            continue;
        }

        Sem_Monitor_Enable_Intr(SEMgr1, SEM_OUTPUT_CPU, TRUE);

        if (signal_index < APBMUX2_SEM1_SEL) {
            signal_value = ((REG_READ32(APB_EFUSEC_BASE + FUSE_MISC_CFG_ADDR_OFF(signal_index)) >>
                             (signal_index % 32)) & 0x01);
        } else if (signal_index >= APBMUX2_SEM1_SEL) {
            signal_value = ((Mcu_ScrGetBit(APB_SCR_SAF_BASE, L16,
                                           g_SemMonSigScrinfo[signal_index - APBMUX2_SEM1_SEL].u32StartPos)) & 0x01);
        } else {
            /*do not anything*/
        }

        if (semsig_tstmode == SEMSIGMON_INJERR) {
            /*Set a wrong expected target value to trigger checking*/
            signal_wrong_value = !signal_value;
            set_sig_value = signal_wrong_value;
            gSemSigMonInjectErrEnable = TRUE;
        } else {
            /*Set an expected target value to monitor sem signal*/
            set_sig_value = signal_value;
            gSemSigMonEnable = TRUE;
        }

        Sem_Monitor_Set_Sig(SEMgr1, signal_index, set_sig_value);
        #ifndef CONFIG_BOARD_X9SP
        Sem_Monitor_Enable_Sig(SEMgr1, signal_index, TRUE);
        #endif /* CONFIG_BOARD_X9SP*/
        //Mcu_udelay(5);
        udelay(5U);

        if (semsig_tstmode == SEMSIGMON_INJERR) {
            /*If not entering SEM interrupt when setting a wrong expected target value to send a notification to the upper*/
            if (gSemSigMonInjectErrIsrFlg != TRUE) {
                if (g_SemSignalMonCurConfig->pSemSignalInjErrUnDetectNotification != NULL_PTR) {
                    g_SemSignalMonCurConfig->pSemSignalInjErrUnDetectNotification();
                }
            } else {
                gSemSigMonInjectErrIsrFlg = FALSE;
            }

            /*Restore sem signal target value*/
            Sem_Monitor_Set_Sig(SEMgr1, signal_index, signal_value);
            Sem_Monitor_Enable_Sig(SEMgr1, signal_index, FALSE);
        }
    }

    gSemSigMonInjectErrEnable = FALSE;
    gSemSigMonEnable = FALSE;
    /*Disable SEM monitor interrupt*/
    Sem_Monitor_Enable_Intr(SEMgr1, SEM_OUTPUT_CPU, FALSE);
    /*Enable SEM output for CPU interrupt*/
    Sem_Interrupt_Ovrd_Enable(SEMgr1, SEM_OUTPUT_CPU, FALSE);
}
#endif

#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
/*run BIST sequence*/
FUNC(void, FUNCSAFETYPKG_CODE) Run_Bist_Sequence_Func
(
    VAR(uint32, AUTOMATIC)         _base_addr,
    VAR(uint32, AUTOMATIC)         bist_timeout1,
    VAR(uint32, AUTOMATIC)         bist_timeout2,
    VAR(Bist_NocType, AUTOMATIC)   Noc_Type
)
{
    uint32 Latent_Fault0 = 0U;
    uint32 Latent_Fault1 = 0U;
    uint32 Latent_Fault2 = 0U;
    uint32 Latent_Fault3 = 0U;
    uint32 wait_cnt = 0U;

    /*enable the reporting of an interrupt on LatentInt*/
    REG_RMW32((_base_addr + INTEN), 0U, 1U, 0x00U);
    /*set the time-out value of timeout1 mechanism of the BIST sequence*/
    REG_RMW32((_base_addr + BISTTO1), 0U, 16U, bist_timeout1);
    /*set the time-out value of timeout2 mechanism of the BIST sequence*/
    REG_RMW32((_base_addr + BISTTO2), 0U, 8U, bist_timeout2);
    /*start a bist sequence*/
    REG_RMW32((_base_addr + BISTCTRL), 0U, 1U, 0x01U);

    while (((REG_READ32(_base_addr + BISTDONE) & 0x01U) == 0U) && (wait_cnt < 5000U)) {
        //Mcu_udelay(1U);
        udelay(1U);
        wait_cnt++;
    }

    /*get latent fault 0 status*/
    Latent_Fault0 = REG_READ32(_base_addr + LATENTFAULT0);
    /*get latent fault 1 status*/
    Latent_Fault1 = REG_READ32(_base_addr + LATENTFAULT1);
    /*get latent fault 2 status*/
    Latent_Fault2 = REG_READ32(_base_addr + LATENTFAULT2);
    /*get latent fault 3 status*/
    Latent_Fault3 = REG_READ32(_base_addr + LATENTFAULT3);

    if ((Latent_Fault0 != LATENTFAULT0_CHK) && (Latent_Fault0 != 0U)) {
        if (g_BistCurConfig->pLatentFault0Notification != NULL_PTR) {
            g_BistCurConfig->pLatentFault0Notification(Noc_Type, Latent_Fault0);
        }
    }

    if ((Latent_Fault1 != LATENTFAULT1_CHK) && (Latent_Fault1 != 0U)) {
        if (g_BistCurConfig->pLatentFault1Notification != NULL_PTR) {
            g_BistCurConfig->pLatentFault1Notification(Noc_Type, Latent_Fault1);
        }
    }

    if (Latent_Fault2 != 0U) {
        if (g_BistCurConfig->pLatentFault2Notification != NULL_PTR) {
            g_BistCurConfig->pLatentFault2Notification(Noc_Type, Latent_Fault2);
        }
    }

    if (Latent_Fault3 != 0U) {
        if (g_BistCurConfig->pLatentFault3Notification != NULL_PTR) {
            g_BistCurConfig->pLatentFault3Notification(Noc_Type, Latent_Fault3);
        }
    }

    /*clear bistdone*/
    REG_RMW32((_base_addr + BISTCTRL), 1U, 1U, 0x01U);
    /*clear the currently reported latent fault state*/
    REG_RMW32((_base_addr + INTCLR), 0U, 2U, 0x03U);
}
#endif

#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
/*Run bist in noc of safe core : noc_saf noc_hpia noc_hpib*/
FUNC(void, FUNCSAFETYPKG_CODE) Run_Bist_Noc_SafeCore(void)
{
    /*noc_saf*/
    Run_Bist_Sequence_Func(GPV_BASE, 0x2000U, 0xffU, NOC_SAFE);
#ifdef CONFIG_BOARD_X9SP
#else
    /*noc_hpia*/
    Run_Bist_Sequence_Func(FAB_GPV_HPIA, 0x2000U, 0xffU, NOC_HPIA);
#endif
    /*noc_hpib*/
    Run_Bist_Sequence_Func(APB_GPV_HPIB_BASE, 0x3000U, 0xffU, NOC_HPIB);

}
#endif

#if (FUSAPKG_WDT_SELFTEST_POWER_ON == STD_ON)
/*self-test mode for wdt*/
FUNC(void, FUNCSAFETYPKG_CODE) Wdt_SelfTst_Func()
{
    uint32 val_temp = 0U;
    // uint32 val_temp_rst = 0U;
    uint32 wait_cnt = 0U;
    uint32 overflow_value = 0U;

    Sem_Enable_Eic(SEMgr1, TRUE);
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_WDT_EN, 1U, 0U);
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_WDT_EN_SRC, 1U, 1U);

    while (((((uint32)0x1U << BM_WDT_CTRL_WDT_EN_STA) & REG_READ32(APB_WDT1_BASE +
             WDT_CTRL_OFF)) > 0u) && (wait_cnt < 500000U)) {
        /*wait until turn off wdg*/
        //Mcu_udelay(1U);
        udelay(1U);
        wait_cnt++;
    }

    wait_cnt = 0U;
    REG_RMW32(APB_WDT1_BASE + WDT_RST_CTL_OFF, BM_WDT_RST_CTL_WDT_RST_EN, 1, 1U);
    REG_RMW32(APB_WDT1_BASE + WDT_RST_CTL_OFF, BM_WDT_RST_CTL_INT_RST_EN, 1, 1U);
    /*Set wdt terminal count value source from register WDT_WTC*/
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_WTC_SRC, 1U, 1U);
    /* Using main_clk: source clock is ss 24M*/
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), 2, 3, 0U);
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), 16U, 16U, 23U);
    /*Enable wdt overflow auto restart*/
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_AUTO_RESTART, 1U, 1U);
    /*trigger self-test*/
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_SELFTEST_TRIG, 1U, 1U);
    /*Enable overflow*/
    REG_RMW32((APB_WDT1_BASE + WDT_INT_OFF), BM_WDT_INT_OVFLOW_INT_EN, 1U, 1U);
    /*Convert to a 32bit overflow value from one byte in wdt test mode*/
    overflow_value = (uint32)BITCONVTOUNIT32((uint8)WdtSelfTestConfig.OverFlowValue);
    REG_WRITE32(overflow_value, (APB_WDT1_BASE + WDT_WTC_OFF));
    /*disable global reset for watchdog*/
    REG_RMW32(APB_RSTGEN_SAF_BASE, 3U, 1U, 0U);
    REG_RMW32(APB_RSTGEN_RTC_BASE, 3U, 1U, 0U);
    /*disable watchdog reset*/
    REG_RMW32((APB_RSTGEN_SAF_BASE + (0x04 << 10U)), 2U, 1U, 0U);
    /*enable wdt*/
    REG_RMW32(APB_WDT1_BASE + WDT_CTRL_OFF, BM_WDT_CTRL_WDT_EN, 1U, 1U);

    while ((!((((uint32)0x1U << BM_WDT_CTRL_WDT_EN_STA) & REG_READ32(APB_WDT1_BASE +
               WDT_CTRL_OFF)) > 0u)) && (wait_cnt < 500000U)) {
        /*wait until enable success*/
        Mcu_udelay(1U);
        wait_cnt++;
    }

    wait_cnt = 0U;
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_SOFT_RST, 1U, 1U);

    while (((((0x1U << BM_WDT_CTRL_SOFT_RST) & REG_READ32(APB_WDT1_BASE +
              WDT_CTRL_OFF))) > 0u) && (wait_cnt < 500000U)) {
        /*wait until soft reset success*/
        Mcu_udelay(1U);
        wait_cnt++;
    }

    wait_cnt = 0U;

    while ((val_temp == 0U) && (wait_cnt < 500000U)) {
        Mcu_udelay(1U);
        wait_cnt++;
        val_temp = (REG_READ32(APB_WDT1_BASE + WDT_INT_OFF) & (1 << BM_WDT_INT_OVFLOW_INT_STA));
    }

    wait_cnt = 0U;

    if (val_temp != 0U) {
        #if 0
        /*clear overflow interrupt status*/
        REG_WRITE32((1 << BM_WDT_INT_OVFLOW_INT_CLR), (APB_WDT1_BASE + WDT_INT_OFF));
        /*wait for rstgen wdt reset status*/
        while ((val_temp_rst == 0U) && (wait_cnt < 500000U)) {
            Mcu_udelay(1U);
            wait_cnt++;
            val_temp_rst = (REG_READ32(APB_RSTGEN_SAF_BASE + (0x4c << 10)) & ((uint32)0x01U << 4U));
        }
        if (val_temp_rst == 0U) {
        }
        #endif
    } else {
          if (g_WdtSelfTestCurConfig->pWdtSelfTestOverFlowResetUndetectNotification != NULL_PTR) {
                g_WdtSelfTestCurConfig->pWdtSelfTestOverFlowResetUndetectNotification();
            }
    }

    /*trigger self-test*/
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_SELFTEST_TRIG, 1U, 0U);
    REG_RMW32((APB_WDT1_BASE + WDT_CTRL_OFF), BM_WDT_CTRL_SOFT_RST, 1U, 1U);
    Sem_Enable_Eic(SEMgr1, FALSE);
    /*enable global reset for watchdog*/
    REG_RMW32(APB_RSTGEN_SAF_BASE, 3U, 1U, 1U);
    REG_RMW32(APB_RSTGEN_RTC_BASE, 3U, 1U, 1U);
    /*enable watchdog reset*/
    REG_RMW32((APB_RSTGEN_SAF_BASE + (0x04 << 10U)), 2U, 1U, 1U);
}
#endif

#if (FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON)
/*Rc clock can check when AP clock lost*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_RcChkAp(void)
{
    uint32 to_lost_int_status_soc = 0U;
    uint32 waitcnt = 0U;
    uint32 u32osc_chkcfg = 0U;
    uint32 u32xtal_chkcfg = 0U;
    osc_out_sel_e cur_sel = ana_osc24_get_sel(0U);

    /*Enable soc lost interrupt*/
    REG_RMW32((APB_RC24M_BASE + XTAL_SOC_INT_OFF), 1U, 1U, 1U);
    /*Set XTAL_AP counter trigger enable*/
    u32xtal_chkcfg = (1 << 0U) + (32 << 2U) + (7 << 12U) + (4 << 17U) + (3 << 27U);
    REG_WRITE32(u32xtal_chkcfg, (APB_XTAL_AP_BASE + XTAL_CHK_OFF));
    /*Set OSC counter trigger enable*/
    u32osc_chkcfg = (1 << 0U) + (32 << 1U) + (7 << 11U) + (4 << 16U) + (3 << 26U);
    REG_RMW32((APB_RC24M_BASE + OSC_CHK_OFF), 1U, 30U, u32osc_chkcfg);

    while (waitcnt < 5U) {
        //Mcu_udelay(500U);
        udelay(500U);
        waitcnt++;
    }

    waitcnt = 0U;

    if (cur_sel == SEL_XTAL1) {
        /*Set XTAL_AP output disable*/
        REG_RMW32(APB_XTAL_AP_BASE, 0U, 1U, 1U);

        do {
            //Mcu_udelay(1U);
            udelay(1U);
            waitcnt++;
            to_lost_int_status_soc = (REG_READ32(APB_RC24M_BASE + XTAL_SOC_INT_OFF) &
                                      BM_XTAL_SOC_INT_TO_LOST_INT_STATUS);
        } while ((to_lost_int_status_soc == 0U) && (waitcnt < 500000U));

        /*Set XTAL_AP output enable*/
        REG_RMW32(APB_XTAL_AP_BASE, 0U, 1U, 0U);

        if (Sem_Interrupt_Status(SEMgr1, RC_24M____SOC_XTAL_LOST_INT) == TRUE) {
            if (to_lost_int_status_soc != 0U) {
                /*Clr soc lost interrupt status*/
                REG_RMW32((APB_RC24M_BASE + XTAL_SOC_INT_OFF), 7U, 1U, 1U);
            }
        } else {
            if (g_FsrefClkChkCurConfig->pRcChkApNotification != NULL_PTR) {
                g_FsrefClkChkCurConfig->pRcChkApNotification();
            }
        }
    }

    /*Set XTAL_AP counter trigger enable*/
    u32xtal_chkcfg = (32 << 2U) + (10 << 12U) + (32 << 17U) + (5 << 27U);
    REG_WRITE32(u32xtal_chkcfg, (APB_XTAL_AP_BASE + XTAL_CHK_OFF));
    /*Set OSC counter trigger enable*/
    u32osc_chkcfg = (32 << 1U) + (10 << 11U) + (32 << 16U) + (5 << 26U);
    REG_RMW32((APB_RC24M_BASE + OSC_CHK_OFF), 1U, 30U, u32osc_chkcfg);
}
#endif

#if (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON)
/*Safe clock can check when AP clock lost*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_SafChkAp(void)
{
    uint32  to_lost_int_status_saf = 0U;
    uint32 waitcnt = 0U;
    uint32 u32xtal_chkcfg = 0U;
    osc_out_sel_e cur_sel = ana_osc24_get_sel(0U);

    /*Enable XTAL Safety lost interrupt*/
    REG_RMW32((APB_XTAL_SAFETY_BASE + XTAL_INT_OFF), 1U, 1U, 1U);
    /*Set XTAL_AP counter trigger enable*/
    u32xtal_chkcfg = (1 << 0U) + (32 << 2U) + (7 << 12U) + (4 << 17U) + (3 << 27U);
    REG_RMW32((APB_XTAL_SAFETY_BASE + XTAL_CHK_OFF), 0U, 31U, u32xtal_chkcfg);
    /*Set osc counter trigger enable*/
    REG_RMW32((APB_XTAL_AP_BASE + XTAL_CHK_OFF), 0U, 31U, u32xtal_chkcfg);

    while (waitcnt < 5U) {
        //Mcu_udelay(200U);
        udelay(200U);
        waitcnt++;
    }

    waitcnt = 0U;

    if (cur_sel == SEL_XTAL1) {
        /*Set XTAL_AP output disable*/
        REG_RMW32(APB_XTAL_AP_BASE, 0U, 1U, 1U);

        do {
            //Mcu_udelay(1U);
            udelay(1U);
            waitcnt++;
            to_lost_int_status_saf = (REG_READ32(APB_XTAL_SAFETY_BASE + XTAL_INT_OFF) &
                                      BM_XTAL_INT_TO_LOST_INT_STATUS);
        } while ((to_lost_int_status_saf == 0U) && (waitcnt < 500000U));

        /*Set XTAL_AP output enable*/
        REG_RMW32(APB_XTAL_AP_BASE, 0U, 1U, 0U);

        if (Sem_Interrupt_Status(SEMgr1, XTAL_SAF____TO_LOST_INT) == TRUE) {
            if (to_lost_int_status_saf != 0U) {
                /*Clr xtal lost interrupt status*/
                REG_RMW32((APB_XTAL_SAFETY_BASE + XTAL_INT_OFF), 7U, 1U, 1U);
            }
        } else {
            if (g_FsrefClkChkCurConfig->pSafeChkApNotification != NULL_PTR) {
                g_FsrefClkChkCurConfig->pSafeChkApNotification();
            }
        }
    }

    /*Set XTAL_AP counter trigger enable*/
    u32xtal_chkcfg = (32 << 2U) + (10 << 12U) + (32 << 17U) + (5 << 27U);
    REG_RMW32((APB_XTAL_SAFETY_BASE + XTAL_CHK_OFF), 0U, 31U, u32xtal_chkcfg);
    /*Set osc counter trigger enable*/
    REG_RMW32((APB_XTAL_AP_BASE + XTAL_CHK_OFF), 0U, 31U, u32xtal_chkcfg);
}
#endif

#if ((FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON) || (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON))
/*Fail safe reference clock check each other among safe,ap,rc clock*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_Check_Func(void)
{
#if (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON)
    Fsrefclk_SafChkAp();
#endif
    /*Wait until AP goes from power down to power on*/
#if ((FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON) && (FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON))
    //Mcu_udelay(5000U);
    udelay(5000U);
#endif
#if (FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON)
    Fsrefclk_RcChkAp();
#endif
}
#endif

#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Injecting 1bit error in data cache for data ram and tag ram*/
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_1bitErr_Injection(void)
{
    Dcache_DataRam_1bitErr_Injection();
    Dcache_TagRam_1bitErr_Injection();
}
#endif


void dump_reg(uint32 addr, uint32 Len)
{
    uint32 i=0u;

    for(i=0; i<Len; i++)
    {
        printf("0x%lx=0x%x\n",    (uint32)(addr+4*i), (uint32_t)readl(addr+4*i));
    }
}

/*Injecting 1bit error in data cache for data ram*/
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
#ifdef CONFIG_BOARD_X9SP
uint32 CacheTest[4096*8];
#endif
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_DataRam_1bitErr_Injection(void)
{
    uint8 bank_sel = 0U;
    uint32 rdata = 0U;
    uint32 rdata_cache = 0U;
    uint8 bank_num = 0U;
    uint8 bank_index = 0U;
    uint16 ram_index = 0U;
    uint32 u32dcacheerrcode = 0U;
    uint32 pmu_cnt = 0U;

    PMU_Set_Event(0x4d);
    PMU_Enable_Event_Cnt();
    PMU_Enable_All_Cnt();

    arch_enable_dcache();
    for (uint16 index = 0U; index < 4096U; index++) {
        #ifdef CONFIG_BOARD_X9SP
        writell((index + 0x1000), (CacheTest + index * 8));
        #else
        writell((index + 0x1000), (IRAM3_ADDR + index * 8));
        #endif
    }
    arch_disable_dcache();
    arch_set_cache_error_check();
    bank_index = g_DCACHEErrCurConfig->pDcacheDataRamErrConfig->u32BankNum;
    ram_index = g_DCACHEErrCurConfig->pDcacheDataRamErrConfig->u32RamIndex;

    if ((bank_index == 0U) || (bank_index == 1U)) {
        bank_sel = 0x01U;
        bank_num = bank_index;
    } else if ((bank_index == 2U) || (bank_index == 3U)) {
        bank_sel = 0x02U;
        bank_num = (bank_index - 2U);
    } else if ((bank_index == 4U) || (bank_index == 5U)) {
        bank_sel = 0x04U;
        bank_num = (bank_index - 4U);
    } else if ((bank_index == 6U) || (bank_index == 7U)) {
        bank_sel = 0x08;
        bank_num = (bank_index - 6U);
    }
    /*Enable AXI slave cache RAM access and non-privileged access*/
    rdata = arch_rd_actlr();
    rdata |= (0x01U << 24U) | (0x01U << 23U);
    arch_wr_actlr(rdata);
    dsb();
    isb();
    /*Error injection global enable*/
    //writel(0x10000, APB_SEM1_BASE);
    writel((0x10000 | readl(APB_SEM1_BASE)), APB_SEM1_BASE);

    /*0x108:eic_r5_addr_ctrl(bit22:19 bit18:15) 0x108_0_7bit:eic_r5addr_ctrl 0x109_0bit:eic_r5_addr_ctrl_en 0x10a_0bit:cache_access_disable(~)*/
    /*[22:19]:0000 single bank data RAM; [18:15]:single bank mode*/
    writel((0x10100 + bank_sel), (APB_EIC_SAF_BASE + 0x108));
    dsb();
    isb();
    // /*bit13:lower/higher numbered bank [12:2]:RAM index*/
    rdata_cache = readl(DCACHE_ADDR + 0x2000 * bank_num + 4 * ram_index);
    dsb();
    isb();
    rdata_cache ^= 0x01;
    writel(rdata_cache, (DCACHE_ADDR + 0x2000 * bank_num + 4 * ram_index));
    /*Disable AXI slave cache RAM access and non-privileged access*/
    dsb();
    isb();
    rdata = arch_rd_actlr();
    rdata &= (0xfe7fffff);
    arch_wr_actlr(rdata);
    dsb();
    isb();
    arch_enable_dcache();
    dsb();
    isb();
    for (uint16 index = 0U; index < 4096U; index++) {
        dsb();
        isb();
        #ifdef CONFIG_BOARD_X9SP
        (void)readll(CacheTest + index * 8);
        #else
        (void)readll(IRAM3_ADDR + index * 8);
        #endif
    }

    pmu_cnt = PMU_Read_Event_Cnt();

    if (pmu_cnt == 0U) {
        u32dcacheerrcode = (bank_index << 16U) + ram_index;

        if (g_DCACHEErrCurConfig->pDcacheDataRamErrConfig->pDcacheDataRam1bitUnDetectNotification !=
            NULL_PTR) {
            g_DCACHEErrCurConfig->pDcacheDataRamErrConfig->pDcacheDataRam1bitUnDetectNotification(
                u32dcacheerrcode);
        }
    }
    PMU_Write_Event_Cnt(0);
    /*Invalidate cache*/
    arch_invalidate_cache_range((void *)IRAM3_ADDR, 0x8000U);
}
#endif

/*Injecting 1bit error in data cache for tag ram*/
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_TagRam_1bitErr_Injection(void)
{
    uint8 bank_sel = 0U;
    uint32 rdata = 0U;
    uint32 rdata_cache = 0U;
    uint8 bank_index = 0U;
    uint16 ram_index = 0U;
    uint32 u32dcacheerrcode = 0U;
    uint32 pmu_cnt = 0U;

    PMU_Set_Event(0x4c);
    PMU_Enable_Event_Cnt();
    PMU_Enable_All_Cnt();
    /* Enable dcache */
    arch_enable_dcache();
    for (uint16 index = 0U; index < 4096U; index++) {
        writell((index + 0x1000), (IRAM3_ADDR + index * 8));
    }
    /* Disable dcache */
    arch_disable_dcache();
    arch_set_cache_error_check();
    bank_index = g_DCACHEErrCurConfig->pDcacheTagRamErrConfig->u32BankNum;
    ram_index = g_DCACHEErrCurConfig->pDcacheTagRamErrConfig->u32RamIndex;

    if (bank_index == 0U) {
        bank_sel = 0x01U;
    } else if (bank_index == 1U) {
        bank_sel = 0x02U;
    } else if (bank_index == 2U) {
        bank_sel = 0x04U;
    } else if (bank_index == 3U) {
        bank_sel = 0x08U;
    }
    /*Enable AXI slave cache RAM access and non-privileged access*/
    rdata = arch_rd_actlr();
    rdata |= (0x01U << 24U) | (0x01U << 23U);
    arch_wr_actlr(rdata);
    dsb();
    isb();
    /*Error injection global enable*/
    //writel(0x10000, APB_SEM1_BASE);
    writel((0x10000 | readl(APB_SEM1_BASE)), APB_SEM1_BASE);
    /*[22:19]:0001 tag RAM [18:15]:[15]bank0 [16]bank1 [17]bank2 [18]bank3*/
    writel((0x10110 + bank_sel), (APB_EIC_SAF_BASE + 0x108));
    dsb();
    isb();
    /*[11:3]: RAM index*/
    rdata_cache = readl(DCACHE_ADDR + 8 * ram_index);
    dsb();
    isb();
    rdata_cache ^= 0x20;
    writel(rdata_cache, (DCACHE_ADDR + 8 * ram_index));
    dsb();
    isb();
    rdata = arch_rd_actlr();
    rdata &= (0xfe7fffff);
    arch_wr_actlr(rdata);
    dsb();
    isb();
    arch_enable_dcache();
    dsb();
    isb();
    for (uint16 index = 0U; index < 4096U; index++) {
        dsb();
        isb();
        (void)readll(IRAM3_ADDR + index * 8);
    }

    pmu_cnt = PMU_Read_Event_Cnt();
    if (pmu_cnt == 0U) {
        u32dcacheerrcode = (bank_index << 16U) + ram_index;

        if (g_DCACHEErrCurConfig->pDcacheTagRamErrConfig->pDcacheTagRam1bitUnDetectNotification !=
            NULL_PTR) {
            g_DCACHEErrCurConfig->pDcacheTagRamErrConfig->pDcacheTagRam1bitUnDetectNotification(
                u32dcacheerrcode);
        }
    }

    PMU_Write_Event_Cnt(0);
    /*Invalidate cache*/
    arch_invalidate_cache_range((void *)IRAM3_ADDR, 0x8000U);
}
#endif


/*Notification for ATCM 1 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) ATCM_Inject_1BitErr_UnDetect (void)
{
#if (FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pATCM1bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pATCM1bitUnDetectNotification();
    }

#endif
}

/*Notification for B0TCM 1 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) B0TCM_Inject_1BitErr_UnDetect (void)
{
#if (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pB0TCM1bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pB0TCM1bitUnDetectNotification();
    }

#endif
}

/*Notification for B1TCM 1 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) B1TCM_Inject_1BitErr_UnDetect (void)
{
#if (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pB1TCM1bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pB1TCM1bitUnDetectNotification();
    }

#endif
}

/*Notification for ATCM 2 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) ATCM_Inject_2BitErr_UnDetect (void)
{
#if (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pATCM2bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pATCM2bitUnDetectNotification();
    }

#endif
}

/*Notification for B0TCM 2 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) B0TCM_Inject_2BitErr_UnDetect (void)
{
#if (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pB0TCM2bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pB0TCM2bitUnDetectNotification();
    }

#endif
}

/*Notification for B1TCM 2 bit error*/
FUNC(void, FUNCSAFETYPKG_CODE) B1TCM_Inject_2BitErr_UnDetect (void)
{
#if (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)

    if (g_TCMErrCurConfig->pB1TCM2bitUnDetectNotification != NULL_PTR) {
        g_TCMErrCurConfig->pB1TCM2bitUnDetectNotification();
    }

#endif
}

#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1/2bits error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_InjSigMulErr_Func (void)
{
#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON))
    TCM_Inject_1bitErr();
#endif
#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
    TCM_Inject_2bitErr();
#endif
}
#endif

FUNC(void, FUNCSAFETYPKG_CODE) TCM_Inj_Err_Addr_Convert(uint32* addr, uint8* tcm)
{
    unsigned long tcm_addr = (unsigned long)&Tcm_Inj_Err[0];

    /* TCM address is mapped to 0 */
    if(tcm_addr < 0x20000) {
        /* The address of injectiong error in TCM is in BTCM */
        if(tcm_addr < 0xffb0) {
            *tcm = B0TCMTST;
            *addr = ROUNDUP(tcm_addr, 0x10);
        } else if(tcm_addr < ATCM_BASE) {
            *tcm = ATCMTST;
            *addr = ATCM_BASE;
        } else {
            *tcm = ATCMTST;
            *addr = ROUNDUP(tcm_addr, 0x08);

        }
    } else {
        /* TCM address is not mapped to 0  */
        #ifndef CONFIG_BOARD_X9SP
        if(tcm_addr < 0x4bffb0) {
        #else
        if(tcm_addr < 0x21Bffb0) {
        #endif
            *tcm = B0TCMTST;
            *addr = ROUNDUP(tcm_addr, 0x10);
        } else if(tcm_addr < ATCM_BASE) {
            *tcm = ATCMTST;
            *addr = ATCM_BASE;
        } else {
            *tcm = ATCMTST;
            *addr = ROUNDUP(tcm_addr, 0x08);
        }
    }
}

#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1bit error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_Inject_1bitErr (void)
{
    unsigned long tcm_addr = (unsigned long)0;
    uint8 tcm_tst = ATCMTST;

    TCM_Inj_Err_Addr_Convert(&tcm_addr, &tcm_tst);
    //printf("tcm_addr=%ld,tcm_tst=%d\n",tcm_addr,tcm_tst);

    if(tcm_tst == B0TCMTST) {
#if (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_1biterr(B0TCMTST, tcm_addr);
#endif
#if (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_1biterr(B1TCMTST, (unsigned long)(tcm_addr + BTCMGAP));
#endif
    } else {
#if (FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_1biterr(ATCMTST, tcm_addr);
#endif
	}
}
#endif

#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 2bits error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_Inject_2bitErr (void)
{
    unsigned long tcm_addr = (unsigned long)0;
    uint8 tcm_tst = ATCMTST;

    TCM_Inj_Err_Addr_Convert(&tcm_addr, &tcm_tst);
    //printf("tcm_addr=%ld,tcm_tst=%d\n",tcm_addr,tcm_tst);
    Inject2bitTcmErrFlg = TRUE;
    if(tcm_tst == B0TCMTST) {
#if (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_2biterr(B0TCMTST, tcm_addr);
#endif
#if (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_2biterr(B1TCMTST, (unsigned long)(tcm_addr + BTCMGAP));
#endif
    } else {
#if (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        cr5_tcm_ecc_tst_s_2biterr(ATCMTST, tcm_addr);
#endif
    }
    Inject2bitTcmErrFlg = FALSE;
}
#endif

/*Cancel injecting error in tcm through writing data to TCM when enters exception*/
#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
FUNC(void, FUNCSAFETYPKG_CODE) Tcm_Inject_2Bit_Err_Exception_Handlr (void)
{
    unsigned long tcm_addr = (unsigned long)0;
    uint8 tcm_tst = ATCMTST;

    TCM_Inj_Err_Addr_Convert(&tcm_addr, &tcm_tst);
    if(tcm_tst == B0TCMTST) {
#if (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        writell(0x0, tcm_addr);
#endif
#if (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        writell(0x0, (unsigned long)(tcm_addr + BTCMGAP));
#endif
    } else {
#if (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON)
        writell(0x0, tcm_addr);
#endif
    }
}
#endif

#if ((FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD == STD_ON))
/*Injecting error Dual-Core lockstep*/
FUNC(void, FUNCSAFETYPKG_CODE) DualCore_LockStep_InjectError (void)
{
    Sem_Enable_Eic(SEMgr1, TRUE);
    /*Enable DCCM0*/
    Mcu_ScrClrBit(APB_SCR_SAF_BASE, L16, SCR_SAF_CR5_SAF_DCCMINP_0_L16_START_BIT);
    Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, DUALCORE_DCCMOUT_0_EIC_POINT, TRUE);
    Mcu_ScrSetBit(APB_SCR_SAF_BASE, L16, SCR_SAF_CR5_SAF_DCCMINP_0_L16_START_BIT);
    /*Enable DCCM2*/
    Mcu_ScrClrBit(APB_SCR_SAF_BASE, L16, SCR_SAF_CR5_SAF_DCCMINP2_0_L16_START_BIT);
    Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, DUALCORE_DCCMOUT2_0_EIC_POINT, TRUE);
    Mcu_ScrSetBit(APB_SCR_SAF_BASE, L16, SCR_SAF_CR5_SAF_DCCMINP2_0_L16_START_BIT);

    /*Only one port not dectect dual core compare fail to report a notification to the upper*/
    if ((Sem_Interrupt_Status(SEMgr1, CR5_SAF____DCCMOUT_0) != TRUE)
        || (Sem_Interrupt_Status(SEMgr1, CR5_SAF____DCCMOUT2_0) != TRUE)) {
        if (g_DualCoreLockStepCurConfig->pDualCoreLockStepCompFailNotDetectedNotification != NULL_PTR) {
            g_DualCoreLockStepCurConfig->pDualCoreLockStepCompFailNotDetectedNotification();
        }
    }

    Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, DUALCORE_DCCMOUT_0_EIC_POINT, FALSE);
    Eic_Enable_Injectpoint(ONE_BIT_EIC_ERR, EIC_SAFE_MODULE, DUALCORE_DCCMOUT2_0_EIC_POINT, FALSE);
    /*Clr DCCM0*/
    Mcu_ScrSetBit(APB_SCR_SAF_BASE, R16W16_RW,
                  SCR_SAF_CR5_SAF_DCCMOUT_0_SIGLAT_SCR_CLR_R16W16_RW_START_BIT);

    while (!Mcu_ScrGetBit(APB_SCR_SAF_BASE, R16W16_RO,
                          SCR_SAF_CR5_SAF_DCCMOUT_0_SIGLAT_SCR_CLR_ACK_R16W16_RO_START_BIT));

    Mcu_ScrClrBit(APB_SCR_SAF_BASE, R16W16_RW,
                  SCR_SAF_CR5_SAF_DCCMOUT_0_SIGLAT_SCR_CLR_R16W16_RW_START_BIT);
    /*Clr DCCM2*/
    Mcu_ScrSetBit(APB_SCR_SAF_BASE, R16W16_RW,
                  SCR_SAF_CR5_SAF_DCCMOUT2_0_SIGLAT_SCR_CLR_R16W16_RW_START_BIT);

    while (!Mcu_ScrGetBit(APB_SCR_SAF_BASE, R16W16_RO,
                          SCR_SAF_CR5_SAF_DCCMOUT2_0_SIGLAT_SCR_CLR_ACK_R16W16_RO_START_BIT));

    Mcu_ScrClrBit(APB_SCR_SAF_BASE, R16W16_RW,
                  SCR_SAF_CR5_SAF_DCCMOUT2_0_SIGLAT_SCR_CLR_R16W16_RW_START_BIT);
}
#endif

#if (FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON == STD_ON)
/*SOC temperature monitor itself,temperature sensor shall be tested by adjusting over-heat threshold*/
FUNC(void, FUNCSAFETYPKG_CODE) Soc_Temp_Monitor(void)
{
    uint32 u32cur_temp = 0U;
    uint32 u32temp_thresh_value = 0U;
    uint32 wait_cnt = 0U;
    /*Read tempearature threshold value for pvt alarm hyst high mode*/
    u32temp_thresh_value = REG_READ32(APB_PVT_SNS_SAF_BASE + 0x08);
    /*Control mode:from register*/
    REG_RMW32(APB_PVT_SNS_SAF_BASE, 0U, 1U, 1U);
    /*Enable pvt alarm high/low mode for ctrl_0*/
    REG_RMW32(APB_PVT_SNS_SAF_BASE, 16U, 1U, 1U);
    /*Enable sw control sensor*/
    REG_RMW32(APB_PVT_SNS_SAF_BASE, 4U, 1U, 1U);
    /*Enable alarm high interrupt*/
    REG_RMW32((APB_PVT_SNS_SAF_BASE + 0x1c), 0U, 1U, 1U);

    /*Wait for pvt dout valid*/
    while ((!(REG_READ32(APB_PVT_SNS_SAF_BASE + 4) & 0x01)) && (wait_cnt < 50000U)) {
        wait_cnt++;
        //Mcu_udelay(1U);
        udelay(1U);
    }

    wait_cnt = 0u;
    /*Read current temperature*/
    u32cur_temp = ((REG_READ32(APB_PVT_SNS_SAF_BASE + 4) >> 1) & 0x3ff);
    /*Set threshold high value*/
    REG_RMW32((APB_PVT_SNS_SAF_BASE + 8), 0U, 10U, ((u32cur_temp - 20) & 0x3ff));

    /*Wait for high interrupt*/
    while ((!(REG_READ32(APB_PVT_SNS_SAF_BASE + 0x20) & 0x01)) && (wait_cnt < 50000U)) {
        wait_cnt++;
        //Mcu_udelay(1U);
        udelay(1U);
    }

    /*Send a notification to the upper if not detected PVT_SNS_SAF____PVT_INT_0*/
    if (Sem_Interrupt_Status(SEMgr1, PVT_SNS_SAF____PVT_INT_0) != TRUE) {
        if (g_SocTempMonitorTstCurConfig->pSocTempMonitorOverHeatIntNotDetectedNotification != NULL_PTR) {
            g_SocTempMonitorTstCurConfig->pSocTempMonitorOverHeatIntNotDetectedNotification();
        }
    }

    /*Set threshold high value*/
    REG_RMW32((APB_PVT_SNS_SAF_BASE + 8), 0U, 10U, (u32temp_thresh_value & 0x3ff));
    /*Clear HYST_HIGH interrupt*/
    REG_RMW32((APB_PVT_SNS_SAF_BASE + 0x24), 0U, 1U, 1U);
}
#endif




////////////////////////////////////////////////////////////////////////////////////////
extern void dump_154_155(uint32_t signal);
////////////////////////////////////////////////////////////////////////////////////////
/*IRQ for sem interrupt*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_Irq(void)
{
    /*Irq for sem signal monitor inject error*/
    if (gSemSigMonInjectErrEnable == TRUE) {
        gSemSigMonInjectErrIsrFlg = TRUE;
        gSemSigMonInjectErrEnable = FALSE;
        Sem_Monitor_Enable_Intr(SEMgr1, SEM_OUTPUT_CPU, FALSE);
    } else if (gSemSigMonEnable == TRUE) {
        gSemSigMonEnable = FALSE;
        Sem_Monitor_Enable_Intr(SEMgr1, SEM_OUTPUT_CPU, FALSE);
#if (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON)
        if (g_SemSignalMonCurConfig->pSemSignalUnMatchNotification != NULL_PTR) {
            g_SemSignalMonCurConfig->pSemSignalUnMatchNotification();
        }

#endif
    } else {
        if (g_SemIntCurConfig->pSemIntOutput2 != NULL_PTR)
        {
            /*Check sem input interrupt status and notify to Appl*/
            for (uint16 i = 0U; i < SEM_INT_OUT2_NUM; i++)
            {
                if (Sem_Interrupt_Status(SEMgr1, g_SemIntCurConfig->pSemIntOutput2[i]) == TRUE)
                {
                    ////////////////////////////////////////////////////////////////////
                    printf("Sem_Irq  id=%d \r\n",g_SemIntCurConfig->pSemIntOutput2[i]);

                    switch (g_SemIntCurConfig->pSemIntOutput2[i])
                    {
                        case DDR_SS____DDR_SS_FUNC_SAF_FATL_INT:
                        case DDR_SS____DDR_SS_FUNC_SAF_NONFATL_INT:
                             dump_154_155(g_SemIntCurConfig->pSemIntOutput2[i]);
                             break;
                        default:
                             break;
                    }
                    ////////////////////////////////////////////////////////////////////
                    if (g_SemIntCurConfig->pSemIntOutput2CpuNotification != NULL_PTR)
                    {
                        g_SemIntCurConfig->pSemIntOutput2CpuNotification((uint32)g_SemIntCurConfig->pSemIntOutput2[i]);
                    }
                    Sem_IntSta_Clr(g_SemIntCurConfig->pSemIntOutput2[i]);
                }
            }
        }
    }
}

void Iram_Inj1BitErr_test(void)
{
    /*Disable SEM input interrupt*/
    SemOutInterruptEnable(FALSE);

    arch_disable_cache(DCACHE);
    Iram_Mem_Inj1BitErr_Func(IRAMCtrl1);
    Iram_Mem_Inj1BitErr_Func(IRAMCtrl2);
    Iram_Mem_Inj1BitErr_Func(IRAMCtrl3);
    Iram_Mem_Inj1BitErr_Func(IRAMCtrl4);
    arch_enable_cache(DCACHE);

    /*Enable SEM input interrupt*/
    SemOutInterruptEnable(TRUE);
}


void sem_check_status(void)
{
    printf("sem_check_status\n");
    for (uint16 i = 0U; i < 318; i++)
    {
        if (Sem_Interrupt_Status(SEMgr1, i) == TRUE)
        {
            printf("sem:%d\n", i);
        }
    }
}

uint32_t IramWrPath_Inject_Error_Check(void)
{
#if ((IRAM1_1BIT_ERROR_INJECT == STD_ON) || (IRAM1_2BIT_ERROR_INJECT == STD_ON))
    plat_disable_interrupt(SEM1_O_SEM_INT_CPU_NUM);
    Fabric_Inject_SigMulErr_Func();
#endif
    return 0U;
}

uint32_t Iram_Inject_1bitError_Check(void)
{
#if (IRAM1_1BIT_ERROR_INJECT == STD_ON)
    arch_disable_cache(DCACHE);
    Iram_Inject_SigMulErr_Func(IRAMCtrl1);
    arch_enable_cache(DCACHE);
#endif
    return 0U;
}

uint32_t Iram_Inject_2bitError_Check(void)
{
#if (IRAM1_2BIT_ERROR_INJECT == STD_ON)
    Iram_Mem_Inj2BitErr_Func(IRAMCtrl1);
#endif
    return 0U;
}

uint32_t Fuse_Inject_Error_Check(void)
{
    Fuse_Inject_Err_Check();
    return 0U;
}

uint32_t Tcm_1bitError_Check(void)
{
    TCM_Inject_1bitErr();
    return 0U;
}

uint32_t Tcm_2bitError_Check(void)
{
    TCM_Inject_2bitErr();
    return 0U;
}

uint32_t Dcache_1bitError_Check(void)
{
    Dcache_1bitErr_Injection();
    return 0U;
}

uint32_t Sem_Signal_Monitor1(void)
{
    Sem_SigMonitor_Func();
    return 0U;
}

uint32_t Run_Bist_NocSafeCore(void)
{
    Run_Bist_Noc_SafeCore();
    return 0U;
}

uint32_t Wdt_SelfTest(void)
{
    Wdt_SelfTst_Func();
    DBG("self test of wdt1 has been finished\n");
    return 0U;
}

uint32_t FsrefClk_RcChkAp(void)
{
    Fsrefclk_RcChkAp();
    return 0U;
}

uint32_t FsrefClk_SafChkAp(void)
{
    Fsrefclk_SafChkAp();
    return 0U;
}

void FuncSafetyPkg_Run_Period_Func_test() {

    while(1) {
        if(FuncSafetyPkg_Run_Period_Run_Flag == true) {
            FuncSafetyPkg_Run_Period_Func();
            printf("\r\n### %s is running!\r\n",__FUNCTION__);
        }
        osDelay(1000);
    }
}

static int FusalibTest(int argc, char *argv[])
{
    if (argc != 1) {
        printf("input argv error\n");
        return 0;
    }

    uint32_t cmd_value = strtoul(argv[0], NULL, 10);
    printf("%s: Fusalib test cmd value: %d\n", __func__, cmd_value);

    switch (cmd_value)
    {
        case TESTID0:
            IramWrPath_Inject_Error_Check();
            break;
        case TESTID1:
            Iram_Inject_1bitError_Check();
            break;
        case TESTID2:
            Iram_Inject_2bitError_Check();
            break;
        case TESTID3:
            Fuse_Inject_Error_Check();
            break;
        case TESTID4:
            Tcm_1bitError_Check();
            break;
        case TESTID5:
            Tcm_2bitError_Check();
            break;
        case TESTID6:
            Dcache_1bitError_Check();
            break;
        case TESTID7:
            Sem_Signal_Monitor1();
            break;
        case TESTID8:
            Run_Bist_NocSafeCore();
            break;
        case TESTID9:
            Wdt_SelfTest();
            break;
        case TESTID10:
            FsrefClk_RcChkAp();
            break;
        case TESTID11:
            FsrefClk_SafChkAp();
            break;
        case TEST_PERIOD_START:
            {
                // process thread
                osThreadAttr_t handler_attr = {.name = "FuncSafetyPkg_Run_Period_Func", .stack_size = 4096, .priority = osPriorityNormal};
                osThreadId_t handler = osThreadNew(FuncSafetyPkg_Run_Period_Func_test,NULL, &handler_attr);
                if (NULL == handler) {
                    printf("[%s] create process thread  failed!!\n", __func__);
                }
            }
            FuncSafetyPkg_Run_Period_Run_Flag = true;
            break;
        case TEST_PERIOD_STOP:
            FuncSafetyPkg_Run_Period_Run_Flag = false;
            break;
        case TESTALL:
            FuncSafetyPkg_Run_PowerOn_Func();
        default:
            printf("input argv error\n");
            break;

    }
        return 0;
}

CLI_CMD("FT", "Fusalib test\r\n\tFT 253 = Start Fusalib period test \r\n\tFT 254 = Stop Fusalib period test \r\n", FusalibTest);
