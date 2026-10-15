
#ifndef __UDP_PUBLIC_MAIN_H
#define __UDP_PUBLIC_MAIN_H

#include "../sampler_base.h"
#include "../../cJSON.h"

//#define TRACE	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //
//#define TRACEX	printf("\n[%s:%d] [%ld]--",__FUNCTION__, __LINE__, time(NULL)), printf //1 ? (void)0 : (void)printf //

#define UDP_MAX_SENTMSG_SIZE	65536
#define MAX_WAITSLV_SEC 		30
#define MAX_COMMFAIL_TIMES		10

#define TIMEOUT_60_SEC			60

#define DEVID_BLOBMETER_CAB		790
#define DEVID_BLOBMETER_GRID	791
#define DEVID_BLOBMETER_AUX1	792
#define DEVID_BLOBMETER_AUX3	793
#define DEVID_BLOBMETER_OFFGRID	794

#define DEVID_BLOBMETER_SLV0	800
#define DEVID_BLOBMETER_SLV1	801
#define DEVID_BLOBMETER_SLV2	802
#define DEVID_BLOBMETER_SLV3	803
#define DEVID_BLOBMETER_SLV4	804
#define DEVID_BLOBMETER_SLV5	805
#define DEVID_BLOBMETER_SLV6	806
#define DEVID_BLOBMETER_SLV7	807
#define DEVID_BLOBMETER_SLV8	808
#define DEVID_BLOBMETER_SLV9	809

#define DEVID_SIGLMETER_CAB     407
#define DEVID_SIGLMETER_GRID    408
#define DEVID_SIGLMETER_AUX1    409
#define DEVID_SIGLMETER_AUX3    410

//---------------- Define Public ---------------------//
typedef enum emUdpMsgSrc
{
	UDP_MSG_C2S = 0,
	UDP_MSG_S2C,

}UDP_MSG_SRC;

typedef enum tagMsgCallType
{
	Q_RECV = 0,
	R_REPLY,
	E_ERROR,

}UDPMSG_CALL_TYPE;

typedef int (*MSG_HANDLER)(void* pArg);

//-------------------- End ---------------------------//


//---------------- Define Udp Client -----------------//
typedef enum emUpdClientCmd
{
	S_10X_METERVALUE_CLIENT = 0, 	//Send
	R_100_METERVALUE_CLIENT,		//Recv
	R_400_EMSMGMT_CLIENT,			//Recv

	Max_MSG_CLIENT,

	MSG_CLIENT_Unkown = -1,

}UDP_CLIENT_CMDTP;


typedef struct tagClientMsgInfo
{
	UDP_CLIENT_CMDTP		emMsgType;
	UDP_MSG_SRC				emMsgSrc;
 	UINT					nMsgId;
	MSG_HANDLER				pfnMsgHandler;

}CLIENT_MSG_INFO;
//--------------------- UdpClient End ----------------------//


//------------------- Define Udp Server --------------------//
typedef enum emUpdServerCmd
{
	S_100_METERVALUE_SERVER = 0, 	//Send
	R_10X_METERVALUE_SERVER,		//Recv

	S_400_EMSMGMT_SERVER, 			//Send

	Max_MSG_SERVER,

	MSG_SERVER_Unkown = -1,

}UDP_SERVER_CMDTP;

typedef struct tagServerMsgInfo
{
	UDP_SERVER_CMDTP		emMsgType;
	UDP_MSG_SRC				emMsgSrc;
 	UINT					nMsgId;
	MSG_HANDLER				pfnMsgHandler;

}SERVER_MSG_INFO;

//--------------------- UdpServer End ----------------------//

//------------ Public Rough Udp Rough Data define Start----------- // 
 
typedef struct stUdpRoughData
{
	HANDLE		                    hSampler; //pointer to sampler
	HANDLE							hSamplerLock;  

	DATA_PROCESSOR*		            pDPR;
	
	ESSYS_TYPE 						emESSystemType;
	
	A_BLOB_NETWORKINFO				blobNetWorkInfo;  

	CLIENT_MSG_INFO					stMsgList_Client[Max_MSG_CLIENT];
	
	SERVER_MSG_INFO					stMsgList_Server[Max_MSG_SERVER];

 	char							szReadBuffer[UDP_MAX_SENTMSG_SIZE];
	int								nReadBuffLen;

	char							szSendBuffer[UDP_MAX_SENTMSG_SIZE];
 	int								nSendBuffLen;

	UINT							uiHandlingUniqID;
	cJSON							*pCurrJson;
 
	BOOL							bHasActiveMsg;	//if yes, need wait if finish before other calls to go.
	int								nConnErrTimes;
	
	UINT 							nConfigSlvNumb;
	time_t							tmLastSuccessExchange[ESSTYPE_MAX];
	UINT							nCommFailedTimes[ESSTYPE_MAX];
	BOOL							bRegisted[ESSTYPE_MAX];

	float 							fKWh_Pos[ESSTYPE_MAX];

	UINT 							nCommFailTimes;
	BOOL 							bCommFail;

	A_BLOB_EMS_CONFIG_PLAN 			emsPlan;

	//Used For Pgrid_Sum Value is Changed.
	A_BLOB_METER_VALUES 			blobMeter;
	float							fPgrid_Sum;
	UINT 							nNoChange_Times;

	time_t 							tm_LastSyncEmsMgmt;
	time_t							tm_LastRecvEmsMgmt;
}STU_UDP_ROUGHDATA;

//----------- Public Rough Udp RoughData Define End ---------------//

BOOL UdpPublic_Initialize(HANDLE hSamplerSelf);
unsigned int  myAtoi(char *str);
char* 	Make_UdpTimeString(STU_UDP_ROUGHDATA* pUdp, time_t tmInout, char* pszOut);
UINT 	Make_UniqID_UDP(void);
char* 	Make_IPV42String(STU_UDP_ROUGHDATA* pUdp, ULONG ulIpV4_IP, char* pszOut);
BOOL 	isIPAddrMatchSysTp(STU_UDP_ROUGHDATA* pUdp);

//--- 	Client Function Define ---//
BOOL 	UdpClient_Initialize(HANDLE hSamplerSelf);
int		UdpClient_Sample(HANDLE hSamplerSelf);
BOOL 	Server_SendData(STU_UDP_ROUGHDATA *pUdp,\
        	cJSON* pJson, UDP_SERVER_CMDTP emMsgType, BOOL bUrget);
int 	PrepareClient_S10X_MeterValue(void* pArg);
int 	HandleClient_R100_MeterValue(void* pArg);
int 	HandleClient_R400_EmsMgmt(void* pArg);

void 	UdpClient_StuffMeterValue(STU_UDP_ROUGHDATA* pUdp, UINT nDevId, A_BLOB_METER_VALUES *pBlobMeter);
//--------- Client End ---------//


//--- 	Server Function Define ---//
BOOL 	UdpServer_Initialize(HANDLE hSamplerSelf);
int		UdpServer_Sample(HANDLE hSamplerSelf);
BOOL 	Server_SendData(STU_UDP_ROUGHDATA *pUdp,\
        cJSON* pJson, UDP_SERVER_CMDTP emMsgType, BOOL bUrget);
int 	PrepareServer_S100_MeterValue (void* pArg);
int 	HandleServer_R10X_MeterValue(void* pArg);
int 	PrepareServer_S400_EmsMgmt(void* pArg);

//--------- Server End ---------//

#endif


