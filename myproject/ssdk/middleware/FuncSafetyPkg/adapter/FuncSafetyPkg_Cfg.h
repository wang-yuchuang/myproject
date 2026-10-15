/**
 * @file FuncSafetyPkg_Cfg.h
 * @brief Function Safety Configuration.
 * @details Function safety package
 * @date 2021-07-06 18:26:23
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


/*Iram inject error configuration*/
extern CONST(Iram_ZoneConfigType, FUNCSAFETYPKG_CONST) Iram_ZoneConfig[];

/*TCM inject error configuration*/
extern CONST(TCMErrConfigType, FUNCSAFETYPKG_CONST) TCMErrConfig;

/*Data cache inject error configuration*/
extern CONST(DCACHEErrConfigType, FUNCSAFETYPKG_CONST) DcacheErrConfig;

/*Fabric inject error configuration*/
extern CONST(FabricErrConfigType, FUNCSAFETYPKG_CONST) FabricErrConfig;

/*Fuse error configuration*/
extern CONST(FuseErrConfigType, FUNCSAFETYPKG_CONST) FuseErrConfig;

/*Sem signal monitor configuration*/
extern CONST(SemSignalMonConfigType, FUNCSAFETYPKG_CONST) SemSigMonitorConfig;

/*Run bist configuration*/
extern CONST(BistConfigType, FUNCSAFETYPKG_CONST) BistConfig;
/*Wdt self test over flow configuration*/
extern CONST(WdtSelfTestConfigType, FUNCSAFETYPKG_CONST) WdtSelfTestConfig;

/*Fail safe reference clock checked(safe clk,ap clk,rc)*/
extern CONST(FsrefClkChkConfigType, FUNCSAFETYPKG_CONST) FsrefClkChkConfig;

/*Sem input interrupt monitor configuration*/
extern CONST(SemIntMonConfigType, FUNCSAFETYPKG_CONST) SemIntMonConfig;

/*Dual core lock step inject error configuration*/
extern CONST(DualCoreLockStepInjectErrConfigType, FUNCSAFETYPKG_CONST) DualCoreLockStepInjectErrConfig;

/*SOC temperature monitor test configuration*/
extern CONST(SocTempMonitorTstConfigType, FUNCSAFETYPKG_CONST) SocTempMonitorTstConfig;


/*Iram 1bit injected error not corrected notifition function*/
extern void Iram_1BitErr_Un_CorrNotification(uint32 ErrCode);

/*Iram 1bit injected error not detected notifition function*/
extern void Iram_1BitErr_Un_DetectNotification(uint32 ErrCode);

/*Iram 2bit injected error not detected notifition function*/
extern void Iram_2BitErr_Un_DetectNotification(uint32 ErrCode);

/*ATCM 1bit injected error not detected notifition function*/
extern void ATCM_1BitErr_Un_DetectNotification();
/*ATCM 2bit injected error not detected notifition function*/
extern void ATCM_2BitErr_Un_DetectNotification();
/*B0TCM 1bit injected error not detected notifition function*/
extern void B0TCM_1BitErr_Un_DetectNotification();
/*B0TCM 2bit injected error not detected notifition function*/
extern void B0TCM_2BitErr_Un_DetectNotification();

/*B1TCM 1bit injected error not detected notifition function*/
extern void B1TCM_1BitErr_Un_DetectNotification();

/*B1TCM 2bit injected error not detected notifition function*/
extern void B1TCM_2BitErr_Un_DetectNotification();

/*Data cache data ram 1bit injected error not detected notifition function*/
extern void DCACHE_Data_Ram_1BitErr_Un_DetectNotification(uint32 DcacheErrCode);

/*Data cache tag ram 1bit injected error not detected notifition function*/
extern void DCACHE_Tag_Ram_1BitErr_Un_DetectNotification(uint32 DcacheErrCode);

/*Fabric 1bit injected error not detected notifition function*/
extern void Fabric_1BitErr_Un_DetectNotification(uint32 ErrCode);

/*Fabric 2bit injected error not detected notifition function*/
extern void Fabric_2BitErr_Un_DetectNotification(uint32 ErrCode);
/*Fuse 1bit injected error error status not detected notifition function*/
extern void Fuse_1BitErr_ErrSta_Un_DetectNotification(uint32 ErrCode);

