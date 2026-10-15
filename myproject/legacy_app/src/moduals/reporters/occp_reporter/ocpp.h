//ocpp.h

#ifndef __REPORTER_OCPP_H
#define __REPORTER_OCPP_H
#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include "../cJSON.h"

#define MAX_TRANS_RELATED_MSG_QUEUE_SIZE	1000
#define MAX_NORMAL_MSG_QUEUE_SIZE			50

#define  TRUE_STR  "true"
#define  FALSE_STR  "false"

typedef enum tagOCPP_MessgeType
{
	Authorize,
	BootNotification,
	ChangeAvailability,
	ChangeConfiguration,
	ClearCache,
	DataTransfer_CS, //this is divided into CS-->CP and CP-->CS
	DataTransfer_CP,
	GetConfiguration,
	HeartBeat,
	MeterValues,
	RemoteStartTransaction,
	RemoteStopTransaction,
	Reset,
	StartTransaction,
	StatusNotification,
	StopTransaction,
	UnlockConnector,
	GetDiagnostics,
	DiagnosticsStatusNotification,
	FirmwareStatusNotification,
	UpdateFirmware,
	GetLocalListVersion,
	SendLocalList,
	CancelReservation,
	ReserveNow,
	ClearChargingProfile,
	GetCompositeSchedule,
	SetChargingProfile,
	TriggerMessage,
	//number of msgs
	Max_OCPP_Msgs,
	//this is just for msg parsing
	Msg_Unkown = -1,
}OCPP_MSG_TYPE;

typedef enum tagOCPPMsgSrc
{
	OCPP_MSG_CS, //from CS
	OCPP_MSG_CP, //from CP
}OCPP_MSG_SRC;

//typedef int (*MSG_HANDLER)(void* pArg, BOOL bSend_Later); //pArg point to the OCPP data structure, bSendLater means queue that msg!
typedef int (*MSG_HANDLER)(void* pArg);

typedef struct tagOCPPMsgInfo
{
	OCPP_MSG_TYPE	emMsgType;
	OCPP_MSG_SRC		emMsgSrc;
	char				szMsgName[32];
	MSG_HANDLER		pfnMsgHandler;
}OCPP_MSG_INFO;

typedef enum tagOCPPConnectionStatus
{
	
	Disconnected,
	Connected,
	Wait4BootNotify_Conf,
	BootNotify_Pending, //pending status can handle Configuration settings
	BootNotify_Rejected,
	Ready, //BootNotify done, now ready for other msgs
}OCPP_CONN_STATUS;

typedef enum tagCPConnStatus
{
	Conn_Idle,
	Conn_InCharging,
	Conn_UnAvailable,
	Conn_Reserved,
	Conn_Failed,
}CP_CONNECTOR_STATUS;

#define Connctor_0 -1 //Jimmy Note: connector0 has to be handled seperatedly
typedef enum tagCPConnectors
{
	//Connctor_0,  //Jimmy Note: connector0 has to be handled seperatedly
	Connctor_1,
	Connctor_2,
	Connctor_3,
	Max_Connctors
}CP_CONNECTORS;

typedef enum tagMeterType
{
	METER_TYPE_NotInstalled = 0,
	METER_TYPR_dc,
	METER_TYPR_ac
}METER_TYPE;

//typedef enum tagTransMsgHandleStatus
//{
//	TRANS_MSG_HAND_Idle = 0,
//	TRANS_MSG_HAND_WaitHandle,
//	TRANS_MSG_HAND_StartTrans,
//	TRANS_MSG_HAND_MeterVals,
//	TRANS_MSG_HAND_StopTrans
//}TRANSMSG_HANDLE_STATUS;

typedef struct tagConnectorData
{
	CP_CONNECTOR_STATUS	emConnctorStauts;
	//time_t				tmLastSuccessSent_ClockAlignMeterval;
	//time_t				tmLastSuccessSent_SampleMeterval;
	//BOOL				bNeedUpdateStatus;//need send StatusNotification immediately
	BOOL					bMeterInterrupted;
	BOOL					bNeedSendMeter_Inter_Begin;
	BOOL					bNeedSendMeter_Inter_End;
	char					szRequestedMsg_UniqID[36];
	OCPP_MSG_TYPE		emMsgType;
	int					nGunChargeStatus; //from sampler raw data;
	int					nGunPlugStatus; //from sampler raw data;
	int					nGunPrepareStatus; //from sampler raw data
	int					nGunSuspendEVSESatus; //from sampler raw data
	//Meter Type
	METER_TYPE			emMeterTp;
	
	//UINT					nCancelReservationId;

	HANDLE					hqInProcessTrans; //store a real time charge record
	//TRANSMSG_HANDLE_STATUS	emInProcessTransMsgHandleStatus;
	A_BLOB_RUN_CHARGE_INFO	blobInHandling_ChargeInfo;
	
	BOOL					bConn_StatusChanged;
	time_t				tmLastConn_StatusChange;

}CONNECTOR_DATA;

