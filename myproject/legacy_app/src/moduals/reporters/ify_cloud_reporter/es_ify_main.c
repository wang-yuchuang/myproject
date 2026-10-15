// #define _TRACE_PRINT_
#include "../reporters_base.h"
#include "es_ify.h"
#include <signal.h>

// edit by kuangyue 20250318
static int queueSize = 0;
static time_t startTime = 0;
static BOOL recordedMessages[Max_ES_IFY_Msgs] = {FALSE};
static MessageEntry messageQueue[Max_ES_IFY_Msgs];
static BOOL allMessagesRecorded(void) 
{
    int i;
    for (i = 0; i < Max_ES_IFY_Msgs; i++) 
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
        ES_IFY_MSG_TYPE currentType = messageQueue[i].type;
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n;
	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response
	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			TRACE("Info:IFY Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEE("EVENT_TYPE_ALARM  alarm_id: [%d]",pAlmEv->nAlarmId);
			pIFY->bNeedRefreshActvAlm = TRUE;
		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("IFY_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		}
		break;
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("IFY_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
		}
		break;
		case EVENT_TYPE_ABILITY_CHANGE:
		{	 
		}
		break;

		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEX("IFY_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;

		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("IFY_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			//Jimmy note: this need to be implemented whenever parameter changes 
			if((pParamChg->parmPTR.nId == 1) && ((pParamChg->parmPTR.nSubId == 15) || 
				(pParamChg->parmPTR.nSubId == 14))) //server URL change or ChargerID changed
			{
				valGet.nGetSize = 255;
				char szEndUrl[255];
				valGet.val.blobValue = (BYTE*)szEndUrl; //&pIFY->szServerURL;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 15, &valGet, sizeof(PARAM_VAL_GET));

				char szChargerID[64];
				valGet.nGetSize = 64;
				valGet.val.blobValue = (BYTE*)szChargerID;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 14, &valGet, sizeof(PARAM_VAL_GET));

				snprintf(pIFY->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);
				pIFY->bBootNotifyDone = FALSE;
				pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_URL_CHANGED;
				pIFY->emIFYConnStatus = Disconnected; //need to reconnect to new server again
				//Percent_EncodeUrl(pIFY->szServerURL); //Jimmy note 20181129: maybe do not need as URL should be correct when input on LCD
			}
			else if ((pParamChg->parmPTR.nSubId == 26))
			{
				// Read ESIFY Config 
				TRACEX("Config Changed Evevt \n");
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pIFY->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY settings can not be retrieved\n");
					return FALSE;
				}
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pIFY->nCfgBMSNum = valGet.val.ulValue;
				if(pIFY->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pIFY->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				TRACEX("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pIFY->nCfgBMSNum);
			}
			else if ((pParamChg->parmPTR.nSubId == 83))
			{
				// Read Config 
				valGet.nGetSize = 512;
				valGet.val.blobValue = (BYTE*)pIFY->szShieldAlarms;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 83, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY settings can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt [%s]\n",pIFY->szShieldAlarms);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 46))
			{
				valGet.nGetSize = 4;// edit by kuangyue 20250516
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 46, &valGet, sizeof(PARAM_VAL_GET));
				pIFY->nLog_Interval = valGet.val.ulValue;

				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY nLog_Interval can not be retrieved\n");
					return FALSE;
				}
				TRACEX("Config Changed Evevt pIFY->nLog_Interval[%d]\n",pIFY->nLog_Interval);
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
					pIFY->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_IFY)? TRUE : FALSE;
				}	
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 85)){
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 85, &valGet, sizeof(PARAM_VAL_GET));
				pIFY->bStandbyEnable = valGet.val.emValue;
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY bStandbyEnable can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt pIFY->bStandbyEnable[%d]\n",pIFY->bStandbyEnable);
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pReporter->pDataHeap;
	if(pReporter->hCommPort != NULL)
	{
		nErrCode = pReporter->pUsedCommPort->Close(pReporter->hCommPort);
		TRACEX("Close The Comm Port errcode:%d\n", nErrCode);
		sprintf(szDump, "IFY Close the comm port[%d], reason[%d]", nErrCode, pIFY->emIFY_Disconn_Reason);
		// DUMP_ESIFYMSG(pReporter, DUMP_TYPE_CLOSE_COMM, szDump);//edit by kuangyue 20250315
		// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_CLOSE_COMM, szDump);
		pReporter->hCommPort = NULL;
		s_nLastOpenErrCode = -999;
	}


	DWORD dwPortAttr = COMM_OCPP1_6_MODE; 
 
	//pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
	//	"ws://121.40.165.18:8800", COMM_CLIENT_MODE, 5000, &nErrCode); //5 sec timeout for connecting...
	pReporter->hCommPort = pReporter->pUsedCommPort->Open(pReporter->info.szReporterSetting,
		pIFY->szServerURL, dwPortAttr, 10000, &nErrCode); //10 sec timeout for connecting...
	TRACE("pReporter->info.szReporterSetting =%s pIFY->szServerURL=%s\n",pReporter->info.szReporterSetting,pIFY->szServerURL);
	
	static char s_strLastServerURL[255] = {0};
	if(s_nLastOpenErrCode != nErrCode) //Jimmy added 20190411 for only record once if the network is not alive
	{
		s_nLastOpenErrCode = nErrCode;
		memcpy(s_strLastServerURL, pIFY->szServerURL, 255);
		sprintf(szDump, "IFY Open the address[%s]: [%s]", pIFY->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		// DUMP_ESIFYMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250315
		// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
	else if(strcmp(s_strLastServerURL, pIFY->szServerURL) != 0) //Jimmy added 20190411 for only record once if the network is not alive
	{
		memcpy(s_strLastServerURL, pIFY->szServerURL, 255);
		sprintf(szDump, "IFY Open the NEW address[%s]: [%s]", pIFY->szServerURL, (pReporter->hCommPort == NULL) ? "Failed" : "Successfylly");
		// DUMP_ESIFYMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250315
		// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
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

static UINT Make_UniqID_ESIFY(void)
{
	#define ES_IFY_UNIQID_START	25689566
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + ES_IFY_UNIQID_START; //just to mix the value
	}
	//static UINT s_nOldUniqID = IFY_UNIQID_START;
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = ES_IFY_UNIQID_START;
	}

	TRACE("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}


static char* MakeESIFY_CallFrame(ES_IFY_DATA_STU* pIFY, ES_IFY_CALL_MSG *pMsg, BOOL bResent)
{
	pIFY->bHasActiveMsg = TRUE;
	pIFY->activeCallMsg.tmLastSent = _NOW_;
	if(!bResent)
	{
		memcpy(&pIFY->activeCallMsg.msgData, pMsg, sizeof(ES_IFY_CALL_MSG));
		pIFY->activeCallMsg.nRetryTimes = 1;
	}
	else
	{
		pIFY->activeCallMsg.nRetryTimes++;
	}
	
	UINT nMsgUniqID = Make_UniqID_ESIFY(); //(UINT)(GetCurrentTime() * 1000);

	if(pIFY->activeCallMsg.nRetryTimes > MAX_ES_IFY_SEND_RETRIES)
	{
		pIFY->activeCallMsg.nRetryTimes = MAX_ES_IFY_SEND_RETRIES;
		pIFY->activeCallMsg.nUniqMsgId[MAX_ES_IFY_SEND_RETRIES - 1] = nMsgUniqID;
	}
	else
	{
		int nWhich = (pIFY->activeCallMsg.nRetryTimes <= 0) ? 0 : pIFY->activeCallMsg.nRetryTimes - 1;
		pIFY->activeCallMsg.nUniqMsgId[nWhich] = nMsgUniqID;
	}

	snprintf(pIFY->szSendBuffer, ES_IFY_MAX_SENTMSG_SIZE, "{\"id\":\"Q%d\",	\"uniqKey\":\"%d\",	\"content\":%s}",
		pIFY->stMsgInfoList[pMsg->emMsgType].nMsgId, nMsgUniqID,
		pIFY->activeCallMsg.msgData.szJsonData);

	pIFY->nSendBuffLen = strlen(pIFY->szSendBuffer);

	TRACEX("Uncompress:pkg is [%s], nSendBuffLen = %d\n", pIFY->szSendBuffer, pIFY->nSendBuffLen);

	return pIFY->szSendBuffer;
}

static BOOL Wait_Ready(REPORTER* pReporter)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pReporter->pDataHeap;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	time_t tmNow = _NOW_;
	static time_t s_tmLastSentBootNotif = 0;
 
	static ES_IFY_COMM_STATUS s_emOldConnStatus = Connected;
	static int s_nConnStatusStableCount = 0;
	if((pIFY->emIFYConnStatus != s_emOldConnStatus) &&
		((pIFY->emIFYConnStatus == Disconnected) || (pIFY->emIFYConnStatus == Ready)))
	{
		s_nConnStatusStableCount++;
		if(s_nConnStatusStableCount >= 3)
		{
			s_nConnStatusStableCount = 0;
			s_emOldConnStatus = pIFY->emIFYConnStatus;
			VAR_VALUE varV;
			varV.emValue = (s_emOldConnStatus == Disconnected) ? 1 : 0;
			TRACED("IFY-server_commom_status [%d]",varV.emValue);
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 12, &varV, 4, 100);//Ify Server Comm Status
		}
	}
	else
	{
		s_nConnStatusStableCount = 0;
	}

	if(pIFY->emIFYConnStatus == Ready)
	{
		pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_NO_FAIL;
		return TRUE;
	}

	int nWaitResent_Boot_Sec = 3; //if it is 0, charger uses 3 as default
	nWaitResent_Boot_Sec = MIN(nWaitResent_Boot_Sec, 10); //Jimmy note 20190215:if heartbeatInterv is very large, could be very slow to try again

	Mutex_Lock(pIFY->hMutex, WAIT_INFINITE);

	TRACEX("Now IFY status = %d, tmDiff = %d, nWaitResent_Boot_Sec = %d \n", 
		pIFY->emIFYConnStatus, ABS(tmNow - s_tmLastSentBootNotif) , nWaitResent_Boot_Sec);

	if(pIFY->emIFYConnStatus == Disconnected)
	{
		Mutex_Unlock(pIFY->hMutex); //Jimmy note 20190412: do not lock here in case the event_handler will be blocked
		TRACEX("ThreadEntry_IFYComm Enter\n");
		//pIFY->emIFYConnStatus = Disconnected;
		TryConnect2Server(pReporter);
		TRACEX("TryConnect2Server Out \n");
		if (pReporter->hCommPort == NULL)
		{
			pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_SERVRER_BROKEN;
			//Mutex_Unlock(pIFY->hMutex);
			Sleep(6000);
			//return FALSE;
		}
		else
		{
			pIFY->emIFYConnStatus = Connected;
		}

		return FALSE;
	}
	else if(pIFY->emIFYConnStatus == Connected)
	{
		pIFY->bHasActiveMsg = FALSE;
		pIFY->bAuthorizeInProcess = FALSE; //added to clear the Auth Status in case auth not resending
		if (!pIFY->bBootNotifyDone)
		{
			pIFY->stMsgInfoList[C_100_BootNotif].pfnMsgHandler((void*)pIFY);
			TRACEX("BootNotification Trigger pIFY->bBootNotifyDone=%d\n",pIFY->bBootNotifyDone);
			//if write error, then we need to goto disconnected again
			if(pIFY->emIFYConnStatus != Disconnected)
			{
				pIFY->emIFYConnStatus = Wait4BootNotify_Conf;
				s_tmLastSentBootNotif = tmNow;
			}
		}
		else
		{
			//Jimmy note 20190111: do not empty 'cause some msg need to be restored after connection recovered
			Queue_Empty(pIFY->hqMsg2Sent); //clear any history queued msg
			Queue_Empty(pIFY->hqHistoryTrans);
			//Need trigger StatusNotif.req soon after reconnect
			if(!pIFY->bCPStatusChanged)
			{
				pIFY->tmLastCPStatusChange = 0;
			}
	 
			pIFY->tmLastSendHeartBeat = tmNow; //do not send heartbeat on first connected
			pIFY->emIFYConnStatus = Ready;
			pIFY->bNeedRefreshActvAlm = TRUE;

			TRACEX("Now Ready for comm, so empty Q first \n");
		}
	}
	else if(pIFY->emIFYConnStatus == Wait4BootNotify_Conf) //waiting BootNotic result
	{
		pIFY->nReadBuffLen = pComm->Read(pReporter->hCommPort, pIFY->szReadBuffer, ES_IFY_BUFFER_LEN);
		TRACEX("Wait bootNotif.conf nReadLen = %d\n", pIFY->nReadBuffLen);
		if(pIFY->nReadBuffLen > 0)
		{
			//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_READ, pIFY->szReadBuffer);
			// LOG_INFO("ES_IFY pReporter=%p, type=%d, Data='%s'",
			// 	pReporter,
			// 	DUMP_TYPE_READ,
			// 	pIFY->szReadBuffer);//edit by kuangyue 20250313
			Parse_ESIFYFrames(pIFY);
		}

		if(pIFY->bBootNotifyDone)
		{
			pIFY->emIFYConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pIFY->emIFYConnStatus = Connected;
			//pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
			//pIFY->emIFYConnStatus = Disconnected;
		}
	}
	else if(pIFY->emIFYConnStatus == BootNotify_Rejected)
	{
		static time_t s_tmLastRejected = 0;
		if(s_tmLastRejected == 0)
		{
			s_tmLastRejected = tmNow;
		}

		if(pIFY->bBootNotifyDone)
		{
			pIFY->emIFYConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastSentBootNotif) > nWaitResent_Boot_Sec)
		{
			pIFY->emIFYConnStatus = Connected;
		}
		else if(ABS(tmNow - s_tmLastRejected) > 60)
		{
			LogOut(MAIN_MODULE_REPORTER, "ES_IFY", LOG_TYPE_ERROR,
				"BootNotif was refused by server, now disconnect and try again...");
			s_tmLastRejected = tmNow;
			pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_REFUSED;
			pIFY->emIFYConnStatus = Disconnected;
		}
	}

	Mutex_Unlock(pIFY->hMutex);

	//TRACEX("pIFY->emIFYConnStatus = %d \n", pIFY->emIFYConnStatus);

	return (pIFY->emIFYConnStatus == Ready) ? TRUE : FALSE;
}

static void Handle_Call_Resend(ES_IFY_DATA_STU* pIFY, COMMPORT* pComm)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	if(pIFY->activeCallMsg.nRetryTimes >= MAX_ES_IFY_SEND_RETRIES)
	{
		TRACEX("----Resend Msg[%d] TIMEOUT!!!-----\n", pIFY->stMsgInfoList[pIFY->activeCallMsg.msgData.emMsgType].nMsgId);
		pIFY->bHasActiveMsg = FALSE;
		//pIFY->nConnErrTimes++;
		pIFY->nConnErrTimes = 5; //Jimmy note: found that even disconnected, write can be still done, so here once resend expires 3 times, we disconnect.
	}
	else
	{
		TRACEX("----Resend Msg[%d] RetryTimes [%d], jSon [%s]-----\n", pIFY->stMsgInfoList[pIFY->activeCallMsg.msgData.emMsgType].nMsgId, 
			pIFY->activeCallMsg.nRetryTimes, pIFY->activeCallMsg.msgData.szJsonData);

		ES_IFY_CALL_MSG* pMsg = &pIFY->activeCallMsg.msgData;
		if (pComm->Write(pReporter->hCommPort,
			MakeESIFY_CallFrame(pIFY, pMsg, TRUE),
			pIFY->nSendBuffLen))
		{
			TRACE("Info: pIFY->nConnErrTimes=%d\n", pIFY->nConnErrTimes);
		}
		else
		{
			pIFY->nConnErrTimes++;
		}
	}

	if (pIFY->nConnErrTimes >= 5)
	{
		pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
		pIFY->nConnErrTimes = 0;
		pIFY->emIFYConnStatus = Disconnected;
		TRACEX("emIFYConnStatus : Disconnected \n");
	}
}