/*Fuse 1bit injected error interrupt status not detected notifition function*/
extern void Fuse_1BitErr_IntSta_Un_DetectNotification(uint32 ErrCode);

/*Fuse 2bit injected error error status not detected notifition function*/
extern void Fuse_2BitErr_ErrSta_Un_DetectNotification(uint32 ErrCode);

/*Fuse 2bit injected error interrupt status not detected notifition function*/
extern void Fuse_2BitErr_IntSta_Un_DetectNotification(uint32 ErrCode);

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
extern void Sem_SigMonitor_UnmatchNotification(void);

/*Sem signal not match not detected to send notifition function when setting a wrong target value */
extern void Sem_SigMon_InjErr_UndetectedNotification(void);

/*Latent fault0 notifition function*/
extern void Latent_Fault0Notification(uint32 Noc_Type, uint32 FaultCode);

/*Latent fault1 notifition function*/
extern void Latent_Fault1Notification(uint32 Noc_Type, uint32 FaultCode);

/*Latent fault2 notifition function*/
extern void Latent_Fault2Notification(uint32 Noc_Type, uint32 FaultCode);

/*Latent fault3 notifition function*/
extern void Latent_Fault3Notification(uint32 Noc_Type, uint32 FaultCode);

/*Wdt self test over flow reset not detected notifition function*/
extern void Wdt_SelfTst_OverFlow_Rst_UndetectNotification(void);

/*Rc check Ap clk not checked notification function*/
extern void RcChkAp_UnCheckNotification(void);

/*Safe check ap clk not checked notification function*/
extern void SafeChkAp_UnCheckNotification(void);

/*Sem monitor input interrupt notification function*/
extern void SemIntOutput2CpuNotification(uint32 SemInputStatus);

/*Dual core lock step compare failed not detected notification function*/
extern void DualCore_LockStep_CompareFail_NotDetectedNotification(void);

/*SOC temperature over heat interrupt not detected notification function*/
extern void SocTempOverHeatInt_NotDetectedNotification(void);


/**
 * @brief Switches self test on or off when ECU powers on
 */
#define FUSAPKG_SELF_TEST_POWER_ON               			(STD_ON)

/**
 * @brief Switches self test on or off periodic
 */
#define FUSAPKG_SELF_TEST_PERIOD                            (STD_ON)

/**
 * @brief Switches inject 1bit error in iram1 on or off when ECU powers on
 */
#define FUSAPKG_IRAM_1BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 2bit error in iram1 on or off when ECU powers on
 */
#define FUSAPKG_IRAM_2BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 1bit error in ATCM on or off when ECU powers on
 */
#define FUSAPKG_ATCM_1BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 2bit error in ATCM on or off when ECU powers on
 */
#define FUSAPKG_ATCM_2BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 1bit error in B0TCM on or off when ECU powers on
 */
#define FUSAPKG_B0TCM_1BIT_ERROR_INJECT_POWER_ON            (STD_ON)

/**
 * @brief Switches inject 2bit error in B0TCM on or off when ECU powers on
 */
#define FUSAPKG_B0TCM_2BIT_ERROR_INJECT_POWER_ON            (STD_ON)

/**
 * @brief Switches inject 1bit error in B1TCM on or off when ECU powers on
 */
#define FUSAPKG_B1TCM_1BIT_ERROR_INJECT_POWER_ON            (STD_ON)

/**
 * @brief Switches inject 2bit error in B1TCM on or off when ECU powers on
 */
#define FUSAPKG_B1TCM_2BIT_ERROR_INJECT_POWER_ON            (STD_ON)

/**
 * @brief Switches inject 1bit error in DCACHE on or off when ECU powers on
 */
#define FUSAPKG_DCACHE_1BIT_ERROR_INJECT_POWER_ON           (STD_ON)

/**
 * @brief Switches inject 1bit error on fabric on or off when ECU powers on
 */
#define FUSAPKG_FABRIC_1BIT_ERROR_INJECT_POWER_ON           (STD_ON)

/**
 * @brief Switches inject 2bit error on fabric on or off when ECU powers on
 */
#define FUSAPKG_FABRIC_2BIT_ERROR_INJECT_POWER_ON           (STD_ON)

/**
 * @brief Switches inject 1bit error on fuse on or off when ECU powers on
 */
#define FUSAPKG_FUSE_1BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 2bit error on fuse on or off when ECU powers on
 */
