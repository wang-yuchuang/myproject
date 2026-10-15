#include "maitain_be.h"

char* Make_MaintainBeTimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	snprintf(timeinstring, 32, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

time_t  Parse_MaintainBeTime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

int Prepare_C100_Regsiter(void* pArg)
{
	TRACEX("Prepare_100_Register \n");

	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pArg;

	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	
	cJSON *root;
	/* Get Info for BootNotification */
	PARAM_VAL_GET valGet;
	A_BLOB_CHARGER_BOOTINFO bootInfo;
	valGet.nGetSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
	valGet.val.blobValue = (BYTE*)&bootInfo;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 17, &valGet, sizeof(PARAM_VAL_GET));
	
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("Prepare_BootNotification -> Get Data fail \n");
		return -1;
	}

	//save the current SN
	snprintf(pOCPP->szCurrChargerSN, 32,  "%s", bootInfo.szChargePointSerualNuml);
	root = cJSON_CreateObject();

	//cJSON_AddStringToObject(root, "chargePointModel", bootInfo.szChargePointModel);
	cJSON_AddStringToObject(root, "model", bootInfo.szChargePointModel);
	cJSON_AddStringToObject(root, "sn", bootInfo.szChargePointSerualNuml);	
	cJSON_AddNumberToObject(root, "sys_type", 0); //always be zero this time
	
	Send_Call(pOCPP, C_100_Register, root);

	return 0;
}

