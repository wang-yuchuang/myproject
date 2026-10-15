#include "es_mqtt.h"

static const char* s_strGetConfig[] = {
			"Charger_ID",
			"Server_URL",
			"Protect_SOC",
			"SysACInput_PwrLmt",
			"SysACInput_PwrLmtHV", // LJH240705 AitiOverLoad HVMeter
			"PwrCtrlMode", // LJH240911 AvgPwrFirst
			"PwrLmtMin_AvgPwrMode",
			"Min_ACLoad_StartDisChg",
			"Max_Output2AC_PwrLmt",
			"Min_Output2AC_PwrLmt",
			"PreProtect_RefluxAndOverLoad",
			"Output2AC_PwrRate",
			"UrgetAlterDischrgCurr",
			"AdjustUrgetModeCurr",
			"EnterUrgetDisChrgModePower",
			"EixtUrgetDischrgModePower", 
			"GridInputLimit_Mode",
			"FixedGridInput_PwrLmt", // Jeffery241205 Dynamic Grid Input Limit
			"PCS_HotStandby_Mode",
			"backend_reserved",
			"Cmd_101_Interval",
			"Cmd_102_Interval",
			"Cmd_103_Interval",
			"Cmd_104_Interval",
			"Cmd_105_Interval",
			"Cmd_106_Interval",
			"Cmd_107_Interval",
			"Cmd_108_Interval",
			"Cmd_109_Interval",
			"Cmd_110_Interval",
			"Cmd_114_Interval", // CXC 260122
			"Cmd_115_Interval", // CXC 260128
			"Cmd_202_Interval",
			"Cmd_204_Interval",
			"Cmd_206_Interval",
			"Cmd_155_Interval", 
			"Cmd_157_Interval", 
	};
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S111_TriggerMessage()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S111_TriggerMessage(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	TRACEX("---Handle_TriggerMessage--- \n");

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

  	if (temp1 = cJSON_GetObjectItem(pJson, "trig_msg"))
	{	 
		if(temp1->valuestring != NULL)
		{ 
			if (strcmp(temp1->valuestring, "101") == NULL)
			{
 				pMqtt->emTriggerMsgType = Trigger_C101;
			}
			else if(strcmp(temp1->valuestring, "102") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C102;
			}
			else if (strcmp(temp1->valuestring, "103") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C103;
			}
			else if (strcmp(temp1->valuestring, "105") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C105;
			}
			else if (strcmp(temp1->valuestring, "107") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C107;
			}
			else if (strcmp(temp1->valuestring, "206") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C206;
			}// edit by kuangyue 20250517
			else if (strcmp(temp1->valuestring, "114") == NULL)
			{
				pMqtt->emTriggerMsgType = Trigger_C114;
			}// CXC40058 20260121
			else 
			{
 				Send_CallError(pMqtt, S_111_Trigger, CallError_FormatError, "TriggleMsg is not recognized!");
				return -1;
			}
		}
	}
	else
	{
 		Send_CallError(pMqtt, S_111_Trigger, CallError_FormatError, "requestedMessage is missing!");
		return -1;
	}
	
	ERRCODE_DAI err;
	err = ERRCODE_DAI_OK;
	pResp = cJSON_CreateObject();
	//cJSON_AddStringToObject(pResp, "status", (pMqtt->emTriggerMsgType != Trigger_NoTriggle) ? "Accepted" : "Rejected");
	cJSON_AddNumberToObject(pResp, "accept", (pMqtt->emTriggerMsgType != Trigger_NoTriggle) ? 0 : 1);
	Send_CallResult(pMqtt, pResp, S_111_Trigger);

 
	switch (pMqtt->emTriggerMsgType)
	{
		case Trigger_C101:
			pMqtt->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C102:
			pMqtt->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C103:
			pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C105:
			pMqtt->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C107:
			pMqtt->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C206:
			pMqtt->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		case Trigger_C114:
			pMqtt->stMsgInfoList[C_114_PCSUnitInfo].pfnMsgHandler((void*)pMqtt);
			Publish_MqttMessage(pMqtt);
			break;
		default:
			break;
	}
	
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S401_Reset()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S401_Reset(void* pArg)
{
	// need to discuss two Reset 
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	TRACEX("Handle_Reset\n");

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;
	BOOL bHardReset = -1;

	char *szResult;
    szResult = cJSON_Print(pJson);
 
    printf("%s strlen(szResult) =%d\n",szResult, strlen(szResult));

	if (temp1 = cJSON_GetObjectItem(pJson, "rst_type"))
	{
		if (temp1->valueint == 1)
		{
			bHardReset = TRUE;
		}
		else if(temp1->valueint == 0)
		{
			bHardReset = FALSE;
		}
	}
	else
	{
		Send_CallError(pMqtt, S_401_RmtRest, CallError_FormatError, "Reset type is missing!");
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", 1/*bAccepted ? 1 : 2*/);
	
	Send_CallResult(pMqtt, pResp, S_401_RmtRest);
	//2021-12-01,Reply First, Then Reboot System

	//BOOL bAccepted = FALSE;
	if(bHardReset != -1)
	{
		VAR_VALUE varV;
		varV.emValue = bHardReset ? 1 : 0;
		ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 2, &varV,4, 3000);
		//bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	}
	else
	{
		Send_CallError(pMqtt, S_401_RmtRest, CallError_FormatError, "Reset type not correct!");
		return -1;
	}

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S402_ChangeEVSEAvailability()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S402_ChangeEVSEAvailability(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	cJSON *pJson = pMqtt->pCurrJson;
	
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	BOOL bAccpeted = FALSE;
	int AvalibilityStatus = -1;
	enum
	{
		ChangeAvail_Rejected = 0,
		ChangeAvail_Accepted,
		ChangeAvail_Scheduled,
	}Status;

	if ((temp1 = cJSON_GetObjectItem(pJson, "cmd_type")))
	{
		if (temp1->valueint == 1)
		{
			AvalibilityStatus = 0; //diable it
			bAccpeted = TRUE;
		}
		else if (temp1->valueint == 0)
		{
			AvalibilityStatus = 1; //enable it
			bAccpeted = TRUE;
		}
	}
	
	if (bAccpeted)
	{
		VAR_VALUE varV;
		varV.emValue = AvalibilityStatus;
		ERRCODE_DAI err;
		Status = ChangeAvail_Accepted; //need initialize, or else system will go crashed
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			200, 198, &varV,4, 1000);
		Status = (err == ERRCODE_DAI_OK) ? Status : ChangeAvail_Rejected;
		TRACE("Info: Status =%d err=%d\n", Status, err);
	}
	else
	{
		//Status = ChangeAvail_Rejected;
		Send_CallError(pMqtt, S_402_RmtChgEVESEAbi, CallError_FormatError, "cmd_type is missing!");
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", (Status == ChangeAvail_Accepted) ? 1 : 0);

	//Msg_Send(pMqtt, pResp, ChangeAvailability, CallResult,0, bSend_Later);
	Send_CallResult(pMqtt, pResp, S_402_RmtChgEVESEAbi);

	//Jimmy added 20190701 for OCTT test case: shall send StatusNotif.conf after handing this msg
	pMqtt->emTriggerMsgType = Trigger_C101;
	pMqtt->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pMqtt);

	return 0;
}
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S403_RemoteSOCManualCalibration()
*  CREATOR       : LZX
*  CREATED DATE  : 2026-02-06 10:13:53
*  DESCRIPTION   : For Electra Remote SOC Manual Calibration
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S403_RemoteSOCManualCalibration(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	cJSON *pJson = pMqtt->pCurrJson;
	
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	BOOL bAccpeted = FALSE;
	int AvalibilityStatus = -1;

	enum
	{
		ChangeAvail_Disable = 0,
		ChangeAvail_Enable,
		ChangeAvail_Rejected,
	}Status;

	if ((temp1 = cJSON_GetObjectItem(pJson, "soc_cmd_type")))
	{
		if (temp1->valueint == 0)
		{
			AvalibilityStatus = 0; //disable it
			bAccpeted = TRUE;
		}
		else if (temp1->valueint == 1)
		{
			AvalibilityStatus = 1; //enable it
			bAccpeted = TRUE;
		}
	}
	
	if (bAccpeted)
	{
		VAR_VALUE varV;
		varV.emValue = AvalibilityStatus;
		ERRCODE_DAI err;
		Status = ChangeAvail_Disable; //need initialize, or else system will go crashed
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			501, 604, &varV, 4, 1000);
		Status = (err == ERRCODE_DAI_OK) ? Status : ChangeAvail_Rejected;
	}
	else
	{
		Status = ChangeAvail_Rejected;
		Send_CallError(pMqtt, S_403_SOCCalibration, CallError_FormatError, "soc_cmd_type is missing!");
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", (Status == ChangeAvail_Rejected) ? 0 : 1);

	//Msg_Send(pMqtt, pResp, ChangeAvailability, CallResult,0, bSend_Later);
	Send_CallResult(pMqtt, pResp, S_403_SOCCalibration);

	//Jimmy added 20190701 for OCTT test case: shall send StatusNotif.conf after handing this msg
	pMqtt->emTriggerMsgType = Trigger_C103;
	pMqtt->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pMqtt);

	return 0;
}
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S404_UpdateFirmware()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Handle_S404_UpdateFirmware(void* pArg)
{
	// need to Check Later Yash 
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	TRACEX("Handle_UploadFirmware \n");

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *temp1;
	cJSON *pResp = NULL;
	A_BLOB_UPDATEFIRMWARE_CMD cmd;
	ZERO_POBJS(&cmd, 1);
	cmd.emBlobType = BLOB_TYPE_UPDATE_FIRMWARE_CMD;
	if (temp1 = cJSON_GetObjectItem(pJson, "url"))
	{
		int nStrlen = strlen(temp1->valuestring);
		if((nStrlen < 10) || (nStrlen > 255))
		{
			Send_CallError(pMqtt, S_404_RmtUpdtFirmw, CallError_FormatError, "location length is not correct!");
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
		//memcpy(cmd.szURL, temp1->valuestring, MIN(255, strlen(temp1->valuestring)));
	}
	else
	{
		Send_CallError(pMqtt, S_404_RmtUpdtFirmw, CallError_FormatError, "location is missing!");
		return -1;
	}

	cmd.dtRetrieveDate = 0;
	if (temp1 = cJSON_GetObjectItem(pJson, "inst_time"))
	{
		cmd.dtRetrieveDate = Parse_MqttTime(temp1->valuestring);
	}
	// else
	// {
	// 	Send_CallError(pMqtt, UpdateFirmware, CallError_FormationViolation, "retrieveDate is missing!", NULL);
	// 	return -1;
	// }
	cmd.nSemsIdx = -1;
	if( temp1 = cJSON_GetObjectItem(pJson, "ems_id"))
	{
		cmd.nSemsIdx = temp1->valueint;
	}

	ERRCODE_DAI err;
	VAR_VALUE varV;
	varV.blobValue = &cmd;
	if(cmd.nSemsIdx == -1) //如果是站控就把站控和从控一起全部升级
	{
		// printf("Upgrade All SEMS\n");
		cmd.dtRetrieveDate += 10; //延时10秒升级时间
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);	
		
		if(pMqtt->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
		{//如果是站控就把站控和从控一起全部升级
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
						200, 251, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);	
		}
		
	}
	else if(cmd.nSemsIdx == 0)
	{//升级站控
		// printf("Upgrade SEMS Master\n");
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);		
	}
	else
	{//升级单个从控
		// printf("Upgrade SEMS Slave %d\n", cmd.nSemsIdx);
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			200, 251, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);		
	}

	pResp = cJSON_CreateObject();		//resp is empty
	cJSON_AddNumberToObject(pResp, "accept", (err == ERRCODE_DAI_OK) ? 1 : 0);
	
	Send_CallResult(pMqtt, pResp, S_404_RmtUpdtFirmw);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C405_FirmwareStatusNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C405_FirmwareStatusNotification(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pJson_firmwarestatus;

	pJson_firmwarestatus = cJSON_CreateObject();
	// need to ask Jimmy for Firmware Status 
	//cJSON_AddStringToObject(pJson_firmwarestatus, "status", FIRMWARESTATUS_STRING[pMqtt->emFirmwareUpgradeStatus]);
	cJSON_AddNumberToObject(pJson_firmwarestatus, "status", pMqtt->emFirmwareUpgradeStatus);

	Send_Call(pMqtt, pJson_firmwarestatus, C_405_StatOfFirmw, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S406_GetDiagnostics()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S406_GetDiagnostics(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	TRACEX("Handle_UploadDiagnostic \n");

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *temp1;
	cJSON *pResp = NULL;
	A_BLOB_UPLOADDIAGN_CMD cmd;
	ZERO_POBJS(&cmd, 1);
	cmd.emBlobType = BLOB_TYPE_UPLOAD_DIAGNOSTIC;
	if (temp1 = cJSON_GetObjectItem(pJson, "url"))
	{
		int nStrlen = strlen(temp1->valuestring);
		if((nStrlen < 10) || (nStrlen > 255))
		{
			Send_CallError(pMqtt, S_406_RmtCallDiag, CallError_FormatError, "location length is not correct!");
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
	}
	else
	{
		Send_CallError(pMqtt, S_406_RmtCallDiag, CallError_FormatError, "location is missing!");
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "start_time"))
	{
		cmd.dtStartTime = Parse_MqttTime(temp1->valuestring);
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "stop_time"))
	{
		cmd.dtStopTime = Parse_MqttTime(temp1->valuestring);
	}

	char szTime[32];
	sprintf(cmd.szDiagnosticFileName, "diagnostic.%s", TimeToString(_NOW_, TIME_TRIM_FMT, szTime, 32));

	VAR_VALUE varV;
	varV.blobValue = &cmd;
	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, 9, &varV, sizeof(A_BLOB_UPLOADDIAGN_CMD), 0);

	pResp = cJSON_CreateObject();

	cJSON_AddStringToObject(pResp, "file_name", cmd.szDiagnosticFileName);

	Send_CallResult(pMqtt, pResp, S_406_RmtCallDiag);

	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare C407_DiagnosticsStatusNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C407_DiagnosticsStatusNotification(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pJson_Dignostics;

	pJson_Dignostics = cJSON_CreateObject();
	// need to ask Jimmy for Dignostics Status 
	cJSON_AddNumberToObject(pJson_Dignostics, "status", pMqtt->emDiagnosUploadStatus);
	//cJSON_AddStringToObject(pJson_Dignostics, "status", DIAGNOSTICSSTATUS_STRING[pMqtt->emDiagnosUploadStatus]);

	Send_Call(pMqtt, pJson_Dignostics, C_407_StatOfDiag, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

typedef enum 
{
	ES_iTYPE_INT,
	ES_iTYPE_BOOL,
	ES_iTYPE_FLOAT_DOT1, //1-dot float number
	ES_iTYPE_String64,
	ES_iType_48IntArray,
	ES_iType_GunMaxIntArray //gun-currMax array
}ES_iOCPP_CONFIG_TYPE;

typedef struct tagOCPPSettingsConfig
{
	char *ConfigName;
	BOOL  Readonly;
	ES_iOCPP_CONFIG_TYPE  ValueType;
	void  *ptrValue;

}GET_CHAANGE_CONFIG_PARA;

cJSON *GetConfig_Response(void* pArg,const char *data,BOOL bChangeCongfig, int nKeyIdx)
{
	TRACEX("Here is in Get Config \n");
	// Operation  0(Get Config ),1(Change settings)
	BOOL bAccpeted = FALSE;
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	cJSON *temp, *temp1;
	temp1 = pMqtt->pCurrJson;
	


	GET_CHAANGE_CONFIG_PARA TestOcppSettigs[] =
	{
		{"Charger_ID" ,					FALSE,  ES_iTYPE_String64 ,		(void*)(pMqtt->blobBackendSet.szCharger_ID) }, //0
		{"Server_URL" ,					FALSE,  ES_iTYPE_String64 ,		(void*)(pMqtt->blobBackendSet.szServer_URL) }, //0
		{"Protect_SOC" ,				FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nProtect_SOC) }, //0
		{"SysACInput_PwrLmt" ,			FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fSysACInput_PwrLmt) }, //0
		// LJH240705 AitiOverLoad HVMeter
		{"SysACInput_PwrLmtHV" ,		FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fSysACInput_PwrLmtHV) },
		// LJH240911 AvgPwrFirst
		{"PwrCtrlMode" ,			FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.emPowCtrlMode) },
		{"PwrLmtMin_AvgPwrMode" ,	FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fPwrLmtMin_AvgPwrMode) },
		{"Min_ACLoad_StartDisChg" ,		FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fMin_ACLoadPwr_Start_Dischg) }, //0
		{"Max_Output2AC_PwrLmt" ,		FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fMax_Output2AC_PwrLmt) }, //0
		{"Min_Output2AC_PwrLmt" ,		FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fMin_Output2AC_PwrLmt) }, //0
		{"PreProtect_RefluxAndOverLoad" ,		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fPreProt4RefluxAndOverLoad) },//1
		{"Output2AC_PwrRate" ,			FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fOutput2AC_PwrRate) },//1
		{"UrgetAlterDischrgCurr" ,		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fUrgetAlterDischrgCurr) },//1 
		{"AdjustUrgetModeCurr" ,		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fAdjustUrgetModeCurr) },//1 
		{"EnterUrgetDisChrgModePower",	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fEnter_UrgetDischrgPwr) },
		{"EixtUrgetDischrgModePower", 	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pMqtt->blobBackendSet.fExit_UrgetDischrgPwr) },
		// Jeffery241205 Dynamic Grid Input Limit
		{"GridInputLimit_Mode",			FALSE, 	ES_iTYPE_INT,			(void*)&(pMqtt->blobBackendSet.emDynamicGridInputLimit_Mode)},
		{"FixedGridInput_PwrLmt",		FALSE, 	ES_iTYPE_FLOAT_DOT1,	(void*)&(pMqtt->blobBackendSet.fFixed_GridInput_Limit)},
		{"PCS_HotStandby_Mode",			FALSE, 	ES_iTYPE_FLOAT_DOT1,	(void*)&(pMqtt->blobBackendSet.emPCS_HotStandby_Mode)},
		{"backend_reserved",			FALSE, 	ES_iTYPE_FLOAT_DOT1,	(void*)&(pMqtt->blobBackendSet.backend_reserved)},
		{"Cmd_101_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_101_Interval) },
		{"Cmd_102_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_102_Interval) },
		{"Cmd_103_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_103_Interval) },
		{"Cmd_104_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_104_Interval) },
		{"Cmd_105_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_105_Interval) },
		{"Cmd_106_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_106_Interval) },
		{"Cmd_107_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_107_Interval) },
		{"Cmd_108_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_108_Interval) },
		{"Cmd_109_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_109_Interval) },
		{"Cmd_110_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_110_Interval) },
		{"Cmd_114_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_114_Interval) },// CXC20260122
		{"Cmd_115_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_115_Interval) },// CXC20260128
		{"Cmd_200_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_200_Interval) },
		{"Cmd_202_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_202_Interval) },
		{"Cmd_204_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_204_Interval) },
		{"Cmd_206_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_206_Interval) },
		{"Cmd_155_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_155_Interval) },
		{"Cmd_157_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pMqtt->blobBackendSet.nCmd_157_Interval) },

	};
	GET_CHAANGE_CONFIG_PARA *pElementsList = &TestOcppSettigs[0];

	int n;
	BOOL bFoundKey = FALSE;
	for (n = 0; n < ITEM_OF(TestOcppSettigs) ; n++)
	{
		TRACE(" n = %d   %s data %s \n", n, pElementsList[n].ConfigName,data);
		if (strcmp(pElementsList[n].ConfigName, data) == 0)
		{
			TRACEX(" n = %d  %s \n",n, pElementsList[n].ConfigName);
			bFoundKey = TRUE;
			break;
		}
	}

	if (!bFoundKey)
	{
		TRACEX("Cannot get the key[%s]\n", data);
		return NULL;
	}

	temp = cJSON_CreateObject();

	cJSON *valGet1 = NULL; 
	char *szValOfKey = NULL;
	if (!bChangeCongfig) 
	{
		cJSON_AddStringToObject(temp, "name", pElementsList[n].ConfigName);
	}
	else
	{
		cJSON *pJSKeyValues = cJSON_GetObjectItem(temp1, "key_values");
		if(pJSKeyValues == NULL)
		{
			TRACEX("Error: Get pJSKeyValues Faield!\n");
			return NULL;
		}
		cJSON *pJsCurrItem = cJSON_GetArrayItem(pJSKeyValues, nKeyIdx);
		if(pJsCurrItem == NULL)
		{
			TRACEX("Can Not Get ChangeConfigArray Item nKeyIdx[%d]\n", nKeyIdx);
			return NULL;
		}
		if ((valGet1 = cJSON_GetObjectItem(pJsCurrItem, "val")))
		{
			TRACE("nKeyIdx=[%d] valGet1->valuestring=%s\n", nKeyIdx, valGet1->valuestring);
			if (valGet1->valuestring != NULL)
			{
				szValOfKey = valGet1->valuestring;
			}
		}
	}

	if (pElementsList[n].ValueType == ES_iTYPE_BOOL)
	{
		if(!bChangeCongfig)
		{
			int tempVal =  *((BOOL*)pElementsList[n].ptrValue);
			TRACEX("Get Config  %d \n ",tempVal);
			cJSON_AddStringToObject(temp, "val", tempVal ? "1" : "0" );
			TRACEX("Get Config  %d \n ", tempVal);
		}
		else
		{
			TRACEX("ChangeConfig Config  %d \n ");
			// for change config 
			{
				if (szValOfKey != NULL)
				{
					if ((strcmp(szValOfKey, "1") == 0))
					{
						TRACEX("ChangeConfig Config  %d \n ");
						int *tempVal = ((BOOL*)pElementsList[n].ptrValue);
						*tempVal = 1;
						TRACEX("ChangeConfig Config  %d \n ", *tempVal);
						 bAccpeted = TRUE;
					}
					else if ((strcmp(szValOfKey, "0") == 0))
					{
						if (pElementsList[n].Readonly == FALSE)
						{
							int *tempVal = ((BOOL*)pElementsList[n].ptrValue);
							*tempVal = 0;
							bAccpeted = TRUE;
						}
					}

				}
			}
		}
	}
	else if (pElementsList[n].ValueType == ES_iTYPE_INT)
	{
		if(!bChangeCongfig)
		{
			int tempVal =  *((int*)pElementsList[n].ptrValue);
			TRACEX("Get Config  %d \n ",tempVal);
			//cJSON_AddNumberToObject(temp, "value", tempVal);
			//Jimmy note 20190701: OCTT found the value shall always be STRING
			char szNum[64] = {0};
			sprintf(szNum, "%d", tempVal);
			cJSON_AddStringToObject(temp, "val", szNum);
		}
		else
		{
			{
				if (szValOfKey != NULL)
				{
					//Jimmy note: judge if this is a valid Number
					BOOL bIsValidNum = TRUE;
					int nValLen = strlen(szValOfKey);
					int kk;
					char s;
					for(kk = 0; kk < nValLen; kk++)
					{
						s = *(szValOfKey + kk);
						if((s < '0') || (s > '9'))
						{
							bIsValidNum = FALSE;
							break;
						}
					}

					if(bIsValidNum)
					{
						TRACEX("Value  %s \n", szValOfKey);
						int Value = atoi(szValOfKey);
						//Jimmy Note: value range will be verified by Set API itself
						if (pElementsList[n].Readonly == FALSE)
						{
							int *tempVal = ((UINT*)pElementsList[n].ptrValue);
							*tempVal = Value;
							bAccpeted = TRUE;
						}
					}
				}
			}
		}
	}
	else if (pElementsList[n].ValueType == ES_iTYPE_FLOAT_DOT1)
	{
		if(!bChangeCongfig)
		{
			float tempVal =  *((float*)pElementsList[n].ptrValue);
			TRACEX("Get Config  %d \n ",tempVal);
			//cJSON_AddNumberToObject(temp, "value", tempVal);
			//Jimmy note 20190701: OCTT found the value shall always be STRING
			char szNum[64] = {0};
			sprintf(szNum, "%.2f", tempVal);
			cJSON_AddStringToObject(temp, "val", szNum);
		}
		else
		{
			{
				if (szValOfKey != NULL)
				{
					//Jimmy note: judge if this is a valid Number
					BOOL bIsValidNum = TRUE;
					int nValLen = strlen(szValOfKey);
					int kk;
					char s;
					int nDot = 0;
					for(kk = 0; kk < nValLen; kk++)
					{
						s = *(szValOfKey + kk);
						if((s < '0') || (s > '9'))
						{
							if(s != '.')
							{
								bIsValidNum = FALSE;
								break;	
							}
							else if(nDot < 1)
							{
								nDot++;
							}
							else
							{
								bIsValidNum = FALSE;
								break;	
							}							
						}
					}

					if(bIsValidNum)
					{
						TRACEX("Value  %s \n", szValOfKey);
						float Value = atof(szValOfKey);
						//Jimmy Note: value range will be verified by Set API itself
						if (pElementsList[n].Readonly == FALSE)
						{
							float *tempVal = ((float*)pElementsList[n].ptrValue);
							*tempVal = Value;
							bAccpeted = TRUE;
						}
					}
				}
			}
		}
	}
	else if (pElementsList[n].ValueType == ES_iTYPE_String64)
	{
		if(!bChangeCongfig)
		{
			char* szVal =  (char*)pElementsList[n].ptrValue;
			TRACEX("Get Config  %d \n ",szVal);
			cJSON_AddStringToObject(temp, "val", szVal);
		}
		else
		{
			if (szValOfKey != NULL)
			{
				TRACEX(" Value  %s \n", szValOfKey);
				//Jimmy Note: value range will be verified by Set API itself
				if (pElementsList[n].Readonly == FALSE)
				{

					char *tempVal = ((char*)pElementsList[n].ptrValue);
					snprintf(tempVal, 256, "%s", szValOfKey);
					bAccpeted = TRUE;
					

				}
			}
		}
	}

	if(bChangeCongfig)
	{
		cJSON_Delete(temp);
		temp = NULL;
		if(bAccpeted)
		{
			temp = pMqtt->pCurrJson; //just for judge purpose
		}
	}

	return temp;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S411_GetConfiguration()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S411_GetConfiguration(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	int NoofConfig = -1;
	//OCPP_SENT_MSG msg;
	TRACEX("Here is the Handle_GetConfiguration = %d\n");
	cJSON *Arr;
	int n = 0;
	int TotalConfig = ITEM_OF(s_strGetConfig);
	if (temp1 = cJSON_GetObjectItem(pJson, "keys"))
	{
		NoofConfig = cJSON_GetArraySize(temp1);
		TRACEX("Handle_GetConfiguration = %d\n", NoofConfig = cJSON_GetArraySize(temp1));
		if (NoofConfig == 0) //means get all keys
		{
			TRACEX("Inside Handle_GetConfiguration = %d\n", NoofConfig = cJSON_GetArraySize(temp1));			
			Arr = cJSON_CreateArray();
			//for (n = 0; n < pMqtt->stOCPPSettings.nGetConfigMaxKeys_R; n++, nKeyIdx++)
			for (n = 0; n < TotalConfig ; n++)
			{
				cJSON *temp;
				temp = GetConfig_Response(pMqtt, s_strGetConfig[n], FALSE, n);
				cJSON_AddItemToArray(Arr, temp);
			}
			cJSON *Getconfig;
			Getconfig = cJSON_CreateObject();
			cJSON_AddItemToObject(Getconfig, "key_values", Arr);
			Send_CallResult(pMqtt, Getconfig, S_411_RmtGetConfig);
		}
		else //if (NoofConfig <= pMqtt->stOCPPSettings.nGetConfigMaxKeys_R)// that MAXGetConfigKey Consider Here 
		{
			cJSON *Arr;
			cJSON *Getconfig;
			Arr = cJSON_CreateArray();
			Getconfig = cJSON_CreateObject();
			cJSON_AddItemToObject(Getconfig, "key_values", Arr);
			cJSON *ArrUnknown = cJSON_CreateArray();
			int nUnknownItem = 0;
			int n;
			//int nSendKeys = 0;
			for (n = 0; n < MIN(TotalConfig, NoofConfig); n++)
			{
				cJSON *configKeyVal = cJSON_GetArrayItem(temp1, n);
				TRACEX("Handle_GetConfiguration = %s \n", configKeyVal->valuestring);
				cJSON *temp;
				temp = GetConfig_Response(pMqtt, configKeyVal->valuestring, 0, n);
				if(temp == NULL)
				{
					cJSON_AddItemToArray(ArrUnknown, cJSON_CreateString(configKeyVal->valuestring));
					nUnknownItem++;
				}
				else
				{
					///nSendKeys++;
					cJSON_AddItemToArray(Arr, temp);
				}
			}

			if(nUnknownItem < 1)
			{
				cJSON_Delete(ArrUnknown);
			}
			else
			{
				cJSON_AddItemToObject(Getconfig, "unkown_key", ArrUnknown);
			}

			//if(nSendKeys > 0)
			{
				Send_CallResult(pMqtt, Getconfig, S_411_RmtGetConfig);
			}
			// else
			// {
			// 	cJSON_Delete(Getconfig);
			// }
		}
	}
	else //also means get all keys
	{
		TRACEX("Get config All Keys\n");			
		Arr = cJSON_CreateArray();
		//for (n = 0; n < pMqtt->stOCPPSettings.nGetConfigMaxKeys_R; n++, nKeyIdx++)
		for (n = 0; n < TotalConfig ; n++)
		{
			cJSON *temp;
			temp = GetConfig_Response(pMqtt, s_strGetConfig[n], FALSE, n);
			cJSON_AddItemToArray(Arr, temp);
		}
		cJSON *Getconfig;
		Getconfig = cJSON_CreateObject();
		cJSON_AddItemToObject(Getconfig, "key_values", Arr);
		Send_CallResult(pMqtt, Getconfig, S_411_RmtGetConfig);
	}

	return 0;
	
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare Handle_S412_ChangeConfiguration()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S412_ChangeConfiguration(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	TRACEX("Handle_ChangeConfiguration \n");
	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *Resp = NULL;
	int NoofConfig = -1 ;

	if (temp1 = cJSON_GetObjectItem(pJson, "key_values"))
	{
		int nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > ITEM_OF(s_strGetConfig)))
		{
			Send_CallError(pMqtt, S_412_RmtSetConfig, CallError_FormatError, "key_value size NOT correct!");
			return -1;
		}

		TRACEX("Handle_Change_Configuration = %d \n", nItemTotal);
		int n;
		cJSON* pJsCurrItem = NULL;
		cJSON* pItemName = NULL;
		BOOL bAccept = TRUE;
		for(n = 0; n < nItemTotal; n++)
		{
			pJsCurrItem = cJSON_GetArrayItem(temp1, n);
			pItemName = cJSON_GetObjectItem(pJsCurrItem, "name");
			cJSON *Changeconfig = GetConfig_Response(pMqtt, pItemName->valuestring, 1, n);
			// Jeffery 20240823 同步修改 LJH240813 忽略不存在的参数，其他参数继续生效
			// if (Changeconfig == NULL)
			// {
			// 	bAccept = FALSE;
			// 	break;
			// }
		}
		
		//now update into OCPP structure and DB
		if (bAccept)
		{
			VAR_VALUE varV;
			ERRCODE_DAI err = -1;
			pMqtt->blobBackendSet.emBlobType = BLOB_TYPE_ES_BACKEND_CFG;
			varV.blobValue = &(pMqtt->blobBackendSet);
			bAccept = FALSE;
			//varV.blobValue = OCPP_Settings;
			//if ((pElementsList[n].Readonly == FALSE) && (bAccept))
			{
				err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
					1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG), 1000);
			}

			if (err == ERRCODE_DAI_OK)
			{
				bAccept = TRUE;
			}
		}

		if(!bAccept)
		{
			//Revert back OCPP settings
			PARAM_VAL_GET valGet;
			valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
			valGet.val.blobValue = (BYTE*)&pMqtt->blobBackendSet;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 26, &valGet, sizeof(PARAM_VAL_GET));
		}

		Resp = cJSON_CreateObject();
		cJSON_AddNumberToObject(Resp, "accept", bAccept ? 1 : 2);
		Send_CallResult(pMqtt, Resp, S_412_RmtSetConfig);
	}
	else
	{
		Send_CallError(pMqtt, S_412_RmtSetConfig, CallError_FormatError, "key_values is missing!");
		return -1;
	}

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S503_GetEMSPlan()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S503_GetEMSPlan(void* pArg)
{	
	TRACE("---Handle_S503_GetEMSPlan---\n");
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	int n = 0;
	PARAM_VAL_GET valGet;

	A_BLOB_EMS_CONFIG_PLAN emsPlan;
 	ZERO_POBJS(&emsPlan, 1);
	emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	valGet.val.blobValue = (BYTE*)&emsPlan;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "ems_plan_ver", emsPlan.nEMSMgmt_Ver);

	cJSON *pSector_Array = cJSON_CreateArray();
	for(n = 0; n < emsPlan.nTotal_EmsLmt_Sectors; n++)
	{
		cJSON *pSector_Obj = cJSON_CreateObject();

		char szTime[32];
		cJSON_AddStringToObject(pSector_Obj, "start_time", Make_MqttTimeString(pMqtt, emsPlan.emsLmtSectors[n].tmStart, szTime));

		cJSON_AddNumberToObject(pSector_Obj, "interval", 				emsPlan.emsLmtSectors[n].nInterval);
		cJSON_AddNumberToObject(pSector_Obj, "stop_soc", 				emsPlan.emsLmtSectors[n].nStopSOC);

		cJSON_AddFloatToObject(pSector_Obj,  "actv_pwr_lmt_rate", 		(float)(emsPlan.emsLmtSectors[n].nActvPwrLmt), 2);
		cJSON_AddFloatToObject(pSector_Obj,  "reactv_pwr_lmt_rate", 	(float)(emsPlan.emsLmtSectors[n].nReActvPwrLmt), 2);
		cJSON_AddFloatToObject(pSector_Obj,  "battgrp_pwr_lmt_rate", 	(float)(emsPlan.emsLmtSectors[n].nBattGrpPwrLmt), 2);

		cJSON_AddNumberToObject(pSector_Obj, "batt_workmode", 			emsPlan.emsLmtSectors[n].emBatt_WorkMode);
		// cJSON_AddNumberToObject(pSector_Obj, "feerate_attri", 			emsPlan.emsLmtSectors[n].emFeeRate); // Not to upload feerate in Q503, updated by Jeffery 20240723
 		cJSON_AddNumberToObject(pSector_Obj, "dynamic_grid_input_lmt", 	emsPlan.emsLmtSectors_ext[n].nGridInputPwrLmt);

		cJSON_AddItemToArray(pSector_Array, pSector_Obj);
	}

	cJSON_AddItemToObject(pRoot, "ems_plan_cfg", pSector_Array);
	cJSON_AddNumberToObject(pRoot, "ems_plan_repeat", 1);
	
	Send_CallResult(pMqtt, pRoot, S_503_RmtGetEMSPlan);

 	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S504_SetEMSPlanProfile()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S504_SetEMSPlanProfile(void* pArg)
{
	TRACE("---Handle_S504_SetEMSPlanProfile---\n");
	
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	int n = 0, nProtectSOC = 0;
	int nTotalIntervalTime = 0;
	int nItemTotal;
	BOOL bAccepted = FALSE;
 	char szErrString[64];
	UINT nRecvEMSVer = 0;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				500, 503, &valGet, sizeof(PARAM_VAL_GET));
 	nProtectSOC = valGet.val.ulValue;

	A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
	ZERO_POBJS(&blobEmsCfg, 1);
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	valGet.val.blobValue = (BYTE*)&blobEmsCfg;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
 
 	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_ver"))
	{
		if(cJSON_Number == temp1->type)
		{
			nRecvEMSVer = (UINT)temp1->valuedouble;
		}
		else if(cJSON_String == temp1->type)
		{
			nRecvEMSVer = myAtoi(temp1->valuestring);
		}
		else
		{
			Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_ver Wrong DataType!");
			return -1;
		}

		if(nRecvEMSVer != blobEmsCfg.nEMSMgmt_Ver)
		{
			blobEmsCfg.nEMSMgmt_Ver = nRecvEMSVer;
		}
		else
		{
			TRACEX(" nRecvEMSVer=[%d] blobEmsCfg.nEMSMgmt_Ver=[%d]\n", nRecvEMSVer, blobEmsCfg.nEMSMgmt_Ver);
			Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_ver is Lower Or Some!");
			return -1;
		}

		TRACE("nRecvEMSVer =%d nEMSMgmt_Ver=%d temp1->type=[%d]\n", nRecvEMSVer, blobEmsCfg.nEMSMgmt_Ver, temp1->type);
	}
	else
	{
		Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_ver is Missing!");
		return -1;
	}
	
	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_cfg"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > MAX_EMSMGMT_LMT_SETOR))
		{
			TRACE("nItemTotal =%d\n", nItemTotal);
			Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_cfg Sector Number is INVALID!");
			return -1;
		}

		blobEmsCfg.nTotal_EmsLmt_Sectors = nItemTotal;

		TRACE("Handle_S504_SetEMSPlanProfile nItemTotal= %d \n", nItemTotal);
		
		int n;
		cJSON* pJsCurrItem = NULL;
		cJSON* pItemName = NULL;
		bAccepted = TRUE;
 		for(n = 0; n < nItemTotal; n++)
		{
			pJsCurrItem = cJSON_GetArrayItem(temp1, n);
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "start_time"))
			{
				if(cJSON_String != pItemName->type)
				{
					sprintf(szErrString, "%s", "Error:start_time Fromat isn't String!");
					bAccepted = FALSE;
					break;
				} 
				else
				{
					blobEmsCfg.emsLmtSectors[n].tmStart = Parse_MqttTime(pItemName->valuestring);
				}
			}
			else
			{
				sprintf(szErrString, "%s", "Error:start_time is Missing");
				bAccepted = FALSE;
				break;
			}
			
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "interval"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nInterval = pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nInterval = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:interval DataType Shoudle Number/String!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "Error:interval is Missing");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "batt_workmode"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode = (STBATT_PRI_WORKMODE)pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:batt_workmode DataType should Number/String!");	
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "Error:batt_workmode is Missing");	
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "stop_soc"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nStopSOC = pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nStopSOC = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:stop_soc DataType should Number/String!");
					bAccepted = FALSE;
					break;
				}

				if((blobEmsCfg.emsLmtSectors[n].nStopSOC < 0) || 
					(blobEmsCfg.emsLmtSectors[n].nStopSOC > 100))
				{
					sprintf(szErrString, "%s", "Error:stop_soc Range Should From 0~100%%.");
					bAccepted = FALSE;
					break;
				}
			} 
			else
			{
				sprintf(szErrString, "%s", "Error:stop_soc is Missing");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "feerate_attri"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].emFeeRate = (FEERATE_ATTRI)pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].emFeeRate = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:feerate_attri DataType should Number/String!");
					bAccepted = FALSE;
					break;
				}

				if((blobEmsCfg.emsLmtSectors[n].emFeeRate < FEERATE_SHARP) || 
					(blobEmsCfg.emsLmtSectors[n].emFeeRate > FEERATE_VALLEY))
				{
					sprintf(szErrString, "%s", "Error:feerate_attri Range Should From 0~3.");
					bAccepted = FALSE;
					break;
				}
			} 
			//2022-02-24 John Remove FeeRate Section Check
			//else
			//{
			//	sprintf(szErrString, "%s", "Error:feerate_attri is Missing");
			//	bAccepted = FALSE;
			//	break;
			//}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "actv_pwr_lmt_rate"))
 			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nActvPwrLmt = (UINT)(pItemName->valuedouble);

				}
				else if(cJSON_String == pItemName->type)
				{					
					blobEmsCfg.emsLmtSectors[n].nActvPwrLmt = (UINT)(atof(pItemName->valuestring));
				}
				else
				{
					sprintf(szErrString, "%s", "actv_pwr_lmt_rate DataType Should Number/String");
					bAccepted = FALSE;
					break;
				}

				if((blobEmsCfg.emsLmtSectors[n].nActvPwrLmt < 0) || 
					(blobEmsCfg.emsLmtSectors[n].nActvPwrLmt > 999999))
				{
					sprintf(szErrString, "%s", "actv_pwr_lmt_rate out of Range[0,1] or Invalid!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "actv_pwr_lmt_rate is Missing!");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "reactv_pwr_lmt_rate"))
 			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt = (int)(pItemName->valuedouble);
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt = (int)(atof(pItemName->valuestring));
				}
				else
				{
					sprintf(szErrString, "%s", "reactv_pwr_lmt_rate out of Range or Invalid!");
					bAccepted = FALSE;
					break;
				}

				//if((blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt < 0) || 
				//	(blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt > 100))
				//{
				//	sprintf(szErrString, "%s", "reactv_pwr_lmt_rate out of Range[0,1] or Invalid!");
				//	bAccepted = FALSE;
				//	break;
				//}
			}
			else
			{
				sprintf(szErrString, "%s", "reactv_pwr_lmt_rate is Missing!");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "battgrp_pwr_lmt_rate"))
 			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt = (UINT)(pItemName->valuedouble);
				}
				else if(cJSON_String == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt = (UINT)(atof(pItemName->valuestring));
				}
				else
				{
					sprintf(szErrString, "%s", "battgrp_pwr_lmt_rate DataType should Number/String!");
					bAccepted = FALSE;
					break;
				}

				if((blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt < 0) || (blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt > 999999))
				{
					sprintf(szErrString, "%s", "battgrp_pwr_lmt_rate out of Range[0,1] or Invalid!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "battgrp_pwr_lmt_rate is Missing!");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "dynamic_grid_input_lmt"))
 			{
				if(cJSON_Number == pItemName->type)
				{
					blobEmsCfg.emsLmtSectors_ext[n].nGridInputPwrLmt = (UINT)(pItemName->valuedouble);

				}
				else if(cJSON_String == pItemName->type)
				{					
					blobEmsCfg.emsLmtSectors_ext[n].nGridInputPwrLmt = (UINT)(atof(pItemName->valuestring));
				}
				else
				{
					sprintf(szErrString, "%s", "dynamic_grid_input_lmt DataType Should Number/String");
					bAccepted = FALSE;
					break;
				}

				if((blobEmsCfg.emsLmtSectors_ext[n].nGridInputPwrLmt < 0) || 
					(blobEmsCfg.emsLmtSectors_ext[n].nGridInputPwrLmt > 999999))
				{
					sprintf(szErrString, "%s", "dynamic_grid_input_lmt out of Range or Invalid!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				// sprintf(szErrString, "%s", "dynamic_grid_input_lmt is missing, set to 0!");
				blobEmsCfg.emsLmtSectors_ext[n].nGridInputPwrLmt = 0;
			}
		
			if(blobEmsCfg.emsLmtSectors[n].nStopSOC < nProtectSOC)
			{
				SYSTEM_TYPE emSystemType = pMqtt->lcCfg.emSystemType;
				if(emSystemType == SYSTYPE_ESS_AIR || emSystemType == SYSTYPE_ESS_LIQUIDED)
				{
					sprintf(szErrString, "%s:Idx=[%d]Stop=[%d]Prot=[%d]", "nStop_SOC is Below nProtectSOC", n, blobEmsCfg.emsLmtSectors[n].nStopSOC, nProtectSOC);
					bAccepted = FALSE;
					break;	
				}
			}

			TRACE("Item[%d] start_time=%d interval=%d batt_Priworkmode=%d \
				nActvPwrLmt=%d nReActvPwrLmt=%d nStopSOC=%d emFeeRate=%d\n", \
				n,blobEmsCfg.emsLmtSectors[n].tmStart, \
				blobEmsCfg.emsLmtSectors[n].nInterval, \ 
				blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode,\
				blobEmsCfg.emsLmtSectors[n].nActvPwrLmt, \
				blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt, \
				blobEmsCfg.emsLmtSectors[n].nStopSOC, \
				blobEmsCfg.emsLmtSectors[n].emFeeRate);
		}
	}	

	if(!bAccepted)
	{
		Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, szErrString);
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_repeat"))
	{
		if((cJSON_Number != temp1->type) && (cJSON_String != temp1->type))
		{
			Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "chgplan_repeat Type is Wrong!");
			return -1;
		}
		else
		{
			//blobEmsCfg.emBattChgPlan_Repeat = temp1->valueint;
			for(n= 0; n < nItemTotal; n++)
			{
				nTotalIntervalTime += blobEmsCfg.emsLmtSectors[n].nInterval;
			}
			TRACE("Info: nItemTotal=%d nTotalIntervalTime=%d\n", nItemTotal, nTotalIntervalTime);
			if(nTotalIntervalTime % DAILY_INTERVAL_TIME != 0)
			{
				Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "Daily Total interval is Wrong!");
				return -1;
			}
		}
	}
	else
	{
		for(n= 0; n < nItemTotal; n++)
		{
			nTotalIntervalTime += blobEmsCfg.emsLmtSectors[n].nInterval;
		}
		TRACE("Info: chgplan_repeat =%d nItemTotal=%d nTotalIntervalTime=%d\n", nItemTotal, nTotalIntervalTime);

		if(nTotalIntervalTime % DAILY_INTERVAL_TIME != 0)
		{
			Send_CallError(pMqtt, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "Daily Total interval is Wrong!");
			return -1;
		}
	}
	
	VAR_VALUE varV;
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	varV.blobValue = &blobEmsCfg;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACEX("Info: EMS Config Plan Set Sucessfully\n");
	}
	else
	{
		TRACEX("Error: EMS Config Plan Update Failure!\n");
		//Revert EMS Config Settings
		valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
		valGet.val.blobValue = (BYTE*)&blobEmsCfg;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
	}

	cJSON *pRep = cJSON_CreateObject();
 	cJSON_AddNumberToObject(pRep, "accept", (err == ERRCODE_DAI_OK) ? 1 : 0);
	Send_CallResult(pMqtt, pRep, S_504_RmtSetEMSPlanProfile);
 
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S601_GetFeeRate()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S601_GetFeeRate(void* pArg)
{
	TRACEX("---Handle Get S601 blobFeeRateSet---\n");

	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	PARAM_VAL_GET valGet;
	//ERRCODE_DAI err = -1;

	int n = 0;
 	A_BLOB_CHARGE_FEESET blobFeeRateSet;
	ZERO_POBJS(&blobFeeRateSet, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGE_FEESET);
	valGet.val.blobValue = (BYTE*)&blobFeeRateSet;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));

	cJSON *pRoot = cJSON_CreateObject();
	
	cJSON *pSector_Array = cJSON_CreateArray();
	for(n = 0; n < blobFeeRateSet.nTotal_FeeRate_Sectors; n++)
	{
		cJSON *pSector_Obj = cJSON_CreateObject();

		char szTime[32];
		cJSON_AddStringToObject(pSector_Obj, "start_time", 		Make_MqttTimeString(pMqtt, blobFeeRateSet.feeRateSectors[n].tmStart, szTime));
		cJSON_AddNumberToObject(pSector_Obj, "interval", 		blobFeeRateSet.feeRateSectors[n].nInterval);
		cJSON_AddNumberToObject(pSector_Obj, "feerate_attri", 	blobFeeRateSet.feeRateSectors[n].emFeeRate);
		cJSON_AddFloatToObject(pSector_Obj,  "fee_price", 		blobFeeRateSet.feeRateSectors[n].fPrice, 4);
 
		cJSON_AddItemToArray(pSector_Array, pSector_Obj);
	}

	cJSON_AddItemToObject(pRoot, "fee_rate_cfg", pSector_Array);
 	
	Send_CallResult(pMqtt, pRoot, S_601_RmtGetFeeRateCfg);

 	return 0;

}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Handle_S602_SetFeeRateProfile()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S602_SetFeeRateProfile(void* pArg)
{
    TRACE("---Handle_S602_SetFeeRateProfile---\n");
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;

	cJSON *pJson = pMqtt->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	int n = 0, nTotalIntervalTime = 0, nItemTotal = 0;
	BOOL bAccepted = FALSE;
 	char szErrString[64];

 	A_BLOB_CHARGE_FEESET blobFeeRateSet;
	ZERO_POBJS(&blobFeeRateSet, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGE_FEESET);
	valGet.val.blobValue = (BYTE*)&blobFeeRateSet;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));

	if (temp1 = cJSON_GetObjectItem(pJson, "fee_rate_cfg"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > MAX_EMSMGMT_LMT_SETOR))
		{
			TRACE("nItemTotal =%d\n", nItemTotal);
			Send_CallError(pMqtt, S_602_RmtSetFeeRateProfile, CallError_FormatError, "fee_rate_cfg Sector Number is INVALID!");
			return -1;
		}

		blobFeeRateSet.nTotal_FeeRate_Sectors = nItemTotal;

		TRACE("Handle_S602_SetFeeRateProfile nItemTotal= %d \n", nItemTotal);
		
 		cJSON* pJsCurrItem = NULL;
		cJSON* pItemName = NULL;
		bAccepted = TRUE;
 		for(n = 0; n < nItemTotal; n++)
		{
			pJsCurrItem = cJSON_GetArrayItem(temp1, n);
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "start_time"))
			{
				if(cJSON_String != pItemName->type)
				{
					sprintf(szErrString, "%s", "Error:start_time Fromat isn't String!");
					bAccepted = FALSE;
					break;
				} 
				else
				{
					blobFeeRateSet.feeRateSectors[n].tmStart = Parse_MqttTime(pItemName->valuestring);
				}
			}
			else
			{
				sprintf(szErrString, "%s", "Error:start_time is Missing");
				bAccepted = FALSE;
				break;
			}
			
			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "interval"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].nInterval = pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].nInterval = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:interval Fromat Isn't Number!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "Error:interval is Missing");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "feerate_attri"))
			{
				if(cJSON_Number == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].emFeeRate = (FEERATE_ATTRI)pItemName->valueint;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].emFeeRate = myAtoi(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "Error:feerate_attri Range Should Number/String");
					bAccepted = FALSE;
					break;
				}

				if((blobFeeRateSet.feeRateSectors[n].emFeeRate < FEERATE_SHARP) || 
					(blobFeeRateSet.feeRateSectors[n].emFeeRate > FEERATE_VALLEY))
				{
					sprintf(szErrString, "%s", "Error:feerate_attri Range Should From [0,3].");
					bAccepted = FALSE;
					break;
				}
			} 
			else
			{
				sprintf(szErrString, "%s", "Error:feerate_attri is Missing");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "fee_price"))
 			{
				if(cJSON_Number == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].fPrice = pItemName->valuedouble;
				}
				else if(cJSON_String == pItemName->type)
				{
					blobFeeRateSet.feeRateSectors[n].fPrice = atof(pItemName->valuestring);
				}
				else
				{
					sprintf(szErrString, "%s", "fee_price DataType Should Number/String!");
					bAccepted = FALSE;
					break;
				}

				if((blobFeeRateSet.feeRateSectors[n].fPrice < 0) || (blobFeeRateSet.feeRateSectors[n].fPrice > 10000.0))
				{
					sprintf(szErrString, "%s", "fee_price out of Range[0,100] or Invalid!");
					bAccepted = FALSE;
					break;
				}
			}
			else
			{
				sprintf(szErrString, "%s", "fee_price is Missing!");
				bAccepted = FALSE;
				break;
			}

			TRACE("Item[%d] start_time=[%d] interval=[%d] emFeeRate=[%d] fPrice=[%f]\n", \
				n,
				blobFeeRateSet.feeRateSectors[n].tmStart, \
				blobFeeRateSet.feeRateSectors[n].nInterval, \ 
				blobFeeRateSet.feeRateSectors[n].emFeeRate,\
				blobFeeRateSet.feeRateSectors[n].fPrice);
		}
	}	

	if(!bAccepted)
	{
		Send_CallError(pMqtt, S_602_RmtSetFeeRateProfile, CallError_FormatError, szErrString);
		return -1;
	}

	for(n= 0; n < nItemTotal; n++)
	{
		nTotalIntervalTime += blobFeeRateSet.feeRateSectors[n].nInterval;
	}
	TRACE("Info: nItemTotal=%d nTotalIntervalTime=%d\n", nItemTotal, nTotalIntervalTime);

	if(nTotalIntervalTime != DAILY_INTERVAL_TIME)
	{
		Send_CallError(pMqtt, S_602_RmtSetFeeRateProfile, CallError_FormatError, "FeeRate Total interval is Wrong!");
		return -1;
	}
	
	VAR_VALUE varV;
	blobFeeRateSet.emBlobType = BLOB_TYPE_CHARGE_FEE_SET;
	varV.blobValue = &blobFeeRateSet;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			1, 12, &varV, sizeof(A_BLOB_CHARGE_FEESET), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACEX("Info:FeeRate Cfg Set Sucessfully\n");
	}
	else
	{
		TRACEX("Error:FeeRate Cfg Update Failure!\n");
		//Revert FeeRate Config Settings
		valGet.nGetSize = sizeof(A_BLOB_CHARGE_FEESET);
		valGet.val.blobValue = (BYTE*)&blobFeeRateSet;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 12, &valGet, sizeof(PARAM_VAL_GET));
	}

	cJSON *pRep = cJSON_CreateObject();
 	cJSON_AddNumberToObject(pRep, "accept", (err == ERRCODE_DAI_OK) ? 1 : 0);
	Send_CallResult(pMqtt, pRep, S_602_RmtSetFeeRateProfile);

	return 0;
}


 
