/**
 * @file FuncSafetyPkg_Cfg.c
 * @brief Function Safety Configuration.
 * @details Function safety package
 * @date [!"$datetime"!]
 */

 /********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#include "FuncSafetyPkg.h"


[!NOCODE!][!//
[!IF "((node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Iram2BitErrInjectPowerOn) = 'true'))       or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram1BitErrInjectPeriodic) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Iram2BitErrInjectPeriodic) = 'true')))"!][!//
[!CODE!][!//

/*Configuration for injecting 1/2bits error in Iram*/
CONST(Iram_ZoneConfigType, FUNCSAFETYPKG_CONST) Iram_ZoneConfig[[!"num:i(count(FuncSafetyPkgCommon/IramErrInjectConfigSet/*/*))"!]] = {
[!ENDCODE!][!// 
[!FOR "x" = "0" TO "num:i(count(FuncSafetyPkgCommon/IramErrInjectConfigSet/*/*))"!][!//
[!SELECT "FuncSafetyPkgCommon/IramErrInjectConfigSet/IramZoneConfiguration/*/IramZoneId[.=$x]/.."!][!//
[!NOCODE!][!//
    [!VAR "IramZoneNm" = "IramInjectErrZone"!][!//
    [!IF "node:exists(IramOneBitErrInject/Iram_1BitErr_Un_CorrNotification)"!][!//
        [!VAR "Iram1BitUnCorrNotification" = "IramOneBitErrInject/Iram_1BitErr_Un_CorrNotification"!][!//
    [!ELSE!][!//
        [!VAR "Iram1BitUnCorrNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(IramOneBitErrInject/Iram_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "Iram1BitUnDetectNotification" = "IramOneBitErrInject/Iram_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Iram1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(IramTwoBitErrInject/Iram_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "Iram2BitUnDetectNotification" = "IramTwoBitErrInject/Iram_2BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Iram2BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//       
[!CODE!][!//
  { 
    (Iram_ZoneIdType)([!"num:i(IramZoneId)"!]), /* The unique identifier of the IRAM zone*/
    (Iram_ErrZoneType)([!"$IramZoneNm"!]), /*The unique name for every iram:IRAM1 IRAM2 IRAM3 IRAM4*/
    ([!"$Iram1BitUnCorrNotification"!]),
    ([!"$Iram1BitUnDetectNotification"!]),   
    ([!"$Iram2BitUnDetectNotification"!])
  },
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDSELECT!][!//
[!ENDFOR!][!//
[!CODE!][!//
};
[!ENDCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM1BitErrInjectPowerOn) = 'true'  or            node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/ATCM2BitErrInjectPowerOn) = 'true'  or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM1BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B0TCM2BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM1BitErrInjectPowerOn) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/B1TCM2BitErrInjectPowerOn) = 'true'))"!][!//
[!CODE!][!//

/*TCM inject error configuration*/
CONST(TCMErrConfigType, FUNCSAFETYPKG_CONST) TCMErrConfig = {   
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMOneBitErrInject/ATCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "ATCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMOneBitErrInject/ATCM_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "ATCM1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMTwoBitErrInject/ATCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "ATCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/ATCMTwoBitErrInject/ATCM_2BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "ATCM2BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMOneBitErrInject/B0TCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "B0TCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMOneBitErrInject/B0TCM_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "B0TCM1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMTwoBitErrInject/B0TCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "B0TCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B0TCMTwoBitErrInject/B0TCM_2BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "B0TCM2BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMOneBitErrInject/B1TCM_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "B1TCM1BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMOneBitErrInject/B1TCM_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "B1TCM1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMTwoBitErrInject/B1TCM_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "B1TCM2BitUnDetectNotification" = "FuncSafetyPkgCommon/TCMErrInjectConfigSet/B1TCMTwoBitErrInject/B1TCM_2BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "B1TCM2BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//       
[!CODE!][!//
    ([!"$ATCM1BitUnDetectNotification"!]),  
    ([!"$ATCM2BitUnDetectNotification"!]),
    ([!"$B0TCM1BitUnDetectNotification"!]),    
    ([!"$B0TCM2BitUnDetectNotification"!]),
    ([!"$B1TCM1BitUnDetectNotification"!]),
    ([!"$B1TCM2BitUnDetectNotification"!])
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DCACHE1BitErrInjectPowerOn) = 'true'"!][!//
[!CODE!][!//

/*Data cache data ram inject error configuration*/
CONST(DCACHEDataRamErrConfigType, FUNCSAFETYPKG_CONST) DcacheDataRamErrConfig = {   
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/DCACHE_Data_Ram_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "DcacheDataRam1BitUnDetectNotification" = "FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/DCACHE_Data_Ram_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "DcacheDataRam1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//       
[!CODE!][!//
    ([!"num:i(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/BankNumber)"!]U),   
    ([!"num:i(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/DataRamOfDCACHE/RamIndex)"!]U),    
    ([!"$DcacheDataRam1BitUnDetectNotification"!]) 
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//

[!CODE!][!//

/*Data cache tag ram inject error configuration*/
CONST(DCACHETagRamErrConfigType, FUNCSAFETYPKG_CONST) DcacheTagRamErrConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/DCACHE_Tag_Ram_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "DcacheTagRam1BitUnDetectNotification" = "FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/DCACHE_Tag_Ram_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "DcacheTagRam1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//       
[!CODE!][!//
    ([!"num:i(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/BankNumber)"!]U),    
    ([!"num:i(FuncSafetyPkgCommon/CACHEErrInjectConfigSet/DCACHEOneBitErrInject/TagRamOfDCACHE/RamIndex)"!]U),    
    ([!"$DcacheTagRam1BitUnDetectNotification"!])
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//

[!CODE!][!//

/*Data cache inject error configuration*/
CONST(DCACHEErrConfigType, FUNCSAFETYPKG_CONST) DcacheErrConfig = {             
    &DcacheDataRamErrConfig,  
    &DcacheTagRamErrConfig
};  
[!ENDCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fabric2BitErrInjectPowerOn) = 'true'))"!][!//
[!CODE!][!//

/*Fabric inject error configuration*/
CONST(FabricErrConfigType, FUNCSAFETYPKG_CONST) FabricErrConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricOneBitErrInject/Fabric_1BitErr_Un_DetectNotification)"!][!//
        [!VAR "Fabric1BitUnDetectNotification" = "FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricOneBitErrInject/Fabric_1BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fabric1BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricTwoBitErrInject/Fabric_2BitErr_Un_DetectNotification)"!][!//
        [!VAR "Fabric2BitUnDetectNotification" = "FuncSafetyPkgCommon/FabricErrInjectConfigSet/FabricTwoBitErrInject/Fabric_2BitErr_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fabric2BitUnDetectNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$Fabric1BitUnDetectNotification"!]),  
    ([!"$Fabric2BitUnDetectNotification"!])   
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse1BitErrInjectPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/Fuse2BitErrInjectPowerOn) = 'true')) or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse1BitErrInjectPeriodic) = 'true' or node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/Fuse2BitErrInjectPeriodic) = 'true'))"!][!//
[!CODE!][!//

/*Fuse error configuration*/
CONST(FuseErrConfigType, FUNCSAFETYPKG_CONST) FuseErrConfig = {
[!ENDCODE!][!//
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_ErrSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse1BitErrStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_ErrSta_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fuse1BitErrStaUnDetectNoticifation" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_IntSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse1BitIntStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseOneBitErrInject/Fuse_1BitErr_IntSta_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fuse1BitIntStaUnDetectNoticifation" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_ErrSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse2BitErrStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_ErrSta_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fuse2BitErrStaUnDetectNoticifation" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_IntSta_Un_DetectNotification)"!][!//
        [!VAR "Fuse2BitIntStaUnDetectNoticifation" = "FuncSafetyPkgCommon/FuseConfigSet/FuseTwoBitErrInject/Fuse_2BitErr_IntSta_Un_DetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "Fuse2BitIntStaUnDetectNoticifation" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//       
[!CODE!][!//
    ([!"$Fuse1BitErrStaUnDetectNoticifation"!]),  
    ([!"$Fuse1BitIntStaUnDetectNoticifation"!]),
    ([!"$Fuse2BitErrStaUnDetectNoticifation"!]),
    ([!"$Fuse2BitIntStaUnDetectNoticifation"!]),
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonitorPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SemSigMonInjectErrPowerOn) = 'true'))"!][!//
[!CODE!][!//

/*Sem signal monitor configuration*/
CONST(SemSignalMonConfigType, FUNCSAFETYPKG_CONST) SemSigMonitorConfig = {  
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitor/Sem_SigMonitor_UnmatchNotification)"!][!//
        [!VAR "SemSignalUnMatchNotification" = "FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitor/Sem_SigMonitor_UnmatchNotification"!][!//
    [!ELSE!][!//
        [!VAR "SemSignalUnMatchNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitorInjectError/Sem_SigMon_InjErr_UndetectedNotification)"!][!//
        [!VAR "SemSignalInjErrUndetectedNotification" = "FuncSafetyPkgCommon/SemSigMonConfigSet/SemSignalMonitorInjectError/Sem_SigMon_InjErr_UndetectedNotification"!][!//
    [!ELSE!][!//
        [!VAR "SemSignalInjErrUndetectedNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$SemSignalUnMatchNotification"!]),      
    ([!"$SemSignalInjErrUndetectedNotification"!]), 
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/RunNocBISTPowerOn) = 'true')"!][!//
[!CODE!][!//

/*Run bist configuration*/
CONST(BistConfigType, FUNCSAFETYPKG_CONST) BistConfig = {   
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault0Notification)"!][!//
        [!VAR "LatentFault0Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault0Notification"!][!//
    [!ELSE!][!//
        [!VAR "LatentFault0Notification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault1Notification)"!][!//
        [!VAR "LatentFault1Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault1Notification"!][!//
    [!ELSE!][!//
        [!VAR "LatentFault1Notification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault2Notification)"!][!//
        [!VAR "LatentFault2Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault2Notification"!][!//
    [!ELSE!][!//
        [!VAR "LatentFault2Notification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault3Notification)"!][!//
        [!VAR "LatentFault3Notification" = "FuncSafetyPkgCommon/NocBistConfigSet/Run_Noc_Bist_Sequence/Latent_Fault3Notification"!][!//
    [!ELSE!][!//
        [!VAR "LatentFault3Notification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$LatentFault0Notification"!]),
    ([!"$LatentFault1Notification"!]),
    ([!"$LatentFault2Notification"!]),  
    ([!"$LatentFault3Notification"!])
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/WdtSelfTestPowerOn) = 'true')"!][!//
[!CODE!][!//

/*Wdt self test over flow configuration*/
CONST(WdtSelfTestConfigType, FUNCSAFETYPKG_CONST) WdtSelfTestConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/WdtSelfTestConfigSet/WdtSelfTest/Wdt_SelfTst_OverFlow_Rst_UndetectNotification)"!][!//
        [!VAR "WdtSelfTstOverFlowNotification" = "FuncSafetyPkgCommon/WdtSelfTestConfigSet/WdtSelfTest/Wdt_SelfTst_OverFlow_Rst_UndetectNotification"!][!//
    [!ELSE!][!//
        [!VAR "WdtSelfTstOverFlowNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"num:inttohex(num:i(FuncSafetyPkgCommon/WdtSelfTestConfigSet/WdtSelfTest/OverFlowValue))"!]),  
    ([!"$WdtSelfTstOverFlowNotification"!])  
};    
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and (node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkRcChkAPPowerOn) = 'true' or         node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/FSRefClkSafeChkApPowerOn) = 'true'))"!][!//
[!CODE!][!//

/*Fail safe reference clock checked(safe clk,ap clk,rc)*/
CONST(FsrefClkChkConfigType, FUNCSAFETYPKG_CONST) FsrefClkChkConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//  
    [!IF "node:exists(FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkRcChkApTest/RcChkAp_UnCheckNotification)"!][!//
        [!VAR "RcChkApUnCheckNotification" = "FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkRcChkApTest/RcChkAp_UnCheckNotification"!][!//
    [!ELSE!][!//
        [!VAR "RcChkApUnCheckNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkSafeChkApTest/SafeChkAp_UnCheckNotification)"!][!//
        [!VAR "SafeChkApUnCheckNotification" = "FuncSafetyPkgCommon/FsrefclkchkTestConfigSet/FsRefClkSafeChkApTest/SafeChkAp_UnCheckNotification"!][!//
    [!ELSE!][!//
        [!VAR "SafeChkApUnCheckNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$RcChkApUnCheckNotification"!]),
    ([!"$SafeChkApUnCheckNotification"!])   
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!NOCODE!][!//
    [!VAR "i" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut0Value" = "num:i(./SemIntOut0)"!][!//
    [!IF "num:i($SemIntOut0Value) < 319"!][!//
    [!VAR "i" = "$i+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
    [!IF "num:i($i) > 0"!][!//
[!CODE!][!//

/*Sem input interrupt to combine with SemIntOut1 to generate PAD interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut0[SEM_INT_OUT0_NUM] = {[!//
[!ENDCODE!][!// 
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut0Value" = "num:i(./SemIntOut0)"!][!//
    [!IF "num:i($SemIntOut0Value) < 319"!][!//
[!CODE!][!//    
    (Sem_Int_Input)([!"num:i($SemIntOut0Value)"!]),[!//
[!ENDCODE!][!//     
    [!ENDIF!][!//
    [!ENDLOOP!][!// 
[!CODE!][!//    
};
[!ENDCODE!][!// 
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "j" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut1Value" = "num:i(./SemIntOut1)"!][!//
    [!IF "num:i($SemIntOut1Value) < 319"!][!//
    [!VAR "j" = "$j+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
    [!IF "num:i($j) > 0"!][!//
[!CODE!][!//

/*Sem input interrupt to combine with SemIntOut0 to generate PAD interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut1[SEM_INT_OUT1_NUM] = {[!//
[!ENDCODE!][!// 
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut1Value" = "num:i(./SemIntOut1)"!][!//
    [!IF "num:i($SemIntOut1Value) < 319"!][!//
[!CODE!][!//    
    (Sem_Int_Input)([!"num:i($SemIntOut1Value)"!]),[!//
[!ENDCODE!][!//     
    [!ENDIF!][!//
    [!ENDLOOP!][!// 
[!CODE!][!//    
};
[!ENDCODE!][!// 
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "k" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut2Value" = "num:i(./SemIntOut2)"!][!//
    [!IF "num:i($SemIntOut2Value) < 319"!][!//
    [!VAR "k" = "$k+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
    [!IF "num:i($k) > 0"!][!//
[!CODE!][!//

/*Sem input interrupt to generate CPU interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut2[SEM_INT_OUT2_NUM] = {[!//
[!ENDCODE!][!// 
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut2Value" = "num:i(./SemIntOut2)"!][!//
    [!IF "num:i($SemIntOut2Value) < 319"!][!//
[!CODE!][!//    
    (Sem_Int_Input)([!"num:i($SemIntOut2Value)"!]),[!//
[!ENDCODE!][!//     
    [!ENDIF!][!//
    [!ENDLOOP!][!// 
[!CODE!][!//    
};
[!ENDCODE!][!// 
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!NOCODE!][!//
    [!VAR "l" = "0"!][!//   
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut3Value" = "num:i(./SemIntOut3)"!][!//
    [!IF "num:i($SemIntOut3Value) < 319"!][!//
    [!VAR "l" = "$l+1"!][!//
    [!ENDIF!][!//
    [!ENDLOOP!][!//
    [!IF "num:i($l) > 0"!][!//
[!CODE!][!//

/*Sem input interrupt to generate reset*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut3[SEM_INT_OUT3_NUM] = {[!//
[!ENDCODE!][!// 
    [!LOOP "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemInterruptMonitorConfiguration/*"!][!//
    [!VAR "SemIntOut3Value" = "num:i(./SemIntOut3)"!][!//
    [!IF "num:i($SemIntOut3Value) < 319"!][!//
[!CODE!][!//    
    (Sem_Int_Input)([!"num:i($SemIntOut3Value)"!]),[!//
[!ENDCODE!][!//     
    [!ENDIF!][!//
    [!ENDLOOP!][!// 
[!CODE!][!//    
};
[!ENDCODE!][!// 
    [!ENDIF!][!//
[!ENDNOCODE!][!//

[!CODE!][!//

/*Sem input interrupt monitor configuration*/
CONST(SemIntMonConfigType, FUNCSAFETYPKG_CONST) SemIntMonConfig = { 
[!ENDCODE!][!// 
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemIntOutput2Cpu/SemIntOutput2CpuNotification)"!][!//
        [!VAR "SemInputIntNotification" = "FuncSafetyPkgCommon/SemInterruptMonitorConfigSet/SemIntOutput2Cpu/SemIntOutput2CpuNotification"!][!//
    [!ELSE!][!//
        [!VAR "SemInputIntNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
[!IF "num:i($i) > 0"!][!//
    SemIntOut0,
[!ELSE!][!//
    NULL_PTR,
[!ENDIF!][!//
[!IF "num:i($j) > 0"!][!//
    SemIntOut1,
[!ELSE!][!//
    NULL_PTR,
[!ENDIF!][!//
[!IF "num:i($k) > 0"!][!//
    SemIntOut2,
[!ELSE!][!//
    NULL_PTR,
[!ENDIF!][!//
[!IF "num:i($l) > 0"!][!//
    SemIntOut3,
[!ELSE!][!//
    NULL_PTR,
[!ENDIF!][!//
    ([!"$SemInputIntNotification"!])      
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//


[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/DualCoreLockStepInjectErrorPowerOn) = 'true') or (node:value(FuncSafetyPkgCommon/SelfTestPeriodic) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPeriodicOptions/DualCoreLockStepInjectErrorPeriodic) = 'true')"!][!//
[!CODE!][!//

/*Dual core lock step inject error configuration*/
CONST(DualCoreLockStepInjectErrConfigType, FUNCSAFETYPKG_CONST) DualCoreLockStepInjectErrConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/DualCoreLockStepInjectErrConfigSet/DualCoreLockStepInjectErr/DualCore_LockStep_CompareFail_NotDetectedNotification)"!][!//
        [!VAR "DualCoreLockStepCompFailNotDetectedNotification" = "FuncSafetyPkgCommon/DualCoreLockStepInjectErrConfigSet/DualCoreLockStepInjectErr/DualCore_LockStep_CompareFail_NotDetectedNotification"!][!//
    [!ELSE!][!//
        [!VAR "DualCoreLockStepCompFailNotDetectedNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$DualCoreLockStepCompFailNotDetectedNotification"!])   
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//

[!IF "(node:value(FuncSafetyPkgCommon/SelfTestPowerOn) = 'true' and node:value(FuncSafetyPkgCommon/SelfTestPowerOnOptions/SocTempMonitorPowerOn) = 'true')"!][!//
[!CODE!][!//

/*SOC temperature monitor test configuration*/
CONST(SocTempMonitorTstConfigType, FUNCSAFETYPKG_CONST) SocTempMonitorTstConfig = { 
[!ENDCODE!][!//
[!NOCODE!][!//
    [!IF "node:exists(FuncSafetyPkgCommon/SocTempMonitorTstConfigSet/SocTempMonitorTst/SocTempOverHeatInt_NotDetectedNotification)"!][!//
        [!VAR "SocTempOverHeatIntNotDetectedNotification" = "FuncSafetyPkgCommon/SocTempMonitorTstConfigSet/SocTempMonitorTst/SocTempOverHeatInt_NotDetectedNotification"!][!//
    [!ELSE!][!//
        [!VAR "SocTempOverHeatIntNotDetectedNotification" = "'NULL_PTR'"!][!//
    [!ENDIF!][!//   
[!CODE!][!//
    ([!"$SocTempOverHeatIntNotDetectedNotification"!]) 
};  
[!ENDCODE!][!//
[!ENDNOCODE!][!//
[!ENDIF!][!//
[!ENDNOCODE!][!//
