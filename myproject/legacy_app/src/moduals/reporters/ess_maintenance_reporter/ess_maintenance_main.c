//#define   _TRACE_PRINT_
#include "ess_maintenance.h"

static WEBAPI_REPORTER_ROUGH 	s_webReporterRough;
static WEB_REQPASER_ITEM		s_webReqParserInfos[MAX_SUPPORTED_WEBREQ_TYPE];

#define DEF_WEBREQPARSER(_emReq, _pfnParser) do{ \
				int nIdx = (_emReq) - WEB_REQTYPE_GET_VERSION_IINFO; \
				s_webReqParserInfos[nIdx].emReq = (_emReq); \   
				s_webReqParserInfos[nIdx].pfnParser = (_pfnParser); \   
			}while(0)

// typedef struct tagEvHandTags
// {
// 	int nEvTag;
// 	int nMsgTag;
// }EVHAND_TAGS;

// static int Event_Handler(HANDLE	hModuleSeif,
// 	_EVENT_TYPE	emEvType,
// 	_EVENT_MSG	emMsgType,
// 	UINT			nEvDataSize,
// 	void*			pEvData,
// 	BOOL			bBlockMode,
// 	int			nMaxBlockMSec);
static DWORD _ThreadEntry_MaintainTCPComm(void* pArg);

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEI("---------------Initialize New MainTain Backend Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	s_webReporterRough.pReporter = pReporter;

	DEF_WEBREQPARSER(WEB_REQTYPE_GET_VERSION_IINFO,		ESS_Get_VersionInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_DEVINFO,		ESS_Get_AllDevInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_PARAMINFO,		ESS_Get_AllParamInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_ALARMINFO,		ESS_Get_AllAlarmInfos);
	DEF_WEBREQPARSER(WEB_REQTYPE_GET_ALL_PARAMVALS,		ESS_Get_AllParmsVals);
	DEF_WEBREQPARSER(WEB_REQTYPE_SET_PARAM_SERETCTRL,	ESS_Set_ParamCtrlSet);
	DEF_WEBREQPARSER(WEB_REQTYPE_HANDLE_HEARTBEAT,		ESS_Handle_Heartbeat);//41

	int nErrCode;
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 255;
 	valGet.val.blobValue = (BYTE*)s_webReporterRough.szServerUrl;
	nErrCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 52, &valGet, sizeof(PARAM_VAL_GET));
	if (nErrCode != ERRCODE_DAI_OK)
	{
		TRACE("ERROR: server URL can not be retrieved\n");
		return FALSE;
	}

	TRACEI("Server URL: %s", s_webReporterRough.szServerUrl);

	s_webReporterRough.hMutex = Mutex_Create(TRUE);
	if(s_webReporterRough.hMutex == NULL)
	{
		return FALSE;
	}

	// pReporter->hqEvent = Queue_Create(MAX_WEB_CAP_MSGQUEUE, sizeof(WEB_FROMAAPP_MSG), 0);
	// if(pReporter->hqEvent == NULL)
	// {
	// 	return FALSE;
	// }

	pReporter->bExit = FALSE;
	s_webReporterRough.bIsRegistered = FALSE;//TRUE
	s_webReporterRough.tmRegiseterd = 0;
	s_webReporterRough.tmHeartbeatedLast = 0;
	s_webReporterRough.nisFirstBoot = TRUE;

	// EVHAND_TAGS tags[] = 
	// {
	// 	{EVENT_TYPE_ALARM,			EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE | EVENT_MSG_STATUS_CHANGED},
	// 	{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
	// 	{EVENT_TYPE_CHARGING,			EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE | EVENT_MSG_STATUS_CHANGED},
	// 	//{EVENT_TYPE_USER_AUTHORIZE,	EVENT_MSG_END}, //from CR or OCPP
	// 	{EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED},
	// 	//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
 	// 	{EVENT_TYPE_BMSRUNDATA_UPDATE,	EVENT_MSG_STATUS_CHANGED},
	// 	{EVENT_TYPE_HEALTHDATA_UPDATE,	EVENT_MSG_STATUS_CHANGED},
	// 	{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
	// 	//{EVENT_TYPE_UI_NOTICE,			EVENT_MSG_BEGIN},
	// };
	// int n;
	// for(n = 0; n < ITEM_OF(tags); n++)
	// {
	// 	pReporter->pDAI->Regsiter_EvHandler("MaintainTCP_CommEventHand",
	// 		pReporterThis,
	// 		Event_Handler,
	// 		tags[n].nEvTag,
	// 		tags[n].nMsgTag,
	// 		FALSE);
	// }
	
	pReporter->hThread = RunThread_Create("MaintainTCPComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_MaintainTCPComm,
		(void *)(&s_webReporterRough),
		(DWORD*)&pReporter->bExit);
	if(pReporter->hThread == NULL)
	{
		LogOut(MAIN_MODULE_REPORTER, "MaintainTCPComm", LOG_TYPE_ERROR, 
			"Init MaintainTCPComm FAILED!!!");	
		return FALSE;
	}

/*
#ifndef _DEBUG
	_SYSTEM("/app/web/server/start_web_server"); //start web in shell
#endif
*/

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
	BYTE* pReplyPos = pWebRough->byRepOut_Buffer + 10; //reply data pos
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
	//sprintf(pWebRough->byRepOut_Buffer, "W-REP:");
	pWebRough->byRepOut_Buffer[0] = 0x68;
	memcpy(pWebRough->byRepOut_Buffer + 1, varV.abyValue, 4);
	//pWebRough->byRepOut_Buffer[10] = ':';
	varV.ulValue = nDataSize;
	memcpy(pWebRough->byRepOut_Buffer + 5, varV.abyValue, 4);
	//pWebRough->byRepOut_Buffer[15] = ':';
	pWebRough->byRepOut_Buffer[9] = 0x68;
	pWebRough->byRepOut_Buffer[10 + nDataSize] = 0x16;

	return nDataSize + ESS_TCP_MAINTEN_FIX_LEN;
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
	ULONG ulChkSun;
	ULONG ulFrameSize;
	BYTE byReqType = 0;
	int nWriteBytes;
	REPORTER* pReporter = pWebRough->pReporter;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	BYTE* pbyStart = pWebRough->byReqIn_Buffer;
	int nReadBytes = pWebRough->nReadBytes;
	int n,m;
	
	TRACEI("-------Now read bytes->%d", nReadBytes);
	while(nReadBytes > 0)
	{
		// if(strncmp(pbyStart, "W-REQ:", 6) != 0)
		// {
		// 	TRACEX("req head is [%s], nReadBytes:%d\n", pbyStart, nReadBytes);
		// 	pbyStart +=  1;
		// 	nReadBytes -= 1;
		// 	continue;
		// }

		if (pbyStart[0] != 0x68)
		{
			pbyStart +=  1;
			nReadBytes -= 1;
			continue;
		}
		else
		{
			if (pbyStart[9] != 0x68)
			{
				pbyStart +=  1;
				nReadBytes -= 1;
				continue;
			}
		}

		//TRACEX("szBuff = %08X-%08X-%08X-%08X-%08X.\n", *(ULONG*)szBuffer, *(ULONG*)(szBuffer + 4),
		//	*(ULONG*)(szBuffer + 8), *(ULONG*)(szBuffer + 12), *(ULONG*)(szBuffer + 16));
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField = pbyStart + 1;
		memcpy(varV.abyValue, pField, 4);
		ulChkSun = varV.ulValue;
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField += 4;
		memcpy(varV.abyValue, pField, 4);
		ulFrameSize = varV.ulValue;
		if(ulFrameSize + ESS_TCP_MAINTEN_FIX_LEN > nReadBytes)
		{
			TRACEE("frame len not correct! ulFramLen = %d\n", ulFrameSize);
			break;
		}
		
		if(!CheckFrame_Chkum(pbyStart + 10, ulFrameSize, ulChkSun))
		{
			TRACEE("frame chksum not correct, chkSum->%d, ulFrmSize->%d!\n", ulChkSun, ulFrameSize);
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

		pWebRough->pbyParsed_InDataPos = pbyStart;
		pWebRough->nParsed_InDataLen = ulFrameSize;

		TRACEX("Before parse req from WEB! ReqType = %d.\n", byReqType);

		nWriteBytes = Handle_MakeRepFrame(pWebRough, (WEB_REQTYPE)byReqType);
		
		TRACE("Parsed req from WEB successfully! ReqType = %d, and reply bytes = %d.\n",
				 byReqType, nWriteBytes);

		if((nWriteBytes > 0) && (nWriteBytes <= TCP_BUFF_LEN))
		{
			pComm->Write(hPort, (char*)pWebRough->byRepOut_Buffer, nWriteBytes);
		}

        printf("newmaintain----send=");
        for (n = 0; n < 50; n++)
        {
            printf("[%02x] ",pWebRough->byRepOut_Buffer[n]);
        }
        printf("\n");

		//now parse another frame
		//pbyStart += ulFrameSize;
		//nReadBytes -= (ulFrameSize + 17);
		break;
	}
	
}

