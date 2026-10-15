// #define _TRACE_PRINT_
#include "es_ify.h"

static const char* s_strGetConfig[] = {
			"Charger_ID" ,	
			"Server_URL" ,
			"Protect_SOC" ,		
			"SysACInput_PwrLmt" ,
			"SysACInput_PwrLmtHV", // LJH240705 AitiOverLoad HVMeter
			"PwrCtrlMode", // LJH240911 AvgPwrFirst
			"PwrLmtMin_AvgPwrMode",
			"Max_Output2AC_PwrLmt" ,
			"Min_Output2AC_PwrLmt" ,
			"PreProtect_RefluxAndOverLoad" ,
			"Output2AC_PwrRate" ,
			"UrgetAlterDischrgCurr" ,
			"AdjustUrgetModeCurr" ,	
			"EnterUrgetDisChrgModePower",
			"EixtUrgetDischrgModePower", 
			"TotalPositiveMaxDemand", // XSY250908 正向有功总最大需量
			"Cmd_101_Interval",		  
			"Cmd_102_Interval",		  
			"Cmd_103_Interval",		  
			"Cmd_104_Interval",		  
			"Cmd_105_Interval",		  
			"Cmd_106_Interval",		  
			"Cmd_107_Interval",		  
			"Cmd_108_Interval",		  
			"Cmd_109_Interval",		  
			"Cmd_202_Interval",		  
			"Cmd_204_Interval",
			"Cmd_206_Interval",
			"air_brand",
			"EnableQuickResponseChannel",
			"PowerRate_Chrg",
			"PreProt_Chrg",
			"Standby_Enable"
	};

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C100_BootNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static char* Make_ESIFYDateString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	snprintf(timeinstring, 32, "%4d-%02d-%02d",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C100_BootNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
char* Make_ESIFYTimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	sprintf(timeinstring, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

time_t  Parse_ESIFYTime(char *pszOut)
{
	time_t t = Parse_ISO8601TimeStr(pszOut);
	return t;
}

int Prepare_C100_BootNotification(void* pArg)
{
	TRACEX("Prepare_BootNotification \n");

	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;

	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	
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

	root = cJSON_CreateObject();

	//cJSON_AddStringToObject(root, "chargePointModel", bootInfo.szChargePointModel);
	cJSON_AddStringToObject(root, "vendor", bootInfo.szChargePointVendor);
	//if (bootInfo.emBootInfoOptions.chargerPointSN
	//if(bootInfo.emBootInfoOptions.chargerPointSN || (strlen(bootInfo.szChargePointSerualNuml) > 0))
	{
		cJSON_AddStringToObject(root, "serial_no", bootInfo.szChargePointSerualNuml);
	}

	int nPkgVer = 0;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PKG_VRESION_R,
		0, 0, &nPkgVer, 4);
	
	cJSON_AddNumberToObject(root, "sw_pkg_ver", nPkgVer);
 
	
	Send_Call(pIFY, root, C_100_BootNotif, CALL_SRC_Normal, 0, TRUE);

	return 0;
}


int Prepare_C101_StatusNotification(void* pArg)
{
	TRACEX("Prepare_StatusNotification \n");

	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	cJSON *pRoot = cJSON_CreateObject();
	
	PARAM_VAL_GET valGet;
	int n = 0;
	BOOL bSendUrgent = (pIFY->emTriggerMsgType == Trigger_C101) ? TRUE : FALSE;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
	pIFY->emSysStatus_For101 = valGet.val.emValue;

	cJSON_AddNumberToObject(pRoot, "cp_status", pIFY->emSysStatus_For101);

	cJSON_AddNumberToObject(pRoot, "grid_status", 0); //0-Merge Grid, 1-OFF Grid

	if(pIFY->emSysStatus_For101 > 0)
	{
		//cJSON_AddNumberToObject(pJson_Status, "cp_fault", GetConnectorErrCode_ByAlarm(pReporter, Connctor_0));
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 6, &valGet, sizeof(PARAM_VAL_GET));

		cJSON_AddNumberToObject(pRoot, "cp_fault", valGet.val.emValue);
	}
 
	//Msg_Send(pIFY, pJson_Status, StatusNotification, Call, pIFY->nCurrChargeStatusHandled_Conn, bSend_Later);
	Send_Call(pIFY, pRoot, C_101_StatusNotif, CALL_SRC_Normal, 0, bSendUrgent);

	return 0;
}

