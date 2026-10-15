#ifndef __ST_BATT_SAMPLER_MAIN_H
#define __ST_BATT_SAMPLER_MAIN_H

#include <semaphore.h>
#include "../sampler_base.h"
#include "imeu2_can.h"
#include "imeu3_can.h"

//#define TRACE	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //
//#define TRACEX	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //

//#pragma pack(push)
//#pragma pack(n) n=1,2,4
//#pragma pack(pop)

#define CAN1_READ_TIMEOUT				50



#define ONE_MSG_LEN                     64

#define MAX_PWRONPCS_SEC                60

#define MAX_READ_BUFF_SIZE	            200

#define  CAN_SAMPLE_OK		            0
#define  CAN_WRITE_ERROR	            -5
#define  CAN_READ_ERROR	                -6

#define BATT_COMM_FAIL_TIMES	        40 //2021-09-28 SBK Suggest 4S,BMS Erase Falsh Need over 500ms

#define IMEU_COMM_FAIL_TIMES	        40

#define CAN_CMD_BROADCAST	            0
#define MAX_CAN_SAMPLER_LOCK_WAIT	    100
#define MAX_READ_RETRY_TIMES            3

#define STBATT_PUSH2CCU_PERIOD         10
#define URGET_STBATT_PUSH2CCU_PERIOD	1

#define MAX_CYCLE_SEND_TIMES	        180 //BMS one Times MAX Recv 170 MsgID==1K
#define MAX_BMS_SINGLE_MSGLEN 	        6

#define MAX_BMS_ABNORMAL_TIMES          20

#define STAT_RELAY_ON  	1
#define STAT_RELAY_OFF 	0

#define STBATT_V33_HOLDSTAT_CMD		0x01
#define STBATT_V33_POWERON_CMD 		0x02
#define STBATT_V33_POWEROFF_CMD 	0x03

#define MIN_BMS_BINSIZE 			4*1024
#define MAX_BMS_BINSIZE				10*1024*1024 

#define BMS_RETRY_POWERON_TIMES 	3

#define MAX_RECSAVE_NUM			    100
#define MAX_SUPPORTTED_VAR_FILES    100

#define MAX_IMEU_NUM 4

#define MAX_COOLINGPACK_NUM 5
#define COOLINGPACK_PARAMETER_NUM 10  //Used for liquid cooled PACK push parameter array

#define FLOAT_EQU_ZERO(fValue)  	(fabs((fValue)) <= 1e-6)

#define AirCondi_Interval_Time 3

typedef enum Freq_Code
{
	NORUNNING = 0,
    RUNNING = 1,
    FINISHED = 2, // added by hepingfu 
}Freq_Flag;

typedef unsigned long long ULLONG;//edit by kuangyue 20250306

typedef enum tagERR_Code
{
	ERR_CAN_R_NO_ERR	= 0x00,
    ERR_CAN_R_COMM_FAIL = 0x01,	
    ERR_CAN_R_NO_LOCK             /*by control Cmd Locked, need wait to sample */

}ERR_CODE_CAN_R;

typedef enum
{
    IMEU_STATE_FAULT = 0,   // 故障/急停（红色，最高优先级）
    IMEU_STATE_CHARGE,      // 充电中（黄色）
    IMEU_STATE_DISCHARGE,   // 放电中（蓝色）
    IMEU_STATE_STANDBY,     // 待机/空闲（绿色，最低优先级）
    IMEU_STATE_OFFGRID,     // 离网状态（橙色）
    IMEU_STATE_UNKNOWN      // 数据获取失败或未知状态（可选）

} IMEU_LED_STATE;

typedef enum tagLEDSTATE
{
	START1_STATE = -1,
	GREEN = 1,
	RED,
    YELLOW,
	BLUE,
	ORANGE,

}LED_STATE;

typedef enum tag_STBATT_STATE_MACHINE_STATE
{
    STBATT_STATE_IDLE = 0,
    STBATT_STATE_WAIT_CHARGE,
    STBATT_STATE_IN_CHARGING,
    //STBATT_STATE_WAIT_DISCHARGE,
    STBATT_STATE_IN_DISCHARGING,
    STBATT_STATE_FAULTED,
    STBATT_STATE_OFFGRID,
    STBATT_STATE_MAX_ST,
}STBATT_STATEMACHINE_STATE;