static BOOL JudgeisConnect(WEBAPI_REPORTER_ROUGH* pWebRough)
{
	VAR_VALUE varV;

	if (pWebRough->nisFirstBoot == TRUE)//first Time no need to judge 
	{
		pWebRough->nisFirstBoot = FALSE;
		return ;
	}

	if ((pWebRough->bIsRegistered) && ((_NOW_ - pWebRough->tmHeartbeatedLast) < 60)) //normal
	{
		varV.emValue = 0;
	}
	else//alarm
	{
		varV.emValue = 1;   
	}

	// pWebRough->pReporter->pDAI->Set((HANDLE)pWebRough->pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
	// 	1, 5, &varV, 4, 100);
	pWebRough->pReporter->pDAI->Set((HANDLE)pWebRough->pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
		1, 32, &varV, 4, 100);

	return;
}

static BOOL TryConnect2Server(WEBAPI_REPORTER_ROUGH* pWebRough)
{
	COMM_TIMEOUTS timeOut;

	if (pWebRough->bIsRegistered)
	{
		return TRUE;
	}
	REPORTER* pReporter = pWebRough->pReporter;
	int  nErrCode;

	pReporter->hCommPort = pReporter->pUsedCommPort->Open("COM_MAINTAIN_TCP",
		s_webReporterRough.szServerUrl, COMM_CLIENT_MODE, 1000, &nErrCode);
	if(nErrCode != ERR_COMM_OK)
	{
		//LogOut("OpenGUI Comm Port Failed-->%d");
		TRACEE("COM_MAINTAIN_TCP Open TCP Error:%d\n", nErrCode);
		return FALSE;
	}
			
	timeOut.nReadTimeout = 500;
	timeOut.nWriteTimeout = 500;
	timeOut.nIntervalTimeout = 50;
	pReporter->pUsedCommPort->Control(pReporter->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));

	TRACEI("TryConnect2Server successfully ! ! ! ");
 
	pWebRough->bIsRegistered = TRUE;

	return TRUE;
}