int Prepare_C102_BattCabStaNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	//VAR_VALUE 		varV;

	int n = 0, m = 0;
	//int nIMEU2CommStatu = 0;
	int nDevID = 0, nSigID = 0;

	A_BLOB_BATTCAB_IMEU2_V20	blobIMEU2_V20[MAX_BATTCABINET_NUM];
	
	cJSON 	*pRoot = cJSON_CreateObject();

	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	
	cJSON_AddNumberToObject(pRoot, "batt_cab_num", pIFY->nCfgBMSNum);

	if(pIFY->nCfgBMSNum > MAX_BATTCABINET_NUM)
	{	
		Send_CallError(pIFY, C_102_BattCabStaNotif, CallError_FormatError, "BattCabinet Cfg is Invalid!");
		return -1;
	}

	cJSON 	*pBattCab_Array = cJSON_CreateArray();
	for(n = 0; n < pIFY->nCfgBMSNum; n++)
	{
		//Need Judge IMEU2 Installed Status !!! Process Later
		valGet.nGetSize = 4;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				601 + n, 450, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}
 
		nDevID = 601 + n;
		nSigID = 450;
 
		ZERO_POBJS(&blobIMEU2_V20[n], 1);
		blobIMEU2_V20[n].emBlobType = BLOB_TYPE_BATTCAB_IMEU2_V20;
		valGet.nGetSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);

		valGet.val.blobValue = (BYTE*)&blobIMEU2_V20[n];
	 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}

		cJSON 	*pJS_SiglCab = cJSON_CreateObject();

		cJSON_AddNumberToObject(pJS_SiglCab, "cab_idx", n);

		cJSON_AddNumberToObject(pJS_SiglCab, "comm_statu", 						blobIMEU2_V20[n].emIMEU2_CommFail);
		cJSON_AddNumberToObject(pJS_SiglCab, "flood_alarm", 					blobIMEU2_V20[n].emFlood);
		cJSON_AddNumberToObject(pJS_SiglCab, "dooropen_alarm", 					blobIMEU2_V20[n].emDI3_DoorOpen);
		cJSON_AddNumberToObject(pJS_SiglCab, "smoke_alarm", 					blobIMEU2_V20[n].emDI2_Smoke);
		cJSON_AddNumberToObject(pJS_SiglCab, "fire_alarm", 						blobIMEU2_V20[n].emDI1_Fire);
		cJSON_AddNumberToObject(pJS_SiglCab, "combustible_gas_alarm", 			blobIMEU2_V20[n].emDI4_FlammableGas);
		cJSON_AddNumberToObject(pJS_SiglCab, "dump_alarm", 						0);
		cJSON_AddNumberToObject(pJS_SiglCab, "electric_blinds_open_stat", 		blobIMEU2_V20[n].emShutterOpenedOver);
		cJSON_AddNumberToObject(pJS_SiglCab, "electric_blinds_close_stat", 		blobIMEU2_V20[n].emDI6_ShutterClosedOver);
		
		//cab_temphumi_data
		cJSON 	*pTempHumi_Array = cJSON_CreateArray();
		for(m = 0; m < blobIMEU2_V20[n].nHumiTempSensorNumber; m++)
		{
			cJSON *pTempHumi_Obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(pTempHumi_Obj, "temp_humi_sensor_comm_stat", 	blobIMEU2_V20[n].emTempHumi_CommFail[m]);
			cJSON_AddFloatToObject(pTempHumi_Obj, "cab_temp_value", 				blobIMEU2_V20[n].fTemperature[m], 1);
			cJSON_AddFloatToObject(pTempHumi_Obj, "cab_humi_value", 				blobIMEU2_V20[n].fHumidity[m], 1);
			cJSON_AddItemToArray(pTempHumi_Array, pTempHumi_Obj);
		}
		cJSON_AddItemToObject(pJS_SiglCab, "cab_temphumi_data", pTempHumi_Array);

		//--- cab_pt1000_data ---//
		cJSON 	*pPT1000_Array = cJSON_CreateArray();
		for(m = 0; m < 4; m++)
		{
			cJSON *pPT1000_Obj = cJSON_CreateObject();
 			cJSON_AddFloatToObject(pPT1000_Obj, "comb_pt1000_value", 			blobIMEU2_V20[n].fPT1000Temperature[m], 1);
 			cJSON_AddItemToArray(pPT1000_Array, pPT1000_Obj);
		}
		cJSON_AddItemToObject(pJS_SiglCab, "cab_pt1000_data", pPT1000_Array);

		cJSON_AddFloatToObject(pJS_SiglCab, "exhaust_fan_speed_value", 			(float)blobIMEU2_V20[n].nExhaustFanSpeed, 1);
		cJSON_AddNumberToObject(pJS_SiglCab, "aircond_comm_stat", 				blobIMEU2_V20[n].emAirCond_CommFail);
		cJSON_AddNumberToObject(pJS_SiglCab, "aircond_alarm", 					blobIMEU2_V20[n].emAirCondTotalAlm);
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_ret_temp", 				blobIMEU2_V20[n].fRealTimeCabTemp, 1);
		
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_evaporator_temp", 			blobIMEU2_V20[n].fRealTimeCabTemp, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_condenser_temp", 			blobIMEU2_V20[n].fRealTimeCabTemp, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_compressor_start_temp", 	blobIMEU2_V20[n].fCoolSetPoint, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_heaterstart_temp", 		blobIMEU2_V20[n].fHeatDeviation, 1);
		cJSON_AddNumberToObject(pJS_SiglCab, "aircond_machine_state", 			blobIMEU2_V20[n].emOnOrOffd);
				
		cJSON_AddFloatToObject(pJS_SiglCab, "dehumi_start_temperature", 		blobIMEU2_V20[n].fDehumiStartTemp, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "dehumi_stop_temperature ", 		blobIMEU2_V20[n].fDehumiStopTemp, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "dehumi_start_humidity", 			blobIMEU2_V20[n].fDehumiStartHumi, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "dehumi_stop_humidity", 			blobIMEU2_V20[n].fDehumiStopHumi, 1);

		//comb_gas_datainfo
		cJSON 	*pCombGas_Array = cJSON_CreateArray();
		for(m = 0; m < blobIMEU2_V20[n].nGasSensorNum; m++)
		{
			cJSON *pCombGas_Obj = cJSON_CreateObject();
 			
			cJSON_AddNumberToObject(pCombGas_Obj, "combustible_gas_comm_stat", 	blobIMEU2_V20[n].emGasSensor_CommFail[m]);
			cJSON_AddNumberToObject(pCombGas_Obj, "combustible_gas_alarm", 		blobIMEU2_V20[n].emDI4_FlammableGas);
			cJSON_AddFloatToObject(pCombGas_Obj, "comb_gas_concentration_value",blobIMEU2_V20[n].fCombGasPotency[m], 1);

 			cJSON_AddItemToArray(pCombGas_Array, pCombGas_Obj);
		}
		cJSON_AddItemToObject(pJS_SiglCab, "comb_gas_datainfo", pCombGas_Array);

		float inletwater_temp = 0;
		float outletwater_temp = 0;
		float fEnvTemp = 0;
		int iAirWorkMode = 0;
		int iAirOperatMode = 0;
		int iCompressorStatus = 0;
		float fAirCondHz = 0;
		int iAirWorkModeCloud = INVALID_VALUE;
		float fOutPressure = INVALID_VALUE;
		float fRecPressure = INVALID_VALUE;
		valGet.nGetSize = 4;
		if(pIFY->emAirType == TONGFEI_AIR)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1779, &valGet, sizeof(PARAM_VAL_GET));
			inletwater_temp = valGet.val.fValue;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1780, &valGet, sizeof(PARAM_VAL_GET));
			outletwater_temp = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1789, &valGet, sizeof(PARAM_VAL_GET));//出水压力
			fOutPressure = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1790, &valGet, sizeof(PARAM_VAL_GET));//进水压力
			fRecPressure = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1783, &valGet, sizeof(PARAM_VAL_GET));
			fEnvTemp = valGet.val.fValue;

			//, 951 + n, 1778,机组运行模式
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1778, &valGet, sizeof(PARAM_VAL_GET));
			iAirOperatMode = valGet.val.emValue; //0：待机 1：制冷 2：制热 3：自循环

			iCompressorStatus = (iAirOperatMode == 0) ? 0 : 1; //0：待机 1：工作

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1771, &valGet, sizeof(PARAM_VAL_GET));
			iAirWorkMode = valGet.val.emValue; //0：待机 1：制冷 2：加热 3：自循环 4：自动

			int mode[5] = {AIR_MODE_STANDBY, AIR_MODE_COOLING, AIR_MODE_HEATING, AIR_MODE_CRICULATION, AIR_MODE_AUTO};
			iAirWorkModeCloud = (iAirWorkMode >= 0 && iAirWorkMode <= 4) ? mode[iAirWorkMode] : INVALID_VALUE;
		}
		else if(pIFY->emAirType == IMEU3_AIR)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3535, &valGet, sizeof(PARAM_VAL_GET));
			inletwater_temp = valGet.val.fValue;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3536, &valGet, sizeof(PARAM_VAL_GET));
			outletwater_temp = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3545, &valGet, sizeof(PARAM_VAL_GET));//出水压力
			fOutPressure = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3544, &valGet, sizeof(PARAM_VAL_GET));//进水压力
			fRecPressure = valGet.val.fValue;

			//, 951 + n, 3512,压缩机运行频率
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3512, &valGet, sizeof(PARAM_VAL_GET));
			fAirCondHz = valGet.val.fValue;
			iCompressorStatus = (fAirCondHz > 0.0f) ? 1 : 0; // 0：待机，1：工作

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3539, &valGet, sizeof(PARAM_VAL_GET));
			fEnvTemp = valGet.val.fValue;
			
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3502, &valGet, sizeof(PARAM_VAL_GET));
			iAirWorkMode = valGet.val.emValue; //1：制冷（压缩机制冷） 2：制热模式 3：自循环模式 5：自然冷模式

			int mode[6] = {INVALID_VALUE, AIR_MODE_COOLING, AIR_MODE_HEATING, AIR_MODE_CRICULATION, INVALID_VALUE, INVALID_VALUE};
			iAirWorkModeCloud = (iAirWorkMode >= 0 && iAirWorkMode <= 5) ? mode[iAirWorkMode] : INVALID_VALUE;
		}
		else if(pIFY->emAirType == INFYPOWER_AIR)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1541, &valGet, sizeof(PARAM_VAL_GET));
			inletwater_temp = valGet.val.fValue;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1542, &valGet, sizeof(PARAM_VAL_GET));
			outletwater_temp = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1536, &valGet, sizeof(PARAM_VAL_GET));
			fEnvTemp = valGet.val.fValue;
			
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 1521, &valGet, sizeof(PARAM_VAL_GET));
			iAirWorkMode = valGet.val.emValue;	//0：待机模式 1：自循环模式 2：制冷模式 3：制热模式 4：自动模式
			
			int mode[5] = {AIR_MODE_STANDBY, AIR_MODE_CRICULATION, AIR_MODE_COOLING, AIR_MODE_HEATING, AIR_MODE_AUTO};
			iAirWorkModeCloud = (iAirWorkMode >= 0 && iAirWorkMode <= 4) ? mode[iAirWorkMode] : INVALID_VALUE;
		}
		else if(pIFY->emAirType == XINMIAO_AIR)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3648, &valGet, sizeof(PARAM_VAL_GET));
			inletwater_temp = valGet.val.fValue;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3647, &valGet, sizeof(PARAM_VAL_GET));
			outletwater_temp = valGet.val.fValue;
			
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3649, &valGet, sizeof(PARAM_VAL_GET));//出水压力
			fOutPressure = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3650, &valGet, sizeof(PARAM_VAL_GET));//进水压力
		 	fRecPressure = valGet.val.fValue;

			//, 951 + n, 3643,1#压缩机运行状态
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3643, &valGet, sizeof(PARAM_VAL_GET));
			iAirOperatMode = valGet.val.emValue;	// 0：待机 1：工作

			iCompressorStatus = (iAirOperatMode == 0) ? 0 : 1; // 0：待机 1：工作

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3651, &valGet, sizeof(PARAM_VAL_GET));
			fEnvTemp = valGet.val.fValue;
			
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + n, 3654, &valGet, sizeof(PARAM_VAL_GET));
			iAirWorkMode = valGet.val.emValue;	//1：自循环模式 2：制冷模式 3：制热模式 4：自动模式

			int mode[5] = {INVALID_VALUE, AIR_MODE_CRICULATION, AIR_MODE_COOLING, AIR_MODE_HEATING, AIR_MODE_AUTO};
			iAirWorkModeCloud = (iAirWorkMode >= 0 && iAirWorkMode <= 4) ? mode[iAirWorkMode] : INVALID_VALUE;
		}

		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_inletwater_temp", inletwater_temp, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "aircond_outletwater_temp", outletwater_temp, 1);

		float pdu_temp = (float)iAirWorkModeCloud;
		if (DEFAULT_DEVICE != pIFY->devType)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 611, 1672, &valGet, sizeof(PARAM_VAL_GET));
		 	pdu_temp = valGet.val.fValue;
		}

		cJSON_AddFloatToObject(pJS_SiglCab, "pdu_temp",	pdu_temp, 1);
		cJSON_AddNumberToObject(pJS_SiglCab, "air_mode",	iAirWorkModeCloud);
		cJSON_AddFloatToObject(pJS_SiglCab, "outer_temp", 	fEnvTemp, 1);
		cJSON_AddNumberToObject(pJS_SiglCab, "air_compressor_status", iCompressorStatus);

		//PACK的铜排温度信息
		cJSON 	*pPackCuTempArray = cJSON_CreateArray();
		int k = 0;
		for (k = 0; k < pIFY->iLiquidPackNum; ++k)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 981 + n, 1700 + k * 10, &valGet, sizeof(PARAM_VAL_GET));
			float fCuPositiveTemp = valGet.val.fValue;

			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 981 + n, 1701 + k * 10, &valGet, sizeof(PARAM_VAL_GET));
			float fCuNegTemp = valGet.val.fValue;

			cJSON *pLiquidPack_Obj = cJSON_CreateObject();
 			
			cJSON_AddFloatToObject(pLiquidPack_Obj, "pack_cu_pos_temp", fCuPositiveTemp, 1);
			cJSON_AddFloatToObject(pLiquidPack_Obj, "pack_cu_neg_temp", fCuNegTemp, 1);

			cJSON_AddItemToArray(pPackCuTempArray, pLiquidPack_Obj);	
		}
		cJSON_AddItemToObject(pJS_SiglCab, "pack_cu_temp", pPackCuTempArray);

		cJSON_AddFloatToObject(pJS_SiglCab, "air_in_water_pressure",	fRecPressure, 1);
		cJSON_AddFloatToObject(pJS_SiglCab, "air_out_water_pressure",	fOutPressure, 1);


		cJSON_AddItemToArray(pBattCab_Array, pJS_SiglCab);
	}
	cJSON_AddItemToObject(pRoot, "batt_cab_data", pBattCab_Array);
	Send_Call(pIFY, pRoot, C_102_BattCabStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C103_ESBMSStatusNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C103_ESBMSStatusNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	//int nOnLineSTBatt = 0;
	int nDevID = 0, nSigID = 0;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;

	int n = 0;
 
	if(pIFY->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		Send_CallError(pIFY, C_103_ESBMSStaNotif, CallError_FormatError, "BMS Number Cfg is Invalid!");
		return -1;
	}

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	cJSON_AddNumberToObject(pRoot, "online_batts", pIFY->nCfgBMSNum);

	//Single STBattery Object Infomation
	cJSON 	*pBMS_Array = cJSON_CreateArray();

 	A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
	for(n = 0; n < pIFY->nCfgBMSNum; n++)
	{

		nDevID = 501 + n;
		nSigID = 551;
			 
		ZERO_POBJS(&blobSiglBatt, 1);
		blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
		valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		valGet.val.blobValue = (BYTE*)&blobSiglBatt;
	 	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}
 
        cJSON *pBMS_Single = cJSON_CreateObject();
		cJSON_AddNumberToObject(pBMS_Single, "batt_idx", n);
		if(blobSiglBatt.fPackTotalCurr >= 2.0)
		{
			cJSON_AddNumberToObject(pBMS_Single, "run_statu", STBATT_RUNSTAT_CHARGE);
		}
		else if(blobSiglBatt.fPackTotalCurr <= -2.0)
		{	
			cJSON_AddNumberToObject(pBMS_Single, "run_statu", STBATT_RUNSTAT_DISCHARGE);
		}
		else
		{
			cJSON_AddNumberToObject(pBMS_Single, "run_statu", blobSiglBatt.emBattStatus_Run);
		}
 
		cJSON_AddFloatToObject(pBMS_Single, "pack_volt", blobSiglBatt.fPackTotalVolt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "pack_curr", blobSiglBatt.fPackTotalCurr, 1);
		
		cJSON_AddNumberToObject(pBMS_Single, "soh", blobSiglBatt.nSOH);
		cJSON_AddNumberToObject(pBMS_Single, "soc", blobSiglBatt.nSOC);

		cJSON_AddNumberToObject(pBMS_Single, "cycle", blobSiglBatt.nLifeCycleTimes);

		cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_volt", blobSiglBatt.fAllow_LmtChargeVolt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_curr", blobSiglBatt.fAllow_MaxChargeCurr, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_volt", blobSiglBatt.fAllow_LmtDisChargeVolt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_curr", blobSiglBatt.fAllow_MaxDisChargeCurr, 1);
		cJSON_AddFloatToObject(pBMS_Single, "cell_max_volt", blobSiglBatt.fMax_CellVolt, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_min_volt", blobSiglBatt.fMin_CellVolt, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_avg_volt", blobSiglBatt.fAvg_CellVolt, 3);
		
		cJSON_AddFloatToObject(pBMS_Single, "cell_max_temp", blobSiglBatt.fMax_CellTemp, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_min_temp", blobSiglBatt.fMin_CellTemp, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_avg_temp", blobSiglBatt.fAvg_CellTemp, 1);

		cJSON_AddFloatToObject(pBMS_Single, "ucell_max_csc",  (float)blobSiglBatt.uiBAT_Ucell_Max_CSC, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_max_cell", (float)blobSiglBatt.uiBAT_Ucell_Max_CELL, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_min_csc",  (float)blobSiglBatt.uiBAT_Ucell_Min_CSC, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_min_cell", (float)blobSiglBatt.uiBAT_Ucell_Min_CELL, 1);
		
		cJSON_AddFloatToObject(pBMS_Single, "temp_max_csc",  (float)blobSiglBatt.nBAT_T_Max_CSC, 1);
		cJSON_AddFloatToObject(pBMS_Single, "temp_min_csc", (float)blobSiglBatt.nBAT_T_Min_CSC, 1);
		
		cJSON_AddFloatToObject(pBMS_Single, "remain_charge_energy", (float)blobSiglBatt.fChrg_Energy, 2);
		cJSON_AddFloatToObject(pBMS_Single, "remain_discharge_energy", (float)blobSiglBatt.fDisChrging_Energy, 2);

		cJSON_AddNumberToObject(pBMS_Single, "chrg_forbid", blobSiglBatt.bIsBatt_ChargeForbidden);
		cJSON_AddNumberToObject(pBMS_Single, "dischrg_forbid", blobSiglBatt.bIsBatt_DisChargeForbidden);
 		
		cJSON_AddNumberToObject(pBMS_Single, "alarm_status", blobSiglBatt.emBattStatus_Alm); 
		cJSON_AddStringToObject(pBMS_Single, "alarm_info", blobSiglBatt.szS2A_AlarmInfo);

		cJSON_AddStringToObject(pBMS_Single, "alarm2_info", blobSiglBatt.szInfy_AlarmInfo);
		cJSON_AddStringToObject(pBMS_Single, "fault_info", blobSiglBatt.szInfy_FaultInfo); 
		cJSON_AddStringToObject(pBMS_Single, "gpio_info", blobSiglBatt.szInfy_GPIOInfo); 

		cJSON_AddFloatToObject(pBMS_Single, "batt_ver", blobSiglBatt.fVersion, 3);

		cJSON_AddItemToArray(pBMS_Array, pBMS_Single);

	}
	cJSON_AddItemToObject(pRoot, "batt_grp_data", pBMS_Array);

	Send_Call(pIFY, pRoot, C_103_ESBMSStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C104_PwrDistrStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C104_PwrDistrStaNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;

	int n = 0;
	int nDevID = 0, nSigID = 0;
	if(pIFY->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		Send_CallError(pIFY, C_104_PwrDistrStaNotif, CallError_FormatError, "Branch Number Cfg is Invalid!");
		return -1;
	}
	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	cJSON_AddNumberToObject(pRoot, "online_branch_num", pIFY->nCfgBMSNum);

	nDevID = 221;
	nSigID = 203;

	A_BLOB_BRANCH_NODE_CALC_DATA nodeCalcData;
	ZERO_POBJS(&nodeCalcData, 1);
	nodeCalcData.emBlobType = BLOB_TYPE_BRANCH_NODE_CALC_DATA;
	valGet.nGetSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
	valGet.val.blobValue = (BYTE*)&nodeCalcData;
	cJSON 	*pBranch_Array = cJSON_CreateArray();
	for(n = 0; n < pIFY->nCfgBMSNum; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDevID + n, nSigID, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}

		cJSON *pBranch_Single = cJSON_CreateObject();
		
		cJSON_AddNumberToObject(pBranch_Single, "branch_idx", n);
		cJSON_AddFloatToObject(pBranch_Single, "batt_max_dischrg_pwr", 		nodeCalcData.fBatt_Max_DisChrg_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_max_chrg_pwr_req", 	nodeCalcData.fBatt_Max_Chrg_Pwr_Req, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_distri4acdc_pwr", 		nodeCalcData.fBatt_Distri_4_ACDC_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_distri4dcdc_pwr", 		nodeCalcData.fDCDC_Max_Ability_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "acdc_rate_pwr", 			nodeCalcData.fACDC_Max_Ability_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "dcdc_rate_pwr", 			nodeCalcData.fDCDC_Max_Ability_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "remain_batt_dischrg_pwr", 	nodeCalcData.fRemain_Batt_DisChrg_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "remain_batt_chrg_pwr_req", 	nodeCalcData.fRemain_Batt_Chrg_Pwr_Req, 1);
		cJSON_AddFloatToObject(pBranch_Single, "acdc_distri4batt_chrg_pwr", nodeCalcData.fACDC_Distri_4_BattChrg_Pwr, 1);
 
		cJSON_AddItemToArray(pBranch_Array, pBranch_Single);
	}

	cJSON_AddItemToObject(pRoot, "branch_node_calc_data", pBranch_Array);
	
	Send_Call(pIFY, pRoot, C_104_PwrDistrStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

int Prepare_C105_ESRunDataNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;
	int n= 0, nDevID = 0, nSigID = 0;

	nDevID = 200;
	nSigID = 80;


	A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);
	esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return 0;
	}

 	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	cJSON_AddNumberToObject(pRoot, "ac_workstat", esRunData.emAC_WorkStatus);

 	cJSON	*pACVolt_Array = cJSON_CreateArray();
	cJSON_AddItemToArray(pACVolt_Array, cJSON_CreateFloat(esRunData.fACVolt_L1, 1));
    cJSON_AddItemToArray(pACVolt_Array, cJSON_CreateFloat(esRunData.fACVolt_L2, 1));
    cJSON_AddItemToArray(pACVolt_Array, cJSON_CreateFloat(esRunData.fACVolt_L3, 1));
    cJSON_AddItemToObject(pRoot, "ac_volts", pACVolt_Array);
 	
	cJSON* pACCurr_Array = cJSON_CreateArray();
    cJSON_AddItemToArray(pACCurr_Array, cJSON_CreateFloat(esRunData.fACCurr_L1, 1));
    cJSON_AddItemToArray(pACCurr_Array, cJSON_CreateFloat(esRunData.fACCurr_L2, 1));
    cJSON_AddItemToArray(pACCurr_Array, cJSON_CreateFloat(esRunData.fACCurr_L3, 1));
    cJSON_AddItemToObject(pRoot, "ac_currs", pACCurr_Array);
	
	cJSON_AddFloatToObject(pRoot, "dcload_volt", esRunData.fDCLoad_Volt, 1);
	cJSON_AddFloatToObject(pRoot, "dcload_curr", esRunData.fDCLoad_Curr, 1);
 

	cJSON_AddNumberToObject(pRoot, "position_auxmeter", esRunData.emAuxPos);
	cJSON_AddFloatToObject(pRoot, "kw_meter_grid", 		esRunData.f_PgridMeter, 		1);
	cJSON_AddFloatToObject(pRoot, "kw_meter_cab", 		esRunData.f_PcabMeter, 			1);
	cJSON_AddFloatToObject(pRoot, "kw_meter_aux1", 		esRunData.f_PauxMeter, 			1);
	cJSON_AddFloatToObject(pRoot, "kw_meter_aux2", 		esRunData.f_PauxMeter_3, 		1);
	cJSON_AddFloatToObject(pRoot, "kw_acload", 			esRunData.fACLoad_Power, 		1);
	cJSON_AddFloatToObject(pRoot, "kw_meter_offgrid", 	esRunData.fPoffgrid_LoadMeter, 	1);

	//cJSON_AddItemToObject(pRoot, "branch_node_calc_data", pObj_ACMeter);
	cJSON	*pRectGrp_Array = cJSON_CreateArray();
	for(n = 0; n < pIFY->nCfgBMSNum; n++)
	{
		cJSON 	*pSiglRectGrp = cJSON_CreateObject();

		cJSON_AddNumberToObject(pSiglRectGrp, "branch_grp_idx", n);

		cJSON_AddNumberToObject(pSiglRectGrp, "acdc_exist_status",  esRunData.isACDC_Grp_Exist[n]);
		cJSON_AddNumberToObject(pSiglRectGrp, "batt_exist_status",  (esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)?0:1); 
		cJSON_AddNumberToObject(pSiglRectGrp, "dcdc_exist_status",  esRunData.isDCDC_Grp_Exist[n]);
		cJSON_AddNumberToObject(pSiglRectGrp, "mppt_exist_status",  esRunData.isMPPT_Grp_Exist[n]);

		cJSON_AddNumberToObject(pSiglRectGrp, "acdc_workstat",		esRunData.emACDC_WorkStatu[n]);
		cJSON_AddFloatToObject(pSiglRectGrp, "acdc_volt", 			esRunData.fACDC_OutVolt[n], 1);
		cJSON_AddFloatToObject(pSiglRectGrp, "acdc_curr", 			esRunData.fACDC_OutCurr[n], 1);
		cJSON_AddFloatToObject(pSiglRectGrp, "acdc_ext_volt", 		esRunData.fACDCOutRelayVolt[n], 1);

		UINT emBattStatus;
		if(esRunData.fSTBatt_OutCurr[n] > 1.0)
		{
			emBattStatus = 2; //Charging
		}
		else if(esRunData.fSTBatt_OutCurr[n] < -1.0)
		{
			emBattStatus = 3;//DisCharging
		}
		else
		{
			emBattStatus = 1;//Idle
		}
		int nSTBattStatu;
		if(esRunData.fSTBatt_OutCurr[n] > 1.0)
		{
			nSTBattStatu = 2;
		}
		else if(esRunData.fSTBatt_OutCurr[n] < -1.0)
		{
			nSTBattStatu = 3;
		}
		else
		{
			nSTBattStatu = 1;
		}
		cJSON_AddNumberToObject(pSiglRectGrp, "batt_workstat", 		nSTBattStatu);

		cJSON_AddNumberToObject(pSiglRectGrp, "batt_soc", 			esRunData.nSTBatt_SOC[n]);

		cJSON_AddFloatToObject(pSiglRectGrp, "batt_volt", 			esRunData.fSTBatt_OutVolt[n], 1);
		cJSON_AddFloatToObject(pSiglRectGrp, "batt_curr", 			esRunData.fSTBatt_OutCurr[n], 1);
		//cJSON_AddNumberToObject(pSiglRectGrp, "batt_run_status", 	esRunData.emSTBatt_WorkStatu[n]);

		cJSON_AddNumberToObject(pSiglRectGrp, "dcdc_workstat", 		esRunData.emDCDC_WorkStatu[n]);
		cJSON_AddFloatToObject(pSiglRectGrp, "dcdc_volt", 			esRunData.fDCDC_OutVolt[n], 1);
		cJSON_AddFloatToObject(pSiglRectGrp, "dcdc_curr", 			esRunData.fDCDC_OutCurr[n], 1);

		cJSON_AddNumberToObject(pSiglRectGrp, "mppt_workstat", 		(esRunData.fMPPT_OutVolt[n] > 400)?1:0/*esRunData.emMPPT_WorkStatu[n]*/);
		cJSON_AddFloatToObject(pSiglRectGrp, "mppt_volt", 			esRunData.fMPPT_OutVolt[n], 1);
		cJSON_AddFloatToObject(pSiglRectGrp, "mppt_curr", 			esRunData.fMPPT_OutCurr[n], 1);
		
		cJSON_AddItemToArray(pRectGrp_Array, pSiglRectGrp);
	}
	cJSON_AddItemToObject(pRoot, "branch_grp_data", pRectGrp_Array);
 	
	Send_Call(pIFY, pRoot, C_105_ESRunDataStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C106_SingleRectStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C106_SingleRectStaNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	int n = 0, nRectType = 0;
	int nOnlineACDCNum = 0, nOnlineDCDCNum = 0, nOnlineMPPTNum = 0;

	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;

	A_BLOB_SINGLE_RECT_INFO sigRect;
	ZERO_POBJS(&sigRect, 	1);
	sigRect.emBlobType = BLOB_TYPE_SINGLE_RECT_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
	valGet.val.blobValue = (BYTE*)&sigRect;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 101, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q106 Failed err=[%d]!\n", err);
		return -1;
	}
	
	A_BLOB_RECT_GRP_DATA rectGrpData;
	ZERO_POBJS(&rectGrpData, 	1);
	valGet.nGetSize = sizeof(A_BLOB_RECT_GRP_DATA);
	valGet.val.blobValue = (BYTE*)&rectGrpData;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 204, &valGet, sizeof(PARAM_VAL_GET));

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 611, 1671, &valGet, sizeof(PARAM_VAL_GET));
	float fPCSOutTemp = valGet.val.fValue;


	cJSON 	*pRoot = cJSON_CreateObject();

	cJSON_AddNumberToObject(pRoot, "data_from", 0);

	cJSON	*pACDC_Array = cJSON_CreateArray();
 	cJSON	*pMPPT_Array = cJSON_CreateArray();
	cJSON	*pDCDC_Array = cJSON_CreateArray();
	
	for(nRectType = 0; nRectType < MAX_RECT_TYPE; nRectType++)
	{
		UINT count = sigRect.nOnlineNum[nRectType];
		TRACED("Prepare_C106_SingleRectStaNotif: nRectType=%d, sigRect.nOnlineNum[nRectType]=%d\n", nRectType, sigRect.nOnlineNum[nRectType]);
		if (count > MAX_SINGLE_RECT_NUM) 
		{
			count = MAX_SINGLE_RECT_NUM;
		}
		switch (nRectType)
		{
		case TYPE_SIG_RECT_ACDC_GRP1:
		case TYPE_SIG_RECT_ACDC_GRP2:
    	case TYPE_SIG_RECT_ACDC_GRP3:
    	case TYPE_SIG_RECT_ACDC_GRP4:
			// for(n = 0; n < sigRect.nOnlineNum[nRectType]; n++)
			for (n = 0; n < count; ++n)
			{
				nOnlineACDCNum++;
				cJSON	*pSiglACDC_Obj = cJSON_CreateObject();
				cJSON_AddNumberToObject(pSiglACDC_Obj, "comm_statu", 		sigRect.arrRect[nRectType][n].byCommStatu);
				cJSON_AddNumberToObject(pSiglACDC_Obj, "belong_grp", 		sigRect.arrRect[nRectType][n].nBelongGrp);
				sigRect.arrRect[nRectType][n].emWorkMode = 
				(sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus != SYS_STATUS_RUNNING) ? 2 : sigRect.arrRect[nRectType][n].emWorkMode;
				cJSON_AddNumberToObject(pSiglACDC_Obj, "work_mode", 		sigRect.arrRect[nRectType][n].emWorkMode);
				cJSON_AddNumberToObject(pSiglACDC_Obj, "err_code", 			0);
				cJSON_AddStringToObject(pSiglACDC_Obj, "rect_mode_errcode", sigRect.arrRect[nRectType][n].byRectErrCode);
				cJSON_AddStringToObject(pSiglACDC_Obj, "invrt_mode_errcode",sigRect.arrRect[nRectType][n].byInvertErrCode);
				
				cJSON_AddFloatToObject(pSiglACDC_Obj, "output_volt", 		sigRect.arrRect[nRectType][n].fRTOutputVolt, 1);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "output_curr", 		sigRect.arrRect[nRectType][n].fRTOutputCurr, 1);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "dcdc_ver", 			sigRect.arrRect[nRectType][n].fRect_DCDCVer, 3);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "pfc_var", 			sigRect.arrRect[nRectType][n].fRect_PFCVer,  3);
				cJSON_AddStringToObject(pSiglACDC_Obj, "bar_code",			sigRect.arrRect[nRectType][n].byBarCode);

				cJSON_AddFloatToObject(pSiglACDC_Obj, "active_power", 		sigRect.arrRect[nRectType][n].fPtotalActive, 1);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "reactive_power", 	sigRect.arrRect[nRectType][n].fPtotalReactive, 1);

				cJSON_AddFloatToObject(pSiglACDC_Obj, "apparent_power", 	sigRect.arrRect[nRectType][n].fPTotalApparent, 1);

				UINT uWorkMode = (sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus == SYS_STATUS_SHUTDOWN) ? 0 : 1;
				cJSON_AddNumberToObject(pSiglACDC_Obj, "status", uWorkMode);

				float fPCSInTemp = sigRect.arrRect[nRectType][n].fAmbit_Temp;
				cJSON_AddFloatToObject(pSiglACDC_Obj, "pcs_in_temp", fPCSInTemp, 1);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "pcs_out_temp", fPCSOutTemp, 1);
				
				cJSON	*pACDC_Vol_Array = cJSON_CreateArray();
				cJSON	*pACDC_Cur_Array = cJSON_CreateArray();
				int kk = 0;
				for (kk = 0; kk < NUM_GRID_PHASE; ++kk)
				{
					cJSON_AddItemToArray(pACDC_Vol_Array, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputVolt[kk], 1));
					cJSON_AddItemToArray(pACDC_Cur_Array, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputCurr[kk], 1));
				}
				
				cJSON_AddItemToObject(pSiglACDC_Obj, "voltage", 	pACDC_Vol_Array);
				cJSON_AddItemToObject(pSiglACDC_Obj, "current", 	pACDC_Cur_Array);

				cJSON_AddFloatToObject(pSiglACDC_Obj, "factor", 	sigRect.arrRect[nRectType][n].fPtotalFactor, 1);
				cJSON_AddFloatToObject(pSiglACDC_Obj, "frequency", 	sigRect.arrRect[nRectType][n].fAC_Freq, 1);

				cJSON_AddItemToArray(pACDC_Array, pSiglACDC_Obj);
			}
			break;

			case TYPE_SIG_RECT_MPPT_GRP1:
			case TYPE_SIG_RECT_MPPT_GRP2:
			case TYPE_SIG_RECT_MPPT_GRP3:
			case TYPE_SIG_RECT_MPPT_GRP4:
				// for(n = 0; n < sigRect.nOnlineNum[nRectType]; n++)
				for (n = 0; n < count; ++n)
				{
					nOnlineMPPTNum++;
					cJSON	*pSiglMPPT_Obj = cJSON_CreateObject();
					cJSON_AddNumberToObject(pSiglMPPT_Obj, "comm_statu", 		sigRect.arrRect[nRectType][n].byCommStatu);
					cJSON_AddNumberToObject(pSiglMPPT_Obj, "belong_grp", 		sigRect.arrRect[nRectType][n].nBelongGrp);
					sigRect.arrRect[nRectType][n].emWorkMode = 
					(sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus != SYS_STATUS_RUNNING) ? 2 : sigRect.arrRect[nRectType][n].emWorkMode;
					cJSON_AddNumberToObject(pSiglMPPT_Obj, "work_mode", 		sigRect.arrRect[nRectType][n].emWorkMode);
					cJSON_AddNumberToObject(pSiglMPPT_Obj, "err_code", 			0);
					cJSON_AddStringToObject(pSiglMPPT_Obj, "rect_mode_errcode", sigRect.arrRect[nRectType][n].byRectErrCode);
					cJSON_AddStringToObject(pSiglMPPT_Obj, "invrt_mode_errcode",sigRect.arrRect[nRectType][n].byInvertErrCode);
					
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "output_volt", 		sigRect.arrRect[nRectType][n].fRTOutputVolt, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "output_curr", 		sigRect.arrRect[nRectType][n].fRTOutputCurr, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "dcdc_ver", 			sigRect.arrRect[nRectType][n].fRect_DCDCVer, 3);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "pfc_var", 			sigRect.arrRect[nRectType][n].fRect_PFCVer,  3);
					cJSON_AddStringToObject(pSiglMPPT_Obj, "bar_code",			sigRect.arrRect[nRectType][n].byBarCode);

					cJSON_AddFloatToObject(pSiglMPPT_Obj, "active_power", 		sigRect.arrRect[nRectType][n].fPtotalActive, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "reactive_power", 	sigRect.arrRect[nRectType][n].fPtotalReactive, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "apparent_power", 	sigRect.arrRect[nRectType][n].fPTotalApparent, 1);
	
	
					UINT uWorkMode = (sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus == SYS_STATUS_SHUTDOWN) ? 0 : 1;
					cJSON_AddNumberToObject(pSiglMPPT_Obj, "status", uWorkMode);

					float fPCSInTemp = sigRect.arrRect[nRectType][n].fAmbit_Temp;
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "pcs_in_temp", fPCSInTemp, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "pcs_out_temp", fPCSOutTemp, 1);
					
					cJSON	*pACDC_Vol_Array = cJSON_CreateArray();
					cJSON	*pACDC_Cur_Array = cJSON_CreateArray();
					int kk = 0;
					for (kk = 0; kk < NUM_GRID_PHASE; ++kk)
					{
						cJSON_AddItemToArray(pSiglMPPT_Obj, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputVolt[kk], 1));
						cJSON_AddItemToArray(pSiglMPPT_Obj, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputCurr[kk], 1));
					}
					
					cJSON_AddItemToObject(pSiglMPPT_Obj, "voltage", 	pACDC_Vol_Array);
					cJSON_AddItemToObject(pSiglMPPT_Obj, "current", 	pACDC_Cur_Array);

					cJSON_AddFloatToObject(pSiglMPPT_Obj, "factor", 	sigRect.arrRect[nRectType][n].fPtotalFactor, 1);
					cJSON_AddFloatToObject(pSiglMPPT_Obj, "frequency", 	sigRect.arrRect[nRectType][n].fAC_Freq, 1);

					cJSON_AddItemToArray(pMPPT_Array, pSiglMPPT_Obj);
				}
			break;

			case TYPE_SIG_RECT_DCDC_GRP1:
			case TYPE_SIG_RECT_DCDC_GRP2:
			case TYPE_SIG_RECT_DCDC_GRP3:
			case TYPE_SIG_RECT_DCDC_GRP4:
				// for(n = 0; n < sigRect.nOnlineNum[nRectType]; n++)
				for (n = 0; n < count; ++n)
				{
					nOnlineDCDCNum++;
					cJSON	*pSiglDCDC_Obj = cJSON_CreateObject();
					cJSON_AddNumberToObject(pSiglDCDC_Obj, "comm_statu", 		sigRect.arrRect[nRectType][n].byCommStatu);
					cJSON_AddNumberToObject(pSiglDCDC_Obj, "belong_grp", 		sigRect.arrRect[nRectType][n].nBelongGrp);
					sigRect.arrRect[nRectType][n].emWorkMode = 
					(sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus != SYS_STATUS_RUNNING) ? 2 : sigRect.arrRect[nRectType][n].emWorkMode;
					cJSON_AddNumberToObject(pSiglDCDC_Obj, "work_mode", 		sigRect.arrRect[nRectType][n].emWorkMode);
					cJSON_AddNumberToObject(pSiglDCDC_Obj, "err_code", 			0);
					cJSON_AddStringToObject(pSiglDCDC_Obj, "rect_mode_errcode", sigRect.arrRect[nRectType][n].byRectErrCode);
					cJSON_AddStringToObject(pSiglDCDC_Obj, "invrt_mode_errcode",sigRect.arrRect[nRectType][n].byInvertErrCode);
					
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "output_volt", 		sigRect.arrRect[nRectType][n].fRTOutputVolt, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "output_curr", 		sigRect.arrRect[nRectType][n].fRTOutputCurr, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "dcdc_ver", 			sigRect.arrRect[nRectType][n].fRect_DCDCVer, 3);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "pfc_var", 			sigRect.arrRect[nRectType][n].fRect_PFCVer,  3);
					cJSON_AddStringToObject(pSiglDCDC_Obj, "bar_code",			sigRect.arrRect[nRectType][n].byBarCode);

					cJSON_AddFloatToObject(pSiglDCDC_Obj, "active_power", 		sigRect.arrRect[nRectType][n].fPtotalActive, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "reactive_power", 	sigRect.arrRect[nRectType][n].fPtotalReactive, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "apparent_power", 	sigRect.arrRect[nRectType][n].fPTotalApparent, 1);

					UINT uWorkMode = (sigRect.arrRect[nRectType][n].mapPCSCode.PCSStatu.SystemStatus == SYS_STATUS_SHUTDOWN) ? 0 : 1;
					cJSON_AddNumberToObject(pSiglDCDC_Obj, "status", uWorkMode);

					float fPCSInTemp = sigRect.arrRect[nRectType][n].fAmbit_Temp;
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "pcs_in_temp", fPCSInTemp, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "pcs_out_temp", fPCSOutTemp, 1);

					cJSON	*pACDC_Vol_Array = cJSON_CreateArray();
					cJSON	*pACDC_Cur_Array = cJSON_CreateArray();
					int kk = 0;
					for (kk = 0; kk < NUM_GRID_PHASE; ++kk)
					{
						cJSON_AddItemToArray(pACDC_Vol_Array, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputVolt[kk], 1));
						cJSON_AddItemToArray(pACDC_Cur_Array, cJSON_CreateFloat(sigRect.arrRect[nRectType][n].fPh_InputCurr[kk], 1));
					}
					
					cJSON_AddItemToObject(pSiglDCDC_Obj, "voltage", 	pACDC_Vol_Array);
					cJSON_AddItemToObject(pSiglDCDC_Obj, "current", 	pACDC_Cur_Array);

					cJSON_AddFloatToObject(pSiglDCDC_Obj, "factor", 	sigRect.arrRect[nRectType][n].fPtotalFactor, 1);
					cJSON_AddFloatToObject(pSiglDCDC_Obj, "frequency", 	sigRect.arrRect[nRectType][n].fAC_Freq, 1);

					
					cJSON_AddItemToArray(pDCDC_Array, pSiglDCDC_Obj);
				}
			break;

			default:
				break;
		}
	}

	cJSON_AddNumberToObject(pRoot, "online_acdc_num", 	nOnlineACDCNum);
	cJSON_AddItemToObject(pRoot, "single_acdc_data", 	pACDC_Array);

	cJSON_AddNumberToObject(pRoot, "online_mppt_num", 	nOnlineMPPTNum);
	cJSON_AddItemToObject(pRoot, "single_mppt_data", 	pMPPT_Array);
	
	cJSON_AddNumberToObject(pRoot, "online_dcdc_num", 	nOnlineDCDCNum);
	cJSON_AddItemToObject(pRoot, "single_dcdc_data", 	pDCDC_Array);
	
 	Send_Call(pIFY, pRoot, C_106_SingleRectStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C107_PwrDistrCabStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
#define MAX_WHMETER_NUM		9
static UINT s_ACMterDevMap[MAX_WHMETER_NUM]={101,102,103,104,105,106,0,99, 100};

#define MAX_TEMPHUMIL_NUM	1
static UINT s_TempHumilDevMap[MAX_TEMPHUMIL_NUM]={0};

int Prepare_C107_PwrDistrCabStaNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	UINT nWhichWh = 0, nDev = 0;
	float fACVolt_L1[MAX_WHMETER_NUM] = {0}, fACVolt_L2[MAX_WHMETER_NUM] = {0}, fACVolt_L3[MAX_WHMETER_NUM] = {0};
	float fACCurr_L1[MAX_WHMETER_NUM] = {0}, fACCurr_L2[MAX_WHMETER_NUM] = {0}, fACCurr_L3[MAX_WHMETER_NUM] = {0};
	float fKwhReading[MAX_WHMETER_NUM] = {0}, fNegKwhReading[MAX_WHMETER_NUM] = {0}, fKvarh_reading[MAX_WHMETER_NUM] = {0};
	float fActive_power[MAX_WHMETER_NUM] = {0}, fReactive_power[MAX_WHMETER_NUM] = {0}, fPower_Factor[MAX_WHMETER_NUM] = {0}, fAC_Freq[MAX_WHMETER_NUM] = {0};
	float fAC_PmaxDemand[MAX_WHMETER_NUM] = {0},fAC_CurrentPosDemand[MAX_WHMETER_NUM] = {0};

	float fTemp_Val[MAX_TEMPHUMIL_NUM] = {0}/*, fHumil_Val[MAX_TEMPHUMIL_NUM] = {0}*/;

	PARAM_VAL_GET 		valGet;
	ERRCODE_DAI 		err;

	A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pAllACMeter_Array = cJSON_CreateArray();

	for(nWhichWh = 0; nWhichWh < MAX_WHMETER_NUM; nWhichWh++)
	{
		nDev = 401 + nWhichWh;
		if((nDev < 401) || (nDev > 409))
		{
			TRACE("Error: Wrong Whmeter Dev=[%d]\n", nDev);
			break;
		}

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Not Installed
		{
			TRACEX("Warning: nDev=[%d] Not Installed!\n", nDev);
			continue;
		}

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 413, &valGet, sizeof(PARAM_VAL_GET));
		fACVolt_L1[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 415, &valGet, sizeof(PARAM_VAL_GET));
		fACVolt_L2[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 417, &valGet, sizeof(PARAM_VAL_GET));
		fACVolt_L3[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 414, &valGet, sizeof(PARAM_VAL_GET));
		fACCurr_L1[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 416, &valGet, sizeof(PARAM_VAL_GET));
		fACCurr_L2[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 418, &valGet, sizeof(PARAM_VAL_GET));
		fACCurr_L3[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 411, &valGet, sizeof(PARAM_VAL_GET));
		fKwhReading[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 422, &valGet, sizeof(PARAM_VAL_GET));
		fNegKwhReading[nWhichWh] = valGet.val.fValue;
		
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 412, &valGet, sizeof(PARAM_VAL_GET));
		fKvarh_reading[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 400, &valGet, sizeof(PARAM_VAL_GET));
		fActive_power[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 404, &valGet, sizeof(PARAM_VAL_GET));
		fReactive_power[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 426, &valGet, sizeof(PARAM_VAL_GET));
		fPower_Factor[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 424, &valGet, sizeof(PARAM_VAL_GET));
		fAC_Freq[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 425, &valGet, sizeof(PARAM_VAL_GET));
		fAC_PmaxDemand[nWhichWh] = valGet.val.fValue;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDev, 432, &valGet, sizeof(PARAM_VAL_GET));
		fAC_CurrentPosDemand[nWhichWh] = valGet.val.fValue;
		cJSON *pACmeter_Obj = cJSON_CreateObject();

		cJSON_AddNumberToObject(pACmeter_Obj, "data_from", 			s_ACMterDevMap[nWhichWh]);  

		cJSON* pVolt_Array = cJSON_CreateArray();
		cJSON_AddItemToArray(pVolt_Array, cJSON_CreateFloat(fACVolt_L1[nWhichWh], 1));
		cJSON_AddItemToArray(pVolt_Array, cJSON_CreateFloat(fACVolt_L2[nWhichWh], 1));
		cJSON_AddItemToArray(pVolt_Array, cJSON_CreateFloat(fACVolt_L3[nWhichWh], 1));
		cJSON_AddItemToObject(pACmeter_Obj, "phase_volts", pVolt_Array);

		cJSON* pCurr_Array = cJSON_CreateArray();
		cJSON_AddItemToArray(pCurr_Array, cJSON_CreateFloat(fACCurr_L1[nWhichWh], 1));
		cJSON_AddItemToArray(pCurr_Array, cJSON_CreateFloat(fACCurr_L2[nWhichWh], 1));
		cJSON_AddItemToArray(pCurr_Array, cJSON_CreateFloat(fACCurr_L3[nWhichWh], 1));
		cJSON_AddItemToObject(pACmeter_Obj, "phase_currs", pCurr_Array);
		
		cJSON_AddFloatToObject(pACmeter_Obj, "kwh_reading", 		fKwhReading[nWhichWh],  1);
		cJSON_AddFloatToObject(pACmeter_Obj, "neg_kwh_reading", 	fNegKwhReading[nWhichWh],  1);
		cJSON_AddFloatToObject(pACmeter_Obj, "kvarh_reading", 		fKvarh_reading[nWhichWh],  1);
		cJSON_AddFloatToObject(pACmeter_Obj, "active_power", 		fActive_power[nWhichWh],  1);
		cJSON_AddFloatToObject(pACmeter_Obj, "reactive_power", 		fReactive_power[nWhichWh],  1);
		
		cJSON_AddFloatToObject(pACmeter_Obj, "power_factor", 		fPower_Factor[nWhichWh],  2);
		cJSON_AddFloatToObject(pACmeter_Obj, "ac_freq", 			fAC_Freq[nWhichWh],  3);
		cJSON_AddFloatToObject(pACmeter_Obj, "ac_pmaxdemand", 		fAC_PmaxDemand[nWhichWh],  3);
		cJSON_AddFloatToObject(pACmeter_Obj, "ac_currentdemand", 	fAC_CurrentPosDemand[nWhichWh],  3);

		cJSON_AddItemToArray(pAllACMeter_Array, pACmeter_Obj);

	}
 	 
	cJSON_AddItemToObject(pRoot, "ac_metervalue", pAllACMeter_Array);

	//add by lpl 24-05-15 for UartServer-TempControlDev Data
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		901, 910, &valGet, sizeof(PARAM_VAL_GET));
	TRACEW("----IFY-CLOUD  UartServer Install Type = %d ",valGet.val.emValue);
	if(valGet.val.emValue != 0)// Installed
	{
		cJSON* pAllTempHumil_Array = cJSON_CreateArray();
		for(nWhichWh = 0; nWhichWh < MAX_TEMPHUMIL_NUM; nWhichWh++)
		{
			nDev = 911 + nWhichWh;
			if(nDev < 911)
			{
				TRACEE("Error: Wrong TempHumil Dev=[%d]\n", nDev);
				break;
			}
			valGet.nGetSize = 4;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					nDev, 915, &valGet, sizeof(PARAM_VAL_GET));
			if(valGet.val.emValue == 0)//Not Installed
			{
				TRACEE("Warning: nDev=[%d] Not Installed!\n", nDev);
				continue;
			}
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 930, &valGet, sizeof(PARAM_VAL_GET));
			fTemp_Val[nWhichWh] = valGet.val.fValue;

			// srand(time(0));
			// fTemp_Val[nWhichWh] = 40.0 + ((float)rand() / RAND_MAX) * 10.0;
			//TRACEE("============================================");

			cJSON *pTempHumil_Obj = cJSON_CreateObject();

			cJSON_AddNumberToObject(pTempHumil_Obj, "data_from", 	s_TempHumilDevMap[nWhichWh]);  
			
			cJSON_AddFloatToObject(pTempHumil_Obj, "tl_val", 		fTemp_Val[nWhichWh],  1);
			//cJSON_AddFloatToObject(pTempHumil_Obj, "Humil_val", 	fHumil_Val[nWhichWh],  1);

			cJSON_AddItemToArray(pAllTempHumil_Array, pTempHumil_Obj);
		}
		cJSON_AddItemToObject(pRoot, "transformer", pAllTempHumil_Array);
	}
	
	esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEE("Error: DAI Get Apply Scenery Config Failed err=[%d]!\n", err);
		return 0;
	}

	float fpacload = 0;
	switch (esRunData.emAuxPos)
	{
		case AUXMETER_NOT_INSTALLED:   //grid - cab
			fpacload = fActive_power[7] - fActive_power[6]; 
			cJSON_AddFloatToObject(pRoot, "pacload", fpacload, 1);
			break;

		case AUXMETER_POS_BRANCH_TRANSFER://aux - cab
			fpacload = fActive_power[8] - fActive_power[6]; 
			cJSON_AddFloatToObject(pRoot, "pacload", fpacload, 1);
			break;

		case AUXMETER_POS_CIVIL_ELECTRIC: //grid - cab
			fpacload = fActive_power[7] - fActive_power[6]; 
			cJSON_AddFloatToObject(pRoot, "pacload", fpacload, 1);
			break;

		case AUXMETER_POS_ACLOAD: //aux
			fpacload = fActive_power[8]; 
			cJSON_AddFloatToObject(pRoot, "pacload", fpacload, 1);
			break;

		case AUXMETER_MULTI_BANCH://grid - cab
			fpacload = fActive_power[7] - fActive_power[6]; 
			cJSON_AddFloatToObject(pRoot, "pacload", fpacload, 1);
			break;

		default:
			break;
	}
	
 	Send_Call(pIFY, pRoot, C_107_PwrDistrCabStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
} 

