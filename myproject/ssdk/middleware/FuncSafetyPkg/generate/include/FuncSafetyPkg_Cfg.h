/**
 * @file FuncSafetyPkg_Cfg.h
 * @brief Function Safety Configuration.
 * @details Function safety package
 * @date [!"$datetime"!]
 */

 /********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef FUNCSAFETYPKG_CFG_H
#define FUNCSAFETYPKG_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FuncSafetyPkg_Type.h"


[!NOCODE!][!//
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram2BitErrInjectPowerOn) = 'true'       or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true'))"!][!//
[!CODE!][!//
/*Iram inject error configuration*/
extern CONST(Iram_ZoneConfigType, FUNCSAFETYPKG_CONST) Iram_ZoneConfig[];

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM1BitErrInjectPowerOn) = 'true'  or            node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM2BitErrInjectPowerOn) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM1BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM2BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM1BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM2BitErrInjectPowerOn) = 'true'))"!][!//
[!CODE!][!//
/*TCM inject error configuration*/
extern CONST(TCMErrConfigType, FUNCSAFETYPKG_CONST) TCMErrConfig; 

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DCACHE1BitErrInjectPowerOn) = 'true')"!][!//
[!CODE!][!//
/*Data cache inject error configuration*/
extern CONST(DCACHEErrConfigType, FUNCSAFETYPKG_CONST) DcacheErrConfig;

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric2BitErrInjectPowerOn) = 'true'))"!][!//
[!CODE!][!//
/*Fabric inject error configuration*/
extern CONST(FabricErrConfigType, FUNCSAFETYPKG_CONST) FabricErrConfig; 

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse2BitErrInjectPowerOn) = 'true')) or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse1BitErrInjectPeriodic) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse2BitErrInjectPeriodic) = 'true'))"!][!//
[!CODE!][!//
/*Fuse error configuration*/
extern CONST(FuseErrConfigType, FUNCSAFETYPKG_CONST) FuseErrConfig;

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonitorPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonInjectErrPowerOn) = 'true'))"!][!//
[!CODE!][!//
/*Sem signal monitor configuration*/
extern CONST(SemSignalMonConfigType, FUNCSAFETYPKG_CONST) SemSigMonitorConfig; 

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/RunNocBISTPowerOn) = 'true')"!][!//
[!CODE!][!//
/*Run bist configuration*/
extern CONST(BistConfigType, FUNCSAFETYPKG_CONST) BistConfig;
[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/WdtSelfTestPowerOn) = 'true')"!][!//
[!CODE!][!//
/*Wdt self test over flow configuration*/
extern CONST(WdtSelfTestConfigType, FUNCSAFETYPKG_CONST) WdtSelfTestConfig;

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkRcChkAPPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkSafeChkApPowerOn) = 'true'))"!][!//
[!CODE!][!//
/*Fail safe reference clock checked(safe clk,ap clk,rc)*/
extern CONST(FsrefClkChkConfigType, FUNCSAFETYPKG_CONST) FsrefClkChkConfig;

[!ENDCODE!][!// 
[!ENDIF!][!//

[!CODE!][!//
/*Sem input interrupt monitor configuration*/
extern CONST(SemIntMonConfigType, FUNCSAFETYPKG_CONST) SemIntMonConfig; 

[!ENDCODE!][!// 

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DualCoreLockStepInjectErrorPowerOn) = 'true') or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodic) = 'true')"!][!//
[!CODE!][!//
/*Dual core lock step inject error configuration*/
extern CONST(DualCoreLockStepInjectErrConfigType, FUNCSAFETYPKG_CONST) DualCoreLockStepInjectErrConfig; 

[!ENDCODE!][!// 
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SocTempMonitorPowerOn) = 'true')"!][!//
[!CODE!][!//
/*SOC temperature monitor test configuration*/
extern CONST(SocTempMonitorTstConfigType, FUNCSAFETYPKG_CONST) SocTempMonitorTstConfig; 

[!ENDCODE!][!// 
[!ENDIF!][!//

[!FOR "x" = "0" TO "num:i(count(FuncSafetyPkgCommon/IramErrInjectConfigSet/*/*))"!][!//
[!SELECT "FuncSafetyPkgCommon/IramErrInjectConfigSet/IramZoneConfiguration/*/IramZoneId[.=$x]/.."!][!//
[!NOCODE!][!//
    [!IF "node:exists(IramOneBitErrInject/Iram_1BitErr_Un_CorrNotification)"!][!//
        [!VAR "Iram1BitUnCorrNotification" = "IramOneBitErrInject/Iram_1BitErr_Un_CorrNotification"!][!//
[!CODE!][!//

/*Iram 1bit injected error not corrected notifition function*/
extern void [!"$Iram1BitUnCorrNotification"!](uint32 ErrCode);  
[!ENDCODE!][!//
    [!ENDIF!][!//
    
    [!IF "node:exists(IramOneBitErrInject/Iram_1BitErr_Un_DetectNotification)"!][!//    
        [!VAR "Iram1BitUnDetectNotification" = "IramOneBitErrInject/Iram_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Iram 1bit injected error not detected notifition function*/
extern void [!"$Iram1BitUnDetectNotification"!](uint32 ErrCode);    
[!ENDCODE!][!//
    [!ENDIF!][!//
    
    [!IF "node:exists(IramTwoBitErrInject/Iram_2BitErr_Un_DetectNotification)"!][!//    
        [!VAR "Iram2BitUnDetectNotification" = "IramTwoBitErrInject/Iram_2BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Iram 2bit injected error not detected notifition function*/
extern void [!"$Iram2BitUnDetectNotification"!](uint32 ErrCode);    
[!ENDCODE!][!//
    [!ENDIF!][!//   
    
[!ENDNOCODE!][!//
[!ENDSELECT!][!//
[!ENDFOR!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMOneBitErrInject/ATCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "ATCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMOneBitErrInject/ATCM_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*ATCM 1bit injected error not detected notifition function*/
extern void [!"$ATCM1BitUnDetectNotification"!]();  
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMTwoBitErrInject/ATCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "ATCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMTwoBitErrInject/ATCM_2BitErr_Un_DetectNotification"!][!//
[!CODE!][!//
/*ATCM 2bit injected error not detected notifition function*/
extern void [!"$ATCM2BitUnDetectNotification"!]();  
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMOneBitErrInject/B0TCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "B0TCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMOneBitErrInject/B0TCM_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//
/*B0TCM 1bit injected error not detected notifition function*/
extern void [!"$B0TCM1BitUnDetectNotification"!](); 
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMTwoBitErrInject/B0TCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "B0TCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMTwoBitErrInject/B0TCM_2BitErr_Un_DetectNotification"!][!//
[!CODE!][!//
/*B0TCM 2bit injected error not detected notifition function*/
extern void [!"$B0TCM2BitUnDetectNotification"!](); 
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMOneBitErrInject/B1TCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "B1TCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMOneBitErrInject/B1TCM_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*B1TCM 1bit injected error not detected notifition function*/
extern void [!"$B1TCM1BitUnDetectNotification"!](); 
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMTwoBitErrInject/B1TCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "B1TCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMTwoBitErrInject/B1TCM_2BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*B1TCM 2bit injected error not detected notifition function*/
extern void [!"$B1TCM2BitUnDetectNotification"!](); 
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/DCACHE_Data_Ram_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "DcacheDataRam1BitUnDetectNotification" = "FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/DCACHE_Data_Ram_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Data cache data ram 1bit injected error not detected notifition function*/
extern void [!"$DcacheDataRam1BitUnDetectNotification"!](uint32 DcacheErrCode); 
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/DCACHE_Tag_Ram_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "DcacheTagRam1BitUnDetectNotification" = "FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/DCACHE_Tag_Ram_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Data cache tag ram 1bit injected error not detected notifition function*/
extern void [!"$DcacheTagRam1BitUnDetectNotification"!](uint32 DcacheErrCode);  
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricOneBitErrInject/Fabric_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "Fabric1BitUnDetectNotification" = "FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricOneBitErrInject/Fabric_1BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Fabric 1bit injected error not detected notifition function*/
extern void [!"$Fabric1BitUnDetectNotification"!](uint32 ErrCode);  
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//   

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricTwoBitErrInject/Fabric_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "Fabric2BitUnDetectNotification" = "FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricTwoBitErrInject/Fabric_2BitErr_Un_DetectNotification"!][!//
[!CODE!][!//

/*Fabric 2bit injected error not detected notifition function*/
extern void [!"$Fabric2BitUnDetectNotification"!](uint32 ErrCode);  
[!ENDCODE!][!//
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_ErrSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse1BitErrStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_ErrSta_Un_DetectNotification"!][!//    
[!CODE!][!//
/*Fuse 1bit injected error error status not detected notifition function*/
extern void [!"$Fuse1BitErrStaUnDetectNoticifation"!](uint32 ErrCode);  
[!ENDCODE!][!//
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_IntSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse1BitIntStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_IntSta_Un_DetectNotification"!][!//
[!CODE!][!//

/*Fuse 1bit injected error interrupt status not detected notifition function*/
extern void [!"$Fuse1BitIntStaUnDetectNoticifation"!](uint32 ErrCode);  
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_ErrSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse2BitErrStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_ErrSta_Un_DetectNotification"!][!//        
[!CODE!][!//

/*Fuse 2bit injected error error status not detected notifition function*/
extern void [!"$Fuse2BitErrStaUnDetectNoticifation"!](uint32 ErrCode);  
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//
    
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_IntSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse2BitIntStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_IntSta_Un_DetectNotification"!][!//        
[!CODE!][!//

/*Fuse 2bit injected error interrupt status not detected notifition function*/
extern void [!"$Fuse2BitIntStaUnDetectNoticifation"!](uint32 ErrCode);  
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//
    
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitor/Sem_SigMonitor_UnmatchNotification)"!][!//
        [!VAR "SemSignalUnMatchNotification" = "FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitor/Sem_SigMonitor_UnmatchNotification"!][!//       
[!CODE!][!//

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
extern void [!"$SemSignalUnMatchNotification"!](void);  
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitorInjectError/Sem_SigMon_InjErr_UndetectedNotification)"!][!//
        [!VAR "SemSignalInjErrUndetectedNotification" = "FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitorInjectError/Sem_SigMon_InjErr_UndetectedNotification"!][!//     
[!CODE!][!//

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
extern void [!"$SemSignalInjErrUndetectedNotification"!](void); 
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault0Notification)"!][!//
        [!VAR "LatentFault0Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault0Notification"!][!//     
[!CODE!][!//

/*Latent fault0 notifition function*/
extern void [!"$LatentFault0Notification"!](uint32 Noc_Type, uint32 FaultCode); 
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault1Notification)"!][!//
        [!VAR "LatentFault1Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault1Notification"!][!//     
[!CODE!][!//

/*Latent fault1 notifition function*/
extern void [!"$LatentFault1Notification"!](uint32 Noc_Type, uint32 FaultCode); 
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault2Notification)"!][!//
        [!VAR "LatentFault2Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault2Notification"!][!//     
[!CODE!][!//

/*Latent fault2 notifition function*/
extern void [!"$LatentFault2Notification"!](uint32 Noc_Type, uint32 FaultCode); 
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault3Notification)"!][!//
        [!VAR "LatentFault3Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault3Notification"!][!//     
[!CODE!][!//

/*Latent fault3 notifition function*/
extern void [!"$LatentFault3Notification"!](uint32 Noc_Type, uint32 FaultCode); 
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/WdtSelfTestConfigSet/WdtSelfTest/Wdt_SelfTst_OverFlow_Rst_UndetectNotification)"!][!//
        [!VAR "WdtSelfTstOverFlowNotification" = "FuncSafetyPkgCommon/WdtSelfTestConfigSet/WdtSelfTest/Wdt_SelfTst_OverFlow_Rst_UndetectNotification"!][!//     
[!CODE!][!//

/*Wdt self test over flow reset not detected notifition function*/
extern void [!"$WdtSelfTstOverFlowNotification"!](void);    
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkRcChkAPPowerOn) = 'true')"!][!//
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkRcChkApTest/RcChkAp_UnCheckNotification)"!][!//
        [!VAR "RcChkApUnCheckNotification" = "FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkRcChkApTest/RcChkAp_UnCheckNotification"!][!//       
[!CODE!][!//

/*Rc check Ap clk not checked notification function*/
extern void [!"$RcChkApUnCheckNotification"!](void);    
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkSafeChkApTest/SafeChkAp_UnCheckNotification)"!][!//
        [!VAR "SafeChkApUnCheckNotification" = "FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkSafeChkApTest/SafeChkAp_UnCheckNotification"!][!//     
[!CODE!][!//

/*Safe check ap clk not checked notification function*/
extern void [!"$SafeChkApUnCheckNotification"!](void);  
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemIntOutput2Cpu/SemIntOutput2CpuNotification)"!][!//
        [!VAR "SemMonitorInputIntNotification" = "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemIntOutput2Cpu/SemIntOutput2CpuNotification"!][!//     
[!CODE!][!//

/*Sem monitor input interrupt notification function*/
extern void [!"$SemMonitorInputIntNotification"!](uint32 SemInputStatus);   
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DualCoreLockStepInjectErrorPowerOn) = 'true') or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodic) = 'true')"!][!//
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/DualCoreLockStepInjectErrConfigSet/DualCoreLockStepInjectErr/DualCore_LockStep_CompareFail_NotDetectedNotification)"!][!//
        [!VAR "DualCoreLockStepCompFailNotDetectedNotification" = "FuncSafetyPkgCommon/DualCoreLockStepInjectErrConfigSet/DualCoreLockStepInjectErr/DualCore_LockStep_CompareFail_NotDetectedNotification"!][!//        
[!CODE!][!//

/*Dual core lock step compare failed not detected notification function*/
extern void [!"$DualCoreLockStepCompFailNotDetectedNotification"!](void);   
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SocTempMonitorPowerOn) = 'true')"!][!//
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/SocTempMonitorTstConfigSet/SocTempMonitorTst/SocTempOverHeatInt_NotDetectedNotification)"!][!//
[!VAR "SocTempOverHeatIntNotDetectedNotification" = "FuncSafetyPkgCommon/SocTempMonitorTstConfigSet/SocTempMonitorTst/SocTempOverHeatInt_NotDetectedNotification"!][!//     
[!CODE!][!//

/*SOC temperature over heat interrupt not detected notification function*/
extern void [!"$SocTempOverHeatIntNotDetectedNotification"!](void);
[!ENDCODE!][!//     
    [!ENDIF!][!//   
[!ENDNOCODE!][!//
[!ENDIF!][!//   
[!ENDNOCODE!][!//   

/**
 * @brief Switches self test on or off when ECU powers on
 */
#define FUSAPKG_SELF_TEST_POWER_ON                          ([!IF "FuncSafetyPkgCommon/SelfTestPowerOn"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches self test on or off periodic
 */
#define FUSAPKG_SELF_TEST_PERIOD                            ([!IF "FuncSafetyPkgCommon/SelfTestPeriodic"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])
    
/**
 * @brief Switches inject 1bit error in iram1 on or off when ECU powers on
 */
#define FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error in iram1 on or off when ECU powers on
 */
#define FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error in ATCM on or off when ECU powers on
 */
#define FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error in ATCM on or off when ECU powers on
 */
#define FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error in B0TCM on or off when ECU powers on
 */
#define FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON            ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error in B0TCM on or off when ECU powers on
 */
#define FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON            ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error in B1TCM on or off when ECU powers on
 */
#define FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON            ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error in B1TCM on or off when ECU powers on
 */
#define FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON            ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error in DCACHE on or off when ECU powers on
 */
#define FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON           ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DCACHE1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error on fabric on or off when ECU powers on
 */
#define FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON           ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error on fabric on or off when ECU powers on
 */
#define FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON           ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error on fuse on or off when ECU powers on
 */
#define FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse1BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error on fuse on or off when ECU powers on
 */
#define FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse2BitErrInjectPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches Rc clock check Ap clock on or off when ECU powers on
 */
#define FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON                 ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkRcChkAPPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches Safe clock check Ap clock on or off when ECU powers on
 */
#define FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON               ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkSafeChkApPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches run BIST on or off when ECU power on
 */
#define FUSAPKG_RUN_BIST_POWER_ON                           ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/RunNocBISTPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches WDT self test for overflow reset request on or off when ECU powers on
 */
#define FUSAPKG_WDT_SELFTEST_POWER_ON                       ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/WdtSelfTestPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches SEM signal monitor on or off when ECU power on
 */
#define FUSAPKG_SEM_SIGMONITOR_POWER_ON                       ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonitorPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches SEM signal monitor inject error on or off when ECU powers on
 */
#define FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON                       ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonInjectErrPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches Dual core lock step on or off when ECU powers on
 */
#define FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DualCoreLockStepInjectErrorPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches SOC temperature monitor test on or off when ECU powers on
 */
#define FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON             ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DualCoreLockStepInjectErrorPowerOn) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])
        
/**
 * @brief Switches inject 1bit error in iram1 on or off period
 */
#define FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD               ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error in iram1 on or off period
 */
#define FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD               ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 1bit error on fuse on or off period
 */
#define FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD               ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse1BitErrInjectPeriodic) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches inject 2bit error on fuse on or off period
 */
#define FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD               ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse2BitErrInjectPeriodic) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

/**
 * @brief Switches dual_core lock step inject error on or off period
 */
#define FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD                ([!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodic) = 'true'"!]STD_ON[!ELSE!]STD_OFF[!ENDIF!])

[!NOCODE!][!// 
[!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true')"!][!// 
    [!IF "node:exists(FuncSafetyPkgCommon/SelfTestPeriodicOptions/IramErrInjectPeriodCnt)"!][!//
        [!VAR "IramErrInjectPeriodCntTemp" = "num:i(FuncSafetyPkgCommon/SelfTestPeriodicOptions/IramErrInjectPeriodCnt)"!][!//
    [!ELSE!][!//
        [!VAR "IramErrInjectPeriodCntTemp" = "1"!][!//
    [!ENDIF!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting 1/2 bits error in Iram periodically.
 */
#define IRAM_ERROR_INJECT_PERIOD_CNT  ([!"num:i($IramErrInjectPeriodCntTemp)"!]) 
[!ENDCODE!][!// 
[!ELSE!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting 1/2 bits error in Iram periodically.
 */
#define IRAM_ERROR_INJECT_PERIOD_CNT  1
[!ENDCODE!][!// 
[!ENDIF!][!//
[!ENDNOCODE!][!// 

[!NOCODE!][!// 
[!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse1BitErrInjectPeriodic) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse2BitErrInjectPeriodic) = 'true')"!][!//
[!NOCODE!][!// 
    [!IF "node:exists(FuncSafetyPkgCommon/SelfTestPeriodicOptions/FuseErrInjectPeriodCnt)"!][!//
        [!VAR "EfuseErrInjectPeriodCntTemp" = "num:i(FuncSafetyPkgCommon/SelfTestPeriodicOptions/FuseErrInjectPeriodCnt)"!][!//
    [!ELSE!][!//
        [!VAR "EfuseErrInjectPeriodCntTemp" = "2"!][!//
    [!ENDIF!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting 1/2 bits error in Efuse periodically.
 */
#define FUSE_ERROR_INJECT_PERIOD_CNT  ([!"num:i($EfuseErrInjectPeriodCntTemp)"!]) 
[!ENDCODE!][!// 
[!ENDNOCODE!][!// 
[!ELSE!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting 1/2 bits error in Efuse periodically.
 */
#define FUSE_ERROR_INJECT_PERIOD_CNT  2
[!ENDCODE!][!// 
[!ENDIF!][!//
[!ENDNOCODE!][!// 

[!NOCODE!][!//
[!IF "node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodic) = 'true'"!][!//
[!NOCODE!][!// 
    [!IF "node:exists(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodCnt)"!][!//
        [!VAR "DualcorelockstepPeriodCntTemp" = "num:i(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodCnt)"!][!//
    [!ELSE!][!//
        [!VAR "DualcorelockstepPeriodCntTemp" = "3"!][!//
    [!ENDIF!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting error in dual core lock step periodically.
 */
#define DUALCORE_LOCKSTEP_ERROR_INJECT_PERIOD_CNT  ([!"num:i($DualcorelockstepPeriodCntTemp)"!])[!// 
[!ENDCODE!][!// 
[!ENDNOCODE!][!// 
[!ELSE!][!//
[!CODE!][!// 
/**
 * @brief The period count for injecting error in dual core lock step periodically.
 */
#define DUALCORE_LOCKSTEP_ERROR_INJECT_PERIOD_CNT  3
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!// 

[!NOCODE!][!// 
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram1BitErrInjectPowerOn) = 'true') or        (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true')"!][!//
[!CODE!][!//
/**
 * @brief Inject 1bit error in iram mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define IRAM_1BIT_ERROR_MODE                        ([!"FuncSafetyPkgCommon/IramErrInjectConfigSet/IramInject1BitErrMode"!])
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!// 

[!NOCODE!][!// 
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram2BitErrInjectPowerOn) = 'true') or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true')"!][!//
[!CODE!][!//
/**
 * @brief Inject 2bit error in iram mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define IRAM_2BIT_ERROR_MODE                        ([!"FuncSafetyPkgCommon/IramErrInjectConfigSet/IramInject2BitErrMode"!])
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!// 

[!NOCODE!][!// 
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric2BitErrInjectPowerOn) = 'true'))"!][!//
[!CODE!][!//
/**
 * @brief Inject 1/2bit error in fabric mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define FABRIC_INJECT_ERROR_MODE                    ([!"FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricInjectErrMode"!])
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!// 
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse2BitErrInjectPowerOn) = 'true')) or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse1BitErrInjectPeriodic) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse2BitErrInjectPeriodic) = 'true'))"!][!//
[!CODE!][!//
/**
 * @brief Inject 1/2bit error in fuse mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define FUSE_INJECT_ERROR_MODE                      ([!"FuncSafetyPkgCommon/FuseConfigSet/FuseInjectErrMode"!])
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram2BitErrInjectPowerOn) = 'true')) or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true'))"!][!//
[!CODE!][!//
/**
 * @brief Injecting errors in several IRAM
 */
#define IRAM_ERR_NUM                                ([!"num:i(count(FuncSafetyPkgCommon/IramErrInjectConfigSet/*/*))"!]U)
[!ENDCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "i" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut0Value" = "num:i(./SemIntOut0)"!][!//
    [!IF "num:i($SemIntOut0Value) < 319"!][!//
    [!VAR "i" = "$i+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
[!CODE!][!//    
/**
 * @brief The number of SEM interrupt input for SEM interrupt output0
 */
#define SEM_INT_OUT0_NUM    [!"num:i($i)"!]
[!ENDCODE!][!// 
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "j" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut1Value" = "num:i(./SemIntOut1)"!][!//
    [!IF "num:i($SemIntOut1Value) < 319"!][!//
    [!VAR "j" = "$j+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
[!CODE!][!//
/**
 * @brief The number of SEM interrupt input for SEM interrupt output1
 */
#define SEM_INT_OUT1_NUM    [!"num:i($j)"!]
[!ENDCODE!][!// 
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "k" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut2Value" = "num:i(./SemIntOut2)"!][!//
    [!IF "num:i($SemIntOut2Value) < 319"!][!//
    [!VAR "k" = "$k+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
[!CODE!][!//
/**
 * @brief The number of SEM interrupt input for SEM interrupt output2
 */
#define SEM_INT_OUT2_NUM    [!"num:i($k)"!]
[!ENDCODE!][!// 
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "l" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut3Value" = "num:i(./SemIntOut3)"!][!//
    [!IF "num:i($SemIntOut3Value) < 319"!][!//
    [!VAR "l" = "$l+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
[!CODE!][!//
/**
 * @brief The number of SEM interrupt input for SEM interrupt output2
 */
#define SEM_INT_OUT3_NUM    [!"num:i($l)"!]
[!ENDCODE!][!// 
[!ENDNOCODE!][!//

#ifdef __cplusplus
}
#endif

#endif 
