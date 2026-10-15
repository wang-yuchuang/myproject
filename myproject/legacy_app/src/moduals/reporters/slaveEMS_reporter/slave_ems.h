#ifndef __SLAVE_EMS_H
#define __SLAVE_EMS_H

#include "std_sys.h"
#include "app_incs.h"
#include "../reporters_base.h"
#include "../../../include/data_model/protocol_model.h"

#define TCP_BUFF_LEN				1024*10

#define NLMSG_FRME_MIN_LEN 18

#define  NLMSG_SOI		0xAA7EAA
#define  NLMSG_EOI		0x0D
#define  NLMSG_MIN_LEN	9


#define  NLMSG_SEMS_CFGG_CMD	0x61
#define  NLMSG_SEMS_CFGS_CMD	0x62
#define  NLMSG_SEMS_UCTL_CMD	0x63
#define  NLMSG_SEMS_RTIF_CMD	0x64
#define  NLMSG_SEMS_PCS_CMD		0x65
#define  NLMSG_SEMS_USYS_CMD	0xA6
#define  NLMSG_SEMS_LSYS_CMD	0xB6
#define  NLMSG_SEMS_LSYS_CMD	0xB6
#define  NLMSG_SEMS_LDEV_CMD	0xC6

// typedef enum tagNLMsgType
// {
// 	NL_MSG_SEMS_CFGG = 0,
// 	NL_MSG_SEMS_CFGS,
// 	NL_MSG_SEMS_UCTL,
// 	NL_MSG_SEMS_RTIF,
// 	NL_MSG_SEMS_USYS,

// 	NL_MSG_Max
// }NL_MSGTYPE;

typedef enum tagNLMsgType
{
	NL_MSG_CFGG = 0,
	NL_MSG_CFGS,
	NL_MSG_UCTL,
	NL_MSG_RTIF,
	NL_MSG_GCAN,
	NL_MSG_PCSINFO,

	NL_MSG_PCSGET,
	NL_MSG_PCSSET,
	NL_MSG_READ_PCSSET,

 	NL_MSG_USIZ,
	NL_MSG_UDTA,

	NL_MSG_USYS,
	NL_MSG_LSYS,
	NL_MSG_LDEV,

	NL_MSG_Max
}NL_MSGTYPE;

typedef enum tagProtocolType
{
	PROTOCOL_TCP = 0,
	PROTOCOL_UDP,

	PROTOCOL_MAX
}PROTOCOL_TYPE;
typedef int (*NLMSG_FRAME_PARSER)(void* pArg, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);//bProtocol : 0-TCP 1-UDP
typedef struct tagNLMSGFrame
{
	NL_MSGTYPE	msgTp;
	//Frame Info
	BYTE				byCmd;
	NLMSG_FRAME_PARSER	pfnParser;
	USHORT				usMin_ExpectDataLen; //minimal expected data length
}NLMSG_FRAME_INFO;

typedef struct stRoughReportersSlaveData
{
	REPORTER*				pReporter;
	HANDLE					hMutex;
	NLMSG_FRAME_INFO		nlFrameInfo[NL_MSG_Max];
	char            		szStationEMSUrl[255]; //Host URL
	time_t					tmLast_Sent_LSYS;
	// time_t					tmLastRecv_USYS; //last time received USYS msg
	time_t					tmLastRecv; // LJH250625 收到任何server数据就认为通信成功
	UINT  					nisFirstBoot;

	UINT					Sent_SEMS_LSYS_Period;
	char 					szIMMU2_SWVer[MAX_LEN_MU2SWVER];
	UINT					nCfgBMSNum;
	UINT 					nConnectSlvNum; // edit by KYA 20250520
	UINT					nCommFailTimes;
	BOOL 					bHadRecvCFGS; //Registered with SEMS flag
	BOOL					nSemWithCCUAndSlave;

	// A_BLOB_SINGLE_STBATT_RUNINFO 	blobSingleBattData[MAX_SINGLE_STBATT_NUM];	//单个电池柜运行状态信息
	// A_BLOB_BRANCH_GRP_RECTDATA		blobBranchGrpRectData[MAX_SINGLE_STBATT_NUM];	//每个支路ACDC模块信息
	// A_BLOB_BRANCH_NODE_CALC_DATA 	blobnodeCalcData[MAX_SINGLE_STBATT_NUM];	//计算每个支路
	// A_BLOB_RECT_GRP_DATA			blobGrpRectData;
	A_BLOB_NETWORKINFO				blobNetWorkInfo;
	A_BLOB_ES_BACKEND_CFG		    blobBackendSet;
	
	SEMS_A6_USYS_DATA_INFO          A6_USYS_Last;
	// SEMS_B6_LSYS_DATA_INFO		strLSYSInfo;

	BOOL							bNeedRefreshActvAlm;
	
    int                 			nActiveAlmTotal;
    EVENT_ALARM     				activeAlms[MAX_QUERY_ACTV_ALM];

	int                 			nAlarmInfosTotal;
    ALARM_INFO      				*pAllAlarmInfo;

	// SEMS_B6_LSYS_BATT_GRP_INFO * pAllBattGrpData;

	//regiser info
	BOOL                         bIsRegistered; //tcp客户端连接上服务端标记
	time_t                       tmRegiseterd;
	// A_BLOB_SEMS_CLIENTINFO_SUM   clientInfoSum;

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
	LCCfg                           lcCfg; // LJH250508 LCCfg
	UINT							nCurrBMS;
	char 							szShieldAlarms[512];
	UINT* 							ShieldAlarms;
	UINT 							nShieldAlarmsNum;

	ESS_DEVICE_TYPE					emDevType;
	int 							iBattCellNumPerGroup;//每组电池的单体数量
}ROUGH_SLAVE_ROUGH;

void DebugTest(ROUGH_SLAVE_ROUGH* pSlave);
int Slave_WriteData(ROUGH_SLAVE_ROUGH* pSlave, BYTE *szDataBuff, BYTE CMD, int nSendDataLen, PROTOCOL_TYPE bProtocol);
int Slave_ReadData(ROUGH_SLAVE_ROUGH* pSlave, BYTE *rdBuff, int nReadLen);
USHORT MakeChkSum(BYTE byCmd, USHORT usDataLen, BYTE* byData);
void Send_SEMS_LSYS(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);
void Send_SEMS_LDEV_C6_101(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
void Send_SEMS_LDEV_C6_102(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
void Send_SEMS_LDEV_C6_103(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
void Send_SEMS_LDEV_C6_104(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
void Send_SEMS_LDEV_C6_155(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);
void Send_SEMS_LDEV_C6_157(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol); // LJH250520 cmd157
void Send_SEMS_LDEV_C6_106(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
void Send_SEMS_LDEV_C6_108(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by KYA 20250520
int Handler_SEMS_CFGG(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
int Handler_SEMS_CFGS(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
int Handler_SEMS_UCTL(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
int Handler_SEMS_RTIF(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
int Parse_Upgrade_Status(ROUGH_SLAVE_ROUGH* pSlave, FIRM_UPDATESTT UpgradeSTATUS, PROTOCOL_TYPE bProtocol);
int Handler_PCS_INFO(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
void Parse_AlarmLight_Status(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);// edit by Kuangyue 20251112

int Parse_SEMS_USYS(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol);
char* Make_IPV42String(ROUGH_SLAVE_ROUGH* pSlave, ULONG ulIpV4_IP, char* pszOut);



void Send_SEMS_LDEV_C6_109(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);
void Send_SEMS_LDEV_C6_110(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);
void Send_SEMS_LDEV_C6_202(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol);


#endif
