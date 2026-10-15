// #define _TRACE_PRINT_
#include "reporter_webapi.h"

static WEBAPI_REPORTER_ROUGH 	s_webReporterRough;
static WEB_REQPASER_ITEM		s_webReqParserInfos[MAX_SUPPORTED_WEBREQ_TYPE];

#define DEF_WEBREQPARSER(_emReq, _pfnParser) do{ \
				int nIdx = (_emReq) - WEB_REQTYPE_GET_VERSION_IINFO; \
				s_webReqParserInfos[nIdx].emReq = (_emReq); \   
				s_webReqParserInfos[nIdx].pfnParser = (_pfnParser); \   
			}while(0)

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec);
static DWORD _ThreadEntry_WebAPIComm(void* pArg);

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	s_webReporterRough.pReporter = pReporter;

	DEF_WEBREQPARSER(WEB_REQTYPE_GET_VERSION_IINFO,		Get_VersionInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_DEVINFO,		Get_AllDevInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_PARAMINFO,		Get_AllParamInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_ALARMINFO,		Get_AllAlarmInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_PARAMVALS,		Get_AllParmsVals);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_READBACK_VALS,		Get_ReadbackVals);
	//DEF_WEBREQPARSER(WEB_REQTYPE_GET_LOCALLIST,			Get_LocalList);
	//DEF_WEBREQPARSER(WEB_REQTYPE_GET_LOCALCACHE,		Get_LocalCache);
	DEF_WEBREQPARSER(WEB_REQTYPE_SET_PARAM_SERETCTRL,	Set_ParamCtrlSet);
	//DEF_WEBREQPARSER(WEB_REQTYPE_QUERY_FIRMWARE_UPDT,	Query_FirmwUpdt);
	//DEF_WEBREQPARSER(WEB_REQTYPE_GET_CHGPROFILES,		Get_ChgProfiles);
	//DEF_WEBREQPARSER(WEB_REQTYPE_OTH_LOCALLIST_UPDT,	Handle_LoacalListUpdt);

	int nErrCode;
	pReporter->hCommPort = pReporter->pUsedCommPort->Open("COM_WEB_IPC",
		WEB_IPC_KEY, COMM_SERVER_MODE, 100, &nErrCode);
	if(nErrCode != ERR_COMM_OK)
	{
		//LogOut("OpenGUI Comm Port Failed-->%d");
		TRACE("WEB Open IPC Error:%d\n", nErrCode);
		return FALSE;
	}
	//set timeout
	//COMM_TIMEOUTS timeouts;
	//timeouts.nReadTimeout = 500;
	//timeouts.nWriteTimeout = 500;
	//pReporter->pUsedCommPort->Control(pReporter->hCommPort,
	//	COMM_SET_TIMEOUTS, 
	//	&timeouts, sizeof(timeouts));

	s_webReporterRough.hMutex = Mutex_Create(TRUE);
	if(s_webReporterRough.hMutex == NULL)
	{
		return FALSE;
	}

	pReporter->hqEvent = Queue_Create(MAX_WEB_CAP_MSGQUEUE, sizeof(WEB_FROMAAPP_MSG), 0);
	if(pReporter->hqEvent == NULL)
	{
		return FALSE;
	}

	pReporter->bExit = FALSE;
	s_webReporterRough.bIsRegistered = FALSE;
	s_webReporterRough.tmRegiseterd = 0;
	s_webReporterRough.lcCfg = GetLCCfg_Reporter(pReporter);
	
	TRACED("reporter_webapi_main:Initialize emSystemType:%d lcType:%d",
		s_webReporterRough.lcCfg.emSystemType, s_webReporterRough.lcCfg.lcType);

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_ALARM,			EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE | EVENT_MSG_STATUS_CHANGED},
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_CHARGING,			EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE | EVENT_MSG_STATUS_CHANGED},
		//{EVENT_TYPE_USER_AUTHORIZE,	EVENT_MSG_END}, //from CR or OCPP
		{EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED},
		//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
 		{EVENT_TYPE_BMSRUNDATA_UPDATE,	EVENT_MSG_STATUS_CHANGED},
		{EVENT_TYPE_HEALTHDATA_UPDATE,	EVENT_MSG_STATUS_CHANGED},
		{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
		//{EVENT_TYPE_UI_NOTICE,			EVENT_MSG_BEGIN},
		{EVENT_TYPE_BMS_HISTFAULT,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_BMS_HISTEVENT,		EVENT_MSG_BEGIN},
	};
	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("Web_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
	
	pReporter->hThread = RunThread_Create("WebAPIComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_WebAPIComm,
		(void *)(&s_webReporterRough),
		(DWORD*)&pReporter->bExit);
	if(pReporter->hThread == NULL)
	{
		//LogOut("Create GUIComm thread Failed");
		return FALSE;
	}

#ifndef _DEBUG
	_SYSTEM("/app/web/server/start_web_server");
#endif

	return TRUE;
}