int Prepare_C101_VersionInfo(void* pArg)
{
	TRACEX("Prepare_101_VersionInfo \n");

	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pArg;

	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	
	cJSON *root;
	/* Get Info for BootNotification */
	PARAM_VAL_GET valGet;
	//A_BLOB_CHARGERSPECIFIC_SETTINGS ccuSet;
	valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
	valGet.val.blobValue = (BYTE*)&pOCPP->blbCurrCCUSet;
	int err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		200, 200, &valGet, sizeof(PARAM_VAL_GET));	
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Prepare_C101_VersionInfo -> Get Data fail \n");
		return -1;
	}

	char szIMMU2_SWVer[MAX_LEN_MU2SWVER] = {0};
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R,
			0, 0, szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	char szIMMU2Ver[32] = {0};		
	snprintf(szIMMU2Ver, 32, "%s(%s)", szIMMU2_SWVer, GetIMMU2_HWVersion());
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "main_ctr_ver", szIMMU2_SWVer);
	cJSON_AddStringToObject(root, "ext_ctr_ver", pOCPP->blbCurrCCUSet.ccuSettings.szSWVer);
	//other-dev ver (power module)
	cJSON *pJOtherDevVer = cJSON_CreateArray();
	/*valGet.nGetSize = 4;
	valGet.val.ulValue = 0;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		250, 149, &valGet, sizeof(PARAM_VAL_GET));*/
	//if(valGet.val.ulValue > 0)
	{
		char szChargerID[64];
		valGet.nGetSize = 64;
		valGet.val.blobValue = (BYTE*)szChargerID;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 2, &valGet, sizeof(PARAM_VAL_GET));
		cJSON *pChargerID = cJSON_CreateObject();
		cJSON_AddStringToObject(pChargerID, "DevID:", szChargerID);
		cJSON_AddItemToArray(pJOtherDevVer, pChargerID);

		char szBattVer[32];
		A_BLOB_SINGLE_STBATT_RUNINFO stBattDataInfo;
		ZERO_POBJS(&stBattDataInfo, 1);
		valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		valGet.val.blobValue = (BYTE*)&stBattDataInfo;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				501, 551, &valGet, sizeof(PARAM_VAL_GET));
		if(stBattDataInfo.fVersion > 0)
		{
			snprintf(szBattVer, 32, "%.3f", stBattDataInfo.fVersion);//edit by Kuangyue 20250227
		}
		else
		{
			sprintf(szBattVer, "--");
		}
 
		cJSON *pBMSVer = cJSON_CreateObject();
		cJSON_AddStringToObject(pBMSVer, "BMS Ver:", szBattVer);
		cJSON_AddItemToArray(pJOtherDevVer, pBMSVer);

		/*int nScannedRectTotal = valGet.val.lValue;
		valGet.nGetSize = 4;
		valGet.val.lValue = 0;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			250, 150, &valGet, sizeof(PARAM_VAL_GET));
		char szRectNum[16];
		snprintf(szRectNum, 16, "%d/%d", valGet.val.lValue, nScannedRectTotal);
		//online/scanned total rectifiers
		cJSON *pJScanOnlineTotal = cJSON_CreateObject();
		cJSON_AddStringToObject(pJScanOnlineTotal, "Online/Scan Retifiers", szRectNum);
		cJSON_AddItemToArray(pJOtherDevVer, pJScanOnlineTotal);
		//rectifiers overview DC/PFC ver
		valGet.nGetSize = 4;
		valGet.val.lValue = 0;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));
		float fDCVer = valGet.val.fValue;
		valGet.nGetSize = 4;
		valGet.val.lValue = 0;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 13, &valGet, sizeof(PARAM_VAL_GET));
		char szRectOverviewVer[32];
		snprintf(szRectOverviewVer, 32, "%.3f/%.3f", fDCVer, valGet.val.fValue);
		cJSON *pJDCPFCVer = cJSON_CreateObject();
		cJSON_AddStringToObject(pJDCPFCVer, "DC/PFC OverView Ver", szRectOverviewVer);
		cJSON_AddItemToArray(pJOtherDevVer, pJDCPFCVer);*/		
	}	
	cJSON_AddItemToObject(root, "other_dev_ver", pJOtherDevVer);
	//net-info
	A_BLOB_NETWORKINFO netInfo;
	valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
	valGet.val.blobValue = (BYTE*)&netInfo;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 3, &valGet, sizeof(PARAM_VAL_GET));	
	if(err == ERRCODE_DAI_OK)
	{
		memcpy(&pOCPP->blbCurrNetInfo, &netInfo, valGet.nGetSize);
		cJSON *pJNetInfo = cJSON_CreateArray();
		//wireless typr
		cJSON *pJWirelessType = cJSON_CreateObject();
		cJSON_AddStringToObject(pJWirelessType, "wireless_tp", (netInfo.bIsEnable_LTEModem == 2) ? "ZR2000-Ext.Router(to ETH1)" :
			((netInfo.bIsEnable_LTEModem == 1) ? "ME3630-Int.LTEModem" : "Disabled"));
		cJSON_AddItemToArray(pJNetInfo, pJWirelessType);
		if(netInfo.bIsEnable_LTEModem > 0)
		{
			//LTE-IMSI&ICCID
			cJSON *pJIMSI = cJSON_CreateObject();
			cJSON_AddStringToObject(pJIMSI, "IMSI", netInfo.lteModemINfo.szIMSI);
			cJSON_AddItemToArray(pJNetInfo, pJIMSI);
			cJSON *pJICCID = cJSON_CreateObject();
			cJSON_AddStringToObject(pJICCID, "ICCID", netInfo.lteModemINfo.szICCID);
			cJSON_AddItemToArray(pJNetInfo, pJICCID);
			//net type
			cJSON *pJNetType = cJSON_CreateObject();
			cJSON_AddStringToObject(pJNetType, "NetType", netInfo.lteModemINfo.szNetwork_Type);
			cJSON_AddItemToArray(pJNetInfo, pJNetType);
			//signal strength
			cJSON *pJSigStrength = cJSON_CreateObject();
			char szSigStr[8];
			if(netInfo.lteModemINfo.nSignal_dBm >= 0)
			{
				snprintf(szSigStr, 8, "rssi %d", netInfo.lteModemINfo.nSignal_dBm);
			}
			else
			{
				snprintf(szSigStr, 8, "%d dBm", netInfo.lteModemINfo.nSignal_dBm);
			}			
			cJSON_AddStringToObject(pJSigStrength, "SignalStrength", szSigStr);
			cJSON_AddItemToArray(pJNetInfo, pJSigStrength);
		}
		cJSON_AddItemToObject(root, "net_info", pJNetInfo);
	}

	Send_Call(pOCPP, C_101_VersionInfo, root);

	return 0;
}

int Prepare_C102_StatusNotif(void* pArg)
{
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;
	cJSON *pJson_Status = cJSON_CreateObject();
	
	TRACEX("Prepare_C102_StatusNotif \n");
	//time stamp
	char szTime[32];
	cJSON_AddStringToObject(pJson_Status, "time_stamp", Make_MaintainBeTimeString(_NOW_, szTime));

	//system run status
	cJSON_AddNumberToObject(pJson_Status, "sys_runstat", pOCPP->stCurr_102Info.emSysRunStatus);

	//alarmStatus & alarm table
	int nActAlmTotal = pOCPP->stCurr_102Info.nActiveAlmTotal;
	if(nActAlmTotal < 1)
	{
		cJSON_AddNumberToObject(pJson_Status, "sys_almstat", 0);
	}
	else
	{
		//cJSON_AddNumberToObject(pJson_Status, "sys_almstat", 1);
		cJSON* pJAlmTable = cJSON_CreateArray();
		int n;
		MAINT_BE_ALARM *pAlm = pOCPP->stCurr_102Info.actAlmTable;
		BOOL bHasCAAlarm = FALSE;
		int nLevel = 0;
		for(n = 0; n < nActAlmTotal; n++, pAlm++)
		{
			cJSON* pJ = cJSON_CreateObject();
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETAALARM_LEVEL_BYID_R,
				pAlm->nAlarmId, 0, &nLevel, 4);	
			cJSON_AddNumberToObject(pJ, "lev", nLevel - 1);
			if(nLevel == ALARM_LEVEL_CRITICAL)
			{
				bHasCAAlarm = TRUE;
			}
			char szAlmName[32];
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETAALARMNAME_BYID_R,
				pAlm->nAlarmId, 0, szAlmName, 32);	
			cJSON_AddStringToObject(pJ, "name", szAlmName);	
			char szDevName[32];
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETDEVNAME_BYID_R,
				pAlm->nDevId, 0, szDevName, 32);	
			//printf("--------DEEEEv=>%d, NAAAAAAAm=>%s------\n", pAlm->nDevId, szDevName);
			cJSON_AddStringToObject(pJ, "src", szDevName);	
			cJSON_AddStringToObject(pJ, "start_tm", Make_MaintainBeTimeString(pAlm->tmStart, szTime));
			cJSON_AddItemToArray(pJAlmTable, pJ);
		}
		cJSON_AddNumberToObject(pJson_Status, "sys_almstat", bHasCAAlarm ? 2 : 1);
		cJSON_AddItemToObject(pJson_Status, "alm_table", pJAlmTable);
	}

	//vconn status
	cJSON_AddNumberToObject(pJson_Status, "vcon_stat", pOCPP->stCurr_102Info.emVPNConnStatus);

	//in case of need update the status
	memcpy(&pOCPP->stLast_102Info, &pOCPP->stCurr_102Info, sizeof(MAINT_BE_STATUS_INFO));

	Send_Call(pOCPP, C_102_StatusNotif, pJson_Status);

	return 0;
}

int Handle_S201_StartVPN(void* pArg)
{
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;


	cJSON *pJson = pOCPP->pCurrJson;	
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	VAR_VALUE varV;	
	ERRCODE_DAI err;

	//vpn type: 0-nps tcp, 1-nps sock5
	int nVPNConnType = 0;
	if((temp1 = cJSON_GetObjectItem(pJson, "type")))
	{
		nVPNConnType = temp1->valueint;
	}

	//vpn addr:format=>IP:Port
	char szConnAddr[64] = {0};
	if((temp1 = cJSON_GetObjectItem(pJson, "addr")))
	{
		snprintf(szConnAddr, 64,  "%s", temp1->valuestring);
	}

	//vkey
	char szVKey[64] = {0};
	if((temp1 = cJSON_GetObjectItem(pJson, "vkey")))
	{
		snprintf(szVKey, 64,  "%s", temp1->valuestring);
	}

	//ext_param (optional)
	char szExParams[64] = {0};
	if((temp1 = cJSON_GetObjectItem(pJson, "ext_param")))
	{
		snprintf(szExParams, 64,  "%s", temp1->valuestring);
	}

	BOOL bAccept = FALSE;
	if(nVPNConnType == 0)
	{
		char szCmd_NPS[128];
		snprintf(szCmd_NPS, 128, "-server=%s -vkey=%s -type=tcp -log_level=0", szConnAddr, szVKey);	
		varV.blobValue = szCmd_NPS;	
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				1, 21, &varV, 128, 3000);
		bAccept = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	}
	else
	{
		//no support other than TCP
		Send_CallError(pOCPP, S_201_StartVPN, CallError_FormatError, "Only support TCP type yet!");
		return 0;
	}
	
	// if(!bAccept)
	// {
	// 	Send_CallError(pOCPP, S_201_StartVPN, CallError_GenericError, "Execute ERROR!");
	// 	return 0;
	// }

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "result", bAccept ? 0 : 1);

	if(bAccept)
	{
		pOCPP->tmLastExe_VPNStartCmd = _NOW_;
	}

	Send_CallResult(pOCPP, S_201_StartVPN, pResp);

	pOCPP->bCPStatusChanged = TRUE;	

	return 0;
}

int Handle_S202_StopVPN(void* pArg)
{
	MAINTAINBE_DATA_STU* pOCPP = (MAINTAINBE_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	VAR_VALUE varV;	
	varV.emValue = 0;
	ERRCODE_DAI err;

	BOOL bAccept = FALSE;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
				1, 22, &varV, 4, 3000);
	bAccept = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;

	cJSON* pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "result", bAccept ? 0 : 1);

	Send_CallResult(pOCPP, S_202_StopVPN, pResp);

	pOCPP->bCPStatusChanged = TRUE;	

	return 0;
}
