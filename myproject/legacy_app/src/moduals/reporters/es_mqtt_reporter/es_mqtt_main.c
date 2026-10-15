#include "../reporters_base.h"
#include "es_mqtt.h"
#include <signal.h>

 
static void ReCreate_MqttClient_AndInit(MQTT_DATA_ROUGH* pMqtt);

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Event_Handler()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-27 22:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	    emEvType,
	_EVENT_MSG	    emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			    nMaxBlockMSec)
{
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n = 0;

	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response

	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			TRACE("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}

			pMqtt->bNeedRefreshActvAlm = TRUE;
		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			//Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);
			pMqtt->emFirmwareUpgradeStatus = pFirmUpSta->emUpdateStatus;
			//Now call the msg handler to create the msg to send
			pMqtt->stMsgInfoList[C_405_StatOfFirmw].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			//Mutex_Unlock(pMqtt->hMutex);
		}
		break;
		
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
			//Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);
			pMqtt->emDiagnosUploadStatus = pDiagUpSta->emUploadStatus;
			//Now call the msg handler to create the msg to send
			pMqtt->stMsgInfoList[C_407_StatOfDiag].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			//Mutex_Unlock(pMqtt->hMutex);
		}
		break;
		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;
		
		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACE("MQTT_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
			
			if (pParamChg->parmPTR.nSubId == 26)
			{
				// Read ES Backend Config 
				TRACEX("Config Changed Evevt \n");
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pMqtt->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
					return FALSE;
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 25))
			{
				TRACE("nId=[%d] nSubId=[%d]\n", pParamChg->parmPTR.nId ,pParamChg->parmPTR.nSubId);
				char	szServerUrl[255]; //Host URL
				valGet.nGetSize = 255;
				valGet.val.blobValue = (BYTE*)szServerUrl;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 25, &valGet, sizeof(PARAM_VAL_GET));
				if (err == ERRCODE_DAI_OK)
				{
					if(strcmp(szServerUrl, pMqtt->szServerUrl)!= 0)
					{
						snprintf(pMqtt->szServerUrl, 255, "%s", szServerUrl);
						pMqtt->emMqttCommStatus = MQTT_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason = MQTT_FAIL_REASON_SERVERADDR_CHANGED;
						TRACE("Update URL: szServerUrl=[%s]\n", szServerUrl);
					}	
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 11))
			{
				valGet.nGetSize = 4;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 11, &valGet, sizeof(PARAM_VAL_GET));
				pMqtt->bZlibCompress = (valGet.val.emValue == 0) ? FALSE : TRUE;
				//printf("pMqtt->bZlibCompress = %d\n", pMqtt->bZlibCompress);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 34))
			{
				valGet.nGetSize = 4;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
						1, 34, &valGet, sizeof(PARAM_VAL_GET));
				pMqtt->emApplyArea = valGet.val.emValue;
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 35))
			{
				TRACE("nId=[%d] nSubId=[%d]\n", pParamChg->parmPTR.nId ,pParamChg->parmPTR.nSubId);
				char szSelfTopic[32]; 
				valGet.nGetSize = 32;
				valGet.val.blobValue = (BYTE*)szSelfTopic;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 35, &valGet, sizeof(PARAM_VAL_GET));
				if (err == ERRCODE_DAI_OK)
				{
					if(strcmp(szSelfTopic, pMqtt->szSelfTopic)!= 0)
					{
						snprintf(pMqtt->szSelfTopic, 32, "%s", szSelfTopic);
						snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/up", pMqtt->szSelfTopic, pMqtt->szClientID);
						snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/down/#", pMqtt->szSelfTopic, pMqtt->szClientID);
						pMqtt->emMqttCommStatus = MQTT_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason = MQTT_FAIL_REASON_SERVERADDR_CHANGED;
						TRACE("Update szSelfTopic: [%s]\n", szSelfTopic);
					}	
				}
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pMqtt->nCfgBMSNum = valGet.val.ulValue;
				if(pMqtt->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pMqtt->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				TRACEX("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pMqtt->nCfgBMSNum);
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

time_t	Parse_MqttTime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Make MqttTimeString()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : ISO8601 Format  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
char* Make_MqttTimeString(MQTT_DATA_ROUGH* pMqtt, time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	if(pMqtt->emApplyArea == 0)//Local
	{
		snprintf(timeinstring, 32, "%4d-%02d-%02d %02d:%02d:%02d",currenttime.tm_year, currenttime.tm_mon + 1, 
			currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);
	}
	else
	{
		snprintf(timeinstring, 32,"%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
			currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);
	}
	memcpy(pszOut, timeinstring, 32);
	return pszOut;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Make Make_UniqID_Mqtt()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static UINT Make_UniqID_Mqtt(void)
{
	//time_t tmNow = _NOW_;
	//double tmPrecNow = GetCurrentTime();
	//double diffMsec= (tmPrecNow - tmNow) * 100.0;

	//UINT nUniqID = tmNow * 100 + (UINT)diffMsec;
        #define MQTT_UNIQID_START	25689566
	static UINT s_nOldUniqID = 0;
	if(s_nOldUniqID == 0)
	{
		s_nOldUniqID = (UINT)_NOW_ + MQTT_UNIQID_START; //just to mix the value
	}
	//static UINT s_nOldUniqID = OCPP_UNIQID_START;
	UINT nUniqID = s_nOldUniqID;

	s_nOldUniqID++;
	if(s_nOldUniqID == 0xFFFFFFFF)
	{
		s_nOldUniqID = MQTT_UNIQID_START;
	}

	TRACE("---The uniqID = %lu----\n", nUniqID);

	return nUniqID;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT myAtoi()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-26 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
unsigned int  myAtoi(char *str)
{
	UINT res = 0; // Initialize result 

	// Iterate through all characters of input string and 
	// update result 
	int i;
	for (i = 0; str[i] != '\0'; ++i)
		res = res * 10 + str[i] - '0';

	// return result. 
	return res;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Log_ConnectStatuInfo()
*  CREATOR       : John
*  CREATED DATE  : 2021-12-15 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void LOG_MQTT(MQTT_DATA_ROUGH* pMqtt, int rc)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	static MQTT_COMM_STATUS emLast_CommStatuCode = MQTT_COMM_DISCONECTED;
	char szDump[256] = {0};

	if(pMqtt->emMqttCommStatus == MQTT_COMM_CONNECTED)
	{
		sprintf(szDump, "Open[%s]ID[%s]Name[%s]PWD[%s]rc[%d]:[%s]", pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName,pMqtt->szPassWord, rc,"Success");
		DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_DISCONECTED)
	{
		if(pMqtt->emMqttCommStatus != emLast_CommStatuCode)
		{
			sprintf(szDump, "Close[%s]ID[%s]Name[%s]PWD[%s]:Reason[%d]rc[%d]", pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName,pMqtt->szPassWord, pMqtt->emMqtt_Disconn_Reason,rc);
			DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_CLOSE_COMM, szDump);
		}
	}
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_WAIT_REBUILD_CLIENT)
	{
		sprintf(szDump, "Rebuild[%s]ID[%s]Name[%s]PWD[%s]:Reason[%d]rc[%d]", pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName,pMqtt->szPassWord, pMqtt->emMqtt_Disconn_Reason,rc);
		DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
	
	emLast_CommStatuCode = pMqtt->emMqttCommStatus;  

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT PackMqtt_CallFrame()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-26 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
char *PackMqtt_CallFrame(MQTT_DATA_ROUGH* pMqtt, MQTT_CALL_MSG *pMsg)
{
     TRACE("--- Start PackMqtt_CallFrame ---\n");
     char szTime[32];
     memset(pMqtt->szSendBuffer, 0 , MAX_MQTT_BUFFER_LEN);

     UINT nMsgUniqID = Make_UniqID_Mqtt(); //(UINT)(GetCurrentTime() * 1000);
     
     snprintf(pMqtt->szSendBuffer, MAX_MQTT_BUFFER_LEN, "{\"id\":\"Q%d\",	\"uniqKey\":\"%d\",     \"dateTime\":\"%s\",	\"content\":%s}",
	    pMqtt->stMsgInfoList[pMsg->emMsgType].nMsgId, 
        nMsgUniqID,
        Make_MqttTimeString(pMqtt, time(NULL), szTime),
	    pMsg->szJsonData);

	pMqtt->nSendBuffLen = strlen(pMqtt->szSendBuffer);

	if(pMqtt->bZlibCompress == TRUE)
	{
		Zlib_CompressData(pMqtt);
	}

	//printf("The sent out of WS pkg is [%s], nSendBuffLen = %d,  nCompressSendBuffLen=[%d]\n", \
	//	pMqtt->szSendBuffer, pMqtt->nSendBuffLen,  pMqtt->nCompressSendBuffLen);

 	return pMqtt->szSendBuffer;

}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Send_Call()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-26 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Send_Call(MQTT_DATA_ROUGH* pMqtt, cJSON* pJson, MQTT_MSG_TYPE msgType, 
			MSG_CALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent)
{
	MQTT_CALL_MSG   msg;
	ZERO_POBJS(&msg, 1);
	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("MsgCall[%d], msgSrc = %d, Json_Print = %s\n",pMqtt->stMsgInfoList[msgType].nMsgId, emMsgSrc,
	//	szData);

	cJSON_Delete(pJson);

	//make the msg
	msg.emMsgType = msgType;
	msg.emSrc = emMsgSrc;
	msg.nBelongedConn = nConnNum;
	//memcpy(msg.szUniqMsgID, pMqtt->stSuspendedMsgList[connectorNo].nUniqMsgId, sizeof(pMqtt->stSuspendedMsgList[connectorNo].nUniqMsgId));
	//sprintf(msg.nUniqMsgId, "%lu", (unsigned int)(GetCurrentTime() * 1000));
	memset(msg.szJsonData, 0, MAX_MQTT_BUFFER_LEN);
	//memcpy(msg.szJsonData, szData, strlen(szData));
	snprintf(msg.szJsonData, MAX_MQTT_BUFFER_LEN, "%s", szData);
	//msg.nSentLength = strlen(szData);
	free(szData);

	//HANDLE hq = pMqtt->hqMsg2Sent;
	//Queue_Put(hq, &msg, bUrgent);

    PackMqtt_CallFrame(pMqtt, &msg);

	TRACEX("Queue_Put NORMAL Call Successfully into Socket: %s \n", msg.szJsonData);
        
    return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Send_CallResult()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Send_CallResult(MQTT_DATA_ROUGH* pMqtt, cJSON* pJson, MQTT_MSG_TYPE msgType)
{
	//REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

    char szTime[32];
	char *szData = 0;

	szData = cJSON_Print(pJson);
	TRACEX("Send_CallResult Json_Print = %s\n",szData);

	cJSON_Delete(pJson);

    snprintf(pMqtt->szSendBuffer, MAX_MQTT_BUFFER_LEN, "{\"id\":\"R%d\",	\"uniqKey\":\"%s\",     \"dateTime\":\"%s\",	\"content\":%s}",
        pMqtt->stMsgInfoList[msgType].nMsgId,
        pMqtt->szHandlingUniqID,
        Make_MqttTimeString(pMqtt, time(NULL), szTime),
        szData);

	free(szData);

	pMqtt->nSendBuffLen = strlen(pMqtt->szSendBuffer);

	if(pMqtt->bZlibCompress == TRUE)
	{
		Zlib_CompressData(pMqtt);
	}

    Publish_MqttMessage(pMqtt);

	TRACEX("Send_CallResult: EVERYMSG == %s  Length = %d \n", pMqtt->szSendBuffer, pMqtt->nSendBuffLen);

	//TRACEX("Write CallResult Successfully into WebSocket\n");
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Send_CallError(MQTT_DATA_ROUGH* pMqtt, MQTT_MSG_TYPE msgType, 
		MQTT_CALLERROR_CODE emCode, const char* szErrDescrip)
{
	//REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    char szTime[32];

	snprintf(pMqtt->szSendBuffer, MAX_MQTT_BUFFER_LEN, "{\"id\":\"E%d\",\"uniqKey\":\"%s\",	\"dateTime\":\"%s\", \"content\":{\"code\":%d, \"err_desp\":\"%s\"}}",
		pMqtt->stMsgInfoList[msgType].nMsgId,
		pMqtt->szHandlingUniqID,
        Make_MqttTimeString(pMqtt, time(NULL), szTime),
		emCode, 
        szErrDescrip);

	pMqtt->nSendBuffLen = strlen(pMqtt->szSendBuffer);
	TRACEX("Send_CallError: EVERYMSG == %s  Length = %d \n", pMqtt->szSendBuffer, pMqtt->nSendBuffLen);

	if(pMqtt->bZlibCompress == TRUE)
	{
		Zlib_CompressData(pMqtt);
	}

    Publish_MqttMessage(pMqtt);

 	//TRACEX("Write CallError Successfully into MQTT\n");
	LogOut(MAIN_MODULE_REPORTER, "MQTT", LOG_TYPE_WARING,
		"MsgCallError for [%d], ErrDescription = [%s], is sent out...", 
		pMqtt->stMsgInfoList[msgType].nMsgId, szErrDescrip);
 
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  FindMsg ByName()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/  
static MQTT_MSG_TYPE FindMsg_ByMsgId(MQTT_DATA_ROUGH* pMqtt, int nMsgId, MSG_CALL_TYPE callType)
{
	int n = 0;
	for(n = 0; n < MAX_PUB_MSG_NUMBER; n++)
	{
		TRACEX("\t pMqtt->stMsgInfoList[%d].nMsgId=[%d] \n", n, pMqtt->stMsgInfoList[n].nMsgId);
		if(pMqtt->stMsgInfoList[n].nMsgId == nMsgId)
		{
			TRACEX("nMsgId :[%d] \t stMsgInfoList[n].nMsgId:[%d]nMsgType=[%d]\n", \
				nMsgId, pMqtt->stMsgInfoList[n].nMsgId, pMqtt->stMsgInfoList[n].emMsgType);
			return pMqtt->stMsgInfoList[n].emMsgType;
		}
	}

	return Msg_Unkown;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  (SafeFree_ArrivedMsg)
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
//Jimmy note: !!!!!do NOT use 'free' but need use 'MQTTAsync_free' because the lib re-defined the HEAP
static int SafeFree_ArrivedMsg(char *topicName, MQTTClient_message *message)
{
	if(message != NULL)
	{
		MQTTClient_freeMessage(&message); 
		MQTTClient_free(topicName);
	}	

	return MQTTCLIENT_SUCCESS; //should return 0
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_MqttSyncTime
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Handle_MqttSyncTime(MQTT_DATA_ROUGH *pMqtt, time_t tm)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	static time_t	tm_Last;
	VAR_VALUE		varV;

	if(ABS(tm - _NOW_) > 5) //sync RTC
	{
		TRACE("tm=[%ld] _NOW_=[%ld]\n", tm, _NOW_);
	
		varV.dtValue = tm;
		if(ABS(_NOW_ - tm_Last) > 10) // Avoid Sync Time Too Frequently
		{
			tm_Last = _NOW_;
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
		}
	}
	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Disconnect_MqttClientComm()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-13 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Disconnect_MqttClientComm(MQTT_DATA_ROUGH *pMqtt, int nMSecTimeout)
{       
    TRACEX("---Handle_DisconnectComm---\n");
    int rc = MQTTCLIENT_SUCCESS;

	if(pMqtt->client == NULL)
	{
		TRACE("Error:Disconnect_MqttClientComm:pMqtt->client == NULL\n");
		LogOut(MAIN_MODULE_REPORTER, "MQTT", LOG_TYPE_WARING,
			"Disconnect_MqttClientComm: pMqtt->client == NULL");
	}
	
	//printf("MQTTClient_unsubscribe: 111\n");

	// unsub all old subscribtions before new-sub
	//??if Subscribe Not ok,need unsubscribe?
	if(pMqtt->emMqttCommStatus == MQTT_COMM_READY)
	{
		rc = MQTTClient_unsubscribe(pMqtt->client, pMqtt->szSubTopic);
	}
	
	//printf("MQTTClient_unsubscribe: 2222\n");

	Sleep(1000);

	pMqtt->tmLastSend_DisConn = _NOW_;  
	pMqtt->emMqttCommStatus = MQTT_COMM_DISCONECTED;
	rc = MQTTClient_disconnect(pMqtt->client, nMSecTimeout);
	
	//printf("MQTTClient_disconnect: 333\n");

	////LOG_MQTT(MQTT_LOG_DISCONNECT, "reason->[%d], rc->%d", pMqtt->emMqtt_Disconn_Reason, rc);
	LOG_MQTT(pMqtt, rc);

	if(rc != MQTTCLIENT_SUCCESS)
	{
		TRACEX("Failed to Disconnect, return code %d\n", rc);
		return;
	}
	
	pMqtt->bIs_MqttConnOK = FALSE;//means already disconn
	
	return;
}


static void onConnLost(MQTT_DATA_ROUGH *pMqtt, int rc)
{
	time_t tmNow = _NOW_;
	if(ABS(_NOW_ - pMqtt->stMsgInfoList[C_101_StatusNotif].tmLastArrived) < 90)
	{ 
		return;
	}

 	Disconnect_MqttClientComm(pMqtt, 5000);
	pMqtt->emMqtt_Disconn_Reason = MQTT_FAIL_SERVER_LOSTCONN;
	LOG_MQTT(pMqtt, rc);

	return;
}


static int Receive_Resp_Messge(MQTT_DATA_ROUGH *pMqtt, int nMSecTimeout)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	MSG_CALL_TYPE	emCallType = 0;
	MQTT_MSG_TYPE	msgType = Msg_Unkown;

	if(pMqtt->client == NULL)
	{
		LogOut(MAIN_MODULE_REPORTER, "Mqtt", LOG_TYPE_ERROR,
				"Receive_Resp_Messge:pMqtt->client == NULL");	
		return;
	}

	char* 	topicName = NULL;
	int 	topicLen = 0;

	MQTTClient_message* message = NULL;
	int rc = MQTTClient_receive(pMqtt->client, &topicName, &topicLen, &message, nMSecTimeout);
	if (rc != 0)
	{
		onConnLost(pMqtt, rc);
		TRACEX("rc=[%d] receive Failed!!\n", rc);
		return 0;
	}

	if(message == NULL)
	{
		TRACEX("!!!Read Nothing From Server!!!\n");
		onConnLost(pMqtt, rc);
		return 0;
	}

	//filter those invalid topic/messages
	if((message->payloadlen < 0) || (topicName == NULL)/* || (topicLen < 3)*/) //topicLen maybe 0 as of the remaing topics in queue
	{
		TRACEX("INVALIDE topic ->[%s, %d], message->payloadlen->[%d]\n", (topicName == NULL) ? "NULL" : topicName, 
			topicLen, message->payloadlen);
		onConnLost(pMqtt, rc);
		return SafeFree_ArrivedMsg(topicName, message);
	}

	TRACEX("topicsssss[%s, %d]-arravied, payloadlen=[%d], payload->[%s]\n", 
		topicName, topicLen, message->payloadlen, 
		(message->payload == NULL) ? "NULL" : message->payload);

	if(pMqtt->emRepInstType == REP_TYPE_MQTT_TLS)
	{
		if(strcmp(topicName, "sub") < 0) //if Recv Msg is Not Sub Topic,Ignore
		{
			return SafeFree_ArrivedMsg(topicName, message);
		}
	}
	else
	{
		if(strstr(topicName, "down") < 0) //if Recv Msg is Not Sub Topic /001,Ignore
		{
			//!!! 2021-11-30 John.Dont Know Why Recv Abnormal Topic Data,Need Process Later
			TRACEX("Error: Wrong Topic, Just Ignore!!!\n");
			return SafeFree_ArrivedMsg(topicName, message);
		}
	}	
	
	char *pszPos = (char*)message->payload;
    char *pszEnd = NULL;
    
	if(pszPos == NULL)
    {
        TRACE("Wrong Msg message->payload is NULL: \n out!!");
        return SafeFree_ArrivedMsg(topicName, message);
    }

    pMqtt->nReadBuffLen = message->payloadlen;

	memset(pMqtt->szReadBuffer, 0 ,MAX_MQTT_BUFFER_LEN);
	memcpy(pMqtt->szReadBuffer, pszPos, pMqtt->nReadBuffLen);
	DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_READ, pMqtt->szReadBuffer);

    while(pMqtt->nReadBuffLen > 0)
    {
        cJSON *json = NULL;
        json = cJSON_ParseWithOpts(pszPos,&pszEnd, FALSE);
        if (!json)
        {
            TRACEX("Error ParseMessage [%s] pMqtt->nReadBuffLen=[%d]\n", cJSON_GetErrorPtr(), pMqtt->nReadBuffLen);
            //TRACEX(" Here IS the buffer %s \n", pszPos);
            goto exit;
        }

        cJSON *sectionId = cJSON_GetObjectItem(json, "id");
        cJSON *sectionUniqKey = cJSON_GetObjectItem(json, "uniqKey");
        cJSON *sectionDateTime = cJSON_GetObjectItem(json, "dateTime"); 
        cJSON *sectionContent = cJSON_GetObjectItem(json, "content");
        if((sectionId == NULL) || (sectionUniqKey == NULL) || (sectionContent == NULL) || (sectionDateTime == NULL))
        {
            TRACE("11111Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" 11111118Here IS the buffer %s \n", pszPos);
            cJSON_Delete(json);
            goto exit;
        }
        else if((cJSON_String != sectionId->type) || (cJSON_String != sectionUniqKey->type) ||
                (cJSON_String != sectionDateTime->type) || (cJSON_Object != sectionContent->type))
        {
            TRACEX("88888Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" 8888888Here IS the buffer %s \n", pszPos);
            cJSON_Delete(json);
            goto exit;
        }

        if((sectionId->valuestring == NULL) || (sectionUniqKey->valuestring == NULL))
        {
            TRACEX("22222Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" 22222222Here IS the buffer %s \n", pszPos);
            cJSON_Delete(json);
            goto exit;
        }

        if(strlen(sectionId->valuestring) != 4)
        {
            TRACEX("333333Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" 3333333333Here IS the buffer %s \n", pszPos);
            cJSON_Delete(json);
            goto exit;
        }

		TRACEX(" Here IS the buffer %s \n", pszPos);

        //now parse the id Section
        char c0 = *sectionId->valuestring;
        if(c0 == 'Q')
        {
            emCallType = Call;
        }
        else if(c0 == 'R')
        {
            emCallType = CallResult;
        }
        else if(c0 == 'E')
        {
            emCallType = CallError;
        }
        else
        {
            TRACEX("4444Error ParseMessage [%s]\n", cJSON_GetErrorPtr());
            TRACEX(" 4444Here IS the buffer %s \n", pszPos);
            cJSON_Delete(json);
            goto exit;
        }

        int nMsgId = myAtoi(sectionId->valuestring + 1);
        
		TRACEX("nMsgId=[%d] emCallType=[%d] sectionId->valuestring=%s\n", nMsgId, emCallType, sectionId->valuestring);

        if(emCallType == Call)
        {
			//printf("Start Find Msg\n");
            msgType = FindMsg_ByMsgId(pMqtt, nMsgId, Call);
            TRACEX("!!! msgType =%d sectionId->valuestring=%s\n", msgType, sectionId->valuestring);
            
            //now handling cmd
            pMqtt->pCurrJson = sectionContent;
            sprintf(pMqtt->szHandlingUniqID, "%s", sectionUniqKey->valuestring);
            //JimmyNote 20190307: if use cJSON_Print, MUST free the string after done
            //TRACEX("!!!!!Server Call = Msg [%s], Cont = [%s]\n", pMqtt->stMsgInfoList[msgType].szMsgName,
            //	cJSON_Print(json));

            if(msgType != Msg_Unkown)
            {
                pMqtt->stMsgInfoList[msgType].pfnMsgHandler(pMqtt);
                //pMqtt->tmLastSuccessExchange = _NOW_;
            }
            else
            {
                char szErrMsgName[256] = {0};
                sprintf(szErrMsgName, "Unkown Messege -> [%s]!", sectionId->valuestring);
                TRACEX("Unkown Messege -> szErrMsgName=[%s]\n", szErrMsgName);
                //Send_CallError(pMqtt, Msg_Unkown, CallError_FormatError, szErrMsgName);
            }     
        }
        else if(emCallType == CallResult)
        {
			TRACEX("--- Recv Mqtt CallResult ---\n");
			msgType = FindMsg_ByMsgId(pMqtt, nMsgId, CallResult);
            TRACEX("!!! msgType =%d sectionId->valuestring=%s\n", msgType, sectionId->valuestring);
            if(msgType != Msg_Unkown)
            {
				if (msgType == C_101_StatusNotif)
				{
					TRACEX("Status Response Recv 101 Resp.\n");
					pMqtt->stMsgInfoList[C_101_StatusNotif].tmLastArrived = _NOW_;
					cJSON *temp1 = NULL;
 					if (temp1 = cJSON_GetObjectItem(sectionContent, "time"))
					{
						if (cJSON_String == temp1->type)
						{
							time_t tm = Parse_MqttTime(temp1->valuestring);
							Handle_MqttSyncTime(pMqtt, tm);
						}
					}	
				}
			}
			else
			{
				TRACEX("Unkown Messege -> szErrMsgName=[%s]\n", sectionId->valuestring);
			}
		}
		else //CallError
		{
			TRACEX("Warning: MQTT No CallError!!!.Recv CallError emCallType=[%d]\n", emCallType);	 
		}

		cJSON_Delete(json);     //!!! do not forget to destroy the json
		//now continue parsing for another frame

        TRACEX("pszPos = %lu \npszEnd = %lu \n diff = %d \n", pszPos, pszEnd, (pszPos - pszEnd));
        pMqtt->nReadBuffLen -= (pszEnd - pszPos);
        TRACEX("Len pMqtt->nReadBuffLen : %d \n  ", pMqtt->nReadBuffLen);
        pszPos = pszEnd;
    }

  
    TRACEX("message->payloadlen=[%d]\n", message->payloadlen);

exit:
	TRACE("Warning: Do Not Forget Free Recv Message!!\n");
	return SafeFree_ArrivedMsg(topicName, message);
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Zlib_CompressData()
*  CREATOR       : John
*  CREATED DATE  : 2022-02-22 22:22:2
*  DESCRIPTION   : Zlib Compress
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Zlib_CompressData(MQTT_DATA_ROUGH *pMqtt)
{
 	int err = 0;

	pMqtt->nCompressSendBuffLen = MAX_MQTT_BUFFER_LEN;
	err = compress(pMqtt->szCompressSendBuff, &pMqtt->nCompressSendBuffLen, (const Bytef*)pMqtt->szSendBuffer, pMqtt->nSendBuffLen);
	if(err != 0)
 	{
		TRACE("Error: err=%d pMqtt->nCompressSendBuffLen=%d:Compress Failed!\n", err, pMqtt->nCompressSendBuffLen);
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Publish_MqttMessage()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : MQTT Main() Entrance Function
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Publish_MqttMessage(MQTT_DATA_ROUGH *pMqtt)
{
	int rc = 0;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	if(pMqtt->client == NULL)
	{
		LogOut(MAIN_MODULE_REPORTER, "Mqtt", LOG_TYPE_ERROR,
				"Publish_MqttMessage:pMqtt->client == NULL");	
		return -1;
	}

	if(pMqtt->bZlibCompress == TRUE)
	{
		pubmsg.payload = pMqtt->szCompressSendBuff; 
		pubmsg.payloadlen = pMqtt->nCompressSendBuffLen;
	}
	else
	{
		pubmsg.payload = pMqtt->szSendBuffer;
		pubmsg.payloadlen = pMqtt->nSendBuffLen; 
	}

	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	static int s_nErrCounter = 0;
	MQTTResponse response = MQTTClient_publishMessage5(pMqtt->client, pMqtt->szPubTopic, &pubmsg, &token);
	rc = (int)response.reasonCode;
	if(rc != MQTTCLIENT_SUCCESS)
	{
		TRACEX("Failed to start sendTopic->[%s], payload=[%s], return code %d\n",
			pMqtt->szPubTopic, pubmsg.payload, rc);
		s_nErrCounter++;
	}
	else
	{
		//this is to wait sending for 500sec
		rc = MQTTClient_waitForCompletion(pMqtt->client, token, 500);
		if(rc == MQTTCLIENT_SUCCESS)
		{
			s_nErrCounter = 0;
 		}
		else
		{
			TRACEX("Failed To WaitForCompletion,token=[%d] rc=[%d]\n", token, rc);
			s_nErrCounter++;
		}
	}

	if(s_nErrCounter >= 10)
	{
		s_nErrCounter = 10;
		pMqtt->emMqttCommStatus = MQTT_COMM_DISCONECTED;
		//pMqtt->emMqtt_Disconn_Reason = MQTT_FAIL_PUB_ERROR; ??? Process Later
	}

	DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pMqtt->szSendBuffer);
	
	TRACEX("mqtt sendTopic->[%s], qos->[%d], token ->[%d], rc=[%d], pubmsg.payload=[%s]\n", \
		pMqtt->szPubTopic, pubmsg.qos, token, rc, pubmsg.payload);

	return (s_nErrCounter == 0)? 0 : -1;

}	


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT QueryActvAlarmInfo()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void QueryActvAlarmInfo(MQTT_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
 	ERRCODE_DAI errCode;
	UINT n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize = sizeof(EVENT_ALARM);
	cond.pCondArg = &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount = MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pMqtt->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pMqtt->activeAlms , &nQueryCount);
	TRACE("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

	if(errCode != ERRCODE_DAI_OK)
	{
		TRACEX("Error: Query Actv Alarm Failed!\n");
	}
	else
	{
		pMqtt->nActiveAlmTotal = MIN(nQueryCount, MAX_QUERY_ACTV_ALM);
		for(n = 0; n < pMqtt->nActiveAlmTotal; n++)
		{
			TRACEX("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pMqtt->activeAlms[n].nDevId, pMqtt->activeAlms[n].nAlarmId);
		}
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Check IfActvAlmStatuChanged()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Check_IfActvAlmStatuChanged(MQTT_DATA_ROUGH* pMqtt, int nDataFrom)
{
	BOOL bRet = FALSE;
	if(nDataFrom == 0)
	{
		if(pMqtt->bNeedRefreshActvAlm)
		{
			pMqtt->bNeedRefreshActvAlm = FALSE;
			QueryActvAlarmInfo(pMqtt);
			return TRUE;
		}
	}
	else
	{
		//TRACE("\n");
	}
	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Main Run Func()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : MQTT Main() Entrance Function
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Pub_MsgToServer(MQTT_DATA_ROUGH *pMqtt)
{       
    //int 			rc = 0;
    time_t 			tmNow = time(NULL);
	PARAM_VAL_GET 	valGet;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	
	//TRACE("pMqtt->blobBackendSet.nCmd_101_Interval=[%d]\n", pMqtt->blobBackendSet.nCmd_101_Interval);

    if(ABS(tmNow - pMqtt->stMsgInfoList[C_101_StatusNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_101_Interval)))
    {
        pMqtt->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pMqtt);
	    Publish_MqttMessage(pMqtt);
        pMqtt->stMsgInfoList[C_101_StatusNotif].tm_LastSend = tmNow;
    }

    if(ABS(tmNow - pMqtt->stMsgInfoList[C_102_BattCabStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_102_Interval)))
    {
        pMqtt->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pMqtt);
	    Publish_MqttMessage(pMqtt);
        pMqtt->stMsgInfoList[C_102_BattCabStaNotif].tm_LastSend = tmNow;
    }
    
    if(ABS(tmNow - pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_103_Interval)))
    {
        pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pMqtt);
	    Publish_MqttMessage(pMqtt);
        pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].tm_LastSend = tmNow;
    }
    
	if(pMqtt->blobBackendSet.nCmd_104_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_104_Interval)))
		{
			pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].tm_LastSend = tmNow;
		}
	}

    if(ABS(tmNow - pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_105_Interval)))
    {
        pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pMqtt);
	    Publish_MqttMessage(pMqtt);
        pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend = tmNow;
    }

	if(pMqtt->blobBackendSet.nCmd_106_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_106_SingleRectStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_106_Interval)))
		{
			pMqtt->stMsgInfoList[C_106_SingleRectStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_106_SingleRectStaNotif].tm_LastSend = tmNow;
		}
	}

	if(pMqtt->blobBackendSet.nCmd_107_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_107_Interval)))
		{
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend = tmNow;
		}
	}

    if(ABS(tmNow - pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].tm_LastSend) > (MAX(10, pMqtt->blobBackendSet.nCmd_108_Interval)))
    {
        pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].pfnMsgHandler((void*)pMqtt);
	    Publish_MqttMessage(pMqtt);
        pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].tm_LastSend = tmNow;
    }

	//printf("111 pMqtt->blobBackendSet.nCmd_110_Interval=[%d]\n", pMqtt->blobBackendSet.nCmd_110_Interval);
	if(pMqtt->blobBackendSet.nCmd_110_Interval != 0) //0 Means Not send To Cloud Platform.
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_110_SingleBattryInfo].tm_LastSend) > (MAX(10, pMqtt->blobBackendSet.nCmd_110_Interval)))
		{
			//printf("222 tmNow=[%d]\n", tmNow);
			int n = 0;
			for(n = 0; n < pMqtt->nCfgBMSNum; n++)
			{
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					501 + n, 550, &valGet, sizeof(PARAM_VAL_GET));
				if(valGet.val.emValue <= STBATT_RUNSTAT_COMM_FAIL)
				{
					continue;
				}
				pMqtt->nCurrBMS = n;
				pMqtt->stMsgInfoList[C_110_SingleBattryInfo].pfnMsgHandler((void*)pMqtt);
				Publish_MqttMessage(pMqtt);
			}
			pMqtt->stMsgInfoList[C_110_SingleBattryInfo].tm_LastSend = tmNow;
		}
	}

	BOOL bActvAlmChanged = Check_IfActvAlmStatuChanged(pMqtt, pMqtt->nCurrDataFrom);
	if((bActvAlmChanged == TRUE) ||
		(ABS(tmNow - pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_109_Interval))))
	{
		pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].pfnMsgHandler((void*)pMqtt);
		Publish_MqttMessage(pMqtt);
		pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend = tmNow;	
	}

	if(ABS(tmNow - pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_202_Interval)))
	{
		pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].pfnMsgHandler((void*)pMqtt);
		Publish_MqttMessage(pMqtt);
		pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend = tmNow;	
	}

	if(ABS(tmNow - pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_204_Interval)))
	{
		pMqtt->stMsgInfoList[C_204_DailyStaticKwh].pfnMsgHandler((void*)pMqtt);
		Publish_MqttMessage(pMqtt);
		pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend = tmNow;	
	}

	//return rc;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Init_MqttMsgInfo()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
#define INIT_MQTT_MSG(_pMsg,_emMsgType, _nMsgId, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
	(_pMsg)->nMsgId = (_nMsgId); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)
void Init_MqttMsgInfo(MQTT_DATA_ROUGH *pMqtt)
{
    MQTT_MSG_INFO* p = &(pMqtt->stMsgInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_MQTT_MSG(p,	C_100_BootNotif,			    100,			MQTT_MSG_C2S,		Prepare_C100_BootNotification);
	INIT_MQTT_MSG(p,	C_101_StatusNotif,			    101,			MQTT_MSG_C2S,		Prepare_C101_StatusNotification);
	INIT_MQTT_MSG(p,	C_102_BattCabStaNotif,			102,			MQTT_MSG_C2S,		Prepare_C102_BattCabStaNotif);
	INIT_MQTT_MSG(p,	C_103_ESBMSStaNotif,			103,			MQTT_MSG_C2S,		Prepare_C103_ESBMSStatusNotif);
	INIT_MQTT_MSG(p,	C_104_PwrDistrStaNotif,			104,			MQTT_MSG_C2S,		Prepare_C104_PwrDistrStaNotif);
	INIT_MQTT_MSG(p,	C_105_ESRunDataStaNotif,		105,			MQTT_MSG_C2S,		Prepare_C105_ESRunDataNotif);
	INIT_MQTT_MSG(p,	C_106_SingleRectStaNotif,		106,			MQTT_MSG_C2S,		Prepare_C106_SingleRectStaNotif);
	INIT_MQTT_MSG(p,	C_107_PwrDistrCabStaNotif,		107,			MQTT_MSG_C2S,		Prepare_C107_PwrDistrCabStaNotif);
	INIT_MQTT_MSG(p,	C_108_SysVersionStaNotif,		108,			MQTT_MSG_C2S,		Prepare_C108_SysVersionStaNotif);
	INIT_MQTT_MSG(p,	C_109_SysActvAlmNotif,			109,			MQTT_MSG_C2S,		Prepare_C109_SysActvAlmNotif);
	INIT_MQTT_MSG(p,	C_110_SingleBattryInfo,			110,			MQTT_MSG_C2S,		Prepare_C110_SiglBatDataNotif);

	INIT_MQTT_MSG(p,	C_155_ESRunDataStaNotif,		155,			MQTT_MSG_S2C,		Prepare_C105_ESRunDataNotif); // LJH250428 cmd155
	INIT_MQTT_MSG(p,	C_157_PwrDistrCabStaNotif,		157,			MQTT_MSG_S2C,		Prepare_C107_PwrDistrCabStaNotif); // LJH250520 cmd157

	INIT_MQTT_MSG(p,	S_111_Trigger,				    111,			MQTT_MSG_S2C,		Handle_S111_TriggerMessage);
 	INIT_MQTT_MSG(p,	C_202_HourSectorStaticKwh,		202,			MQTT_MSG_C2S,		Prepare_C202_HourSectorStaticKwh);
	INIT_MQTT_MSG(p,	C_204_DailyStaticKwh,			204,			MQTT_MSG_C2S,		Prepare_C204_DailyStaticKwh);
	
	INIT_MQTT_MSG(p,	S_401_RmtRest,				    401,			MQTT_MSG_S2C,		Handle_S401_Reset);
	INIT_MQTT_MSG(p,	S_402_RmtChgEVESEAbi,			402,			MQTT_MSG_S2C,		Handle_S402_ChangeEVSEAvailability);
 	INIT_MQTT_MSG(p,	S_404_RmtUpdtFirmw,			    404,			MQTT_MSG_S2C,		Handle_S404_UpdateFirmware);
	INIT_MQTT_MSG(p,	C_405_StatOfFirmw,			    405,			MQTT_MSG_C2S,		Prepare_C405_FirmwareStatusNotification);
	INIT_MQTT_MSG(p,	S_406_RmtCallDiag,			    406,			MQTT_MSG_S2C,		Handle_S406_GetDiagnostics);
	INIT_MQTT_MSG(p,	C_407_StatOfDiag,			    407,			MQTT_MSG_C2S,		Prepare_C407_DiagnosticsStatusNotification);
	INIT_MQTT_MSG(p,	S_411_RmtGetConfig,			    411,			MQTT_MSG_S2C,		Handle_S411_GetConfiguration);
	INIT_MQTT_MSG(p,	S_412_RmtSetConfig,			    412,			MQTT_MSG_S2C,		Handle_S412_ChangeConfiguration);

	INIT_MQTT_MSG(p,	S_503_RmtGetEMSPlan,			503,			MQTT_MSG_S2C,		Handle_S503_GetEMSPlan);
	INIT_MQTT_MSG(p,	S_504_RmtSetEMSPlanProfile,		504,			MQTT_MSG_S2C,		Handle_S504_SetEMSPlanProfile);
	
	INIT_MQTT_MSG(p,	S_601_RmtGetFeeRateCfg,			601,			MQTT_MSG_S2C,		Handle_S601_GetFeeRate);
	INIT_MQTT_MSG(p,	S_602_RmtSetFeeRateProfile,		602,			MQTT_MSG_S2C,		Handle_S602_SetFeeRateProfile);
	
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_CommStatus()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-14 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Handle_CommStatus(MQTT_DATA_ROUGH *pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	VAR_VALUE varV;
	static BOOL bLastCommOK = FALSE;

	if(pMqtt->bIs_MqttConnOK != bLastCommOK)
	{
		TRACE("pMqtt->bIs_MqttConnOK=[%d] bLastCommOK=[%d]\n", pMqtt->bIs_MqttConnOK, bLastCommOK);
		
		bLastCommOK = pMqtt->bIs_MqttConnOK;

		varV.emValue = (pMqtt->bIs_MqttConnOK == TRUE) ? 0 : 1;
		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);
	}
	
	return;
}
 




static BOOL Subscrible_AllTopics(MQTT_DATA_ROUGH *pMqtt)
{
	int rc = 0;
	if(pMqtt->client == NULL)
	{
		LogOut(MAIN_MODULE_REPORTER, "Mqtt", LOG_TYPE_ERROR,
				"Subscrible_AllTopics:pMqtt->client == NULL");	
		return FALSE;
	}

	pMqtt->emMqttCommStatus = MQTT_COMM_SUBSCRIBING;
	pMqtt->tmLast_CallSubscribe = _NOW_;

	

	if (pMqtt->emMQTTVersion == VERSION_5)
	{
		MQTTResponse response = MQTTClient_subscribe5(pMqtt->client, pMqtt->szSubTopic, QOS, NULL, NULL);
		rc = response.reasonCode;
		MQTTResponse_free(response);
	}
	else
	{
		rc = MQTTClient_subscribe(pMqtt->client, pMqtt->szSubTopic, QOS);
	}
	
	if(rc != MQTTCLIENT_SUCCESS)
	{
		TRACE("MQTTClient_subscribe Failed!!!\n");
		return FALSE;
	}

	pMqtt->emMqttCommStatus = MQTT_COMM_READY;

	//---Only Send One Times when First Time Register---//
	pMqtt->stMsgInfoList[C_100_BootNotif].pfnMsgHandler((void*)pMqtt);
	Publish_MqttMessage(pMqtt);

	TRACEX("szSubTopic=[%s] subscribe OK!\n", pMqtt->szSubTopic);
	return TRUE;
}

static void onMqtt_ConnectOK(MQTT_DATA_ROUGH* pMqtt)
{
	TRACEX("---Successful connection---\n");
	int rc = 0;
	pMqtt->emMqttCommStatus = MQTT_COMM_CONNECTED;
	pMqtt->bIs_MqttConnOK = TRUE;
	pMqtt->nConnFailedTimes = 0;

	//Log_ConnectStatuCode(pMqtt);

	pMqtt->stMsgInfoList[C_101_StatusNotif].tmLastArrived = _NOW_;
	pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend = _NOW_;
	pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend = _NOW_;
	
	//---Note:Sometimes Abnormal Reboot Cause /
	//System Not Avaliable send to CloudPlatfrom---/
	pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend = _NOW_;

	LOG_MQTT(pMqtt, rc);	
	
	return;
}

//static void onMqtt_ConnectFailed(void* context, MQTTAsync_failureData5* response)
static void onMqtt_ConnectFailed(MQTT_DATA_ROUGH* pMqtt, int rc)
{
	pMqtt->nConnFailedTimes++;
	if(pMqtt->nConnFailedTimes <= 5)
	{
		LOG_MQTT(pMqtt, rc);	
	}
	else if(pMqtt->nConnFailedTimes > 6)
	{
		pMqtt->nConnFailedTimes = 6;
	}
	else
	{
		pMqtt->nConnFailedTimes = 6;
	}

	return;
}

void trace_callback(enum MQTTCLIENT_TRACE_LEVELS level, char* message)
{
	fprintf(stderr, "Trace : %d, %s\n", level, message);
}

static void Connect_2MqttServer(MQTT_DATA_ROUGH* pMqtt)
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		MQTTClient_connectOptions conn_opts_5 = MQTTClient_connectOptions_initializer5;
		memcpy(&conn_opts, &conn_opts_5, sizeof(MQTTClient_connectOptions));
	}

	//ssl option
    MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
    sslopts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
	
	/*
	// TLS_AUTH_Unidirectional:
	// sslopts.trustStore = "/app/config/cacert.pem"; 	 // Path to the server's CA certificate, which the client uses to verify the server's certificate.
	// sslopts.keyStore = "/app/config/clientcert.pem";  // this is usually not needed, it is only required in a two-way authentication.
	// sslopts.privateKey = "/app/config/clientkey.pem"; // the client's private key is usually not needed, it is only required in a two-way authentication.
	// sslopts.privateKeyPassword = "your_key_password"; // the password for the client's private key is usually not needed, it is only required in a two-way authentication.

	// TLS_AUTH_Mutual:
	// sslopts.trustStore = "/app/config/cacert.pem";	 // Path to the server's CA certificate, which the client uses to verify the server's certificate.
	// sslopts.keyStore = "/app/config/clientcert.pem";	 // Path to the client's certificate, which the server uses to verify the client's certificate.
	// sslopts.privateKey = "/app/config/clientkey.pem"; // Path to the client's private key, which is used by the client to prove its claimed identity during the TLS handshake process.
	// sslopts.privateKeyPassword = "your_key_password"; // Password for the client's private key, which is required if the private key file is encrypted, to be provided when using the private key.

	if(pMqtt->emAuthLevel == TLS_AUTH_None)
	{
		sslopts.enableServerCertAuth = 0; //0-None CA Auth.
	}
	else if(pMqtt->emAuthLevel == TLS_AUTH_Unidirectional)
	{
		//----------John 20240218 Added For TLS CA -------------//
		const unsigned char alpn_protos[]={4, 'm', 'q', 't', 't'};
		sslopts.trustStore = "/app/config/cacert.pem";
		sslopts.protos = alpn_protos;
		sslopts.protos_len = strlen(sslopts.protos);
		sslopts.enableServerCertAuth = 1; //0-None CA Auth.
		//------------------TLS CA End -------------------------// 
	}
	else if(pMqtt->emAuthLevel == TLS_AUTH_Mutual)
	{
		const unsigned char alpn_protos[]={5, 'm', 'q', 't', 't','s'};
		sslopts.protos = alpn_protos;
		sslopts.protos_len = strlen(sslopts.protos);

		sslopts.enableServerCertAuth = 1; // Server certificate authentication required
		sslopts.trustStore = "/app/config/cacert.pem"; // Path to the server CA certificate
		sslopts.keyStore = "/app/config/client.pem"; // Client certificate
		sslopts.privateKey = "/app/config/client.key"; // Client private key
		// sslopts.privateKeyPassword = "your_key_password"; // Password for the private key, fill according to actual situation
	}
	*/

	// ALPN protocol names setup
	const unsigned char alpn_protos_uni[] = {4, 'm', 'q', 't', 't'};
	const unsigned char alpn_protos_mut[] = {5, 'm', 'q', 't', 't', 's'};

	// Common settings
	sslopts.enableServerCertAuth = (pMqtt->emAuthLevel != TLS_AUTH_None); // Enable server certificate authentication unless explicitly disabled
	sslopts.trustStore = "/app/config/cacert.pem"; // Path to CA certificate

	// Configure specific parameters based on the authentication level
	switch(pMqtt->emAuthLevel) 
	{
	    case TLS_AUTH_None:
	        // No certificate authentication
	        sslopts.enableServerCertAuth = 0;
	        break;
	    case TLS_AUTH_Unidirectional:
	        // Unidirectional authentication
	        sslopts.protos = alpn_protos_uni;
	        sslopts.protos_len = sizeof(alpn_protos_uni);
	        break;
	    case TLS_AUTH_Mutual:
	        // Mutual authentication
	        sslopts.protos = alpn_protos_mut;
	        sslopts.protos_len = sizeof(alpn_protos_mut);
	        sslopts.keyStore = "/app/config/client.pem"; // Client certificate
	        sslopts.privateKey = "/app/config/client.key"; // Client private key
	        // sslopts.privateKeyPassword = "your_key_password"; // Password for the private key
	        break;
	}

	conn_opts.ssl = &sslopts;
	conn_opts.ssl->verify = 1;
   	conn_opts.keepAliveInterval = 30;
	conn_opts.connectTimeout = MQTT_CONNET_TIMEOUT;
	conn_opts.username = pMqtt->szUserName; 
	conn_opts.password = pMqtt->szPassWord;  

	//----John 20240516 Added For Debug----//
	//MQTTClient_setTraceCallback(trace_callback);
	//MQTTClient_setTraceLevel(MQTTCLIENT_TRACE_MAXIMUM);
	//--------------End-------------------//

	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		conn_opts.cleanstart = 1; //this is for mqtt5 and for mqtt5 cleansession should be 0
		conn_opts.cleansession = 0;
		MQTTProperties props = MQTTProperties_initializer;
		MQTTProperty property;	
		property.identifier = MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
		property.value.integer4 = 0; //should be 0 to clean session after reconnection
		MQTTProperties_add(&props, &property); 
		MQTTResponse response = MQTTClient_connect5(pMqtt->client, &conn_opts, &props, NULL);
		if (response.reasonCode != MQTTREASONCODE_SUCCESS)
		{
			return onMqtt_ConnectFailed(pMqtt, (int)response.reasonCode);
		}
	}	
	else
	{
		conn_opts.cleanstart = 0;
		conn_opts.cleansession =  1;// should be 0 form MQTT5, 1 means not to save information from a previous connection of the same client
		int rc = MQTTClient_connect(pMqtt->client, &conn_opts);
		if (rc != MQTTCLIENT_SUCCESS)
		{
			return onMqtt_ConnectFailed(pMqtt, rc);
		}
	}

	return onMqtt_ConnectOK(pMqtt);
}


static void ReCreate_MqttClient_AndInit(MQTT_DATA_ROUGH* pMqtt)
{	
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	//disconnect old connection if have
	if(pMqtt->bIs_MqttConnOK)
	{
		Disconnect_MqttClientComm(pMqtt, 5000);
	}

	//destroy old client if have
	if(pMqtt->client != NULL)
	{
		TRACEX("----Remove old MQTT client before create!----\n");
		MQTTClient_destroy(&pMqtt->client);
	}

	int rc = 0;

	MQTTClient_createOptions createOptions = MQTTClient_createOptions_initializer;
	
	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		createOptions.MQTTVersion = MQTTVERSION_5;
	}
	else
	{
		createOptions.MQTTVersion = MQTTVERSION_3_1_1;  
	}
	
	rc = MQTTClient_createWithOptions(&pMqtt->client, pMqtt->szServerUrl, pMqtt->szClientID, 
		MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOptions);

 	if(pMqtt->nConnFailedTimes <= 5)
	{
		LogOut(MAIN_MODULE_SAMPLER, "Rep-MQTT", LOG_TYPE_INFO, "ReCrt_Init:rc[%d]Url[%s]ID[%s]Name[%s]Psd[%s]", \
			rc, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord);
	}
	else
	{
		//ConnFailedTimes Over 5 Times, Not Log.
	}

	return;
}