static DWORD _ThreadEntry_MaintainTCPComm(void* pArg)
{
	WEBAPI_REPORTER_ROUGH* pWebRough = (WEBAPI_REPORTER_ROUGH*)pArg;
	REPORTER* pReporter = pWebRough->pReporter;
	WEB_FROMAAPP_MSG msg;
	// HANDLE hPort = pReporter->hCommPort;
	// COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;
	int n;

	while(!pReporter->bExit)
	{
		RunThread_Heartbeat(pReporter->hThread);

		JudgeisConnect(pWebRough);

		if (!TryConnect2Server(pWebRough))
		{
			Sleep(3000);
			continue;
		}
		
		//get msg from queue to be sent out
		if(Mutex_Lock(pWebRough->hMutex, 50) != ERR_MUTEX_OK)
		{
			Sleep(50);
			continue;
		}

		// tmNow = _NOW_;
		// if((pWebRough->bIsRegistered) && (ABS(tmNow - pWebRough->tmRegiseterd) >= 5))
		// {
		// 	TRACEI("----Now Queue_Get Msg---");
		// 	if(Queue_Get(pReporter->hqEvent, &msg, FALSE, 100) == ERR_MUTEX_OK)
		// 	{
		// 		TRACED("----Now send msg to Web msg.byEvType = [%d]",msg.byEvType);
		// 		//pComm->Write(hPort, (BYTE*)&msg, sizeof(WEB_FROMAAPP_MSG));
		// 	}
		// }
	
		//get req from WEB front
		//TRACEI("now reading recrive data...");
		memset(pWebRough->byReqIn_Buffer, 0, TCP_BUFF_LEN);
		pWebRough->nReadBytes = pReporter->pUsedCommPort->Read(pReporter->hCommPort, (char*)pWebRough->byReqIn_Buffer, TCP_BUFF_LEN);
		//TRACEI("finish reading recrive data...");
		if(pWebRough->nReadBytes <= 0)
		{
			Mutex_Unlock(pWebRough->hMutex);
			continue;
		}

		printf("ESS_maintain----read=");
		for (n = 0; n < 50; n++)
		{
		    printf("[%02x] ",pWebRough->byReqIn_Buffer[n]);
		}
		printf("\n");

		//Now parse REQ and handle it
		Parse_WebReq_AndHandle(pWebRough);
	
		Mutex_Unlock(pWebRough->hMutex);
		
	}

	//do clean job
	//THREAD_CANCEL_ALL means must system will reboot
	return THREAD_CANCEL_ALL;
}

// static int Event_Handler(HANDLE	hModuleSeif,
// 	_EVENT_TYPE	emEvType,
// 	_EVENT_MSG	emMsgType,
// 	UINT			nEvDataSize,
// 	void*			pEvData,
// 	BOOL			bBlockMode,
// 	int			nMaxBlockMSec)
// {
// 	REPORTER* pReporter = (REPORTER*)hModuleSeif;
// 	//Make buffer
// 	static WEB_FROMAAPP_MSG msg;
// 	ZERO_POBJS(&msg, 1);
// 	VAR_VALUE varV;
// 	int n;
// 	//("Event_Handler--nEvDataSize[%d]",nEvDataSize);
// 	if((nEvDataSize > MAX_WEB_MSG_SIZE) || (nEvDataSize < 0) || (pEvData == NULL))
// 	{
// 		return EVENT_HANDED_FAILED;
// 	}

// 	//head + len + chksum
// 	msg.bySOI = WEB_IPC_MSG_SOI;
// 	msg.ulLength = nEvDataSize + 2;		
// 	msg.ulChkSum = 0;
// 	for(n = 0; n < nEvDataSize; n++)
// 	{
// 		msg.ulChkSum += *(BYTE*)((BYTE*)pEvData + n);
// 	}

// 	//event data
// 	msg.byEvType = (BYTE)emEvType;
// 	msg.byMsgType = (BYTE)emMsgType;	
// 	memcpy(msg.byEvData, pEvData, nEvDataSize);
	
// 	// if(bBlockMode && s_webReporterRough.bIsRegistered)
// 	// {
// 	// 	if(Mutex_Lock(s_webReporterRough.hMutex, nMaxBlockMSec) != ERR_MUTEX_OK)
// 	// 	{
// 	// 		return EVENT_HANDED_TIMEOUT;
// 	// 	}

// 	// 	Set_IPCMode(pReporter, TRUE);

// 	// 	pReporter->pUsedCommPort->Write(
// 	// 		pReporter->hCommPort, &msg, sizeof(msg));

// 	// 	Mutex_Unlock(s_hMutex);
// 	// }
// 	// else
// 	{
// 		//TRACED("Received a msg type:%d, Block:%d.\n", emEvType, bBlockMode);
// 		Queue_Put(pReporter->hqEvent, &msg, FALSE);
// 	}

// 	return EVENT_HANDED_OK;
// }



