// #define _TRACE_PRINT_ // cmd
#include "../reporters_base.h"
#include "es_mqtt.h"
#include <signal.h>

//#define ADDRESS     					"ssl://emqx.lnhlcloud.com:2883"
//#define CLIENTID    					"dgnlgx_1"
//#define PUB_TOPIC   					"pub/dgnlgx_1"
//#define SUB_TOPIC   					"sub/dgnlgx_1"

volatile MQTTAsync_token        Callback_Deliveredtoken;
extern void Init_GH_MqttMsgInfo(MQTT_DATA_ROUGH *pMqtt);
extern void Pub_GH_MsgToServer(MQTT_DATA_ROUGH *pMqtt);
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
			TRACEE("EVENT_TYPE_ALARM  alarm_id: [%d]",pAlmEv->nAlarmId);
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
			TRACEX("MQTT_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
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
						pMqtt->emUserCommStatus 		= USER_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason 	= MQTT_FAIL_REASON_SERVERADDR_CHANGED;
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
					if(strcmp(szSelfTopic, pMqtt->szSelfTopic)!= 0 && strcmp(szSelfTopic, "custom")!= 0)
					{
						snprintf(pMqtt->szSelfTopic, 32, "%s", szSelfTopic);
						snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/up", pMqtt->szSelfTopic, pMqtt->szClientID);
						snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/down/#", pMqtt->szSelfTopic, pMqtt->szClientID);
						pMqtt->emUserCommStatus 		= USER_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason 	= MQTT_FAIL_REASON_SERVERADDR_CHANGED;
						TRACE("Update szSelfTopic: [%s]\n", szSelfTopic);
					}
					if(pMqtt->emCustomType == GH_CUSTOM)
					{
						snprintf(pMqtt->szSelfTopic, 32, "%s", szSelfTopic);
						//printf("Update szSelfTopic: [%s]\n", szSelfTopic);						
					}	
				}
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 38))
			{
				TRACE("nId=[%d] nSubId=[%d]\n", pParamChg->parmPTR.nId ,pParamChg->parmPTR.nSubId);
				char szManualPubTopic[128]; 
				valGet.nGetSize = 128;
				valGet.val.blobValue = (BYTE*)szManualPubTopic;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 38, &valGet, sizeof(PARAM_VAL_GET));

				char szSelfTopic[32]; 
				valGet.nGetSize = 32;
				valGet.val.blobValue = (BYTE*)szSelfTopic;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 35, &valGet, sizeof(PARAM_VAL_GET));
				if (err == ERRCODE_DAI_OK)
				{
					if(strcmp(szManualPubTopic, pMqtt->szManualPubTopic)!= 0&& strcmp(szSelfTopic, "custom")== 0)
					{	
						snprintf(pMqtt->szManualPubTopic, 128, "%s", szManualPubTopic);
						snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "%s", pMqtt->szManualPubTopic);
						pMqtt->emUserCommStatus 		= USER_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason 	= MQTT_FAIL_REASON_SERVERADDR_CHANGED;
					}	
				}
				TRACEX("-----Publish Topic: %s\n-----", pMqtt->szPubTopic);
				TRACEX("-----Subscribe Topic: %s\n-----", pMqtt->szSubTopic);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 39))
			{
				TRACE("nId=[%d] nSubId=[%d]\n", pParamChg->parmPTR.nId ,pParamChg->parmPTR.nSubId);
				char szManualSubTopic[128]; 
				valGet.nGetSize = 128;
				valGet.val.blobValue = (BYTE*)szManualSubTopic;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 39, &valGet, sizeof(PARAM_VAL_GET));

				char szSelfTopic[32]; 
				valGet.nGetSize = 32;
				valGet.val.blobValue = (BYTE*)szSelfTopic;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 35, &valGet, sizeof(PARAM_VAL_GET));

				if (err == ERRCODE_DAI_OK)
				{
					if(strcmp(szManualSubTopic, pMqtt->szManualSubTopic)!= 0 && strcmp(szSelfTopic, "custom")== 0)
					{
						snprintf(pMqtt->szManualSubTopic, 128, "%s", szManualSubTopic);
						snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "%s", pMqtt->szManualSubTopic);
						pMqtt->emUserCommStatus 		= USER_COMM_WAIT_REBUILD_CLIENT;
						pMqtt->emMqtt_Disconn_Reason 	= MQTT_FAIL_REASON_SERVERADDR_CHANGED;
					}	
				}
				TRACEX("-----Publish Topic: %s\n-----", pMqtt->szPubTopic);
				TRACEX("-----Subscribe Topic: %s\n-----", pMqtt->szSubTopic);
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
			else if ((pParamChg->parmPTR.nSubId == 83))
			{
				// Read Config 
				valGet.nGetSize = 512;
				valGet.val.blobValue = (BYTE*)pMqtt->szShieldAlarms;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 83, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: IFY settings can not be retrieved\n");
					return FALSE;
				}
				TRACEI("Config Changed Evevt [%s]\n",pMqtt->szShieldAlarms);
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 87))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 87, &valGet, sizeof(PARAM_VAL_GET));	
				pMqtt->emCustomType = valGet.val.ulValue; 					
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 88))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 88, &valGet, sizeof(PARAM_VAL_GET));	
				pMqtt->nRemoteControlTime = valGet.val.ulValue;			
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 90))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 90, &valGet, sizeof(PARAM_VAL_GET));	
				pMqtt->nSendTimeInterval = valGet.val.ulValue; 			
			}
			else if((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 89))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 89, &valGet, sizeof(PARAM_VAL_GET));	
				pMqtt->emControlMode = valGet.val.emValue;
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
					pMqtt->TimeCalibration = ((TIME_CALIBRATION)valGet.val.emValue == TIME_MQTT)? TRUE : FALSE;
				}	
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

time_t Parse_MqttTime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

