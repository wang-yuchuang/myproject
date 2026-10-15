//IFY.h

#ifndef __REPORTER_ES_IFY_H
#define __REPORTER_ES_IFY_H
#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include "../../cJSON.h"
#include "../reporters_base.h"

//#define TRACE	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //
//#define TRACEX	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //

#define ES_MAX_TRANS_RELATED_MSG_QUEUE_SIZE		10
#define ES_MAX_NORMAL_MSG_QUEUE_SIZE			30

//#define  TRUE_STR  "true"
//#define  FALSE_STR  "false"

typedef enum tagES_IFY_MessgeType
{	
	C_100_BootNotif = 0, 			//BootNotification,
	C_101_StatusNotif, 			//StatusNotification
	C_102_BattCabStaNotif,		
	C_103_ESBMSStaNotif,		
	C_104_PwrDistrStaNotif,		
	C_105_ESRunDataStaNotif,	
	C_106_SingleRectStaNotif,	
	C_107_PwrDistrCabStaNotif,	
	C_108_SysVersionStaNotif,	
	C_109_SysActvAlmNotif,
	C_110_SingleBattryInfo,
	S_111_Trigger,				//Trigger Msg
	C_202_HourSectorStaticKwh,  	//---历史统计数据报文---
	C_204_DailyStaticKwh,
	C_206_FeeRateKwh,                                                                                                                	 
	S_401_RmtRest, 				//RemoteReset
	S_402_RmtChgEVESEAbi, 		//RemoteChangeEVSEAbility
	//S_403_RmtChgConnAbi, 		//RemoteChangeConnectorAbility
	S_404_RmtUpdtFirmw, 		//RemoteUpdateFirmware
	C_405_StatOfFirmw, 			//StatusOfFirmwareUpdating
	S_406_RmtCallDiag, 			//RemoteCallDiagnostic
	C_407_StatOfDiag, 			//StatusOfDiagnosticUploading
	S_408_ReadHistoryData, 
	S_411_RmtGetConfig, 		//RemoteGetConfiguration
	S_412_RmtSetConfig, 		//RemoteSetConfiguration
	S_413_RmtGetLocalListVersion, //RemoteGetLocalListVersion

	//---EMS Mgmt Plan---//
	S_503_RmtGetEMSPlan, 		//RemoteGetEMSPlan
	S_504_RmtSetEMSPlanProfile, //RemoteSetEMSPlanProfile

	//---Fee Rate Cfg---//
	S_601_RmtGetFeeRateCfg,	
	S_602_RmtSetFeeRateProfile,

	//number of msgs
	Max_ES_IFY_Msgs,
	//this is just for msg parsing
	Msg_Unkown = -1,
}ES_IFY_MSG_TYPE;

typedef enum tagES_IFYMsgSrc
{
	ES_IFY_MSG_CS, //from CS
	ES_IFY_MSG_CP, //from CP
}ES_IFY_MSG_SRC;



//typedef int (*MSG_HANDLER)(void* pArg, BOOL bSend_Later); //pArg point to the IFY data structure, bSendLater means queue that msg!
typedef int (*MSG_HANDLER)(void* pArg);

typedef struct tagES_IFYMsgInfo
{
	ES_IFY_MSG_TYPE		emMsgType;
	ES_IFY_MSG_SRC		emMsgSrc;
	//char				szMsgName[32];
	int					nMsgId;
	MSG_HANDLER			pfnMsgHandler;
}ES_IFY_MSG_INFO;

typedef enum tagES_IFYConnectionStatus
{
	Disconnected = 0,
	Connected,
	Wait4BootNotify_Conf,
	//BootNotify_Pending, //pending status can handle Configuration settings
	BootNotify_Rejected,
	Ready, //BootNotify done, now ready for other msgs
}ES_IFY_COMM_STATUS;


typedef enum tagMsgSentType
{
	Call,
	CallResult,
	CallError,
}MSG_CALL_TYPE;

typedef enum tagActiveMsgSrc
{
	CALL_SRC_Dummy = 0,
	CALL_SRC_Normal,
	CALL_SRC_InProcessTrans,
	CALL_SRC_HisTrans
}MSGCALL_SRC;

#define ES_IFY_MAX_SENTMSG_SIZE	4096*4
typedef struct tagSentMsgData
{
	ES_IFY_MSG_TYPE		emMsgType;
	MSGCALL_SRC			emSrc;
	time_t				tmRelatedTransStart; //for identify diffent Trans session
	time_t				tmSampled; //Jimmy added 20190309, for metervalues update use
	int					nBelongedConn; //0 means conn0
	char				szJsonData[ES_IFY_MAX_SENTMSG_SIZE];

	time_t				tmLastErrorRespArrived; //last time received error msg fro CS
	int					nMsgErrorRecievedCount; //Error msg received count, according to IFY doc, it is Trans-Related msg, need to re-send as many attempts as the key set
}ES_IFY_CALL_MSG;

