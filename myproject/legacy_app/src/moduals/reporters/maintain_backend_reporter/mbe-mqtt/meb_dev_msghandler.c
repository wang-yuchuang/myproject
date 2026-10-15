#include "mbe_dev_mqtt.h"

static char* Make_MbeMqtt_TimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	snprintf(timeinstring, 32, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring, 32);
	return pszOut;
}

time_t  Parse_MBETime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

int Prepare_Topic_AuthReq(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;

	cJSON* pCurrJson_2Pub = cJSON_CreateObject();
	cJSON_AddStringToObject(pCurrJson_2Pub, "SN", pMqtt->szDevSN);
	cJSON_AddStringToObject(pCurrJson_2Pub, "MAC", pMqtt->szDevMAC);
	cJSON_AddNumberToObject(pCurrJson_2Pub, "proto_ver", 0); //current use 0
	cJSON_AddNumberToObject(pCurrJson_2Pub, "req_max_keys", MBE_DEV_SUPPORT_MAX_CFGPARM_ITEMS); 
	cJSON_AddNumberToObject(pCurrJson_2Pub, "set_max_keys", MBE_DEV_SUPPORT_MAX_MULTISET_ITEMS);

	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_AUTH_REQ, pCurrJson_2Pub);

	return 0;
}

int Prepare_Topic_Status(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	char szTime[32];

	cJSON* pCurrJson_2Pub = cJSON_CreateObject();
	cJSON_AddStringToObject(pCurrJson_2Pub, "ver_info", pMqtt->stCurrStatusData.szVerInfo);
	cJSON_AddStringToObject(pCurrJson_2Pub, "time_stamp", Make_MbeMqtt_TimeString(_NOW_, szTime));
	cJSON_AddNumberToObject(pCurrJson_2Pub, "work_stat", pMqtt->stCurrStatusData.emWorkStatus);
	cJSON_AddNumberToObject(pCurrJson_2Pub, "remote_cmd_exe_stat", pMqtt->stCurrStatusData.emRemoteCmd_ExeStat);
	cJSON_AddNumberToObject(pCurrJson_2Pub, "fault_stat", pMqtt->stCurrStatusData.emFaultyStat);
	cJSON_AddNumberToObject(pCurrJson_2Pub, "vcon_stat", pMqtt->stCurrStatusData.emVPNStat);

	if(pMqtt->stCurrStatusData.nActiveAlmTotal > 0)
	{
		//cJSON_AddNumberToObject(pJson_Status, "sys_almstat", 1);
		cJSON* pJAlmTable = cJSON_CreateArray();
		int n;
		MBE_DEV_ALARM *pAlm = pMqtt->stCurrStatusData.actAlmTable;
		//BOOL bHasCAAlarm = FALSE;
		int nLevel = 0;
		for(n = 0; n < pMqtt->stCurrStatusData.nActiveAlmTotal; n++, pAlm++)
		{
			cJSON* pJ = cJSON_CreateObject();
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETAALARM_LEVEL_BYID_R,
				pAlm->nAlarmId, 0, &nLevel, 4);	
			cJSON_AddNumberToObject(pJ, "lev", nLevel - 1);
			// if(nLevel == ALARM_LEVEL_CRITICAL)
			// {
			// 	bHasCAAlarm = TRUE;
			// }
			char szAlmName[32];
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETAALARMNAME_BYID_R,
				pAlm->nAlarmId, 0, szAlmName, 32);	
			cJSON_AddStringToObject(pJ, "name", szAlmName);	
			char szDevName[32];
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETDEVNAME_BYID_R,
				pAlm->nDevId, 0, szDevName, 32);	
			//printf("--------DEEEEv=>%d, NAAAAAAAm=>%s------\n", pAlm->nDevId, szDevName);
			cJSON_AddStringToObject(pJ, "src", szDevName);	
			cJSON_AddStringToObject(pJ, "start_tm", Make_MbeMqtt_TimeString(pAlm->tmStart, szTime));
			cJSON_AddNumberToObject(pJ, "alm_id", pAlm->nAlarmId);
			cJSON_AddItemToArray(pJAlmTable, pJ);
		}
		//cJSON_AddNumberToObject(pJson_Status, "sys_almstat", bHasCAAlarm ? 2 : 1);
		cJSON_AddItemToObject(pCurrJson_2Pub, "alm_table", pJAlmTable);		
	}	

	//added by Jimmy just in case we send busy status and don't sync to server
	if(pMqtt->stCurrStatusData.emRemoteCmd_ExeStat != 0)
	{
		pMqtt->tmLastSent_RemoteExeCmd_Busy = _NOW_;
	}
	else 
	{
		pMqtt->tmLastSent_RemoteExeCmd_Busy = 0;
	}

	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_STATUS, pCurrJson_2Pub);

	return 0;
}

