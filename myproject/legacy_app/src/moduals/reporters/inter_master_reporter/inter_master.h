#ifndef __REPORTER_INTER_MASTER_H
#define __REPORTER_INTER_MASTER_H
#include "std_sys.h"
#include "app_incs.h"
#include "data_model/blob_model.h"
#include "3rd_party/zlib/zlib.h"


#define TCP_BUFF_LEN 						8192
#define  MIN_MST_FRM_LEN					10

#define MAX_TRANS_RELATED_MSG_QUEUE_SIZE	1000
#define MAX_MST_EVENT_Q_SIZE				50

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


typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

typedef enum tagClientConnectStatus
{
	CLIENT_STATU_NOT_INSTALLED=0,
	CLIENT_STATU_INSTALLED,
	CLIENT_STATU_COMM_OK,
	CLIENT_STATU_COMM_FAIL,

}CLIENT_CONN_STATU;


typedef enum tagMasterMsgType
{
	_M_C2S_CFGNTF_MSG = 0,
	_M_C2S_SYSINFO_MSG,
	_M_C2S_BMS_MSG,
	_M_C2S_ENGFLOW_MSG,
	_M_C2S_SIGRECT_MSG,
	_M_C2S_BATTCAB_MSG,
	_M_C2S_STATISDATA_MSG,

	_M_S2C_SETCFG_MSG,
	_M_S2C_CTRL_MSG,
	_M_S2C_APPSIZE_MSG,
	_M_S2C_APPUPD_MSG,
	_M_S2C_CHKSUM_MSG,

	_M_MAX_NUM_MSG,

}MASTER_MSG_TYPE;

typedef union tagSlaveSysAlarm
{
	USHORT	usFlag;
	struct
	{
		USHORT	DoorOpen : 1;
		USHORT	CRCommFail : 1;
		USHORT	EPO : 1;
	}Alarm;
}SLAVE_SYS_ALARM_MAP;

typedef enum tagSlvConnectors
{
	Slv_Connctor_1 =0,
	Slv_Connctor_2,
	Slv_Connctor_3, 	//For Reserve
	Max_Slv__Connctors
}SLAVE_CONNECTORS;


typedef struct tagSlvConnectorData
{
	//CP_CONNECTOR_STATUS		emConnctorStauts;
 

 	int							nGunChargeStatus; //from sampler raw data;
	int							nGunPlugStatus; //from sampler raw data;
	//int						nGunPrepareStatus; //from sampler raw data
	//int						nGunSuspendEVSESatus; //from sampler raw data
 

	//UINT						nCancelReservationId;

	//HANDLE					hqInProcessTrans; //store a real time charge record
	//TRANSMSG_HANDLE_STATUS	emInProcessTransMsgHandleStatus;
 	
	A_BLOB_STBATT_CHARGE_CTRL_CMD blobTwoWayChrgCmd;
	//BOOL						bConn_StatusChanged;
	//time_t					tmLastConn_StatusChange;

}SLAVE_CONNECTOR_DATA;


typedef int (*S_MST_MSG_HANDLER)(void* pArgs, OUT BYTE* pOrigData, IN USHORT nDataLen);
typedef int (*R_MST_MSG_HANDLER)(void* pArgs, IN BYTE* pOrigData,  OUT USHORT nResult);
typedef struct tagMasterMsgFrame
{
	MASTER_MSG_TYPE			emMsgType;
 
	BYTE					bySCmdNo;
	BYTE					byRCmdNo;

	S_MST_MSG_HANDLER		pfnSHandler;//每个会话都有发的处理函数和收的处理函数/**/
	R_MST_MSG_HANDLER		pfnRHandler;
 
	BOOL					bHasSubCmd;

}MST_MSG_FRM_INFO;

//frame format: SOI-1 SOI-1 SRC-1 DST-1 CMD-1 [LEN-2] [DATA-x] [CHKSUM-2] EOI-1
#define MST_PROT_SOI		0x7E
#define MST_PROT_EOI		0x0D
#define MST_SRC_ADDR		0xFF
#define MST_NTF_CMD			1
typedef struct taMasterMsgFrame
{
	BYTE	bySrcAddr;
	BYTE	byDestAddr;

	BYTE	byCmd;
	UINT	ulDataLen;
	
	BYTE	byData[TCP_BUFF_LEN - MIN_MST_FRM_LEN];

	USHORT 	ulCalcSum;

}A_MST_FRM_INFO;