static HANDLE Dispatch_Q(ES_IFY_DATA_STU* pIFY)
{
	if(pIFY->nQUrgentMsgCount > 0)
	{
		pIFY->nQUrgentMsgCount--;
		TRACEX("-------Now has urgent msg to send out[%d]-------\n", pIFY->nQUrgentMsgCount);
		return pIFY->hqMsg2Sent;
	}

	return pIFY->hqMsg2Sent;
}

static void Handle_Queued_Call(ES_IFY_DATA_STU* pIFY, COMMPORT* pComm)
{
	ES_IFY_CALL_MSG stSentMsg;
	ES_IFY_CALL_MSG* pMsg = &stSentMsg;
	//static HANDLE s_LastHanlded_InProcQ = NULL;
	//Strategy: get queue from hqSent/hqHis/hqInProc in-turn
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	time_t tmNow = _NOW_;//edit by kuangyue 20250318

	TRACE("Info: Handle_Queued_Call Dispatch_Q\n");
	HANDLE hq = Dispatch_Q(pIFY);
	if(hq == NULL)
	{
		return;
	}

	if(Mutex_Lock(pIFY->hMutex, 100) == ERR_MUTEX_OK)
	{
		if(Queue_Get(hq, &stSentMsg, FALSE, 100) == ERR_QUEUE_OK)
		{
			if (pComm->Write(pReporter->hCommPort,
					MakeESIFY_CallFrame(pIFY, pMsg, FALSE),
					pIFY->nSendBuffLen))
			{
				ES_IFY_MSG_TYPE currentType = pIFY->activeCallMsg.msgData.emMsgType;
				TRACED("----Kuangyue10 currentType = %d----\n", currentType);
				if (recordedMessages[currentType]) 
				{
					TRACED("Message type %d has been recorded, skipping addition to queue\n", currentType);
				}
				else if (queueSize < Max_ES_IFY_Msgs) 
				{
					messageQueue[queueSize].type = currentType;
					strncpy(messageQueue[queueSize].szSendBuffer, pIFY->szSendBuffer, sizeof(messageQueue[queueSize].szSendBuffer));
					messageQueue[queueSize].timestamp = tmNow;
					queueSize++;
				}

				if (pIFY->nLog_Interval > 0)
				{
					manageMessageQueue(pIFY->nLog_Interval);
				}
				else if(pIFY->nLog_Interval == 0)// edit by kuangyue 20250318
				{
					// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);//edit by kuangyue 20250315
					// LOG_INFO("pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);
					LOG_INFO("Message Type: %d, Content: %s\n", currentType, pIFY->szSendBuffer);
					//sent succsessfully
					//pIFY->nConnErrTimes = 0;
				}
				else
				{
					LogOut(MAIN_MODULE_REPORTER, "ES_IFY", LOG_TYPE_ERROR,
						"nLog_Interval was false...");
					return; //do not log if nLog_Interval is negative
				}
				//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);
				// LOG_INFO("ES_IFY pReporter=%p, type=%d, Data='%s'",
				// 	pReporter,
				// 	DUMP_TYPE_WRITE,
				// 	pIFY->szSendBuffer);//edit by kuangyue 20250313
				//sent succsessfully
				//pIFY->nConnErrTimes = 0;
			}
			else
			{
				pIFY->nConnErrTimes++;
				if (pIFY->nConnErrTimes >= 5)
				{
					pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pIFY->nConnErrTimes = 0;
					pIFY->emIFYConnStatus = Disconnected;
					TRACEX("emIFYConnStatus : Disconnected \n");
				}
			}
			
		}
		Mutex_Unlock(pIFY->hMutex);
	}
	return;
}
 
