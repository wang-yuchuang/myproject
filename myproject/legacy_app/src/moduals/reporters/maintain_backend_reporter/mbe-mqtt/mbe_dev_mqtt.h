#ifndef __REPORTER_MBE_DEV_MQTT_H
#define __REPORTER_MBE_DEV_MQTT_H

#include "std_sys.h"
#include "app_incs.h"
#include "../../../cJSON.h"
#include "3rd_party/libpahomqtt/include/MQTTClient.h"

#define MBE_SERVER_TOPIC_PREFIX				"mbe/ser"
#define MBE_DEVICE_TOPIC_PREFIX				"mbe/dev"
#define MBE_MQTT_CONNET_TIMEOUT				15 //30
#define MBE_DEV_SUPPORT_MAX_CFGPARM_ITEMS	200	
#define MBE_DEV_SUPPORT_MAX_MULTISET_ITEMS	10	//for multi-set from mbe-server
#define ONE_HOUR_SEC						3600

#define MBE_PARAM_MASK_VALID          		0x01


typedef enum 
{
	//broadcast
	MBE_MSG_BRSD_ALL = 0, //broad -> all
	MBE_MSG_BRSD_ME, //broad -> me
	//auth
	MBE_MSG_AUTH_OK,
	MBE_MSG_AUTH_NOK,
	//trig.stat
	MBE_MSG_TRIG_STAT,
	//cfg 
	MBE_MSG_CFG_REQ,
	//param 
	MBE_MSG_PARAM_REQ,
	//set 
	MBE_MSG_SET_REQ,
	//ctrl 
	MBE_MSG_CTRL_REQ,
	//error
	MBE_MSG_ERR_SERVER,

	//number of sub msgs
	MAX_NUM_MBE_MSG_SUB,
}MBE_DEV_SUB_MSGID;

typedef enum 
{	
	//auth
	MBE_MSG_AUTH_REQ = 0,
	//status
	MBE_MSG_STATUS,
	//cfg 
	MBE_MSG_CFG_CONF,
	//param
	MBE_MSG_PARAM_CONF,
	//set
	MBE_MSG_SET_CONF,
	//ctrl
	MBE_MSG_CTRL_CONF,
	//error
	MBE_MSG_ERR_DEV,

	//number of pub msgs
	MAX_NUM_MBE_MSG_PUB
}MBE_DEV_PUB_MSGID;

typedef int (*MSG_HANDLER)(void* pArg);
typedef enum
{
	QoS_0,
	QoS_1,
	QoS_2
}MQTT_QoS;

typedef struct 
{
	//msg info
	MBE_DEV_SUB_MSGID	emMsgId;
	char 				szMsgName[16];
	MQTT_QoS			qos;
	MSG_HANDLER			pfnParser;
	char 				szTopicName[128]; //for subscribe filter
	//MQTTAsync_token 	msgToken; //for judging the msg sent OK
	//runtime info
	BOOL 				bIsSubscribedOK;
	time_t				tmLastArrived;
	//[senderId] just for some special cmd, eg set/ctrl
	//BOOL				bHasSenderId;
	//char 				szSenderId[64]; 
}MBE_DEV_SUB_MESSAGE;

typedef struct 
{
	MBE_DEV_PUB_MSGID			emMsgId;
	char 						szMsgName[16];
	MQTT_QoS					qos;
	MSG_HANDLER					pfnHandler;	
	//MQTTAsync_token 			msgToken; //for judging the msg sent OK
	char 						szTopicName[128]; //for subscribe filter
	time_t						tmLastSent;
	time_t						tmLastPubOK;
}MBE_DEV_PUB_MESSAGE;

typedef enum  
{
	MBE_COMM_DISCONECTED = 0, 
	MBE_COMM_CONNECTED,
	MBE_COMM_SUBSCRIBING, //doing the subscribe when connct to broker
	MBE_COMM_WAIT_AUTH,
	MBE_COMM_AUTHING, //doing the auth
	MBE_COMM_AUTH_REJECTED,
	MBE_COMM_READY, //ready
	MBE_COMM_WAIT_REBUILD_CLIENT, //means sth wrong that needs rebuild mqtt-client
}MBE_COMM_STATUS;

typedef struct 
{
	int		nAlarmId;
	int		nDevId;
	time_t	tmStart;
}MBE_DEV_ALARM;