int Parse_MqttTimeZone(char *pszOut)
{
	int timezone = Parse_ISO8601TimeZone(pszOut);
	return timezone;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Make MqttTimeString()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : ISO8601 Format  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
char* Make_MqttTimeString(MQTT_DATA_ROUGH* pMqtt, time_t tmInout, char* pszOut)
{	
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

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
		PARAM_VAL_GET valGet;
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 67, &valGet, sizeof(PARAM_VAL_GET));
		int timezone = valGet.val.fValue;
		if(timezone == 0)
		{
			snprintf(timeinstring, 32,"%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
				currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);
		}
		else if(timezone > 0)
		{
			snprintf(timeinstring, 32,"%4d-%02d-%02dT%02d:%02d:%02d+%02d:00",currenttime.tm_year, currenttime.tm_mon + 1, 
				currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec, timezone);
		}
		else if(timezone < 0)
		{
			snprintf(timeinstring, 32,"%4d-%02d-%02dT%02d:%02d:%02d-%02d:00",currenttime.tm_year, currenttime.tm_mon + 1, 
				currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec, -timezone);
		}
		
	}
	memcpy(pszOut, timeinstring, 32);
	return pszOut;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Make UniqID_ESiOCPP()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
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
static void Log_ConnectStatuCode(MQTT_DATA_ROUGH* pMqtt, char *pszFormat)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	char 	szDump[256] = {0};
	static 	USER_COMM_STATUS emLast_CommStatus = USER_COMM_DISCONECTED;

	time_t 		  tmLogTime = _NOW_;
	static time_t tmLast_Log;

	if(ABS(tmLogTime - tmLast_Log) < 3)
	{
		return;
	}
	tmLast_Log = tmLogTime;


	if(pMqtt->emUserCommStatus == USER_COMM_DISCONECTED)
	{
		snprintf(szDump, 256, "[%s]Last_STAT[%d]->STAT[%d]Reasn[%d]Url[%s]ClientID[%s]UsrName[%s]PassWd[%s][%s]", 
			"DisCnt", emLast_CommStatus, pMqtt->emUserCommStatus, pMqtt->emDisCnt_Reason, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_CLOSE_COMM, szDump);//edit by kuangyue 20250313
		LOG_INFO("ES_MQTT pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_CLOSE_COMM, szDump);
		
	}
	else if(pMqtt->emUserCommStatus == USER_COMM_CONNECTED)
	{
		snprintf(szDump, 256, "[%s]Last_STAT[%d]->STAT[%d]Reasn[%d]Url[%s]ClientID[%s]UsrName[%s]PassWd[%s][%s]", 
			"Concted", emLast_CommStatus, pMqtt->emUserCommStatus, pMqtt->emDisCnt_Reason, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250313
		LOG_INFO("ES_MQTT pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
		else if(pMqtt->emUserCommStatus == USER_COMM_SUBSCRIBING)
	{
		snprintf(szDump, 256, "[%s]Last_STAT[%d]->STAT[%d]Reasn[%d]Url[%s]ClientID[%s]UsrName[%s]PassWd[%s][%s]", 
			"Subscribe", emLast_CommStatus, pMqtt->emUserCommStatus, pMqtt->emDisCnt_Reason, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, szDump);//edit by kuangyue 20250313
		LOG_INFO("ES_MQTT pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_WRITE, szDump);
	}
		else if(pMqtt->emUserCommStatus == USER_COMM_READY)
	{
		snprintf(szDump, 256, "[%s]Last_STAT[%d]->STAT[%d]Reasn[%d]Url[%s]ClientID[%s]UsrName[%s]PassWd[%s][%s]", 
			"Ready", emLast_CommStatus, pMqtt->emUserCommStatus, pMqtt->emDisCnt_Reason, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_INFO, szDump);//edit by kuangyue 20250313
		LOG_INFO("ES_MQTT pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_INFO, szDump);
	}
	else if(pMqtt->emUserCommStatus == USER_COMM_WAIT_REBUILD_CLIENT)
	{
		snprintf(szDump, 256, "[%s]Last_STAT[%d]->STAT[%d]Reasn[%d]Url[%s]ClientID[%s]UsrName[%s]PassWd[%s][%s]", 
			"Rebuild", emLast_CommStatus, pMqtt->emUserCommStatus, pMqtt->emDisCnt_Reason, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_OPEN_COMM, szDump);//edit by kuangyue 20250313
		LOG_INFO("ES_MQTT pReporter=%p, type=%d, Data='%s'", pReporter, DUMP_TYPE_OPEN_COMM, szDump);
	}
	 
	emLast_CommStatus = pMqtt->emUserCommStatus;
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
*  CREATED DATE  : 2021-11-23 11:13:53
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
*  CREATED DATE  : 2021-11-23 11:13:53
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
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecOkCall_onSubscribe(void* context, MQTTAsync_successData* response)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;
    if(pMqtt == NULL)
    {
        TRACE("onSubscribe: context is NULL\n");
        return;
    }
	TRACEX("Info: Topic Subscribe succeeded\n");
	pMqtt->bTopic_Subscribed 	= TRUE;
	pMqtt->emUserCommStatus 	= USER_COMM_READY;
	
	Log_ConnectStatuCode(pMqtt, "CallSubOk");

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecFailCall_onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;
    if(pMqtt == NULL)
    {
        TRACEX("onSubscribeFailure: context is NULL\n");
    }
	TRACEX("Subscribe failed, rc %d\n", response ? response->code : 0);

	pMqtt->emUserCommStatus 		= USER_COMM_SUBSCRIBING;
	pMqtt->emDisCnt_Reason 			= STATCODE_COMM_FAIL_REASON_SCRIBED_TIMEOUT;

	Log_ConnectStatuCode(pMqtt, "CallSubFail");
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
/*void Callback_Delivered(void *context, MQTTAsync_token  dt) 
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;
    if(pMqtt == NULL)
    {
            TRACEX("Callback_Delivered: pMqtt is NULL\n"); 
            return;  
    }

    TRACEX("Message with token value %d delivery confirmed. clientid=[%s] pubtopic=[%s] szSubTopic=[%s]\n", \
            dt ,pMqtt->szClientID, pMqtt->szPubTopic, pMqtt->szSubTopic);
    Callback_Deliveredtoken = dt;
    
    return;
}*/


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  FindMsg ByName()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
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
			TRACEX("nMsgId :[%d] \t pMqtt->stMsgInfoList[n].nMsgId : [%d]  nMsgType=[%d] \n", nMsgId, pMqtt->stMsgInfoList[n].nMsgId, pMqtt->stMsgInfoList[n].emMsgType);
			return pMqtt->stMsgInfoList[n].emMsgType;
		}
	}

	return Msg_Unkown;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  (SafeFree_ArrivedMsg)
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/    

//Jimmy note: !!!!!do NOT use 'free' but need use 'MQTTAsync_free' because the lib re-defined the HEAP
#define MQTT_SAFE_DELETE(p)	\
	do { if ((p) != NULL) { MQTTAsync_free(p); (p) = NULL; } } while(0)
static int SafeFree_ArrivedMsg(char *topicName, MQTTAsync_message *message)
{
	//Jimmy note: should return > 0 so as to resolve the double free problem
	if(message != NULL)
	{
		MQTTProperties_free(&message->properties);
		MQTT_SAFE_DELETE(message->payload);
		MQTT_SAFE_DELETE(message);	
	}	
	MQTT_SAFE_DELETE(topicName);
	//TRACEX("-------Now free the ASYNCCCCCCCC messages!!!!!!!!------\n");
	// MQTTAsync_freeMessage(&message); 
	// MQTTAsync_free(topicName);
	return 1;
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
static void Handle_MqttSyncTime(MQTT_DATA_ROUGH *pMqtt, time_t tm, int timezone)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	static time_t	tm_Last;
	VAR_VALUE		varV;
	VAR_VALUE		varV2;

	if(ABS(tm - _NOW_) > 5) //sync RTC
	{
		TRACE("tm=[%ld] _NOW_=[%ld]\n", tm, _NOW_);
	
		varV.dtValue = tm;
		varV2.fValue = timezone;
		if(ABS(_NOW_ - tm_Last) > 10) // Avoid Sync Time Too Frequently
		{
			tm_Last = _NOW_;
			if ((pMqtt->lcCfg.lcType == LC_TYPE_1st) && pMqtt->TimeCalibration)
			{
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
			}

			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 67, &varV2, 4, 0);
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
static void Disconnect_MqttClientComm(MQTT_DATA_ROUGH *pMqtt)
{       
    TRACEX("--- Disconnect_MqttClientComm ---\n");
    int rc = MQTTASYNC_SUCCESS;
 	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

	if(pMqtt->client == NULL)
	{
		TRACE("Error:Disconnect_MqttClientComm:pMqtt->client == NULL\n");
		LogOut(MAIN_MODULE_REPORTER, "MQTT", LOG_TYPE_WARING,
			"Disconnect_MqttClientComm: pMqtt->client == NULL");
	}
	
	// unsub all old subscribtions before new-sub
	//??if Subscribe Not ok,need unsubscribe?
	if (pMqtt->emUserCommStatus == USER_COMM_READY)
	{
		rc = MQTTAsync_unsubscribe(pMqtt->client, pMqtt->szSubTopic, &pMqtt->conn_opts);
	}
	if(pMqtt->emCustomType == GH_CUSTOM)
	{
		int n = 0;
		for(n = 0; n < GH_SUBTOPIC_NUM; ++n)
		{	
			char  szSubTopicBuffer[MQTT_TOPIC_SIZE];
			snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%s", pMqtt->szSelfTopic, pMqtt->stGHSetCmdIndoList[n].szSubTopic);
			rc = MQTTAsync_unsubscribe(pMqtt->client, szSubTopicBuffer, &pMqtt->conn_opts);
		}
	}
	
	Sleep(1000);

	pMqtt->tmLastSend_DisConn 	= _NOW_;  
	pMqtt->emUserCommStatus 	= USER_COMM_DISCONECTED;

    if ((rc = MQTTAsync_disconnect(pMqtt->client, &opts)) != MQTTASYNC_SUCCESS)
    {	
 		Log_ConnectStatuCode(pMqtt, "DisCnctFail");
		//return;
	}
	
	pMqtt->bIs_MqttConnOK = FALSE;//means already disconn
	return;
}

static void onConnLost_NotRecvR101Cmd(MQTT_DATA_ROUGH *pMqtt)
{
	time_t tmNow = _NOW_;
	if(ABS(_NOW_ - pMqtt->stMsgInfoList[C_101_StatusNotif].tmLastArrived) < 60*5)
	{ 
		return;
	}
	if (!MQTTAsync_isConnected(pMqtt->client)) 
	{
		return;
	}
	if(pMqtt->emCustomType == GH_CUSTOM)
	{
		return; //GH_CUSTOM Protocol, No need to check R101
	}

	Disconnect_MqttClientComm(pMqtt ); 

	pMqtt->bIs_MqttConnOK 	= FALSE;
	pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
	pMqtt->emDisCnt_Reason 	= STATCODE_COMM_REPLYTIMEOUT;

 	Log_ConnectStatuCode(pMqtt,"R101_Timeout");

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/    
void Callback_MsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

    MSG_CALL_TYPE   emCallType = 0;
	MQTT_MSG_TYPE   msgType = Msg_Unkown;
	
	static time_t tm_Last = 0;

	if((message == NULL) || (context == NULL))
	{
		TRACEX("__________RRRRRRRRRRRRRRRRRR_________\n");
		onConnLost_NotRecvR101Cmd(pMqtt);
		return SafeFree_ArrivedMsg(topicName, message);
	}

	//filter those invalid topic/messages
	if((message->payloadlen < 0) || (topicName == NULL)/* || (topicLen < 3)*/) //topicLen maybe 0 as of the remaing topics in queue
	{
		TRACEX("INVALIDE topic ->[%s, %d], message->payloadlen->[%d]\n", (topicName == NULL) ? "NULL" : topicName, 
			topicLen, message->payloadlen);
		onConnLost_NotRecvR101Cmd(pMqtt);
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
		if(strstr(topicName, "down") < 0) //if Recv Msg is Not Sub Topic,Ignore
		{
			//!!! 2021-11-30 John.Dont Know Why Recv Abnormal Topic Data,Need Process Later
			TRACEX("Error: Wrong Topic, Just Ignore!!!\n");
			onConnLost_NotRecvR101Cmd(pMqtt);
        	return SafeFree_ArrivedMsg(topicName, message);
		}
	}

	if((pMqtt->emUserCommStatus 	== USER_COMM_DISCONECTED) 	||
		(pMqtt->emUserCommStatus 	== USER_COMM_CONNECTED) 	||
		(pMqtt->emUserCommStatus 	== USER_COMM_SUBSCRIBING) 	||
		(pMqtt->emUserCommStatus 	== USER_COMM_WAIT_REBUILD_CLIENT))
	{
		TRACEX("----MQTT User-Client is NOT ready to Recv ANY topic, emUserCommStatus = [%d]!!!----\n", pMqtt->emUserCommStatus);
		return SafeFree_ArrivedMsg(topicName, message);
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
	// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_READ, pMqtt->szReadBuffer);//edit by kuangyue 20250315
	// LOG_NOTICE("ES_MQTT Callback_MsgArrvd pReporter=%p, type=%d, Data='%s'",
	// 	pReporter,
	// 	DUMP_TYPE_READ,
	// 	pMqtt->szReadBuffer);	

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

		if(pMqtt->emCustomType == GH_CUSTOM)
		{
			int n = 0;
			for(n = 0; n < GH_SUBTOPIC_NUM; ++n)
			{
				char  szSubTopicBuffer[MQTT_TOPIC_SIZE];
				snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%s", pMqtt->szSelfTopic, pMqtt->stGHSetCmdIndoList[n].szSubTopic);
				if(strstr(topicName, szSubTopicBuffer) != NULL)
				{
					if(pMqtt->stGHSetCmdIndoList[n].pfnMsgHandler((void*)pMqtt, json) == -1)
					{
						//printf("GH ParseMessage [%s]\n", cJSON_GetErrorPtr());
						cJSON_Delete(json);
						goto exit;
					}
					break;
				}
			}
			cJSON_Delete(json);     //!!! do not forget to destroy the json
			//now continue parsing for another frame
			pMqtt->nReadBuffLen -= (pszEnd - pszPos);

			pszPos = pszEnd;
			continue; //GH 不需要解析其他数据，直接结束当前循环
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
							int timezone = Parse_MqttTimeZone(temp1->valuestring);
							
							Handle_MqttSyncTime(pMqtt, tm, timezone);
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
		//
		//	MQTTProperties_free(&(*message)->properties);
		//	free((*message)->payload);
		//	free(*message);
		TRACE("--- ThreadId= 0x%08x ---\n", RunThread_GetId(NULL));
 

		//MQTTAsync_freeMessage(&message); 
	 
		//if(message->properties == NULL)
		//{
		//	printf("Error: message->properties == NULL!!!\n");
		//}
	return SafeFree_ArrivedMsg(topicName, message);
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Callback_ConnLost()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : when Comm Lost will Call This Function.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Callback_ConnLost(void *context, char *cause)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;
    
    printf("\n ------ Warning:Connection lost cause: %s ------\n", cause);
 
    pMqtt->bIs_MqttConnOK 	= FALSE;
	pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
	pMqtt->emDisCnt_Reason	= STATCODE_COMM_FAIL_SERVER_LOSTCONN;
	
	Log_ConnectStatuCode(pMqtt, "CallConnLast");
    
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT onDisconnect()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
/*void ExecOkCall_onDisconnect(void* context, MQTTAsync_successData* response)
{       
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)context;

    TRACEX("Warning:Disconnected Success. client resp=[%s]\n",response ? response->alt.connect.serverURI : 0);
      
    pMqtt->bIs_MqttConnOK = FALSE;
	pMqtt->emUserCommStatus = STATCODE_COMM_DISCONECT;
	
	Log_ConnectStatuCode(pMqtt);
    return;
}*/

/*********************************************************************************
*  
*  FUNCTION NAME : Manual Disconnect MQTT Communication()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
/*BOOL Handle_DisconnectComm(MQTT_DATA_ROUGH *pMqtt)
{       
    TRACEX("---Handle_DisconnectComm---\n");
    int rc = MQTTASYNC_SUCCESS;
    MQTTAsync client = pMqtt->client;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
       
    opts.onSuccess = ExecOkCall_onDisconnect;
    opts.context = client;
    if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
    {
            TRACEX("Failed to Disconnect, return code %d\n", rc);
            return FALSE;
    }

    return TRUE;
}*/

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecOkCall_onSend(void* context, MQTTAsync_successData* response)
{
    //MQTTAsync client = (MQTTAsync)context;
    //MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    //int rc;
    TRACEX("Publish: Message with token value %d delivery confirmed\n", response->token);
    
    /*opts.onSuccess = onDisconnect;
    opts.context = client;
    if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
    {
            TRACEX("Failed to start sendMessage, return code %d\n", rc);
            exit(EXIT_FAILURE);
    }
    */
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecFailCall_onSend(void* context, MQTTAsync_failureData* response)
{
    if(context == NULL)
    {
        TRACEX("ExecFailCall_onSend: context is NULL\n");
    }
	TRACEX("Send Publish Msg failed, rc %d\n", response ? response->code : 0);
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : When Publish to Server failed, will Callback this Function.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecFailCall_onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    MQTT_DATA_ROUGH *pMqtt 	= (MQTT_DATA_ROUGH*)context;
    pMqtt->bIs_MqttConnOK 	= FALSE;
	pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
	pMqtt->emDisCnt_Reason	= STATCODE_COMM_CONECTFAILURE;

	Log_ConnectStatuCode(pMqtt, "CallCnctFail");
    TRACEX("ExecFailCall_onConnectFailure:Connect failed, rc %d\n", response ? response->code : 0);
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  init SubscribleMsg()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 连接成功，则调用该函数接口
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int init_SubscribleMsg(MQTT_DATA_ROUGH *pMqtt)
{               
    int rc = 0;
 	MQTTAsync client 				= pMqtt->client;  
    MQTTAsync_responseOptions opts 	= MQTTAsync_responseOptions_initializer;
    
    TRACEX("Subscribing to topic %s\nfor client %s using QoS%d\n\n", \
            pMqtt->szSubTopic, pMqtt->szClientID, QOS);

	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		opts.onSuccess5 			= ExecOkCall_onSubscribe;
		opts.onFailure5 			= ExecFailCall_onSubscribeFailure;
	}
	else
	{
		opts.onSuccess 				= ExecOkCall_onSubscribe;
		opts.onFailure 				= ExecFailCall_onSubscribeFailure;
	}
	opts.context 			= pMqtt;
	Callback_Deliveredtoken = 0;

    if ((rc = MQTTAsync_subscribe(client,  pMqtt->szSubTopic/*SUB_TOPIC*/, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
		TRACE("Failed to start subscribe, return code %d\n", rc);
		rc = -1;
	}
	if(pMqtt->emCustomType == GH_CUSTOM)
	{
		int n = 0;
		for(n = 0; n < GH_SUBTOPIC_NUM; n++)
		{	
			char  szSubTopicBuffer[MQTT_TOPIC_SIZE];
			snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%s", pMqtt->szSelfTopic, pMqtt->stGHSetCmdIndoList[n].szSubTopic);
			if ((rc = MQTTAsync_subscribe(client,  szSubTopicBuffer, QOS, &opts)) != MQTTASYNC_SUCCESS)
			{
				//printf("id = %d, topic = %s\n", n, szSubTopicBuffer);
				return -1;
			}
		}
	}

    TRACEX("Good rc from subscribe rc=[%d] MQTTASYNC_SUCCESS\n", rc);
    return rc;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  ExecOkCall onConnect()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 连接成功，则调用该函数接口
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExecOkCall_onConnect(void* context, MQTTAsync_successData* response)
{
    MQTT_DATA_ROUGH *pMqtt 	= (MQTT_DATA_ROUGH*)context;
    //MQTTAsync client 		= pMqtt->client;

    TRACEX("---Successful connection---\n");
    pMqtt->bIs_MqttConnOK 	= TRUE;

	pMqtt->emUserCommStatus = USER_COMM_CONNECTED;
	pMqtt->emDisCnt_Reason 	= STATCODE_COMM_OK;

	pMqtt->stMsgInfoList[C_101_StatusNotif].tmLastArrived 		= _NOW_;
	pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend = _NOW_;
	pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend 		= _NOW_;
	pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend 	= _NOW_;
	pMqtt->stMsgInfoList[C_206_FeeRateKwh].tm_LastSend 			= _NOW_;

	Log_ConnectStatuCode(pMqtt, "CallOnCnct");

    TRACEX("---Start Init SubscribleMsg---\n");
    init_SubscribleMsg(pMqtt);  

    //---Only Send One Times when First Time Register---//
    pMqtt->stMsgInfoList[C_100_BootNotif].pfnMsgHandler((void*)pMqtt);
	Publish_MqttMessage(pMqtt);

    return;
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
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : MQTT Main() Entrance Function
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Publish_MqttMessage(MQTT_DATA_ROUGH *pMqtt)
{       
    int rc = 0;
	REPORTER* pReporter 			= (REPORTER*)pMqtt->hReporter;
    MQTTAsync client 				= pMqtt->client;
    MQTTAsync_responseOptions opts 	= MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg 		= MQTTAsync_message_initializer;

	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		opts.onSuccess5 	= ExecOkCall_onSend;
    	opts.onFailure5 	= ExecFailCall_onSend;
	}
	else
	{
    	opts.onSuccess 	= ExecOkCall_onSend;
    	opts.onFailure 	= ExecFailCall_onSend;
	}
    opts.context 	= client;

	if(pMqtt->bZlibCompress == TRUE)
	{
  		pubmsg.payload 		= pMqtt->szCompressSendBuff; 
		pubmsg.payloadlen 	= pMqtt->nCompressSendBuffLen;  
	}
	else
	{
		pubmsg.payload 		= pMqtt->szSendBuffer;           
   		pubmsg.payloadlen 	= pMqtt->nSendBuffLen; 
	}

    pubmsg.qos 				= QOS;
    pubmsg.retained 		= 0;
    Callback_Deliveredtoken = 0;
	static int s_nErrCounter= 0;

    if ((rc = MQTTAsync_sendMessage(client, pMqtt->szPubTopic/*PUB_TOPIC*/, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        TRACEX("Failed to start sendMessage, return code %d\n", rc);
        rc = -1;
        //Need ReConnect
        s_nErrCounter++;
		if(s_nErrCounter >= 5)
		{
			s_nErrCounter = 5;
			pMqtt->bIs_MqttConnOK 	= FALSE;
			pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
			pMqtt->emDisCnt_Reason	= STATCODE_COMM_CONECTFAILURE;			
		}
    }
	else
	{
		s_nErrCounter = 0;
	}

    TRACE("mqtt MainRun=[%d] pubmsg.payload=[%d] pubmsg.payload=[%s]\n", \
        rc, pubmsg.payloadlen, pubmsg.payload);

	// if(s_nErrCounter < 5)
	// {
	// 	// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pMqtt->szSendBuffer);//edit by kuangyue 20250313
	// 	LOG_INFO("ES_MQTT Publish_MqttMessage pReporter=%p, type=%d, Data='%s'",
	// 		pReporter,
	// 		DUMP_TYPE_WRITE,
	// 		pMqtt->szSendBuffer);	
	// }

    return rc;

}	


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT QueryActvAlarmInfo()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void QueryActvAlarmInfo(MQTT_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	COMMPORT* pComm 	= pReporter->pUsedCommPort;
	ERRCODE_DAI 		errCode;
	UINT 			  	n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize 		= sizeof(EVENT_ALARM);
	cond.pCondArg 		= &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount 	= MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pMqtt->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pMqtt->activeAlms , &nQueryCount);
	TRACE("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);
	TRACEI("-----Qt query Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

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
			TRACEI("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pMqtt->activeAlms[n].nDevId, pMqtt->activeAlms[n].nAlarmId);
		}
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Check IfActvAlmStatuChanged()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
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

BOOL ShieldAlarms_StringToUINT(MQTT_DATA_ROUGH* pMqtt)
{
	TRACEW("allow to shield alarms [%s]",pMqtt->szShieldAlarms);
	UINT index = 0 ,ncount = 1;
	char szInput[512];

	ZERO_POBJS(pMqtt->ShieldAlarms, pMqtt->nAlarmInfosTotal);   //clear buff
    if (pMqtt->szShieldAlarms[0] == '\0' || strcmp(pMqtt->szShieldAlarms, "alm_id/alm_id/alm_id") == 0) 
	{
		TRACEE("NO alarm to be shield ...");
		pMqtt->nShieldAlarmsNum = 0;
        return FALSE;
    }

    for (index = 0; pMqtt->szShieldAlarms[index] != '\0'; index++) 
	{
        if (pMqtt->szShieldAlarms[index] == '/') 
		{
            ncount++;
        }
    }
	pMqtt->nShieldAlarmsNum = ncount;

	strncpy(szInput, pMqtt->szShieldAlarms, 512);
	szInput[511] = '\0'; 

	char *token = strtok(szInput, "/");
    index = 0;

    while (token != NULL && index < ncount) 
	{
        pMqtt->ShieldAlarms[index] = (UINT)strtoul(token, NULL, 10);
        token = strtok(NULL, "/");
		index++;
    }

	// printf("pMqtt->ShieldAlarms = ");
	// for (index=0;index < ncount;index++)
	// {
	// 	printf("[%d] ",pMqtt->ShieldAlarms[index]);
	// }
	// printf("\n");

    return TRUE;

}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT Main Run Func()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : MQTT Main() Entrance Function
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int mqtt_MainRun(MQTT_DATA_ROUGH *pMqtt, const MQTTAsync_connectOptions* options)
{       
    int 			rc = 0;
    time_t 			tmNow = time(NULL);
	PARAM_VAL_GET 	valGet;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	
	if(pMqtt->bTopic_Subscribed == FALSE)
	{
		init_SubscribleMsg(pMqtt);  
	}

	//TRACE("pMqtt->blobBackendSet.nCmd_101_Interval=[%d]\n", pMqtt->blobBackendSet.nCmd_101_Interval);

	if(pMqtt->blobBackendSet.nCmd_101_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_101_StatusNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_101_Interval)))
		{
			pMqtt->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_101_StatusNotif].tm_LastSend = tmNow;
		}
	}
	if(pMqtt->blobBackendSet.nCmd_102_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_102_BattCabStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_102_Interval)))
		{
			pMqtt->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_102_BattCabStaNotif].tm_LastSend = tmNow;
		}
	}
    
	if(pMqtt->blobBackendSet.nCmd_103_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_103_Interval)))
		{
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				//Prepare_SEMS_C103_ESBMSStatusNotif((void*)pMqtt);
				Prepare_SEMS_C103_ESBMSStatusNotif_Electra((void*)pMqtt);
			}
			else
			{
				pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pMqtt);
				Publish_MqttMessage(pMqtt);	
			}

			pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].tm_LastSend = tmNow;
		}
	}
	if(pMqtt->blobBackendSet.nCmd_104_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_104_Interval)))
		{
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				Prepare_SEMS_C104_PwrDistrStaNotif_Electra((void*)pMqtt);
			}
			else
			{
				pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].pfnMsgHandler((void*)pMqtt);
				Publish_MqttMessage(pMqtt);
			}

			pMqtt->stMsgInfoList[C_104_PwrDistrStaNotif].tm_LastSend = tmNow;
		}
	}

	if(pMqtt->blobBackendSet.nCmd_155_Interval != 0) //155优先，若155设置为0支持105原有控制155的逻辑
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_155_Interval)))
		{
			pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend = tmNow;
		}
	}
	else if(pMqtt->blobBackendSet.nCmd_105_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_105_Interval)))
		{
			pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].tm_LastSend = tmNow;
		}
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

	if(pMqtt->blobBackendSet.nCmd_157_Interval != 0) //157优先，若157设置为0支持107原有控制157的逻辑
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_157_Interval)))
		{
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend = tmNow;
		}
	}
	else if(pMqtt->blobBackendSet.nCmd_107_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_107_Interval)))
		{
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].tm_LastSend = tmNow;
		}
	}

	if(pMqtt->blobBackendSet.nCmd_108_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].tm_LastSend) > (MAX(10, pMqtt->blobBackendSet.nCmd_108_Interval)))
		{
			pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_108_SysVersionStaNotif].tm_LastSend = tmNow;
		}
	}
	if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
	{
		if(pMqtt->blobBackendSet.nCmd_109_Interval != 0)
		{
			send_SEMS_AlarmStatusChanged((void*)pMqtt);
			if((ABS(tmNow - pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_109_Interval))))
			{
				PrePare_SEMS_C109_SysActvAlmNotif((void*)pMqtt);
				pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend = tmNow;	
			}
		}
	}
	else
	{
		BOOL bActvAlmChanged = Check_IfActvAlmStatuChanged(pMqtt, pMqtt->nCurrDataFrom);
		if((bActvAlmChanged == TRUE) ||
		(ABS(tmNow - pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_109_Interval))))
		{
			ShieldAlarms_StringToUINT(pMqtt);
			pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend = tmNow;	
		}
	}

	//printf("111 pMqtt->blobBackendSet.nCmd_110_Interval=[%d]\n", pMqtt->blobBackendSet.nCmd_110_Interval);
	if(pMqtt->blobBackendSet.nCmd_110_Interval != 0) //0 Means Not send To Cloud Platform.
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_110_SingleBattryInfo].tm_LastSend) > (MAX(10, pMqtt->blobBackendSet.nCmd_110_Interval)))
		{
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				//Prepare_SEMS_C110_BatDataNotif((void*)pMqtt);
			}
			else
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
			}
			
			pMqtt->stMsgInfoList[C_110_SingleBattryInfo].tm_LastSend = tmNow;
		}
	}

	// 如果设置了时间间隔，则根据时间间隔发送
	if(pMqtt->blobBackendSet.nCmd_114_Interval != 0) 
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_114_PCSUnitInfo].tm_LastSend) >= (MAX(10, pMqtt->blobBackendSet.nCmd_114_Interval))) 
		{ 
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				Prepare_SEMS_C114_PCSUnitInfo((void*)pMqtt);
			}
			// else
			// {
			// 	pMqtt->stMsgInfoList[C_114_PCSUnitInfo].pfnMsgHandler((void*)pMqtt);
			// }

			pMqtt->stMsgInfoList[C_114_PCSUnitInfo].tm_LastSend = tmNow;
 
		} 
	} 
	// 如果没有设置时间间隔，则上电两分钟后发送一次
	else 
	{
		if(pMqtt->stMsgInfoList[C_114_PCSUnitInfo].tm_LastSend == 0 && ABS(tmNow - pMqtt->stMsgInfoList[C_100_BootNotif].tm_LastSend) >= 120) 
		{ 
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				Prepare_SEMS_C114_PCSUnitInfo((void*)pMqtt);
			}
			// else
			// {
			// 	pMqtt->stMsgInfoList[C_114_PCSUnitInfo].pfnMsgHandler((void*)pMqtt);
			// } 
			pMqtt->stMsgInfoList[C_114_PCSUnitInfo].tm_LastSend = tmNow; 
		}
	}

	if(pMqtt->blobBackendSet.nCmd_115_Interval != 0) 
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_115_SummaryInfo].tm_LastSend) >= (MAX(1, pMqtt->blobBackendSet.nCmd_115_Interval))) 
		{ 
			if(pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				Prepare_SEMS_C115_SummaryInfo((void*)pMqtt);

			}
			// else
			// {
			// 	pMqtt->stMsgInfoList[C_115_SummaryInfo].pfnMsgHandler((void*)pMqtt);
			// }
			pMqtt->stMsgInfoList[C_115_SummaryInfo].tm_LastSend = tmNow;

		} 
	}

	if(pMqtt->blobBackendSet.nCmd_200_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_200_RTPowerInfo].tm_LastSend) >= (MAX(1, pMqtt->blobBackendSet.nCmd_200_Interval)))
		{

			pMqtt->stMsgInfoList[C_200_RTPowerInfo].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);

			pMqtt->stMsgInfoList[C_200_RTPowerInfo].tm_LastSend = tmNow;
		}
	}	


	if(pMqtt->blobBackendSet.nCmd_202_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_202_Interval)))
		{
			if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
			{
				//Prepare_SEMS_C202_HourSectorStaticKwh((void*)pMqtt);
			}
			else
			{
				pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].pfnMsgHandler((void*)pMqtt);
				Publish_MqttMessage(pMqtt);
			}
			
			pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend = tmNow;	
		}
	}
    
	if(pMqtt->blobBackendSet.nCmd_204_Interval != 0)
	{
		if(ABS(tmNow - pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend) > (MAX(1, pMqtt->blobBackendSet.nCmd_204_Interval)))
    	{
        	pMqtt->stMsgInfoList[C_204_DailyStaticKwh].pfnMsgHandler((void*)pMqtt);
	    	Publish_MqttMessage(pMqtt);
        	pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend = tmNow;	
    	}
	}
	
	if(pMqtt->blobBackendSet.nCmd_206_Interval != 0)
	{
		if(pMqtt->blobBackendSet.nCmd_206_Interval > 0 && ABS(tmNow - pMqtt->stMsgInfoList[C_206_FeeRateKwh].tm_LastSend) >= MAX(1, pMqtt->blobBackendSet.nCmd_206_Interval))
		{
			pMqtt->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			pMqtt->stMsgInfoList[C_206_FeeRateKwh].tm_LastSend = tmNow;	
		}
	}
    //return rc;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Init_MqttMsgInfo()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
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

	INIT_MQTT_MSG(p,	C_114_PCSUnitInfo,				114,			MQTT_MSG_C2S,		Prepare_SEMS_C114_PCSUnitInfo); // CXC 260122
	INIT_MQTT_MSG(p,	C_115_SummaryInfo,				115,			MQTT_MSG_C2S,		Prepare_SEMS_C115_SummaryInfo); // CXC 260128
	
	INIT_MQTT_MSG(p,	S_111_Trigger,				    111,			MQTT_MSG_S2C,		Handle_S111_TriggerMessage);

	INIT_MQTT_MSG(p,	C_200_RTPowerInfo,				200,			MQTT_MSG_C2S,		Prepare_C200_RTPowerInfo); // Jeffery250922 Electra Real Time Power Allocation Algorithm

 	INIT_MQTT_MSG(p,	C_202_HourSectorStaticKwh,		202,			MQTT_MSG_C2S,		Prepare_C202_HourSectorStaticKwh);
	INIT_MQTT_MSG(p,	C_204_DailyStaticKwh,			204,			MQTT_MSG_C2S,		Prepare_C204_DailyStaticKwh);
	INIT_MQTT_MSG(p,	C_206_FeeRateKwh,				206,			MQTT_MSG_C2S,		Prepare_C206_SharpPeakFlatValleyKwh);
	
	INIT_MQTT_MSG(p,	S_401_RmtRest,				    401,			MQTT_MSG_S2C,		Handle_S401_Reset);
	INIT_MQTT_MSG(p,	S_402_RmtChgEVESEAbi,			402,			MQTT_MSG_S2C,		Handle_S402_ChangeEVSEAvailability);
	INIT_MQTT_MSG(p,	S_403_SOCCalibration,			403,			MQTT_MSG_S2C,		Handle_S403_RemoteSOCManualCalibration);

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
	
	pMqtt->stMsgInfoList[C_200_RTPowerInfo].tm_LastSend = _NOW_;
	pMqtt->stMsgInfoList[C_202_HourSectorStaticKwh].tm_LastSend = _NOW_;
	pMqtt->stMsgInfoList[C_204_DailyStaticKwh].tm_LastSend = _NOW_;
	pMqtt->stMsgInfoList[C_206_FeeRateKwh].tm_LastSend = _NOW_;

	//---Note:Sometimes Abnormal Reboot Cause /
	//System Not Avaliable send to CloudPlatfrom---/
	pMqtt->stMsgInfoList[C_109_SysActvAlmNotif].tm_LastSend = _NOW_;

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_CommStatus()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
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
		TRACEI("MQTT-server_commom_status [%d]",varV.emValue);
		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);

		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 30, &varV, 4, 100);
	}

	return;
}