typedef enum tagMsgSentType
{
	Call = 2,
	CallResult,
	CallError,
}MSG_CALL_TYPE;

typedef enum tagActiveMsgSrc
{
	CALL_SRC_Dummy = 0,
	CALL_SRC_Normal,
	CALL_SRC_InProcessTrans,
	CALL_SRC_HisTrans
}OCPP_MSGCALL_SRC;

#define OCPP_MAX_SENTMSG_SIZE	4096
typedef struct tagSentMsgData
{
	OCPP_MSG_TYPE	emMsgType;
	OCPP_MSGCALL_SRC	emSrc;
	time_t			tmRelatedTransStart; //for identify diffent Trans session
	time_t			tmSampled; //Jimmy added 20190309, for metervalues update use
	int				nBelongedConn; //0 means conn0
	char				szJsonData[OCPP_MAX_SENTMSG_SIZE];

	time_t			tmLastErrorRespArrived; //last time received error msg fro CS
	int				nMsgErrorRecievedCount; //Error msg received count, according to OCPP doc, it is Trans-Related msg, need to re-send as many attempts as the key set
}OCPP_CALL_MSG;

typedef struct tagActiveMsg
{
#define MAX_OCPP_SEND_RETRIES	3
	OCPP_CALL_MSG	msgData;
	time_t			tmLastSent;
	UINT				nUniqMsgId[MAX_OCPP_SEND_RETRIES]; //manage it here to match req/rep
	int				nRetryTimes;
}OCPP_ACTIVE_CALL_MSG;

typedef enum tagMeterValueHandleType
{
	Interrupt_Begin,
	Interrupt_End,
	Other,
	Sample_Clock,
	Sample_Periodic,
	Transaction_Begin,
	Transaction_End,
	Trigger,
}METERVALUES_HANDLE_TYPE;


typedef enum tagTriggerMessageType
{
	Trigger_NoTriggle, //just for identifying
	Trigger_BootNotification,
	Trigger_DiagnosticsStatusNotification,
	Trigger_FirmwareStatusNotification,
	Trigger_Heartbeat,
	Trigger_MeterValues,
	Trigger_StatusNotification,
	
}TRIGGERMSG_HANDLE_TYPE;

typedef enum tagOCPPChargeStopReason
{
	REASON_EMERGENCYSTOP = 0,        //EmergencyStop=0,
	REASON_EVDISCONNECTED,       //EVDisconnected,
	REASON_HARDRESET,            //HardReset,
	REASON_LOCAL,                //Local,
	REASON_OTHER,                //Other,
	REASON_POWERLOSS,            //PowerLoss,
	REASON_REBOOT,               //Reboot,
	REASON_REMOTE,               //Remote,
	REASON_SOFTRESET,            //SoftReset,
	REASON_UNLOCKCOMMAND,        //UnlockCommand,
	REASON_DEAUTHORIZED,         //DeAuthorized*/
}OCPP_CHARGE_STOPREASON;


typedef struct tagOCPPDataStructs
{
	HANDLE			hReporter;
	OCPP_MSG_INFO	stMsgInfoList[Max_OCPP_Msgs];

	HANDLE			hqMsg2Sent;

	HANDLE					hqHistoryTrans; //store an off line charge record
	//TRANSMSG_HANDLE_STATUS	emHistoryTransMsgHandleStatus;
	//HIS_METER_VALUE_REC		recHandling_HistoryMeterVals;
	HIS_CHARGE_RECORD		recInHandling_HistoryChargeRec;

	METERVALUES_HANDLE_TYPE	emHandlingMeterType;
	A_CHARGE_METERVALUE_ITEM	currHandlingHisMeterVal;
	
//#define MAX_SUSPENDING_MSGS	5
//	OCPP_ACTIVE_MSG	stSuspendedMsgList[MAX_SUSPENDING_MSGS];
//	int				nSuspendedMsgNum;
	BOOL						bHasActiveMsg;//if yes, need wait if finish before other calls to go
	OCPP_ACTIVE_CALL_MSG		activeCallMsg; //shall always be call type

#define OCPP_BUFFER_LEN	65536
	BYTE				szReadBuffer[OCPP_BUFFER_LEN];
	int				nReadBuffLen;

	char				szSendBuffer[OCPP_MAX_SENTMSG_SIZE];
	int				nSendBuffLen;

	OCPP_CONN_STATUS	emOCPPConnStatus;
#define OCPP_COMM_FAIL_REASON_NO_FAIL			0
#define OCPP_COMM_FAIL_REASON_NO_PONG			101
#define OCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE	102
#define OCPP_COMM_FAIL_REASON_SERVRER_BROKEN	103
#define OCPP_COMM_FAIL_REASON_URL_CHANGED			104
#define OCPP_COMM_FAIL_REASON_REFUSED				105
#define OCPP_COMM_FAIL_SYSTEM_EXIT				106
	int				emOCPP_Disconn_Reason;
	BOOL			bNoticedExitOCPP; //Main noticed to exit OCPP!

	time_t			tmLastSuccessExchange; //for hearbeat calculating
	time_t			tmLastSendHeartBeat;

	CONNECTOR_DATA	stConnectorData[Max_Connctors];

	HANDLE			hMutex;

	BOOL				bAuthorizeInProcess; //if Authroze.req in handling
	AUTHORIZATION_DATA  stAuthData; //current auth data


	int				emHandlingConnector; //which connector is in handling

	A_BLOB_OCPP_CONFIGS	stOCPPSettings;

	BOOL			bBootNotifyDone;
	cJSON			*pCurrJson;
	char			szHandlingUniqID[36]; // this only for the Response for Server Requested Messages 

	int				nConnErrTimes;


	TRIGGERMSG_HANDLE_TYPE emTriggerMsgType;

	BOOL				bCPStatusChanged;
	time_t			tmLastCPStatusChange;
	A_BLOB_CAHRGEPOINT_STATUSINFO  blobCPStatusInfo;

	METER_TYPE			emMainPowerMeterTp;	//for Conn0 meterVals
	BOOL					bMainMeterInterrupted;
	BOOL					bNeedSendMainMeter_Inter_Begin;
	BOOL					bNeedSendMainMeter_Inter_End;

	DIAGNOS_UPLOAD_STATUS	emDiagnosUploadStatus;
	FIRM_UPDATESTT		emFirmwareUpgradeStatus;
	time_t				tmLastSuccessSent_ClockAlignMeterval;

	char					szServerURL[255];
	//the time out setting for OCPP call
	int					nMsgCallTimeOut_Sec;

	BOOL					bTransMsgFromHistory; //this flag tells the msg_handler where the msg is from

	int					nQUrgentMsgCount; //Queue has urgent msg to handle

	BOOL					bWaitRebootMsg_Ready; //need wait msg from reboot to know whether it is SOFT or HARD

	time_t				tmLastCheckedGunstatus;

	time_t				tmLastSendPingToServer;
	time_t				tmLastRecivedPongFromServer;
	BOOL				TimeCalibration;
}OCPP_DATA_STU;