BOOL Check_IfStatusChanged(ES_IFY_DATA_STU* pIFY)
{
	PARAM_VAL_GET valGet;
	//ERRCODE_DAI err;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	BOOL bStatusHasChanged = FALSE;

	pIFY->blobCPStatusInfo.emChargePointStatus = CP_STATUS_Available; //initial state
	pIFY->blobCPStatusInfo.emChargePointErrcode = CP_ERRCODE_NoError;
	valGet.nGetSize = sizeof(A_BLOB_CAHRGEPOINT_STATUSINFO);
	valGet.val.blobValue = (BYTE*)&pIFY->blobCPStatusInfo;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 10, &valGet, sizeof(PARAM_VAL_GET));
	
	//first judge sytem status change?
	//A_BLOB_CAHRGEPOINT_STATUSINFO* pStatusInfo = &pIFY->blobCPStatusInfo;
	static int s_nOldSysStatus = -1;
	pIFY->emSysStatus_For101 = 0;
	if(pIFY->blobCPStatusInfo.emChargePointStatus == CP_STATUS_Unavailable)
	{
		pIFY->emSysStatus_For101 = 1;
	}
	else if(pIFY->blobCPStatusInfo.emChargePointStatus == CP_STATUS_Faulted)
	{
		pIFY->emSysStatus_For101 = 2;
	}

	if(s_nOldSysStatus != pIFY->emSysStatus_For101)
	{
		s_nOldSysStatus = pIFY->emSysStatus_For101;
		bStatusHasChanged = TRUE;
	}

	return bStatusHasChanged;
}
 

BOOL Check_IfActvAlmStatuChanged(ES_IFY_DATA_STU* pIFY)
{
	BOOL bRet = FALSE;

	if(pIFY->bNeedRefreshActvAlm)
	{
		pIFY->bNeedRefreshActvAlm = FALSE;
		QueryActvAlarmInfo(pIFY);
		return TRUE;
	}

	return bRet;
}

BOOL ShieldAlarms_StringToUINT(ES_IFY_DATA_STU* pIFY)
{
	TRACEW("allow to shield alarms [%s]",pIFY->szShieldAlarms);
	UINT index = 0 ,ncount = 1;
	char szInput[512];

	ZERO_POBJS(pIFY->ShieldAlarms, pIFY->nAlarmInfosTotal);   //clear buff
    if (pIFY->szShieldAlarms[0] == '\0' || strcmp(pIFY->szShieldAlarms, "alm_id/alm_id/alm_id") == 0) 
	{
		TRACEE("NO alarm to be shield ...");
		pIFY->nShieldAlarmsNum = 0;
        return FALSE;
    }

    for (index = 0; pIFY->szShieldAlarms[index] != '\0'; index++) 
	{
        if (pIFY->szShieldAlarms[index] == '/') 
		{
            ncount++;
        }
    }
	pIFY->nShieldAlarmsNum = ncount;

	strncpy(szInput, pIFY->szShieldAlarms, 512);
	szInput[511] = '\0'; 

	char *token = strtok(szInput, "/");
    index = 0;

    while (token != NULL && index < ncount) 
	{
        pIFY->ShieldAlarms[index] = (UINT)strtoul(token, NULL, 10);
        token = strtok(NULL, "/");
		index++;
    }

	// printf("pIFY->ShieldAlarms = ");
	// for (index=0;index < ncount;index++)
	// {
	// 	printf("[%d] ",pIFY->ShieldAlarms[index]);
	// }
	// printf("\n");

    return TRUE;

}
void Pro_SysCmdDataInfo(ES_IFY_DATA_STU* pIFY)
{
	time_t tmNow = _NOW_;
	PARAM_VAL_GET 	valGet;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	
	BOOL bSysStatuChanged = Check_IfStatusChanged(pIFY);

	if((ABS(tmNow - pIFY->tmLastSent_101) >= MAX(3, pIFY->blobBackendSet.nCmd_101_Interval)) || bSysStatuChanged)
	{
		pIFY->tmLastSent_101 = tmNow;
		pIFY->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pIFY); 
	}

	if(ABS(tmNow - pIFY->tmLastSent_102) >= MAX(3, pIFY->blobBackendSet.nCmd_102_Interval))
	{
		pIFY->tmLastSent_102 = tmNow;
		pIFY->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pIFY);
	}

 	if(ABS(tmNow - pIFY->tmLastSent_103) >= MAX(3, pIFY->blobBackendSet.nCmd_103_Interval))
	{
		pIFY->tmLastSent_103 = tmNow;
		pIFY->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_104) >= MAX(3, pIFY->blobBackendSet.nCmd_104_Interval))
	{
		pIFY->tmLastSent_104 = tmNow;
		pIFY->stMsgInfoList[C_104_PwrDistrStaNotif].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_105) >= MAX(3, pIFY->blobBackendSet.nCmd_105_Interval))
	{
		pIFY->tmLastSent_105 = tmNow;
		pIFY->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_106) >= MAX(3, pIFY->blobBackendSet.nCmd_106_Interval))
	{
		pIFY->tmLastSent_106 = tmNow;
		pIFY->stMsgInfoList[C_106_SingleRectStaNotif].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_107) >= MAX(3, pIFY->blobBackendSet.nCmd_107_Interval))
	{
		pIFY->tmLastSent_107 = tmNow;
		pIFY->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_108) >= MAX(3, pIFY->blobBackendSet.nCmd_108_Interval))
	{
		pIFY->tmLastSent_108 = tmNow;
		pIFY->stMsgInfoList[C_108_SysVersionStaNotif].pfnMsgHandler((void*)pIFY);
	}
	
	BOOL bActvAlmChanged = Check_IfActvAlmStatuChanged(pIFY);
	if((bActvAlmChanged == TRUE)||
		(ABS(tmNow - pIFY->tmLastSent_109) >= MAX(3, pIFY->blobBackendSet.nCmd_109_Interval)))
	{
		ShieldAlarms_StringToUINT(pIFY);
 		pIFY->stMsgInfoList[C_109_SysActvAlmNotif].pfnMsgHandler((void*)pIFY);
		pIFY->tmLastSent_109 = tmNow;
	}


	if(ABS(tmNow - pIFY->tmLastSent_110) > (MAX(300, pIFY->blobBackendSet.nCmd_110_Interval)))
	{
		int n = 0;
		for(n = 0; n < pIFY->nCfgBMSNum; n++)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				501 + n, 550, &valGet, sizeof(PARAM_VAL_GET));
			if(valGet.val.emValue <= STBATT_RUNSTAT_COMM_FAIL)
			{
				continue;
			}
			pIFY->nCurrBMS = n;
			pIFY->stMsgInfoList[C_110_SingleBattryInfo].pfnMsgHandler((void*)pIFY);
		}
		pIFY->tmLastSent_110 = tmNow;
	}
	

	int nValue = tmNow - pIFY->tmLastSent_202;
	TRACEX("tmNow:[%ld] - pIFY->tmLastSent_202[%d]:%ld = %d\n",\
		tmNow, pIFY->tmLastSent_202, nValue);
	if(ABS(tmNow - pIFY->tmLastSent_202) >= MAX(3, pIFY->blobBackendSet.nCmd_202_Interval))
	{
		pIFY->tmLastSent_202 = tmNow;
		pIFY->stMsgInfoList[C_202_HourSectorStaticKwh].pfnMsgHandler((void*)pIFY);
	}

	if(ABS(tmNow - pIFY->tmLastSent_204) >= MAX(3, pIFY->blobBackendSet.nCmd_204_Interval))
	{
		pIFY->tmLastSent_204 = tmNow;
		pIFY->stMsgInfoList[C_204_DailyStaticKwh].pfnMsgHandler((void*)pIFY);
	}

	if(pIFY->blobBackendSet.nCmd_206_Interval > 0 && ABS(tmNow - pIFY->tmLastSent_206) >= MAX(3, pIFY->blobBackendSet.nCmd_206_Interval))
	{
		pIFY->tmLastSent_206 = tmNow;
		pIFY->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pIFY);
	}

	if(pIFY->stCloudUpload.bUploadEnable && (ABS(tmNow - pIFY->stCloudUpload.tmLastUpload) >= MAX_CLOUDUPLOAD_INTERNAL))
	{
		ReadHistoryData((void*)pIFY);
	}

	return;
}

void QueryActvAlarmInfo(ES_IFY_DATA_STU* pIFY)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI errCode;
	int n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize = sizeof(EVENT_ALARM);
	cond.pCondArg = &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount = MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pIFY->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pIFY->activeAlms , &nQueryCount);
	TRACE("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);
	TRACEI("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

	if(errCode != ERRCODE_DAI_OK)
	{
		TRACE("Error: Query Actv Alarm Failed!\n");
 	}
	else
	{
		pIFY->nActiveAlmTotal = MIN(nQueryCount, MAX_QUERY_ACTV_ALM);
 		for(n = 0; n < pIFY->nActiveAlmTotal; n++)
		{
			TRACE("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pIFY->activeAlms[n].nDevId, pIFY->activeAlms[n].nAlarmId);
			TRACEI("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pIFY->activeAlms[n].nDevId, pIFY->activeAlms[n].nAlarmId);
		}
	}

	return;
}


static DWORD _ThreadEntry_IFYComm(void* pArg)
{
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	printf("_ThreadEntry_IFYComm: Start Enter \n");
	int n = 0;
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pReporter->pDataHeap;
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
		if(pIFY->bHasActiveMsg)
		{
			if(ABS(tmNow - pIFY->activeCallMsg.tmLastSent) > pIFY->nMsgCallTimeOut_Sec)
			{
				// printf("tmNow=[%ld],pIFY->activeCallMsg.tmLastSent=[%ld]\n" ,tmNow ,pIFY->activeCallMsg.tmLastSent);
				//Resend_Msg
				Handle_Call_Resend(pIFY, pComm);
			}
		}
		else
		{
			Handle_Queued_Call(pIFY, pComm);
		}
		//added by Jimmy 20190702 for OCTT test. if disconn, should skip going ahead
		if(pIFY->emIFYConnStatus == Disconnected)
		{
			printf("Info : pIFY->emIFYConnStatus =%d\n", pIFY->emIFYConnStatus);
			continue;
		}
		else if(pIFY->bNoticedExitIFY)
		{
			if(pIFY->nQUrgentMsgCount < 1)
			{
				TRACEX("-----Now ready for IFY exiting-----\n");
				pIFY->bNoticedExitIFY = FALSE;
				pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_SYSTEM_EXIT;
				pIFY->emIFYConnStatus = Disconnected; //need handle urgent msg before exit
				continue;
			}
		}

		//Now read and parse
		pIFY->emTriggerMsgType = Trigger_NoTriggle;
		pIFY->nReadBuffLen = pComm->Read(pReporter->hCommPort, pIFY->szReadBuffer, ES_IFY_BUFFER_LEN);
		if(pIFY->nReadBuffLen> 0)
		{
			//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_READ, pIFY->szReadBuffer);
			// LOG_INFO("ES_IFY pReporter=%p, type=%d, Data='%s'",
			// 	pReporter,
			// 	DUMP_TYPE_READ,
			// 	pIFY->szReadBuffer);//edit by kuangyue 20250313
			Parse_ESIFYFrames(pIFY);
		}
		else if(pIFY->nReadBuffLen == -1) //means server close session
		{
			TRACEX("Server Connection Error Close the Socket\n");
			pIFY->nReadBuffLen = 0;      // need to make Zero Otherwise same Case Excuting Continue
			pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_SERVRER_BROKEN;
			pIFY->emIFYConnStatus = Disconnected;
			Sleep(100);
			continue;
		}
		else
		{
			//TRACE("Info: nReadBuffLen=%d\n", pIFY->nReadBuffLen);
		}

		Pro_SysCmdDataInfo(pIFY); 

		Sleep(500);
	}

	TRACE("_ThreadEntry_IFYComm: Exit\n");
	return THREAD_CANCEL_ALL;
}

#define INIT_ESIFY_MSG(_pMsg,_emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)


void InitIFYMsgInfo(ES_IFY_DATA_STU* pIFY)
{
	ES_IFY_MSG_INFO* p = &(pIFY->stMsgInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_ESIFY_MSG(p,	C_100_BootNotif,				100,			ES_IFY_MSG_CP,		Prepare_C100_BootNotification);
	INIT_ESIFY_MSG(p,	C_101_StatusNotif,				101,			ES_IFY_MSG_CP,		Prepare_C101_StatusNotification);
	INIT_ESIFY_MSG(p,	C_102_BattCabStaNotif,			102,			ES_IFY_MSG_CP,		Prepare_C102_BattCabStaNotif);
	INIT_ESIFY_MSG(p,	C_103_ESBMSStaNotif,			103,			ES_IFY_MSG_CP,		Prepare_C103_ESBMSStatusNotif);
	INIT_ESIFY_MSG(p,	C_104_PwrDistrStaNotif,			104,			ES_IFY_MSG_CP,		Prepare_C104_PwrDistrStaNotif);
	INIT_ESIFY_MSG(p,	C_105_ESRunDataStaNotif,		105,			ES_IFY_MSG_CP,		Prepare_C105_ESRunDataNotif);
	INIT_ESIFY_MSG(p,	C_106_SingleRectStaNotif,		106,			ES_IFY_MSG_CP,		Prepare_C106_SingleRectStaNotif);
	INIT_ESIFY_MSG(p,	C_107_PwrDistrCabStaNotif,		107,			ES_IFY_MSG_CP,		Prepare_C107_PwrDistrCabStaNotif);
	INIT_ESIFY_MSG(p,	C_108_SysVersionStaNotif,		108,			ES_IFY_MSG_CP,		Prepare_C108_SysVersionStaNotif);
	INIT_ESIFY_MSG(p,	C_109_SysActvAlmNotif,			109,			ES_IFY_MSG_CP,		Prepare_C109_SysActvAlmNotif);
	INIT_ESIFY_MSG(p,	C_110_SingleBattryInfo,			110,			ES_IFY_MSG_CP,		Prepare_C110_SiglBatDataNotif);

	INIT_ESIFY_MSG(p,	S_111_Trigger,					111,			ES_IFY_MSG_CS,		Handle_S111_TriggerMessage);
 	INIT_ESIFY_MSG(p,	C_202_HourSectorStaticKwh,		202,			ES_IFY_MSG_CP,		Prepare_C202_HourSectorStaticKwh);
	INIT_ESIFY_MSG(p,	C_204_DailyStaticKwh,			204,			ES_IFY_MSG_CP,		Prepare_C204_DailyStaticKwh);
	INIT_ESIFY_MSG(p,	C_206_FeeRateKwh,				206,			ES_IFY_MSG_CP,		Prepare_C206_SharpPeakFlatValleyKwh);
	INIT_ESIFY_MSG(p,	S_401_RmtRest,					401,			ES_IFY_MSG_CS,		Handle_S401_Reset);
	INIT_ESIFY_MSG(p,	S_402_RmtChgEVESEAbi,			402,			ES_IFY_MSG_CS,		Handle_S402_ChangeEVSEAvailability);
 	INIT_ESIFY_MSG(p,	S_404_RmtUpdtFirmw,				404,			ES_IFY_MSG_CS,		Handle_S404_UpdateFirmware);
	INIT_ESIFY_MSG(p,	C_405_StatOfFirmw,				405,			ES_IFY_MSG_CP,		Prepare_C405_FirmwareStatusNotification);
	INIT_ESIFY_MSG(p,	S_406_RmtCallDiag,				406,			ES_IFY_MSG_CS,		Handle_S406_GetDiagnostics);
	INIT_ESIFY_MSG(p,	C_407_StatOfDiag,				407,			ES_IFY_MSG_CP,		Prepare_C407_DiagnosticsStatusNotification);
	INIT_ESIFY_MSG(p,	S_408_ReadHistoryData,			408,			ES_IFY_MSG_CS,		Handle_S408_ReadHistoryData);
	INIT_ESIFY_MSG(p,	S_411_RmtGetConfig,				411,			ES_IFY_MSG_CS,		Handle_S411_GetConfiguration);
	INIT_ESIFY_MSG(p,	S_412_RmtSetConfig,				412,			ES_IFY_MSG_CS,		Handle_S412_ChangeConfiguration);
	INIT_ESIFY_MSG(p,	S_413_RmtGetLocalListVersion,   413,			ES_IFY_MSG_CS,		Handle_S413_DevVersionInfo);
	
	INIT_ESIFY_MSG(p,	S_503_RmtGetEMSPlan,			503,			ES_IFY_MSG_CS,		Handle_S503_GetEMSPlan);
	INIT_ESIFY_MSG(p,	S_504_RmtSetEMSPlanProfile,		504,			ES_IFY_MSG_CS,		Handle_S504_SetEMSPlanProfile);
	
	INIT_ESIFY_MSG(p,	S_601_RmtGetFeeRateCfg,			601,			ES_IFY_MSG_CS,		Handle_S601_GetFeeRate);
	INIT_ESIFY_MSG(p,	S_602_RmtSetFeeRateProfile,		602,			ES_IFY_MSG_CS,		Handle_S602_SetFeeRateProfile);
	
	
	pIFY->tmLastSent_202 = _NOW_;
	pIFY->tmLastSent_204 = _NOW_;
	pIFY->tmLastSent_206 = _NOW_;

	return;
 }

static BOOL Init_GetSysAlarmDataInfo(ES_IFY_DATA_STU* pIFY)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI err;
	BOOL bRet = TRUE;
	int n = 0;
	//now get alarm info
 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_TOTAL_R, 0, 0, &pIFY->nAlarmInfosTotal, 4);
	TRACE("Info:nTotalCfgvAlmInfo = %d \n", pIFY->nAlarmInfosTotal);
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get Cfg Alarm Total, err[%d]!", err);
		return FALSE; //do not reply
	}
	if((pIFY->nAlarmInfosTotal < 0) || (pIFY->nAlarmInfosTotal >65535))
	{
		TRACE("Error: Get AlarmInfoTotla=[%d] Out Of Range!", pIFY->nAlarmInfosTotal);
		return FALSE;
	}

	TRACE("Info:---Now get alarm total->%d---\n", pIFY->nAlarmInfosTotal);
	pIFY->ShieldAlarms = NEW(UINT,pIFY->nAlarmInfosTotal);
	if( pIFY->ShieldAlarms == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	ZERO_POBJS(pIFY->ShieldAlarms, pIFY->nAlarmInfosTotal);
    pIFY->pAllAlarmInfo = NEW(ALARM_INFO, pIFY->nAlarmInfosTotal);
    if( pIFY->pAllAlarmInfo == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	
	ALARM_INFO alarmInfo;
	for(n = 0; n < pIFY->nAlarmInfosTotal; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_INFO_R,
			n, 0, &alarmInfo, sizeof(ALARM_INFO));
		if(err != ERRCODE_DAI_OK)
		{
			TRACE("Error:Get Alarm info, Idx[%d], err[%d]!\n", n, err);
			return FALSE; 
		} 
		memcpy(&pIFY->pAllAlarmInfo[n], &alarmInfo, sizeof(ALARM_INFO));	

		//TRACE("Info:---Now get the alarm[%d, %s, %d]---\n",\
		//	pIFY->pAllAlarmInfo[n].nAlarmId,\
		//	pIFY->pAllAlarmInfo[n].pRes_Name->szName,\
		//	pIFY->pAllAlarmInfo[n].emLevel);
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
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	int n = 0;

	//Create data
	pReporter->pDataHeap = NEW(ES_IFY_DATA_STU, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for IFY dataStructs!\n");
		return FALSE;
	}

	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pReporter->pDataHeap;
	ZERO_POBJS(pIFY, 1);
	pIFY->hReporter = pReporterThis;
	InitIFYMsgInfo(pIFY);

	pIFY->hMutex = Mutex_Create(TRUE);
	pIFY->hqMsg2Sent = Queue_Create(ES_MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(ES_IFY_CALL_MSG), 0);
	pIFY->hqHistoryTrans = Queue_Create(ES_MAX_TRANS_RELATED_MSG_QUEUE_SIZE, sizeof(ES_IFY_CALL_MSG), 0);
	if((pIFY->hMutex == NULL) || (pIFY->hqMsg2Sent == NULL) || (pIFY->hqHistoryTrans == NULL))
	{
		printf("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}


	//20190223:Now initial the TSL root CA
	char	szRootCAFile[255] = {0};
	MakeFullFileName(szRootCAFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", "cacert.pem");
	pReporter->pUsedCommPort->Control(NULL, COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE, (void*)szRootCAFile, 0);

	pIFY->nMsgCallTimeOut_Sec = 10; //default timeout seconds
	pIFY->emFirmwareUpgradeStatus = FIRMUPDATE_Idle;
	pIFY->bNeedRefreshActvAlm = TRUE;
	
	//server URL , format[ end url + "/ChargerID"]	
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 255;
	char szEndUrl[255];
	valGet.val.blobValue = (BYTE*)szEndUrl; //&pIFY->szServerURL;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 15, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		printf("ERROR: IFY server URL can not be retrieved\n");
		return FALSE;
	}

	char szChargerID[64];
	valGet.nGetSize = 64;
	valGet.val.blobValue = (BYTE*)szChargerID;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 14, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		printf("ERROR: IFY chargerID can NOT be retrieved\n");
		return FALSE;
	}

	snprintf(pIFY->szServerURL, 255, "%s/%s", szEndUrl, szChargerID);

	//-------------Get Backend Config-------------// 
	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pIFY->blobBackendSet;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		printf("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}

	//--------------Get Config BMS Num----------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			500, 502, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->nCfgBMSNum = valGet.val.ulValue;
	if(pIFY->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pIFY->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}
	
	//--------------Get Log Interval Num----------------//
	valGet.nGetSize = 4;// edit by kuangyue 20250516
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 46, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->nLog_Interval = valGet.val.ulValue;

	if (err != ERRCODE_DAI_OK)
	{
		printf("ERROR: nLog_Interval settings Get Failed!\n");
		return FALSE;
	}
	if(!Init_GetSysAlarmDataInfo(pIFY))
	{
		printf("ERROR: Get Sys Alarm DataInfo Failed!\n");
		return FALSE;
	}

	//--------------Get Shield Alarms id----------------//
	valGet.nGetSize = 512;
	valGet.val.blobValue = (BYTE*)pIFY->szShieldAlarms;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 83, &valGet, sizeof(PARAM_VAL_GET));
	pReporter->bExit = FALSE;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		951, 448, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->emAirType = valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 85, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->bStandbyEnable = valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	ESS_DEVICE_TYPE devType = (ESS_DEVICE_TYPE)valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->devType = (ESS_DEVICE_TYPE)valGet.val.emValue;
	pIFY->iLiquidPackNum = getLiquidCooledPackNum(pIFY->devType);

	pIFY->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd

	memset(&pIFY->stCloudUpload, 0, sizeof(CLOUD_UPLOAD));

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 55, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_IFY)? TRUE : FALSE; 
	//-----------------------End----------------------//
	
	pReporter->hThread = RunThread_Create("ES-IFYComm",
		(RUN_THREAD_START_PROC)_ThreadEntry_IFYComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACEX("ERROR : Create IFYComm thread Failed \n");
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
	};

	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("IFY_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return TRUE;
}
 
void Send_Call(ES_IFY_DATA_STU* pIFY, cJSON* pJson, ES_IFY_MSG_TYPE msgType, 
			MSGCALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	ES_IFY_CALL_MSG msg;
	ZERO_POBJS(&msg, 1);
	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("MsgCall[%d], msgSrc = %d, Json_Print = %s\n",pIFY->stMsgInfoList[msgType].nMsgId, emMsgSrc,
	//	szData);

	cJSON_Delete(pJson);

	//make the msg
	msg.emMsgType = msgType;
	msg.emSrc = emMsgSrc;
	msg.nBelongedConn = nConnNum;
	//memcpy(msg.szUniqMsgID, pIFY->stSuspendedMsgList[connectorNo].nUniqMsgId, sizeof(pIFY->stSuspendedMsgList[connectorNo].nUniqMsgId));
	//sprintf(msg.nUniqMsgId, "%lu", (unsigned int)(GetCurrentTime() * 1000));
	memset(msg.szJsonData, 0, ES_IFY_MAX_SENTMSG_SIZE);
	//memcpy(msg.szJsonData, szData, strlen(szData));
	snprintf(msg.szJsonData, ES_IFY_MAX_SENTMSG_SIZE, "%s", szData);
	//msg.nSentLength = strlen(szData);
	free(szData);

	HANDLE hq = pIFY->hqMsg2Sent;
	if(emMsgSrc == CALL_SRC_HisTrans)
	{
		hq = pIFY->hqHistoryTrans;
	}


	if(pIFY->emIFYConnStatus != Ready)
	{
		if((msgType == C_100_BootNotif)) //can allow only these 3 types while pending
		{
			if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
				MakeESIFY_CallFrame(pIFY, &msg, FALSE), pIFY->nSendBuffLen) <= 0)
			{
				pIFY->nConnErrTimes++;
				if (pIFY->nConnErrTimes >= 3)
				{
					pIFY->emIFY_Disconn_Reason = ES_IFY_COMM_FAIL_REASON_TIMEOUT_RECEIVE;
					pIFY->nConnErrTimes = 0;
					pIFY->emIFYConnStatus = Disconnected;
					TRACEX("emIFYConnStatus : Disconnected \n");
				}
				//Queue_Put(hq, &msg, bUrgent);
				TRACEX("Write Boot-Related-Msg [%s] failed\n", pIFY->szSendBuffer);
			}
			else
			{
				//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);
				//pIFY->nConnErrTimes = 0;
				TRACEX("Write Boot-Related-Msg [%s] successfully\n", pIFY->szSendBuffer);
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
		if(bUrgent && (hq == pIFY->hqMsg2Sent))
		{
			pIFY->nQUrgentMsgCount++;
		}
		//Jimmy added 20190319: if there are 2 sessions stored while off line, and connection is restored,
		//we should discard the old session(history list will handle it) in case queue is too old and new session is missing unexpectedly
		// if((msgType == StartTransaction) && (emMsgSrc == CALL_SRC_InProcessTrans))
		// {
		// 	if(Queue_GetCount(hq, NULL) > 10) //give 10 is to leave some extra time for old session handling
		// 	{
		// 		LogOut(MAIN_MODULE_REPORTER, "IFY", LOG_TYPE_WARING,
		// 			"Offline charge session for gun[%d] is removed from transaction queue!", nConnNum + 1);
		// 		Queue_Empty(hq);
		// 	}
		// }
		Queue_Put(hq, &msg, bUrgent);
		TRACEX("Queue_Put NORMAL Call Successfully into WebSocket %s \n", msg.szJsonData);
	}
}

void Send_CallResult(ES_IFY_DATA_STU* pIFY, cJSON* pJson, ES_IFY_MSG_TYPE msgType)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

	snprintf(pIFY->szSendBuffer, ES_IFY_MAX_SENTMSG_SIZE, "{\"id\":\"R%d\",	\"uniqKey\":\"%s\",	\"content\":%s}",
		pIFY->stMsgInfoList[msgType].nMsgId,
		pIFY->szHandlingUniqID,
		szData);

	free(szData);

	pIFY->nSendBuffLen = strlen(pIFY->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pIFY->szSendBuffer, pIFY->nSendBuffLen);

	if (pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pIFY->szSendBuffer, pIFY->nSendBuffLen))
	{
		//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);
		DUMP_ESIFYMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);//edit by kuangyue 20250313
		// LOG_INFO("ES_IFY pReporter=%p, type=%d, Data='%s'",
		// 	pReporter,
		// 	DUMP_TYPE_WRITE,
		// 	pIFY->szSendBuffer);
		TRACEX("Write CallResult Successfully into WebSocket\n");
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "IFY", LOG_TYPE_ERROR,
			"MsgCallResult [%d] is NOT written...", pIFY->stMsgInfoList[msgType].nMsgId);
		TRACEX("Write CallResult ERROR by WebSocket\n");
	}
}