void trace_callback(enum MQTTASYNC_TRACE_LEVELS level, char* message)
{
	fprintf(stderr, "Trace : %d, %s\n", level, message);
}

static BOOL Connect_2MqttServer(MQTT_DATA_ROUGH* pMqtt)
{
	int rc = 0;
	
	//ssl option
    MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
    sslopts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;

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
	    case TLS_AUTH_Mutual: // The URL of AWS broker has to use mqtts:// with port 8883!
	        // Mutual authentication
	        sslopts.protos = alpn_protos_mut;
	        sslopts.protos_len = sizeof(alpn_protos_mut);
	        sslopts.keyStore = "/app/config/client.pem"; // Client certificate
	        sslopts.privateKey = "/app/config/client.key"; // Client private key
	        // sslopts.privateKeyPassword = "your_key_password"; // Password for the private key
	        break;
	}

	pMqtt->conn_opts.ssl = &sslopts;
	pMqtt->conn_opts.ssl->verify = 1;
   	pMqtt->conn_opts.keepAliveInterval = 30;
	pMqtt->conn_opts.connectTimeout = MQTT_CONNET_TIMEOUT;
	pMqtt->conn_opts.username = pMqtt->szUserName; 
	pMqtt->conn_opts.password = pMqtt->szPassWord;

	//----John 20240516 Added For Debug----//
	// MQTTAsync_setTraceCallback(trace_callback);
	// MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
	//--------------End-------------------//

	if(pMqtt->emMQTTVersion == VERSION_5)
	{	
		pMqtt->conn_opts.cleanstart = 1; //this is for mqtt5 and for mqtt5 cleansession should be 0
		pMqtt->conn_opts.cleansession = 0;
		MQTTProperties props = MQTTProperties_initializer;
		MQTTProperty property;	
		property.identifier = MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
		property.value.integer4 = 0;
		MQTTProperties_add(&props, &property);
		pMqtt->conn_opts.connectProperties = &props;
	}
	else
	{
		pMqtt->conn_opts.cleanstart = 0; //this is for mqtt5 and for mqtt5 cleansession should be 0
		pMqtt->conn_opts.cleansession = 1;
	}

	if ((rc = MQTTAsync_connect(pMqtt->client, &pMqtt->conn_opts)) != MQTTASYNC_SUCCESS)
	{	
		pMqtt->nConnFailedTimes++;
		if(pMqtt->nConnFailedTimes < 3)
		{
			Log_ConnectStatuCode(pMqtt, "CnctFail");
		}
		else if(pMqtt->nConnFailedTimes >= 6)
		{
			pMqtt->nConnFailedTimes = 6;
		}
		else
		{
			//Do Nothing!
		}
		TRACEX("Error: Failed to MQTTAsync connect, return code %d\n", rc);
	}
	else
	{
		TRACEX("MQTTAsync connect ok!\n");
	}

	return (rc == MQTTASYNC_SUCCESS) ? TRUE : FALSE;
}

/*
static BOOL Connect_2MqttServer(MQTT_DATA_ROUGH* pMqtt)
{
	int rc = -1;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer; 
	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		MQTTAsync_connectOptions conn_opts_5 = MQTTAsync_connectOptions_initializer5;
		memcpy(&conn_opts, &conn_opts_5, sizeof(MQTTAsync_connectOptions));
	}
	pMqtt->conn_opts = conn_opts;

	//ssl option
    MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
    sslopts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
	
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

	pMqtt->conn_opts.ssl = &sslopts;
	pMqtt->conn_opts.ssl->verify = 1;
   	pMqtt->conn_opts.keepAliveInterval = 30;
	pMqtt->conn_opts.connectTimeout = MQTT_CONNET_TIMEOUT;
	pMqtt->conn_opts.username = pMqtt->szUserName; 
	pMqtt->conn_opts.password = pMqtt->szPassWord;  

	//----John 20240516 Added For Debug----//
	MQTTAsync_setTraceCallback(trace_callback);
	MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
	//--------------End-------------------//
	

	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		pMqtt->conn_opts.cleanstart = 1; //this is for mqtt5 and for mqtt5 cleansession should be 0
		pMqtt->conn_opts.cleansession = 0;
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

	rc = MQTTAsync_setCallbacks(pMqtt->client, pMqtt, Callback_ConnLost, Callback_MsgArrvd, Callback_Delivered);
	if(rc != MQTTASYNC_SUCCESS)
	{
		TRACEX("---------MQTTAsync_setCallbacks rc=[%d]--- \n", rc);
		return FALSE;
	}

	printf("keepAliveInterval=[%d] MQTTVersion=[%d]\n",  \
		pMqtt->conn_opts.keepAliveInterval, pMqtt->conn_opts.MQTTVersion);

	TRACE("Try MQTTAsync_connect: !!!\n");

	if ((rc = MQTTAsync_connect(pMqtt->client, &pMqtt->conn_opts)) != MQTTASYNC_SUCCESS)
    {
        TRACEX("Error: Failed to MQTTAsync connect, return code %d\n", rc);
		return FALSE;
	}

	TRACE("MQTTAsync_connect: Success!!!\n");

	return TRUE;
}*/

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Initialize()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   : 
*  
*    //-------------Init Will Topic--------------//
*    //conn_opts.will = &will_opts;
*	//conn_opts.will->message = "will message";
*	//conn_opts.will->qos = 1;
*	//conn_opts.will->retained = 0;
*	//conn_opts.will->topicName = "will topic";
*	//conn_opts.will = NULL;
*    //-------------------End--------------------//
*  CHANGE HISTORY: 
*  
***********************************************************************************/
/*static BOOL Create_Client_AndInit(MQTT_DATA_ROUGH* pMqtt)
{
	//destroy old client if have
	if(pMqtt->client != NULL)
	{
		TRACEX("----Remove old MQTT client before create!----\n");
		MQTTAsync_destroy(&pMqtt->client);
	}

    int rc = 0;    
    MQTTAsync client;
	rc = MQTTAsync_create(&client, pMqtt->szServerUrl, pMqtt->szClientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	TRACEX("-MQTTAsync_create rc=[%d]-szServerUrl=[%s] szClientID=[%s] Name[%s] Psd[%s]-\n", \
		rc, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName,pMqtt->szPassWord);

	LogOut(MAIN_MODULE_SAMPLER, "Rep-MQTT", LOG_TYPE_INFO, "Create:rc[%d]Url[%s]ID[%s]Name[%s]Psd[%s]", \
		rc, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);

	if(rc != MQTTASYNC_SUCCESS)
	{
		return FALSE;
	}

    pMqtt->client = client;
 
 	//init conn-options
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer; 
	pMqtt->conn_opts = conn_opts;

    MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
    sslopts.enableServerCertAuth = 0;
    //sslopts.trustStore = "cacert.pem";
    sslopts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;

   	pMqtt->conn_opts.keepAliveInterval = 30;
	pMqtt->conn_opts.connectTimeout = MQTT_CONNET_TIMEOUT;
    pMqtt->conn_opts.cleansession = 1;// 1 means not to save information from a previous connection of the same client
   	pMqtt->conn_opts.onSuccess = ExecOkCall_onConnect;
    pMqtt->conn_opts.onFailure = ExecFailCall_onConnectFailure;
    pMqtt->conn_opts.context = pMqtt;
    pMqtt->conn_opts.username = pMqtt->szUserName; 
    pMqtt->conn_opts.password = pMqtt->szPassWord;  	
    
	//-----Init SSL Encrpt Parameter-----//
    pMqtt->conn_opts.ssl = &sslopts;
	pMqtt->conn_opts.ssl->verify = 1;
		//----John 20240516 Added For Debug----//
	MQTTAsync_setTraceCallback(trace_callback);
	MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MAXIMUM);
	//--------------End-------------------//

	//memcpy(&pMqtt->conn_opts, &conn_opts, sizeof(MQTTAsync_connectOptions));
    rc = MQTTAsync_setCallbacks(pMqtt->client, pMqtt, Callback_ConnLost, Callback_MsgArrvd, Callback_Delivered);
	if(rc != MQTTASYNC_SUCCESS)
	{
		TRACEX("---------MQTTAsync_setCallbacks rc=[%d]--- \n", rc);
		//	return THREAD_CANCEL_ALL;
	}

	return TRUE;
}*/


static void ReCreate_MqttClient_AndInit(MQTT_DATA_ROUGH* pMqtt)
{	
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	int 			rc 	= 0;

	//disconnect old connection if have
	if(pMqtt->bIs_MqttConnOK)
	{
		Disconnect_MqttClientComm(pMqtt); 
	}

	//for test purpose
	// MQTTAsync_setTraceCallback(Mqtt_Trace_Callback);
	// MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_MINIMUM);

	//destroy old client if have
	if(pMqtt->client != NULL)
	{
		TRACEX("----Remove old MQTT client before create!----\n");
    	MQTTAsync_destroy(&pMqtt->client);
	}

    MQTTAsync client;

	MQTTAsync_createOptions createOptions = MQTTAsync_createOptions_initializer;
 	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		createOptions.MQTTVersion = MQTTVERSION_5;
	}
	else
	{
		createOptions.MQTTVersion = MQTTVERSION_3_1_1;  
	}
	
	rc = MQTTAsync_createWithOptions(&client, pMqtt->szServerUrl, pMqtt->szClientID, 
		MQTTCLIENT_PERSISTENCE_NONE, NULL, &createOptions);

 	pMqtt->client = client;

	//init conn-options	
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		MQTTAsync_connectOptions conn_opts_5 = MQTTAsync_connectOptions_initializer5;
		memcpy(&conn_opts, &conn_opts_5, sizeof(MQTTAsync_connectOptions));
	}	 
    
	MQTTAsync_SSLOptions sslopts 	= MQTTAsync_SSLOptions_initializer;
    sslopts.enableServerCertAuth 	= 0;
    //sslopts.trustStore 			= "cacert.pem";
    sslopts.sslVersion 				= MQTT_SSL_VERSION_TLS_1_2;

	conn_opts.MQTTVersion			= createOptions.MQTTVersion;
	
   	conn_opts.keepAliveInterval 	= 30;
	conn_opts.connectTimeout 		= MQTT_CONNET_TIMEOUT;
	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		conn_opts.cleanstart 		= 1; //this is for mqtt5 and for mqtt5 cleansession should be 0
		MQTTProperties props 		= MQTTProperties_initializer;
		MQTTProperty property;	
		property.identifier 		= MQTTPROPERTY_CODE_SESSION_EXPIRY_INTERVAL;
		property.value.integer4 	= 0; //should be 0 to clean session after reconnection
		MQTTProperties_add(&props, &property); 
		conn_opts.connectProperties = &props;
		conn_opts.cleansession 		= 0;
		conn_opts.onSuccess5 		= ExecOkCall_onConnect;
    	conn_opts.onFailure5 		= ExecFailCall_onConnectFailure;
	}	
	else
	{
		conn_opts.cleanstart 		= 0;
		conn_opts.cleansession 		= 1;// should be 0 form MQTT5, 1 means not to save information from a previous connection of the same client
		conn_opts.onSuccess 		= ExecOkCall_onConnect;
		conn_opts.onFailure 		= ExecFailCall_onConnectFailure;
	}
    conn_opts.context 				= pMqtt;
    conn_opts.username 				= pMqtt->szUserName;
    conn_opts.password 				= pMqtt->szPassWord;

    //-----Init SSL Encrpt Parameter-----//
	memcpy(&pMqtt->sslopts, &sslopts, sizeof(MQTTAsync_SSLOptions));
    conn_opts.ssl 					= &pMqtt->sslopts;
	//conn_opts.ssl->verify 		= 1;
	memcpy(&pMqtt->conn_opts, &conn_opts, sizeof(MQTTAsync_connectOptions));

    rc = MQTTAsync_setCallbacks(pMqtt->client, pMqtt, Callback_ConnLost, Callback_MsgArrvd, NULL);
	if(rc != MQTTASYNC_SUCCESS)
	{
		Log_ConnectStatuCode(pMqtt, "SetCallBackFail");
	}

	TRACEX("-----------MQTTAsync_setCallbacks rc=[%d]--- \n", rc);

	Log_ConnectStatuCode(pMqtt, "ReInitOK");

	return;
}
 
static BOOL Wait_Mqtt_CommReady(REPORTER* pReporter)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
	int 				rc = 0;
	static int 	s_nErrorCounter = 0;	
	TRACEX("-------emUserCommStatus=[%d]-------------\n", pMqtt->emUserCommStatus);
	//handle the connection to mqtt broker
	if(pMqtt->emUserCommStatus == USER_COMM_DISCONECTED)
    {	
		//disconnect old connection if have
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt);
		}
		
		//to avoid to reconnect too frequenty (max 15 * 30 ~= 7.5min)
		int RECONN_WAIT_MAX_TIMEOUT_SEC = pMqtt->nConnFailedTimes * MQTT_CONNET_TIMEOUT;
		RECONN_WAIT_MAX_TIMEOUT_SEC 	= MAX(10, RECONN_WAIT_MAX_TIMEOUT_SEC);
		//added by Jimmy to avoid conn-fail too frequently
		RECONN_WAIT_MAX_TIMEOUT_SEC 	= MAX(RECONN_WAIT_MAX_TIMEOUT_SEC, pMqtt->nConnFailedTimes * 10);
		if(ABS(_NOW_ - pMqtt->tmLastSend_Conn) >= RECONN_WAIT_MAX_TIMEOUT_SEC)
		{
			pMqtt->tmLastSend_Conn = _NOW_;
 			if(Connect_2MqttServer(pMqtt) == FALSE)
			{
				ReCreate_MqttClient_AndInit(pMqtt);
			}
		}
		else
		{
			TRACEX("---------Now disconnt[%d], ERRRRRRRRR222222222-------\n", pMqtt->nConnFailedTimes);
		}		
		//needs to return here!!!
		return FALSE;
    }
	else if(pMqtt->emUserCommStatus == USER_COMM_CONNECTED)
	{
		s_nErrorCounter = 0;
		if(pMqtt->bTopic_Subscribed == FALSE)
		{
			init_SubscribleMsg(pMqtt); 
			pMqtt->emUserCommStatus = USER_COMM_SUBSCRIBING;
		}
		else
		{
			pMqtt->emUserCommStatus = USER_COMM_READY;	
		}
	}
	else if(pMqtt->emUserCommStatus == USER_COMM_SUBSCRIBING)
	{
		s_nErrorCounter++;
	}
	else if(pMqtt->emUserCommStatus == USER_COMM_READY)
	{
		//TRACEX("-------THE MQTT-Client is READY!!!!--------\n");
		return TRUE;
	}
	else if(pMqtt->emUserCommStatus == USER_COMM_WAIT_REBUILD_CLIENT)
	{	
		pMqtt->nConnFailedTimes = 0;			
		ReCreate_MqttClient_AndInit(pMqtt);
		pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
		pMqtt->emDisCnt_Reason 	= STATCODE_COMM_FAIL_REASON_SERVERADDR_CHANGED;
	}
	else
	{
		TRACE("--- Wrong User Comm Status, Disconetd ---!!!\n");
		s_nErrorCounter++;
	}

	if(s_nErrorCounter >= 1)
	{
		//disconn the comm forcely
		TRACEX("----Now needs to disconnecte MQTT-conn so as to reconnect");
		if(pMqtt->bIs_MqttConnOK)
		{
			Disconnect_MqttClientComm(pMqtt);
		}
		Sleep(1000);
		pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
		pMqtt->emDisCnt_Reason 	= STATCODE_COMM_FAIL_REASON_SCRIBED_TIMEOUT;
	}

	return FALSE;
}

