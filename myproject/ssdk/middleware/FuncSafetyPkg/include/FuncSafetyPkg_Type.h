/**
 *   @file    FuncSafetyPkg_Type.h
 *
 *   @brief   FuncSafetyPkg(Functional safety package) header file.
 *   @details
 *           
 */

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#ifndef FUNCSAFETYPKG_TYPE_H
#define FUNCSAFETYPKG_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"


/* Numeric identifier of Iram. */
typedef uint8 Iram_ZoneIdType;

/* Definition of the type of bist noc. */
typedef enum {
    NOC_SAFE = 0U,
    NOC_SEC,
    NOC_HPIA,
    NOC_HPIB
} Bist_NocType;

/* Definition of the type of injecting error in which iram. */
typedef enum {
    IRAM1ERR = 0U,
    IRAM2ERR,
    IRAM3ERR,
    IRAM4ERR
} Iram_ErrZoneType;

/*Error injection postion type on fabric*/
typedef enum {
    E2 = 0U,
    E5,
    E6,
    E7,
    E8
} Error_PosType;

/*Sem signal monitor and inject error type*/
typedef enum {
    SEM_SIG_MON = 0U,
    SEMSIGMON_INJERR
} SemSigMonTstType;

/*Array index enum for g_SemScrinfo[](Clr sem interrupt through scr)*/
#ifndef CONFIG_BOARD_X9SP
typedef enum {
    SEC_NOCSEC_M_0_MAINSGLECCERR = 0U,
    SEC_NOCSEC_M_1_MAINSGLECCERR,
    SEC_NOCSEC_M_2_MAINSGLECCERR,
    SEC_NOCSEC_M_3_MAINSGLECCERR,
    SEC_NOCSEC_M_5_MAINSGLARADDRECCERR,
    SEC_NOCSEC_M_5_MAINSGLAWADDRECCERR,
    SEC_NOCSEC_M_5_MAINSGLDATAECCERR,
    SEC_NOCSEC_S_0_MAINSGLECCERR,
    SEC_NOCSEC_S_1_MAINSGLECCERR,
    SEC_NOCSEC_S_2_MAINSGLDATAECCERR,
    SEC_NOCSEC_S_3_MAINSGLDATAECCERR,
    SEC_NOCSEC_S_13_MAINSGLDATAECCERR,
    SEC_NOCSEC_S_14_MAINSGLDATAECCERR,
    SEC_NOCSEC_S_17_MAINSGLDATAECCERR,
    SEC_NOCMAIN_M_11_MAINSGLARADDRECCERR,
    SEC_NOCMAIN_M_11_MAINSGLAWADDRECCERR,
    SEC_NOCMAIN_M_11_MAINSGLDATAECCERR,
    SEC_NOCMAIN_M_12_MAINSGLDATAECCERR,
    SEC_NOCMAIN_M_12_MAINSGLAWADDRECCERR,
    SEC_NOCMAIN_M_12_MAINSGLARADDRECCERR,
    SEC_NOCMAIN_M_24_MAINSGLARADDRECCERR,
    SEC_NOCMAIN_M_24_MAINSGLAWADDRECCERR,
    SEC_NOCMAIN_M_24_MAINSGLDATAECCERR,
    SEC_NOCMAIN_S_0_SBP_MAINSGLDATAECCERR,
    SEC_NOCMAIN_S_3_MAINSGLDATAECCERR,
    SEC_NOCMAIN_S_4_MAINSGLDATAECCERR,
    SEC_NOCMAIN_S_11_MAINSGLDATAECCERR,
    SAF_NOCMAINB_M_0_MAINSGLARADDRECCERR,
    SAF_NOCMAINB_M_0_MAINSGLAWADDRECCERR,
    SAF_NOCMAINB_M_0_MAINSGLDATAECCERR,
    SAF_NOCSAF_M_0_MAINDUPL_SGLECCERR,
    SAF_NOCSAF_M_0_MAINMAIN_SGLECCERR,    
    SAF_NOCSAF_M_1_MAINDUPL_SGLECCERR,
    SAF_NOCSAF_M_1_MAINMAIN_SGLECCERR,
    SAF_NOCSAF_M_4_MAINMAIN_SGLARADDRECCERR,
    SAF_NOCSAF_M_4_MAINMAIN_SGLAWADDRECCERR,
    SAF_NOCSAF_M_4_MAINMAIN_SGLDATAECCERR,
    SAF_NOCSAF_M_4_MAINDUPL_SGLARADDRECCERR,
    SAF_NOCSAF_M_4_MAINDUPL_SGLAWADDRECCERR,
    SAF_NOCSAF_M_4_MAINDUPL_SGLDATAECCERR,
    SAF_NOCSAF_S_0_MAINDUPL_SGLECCERR,
    SAF_NOCSAF_S_0_MAINMAIN_SGLECCERR,
    SAF_NOCSAF_S_1_MAINMAIN_SGLDATAECCERR,
    SAF_NOCSAF_S_1_MAINDUPL_SGLDATAECCERR,
    SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR,
    SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR,
    SAF_NOCSAF_S_4_MAINDUPL_SGLDATAECCERR,
    SAF_NOCSAF_S_4_MAINMAIN_SGLDATAECCERR,
    SAF_CR5SAF_AXIMCORR0,
    SAF_CR5SAF_AXISCORR0,
    SAF_CR5SAF_PPXCORR0,
    SAF_CR5SAF_PPHCORR0,
    SAF_CR5SAF_AXIMFATAL0_0,
    SAF_CR5SAF_AXIMFATAL0_1,
    SAF_CR5SAF_AXIMFATAL0_2,
    SAF_CR5SAF_AXIMFATAL0_3,
    SAF_CR5SAF_AXIMFATAL0_4,
    SAF_CR5SAF_AXISFATAL0_0,
    SAF_CR5SAF_AXISFATAL0_1,
    SAF_CR5SAF_AXISFATAL0_2,
    SAF_CR5SAF_AXISFATAL0_3,
    SAF_CR5SAF_AXISFATAL0_4,
    SAF_CR5SAF_PPXFATAL0_0,
    SAF_CR5SAF_PPXFATAL0_1,
    SAF_CR5SAF_PPXFATAL0_2,
    SAF_CR5SAF_PPXFATAL0_3,
    SAF_CR5SAF_PPXFATAL0_4,
    SAF_CR5SAF_DCCMOUT_0,
    SAF_CR5SAF_DCCMOUT2_0,
    SEC_CR5SEC_AXIMCORR0,
    SEC_CR5SEC_PPXCORR0,
    SEC_CR5SEC_PPHCORR0,
    SEC_CR5SEC_AXIMFATAL0_0,
    SEC_CR5SEC_AXIMFATAL0_1,
    SEC_CR5SEC_AXIMFATAL0_2,
    SEC_CR5SEC_AXIMFATAL0_3,
    SEC_CR5SEC_AXIMFATAL0_4,
    SEC_CR5SEC_AXISFATAL0_0,
    SEC_CR5SEC_AXISFATAL0_1,
    SEC_CR5SEC_AXISFATAL0_2,
    SEC_CR5SEC_AXISFATAL0_3,
    SEC_CR5SEC_AXISFATAL0_4,
    SEC_CR5SEC_PPXFATAL0_0,
    SEC_CR5SEC_PPXFATAL0_1,
    SEC_CR5SEC_PPXFATAL0_2,
    SEC_CR5SEC_PPXFATAL0_3,
    SEC_CR5SEC_PPXFATAL0_4,
    SEC_CR5SEC_DCCMOUT_0,
    SEC_CR5SEC_DCCMOUT2_0
} Sem_Scr_Index;
#else
typedef enum {
    SAF_NOCSAF_S_2_MAINMAIN_SGLDATAECCERR= 0U,
    SAF_NOCSAF_S_2_MAINDUPL_SGLDATAECCERR,
    SAF_NOCSAF_M_0_MAINDUPL_SGLECCERR,
    SAF_NOCSAF_M_0_MAINMAIN_SGLECCERR,
    SAF_NOCSAF_M_1_MAINDUPL_SGLECCERR,
    SAF_NOCSAF_M_1_MAINMAIN_SGLECCERR,
    SAF_CR5SAF_AXIMCORR0,
    SAF_CR5SAF_AXIMFATAL0_2,
    SAF_CR5SAF_PPXCORR0,
    SAF_CR5SAF_PPXFATAL0_0,
    SAF_CR5SAF_PPXFATAL0_1,
    SAF_CR5SAF_PPXFATAL0_2,
    SAF_CR5SAF_PPXFATAL0_3,
    SAF_CR5SAF_PPXFATAL0_4,
} Sem_Scr_Index;
#endif
/* FuncSafetyPkg: notification callback */
typedef void (*Iram1BitErrUnCorr_NotifyType)(uint32 Iram1bitErrCode);
typedef void (*Iram1BitErrUnDetect_NotifyType)(uint32 Iram1bitErrCode);
typedef void (*Iram2BitErrUnDetect_NotifyType)(uint32 Iram2bitErrCode);
typedef void (*ATCM1BitErrUnDetect_NotifyType)(void);
typedef void (*ATCM2BitErrUnDetect_NotifyType)(void);
typedef void (*B0TCM1BitErrUnDetect_NotifyType)(void);
typedef void (*B0TCM2BitErrUnDetect_NotifyType)(void);
typedef void (*B1TCM1BitErrUnDetect_NotifyType)(void);
typedef void (*B1TCM2BitErrUnDetect_NotifyType)(void);
typedef void (*DcacheDataRam1BitErrUnDetect_NotifyType)(uint32 DcacheErrCode);
typedef void (*DcacheTagRam1BitErrUnDetect_NotifyType)(uint32 DcacheErrCode); 
typedef void (*Fabric1BitErrUnDetect_NotifyType)(uint32 Fabric1bitErrCode);
typedef void (*Fabric2BitErrUnDetect_NotifyType)(uint32 Fabric2bitErrCode);
typedef void (*Fuse1BitErrErrStaUnDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*Fuse1BitErrIntStaUnDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*Fuse2BitErrErrStaUnDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*Fuse2BitIntErrStaUnDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*FuseCorrDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*FuseErrDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*FuseIntDetect_NotifyType)(uint32 FuseErrCode);
typedef void (*SemSignal_NotifyType)(void);
typedef void (*LatentFault0_NotifyType)(uint32 Noc_Type, uint32 FaultCode);
typedef void (*LatentFault1_NotifyType)(uint32 Noc_Type, uint32 FaultCode);
typedef void (*LatentFault2_NotifyType)(uint32 Noc_Type, uint32 FaultCode);
typedef void (*LatentFault3_NotifyType)(uint32 Noc_Type, uint32 FaultCode);
typedef void (*WdtSelfTstOverFlow_NotifyType)(void);
typedef void (*RcChkAp_NotifyType)(void);
typedef void (*SafeChkAp_NotifyType)(void);
typedef void (*SemIntMonitor_NotifyType)(uint32 SemInputStatus);
typedef void (*DualCoreLockStepInjectErr_NotifyType)(void);
typedef void (*SocTempMonitorTst_NotifyType)(void);