void Send_CallError(ES_IFY_DATA_STU* pIFY, ES_IFY_MSG_TYPE msgType, 
		ES_IFY_CALLERROR_CODE emCode, const char* szErrDescrip)
{
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	snprintf(pIFY->szSendBuffer, ES_IFY_MAX_SENTMSG_SIZE, "{\"id\":\"E%d\",	\"uniqKey\":\"%s\",	\"content\":{\"code\":%d, \"err_desp\":\"%s\"}}",
		pIFY->stMsgInfoList[msgType].nMsgId,
		pIFY->szHandlingUniqID,
		emCode, szErrDescrip);

	pIFY->nSendBuffLen = strlen(pIFY->szSendBuffer);
	TRACEX("EVERYMSG == %s  Length = %d \n", pIFY->szSendBuffer, pIFY->nSendBuffLen);

	if(pReporter->pUsedCommPort->Write(pReporter->hCommPort,
		pIFY->szSendBuffer, pIFY->nSendBuffLen))
	{
		//DUMP_ESIFYMSG(pReporter, DUMP_TYPE_WRITE, pIFY->szSendBuffer);
		TRACEX("Write CallError Successfully into WebSocket\n");
		LogOut(MAIN_MODULE_REPORTER, "IFY", LOG_TYPE_WARING,
			"MsgCallError for [%d], ErrDescription = [%s], is sent out...", 
			pIFY->stMsgInfoList[msgType].nMsgId, szErrDescrip);
	}
	else
	{
		LogOut(MAIN_MODULE_REPORTER, "IFY", LOG_TYPE_ERROR,
			"MsgCallError for [%d], ErrDescription = [%s], is NOT written...", 
			pIFY->stMsgInfoList[msgType].nMsgId, szErrDescrip);
		TRACE("Write CallResult ERROR by WebSocket\n");
	}
	return;
}
 


