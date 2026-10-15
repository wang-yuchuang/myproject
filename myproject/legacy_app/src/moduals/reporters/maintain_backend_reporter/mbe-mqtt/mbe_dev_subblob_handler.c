#include "mbe_dev_mqtt.h"
#include "../../web_reporter/back_api/blob_param_mgmt.h"

#define MAX_SUPPORT_MBEDEV_BLOBPARAM_SZIE     512*1024 //max support 52K max for blob
typedef struct 
{
	int			nSubIdx;
	void*		pSubVal;
#define MBEDEV_SUBBLOB_VALTYPE_INT		    0
#define MBEDEV_SUBBLOB_VALTYPE_UINT		    1
#define MBEDEV_SUBBLOB_VALTYPE_BYTE		    2
#define MBEDEV_SUBBLOB_VALTYPE_SHORT	    3
#define MBEDEV_SUBBLOB_VALTYPE_USHORT	    4
#define MBEDEV_SUBBLOB_VALTYPE_FLOAT_0	    10
#define MBEDEV_SUBBLOB_VALTYPE_FLOAT_1	    11
#define MBEDEV_SUBBLOB_VALTYPE_FLOAT_2	    12
#define MBEDEV_SUBBLOB_VALTYPE_FLOAT_3	    13
#define MBEDEV_SUBBLOB_VALTYPE_STRING	    20
#define MBEDEV_SUBBLOB_VALTYPE_IP		    21
#define MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT	22 //ushort with bit-offset
#define MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT	 23 //byte with bit-offset
	int			emValType;
	int			nValSize; //for string, should set this size to strlen
    BOOL        bIsRemote_SetAble; //if false, means not support remote set
}MBEDEV_SUBBLOB_ITEM;

