
#ifndef __INTER_SLAVE_MAIN_H
#define __INTER_SLAVE_MAIN_H

#include "../sampler_base.h"
#include "3rd_party/zlib/zlib.h"

//#define TRACE	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //
//#define TRACEX	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //

 /*------------Rough Slave Data define Start----------- */ 

#define  MIN_SLV_FRM_LEN	10
#define SLV_COMMFAIL_TIMES  20
#define INVALID_RECV_CMD	0x00
#define MAX_SLAVE_GUN_NUM	2

typedef enum tagSlvMsgType
{
	_S_C2S_CFGNTF_MSG = 0,
	_S_C2S_SYSINFO_MSG ,
	_S_C2S_BMS_MSG,
	_S_C2S_ENGFLOW_MSG,
	_S_C2S_SIGRECT_MSG,
	_S_C2S_BATTCAB_MSG,
	_S_C2S_STATISDATA_MSG,

	_S_S2C_SETCFG_MSG,
	_S_S2C_CTRL_MSG,
	_S_S2C_APPSIZE_MSG,
	_S_S2C_APPUPD_MSG,
	_S_S2C_CHKSUM_MSG,

	_S_MAX_NUM_MSG,

}SLV_MSG_TYPE;


typedef struct tagCCUUpgradeParam
{
	time_t 						tmLastEnterUpgrade;

	MASTER_UPGRADE_PARAM    	tagMstUpgradeParam;

	UPDOWN_FILE_INFO 			strUpdownFileInfo;

}CCU_UPGRADE_PARAM;


typedef union tagSlaveSysAlarm
{
	USHORT	usFlag;
	struct
	{
		USHORT	DoorOpen : 1;
		USHORT	CRCommFail : 1;
		USHORT	EPO : 1;
	}Alarm;
}SLV_SYS_ALARM_MAP;
 
typedef enum tagSLVConnectionStatus
{
	DIS_CONNECTED = 0,
	CONNECTED,
	SYNC_PENDING, 			//pending status can handle Configuration settings
	SYNC_SUCCESS, 			//SYNC done, now ready for other msgs
	SYNC_FAILED,

}SLV_CONN_STATUS;

typedef enum tagLSYSAvailStatus
{
	SYS_STATUS_Availible = 0,
	SYS_STATUS_Alarm,
	SYS_STATUS_Faulted,
	SYS_STATUS_SWUpdate

 }LSYS_AVAILBLESTATE;

typedef enum tagNLMSG_RTN
{
	_RTN_OK = 0,
	_RTN_EXECUTE_ERR,
	_RTN_INVALID_CMD,
	_RTN_WRONG_FORMAT,
	_RTN_WRONG_DATA
}NLMSG_RTN;

typedef struct tagSlaveSysStatus
{
 	LSYS_AVAILBLESTATE		emSlvSysStatus;
 	
	
 	
	time_t 					tmRTC;
}SLV_SYS_STATUS_INFO;

//frame format: SOI-1 SOI-1 SRC-1 DST-1 CMD-1 [LEN-2] [DATA-x] [CHKSUM-2] EOI-1
#define SLV_PROT_SOI		0x7E
#define SLV_PROT_EOI		0x0D
#define SLV_DST_ADDR		0xFF
typedef struct taSlvMsgFrame
{
	BYTE	bySrcAddr;
	BYTE	byDestAddr;

	BYTE	byCmd;
	UINT	ulDataLen;
	BYTE	byData[TCP_BUFF_LEN - MIN_SLV_FRM_LEN];
	USHORT 	ulCalcSum;

}A_SLV_FRM_INFO;

typedef int (*S_SLV_MSG_HANDLER)(void* pArgs, OUT BYTE* pOrigData, IN USHORT nDataLen);
typedef int (*R_SLV_MSG_HANDLER)(void* pArgs, IN BYTE* pOrigData,  OUT USHORT nResult);
typedef struct tagSlvMsgFrame
{
	SLV_MSG_TYPE				emMsgType;
 
	BYTE						bySCmdNo;
	BYTE						byRCmdNo;

	S_SLV_MSG_HANDLER			pfnSHandler;
	R_SLV_MSG_HANDLER			pfnRHandler;
 
	BOOL						bHasSubCmd;

}SLV_MSG_FRM_INFO;


