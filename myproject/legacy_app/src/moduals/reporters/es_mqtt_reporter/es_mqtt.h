#ifndef __REPORTER_ES_MQTT_H
#define __REPORTER_ES_MQTT_H
/*
arm-linux-gnueabihf-gcc  -fPIC  -Wall -O3 -o  Mqtt_Test Mqtt_Test.c -L   libpaho-mqtt3a.so libcrypto.so.1.1 libssl.so.1.1  libpaho-mqtt3a.so  libpaho-mqtt3cs.so   libpaho-mqtt3c.so  
*/

#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include "../../cJSON.h"
//John 2022-12-13 Use cync Instead Async
//#include "3rd_party/libpahomqtt/include/MQTTClient.h"
#include "3rd_party/libpahomqtt/include/MQTTAsync.h"
#include "3rd_party/zlib/zlib.h"
#include "../reporters_base.h" // LJH250428 cmd
#include "GH_DigitalEnergy.h"

#define LOGA_DEBUG 				0
#define LOGA_INFO 				1

// #define TRACE		printf
// trace with function and line number
// #define TRACEX		printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf



#define QOS								0
#define TIMEOUT							10000L

#define MQTT_TOPIC_SIZE					(128)		//订阅和发布主题长度
#define MQTT_BUF_SIZE					(8 * 1024)	//接收后发送缓冲区大小

#define MAX_QUERY_ACTV_ALM				50

#define MAX_MQTT_BUFFER_LEN				65535
#define MAX_NORMAL_MSG_QUEUE_SIZE		30

#define MQTT_CONNET_TIMEOUT				10
#define MQTT_MQTT_CONNET_TIMEOUT		60


typedef enum  
{
	USER_COMM_DISCONECTED = 0, 
	USER_COMM_CONNECTED,
	USER_COMM_SUBSCRIBING, //doing the subscribe when connct to broker
	USER_COMM_READY, //ready
	USER_COMM_WAIT_REBUILD_CLIENT, //means sth wrong that needs rebuild mqtt-client

}USER_COMM_STATUS;

typedef enum tagES_iOCPP_MessgeType
{	
	//---RealTime/Status DataInfo---
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

	C_155_ESRunDataStaNotif,   // LJH250428 cmd155
	C_157_PwrDistrCabStaNotif, // LJH250520 cmd157

	C_114_PCSUnitInfo,        //CXC40508  SEMS PCS Info
	C_115_SummaryInfo,        //CXC40508  SEMS Summary Info

	S_111_Trigger,				//Trigger Msg

	C_200_RTPowerInfo, // Jeffery 250922 cmd200

	C_202_HourSectorStaticKwh,  	//---History Statistic KWH Data---
	C_204_DailyStaticKwh,
	C_206_FeeRateKwh,                                                                                                                 	 
	S_401_RmtRest, 				//RemoteReset
	S_402_RmtChgEVESEAbi, 		//RemoteChangeEVSEAbility
	S_403_SOCCalibration, 		//RemoteSOCCalibration
	S_404_RmtUpdtFirmw, 		//RemoteUpdateFirmware
	C_405_StatOfFirmw, 			//StatusOfFirmwareUpdating
	S_406_RmtCallDiag, 			//RemoteCallDiagnostic
	C_407_StatOfDiag, 			//StatusOfDiagnosticUploading
	S_411_RmtGetConfig, 		//RemoteGetConfiguration
	S_412_RmtSetConfig, 		//RemoteSetConfiguration

	//---EMS Mgmt Plan---//
	S_503_RmtGetEMSPlan, 		//RemoteGetEMSPlan
	S_504_RmtSetEMSPlanProfile, //RemoteSetEMSPlanProfile

	//---Fee Rate Cfg---//
	S_601_RmtGetFeeRateCfg,	
	S_602_RmtSetFeeRateProfile,
	//number of msgs
	MAX_PUB_MSG_NUMBER,
	//this is just for msg parsing
	Msg_Unkown = -1,
}MQTT_MSG_TYPE;

//------------------------End---------------------------//

//-------------------Subscribe Topic Defination---------------------//
typedef enum emSubMsgTopicDefine
{
	SUB_MSG_TOPIC_S2C_Q401_Reboot = 0,
	SUB_MSG_TOPIC_S2C_Q402_LockSys,
	SUB_MSG_TOPIC_S2C_Q404_Upgrade,
	SUB_MSG_TOPIC_S2C_Q411_GetCfg,
	SUB_MSG_TOPIC_S2C_Q412_SetCfg,
	SUB_MSG_TOPIC_S2C_Q503_EmsVer,
	SUB_MSG_TOPIC_S2C_Q504_EmsPlan,

	MAX_SUB_MSG_NUMBER

}ENUM_SUB_MSG_TOPIC_DEF;

//------------------------End---------------------------//
typedef enum tagCloudReportType
{
	REP_TYPE_WEBSOCKET = 0,
	REP_TYPE_MQTT_TLS,
	REP_TYPE_MQTT_SELFTOPIC,

}CLOUDREP_INST_TYPE;

typedef enum tagTriggerReportType
{
	Trigger_NoTriggle, //just for identifying
	Trigger_C101,
	Trigger_C102,
	Trigger_C103,
	Trigger_C105,
	Trigger_C107,
	Trigger_C206,// edit by kuangyue 20250517
	Trigger_C114, // CXC40508  SEMS PCS Info
	
}TRIGGER_REPORT_MSG_TYPE;

typedef enum tagMsgSentType
{
	Call = 0,
	CallResult,
	CallError,
}MSG_CALL_TYPE;

typedef enum tagMqtt_CallErrorCode
{
	CallError_NotSupported = 101,
	CallError_FormatError = 102,
	CallError_InvalidData = 103,
	CallError_GenericError
}MQTT_CALLERROR_CODE;

typedef enum tagActiveMsgSrc
{
	CALL_SRC_Dummy = 0,
	CALL_SRC_Normal,
	CALL_SRC_InProcessTrans,
	CALL_SRC_HisTrans
}MSG_CALL_SRC;

typedef enum tagCommStatusCode 
{
	STATCODE_COMM_OK = 0,
	STATCODE_COMM_DISCONECT = 101,
	STATCODE_COMM_CONECTFAILURE,
	STATCODE_COMM_CONECTLOST,
 	STATCODE_COMM_REPLYTIMEOUT,
	STATCODE_COMM_FAIL_SERVER_LOSTCONN,
	STATCODE_COMM_FAIL_REASON_SERVERADDR_CHANGED,
	STATCODE_COMM_FAIL_REASON_SCRIBED_TIMEOUT,
	
}COMM_STATU_CODE;

typedef enum MQTT_COMM_STATUS
{
	MQTT_COMM_DISCONECTED = 0, 
	MQTT_COMM_CONNECTED,
	MQTT_COMM_SUBSCRIBING, //doing the subscribe when connct to broker
	MQTT_COMM_READY, //ready
	MQTT_COMM_WAIT_REBUILD_CLIENT, //means sth wrong that needs rebuild mqtt-client

}MQTT_COMM_STATUS;

typedef enum emTLSAuthLevel
{
	TLS_AUTH_None = 0,
	TLS_AUTH_Unidirectional,
	TLS_AUTH_Mutual,

}TLS_AUTH_LEVEL;

typedef enum emMQTTVersion
{
	VERSION_3 = 0,
	VERSION_5,

}MQTT_VERSION;
typedef struct tagSentMsgData
{
	MQTT_MSG_TYPE		emMsgType;
	MSG_CALL_SRC		emSrc;
	time_t				tmRelatedTransStart; //for identify diffent Trans session
	time_t				tmSampled;  
	int					nBelongedConn; //0 means conn0
	char				szJsonData[MAX_MQTT_BUFFER_LEN];

	time_t				tmLastErrorRespArrived; //last time received error msg fro CS
	int					nMsgErrorRecievedCount; //Error msg received count 
}MQTT_CALL_MSG;

/*
typedef struct tagActiveMsg
{
	#define MAX_MQTT_SEND_RETRIES	3
	MQTT_CALL_MSG		msgData;
	time_t				tmLastSent;
	UINT				nUniqMsgId[MAX_MQTT_SEND_RETRIES]; //manage it here to match req/rep
	int					nRetryTimes;

}MQTT_ACTIVE_CALL_MSG;
*/

typedef enum tagMQTTMsgSrc
{
	MQTT_MSG_C2S = 0, //from CS
	MQTT_MSG_S2C, //from CP

}MQTT_MSG_SRC;

//typedef int (*MSG_HANDLER)(void* pArg, BOOL bSend_Later); //pArg point to the MQTT data structure, bSendLater means queue that msg!
typedef int (*MSG_HANDLER)(void* pArg);

