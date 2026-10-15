// #define _TRACE_PRINT_
#include "../reporters_base.h"
#include "es_iocpp.h"
#include <signal.h>

// edit by kuangyue 20250318
static int queueSize = 0;
static time_t startTime = 0;
static BOOL recordedMessages[Max_ES_iOCPP_Msgs] = {FALSE};
static MessageEntry messageQueue[Max_ES_iOCPP_Msgs];
static BOOL allMessagesRecorded(void) 
{
    int i;
    for (i = 0; i < Max_ES_iOCPP_Msgs; i++) 
    {
        if (!recordedMessages[i]) 
        {
            return FALSE;
        }
    }
    return TRUE;
}
static void resetRecording(void) 
{
    memset(recordedMessages, 0, sizeof(recordedMessages));
    queueSize = 0;
    startTime = _NOW_;
}

static void manageMessageQueue(UINT logInterval)
{
    time_t now = _NOW_;
	TRACED("Queue size: %d\n", queueSize);
    
    int i = 0;
    while (i < queueSize)
    {
        ES_iOCPP_MSG_TYPE currentType = messageQueue[i].type;
        if (!recordedMessages[currentType]) 
        {
            LOG_INFO("Message Type: %d, Content: %s\n", currentType, messageQueue[i].szSendBuffer);
            recordedMessages[currentType] = TRUE;
            int j;
            for (j = i; j < queueSize - 1; j++) 
            {
                messageQueue[j] = messageQueue[j + 1];
            }
            queueSize--;
        } 
        else 
        {
            i++;
        }
    }
    
    if (now - startTime > logInterval) 
    {
		LOG_INFO("----Log recording interval: %u seconds----\n", logInterval);
        resetRecording();
        return;
    }
    
    if (allMessagesRecorded()) 
    {
        resetRecording();
    }
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
	ES_iOCPP_DATA_STU* pOCPP = (ES_iOCPP_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			TRACE("Info:IOCPP Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEE("EVENT_TYPE_ALARM  alarm_id: [%d]",pAlmEv->nAlarmId);
			pOCPP->bNeedRefreshActvAlm = TRUE;
		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			pOCPP->emFirmwareUpgradeStatus = pFirmUpSta->emUpdateStatus;
			//Now call the msg handler to create the msg to send
			pOCPP->stMsgInfoList[C_405_StatOfFirmw].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pOCPP->hMutex);
		}
		break;
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
			pOCPP->emDiagnosUploadStatus = pDiagUpSta->emUploadStatus;
			//Now call the msg handler to create the msg to send
			pOCPP->stMsgInfoList[C_407_StatOfDiag].pfnMsgHandler((void*)pOCPP);
			Mutex_Unlock(pOCPP->hMutex);
		}
		break;
		case EVENT_TYPE_ABILITY_CHANGE:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ABILITY_CHG * pEvAbChg = (EVENT_ABILITY_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ABILITY_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
			if(pEvAbChg->nWhichGun > Max_Connctors)
			{
				return EVENT_HANDED_FAILED;
			}

			pEvAbChg->nWhichGun -= 1; //this is from 1~3

			if(pEvAbChg->nWhichGun == Connctor_0)
			{
				TRACEX("Ability Change Event for The Connnector 0  \n ");
				A_BLOB_CAHRGEPOINT_STATUSINFO StatusNotiData;
				valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
				valGet.val.blobValue = (BYTE*)&StatusNotiData;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					1, 10, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					return EVENT_HANDED_FAILED;
				}
				if((StatusNotiData.emChargePointStatus != pOCPP->blobCPStatusInfo.emChargePointStatus) ||
					(StatusNotiData.emChargePointErrcode != pOCPP->blobCPStatusInfo.emChargePointErrcode))
				{
					Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
					//pOCPP->nCurrChargeStatusHandled_Conn = pEvChag->emGunNum; // for Zero connector 
					//pOCPP->byCuurConn_ChgType = pEvChag->emChangeType;
					memcpy(&pOCPP->blobCPStatusInfo, &StatusNotiData, sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO));
					pOCPP->bCPStatusChanged = TRUE;
					//Now call the msg handler to create the msg to send
					//pOCPP->stMsgInfoList[StatusNotification].pfnMsgHandler((void*)pOCPP);
					Mutex_Unlock(pOCPP->hMutex);
				}
			}
		}
		break;

		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;

		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("OCPP_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			//Jimmy note: this need to be implemented whenever parameter changes 
			if((pParamChg->parmPTR.nId == 1) && ((pParamChg->parmPTR.nSubId == 25) || 
				(pParamChg->parmPTR.nSubId == 2))) //server URL change or ChargerID changed
			{
				//PARAM_VAL_GET valGet;
				//valGet.nGetSize = 255;
				//valGet.val.blobValue = (BYTE*)&pOCPP->szServerURL;
				//pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				//	1, 18, &valGet, sizeof(PARAM_VAL_GET));

				valGet.nGetSize = 255;
				char szEndUrl[255];
				valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 25, &valGet, sizeof(PARAM_VAL_GET));

				char szChargerID[64];
				valGet.nGetSize = 64;
				valGet.val.blobValue = (BYTE*)szChargerID;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 2, &valGet, sizeof(PARAM_VAL_GET));

				snprintf(pOCPP->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);
				pOCPP->bBootNotifyDone = FALSE;
				pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_URL_CHANGED;
				pOCPP->emOCPPConnStatus = Disconnected; //need to reconnect to new server again
				//Percent_EncodeUrl(pOCPP->szServerURL); //Jimmy note 20181129: maybe do not need as URL should be correct when input on LCD
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 11))
			{
				valGet.nGetSize = 4;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 11, &valGet, sizeof(PARAM_VAL_GET));
				pOCPP->bZlibCompress = (valGet.val.emValue == 0) ? FALSE : TRUE;
				//printf("pOCPP->bZlibCompress = %d\n", pOCPP->bZlibCompress);
			}
			else if ((pParamChg->parmPTR.nSubId == 26))
			{
				// Read ESiOCPP Config 
				TRACEX("Config Changed Evevt \n");
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pOCPP->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
					return FALSE;
				}
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pOCPP->nCfgBMSNum = valGet.val.ulValue;
				if(pOCPP->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pOCPP->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				TRACEX("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pOCPP->nCfgBMSNum);
			}
			else if ((pParamChg->parmPTR.nSubId == 83))
			{
				// Read Config 
				valGet.nGetSize = 512;
				valGet.val.blobValue = (BYTE*)pOCPP->szShieldAlarms;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 83, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY settings can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt [%s]\n",pOCPP->szShieldAlarms);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 46))
			{
				valGet.nGetSize = 4;// edit by kuangyue 20250516
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 46, &valGet, sizeof(PARAM_VAL_GET));
				pOCPP->nLog_Interval = valGet.val.ulValue;

				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: pOCPP nLog_Interval can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt pOCPP->nLog_Interval[%d]\n",pOCPP->nLog_Interval);
			}
			else if((pParamChg->parmPTR.nId == 1001) &&
				(pParamChg->parmPTR.nSubId == 3313 || pParamChg->parmPTR.nSubId == 3314 ||
				pParamChg->parmPTR.nSubId == 3315))
			{// LJH250428 cmd
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, &valGet, sizeof(PARAM_VAL_GET));
				TRACED("es_iocpp_main::Event_Handler SEMS Changed err:%d nId:%d nSubId:%d ",
					err, pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId);
				if (err != ERRCODE_DAI_OK)
				{
					TRACEE("es_iocpp_main pDAI->Get err:%d", err);
					return EVENT_HANDED_FAILED;
				}
				switch (pParamChg->parmPTR.nSubId)
				{
					case 3313: // Send CMD Info To LC Server 1001, 3313,
						pOCPP->lcCfg.bSendCmdByLC1stOnly = (BOOL)valGet.val.emValue;
						break;
					case 3314: // Max ESC Num In Branch 1001, 3314,
						pOCPP->lcCfg.nESCNum = valGet.val.ulValue;
						break;
					case 3315: // Branch Config Number 1001, 3315,
						pOCPP->lcCfg.nBranchNum = valGet.val.ulValue;
						break;
					default:
						break;
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 55))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, &valGet, sizeof(PARAM_VAL_GET));	
				if(err != ERRCODE_DAI_OK)	
				{
					return EVENT_HANDED_FAILED;
				}	
				else
				{
					pOCPP->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_IOCPP)? TRUE : FALSE;
				}	
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 85))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 85, &valGet, sizeof(PARAM_VAL_GET));
				pOCPP->bStandbyEnable = valGet.val.emValue;
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: pOCPP bStandbyEnable can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt pOCPP->bStandbyEnable[%d]\n",pOCPP->bStandbyEnable);
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

