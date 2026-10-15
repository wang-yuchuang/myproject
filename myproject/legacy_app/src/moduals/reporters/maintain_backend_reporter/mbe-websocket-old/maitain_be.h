//ocpp.h

#ifndef __REPORTER_MAINTAIN_BE_H
#define __REPORTER_MAINTAIN_BE_H
#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include "../../../cJSON.h"

typedef enum tagMaintainBE_MessgeType
{	
	//---boot/Version/status msgs---
	C_100_Register, //register to backend,
	C_101_VersionInfo, //version information
	C_102_StatusNotif, //status notification

	//---vpn related msgs--
	S_201_StartVPN, //remote start vpn request
	S_202_StopVPN, //remote stop vpn request

	//number of msgs
	Max_Maintain_Be_Msgs,
	//this is just for msg parsing
	Msg_Unkown = -1,
}MAINTAINBE_MSG_TYPE;

typedef enum tagMaintainBe_MsgSrc
{
	MaitainBe_MSG_CS, //from CS
	MaitainBe_MSG_CP, //from CP
}MAINTAINBE_MSG_SRC;

typedef enum
{
	Call = 2,
	CallResult,
	CallError,
}MAINTAINBE_MSG_CALLTYPE;

//typedef int (*MSG_HANDLER)(void* pArg, BOOL bSend_Later); //pArg point to the OCPP data structure, bSendLater means queue that msg!
typedef int (*MSG_HANDLER)(void* pArg);

typedef struct
{
	MAINTAINBE_MSG_TYPE	emMsgType;
	MAINTAINBE_MSG_SRC	emMsgSrc;
	//char				szMsgName[32];
	int					nMsgId;
	MSG_HANDLER			pfnMsgHandler;
}MAINTAINBE_MSG_INFO;

typedef enum tagMAINTAINBEConnectionStatus
{
	Disconnected,
	Connected,
	Wait4BootNotify_Conf,
	//BootNotify_Pending, //pending status can handle Configuration settings
	BootNotify_Rejected,
	Ready, //BootNotify done, now ready for other msgs
}MAINTAINBE_COMM_STATUS;

typedef struct 
{
	int		nAlarmId;
	int		nDevId;
	time_t	tmStart;
}MAINT_BE_ALARM;

typedef struct
{
	int 			emSysRunStatus; //0-空闲可用，1-充电中， 2-故障不可用，3- 禁用维护中，4-程序升级中
#define MAINBE_MAX_SUPPORT_ACTALM	50
	int 			nActiveAlmTotal; //0 means NO alarm
	MAINT_BE_ALARM	actAlmTable[MAINBE_MAX_SUPPORT_ACTALM];
	int				emVPNConnStatus; //0-未启用，1-已启用，未连接，2-已连接
}MAINT_BE_STATUS_INFO;