int Prepare_C108_SysVersionStaNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
		
	char			szIMMU2_SWVer[MAX_LEN_MU2SWVER];
    char            szIMMU2_HWVer[8];
	memset(szIMMU2_SWVer, 0 ,sizeof(szIMMU2_SWVer));
	memset(szIMMU2_HWVer, 0 ,sizeof(szIMMU2_HWVer));
	
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;
	ZERO_POBJS(&valGet, 1);

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pVer_Array = cJSON_CreateArray();
	cJSON *pMainVer_Obj = cJSON_CreateObject();
	//cJSON *pSlvVer_Obj = cJSON_CreateObject();

	//if(0 == 0)
	{
		cJSON_AddNumberToObject(pMainVer_Obj, "data_from", 		0);
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R,0, 0, szIMMU2_SWVer, MAX_LEN_MU2SWVER);
		if(err == ERRCODE_DAI_OK)
		{
			cJSON_AddStringToObject(pMainVer_Obj, "sw_main_ver",	szIMMU2_SWVer);
		}
		else
		{
			cJSON_AddStringToObject(pMainVer_Obj, "sw_main_ver",	"--");
		}
		cJSON_AddStringToObject(pMainVer_Obj, "hw_main_ver",	GetIMMU2_HWVersion());

		//IMSU-D version
		A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
		ZERO_POBJS(&blobCCUSett, 1);
		valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
		valGet.val.blobValue = (BYTE*)&blobCCUSett;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 200, &valGet, sizeof(PARAM_VAL_GET));
		if(err == ERRCODE_DAI_OK)
		{
			cJSON_AddStringToObject(pMainVer_Obj, "sw_lccu_ver",	blobCCUSett.ccuSettings.szSWVer);
		}
		else
		{
			cJSON_AddStringToObject(pMainVer_Obj, "sw_lccu_ver",	"--");
		}
		cJSON_AddStringToObject(pMainVer_Obj, "hw_lccu_ver",	"--");

		A_BLOB_NETWORKINFO netInfo;
		valGet.nGetSize = sizeof(A_BLOB_NETWORKINFO);
		valGet.val.blobValue = (BYTE*)&netInfo;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 3, &valGet, sizeof(PARAM_VAL_GET));
		if(err == ERRCODE_DAI_OK)
		{
			cJSON_AddStringToObject(pMainVer_Obj, "simcard_iccid",	netInfo.lteModemINfo.szICCID);
		}
		else
		{
			cJSON_AddStringToObject(pMainVer_Obj, "simcard_iccid",	"--");
		}
		
		cJSON_AddItemToArray(pVer_Array, pMainVer_Obj);
	}

	cJSON_AddItemToObject(pRoot, "sw_dev_ver", pVer_Array);
	
	Send_Call(pIFY, pRoot, C_108_SysVersionStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}


static ALARM_INFO* GetCfgAlarm_ById(ES_IFY_DATA_STU* pIFY, int nAlarmId)
{
	ALARM_INFO* pAlarm = NULL;
 	int n = 0;
 	ALARM_INFO *pCfgAlarm = &pIFY->pAllAlarmInfo[0];
	if(pCfgAlarm == NULL)
	{
		return pAlarm;
	}
	for(n = 0; n < pIFY->nAlarmInfosTotal; n++, pCfgAlarm++)
	{
		if(pCfgAlarm->nAlarmId == nAlarmId)
		{
			pAlarm = pCfgAlarm;
			break;
		}
	}
	return pAlarm;
}

BOOL CheckIfNeedShieldAlarms(int n,ES_IFY_DATA_STU* pIFY)
{
	ALARM_INFO *pCfgAlmInfo = NULL;
	int index = 0;

	pCfgAlmInfo = GetCfgAlarm_ById(pIFY,	pIFY->activeAlms[n].nAlarmId);
	if(pCfgAlmInfo == NULL)
	{
		TRACEE("IFY-Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", \
			pIFY->activeAlms[n].nDevId,pIFY->activeAlms[n].nAlarmId);
		return TRUE;
	}
	if (pCfgAlmInfo->emLevel == ALARM_LEVEL_OBSERVATION)  //OA level do not reporter
	{
		TRACEW("pIFY->activeAlms[n].nAlarmId[%d] == ALARM_LEVEL_OBSERVATION",pIFY->activeAlms[n].nAlarmId);
		return TRUE;
	}
	for (index = 0;index < pIFY->nShieldAlarmsNum;index++)
	{
		if (pIFY->activeAlms[n].nAlarmId == pIFY->ShieldAlarms[index])
		{
			TRACEW("pIFY->activeAlms[n].nAlarmId[%d] == pIFY->ShieldAlarms_id[%d]",pIFY->activeAlms[n].nAlarmId,pIFY->ShieldAlarms[index]);
			return TRUE;
		}
	}
	return FALSE;
}