typedef struct tagMQTTMsgInfo
{
	MQTT_MSG_TYPE		emMsgType;
	MQTT_MSG_SRC		emMsgSrc;
	int					nMsgId;
	MSG_HANDLER			pfnMsgHandler;
	time_t				tm_LastSend;
	time_t				tmLastArrived;
}MQTT_MSG_INFO;

//-------------------End------------------------//

typedef struct  strMqttDataRough
{
	//-----------------MQTT Public Relevant Defination-------------------//
	MQTTAsync					client;
	MQTTAsync_connectOptions	conn_opts;
	MQTTAsync_responseOptions 	resp_opts;
	MQTTAsync_SSLOptions 		sslopts;
	MQTT_MSG_INFO				stMsgInfoList[MAX_PUB_MSG_NUMBER];
	COMM_STATU_CODE				emDisCnt_Reason;
	//-------------------End-------------------------//

	HANDLE						hReporter;
	HANDLE						hqMsg2Sent;
	HANDLE						hMutex;
	
	//MQTTClient				client;
	//MQTTClient_SSLOptions		ssl_opts;
	USER_COMM_STATUS			emUserCommStatus;

 	MQTT_COMM_STATUS			emMqttCommStatus;
	#define MQTT_FAIL_REASON_SCRIBED_TIMEOUT	102
	#define MQTT_FAIL_REASON_AUTH_TIMEOUT		103
	#define MQTT_FAIL_REASON_SN_CHANGED			104
	#define MQTT_FAIL_REASON_REFUSED			105
	#define MQTT_FAIL_SYSTEM_EXIT				106
	#define MQTT_FAIL_REASON_TOO_MANY_RETRY		108
	#define MQTT_FAIL_SERVER_LOSTCONN			109
	#define MQTT_FAIL_PUB_ERROR					110
	#define MQTT_FAIL_REASON_SERVERADDR_CHANGED	111
	int							emMqtt_Disconn_Reason;
	time_t						tmLastSend_DisConn;
	time_t						tmLastSend_Conn;
	time_t						tmLast_CallSubscribe;
	BOOL						bIsMqttConn_Ready;
	BOOL						bIs_MqttConnOK; //connection OK for mqtt
	int							nConnFailedTimes;
	
	BOOL						bTopic_Subscribed;
	BOOL						bNeedRefreshActvAlm;
	UINT						nActiveAlmTotal;
	EVENT_ALARM					activeAlms[MAX_QUERY_ACTV_ALM];

	CLOUDREP_INST_TYPE			emRepInstType;

	UINT						nAlarmInfosTotal;
	ALARM_INFO					*pAllAlarmInfo;

	TRIGGER_REPORT_MSG_TYPE 	emTriggerMsgType;

	UINT						nCurrDataFrom; //0-Master, 1-Slave1,2-Slave2
	UINT 						nSlvCommStatu[MAX_SUPPORTED_CLIENTS];
	UINT 						nConnectSlvNum;

	BYTE						szReadBuffer[MAX_MQTT_BUFFER_LEN];
	int							nReadBuffLen;

	char						szSendBuffer[MAX_MQTT_BUFFER_LEN];
	int							nSendBuffLen;

	char						szCompressSendBuff[MAX_MQTT_BUFFER_LEN];
	int							nCompressSendBuffLen;

	cJSON						*pCurrJson;

	DIAGNOS_UPLOAD_STATUS		emDiagnosUploadStatus;
	FIRM_UPDATESTT				emFirmwareUpgradeStatus;
	
	A_BLOB_ES_BACKEND_CFG		blobBackendSet;

 	//MQTT_ACTIVE_CALL_MSG		activeCallMsg; //shall always be call type
	char						szHandlingUniqID[36]; // this only for the Response for Server Requested Messages 

	char                    	szServerUrl[255]; //Host URL
	char                    	szClientID[64];   //Client ID

	char                    	szUserName[32];	
	char                    	szPassWord[32];

 	char                    	szPubTopic[MQTT_TOPIC_SIZE];
	char                    	szSubTopic[MQTT_TOPIC_SIZE];
	char                    	szManualPubTopic[MQTT_TOPIC_SIZE];
	char                    	szManualSubTopic[MQTT_TOPIC_SIZE];

	char						szSelfTopic[32];
	UINT						emSysStatus_For101;
	
	UINT						nCfgBMSNum;
	BOOL						bZlibCompress;

	UINT						nCurrBMS; //0-1-2-3-4

	TLS_AUTH_LEVEL				emAuthLevel;

	UINT 						emApplyArea;

	MQTT_VERSION				emMQTTVersion;

	char 						szShieldAlarms[512];
	UINT* 						ShieldAlarms;
	UINT 						nShieldAlarmsNum;
	LCCfg                       lcCfg; // LJH250428 cmd

	char						szSelfTopic1[32];
	GH_MQTT_INFO				stGHmqttInfoList[MAX_GH_MQTT_TYPE];
	GH_SETCMD_INFO				stGHSetCmdIndoList[MAX_GH_SETCMD];
	char                    	szGH_SubTopic[3][MQTT_TOPIC_SIZE];
	CUSTOM_TYPE					emCustomType; 
	AIR_TYPE 					emAirType;
	COMTROL_MODE				emControlMode;	
	UINT						nSendTimeInterval;
	UINT						nRemoteControlTime; 
	BOOL 						bDisPower; 
	time_t						tSendDisPower;

	BOOL						TimeCalibration;
	A_BLOB_SEMS_CLIENTINFO_SUM		clientInfoSum;
}MQTT_DATA_ROUGH;

//---------Mqtt Rept Function Defination---------//

void Prepare_SubTopicMsgName(IN MQTT_DATA_ROUGH *pMqtt);
//void Pack_PublishTopicMsg(MQTT_MSG_TYPE emPubMsgNo, MQTTClient_message *pMsg);

//------------MQTT Public Function---------------//
void Send_Call(MQTT_DATA_ROUGH* pMqtt, cJSON* pJson, MQTT_MSG_TYPE msgType, 
			MSG_CALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent);

void Send_CallError(MQTT_DATA_ROUGH* pMqtt, MQTT_MSG_TYPE msgType, 
					MQTT_CALLERROR_CODE emCode, const char* szErrDescrip);

void QueryActvAlarmInfo(MQTT_DATA_ROUGH* pMqtt);

time_t  Parse_MqttTime(char *pszOut);
char* Make_MqttTimeString(MQTT_DATA_ROUGH* pMqtt, time_t tmInout, char* pszOut);
char *PackMqtt_CallFrame(MQTT_DATA_ROUGH* pMqtt, MQTT_CALL_MSG *pMsg);

int Publish_MqttMessage(MQTT_DATA_ROUGH *pMqtt);

unsigned int  myAtoi(char *str);

//----------------Mqtt Msg Handler----------------//
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

int Prepare_C200_RTPowerInfo(void* pArg);

int Prepare_C202_HourSectorStaticKwh(void* pArg);
int Prepare_C204_DailyStaticKwh(void* pArg);
int Prepare_C206_SharpPeakFlatValleyKwh(void* pArg);


int Handle_S401_Reset(void* pArg);
int Handle_S402_ChangeEVSEAvailability(void* pArg);
int Handle_S403_RemoteSOCManualCalibration(void* pArg);
int Handle_S404_UpdateFirmware(void* pArg);
int Prepare_C405_FirmwareStatusNotification(void* pArg);
int Handle_S406_GetDiagnostics(void* pArg);
int Prepare_C407_DiagnosticsStatusNotification(void* pArg);

int Handle_S411_GetConfiguration(void* pArg);
int Handle_S412_ChangeConfiguration(void* pArg);

int Handle_S503_GetEMSPlan(void* pArg);
int Handle_S504_SetEMSPlanProfile(void* pArg);

int Handle_S601_GetFeeRate(void* pArg);
int Handle_S602_SetFeeRateProfile(void* pArg);

int send_SEMS_AlarmStatusChanged(void* pArg);
int Prepare_SEMS_C103_ESBMSStatusNotif(void* pArg);
int Prepare_SEMS_C103_ESBMSStatusNotif_Electra(void* pArg);
int Prepare_SEMS_C104_PwrDistrStaNotif_Electra(void* pArg);
int Prepare_SEMS_C110_BatDataNotif(void* pArg);
int Prepare_SEMS_C114_PCSUnitInfo(void* pArg);
int Prepare_SEMS_C115_SummaryInfo(void* pArg);
int Prepare_SEMS_C202_HourSectorStaticKwh(void* pArg);
//-----------------------End----------------------//

void Init_GH_MqttMsgInfo(MQTT_DATA_ROUGH *pMqtt);
void Pub_GH_MsgToServer(MQTT_DATA_ROUGH *pMqtt);
int EMU_PowerCmd(void* pArg, cJSON *json);
int EMU_dispatchPower(void* pArg, cJSON *json);
int EMU_powerLimit(void* pArg, cJSON *json);

#define DUMP_ESiOCPPMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif

