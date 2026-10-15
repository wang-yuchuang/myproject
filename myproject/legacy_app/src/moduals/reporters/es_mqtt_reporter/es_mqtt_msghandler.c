// #define _TRACE_PRINT_ // cmd
#include "es_mqtt.h"
 
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

 /*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C100_BootNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C100_BootNotification(void* pArg)
{
	TRACEX("Prepare_BootNotification \n");

    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;

	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	
	cJSON *pRoot;
	//----------- Get Info for BootNotification -----------///
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

	pRoot = cJSON_CreateObject();

	//cJSON_AddStringToObject(pRoot, "chargePointModel", bootInfo.szChargePointModel);
	cJSON_AddStringToObject(pRoot, "vendor", bootInfo.szChargePointVendor);
	//if (bootInfo.emBootInfoOptions.chargerPointSN
	//if(bootInfo.emBootInfoOptions.chargerPointSN || (strlen(bootInfo.szChargePointSerualNuml) > 0))
	{
		cJSON_AddStringToObject(pRoot, "serial_no", bootInfo.szChargePointSerualNuml);
	}

	//cJSON_AddStringToObject(pRoot, "hw_ver", "A02"); //in future will automatically identified
	//
	//if(bootInfo.emBootInfoOptions.firmwareVer)
	//{
	//	cJSON_AddStringToObject(pRoot, "sw_ver_main", bootInfo.szFirmwareVer);
	//}

	int nPkgVer = 0;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PKG_VRESION_R,
		0, 0, &nPkgVer, 4);
	
	cJSON_AddNumberToObject(pRoot, "sw_pkg_ver", nPkgVer);
 
	
	Send_Call(pMqtt, pRoot, C_100_BootNotif, CALL_SRC_Normal, 0, TRUE);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C101_StatusNotification()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C101_StatusNotification(void* pArg)
{
	TRACEX("Prepare_StatusNotification \n");

	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject();
	
	PARAM_VAL_GET valGet;
	int n = 0;
	BOOL bSendUrgent = (pMqtt->emTriggerMsgType == Trigger_C101) ? TRUE : FALSE;

	//if(pMqtt->emTriggerMsgType == Trigger_C101)
	//{
	//	Check_IfStatusChanged(pMqtt);
	//}

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
	pMqtt->emSysStatus_For101 = valGet.val.emValue;

	cJSON_AddNumberToObject(pRoot, "cp_status", pMqtt->emSysStatus_For101);

	cJSON_AddNumberToObject(pRoot, "grid_status", 0); //0-Merge Grid, 1-OFF Grid

	//if(pMqtt->emSysStatus_For101 > 0)
	{
		//cJSON_AddNumberToObject(pJson_Status, "cp_fault", GetConnectorErrCode_ByAlarm(pReporter, Connctor_0));
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 6, &valGet, sizeof(PARAM_VAL_GET));

		//cJSON_AddNumberToObject(pRoot, "cp_fault", valGet.val.emValue);
		cJSON_AddNumberToObject(pRoot, "cp_fault", 0);
	}

	//Now get each Slave Cabinet status
	pMqtt->nConnectSlvNum = 0; //??? Should Get Slave Info By Get 
 	cJSON_AddNumberToObject(pRoot, "chrg_slvcab_num", pMqtt->nConnectSlvNum);
	
	cJSON 	*pSlv_Array = cJSON_CreateArray();
	if(pMqtt->nConnectSlvNum > 0)
	{
		for(n = 0; n < pMqtt->nConnectSlvNum; n++) //
		{
			cJSON *pJson_Slv = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJson_Slv, "data_from", 		n+1);
			cJSON_AddNumberToObject(pJson_Slv, "comm_status", 		0);
			cJSON_AddNumberToObject(pJson_Slv, "cp_status", 		0);
			cJSON_AddNumberToObject(pJson_Slv, "cp_fault", 			0);	
	 		cJSON_AddItemToArray(pSlv_Array, pJson_Slv);
		}
	}
	cJSON_AddItemToObject(pRoot, "slv_sys_stat", pSlv_Array);

	//Msg_Send(pMqtt, pJson_Status, StatusNotification, Call, pMqtt->nCurrChargeStatusHandled_Conn, bSend_Later);
	Send_Call(pMqtt, pRoot, C_101_StatusNotif, CALL_SRC_Normal, 0, bSendUrgent);

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C102_BattCabStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C102_BattCabStaNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	//VAR_VALUE 		varV;

	int n = 0, m = 0;
	//int nIMEU2CommStatu = 0;
	int nDevID = 0, nSigID = 0;

	A_BLOB_BATTCAB_IMEU2_V20	blobIMEU2_V20[MAX_BATTCABINET_NUM];
	
	cJSON 	*pRoot = cJSON_CreateObject();

	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	
	cJSON_AddNumberToObject(pRoot, "batt_cab_num", pMqtt->nCfgBMSNum);

	if(pMqtt->nCfgBMSNum > MAX_BATTCABINET_NUM)
	{	
		Send_CallError(pMqtt, C_102_BattCabStaNotif, CallError_FormatError, "BattCabinet Cfg is Invalid!");
		return -1;
	}

	cJSON 	*pBattCab_Array = cJSON_CreateArray();
	for(n = 0; n < pMqtt->nCfgBMSNum; n++)
	{
		//Need Judge IMEU2 Installed Status !!! Process Later
		valGet.nGetSize = 4;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				601 + n, 450, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}

		if(pMqtt->nCurrDataFrom == 0)
		{
			nDevID = 601 + n;
			nSigID = 450;
		}
		else
		{
			nDevID = 2;
			nSigID = 1080 + n;
		}

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

		//err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
		//		601 + n, 456, &varV, 4);
		//if((varV.emValue == 1) || (err != ERRCODE_DAI_OK))
		//{
		//	nIMEU2CommStatu = 1;
		//}
		//else
		//{
		//	nIMEU2CommStatu = 0;
		//}
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

		cJSON_AddItemToArray(pBattCab_Array, pJS_SiglCab);
	}
	cJSON_AddItemToObject(pRoot, "batt_cab_data", pBattCab_Array);
	Send_Call(pMqtt, pRoot, C_102_BattCabStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

int Prepare_SEMS_C103_ESBMSStatusNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	
	A_BLOB_SEMS_CMD103 dataBloCmd103;
	dataBloCmd103.emBlobType = BLOB_TYPE_SEMS_CMD103;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD103);
	valGet.val.blobValue = (BYTE*)&dataBloCmd103;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3403, &valGet, sizeof(PARAM_VAL_GET));
	
	int i = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum; ++i)
	{
		CTREAT_SEMS_JSON_INFO createInfo = {0};
		createInfo.iClientIdx = i;
		createInfo.nCurrDataFrom = pMqtt->nCurrDataFrom;

		cJSON *pRoot = CreateJSON_SEMS_C103(&dataBloCmd103, &createInfo);
		if (NULL != pRoot)
		{
			Send_Call(pMqtt, pRoot, C_103_ESBMSStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
			Publish_MqttMessage(pMqtt);
		}
	}
	
	return 0;
}

//法国客户要求：移除字段dev_id，把下级设备消息汇总 用batt_idx来区分下级设备，用一条发出
int Prepare_SEMS_C103_ESBMSStatusNotif_Electra(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	
	A_BLOB_SEMS_CMD103 dataBloCmd103;
	dataBloCmd103.emBlobType = BLOB_TYPE_SEMS_CMD103;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD103);
	valGet.val.blobValue = (BYTE*)&dataBloCmd103;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3403, &valGet, sizeof(PARAM_VAL_GET));
	
	cJSON 	*pBMS_Array = cJSON_CreateArray();
	int i = 0;
	int iBattGroupNum = 0;
	int iBattIdx = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum; ++i)
	{
		if (dataBloCmd103.clientBMS[i].iBattGroupNum > MAX_BATTERY_NUM)
		{
			continue;
		}
		iBattGroupNum += dataBloCmd103.clientBMS[i].iBattGroupNum;

		int j = 0;
		for (j = 0; j < dataBloCmd103.clientBMS[i].iBattGroupNum; ++j)
		{
			cJSON *pBMS_Single = cJSON_CreateObject();
			cJSON_AddNumberToObject(pBMS_Single, "batt_idx", iBattIdx++);

			SC6_LDEV_103_Data* pBmsData = &dataBloCmd103.clientBMS[i].bmsData[j];

			cJSON_AddNumberToObject(pBMS_Single, "run_statu", pBmsData->run_statu);

			cJSON_AddFloatToObject(pBMS_Single, "pack_volt", pBmsData->pack_volt, 1);
			cJSON_AddFloatToObject(pBMS_Single, "pack_curr", pBmsData->pack_curr, 1);
			
			cJSON_AddNumberToObject(pBMS_Single, "soh", pBmsData->soh);
			cJSON_AddNumberToObject(pBMS_Single, "soc", pBmsData->soc);

			cJSON_AddNumberToObject(pBMS_Single, "cycle", pBmsData->cycle);

			cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_volt", pBmsData->max_allow_chg_volt, 1);
			cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_curr", pBmsData->max_allow_chg_curr, 1);
			cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_volt", pBmsData->max_allow_dischg_volt, 1);
			cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_curr", pBmsData->max_allow_dischg_curr, 1);

			cJSON_AddFloatToObject(pBMS_Single, "cell_max_volt", pBmsData->cell_max_volt, 3);
			cJSON_AddFloatToObject(pBMS_Single, "cell_min_volt", pBmsData->cell_min_volt, 3);
			cJSON_AddFloatToObject(pBMS_Single, "cell_avg_volt", pBmsData->cell_avg_volt, 3);
			
			cJSON_AddFloatToObject(pBMS_Single, "cell_max_temp", pBmsData->cell_max_temp, 3);
			cJSON_AddFloatToObject(pBMS_Single, "cell_min_temp", pBmsData->cell_min_temp, 3);
			cJSON_AddFloatToObject(pBMS_Single, "cell_avg_temp", pBmsData->cell_avg_temp, 1);

			cJSON_AddFloatToObject(pBMS_Single, "ucell_max_csc",  (float)pBmsData->ucell_max_csc, 1);
			cJSON_AddFloatToObject(pBMS_Single, "ucell_max_cell", (float)pBmsData->ucell_max_cell, 1);
			cJSON_AddFloatToObject(pBMS_Single, "ucell_min_csc",  (float)pBmsData->ucell_min_csc, 1);
			cJSON_AddFloatToObject(pBMS_Single, "ucell_min_cell", (float)pBmsData->ucell_min_cell, 1);
			
			cJSON_AddFloatToObject(pBMS_Single, "temp_max_csc",  (float)pBmsData->temp_max_csc, 1);
			cJSON_AddFloatToObject(pBMS_Single, "temp_min_csc", (float)pBmsData->temp_min_csc, 1);

			cJSON_AddFloatToObject(pBMS_Single, "remain_charge_energy", (float)pBmsData->remain_charge_energy, 2);
			cJSON_AddFloatToObject(pBMS_Single, "remain_discharge_energy", (float)pBmsData->remain_discharge_energy, 2);

			cJSON_AddNumberToObject(pBMS_Single, "chrg_forbid", pBmsData->chrg_forbid);
			cJSON_AddNumberToObject(pBMS_Single, "dischrg_forbid", pBmsData->dischrg_forbid);
			
			cJSON_AddNumberToObject(pBMS_Single, "alarm_status", pBmsData->alarm_status); 
			cJSON_AddStringToObject(pBMS_Single, "alarm_info", pBmsData->alarm_info);

			cJSON_AddStringToObject(pBMS_Single, "alarm2_info", pBmsData->alarm2_info);
			cJSON_AddStringToObject(pBMS_Single, "fault_info", pBmsData->fault_info); 
			cJSON_AddStringToObject(pBMS_Single, "gpio_info", pBmsData->gpio_info); 

			cJSON_AddFloatToObject(pBMS_Single, "batt_ver", pBmsData->batt_ver, 3);

			cJSON_AddItemToArray(pBMS_Array, pBMS_Single);
		}
	}

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "online_batts", iBattGroupNum);
	cJSON_AddItemToObject(pRoot, "batt_grp_data", pBMS_Array);

	Send_Call(pMqtt, pRoot, C_103_ESBMSStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	Publish_MqttMessage(pMqtt);
	
	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C103_ESBMSStatusNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C103_ESBMSStatusNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	//int nOnLineSTBatt = 0;
	int nDevID = 0, nSigID = 0;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;
	BOOL bIsOCVFlag;
	int n = 0;

	if(pMqtt->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		Send_CallError(pMqtt, C_103_ESBMSStaNotif, CallError_FormatError, "BMS Number Cfg is Invalid!");
		return -1;
	}

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "online_batts", pMqtt->nCfgBMSNum);

	//Single STBattery Object Infomation
	cJSON 	*pBMS_Array = cJSON_CreateArray();

 	A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
	for(n = 0; n < pMqtt->nCfgBMSNum; n++)
	{
		if(pMqtt->nCurrDataFrom == 0)
		{
			nDevID = 501 + n;
			nSigID = 551;
		}
		else
		{
			nDevID = 2;
			nSigID = 1050 + n;
		}
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
		
		valGet.nGetSize = 4;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					nDevID, 3911, &valGet, sizeof(PARAM_VAL_GET));
		bIsOCVFlag = valGet.val.emValue;

		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}
		//cJSON *pBattInfo_Single = cJSON_CreateObject();
 
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
		cJSON_AddNumberToObject(pBMS_Single, "ocv_flag", bIsOCVFlag);
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

	Send_Call(pMqtt, pRoot, C_103_ESBMSStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}


//法国客户要求：移除字段dev_id，把下级设备消息汇总 用batt_idx来区分下级设备，用一条发出
int Prepare_SEMS_C104_PwrDistrStaNotif_Electra(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	
	A_BLOB_SEMS_CMD104 dataBloCmd104;
	dataBloCmd104.emBlobType = BLOB_TYPE_SEMS_CMD104;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD104);
	valGet.val.blobValue = (BYTE*)&dataBloCmd104;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3404, &valGet, sizeof(PARAM_VAL_GET));
	
	cJSON 	*pPowerCalc_Array = cJSON_CreateArray();
	int i = 0;
	int iBattGroupNum = 0;
	int iBattIdx = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum; ++i)
	{
		if (dataBloCmd104.clientPowerCalc[i].iBattGroupNum > MAX_BATTERY_NUM)
		{
			continue;
		}
		iBattGroupNum += dataBloCmd104.clientPowerCalc[i].iBattGroupNum;

		int j = 0;
		for (j = 0; j < dataBloCmd104.clientPowerCalc[i].iBattGroupNum; ++j)
		{
			SC6_LDEV_104_Data* pPowerCalcData = &dataBloCmd104.clientPowerCalc[i].powerCalcData[j];

			cJSON *pPowerCalc_Single = cJSON_CreateObject();

			cJSON_AddNumberToObject(pPowerCalc_Single, "branch_idx", iBattIdx++);
			cJSON_AddFloatToObject(pPowerCalc_Single, "batt_max_dischrg_pwr", pPowerCalcData->batt_max_dischrg_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "batt_max_chrg_pwr_req", pPowerCalcData->batt_max_chrg_pwr_req, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "batt_distri4acdc_pwr", pPowerCalcData->batt_distri4acdc_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "batt_distri4dcdc_pwr", pPowerCalcData->batt_distri4dcdc_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "acdc_rate_pwr", pPowerCalcData->acdc_rate_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "dcdc_rate_pwr", pPowerCalcData->dcdc_rate_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "remain_batt_dischrg_pwr", pPowerCalcData->remain_batt_dischrg_pwr, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "remain_batt_chrg_pwr_req", pPowerCalcData->remain_batt_chrg_pwr_req, 1);
			cJSON_AddFloatToObject(pPowerCalc_Single, "acdc_distri4batt_chrg_pwr", pPowerCalcData->acdc_distri4batt_chrg_pwr, 1);

			cJSON_AddItemToArray(pPowerCalc_Array, pPowerCalc_Single);
		}
	}

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "online_branch_num", iBattGroupNum);
	cJSON_AddItemToObject(pRoot, "branch_node_calc_data", pPowerCalc_Array);

	Send_Call(pMqtt, pRoot, C_104_PwrDistrStaNotif, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	Publish_MqttMessage(pMqtt);
	
	return 0;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C104_PwrDistrStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C104_PwrDistrStaNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;

	int n = 0;
	int nDevID = 0, nSigID = 0;
	if(pMqtt->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		Send_CallError(pMqtt, C_104_PwrDistrStaNotif, CallError_FormatError, "Branch Number Cfg is Invalid!");
		return -1;
	}
	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "online_branch_num", pMqtt->nCfgBMSNum);
	
	if(pMqtt->nCurrDataFrom == 0)
	{
		nDevID = 221;
		nSigID = 203;
	}
	else
	{
		nDevID = 223;
		nSigID = 203;
	}

	A_BLOB_BRANCH_NODE_CALC_DATA nodeCalcData;
	ZERO_POBJS(&nodeCalcData, 1);
	nodeCalcData.emBlobType = BLOB_TYPE_BRANCH_NODE_CALC_DATA;
	valGet.nGetSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
	valGet.val.blobValue = (BYTE*)&nodeCalcData;
	cJSON 	*pBranch_Array = cJSON_CreateArray();
	for(n = 0; n < pMqtt->nCfgBMSNum; n++)
	{
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			nDevID + n, nSigID, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			continue;
		}

		cJSON *pBranch_Single = cJSON_CreateObject();
		
		cJSON_AddNumberToObject(pBranch_Single, "dev_id", n);
		cJSON_AddFloatToObject(pBranch_Single, "batt_max_dischrg_pwr", 		nodeCalcData.fBatt_Max_DisChrg_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_max_chrg_pwr_req", 	nodeCalcData.fBatt_Max_Chrg_Pwr_Req, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_distri4acdc_pwr", 		nodeCalcData.fBatt_Distri_4_ACDC_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "batt_distri4dcdc_pwr", 		nodeCalcData.fBatt_Distri_4_DCDC_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "acdc_rate_pwr", 			nodeCalcData.fACDC_Max_Ability_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "dcdc_rate_pwr", 			nodeCalcData.fDCDC_Max_Ability_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "remain_batt_dischrg_pwr", 	nodeCalcData.fRemain_Batt_DisChrg_Pwr, 1);
		cJSON_AddFloatToObject(pBranch_Single, "remain_batt_chrg_pwr_req", 	nodeCalcData.fRemain_Batt_Chrg_Pwr_Req, 1);
		cJSON_AddFloatToObject(pBranch_Single, "acdc_distri4batt_chrg_pwr", nodeCalcData.fACDC_Distri_4_BattChrg_Pwr, 1);
 
		cJSON_AddItemToArray(pBranch_Array, pBranch_Single);
	}

	cJSON_AddItemToObject(pRoot, "branch_node_calc_data", pBranch_Array);
	
	Send_Call(pMqtt, pRoot, C_104_PwrDistrStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C105_ESRunDataNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C105_ESRunDataNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;
	int n= 0, nDevID = 0, nSigID = 0;
	if(pMqtt->nCurrDataFrom == 0)
	{
		nDevID = 200;
		nSigID = 80;
	}
	else
	{
		nDevID = 2;
		nSigID = 1060;
	}

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
	
	if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
	{// LJH250428 cmd155
		TRACED("Prepare_C105_ESRunDataNotif 155 bSendCmdByLC1stOnly");
		
		// Jeffery 250731 155设置优先
		int nCmd_155_Interval = (pMqtt->blobBackendSet.nCmd_155_Interval != 0) ? pMqtt->blobBackendSet.nCmd_155_Interval : pMqtt->blobBackendSet.nCmd_105_Interval;
		cJSON* pRoot = CreateJSON_C155(pReporter, &pMqtt->lcCfg, nCmd_155_Interval, &esRunData);
		
		if (pRoot != NULL)
		{
			Send_Call(pMqtt, pRoot, C_155_ESRunDataStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		}
	}
	else
	{
		// 各自上报
		cJSON 	*pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
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
		for(n = 0; n < pMqtt->nCfgBMSNum; n++)
		{
			cJSON 	*pSiglRectGrp = cJSON_CreateObject();

			cJSON_AddNumberToObject(pSiglRectGrp, "branch_grp_idx", n);

			cJSON_AddNumberToObject(pSiglRectGrp, "acdc_exist_status",  esRunData.isACDC_Grp_Exist[n]);
			cJSON_AddNumberToObject(pSiglRectGrp, "batt_exist_status",  (esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)?0:1); 
			cJSON_AddNumberToObject(pSiglRectGrp, "dcdc_exist_status",  esRunData.isDCDC_Grp_Exist[n]);
			cJSON_AddNumberToObject(pSiglRectGrp, "mppt_exist_status",  esRunData.isMPPT_Grp_Exist[n]);

			ACDC_WORKSTATU emStatus = (esRunData.emACDC_WorkStatu[n] == STATU_ACDC_STANDBY) ? STATU_ACDC_IDLE : esRunData.emACDC_WorkStatu[n]; 
			cJSON_AddNumberToObject(pSiglRectGrp, "acdc_workstat",		emStatus);
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
		
		Send_Call(pMqtt, pRoot, C_105_ESRunDataStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}

	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C106_SingleRectStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C106_SingleRectStaNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	
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
				//TRACE("nRectType=[%d] n =[%d] emWorkMode=[%d]\n", nRectType, n, sigRect.arrRect[nRectType][n].emWorkMode);
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

 	Send_Call(pMqtt, pRoot, C_106_SingleRectStaNotif,  CALL_SRC_Normal, 0, FALSE); 
	
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C107_PwrDistrCabStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
#define MAX_WHMETER_NUM		9
static UINT s_ACMterDevMap[MAX_WHMETER_NUM]={101,102,103,104,105,106,0,99,100};

#define MAX_TEMPHUMIL_NUM	1
static UINT s_TempHumilDevMap[MAX_TEMPHUMIL_NUM]={0};


int Prepare_C107_PwrDistrCabStaNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	if (pMqtt->lcCfg.bSendCmdByLC1stOnly)
	{// LJH250520 cmd157
		TRACED("Prepare_C107_PwrDistrCabStaNotif 157 bSendCmdByLC1stOnly");

		// Jeffery 250731 157设置优先
		int nCmd_157_Interval = (pMqtt->blobBackendSet.nCmd_157_Interval != 0) ? pMqtt->blobBackendSet.nCmd_157_Interval : pMqtt->blobBackendSet.nCmd_107_Interval;
		cJSON* pRoot = CreateJSON_C157(pReporter, &pMqtt->lcCfg, nCmd_157_Interval);

		if (pRoot != NULL)
		{
			Send_Call(pMqtt, pRoot, C_157_PwrDistrCabStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		}
	}
	else
	{
		// 各自上报
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
		
		cJSON *pSlv1_ACmeter_Obj = cJSON_CreateObject();

		if(pMqtt->nSlvCommStatu[0] == 2)//2-Comm Normal 
		{
			TRACE("nSlv1CommStatu[%d] OK!\n", pMqtt->nSlvCommStatu[0]);
			//---------------Slave 1 ACMeter Info-------------------//
			A_BLOB_ES_RUN_DATA	blobESFlow;
			ZERO_POBJS(&blobESFlow ,	1);
			blobESFlow.emBlobType = BLOB_TYPE_ES_RUNDATA;
			valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
			valGet.val.blobValue = (BYTE*)&blobESFlow;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				2, 1060, &valGet, sizeof(PARAM_VAL_GET));
			if(err != ERRCODE_DAI_OK)
			{
				TRACE("Error: DAI Get A_BLOB_ES_RUN_DATA Failed err=[%d]!\n", err);
				return 0;
			}

			float fSlv1_Active_power = 0, fSlv1_KwhReading = 0, fSlv1_NegKwhReading = 0;
			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
						1, 32, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				fSlv1_Active_power = valGet.val.fValue;
			}

			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
						1, 33, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				fSlv1_KwhReading = valGet.val.fValue;
			}

			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
						1, 34, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				fSlv1_NegKwhReading = valGet.val.fValue;
			}

			cJSON_AddNumberToObject(pSlv1_ACmeter_Obj, "data_from", 		1); //0-Slv1 ACMeter Data

			cJSON* pSlv1_Volt_Array = cJSON_CreateArray();
			cJSON_AddItemToArray(pSlv1_Volt_Array, cJSON_CreateFloat(blobESFlow.fACVolt_L1, 1));
			cJSON_AddItemToArray(pSlv1_Volt_Array, cJSON_CreateFloat(blobESFlow.fACVolt_L2, 1));
			cJSON_AddItemToArray(pSlv1_Volt_Array, cJSON_CreateFloat(blobESFlow.fACVolt_L3, 1));
			cJSON_AddItemToObject(pSlv1_ACmeter_Obj, "phase_volts", pSlv1_Volt_Array);

			cJSON* pSlv1_Curr_Array = cJSON_CreateArray();
			cJSON_AddItemToArray(pSlv1_Curr_Array, cJSON_CreateFloat(blobESFlow.fACCurr_L1, 1));
			cJSON_AddItemToArray(pSlv1_Curr_Array, cJSON_CreateFloat(blobESFlow.fACCurr_L2, 1));
			cJSON_AddItemToArray(pSlv1_Curr_Array, cJSON_CreateFloat(blobESFlow.fACCurr_L3, 1));
			cJSON_AddItemToObject(pSlv1_ACmeter_Obj, "phase_currs", pSlv1_Curr_Array);
			

			cJSON_AddFloatToObject(pSlv1_ACmeter_Obj, "kwh_reading", 			fSlv1_KwhReading,  1);
			cJSON_AddFloatToObject(pSlv1_ACmeter_Obj, "neg_kwh_reading", 		fSlv1_NegKwhReading,  1);
			cJSON_AddFloatToObject(pSlv1_ACmeter_Obj, "active_power", 			fSlv1_Active_power,  1);
			cJSON_AddFloatToObject(pSlv1_ACmeter_Obj, "power_factor", 				1.0,  2);
			cJSON_AddFloatToObject(pSlv1_ACmeter_Obj, "ac_freq", 					50.0,  3);
			cJSON_AddItemToArray(pAllACMeter_Array, pSlv1_ACmeter_Obj);
		}
		
		cJSON_AddItemToObject(pRoot, "ac_metervalue", pAllACMeter_Array);

		//add by lpl 24-05-15 for UartServer-TempControlDev Data
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			901, 910, &valGet, sizeof(PARAM_VAL_GET));
		TRACEW("----MQTT----UartServer Install Type = %d ",valGet.val.emValue);
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
				// TRACEE("============================================");

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
 		Send_Call(pMqtt, pRoot, C_107_PwrDistrCabStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}
	return 0;
} 

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C108_SysVersionStaNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C108_SysVersionStaNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
		
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

	//if(pMqtt->nCurrDataFrom == 0)
	{
		cJSON_AddNumberToObject(pMainVer_Obj, "data_from", 		pMqtt->nCurrDataFrom);
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
		cJSON_AddItemToArray(pVer_Array, pMainVer_Obj);
	}
	//else
	{
		//Process Later, ???
		//cJSON_AddNumberToObject(pSlvVer_Obj, "data_from", 		pMqtt->nCurrDataFrom);
		//cJSON_AddStringToObject(pSlvVer_Obj, "sw_main_ver",		"V1.05.319T1");
		//cJSON_AddStringToObject(pSlvVer_Obj, "hw_main_ver",		"A03");
		//cJSON_AddStringToObject(pSlvVer_Obj, "sw_lccu_ver",		"1.03");
		//cJSON_AddStringToObject(pSlvVer_Obj, "hw_lccu_ver",		"A6");
		//cJSON_AddItemToArray(pVer_Array, pSlvVer_Obj);
	}


	cJSON_AddItemToObject(pRoot, "sw_dev_ver", pVer_Array);
	
	Send_Call(pMqtt, pRoot, C_108_SysVersionStaNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  GetCfgAlarm_ById()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static ALARM_INFO* GetCfgAlarm_ById(MQTT_DATA_ROUGH* pMqtt, int nAlarmId)
{
	ALARM_INFO* pAlarm = NULL;
 	int n = 0;
 	ALARM_INFO *pCfgAlarm = &pMqtt->pAllAlarmInfo[0];
	if(pCfgAlarm == NULL)
	{
		return pAlarm;
	}
	for(n = 0; n < pMqtt->nAlarmInfosTotal; n++, pCfgAlarm++)
	{
		if((pCfgAlarm->nAlarmId == nAlarmId)) //&&
			//(pCfgAlarm->emLevel == ALARM_LEVEL_CRITICAL))
		{
			pAlarm = pCfgAlarm;
			break;
		}
	}
	return pAlarm;
}

BOOL CheckIfNeedShieldAlarms(int n,MQTT_DATA_ROUGH* pMqtt)
{
	ALARM_INFO *pCfgAlmInfo = NULL;
	int index = 0;

	pCfgAlmInfo = GetCfgAlarm_ById(pMqtt,	pMqtt->activeAlms[n].nAlarmId);
	if(pCfgAlmInfo == NULL)
	{
		TRACE("MQTT-Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", \
			pMqtt->activeAlms[n].nDevId,pMqtt->activeAlms[n].nAlarmId);
		return TRUE;
	}
	if (pCfgAlmInfo->emLevel == ALARM_LEVEL_OBSERVATION)  //OA level do not reporter
	{
		TRACEW("pMqtt->activeAlms[n].nAlarmId[%d] == ALARM_LEVEL_OBSERVATION",pMqtt->activeAlms[n].nAlarmId);
		return TRUE;
	}
	for (index = 0;index < pMqtt->nShieldAlarmsNum;index++)
	{
		if (pMqtt->activeAlms[n].nAlarmId == pMqtt->ShieldAlarms[index])
		{
			TRACEW("pMqtt->activeAlms[n].nAlarmId[%d] == pMqtt->ShieldAlarms_id[%d]",pMqtt->activeAlms[n].nAlarmId,pMqtt->ShieldAlarms[index]);
			return TRUE;
		}
	}
	return FALSE;
}




cJSON* create_SEMS_C109(MQTT_DATA_ROUGH *pMqtt, A_BLOB_SEMS_CMD109* pDataBloCmd109, int iClientIdx)
{
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	char szDevName[128];
	
	int iAlarmCount = pDataBloCmd109->clientActiveAlarms[iClientIdx].iAlarmCount;
	// int iAlarmCount = pMqtt->nActiveAlmTotal;
		
	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pActvAlm_Array = cJSON_CreateArray();
		
	// cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	// cJSON_AddNumberToObject(pRoot, "dev_id", pDataBloCmd109->clientActiveAlarms[iClientIdx].iDevID);
	cJSON_AddNumberToObject(pRoot, "dev_id", iClientIdx);
	cJSON_AddNumberToObject(pRoot, "total_actv_alm", iAlarmCount);

	int n = 0;
	for (n = 0; n < iAlarmCount && n < MAX_QUERY_ACTV_ALM; ++n)
	{
		SC6_LDEV_109* pAlarm = 	&pDataBloCmd109->clientActiveAlarms[iClientIdx].activeAlms[n];

		ALARM_INFO *pCfgAlmInfo = GetCfgAlarm_ById(pMqtt, pAlarm->nAlarmId);
		if(pCfgAlmInfo == NULL)
		{
			TRACE("Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", pMqtt->activeAlms[n].nDevId,pMqtt->activeAlms[n].nAlarmId);
			continue;
		}

		cJSON *pActvAlm_Obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alarm_id", 			pAlarm->nAlarmId);
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_level", 			pCfgAlmInfo->emLevel);

		memset(szDevName, 0, sizeof(szDevName));
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETDEVNAME_BYID_R, pAlarm->nDevId, 0, szDevName, 32);
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_dev",				szDevName);//pCfgAlmInfo->pBelonged_DevModel->szDevModelName);
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_name",				pCfgAlmInfo->pRes_Name->szName);
			
		char szTime[32];
		cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_begin_time", 		Make_MqttTimeString(pMqtt, pAlarm->tmStart, szTime));

		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_status", 			(pAlarm->status == ALARM_STATUS_ACTIVE) ? 0 : 1);
		cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_dev_id", 			pAlarm->nDevId);

		cJSON_AddItemToArray(pActvAlm_Array, pActvAlm_Obj);
	}

	cJSON_AddItemToObject(pRoot, "actv_alm_info", pActvAlm_Array);

	return pRoot;
	
}

int send_SEMS_AlarmStatusChanged(void* pArg)
{
	MQTT_DATA_ROUGH* pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	static A_BLOB_SEMS_CMD109 dataBloCmd109_Old = {0};
 	
	A_BLOB_SEMS_CMD109 dataBloCmd109;
	dataBloCmd109.emBlobType = BLOB_TYPE_SEMS_CMD109;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD109);
	valGet.val.blobValue = (BYTE*)&dataBloCmd109;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3409, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}
	cJSON *pRoot = cJSON_CreateObject();
	cJSON *SEMS_Alarm_Obj = cJSON_CreateObject();
	cJSON *SEMS_pActvAlm_Array = cJSON_CreateArray();
	BOOL bNeedSend = FALSE; 

	int i = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum && i < MAX_NUM_ESC; ++i)
	{
		int iLen = sizeof(dataBloCmd109.clientActiveAlarms[i].activeAlms);

		if (0 != memcmp(dataBloCmd109_Old.clientActiveAlarms[i].activeAlms, dataBloCmd109.clientActiveAlarms[i].activeAlms, iLen))
		{
			// cJSON* pRoot = create_SEMS_C109(pMqtt, &dataBloCmd109, i);
			SEMS_Alarm_Obj = create_SEMS_C109(pMqtt, &dataBloCmd109, i);
			cJSON_AddItemToArray(SEMS_pActvAlm_Array, SEMS_Alarm_Obj);
			memcpy(dataBloCmd109_Old.clientActiveAlarms[i].activeAlms, dataBloCmd109.clientActiveAlarms[i].activeAlms, iLen);

			bNeedSend = TRUE;
		}
	}
	if (bNeedSend)
	{
		cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
		cJSON_AddItemToObject(pRoot, "Alarm_grp_data", SEMS_pActvAlm_Array);
	
		Send_Call(pMqtt, pRoot, C_109_SysActvAlmNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		Publish_MqttMessage(pMqtt);	
		
		TRACE("Alarm Status Changed, Need to Send C109_SysActvAlmNotif\n");
		return 0;
	}
	else
	{
		cJSON_Delete(pRoot);
		cJSON_Delete(SEMS_pActvAlm_Array);
		TRACE("Alarm Status Changed, No Need to Send C109_SysActvAlmNotif\n");
	}

	return 0;
}

int PrePare_SEMS_C109_SysActvAlmNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;
	A_BLOB_SEMS_CMD109 dataBloCmd109;
 	
	dataBloCmd109.emBlobType = BLOB_TYPE_SEMS_CMD109;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD109);
	valGet.val.blobValue = (BYTE*)&dataBloCmd109;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3409, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}
	cJSON *pRoot = cJSON_CreateObject();
	cJSON *SEMS_Alarm_Obj = cJSON_CreateObject();
	cJSON *SEMS_pActvAlm_Array = cJSON_CreateArray();
	int i = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum; ++i)
	{
		// cJSON* pRoot = create_SEMS_C109(pMqtt, &dataBloCmd109, i);
		SEMS_Alarm_Obj = create_SEMS_C109(pMqtt, &dataBloCmd109, i);
		cJSON_AddItemToArray(SEMS_pActvAlm_Array, SEMS_Alarm_Obj);
		// Send_Call(pMqtt, pRoot, C_109_SysActvAlmNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		// Publish_MqttMessage(pMqtt);	
	}
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddItemToObject(pRoot, "Alarm_grp_data", SEMS_pActvAlm_Array);

	Send_Call(pMqtt, pRoot, C_109_SysActvAlmNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	Publish_MqttMessage(pMqtt);	
	return 0;
}
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C109_SysActvAlmNotif()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

int Prepare_C109_SysActvAlmNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	int n = 0,index = 0,nCount = 0,nRet = FALSE,nNeedReporterAlmCount = 0;
	//ERRCODE_DAI 	err;
	//PARAM_VAL_GET 	valGet;
	char szDevName[32];

	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pActvAlm_Array = cJSON_CreateArray();

	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	
	if(pMqtt->nCurrDataFrom == 0)
	{
		ALARM_INFO *pCfgAlmInfo = NULL;
		for(n = 0; n < pMqtt->nActiveAlmTotal; n++)   //count need reporter alarms sum
		{
			nRet = CheckIfNeedShieldAlarms(n,pMqtt);
			if (nRet == FALSE)
			{
				nCount++;
			}
		}
		TRACED("NeesReporterAlarmCount=[%d]  pMqtt->nActiveAlmTotal=[%d]",nCount,pMqtt->nActiveAlmTotal);
		// if (nCount == 0)
		// {
		// 	return 0;	//when all activeAlarms are shielded,no need to reporter
		// }
		cJSON_AddNumberToObject(pRoot, "total_actv_alm", nCount);
 		for(n = 0; n < pMqtt->nActiveAlmTotal; n++)
		{
			nRet = CheckIfNeedShieldAlarms(n,pMqtt);
			if (nRet == TRUE)
			{
				continue;
			}
			pCfgAlmInfo = GetCfgAlarm_ById(pMqtt,	pMqtt->activeAlms[n].nAlarmId);
			if(pCfgAlmInfo == NULL)
			{
				TRACE("Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", \
					pMqtt->activeAlms[n].nDevId,pMqtt->activeAlms[n].nAlarmId);
				continue;
			}
			cJSON *pActvAlm_Obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(pActvAlm_Obj, 	"alarm_id", 			pCfgAlmInfo->nAlarmId);
			cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_level", 			pCfgAlmInfo->emLevel);

			memset(szDevName, 0, sizeof(szDevName));
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETDEVNAME_BYID_R,
					pMqtt->activeAlms[n].nDevId, 0, szDevName, 32);
			cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_dev",				szDevName);//pCfgAlmInfo->pBelonged_DevModel->szDevModelName);
			cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_name",				pCfgAlmInfo->pRes_Name->szName);
			
			char szTime[32];
			cJSON_AddStringToObject(pActvAlm_Obj, 	"alm_begin_time", 		Make_MqttTimeString(pMqtt, pMqtt->activeAlms[n].tmStart, szTime));

			cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_status", 			(pMqtt->activeAlms[n].status == ALARM_STATUS_ACTIVE) ? 0 : 1);
			cJSON_AddNumberToObject(pActvAlm_Obj, 	"alm_dev_id", 			pMqtt->activeAlms[n].nDevId);

			cJSON_AddItemToArray(pActvAlm_Array, pActvAlm_Obj);
		}
	}
 
	cJSON_AddItemToObject(pRoot, "actv_alm_info", pActvAlm_Array);
	
	Send_Call(pMqtt, pRoot, C_109_SysActvAlmNotif,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

int Prepare_SEMS_C110_BatDataNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	A_BLOB_SEMS_CMD110 dataBloCmd110;
	dataBloCmd110.emBlobType = BLOB_TYPE_SEMS_CMD110;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD110);
	valGet.val.blobValue = (BYTE*)&dataBloCmd110;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3410, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}

	int i = 0, n = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum; ++i)
	{
		for (n = 0; n < dataBloCmd110.clientBattCell[i].iBattGroupNum && n < MAX_SINGLE_STBATT_NUM; ++n)
		{
			CTREAT_SEMS_JSON_INFO createInfo = {0};
			createInfo.iClientIdx = i;
			createInfo.nCurrDataFrom = pMqtt->nCurrDataFrom;
			createInfo.iBattIdx = n;

			cJSON 	*pRoot =  CreateJSON_SEMS_C110(&dataBloCmd110, &createInfo);

			Send_Call(pMqtt, pRoot, C_110_SingleBattryInfo, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
			Publish_MqttMessage(pMqtt);
		}
	}
	return 0;
}

int Prepare_C110_SiglBatDataNotif(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	if(pMqtt->nCurrBMS >= MAX_SINGLE_STBATT_NUM)
	{
		return -1;
	}

	int m = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;
	
 	A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
	ZERO_POBJS(&blobIfySiglBat, 1);
	blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
	valGet.val.blobValue = (BYTE*)&blobIfySiglBat;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501 + pMqtt->nCurrBMS, 557, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: Get IFY Single BMS Data Failed!!!\n");
		return -1;
	}
	
	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "batt_idx", 	pMqtt->nCurrBMS);
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

	Send_Call(pMqtt, pRoot, C_110_SingleBattryInfo, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

	return 0;
}

int Prepare_SEMS_C114_PCSUnitInfo(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	A_BLOB_SEMS_CLIENTINFO blobClientInfo;
	ZERO_POBJS(&blobClientInfo,	1);
	blobClientInfo.emBlobType = BLOB_TYPE_SEMS_CLIENTNFO;

	cJSON* pJSClientInfo_SEMS = cJSON_CreateArray();
	int n;
	BOOL bSendUrgent = (pMqtt->emTriggerMsgType == Trigger_C114) ? TRUE : FALSE;

	int nSlaveNumber;

	valGet.nGetSize	= 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 43, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}
	nSlaveNumber = valGet.val.lValue;

	cJSON 	*pRoot = cJSON_CreateObject();

	for(n = 0; n < nSlaveNumber; n++)
	{
		valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO);
		valGet.val.blobValue = (BYTE*)&blobClientInfo;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3301 + n, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO Data----%d", 3301 + n);
			continue;
		}
		
		SEMS_PCS_INFO_DATA* PCS_info = &blobClientInfo.stPcsInfoData; 

		int i;
		
		for(i = 0; i < PCS_info->nPCSNum; ++i)
		{
			cJSON* pJSGrpPMInfo = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJSGrpPMInfo, "Grp", n + 1);
			cJSON_AddNumberToObject(pJSGrpPMInfo, "Index", i + 1);
			cJSON_AddStringToObject(pJSGrpPMInfo, "version", PCS_info->szVersion[i]);
			cJSON_AddStringToObject(pJSGrpPMInfo, "barcode", PCS_info->szBar_code[i]);

			cJSON_AddItemToArray(pJSClientInfo_SEMS, pJSGrpPMInfo);
		}
	}
	cJSON_AddItemToObject(pRoot, "PCS_Unit_Info", pJSClientInfo_SEMS);

	if (NULL != pRoot)
	{
		Send_Call(pMqtt, pRoot, C_114_PCSUnitInfo,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		Publish_MqttMessage(pMqtt);

    }
	return 0;

}

int Prepare_SEMS_C115_SummaryInfo(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	ZERO_POBJS(&pMqtt->clientInfoSum, 1);
	pMqtt->clientInfoSum.emBlobType = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
	valGet.val.blobValue = (BYTE*)&pMqtt->clientInfoSum;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3900, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}

	cJSON 	*pRoot = cJSON_CreateObject();
		
	cJSON_AddNumberToObject(pRoot, "EMS_ctrl_mode", pMqtt->clientInfoSum.emEMSCtrlMode_REMS);
	cJSON_AddFloatToObject(pRoot, "totact_powtgt", pMqtt->clientInfoSum.fActvPwrReq_REMS, 1);
	cJSON_AddNumberToObject(pRoot, "sys_status", pMqtt->clientInfoSum.emCCUStateSum);
	cJSON_AddNumberToObject(pRoot, "run_mode", pMqtt->clientInfoSum.emACDC_WorkStatuSum);
	cJSON_AddFloatToObject(pRoot, "total_soc", pMqtt->clientInfoSum.fSOCSum, 1);
	cJSON_AddFloatToObject(pRoot, "t_actpow_from_cabmeter", pMqtt->clientInfoSum.f_PcabMeterSum, 1);
	cJSON_AddFloatToObject(pRoot, "tmax_chgpow_limit", pMqtt->clientInfoSum.fMax_Chrg_PwrAbilitySum, 1);
	cJSON_AddFloatToObject(pRoot, "tmax_dischgpow_limit", pMqtt->clientInfoSum.fMax_DisChrg_PwrAbilitySum, 1);
	cJSON_AddFloatToObject(pRoot, "t_chgkWh", pMqtt->clientInfoSum.fChrgedEnergySum, 1);
	cJSON_AddFloatToObject(pRoot, "t_output_curr", pMqtt->clientInfoSum.fRealTime_MaxCurrSum, 1);

	if (NULL != pRoot)
	{
		Send_Call(pMqtt, pRoot, C_115_SummaryInfo,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		Publish_MqttMessage(pMqtt);
	}
	return 0;
}

int Prepare_C200_RTPowerInfo(void* pArg)
{	
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	//int nOnLineSTBatt = 0;
	int nDevID = 0, nSigID = 0;
	ERRCODE_DAI 		err;
	PARAM_VAL_GET 		valGet;
	int TimeSector_StopSOC = 0;

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pMqtt->nCurrDataFrom);
	// cJSON_AddNumberToObject(pRoot, "online_batts", pMqtt->nCfgBMSNum);

	// From SEMS Q155
	A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);
	esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		2, 1060, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{	
		return 0;
	}

	cJSON_AddNumberToObject(pRoot, "ac_workstat", esRunData.emAC_WorkStatus);
	cJSON_AddFloatToObject(pRoot, "kw_meter_cab", esRunData.f_PcabMeter, 1);

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1001, 3905, &valGet, sizeof(PARAM_VAL_GET));
	TimeSector_StopSOC = valGet.val.fValue;

	// From SEMS Q104
	A_BLOB_SEMS_CMD104 dataBloCmd104;
	dataBloCmd104.emBlobType = BLOB_TYPE_SEMS_CMD104;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD104);
	valGet.val.blobValue = (BYTE*)&dataBloCmd104;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3404, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{	
		return 0;
	}

	// 累计可放功率
	float AccMaxOutputPower= 0;
	int i = 0;
	int iBattGroupNum = 0;
	for(i = 0; i < pMqtt->lcCfg.nESCNum; i++)
	{
		// iBattGroupNum += dataBloCmd104.clientPowerCalc[i].iBattGroupNum;
		
		// int j = 0;
		// for (j = 0; j < dataBloCmd104.clientPowerCalc[i].iBattGroupNum; ++j)
		// {
			SC6_LDEV_104_Data* pPowerCalcData = &dataBloCmd104.clientPowerCalc[i].powerCalcData[0];
			
			// Alarm状态可用功率处理
			A_BLOB_SEMS_CMD109 dataBloCmd109;
			dataBloCmd109.emBlobType = BLOB_TYPE_SEMS_CMD109;
			valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD109);
			valGet.val.blobValue = (BYTE*)&dataBloCmd109;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3409, &valGet, sizeof(PARAM_VAL_GET));
			if(err != ERRCODE_DAI_OK)
			{
				return 0;
			}
			
			A_BLOB_SEMS_CMD109* pDataBloCmd109 = &dataBloCmd109;
			int iAlarmCount = pDataBloCmd109->clientActiveAlarms[i].iAlarmCount;

			int RealtimeSOC = esRunData.nSTBatt_SOC[i];
			if (RealtimeSOC > TimeSector_StopSOC + 5 || iAlarmCount == 0)
			{
				AccMaxOutputPower = AccMaxOutputPower + pPowerCalcData->batt_max_dischrg_pwr;
			}
		// }
	}

	cJSON_AddNumberToObject(pRoot, "available_max_output_power", AccMaxOutputPower);

	Send_Call(pMqtt, pRoot, C_200_RTPowerInfo, CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	return 0;
}

int Prepare_SEMS_C202_HourSectorStaticKwh(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	A_BLOB_SEMS_CMD202 dataBloCmd202;
	dataBloCmd202.emBlobType = BLOB_TYPE_SEMS_CMD202;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CMD202);
	valGet.val.blobValue = (BYTE*)&dataBloCmd202;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1001, 3411, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		return -1;
	}

	int i = 0, n = 0;
	for (i = 0; i < pMqtt->lcCfg.nESCNum && i < MAX_NUM_ESC; ++i)
	{
		CTREAT_SEMS_JSON_INFO createInfo = {0};
		createInfo.iClientIdx = i;
		createInfo.nCurrDataFrom = pMqtt->nCurrDataFrom;
		createInfo.iBattGroupNum = pMqtt->nCfgBMSNum;
		

		cJSON *pRoot = CreateJSON_SEMS_C202(&dataBloCmd202, &createInfo);
		
		Send_Call(pMqtt, pRoot, C_202_HourSectorStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
		Publish_MqttMessage(pMqtt);
	}

	return 0;
}
/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C202_HourSectorStaticKwh()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
#define GetFeeRateKWHDevNum 2
static BOOL ins_flag = TRUE;
static UINT s_GetFeeRateKWHDevMap[GetFeeRateKWHDevNum]={407,401};
int Prepare_C202_HourSectorStaticKwh(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

	int 			n = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;

	int nDevID = 0, nSigID = 0;
	if(pMqtt->nCurrDataFrom == 0)
	{
		nDevID = 200;
		nSigID = 79;
	}
	else
	{
		nDevID = 2;
		nSigID = 1090;
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
		TRACE("Error: DAI Get Q202 Failed err=[%d]!\n", err);
		return 0;
	}

	int nDev = 407;
	VAR_VALUE varV;
	// for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	// {
	// 	nDev = s_GetFeeRateKWHDevMap[nWhichWh]; //

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Not Installed
		{
			TRACEX("Warning: nDev=[%d] Not Installed!\n", nDev);
			ins_flag = FALSE;
		}
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
				nDev, 401, &varV, 4);
		if (err == ERRCODE_DAI_OK && varV.emValue != 0)
		{
			TRACEX("Warning: nDev=[%d] kWhMeter CommFail!\n", nDev);
			ins_flag = FALSE;
		}
	// }
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
 
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	pMqtt->nCurrDataFrom);

	TRACE("Q202: nCurrDataFrom=[%d]\n", pMqtt->nCurrDataFrom);

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
	for(nGrp = 0; nGrp < pMqtt->nCfgBMSNum; nGrp++)
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

	for(nGrp = 0; nGrp < pMqtt->nCfgBMSNum; nGrp++)
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
	if(ins_flag == TRUE)
	{
		Send_Call(pMqtt, pRoot, C_202_HourSectorStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}
	else
	{
		cJSON_Delete(pRoot);
		pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, 	"data_from", 	pMqtt->nCurrDataFrom);
		Send_Call(pMqtt, pRoot, C_202_HourSectorStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}
	return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME : MQTT  Prepare_C204_DailyStaticKwh()
*  CREATOR       : John
*  CREATED DATE  : 2021-11-23 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Prepare_C204_DailyStaticKwh(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	static UINT nLast_Day = 0;

	int nGrp = 0;
	ERRCODE_DAI 	err;
	PARAM_VAL_GET 	valGet;

	int nDevID = 0, nSigID = 0;
	if(pMqtt->nCurrDataFrom == 0)
	{
		nDevID = 200;
		nSigID = 79;
	}
	else
	{
		nDevID = 2;
		nSigID = 1090;
	}

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

	int nDev = 407;
	VAR_VALUE varV;
	// for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	// {
	// 	nDev = s_GetFeeRateKWHDevMap[nWhichWh]; //

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Not Installed
		{
			TRACEX("Warning: nDev=[%d] Not Installed!\n", nDev);
			ins_flag = FALSE;
		}
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
				nDev, 401, &varV, 4);
		if (err == ERRCODE_DAI_OK && varV.emValue != 0)
		{
			TRACEX("Warning: nDev=[%d] kWhMeter CommFail!\n", nDev);
			ins_flag = FALSE;
		}
	// }
	if((blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day] == 0))
	{
		TRACE("nCurr_Day=[%d] Data Is Not Saved Yet,Go Back!\n");
		return 0;
	}
	cJSON *pRoot = cJSON_CreateObject();
 
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	pMqtt->nCurrDataFrom);
 
	char szTime[32];
	cJSON_AddStringToObject(pRoot, "time_stamp", Make_MqttTimeString(pMqtt, blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day], szTime));

	cJSON_AddFloatToObject(pRoot, "rmt_ac_realtime_kwh",		blobStatData.es_Data.days_Data.fAC_CurrKWH_Reading[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_cost_kwh", 			blobStatData.es_Data.days_Data.fACTotalCostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_cost_kwh", 		blobStatData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_revs_kwh", 			blobStatData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nCurr_Day],  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_input_kwh", 		blobStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_output_kwh", 		blobStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_Day],  1);
	cJSON_AddFloatToObject(pRoot, "dcload_cost_kwh", 			blobStatData.es_Data.days_Data.fDCLoad_CostKWH_D[nCurr_Day],  1);
	
	cJSON* pMppt_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < pMqtt->nCfgBMSNum; nGrp++)
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
	for(nGrp = 0; nGrp < pMqtt->nCfgBMSNum; nGrp++)
	{
		cJSON *pBattGrp_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_chrg", 	blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[nGrp][nCurr_Day],  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_dischrg", 	blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[nGrp][nCurr_Day],  1);
		
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
	for(nGrp = 0; nGrp < pMqtt->nCfgBMSNum; nGrp++)
	{
		cJSON *pExcTimes_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_chrg_times", 		blobStatData.es_Data.days_Data.fBattChrgTimes_D[nGrp][nCurr_Day],  2);
		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_dischrg_times", 	blobStatData.es_Data.days_Data.fBattDisChrgTimes_D[nGrp][nCurr_Day],  2);
		cJSON_AddItemToArray(pExcTimes_Array, pExcTimes_Obj);
	}
	cJSON_AddItemToObject(pRoot, "batt_chrgdischrg_times", pExcTimes_Array);
 	if(ins_flag == TRUE)
	{
		Send_Call(pMqtt, pRoot, C_204_DailyStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}
	else
	{
		cJSON_Delete(pRoot);
		pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, 	"data_from", 	pMqtt->nCurrDataFrom);
		Send_Call(pMqtt, pRoot, C_204_DailyStaticKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!
	}
	return 0;
}

// #define GetFeeRateKWHDevNum 2
#define GetFeeRateKWHTimeNum 3  //current,last1,last2
// static UINT s_GetFeeRateKWHDevMap[GetFeeRateKWHDevNum]={407,401};
int Prepare_C206_SharpPeakFlatValleyKwh(void* pArg)
{
	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

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
	VAR_VALUE varV;
	for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	{
		nDev = s_GetFeeRateKWHDevMap[nWhichWh]; //

		valGet.nGetSize = 4;
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
			TRACED("Warning: nDev=[%d] kWhMeter CommFail!\n", nDev);

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

	Send_Call(pMqtt, pRoot, C_206_FeeRateKwh,  CALL_SRC_Normal, 0, FALSE); //SHOULD be FALSE!!!!

}
 