typedef enum tagMasterSyncStatu
{
	MASTER_SYNC_START = 0,
	MASTER_SYNC_PENDING ,
	MASTER_SYNC_PENDING_EMSCFG ,
	MASTER_SYNC_PENDING_BATTMGMT ,

	MASTER_SYNC_FINISHED ,

}MASTER_SYNC_STATU;

typedef struct tagInterClient
{
	HANDLE					hMutex;
	HANDLE					hqMsg2Sent;

	int 					nClientAddr;	//分体柜的实际物理地址，EXP：从1-(addr=1)，从2-(addr=2)
	int						nMapClientAddr; //该地址是映射到的线程，通过该地址，可以找到对应通信的线程

	HANDLE					hDataComm;
	
	CLIENT_CONN_STATU	 	emStatus;
	HANDLE  				hqMsgIn; //form master ctrl/Set

	BYTE					szReadBuffer[TCP_BUFF_LEN];
	int						nReadBuffLen;

	MST_MSG_FRM_INFO		msgFrameInfo[_M_MAX_NUM_MSG];

	//A_BLOB_ES_iOCPP_CONFIGURAION	stESiOCPPSettings;

	A_BLOB_SYNC_SLAVE_CONFIG		blobSyncConfig;

 
	int							emSysStatus4Slave;

	SLAVE_SYS_ALARM_MAP			slvSysAlarmMap;

	SLAVE_CONNECTOR_DATA	stConnectorData[Max_Slv__Connctors];

	int						nStopReason;

	MASTER_SYNC_STATU 		emSyncStatus;
	int 					nSyncTimes;

	MASTER_UPGRADE_PARAM	tagMstUpgradeParam;

	UPDOWN_FILE_INFO 		strUpdownFileInfo;

	BYTE 					szAuthCardID[64];
	BOOL 					bHasCardAuth;
	time_t 					t_LastRecvCardAuth_Sec;

	int 					nCommFailTimes;
	BOOL 					bNeedExit;
	REPORTER* 				pReporter;
}INTER_CLIENT;
 


typedef struct tagMasterSentEvent
{
	MASTER_MSG_TYPE			emOutMsgType;
	//int					emOutMsgType;//if timeout event, set to -1
	double					tmEventOccur;
	BYTE					byData[TCP_BUFF_LEN]; 
	int						nDataSize;

}MASTER_SEND_EVENT;

/*--------主柜执行分体柜的请求命令--------*/
int ExcuteCfgNtfCmd01(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteSysInfoCmd03(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);
int ExcuteBmsCmd05(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);		
int ExcuteEngFlowCmd07(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);
int ExcuteSigRectCmd09(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteBattCabCmd0B(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteStaticDataCmd0D(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);

/*--------主柜回复分体柜的请求命令--------*/
int PrepReplyCfgNtfCmd02(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplySysInfoCmd04(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyBmsCmd06(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyEngFlowCmd08(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplySigRectCmd0A(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyBattCabCmd0C(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyStatisDataCmd0E(void* pArgs, OUT BYTE* pData, IN USHORT nResult);

/*--------主柜准备控制分体柜的命令--------*/
int PrepareSetCfgCmdA1(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareCtrlCmdA3(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareAppSizeCmdA5(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareAppUpdCmdA7(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
 
/*--------主柜解析控制分体柜的回复命令--------*/
int ExtractSetCfgCmdA2(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractCtrlCmdA4(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractAppSizeCmdA6(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractAppUpdCmdA8(void* pArgs, IN BYTE* pData, OUT USHORT nResult);

BOOL Send_MST_Frame(INTER_CLIENT *pClient,  MASTER_MSG_TYPE emMsgTp, BYTE* pData, USHORT nDataSize);

ULONG Bytes2Ulong(BYTE* pBytes, int nBytes);
void MakeUlong(ULONG ulVal, OUT BYTE* pData);
void MakeShort(short sVal, OUT BYTE* pData);
#endif