static BOOL TryConnect2Server(REPORTER* pReporter)
{
	int nErrCode;
	char szDump[256] = {0};
	static int s_nLastOpenErrCode = -999;
	ES_iOCPP_DATA_STU* pOCPP = (ES_iOCPP_DATA_STU*)pReporter->pDataHeap;
	if(pReporter->hCommPort != NULL)
	{
		nErrCode = pReporter->pUsedCommPort->Close(pReporter->hCommPort);
		TRACEX("Close The Comm Port errcode:%d\n", nErrCode);
		sprintf(szDump, "Close the comm port[%d], reason[%d]", nErrCode, pOCPP->emOCPP_Disconn_Reason);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_CLOSE_COMM, szDump);//edit by kuangyue 20250315
		//LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_CLOSE_COMM, szDump);
		LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR, szDump);
		
		pReporter->hCommPort = NULL;
		s_nLastOpenErrCode = -999;
	}


	DWORD dwPortAttr = (pOCPP->bZlibCompress == TRUE) ? COMM_BINARY_MODE : COMM_OCPP1_6_MODE;
 
	//pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
	//	"ws://121.40.165.18:8800", COMM_CLIENT_MODE, 5000, &nErrCode); //5 sec timeout for connecting...
	pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
		pOCPP->szServerURL, dwPortAttr, 10000, &nErrCode); //10 sec timeout for connecting...
	TRACE("pReporter->info.szReporterSetting =%s pOCPP->szServerURL=%s\n",pReporter->info.szReporterSetting,pOCPP->szServerURL);
	
	static char s_strLastServerURL[255] = {0};
	if(s_nLastOpenErrCode != nErrCode) //Jimmy added 20190411 for only record once if the network is not alive
	{
		s_nLastOpenErrCode = nErrCode;
		memcpy(s_strLastServerURL, pOCPP->szServerURL, 255);
		sprintf(szDump, "Open the address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250315
		//LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
		LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR, szDump);
	}
	else if(strcmp(s_strLastServerURL, pOCPP->szServerURL) != 0) //Jimmy added 20190411 for only record once if the network is not alive
	{
		memcpy(s_strLastServerURL, pOCPP->szServerURL, 255);
		sprintf(szDump, "Open the NEW address[%s]: [%s]", pOCPP->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250315
		LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}

	if(pReporter->hCommPort == NULL)
	{
		TRACEX("The open errcode:%d\n", nErrCode);
		if(nErrCode != ERR_COMM_TIMEOUT)
		{
			//LogOut("OpenGUI Comm Port Failed-->%d");
			return FALSE;
		}
	}
	else
	{
		COMM_TIMEOUTS timeOut;
		timeOut.nReadTimeout = 500;
		timeOut.nWriteTimeout = 100;
		timeOut.nIntervalTimeout = 50;
		pReporter->pUsedCommPort->Control(pReporter->hCommPort,COMM_SET_TIMEOUTS,&timeOut, sizeof(COMM_TIMEOUTS));
	}

	return TRUE;
}

static UINT Make_UniqID_ESiOCPP(void)
{
	//time_t tmNow = _NOW_;
	//double tmPrecNow = GetCurrentTime();
	//double diffMsec= (tmPrecNow - tmNow) * 100.0;

	//UINT nUniqID = tmNow * 100 + (UINT)diffMsec;
#define ES_iOCPP_UNIQID_START	25689566
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + ES_iOCPP_UNIQID_START; //just to mix the value
	}
	//static UINT s_nOldUniqID = OCPP_UNIQID_START;
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = ES_iOCPP_UNIQID_START;
	}

	TRACE("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}

/*int compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
**
compress函数将 source 缓冲区中的内容压缩到 dest 缓冲区。 sourceLen 表示source 缓冲区的大小(以字节计)。注意函数的第二个参数destLen 是传址调用。当调用函数时，destLen表示 dest 缓冲区的大小，destLen > (sourceLen + 12)*100.1%。当函数退出后，destLen 表示压缩后缓冲区的实际大小。此时 destLen / sourceLen 正好是压缩率。

compress 若成功，则返回 Z_OK；若没有足够内存，则返回 Z_MEM_ERROR；若输出缓冲区不够大，则返回 Z_BUF_ERROR。
**/

//static char* MakeOCPP_CallFrame(OCPP_DATA_STU* pOCPP, OCPP_MSGCALL_SRC emSrc, OCPP_MSG_TYPE emMsgType, char* szJsonData, BOOL bResent)
static char* MakeESiOCPP_CallFrame(ES_iOCPP_DATA_STU* pOCPP, ES_iOCPP_CALL_MSG *pMsg, BOOL bResent)
{
	TRACEX("MakeOCPPCallFrame\n");

	pOCPP->bHasActiveMsg = TRUE;
	pOCPP->activeCallMsg.tmLastSent = _NOW_;
	if(!bResent)
	{
		//ZERO_POBJS(&pOCPP->activeCallMsg.msgData, 1);
		//memcpy(pOCPP->activeCallMsg.msgData.szJsonData, szJsonData, strlen(szJsonData));
		//pOCPP->activeCallMsg.msgData.emMsgType = emMsgType;
		//pOCPP->activeCallMsg.msgData.emSrc = emSrc;
		memcpy(&pOCPP->activeCallMsg.msgData, pMsg, sizeof(ES_iOCPP_CALL_MSG));
		pOCPP->activeCallMsg.nRetryTimes = 1;
	}
	else
	{
		pOCPP->activeCallMsg.nRetryTimes++;
	}
	
	UINT nMsgUniqID = Make_UniqID_ESiOCPP(); //(UINT)(GetCurrentTime() * 1000);

	if(pOCPP->activeCallMsg.nRetryTimes > MAX_ES_iOCPP_SEND_RETRIES)
	{
		pOCPP->activeCallMsg.nRetryTimes = MAX_ES_iOCPP_SEND_RETRIES;
		pOCPP->activeCallMsg.nUniqMsgId[MAX_ES_iOCPP_SEND_RETRIES - 1] = nMsgUniqID;
	}
	else
	{
		int nWhich = (pOCPP->activeCallMsg.nRetryTimes <= 0) ? 0 : pOCPP->activeCallMsg.nRetryTimes - 1;
		pOCPP->activeCallMsg.nUniqMsgId[nWhich] = nMsgUniqID;
	}

	//sprintf(pOCPP->activeCallMsg.nUniqMsgId, "%lu", );
	snprintf(pOCPP->szSendBuffer, ES_iOCPP_MAX_SENTMSG_SIZE, "{\"id\":\"Q%d\",	\"uniqKey\":\"%d\",	\"content\":%s}",
		pOCPP->stMsgInfoList[pMsg->emMsgType].nMsgId, nMsgUniqID,
		pOCPP->activeCallMsg.msgData.szJsonData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);

	// 2021-11-15 Encrpt For Test

	if(pOCPP->bZlibCompress == TRUE)
	{
		int err = 0;
		pOCPP->nCompressSendBuffLen = ES_iOCPP_MAX_SENTMSG_SIZE;
		err = compress(pOCPP->szCompressSendBuff, &pOCPP->nCompressSendBuffLen, (const Bytef*)pOCPP->szSendBuffer, pOCPP->nSendBuffLen);
		if(err != Z_OK)
		{
			TRACE("Error: err=%d pOCPP->nCompressSendBuffLen=%d\n", err, pOCPP->nCompressSendBuffLen);
		}

		TRACEX("Compress: The sent out of WS pkg is [%s], nSendBuffLen = %d,  nCompressSendBuffLen=[%d]\n", \
			pOCPP->szSendBuffer, pOCPP->nSendBuffLen,  pOCPP->nCompressSendBuffLen);

		return pOCPP->szCompressSendBuff;
	}
	else
	{
		//---------John 2021-12-14 no need compress send to cloud Platform-------------//
		TRACEX("Uncompress:pkg is [%s], nSendBuffLen = %d\n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);
		pOCPP->nCompressSendBuffLen = pOCPP->nSendBuffLen;
		return pOCPP->szSendBuffer;
	}

}

static BOOL Wait_Ready(REPORTER* pReporter)
{
	ES_iOCPP_DATA_STU* pOCPP = (ES_iOCPP_DATA_STU*)pReporter->pDataHeap;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;
	static time_t s_tmLastSentBootNotif = 0;
 
	static ES_iOCPP_COMM_STATUS s_emOldConnStatus = Connected;
	static int s_nConnStatusStableCount = 0;
	if((pOCPP->emOCPPConnStatus != s_emOldConnStatus) &&
		((pOCPP->emOCPPConnStatus == Disconnected) || (pOCPP->emOCPPConnStatus == Ready)))
	{
		s_nConnStatusStableCount++;
		if(s_nConnStatusStableCount >= 3)
		{
			s_nConnStatusStableCount = 0;
			s_emOldConnStatus = pOCPP->emOCPPConnStatus;
			VAR_VALUE varV;
			varV.emValue = (s_emOldConnStatus == Disconnected) ? 1 : 0;
			TRACED("IOCPP-server_commom_status [%d]",varV.emValue);
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 29, &varV, 4, 100);
		}
	}
	else
	{
		s_nConnStatusStableCount = 0;
	}

	if(pOCPP->emOCPPConnStatus == Ready)
	{
		pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_NO_FAIL;
		return TRUE;
	}

	int nWaitResent_Boot_Sec = 3; //if it is 0, charger uses 3 as default
	nWaitResent_Boot_Sec = MIN(nWaitResent_Boot_Sec, 10); //Jimmy note 20190215:if heartbeatInterv is very large, could be very slow to try again

	Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);

	TRACEX("Now OCPP status = %d, tmDiff = %d, nWaitResent_Boot_Sec = %d \n", 
		pOCPP->emOCPPConnStatus, ABS(tmNow - s_tmLastSentBootNotif) , nWaitResent_Boot_Sec);

	if(pOCPP->emOCPPConnStatus == Disconnected)
	{
		Mutex_Unlock(pOCPP->hMutex); //Jimmy note 20190412: do not lock here in case the event_handler will be blocked
		TRACEX("ThreadEntry_OCPPComm Enter\n");
		//pOCPP->emOCPPConnStatus = Disconnected;
		TryConnect2Server(pReporter);
		TRACEX("TryConnect2Server Out \n");
		if (pReporter->hCommPort == NULL)
		{
			pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_SERVRER_BROKEN;
			//Mutex_Unlock(pOCPP->hMutex);
			Sleep(6000);
			//return FALSE;
		}
		else
		{
			pOCPP->emOCPPConnStatus = Connected;
		}

		return FALSE;
	}
	else if(pOCPP->emOCPPConnStatus == Connected)
	{
		pOCPP->bHasActiveMsg = FALSE;
		pOCPP->bAuthorizeInProcess = FALSE; //added to clear the Auth Status in case auth not resending
		if (!pOCPP->bBootNotifyDone)
		{
			pOCPP->stMsgInfoList[C_100_BootNotif].pfnMsgHandler((void*)pOCPP);
			TRACEX("BootNotification Trigger pOCPP->bBootNotifyDone=%d\n",pOCPP->bBootNotifyDone);
			//if write error, then we need to goto disconnected again
			if(pOCPP->emOCPPConnStatus != Disconnected)
			{
				pOCPP->emOCPPConnStatus = Wait4BootNotify_Conf;
				s_tmLastSentBootNotif = tmNow;
			}
		}
		else
		{
			//Jimmy note 20190111: do not empty 'cause some msg need to be restored after connection recovered
			Queue_Empty(pOCPP->hqMsg2Sent); //clear any history queued msg
			Queue_Empty(pOCPP->hqHistoryTrans);
			//Need trigger StatusNotif.req soon after reconnect
			if(!pOCPP->bCPStatusChanged)
			{
				pOCPP->tmLastCPStatusChange = 0;
			}
	 
			pOCPP->tmLastSendHeartBeat = tmNow; //do not send heartbeat on first connected
			pOCPP->emOCPPConnStatus = Ready;
			pOCPP->bNeedRefreshActvAlm = TRUE;

			TRACEX("Now Ready for comm, so empty Q first \n");
		}
	}
	else if(pOCPP->emOCPPConnStatus == Wait4BootNotify_Conf) //waiting BootNotic result
	{
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, ES_iOCPP_BUFFER_LEN);
		TRACEX("Wait bootNotif.conf nReadLen = %d\n", pOCPP->nReadBuffLen);
		if(pOCPP->nReadBuffLen > 0)
		{
			// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);//edit by kuangyue 20250315
			LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
			Parse_ESiOCPPFrames(pOCPP);
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pOCPP->emOCPPConnStatus = Connected;
			//pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
			//pOCPP->emOCPPConnStatus = Disconnected;
		}
	}
	// else if(pOCPP->emOCPPConnStatus == BootNotify_Pending) //now we can handling OCPP configure setting things 
	// {
	// 	if((pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, OCPP_BUFFER_LEN)) > 0)
	// 	{
	// 		DUMP_OCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
	// 		Parse_OCPPFrames(pOCPP);
	// 	}

	// 	if(pOCPP->bBootNotifyDone)
	// 	{
	// 		pOCPP->emOCPPConnStatus = Connected;
	// 	}
	// 	else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
	// 	{
	// 		pOCPP->emOCPPConnStatus = Connected;
	// 	}
	// }
	else if(pOCPP->emOCPPConnStatus == BootNotify_Rejected)
	{
		static time_t s_tmLastRejected = 0;
		if(s_tmLastRejected == 0)
		{
			s_tmLastRejected = tmNow;
		}

		if(pOCPP->bBootNotifyDone)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pOCPP->emOCPPConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastRejected) > 60)
		{
			LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR,
				"BootNotif was refused by server, now disconnect and try again...");
			s_tmLastRejected = tmNow;
			pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_REFUSED;
			pOCPP->emOCPPConnStatus = Disconnected;
		}
	}

	Mutex_Unlock(pOCPP->hMutex);

	//TRACEX("pOCPP->emOCPPConnStatus = %d \n", pOCPP->emOCPPConnStatus);

	return (pOCPP->emOCPPConnStatus == Ready) ? TRUE : FALSE;
}

static void Handle_Call_Resend(ES_iOCPP_DATA_STU* pOCPP, COMMPORT* pComm)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	if(pOCPP->activeCallMsg.nRetryTimes >= MAX_ES_iOCPP_SEND_RETRIES)
	{
		TRACEX("----Resend Msg[%d] TIMEOUT!!!-----\n", pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId);
		pOCPP->bHasActiveMsg = FALSE;
		//pOCPP->nConnErrTimes++;
		pOCPP->nConnErrTimes = 3; //Jimmy note: found that even disconnected, write can be still done, so here once resend expires 3 times, we disconnect.
	}
	else
	{
		TRACEX("----Resend Msg[%d] RetryTimes [%d], jSon [%s]-----\n", pOCPP->stMsgInfoList[pOCPP->activeCallMsg.msgData.emMsgType].nMsgId, 
			pOCPP->activeCallMsg.nRetryTimes, pOCPP->activeCallMsg.msgData.szJsonData);

		ES_iOCPP_CALL_MSG* pMsg = &pOCPP->activeCallMsg.msgData;
		if (pComm->Write(pReporter->hCommPort,
			MakeESiOCPP_CallFrame(pOCPP, pMsg, TRUE),
			/*pOCPP->nSendBuffLen*/pOCPP->nCompressSendBuffLen))
		{
			// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);//edit by kuangyue 20250315
			LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
			//sent successfully
			//it is possible that SERVER just does not reply but still can send data
			//pOCPP->nConnErrTimes = 0;
			TRACE("Info: pOCPP->nConnErrTimes=%d\n", pOCPP->nConnErrTimes);
		}
		else
		{
			pOCPP->nConnErrTimes++;
		}
	}

	if (pOCPP->nConnErrTimes >= 3)
	{
		pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
		pOCPP->nConnErrTimes = 0;
		pOCPP->emOCPPConnStatus = Disconnected;
		TRACEX("emOCPPConnStatus : Disconnected \n");
		LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR, "Handle_Call_Resend more than 3 times, disconnected.");
	}
}

static HANDLE Dispatch_Q(ES_iOCPP_DATA_STU* pOCPP)
{
	if(pOCPP->nQUrgentMsgCount > 0)
	{
		pOCPP->nQUrgentMsgCount--;
		TRACEX("-------Now has urgent msg to send out[%d]-------\n", pOCPP->nQUrgentMsgCount);
		return pOCPP->hqMsg2Sent;
	}

	static HANDLE s_LastHanlded_InProcQ = NULL;
	HANDLE hqList[Max_Connctors + 1];
	int n;
	//Strategy: get queue from hqSent/hqHis/hqInProc in-turn
	if(s_LastHanlded_InProcQ == pOCPP->hqMsg2Sent)
	{
		TRACEX("Info: s_LastHanlded_InProcQ == pOCPP->hqMsg2Sent\n");
		// hqList[0] = pOCPP->stConnectorData[Connctor_1].hqInProcessTrans;
		// hqList[1] = pOCPP->stConnectorData[Connctor_2].hqInProcessTrans;
		// hqList[2] = pOCPP->stConnectorData[Connctor_3].hqInProcessTrans;
		for(n = 0; n < Max_Connctors; n++)
		{
			hqList[n] = pOCPP->stConnectorData[Connctor_1 + n].hqInProcessTrans;
		}
		hqList[Max_Connctors] = pOCPP->hqMsg2Sent;
	}
	else
	{
		TRACEX("Info: s_LastHanlded_InProcQ != pOCPP->hqMsg2Sent\n");

		int nCurrQIdx = Connctor_1;
		for(n = 0; n < Max_Connctors; n++)
		{
			if(s_LastHanlded_InProcQ == pOCPP->stConnectorData[Connctor_1 + n].hqInProcessTrans)
			{
				nCurrQIdx = Connctor_1 + n;
				break;
			}
		}

		int nIdx = 0;
		//bottom part
		for(n = nCurrQIdx + 1; n < Max_Connctors; n++)
		{
			if(nCurrQIdx + 1 >= Max_Connctors)
			{
				break;
			}
			hqList[nIdx] = pOCPP->stConnectorData[n].hqInProcessTrans;
			nIdx++;
		}
		//middle part
		hqList[nIdx] = pOCPP->hqMsg2Sent;
		nIdx++;
		//up part
		for(n = 0; n <= nCurrQIdx; n++)
		{
			hqList[nIdx] = pOCPP->stConnectorData[n].hqInProcessTrans;
			nIdx++;
		}
	}	

	HANDLE hq = NULL;
	for(n = 0; n < ITEM_OF(hqList); n++)
	{
		if(Queue_GetCount(hqList[n], NULL) > 0)
		{
			hq = hqList[n];
			s_LastHanlded_InProcQ = hq;
			break;
		}
	}

	if((hq == NULL) && (Queue_GetCount(pOCPP->hqHistoryTrans, NULL) > 0))
	{
		hq = pOCPP->hqHistoryTrans;
	}

	TRACEX("------The dispatched Q is %d-----\n", hq);
	return hq;
}

static void Handle_Queued_Call(ES_iOCPP_DATA_STU* pOCPP, COMMPORT* pComm)
{
	ES_iOCPP_CALL_MSG stSentMsg;
	ES_iOCPP_CALL_MSG* pMsg = &stSentMsg;
	//static HANDLE s_LastHanlded_InProcQ = NULL;
	//Strategy: get queue from hqSent/hqHis/hqInProc in-turn
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	time_t tmNow = _NOW_;//edit by kuangyue 20250318

	TRACE("Info: Handle_Queued_Call Dispatch_Q\n");
	HANDLE hq = Dispatch_Q(pOCPP);
	if(hq == NULL)
	{
		return;
	}

	if(Mutex_Lock(pOCPP->hMutex, 100) == ERR_MUTEX_OK)
	{
		if(Queue_Get(hq, &stSentMsg, FALSE, 100) == ERR_QUEUE_OK)
		{
			if (pComm->Write(pReporter->hCommPort,
					MakeESiOCPP_CallFrame(pOCPP, pMsg, FALSE),
					/*pOCPP->nSendBuffLen*/pOCPP->nCompressSendBuffLen))
			{
				ES_iOCPP_MSG_TYPE currentType = pOCPP->activeCallMsg.msgData.emMsgType;
				TRACED("----Kuangyue10 currentType = %d----\n", currentType);
				if (recordedMessages[currentType]) 
				{
					TRACED("Message type %d has been recorded, skipping addition to queue\n", currentType);
				}
				else if (queueSize < Max_ES_iOCPP_Msgs) 
				{
					messageQueue[queueSize].type = currentType;
					strncpy(messageQueue[queueSize].szSendBuffer, pOCPP->szSendBuffer, sizeof(messageQueue[queueSize].szSendBuffer));
					messageQueue[queueSize].timestamp = tmNow;
					queueSize++;
				}

				if (pOCPP->nLog_Interval > 0)
				{
					manageMessageQueue(pOCPP->nLog_Interval);
				}
				else if(pOCPP->nLog_Interval == 0)// edit by kuangyue 20250318
				{
					// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);//edit by kuangyue 20250315
					// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
					LOG_INFO("Message Type: %d, Content: %s\n", currentType, pOCPP->szSendBuffer);
					//sent succsessfully
					//pOCPP->nConnErrTimes = 0;
				}
				else
				{
					LogOut(MAIN_MODULE_REPORTER, "pOCPP", LOG_TYPE_ERROR,
						"nLog_Interval was false...");
					return; //do not log if nLog_Interval is negative
				}
			}
			else
			{
				pOCPP->nConnErrTimes++;
				if (pOCPP->nConnErrTimes >= 3)
				{
					pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pOCPP->nConnErrTimes = 0;
					pOCPP->emOCPPConnStatus = Disconnected;
					TRACEX("emOCPPConnStatus : Disconnected \n");
					LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR, "Handle_Queued_Call more than 3 times, disconnected.");
				}
			}
			
		}
		Mutex_Unlock(pOCPP->hMutex);
	}
	return;
}

/*
static BOOL PushHistoricalTrans_2Q(ES_iOCPP_DATA_STU* pOCPP)
{
	int n;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	QUERYCOND_CHARGERECORD recCond;
	QUERY_COND cond;
	recCond.nReportFlag = 0;
	cond.nArgSize = sizeof(QUERYCOND_CHARGERECORD);
	cond.pCondArg = &recCond;
	cond.nQueryCondType = QUERY_COND_REPORTED_FLAG;
	UINT nQueryCount = 1;
	ERRCODE_DAI errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_CHARGEREC, 
		cond, (void*)&pOCPP->recInHandling_HistoryChargeRec, &nQueryCount);
	if((errCode != ERRCODE_DAI_OK) || (nQueryCount < 1))
	{
		nQueryCount = 1;
		recCond.nReportFlag = REPORT_FLAG_OCPP_BEGIN;
		errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_CHARGEREC, 
			cond, (void*)&pOCPP->recInHandling_HistoryChargeRec, &nQueryCount);
	}

	TRACEX("-------Query history charger record fo OCPP, nQueryCount = %d----------\n", nQueryCount);
	if((errCode == ERRCODE_DAI_OK) && (nQueryCount > 0))
	{
		Mutex_Lock(pOCPP->hMutex, WAIT_INFINITE);
		pOCPP->bTransMsgFromHistory = FALSE;
		pOCPP->emHandlingConnector = pOCPP->recInHandling_HistoryChargeRec.emGunNum - 1;
		if((pOCPP->emHandlingConnector < 0) ||
			(pOCPP->emHandlingConnector > Connctor_3))
		{
			//Jimmy: there is error, in future need think about how to handle this error, because if just skip it this time, another time will get same record!
			Mutex_Unlock(pOCPP->hMutex);
			return FALSE;
		}
		//Jimmy 20190319: just ignore the in-process session in case sending repeatedly
		int nConnIdx = pOCPP->emHandlingConnector;
		if((pOCPP->stConnectorData[nConnIdx].blobInHandling_ChargeInfo.tmChargeStart == 
			pOCPP->recInHandling_HistoryChargeRec.tmChargeStart) && 
			((Queue_GetCount(pOCPP->stConnectorData[nConnIdx].hqInProcessTrans, NULL) > 0) ||
			(pOCPP->stConnectorData[nConnIdx].nGunChargeStatus >= GUN_STATE_WAITING_START)))
		{
			Mutex_Unlock(pOCPP->hMutex);
			return FALSE;
		}		
 

		Mutex_Unlock(pOCPP->hMutex);

		return pOCPP->bTransMsgFromHistory;
	}
	return FALSE;
}
*/
BOOL Check_IfStatusChanged(ES_iOCPP_DATA_STU* pOCPP, int nDataFrom)
{
	PARAM_VAL_GET valGet;
	//ERRCODE_DAI err;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	BOOL bStatusHasChanged = FALSE;

	pOCPP->blobCPStatusInfo.emChargePointStatus = CP_STATUS_Available; //initial state
	pOCPP->blobCPStatusInfo.emChargePointErrcode = CP_ERRCODE_NoError;
	valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
	valGet.val.blobValue = (BYTE*)&pOCPP->blobCPStatusInfo;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 10, &valGet, sizeof(PARAM_VAL_GET));
	
	//first judge sytem status change?
	//A_BLOB_CAHRGEPOINT_STATUSINFO* pStatusInfo = &pOCPP->blobCPStatusInfo;
	static int s_nOldSysStatus = -1;
	pOCPP->emSysStatus_For101 = 0;
	if(pOCPP->blobCPStatusInfo.emChargePointStatus == CP_STATUS_Unavailable)
	{
		pOCPP->emSysStatus_For101 = 1;
	}
	else if(pOCPP->blobCPStatusInfo.emChargePointStatus == CP_STATUS_Faulted)
	{
		pOCPP->emSysStatus_For101 = 2;
	}

	if(s_nOldSysStatus != pOCPP->emSysStatus_For101)
	{
		s_nOldSysStatus = pOCPP->emSysStatus_For101;
		bStatusHasChanged = TRUE;
	}

	return bStatusHasChanged;
}

BOOL Check_IfSTBattGrpStatusChanged(ES_iOCPP_DATA_STU* pOCPP)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	static int s_nOldSTBattStatus = -1;
	BOOL bSTBattStatuChged = FALSE;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;

	//ST Batt Stat Data
	A_BLOB_GROUP_STBATT_RUNINFO blobGrpSTBattData;
    memset(&blobGrpSTBattData, 0 ,sizeof(A_BLOB_GROUP_STBATT_RUNINFO));
	valGet.nGetSize = sizeof(A_BLOB_GROUP_STBATT_RUNINFO);
	valGet.val.blobValue = (BYTE*)&blobGrpSTBattData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			500, 503, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Get A_BLOB_GROUP_STBATT_RUNINFO Error!!!\n");
		return bSTBattStatuChged;
	}
   
    if(s_nOldSTBattStatus != blobGrpSTBattData.emBattGrpRunStatus)
	{
		s_nOldSTBattStatus = blobGrpSTBattData.emBattGrpRunStatus;

		bSTBattStatuChged = TRUE;
	}

	return bSTBattStatuChged;
}

BOOL Check_IfActvAlmStatuChanged(ES_iOCPP_DATA_STU* pOCPP, int nDataFrom)
{
	BOOL bRet = FALSE;
	if(nDataFrom == 0)
	{
		if(pOCPP->bNeedRefreshActvAlm)
		{
			pOCPP->bNeedRefreshActvAlm = FALSE;
			QueryActvAlarmInfo(pOCPP);
			return TRUE;
		}
	}
	else
	{
		//TRACE("\n");
	}
	return bRet;
}

BOOL ShieldAlarms_StringToUINT(ES_iOCPP_DATA_STU* pOCPP)
{
	TRACEW("allow to shield alarms [%s]",pOCPP->szShieldAlarms);
	UINT index = 0 ,ncount = 1;
	char szInput[512];

	ZERO_POBJS(pOCPP->ShieldAlarms, pOCPP->nAlarmInfosTotal);   //clear buff
    if (pOCPP->szShieldAlarms[0] == '\0' || strcmp(pOCPP->szShieldAlarms, "alm_id/alm_id/alm_id") == 0) 
	{
		TRACEE("NO alarm to be shield ...");
		pOCPP->nShieldAlarmsNum = 0;
        return FALSE;
    }

    for (index = 0; pOCPP->szShieldAlarms[index] != '\0'; index++) 
	{
        if (pOCPP->szShieldAlarms[index] == '/') 
		{
            ncount++;
        }
    }
	pOCPP->nShieldAlarmsNum = ncount;

	strncpy(szInput, pOCPP->szShieldAlarms, 512);
	szInput[511] = '\0'; 

	char *token = strtok(szInput, "/");
    index = 0;

    while (token != NULL && index < ncount) 
	{
        pOCPP->ShieldAlarms[index] = (UINT)strtoul(token, NULL, 10);
        token = strtok(NULL, "/");
		index++;
    }

	// printf("pOCPP->ShieldAlarms = ");
	// for (index=0;index < ncount;index++)
	// {
	// 	printf("[%d] ",pOCPP->ShieldAlarms[index]);
	// }
	// printf("\n");

    return TRUE;

}

void Pro_SysCmdDataInfo(ES_iOCPP_DATA_STU* pOCPP, int nDataFrom)
{
	TRACE("Info: nDataFrom=[%d]\n", nDataFrom);
	time_t tmNow = _NOW_;
	PARAM_VAL_GET 	valGet;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	
	BOOL bSysStatuChanged = Check_IfStatusChanged(pOCPP, nDataFrom);

	if((ABS(tmNow - pOCPP->tmLastSent_101[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_101_Interval)) || bSysStatuChanged)
	{
		pOCPP->tmLastSent_101[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pOCPP); 
	}

	if(ABS(tmNow - pOCPP->tmLastSent_102[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_102_Interval))
	{
		pOCPP->tmLastSent_102[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pOCPP);
	}

 	if(ABS(tmNow - pOCPP->tmLastSent_103[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_103_Interval))
	{
		if (pOCPP->lcCfg.bSendCmdByLC1stOnly)
		{
			Prepare_SEMS_C103_ESBMSStatusNotif((void*)pOCPP);
		}
		else
		{
			pOCPP->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pOCPP);
		}

		pOCPP->tmLastSent_103[nDataFrom] = tmNow;
		
	}

	if(ABS(tmNow - pOCPP->tmLastSent_104[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_104_Interval))
	{
		pOCPP->tmLastSent_104[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_104_PwrDistrStaNotif].pfnMsgHandler((void*)pOCPP);
	}

	if(ABS(tmNow - pOCPP->tmLastSent_105[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_105_Interval))
	{
		pOCPP->tmLastSent_105[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pOCPP);
	}

	if(ABS(tmNow - pOCPP->tmLastSent_106[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_106_Interval))
	{
		pOCPP->tmLastSent_106[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_106_SingleRectStaNotif].pfnMsgHandler((void*)pOCPP);
	}

	if(ABS(tmNow - pOCPP->tmLastSent_107[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_107_Interval))
	{
		pOCPP->tmLastSent_107[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pOCPP);
	}

	if(ABS(tmNow - pOCPP->tmLastSent_108[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_108_Interval))
	{
		pOCPP->tmLastSent_108[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_108_SysVersionStaNotif].pfnMsgHandler((void*)pOCPP);
	}
	
	if (pOCPP->lcCfg.bSendCmdByLC1stOnly)
	{
	//	send_SEMS_AlarmStatusChanged((void*)pOCPP);

		if((ABS(tmNow - pOCPP->tmLastSent_109[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_109_Interval)))
		{
		//	PrePare_SEMS_C109_SysActvAlmNotif((void*)pOCPP);
			pOCPP->tmLastSent_109[nDataFrom] = tmNow;
		}
	}
	else
	{
		BOOL bActvAlmChanged = Check_IfActvAlmStatuChanged(pOCPP, nDataFrom);
		if((bActvAlmChanged == TRUE)||
		(ABS(tmNow - pOCPP->tmLastSent_109[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_109_Interval)))
		{
			ShieldAlarms_StringToUINT(pOCPP);
			TRACE("Info:nDataFrom=[%d] Refresh Actv Alarms\n",nDataFrom);
			pOCPP->stMsgInfoList[C_109_SysActvAlmNotif].pfnMsgHandler((void*)pOCPP);
			pOCPP->tmLastSent_109[nDataFrom] = tmNow;
		}
	}


	if(pOCPP->blobBackendSet.nCmd_110_Interval != 0) //0 Means Not send To Cloud Platform.
	{
		//if(ABS(tmNow - pOCPP->tmLastSent_110[nDataFrom]) > (MAX(10, pOCPP->blobBackendSet.nCmd_110_Interval)))
		if(ABS(tmNow - pOCPP->tmLastSent_110[nDataFrom]) >= (MAX(1, pOCPP->blobBackendSet.nCmd_110_Interval)))
		{
			if (pOCPP->lcCfg.bSendCmdByLC1stOnly)
			{
			//	 Prepare_SEMS_C110_BatDataNotif((void*)pOCPP);
			}
			else
			{
				int n = 0;
				for(n = 0; n < pOCPP->nCfgBMSNum; n++)
				{
					pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
						501 + n, 550, &valGet, sizeof(PARAM_VAL_GET));
					if(valGet.val.emValue <= STBATT_RUNSTAT_COMM_FAIL)
					{
						continue;
					}
					pOCPP->nCurrBMS = n;
					pOCPP->stMsgInfoList[C_110_SingleBattryInfo].pfnMsgHandler((void*)pOCPP);
				}
			}
			
			pOCPP->tmLastSent_110[nDataFrom] = tmNow;
		}
	}

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 545, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue != 0)
	{
		int n = 0;
		for(n = 0; n < pOCPP->nCfgBMSNum; n++)
		{	
			int nDevID = 0, nSigID = 0;
			if(pOCPP->nCurrDataFrom == 0)
			{
				nDevID = 501 + n;
				nSigID = 551;
			}
			else
			{
				nDevID = 2;
				nSigID = 1050 + n;
			}
			ERRCODE_DAI 		errBatt;
			A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
			ZERO_POBJS(&blobSiglBatt, 1);
			blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
			valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
			valGet.val.blobValue = (BYTE*)&blobSiglBatt;
			errBatt = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
			if(errBatt != ERRCODE_DAI_OK)
			{
				continue;
			}

			if(pOCPP->nCurrDataFrom == 0)
			{
				nDevID = 200;
				nSigID = 80;
			}
			else
			{
				nDevID = 2;
				nSigID = 1060;
			}
			ERRCODE_DAI 		errES;
			A_BLOB_ES_RUN_DATA	esRunData;
			ZERO_POBJS(&esRunData, 1);
			esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
			valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
			valGet.val.blobValue = (BYTE*)&esRunData;
			errES = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
			if(errES != ERRCODE_DAI_OK)
			{
				continue;
			}

			if((blobSiglBatt.fMin_CellVolt < 3.2 && esRunData.fSTBatt_OutCurr[n] < -2.0f) 
				|| (blobSiglBatt.fMax_CellVolt > 3. && esRunData.fSTBatt_OutCurr[n] > 2.0f))
			{
				if(ABS(tmNow - pOCPP->tmLastSent_110[nDataFrom]) >= 1)
				{
					if (pOCPP->lcCfg.bSendCmdByLC1stOnly)
					{
					//	 Prepare_SEMS_C110_BatDataNotif((void*)pOCPP);
					}
					else
					{	
						pOCPP->nCurrBMS = n;
						pOCPP->stMsgInfoList[C_110_SingleBattryInfo].pfnMsgHandler((void*)pOCPP);				
					}

					pOCPP->tmLastSent_110[nDataFrom] = tmNow;
				}	
			}		
		}
	}

	if(pOCPP->blobBackendSet.nCmd_112_Interval > 0 && ABS(tmNow - pOCPP->tmLastSent_112[nDataFrom]) >= MAX(3, pOCPP->blobBackendSet.nCmd_112_Interval))
	{
		pOCPP->tmLastSent_112[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_112_PVInverterDataNotif].pfnMsgHandler((void*)pOCPP);
	}

	int nValue = tmNow - pOCPP->tmLastSent_202[nDataFrom];
	TRACEX("tmNow:[%ld] - pOCPP->tmLastSent_202[%d]:%ld = %d\n",\
		tmNow, nDataFrom, pOCPP->tmLastSent_202[nDataFrom], nValue);
	if(ABS(tmNow - pOCPP->tmLastSent_202[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_202_Interval))
	{
		if (pOCPP->lcCfg.bSendCmdByLC1stOnly)
		{
			//Prepare_SEMS_C202_HourSectorStaticKwh((void*)pOCPP);
		}
		else
		{
			pOCPP->stMsgInfoList[C_202_HourSectorStaticKwh].pfnMsgHandler((void*)pOCPP);
		}
		pOCPP->tmLastSent_202[nDataFrom] = tmNow;
		
	}

	if(ABS(tmNow - pOCPP->tmLastSent_204[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_204_Interval))
	{
		pOCPP->tmLastSent_204[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_204_DailyStaticKwh].pfnMsgHandler((void*)pOCPP);
	}
	if(pOCPP->blobBackendSet.nCmd_206_Interval > 0 && ABS(tmNow - pOCPP->tmLastSent_206[nDataFrom]) >= MAX(1, pOCPP->blobBackendSet.nCmd_206_Interval))
	{
		pOCPP->tmLastSent_206[nDataFrom] = tmNow;
		pOCPP->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pOCPP);
	}

	if(pOCPP->stCloudUpload.bUploadEnable && (ABS(tmNow - pOCPP->stCloudUpload.tmLastUpload) >= MAX_CLOUDUPLOAD_INTERNAL))
	{
		ReadHistoryData((void*)pOCPP);
	}

	return;
}

void QueryActvAlarmInfo(ES_iOCPP_DATA_STU* pOCPP)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI errCode;
	int n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize = sizeof(EVENT_ALARM);
	cond.pCondArg = &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount = MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pOCPP->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pOCPP->activeAlms , &nQueryCount);
	TRACE("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);
	TRACEI("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

	if(errCode != ERRCODE_DAI_OK)
	{
		TRACE("Error: Query Actv Alarm Failed!\n");
 	}
	else
	{
		pOCPP->nActiveAlmTotal = MIN(nQueryCount, MAX_QUERY_ACTV_ALM);
 		for(n = 0; n < pOCPP->nActiveAlmTotal; n++)
		{
			TRACE("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pOCPP->activeAlms[n].nDevId, pOCPP->activeAlms[n].nAlarmId);
			TRACEI("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pOCPP->activeAlms[n].nDevId, pOCPP->activeAlms[n].nAlarmId);
		}
	}

	return;
}


static DWORD _ThreadEntry_OCPPComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	TRACEX("_ThreadEntry_OCPPComm: Start Enter \n");
	int n = 0;
	ES_iOCPP_DATA_STU* pOCPP = (ES_iOCPP_DATA_STU*)pReporter->pDataHeap;
	//int nWaitRebootMsgCount = 0;
	//time_t tmLastQueryHisStatisKWH = 0;
	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
 
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		time_t tmNow = _NOW_;
		
 		if(!Wait_Ready(pReporter))
		{
			Sleep(100);
			continue;
		}

		//Now ready
		if(pOCPP->bHasActiveMsg)
		{
			if(ABS(tmNow - pOCPP->activeCallMsg.tmLastSent) > pOCPP->nMsgCallTimeOut_Sec)
			{
				//Resend_Msg
				Handle_Call_Resend(pOCPP, pComm);
			}
		}
		else
		{
			Handle_Queued_Call(pOCPP, pComm);
		}

		//added by Jimmy 20190702 for OCTT test. if disconn, should skip going ahead
		if(pOCPP->emOCPPConnStatus == Disconnected)
		{
			TRACE("Info : pOCPP->emOCPPConnStatus =%d\n", pOCPP->emOCPPConnStatus);
			continue;
		}
		else if(pOCPP->bNoticedExitOCPP)
		{
			if(pOCPP->nQUrgentMsgCount < 1)
			{
				TRACEX("-----Now ready for OCPP exiting-----\n");
				pOCPP->bNoticedExitOCPP = FALSE;
				pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_SYSTEM_EXIT;
				pOCPP->emOCPPConnStatus = Disconnected; //need handle urgent msg before exit
				LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_INFO, "Noticed Exit OCPP, disconnected.");
				continue;
			}
		}
		
		//Now read and parse
		pOCPP->emTriggerMsgType = Trigger_NoTriggle;
		pOCPP->nReadBuffLen = pComm->Read(pReporter->hCommPort, pOCPP->szReadBuffer, ES_iOCPP_BUFFER_LEN);
		if(pOCPP->nReadBuffLen> 0)
		{
			// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);//edit by kuangyue 20250315
			// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_READ, pOCPP->szReadBuffer);
			Parse_ESiOCPPFrames(pOCPP);
		}
		else if(pOCPP->nReadBuffLen == -1) //means server close session
		{
			TRACEX("Server Connection Error Close the Socket\n");
			pOCPP->nReadBuffLen = 0;      // need to make Zero Otherwise same Case Excuting Continue
			pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_SERVRER_BROKEN;
			pOCPP->emOCPPConnStatus = Disconnected;
			LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_INFO, "Server Connection Error Close the Socket, disconnected.");
			Sleep(100);
			continue;
		}
		else
		{
			//TRACE("Info: nReadBuffLen=%d\n", pOCPP->nReadBuffLen);
		}
		
  
		//------ Start Process Slave System DataInfo ------//
		for(n = 0; n < MAX_SUPPORTED_CLIENTS; n++)
		{	
			pOCPP->nCurrDataFrom = n;
			if(n == 0)
			{
				Pro_SysCmdDataInfo(pOCPP, pOCPP->nCurrDataFrom); //0-Master System 
				continue;
			}
			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 144 + n -1, &valGet, sizeof(PARAM_VAL_GET));
			pOCPP->nSlvCommStatu[n-1] = valGet.val.emValue;
			if((err != ERRCODE_DAI_OK) || (valGet.val.emValue != 2)) //0-NotInstall, 1-CommFail, 2-Normal
			{
				continue;
			}
 			Pro_SysCmdDataInfo(pOCPP, pOCPP->nCurrDataFrom); //0-Master System 	
		}
 
		//Now check historical msg queue
		/*static int nQueryInterv = 30;  //do query every 30 sec
		if((!pOCPP->bHasActiveMsg) || (ABS(tmNow - tmLastQueryHisChgRec) >= nQueryInterv))
		{
			TRACEX("Now we handle histor record, nQuerenInterv = %d\n", nQueryInterv);
 
			if(Queue_GetCount(pOCPP->hqHistoryTrans, NULL) == 0)
			{
				tmLastQueryHisChgRec = tmNow;
				if(PushHistoricalTrans_2Q(pOCPP)) //means we have more data to handle
				{
					nQueryInterv = 5;
				}
				else
				{
					nQueryInterv = 30;
				}
			}
		}*/

		Sleep(100);
	}

	TRACE("_ThreadEntry_OCPPComm: Exit\n");
	return THREAD_CANCEL_ALL;
}

#define INIT_ESiOCPP_MSG(_pMsg,_emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)


void InitOCPPMsgInfo(ES_iOCPP_DATA_STU* pOCPP)
{
	ES_iOCPP_MSG_INFO* p = &(pOCPP->stMsgInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_ESiOCPP_MSG(p,	C_100_BootNotif,				100,			ES_iOCPP_MSG_CP,		Prepare_C100_BootNotification);
	INIT_ESiOCPP_MSG(p,	C_101_StatusNotif,				101,			ES_iOCPP_MSG_CP,		Prepare_C101_StatusNotification);
	INIT_ESiOCPP_MSG(p,	C_102_BattCabStaNotif,			102,			ES_iOCPP_MSG_CP,		Prepare_C102_BattCabStaNotif);
	INIT_ESiOCPP_MSG(p,	C_103_ESBMSStaNotif,			103,			ES_iOCPP_MSG_CP,		Prepare_C103_ESBMSStatusNotif);
	INIT_ESiOCPP_MSG(p,	C_104_PwrDistrStaNotif,			104,			ES_iOCPP_MSG_CP,		Prepare_C104_PwrDistrStaNotif);
	INIT_ESiOCPP_MSG(p,	C_105_ESRunDataStaNotif,		105,			ES_iOCPP_MSG_CP,		Prepare_C105_ESRunDataNotif);
	INIT_ESiOCPP_MSG(p,	C_106_SingleRectStaNotif,		106,			ES_iOCPP_MSG_CP,		Prepare_C106_SingleRectStaNotif);
	INIT_ESiOCPP_MSG(p,	C_107_PwrDistrCabStaNotif,		107,			ES_iOCPP_MSG_CP,		Prepare_C107_PwrDistrCabStaNotif);
	INIT_ESiOCPP_MSG(p,	C_108_SysVersionStaNotif,		108,			ES_iOCPP_MSG_CP,		Prepare_C108_SysVersionStaNotif);
	INIT_ESiOCPP_MSG(p,	C_109_SysActvAlmNotif,			109,			ES_iOCPP_MSG_CP,		Prepare_C109_SysActvAlmNotif);
	INIT_ESiOCPP_MSG(p,	C_110_SingleBattryInfo,			110,			ES_iOCPP_MSG_CP,		Prepare_C110_SiglBatDataNotif);
	INIT_ESiOCPP_MSG(p,	C_112_PVInverterDataNotif,		112,			ES_iOCPP_MSG_CP,		Prepare_C112_PVInverterDataNotif);

	INIT_ESiOCPP_MSG(p,	C_155_ESRunDataStaNotif,		155,			ES_iOCPP_MSG_CP,		Prepare_C105_ESRunDataNotif); // LJH250428 cmd155

	INIT_ESiOCPP_MSG(p,	S_111_Trigger,					111,			ES_iOCPP_MSG_CS,		Handle_S111_TriggerMessage);
 	INIT_ESiOCPP_MSG(p,	C_202_HourSectorStaticKwh,		202,			ES_iOCPP_MSG_CP,		Prepare_C202_HourSectorStaticKwh);
	INIT_ESiOCPP_MSG(p,	C_204_DailyStaticKwh,			204,			ES_iOCPP_MSG_CP,		Prepare_C204_DailyStaticKwh);
	INIT_ESiOCPP_MSG(p,	C_206_FeeRateKwh,			    206,			ES_iOCPP_MSG_CP,		Prepare_C206_SharpPeakFlatValleyKwh);
	INIT_ESiOCPP_MSG(p,	S_401_RmtRest,					401,			ES_iOCPP_MSG_CS,		Handle_S401_Reset);
	INIT_ESiOCPP_MSG(p,	S_402_RmtChgEVESEAbi,			402,			ES_iOCPP_MSG_CS,		Handle_S402_ChangeEVSEAvailability);
 	INIT_ESiOCPP_MSG(p,	S_404_RmtUpdtFirmw,				404,			ES_iOCPP_MSG_CS,		Handle_S404_UpdateFirmware);
	INIT_ESiOCPP_MSG(p,	C_405_StatOfFirmw,				405,			ES_iOCPP_MSG_CP,		Prepare_C405_FirmwareStatusNotification);
	INIT_ESiOCPP_MSG(p,	S_406_RmtCallDiag,				406,			ES_iOCPP_MSG_CS,		Handle_S406_GetDiagnostics);
	INIT_ESiOCPP_MSG(p,	C_407_StatOfDiag,				407,			ES_iOCPP_MSG_CP,		Prepare_C407_DiagnosticsStatusNotification);
	INIT_ESiOCPP_MSG(p,	S_408_ReadHistoryData,			408,			ES_iOCPP_MSG_CS,		Handle_S408_ReadHistoryData);
	INIT_ESiOCPP_MSG(p,	S_411_RmtGetConfig,				411,			ES_iOCPP_MSG_CS,		Handle_S411_GetConfiguration);
	INIT_ESiOCPP_MSG(p,	S_412_RmtSetConfig,				412,			ES_iOCPP_MSG_CS,		Handle_S412_ChangeConfiguration);
	INIT_ESiOCPP_MSG(p,	S_413_RmtGetLocalListVersion,	413,			ES_iOCPP_MSG_CS,		Handle_S413_DevVersionInfo);
	INIT_ESiOCPP_MSG(p,	S_420_RmtSendFreqPoint,			420,			ES_iOCPP_MSG_CS,		Handle_S420_SendFreqPoint);
	INIT_ESiOCPP_MSG(p,	S_421_RmtSendVpnControl,		421,			ES_iOCPP_MSG_CS,		Handle_S421_SendVpnControl);
	
	INIT_ESiOCPP_MSG(p,	S_503_RmtGetEMSPlan,			503,			ES_iOCPP_MSG_CS,		Handle_S503_GetEMSPlan);
	INIT_ESiOCPP_MSG(p,	S_504_RmtSetEMSPlanProfile,		504,			ES_iOCPP_MSG_CS,		Handle_S504_SetEMSPlanProfile);
	
	INIT_ESiOCPP_MSG(p,	S_601_RmtGetFeeRateCfg,			601,			ES_iOCPP_MSG_CS,		Handle_S601_GetFeeRate);
	INIT_ESiOCPP_MSG(p,	S_602_RmtSetFeeRateProfile,		602,			ES_iOCPP_MSG_CS,		Handle_S602_SetFeeRateProfile);
	
	
	pOCPP->tmLastSent_202[0] = _NOW_;
	pOCPP->tmLastSent_204[0] = _NOW_;
	pOCPP->tmLastSent_206[0] = _NOW_;

	return;
 }

static BOOL Init_GetSysAlarmDataInfo(ES_iOCPP_DATA_STU* pOCPP)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI err;
	BOOL bRet = TRUE;
	int n = 0;
	//now get alarm info
 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_TOTAL_R, 0, 0, &pOCPP->nAlarmInfosTotal, 4);
	TRACE("Info:nTotalCfgvAlmInfo = %d \n", pOCPP->nAlarmInfosTotal);
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get Cfg Alarm Total, err[%d]!", err);
		return FALSE; //do not reply
	}
	if((pOCPP->nAlarmInfosTotal < 0) || (pOCPP->nAlarmInfosTotal >65535))
	{
		TRACE("Error: Get AlarmInfoTotla=[%d] Out Of Range!", pOCPP->nAlarmInfosTotal);
		return FALSE;
	}

	TRACE("Info:---Now get alarm total->%d---\n", pOCPP->nAlarmInfosTotal);
	pOCPP->ShieldAlarms = NEW(UINT,pOCPP->nAlarmInfosTotal);
	if( pOCPP->ShieldAlarms == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	ZERO_POBJS(pOCPP->ShieldAlarms, pOCPP->nAlarmInfosTotal);
    pOCPP->pAllAlarmInfo = NEW(ALARM_INFO, pOCPP->nAlarmInfosTotal);
    if( pOCPP->pAllAlarmInfo == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	
	ALARM_INFO alarmInfo;
	for(n = 0; n < pOCPP->nAlarmInfosTotal; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_INFO_R,
			n, 0, &alarmInfo, sizeof(ALARM_INFO));
		if(err != ERRCODE_DAI_OK)
		{
			TRACE("Error:Get Alarm info, Idx[%d], err[%d]!\n", n, err);
			return FALSE; 
		} 
		memcpy(&pOCPP->pAllAlarmInfo[n], &alarmInfo, sizeof(ALARM_INFO));	

		//TRACE("Info:---Now get the alarm[%d, %s, %d]---\n",\
		//	pOCPP->pAllAlarmInfo[n].nAlarmId,\
		//	pOCPP->pAllAlarmInfo[n].pRes_Name->szName,\
		//	pOCPP->pAllAlarmInfo[n].emLevel);
	}
	
	return bRet;
}

//根据设备类型获取每族电池的液冷PACK数量
int getLiquidCooledPackNum(ESS_DEVICE_TYPE devType)
{
	if (LIQUID_COOLING == devType || LIQUID_COOLING_ADD_STS == devType)
	{
		return SYSTYPE_ESS_LIQUIDED_PACK_NUM;
	}
	else if (LIQUID_COOLING_BACKUP == devType)
	{
		return LIQUID_COOLING_BACKUP_PACK_NUM;
	}
	else
	{
		return 0;
	}
	
}

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("Initialize OCPP Start !! \n");
	
	REPORTER* pReporter = (REPORTER*)pReporterThis;

	//Create data
	pReporter->pDataHeap = NEW(ES_iOCPP_DATA_STU, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for OCPP dataStructs!\n");
		return FALSE;
	}

	ES_iOCPP_DATA_STU* pOCPP = (ES_iOCPP_DATA_STU*)pReporter->pDataHeap;
	ZERO_POBJS(pOCPP, 1);
	pOCPP->hReporter = pReporterThis;
	InitOCPPMsgInfo(pOCPP);

	pOCPP->hMutex = Mutex_Create(TRUE);
	pOCPP->hqMsg2Sent = Queue_Create(ES_MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(ES_iOCPP_CALL_MSG), 0);
	pOCPP->hqHistoryTrans = Queue_Create(ES_MAX_TRANS_RELATED_MSG_QUEUE_SIZE, sizeof(ES_iOCPP_CALL_MSG), 0);
	if((pOCPP->hMutex == NULL) || (pOCPP->hqMsg2Sent == NULL) || (pOCPP->hqHistoryTrans == NULL))
	{
		TRACEX("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	int n = 0;
	for(n = 0; n < Max_Connctors; n++)
	{
		pOCPP->stConnectorData[n].nGunChargeStatus = GUN_STATE_NOT_INSTALL; //20190302: all connector's initial state should be not_install
		pOCPP->stConnectorData[n].hqInProcessTrans = Queue_Create(ES_MAX_TRANS_RELATED_MSG_QUEUE_SIZE, sizeof(ES_iOCPP_CALL_MSG), 0);
		if(pOCPP->stConnectorData[n].hqInProcessTrans == NULL)
		{
			TRACEX("ERROR : Fail to Create RT Trans Queue \n");
			return FALSE;
		}
	}
	
	//20190223:Now initial the TSL root CA
	char	szRootCAFile[255] = {0};
	MakeFullFileName(szRootCAFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", "cacert.pem");
	pReporter->pUsedCommPort->Control(NULL, COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE, (void*)szRootCAFile, 0);

	pOCPP->nMsgCallTimeOut_Sec = 10; //default timeout seconds
	pOCPP->emFirmwareUpgradeStatus = FIRMUPDATE_Idle;
	pOCPP->bNeedRefreshActvAlm = TRUE;

	//server URL , format[ end url + "/ChargerID"]	
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 47, &valGet, sizeof(PARAM_VAL_GET));
	BOOL bSTDEnable = (valGet.val.emValue == 0) ? FALSE : TRUE;
	valGet.nGetSize = 255;
	char szEndUrl[255];
	valGet.val.blobValue = (BYTE*)szEndUrl; //&pOCPP->szServerURL;
	// fixed by chenjijun 20251014 for support Separate STD reporter
	ERRCODE_DAI err;
	if (bSTDEnable)
	{ 
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 48, &valGet, sizeof(PARAM_VAL_GET));
	}
	else
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 25, &valGet, sizeof(PARAM_VAL_GET));
	}
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: OCPP server URL can not be retrieved\n");
		return FALSE;
	}

	char szChargerID[64];
	valGet.nGetSize = 64;
	valGet.val.blobValue = (BYTE*)szChargerID;
	// fixed by chenjijun 20251014 for support Separate STD reporter
	if (bSTDEnable)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 49, &valGet, sizeof(PARAM_VAL_GET));
	}
	else
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 2, &valGet, sizeof(PARAM_VAL_GET));
	}
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: OCPP chargerID can NOT be retrieved\n");
		return FALSE;
	}

	snprintf(pOCPP->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);

	//-------------Get Backend Config-------------// 
	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pOCPP->blobBackendSet;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}

	//--------------Get Config BMS Num----------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			500, 502, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->nCfgBMSNum = valGet.val.ulValue;
	if(pOCPP->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pOCPP->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}

	//--------------Get Log Interval Num----------------//
	valGet.nGetSize = 4;// edit by kuangyue 20250516
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 46, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->nLog_Interval = valGet.val.ulValue;

	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: nLog_Interval settings Get Failed!\n");
		return FALSE;
	}
	//----- Get Zlib Compress Config -----//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 11, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->bZlibCompress = (valGet.val.emValue == 0) ? FALSE : TRUE;

	//printf("bZlibCompress=[%d]\n", pOCPP->bZlibCompress);

	if(!Init_GetSysAlarmDataInfo(pOCPP))
	{
		TRACEX("ERROR: Get Sys Alarm DataInfo Failed!\n");
		return FALSE;
	}
	//--------------Get Shield Alarms id----------------//
	valGet.nGetSize = 512;
	valGet.val.blobValue = (BYTE*)pOCPP->szShieldAlarms;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 83, &valGet, sizeof(PARAM_VAL_GET));

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		951, 448, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->emAirType = valGet.val.emValue;
	
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 85, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->bStandbyEnable = valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->devType = (ESS_DEVICE_TYPE)valGet.val.emValue;
	pOCPP->iLiquidPackNum = getLiquidCooledPackNum(pOCPP->devType);
	

	pOCPP->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd

	memset(&pOCPP->stCloudUpload, 0, sizeof(CLOUD_UPLOAD));

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 55, &valGet, sizeof(PARAM_VAL_GET));
	pOCPP->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_IOCPP)? TRUE : FALSE; 
	//--------------------------------End-------------------------//
	
	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("ES-iOCPPComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_OCPPComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACEX("ERROR : Create OCPPComm thread Failed \n");
		return FALSE;
	}
	
	//Init ev handler
	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
			//{EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING,	EVENT_MSG_BEGIN},
	};

	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("OCPP_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return TRUE;
}
 