//-------------Start----------------//
typedef enum tagstNDBattCmd_Idx
{ 
	_CAN_R_CMD_B2C_STATU = 0,
	_CAN_R_CMD_B2C_SUMDATA1, 
	_CAN_R_CMD_B2C_SUMDATA2, 
	_CAN_R_CMD_B2C_SUMDATA3, 
	_CAN_R_CMD_B2C_LIMIT, 

	_CAN_R_CMD_S2A_ALARM,
	_CAN_R_CMD_S2A_SUMDATA3,
	_CAN_R_CMD_S2A_SUMDATA4,
    
	_CAN_R_CMD_B2C_INFYALARM,
	_CAN_R_CMD_B2C_INFYFAULT,
	_CAN_R_CMD_B2C_INFYGPIO,
	_CAN_R_CMD_B2C_UPGRADESTATU,
	 

	_CAN_S_CMD_C2B_STATU,   

	_CAN_S_CMD_C2B_HANDSHAKE,

    _CAN_S_CMD_C2B_UNLOCK,

    _CAN_S_CMD_C2B_EXCHANGE_KEY,

    _CAN_S_CMD_C2B_WRITE_REQUEST,

     _CAN_S_CMD_C2B_WRITE_REQUEST_2,

     _CAN_S_CMD_C2B_ROUTINE_CONTROL,

     _CAN_S_CMD_C2B_DOWNLOAD_REQUSET,

     _CAN_S_CMD_C2B_DOWNLOAD_FILE,

     _CAN_S_CMD_C2B_DOWNLOAD_EXIT,

     _CAN_S_CMD_C2B_CHECK_FILE,
 
    _CAN_S_CMD_C2B_CHECK_COMPATIBILITY,

    _CAN_S_CMD_C2B_RESET_ECU,

	_CAN_S_CMD_C2B_SECTORDATA,

	_CAN_S_CMD_C2B_DATAFINISH, //Old Ver Need, New Don't Need

	_CAN_S_CMD_C2B_INSTALL,

	_CAN_S_CMD_C2B_STOPUPGRADE,

	_MAX_ND_CAN_CMD,

}ID_CMD_ND_BATT;

#define UPGRADE_UDS_RECEIVE_TIMEOUT     6

#define UPGRADE_FRAME_ID_UDS            0x1bf00000
#define UPGRADE_SEED_LEN                4
#define UPGRADE_WRITE_REQUEST_LEN       16
#define UPGRADE_ROUNTINE_CONTROL_LEN    9
#define UPGRADE_DOWNLOAD_REQUEST_LEN    8
#define UPGRADE_CRC32_DATA_LEN          4

#define UDS_SERVICE_HANDSHAKE_10          0x10
#define UDS_SERVICE_UNLOCK_27              0x27
#define UDS_SERVICE_EXCHANGE_KEY_27        0x27
#define UDS_SERVICE_WRITE_REQUEST_2E       0x2e
#define UDS_SERVICE_ROUNTINE_CONTROL_31    0x31
#define UDS_SERVICE_DOWNLOAD_REQUEST_34    0x34
#define UDS_SERVICE_DOWNLOAD_36            0x36
#define UDS_SERVICE_DOWNLOAD_EXIT_37       0x37
#define UDS_SERVICE_RESET_ECU_11           0x11

#define UDS_FRAME_TYPE_SINGLE               0
#define UDS_FRAME_TYPE_FLOW_CONTROL         3
#define UDS_REPLY_POSITIVE                  0x40
#define UDS_REPLY_NEGATIVE                  0x7f
#define UDS_REPLY_PENDING                   0x78

