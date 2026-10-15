/********************************************************
 *   @file    FuncSafetyPkg.h
 *
 *   @brief   Function safety package header file.
 *   @details
 *
 *********************************************************/

/*********************************************************
 *        Copyright(c) 2020    Semidrive                 *
 *        All rights reserved.                           *
 *********************************************************/
#ifndef _FUNCSAFETYPKG_H_
#define _FUNCSAFETYPKG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include"eic_hw.h"
#include"sem_hw.h"
#include"Eic_Ip.h"
#include"Sem_Ip.h"
#include"Iram_Ip.h"
#include"Fuse_Ip.h"
#include"FuncSafetyPkg_Cfg.h"
#include"FuncSafetyPkg_Type.h"

/*Address for injecting error in IAR */
#if defined(_IAR_C_)
#define __inject_err_iram1_start region_IRAM1INJERR_start__
#define __inject_err_btcm_start region_BTCMINJERR_start__
#define __inject_err_atcm_start region_ATCMINJERR_start__
#endif

/*Macro for TCM test*/
#define BTCMGAP         0x08
#define ATCMTST         0x01
#define B0TCMTST        0x02
#define B1TCMTST        0x04

/*The total numbers of clear sem interrupt through SCR*/
#ifndef CONFIG_BOARD_X9SP
#define SEM_SCR_NUM                              89U
#else
#define SEM_SCR_NUM                              14U
#endif
/*The number of function safety task when ecu powers on*/
#define FUNCSAFETY_POWERONTASK_NUM               11U

/*The number of function safety periodic task*/
#define FUNCSAFETY_PERIODTASK_NUM               3U

/*The total numbers to get signal value for sem monitor signal through SCR */
#define SEM_MON_SIG_SCR_NUM                     56U

#ifdef CONFIG_BOARD_X9SP
/*The number of eic point for Iram1/2/3/4/Fuse*/
#define IRAM1_EIC_POINT                         46U
#define IRAM2_EIC_POINT                         61U
#define IRAM3_EIC_POINT                         62U
#define IRAM4_EIC_POINT                         63U
#define FUSE_EIC_POINT                          48U
#define LATENTFAULT0_CHK                        0x00100000U
#define LATENTFAULT1_CHK                        0x00008000U
#else
/*The number of eic point for Iram1/2/3/4/Fuse*/
#define IRAM1_EIC_POINT                         32U
#define IRAM2_EIC_POINT                         49U
#define IRAM3_EIC_POINT                         50U
#define IRAM4_EIC_POINT                         51U
#define FUSE_EIC_POINT                          34U
#define LATENTFAULT0_CHK                        0x4000U
#define LATENTFAULT1_CHK                        0x08U
#endif

#define DUALCORE_DCCMOUT_0_EIC_POINT            55U
#define DUALCORE_DCCMOUT2_0_EIC_POINT           63U

/*BIST Ctrl register*/
#define INTEN                                   0x2CU
#define LATENTFAULT0                            0X18U
#define LATENTFAULT1                            0X1CU
#define LATENTFAULT2                            0X20U
#define LATENTFAULT3                            0X24U
#define INTCLR                                  0X30U
#define BISTCTRL                                0x34U
#define BISTDONE                                0x38U
#define BISTTO1                                 0x3CU
#define BISTTO2                                 0x40U
#ifndef CONFIG_BOARD_X9SP
#define FAB_GPV_HPIA                            (APB_GPV_HPIA_BASE+0x2880U)
#endif

#define BANK_SHORT_LEN                          0x01U
#define BANK_LONG_LEN                           0x04U
#define BIT_SHORT_LEN                           0x10U
#define BIT_LONG_LEN                            0x80U

/*Select one point for iram inject 1/2 bit error when injecting error mode is INJECT_FIXED_POINT*/
#define IRAM_INJECT_ERR_POINT                   0U

/*Define default address for IRAM3~IRAM4*/
#define IRAM2_INJERR_ADDR                       0X17FF00
#define IRAM3_INJERR_ADDR                       0x1BFF00
#define IRAM4_INJERR_ADDR                       0x1FFF00

/* Inject error mode in Iram or fabric when ECU power on                        *
 * INJECT_ALL_TRAVEL: Inject 1/2 bit error in all points                        *
 * INJECT_FIXED_POINT: Inject 1/2 bit error in one point for one bank in iram   *
 *  or wdata/waddr/rdata/raddr in fabric                                        */
#define INJECT_ALL_TRAVEL                       0x00U
#define INJECT_FIXED_POINT                      0x01U

#define FUSE_BIT_LONG_LEN                       0x20U
#define FUSE_BIT_SHORT_LEN                      0x07U

/*Select one bit for fuse inject 1/2 bit error when injecting error mode is INJECT_FIXED_POINT*/
#define FUSE_INJECT_ERR_BIT                     0x03U

/*To distinguish Data cache data ram/tag ram*/
#define DCACHE_DATARAM                          0x01U
#define DCACHE_TAGRAM                           0x02U

#ifndef CONFIG_BOARD_X9SP
/*Data cache address*/
#define DCACHE_ADDR                             0x4F8000U

/*Instruction cache address*/
#define ICACHE_ADDR                             0x4F0000U

/*IRAM1 address*/
#define IRAM1_ADDR                              0x00100000U

/*IRAM2 address*/
#define IRAM2_ADDR                              0x00140000U

/*IRAM3 address*/
#define IRAM3_ADDR                              0x00180000U
#else
/*Data cache address*/
#define DCACHE_ADDR                             0x021F8000U

/*Instruction cache address*/
#define ICACHE_ADDR                             0x021F0000U

/*IRAM1 address*/
#define IRAM1_ADDR                              0x00600000U

/*IRAM2 address*/
#define IRAM2_ADDR                              0x00700000U

/*IRAM3 address*/
#define IRAM3_ADDR                              0x00740000U
#endif

/*IRAM4 address*/
#define IRAM4_ADDR                              0x001C0000U

/*Inject error info : address offset and value*/
#define INJECT_ERR_REG_ADDR_oFF                 0x580U
#define INJECT_ERR_DDR_ADDR_oFF                 0x10000U
#define INJECT_ERR_VALUE                        0x1234abcdU

/*Correctable interrupt error status reset mask*/
#define CERR_INT_STA_RST_MASK                 0x0bU
/*Uncorrectable interrupt error status reset mask*/
#define UNCERR_INT_STA_RST_MASK                 0x8f7015abU

/*SCR start bit for APB MUX2 RPC SAFE*/
#define SCR_SAF_APBMUX2_RPC_SAF_SELECT_L16_START_BIT 0x00U

/*one byte convert to 32bit integer*/
#define BITCONVTOUNIT32(x)   (((x) << 24) | ((x) << 16) | ((x) << 8) | (x))

/* SCR register information lookup table. */
#define SCR_SEM_INFO(domin, ip, port) \
    {APB_SCR_##domin##_BASE, \
     SCR_##domin##_##ip##_##port##_SIGLAT_SCR_CLR_R16W16_RW_START_BIT, \
     SCR_##domin##_##ip##_##port##_SIGLAT_SCR_CLR_ACK_R16W16_RO_START_BIT, \
     ip##____##port \
    }

/* SCR register information lookup table. */
#define SCR_SEM_MON_SIG_INFO(input_signal) \
    {SCR_SAF_##input_signal##ECT_L16_START_BIT, \
    }

#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*global variable*/
extern boolean Inject2bitTcmErrFlg;
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*1/2bits Error injection for fabric signal,then wr/rd value */
FUNC(void, FUNCSAFETYPKG_CODE) Eic_Inject_Err_WrRd_Val_Comp_Func
(
    VAR(enum eic_signal, AUTOMATIC)     _signal_index,
    VAR(uint32, AUTOMATIC)              _addr,
    VAR(uint32, AUTOMATIC)              _wr_val,
    VAR(Eic_Error_BitType, AUTOMATIC)   _err_bit
)_SECTION(.tcm_func);

/*Inject 1/2 bits error  on fabric*/
FUNC(void, FUNCSAFETYPKG_CODE) Fabric_Inject_SigMulErr
(
    VAR(Error_PosType, AUTOMATIC)       err_position,
    VAR(enum eic_signal, AUTOMATIC)     signal_first,
    VAR(enum eic_signal, AUTOMATIC)     signal_last,
    VAR(uint32, AUTOMATIC)              _addr,
    VAR(uint32, AUTOMATIC)              _wr_val,
    VAR(Eic_Error_BitType, AUTOMATIC)   _err_bit
);
#endif

/*Function safety pakage init*/
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Init(void);

#if (FUSAPKG_SELF_TEST_POWER_ON == STD_ON)
/*Function Safety package main function during ECU power on*/
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Run_PowerOn_Func(void);
#endif

#if (FUSAPKG_SELF_TEST_PERIOD == STD_ON)
/*Function Safety package main function period*/
FUNC(void, FUNCSAFETYPKG_CODE) FuncSafetyPkg_Run_Period_Func(void);
#endif

#if ((FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1/2 bits error on fabric for all traversal*/
FUNC(void, FUNCSAFETYPKG_CODE) Fabric_Inject_SigMulErr_Func(void);
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
) _SECTION(.tcm_func);
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
);
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
);
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject one bit error in iram memory wr/rd path for all traversal or one fixed point*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj1BitErr_Func
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
);
#endif

#if ((FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject two bits error in iram memory wr/rd path for all traversal*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Mem_Inj2BitErr_Func
(
    VAR(IramCtlType, AUTOMATIC) IramCtl
);
#endif

#if ((FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD == STD_ON) || \
     (FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2 bit error in Iram WR/RD path,WR path is only for Iram1,RD path can only for Iram1/2/3/4*/
FUNC(void, FUNCSAFETYPKG_CODE) Iram_Inject_SigMulErr_Func(void);
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2bits error mode*/
FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Inject_Err_Mode
(
    VAR(FuseInjectErrType, AUTOMATIC)       Inject_Err,
    VAR(FuseErrStaType, AUTOMATIC)          Err_Sta,
    VAR(uint8, AUTOMATIC)                   Bit_Len,
    VAR(Fuse_Error_BitType, AUTOMATIC)      Err_Bit,
    VAR(uint32, AUTOMATIC)                  Sig_Bit
);
#endif

#if ((FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD == STD_ON) || (FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD == STD_ON))
/*Inject 1/2 bit error in fuse register and check errror*/
FUNC(void, FUNCSAFETYPKG_CODE) Fuse_Inject_Err_Check(void);
#endif

/*IRQ for sem interrupt*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_Irq(void);

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
/*SEM signal Monitor and inject error*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_SigMonitor_Func(void);
#endif

#if ((FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_SEM_SIGMONITOR_POWER_ON == STD_ON))
/*Set SEM signal wrong expected target value to trigger checking or an expected target value to monitor sem signal*/
FUNC(void, FUNCSAFETYPKG_CODE) Sem_Signal_Monitor
(
    SemSigMonTstType semsig_tstmode
);
#endif

/*Inject 1 bit error in cr5 tcm*/
FUNC(void, FUNCSAFETYPKG_CODE) cr5_tcm_ecc_tst_s_2biterr
(
    VAR(uint32, AUTOMATIC) flgs,
    VAR(uint32, AUTOMATIC) inject_err_addr
);

/*Inject 2 bits error in cr5 tcm*/
FUNC(void, FUNCSAFETYPKG_CODE) cr5_tcm_ecc_tst_s_1biterr
(
    VAR(uint32, AUTOMATIC) flgs,
    VAR(uint32, AUTOMATIC) inject_err_addr
);

#if ((FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON == STD_ON) || (FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD == STD_ON))
/*Injecting error Dual-Core lockstep*/
FUNC(void, FUNCSAFETYPKG_CODE) DualCore_LockStep_InjectError (void);
#endif

/*Enable data cache*/
FUNC(void, FUNCSAFETYPKG_CODE) arch_enable_dcache(void);

/*Disable data cache*/
FUNC(void, FUNCSAFETYPKG_CODE) arch_disable_dcache(void);

/*Enable instruction cache*/
FUNC(void, FUNCSAFETYPKG_CODE) arch_enable_icache(void);

/*Disable instruction cache*/
FUNC(void, FUNCSAFETYPKG_CODE) arch_disable_icache(void);

/*Set event bus*/
FUNC(void, FUNCSAFETYPKG_CODE) PMU_Set_Event(uint32 event_bit);

/*Enable the event count in PMCNTENSET register*/
FUNC(void, FUNCSAFETYPKG_CODE) PMU_Enable_Event_Cnt(void);

/*Enable all counts in PMCR register*/
FUNC(void, FUNCSAFETYPKG_CODE) PMU_Enable_All_Cnt(void);

/*Read current PMNx register.*/
FUNC(uint32, FUNCSAFETYPKG_CODE) PMU_Read_Event_Cnt(void);

/*Write current PMNx register.*/
FUNC(void, FUNCSAFETYPKG_CODE) PMU_Write_Event_Cnt(uint32 cnt);

/*Set CACHE ecc error check*/
FUNC(void, FUNCSAFETYPKG_CODE) arch_set_cache_error_check(void);

#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
/*Injecting 1bit error in data cache for data ram and tag ram*/
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_1bitErr_Injection(void);
#endif

/*Injecting 1bit error in data cache for data ram*/
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_DataRam_1bitErr_Injection(void);
#endif

/*Injecting 1bit error in data cache for tag ram*/
#if (FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON == STD_ON)
FUNC(void, FUNCSAFETYPKG_CODE) Dcache_TagRam_1bitErr_Injection(void);
#endif

#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || \
     (FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1/2bits error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_InjSigMulErr_Func (void);
#endif

#if ((FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 1bit error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_Inject_1bitErr (void);
#endif

#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
/*Inject 2bits error in ATCM/B0TCM/B1TCM*/
FUNC(void, FUNCSAFETYPKG_CODE) TCM_Inject_2bitErr (void);
#endif

/*Cancel injecting error in tcm through writing data to TCM when enters exception*/
#if ((FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON) || (FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON == STD_ON))
FUNC(void, FUNCSAFETYPKG_CODE) Tcm_Inject_2Bit_Err_Exception_Handlr (void);
#endif

#if (FUSAPKG_RUN_BIST_POWER_ON == STD_ON)
/*run BIST sequence*/
FUNC(void, FUNCSAFETYPKG_CODE) Run_Bist_Sequence_Func
(
    VAR(uint32, AUTOMATIC)         _base_addr,
    VAR(uint32, AUTOMATIC)         bist_timeout1,
    VAR(uint32, AUTOMATIC)         bist_timeout2,
    VAR(Bist_NocType, AUTOMATIC)   Noc_Type
);

/*Run bist in noc of safe core : noc_saf noc_hpia noc_hpib*/
FUNC(void, FUNCSAFETYPKG_CODE) Run_Bist_Noc_SafeCore(void);
#endif

#if (FUSAPKG_WDT_SELFTEST_POWER_ON == STD_ON)
/*self-test mode for wdt*/
FUNC(void, FUNCSAFETYPKG_CODE) Wdt_SelfTst_Func();
#endif

#if (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON)
/*Fail safe reference clock : safe check ap*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_SafChkAp(void);
#endif

#if (FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON)
/*Fail safe reference clock : rc check ap*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_RcChkAp(void);
#endif

#if ((FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON == STD_ON) || (FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON == STD_ON))
/*Fail safe reference clock check each other among safe,ap,rc clock*/
FUNC(void, FUNCSAFETYPKG_CODE) Fsrefclk_Check_Func(void);
#endif

#if (FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON == STD_ON)
/*SOC temperature monitor itself*/
FUNC(void, FUNCSAFETYPKG_CODE) Soc_Temp_Monitor(void);
#endif


#ifdef __cplusplus
}
#endif

#endif