static BOOL CheckFrame_Chkum(BYTE* pData, int nSize, ULONG ulChkSum)
{
	ULONG ulCalcChk = 0;
	int n;
	for( n = 0; n < nSize; n++)
	{
		ulCalcChk += pData[n];
	}
	if(ulCalcChk != ulChkSum)
	{
		TRACEX("calcSum->%d, chkSUm->%d\n", ulCalcChk, ulChkSum);
		return FALSE;
	}
	return TRUE;
}

int Handle_MakeRepFrame(WEBAPI_REPORTER_ROUGH* pWebRough, WEB_REQTYPE emReqType)
{
	int n;
	VAR_VALUE varV;
	//BYTE* pReqestPos = pReqData;
	BYTE* pReplyPos = pWebRough->byRepOut_Buffer + 16; //reply data pos
	BYTE* pRepDataStartPos = pReplyPos;
	pReplyPos[0] = (BYTE)emReqType;
	pReplyPos[1] = WEB_ERRCODE_OK;
	pReplyPos += 2;
	//BYTE* pDataStartPos = pReplyPos;

	int nDataSize = 2;
	pWebRough->nParseHandleRTN = WEB_ERRCODE_OK;
	for(n = 0; n < MAX_SUPPORTED_WEBREQ_TYPE; n++)
	{
		if(emReqType == s_webReqParserInfos[n].emReq)
		{
			nDataSize += s_webReqParserInfos[n].pfnParser(pWebRough, pReplyPos);
			break;
		}
	}

	if(pWebRough->nParseHandleRTN != WEB_ERRCODE_OK)
	{
		pRepDataStartPos[1] = pWebRough->nParseHandleRTN;
	}

	//now make header for reply frame
	varV.ulValue = 0;
	for(n = 0; n < nDataSize; n++)
	{
		varV.ulValue += pRepDataStartPos[n];
	}
	TRACEX("---Now send out a Rep.DataLen = %d, and emReqType = %d.----\n", nDataSize, emReqType);
	//header
	sprintf(pWebRough->byRepOut_Buffer, "W-REP:");
	memcpy(pWebRough->byRepOut_Buffer + 6, varV.abyValue, 4);
	pWebRough->byRepOut_Buffer[10] = ':';
	varV.ulValue = nDataSize;
	memcpy(pWebRough->byRepOut_Buffer + 11, varV.abyValue, 4);
	pWebRough->byRepOut_Buffer[15] = ':';

	return nDataSize + WEB_IPC_HEADER_LEN;
}

static void Set_IPCMode(REPORTER* pReporter, BOOL bPushPull)
{
	COMMPORT* pComm = pReporter->pUsedCommPort;
	HANDLE hPort = pReporter->hCommPort;
	int nMode = bPushPull ? IPC_MODE_PUSH_PULL : IPC_MODE_REQ_REP;
	pComm->Control(hPort, COMM_SET_IPC_MODE, &nMode, 4);
}