#define BMU_MIN_ADDRESS                 0x32
#define BMU_MAX_ADDRESS                 0x35
#define BSU_MIN_ADDRESS                 0x00
#define BSU_MAX_ADDRESS                 0x0B
#define BMU_AUX_MIN_ADDRESS             0xC8
#define BMU_AUX_MAX_ADDRESS             0xCB
#define IMEU2_MIN_ADDRESS               0x28
#define IMEU2_MAX_ADDRESS               0x2B
#define IMEU3_MIN_ADDRESS               0x1E
#define IMEU3_MAX_ADDRESS               0x21
#define IMEU3_STSCAbinet_ADDRESS        0x2D
#define Air_IMEU3_MIN_ADDRESS           0x2B
#define Air_IMEU3_MAX_ADDRESS           0x2C


#define BMU_PROGRAM_ADDRESS             0x014000
#define BSU_PROGRAM_ADDRESS             0X0800A000
//#define BMU_AUX_PROGRAM_ADDRESS         0X0800A000
#define BMU_AUX_PROGRAM_ADDRESS         0X0000A000
#define IMEU2_PROGRAM_ADDRESS           0x0009800
#define IMEU3_PROGRAM_ADDRESS           0x0800C800
#define IMEU3_AIR_ADDRESS               0x08008000
typedef enum _UDS_RESPOND
{
    SERVICE_NEGATIVE_FAILED = 0,
    SERVICE_POSITIVE_OK = 1,
    SERVICE_NEGATIVE_BUSY = 2,
    SERVICE_NOT_RESPOND = 3
}UDS_RESPOND;

typedef enum _UPGRADE_DEV_TYPE
{
    DEV_NONE = 0,
    DEV_BSU,
    DEV_BMU,
    DEV_BMU_AUX = 4,
    DEV_IMEU2 = 6,
    DEV_IMEU3 = 6

}UPGRADE_DEV_TYPE;
 
typedef enum emBMSCanMsgSaveStatu
{
    STAT_ON_START = 0,
    STAT_ON_PENDING,
    STAT_ON_END

}STATE_BMS_SAVE;

typedef enum emTriggerMsgType
{
    TP_TRIG_SUSPEND = 0,
    TP_TRIG_MANUAL,
    TP_TRIG_FAULT,
    TP_TRIG_LOG, 
    TP_TRIG_WAVE,  
    TP_TRIG_DOD,  


}TP_TRIGGER_MSG;

typedef struct tagA_ND_BattFrame
{
	BYTE 			byRawData[CAN_FRAME_LEN];

	/*CAN Protocal Head Info,Total 29 bits,extend frame*/  //0x040E8F(91-9A)
	BYTE			byErrCode;
 	BYTE			byResv1;
	BYTE			byCmdType;

 	int				nSubCmd;
    BYTE    		byBMSGrpAddr;   

	UINT    		nBMSGrpMapIdx;
	
	BYTE 			byPCSAddr;

	ID_CMD_ND_BATT  emFuncCmd;

	UINT 			uiFrmID;
	UINT 			uiDataLen;
	BYTE   	 		byData[CAN_DATA_AREA_LEN + 1];      /* Data Area */

	BOOL    bIsCtrlCmd;     /* if true, need send urgently */

}A_CAN_ND_BATT_FRAME;

typedef struct tagBMSCANMsgSave
{
	time_t					tm_LastStatEnter;
	
	STATE_BMS_SAVE			emSaveState;


 }STU_BMS_CANMSG_STATMACH;


typedef struct tagBMSMsgFileInfo
{
	time_t			tmFileGenerated;
	UINT			nBMSIdx;
	ULONG			nTotalMsgNum;  
	char			szFileName[64];

}BMSMSG_FILEINFO;

#define MIN_DATA_FRAME              8
#define SIZE_WRITE_BUFF             1024

typedef struct tagCanDataSave
{
    HANDLE						hThread;
    BOOL						bNeedExitThread;

    HANDLE						hMutex;
    sem_t						hSemWrite[MAX_SINGLE_STBATT_NUM];
 
    BOOL						bIsStartedWrite[MAX_SINGLE_STBATT_NUM];
    BOOL						bNeedWriteMsg2File[MAX_SINGLE_STBATT_NUM];
    BOOL						bNeedPost[MAX_SINGLE_STBATT_NUM];

    FILE						*hFile[MAX_SINGLE_STBATT_NUM];

	char						szWriteBuff[MAX_SINGLE_STBATT_NUM][SIZE_WRITE_BUFF];

    UINT                        nTotalMsgNum[MAX_SINGLE_STBATT_NUM];

    //time_t                      tmFileGenerated[MAX_SINGLE_STBATT_NUM];

    UINT						nUsedTotalFiles;
	BMSMSG_FILEINFO				usedFileInfo[MAX_SUPPORTTED_VAR_FILES];
	BOOL						bIsUSBDisk_Mounted;

	BMSMSG_FILEINFO*			pFileInfo;


    //BYTE						szFrame[MIN_DATA_FRAME*MAX_WRITEMSG_NUM+1];

    STU_BMS_CANMSG_STATMACH		stuStatMach[MAX_SINGLE_STBATT_NUM];

    TP_TRIGGER_MSG		        emTrigMsgTp[MAX_SINGLE_STBATT_NUM];


}STU_CANDATA_SAVE;

//-----------------------------BMS Test Mode Rough Data----------------//

#define     MIN_PM_OUTPUT_VOLT     250
typedef enum tagFailFrom
{
	FAIL_FROM_NONE = 0,
	FAIL_FROM_MANULSTOP,
	FAIL_FROM_ABNORMAL

}FAIL_FROM;

typedef enum emTestModeFailCode
{
    FAIL_NO_FAIL = 0,
    FAIL_RELAY_VOLT_ABNORMAL,
    FAIL_SELFCHECK,
    FAIL_FULLDISCHG_STEP1,
    FAIL_FULLCHG_STEP3,
    FAIL_FULLDISCHG_STEP5,
    FAIL_HALFCHG_STEP7,

    FAIL_SYSSTATU_FAIL,
    FAIL_BAT_FAULT,
    FAIL_BAT_COMMFAIL,

}TESTMODE_FAIL_CODE;


typedef STATE_MACHINE_STAT_TEST     (*ON_STATE_PROC)(void* pArg, int nBattIdx);
typedef void (*ON_FAIL_ACTION)(void *pArg, FAIL_FROM emFailFrom, TESTMODE_FAIL_CODE emFailCode, int nBattIdx);

typedef struct tagStateMachineItem
{    
    STATE_MACHINE_STAT_TEST     emState;
    ON_STATE_PROC           pfnAction;
 
    time_t                  tm_Enter;
    FAIL_FROM               emFailFrom;
    TESTMODE_FAIL_CODE      emFailCode;
    time_t                  tm_PredictRemTM;

}STATE_MACHINE_ITEM_TEST;

typedef struct stRoughBMSTestMode
{ 

    STATE_MACHINE_ITEM_TEST		stateMachineLoop[STATE_MAX_ST];

    STATE_MACHINE_ITEM_TEST		*pLastState[MAX_SINGLE_STBATT_NUM];
    STATE_MACHINE_ITEM_TEST		*pCurrState[MAX_SINGLE_STBATT_NUM];

    ON_FAIL_ACTION				pfnOnFail[MAX_SINGLE_STBATT_NUM];
	TESTMODE_FAIL_CODE			emFailCode[MAX_SINGLE_STBATT_NUM];
	int				    		nFailClass[MAX_SINGLE_STBATT_NUM];
    int                 		nFailTimes[MAX_SINGLE_STBATT_NUM];

    STATE_MACHINE_STAT_TEST     emTrigState[MAX_SINGLE_STBATT_NUM];

    STATE_MACHINE_STAT_TEST     emRealTimeState[MAX_SINGLE_STBATT_NUM];

    #define TEST_STOP_NONE  	0
    #define TEST_STOP_NORMAL	1
    #define TEST_STOP_URGET 	2
    #define TEST_STOP_BMS   	3

    int							nStopCond[MAX_SINGLE_STBATT_NUM];

}BMS_TESTMODE_ROUGH;


//-----------------------------------End--------------------------------//
 
// 用于记录最新200条数据的环形缓冲区
typedef struct {
    BYTE buffer[200][CAN_FRAME_LEN];
    UINT head;  // 当前位置
    UINT tail;  // 末尾位置
    UINT count;
} CanRecCirBuffer;