void Send_Call(ES_iOCPP_DATA_STU* pOCPP, cJSON* pJson, ES_iOCPP_MSG_TYPE msgType, 
			MSGCALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	ES_iOCPP_CALL_MSG msg;
	ZERO_POBJS(&msg, 1);
	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("MsgCall[%d], msgSrc = %d, Json_Print = %s\n",pOCPP->stMsgInfoList[msgType].nMsgId, emMsgSrc,
	//	szData);

	cJSON_Delete(pJson);

	//make the msg
	msg.emMsgType = msgType;
	msg.emSrc = emMsgSrc;
	msg.nBelongedConn = nConnNum;
	//memcpy(msg.szUniqMsgID, pOCPP->stSuspendedMsgList[connectorNo].nUniqMsgId, sizeof(pOCPP->stSuspendedMsgList[connectorNo].nUniqMsgId));
	//sprintf(msg.nUniqMsgId, "%lu", (unsigned int)(GetCurrentTime() * 1000));
	memset(msg.szJsonData, 0, ES_iOCPP_MAX_SENTMSG_SIZE);
	//memcpy(msg.szJsonData, szData, strlen(szData));
	snprintf(msg.szJsonData, ES_iOCPP_MAX_SENTMSG_SIZE, "%s", szData);
	//msg.nSentLength = strlen(szData);
	free(szData);

	HANDLE hq = pOCPP->hqMsg2Sent;
	if(emMsgSrc == CALL_SRC_HisTrans)
	{
		hq = pOCPP->hqHistoryTrans;
		//Jimmy 20190305: metervalues should be the same as charge recode on Start time
		//msg.tmRelatedTransStart = pOCPP->recInHandling_HistoryChargeRec.tmChargeStart;
	}
	else if(emMsgSrc == CALL_SRC_InProcessTrans)
	{
		if((nConnNum < Connctor_1) || (nConnNum >= Max_Connctors))
		{
			return;
		}
		hq = pOCPP->stConnectorData[nConnNum].hqInProcessTrans;
		//msg.tmRelatedTransStart = pOCPP->stConnectorData[nConnNum].blobInHandling_ChargeInfo.tmChargeStart;
		// if(msgType == MeterValues)
		// {
		// 	msg.tmSampled = pOCPP->currHandlingHisMeterVal.tmSampled;
		// }
		//if(msgType == StartTransaction)
		//{
		//	TRACEX("---msgType = %d, tmRelatedTransStart = %d---\n", msgType, msg.tmRelatedTransStart);
		//}
	}

	if(pOCPP->emOCPPConnStatus != Ready)
	{
		if((msgType == C_100_BootNotif)) //can allow only these 3 types while pending
		{
			if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
				MakeESiOCPP_CallFrame(pOCPP, &msg, FALSE), /*pOCPP->nSendBuffLen*/pOCPP->nCompressSendBuffLen) <= 0)
			{
				pOCPP->nConnErrTimes++;
				if (pOCPP->nConnErrTimes >= 3)
				{
					pOCPP->emOCPP_Disconn_Reason = ES_iOCPP_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pOCPP->nConnErrTimes = 0;
					pOCPP->emOCPPConnStatus = Disconnected;
					TRACEX("emOCPPConnStatus : Disconnected \n");
					LogOut(MAIN_MODULE_REPORTER, "ES_iOCPP", LOG_TYPE_ERROR, "Send_Call Write fail more than 3 times, disconnected.");
				}
				//Queue_Put(hq, &msg, bUrgent);
				TRACEX("Write Boot-Related-Msg [%s] failed\n", pOCPP->szSendBuffer);
			}
			else
			{
				// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);//edit by kuangyue 20250315
				LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
				//pOCPP->nConnErrTimes = 0;
				TRACEX("Write Boot-Related-Msg [%s] successfully\n", pOCPP->szSendBuffer);
			}
		}
		else if(emMsgSrc == CALL_SRC_InProcessTrans) //Jimmy 20190306: if disconnect and charging is going on, should keep the messages in the queue
		{
			//Jimmy added 20190319: shall keep sure there is only ONE session be stored in the trans queue!
			// if(msgType == StartTransaction)
			// {
			// 	Queue_Empty(hq);
			// }
			Queue_Put(hq, &msg, bUrgent);
			TRACEX("Queue_Put Inprocess Call Successfully into WebSocket %s \n", msg.szJsonData);
		}
		else //need handle it later
		{
			Queue_Put(hq, &msg, bUrgent);
		}
	}
	else
	{
		if(bUrgent && (hq == pOCPP->hqMsg2Sent))
		{
			pOCPP->nQUrgentMsgCount++;
		}
		//Jimmy added 20190319: if there are 2 sessions stored while off line, and connection is restored,
		//we should discard the old session(history list will handle it) in case queue is too old and new session is missing unexpectedly
		// if((msgType == StartTransaction) && (emMsgSrc == CALL_SRC_InProcessTrans))
		// {
		// 	if(Queue_GetCount(hq, NULL) > 10) //give 10 is to leave some extra time for old session handling
		// 	{
		// 		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_WARING,
		// 			"Offline charge session for gun[%d] is removed from transaction queue!", nConnNum + 1);
		// 		Queue_Empty(hq);
		// 	}
		// }
		Queue_Put(hq, &msg, bUrgent);
		TRACEX("Queue_Put NORMAL Call Successfully into WebSocket %s \n", msg.szJsonData);
	}
}