static BOOL Find_SubBlobItem_FromABlob(IN int nBlobId, IN void* pBlob, int nSubIdx, OUT MBEDEV_SUBBLOB_ITEM* pSubBlobItem)
{
    int n;
    MBEDEV_SUBBLOB_ITEM* pItemLst = NULL;
    int nItemTotal = 0;
    if(nBlobId == BLOB_TYPE_NETWORKPARAMS)
    {
        A_BLOB_NETWORKINFO* pBlb_NetWork = (A_BLOB_NETWORKINFO*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            //eth 1
            {SUBIDX_NETWORK_STATUS_WIRED_1,         &pBlb_NetWork->netInfo_eht0.emNetPort_Status,   MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_DHCP_WIRED_1,           &pBlb_NetWork->bDHCP_On_eth0,                   MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_IP_WIRED_1,             &pBlb_NetWork->netInfo_eht0.ulIpV4_IP,          MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_NETMASK_WIRED_1,        &pBlb_NetWork->netInfo_eht0.ulIpV4_Mask,        MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_GATEWAY_WIRED_1,        &pBlb_NetWork->netInfo_eht0.ulIpV4_Gateway,     MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_DNSSERVER_WIRED_1,      &pBlb_NetWork->netInfo_eht0.ulIpV4_DNSServer,   MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_MAC_ADDR_1,             &pBlb_NetWork->netInfo_eht0.szMacAddr,          MBEDEV_SUBBLOB_VALTYPE_STRING,  18, FALSE},
            //eth2  
            {SUBIDX_NETWORK_STATUS_WIRED_2,         &pBlb_NetWork->netInfo_eht1.emNetPort_Status,   MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_DHCP_WIRED_2,           &pBlb_NetWork->bDHCP_On_eth1,                   MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_IP_WIRED_2,             &pBlb_NetWork->netInfo_eht1.ulIpV4_IP,          MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_NETMASK_WIRED_2,        &pBlb_NetWork->netInfo_eht1.ulIpV4_Mask,        MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_GATEWAY_WIRED_2,        &pBlb_NetWork->netInfo_eht1.ulIpV4_Gateway,     MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_MAC_ADDR_2,             &pBlb_NetWork->netInfo_eht1.szMacAddr,          MBEDEV_SUBBLOB_VALTYPE_STRING,  18, FALSE},
            //lte   
            {SUBIDX_NETWORK_ENABLE_LTE_MODEM,       &pBlb_NetWork->bIsEnable_LTEModem,              MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  TRUE},
            {SUBIDX_NETWORK_STATUS_WIRELESS,        &pBlb_NetWork->netInfo_usb0.emNetPort_Status,   MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_APN_SEL_WIRELESS,       &pBlb_NetWork->lteModemINfo.bIsManulSet_APN,    MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_APN_NAME_WIRELESS,      &pBlb_NetWork->lteModemINfo.szAPN,              MBEDEV_SUBBLOB_VALTYPE_STRING,  32, FALSE},
            {SUBIDX_NETWORK_APN_USER_WIRELESS,      &pBlb_NetWork->lteModemINfo.szAPN_User,         MBEDEV_SUBBLOB_VALTYPE_STRING,  32, FALSE},
            {SUBIDX_NETWORK_APN_PASSWD_WIRELESS,    &pBlb_NetWork->lteModemINfo.szAPN_Passwd,       MBEDEV_SUBBLOB_VALTYPE_STRING,  32, FALSE},
            {SUBIDX_NETWORK_IP_WIRELESS,            &pBlb_NetWork->netInfo_usb0.ulIpV4_IP,          MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_NETMASK_WIRELESS,       &pBlb_NetWork->netInfo_usb0.ulIpV4_Mask,        MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_GATEWAY_WIRELESS,       &pBlb_NetWork->netInfo_usb0.ulIpV4_Gateway,     MBEDEV_SUBBLOB_VALTYPE_IP,      4,  FALSE},
            {SUBIDX_NETWORK_MAC_ADDR_WIRELESS,      &pBlb_NetWork->netInfo_usb0.szMacAddr,          MBEDEV_SUBBLOB_VALTYPE_STRING,  18, FALSE},            
            {SUBIDX_NETWORK_IMSI_WIRELESS,          &pBlb_NetWork->lteModemINfo.szIMSI,             MBEDEV_SUBBLOB_VALTYPE_STRING,  20, FALSE},            
            {SUBIDX_NETWORK_ICCID_WIRELESS,         &pBlb_NetWork->lteModemINfo.szICCID,            MBEDEV_SUBBLOB_VALTYPE_STRING,  32, FALSE},
            {SUBIDX_NETWORK_NETTYPE_WIRELESS,       &pBlb_NetWork->lteModemINfo.szNetwork_Type,     MBEDEV_SUBBLOB_VALTYPE_STRING,  32, FALSE},
            {SUBIDX_NETWORK_SIGLEVEL_WIRELESS,      &pBlb_NetWork->lteModemINfo.nSignal_dBm,        MBEDEV_SUBBLOB_VALTYPE_UINT,    4,  FALSE},
            {SUBIDX_NETWORK_SN_WIRELESS,            &pBlb_NetWork->lteModemINfo.szSN,               MBEDEV_SUBBLOB_VALTYPE_STRING,  64, FALSE},

        };
        
        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
    else if(nBlobId == BLOB_TYPE_CHARGER_BOOTINFO)
    {
        A_BLOB_CHARGER_BOOTINFO* pBlb_BootInfo = (A_BLOB_CHARGER_BOOTINFO*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_BOOTSET_CHARERMODEL,            &pBlb_BootInfo->szChargePointModel,             MBEDEV_SUBBLOB_VALTYPE_STRING,  20, TRUE},            
            {SUBIDX_BOOTSET_CHARERSN,               &pBlb_BootInfo->szChargePointSerualNuml,        MBEDEV_SUBBLOB_VALTYPE_STRING,  25, FALSE},
            {SUBIDX_BOOTSET_CHARERVENDOR,           &pBlb_BootInfo->szChargePointVendor,            MBEDEV_SUBBLOB_VALTYPE_STRING,  20, TRUE},
            {SUBIDX_BOOTSET_APPVERSION,             &pBlb_BootInfo->szFirmwareVer,                  MBEDEV_SUBBLOB_VALTYPE_STRING,  50,  FALSE},
        };
        
        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
   /* else if(nBlobId == BLOB_TYPE_CHARGER_SPECIFIC_SETTING)
    {
        A_BLOB_CHARGERSPECIFIC_SETTINGS* pBlb_CCUSet = (A_BLOB_CHARGERSPECIFIC_SETTINGS*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_CCUSETT_CP_TYPE,                &pBlb_CCUSet->emSystemConfiguration,                MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},            
            {SUBIDX_CCUSETT_PLCBOARD_TYPE,          &pBlb_CCUSet->ccuSettings.emPLCBoardType,           MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},            
            {SUBIDX_CCUSETT_PLC_WORKMODE,           &pBlb_CCUSet->emPLCProtocolMode,                    MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},   
            {SUBIDX_CCUSETT_PnC_ENABLE,             &pBlb_CCUSet->emPlugAndChargeEnabled,               MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},   
            {SUBIDX_CCUSETT_AC_SOCKET_INST,         &pBlb_CCUSet->ccuSettings.evseAbilities,            MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT, 1,  TRUE},  //1-offset 
            {SUBIDX_CCUSETT_GND_DETECT_ENB,         &pBlb_CCUSet->ccuSettings.emGoundFaultDetectEnable, MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},   
            {SUBIDX_CCUSETT_CCU_ADDRESS,            &pBlb_CCUSet->ccuSettings.nCCUAddress,              MBEDEV_SUBBLOB_VALTYPE_INT,      4,  FALSE},   
            {SUBIDX_CCUSETT_POWERMERGE_MODE,        &pBlb_CCUSet->ccuSettings.emChargeMergeMode,        MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},  
            {SUBIDX_CCUSETT_CCU_BARCODE,            &pBlb_CCUSet->ccuSettings.szBarcode,                MBEDEV_SUBBLOB_VALTYPE_STRING,   24, FALSE},  
            {SUBIDX_CCUSETT_CCU_SWVER,              &pBlb_CCUSet->ccuSettings.szSWVer,                  MBEDEV_SUBBLOB_VALTYPE_STRING,   8,  FALSE},  
            {SUBIDX_CCUSETT_CCU_MAC_ADDR,           &pBlb_CCUSet->ccuSettings.szMacAaddress,            MBEDEV_SUBBLOB_VALTYPE_STRING,   16, FALSE},  
            {SUBIDX_CCUSETT_AVAIBL_GUN_NUM,         &pBlb_CCUSet->ccuSettings.nAailableGunNum,          MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  FALSE},   
            {SUBIDX_CCUSETT_ACINPUT_PWRLMT,         &pBlb_CCUSet->ccuSettings.fInputACTotalPowerLmt,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},   
            {SUBIDX_CCUSETT_ACINPUT_CURRLMT,        &pBlb_CCUSet->fACInpurCurrLmt_4Rectifier,           MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},   
            {SUBIDX_CCUSETT_MAX_VOLT_LMT,           &pBlb_CCUSet->ccuSettings.fMaxVoltEVSEOffers,       MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},  
            {SUBIDX_CCUSETT_ACGUN_MAXPWRLMT,        &pBlb_CCUSet->ccuSettings.fACPowerSetting,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},  
            {SUBIDX_CCUSETT_MAXCURRLMT_GUN1,        &pBlb_CCUSet->ccuSettings.fGun1_MaxCurrOutLimit,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},  
            {SUBIDX_CCUSETT_MAXCURRLMT_GUN2,        &pBlb_CCUSet->ccuSettings.fGun2_MaxCurrOutLimit,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_0,   4,  TRUE},  
            {SUBIDX_CCUSETT_MAX_SYSFAN_SPEED,       &pBlb_CCUSet->ccuSettings.bySysFanSpeedLevel,       MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_MAX_SYSFAN_SPEED_AUX,   &pBlb_CCUSet->byAuxSysFanSpeedLevel,                MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_INSULDETCT_NUM,         &pBlb_CCUSet->ccuSettings.byInsulDetectorNumber,    MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},
            {SUBIDX_CCUSETT_INSULDETCT_TYPE,        &pBlb_CCUSet->ccuSettings.emInsulDetectorTYpe,      MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},
            {SUBIDX_CCUSETT_EPO_VOLTLEVEL,          &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 0,  TRUE}, //0-offset 
            {SUBIDX_CCUSETT_DOOROPEN_ENABLE,        &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 2,  TRUE}, //2-offset 
            {SUBIDX_CCUSETT_SPD_ENABLE,             &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 4,  TRUE}, //4-offset 
            {SUBIDX_CCUSETT_ELOCK_TYPE,             &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 6,  TRUE}, //6-offset 
            {SUBIDX_CCUSETT_ONECLICK_DISTART,       &pBlb_CCUSet->ccuSettings.emDICtrlCharge,           MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_SELFCHK_ENB_GUN1,       &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 8,  TRUE}, //8-offset 
            {SUBIDX_CCUSETT_SELFCHK_ENB_GUN2,       &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 10,  TRUE}, //10-offset 
            {SUBIDX_CCUSETT_SELFCHK_ENB_GUN3,       &pBlb_CCUSet->ccuSettings.gunAtributes,             MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT, 12,  TRUE}, //12-offset 
            {SUBIDX_CCUSETT_AC_CUTOFF_DELAY,        &pBlb_CCUSet->byACCutOffDelay_InMin,                MBEDEV_SUBBLOB_VALTYPE_BYTE,      1,  TRUE},  
            {SUBIDX_CCUSETT_DEHUM_FANMODE_RUN,      &pBlb_CCUSet->bySysFanWorkMode_M,                   MBEDEV_SUBBLOB_VALTYPE_BYTE,      1,  TRUE},  
            {SUBIDX_CCUSETT_DEHUM_FANMODE_STOP,     &pBlb_CCUSet->bySysFanWorkMode_N,                   MBEDEV_SUBBLOB_VALTYPE_BYTE,      1,  TRUE},  
            {SUBIDX_CCUSETT_CURRLMT_CONNOVETTEMP,   &pBlb_CCUSet->nLmtPercent_onGunOverTemp,            MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_CONNOVERTEMP_DERATE_INTERV,  &pBlb_CCUSet->nGunOverTempDeRating_Interv,     MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_GBT_TEMPSAMP_TYPE,      &pBlb_CCUSet->emGBT_TempSampType,                   MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_CONNOVETTEMP_POINT,     &pBlb_CCUSet->nGunOverTemp_Set,                     MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_RECOV_FROMOVERTEMP_POINT,&pBlb_CCUSet->nGunOverTemp_Clear,                  MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_CONNULTRATEMP_POINT,     &pBlb_CCUSet->nGunUltratemp,                       MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_SYSOVRTEMP_POINT,       &pBlb_CCUSet->nSysOverTemp_4Cooling,                MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_SYSULTRATEMP_POINT,     &pBlb_CCUSet->nSysUltraTemp_4Cooling,               MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_DO6_USAGE,              &pBlb_CCUSet->emDO6_Usage,                          MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_INSUL_CHECK_DURATION,   &pBlb_CCUSet->nInsulationCheckDuration,             MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_POERMODULE_WORKMODE,   &pBlb_CCUSet->byPowerModuleWorkMode,                 MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_POERMODULE_DENOISEMODE,   &pBlb_CCUSet->byPowerModule_DenoiseMode,          MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_SYSFAN_DENOSISE_STRATEGY,   &pBlb_CCUSet->bySystemFan_DenoseStrategy,       MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_DERATING_WHENFOLLOW_PWRMODULE,   &pBlb_CCUSet->byDeratingLevel_whenFollowPowerModule, MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_PWRMODULE_HIGHLOW_VOLTINTERV,   &pBlb_CCUSet->byPowerModuleHighLow_VoltInterv,        MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},  
            {SUBIDX_CCUSETT_ENAB_INSULDETECT_INCHARGING_A,   &pBlb_CCUSet->ccuSettings.evseAbilities,               MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT, 2,  TRUE},  //2-offset  
            {SUBIDX_CCUSETT_ENAB_INSULDETECT_INCHARGING_B,   &pBlb_CCUSet->ccuSettings.evseAbilities,               MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT, 3,  TRUE},  //3-offset  
            {SUBIDX_CCUSETT_INSULDETECT_ALAMR_KOHM,   &pBlb_CCUSet->nIMIU_InsulAlarm_kOhm,             MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE}, 
            {SUBIDX_CCUSETT_INSULDETECT_MAXVOLTAGE,   &pBlb_CCUSet->nIMIU_InsulStart_MaxVoltLmt,       MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE}, 
            {SUBIDX_CCUSETT_RECTIFER_INPUT_OVERVOLT,   &pBlb_CCUSet->nRectifier_Input_OverVoltLmt,     MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE},   
            {SUBIDX_CCUSETT_RECTIFER_INPUT_LOWVOLT,   &pBlb_CCUSet->nRectifier_Input_LowVoltLmt,       MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE}, 
            {SUBIDX_CCUSETT_ACGUN_STATE_B_TIMEOUT,   &pBlb_CCUSet->nACGun_StateB_Timeou_Sec,           MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  TRUE}, 

        };
        
        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    } */
    else if(nBlobId == BLOB_TYPE_ES_BACKEND_CFG)
    {
        A_BLOB_ES_BACKEND_CFG* pESBackend = (A_BLOB_ES_BACKEND_CFG*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_SysACInput_PwrLmt,             &pESBackend->fSysACInput_PwrLmt,             MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Enter_UrgetDischrgPwr,         &pESBackend->fEnter_UrgetDischrgPwr,         MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Exit_UrgetDischrgPwr,          &pESBackend->fExit_UrgetDischrgPwr,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_ACLoadPwrProtcPoint,           &pESBackend->fACLoadPwrProtcPoint,           MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Min_ACLoad_StartDischg,        &pESBackend->fMin_ACLoadPwr_Start_Dischg,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Max_Output2AC_PwrLmt,          &pESBackend->fMax_Output2AC_PwrLmt,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Min_Output2AC_PwrLmt,          &pESBackend->fMin_Output2AC_PwrLmt,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,   4,  TRUE},
            {SUBIDX_Output2AC_PwrRate,             &pESBackend->fOutput2AC_PwrRate,             MBEDEV_SUBBLOB_VALTYPE_FLOAT_2,   4,  TRUE},
            {SUBIDX_nCmd_101_Interval,             &pESBackend->nCmd_101_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_102_Interval,             &pESBackend->nCmd_102_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_103_Interval,             &pESBackend->nCmd_103_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_104_Interval,             &pESBackend->nCmd_104_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_105_Interval,             &pESBackend->nCmd_105_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_106_Interval,             &pESBackend->nCmd_106_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_107_Interval,             &pESBackend->nCmd_107_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_108_Interval,             &pESBackend->nCmd_108_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_109_Interval,             &pESBackend->nCmd_109_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_110_Interval,             &pESBackend->nCmd_110_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_202_Interval,             &pESBackend->nCmd_202_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_204_Interval,             &pESBackend->nCmd_204_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
            {SUBIDX_nCmd_206_Interval,             &pESBackend->nCmd_206_Interval,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,   TRUE},
        };

        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
    else if(nBlobId == BLOB_TYPE_SINGLE_STBATT_RUNINFO)
    {
        A_BLOB_SINGLE_STBATT_RUNINFO* pSgBatt = (A_BLOB_SINGLE_STBATT_RUNINFO*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_BATT_RUNSTAT,             &pSgBatt->emBattStatus_Run,                 MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  FALSE},
            {SUBIDX_BATT_SOC,                 &pSgBatt->nSOC ,                            MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  FALSE},
            {SUBIDX_BATT_SOH,                 &pSgBatt->nSOH ,                            MBEDEV_SUBBLOB_VALTYPE_INT,      4,  FALSE},
            {SUBIDX_BATT_PACKVOLT,            &pSgBatt->fPackTotalVolt ,                  MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_PACKCURR,            &pSgBatt->fPackTotalCurr ,                  MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_LMTCHRGVOLT,         &pSgBatt->fAllow_LmtChargeVolt ,            MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_LMTDISCHRGVOLT,      &pSgBatt->fAllow_LmtDisChargeVolt ,         MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_MAXCHRGCURR,         &pSgBatt->fAllow_MaxChargeCurr ,            MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_MAXDISCHRGCURR,      &pSgBatt->fAllow_MaxDisChargeCurr ,         MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_MAXCELLVOLT,         &pSgBatt->fMax_CellVolt ,                   MBEDEV_SUBBLOB_VALTYPE_FLOAT_3,  4,  FALSE},
            {SUBIDX_BATT_MINCELLVOLT,         &pSgBatt->fMin_CellVolt ,                   MBEDEV_SUBBLOB_VALTYPE_FLOAT_3,  4,  FALSE},
            {SUBIDX_BATT_MAXCELLTEMP,         &pSgBatt->fMax_CellTemp ,                   MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_MINCELLTEMP,         &pSgBatt->fMin_CellTemp ,                   MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_UCELL_MAX_CSC,       &pSgBatt->uiBAT_Ucell_Max_CSC,             MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  FALSE},
            {SUBIDX_BATT_UCELL_MAX_CELL,      &pSgBatt->uiBAT_Ucell_Max_CELL,            MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  FALSE},
            {SUBIDX_BATT_UCELL_MIN_CSC,       &pSgBatt->uiBAT_Ucell_Min_CSC,             MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  FALSE},
            {SUBIDX_BATT_UCELL_MIN_CELL,      &pSgBatt->uiBAT_Ucell_Min_CELL,            MBEDEV_SUBBLOB_VALTYPE_UINT,      4,  FALSE},
            {SUBIDX_STBATT_CYCLETIMES,        &pSgBatt->nLifeCycleTimes,                  MBEDEV_SUBBLOB_VALTYPE_USHORT,   4,  FALSE},
            {SUBIDX_STBATT_CHRGFIRBIDDEN,     &pSgBatt->bIsBatt_ChargeForbidden,        MBEDEV_SUBBLOB_VALTYPE_UINT,       4,  FALSE},
            {SUBIDX_STBATT_DISCHRGFORBIDDEN,  &pSgBatt->bIsBatt_DisChargeForbidden,     MBEDEV_SUBBLOB_VALTYPE_UINT,       4,  FALSE},
            {SUBIDX_STBATT_VERSION,           &pSgBatt->fVersion ,                      MBEDEV_SUBBLOB_VALTYPE_FLOAT_3,    4,  FALSE}, 
        };

        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
    else if(nBlobId == BLOB_TYPE_MANUALCTRL_RECT)
    {    
        A_BLOB_MANUALCTRL_RECT_CMD* pRectCtrl = (A_BLOB_MANUALCTRL_RECT_CMD*)pBlob;
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_RECTCTRL_WORK_MODE,         &pRectCtrl->emState,            MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},
            {SUBIDX_RECTCTRL_GRP_NUMBER,        &pRectCtrl->nGrp ,              MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},
            {SUBIDX_RECTCTRL_ADJ_VOLT,          &pRectCtrl->fGrpVolt ,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  TRUE},
            {SUBIDX_RECTCTRL_ADJ_CURR,          &pRectCtrl->fGrpCurr ,          MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  TRUE},
            {SUBIDX_RECTCTRL_STOP_SOC,          &pRectCtrl->nStopSOC ,          MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  TRUE},
 
        };

        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
    else if(nBlobId == BLOB_TYPE_BRANCH_NODE_CALC_DATA)
    {
        A_BLOB_BRANCH_NODE_CALC_DATA* pNodeCalc = (A_BLOB_BRANCH_NODE_CALC_DATA*)pBlob;
 
        MBEDEV_SUBBLOB_ITEM items[] = 
        {
            {SUBIDX_BATT_DISTRI4ACDC_DISCHRGPWR,    &pNodeCalc->fBatt_Distri_4_ACDC_Pwr ,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_ACDC_MAX_POWER,                 &pNodeCalc->fACDC_Max_Ability_Pwr ,      MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_ACDC_DISTRI4BATT_CHRGPWR,       &pNodeCalc->fACDC_Distri_4_BattChrg_Pwr, MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_REMAIN_BATT_DISCHRGPWR,            &pNodeCalc->fRemain_Batt_DisChrg_Pwr,    MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_REMAIN_CHRG_PWR_REQ,              &pNodeCalc->fRemain_Batt_Chrg_Pwr_Req,   MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_BATT_PRI_WORKMODE,              &pNodeCalc->emBatt_WorkMode,             MBEDEV_SUBBLOB_VALTYPE_UINT,     4,  FALSE},
	        {SUBIDX_AVG_CHRGLMTPWR,                 &pNodeCalc->fAvg_ChrgLmtPower,           MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
            {SUBIDX_AVG_DISCHRGINGLMTPWR,           &pNodeCalc->fAvg_DisChrgingLmtPower,     MBEDEV_SUBBLOB_VALTYPE_FLOAT_1,  4,  FALSE},
        };

        nItemTotal = ITEM_OF(items);
        pItemLst = items;
    }
    if(pItemLst != NULL)
    {
        for(n = 0; n < nItemTotal; n++, pItemLst++)
        {
            if(pItemLst->nSubIdx == nSubIdx)
            {
                memcpy(pSubBlobItem, pItemLst, sizeof(MBEDEV_SUBBLOB_ITEM));
                return TRUE;
            }
        }
    }    

    return FALSE;
}

char* Get_ASubParmaVal_FromKey(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY* pSigKeyInfo, OUT char* pszVal)
{
    REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;	
    PARAM_VAL_GET valGet;
    //first get the blobVal
    static BYTE s_byBlobRawVal[MAX_SUPPORT_MBEDEV_BLOBPARAM_SZIE];
    if(pSigKeyInfo->nValueSize > MAX_SUPPORT_MBEDEV_BLOBPARAM_SZIE)
    {
        return NULL;
    }
    else if(pSigKeyInfo->nValueSize < 1)
    {
        return NULL;
    }   
    valGet.nGetSize = pSigKeyInfo->nValueSize;
	valGet.val.blobValue = s_byBlobRawVal;
    int nDAI_Type = (pSigKeyInfo->emSigType == PARAM_TYPE_SETTING) ?
		DAITYPE_SETT_PARAMVALUE_RW : DAITYPE_STAT_PARAMVALUE_R;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, nDAI_Type,
			pSigKeyInfo->nDev, pSigKeyInfo->nSigId, &valGet, sizeof(PARAM_VAL_GET));
    //printf("nDev=[%d], nDAI_Type=[%d], nSigId=[%d]\n", pSigKeyInfo->nDev, nDAI_Type, pSigKeyInfo->nSigId);
    if(err != ERRCODE_DAI_OK)
    {
        return NULL;
    }

    MBEDEV_SUBBLOB_ITEM item;
    if(Find_SubBlobItem_FromABlob(pSigKeyInfo->nBlobId, (void*)s_byBlobRawVal, 
        pSigKeyInfo->nSigSubId, &item))
    {
        if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_INT)
        {
            sprintf(pszVal, "%d", *(int*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_UINT)
        {
            sprintf(pszVal, "%u", *(UINT*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_BYTE)
        {
            sprintf(pszVal, "%u", *(BYTE*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_SHORT)
        {
            sprintf(pszVal, "%d", *(SHORT*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_USHORT)
        {
            sprintf(pszVal, "%u", *(USHORT*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_0)
        {
            sprintf(pszVal, "%.0f", *(float*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_1)
        {
            sprintf(pszVal, "%.1f", *(float*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_2)
        {
            sprintf(pszVal, "%.2f", *(float*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_3)
        {
            sprintf(pszVal, "%.3f", *(float*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_STRING)
        {
            sprintf(pszVal, "%s", (char*)item.pSubVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_IP)
        {
            sprintf(pszVal, "%d.%d.%d.%d", 
                *(BYTE*)((BYTE*)item.pSubVal + 0),
                *(BYTE*)((BYTE*)item.pSubVal + 1),
                *(BYTE*)((BYTE*)item.pSubVal + 2),
                *(BYTE*)((BYTE*)item.pSubVal + 3));
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT)
        {
            USHORT usVal = *(USHORT*)item.pSubVal;
            usVal = (usVal >> item.nValSize);
            usVal &= 0x03;
            sprintf(pszVal, "%u", usVal);
            return pszVal;
        }
        else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT)
        {
            BYTE byVal = *(BYTE*)item.pSubVal;
            byVal = (byVal >> item.nValSize);
            byVal &= 0x01;
            sprintf(pszVal, "%u", byVal);
            return pszVal;
        }
    }
    
	return NULL;
}

int Set_ASubParamVal_FromKeyVal(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY_VAL* pKeyVal, int nTimeOut_mSec)
{
    REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;	
    PARAM_VAL_GET valGet;
    //first get the blobVal
    static BYTE s_byBlobRawVal[MAX_SUPPORT_MBEDEV_BLOBPARAM_SZIE];
    if(pKeyVal->key.nValueSize > MAX_SUPPORT_MBEDEV_BLOBPARAM_SZIE)
    {
        return MBEDEV_EXESET_RESULT_WRONG;
    }
    else if(pKeyVal->key.nValueSize < 1)
    {
        return MBEDEV_EXESET_RESULT_WRONG;
    }  
    valGet.nGetSize = pKeyVal->key.nValueSize;
	valGet.val.blobValue = s_byBlobRawVal;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			pKeyVal->key.nDev, pKeyVal->key.nSigId, &valGet, sizeof(PARAM_VAL_GET));
    if(err != ERRCODE_DAI_OK)
    {
        return MBEDEV_EXESET_RESULT_WRONG;
    }

    //now find the subItem and set it
    MBEDEV_SUBBLOB_ITEM item;
    if(!Find_SubBlobItem_FromABlob(pKeyVal->key.nBlobId, (void*)s_byBlobRawVal, 
        pKeyVal->key.nSigSubId, &item))
    {
        return MBEDEV_EXESET_RESULT_WRONG;
    }

    if(!item.bIsRemote_SetAble) //not settable
    {
        return MBEDEV_EXESET_RESULT_WRONG;
    }        

    if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_INT)
    {
        *(int*)item.pSubVal = atoi(pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_UINT)
    {
        *(UINT*)item.pSubVal = (UINT)atoi(pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_BYTE)
    {
        *(BYTE*)item.pSubVal = (BYTE)atoi(pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_SHORT)
    {
        *(SHORT*)item.pSubVal = (SHORT)atoi(pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_USHORT)
    {
        *(USHORT*)item.pSubVal = (USHORT)atoi(pKeyVal->szVal);
    }
    else if((item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_0) ||
            (item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_1) ||
            (item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_2) ||
            (item.emValType == MBEDEV_SUBBLOB_VALTYPE_FLOAT_3))
    {
        *(float*)item.pSubVal = atof(pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_STRING)
    {
        snprintf((char*)item.pSubVal, item.nValSize, "%s", pKeyVal->szVal);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_IP)
    {
        UINT nIP = inet_addr(pKeyVal->szVal);
        if(nIP == INADDR_NONE)
        {
            return MBEDEV_EXESET_RESULT_WRONG;
        }
        *(USHORT*)item.pSubVal = nIP;
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_USHORT_2_BIT)
    {
        USHORT usVal = (USHORT)atoi(pKeyVal->szVal);
        usVal &= 0x0003;
        USHORT ulCurrVal = *(USHORT*)item.pSubVal;
        ulCurrVal &= ((0x0003 << item.nValSize) ^ 0xFFFF);
        *(USHORT*)item.pSubVal = ulCurrVal | (usVal << item.nValSize);
    }
    else if(item.emValType == MBEDEV_SUBBLOB_VALTYPE_BYTE_1_BIT)
    {
        BYTE byVal = (BYTE)atoi(pKeyVal->szVal);
        byVal &= 0x01;
        BYTE byCurrVal = *(BYTE*)item.pSubVal;
        byCurrVal &= ((0x01 << item.nValSize) ^ 0xFF);
        *(BYTE*)item.pSubVal = byCurrVal | (byVal << item.nValSize);
    }

    VAR_VALUE varV;
    varV.blobValue = s_byBlobRawVal;
	int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pKeyVal->key.nDev, pKeyVal->key.nSigId, 
					&varV, pKeyVal->key.nValueSize, nTimeOut_mSec);	

	return (nErr == ERRCODE_DAI_OK) ? MBEDEV_EXESET_RESULT_SUCCESS : MBEDEV_EXESET_RESULT_FAILED;
}