static void Send_Error_fromServPub(MQTT_MBE_DATA_ROUGH* pMbeDev, MBEDEV_ERRMSG_ERRTYPE emErrorCode, MBE_DEV_SUB_MSGID emMsgId, char* szErrDesp)
{
	cJSON* pCurrJson_2Pub = cJSON_CreateObject();
	cJSON_AddNumberToObject(pCurrJson_2Pub, "code", emErrorCode);	
	cJSON_AddStringToObject(pCurrJson_2Pub, "topicId", pMbeDev->stSubMsgList[emMsgId].szTopicName);
	if(szErrDesp != NULL)
	{
		cJSON_AddStringToObject(pCurrJson_2Pub, "err_description", szErrDesp);
	}
	Publish_AMqttJsonMsg(pMbeDev, MBE_MSG_ERR_DEV, pCurrJson_2Pub);
}

int Handle_Topic_Brsd(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	cJSON *temp = NULL;
	int nEventType = -999;
	if (temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "event"))
	{
		if (cJSON_Number == temp->type)
		{
			nEventType = temp->valueint;
		}
	}

	if(nEventType < 0)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "WRONG or EMPTY [event]!");
		return -1;
	}

	if((nEventType == 0) || (nEventType == 1))
	{
		pMqtt->bIsNeed_Pause_Publish = TRUE;
	}
	else if(nEventType == 2)
	{
		pMqtt->bIsNeed_Pause_Publish = FALSE;
	}
	else if(nEventType == 3)
	{
		pMqtt->emMqttCommStatus = MBE_COMM_AUTH_REJECTED; //needs to reconnect immediately
		//return Prepare_Topic_AuthReq(pMqtt); //send auth.req immediately
	}

	return 0;
}

int Handle_Topic_AuthOK(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *temp = NULL;
	//time
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "time"))
	{
		if (cJSON_String == temp->type)
		{
			time_t tm = Parse_MBETime(temp->valuestring) + 8*ONE_HOUR_SEC; //MBE?0????，需要加8?小?
			if (ABS(tm - _NOW_) > 3) //sync RTC
			{
				VAR_VALUE varV;
				varV.dtValue = tm;
				if(pMqtt->TimeCalibration)
				{
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
				}
			}
			//don't use this at the moment
			//time_t tmServerCurrentTime = Parse_ISO8601TimeStr(temp->valuestring);
		}
	}
	else
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[time] NOT found!");
		return -1;
	}

	//refresh_intrv
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "refresh_intrv"))
	{
		if(cJSON_Number == temp->type)
		{
			pMqtt->nStatusReport_Interval = (UINT)temp->valueint;
			//should be 1~1440
			if(pMqtt->nStatusReport_Interval > 1440)
			{
				pMqtt->nStatusReport_Interval = 1440;
			}
			// else if(pMqtt->nStatusReport_Interval < 1)
			// {
			// 	pMqtt->nStatusReport_Interval = 30;
			// }
		}
	}
	else
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[refresh_intrv] NOT found!");
		return -1;
	}

	//now judge if the comm-status needs to be set -> ready
	if((pMqtt->emMqttCommStatus >= MBE_COMM_AUTHING) &&
		(pMqtt->emMqttCommStatus <= MBE_COMM_AUTH_REJECTED))
	{
		pMqtt->emMqttCommStatus = MBE_COMM_READY;
	}

	return 0;
}
 
int Handle_Topic_AuthNOK(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	cJSON *temp = NULL;

	//code
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "code"))
	{
		if (cJSON_Number == temp->type)
		{
			int nCode = temp->valueint;
		}
	}
	else
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[code] NOT found!");
		return -1;
	}

	//now judge if the comm-status needs to be set -> ready
	if((pMqtt->emMqttCommStatus >= MBE_COMM_AUTHING) &&
		(pMqtt->emMqttCommStatus <= MBE_COMM_READY))
	{
		pMqtt->emMqttCommStatus = MBE_COMM_AUTH_REJECTED;
		pMqtt->nAuthReject_Times++;
	}
	//pMqtt->emOCPP_Disconn_Reason = MBEMQTT_COMM_FAIL_REASON_REFUSED;

	return 0;
}

int Handle_Topic_TrigStat(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	//added by Jimmy in case to sync the status
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	Judge_IfNeed_SendStatus(pReporter);
	pMqtt->tmLastSentTrigStatus = _NOW_;
	return Prepare_Topic_Status(pMqtt); //send status immediately
}

static BOOL Parse_ASig_FromMqtt_KeyName(REPORTER* pReporter, char* szKeyName, OUT MBE_MQTT_ASIG_KEY* pSigKey)
{
	//printf("-------Now begindoing the parse for key->[%s]------\n", szKeyName);
	//sigkeyName format: x.x.xx, eg, 1.0.20
#define MQTT_SIG_SPLITTER	'.'
	int nSpiltterCount = GetSplitStringCount(szKeyName, MQTT_SIG_SPLITTER);
	if((nSpiltterCount < 3) || (nSpiltterCount > 4))
	{
		return FALSE;
	}
	char* pos = NULL;
	//dev
	char* szDest1 = SplitString(szKeyName, &pos, MQTT_SIG_SPLITTER);
	if(pos != NULL)
	{
		pSigKey->nDev = atoi(pos);
	}
	else
	{
		return FALSE;
	}
	//sig type
	pos = NULL;
	char* szDest2 = SplitString(szDest1, &pos, MQTT_SIG_SPLITTER);
	if(pos != NULL)
	{
		pSigKey->emSigType = atoi(pos);
	}
	else
	{
		return FALSE;
	}
	//sig id
	pos = NULL;
	char* szDest3 = SplitString(szDest2, &pos, MQTT_SIG_SPLITTER);
	if(pos != NULL)
	{
		pSigKey->nSigId = atoi(pos);
	}
	else
	{
		return FALSE;
	}
	//sub sig id(for blob)
	if(nSpiltterCount == 4)
	{
		pos = NULL;
		SplitString(szDest3, &pos, MQTT_SIG_SPLITTER);
		if(pos != NULL)
		{
			pSigKey->bIsSubSig = TRUE;
			pSigKey->nSigSubId = atoi(pos);
		}
	}

	//don't check for alarm
	if(pSigKey->emSigType == MBE_SIGTYPE_ALARM)
	{
		return TRUE;
	}

	//get param info to decide its val-type
	//("-------Now beginfinding the param->[%d, %d, %d], nSpiltterCount = %d, bIsSubSig = %d------\n", 
	//	pSigKey->nDev, pSigKey->emSigType, pSigKey->nSigId, nSpiltterCount, pSigKey->bIsSubSig);
	PARAM_BASIC_INFO paramInfo;
	int errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PARAM_INFO_BY_PARAMID_R,
				pSigKey->emSigType, pSigKey->nSigId, &paramInfo, sizeof(PARAM_BASIC_INFO));
	if(errCode != ERRCODE_DAI_OK)
	{
		return FALSE;
	}

	//printf("-------Now founnnnnnd the param->[%d, %d, %d], valInfo->[%d, %d]------\n", 
	//		pSigKey->nDev, pSigKey->emSigType, pSigKey->nSigId,
	//		paramInfo.emValueType, paramInfo.nValueSize);
	
	//now judge the valueTYpe
	pSigKey->emValTYpe = paramInfo.emValueType;
	pSigKey->nFloatPoint = paramInfo.nPrecision;
	pSigKey->nValueSize = paramInfo.nValueSize;	
	if(paramInfo.emValueType == VALUE_TYPE_BLOB)
	{
		if(!pSigKey->bIsSubSig)
		{
			return FALSE;
		}
		//reuse precision
		pSigKey->nBlobId = paramInfo.nPrecision;
	}
	else if(pSigKey->bIsSubSig)
	{
		return FALSE;
	}

	TRACEX("---------Now parsed a sig->[%d, %d, %d] nValueSize=%d ,IsBlob->%d, OKKKKKKK-------\n",
		pSigKey->nDev, pSigKey->emSigType, pSigKey->nSigId, pSigKey->nValueSize, pSigKey->bIsSubSig);

	return TRUE;
}

static char* Get_NormalParmaVal_FromKey(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY* pSigKeyInfo, char* pszVal)
{
	TRACEX("-------Now trying to get a normalsigval!!!------\n");

	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	PARAM_VAL_GET valGet;
	int nDAI_Type = (pSigKeyInfo->emSigType == PARAM_TYPE_SETTING) ?
		DAITYPE_SETT_PARAMVALUE_RW : DAITYPE_STAT_PARAMVALUE_R;
	
	if(pSigKeyInfo->emValTYpe != VALUE_TYPE_STRING)
	{
		valGet.nGetSize = 4;
		if(pReporter->pDAI->Get((HANDLE)pReporter, nDAI_Type,
			pSigKeyInfo->nDev, pSigKeyInfo->nSigId, &valGet, sizeof(PARAM_VAL_GET)) != ERRCODE_DAI_OK)
		{
			return NULL;
		}

		//added by Jimmy to avoid invlid value to be sent
		if((valGet.nValueAttribute & MBE_PARAM_MASK_VALID) != MBE_PARAM_MASK_VALID)
		{
			sprintf(pszVal, "--");
			return pszVal;
		}

		if(pSigKeyInfo->emValTYpe == VALUE_TYPE_LONG)
		{
			sprintf(pszVal, "%d", valGet.val.lValue);
			return pszVal;
		}
		else if((pSigKeyInfo->emValTYpe == VALUE_TYPE_ULONG) ||
				(pSigKeyInfo->emValTYpe == VALUE_TYPE_ENUM))
		{
			sprintf(pszVal, "%u", valGet.val.ulValue);
			return pszVal;
		}
		else if(pSigKeyInfo->emValTYpe == VALUE_TYPE_DATETIME)
		{
			Make_MbeMqtt_TimeString(valGet.val.ulValue, pszVal);
			return pszVal;
		}
		else if(pSigKeyInfo->emValTYpe == VALUE_TYPE_FLOAT)
		{
			if(pSigKeyInfo->nFloatPoint >= 3)
			{
				sprintf(pszVal, "%.3f", valGet.val.fValue);
			}
			else if(pSigKeyInfo->nFloatPoint == 2)
			{
				sprintf(pszVal, "%.2f", valGet.val.fValue);
			}
			else if(pSigKeyInfo->nFloatPoint == 1)
			{
				sprintf(pszVal, "%.1f", valGet.val.fValue);
			}
			else
			{
				sprintf(pszVal, "%.0f", valGet.val.fValue);
			}
			return pszVal;
		}
	}
	else
	{
		//valGet.nGetSize = pSigKeyInfo->nValueSize;
		char szStringVal[256] = {0}; //max 256-char for string-parameter
		valGet.val.blobValue = szStringVal;
		valGet.nGetSize = MIN(pSigKeyInfo->nValueSize, 256);
		if(pReporter->pDAI->Get((HANDLE)pReporter, nDAI_Type,
			pSigKeyInfo->nDev, pSigKeyInfo->nSigId, &valGet, sizeof(PARAM_VAL_GET)) != ERRCODE_DAI_OK)
		{
			return NULL;
		}

		//added by Jimmy to avoid invlid value to be sent
		if((valGet.nValueAttribute & MBE_PARAM_MASK_VALID) != MBE_PARAM_MASK_VALID)
		{
			sprintf(pszVal, "--");
			return pszVal;
		}

		snprintf(pszVal, 256, "%s", szStringVal);
		return pszVal;
	}

	return NULL;
}

static char* GetCfg_Val_ByKeyName(MQTT_MBE_DATA_ROUGH* pMqtt, char* szKeyName, char* pszVal)
{
	MBE_MQTT_ASIG_KEY stSigKey;
	ZERO_POBJS(&stSigKey, 1);
	VAR_VALUE varV;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;	

	if(!Parse_ASig_FromMqtt_KeyName(pReporter, szKeyName, &stSigKey))
	{
		return NULL;
	}

	if((stSigKey.emSigType == MBE_SIGTYPE_ALARM) && (!stSigKey.bIsSubSig))
	{
		//alarm
		if(pReporter->pDAI->Get(pMqtt->hReporter, DAITYPE_ALARMSTAT_R,
			stSigKey.nDev, stSigKey.nSigId, &varV, 4) != ERRCODE_DAI_OK)
		{
			return NULL;
		}
		sprintf(pszVal, "%d", (varV.emValue == 1) ? 1 : 0);
		return pszVal;
	}
	else if((stSigKey.emSigType == PARAM_TYPE_STATUS) ||
		(stSigKey.emSigType == PARAM_TYPE_SETTING))
	{
		if(stSigKey.emValTYpe == VALUE_TYPE_BLOB)
		{
			if(!stSigKey.bIsSubSig)
			{
				TRACEX("-----ERROR not-find-the-sub-blob of that key----\n");
				return NULL;
			}
			TRACEX("-----Now trying to get ABLOB-SIGVAL----\n");
			//now handle sub-parm for blob specially
			return Get_ASubParmaVal_FromKey(pMqtt, &stSigKey, pszVal);
		}
		else
		{
			TRACEX("-----Now trying to get ANORMAL-SIGVAL----\n");
			return Get_NormalParmaVal_FromKey(pMqtt, &stSigKey, pszVal);
		}
	}

	return NULL;
}

int Handle_Topic_CfgReq(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	cJSON *temp = NULL;
	cJSON *cfgs = NULL;
	//cfgs
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "cfgs"))
	{
		if (cJSON_Array == temp->type)
		{
			cfgs = temp;
		}
	}

	if(cfgs == NULL)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[cfgs] NOT found!");
		return -1;
	}

	int nCfgTotal = cJSON_GetArraySize(cfgs);
	if(nCfgTotal < 1)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[cfgs] is EMPTY!");
		return -1;
	}
	else if(nCfgTotal > MBE_DEV_SUPPORT_MAX_CFGPARM_ITEMS)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[cfgs] is too heavy!");
		return -1;
	}

	//senderId(optional)
	char* szSenderId = NULL;
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "sender_id"))
	{
		if(cJSON_String == temp->type)
		{
			szSenderId = temp->valuestring;
		}
	}

	int n;
	cJSON* pJCfgItem = NULL;
	char  szCfgItem_Key[32];
	char  szCfgVal[256] = {0};
	cJSON* pCurrJson_2Pub = cJSON_CreateObject();
	cJSON* pAccptedCfgs = NULL;
	cJSON* pUnkowndCfgs = NULL;
	int nResult = 1; //failed
	for(n = 0; n < nCfgTotal; n++)
	{
		pJCfgItem = cJSON_GetArrayItem(cfgs, n);
		if(pJCfgItem == NULL)
		{
			//make and send error msg??
			cJSON_Delete(pCurrJson_2Pub);
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[cfgs] is is INVALID #1!");
			return -1;			
		}
		//szCfgItem_Key = pJCfgItem->valuestring;
		snprintf(szCfgItem_Key, 32,  "%s", pJCfgItem->valuestring);
		if(szCfgItem_Key == NULL)
		{
			//make and send error msg??
			cJSON_Delete(pCurrJson_2Pub);
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[cfgs] is is INVALID #2!");
			return -1;	
		}
		if(GetCfg_Val_ByKeyName(pMqtt, szCfgItem_Key, szCfgVal) != NULL)
		{
			if(pAccptedCfgs == NULL)
			{
				nResult = (nResult == 1) ? 0 : nResult; //success
				pAccptedCfgs = cJSON_CreateArray();
			}
			cJSON* keyVal = cJSON_CreateObject();
			cJSON_AddStringToObject(keyVal, "key", /*szCfgItem_Key*/pJCfgItem->valuestring);
			cJSON_AddStringToObject(keyVal, "val", szCfgVal);
			cJSON_AddItemToArray(pAccptedCfgs, keyVal);
		}
		else
		{
			if(pUnkowndCfgs == NULL)
			{
				nResult = (nResult == 0) ? 2 : nResult; //partly success
				pUnkowndCfgs = cJSON_CreateArray();
			}
			cJSON* keyErr = cJSON_CreateString(/*szCfgItem_Key*/pJCfgItem->valuestring);
			cJSON_AddItemToArray(pUnkowndCfgs, keyErr);			
		}
	}

	cJSON_AddNumberToObject(pCurrJson_2Pub, "result", nResult); 
	if(pAccptedCfgs != NULL)
	{
		cJSON_AddItemToObject(pCurrJson_2Pub, "cfgvals", pAccptedCfgs);
	}

	if(pUnkowndCfgs != NULL)
	{
		cJSON_AddItemToObject(pCurrJson_2Pub, "unsupported_cfgs", pUnkowndCfgs);
	}

	//senderId
	if(szSenderId != NULL)
	{
		cJSON_AddStringToObject(pCurrJson_2Pub, "sender_id", szSenderId);
	}

	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CFG_CONF, pCurrJson_2Pub);

	return 0;
}

static char* GetParam_Val_ByKeyName(MQTT_MBE_DATA_ROUGH* pMqtt, char* szKeyName, char* pszVal)
{
	MBE_MQTT_ASIG_KEY stSigKey;
	ZERO_POBJS(&stSigKey, 1);
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;	

	if(!Parse_ASig_FromMqtt_KeyName(pReporter, szKeyName, &stSigKey))
	{
		return NULL;
	}

	if(stSigKey.emValTYpe == VALUE_TYPE_BLOB)
	{
		if(!stSigKey.bIsSubSig)
		{
			TRACEX("-----ERROR not-find-the-sub-blob of that key----\n");
			return NULL;
		}
		//now handle sub-parm for blob specially
		return Get_ASubParmaVal_FromKey(pMqtt, &stSigKey, pszVal);
	}

	return Get_NormalParmaVal_FromKey(pMqtt, &stSigKey, pszVal);
}

int Handle_Topic_ParamReq(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	cJSON *temp = NULL;
	cJSON *parms = NULL;
	//params
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "params"))
	{
		if (cJSON_Array == temp->type)
		{
			parms = temp;
		}
	}

	if(parms == NULL)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[params] NOT found!");
		return -1;
	}

	int nParamTotal = cJSON_GetArraySize(parms);
	if(nParamTotal < 1)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[params] is EMPTY!");
		return -1;
	}
	else if(nParamTotal > MBE_DEV_SUPPORT_MAX_CFGPARM_ITEMS)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[params] is is too heavy!");
		return -1;
	}

	//senderId(optional)
	char* szSenderId = NULL;
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "sender_id"))
	{
		if (cJSON_String == temp->type)
		{
			szSenderId = temp->valuestring;
		}
	}

	int n;
	cJSON* pJParamItem = NULL;
	char  szParamItem_Key[32];
	char  szParamVal[256] = {0};
	cJSON* pCurrJson_2Pub = cJSON_CreateObject();
	cJSON* pAccptedParams = NULL;
	cJSON* pUnkowndParams = NULL;
	int nResult = 1; //failed
	for(n = 0; n < nParamTotal; n++)
	{
		pJParamItem = cJSON_GetArrayItem(parms, n);
		if(pJParamItem == NULL)
		{
			//make and send error msg??
			cJSON_Delete(pCurrJson_2Pub);
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[params] is is INVALID #1!");
			return -1;			
		}
		//szParamItem_Key = pJParamItem->valuestring;
		snprintf(szParamItem_Key, 32,  "%s", pJParamItem->valuestring);
		if(szParamItem_Key == NULL)
		{
			//make and send error msg??
			cJSON_Delete(pCurrJson_2Pub);
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[params] is is INVALID #2!");
			return -1;	
		}
		if(GetParam_Val_ByKeyName(pMqtt, szParamItem_Key, szParamVal))
		{
			if(pAccptedParams == NULL)
			{
				nResult = (nResult == 1) ? 0 : nResult; //success
				pAccptedParams = cJSON_CreateArray();
			}
			cJSON* keyVal = cJSON_CreateObject();
			cJSON_AddStringToObject(keyVal, "key", pJParamItem->valuestring);
			cJSON_AddStringToObject(keyVal, "val", szParamVal);
			cJSON_AddItemToArray(pAccptedParams, keyVal);
		}
		else
		{
			if(pUnkowndParams == NULL)
			{
				nResult = (nResult == 0) ? 2 : nResult; //partly success
				pUnkowndParams = cJSON_CreateArray();
			}
			cJSON* keyErr = cJSON_CreateString(pJParamItem->valuestring);
			cJSON_AddItemToArray(pUnkowndParams, keyErr);			
		}
	}

	cJSON_AddNumberToObject(pCurrJson_2Pub, "result", nResult); 
	if(pAccptedParams != NULL)
	{
		cJSON_AddItemToObject(pCurrJson_2Pub, "paramvals", pAccptedParams);
	}

	if(pUnkowndParams != NULL)
	{
		cJSON_AddItemToObject(pCurrJson_2Pub, "unsupported_params", pUnkowndParams);
	}

	//senderId
	if(szSenderId != NULL)
	{
		cJSON_AddStringToObject(pCurrJson_2Pub, "sender_id", szSenderId);
	}

	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_PARAM_CONF, pCurrJson_2Pub);

	return 0;
}

int Handle_Topic_SetReq(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *temp = NULL;
	cJSON *keyValList = NULL;
	//key-val[]
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "param_vals"))
	{
		if (cJSON_Array == temp->type)
		{
			keyValList = temp;
		}
	}

	if(keyValList == NULL)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is EMPTY 1!");
		return -1;
	}

	//judge the array-size
	int nParamTotal = cJSON_GetArraySize(keyValList);
	if(nParamTotal < 1)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is EMPTY 2!");
		return -1;
	}
	else if(nParamTotal > MBE_DEV_SUPPORT_MAX_MULTISET_ITEMS)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is too heavy!");
		return -1;
	}

	//sender_id
	memset(pMqtt->szSenderId_4PendingParamSet, 0, 256);
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "sender_id"))
	{
		if(temp->valuestring != NULL)
		{
			snprintf(pMqtt->szSenderId_4PendingParamSet, 256,  "%s", temp->valuestring);
		}
	}

	if(pMqtt->bHasPending_Params_2Set)
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 5); //5=reject for pending other cmd
		if(strlen(pMqtt->szSenderId_4PendingParamSet) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingParamSet);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_SET_CONF, pJReject);
		return -1;
	}
	else if(pMqtt->emMqttCommStatus != MBE_COMM_READY)
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 6); //5=reject for not-auth
		if(strlen(pMqtt->szSenderId_4PendingParamSet) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingParamSet);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_SET_CONF, pJReject);
		return -1;
	}
	
	//now parse the 
	int nTotalParsedKeyValList = 0;
	int n;
	MBE_MQTT_ASIG_KEY_VAL* pCurrKetVal;
	for(n = 0; n < nParamTotal; n++)
	{
		cJSON* pJAKeyVal = cJSON_GetArrayItem(keyValList, n);
		if(pJAKeyVal == NULL)
		{
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is INVALID #1!");
			return -1;			
		}
		temp = cJSON_GetObjectItem(pJAKeyVal, "key");
		if(temp == NULL)
		{
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is INVALID #2!");
			return -1;			
		}
		//now parse the sig with the key-name
		pCurrKetVal = &pMqtt->stPendingParams_KeyValList[nTotalParsedKeyValList];
		ZERO_POBJS(pCurrKetVal, 1);
		snprintf(pCurrKetVal->szRawKeyName, 32,  "%s", temp->valuestring); //to save raw key for sendig response
		pCurrKetVal->bIsParsedOK = Parse_ASig_FromMqtt_KeyName(pReporter, temp->valuestring, &pCurrKetVal->key);
		//printf("----parse set.req -> %d, [%s]----\n", pCurrKetVal->bIsParsedOK, temp->valuestring);
		temp = cJSON_GetObjectItem(pJAKeyVal, "val");
		if(temp == NULL)
		{
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is INVALID #3!");
			return -1;			
		}
		if(temp->valuestring == NULL)
		{
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is INVALID #4!");
			return -1;
		}
		pCurrKetVal->nExecuteResult = MBEDEV_EXESET_RESULT_NONE;
		snprintf(pCurrKetVal->szVal, 256,  "%s", temp->valuestring);
		nTotalParsedKeyValList++;
	}

	if(nTotalParsedKeyValList < 1)
	{
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param_vals] is is INVALID #5!");
		return -1;		
	}

	//now set the flag so that the params will be handled later
	pMqtt->nPending_Params_Count = nTotalParsedKeyValList;
	pMqtt->bHasPending_Params_2Set = TRUE;

	return 0;
}
 
int Handle_Topic_CtrlReq(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *temp = NULL;
	int nEventType = -1;
	//event type
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "type"))
	{
		if (cJSON_Number == temp->type)
		{
			nEventType = temp->valueint;
		}
	}

	if(nEventType < 0)
	{
		//make and send error msg??
		Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[type] is is EMPTY or INVALID!");
		return -1;		
	}

	//sender_id
	memset(pMqtt->szSenderId_4PendingCtrl, 0, 256);
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "sender_id"))
	{
		if(temp->valuestring != NULL)
		{
			snprintf(pMqtt->szSenderId_4PendingCtrl, 256,  "%s", temp->valuestring);
		}
	}

	if((nEventType <= MBEDEV_CTRLCMD_EVENTTYPE_NONE) || (nEventType >= MBEDEV_CTRLCMD_EVENTTYPE_MAX))
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 1); //1=reject for not supporting
		if(strlen(pMqtt->szSenderId_4PendingCtrl) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingCtrl);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CTRL_CONF, pJReject);
		return -1;	
	}
	else if(pMqtt->bHasPending_CtrlParam)
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 2); //2=reject for pending other cmd
		if(strlen(pMqtt->szSenderId_4PendingCtrl) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingCtrl);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CTRL_CONF, pJReject);
		return -1;		
	}	
	else if(pMqtt->emMqttCommStatus != MBE_COMM_READY)
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 3); //3=reject for not-auth
		if(strlen(pMqtt->szSenderId_4PendingCtrl) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingCtrl);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CTRL_CONF, pJReject);
		return -1;
	}
	

	pMqtt->stPendingCtrlCmd.emEventType = nEventType;
	pMqtt->stPendingCtrlCmd.nParamTotal = 0;
	//params
	cJSON* pJParamArr = NULL;
	if(temp = cJSON_GetObjectItem(pMqtt->pCurrJson_Sub, "param"))
	{
		if (cJSON_Array == temp->type)
		{
			pJParamArr = temp;
		}
	}

	if(pJParamArr != NULL)
	{
		int nParamTotal = cJSON_GetArraySize(pJParamArr);
		if(nParamTotal >2)
		{
			Send_Error_fromServPub(pMqtt, MBEDEV_ERRORMSG_CODE_DATAERR, pMqtt->emInHandling_SubMsg, "[param] size is INVALID!");
			return -1;					
		}
		//param-1
		if(temp = cJSON_GetArrayItem(pJParamArr, 0))
		{
			if(temp->valuestring != NULL)
			{
				snprintf(pMqtt->stPendingCtrlCmd.szParams[0], 256, "%s", temp->valuestring);
				pMqtt->stPendingCtrlCmd.nParamTotal++;
			}
		}
		//param-2
		if(temp = cJSON_GetArrayItem(pJParamArr, 1))
		{
			if(temp->valuestring != NULL)
			{
				snprintf(pMqtt->stPendingCtrlCmd.szParams[1], 256, "%s", temp->valuestring);
				pMqtt->stPendingCtrlCmd.nParamTotal++;
			}
		}
	}
	//now roughly judge if the event/param is valid or not
	BOOL bIsCtrlParamValid = FALSE;
	if(nEventType == MBEDEV_CTRLCMD_EVENTTYPE_DISAB_SYS)
	{
		if(pMqtt->stPendingCtrlCmd.nParamTotal >= 1)
		{
			if((pMqtt->stPendingCtrlCmd.szParams[0][0] == '0') || (pMqtt->stPendingCtrlCmd.szParams[0][0] == '1'))
			{
				bIsCtrlParamValid = TRUE;
			}
		}
	}
	else if(nEventType == MBEDEV_CTRLCMD_EVENTTYPE_DISAB_CONN)
	{
		if(pMqtt->stPendingCtrlCmd.nParamTotal == 2)
		{
			if((pMqtt->stPendingCtrlCmd.szParams[0][0] >= '0') && 
				(pMqtt->stPendingCtrlCmd.szParams[0][0] <= '3') &&
				((pMqtt->stPendingCtrlCmd.szParams[1][0] == '0') || (pMqtt->stPendingCtrlCmd.szParams[1][0] == '1')))
			{
				bIsCtrlParamValid = TRUE;
			}
		}
	}
	else if((nEventType == MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPDTFW) ||
		(nEventType == MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPLDLOG))
	{
		if(pMqtt->stPendingCtrlCmd.nParamTotal >= 1)
		{
			if((strstr(pMqtt->stPendingCtrlCmd.szParams[0], "ftp://") != NULL) || 
				(strstr(pMqtt->stPendingCtrlCmd.szParams[0], "ftps://") != NULL))
			{
				bIsCtrlParamValid = TRUE;
			}
		}
	}
	else 
	{
		bIsCtrlParamValid = TRUE;
	}

	if(!bIsCtrlParamValid)
	{
		cJSON* pJReject = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJReject, "result", 1); //1=reject for not supporting
		if(strlen(pMqtt->szSenderId_4PendingCtrl) > 0)
		{
			cJSON_AddStringToObject(pJReject, "sender_id", pMqtt->szSenderId_4PendingCtrl);
		}
		Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CTRL_CONF, pJReject);
		return -1;			
	}

	pMqtt->bHasPending_CtrlParam = TRUE;
	return 0;
}

int Handle_Topic_ServError(void* pArg)
{
	MQTT_MBE_DATA_ROUGH* pMqtt = (MQTT_MBE_DATA_ROUGH*)pArg;
	UNUSED(pMqtt);
	return 0;
}