int Prepare_C109_SysActvAlmNotif(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	int n = 0,index = 0,nCount = 0,nRet = FALSE,nNeedReporterAlmCount = 0;
	//ERRCODE_DAI 	err;
	//PARAM_VAL_GET 	valGet;
	char szDevName[32];

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pActvAlm_Array = cJSON_CreateArray();

	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	
	ALARM_INFO *pCfgAlmInfo = NULL;
	for(n = 0; n < pIFY->nActiveAlmTotal; n++)   //count need reporter alarms sum
	{
		nRet = CheckIfNeedShieldAlarms(n,pIFY);
		if (nRet == FALSE)
		{
			nCount++;
		}
	}
	TRACED("NeesReporterAlarmCount=[%d]  pIFY->nActiveAlmTotal=[%d]",nCount,pIFY->nActiveAlmTotal);
	// if (nCount == 0)
	// {
	// 	return 0;	//when all activeAlarms are shielded,no need to reporter
	// }
	cJSON_AddNumberToObject(pRoot, "total_actv_alm", nCount);
 	for(n = 0; n < pIFY->nActiveAlmTotal; n++)
	{
		nRet = CheckIfNeedShieldAlarms(n,pIFY);
		if (nRet == TRUE)
		{
			continue;
		}
		pCfgAlmInfo = GetCfgAlarm_ById(pIFY,	pIFY->activeAlms[n].nAlarmId);
		if(pCfgAlmInfo == NULL)
		{
			TRACE("Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", \
				pIFY->activeAlms[n].nDevId,pIFY->activeAlms[n].nAlarmId);
			continue;
		}
		cJSON *pActvAlm_Obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alarm_id", 			pCfgAlmInfo->nAlarmId);
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_level", 			pCfgAlmInfo->emLevel);

		memset(szDevName, 0, sizeof(szDevName));
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETDEVNAME_BYID_R,
				pIFY->activeAlms[n].nDevId, 0, szDevName, 32);
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_dev",				szDevName);//pCfgAlmInfo->pBelonged_DevModel->szDevModelName);
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_name",				pCfgAlmInfo->pRes_Name->szName);
		
		char szTime[32];
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_begin_time", 		Make_ESIFYTimeString(pIFY->activeAlms[n].tmStart, szTime));

		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_status", 			(pIFY->activeAlms[n].status == ALARM_STATUS_ACTIVE) ? 0 : 1);
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_dev_id", 			pIFY->activeAlms[n].nDevId);

		cJSON_AddItemToArray(pActvAlm_Array, pActvAlm_Obj);
	}

 
	cJSON_AddItemToObject(pRoot, "actv_alm_info", pActvAlm_Array);
	
	Send_Call(pIFY, pRoot, C_109_SysActvAlmNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}



int Prepare_C110_SiglBatDataNotif(void* pArg)
{
	ES_IFY_DATA_STU *pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	int m = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;
	
	if(pIFY->nCurrBMS >= MAX_SINGLE_STBATT_NUM)
	{
		return -1;
	}

 	A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
	ZERO_POBJS(&blobIfySiglBat, 1);
	blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
	valGet.val.blobValue = (BYTE*)&blobIfySiglBat;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501 + pIFY->nCurrBMS, 557, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get IFY Single BMS Data Failed!!!\n");
		return -1;
	}
	
	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", 0);
	cJSON_AddNumberToObject(pRoot, "batt_idx", 	pIFY->nCurrBMS);
	cJSON_AddNumberToObject(pRoot, "data_type", 0);

	//------Cell Voltagle-----//
	cJSON* pVolt_Array = cJSON_CreateArray();
	for(m = 0; m < blobIfySiglBat.usCellVoltTotalNum; m++)
	{
		cJSON_AddItemToArray(pVolt_Array, cJSON_CreateNumber(blobIfySiglBat.usCellVolt[m]));
	}
	cJSON_AddItemToObject(pRoot, "sigl_cell_volt_data", pVolt_Array);

 	//-------Cell Temp-------//
	cJSON* pTemp_Array = cJSON_CreateArray();
 	for(m = 0; m < blobIfySiglBat.usCellTempTotalNum; m++)
	{
		cJSON_AddItemToArray(pTemp_Array, cJSON_CreateNumber(blobIfySiglBat.cCellTemp[m]));
	}
	cJSON_AddItemToObject(pRoot, "sigl_cell_temp_data", pTemp_Array);

	//-------Unbalnace-------//
	cJSON* pUnblance_Array = cJSON_CreateArray();
 	for(m = 0; m < blobIfySiglBat.usCellVoltTotalNum; m++)
	{
		cJSON_AddItemToArray(pUnblance_Array, cJSON_CreateNumber(blobIfySiglBat.byCellBalanceStat[m]));
	}

	cJSON_AddItemToObject(pRoot, "sigl_cell_balance_data", pUnblance_Array);

	//-------Unsafe-------//
	cJSON* pUnsafe_Array = cJSON_CreateArray();
 	for(m = 0; m < blobIfySiglBat.usCellVoltTotalNum; m++)
	{
		cJSON_AddItemToArray(pUnsafe_Array, cJSON_CreateNumber(blobIfySiglBat.byCellSafeStat[m]));
	}

	cJSON_AddItemToObject(pRoot, "sigl_cell_safe_data", pUnsafe_Array);

	Send_Call(pIFY, pRoot, C_110_SingleBattryInfo, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

int Prepare_C202_HourSectorStaticKwh(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	int 			n = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;

	int nDevID = 0, nSigID = 0;

	char szTime[32];
	time_t tmNow = _NOW_;

	nDevID = 200;
	nSigID = 79;

	A_BLOB_ES_STATISTIC_DATA	blobStatData;
	ZERO_POBJS(&blobStatData ,	1);
	blobStatData.emBlobType = BLOB_TYPE_ES_STATISTICDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
	valGet.val.blobValue = (BYTE*)&blobStatData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q202 Failed err=[%d]!\n", err);
		return 0;
	}

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pACRealTime_Array = cJSON_CreateArray();
	cJSON* pACCost_Array = cJSON_CreateArray();
	cJSON* pOrgLoadCost_Array = cJSON_CreateArray();
	cJSON* pRmtACRevs_Array = cJSON_CreateArray();
	cJSON* pLocalACIN_Array = cJSON_CreateArray();
	cJSON* pLocalACOut_Array = cJSON_CreateArray();
	cJSON* pDCLoadCost_Array = cJSON_CreateArray();
	
	cJSON* pMpptOut_Array = cJSON_CreateArray();
	cJSON* pBattGrp_Array = cJSON_CreateArray();
 
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	0);

	TRACE("Q202: nCurrDataFrom=[%d]\n", 0);

	for(n = 0; n < 48; n++)
	{
		cJSON_AddItemToArray(pACRealTime_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fAC_CurrKWH_Reading[n], 1));
		cJSON_AddItemToArray(pACCost_Array, 		cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fAC_CostKWH_Sector[n], 1));
		cJSON_AddItemToArray(pOrgLoadCost_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[n], 1));
		cJSON_AddItemToArray(pRmtACRevs_Array, 		cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[n], 1));		
		cJSON_AddItemToArray(pLocalACIN_Array, 		cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[n], 1));
		cJSON_AddItemToArray(pLocalACOut_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[n], 1));
		cJSON_AddItemToArray(pDCLoadCost_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[n], 1));
	}

	int nGrp = 0;
	for(nGrp = 0; nGrp < pIFY->nCfgBMSNum; nGrp++)
	{
		cJSON *pMppt_Obj = cJSON_CreateObject();
		cJSON* pMpptValue_Array = cJSON_CreateArray();
		for(n = 0; n < 48; n++)
		{
			cJSON_AddItemToArray(pMpptValue_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fMppt_GKWH_Sector[nGrp][n], 1));
		}
		
		cJSON_AddItemToObject(pMppt_Obj, "value_mppt_output", 	pMpptValue_Array);
		cJSON_AddItemToArray(pMpptOut_Array, pMppt_Obj);
 	}

	for(nGrp = 0; nGrp < pIFY->nCfgBMSNum; nGrp++)
	{
		cJSON *pBatt_Obj = cJSON_CreateObject();
		cJSON* pBattChrg_Array = cJSON_CreateArray();
		cJSON* pBattDisChrg_Array = cJSON_CreateArray();

		for(n = 0; n < 48; n++)
		{
			cJSON_AddItemToArray(pBattChrg_Array, 		cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[nGrp][n], 	1));
			cJSON_AddItemToArray(pBattDisChrg_Array, 	cJSON_CreateFloat(blobStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[nGrp][n], 1));
		}
		
		cJSON_AddItemToObject(pBatt_Obj, "value_batt_chrg", 	pBattChrg_Array);
		cJSON_AddItemToObject(pBatt_Obj, "value_batt_dischrg", 	pBattDisChrg_Array);

		cJSON_AddItemToArray(pBattGrp_Array, pBatt_Obj);
 	}

	cJSON_AddItemToObject(pRoot, "rmt_ac_realtime_kwh", 	pACRealTime_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_cost_kwh", 		pACCost_Array);
	cJSON_AddItemToObject(pRoot, "org_acload_cost_kwh", 	pOrgLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_revs_kwh", 		pRmtACRevs_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_input_kwh", 		pLocalACIN_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_output_kwh", 	pLocalACOut_Array);
	cJSON_AddItemToObject(pRoot, "dcload_cost_kwh", 		pDCLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "mppt_grp_output_kwh", 	pMpptOut_Array);
	cJSON_AddItemToObject(pRoot, "batt_grp_chrgdischrgkwh", pBattGrp_Array);

	cJSON_AddStringToObject(pRoot, "time_stamp", Make_ESIFYTimeString(tmNow, szTime));
	
	Send_Call(pIFY, pRoot, C_202_HourSectorStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

int Prepare_C204_DailyStaticKwh(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	static UINT nLast_Day = 0;

	int nGrp = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;

	int nDevID = 0, nSigID = 0;

	nDevID = 200;
	nSigID = 79;

	time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    UINT nCurr_Day = gmTime.tm_mday - 1;
 	if((gmTime.tm_hour == 0) && (gmTime.tm_min < 30)) //Before 00:30 Send Last Day Value
	{
		nCurr_Day = nLast_Day;
	}

	if(nCurr_Day != nLast_Day)
	{
		nLast_Day = nCurr_Day;
	}

	if(nCurr_Day > MAX_DAYS_SECTOR)
	{
		TRACE("nCurr_Day=[%d] is Outof Range,GoBack!\n");
		return -1;
	}
	A_BLOB_ES_STATISTIC_DATA	blobStatData;
	ZERO_POBJS(&blobStatData ,	1);
	blobStatData.emBlobType = BLOB_TYPE_ES_STATISTICDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
	valGet.val.blobValue = (BYTE*)&blobStatData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, nSigID, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q204 Failed err=[%d]!\n", err);
		return -1;
	}

	if((blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day] == 0))
	{
		TRACE("nCurr_Day=[%d] Data Is Not Saved Yet,Go Back!\n");
		return 0;
	}
	cJSON *pRoot = cJSON_CreateObject();
 
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	0);
 
	char szTime[32];
	cJSON_AddStringToObject(pRoot, "time_stamp", Make_ESIFYTimeString(blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day], szTime));

	cJSON_AddFloatToObject(pRoot, "rmt_ac_realtime_kwh",		blobStatData.es_Data.days_Data.fAC_CurrKWH_Reading[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_cost_kwh", 			blobStatData.es_Data.days_Data.fACTotalCostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_cost_kwh", 		blobStatData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_revs_kwh", 			blobStatData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nCurr_Day],  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_input_kwh", 		blobStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_output_kwh", 		blobStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "dcload_cost_kwh", 			blobStatData.es_Data.days_Data.fDCLoad_CostKWH_D[nCurr_Day],  1);
	
	cJSON* pMppt_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < pIFY->nCfgBMSNum; nGrp++)
	{
		cJSON *pMppt_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_total_output", 	blobStatData.es_Data.days_Data.fMpptTotalGKWH_D[nGrp][nCurr_Day],  1);
		
		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_sharp_output", 	blobStatData.es_Data.days_Data.fMppt_Sharp_GKWH_D[nGrp][nCurr_Day],  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_peak_output", 	blobStatData.es_Data.days_Data.fMppt_Peak_GKWH_D[nGrp][nCurr_Day],  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_flat_output", 	blobStatData.es_Data.days_Data.fMppt_Flat_GKWH_D[nGrp][nCurr_Day],  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_valley_output", 	blobStatData.es_Data.days_Data.fMppt_Valley_GKWH_D[nGrp][nCurr_Day],  1);


		cJSON_AddItemToArray(pMppt_Array, pMppt_Obj);
	}
	cJSON_AddItemToObject(pRoot, "mppt_grp_output_kwh", pMppt_Array);

	cJSON* pBattGrp_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < pIFY->nCfgBMSNum; nGrp++)
	{
		cJSON *pBattGrp_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_chrg", 	blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[nGrp][nCurr_Day],  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_dischrg", 	blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[nGrp][nCurr_Day],  1);
		
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_sharp_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_peak_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_flat_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_valley_chrg", 		0.0,  1);

		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_sharp_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_peak_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_flat_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_valley_dischrg", 	0.0,  1);

		cJSON_AddItemToArray(pBattGrp_Array, pBattGrp_Obj);
	}
	cJSON_AddItemToObject(pRoot, "batt_grp_chrgdischrgkwh", 		pBattGrp_Array);
 
	cJSON_AddFloatToObject(pRoot, "local_ac_sharpinput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_peakinput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_flatinput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_valleyinput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nCurr_Day],  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_sharpoutput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nCurr_Day],  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_peakoutput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_flatoutput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_valleyoutput_kwh", 	blobStatData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nCurr_Day],  1);
	
	cJSON_AddFloatToObject(pRoot, "org_acload_sharpcost_kwh", 	blobStatData.es_Data.days_Data.fOrgACLoad_Sharp_CostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_peakcost_kwh", 	blobStatData.es_Data.days_Data.fOrgACLoad_Peak_CostKWH_D[nCurr_Day],  1);

	cJSON_AddFloatToObject(pRoot, "org_acload_flatcost_kwh", 	blobStatData.es_Data.days_Data.fOrgACLoad_Flat_CostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_valleycost_kwh", 	blobStatData.es_Data.days_Data.fOrgACLoad_Valley_CostKWH_D[nCurr_Day],  1);
	
 	cJSON* pExcTimes_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < pIFY->nCfgBMSNum; nGrp++)
	{
		cJSON *pExcTimes_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_chrg_times", 		blobStatData.es_Data.days_Data.fBattChrgTimes_D[nGrp][nCurr_Day],  2);
		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_dischrg_times", 	blobStatData.es_Data.days_Data.fBattDisChrgTimes_D[nGrp][nCurr_Day],  2);
		cJSON_AddItemToArray(pExcTimes_Array, pExcTimes_Obj);
	}
	cJSON_AddItemToObject(pRoot, "batt_chrgdischrg_times", pExcTimes_Array);
 	
	Send_Call(pIFY, pRoot, C_204_DailyStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

char* Make_ESiOCPPTimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	sprintf(timeinstring, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

#define GetFeeRateKWHDevNum 2
#define GetFeeRateKWHTimeNum 3  //current,last1,last2
static UINT s_GetFeeRateKWHDevMap[GetFeeRateKWHDevNum]={407,401};
int Prepare_C206_SharpPeakFlatValleyKwh(void* pArg)
{
	ES_IFY_DATA_STU* pOCPP = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pOCPP->hReporter;

	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;
	time_t tmNow = _NOW_;

	UINT nWhichWh = 0, nDev = 0, nTimeNum = 0;
	float fPosSumKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegSumKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosSharpKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegSharpKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosPeakKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegPeakKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosFlatKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegFlatKwh[GetFeeRateKWHTimeNum][10] ;
	float fPosWaveKwh[GetFeeRateKWHTimeNum][10] ;
	float fNegWaveKwh[GetFeeRateKWHTimeNum][10] ;

	memset(fPosSumKwh, 0, sizeof(fPosSumKwh));
    memset(fNegSumKwh, 0, sizeof(fNegSumKwh));
    memset(fPosSharpKwh, 0, sizeof(fPosSharpKwh));
    memset(fNegSharpKwh, 0, sizeof(fNegSharpKwh));
    memset(fPosPeakKwh, 0, sizeof(fPosPeakKwh));
    memset(fNegPeakKwh, 0, sizeof(fNegPeakKwh));
    memset(fPosFlatKwh, 0, sizeof(fPosFlatKwh));
    memset(fNegFlatKwh, 0, sizeof(fNegFlatKwh));
    memset(fPosWaveKwh, 0, sizeof(fPosWaveKwh));
    memset(fNegWaveKwh, 0, sizeof(fNegWaveKwh));

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pFEERATE_KWH_ARRAY = cJSON_CreateArray();

	for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	{
		nDev = s_GetFeeRateKWHDevMap[nWhichWh]; //

		valGet.nGetSize = 4;
		VAR_VALUE varV;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Not Installed
		{
			TRACEX("Warning: nDev=[%d] Not Installed!\n", nDev);
			continue;
		}
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
				nDev, 401, &varV, 4);
		if (err == ERRCODE_DAI_OK && varV.emValue != 0)
		{
			TRACEX("Warning: nDev=[%d] kWhMeter CommFail!\n", nDev);
			
			continue;
		}
		for (nTimeNum = 0; nTimeNum < GetFeeRateKWHTimeNum; nTimeNum++) //current,last1,last2
		{
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1491 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fPosSumKwh[nTimeNum][0] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1492 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fPosSharpKwh[nTimeNum][1] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1493 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fPosPeakKwh[nTimeNum][2] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1494 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fPosFlatKwh[nTimeNum][3] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1495 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fPosWaveKwh[nTimeNum][4] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1496 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fNegSumKwh[nTimeNum][5] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1497 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fNegSharpKwh[nTimeNum][6] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1498 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fNegPeakKwh[nTimeNum][7] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);
			
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1499 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fNegFlatKwh[nTimeNum][8] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				nDev, 1500 + nTimeNum * 10, &valGet, sizeof(PARAM_VAL_GET));
			fNegWaveKwh[nTimeNum][9] = (err == ERRCODE_DAI_OK ? valGet.val.fValue : 0.0f);
		}

		cJSON *pAllFeeRateKwh_Obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(pAllFeeRateKwh_Obj, "data_from", 			nDev); 

		cJSON* pTime_Array = cJSON_CreateArray();
		for (nTimeNum = 0; nTimeNum < GetFeeRateKWHTimeNum; nTimeNum++) //current,last1,last2
		{
			cJSON *pFeeRateKwh_Obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(pFeeRateKwh_Obj, "data_from", 			nTimeNum);  
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sum_pos_kwh", 			fPosSumKwh[nTimeNum][0],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sharp_pos_kwh", 		fPosSharpKwh[nTimeNum][1],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "peak_pos_kwh", 		fPosPeakKwh[nTimeNum][2],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "flat_pos_kwh", 		fPosFlatKwh[nTimeNum][3],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "valley_pos_kwh", 		fPosWaveKwh[nTimeNum][4],  3);
			
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sum_neg_kwh", 			fNegSumKwh[nTimeNum][5],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sharp_neg_kwh", 		fNegSharpKwh[nTimeNum][6],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "peak_neg_kwh", 		fNegPeakKwh[nTimeNum][7],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "flat_neg_kwh", 		fNegFlatKwh[nTimeNum][8],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "valley_neg_kwh", 		fNegWaveKwh[nTimeNum][9],  3);

			cJSON_AddItemToArray(pTime_Array, pFeeRateKwh_Obj);
		}
		cJSON_AddItemToObject(pAllFeeRateKwh_Obj, "time_feerate_kwh", pTime_Array);

		cJSON_AddItemToArray(pFEERATE_KWH_ARRAY, pAllFeeRateKwh_Obj);
		
	}

	cJSON_AddItemToObject(pRoot, "feerate_kwh", pFEERATE_KWH_ARRAY);

	//time
	char szTime[32];
	cJSON_AddStringToObject(pRoot, "time", Make_ESiOCPPTimeString(tmNow, szTime));

	Send_Call(pOCPP, pRoot, C_206_FeeRateKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S111_TriggerMessage()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Handle_S111_TriggerMessage(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	TRACEX("Handle_TriggerMessage \n");

	cJSON *pJson = pIFY->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	int ConnectorId = -1;
	OBJ_PTR obj;

	if (temp1 = cJSON_GetObjectItem(pJson, "trig_msg"))
	{
		if(temp1->valuestring != NULL)
		{
			if (strcmp(temp1->valuestring, "101") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C101;
			}
			else if(strcmp(temp1->valuestring, "102") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C102;
			}
			else if (strcmp(temp1->valuestring, "103") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C103;
			}
			else if (strcmp(temp1->valuestring, "105") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C105;
			}
			else if (strcmp(temp1->valuestring, "107") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C107;
			}
			else if (strcmp(temp1->valuestring, "206") == NULL)
			{
				pIFY->emTriggerMsgType = Trigger_C206;
			}// edit by kuangyue 20250517
			else 
			{
				Send_CallError(pIFY, S_111_Trigger, CallError_FormatError, "TriggleMsg is not recognized!");
				return -1;
			}
		}
	}
	else
	{
		Send_CallError(pIFY, S_111_Trigger, CallError_FormatError, "requestedMessage is missing!");
		return -1;
	}
	
	ERRCODE_DAI err;
	err = ERRCODE_DAI_OK;
	pResp = cJSON_CreateObject();
	//cJSON_AddStringToObject(pResp, "status", (pIFY->emTriggerMsgType != Trigger_NoTriggle) ? "Accepted" : "Rejected");
	cJSON_AddStringToObject(pResp, "accept", (pIFY->emTriggerMsgType != Trigger_NoTriggle) ? 0 : 1);
	Send_CallResult(pIFY, pResp, S_111_Trigger);

	switch (pIFY->emTriggerMsgType)
	{
	case Trigger_C101:
		pIFY->stMsgInfoList[C_101_StatusNotif].pfnMsgHandler((void*)pIFY);
		break;
	case Trigger_C102:
		pIFY->stMsgInfoList[C_102_BattCabStaNotif].pfnMsgHandler((void*)pIFY);
		break;
	case Trigger_C103:
		pIFY->stMsgInfoList[C_103_ESBMSStaNotif].pfnMsgHandler((void*)pIFY);
		break;
	case Trigger_C105:
		pIFY->stMsgInfoList[C_105_ESRunDataStaNotif].pfnMsgHandler((void*)pIFY);
		break;
	case Trigger_C107:
		pIFY->stMsgInfoList[C_107_PwrDistrCabStaNotif].pfnMsgHandler((void*)pIFY);
		break;
	case Trigger_C206:
		pIFY->stMsgInfoList[C_206_FeeRateKwh].pfnMsgHandler((void*)pIFY);
		break;
	default:
		break;
	}

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S401_Reset()
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	TRACEX("Handle_Reset\n");

	cJSON *pJson = pIFY->pCurrJson;
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
		Send_CallError(pIFY, S_401_RmtRest, CallError_FormatError, "Reset type is missing!");
		return -1;
	}

	BOOL bAccepted = FALSE;
	if(bHardReset != -1)
	{
		VAR_VALUE varV;
		varV.emValue = bHardReset ? 1 : 0;
		ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 2, &varV,4, 3000);
		bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	}
	else
	{
		Send_CallError(pIFY, S_401_RmtRest, CallError_FormatError, "Reset type not correct!");
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", bAccepted ? 1 : 2);
	
	Send_CallResult(pIFY, pResp, S_401_RmtRest);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S402 ()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Handle_S402_ChangeEVSEAvailability(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	cJSON *pJson = pIFY->pCurrJson;
	
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
		Send_CallError(pIFY, S_402_RmtChgEVESEAbi, CallError_FormatError, "cmd_type is missing!");
		return -1;
	}

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", (Status == ChangeAvail_Accepted) ? 1 : 0);

	//Msg_Send(pIFY, pResp, ChangeAvailability, CallResult,0, bSend_Later);
	Send_CallResult(pIFY, pResp, S_402_RmtChgEVESEAbi);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S404_UpdateFirmware()
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	TRACEX("Handle_UploadFirmware \n");

	cJSON *pJson = pIFY->pCurrJson;
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
			Send_CallError(pIFY, S_404_RmtUpdtFirmw, CallError_FormatError, "location length is not correct!");
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
		//memcpy(cmd.szURL, temp1->valuestring, MIN(255, strlen(temp1->valuestring)));
	}
	else
	{
		Send_CallError(pIFY, S_404_RmtUpdtFirmw, CallError_FormatError, "location is missing!");
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "inst_time"))
	{
		cmd.dtRetrieveDate = Parse_ESIFYTime(temp1->valuestring);
	}
	// else
	// {
	// 	Send_CallError(pIFY, UpdateFirmware, CallError_FormationViolation, "retrieveDate is missing!", NULL);
	// 	return -1;
	// }

	VAR_VALUE varV;
	varV.blobValue = &cmd;
	ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);

	pResp = cJSON_CreateObject();		//resp is empty
	cJSON_AddNumberToObject(pResp, "accept", (err == ERRCODE_DAI_OK) ? 1 : 0);
	
	Send_CallResult(pIFY, pResp, S_404_RmtUpdtFirmw);

	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : Prepare_C405_FirmwareStatusNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C405_FirmwareStatusNotification(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	cJSON *pJson_firmwarestatus;

	pJson_firmwarestatus = cJSON_CreateObject();
	// need to ask Jimmy for Firmware Status 
	//cJSON_AddStringToObject(pJson_firmwarestatus, "status", FIRMWARESTATUS_STRING[pIFY->emFirmwareUpgradeStatus]);
	cJSON_AddNumberToObject(pJson_firmwarestatus, "status", pIFY->emFirmwareUpgradeStatus);

	Send_Call(pIFY, pJson_firmwarestatus, C_405_StatOfFirmw, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S406_GetDiagnostics()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Handle_S406_GetDiagnostics(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	TRACEX("Handle_UploadDiagnostic \n");

	cJSON *pJson = pIFY->pCurrJson;
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
			Send_CallError(pIFY, S_406_RmtCallDiag, CallError_FormatError, "location length is not correct!");
			return -1;
		}
		memcpy(cmd.szURL, temp1->valuestring, nStrlen);
	}
	else
	{
		Send_CallError(pIFY, S_406_RmtCallDiag, CallError_FormatError, "location is missing!");
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "start_time"))
	{
		cmd.dtStartTime = Parse_ESIFYTime(temp1->valuestring);
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "stop_time"))
	{
		cmd.dtStopTime = Parse_ESIFYTime(temp1->valuestring);
	}

	char szTime[32];
	snprintf(cmd.szDiagnosticFileName, 32, "diagnostic.%s", TimeToString(_NOW_, TIME_TRIM_FMT, szTime, 32));

	VAR_VALUE varV;
	varV.blobValue = &cmd;
	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, 9, &varV, sizeof(A_BLOB_UPLOADDIAGN_CMD), 0);

	pResp = cJSON_CreateObject();

	cJSON_AddStringToObject(pResp, "fileName", cmd.szDiagnosticFileName);

	Send_CallResult(pIFY, pResp, S_406_RmtCallDiag);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Prepare C407_DiagnosticsStatusNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
 int Prepare_C407_DiagnosticsStatusNotification(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	cJSON *pJson_Dignostics;

	pJson_Dignostics = cJSON_CreateObject();
	// need to ask Jimmy for Dignostics Status 
	cJSON_AddNumberToObject(pJson_Dignostics, "status", pIFY->emDiagnosUploadStatus);
	//cJSON_AddStringToObject(pJson_Dignostics, "status", DIAGNOSTICSSTATUS_STRING[pIFY->emDiagnosUploadStatus]);

	Send_Call(pIFY, pJson_Dignostics, C_407_StatOfDiag, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

int Handle_S413_DevVersionInfo(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	cJSON *VersionInfo = cJSON_CreateObject(); 
	cJSON *Ver_cuu = cJSON_CreateArray(); 
	cJSON *Ver_mu2 = cJSON_CreateArray(); 
	cJSON *Ver_eu3 = cJSON_CreateArray(); 
	cJSON *Ver_abu_u1 = cJSON_CreateArray(); 
	cJSON *Ver_abu_u2 = cJSON_CreateArray(); 
	cJSON *Ver_bat_idx = cJSON_CreateArray(); 

	PARAM_VAL_GET 	valGet, varV;
	ZERO_POBJS(&valGet, 1);
	ERRCODE_DAI err;
	char szVer[MAX_LEN_MU2SWVER];
	memset(szVer, 0 ,sizeof(szVer));
		
	A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
	ZERO_POBJS(&blobCCUSett, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
	valGet.val.blobValue = (BYTE*)&blobCCUSett;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			200, 200, &valGet, sizeof(PARAM_VAL_GET));
	if(err == ERRCODE_DAI_OK)
	{
		cJSON_AddItemToArray(Ver_cuu, cJSON_CreateString(blobCCUSett.ccuSettings.szSWVer));
	}
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R, 0, 0, szVer, MAX_LEN_MU2SWVER);
	if(err == ERRCODE_DAI_OK)
	{
		cJSON_AddItemToArray(Ver_mu2, cJSON_CreateString(szVer));
	}

	IMEU_TYPE EU_TypeVal;
	varV.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 611, 451, &varV, sizeof(PARAM_VAL_GET));
	EU_TypeVal = (IMEU_TYPE)varV.val.blobValue;
	char szVer_eu3[64];
	if(EU_TypeVal == IMEU_TYPE_IMEU3)
	{
		varV.nGetSize = 64;
		varV.val.blobValue = (BYTE*)szVer_eu3;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 611, 1669, &varV, sizeof(PARAM_VAL_GET));
		if(err == ERRCODE_DAI_OK)
		{
			snprintf(szVer, 32, "%s", szVer_eu3);
			cJSON_AddItemToArray(Ver_eu3, cJSON_CreateString(szVer));
		}		
	}

	A_BLOB_SINGLE_RECT_INFO allsingleRectData;
	ZERO_POBJS(&allsingleRectData, 	1);
	allsingleRectData.emBlobType = BLOB_TYPE_SINGLE_RECT_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
	valGet.val.blobValue = (BYTE*)&allsingleRectData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 101, &valGet, sizeof(PARAM_VAL_GET));

	STU_SIGNLE_RECT_DATA singleRectData;
	singleRectData = allsingleRectData.arrRect[TYPE_SIG_RECT_STS][0];
	if(singleRectData.fRect_PFCVer != 0)
	{
		snprintf(szVer, 32, "%.2f", singleRectData.fRect_PFCVer);
		cJSON_AddItemToArray(Ver_abu_u1, cJSON_CreateString(szVer));
	}
	

	singleRectData = allsingleRectData.arrRect[TYPE_SIG_RECT_STS][0];
	if(singleRectData.fRect_DCDCVer != 0)
	{
		snprintf(szVer, 32, "%.2f", singleRectData.fRect_DCDCVer);
		cJSON_AddItemToArray(Ver_abu_u2, cJSON_CreateString(szVer));
	}

	if(pIFY->nCfgBMSNum > MAX_BATTCABINET_NUM)
	{	
		Send_CallError(pIFY, S_413_RmtGetLocalListVersion, CallError_FormatError, "BattCabinet Cfg is Invalid!");
		return -1;
	}

	int n = 0, i = 0, j = 0;
	for(n = 0; n < pIFY->nCfgBMSNum; n++)
	{
		cJSON *Ver_bat_Group = cJSON_CreateObject(); 

		cJSON *Ver_bms = cJSON_CreateObject(); 
		cJSON *Ver_pcs = cJSON_CreateObject(); 
		cJSON *Ver_temp = cJSON_CreateObject(); 

		cJSON *Ver_bmu = cJSON_CreateArray(); 
		cJSON *Ver_bsu = cJSON_CreateArray(); 
		cJSON *Ver_imeu2 = cJSON_CreateArray(); 
		cJSON *Ver_imeu3 = cJSON_CreateArray(); 

		cJSON *Ver_acdc_u1 = cJSON_CreateArray();
		cJSON *Ver_acdc_u2 = cJSON_CreateArray();
		cJSON *Ver_mppt_u1 = cJSON_CreateArray();
		cJSON *Ver_mppt_u2 = cJSON_CreateArray();
		cJSON *Ver_air = cJSON_CreateArray();
		cJSON *Ver_ariEu3 = cJSON_CreateArray();

		A_BLOB_SINGLE_STBATT_RUNINFO stBattDataInfo;
		ZERO_POBJS(&stBattDataInfo, 1);
		valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		valGet.val.blobValue = (BYTE*)&stBattDataInfo;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				501, 551, &valGet, sizeof(PARAM_VAL_GET));
		snprintf(szVer, 32, "%.3f", stBattDataInfo.fVersion);
		cJSON_AddItemToArray(Ver_bmu, cJSON_CreateString(szVer));

		char szVer_bsu[16];
		varV.nGetSize = 16;
		varV.val.blobValue = (BYTE*)szVer_bsu;
		if(pIFY->lcCfg.emSystemType == SYSTYPE_ESS_AIR)
		{
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 500, 868 + n, &varV, sizeof(PARAM_VAL_GET));
			snprintf(szVer, 16, "%s", szVer_bsu);
			cJSON_AddItemToArray(Ver_bsu, cJSON_CreateString(szVer));
		}
		else if(pIFY->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED && pIFY->devType == LIQUID_COOLING)
		{
			for(i = 0; i < SYSTYPE_ESS_LIQUIDED_PACK_NUM; ++i)
			{
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 500, 868 + i + i*n, &varV, sizeof(PARAM_VAL_GET));
				snprintf(szVer, 16, "%s", szVer_bsu);
				cJSON_AddItemToArray(Ver_bsu, cJSON_CreateString(szVer));
			} 
		}
		else if(pIFY->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED && pIFY->devType == LIQUID_COOLING_BACKUP)
		{
			for(i = 0; i < LIQUID_COOLING_BACKUP_PACK_NUM; ++i)
			{
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 500, 868 + i + i*n, &varV, sizeof(PARAM_VAL_GET));
				snprintf(szVer, 16, "%s", szVer_bsu);
				cJSON_AddItemToArray(Ver_bsu, cJSON_CreateString(szVer));
			} 				
		}

		varV.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 601 + n, 450, &varV, sizeof(PARAM_VAL_GET));
		EU_TypeVal = (IMEU_TYPE)varV.val.blobValue;

		A_BLOB_BATTCAB_IMEU2_V20 EU2;
		ZERO_POBJS(&EU2, 1);
		switch(EU_TypeVal)
		{
			case IMEU_TYPE_IMEU2:
				EU2.emBlobType = BLOB_TYPE_BATTCAB_IMEU2_V20;
				valGet.nGetSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);

				valGet.val.blobValue = (BYTE*)&EU2;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601 + n, 450, &valGet, sizeof(PARAM_VAL_GET));
							
				snprintf(szVer, 16, "%s", EU2.szSoftwareVer);
				cJSON_AddItemToArray(Ver_imeu2, cJSON_CreateString(szVer));
			break;
			case IMEU_TYPE_IMEU3:
				varV.nGetSize = 64;
				varV.val.blobValue = (BYTE*)szVer_eu3;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601 + n, 1633, &varV, sizeof(PARAM_VAL_GET));
				snprintf(szVer, 32, "%s", szVer_eu3);
				cJSON_AddItemToArray(Ver_imeu3, cJSON_CreateString(szVer));	
			break;
		}

		UINT single_idx = n; 
		for(i = 0; i < (int)allsingleRectData.nOnlineNum[single_idx]; ++i)
		{
			singleRectData = allsingleRectData.arrRect[single_idx][i];
			snprintf(szVer, 32, "%.2f", singleRectData.fRect_PFCVer);
			cJSON_AddItemToArray(Ver_acdc_u1, cJSON_CreateString(szVer));
		}

		for(i = 0; i < (int)allsingleRectData.nOnlineNum[single_idx]; ++i)
		{
			singleRectData = allsingleRectData.arrRect[single_idx][i];
			snprintf(szVer, 32, "%.2f", singleRectData.fRect_DCDCVer);
			cJSON_AddItemToArray(Ver_acdc_u2, cJSON_CreateString(szVer));
		}

		single_idx = n + TYPE_SIG_RECT_MPPT_GRP1; 
		for(i = 0; i < (int)allsingleRectData.nOnlineNum[single_idx]; ++i)
		{
			singleRectData = allsingleRectData.arrRect[single_idx][i];
			snprintf(szVer, 32, "%.2f", singleRectData.fRect_PFCVer);
			cJSON_AddItemToArray(Ver_mppt_u1, cJSON_CreateString(szVer));
		}

		for(i = 0; i < (int)allsingleRectData.nOnlineNum[single_idx]; ++i)
		{
			singleRectData = allsingleRectData.arrRect[single_idx][i];
			snprintf(szVer, 32, "%.2f", singleRectData.fRect_DCDCVer);
			cJSON_AddItemToArray(Ver_mppt_u2, cJSON_CreateString(szVer));
		}
		char szVer_air[16];
		varV.val.blobValue = (BYTE*)szVer_air;
		switch(pIFY->emAirType)
		{
			case INFYPOWER_AIR:
				varV.nGetSize = 16;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,951 + n, 1562, &varV, sizeof(PARAM_VAL_GET));
				snprintf(szVer, 16, "%s", szVer_air);
				if(strlen(szVer) > 0)
				{
					cJSON_AddItemToArray(Ver_air, cJSON_CreateString(szVer));
				}

				break;
			case IMEU3_AIR:
				varV.nGetSize = 16;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,951 + n, 3500, &varV, sizeof(PARAM_VAL_GET));
				snprintf(szVer, 16, "%s", szVer_air);
				if(strlen(szVer) > 0)
				{
					cJSON_AddItemToArray(Ver_air, cJSON_CreateString(szVer));
				}
				break;
		}
		cJSON_AddItemToObject(Ver_bms, "bmu", Ver_bmu);
		cJSON_AddItemToObject(Ver_bms, "bsu", Ver_bsu);
		cJSON_AddItemToObject(Ver_bms, "imeu2", Ver_imeu2);
		cJSON_AddItemToObject(Ver_bms, "imeu3", Ver_imeu3);
		cJSON_AddItemToObject(Ver_pcs, "acdc_u1", Ver_acdc_u1);
		cJSON_AddItemToObject(Ver_pcs, "acdc_u2", Ver_acdc_u2);
		cJSON_AddItemToObject(Ver_pcs, "mppt_u1", Ver_mppt_u1);
		cJSON_AddItemToObject(Ver_pcs, "mppt_u2", Ver_mppt_u2);
		cJSON_AddItemToObject(Ver_temp, "air", Ver_air);
		cJSON_AddItemToObject(Ver_temp, "imeu3", Ver_ariEu3);

		cJSON_AddNumberToObject(Ver_bat_Group, "group_index", n);
		cJSON_AddItemToObject(Ver_bat_Group, "bms", Ver_bms);
		cJSON_AddItemToObject(Ver_bat_Group, "pcs", Ver_pcs);
		cJSON_AddItemToObject(Ver_bat_Group, "temp", Ver_temp);

		cJSON_AddItemToArray(Ver_bat_idx, Ver_bat_Group);
	}

	cJSON_AddItemToObject(VersionInfo, "ccu", Ver_cuu);
	cJSON_AddItemToObject(VersionInfo, "immu2", Ver_mu2);
	cJSON_AddItemToObject(VersionInfo, "imeu3", Ver_eu3);
	cJSON_AddItemToObject(VersionInfo, "abu_u1", Ver_abu_u1);
	cJSON_AddItemToObject(VersionInfo, "abu_u2", Ver_abu_u2);
	cJSON_AddItemToObject(VersionInfo, "batt_group_data", Ver_bat_idx);

	Send_CallResult(pIFY, VersionInfo, S_413_RmtGetLocalListVersion);

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
}ES_IFY_CONFIG_TYPE;

typedef struct tagIFYSettingsConfig
{
	char *ConfigName;
	BOOL  Readonly;
	ES_IFY_CONFIG_TYPE  ValueType;
	void  *ptrValue;

}GET_CHAANGE_CONFIG_PARA;

cJSON *GetConfig_Response(void* pArg,const char *data,BOOL bChangeCongfig, int nKeyIdx)
{
	TRACEX("Here is in Get Config \n");
	// Operation  0(Get Config ),1(Change settings)
	BOOL bAccpeted = FALSE;
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	cJSON *temp, *temp1;
	temp1 = pIFY->pCurrJson;
	
	GET_CHAANGE_CONFIG_PARA TestIFYSettigs[] =
	{
		{"Charger_ID" ,				FALSE,  ES_iTYPE_String64 ,		(void*)(pIFY->blobBackendSet.szCharger_ID) }, //0
		{"Server_URL" ,				FALSE,  ES_iTYPE_String64 ,		(void*)(pIFY->blobBackendSet.szServer_URL) }, //0
		{"Protect_SOC" ,			FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nProtect_SOC) }, //0
		{"SysACInput_PwrLmt" ,		FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fSysACInput_PwrLmt) }, //0
		// LJH240705 AitiOverLoad HVMeter
		{"SysACInput_PwrLmtHV" ,	FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fSysACInput_PwrLmtHV) },
		// LJH240911 AvgPwrFirst
		{"PwrCtrlMode" ,			FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.emPowCtrlMode) },
		{"PwrLmtMin_AvgPwrMode" ,	FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fPwrLmtMin_AvgPwrMode) },
		{"Max_Output2AC_PwrLmt" ,	FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fMax_Output2AC_PwrLmt) }, //0
		{"Min_Output2AC_PwrLmt" ,	FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fMin_Output2AC_PwrLmt) }, //0
		{"PreProtect_RefluxAndOverLoad" ,	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fPreProt4RefluxAndOverLoad) },//1
		{"Output2AC_PwrRate" ,		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fOutput2AC_PwrRate) },//1
		{"UrgetAlterDischrgCurr" ,	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fUrgetAlterDischrgCurr) },//1 
		{"AdjustUrgetModeCurr" ,		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fAdjustUrgetModeCurr) },//1 
		{"EnterUrgetDisChrgModePower",	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fEnter_UrgetDischrgPwr) },
		{"EixtUrgetDischrgModePower", 	FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fExit_UrgetDischrgPwr) },
		{"TotalPositiveMaxDemand", 		FALSE , ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fPfactory_SettDemand) },
		{"Cmd_101_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_101_Interval) },
		{"Cmd_102_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_102_Interval) },
		{"Cmd_103_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_103_Interval) },
		{"Cmd_104_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_104_Interval) },
		{"Cmd_105_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_105_Interval) },
		{"Cmd_106_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_106_Interval) },
		{"Cmd_107_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_107_Interval) },
		{"Cmd_108_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_108_Interval) },
		{"Cmd_109_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_109_Interval) },
		{"Cmd_202_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_202_Interval) },
		{"Cmd_204_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_204_Interval) },
		{"Cmd_206_Interval",		  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->blobBackendSet.nCmd_206_Interval) },
		{"air_brand",				  	FALSE,  ES_iTYPE_INT ,			(void*)&(pIFY->emAirType) },
		{"EnableQuickResponseChannel",	FALSE,  ES_iTYPE_BOOL ,			(void*)&(pIFY->blobBackendSet.bEnableQuickResponseChannel) },
		{"PowerRate_Chrg",				FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.PowerRate_Chrg) },
		{"PreProt_Chrg",				FALSE,  ES_iTYPE_FLOAT_DOT1 ,	(void*)&(pIFY->blobBackendSet.fPreProt_Chrg) },
		{"Standby_Enable",				FALSE,  ES_iTYPE_BOOL ,			(void*)&(pIFY->bStandbyEnable)},

	};
	GET_CHAANGE_CONFIG_PARA *pElementsList = &TestIFYSettigs[0];

	int n;
	BOOL bFoundKey = FALSE;
	for (n = 0; n < ITEM_OF(TestIFYSettigs) ; n++)
	{
		TRACEX(" n = %d   %s data %s \n", n, pElementsList[n].ConfigName,data);
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
			temp = pIFY->pCurrJson; //just for judge purpose
		}
	}

	return temp;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S411_GetConfiguration()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S411_GetConfiguration(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	cJSON *pJson = pIFY->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;
	int NoofConfig = -1;
	//IFY_SENT_MSG msg;
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
			//for (n = 0; n < pIFY->stIFYSettings.nGetConfigMaxKeys_R; n++, nKeyIdx++)
			for (n = 0; n < TotalConfig ; n++)
			{
				cJSON *temp;
				temp = GetConfig_Response(pIFY, s_strGetConfig[n], FALSE, n);
				cJSON_AddItemToArray(Arr, temp);
			}
			cJSON *Getconfig;
			Getconfig = cJSON_CreateObject();
			cJSON_AddItemToObject(Getconfig, "key_values", Arr);
			Send_CallResult(pIFY, Getconfig, S_411_RmtGetConfig);
		}
		else //if (NoofConfig <= pIFY->stIFYSettings.nGetConfigMaxKeys_R)// that MAXGetConfigKey Consider Here 
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
			for (n = 0; n < MIN(ITEM_OF(s_strGetConfig), NoofConfig); n++)
			{
				cJSON *configKeyVal = cJSON_GetArrayItem(temp1, n);
				TRACEX("Handle_GetConfiguration = %s \n", configKeyVal->valuestring);
				cJSON *temp;
				temp = GetConfig_Response(pIFY, configKeyVal->valuestring, 0, n);
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
				Send_CallResult(pIFY, Getconfig, S_411_RmtGetConfig);
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
		//for (n = 0; n < pIFY->stIFYSettings.nGetConfigMaxKeys_R; n++, nKeyIdx++)
		for (n = 0; n < TotalConfig ; n++)
		{
			cJSON *temp;
			temp = GetConfig_Response(pIFY, s_strGetConfig[n], FALSE, n);
			cJSON_AddItemToArray(Arr, temp);
		}
		cJSON *Getconfig;
		Getconfig = cJSON_CreateObject();
		cJSON_AddItemToObject(Getconfig, "key_values", Arr);
		Send_CallResult(pIFY, Getconfig, S_411_RmtGetConfig);
	}

	return 0;
	
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S412_ChangeConfiguration()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-30 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Handle_S412_ChangeConfiguration(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;

	TRACEX("Handle_ChangeConfiguration \n");
	cJSON *pJson = pIFY->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *Resp = NULL;
	int NoofConfig = -1 ;

	
	if (temp1 = cJSON_GetObjectItem(pJson, "key_values"))
	{
		int nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > ITEM_OF(s_strGetConfig)))
		{
			Send_CallError(pIFY, S_412_RmtSetConfig, CallError_FormatError, "key_value size NOT correct!");
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
			cJSON *Changeconfig = GetConfig_Response(pIFY, pItemName->valuestring, 1, n);
			// LJH240813 忽略不存在的参数，其他参数继续生效
			// if (Changeconfig == NULL)
			// {
			// 	bAccept = FALSE;
			// 	break;
			// }
		}
		
		//now update into IFY structure and DB
		if (bAccept)
		{
			VAR_VALUE varV;
			ERRCODE_DAI err = -1;
			pIFY->blobBackendSet.emBlobType = BLOB_TYPE_ES_BACKEND_CFG;
			varV.blobValue = &(pIFY->blobBackendSet);
			bAccept = FALSE;
			//varV.blobValue = IFY_Settings;
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
			//Revert back IFY settings
			PARAM_VAL_GET valGet;
			valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
			valGet.val.blobValue = (BYTE*)&pIFY->blobBackendSet;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 26, &valGet, sizeof(PARAM_VAL_GET));
		}
		Resp = cJSON_CreateObject();
		cJSON_AddNumberToObject(Resp, "accept", bAccept ? 1 : 2);
		Send_CallResult(pIFY, Resp, S_412_RmtSetConfig);
	}
	else
	{
		Send_CallError(pIFY, S_412_RmtSetConfig, CallError_FormatError, "key_values is missing!");
		return -1;
	}

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S503_GetEMSPlan()
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	int n = 0;
	PARAM_VAL_GET valGet;

	A_BLOB_EMS_CONFIG_PLAN emsPlan;
 	ZERO_POBJS(&emsPlan, 1);
	emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	valGet.val.blobValue = (BYTE*)&emsPlan;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));


	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "ems_plan_ver", 0);

	cJSON *pSector_Array = cJSON_CreateArray();
	for(n = 0; n < emsPlan.nTotal_EmsLmt_Sectors; n++)
	{
		cJSON *pSector_Obj = cJSON_CreateObject();

		char szTime[32];
		cJSON_AddStringToObject(pSector_Obj, "start_time", Make_ESIFYTimeString(emsPlan.emsLmtSectors[n].tmStart, szTime));

		cJSON_AddNumberToObject(pSector_Obj, "interval", 				emsPlan.emsLmtSectors[n].nInterval);
		cJSON_AddNumberToObject(pSector_Obj, "stop_soc", 				emsPlan.emsLmtSectors[n].nStopSOC);

 
		cJSON_AddFloatToObject(pSector_Obj,  "actv_pwr_lmt_rate", 		(float)(emsPlan.emsLmtSectors[n].nActvPwrLmt), 2);
		cJSON_AddFloatToObject(pSector_Obj,  "reactv_pwr_lmt_rate", 	(float)(emsPlan.emsLmtSectors[n].nReActvPwrLmt), 2);
		cJSON_AddFloatToObject(pSector_Obj,  "battgrp_pwr_lmt_rate", 	(float)(emsPlan.emsLmtSectors[n].nBattGrpPwrLmt), 2);

		cJSON_AddNumberToObject(pSector_Obj, "batt_workmode", 			emsPlan.emsLmtSectors[n].emBatt_WorkMode);
		cJSON_AddNumberToObject(pSector_Obj, "feerate_attri", 			emsPlan.emsLmtSectors[n].emFeeRate);
 		
		cJSON_AddItemToArray(pSector_Array, pSector_Obj);
	}

	cJSON_AddItemToObject(pRoot, "ems_plan_cfg", pSector_Array);
	cJSON_AddNumberToObject(pRoot, "ems_plan_repeat", 1);
	
	Send_CallResult(pIFY, pRoot, S_503_RmtGetEMSPlan);

 	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S504_SetEMSPlanProfile()
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
	
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;

	cJSON *pJson = pIFY->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	int n = 0, nProtectSOC = 0;
	int nTotalIntervalTime = 0;
	int nItemTotal;
	BOOL bAccepted = FALSE;
 	char szErrString[64];

	valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				500, 503, &valGet, sizeof(PARAM_VAL_GET));
 	nProtectSOC = valGet.val.ulValue;
	
	A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
	ZERO_POBJS(&blobEmsCfg, 1);
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	valGet.val.blobValue = (BYTE*)&blobEmsCfg;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
 
 	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_ver"))
	{
		TRACE("ems_plan_ver =%d temp1->valueint=%d\n", blobEmsCfg.nEMSMgmt_Ver, temp1->valueint);
		
		// Jeffery removed the password check for IfyCloud 20240810
		/*
		if(temp1->valueint == 135792068)
		{
			blobEmsCfg.nEMSMgmt_Ver = 0;//(UINT)temp1->valuedouble;
		}
		else
		{
			Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "This ver Not Support Change!");
			return -1;
		}
		*/

		if(temp1->valueint != -1)
		{
			blobEmsCfg.nEMSMgmt_Ver = 0;
		}
		else
		{
			Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "Please Change the Version Number!");
			return -1;
		}

	}
	else
	{
		Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_ver is Missing!");
		return -1;
	}
	
	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_cfg"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > MAX_EMSMGMT_LMT_SETOR))
		{
			TRACE("nItemTotal =%d\n", nItemTotal);
			Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "ems_plan_cfg Sector Number is INVALID!");
			return -1;
		}

		blobEmsCfg.nTotal_EmsLmt_Sectors = nItemTotal;

		TRACEX("Handle_S504_SetEMSPlanProfile nItemTotal= %d \n", nItemTotal);
		
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
					blobEmsCfg.emsLmtSectors[n].tmStart = Parse_ESIFYTime(pItemName->valuestring);
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
				if(cJSON_Number != pItemName->type)
				{
					sprintf(szErrString, "%s", "Error:interval Fromat Isn't Number!");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].nInterval =  pItemName->valueint;
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
				if(cJSON_Number != pItemName->type)
				{
					sprintf(szErrString, "%s", "Error:batt_workmode Fromat isn't Number!");	
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode =  pItemName->valueint;
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
				if((pItemName->valueint < 0) || (pItemName->valueint > 100) || (cJSON_Number != pItemName->type))
				{
					sprintf(szErrString, "%s", "Error:stop_soc Range Should From 0~100%%.");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].nStopSOC = pItemName->valueint;
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
				if((pItemName->valueint < FEERATE_SHARP) || (pItemName->valueint > FEERATE_VALLEY) || (cJSON_Number != pItemName->type))
				{
					sprintf(szErrString, "%s", "Error:feerate_attri Range Should From 0~3.");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].emFeeRate = (FEERATE_ATTRI)pItemName->valueint;
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
				if((pItemName->valuedouble < 0) || (pItemName->valuedouble > 999999.0) || (cJSON_Number != pItemName->type))
				{
					sprintf(szErrString, "%s", "actv_pwr_lmt_rate out of Range or Invalid!");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].nActvPwrLmt = (UINT)(pItemName->valuedouble);
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
				if((pItemName->valuedouble < -999999) || (pItemName->valuedouble > 999999.0) || (cJSON_Number != pItemName->type))
				{
					sprintf(szErrString, "%s", "reactv_pwr_lmt_rate out of Range or Invalid!");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt = (int)(pItemName->valuedouble);
				}
			}
			else
			{
				sprintf(szErrString, "%s", "reactv_pwr_lmt_rate is Missing!");
				bAccepted = FALSE;
				break;
			}

			if(pItemName = cJSON_GetObjectItem(pJsCurrItem, "battgrp_pwr_lmt_rate"))
 			{
				if((pItemName->valuedouble < 0) || (pItemName->valuedouble > 999999.0) || (cJSON_Number != pItemName->type))
				{
					sprintf(szErrString, "%s", "battgrp_pwr_lmt_rate out of Range or Invalid!");
					bAccepted = FALSE;
					break;
				}
				else
				{
					blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt = (UINT)(pItemName->valuedouble);
				}
			}
			else
			{
				//Due to Info Power Not Support  BattGrpPwrLmt Yet,So Used Reactive Power To Instead
				blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt = blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt;
				//sprintf(szErrString, "%s", "battgrp_pwr_lmt_rate is Missing!");
				//bAccepted = FALSE;
				//break;
			}

			if(blobEmsCfg.emsLmtSectors[n].nStopSOC < nProtectSOC)
			{
				SYSTEM_TYPE emSystemType = pIFY->lcCfg.emSystemType;
				if(emSystemType == SYSTYPE_ESS_AIR || emSystemType == SYSTYPE_ESS_LIQUIDED)
				{
					sprintf(szErrString, "%s%d", "nStop_SOC is Below nProtectSOC:", nProtectSOC);
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
		Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, szErrString);
		return -1;
	}

	if (temp1 = cJSON_GetObjectItem(pJson, "ems_plan_repeat"))
	{
		if(cJSON_Number != temp1->type)
		{
			Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "chgplan_repeat Type is Wrong!");
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
				Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "Daily Total interval is Wrong!");
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
			Send_CallError(pIFY, S_504_RmtSetEMSPlanProfile, CallError_FormatError, "Daily Total interval is Wrong!");
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
	Send_CallResult(pIFY, pRep, S_504_RmtSetEMSPlanProfile);
 
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S601_GetFeeRate()
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

	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err = -1;

	int n = 0;
 	A_BLOB_CHARGE_FEESET blobFeeRateSet;
	ZERO_POBJS(&blobFeeRateSet, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGE_FEESET);
	valGet.val.blobValue = (BYTE*)&blobFeeRateSet;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));

	cJSON *pRoot = cJSON_CreateObject();
	
	cJSON *pSector_Array = cJSON_CreateArray();
	for(n = 0; n < blobFeeRateSet.nTotal_FeeRate_Sectors; n++)
	{
		cJSON *pSector_Obj = cJSON_CreateObject();

		char szTime[32];
		cJSON_AddStringToObject(pSector_Obj, "start_time", 		Make_ESiOCPPTimeString(blobFeeRateSet.feeRateSectors[n].tmStart, szTime));
		cJSON_AddNumberToObject(pSector_Obj, "interval", 		blobFeeRateSet.feeRateSectors[n].nInterval);
		cJSON_AddNumberToObject(pSector_Obj, "feerate_attri", 	blobFeeRateSet.feeRateSectors[n].emFeeRate);
		cJSON_AddFloatToObject(pSector_Obj,  "fee_price", 		blobFeeRateSet.feeRateSectors[n].fPrice, 3);
 
		cJSON_AddItemToArray(pSector_Array, pSector_Obj);
	}

	cJSON_AddItemToObject(pRoot, "fee_rate_cfg", pSector_Array);
 	
	Send_CallResult(pIFY, pRoot, S_601_RmtGetFeeRateCfg);

 	return 0;

}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_S602_SetFeeRateProfile()
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
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;

	cJSON *pJson = pIFY->pCurrJson;
	cJSON *pResp = NULL;
	cJSON *temp1 = NULL;

	int n = 0, nTotalIntervalTime = 0, nItemTotal = 0;
	BOOL bAccepted = FALSE;
 	char szErrString[64];

 	A_BLOB_CHARGE_FEESET blobFeeRateSet;
	ZERO_POBJS(&blobFeeRateSet, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGE_FEESET);
	valGet.val.blobValue = (BYTE*)&blobFeeRateSet;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));

	if (temp1 = cJSON_GetObjectItem(pJson, "fee_rate_cfg"))
	{
		nItemTotal = cJSON_GetArraySize(temp1);
		if((nItemTotal < 1) || (nItemTotal > MAX_EMSMGMT_LMT_SETOR))
		{
			TRACE("nItemTotal =%d\n", nItemTotal);
			Send_CallError(pIFY, S_602_RmtSetFeeRateProfile, CallError_FormatError, "fee_rate_cfg Sector Number is INVALID!");
			return -1;
		}

		blobFeeRateSet.nTotal_FeeRate_Sectors = nItemTotal;

		TRACEX("Handle_S602_SetFeeRateProfile nItemTotal= %d \n", nItemTotal);
		
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
					blobFeeRateSet.feeRateSectors[n].tmStart = Parse_ESIFYTime(pItemName->valuestring);
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

				if((blobFeeRateSet.feeRateSectors[n].fPrice < 0) || (blobFeeRateSet.feeRateSectors[n].fPrice > 100.0))
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
		Send_CallError(pIFY, S_602_RmtSetFeeRateProfile, CallError_FormatError, szErrString);
		return -1;
	}

	for(n= 0; n < nItemTotal; n++)
	{
		nTotalIntervalTime += blobFeeRateSet.feeRateSectors[n].nInterval;
	}
	TRACE("Info: nItemTotal=%d nTotalIntervalTime=%d\n", nItemTotal, nTotalIntervalTime);

	if(nTotalIntervalTime != DAILY_INTERVAL_TIME)
	{
		Send_CallError(pIFY, S_602_RmtSetFeeRateProfile, CallError_FormatError, "FeeRate Total interval is Wrong!");
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
	Send_CallResult(pIFY, pRep, S_602_RmtSetFeeRateProfile);

	return 0;
}