typedef struct
{
	HANDLE					hReporter;
	MAINTAINBE_MSG_INFO		stMsgInfoList[Max_Maintain_Be_Msgs];
	
#define MBE_MAX_RECVBUFF_LEN	65536
	BYTE					szReadBuffer[MBE_MAX_RECVBUFF_LEN];
	int						nReadBuffLen;

#define MBE_MAX_SENBUFF_SIZE	8192
	char					szSendBuffer[MBE_MAX_SENBUFF_SIZE];
	int						nSendBuffLen;

	MAINTAINBE_COMM_STATUS	emOCPPConnStatus;
#define MBE_COMM_FAIL_REASON_NO_FAIL			0
#define MBE_COMM_FAIL_REASON_NO_PONG			101
#define MBE_COMM_FAIL_REASON_TIMEOUT_RECEIVE	102
#define MBE_COMM_FAIL_REASON_SERVRER_BROKEN		103
#define MBE_COMM_FAIL_REASON_URL_CHANGED		104
#define MBE_COMM_FAIL_REASON_REFUSED			105
#define MBE_COMM_FAIL_SYSTEM_EXIT				106
#define MBE_COMM_FAIL_REASON_CHGSN_CHANGED		107 //added by Jimmy 20210714
#define MBE_COMM_FAIL_REASON_TOO_MANY_RETRY		108
	int						emOCPP_Disconn_Reason;
	BOOL					bNoticedExitOCPP; //Main noticed to exit OCPP!

	time_t					tmLastSuccessExchange; //for hearbeat calculating
	//time_t					tmLastSendHeartBeat;

	HANDLE					hMutex;

	BOOL					bBootNotifyDone;
	cJSON					*pCurrJson;
	char					szHandlingUniqID[36]; // this only for the Response for Server Requested Messages 

	char					szServerURL[255];
	char					szServerURL_Old[255];

	time_t					tmLastSendPingToServer;
	time_t					tmLastRecivedPongFromServer;

	FIRM_UPDATESTT			emFirmwareUpgradeStatus;

	BOOL					bCPStatusChanged;
	time_t					tmLast_SuccessExChange_102;

	//101 success change
	BOOL					bVerInfo_Changed;
	time_t					tmLast_SuccessExChange_101;

	//LAST TIME  sent out statusNotif.req
	BOOL 					bHasActiveMsg; //for knowing there is activeMsg under handling
	time_t 					tmLastSentCall; //last time sent out call
	MAINT_BE_STATUS_INFO	stCurr_102Info; //current 102 status info
	MAINT_BE_STATUS_INFO	stLast_102Info;	//last 102 status info

	//for BOOTInfo
	char					szCurrChargerSN[32];
	//backend SW ver
	char					szMBE_SWVer[32];
	time_t					tmLastExe_VPNStartCmd; //last time execute StartCmd
	//current netInfo for Version-Updt
	A_BLOB_NETWORKINFO 		blbCurrNetInfo;
	A_BLOB_CHARGERSPECIFIC_SETTINGS	blbCurrCCUSet;

	//added by Jimmy to close connection when new MBE is connected
	BOOL					bIsNewMBE_MqttConnnected;

}MAINTAINBE_DATA_STU;

//from charger side
int Prepare_C100_Regsiter(void* pArg);
int Prepare_C101_VersionInfo(void* pArg);
int Prepare_C102_StatusNotif(void* pArg);
//from backend side
int Handle_S201_StartVPN(void* pArg);
int Handle_S202_StopVPN(void* pArg);

void Send_Call(MAINTAINBE_DATA_STU* pOCPP, MAINTAINBE_MSG_TYPE emMsgType, cJSON* pJson);
void Send_CallResult(MAINTAINBE_DATA_STU* pOCPP, MAINTAINBE_MSG_TYPE emMsgType, cJSON* pJson);
typedef enum tagMAINTAINBE_CallErrorCode
{
	CallError_NotSupported = 101,
	CallError_FormatError = 102,
	CallError_InvalidData = 103,
	CallError_GenericError
}MAINTAINBE_CALLERROR_CODE;
void Send_CallError(MAINTAINBE_DATA_STU* pOCPP,  MAINTAINBE_MSG_TYPE emMsgType, MAINTAINBE_CALLERROR_CODE emCode, const char* szErrDescrip);

char* Make_MaintainBeTimeString(time_t tmInout, char* pszOut);
time_t Parse_MaintainBeTime(char* szTime);
//parse OCPP server call/call result
int Parse_MBE_Frames(MAINTAINBE_DATA_STU* pDataStru);

// #define DUMP_ESiOCPPMSG(pReporter, nRW, szData) do{ \
// 	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
// 	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
// 	}while(0)
#define MBE_LOGTYPE_OPENPORT		0
#define MBE_LOGTYPE_CLOSEPORT		1
#define MBE_LOGTYPE_SENDMSG			2
#define MBE_LOGTYPE_RECVMSG			3
#define MBE_LOGTYPE_SENDERR			4
void DUMP_MBELog(int nLogType, char* szLog);

#endif