typedef struct tagActiveMsg
{
	#define MAX_ES_IFY_SEND_RETRIES	3
	ES_IFY_CALL_MSG	msgData;
	time_t				tmLastSent;
	UINT				nUniqMsgId[MAX_ES_IFY_SEND_RETRIES]; //manage it here to match req/rep
	int					nRetryTimes;
}ES_IFY_ACTIVE_CALL_MSG;


typedef enum tagTriggerReportType
{
	Trigger_NoTriggle, //just for identifying
	Trigger_C101,
	Trigger_C102,
	Trigger_C103,
	Trigger_C105,
	Trigger_C107,
	Trigger_C206,// edit by kuangyue 20250517
	
}TRIGGER_REPORT_MSG_TYPE;


typedef struct tagConnectorData
{
	char					szRequestedMsg_UniqID[36];
	ES_IFY_MSG_TYPE			emMsgType;

	HANDLE					hqInProcessTrans; //store a real time charge record

}CONNECTOR_DATA;


typedef struct tagES_IFYDataStructs
{
	HANDLE							hReporter;
	ES_IFY_MSG_INFO				stMsgInfoList[Max_ES_IFY_Msgs];

	HANDLE							hqMsg2Sent;
	HANDLE							hqHistoryTrans; //store an off line charge record
	
 
	BOOL							bHasActiveMsg;//if yes, need wait if finish before other calls to go
	ES_IFY_ACTIVE_CALL_MSG		activeCallMsg; //shall always be call type

	#define ES_IFY_BUFFER_LEN	65536
	BYTE							szReadBuffer[ES_IFY_BUFFER_LEN];
	int								nReadBuffLen;

	char							szSendBuffer[ES_IFY_MAX_SENTMSG_SIZE];
	int								nSendBuffLen;

	ES_IFY_COMM_STATUS				emIFYConnStatus;
	#define ES_IFY_COMM_FAIL_REASON_NO_FAIL			0
	#define ES_IFY_COMM_FAIL_REASON_NO_PONG			101
	#define ES_IFY_COMM_FAIL_REASON_TIMEOUT_RECEIVE	102
	#define ES_IFY_COMM_FAIL_REASON_SERVRER_BROKEN	103
	#define ES_IFY_COMM_FAIL_REASON_URL_CHANGED		104
	#define ES_IFY_COMM_FAIL_REASON_REFUSED			105
	#define ES_IFY_COMM_FAIL_SYSTEM_EXIT			106
	int								emIFY_Disconn_Reason;
	BOOL							bNoticedExitIFY; //Main noticed to exit IFY!

	time_t							tmLastSuccessExchange; //for hearbeat calculating
	time_t							tmLastSendHeartBeat;

	HANDLE							hMutex;

	BOOL							bAuthorizeInProcess; //if Authroze.req in handling

 
	A_BLOB_ES_BACKEND_CFG			blobBackendSet;

	BOOL							bBootNotifyDone;
	cJSON							*pCurrJson;
	char							szHandlingUniqID[36]; // this only for the Response for Server Requested Messages 

	int								nConnErrTimes;


	TRIGGER_REPORT_MSG_TYPE 		emTriggerMsgType;

	BOOL							bCPStatusChanged;
	time_t							tmLastCPStatusChange;
	A_BLOB_CAHRGEPOINT_STATUSINFO  	blobCPStatusInfo;


	DIAGNOS_UPLOAD_STATUS			emDiagnosUploadStatus;
	FIRM_UPDATESTT					emFirmwareUpgradeStatus;
	time_t							tmLastSuccessSent_ClockAlignMeterval;

	char							szServerURL[255];
	//the time out setting for IFY call
	int								nMsgCallTimeOut_Sec;

	BOOL							bTransMsgFromHistory; //this flag tells the msg_handler where the msg is from

	int								nQUrgentMsgCount; //Queue has urgent msg to handle

	BOOL							bWaitRebootMsg_Ready; //need wait msg from reboot to know whether it is SOFT or HARD

	time_t							tmLastCheckedGunstatus;

	time_t							tmLastSendPingToServer;
	time_t							tmLastRecivedPongFromServer;

	int								emSysStatus_For101;

	UINT							nCfgBMSNum;
	
	BOOL							bNeedRefreshActvAlm;
	#define 						MAX_QUERY_ACTV_ALM        50
    int                 			nActiveAlmTotal;
    EVENT_ALARM     				activeAlms[MAX_QUERY_ACTV_ALM];

    int                 			nAlarmInfosTotal;
    ALARM_INFO      				*pAllAlarmInfo;

	time_t							tmLastSent_101;
	time_t							tmLastSent_102;
	time_t							tmLastSent_103;
	time_t							tmLastSent_104;
	time_t							tmLastSent_105;
	time_t							tmLastSent_106;
	time_t							tmLastSent_107;
	time_t							tmLastSent_108;
	time_t							tmLastSent_109;
	time_t							tmLastSent_110;

	time_t							tmLastSent_202;
	time_t							tmLastSent_204;
	time_t							tmLastSent_206;

	UINT							nCurrBMS;

	char 							szShieldAlarms[512];
	UINT* 							ShieldAlarms;
	UINT 							nShieldAlarmsNum;
	UINT 							nLog_Interval;// edit by kuangyue 20250516

	AIR_TYPE 						emAirType;
	BOOL							bStandbyEnable;
	LCCfg                           lcCfg; // LJH250508 LCCfg
	ESS_DEVICE_TYPE 				devType;
	int 							iLiquidPackNum;
	BOOL							TimeCalibration;

	CLOUD_UPLOAD					stCloudUpload;
}ES_IFY_DATA_STU;

// edit by kuangyue 20250318
typedef struct 
{
    ES_IFY_MSG_TYPE type;
    char szSendBuffer[ES_IFY_BUFFER_LEN];
    time_t timestamp;
} MessageEntry;

int Prepare_C100_BootNotification(void* pArg);
int Prepare_C101_StatusNotification(void* pArg);
int Prepare_C102_BattCabStaNotif(void* pArg);
int Prepare_C103_ESBMSStatusNotif(void* pArg);
int Prepare_C104_PwrDistrStaNotif(void* pArg);
int Prepare_C105_ESRunDataNotif(void* pArg);
int Prepare_C106_SingleRectStaNotif(void* pArg);
int Prepare_C107_PwrDistrCabStaNotif(void* pArg);
int Prepare_C108_SysVersionStaNotif(void* pArg);
int Prepare_C109_SysActvAlmNotif(void* pArg);
int Prepare_C110_SiglBatDataNotif(void* pArg);

int Handle_S111_TriggerMessage(void* pArg);


int Prepare_C202_HourSectorStaticKwh(void* pArg);
int Prepare_C204_DailyStaticKwh(void* pArg);
int Prepare_C206_SharpPeakFlatValleyKwh(void* pArg);


 
//int Prepare_HeartBeat(void* pArg);
//int Prepare_MeterValues(void* pArg);
  
int Handle_S401_Reset(void* pArg);
int Handle_S402_ChangeEVSEAvailability(void* pArg);
int Handle_S404_UpdateFirmware(void* pArg);
int Prepare_C405_FirmwareStatusNotification(void* pArg);
int Handle_S406_GetDiagnostics(void* pArg);
int Prepare_C407_DiagnosticsStatusNotification(void* pArg);

int Handle_S411_GetConfiguration(void* pArg);
int Handle_S412_ChangeConfiguration(void* pArg);
int Handle_S413_DevVersionInfo(void* pArg);
 
 
int Handle_S503_GetEMSPlan(void* pArg);
int Handle_S504_SetEMSPlanProfile(void* pArg);

int Handle_S601_GetFeeRate(void* pArg);
int Handle_S602_SetFeeRateProfile(void* pArg);
int Handle_S408_ReadHistoryData(void* pArg);

//handle msg sending
//int Msg_Send(IFY_DATA_STU* pIFY, cJSON* pJson, IFY_MSG_TYPE msgType, MSG_CALL_TYPE callType, BYTE connectorNo, BOOL bNeedSendLater);
void Send_Call(ES_IFY_DATA_STU* pIFY, cJSON* pJson, ES_IFY_MSG_TYPE msgType, MSGCALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent);
void Send_CallResult(ES_IFY_DATA_STU* pIFY, cJSON* pJson, ES_IFY_MSG_TYPE msgType);
char* Make_ESIFYTimeString(time_t tmInout, char* pszOut);
time_t Parse_ESIFYTime(char* szTime);
//parse IFY server call/call result
int Parse_ESIFYFrames(ES_IFY_DATA_STU* pIFY);
void QueryActvAlarmInfo(ES_IFY_DATA_STU* pIFY);

typedef enum tagES_IFY_CallErrorCode
{
	CallError_NotSupported = 101,
	CallError_FormatError = 102,
	CallError_InvalidData = 103,
	CallError_GenericError
}ES_IFY_CALLERROR_CODE;

void Send_CallError(ES_IFY_DATA_STU* pIFY, ES_IFY_MSG_TYPE msgType, ES_IFY_CALLERROR_CODE emCode, const char* szErrDescrip);

 
#define DUMP_ESIFYMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif