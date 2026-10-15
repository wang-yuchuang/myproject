/**
 * @file FuncSafetyPkg_Cfg.c
 * @brief Function Safety Configuration.
 * @details Function safety package
 * @date 2021-07-06 18:26:23
 */

 /********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "FuncSafetyPkg.h"
#include <cmsis_os2.h>
#include "Platform_Types.h"
#include "Iram_Ip.h"


/*Configuration for injecting 1/2bits error in Iram*/
CONST(Iram_ZoneConfigType, FUNCSAFETYPKG_CONST) Iram_ZoneConfig[1] = {
  {
	(Iram_ZoneIdType)(0), /* The unique identifier of the IRAM zone*/
	(Iram_ErrZoneType)(IRAM1ERR), /*The unique name for every iram:IRAM1 IRAM2 IRAM3 IRAM4*/
	(Iram_1BitErr_Un_CorrNotification),
	(Iram_1BitErr_Un_DetectNotification),
	(Iram_2BitErr_Un_DetectNotification)
  },
};

/*TCM inject error configuration*/
CONST(TCMErrConfigType, FUNCSAFETYPKG_CONST) TCMErrConfig = {
	(ATCM_1BitErr_Un_DetectNotification),
	(ATCM_2BitErr_Un_DetectNotification),
    (B0TCM_1BitErr_Un_DetectNotification),
    (B0TCM_2BitErr_Un_DetectNotification),
    (B1TCM_1BitErr_Un_DetectNotification),
    (B1TCM_2BitErr_Un_DetectNotification)
};

/*Data cache data ram inject error configuration*/
CONST(DCACHEDataRamErrConfigType, FUNCSAFETYPKG_CONST) DcacheDataRamErrConfig = {
    (5U),
    (400U),
	(DCACHE_Data_Ram_1BitErr_Un_DetectNotification)
};

/*Data cache tag ram inject error configuration*/
CONST(DCACHETagRamErrConfigType, FUNCSAFETYPKG_CONST) DcacheTagRamErrConfig = {
    (0U),
    (100U),
	(DCACHE_Tag_Ram_1BitErr_Un_DetectNotification)
};

/*Data cache inject error configuration*/
CONST(DCACHEErrConfigType, FUNCSAFETYPKG_CONST) DcacheErrConfig = {
	&DcacheDataRamErrConfig,
	&DcacheTagRamErrConfig
};

/*Fabric inject error configuration*/
CONST(FabricErrConfigType, FUNCSAFETYPKG_CONST) FabricErrConfig = {
	(Fabric_1BitErr_Un_DetectNotification),
	(Fabric_2BitErr_Un_DetectNotification)
};

/*Fuse error configuration*/
CONST(FuseErrConfigType, FUNCSAFETYPKG_CONST) FuseErrConfig = {
	(Fuse_1BitErr_ErrSta_Un_DetectNotification),
	(Fuse_1BitErr_IntSta_Un_DetectNotification),
	(Fuse_2BitErr_ErrSta_Un_DetectNotification),
	(Fuse_2BitErr_IntSta_Un_DetectNotification),
};

/*Sem signal monitor configuration*/
CONST(SemSignalMonConfigType, FUNCSAFETYPKG_CONST) SemSigMonitorConfig = {
	(Sem_SigMonitor_UnmatchNotification),
	(Sem_SigMon_InjErr_UndetectedNotification),
};

/*Run bist configuration*/
CONST(BistConfigType, FUNCSAFETYPKG_CONST) BistConfig = {
	(Latent_Fault0Notification),
	(Latent_Fault1Notification),
	(Latent_Fault2Notification),
	(Latent_Fault3Notification)
};

/*Wdt self test over flow configuration*/
CONST(WdtSelfTestConfigType, FUNCSAFETYPKG_CONST) WdtSelfTestConfig = {
	(0xa),
	(Wdt_SelfTst_OverFlow_Rst_UndetectNotification)
};

/*Fail safe reference clock checked(safe clk,ap clk,rc)*/
CONST(FsrefClkChkConfigType, FUNCSAFETYPKG_CONST) FsrefClkChkConfig = {
	(RcChkAp_UnCheckNotification),
	(SafeChkAp_UnCheckNotification)
};

/*Sem input interrupt to combine with SemIntOut0 to generate PAD interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut0[SEM_INT_OUT0_NUM] = {
	(Sem_Int_Input)(0),
	(Sem_Int_Input)(2),
	(Sem_Int_Input)(4),
	(Sem_Int_Input)(6),
};

/*Sem input interrupt to combine with SemIntOut0 to generate PAD interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut1[SEM_INT_OUT1_NUM] = {
	(Sem_Int_Input)(1),
	(Sem_Int_Input)(3),
	(Sem_Int_Input)(74),
};

/*Sem input interrupt to generate CPU interrupt*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut2[SEM_INT_OUT2_NUM] = {
	(Sem_Int_Input)(0),     // 0
	(Sem_Int_Input)(2),     // 1
	(Sem_Int_Input)(154),   // 2
	(Sem_Int_Input)(155),   // 3
};

/*Sem input interrupt to generate reset*/
CONST(Sem_Int_Input, FUNCSAFETYPKG_CONST) SemIntOut3[SEM_INT_OUT3_NUM] = {
	(Sem_Int_Input)(3),
};

/*Sem input interrupt monitor configuration*/
CONST(SemIntMonConfigType, FUNCSAFETYPKG_CONST) SemIntMonConfig = {
	SemIntOut0,
	SemIntOut1,
	SemIntOut2,
	SemIntOut3,
	(SemIntOutput2CpuNotification)
};

/*Dual core lock step inject error configuration*/
CONST(DualCoreLockStepInjectErrConfigType, FUNCSAFETYPKG_CONST) DualCoreLockStepInjectErrConfig = {
	(DualCore_LockStep_CompareFail_NotDetectedNotification)
};

/*SOC temperature monitor test configuration*/
CONST(SocTempMonitorTstConfigType, FUNCSAFETYPKG_CONST) SocTempMonitorTstConfig = {
	(SocTempOverHeatInt_NotDetectedNotification)
};



/*Iram 1bit injected error not corrected notifition function*/
void Iram_1BitErr_Un_CorrNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=0x%lx\n",(uint32)ErrCode);
}

/*Iram 1bit injected error not detected notifition function*/
void Iram_1BitErr_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=0x%lx\n",(uint32)ErrCode);
}

/*Iram 2bit injected error not detected notifition function*/
void Iram_2BitErr_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=%lu\n",ErrCode);
}

/*SOC temperature over heat interrupt not detected notification function*/
void SocTempOverHeatInt_NotDetectedNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*ATCM 1bit injected error not detected notifition function*/
void ATCM_1BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*ATCM 2bit injected error not detected notifition function*/
void ATCM_2BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*B0TCM 1bit injected error not detected notifition function*/
void B0TCM_1BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*B0TCM 2bit injected error not detected notifition function*/
void B0TCM_2BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*B1TCM 1bit injected error not detected notifition function*/
void B1TCM_1BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*B1TCM 2bit injected error not detected notifition function*/
void B1TCM_2BitErr_Un_DetectNotification()
{
    printf("%s\n",__FUNCTION__);
}

/*Data cache data ram 1bit injected error not detected notifition function*/
void DCACHE_Data_Ram_1BitErr_Un_DetectNotification(uint32 DcacheErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("DcacheErrCode=0x%lx\n",DcacheErrCode);
}

/*Data cache tag ram 1bit injected error not detected notifition function*/
void DCACHE_Tag_Ram_1BitErr_Un_DetectNotification(uint32 DcacheErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("DcacheErrCode=0x%lx\n",DcacheErrCode);
}

/*Fabric 1bit injected error not detected notifition function*/
void Fabric_1BitErr_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=0x%lx\n",ErrCode);
}

/*Fabric 2bit injected error not detected notifition function*/
void Fabric_2BitErr_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=0x%lx\n",ErrCode);
}

/*Fuse 1bit injected error error status not detected notifition function*/
void Fuse_1BitErr_ErrSta_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=%lu\n",ErrCode);
}

/*Fuse 1bit injected error interrupt status not detected notifition function*/
void Fuse_1BitErr_IntSta_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=%lu\n",ErrCode);
}

/*Fuse 2bit injected error error status not detected notifition function*/
void Fuse_2BitErr_ErrSta_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=%lu\n",ErrCode);
}

/*Fuse 2bit injected error interrupt status not detected notifition function*/
void Fuse_2BitErr_IntSta_Un_DetectNotification(uint32 ErrCode)
{
    printf("%s\n",__FUNCTION__);
    printf("ErrCode=%lu\n",ErrCode);
}

/*Dual core lock step compare failed not detected notification function*/
void DualCore_LockStep_CompareFail_NotDetectedNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
void Sem_SigMonitor_UnmatchNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
void Sem_SigMon_InjErr_UndetectedNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Latent fault0 notifition function*/
void Latent_Fault0Notification(uint32 Noc_Type, uint32 FaultCode)
{
    printf("%s\n",__FUNCTION__);
    printf("Noc_Type=%lu FaultCode=%lu\n",Noc_Type,FaultCode);
}

/*Latent fault1 notifition function*/
void Latent_Fault1Notification(uint32 Noc_Type, uint32 FaultCode)
{
    printf("%s\n",__FUNCTION__);
    printf("Noc_Type=%lu FaultCode=%lu\n",Noc_Type,FaultCode);
}

/*Latent fault2 notifition function*/
void Latent_Fault2Notification(uint32 Noc_Type, uint32 FaultCode)
{
    printf("%s\n",__FUNCTION__);
    printf("Noc_Type=%lu FaultCode=%lu\n",Noc_Type,FaultCode);
}

/*Latent fault3 notifition function*/
void Latent_Fault3Notification(uint32 Noc_Type, uint32 FaultCode)
{
    printf("%s\n",__FUNCTION__);
    printf("Noc_Type=%lu FaultCode=%lu\n",Noc_Type,FaultCode);
}

/*Rc check Ap clk not checked notification function*/
void RcChkAp_UnCheckNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Safe check ap clk not checked notification function*/
void SafeChkAp_UnCheckNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Wdt self test over flow reset not detected notifition function*/
void Wdt_SelfTst_OverFlow_Rst_UndetectNotification(void)
{
    printf("%s\n",__FUNCTION__);
}

/*Sem monitor input interrupt notification function*/
void SemIntOutput2CpuNotification(uint32 SemInputStatus)
{
    printf("%s\n",__FUNCTION__);
    printf("SemInputStatus=%lu\n",SemInputStatus);
}