#define FUSAPKG_FUSE_2BIT_ERROR_INJECT_POWER_ON             (STD_ON)

/**
 * @brief Switches Rc clock check Ap clock on or off when ECU powers on
 */
#define FUSAPKG_FSREFCLK_RC_CHK_AP_POWER_ON                 (STD_ON)

/**
 * @brief Switches Safe clock check Ap clock on or off when ECU powers on
 */
#define FUSAPKG_FSREFCLK_SAFE_CHK_AP_POWER_ON               (STD_ON)

/**
 * @brief Switches run BIST on or off when ECU power on
 */
#define FUSAPKG_RUN_BIST_POWER_ON                           (STD_ON)

/**
 * @brief Switches WDT self test for overflow reset request on or off when ECU powers on
 */
#define FUSAPKG_WDT_SELFTEST_POWER_ON                       (STD_ON)

/**
 * @brief Switches SEM signal monitor on or off when ECU power on
 */
#define FUSAPKG_SEM_SIGMONITOR_POWER_ON                       (STD_ON)

/**
 * @brief Switches SEM signal monitor inject error on or off when ECU powers on
 */
#define FUSAPKG_SEM_SIGMON_INJECTERR_POWER_ON                       (STD_ON)

/**
 * @brief Switches Dual core lock step on or off when ECU powers on
 */
#define FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_POWER_ON             (STD_ON)

/**
 * @brief Switches SOC temperature monitor test on or off when ECU powers on
 */
#define FUSAPKG_SOC_TEMP_MONITOR_TST_POWER_ON             (STD_ON)

/**
 * @brief Switches inject 1bit error in iram1 on or off period
 */
#define FUSAPKG_IRAM_1BIT_ERROR_INJECT_PERIOD               (STD_ON)

/**
 * @brief Switches inject 2bit error in iram1 on or off period
 */
#define FUSAPKG_IRAM_2BIT_ERROR_INJECT_PERIOD               (STD_ON)

/**
 * @brief Switches inject 1bit error on fuse on or off period
 */
#define FUSAPKG_FUSE_1BIT_ERROR_INJECT_PERIOD               (STD_ON)

/**
 * @brief Switches inject 2bit error on fuse on or off period
 */
#define FUSAPKG_FUSE_2BIT_ERROR_INJECT_PERIOD               (STD_ON)

/**
 * @brief Switches dual_core lock step inject error on or off period
 */
#define FUSAPKG_DUALCORE_LOCKSTEP_INJECTERR_PERIOD                (STD_ON)


/**
 * @brief The period count for injecting 1/2 bits error in Iram periodically.
 */
#define IRAM_ERROR_INJECT_PERIOD_CNT  (1)



/**
 * @brief The period count for injecting 1/2 bits error in Efuse periodically.
 */
#define FUSE_ERROR_INJECT_PERIOD_CNT  (2)



/**
 * @brief The period count for injecting error in dual core lock step periodically.
 */
#define DUALCORE_LOCKSTEP_ERROR_INJECT_PERIOD_CNT  (3)


/**
 * @brief Inject 1bit error in iram mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define IRAM_1BIT_ERROR_MODE                		(INJECT_FIXED_POINT)


/**
 * @brief Inject 2bit error in iram mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define IRAM_2BIT_ERROR_MODE                		(INJECT_FIXED_POINT)


/**
 * @brief Inject 1/2bit error in fabric mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define FABRIC_INJECT_ERROR_MODE            	    (INJECT_FIXED_POINT)

/**
 * @brief Inject 1/2bit error in fuse mode:INJECT_FIXED_POINT, INJECT_ALL_TRAVEL
 */
#define FUSE_INJECT_ERROR_MODE            			(INJECT_FIXED_POINT)

/**
 * @brief Injecting errors in several IRAM
 */
#define IRAM_ERR_NUM                        	    (1U)


/**
 * @brief The number of SEM interrupt input for SEM interrupt output0
 */
#define SEM_INT_OUT0_NUM    4

/**
 * @brief The number of SEM interrupt input for SEM interrupt output1
 */
#define SEM_INT_OUT1_NUM    3

/**
 * @brief The number of SEM interrupt input for SEM interrupt output2
 */
#define SEM_INT_OUT2_NUM    4

/**
 * @brief The number of SEM interrupt input for SEM interrupt output2
 */
#define SEM_INT_OUT3_NUM    1

#ifdef __cplusplus
}
#endif

#endif