typedef struct tagSlaveUpgradeParam
{
	HF_UPDOWN_STATUS		emUpdownStatus;	

    FILE            		*fp;

	BYTE        			szFileName[30];

	FIRM_UPDATESTT			emUpdateStatus;

	int						nRecvMsgId;

	int						nLastRecvMsgId;
	
	ULONG 					nTotalFileSize;

	ULONG					nRecvFileSize;
	
	time_t 					tm_StartUpgrade;

}SLV_UPGRADE_PARAM;


typedef struct stRoughSlaveData
{
	HANDLE		                    hSampler; //pointer to sampler
	HANDLE							hSamplerLock; //for ES-Slave only

	DATA_PROCESSOR*		            pDPR;

	SLV_MSG_FRM_INFO				msgFrameInfo[_S_MAX_NUM_MSG];
	
	SLV_SYS_STATUS_INFO				stSysStatus;

	SLV_SYS_ALARM_MAP				mapSlvSysAlarm;

 
	SLV_CONN_STATUS					emSLVConnStatus;

	SLV_UPGRADE_PARAM				tagSlvUpgradeParam;

	BYTE							bySlaveAddr;

	int								nCommReadFailTimes;
	int 							nCommWriteFailTimes;
 
	time_t							tmLastSent_SysInfo;  
	time_t							tmLastSent_SingleBMS;  
	time_t							tmLastSent_EngFlow;
	time_t 							tmLastSent_SigRect;
	time_t 							tmLastSent_BattCab;
	time_t 							tmLastSent_StaticData;

 
	A_BLOB_BRANCH_NODE_CALC_DATA 	nodeCalData[2];

	CCU_STATE 						emMasterCCUState;
	//Backup Battery DataInfo
	UINT 							nCfgBattNum;
	A_BLOB_SINGLE_STBATT_RUNINFO 	blobSingleBatt[MAX_SINGLE_STBATT_NUM];

	HIS_ES_STATISTIC_DAILY			hisESData; //daily statistic data for ES

}ROUGH_SLAVE_DATA;

/*----------Rough Slave Data Define End -----------*/

 
void Var_To4Bytes(IN VAR_VALUE varV, BYTE* pbyOut);

/*-------------------C2S Requst Cmd 分体柜向主柜请求命令-----------------------*/
int PrepareCfgNtfCmd01(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareSysInfoCmd03(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareBmsCmd05(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);	
int PrepareEngFlowCmd07(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareSigRectCmd09(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareBattCabCmd0B(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);
int PrepareStaticDataCmd0D(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen);

/*------------------------S2C 主柜应答分体柜的请求 ---------------------------*/
int ExtractCfgNtfCmd02(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractSysInfoCmd04(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractBmsCmd06(void* pArgs, IN BYTE* pData, OUT USHORT nResult);	
int ExtractEngFlowCmd08(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractSigRectCmd0A(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractBattCabCmd0C(void* pArgs, IN BYTE* pData, OUT USHORT nResult);
int ExtractStaticDataCmd0E(void* pArgs, IN BYTE* pData, OUT USHORT nResult);

/*--------------------S2C 主柜主动发送控制命令给分体柜 -----------------------*/
int ExcuteSetCfgCmdA1(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteCtrlCmdA3(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteAppSizeCmdA5(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteAppUpdCmdA7(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	
int ExcuteChkSumCmdA9(void* pArgs, IN BYTE* pData, IN USHORT nDataLen);	

/*---------------------S2C 分体柜响应主柜的控制请求命令 ----------------------*/
int PrepReplySetCfgCmdA2(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyCtrlCmdA4(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyAppSizeCmdA6(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyAppUpdCmdA8(void* pArgs, OUT BYTE* pData, IN USHORT nResult);
int PrepReplyChkSumCmdAA(void* pArgs, OUT BYTE* pData, IN USHORT nResult);

//void PushAllData(SAMPLER* pSampler);
BOOL PackAndWriteSession(SAMPLER* pSampler, SLV_MSG_TYPE emMsgTp, BYTE* pData, USHORT nDataSize);
BOOL ReadDataAndParse(SAMPLER *pSampler, IN int nTimeout_ms, IN BYTE byRecvCmd);

void PushAllSlaveData(SAMPLER* pSampler);

#endif