int Prepare_Authorize(void* pArg);
int Prepare_BootNotification(void* pArg);
int Handle_ChangeAvailability(void* pArg);
int Handle_ChangeConfiguration(void* pArg);
int Handle_ClearCache(void* pArg);
int Handle_DataTransfer_CS(void* pArg);
int Prepare_DataTransfer_CP(void* pArg);
int Handle_GetConfiguration(void* pArg);
int Prepare_HeartBeat(void* pArg);
int Prepare_MeterValues(void* pArg);
int Handle_RemoteStartTransaction(void* pArg);
int Handle_RemoteStopTransaction(void* pArg);
int Handle_Reset(void* pArg);
int Prepare_StartTransaction(void* pArg);
int Prepare_StatusNotification(void* pArg);
int Prepare_StopTransaction(void* pArg);
int Handle_UnlockConnector(void* pArg);
int Handle_GetDiagnostics(void* pArg);
int Prepare_DiagnosticsStatusNotification(void* pArg);
int Prepare_FirmwareStatusNotification(void* pArg);
int Handle_UpdateFirmware(void* pArg);
int Handle_GetLocalListVersion(void* pArg);
int Handle_SendLocalList(void* pArg);
int Handle_CancelReservation(void* pArg);
int Handle_ReserveNow(void* pArg);
int Handle_ClearChargingProfile(void* pArg);
int Handle_GetCompositeSchedule(void* pArg);
int Handle_SetChargingProfile(void* pArg);
int Handle_TriggerMessage(void* pArg);

//handle msg sending
//int Msg_Send(OCPP_DATA_STU* pOCPP, cJSON* pJson, OCPP_MSG_TYPE msgType, MSG_CALL_TYPE callType, BYTE connectorNo, BOOL bNeedSendLater);
void Send_Call(OCPP_DATA_STU* pOCPP, cJSON* pJson, OCPP_MSG_TYPE msgType, OCPP_MSGCALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent);
void Send_CallResult(OCPP_DATA_STU* pOCPP, cJSON* pJson, OCPP_MSG_TYPE msgType);
char* Make_OCPPTimeString(time_t tmInout, char* pszOut);
time_t Parse_OCPPTime(char* szTime);
//parse OCPP server call/call result
int Parse_OCPPFrames(OCPP_DATA_STU* pOCPP);

typedef enum tagOCPP_CallErrorCode
{
	CallError_NotImplemented = 0,
	CallError_NotSupported,
	CallError_InternalError,
	CallError_ProtocolError,
	CallError_SecurityError,
	CallError_FormationViolation,
	CallError_PropertyConstraintViolation,
	CallError_OccurenceConstraintViolation,
	CallError_TypeConstraintViolation,
	CallError_GenericError
}OCPP_CALLERROR_CODE;

void Send_CallError(OCPP_DATA_STU* pOCPP, OCPP_MSG_TYPE msgType, OCPP_CALLERROR_CODE emCode, const char* szErrDescrip, cJSON* jsonErrDetail);

#define DUMP_OCPPMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_OCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif