#ifndef __REPORTER_WEB_API_H
#define __REPORTER_WEB_API_H

#include "std_sys.h"
#include "app_incs.h"
#include "../../reporters_base.h"
#include "../back_api/web_data_model.h"

#define MAX_SUPPORTED_WEBREQ_TYPE	(WEB_REQTYPE_MAX - WEB_REQTYPE_GET_VERSION_IINFO)


typedef struct tagWebAPIReporterRough
{
	REPORTER*		pReporter;
	//mutex for getset/event_handler
	HANDLE			hMutex;
	//regiser info
	BOOL			bIsRegistered;
	time_t			tmRegiseterd;
	//buffer of IN/Out
	BYTE			byReqIn_Buffer[MAX_WEB_REQDATA_SIZE];
	int				nReadBytes;
	BYTE*			pbyParsed_InDataPos; //pared in data frame start position
	ULONG 			nParsed_InDataLen; //pared in data frame length
	BYTE 			byRepOut_Buffer[MAX_WEB_REPDATA_SIZE];
	int				nParseHandleRTN; //is parse and handle OK or not
	LCCfg           lcCfg; // LJH250508 LCCfg
}WEBAPI_REPORTER_ROUGH;

typedef int (*WEB_REQ_PARSER)(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
typedef struct tagWebReqParserItem
{
	WEB_REQTYPE  	emReq;
	WEB_REQ_PARSER	pfnParser;
}WEB_REQPASER_ITEM;

//parsers
int Get_VersionInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Get_AllDevInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Get_AllParamInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Get_AllAlarmInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Get_AllParmsVals(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Get_ReadbackVals(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_LocalList(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_LocalCache(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
int Set_ParamCtrlSet(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Query_FirmwUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Get_ChgProfiles(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);
//int Handle_LoacalListUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos);

#endif