typedef struct 
{
	char 			szVerInfo[50];
	int				emWorkStatus; //0-idle, 1~10-work mode, 11-fault, 12-disabled, 13-sw update in process
	int				emRemoteCmd_ExeStat; //0-idle, 1-ftp downloading, 2-ftp uploading, 3-device rebooting, 4-cmd executing
	int     		emFaultyStat; //0-no fault, 1-fault(unavailable), 2-fault(available), 3-alarm(available)
#define MBE_DEV_MAX_SUPPORT_ACTALM	50
	UINT 			nActiveAlmTotal; //0 means NO alarm
	MBE_DEV_ALARM	actAlmTable[MBE_DEV_MAX_SUPPORT_ACTALM];
	int				emVPNStat; //0-idle, 1-started but no connection, 2-started and connected OK, 99-not supported

}MBE_MQTT_STATUSMSG_DATA;

typedef enum
{
	MBEDEV_ERRORMSG_CODE_UNKOWNMSG = 0,
	MBEDEV_ERRORMSG_CODE_DATAERR
}MBEDEV_ERRMSG_ERRTYPE;

typedef struct 
{
	int			nDev;
#define MBE_SIGTYPE_ALARM	3
	int			emSigType; //0-status, 1-ctrl, 2-set, 3-alarm
	_VALUE_TYPE	emValTYpe;
	UINT		nValueSize;
	int 		nFloatPoint; //decimal point
	int			nSigId;	
	//following are for blob
	int			nBlobId; //reuse precision
	BOOL 		bIsSubSig;
	int			nSigSubId;
}MBE_MQTT_ASIG_KEY;

typedef struct 
{
	char				szRawKeyName[32]; //to save it in case not parsed OK
	BOOL 				bIsParsedOK; //false means invalid param
#define MBEDEV_EXESET_RESULT_NONE		0
#define MBEDEV_EXESET_RESULT_SUCCESS	1
#define MBEDEV_EXESET_RESULT_FAILED		2
#define MBEDEV_EXESET_RESULT_WRONG		3
	int					nExecuteResult; //0-Not-exet-yet, 1-exeOK, 2-exeFail, 3-wrongFormat
	MBE_MQTT_ASIG_KEY	key;
	char				szVal[256];
}MBE_MQTT_ASIG_KEY_VAL;

typedef enum 
{
	MBEDEV_CTRLCMD_EVENTTYPE_NONE,
	MBEDEV_CTRLCMD_EVENTTYPE_SOFTRESET = 1,
	MBEDEV_CTRLCMD_EVENTTYPE_HARDRESET,
	MBEDEV_CTRLCMD_EVENTTYPE_DISAB_SYS,
	MBEDEV_CTRLCMD_EVENTTYPE_DISAB_CONN,
	MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPDTFW,
	MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPLDLOG,
	MBEDEV_CTRLCMD_EVENTTYPE_VPN_START,
	MBEDEV_CTRLCMD_EVENTTYPE_VPN_STOP,
	MBEDEV_CTRLCMD_EVENTTYPE_RESET_CCU,

	MBEDEV_CTRLCMD_EVENTTYPE_MAX

}MBEDEV_CTRL_EVENTTP;
typedef struct 
{
	MBEDEV_CTRL_EVENTTP		emEventType;
	int						nParamTotal; //max support 2params
	char					szParams[2][256];

}MBE_MQTT_ACTRLCMD;

typedef struct 
{
	//handle
	HANDLE						hReporter;
	//HANDLE						hqMsg2Sent;
	HANDLE						hMutex;
	BOOL						bIsNoticed_Exit;

	//for mqtt lib
	MQTTClient 						client;
	// MQTTAsync_connectOptions 	conn_opts;
	// MQTTAsync_SSLOptions 		sslopts;

	//for mqtt auth
	char 						szMqttClient_UserName[32];
	char 						szMqttClient_Passwd[32];

	//msg definitions
	MBE_DEV_SUB_MESSAGE			stSubMsgList[MAX_NUM_MBE_MSG_SUB];
	MBE_DEV_PUB_MESSAGE			stPubMsgList[MAX_NUM_MBE_MSG_PUB];

	//topic filters
	//MBE_DEV_SUB_TOPIC			stSubTopicFilters[BE_DEV_SUBTOPIC_MAXNUM];	

	//SN/mac for topic
	char 						szDevSN[32];
	char 						szDevMAC[18];
	char 						szCurrClientId[64]; //used for identify all the session, do NOT change it during mqtt connection
	time_t						tmLast_CallSubscribe; //last time call subscribe
	char 						szMqttServerAddress[32];
	BOOL						bIsMqttVersion5; //check if using Mqtt5

	//for dev-Auth
	int 						nAuthReject_Times;

	//comm status
	MBE_COMM_STATUS				emMqttCommStatus;
#define MBEMQTT_COMM_FAIL_REASON_SCRIBED_TIMEOUT	102
#define MBEMQTT_COMM_FAIL_REASON_AUTH_TIMEOUT		103
#define MBEMQTT_COMM_FAIL_REASON_SN_CHANGED			104
#define MBEMQTT_COMM_FAIL_REASON_REFUSED			105
#define MBEMQTT_COMM_FAIL_SYSTEM_EXIT				106
#define MBEMQTT_COMM_FAIL_REASON_TOO_MANY_RETRY		108
#define MBEMQTT_COMM_FAIL_SERVER_LOSTCONN			109
#define MBEMQTT_COMM_FAIL_PUB_ERROR					110
#define MBEMQTT_COMM_FAIL_REASON_SERVERADDR_CHANGED			111
	int							emOCPP_Disconn_Reason;
	time_t						tmLastSend_DisConn;
	time_t						tmLastSend_Conn;
	BOOL						bIsMqttConn_Ready;
	BOOL						bIs_MqttConnOK; //connection OK for mqtt
	int							nConnFailedTimes;

	//for status message
	FIRM_UPDATESTT				emFirmwareUpgradeStatus;
	DIAGNOS_UPLOAD_STATUS		emDiagnosUploadStatus;
	BOOL						bIsStatusChanged;
	UINT 						nStatusReport_Interval; //in min(default 30min)
	MBE_MQTT_STATUSMSG_DATA		stCurrStatusData;

	time_t 						tmLastSent_RemoteExeCmd_Busy; //last time sent staus when remoteExeStatus = busy
	time_t						tmLastSentTrigStatus; //last time sent server the Status for trig.stat

	//when received server maint-msg, we just pause the mqtt publish
	BOOL						bIsNeed_Pause_Publish;

	//for buffer
	cJSON						*pCurrJson_Sub; //just subscribed msg json
	MBE_DEV_SUB_MSGID			emInHandling_SubMsg;

	//pending param-set
	BOOL						bHasPending_Params_2Set;
	int							nPending_Params_Count;
	MBE_MQTT_ASIG_KEY_VAL		stPendingParams_KeyValList[MBE_DEV_SUPPORT_MAX_MULTISET_ITEMS];
	char 						szSenderId_4PendingParamSet[256]; 

	//pending remote-ctrl
	BOOL						bHasPending_CtrlParam;
	MBE_MQTT_ACTRLCMD			stPendingCtrlCmd;
	char 						szSenderId_4PendingCtrl[256]; 
	BOOL						TimeCalibration; 
	time_t						tTimeCalibration;
}MQTT_MBE_DATA_ROUGH;


//dev report msg
int Prepare_Topic_AuthReq(void* pArg);
int Prepare_Topic_Status(void* pArg);
//int Prepare_Topic_Error(void* pArg);
//dev handle msg
int Handle_Topic_Brsd(void* pArg);//should hadle brsd-all and brsd-me together
int Handle_Topic_AuthOK(void* pArg);
int Handle_Topic_AuthNOK(void* pArg);
int Handle_Topic_TrigStat(void* pArg);
int Handle_Topic_CfgReq(void* pArg);
int Handle_Topic_ParamReq(void* pArg);
int Handle_Topic_SetReq(void* pArg);
int Handle_Topic_CtrlReq(void* pArg);
int Handle_Topic_ServError(void* pArg); //server side error

//publish a message(json)
void Publish_AMqttJsonMsg(MQTT_MBE_DATA_ROUGH* pMbeDev, MBE_DEV_PUB_MSGID emPubMsgId, cJSON* json);

//for sub-blob
char* Get_ASubParmaVal_FromKey(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY* pSigKeyInfo, OUT char* pszVal);
int Set_ASubParamVal_FromKeyVal(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY_VAL* pKeyVal, int nTimeOut_mSec);

//for set/ctrl
int Execute_AParamSet_Item(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY_VAL* pParamKeyVal, int nTimeOut_mSec);
void ExeCute_DevCtrlCmd(MQTT_MBE_DATA_ROUGH* pMqtt);
BOOL Judge_IfNeed_SendStatus(REPORTER* pReporter);

time_t  Parse_MBETime(char *sztime);

#endif