static DWORD _ThreadEntry_MqttComm(void* pArg)
{
	TRACE("-----------_ThreadEntry_MqttComm async: Start Enter------- \n");

	REPORTER* pReporter 	= (REPORTER*)pArg;
	HANDLE hPort 			= pReporter->hCommPort;
	COMMPORT* pComm 		= pReporter->pUsedCommPort;
	MQTT_DATA_ROUGH* pMqtt 	= (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
 
    ReCreate_MqttClient_AndInit(pMqtt);

   	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();

		Handle_CommStatus(pMqtt);

		if(!Wait_Mqtt_CommReady(pReporter))
		{
			Sleep(5000);
			continue;
		}

		onConnLost_NotRecvR101Cmd(pMqtt);

		if(pMqtt->bIs_MqttConnOK == TRUE)
		{	
			if(pMqtt->emCustomType == GH_CUSTOM)
			{
				Pub_GH_MsgToServer(pMqtt);
			}
			else
			{
				int rc = mqtt_MainRun(pMqtt, &pMqtt->conn_opts);
			}
		}
		//TRACEX("-----------mqtt_MainRun rc=[%d]--- \n", rc);
        Sleep(200);
    }

    MQTTAsync_destroy(&pMqtt->client);
    SAFELY_DELETE(pMqtt); //Release Allocate Memory

	TRACEX("-----------_ThreadEntry_MqttComm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}


/*
static DWORD _ThreadEntry_MqttComm(void* pArg)
{
	TRACE("-----------_ThreadEntry_MqttComm: Start Enter------- \n");

	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pReporter->pDataHeap;
    //PARAM_VAL_GET 	valGet;
	//ERRCODE_DAI 	err;

	//BOOL bRet = Create_Client_AndInit(pMqtt);
	//if(bRet == FALSE)
	//{
	//	return THREAD_CONTINUE_RUN;
	//}

	//---------MQTT Start-----------//
    int rc = 0;
    //char *username= "dgnlgx"; //添加的用户名
    //char *password = "dgnlgx11!@"; //添加的密码
    
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    pMqtt->conn_opts = conn_opts;
 
    MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
    sslopts.enableServerCertAuth = 0;
    //sslopts.trustStore = "cacert.pem";
    sslopts.sslVersion = MQTT_SSL_VERSION_DEFAULT;

    rc = MQTTAsync_create(&client, pMqtt->szServerUrl, pMqtt->szClientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
	TRACEX("---MQTTAsync_create rc=[%d]-szServerUrl=[%s] szClientID=[%s] Name[%s] Psd[%s]-\n", \
		rc, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName,pMqtt->szPassWord);

	LogOut(MAIN_MODULE_SAMPLER, "Rep-MQTT", LOG_TYPE_INFO, "Create:rc[%d]Url[%s]ID[%s]Name[%s]Psd[%s]", \
        rc, pMqtt->szServerUrl, pMqtt->szClientID, pMqtt->szUserName, pMqtt->szPassWord, pszFormat);
	
	if(rc != MQTTASYNC_SUCCESS)
	{
		return THREAD_CONTINUE_RUN;
	}
    //-------------Init Mqtt Data Rough--------------//
    pMqtt->client = client;

 	 	 
    //--------------------End-----------------------//

    pMqtt->conn_opts.keepAliveInterval = 30;
    pMqtt->conn_opts.cleansession = 0;//1
    pMqtt->conn_opts.onSuccess = ExecOkCall_onConnect;
    pMqtt->conn_opts.onFailure = ExecFailCall_onConnectFailure;
    pMqtt->conn_opts.context = pMqtt;
    pMqtt->conn_opts.username = pMqtt->szUserName;
    pMqtt->conn_opts.password = pMqtt->szPassWord;
	
    //-----Init SSL Encrpt Parameter-----//
    pMqtt->conn_opts.ssl = &sslopts;
	pMqtt->conn_opts.ssl->verify = 1;

    rc = MQTTAsync_setCallbacks(pMqtt->client, pMqtt, Callback_ConnLost, Callback_MsgArrvd, Callback_Delivered);
	if(rc != MQTTASYNC_SUCCESS)
	{
		TRACEX("-----------MQTTAsync_setCallbacks rc=[%d]--- \n", rc);
		//	return THREAD_CANCEL_ALL;
	}
	

   	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();

		Handle_CommStatus(pMqtt);

        if(pMqtt->bIs_MqttConnOK == FALSE)
        {   
            if ((rc = MQTTAsync_connect(pMqtt->client, &pMqtt->conn_opts)) != MQTTASYNC_SUCCESS)
            {
                TRACE("Error: Failed to MQTTAsync connect, return code %d\n", rc);
            }
            Sleep(10000); //Just Wait For 10 Sec
            continue; 
        }
    
        rc = mqtt_MainRun(pMqtt, &pMqtt->conn_opts);

		//TRACEX("-----------mqtt_MainRun rc=[%d]--- \n", rc);
        Sleep(100);
    }

    MQTTAsync_destroy(&pMqtt->client);
    SAFELY_DELETE(pMqtt); //Release Allocate Memory

	TRACEX("-----------_ThreadEntry_MqttComm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}
*/

static BOOL Init_GetSysAlarmDataInfo(MQTT_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI err;
	BOOL bRet = TRUE;
	int n = 0;
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
	pMqtt->ShieldAlarms = NEW(UINT,pMqtt->nAlarmInfosTotal);
	if( pMqtt->ShieldAlarms == NULL)
    {
		TRACE("Error: Allocate to AlarmInfo Failed!\n");
        return FALSE;
    }
	ZERO_POBJS(pMqtt->ShieldAlarms, pMqtt->nAlarmInfosTotal);
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
	pMqtt->bIs_MqttConnOK = FALSE; 
	pMqtt->client = NULL;     
    pMqtt->bTopic_Subscribed = FALSE;  
	pMqtt->nCurrDataFrom = 0;
	pMqtt->bNeedRefreshActvAlm = TRUE;

    Init_MqttMsgInfo(pMqtt); 
	
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 10, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emRepInstType = (CLOUDREP_INST_TYPE)valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 87, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emCustomType = (CUSTOM_TYPE)valGet.val.emValue;
	if(pMqtt->emCustomType == GH_CUSTOM)
	{
		Init_GH_MqttMsgInfo(pMqtt); 
	}
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			951, 448, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emAirType = (AIR_TYPE)valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 89, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emControlMode = (COMTROL_MODE)valGet.val.emValue;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 88, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->nRemoteControlTime = valGet.val.ulValue; 

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 90, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->nSendTimeInterval = valGet.val.ulValue; 

	pMqtt->bDisPower = FALSE;
	pMqtt->tSendDisPower = _NOW_;

	//Get Mqtt URL + ClientID + UserName + PassWord: SSL:// Or TCP://,WS://,WSS://
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
			snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/up", pMqtt->szSelfTopic, pMqtt->szClientID);
			snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/%s/v1/%s/down/#", pMqtt->szSelfTopic, pMqtt->szClientID);			
		}
		else if (strcmp(pMqtt->szSelfTopic, "custom") == 0)
		{
			valGet.nGetSize = 128;
			valGet.val.blobValue = (BYTE*)pMqtt->szManualPubTopic;
			ERRCODE_DAI err1 = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 38, &valGet, sizeof(PARAM_VAL_GET));
			if (err1 != ERRCODE_DAI_OK)
			{
				TRACE("ERROR: Manual PubTopic ERROR!\n");
				return FALSE;
			}

			valGet.nGetSize = 128;
			valGet.val.blobValue = (BYTE*)pMqtt->szManualSubTopic;
			ERRCODE_DAI err2 = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 39, &valGet, sizeof(PARAM_VAL_GET));
			if (err2 != ERRCODE_DAI_OK)
			{
				TRACE("ERROR: Manual SubTopic ERROR!\n");
				return FALSE;
			}
			
			snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "%s", pMqtt->szManualPubTopic);
			snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "%s", pMqtt->szManualSubTopic);
		}
		else
		{
			snprintf(pMqtt->szPubTopic, MQTT_TOPIC_SIZE, "001/ess/v1/%s/up", pMqtt->szClientID);
			snprintf(pMqtt->szSubTopic, MQTT_TOPIC_SIZE, "001/ess/v1/%s/down/#", pMqtt->szClientID);
		}
		
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
	pMqtt->nCfgBMSNum = valGet.val.ulValue;
	if(pMqtt->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pMqtt->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}

	//----- Get Zlib Compress Config -----//
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

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 55, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->TimeCalibration ==  ((TIME_CALIBRATION)valGet.val.emValue == TIME_MQTT)? TRUE : FALSE;;

	//---------- Init System Alarm DataInfo ----------//
	if(!Init_GetSysAlarmDataInfo(pMqtt))
	{
		TRACEX("ERROR: Get Sys Alarm DataInfo Failed!\n");
		return FALSE;
	}

	//--------------Get Shield Alarms id----------------//
	valGet.nGetSize = 512;
	valGet.val.blobValue = (BYTE*)pMqtt->szShieldAlarms;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 83, &valGet, sizeof(PARAM_VAL_GET));

	pMqtt->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd
	//-----------------------End----------------------//

	pMqtt->hMutex = Mutex_Create(TRUE);
	pMqtt->hqMsg2Sent = Queue_Create(MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(MQTT_CALL_MSG), 0);
 	if((pMqtt->hMutex == NULL) || (pMqtt->hqMsg2Sent == NULL))
	{
		TRACEX("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("Mqtt-async",
		(RUN_THREAD_START_PROC)_ThreadEntry_MqttComm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create MqttComm thread Failed \n");
		return FALSE;
	}
	
	//-----Init event handler-----//
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
  