static BOOL Wait_Mqtt_CommReady(REPORTER* pReporter)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
	int rc = 0;
	static int s_nErrorCounter = 0;	
	//TRACEX("-------emMqttCommStatus=[%d]-------------\n", pMqtt->emMqttCommStatus);
	//handle the connection to mqtt broker
	if(pMqtt->emMqttCommStatus == MQTT_COMM_DISCONECTED)
    {	
		//disconnect old connection if have
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt, 2000);
		}
		
		pMqtt->bIsMqttConn_Ready = FALSE;
 
		//to avoid to reconnect too frequenty (max 15 * 30 ~= 7.5min)
		int RECONN_WAIT_MAX_TIMEOUT_SEC =  MQTT_CONNET_TIMEOUT;
		RECONN_WAIT_MAX_TIMEOUT_SEC = MAX(10, RECONN_WAIT_MAX_TIMEOUT_SEC);
		//added by Jimmy to avoid conn-fail too frequently
		RECONN_WAIT_MAX_TIMEOUT_SEC = MAX(RECONN_WAIT_MAX_TIMEOUT_SEC, pMqtt->nConnFailedTimes * 10);
		if(ABS(_NOW_ - pMqtt->tmLastSend_Conn) >= RECONN_WAIT_MAX_TIMEOUT_SEC)
		{
			pMqtt->tmLastSend_Conn = _NOW_;
			ReCreate_MqttClient_AndInit(pMqtt);
			Connect_2MqttServer(pMqtt);	
		}
		else
		{
			TRACEX("---------Now disconnt, ERRRRRRRRR222222222-------\n");
		}		
		//needs to return here!!!
		return FALSE;
    }
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_CONNECTED)
	{
		//init
		s_nErrorCounter = 0;	
		//now first thing is to subscribe
		if(Subscrible_AllTopics(pMqtt) == FALSE)
		{
			s_nErrorCounter++;
		}
	}
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_SUBSCRIBING)
	{
		s_nErrorCounter++; //20230320,Repeat Subscribe Will Cause Reboot.
		//if(ABS(_NOW_ - pMqtt->tmLast_CallSubscribe) > 10)
		//{
		//	Subscrible_AllTopics(pMqtt);
		//	s_nErrorCounter++;
		//}	
	}
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_READY)
	{
		//TRACEX("-------THE MQTT-Client is READY!!!!--------\n");
		pMqtt->bIsMqttConn_Ready = TRUE;
 		return TRUE;
	}
	else if(pMqtt->emMqttCommStatus == MQTT_COMM_WAIT_REBUILD_CLIENT)
	{
		pMqtt->nConnFailedTimes = 0;
		ReCreate_MqttClient_AndInit(pMqtt);
		pMqtt->emMqttCommStatus = MQTT_COMM_DISCONECTED;
	}
	
	if(s_nErrorCounter >= 1)
	{
		//disconn the comm forcely
		TRACEX("----Now needs to disconnecte MQTT-conn so as to reconnect,s_nErrorCounter=[%d]",s_nErrorCounter);
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt, 2000);
		}
		pMqtt->emMqttCommStatus = MQTT_COMM_DISCONECTED;
		pMqtt->emMqtt_Disconn_Reason = MQTT_FAIL_REASON_SCRIBED_TIMEOUT;
		LOG_MQTT(pMqtt, rc);	
	}

	return FALSE;
}