void Send_CallResult(ES_iOCPP_DATA_STU* pOCPP, cJSON* pJson, ES_iOCPP_MSG_TYPE msgType)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

	snprintf(pOCPP->szSendBuffer, ES_iOCPP_MAX_SENTMSG_SIZE, "{\"id\":\"R%d\",	\"uniqKey\":\"%s\",	\"content\":%s}",
		pOCPP->stMsgInfoList[msgType].nMsgId,
		pOCPP->szHandlingUniqID,
		szData);

	free(szData);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	if (pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pOCPP->szSendBuffer, pOCPP->nSendBuffLen))
	{
		DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);//edit by kuangyue 20250315
		LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
		TRACEX("Write CallResult Successfully into WebSocket\n");
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
			"MsgCallResult [%d] is NOT written...", pOCPP->stMsgInfoList[msgType].nMsgId);
		TRACEX("Write CallResult ERROR by WebSocket\n");
	}
}


void Send_CallError(ES_iOCPP_DATA_STU* pOCPP, ES_iOCPP_MSG_TYPE msgType, 
		ES_iOCPP_CALLERROR_CODE emCode, const char* szErrDescrip)
{
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	snprintf(pOCPP->szSendBuffer, ES_iOCPP_MAX_SENTMSG_SIZE, "{\"id\":\"E%d\",	\"uniqKey\":\"%s\",	\"content\":{\"code\":%d, \"err_desp\":\"%s\"}}",
		pOCPP->stMsgInfoList[msgType].nMsgId,
		pOCPP->szHandlingUniqID,
		emCode, szErrDescrip);

	pOCPP->nSendBuffLen = strlen(pOCPP->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pOCPP->szSendBuffer, pOCPP->nSendBuffLen);

	if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pOCPP->szSendBuffer, pOCPP->nSendBuffLen))
	{
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);//edit by kuangyue 20250315
		LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pOCPP->szSendBuffer);
		TRACEX("Write CallError Successfully into WebSocket\n");
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_WARING,
			"MsgCallError for [%d], ErrDescription = [%s], is sent out...", 
			pOCPP->stMsgInfoList[msgType].nMsgId, szErrDescrip);
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "OCPP", LOG_TYPE_ERROR,
			"MsgCallError for [%d], ErrDescription = [%s], is NOT written...", 
			pOCPP->stMsgInfoList[msgType].nMsgId, szErrDescrip);
		TRACE("Write CallResult ERROR by WebSocket\n");
	}
}
 