typedef struct {
    BYTE buffer[200][CAN_FRAME_LEN];
    UINT count;
    BOOL isActive;
} CanRecBuffer;
typedef struct stBattRun_Rough
{
	HANDLE		                        hSampler; //pointer to sampler
	int			                        nSamplerId; //copy from sampler
	HANDLE		                        hSamplerLock;
    DATA_PROCESSOR*		                pDPR;

    CCU_STATE                           ccuStatu;

    STAT_PM_CTRL_MODE                   emPMCtrlMode;
    STAT_PM_CTRL_MODE                   emLast_PMCtrlMode;

	HANDLE				                hCanDevHanle;   /* when can opened, get the handle */
	HANDLE                              hCAN1Write_Mutex;
    COMMPORT*			                pComm;

    BYTE				                abyRcvBuf[MAX_READ_BUFF_SIZE * CAN_FRAME_LEN];

    BYTE                                emSysBattInstallType; //this will be gotton from bootUp setting
    UINT                                nSystemCfgBattNum;

    BOOL                                bIsSysRunAsMaster; //if run as slave, do not sample
    
    BOOL                                bCtrlOutRelayClose[MAX_SINGLE_STBATT_NUM];
    time_t                              ulLastCtrlTime[MAX_SINGLE_STBATT_NUM];
	int                                 nCtrlTimes[MAX_SINGLE_STBATT_NUM];
    int                                 nPowerONTimes[MAX_SINGLE_STBATT_NUM];

	//A_BLOB_BATT_MGMT_PLAN               stESMgmtPlan; //ST battery management plan settings
    A_BLOB_EMS_CONFIG_PLAN 			    blobEmsCfgPlan;

    //A_BLOB_ES_iOCPP_CONFIGURAION		blobESiOCPPCfgs;

    double                              tm_fLastCalcBattEnergyCost;

    //BattGrp sett/status
    BATT_MANUFACTURER                   emBattMFG;

    UINT                                 nProtectSOC;
    UINT                                 emDisChgByStopSOC;
    UINT                                 emFullyChgDischgMode;
    UINT                                 nBackupProtectSOC;

    UINT                                 nPVSysBattRetSOC; 
    UINT                                 nStopSOC[MAX_SINGLE_STBATT_NUM];

    HIS_BMS_RUNTIMEDATA_REC             hisBmsRunDataRec;

	HIS_HEALTH_STATIC_DAILY			    hisHealthRec; //note this pointer should never be deleted!!!


    A_BLOB_SINGLE_STBATT_RUNINFO        blobSingleBattInfo[MAX_SINGLE_STBATT_NUM];
    double                              fSingle_Last_ChrgedEnergyReading[MAX_SINGLE_STBATT_NUM];
    double                              fSingle_Last_DisChrgedEnergyReading[MAX_SINGLE_STBATT_NUM];

    A_BLOB_GROUP_STBATT_RUNINFO         blobGroupBattInfo;
    double                              fGroup_Last_ChrgedEnergyReading;
    double                              fGroup_Last_DisChrgedEnergyReading;
    
    A_BLOB_ES_RUN_DATA                  esRunData;
    
    A_BLOB_IFY_BMS_DATAINFO             blobIfyBMS[MAX_SINGLE_STBATT_NUM];

    A_BLOB_BRANCH_GRP_RECTDATA          branchGrpData[MAX_SINGLE_STBATT_NUM];

    A_BLOB_BRANCH_NODE_CALC_DATA        calcNodeData[MAX_SINGLE_STBATT_NUM];

    A_BLOB_MANUALCTRL_RECT_CMD	        blobManualCtrlPM;

	STS_AUTOMANUAL_ONOFFGRID		    emSTSAutoManual;

    float                               fLastSendReqVolt_2CCU[MAX_SINGLE_STBATT_NUM];
    float                               fLastSendReqCurr_2CCU[MAX_SINGLE_STBATT_NUM];
    GRP_RECT_CTRL_CMDTYPE               emLastCtrlPcsCmd[MAX_SINGLE_STBATT_NUM];

    double                              tm_fLastSendCmd_2CCU[MAX_SINGLE_STBATT_NUM];
    time_t                              tm_LastStopChg_4STBatt[MAX_SINGLE_STBATT_NUM];
    time_t                              tm_LastStartChg_4STBatt[MAX_SINGLE_STBATT_NUM];
    time_t 				                t_FirstBattBoot;

    double                              t_LastTrigIsoStartCmd[MAX_SINGLE_STBATT_NUM];


    double                              fCalc_ChargedEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fCalc_DisChargedEnergy[MAX_SINGLE_STBATT_NUM];

	double                              fLastCalc_ChargedEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fLastCalc_DisChargedEnergy[MAX_SINGLE_STBATT_NUM];
	
    int                                 nCCUAbnormalTimes;
    UINT                                nInsolutionAlmTimes;
    
    UINT                                nPCSFaultTimes;
    UINT                                nLeakageCurrAlmTimes;

    //User Grid ACMeter
    float				                fGridMeter_Volt[3];
	float				                fGridMeter_Curr[3];
    float                               fGridMeter_Power[3];
    float 				                f_QgridMeter;
	float 				                f_QcabMeter;                                
	//Cabinet ACMeter               
	float				                fCabMeter_Volt[3]; 
	float				                fCabMeter_Curr[3]; 
	float				                fCabMeter_Power[3]; 

    float                                fPowerCubeTemp[3];

    IMEU_TYPE			                emIMEUType[MAX_IMEU_NUM];
    IMEU_TYPE			                emPOWER_IMEUType[MAX_IMEU_NUM];
    A_BLOB_BATTCAB_IMEU2_V20            blobIMEU2Data[MAX_SINGLE_STBATT_NUM];
    IMEU2_CAN_ROUGH_DATA                IMEU2CANRoughData[MAX_IMEU_NUM];

    IMEU3_CAN_ROUGH_DATA                IMEU3CANRoughData[MAX_IMEU_NUM];
    IMEU3_CAN_ROUGH_DATA                PowerEU3_OtherEU3_Data[MAX_IMEU_NUM];

    BATT_CELL_TEMP_PRECISION            eBattCellTempPrecison;

    UINT                                nStuffTimes[MAX_SINGLE_STBATT_NUM];

    //----- CAN Frame Save Function -----//
    STU_CANDATA_SAVE                    stuCanSave;

    //-----------Test Mode DataRough -------//
    BMS_TESTMODE_ROUGH                  stuTestMode;

    //------------YFY ISO Detection Trigger -----------------//
    TRIGGER_ISO_STATUS 			        emISOTriggerStatu[MAX_SINGLE_STBATT_NUM];
    float                               fISO_PeriodMin_Sett;

    //---------- Upgrade IMEU-D Bin --------------//
	//CCU_UPGRADE_PARAM						tagCCUUpgradeParam;
	A_BLOB_UPDT_INFO_4CCU					blobUpdCmd;

    //Add 20230704 YFY BMS CMD Send
    UINT					            emEnterSuperuser[MAX_SINGLE_STBATT_NUM];
  	UINT					            emClearFaultType[MAX_SINGLE_STBATT_NUM];
    UINT					            emWorkMode[MAX_SINGLE_STBATT_NUM];

    BOOL                                bEnterReboot[MAX_SINGLE_STBATT_NUM];

    //Calculate charge or discharge energy by Jeffery 20240715
    double                              fSingleStartChargeEnergy[MAX_SINGLE_STBATT_NUM];
    float                               fSingleChargeEnergy[MAX_SINGLE_STBATT_NUM];
    double                              fSingleStartDischargeEnergy[MAX_SINGLE_STBATT_NUM];
    float                               fSingleDischargeEnergy[MAX_SINGLE_STBATT_NUM];  
    double                              tm_fLastCalcBattEnergyCostbyBMS;
    BATT_FULLY_CHGDISCHG_STATE          emFullyChgDischgState[MAX_SINGLE_STBATT_NUM];
    // LJH240715 aircond
    int                                 nSetTemp4Aircond;
    ESS_DEVICE_TYPE                     emESSDevType;
    //---------- BMS TestMode Parm ----------//
    A_BLOB_BMS_TESTMODE_SET             bmsTestModeParm;
    // BMS_TESTMODE_PARM			        autoTestParm[MAX_SINGLE_STBATT_NUM];
    A_BLOB_BMS_TESTMODE_RESULT          bmsAutoTestResult[MAX_SINGLE_STBATT_NUM];
    A_BLOB_BMS_TESTMODE_PROCESS         bmsTestProcessRecord[MAX_SINGLE_STBATT_NUM];
    // LJH250408 CtrlByREMS
    BOOL                                enableREMSCtrl;
    BOOL                                executeChargingWhenReflux;
    //WLD01206 IMEU3 control
    UINT                                nAirCondi_StartUp_Num;
    time_t                              t_AirCondi_StartUp_Time;
    UINT                                nSmokeDetector_num;
    time_t                              t_Smoke_time;
    BOOL                                bSmokeFlag;
    // Jeffery250325 ​Internal Resistance Control
    int                                 nRintFreqIdx[MAX_SINGLE_STBATT_NUM];
    float                               f35KW_OpenTemp;
    float                               f35KW_ShutdownTemp;

    // manu record bms-mu2 can log
    BOOL                                bNeedRecBmsMu2CanLog;
    double                              tm_fLastStartRecBmsMu2canlog;
    // auto record realtime 200 bmu canlogs, and 200 canlogs after alm active 
    CanRecCirBuffer                     bmuCanRecCirBuffer;
    CanRecBuffer                        bmuAlmCanRecCirBuffer;
    double                              tm_fLastRecBmsAlmcanlog;

    // hot-wait flag
    BOOL                                bHotWaitFlag;

    char                                szBsuVersion[16];
    LCCfg                               lcCfg; // LJH250508 LCCfg
    time_t                              t_QF3_triger; 
    UINT                                QF3_Enable;
    // LJH250930 加快自动充放响应,StopSOC_SetSEMS
    BYTE                                nStopSOC_ChrgSet_SEMS; // 充电截止SOC(REMS、自动充电) 1001, 3306,
	BYTE                                nStopSOC_DiscSet_SEMS; // 放电截止SOC(REMS、自动放电) 1001, 3309,
    BOOL                                bStandby_Set; // 1, 85,
}IMEU_SAMPLER_ROUGH;