/* Function safety task for power on*/
typedef void (*pPoweronFuncSafetyTask)(void);

/*Periodic function safety task type*/
typedef void (*pPeriodFuncSafetyTask)(void);

/*The type of function safety package task when ecu power on*/
typedef struct {
    pPoweronFuncSafetyTask pFuncSafetyTask;
} PoweronFuncSafetyPkgTaskType;

/*The type of function safety package task periodically*/
typedef struct {
    pPeriodFuncSafetyTask pFuncSafetyTask;
    uint8 u8RunPeriodCnt;
    boolean bPerformedFlg;
} PeriodFuncSafetyPkgTaskType;

/** @brief sem start/ack bits in SCR.*/
typedef struct {
    uint32 u32DominAddr;
    uint32 u32StartPos;
    uint32 u32AckPos;
    uint32 u32SemIntId;
} Sem_ScrBitsType;

/** @brief sem start bits for SEM signal monitor in SCR.*/
typedef struct {
    uint32 u32StartPos;
} SemMonSig_ScrBitsType;

/*@brief the type of the configuration for Iram*/
typedef struct {
    Iram_ZoneIdType  u8IramZoneId;
    Iram_ErrZoneType    eIram_Zone;
    CONST(Iram1BitErrUnCorr_NotifyType, FUNCSAFETYPKG_CONST)  pIram1bitUnCorrNotification;
    CONST(Iram1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pIram1bitUnDetectNotification;
    CONST(Iram2BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pIram2bitUnDetectNotification;
} Iram_ZoneConfigType;

/*@brief the type of the configuration for TCM*/
typedef struct {
    CONST(ATCM1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)   pATCM1bitUnDetectNotification;
    CONST(ATCM2BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)   pATCM2bitUnDetectNotification;
    CONST(B0TCM1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pB0TCM1bitUnDetectNotification;
    CONST(B0TCM2BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pB0TCM2bitUnDetectNotification;    
    CONST(B1TCM1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pB1TCM1bitUnDetectNotification;
    CONST(B1TCM2BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pB1TCM2bitUnDetectNotification;       
} TCMErrConfigType;

/*@brief the type of the configuration for Data cache data ram*/
typedef struct {
    CONST(uint32, FUNCSAFETYPKG_CONST) u32BankNum;
    CONST(uint32, FUNCSAFETYPKG_CONST) u32RamIndex;
    CONST(DcacheDataRam1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)   pDcacheDataRam1bitUnDetectNotification;    
} DCACHEDataRamErrConfigType;

/*@brief the type of the configuration for Data cache data ram*/
typedef struct {
    CONST(uint32, FUNCSAFETYPKG_CONST) u32BankNum;
    CONST(uint32, FUNCSAFETYPKG_CONST) u32RamIndex;
    CONST(DcacheTagRam1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)    pDcacheTagRam1bitUnDetectNotification;    
} DCACHETagRamErrConfigType;

/*@brief the type of the configuration for Data cache*/
typedef struct {
    P2CONST(DCACHEDataRamErrConfigType, FUNCSAFETYPKG_VAR, FUNCSAFETYPKG_APPL_CONST)   pDcacheDataRamErrConfig;
    P2CONST(DCACHETagRamErrConfigType, FUNCSAFETYPKG_VAR, FUNCSAFETYPKG_APPL_CONST)    pDcacheTagRamErrConfig;      
} DCACHEErrConfigType;

/*@brief the type of the configuration for fabric*/
typedef struct {
    CONST(Fabric1BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pFabric1bitUnDetectNotification;
    CONST(Fabric2BitErrUnDetect_NotifyType, FUNCSAFETYPKG_CONST)  pFabric2bitUnDetectNotification;
} FabricErrConfigType;

/*@brief the type of the configuration for fuse*/
typedef struct {
    CONST(Fuse1BitErrErrStaUnDetect_NotifyType,
          FUNCSAFETYPKG_CONST)  pFuse1bitErrStaUnDetectNotification;
    CONST(Fuse1BitErrIntStaUnDetect_NotifyType,
          FUNCSAFETYPKG_CONST)  pFuse1bitIntStaUnDetectNotification;
    CONST(Fuse2BitErrErrStaUnDetect_NotifyType,
          FUNCSAFETYPKG_CONST)  pFuse2bitErrStaUnDetectNotification;
    CONST(Fuse2BitIntErrStaUnDetect_NotifyType,
          FUNCSAFETYPKG_CONST)  pFuse2bitIntStaUnDetectNotification;
} FuseErrConfigType;

/*@brief the type of the configuration for sem signal monitor*/
typedef struct {
    CONST(SemSignal_NotifyType, FUNCSAFETYPKG_CONST) pSemSignalUnMatchNotification;
    CONST(SemSignal_NotifyType, FUNCSAFETYPKG_CONST) pSemSignalInjErrUnDetectNotification;
} SemSignalMonConfigType;

/*@brief the type of the configuration for bist*/
typedef struct {
    CONST(LatentFault0_NotifyType, FUNCSAFETYPKG_CONST)  pLatentFault0Notification;
    CONST(LatentFault1_NotifyType, FUNCSAFETYPKG_CONST)  pLatentFault1Notification;
    CONST(LatentFault2_NotifyType, FUNCSAFETYPKG_CONST)  pLatentFault2Notification;
    CONST(LatentFault3_NotifyType, FUNCSAFETYPKG_CONST)  pLatentFault3Notification;
} BistConfigType;

/*@brief the type of the configuration for wdt self test*/
typedef struct {
    uint8  OverFlowValue;
    CONST(WdtSelfTstOverFlow_NotifyType, FUNCSAFETYPKG_CONST)  pWdtSelfTestOverFlowResetUndetectNotification;
} WdtSelfTestConfigType;

/*@brief the type of the configuration for fail safe reference clock check*/
typedef struct {
    CONST(RcChkAp_NotifyType, FUNCSAFETYPKG_CONST)    pRcChkApNotification;
    CONST(SafeChkAp_NotifyType, FUNCSAFETYPKG_CONST)  pSafeChkApNotification;
} FsrefClkChkConfigType;

/*@brief the type of the configuration for sem interrupt*/
typedef struct {
    P2CONST(Sem_Int_Input, AUTOMATIC, FUNCSAFETYPKG_APPL_CONST) pSemIntOutput0;
    P2CONST(Sem_Int_Input, AUTOMATIC, FUNCSAFETYPKG_APPL_CONST) pSemIntOutput1;
    P2CONST(Sem_Int_Input, AUTOMATIC, FUNCSAFETYPKG_APPL_CONST) pSemIntOutput2;
    P2CONST(Sem_Int_Input, AUTOMATIC, FUNCSAFETYPKG_APPL_CONST) pSemIntOutput3;
    CONST(SemIntMonitor_NotifyType, FUNCSAFETYPKG_CONST)  pSemIntOutput2CpuNotification;
} SemIntMonConfigType;

/*@brief the type of the configuration for dual core lock step inject error*/
typedef struct {
    CONST(DualCoreLockStepInjectErr_NotifyType, FUNCSAFETYPKG_CONST)  pDualCoreLockStepCompFailNotDetectedNotification;
} DualCoreLockStepInjectErrConfigType;

/*@brief the type of the configuration for SOC temperature monitor test*/
typedef struct {
    CONST(SocTempMonitorTst_NotifyType, FUNCSAFETYPKG_CONST)  pSocTempMonitorOverHeatIntNotDetectedNotification;
} SocTempMonitorTstConfigType;

#ifdef __cplusplus
}
#endif

#endif