static DWORD _ThreadEntry_MqttComm(void* pArg)
{
	TRACE("-----------_ThreadEntry_MqttComm: Start Enter------- \n");
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
 	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;

	ReCreate_MqttClient_AndInit(pMqtt);

	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();

		Handle_CommStatus(pMqtt);

		if(!Wait_Mqtt_CommReady(pReporter))
		{
			Sleep(500);
			continue;
		}

		//--- Recv Mqtt Message From Server---//
		if(pMqtt->bIs_MqttConnOK == TRUE)
		{	
			//--- Pub Mqtt Message To Server ---//
			//Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);	
			Pub_MsgToServer(pMqtt);
			//Mutex_Unlock(pMqtt->hMutex);	

			//Mutex_Lock(pMqtt->hMutex, WAIT_INFINITE);	
			Receive_Resp_Messge(pMqtt, 100);
			//Mutex_Unlock(pMqtt->hMutex);	
		}

		//TRACEX("-----------Pub_MsgToServer rc=[%d]--- \n", rc);
		Sleep(100);
	}

	MQTTClient_destroy(&pMqtt->client);
	SAFELY_DELETE(pMqtt);  

	TRACEX("-----------_ThreadEntry_MqttComm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}


static BOOL Init_GetSysAlarmDataInfo(MQTT_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
 	ERRCODE_DAI err;
	BOOL bRet = TRUE;
	UINT n = 0;
	//----- now get alarm info -----//
 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_TOTAL_R, 0, 0, &pMqtt->nAlarmInfosTotal, 4);
	TRACE("Info:nTotalCfgvAlmInfo = %d \n", pMqtt->nAlarmInfosTotal);
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get Cfg Alarm Total, err[%d]!", err);
		return FALSE; //do not reply
	}

	if(pMqtt->nAlarmInfosTotal > 65535)
	{
		TRACE("Error: Get AlarmInfoTotla=[%d] Out Of Range!", pMqtt->nAlarmInfosTotal);
		return FALSE;
	}

	TRACE("Info:---Now get alarm total->%d---\n", pMqtt->nAlarmInfosTotal);
    pMqtt->pAllAlarmInfo = NEW(ALARM_INFO, pMqtt->nAlarmInfosTotal);
    if( pMqtt->pAllAlarmInfo == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }

	ALARM_INFO alarmInfo;
	for(n = 0; n < pMqtt->nAlarmInfosTotal; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_INFO_R,
			n, 0, &alarmInfo, sizeof(ALARM_INFO));
		if(err != ERRCODE_DAI_OK)
		{
			TRACE("Error:Get Alarm info, Idx[%d], err[%d]!\n", n, err);
			return FALSE; 
		}

		memcpy(&pMqtt->pAllAlarmInfo[n], &alarmInfo, sizeof(ALARM_INFO));	

		//TRACE("Info:---Now get the alarm[%d, %s, %d]---\n",\
		//	pMqtt->pAllAlarmInfo[n].nAlarmId,\
		//	pMqtt->pAllAlarmInfo[n].pRes_Name->szName,\
		//	pMqtt->pAllAlarmInfo[n].emLevel);
	}
	
	return bRet;
}