int Execute_AParamSet_Item(MQTT_MBE_DATA_ROUGH* pMqtt, MBE_MQTT_ASIG_KEY_VAL* pParamKeyVal, int nTimeOut_mSec)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	VAR_VALUE varV;	
	int nValSize = 4;
	if(!pParamKeyVal->bIsParsedOK)
	{
		return MBEDEV_EXESET_RESULT_WRONG;
	}
	else if(pParamKeyVal->key.emSigType != PARAM_TYPE_SETTING)
	{
		return MBEDEV_EXESET_RESULT_WRONG;
	}

	if(pParamKeyVal->key.emValTYpe == VALUE_TYPE_BLOB)
	{
		return Set_ASubParamVal_FromKeyVal(pMqtt, pParamKeyVal, nTimeOut_mSec);
	}
	else if(pParamKeyVal->key.emValTYpe == VALUE_TYPE_FLOAT)
	{
		varV.fValue = atof(pParamKeyVal->szVal);				
	}
	else if(pParamKeyVal->key.emValTYpe == VALUE_TYPE_STRING)
	{
		varV.blobValue = pParamKeyVal->szVal;
		nValSize = 	pParamKeyVal->key.nValueSize;		
	}
	else if(pParamKeyVal->key.emValTYpe == VALUE_TYPE_DATETIME)
	{
		time_t tmSet = 0;		
		if(!Parse_ISO8601TimeStrEx(pParamKeyVal->szVal, &tmSet))
		{
			return MBEDEV_EXESET_RESULT_WRONG;
		}
		varV.ulValue = tmSet;
	}	
	// else if((pParamKeyVal->key.emValTYpe == VALUE_TYPE_LONG) ||
	// 	(pParamKeyVal->key.emValTYpe == VALUE_TYPE_ULONG) ||
	// 	(pParamKeyVal->key.emValTYpe == VALUE_TYPE_ENUM))
	else
	{
		varV.lValue = atoi(pParamKeyVal->szVal);				
	}

	int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					pParamKeyVal->key.nDev, pParamKeyVal->key.nSigId, 
					&varV, nValSize, nTimeOut_mSec);	

	return (nErr == ERRCODE_DAI_OK) ? MBEDEV_EXESET_RESULT_SUCCESS : MBEDEV_EXESET_RESULT_FAILED;
}

void ExeCute_DevCtrlCmd(MQTT_MBE_DATA_ROUGH* pMqtt)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	VAR_VALUE varV;	
	int nResult = 1; //1 means not support that cmd

	MBE_MQTT_ACTRLCMD* pCmd = &pMqtt->stPendingCtrlCmd;
	if((pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_SOFTRESET) ||
		(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_HARDRESET))
	{
		varV.emValue = (pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_HARDRESET) ? 1 : 0;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					1, 2, &varV, 4, 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_DISAB_SYS)
	{
		varV.emValue = (atoi(pCmd->szParams[0]) > 0) ? 1 : 0;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					200, 198, &varV, 4, 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_RESET_CCU)
	{
		varV.emValue = 0;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					200, 212, &varV, 4, 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	/*else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_DISAB_CONN)
	{
		//conn num
		int nConIdx = atoi(pCmd->szParams[0]) - 1;
		if((nConIdx < 0) || (nConIdx > 3))
		{
			nResult = 1; 
		}
		else
		{
			varV.emValue = (atoi(pCmd->szParams[1]) > 0) ? 1 : 0;
			int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
					201 + nConIdx, 106, &varV, 4, 3000);
			nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
		}
	}*/
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPDTFW)
	{
		A_BLOB_UPDATEFIRMWARE_CMD cmd;
		ZERO_POBJS(&cmd, 1);
		cmd.emBlobType = BLOB_TYPE_UPDATE_FIRMWARE_CMD;
		snprintf(cmd.szURL, 255,  "%s", pCmd->szParams[0]);
		cmd.nDownloadRetries = 3;
		cmd.dtRetrieveDate = _NOW_;

		varV.blobValue = &cmd;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_FTP_UPLDLOG)
	{
		A_BLOB_UPLOADDIAGN_CMD cmd;
		ZERO_POBJS(&cmd, 1);
		cmd.emBlobType = BLOB_TYPE_UPLOAD_DIAGNOSTIC;
		snprintf(cmd.szURL, 255,  "%s", pCmd->szParams[0]);
		cmd.nRetries = 3;
		//will only send all the diagnostic this time
		// cmd.dtStopTime = _NOW_;
		// int nGetStartTp = ato(pCmd->szParams[1]);
		// if(nGetStartTp == 2) //last month
		// {

		// }
		// else if(nGetStartTp == 1) //last week
		// {

		// }
		varV.blobValue = &cmd;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 9, &varV, sizeof(A_BLOB_UPLOADDIAGN_CMD), 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_VPN_START)
	{
		char szCmd_NPS[128];
		snprintf(szCmd_NPS, 128, "-server=%s -vkey=%s -type=tcp -log_level=0", pCmd->szParams[0], pCmd->szParams[1]);	
		varV.blobValue = szCmd_NPS;	
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				1, 21, &varV, 128, 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	else if(pCmd->emEventType == MBEDEV_CTRLCMD_EVENTTYPE_VPN_STOP)
	{
		varV.emValue = 0;
		int nErr = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				1, 22, &varV, 4, 5000);
		nResult = (nErr == ERRCODE_DAI_OK) ? 0 : 99; //0-success, 99-failed
	}
	
	cJSON* pJResult = cJSON_CreateObject();
	if(strlen(pMqtt->szSenderId_4PendingCtrl) > 0)
	{
		cJSON_AddStringToObject(pJResult, "sender_id", pMqtt->szSenderId_4PendingCtrl);
	}	
	cJSON_AddNumberToObject(pJResult, "result", nResult); 
	Publish_AMqttJsonMsg(pMqtt, MBE_MSG_CTRL_CONF, pJResult);	
}