typedef struct stUDS_UPGRADE_PARAMETER
{
    BYTE byServiceID;
    BYTE bySubServiceID[3];
    BYTE bySubServiceLen;

}STUDS_UPGRADE_PARAMETER;

typedef struct stHEX2BIN_INFO
{
    int iBinDataLen;
    unsigned int uStartAddr;
    int iErrLine;
}HEX2BIN_INFO;

//-----   Public Function   s-----//
ssize_t Write_CAN1(IN IMEU_SAMPLER_ROUGH *pBattData, IN void * buf,IN size_t count);
void    CAN_ClearReadBuf(IMEU_SAMPLER_ROUGH *pBattData);
void    CAN_Filter_Conf(IMEU_SAMPLER_ROUGH *pBattData);

void* _Thread_BMSCanDataSave(void* pArgs);

char *GetMsgTypeStrByMsgTPId(TP_TRIGGER_MSG emMsgType, char *szMsgType);

#define Grp_gap 1    // Setting for batt0 tows with batt1, batt2 tows with batt3, so fix Grp_gap == 1

//---BMS .Txt Upgrade---//
void Make_SendCmd_4CANFrame(IN A_CAN_ND_BATT_FRAME *pFrm);
//--------------------End-----------------------//

void YFY_IMEU_Sample(IMEU_SAMPLER_ROUGH *pIMEU_Rough);

void IMEU3DoCtl_TargetAlarms(IMEU_SAMPLER_ROUGH *pIMEU_Rough);

int IMEU3SendDOCtlCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, int BatIdx, BYTE WhichDo, BYTE Value);

void RecBmsMu2Log(IMEU_SAMPLER_ROUGH *pIMEU_Rough, BYTE* pbySendBuf, ssize_t uFrameSize);

#endif


