#ifndef __ESS_MAINTENANCE_H
#define __ESS_MAINTENANCE_H

#include "std_sys.h"
#include "app_incs.h"
#include "../reporters_base.h"
//#include "../back_api/web_data_model.h"
#include "../web_reporter/back_api/web_data_model.h"

#define MAX_SUPPORTED_WEBREQ_TYPE	(WEB_REQTYPE_MAX - WEB_REQTYPE_GET_VERSION_IINFO)
#define TCP_BUFF_LEN				1024*8

typedef struct tagWebAPIReporterRough
{
	REPORTER*		pReporter;
	//mutex for getset/event_handler
	HANDLE			hMutex;
	//regiser info
	BOOL			bIsRegistered;	//tcp客户端连接上服务端标记
	time_t			tmRegiseterd;
	//buffer of IN/Out
	BYTE			byReqIn_Buffer[TCP_BUFF_LEN];
	int				nReadBytes;
	BYTE*			pbyParsed_InDataPos; //pared in data frame start position
	ULONG 			nParsed_InDataLen; //pared in data frame length
	BYTE 			byRepOut_Buffer[TCP_BUFF_LEN];
	int				nParseHandleRTN; //is parse and handle OK or not
	char            szServerUrl[255]; //Host URL
	time_t			tmHeartbeatedLast;
	int  			nisFirstBoot;
	
}WEBAPI_REPORTER_ROUGH;

typedef int (*WEB_REQ_PARSER)(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
typedef struct tagWebReqParserItem
{
	WEB_REQTYPE  	emReq;
	WEB_REQ_PARSER	pfnParser;
}WEB_REQPASER_ITEM;

//parsers
int ESS_Get_VersionInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Get_AllDevInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Get_AllParamInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Get_AllAlarmInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Get_AllParmsVals(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_LocalList(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_LocalCache(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Set_ParamCtrlSet(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int ESS_Handle_Heartbeat(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Query_FirmwUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_ChgProfiles(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Handle_LoacalListUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);

#endif