static void Parse_WebReq_AndHandle(WEBAPI_REPORTER_ROUGH* pWebRough)
{
	BYTE *pField;
	VAR_VALUE varV;
	int ulChkSun = 0;
	int ulFrameSize = 0;
	BYTE byReqType = 0;
	int nWriteBytes;
	REPORTER* pReporter = pWebRough->pReporter;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	BYTE* pbyStart = pWebRough->byReqIn_Buffer;
	int nReadBytes = pWebRough->nReadBytes;
	TRACE("-------Now read bytes->%d, %s----------\n", nReadBytes, pbyStart);
	while(nReadBytes > 0)
	{
		if(strncmp(pbyStart, "W-REQ:", 6) != 0)
		{
			TRACEX("req head is [%s], nReadBytes:%d\n", pbyStart, nReadBytes);
			pbyStart +=  1;
			nReadBytes -= 1;
			continue;
		}
		//TRACEX("szBuff = %08X-%08X-%08X-%08X-%08X.\n", *(ULONG*)szBuffer, *(ULONG*)(szBuffer + 4),
		//	*(ULONG*)(szBuffer + 8), *(ULONG*)(szBuffer + 12), *(ULONG*)(szBuffer + 16));
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField = pbyStart + 6;
		memcpy(varV.abyValue, pField, 4);
		ulChkSun = varV.ulValue;
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField += 5;
		memcpy(varV.abyValue, pField, 4);
		ulFrameSize = varV.ulValue;
		if(ulFrameSize + 16 > nReadBytes)
		{
			TRACEX("frame len not correct! ulFramLen = %d\n", ulFrameSize);
			break;
		}
		
		if(!CheckFrame_Chkum(pbyStart + 16, ulFrameSize, ulChkSun))
		{
			TRACEX("frame chksum not correct, chkSum->%d, ulFrmSize->%d!\n", ulChkSun, ulFrameSize);
			break;
		}
		pField += 5;
		pbyStart = pField;
		//parsed OK, now handle req-data
		byReqType = *(BYTE*)pbyStart;
		pbyStart += 1;
		ulFrameSize -= 1;

		if((byReqType < WEB_REQTYPE_GET_VERSION_IINFO) || (byReqType >= WEB_REQTYPE_MAX))
		{
			TRACEX("----Found an invalid msg, so we ned send ERRRR reply for %d----\n", byReqType);
			return;
		}

		int nSendBytes;
		int nTotalSendBytes;
		int n;
		if(byReqType == WEB_REQTYPE_GET_ALL_PARAMINFO_1)
		{
			nWriteBytes = Handle_MakeRepFrame(pWebRough, (WEB_REQTYPE)WEB_REQTYPE_GET_ALL_PARAMINFO);
			//pWebRough->byRepOut_Buffer  切断发送缓冲区数据，前面16个数据是帧头，需要去除掉，重新打包
			nTotalSendBytes = nWriteBytes - WEB_IPC_HEADER_LEN;
			nSendBytes = nTotalSendBytes / 2 + WEB_IPC_HEADER_LEN; //先发送一半数据
			varV.ulValue = 0;
			for(n = 0; n < nTotalSendBytes / 2; n++)
			{
				varV.ulValue += pWebRough->byRepOut_Buffer[n + WEB_IPC_HEADER_LEN];
			}
			memcpy(pWebRough->byRepOut_Buffer + 6, varV.abyValue, 4);  // 重新计算校验和

			varV.ulValue = nTotalSendBytes / 2; 
			memcpy(pWebRough->byRepOut_Buffer + 11, varV.abyValue, 4); //数据长度
			int nwrite = pComm->Write(hPort, (char*)pWebRough->byRepOut_Buffer, nSendBytes);
			// printf("----First part data sent bytes = %d.----\n", nwrite);
			break;
		}
		else if(byReqType == WEB_REQTYPE_GET_ALL_PARAMINFO_2)
		{
			nWriteBytes = Handle_MakeRepFrame(pWebRough, (WEB_REQTYPE)WEB_REQTYPE_GET_ALL_PARAMINFO);
			nTotalSendBytes = nWriteBytes - WEB_IPC_HEADER_LEN;
			int nPart1DataLen = nTotalSendBytes / 2; // 第一段业务数据长度
			nSendBytes = nTotalSendBytes - nPart1DataLen; //发送剩余数据
			varV.ulValue = 0;
			for(n = 0; n < nSendBytes; n++)
			{
				varV.ulValue += pWebRough->byRepOut_Buffer[n + nPart1DataLen +  WEB_IPC_HEADER_LEN];
			}
			memcpy(pWebRough->byRepOut_Buffer + nPart1DataLen + WEB_IPC_HEADER_LEN - 8, varV.abyValue, 4);   // 计算校验和

			varV.ulValue = nSendBytes; //第二段数据长度
			memcpy(pWebRough->byRepOut_Buffer + nPart1DataLen + WEB_IPC_HEADER_LEN - 4, varV.abyValue, 4); 

			int nwrite = pComm->Write(hPort, (char*)(pWebRough->byRepOut_Buffer + nPart1DataLen + WEB_IPC_HEADER_LEN - 8), nSendBytes + 8);
			// printf("----Second part data sent bytes = %d.----\n", nwrite);
			break;
		}

		pWebRough->pbyParsed_InDataPos = pbyStart;
		pWebRough->nParsed_InDataLen = ulFrameSize;

		TRACEX("Before parse req from WEB! ReqType = %d.\n", byReqType);

		nWriteBytes = Handle_MakeRepFrame(pWebRough, (WEB_REQTYPE)byReqType);
		
		TRACE("Parsed req from WEB successfully! ReqType = %d, and reply bytes = %d.\n",
				 byReqType, nWriteBytes);

		if((nWriteBytes > 0) && (nWriteBytes <= MAX_WEB_REPDATA_SIZE))
		{
			pComm->Write(hPort, (char*)pWebRough->byRepOut_Buffer, nWriteBytes);
		}
		//now parse another frame
		//pbyStart += ulFrameSize;
		//nReadBytes -= (ulFrameSize + 17);
		break;
	}
	
}