int Handle_S408_ReadHistoryData(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
    cJSON *pJson = pIFY->pCurrJson;
	cJSON *temp1 = NULL;
	cJSON *pResp = NULL;

	time_t tBegin = 0;
	time_t tEnd = 0;

	if(temp1 = cJSON_GetObjectItem(pJson, "start_time"))
	{
		if (cJSON_String == temp1->type)
		{	
			tBegin = Parse_ESIFYTime(temp1->valuestring);
		}
	}

	if(temp1 = cJSON_GetObjectItem(pJson, "end_time"))
	{
		if (cJSON_String == temp1->type)
		{
			tEnd = Parse_ESIFYTime(temp1->valuestring);
		}
	}

	char* szErrMsg =  NULL;
	sqlite3_stmt* stat = NULL;
	int nRet = SQLITE_OK;

	sqlite3* data_db = Open_Lock_Database(DB_TYPE_CLOUDDATA);
	if(data_db == NULL)
	{
		printf("408 open data fail\n");
		return -1;
	}
	const char* szSql = "SELECT Idx, Time, DataInfo FROM Data_CloudBackupInfo \
		WHERE Time >= ? AND Time <= ?;";

	nRet = sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
	nRet = sqlite3_bind_int64(stat, 1, (sqlite3_int64)tBegin);
	nRet = sqlite3_bind_int64(stat, 2, (sqlite3_int64)tEnd);

	memset(&pIFY->stCloudUpload, 0, sizeof(CLOUD_UPLOAD)); //清空上一次上传数据
	
	pIFY->stCloudUpload.bUploadEnable = TRUE;
	pIFY->stCloudUpload.nSaveCurrtNum = 0;
	pIFY->stCloudUpload.tmLastUpload = _NOW_ - 10;

	int nDataCount = 0;
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		if(nDataCount >= MAX_CLOUDSAVE_NUM)
		{
			break;
		}
		sqlite3_int64 nTime = sqlite3_column_int64(stat, 1);
		pIFY->stCloudUpload.tmSaveNumber[nDataCount++] = (time_t)nTime;
	}
	pIFY->stCloudUpload.nSaveTotalNum = nDataCount;

	sqlite3_finalize(stat);
	Close_UnLock_Database(DB_TYPE_CLOUDDATA);

	pResp = cJSON_CreateObject();
	cJSON_AddNumberToObject(pResp, "accept", 1);
	
	Send_CallResult(pIFY, pResp, S_408_ReadHistoryData);

	return 0;
}

int ReadHistoryData(void* pArg)
{
	ES_IFY_DATA_STU* pIFY = (ES_IFY_DATA_STU*)pArg;
	REPORTER* pReporter = (REPORTER*)pIFY->hReporter;
    cJSON *pJson = pIFY->pCurrJson;
	cJSON *temp1;
	A_BLOB_TYPE_DATASTORAGE blobDataStorage;

	if(pIFY->stCloudUpload.nSaveCurrtNum >= pIFY->stCloudUpload.nSaveTotalNum)
	{
		pIFY->stCloudUpload.bUploadEnable = FALSE;
		return 0;
	}
	pIFY->stCloudUpload.tmLastUpload  = _NOW_;
	//打开数据查询在那天时间段的数据，准备回传给云平台
	char* szErrMsg =  NULL;
	sqlite3_stmt* stat = NULL;
	int nRet = SQLITE_OK;

	sqlite3* data_db = Open_Lock_Database(DB_TYPE_CLOUDDATA);
	if(data_db == NULL)
	{
		printf("ReadHistoryData open data fail\n");
		return -1;
	}
	const char* szSql = "SELECT Idx, Time, DataInfo FROM Data_CloudBackupInfo \
		WHERE Time = ?;";

	time_t tBegin = pIFY->stCloudUpload.tmSaveNumber[pIFY->stCloudUpload.nSaveCurrtNum];
	pIFY->stCloudUpload.nSaveCurrtNum++;

	nRet = sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
	nRet = sqlite3_bind_int64(stat, 1, (sqlite3_int64)tBegin);
	
	if(sqlite3_step(stat) == SQLITE_ROW)
	{
		const void* pData = sqlite3_column_blob(stat, 2);	
		int nDataSize = sqlite3_column_bytes(stat, 2);
		
		memset(&blobDataStorage, 0, sizeof(A_BLOB_TYPE_DATASTORAGE));
		if(pData != NULL && nDataSize > 0)
		{
			sqlite3_parse_blob_uncompressed(stat, 2, 
					(void*)&blobDataStorage, sizeof(A_BLOB_TYPE_DATASTORAGE));
		}
		cJSON *pRoot_202 = CreateJSON_C202(pReporter, &blobDataStorage, pIFY->nCfgBMSNum);
		// printf("202 = %s\n", cJSON_Print(pRoot_202));
		Send_Call(pIFY, pRoot_202, C_202_HourSectorStaticKwh,  CALL_SRC_Normal, 0, FALSE); 

		cJSON *pRoot_204 = CreateJSON_C204(pReporter, &blobDataStorage, pIFY->nCfgBMSNum);
		// printf("204 = %s\n", cJSON_Print(pRoot_204));
		Send_Call(pIFY, pRoot_204, C_204_DailyStaticKwh,  CALL_SRC_Normal, 0, FALSE); 

		cJSON *pRoot_206 = CreateJSON_C206(pReporter, &blobDataStorage);
		// printf("206 = %s\n", cJSON_Print(pRoot_206));
		Send_Call(pIFY, pRoot_206, C_206_FeeRateKwh,  CALL_SRC_Normal, 0, FALSE);
	}

	sqlite3_finalize(stat);
	Close_UnLock_Database(DB_TYPE_CLOUDDATA);

	return 0;
}