static BOOL Init_MqttConfigParam(MQTT_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	PARAM_VAL_GET valGet;

	pMqtt->emMqttCommStatus = MQTT_COMM_DISCONECTED;
	pMqtt->nCurrDataFrom = 0;
	pMqtt->bNeedRefreshActvAlm = FALSE;
	pMqtt->nConnFailedTimes = 0;

	//---Get Cloud Protocal Type:wss/mqtt-xjn/mqtt-selfTopic---//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 10, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emRepInstType = (CLOUDREP_INST_TYPE)valGet.val.emValue;

	//---Get Mqtt URL + ClientID + UserName + PassWord---//
	//--- SSL:// Or TCP://,WS://,WSS:// ---//
	valGet.nGetSize = 255;
 	valGet.val.blobValue = (BYTE*)pMqtt->szServerUrl;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 25, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACE("ERROR: server URL can not be retrieved\n");
		return FALSE;
	}

 	valGet.nGetSize = 64;
	valGet.val.blobValue = (BYTE*)pMqtt->szClientID;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 2, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: MQTT ClientID can NOT be retrieved\n");
		return FALSE;
	}

	if(pMqtt->emRepInstType == REP_TYPE_MQTT_SELFTOPIC)
	{	
		valGet.nGetSize = 32;
		// char szSelfTopic[32];
		valGet.val.blobValue = (BYTE*)pMqtt->szSelfTopic;
		ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 35, &valGet, sizeof(PARAM_VAL_GET));
		if (err != ERRCODE_DAI_OK)
		{
			TRACE("ERROR: Self Publish Topic ERROR!\n");
			return FALSE;
		}

		TRACEX("szSelfTopic: %s\n", pMqtt->szSelfTopic);

		if (pMqtt->szSelfTopic == NULL || pMqtt->szSelfTopic == '\0') 
		{	
			strcpy(pMqtt->szSelfTopic, "ess");			
		}
		snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/up", pMqtt->szSelfTopic, pMqtt->szClientID);
		snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/down/#", pMqtt->szSelfTopic, pMqtt->szClientID);

		TRACEX("Publish Topic: %s\n", pMqtt->szPubTopic);
		TRACEX("Subscribe Topic: %s\n", pMqtt->szSubTopic);

		// //- pub:001/ess/v1/075512345/up/102
		// //- sub:001/ess/v1/075512345/down/#
		// snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/ess/v1/%s/up", 		pMqtt->szClientID);
		// snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/ess/v1/%s/down/#", 	pMqtt->szClientID);
	}
	else
	{
		snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "pub/%s", pMqtt->szClientID);
		snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "sub/%s", pMqtt->szClientID);
	}

	valGet.nGetSize = 32;
	valGet.val.blobValue = (BYTE*)pMqtt->szUserName;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 32, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Username can NOT be retrieved\n");
		return FALSE;
	}

	valGet.nGetSize = 32;
	valGet.val.blobValue = (BYTE*)pMqtt->szPassWord;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 33, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Username can NOT be retrieved\n");
		return FALSE;
	}

 	//-------------Get Backend Config-------------// 
 	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pMqtt->blobBackendSet;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}
	//-----------------------End----------------------//

    //--------------Get Config BMS Num----------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			500, 502, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return FALSE;
	}
	pMqtt->nCfgBMSNum = valGet.val.ulValue;
	if(pMqtt->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pMqtt->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}

	//------------ Get Zlib Compress Config ------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 11, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->bZlibCompress = (valGet.val.emValue == 0) ? FALSE : TRUE;
	
	//-------------- Get TLS Auth Level ---------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 30, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emAuthLevel = (TLS_AUTH_LEVEL)valGet.val.emValue;
	
	//---------------Apply Area ------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 34, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emApplyArea = valGet.val.emValue;
	
	//---------------MQTT Version ------------------//
 	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 36, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emMQTTVersion = (MQTT_VERSION)valGet.val.emValue;

	return TRUE;
}

typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("---------------Initialize MQTT Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;

	//Create data
	pReporter->pDataHeap = NEW(MQTT_DATA_ROUGH, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for MQTT dataStructs!\n");
		return FALSE;
	}

	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pMqtt, 1);

	pMqtt->hReporter = pReporterThis;

	Init_MqttMsgInfo(pMqtt); 
	
	BOOL bRet = Init_MqttConfigParam(pMqtt);
	if(bRet == FALSE)
	{
		return FALSE;
	}
	
 	if(!Init_GetSysAlarmDataInfo(pMqtt))
	{
		TRACEX("ERROR: Get Sys Alarm DataInfo Failed!\n");
		return FALSE;
	}
	pMqtt->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd
	//--------------------------------End-------------------------//

	pMqtt->hMutex = Mutex_Create(TRUE);
	pMqtt->hqMsg2Sent = Queue_Create(MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(MQTT_CALL_MSG), 0);
 	if((pMqtt->hMutex == NULL) || (pMqtt->hqMsg2Sent == NULL))
	{
		TRACE("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("ES-Mqtt",
		(RUN_THREAD_START_PROC)_ThreadEntry_MqttComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create MqttComm thread Failed \n");
		return FALSE;
	}
	
 	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
 	};

	int n = 0;
	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("Mqtt_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
	
	return TRUE;
}
  