static DWORD _ThreadEntry_WebAPIComm(void* pArg)
{
	WEBAPI_REPORTER_ROUGH* pWebRough = (WEBAPI_REPORTER_ROUGH*)pArg;
	REPORTER* pReporter = pWebRough->pReporter;
	WEB_FROMAAPP_MSG msg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;

	while(!pReporter->bExit)
	{
		//get msg from queue to be sent out
		if(Mutex_Lock(pWebRough->hMutex, 50) != ERR_MUTEX_OK)
		{
			Sleep(50);
			continue;
		}

		tmNow = _NOW_;
		if((pWebRough->bIsRegistered) && (ABS(tmNow - pWebRough->tmRegiseterd) >= 5))
		{
			if(Queue_Get(pReporter->hqEvent, &msg, FALSE, 100) == ERR_MUTEX_OK)
			{
				TRACEX("----Now send msg to Web\n");
				Set_IPCMode(pReporter, TRUE);
				pComm->Write(hPort, (BYTE*)&msg, sizeof(WEB_FROMAAPP_MSG));
			}
		}

		Set_IPCMode(pReporter, FALSE);

		RunThread_Heartbeat(pReporter->hThread);

		//get req from WEB front
		//ZERO_POBJS(pWebRough->byReqIn_Buffer, MAX_WEB_REQDATA_SIZE);
		memset(pWebRough->byReqIn_Buffer, 0, MAX_WEB_REQDATA_SIZE);
		pWebRough->nReadBytes = pComm->Read(hPort, (char*)pWebRough->byReqIn_Buffer, MAX_WEB_REQDATA_SIZE);
		if(pWebRough->nReadBytes <= 0)
		{
			Mutex_Unlock(pWebRough->hMutex);
			continue;
		}

		//Now parse REQ and handle it
		Parse_WebReq_AndHandle(pWebRough);
	
		Mutex_Unlock(pWebRough->hMutex);
	}

	//do clean job
	//THREAD_CANCEL_ALL means must system will reboot
	return THREAD_CANCEL_ALL;
}

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	//Make buffer
	static WEB_FROMAAPP_MSG msg;
	ZERO_POBJS(&msg, 1);
	VAR_VALUE varV;
	int n;
	if((nEvDataSize > MAX_WEB_MSG_SIZE) || (nEvDataSize < 0) || (pEvData == NULL))
	{
		return EVENT_HANDED_FAILED;
	}
	//head + len + chksum
	msg.bySOI = WEB_IPC_MSG_SOI;
	msg.ulLength = nEvDataSize + 2;		
	msg.ulChkSum = 0;
	for(n = 0; n < nEvDataSize; n++)
	{
		msg.ulChkSum += *(BYTE*)((BYTE*)pEvData + n);
	}	
	//event data
	msg.byEvType = (BYTE)emEvType;
	msg.byMsgType = (BYTE)emMsgType;	
	memcpy(msg.byEvData, pEvData, nEvDataSize);
	
	// if(bBlockMode && s_webReporterRough.bIsRegistered)
	// {
	// 	if(Mutex_Lock(s_webReporterRough.hMutex, nMaxBlockMSec) != ERR_MUTEX_OK)
	// 	{
	// 		return EVENT_HANDED_TIMEOUT;
	// 	}

	// 	Set_IPCMode(pReporter, TRUE);

	// 	pReporter->pUsedCommPort->Write(
	// 		pReporter->hCommPort, &msg, sizeof(msg));

	// 	Mutex_Unlock(s_hMutex);
	// }
	// else
	{
		TRACEX("Received a msg type:%d, Block:%d.\n", emEvType, bBlockMode);
		Queue_Put(pReporter->hqEvent, &msg, FALSE);
	}

	return EVENT_HANDED_OK;
}



