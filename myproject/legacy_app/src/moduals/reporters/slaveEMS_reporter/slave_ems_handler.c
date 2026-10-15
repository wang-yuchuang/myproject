// #define _TRACE_PRINT_ // SEMSClient
#include "slave_ems.h"


/*
void QueryActvAlarmInfo(ROUGH_SLAVE_ROUGH* pSlave)
{
	REPORTER* pReporter = (REPORTER*)pSlave->pReporter;
	COMMPORT* pComm = pReporter->pUsedCommPort;
	ERRCODE_DAI errCode;
	int n = 0;

	EVENT_ALARM argActvAlm;
	QUERY_COND cond;
	cond.nArgSize = sizeof(EVENT_ALARM);
	cond.pCondArg = &argActvAlm;
	cond.nQueryCondType = 0;
	UINT nQueryCount = MAX_QUERY_ACTV_ALM;
 	ZERO_POBJS(pSlave->activeAlms, MAX_QUERY_ACTV_ALM);
 	errCode = pReporter->pDAI->Query((HANDLE)pReporter, QUERYTYPE_ACTV_ALARM, cond, (void*)pSlave->activeAlms , &nQueryCount);
	TRACE("-----slave ems Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);
	TRACEI("-----slave ems Actv Alarm [%d],  err = %d-----\n", nQueryCount, errCode);

	if(errCode != ERRCODE_DAI_OK)
	{
		TRACE("Error: Query Actv Alarm Failed!\n");
 	}
	else
	{
		pSlave->nActiveAlmTotal = MIN(nQueryCount, MAX_QUERY_ACTV_ALM);
 		for(n = 0; n < pSlave->nActiveAlmTotal; n++)
		{
			TRACE("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pSlave->activeAlms[n].nDevId, pSlave->activeAlms[n].nAlarmId);
			TRACEI("Info:Index[%d] nDevId=[%d] nAlarmId=[%d]\n",\
				n, pSlave->activeAlms[n].nDevId, pSlave->activeAlms[n].nAlarmId);
		}
	}

	return;
}
*/

void Send_SEMS_LSYS(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	TRACED("Enter Send_SEMS_LSYS--B6 -------------------------- ");
	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	int n = 0 ;
	ERRCODE_DAI err = 0;

	A_BLOB_SEMS_CLIENTINFO_SUM  clientSum;
	A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &clientSum;
	ZERO_POBJS(pClientSum, 1);
	pClientSum->emBlobType = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
	valGet.val.blobValue = (BYTE*)pClientSum;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3900, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEE("Send_SEMS_LSYS err:%d", err);
	}
	USHORT nSendDataLen = (USHORT)(sizeof(SEMS_B6_LSYS_DATA_INFO));
	SEMS_B6_LSYS_DATA_INFO sendDataBuff;
	ZERO_POBJS(&sendDataBuff, 1);
	// LJH250610 modified
	sendDataBuff.idxClient = pSlave->lcCfg.idxClientLC; //从机节点编号
	sendDataBuff.tmStamp = GetCurrentBcdTimeStmp();
	sendDataBuff.emCCUStateSum =       (BYTE)pClientSum->emCCUStateSum;
	sendDataBuff.emPMCtrlModeSum =     (BYTE)pClientSum->emPMCtrlModeSum;
	sendDataBuff.emACDC_WorkStatuSum = (BYTE)pClientSum->emACDC_WorkStatuSum;
	// LJH250930 加快自动充放响应,增加待机状态上报
	sendDataBuff.bStandby_SetSum =  pClientSum->bStandby_SetSum;
	sendDataBuff.bStandby_BLLSum = pClientSum->bStandby_BLLSum;
	sendDataBuff.byWorkAsapSum =    pClientSum->byWorkAsapSum;

	sendDataBuff.nSOCMinSum =         pClientSum->nSOCMinSum;
	sendDataBuff.nSOCMaxSum =         pClientSum->nSOCMaxSum;
	sendDataBuff.fSOCSum =            pClientSum->fSOCSum;
	sendDataBuff.fActvPwr_PCSGrpSum = pClientSum->fActvPwr_PCSGrpSum;
	sendDataBuff.fQ_PCSGrpSum =       pClientSum->fQ_PCSGrpSum;

	sendDataBuff.fMax_Chrg_PwrAbilitySum =     pClientSum->fMax_Chrg_PwrAbilitySum;
	sendDataBuff.fMax_DisChrg_PwrAbilitySum =  pClientSum->fMax_DisChrg_PwrAbilitySum;
	sendDataBuff.fMax_Chrg_PwrAbilitySumA =     pClientSum->fMax_Chrg_PwrAbilitySumA;
	sendDataBuff.fMax_DisChrg_PwrAbilitySumA =  pClientSum->fMax_DisChrg_PwrAbilitySumA;
	sendDataBuff.fMax_Chrg_PwrAbilitySum0 =    pClientSum->fMax_Chrg_PwrAbilitySum0;
	sendDataBuff.fMax_DisChrg_PwrAbilitySum0 = pClientSum->fMax_DisChrg_PwrAbilitySum0;

	sendDataBuff.fChrgedEnergySum =       pClientSum->fChrgedEnergySum;
	sendDataBuff.fRealTime_MaxCurrSum =   pClientSum->fRealTime_MaxCurrSum;
	sendDataBuff.fACDC_RealTime_VoltSum = pClientSum->fACDC_RealTime_VoltSum;
	sendDataBuff.fACDC_RealTime_CurrSum = pClientSum->fACDC_RealTime_CurrSum;

	sendDataBuff.f_PgridMeterSum =  pClientSum->f_PgridMeterSum;
	sendDataBuff.f_PcabMeterSum =   pClientSum->f_PcabMeterSum;
	TRACEI("Send_SEMS_LSYS sendDataBuff idxClient:%u "
		"ClientSum emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d "
        "fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
		"\nfMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySum0:%.1f %.1f "
		"fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
		"f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f bStandby_BLLSum:%d byWorkAsapSum:%d",
		sendDataBuff.idxClient,
		sendDataBuff.emCCUStateSum, sendDataBuff.emPMCtrlModeSum, sendDataBuff.emACDC_WorkStatuSum, 
        sendDataBuff.fSOCSum, sendDataBuff.fActvPwr_PCSGrpSum, sendDataBuff.fQ_PCSGrpSum,
		sendDataBuff.fMax_Chrg_PwrAbilitySum, sendDataBuff.fMax_DisChrg_PwrAbilitySum, sendDataBuff.fMax_Chrg_PwrAbilitySum0, sendDataBuff.fMax_DisChrg_PwrAbilitySum0,
		sendDataBuff.fChrgedEnergySum, sendDataBuff.fRealTime_MaxCurrSum, sendDataBuff.fACDC_RealTime_VoltSum, sendDataBuff.fACDC_RealTime_CurrSum,
		sendDataBuff.f_PgridMeterSum, sendDataBuff.f_PcabMeterSum, sendDataBuff.bStandby_BLLSum, sendDataBuff.byWorkAsapSum);
	reverseBytes((BYTE*)&sendDataBuff, nSendDataLen);
	if(Slave_WriteData(pSlave, (BYTE*)&sendDataBuff, NLMSG_SEMS_LSYS_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}

// edit by KYA 20250520
void Send_SEMS_LDEV_C6_101(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_101 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	int n = 0;
	TRACEW("Send_SEMS_LDEV_C6_101 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
			if(err != ERRCODE_DAI_OK)
			{
				TRACEW("Send_SEMS_LDEV_C6_101 get ES RealTime Data failed");
				return;
			}

			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_101_DATA = sizeof(SC6_LDEV_101_Data); // 38H
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        101;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_101_DATA;
			pHead->byDevNum =     pSlave->lcCfg.nESCNum;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);
			
			SC6_LDEV_101_Data* pESSRealTimeStatus = (SC6_LDEV_101_Data*)(&sendDataBuff[0 + SIZE_HEAD]);
			pESSRealTimeStatus->cp_status = (BYTE)valGet.val.emValue;
			pESSRealTimeStatus->grid_status = 0;

			valGet.nGetSize = 4;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 6, &valGet, sizeof(PARAM_VAL_GET));
			// pESSRealTimeStatus->cp_fault = valGet.val.emValue;
			pESSRealTimeStatus->cp_fault = 0;

			pSlave->nConnectSlvNum = 0;
			pESSRealTimeStatus->chrg_slvcab_num = (BYTE)pSlave->nConnectSlvNum;

			if(pSlave->nConnectSlvNum > 0)
			{
				for(n = 0; n < pSlave->nConnectSlvNum; n++) //
				{
					pESSRealTimeStatus->data_from = (BYTE)(n + 1);
					pESSRealTimeStatus->cp_fault = 0;
					pESSRealTimeStatus->comm_status = 0;
					pESSRealTimeStatus->cp_status = 0;
				}
			}

			pESSRealTimeStatus->powercube_sys_temp = 0;

			reverseBytes((BYTE*)pESSRealTimeStatus, SIZE_101_DATA);
			nSendDataLen = SIZE_HEAD + SIZE_101_DATA;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}

// edit by KYA 20250520
void Send_SEMS_LDEV_C6_102(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_102 return is LC_TYPE_1st");
		return;
	}

	int n = 0, m = 0;
	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	TRACEW("Send_SEMS_LDEV_C6_102 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			A_BLOB_BATTCAB_IMEU2_V20	blobIMEU2_V20[MAX_BATTCABINET_NUM];

			if(pSlave->nCfgBMSNum > MAX_BATTCABINET_NUM)
			{	
				TRACEW("BMS Number Cfg is Invalid!");
				return;
			}

			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_102_DATA = sizeof(SC6_LDEV_102_Data); // 38H
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        102;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_102_DATA;
			pHead->byDevNum =     1;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);
			SC6_LDEV_102_Data* pBatteryCabinetStatus = (SC6_LDEV_102_Data*)(&sendDataBuff[0 + SIZE_HEAD]);

			for(n = 0; n < pSlave->nCfgBMSNum; n++)
			{
				ZERO_POBJS(&blobIMEU2_V20[n], 1);
				blobIMEU2_V20[n].emBlobType = BLOB_TYPE_BATTCAB_IMEU2_V20;
				valGet.nGetSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);

				valGet.val.blobValue = (BYTE*)&blobIMEU2_V20[n];
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
					601 + n, 450, &valGet, sizeof(PARAM_VAL_GET));
				if(err != ERRCODE_DAI_OK)
				{
					TRACEW("Send_SEMS_LDEV_C6_102 get Battery Cabinet Status failed");
					continue;
				}
				
				// int idxESC4Blob = GetIdx0_LC4cab(pSlave->lcCfg.idxClientLC);

				// 3.SC6_LDEV_102_Data
				pBatteryCabinetStatus->batt_cab_num = (BYTE)pSlave->nCfgBMSNum;
				pBatteryCabinetStatus->cab_idx = (BYTE)n;

				pBatteryCabinetStatus->comm_statu = (BYTE)blobIMEU2_V20[n].emIMEU2_CommFail;
				pBatteryCabinetStatus->flood_alarm = (BYTE)blobIMEU2_V20[n].emFlood;
				pBatteryCabinetStatus->dooropen_alarm = (BYTE)blobIMEU2_V20[n].emDI3_DoorOpen;
				pBatteryCabinetStatus->smoke_alarm = (BYTE)blobIMEU2_V20[n].emDI2_Smoke;
				pBatteryCabinetStatus->fire_alarm = (BYTE)blobIMEU2_V20[n].emDI1_Fire;
				pBatteryCabinetStatus->combustible_gas_alarm = (BYTE)blobIMEU2_V20[n].emDI4_FlammableGas;

				pBatteryCabinetStatus->dump_alarm = 0;
				pBatteryCabinetStatus->electric_blinds_open_stat = (BYTE)blobIMEU2_V20[n].emShutterOpenedOver;
				pBatteryCabinetStatus->electric_blinds_close_stat = (BYTE)blobIMEU2_V20[n].emDI6_ShutterClosedOver;
				pBatteryCabinetStatus->nHumiTempSensorNumber = blobIMEU2_V20[n].nHumiTempSensorNumber;// KYA20250609

				for(m = 0; m < blobIMEU2_V20[n].nHumiTempSensorNumber; m++)
				{
					pBatteryCabinetStatus->temp_humi_sensor_comm_stat[m] = (BYTE)blobIMEU2_V20[n].emTempHumi_CommFail[m];
					pBatteryCabinetStatus->cab_temp_value[m] = blobIMEU2_V20[n].fTemperature[m];
					pBatteryCabinetStatus->cab_humi_value[m] = blobIMEU2_V20[n].fHumidity[m];
				}

				//--- cab_pt1000_data ---//
				for(m = 0; m < 4; m++)
				{
					pBatteryCabinetStatus->comb_pt1000_value[m] = blobIMEU2_V20[n].fPT1000Temperature[m];
				}

				pBatteryCabinetStatus->exhaust_fan_speed_value = (float)blobIMEU2_V20[n].nExhaustFanSpeed;
				pBatteryCabinetStatus->aircond_comm_stat = (BYTE)blobIMEU2_V20[n].emAirCond_CommFail;
				pBatteryCabinetStatus->aircond_alarm = (BYTE)blobIMEU2_V20[n].emAirCondTotalAlm;
				pBatteryCabinetStatus->aircond_ret_temp = blobIMEU2_V20[n].fRealTimeCabTemp;
				pBatteryCabinetStatus->aircond_evaporator_temp = blobIMEU2_V20[n].fRealTimeCabTemp;

				pBatteryCabinetStatus->aircond_condenser_temp = blobIMEU2_V20[n].fRealTimeCabTemp;
				pBatteryCabinetStatus->aircond_compressor_start_temp = blobIMEU2_V20[n].fCoolSetPoint;
				pBatteryCabinetStatus->aircond_heater_start_temp = blobIMEU2_V20[n].fHeatDeviation;
				pBatteryCabinetStatus->aircond_machine_state = (BYTE)blobIMEU2_V20[n].emOnOrOffd;
				pBatteryCabinetStatus->dehumi_start_temperature = blobIMEU2_V20[n].fDehumiStartTemp;

				pBatteryCabinetStatus->dehumi_stop_temperature = blobIMEU2_V20[n].fDehumiStopTemp;
				pBatteryCabinetStatus->dehumi_start_humidity = blobIMEU2_V20[n].fDehumiStartHumi;
				pBatteryCabinetStatus->dehumi_stop_humidity = blobIMEU2_V20[n].fDehumiStopHumi;
				pBatteryCabinetStatus->nGasSensorNum = blobIMEU2_V20[n].nGasSensorNum;
				//comb_gas_datainfo
				for(m = 0; m < blobIMEU2_V20[n].nGasSensorNum; m++)
				{
					pBatteryCabinetStatus->combustible_gas_comm_stat[m] = (BYTE)blobIMEU2_V20[n].emGasSensor_CommFail[m];
					pBatteryCabinetStatus->combustible_gas_alarm_stat = (BYTE)blobIMEU2_V20[n].emDI4_FlammableGas;
					pBatteryCabinetStatus->comb_gas_concentration_value[m] = blobIMEU2_V20[n].fCombGasPotency[m];
				}
				reverseBytes((BYTE*)pBatteryCabinetStatus, SIZE_102_DATA);
				nSendDataLen = SIZE_HEAD + SIZE_102_DATA;
			}
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}


// edit by KYA 20250520
void Send_SEMS_LDEV_C6_103(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_103 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	TRACEW("Send_SEMS_LDEV_C6_103 typeLC:%d", pSlave->lcCfg.lcType);
	int n = 0;
	if(pSlave->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		TRACEW("BMS Number Cfg is Invalid!");
		return;
	}

	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_103_DATA = sizeof(SC6_LDEV_103_Data); // 38H

			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        103;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_103_DATA;
			pHead->byDevNum =     pSlave->nCfgBMSNum;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);

			nSendDataLen = SIZE_HEAD;

			SC6_LDEV_103_Data* pBMSInfoStatus = (SC6_LDEV_103_Data*)(&sendDataBuff[SIZE_HEAD]);

			A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
			for(n = 0; n < pSlave->nCfgBMSNum; n++)
			{
				ZERO_POBJS(&blobSiglBatt, 1);
				blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
				valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
				valGet.val.blobValue = (BYTE*)&blobSiglBatt;
				 err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				 	501 + n, 551, &valGet, sizeof(PARAM_VAL_GET));
			
				pBMSInfoStatus->batt_idx = (BYTE)n;

				if(blobSiglBatt.fPackTotalCurr >= 2.0)
				{
					pBMSInfoStatus->run_statu = STBATT_RUNSTAT_CHARGE;
				}
				else if(blobSiglBatt.fPackTotalCurr <= -2.0)
				{	
					pBMSInfoStatus->run_statu = STBATT_RUNSTAT_DISCHARGE;
				}
				else
				{
					pBMSInfoStatus->run_statu = blobSiglBatt.emBattStatus_Run;
				}
				pBMSInfoStatus->pack_volt = blobSiglBatt.fPackTotalVolt;
				pBMSInfoStatus->pack_curr = blobSiglBatt.fPackTotalCurr;
				pBMSInfoStatus->soh = (BYTE)blobSiglBatt.nSOH;
				pBMSInfoStatus->soc = (BYTE)blobSiglBatt.nSOC;
				pBMSInfoStatus->cycle = (BYTE)blobSiglBatt.nLifeCycleTimes;

				pBMSInfoStatus->max_allow_chg_volt = blobSiglBatt.fAllow_LmtChargeVolt;
				pBMSInfoStatus->max_allow_chg_curr = blobSiglBatt.fAllow_MaxChargeCurr;
				pBMSInfoStatus->max_allow_dischg_volt = blobSiglBatt.fAllow_LmtDisChargeVolt;
				pBMSInfoStatus->max_allow_dischg_curr = blobSiglBatt.fAllow_MaxDisChargeCurr;
				pBMSInfoStatus->cell_max_volt = blobSiglBatt.fMax_CellVolt;

				pBMSInfoStatus->cell_min_volt = blobSiglBatt.fMin_CellVolt;
				pBMSInfoStatus->cell_avg_volt = blobSiglBatt.fAvg_CellVolt;
				pBMSInfoStatus->cell_max_temp = blobSiglBatt.fMax_CellTemp;
				pBMSInfoStatus->cell_min_temp = blobSiglBatt.fMin_CellTemp;
				pBMSInfoStatus->cell_avg_temp = blobSiglBatt.fAvg_CellTemp;

				pBMSInfoStatus->ucell_max_csc = (float)blobSiglBatt.uiBAT_Ucell_Max_CSC;
				pBMSInfoStatus->ucell_max_cell = (float)blobSiglBatt.uiBAT_Ucell_Max_CELL;
				pBMSInfoStatus->ucell_min_csc = (float)blobSiglBatt.uiBAT_Ucell_Min_CSC;
				pBMSInfoStatus->ucell_min_cell = (float)blobSiglBatt.uiBAT_Ucell_Min_CELL;
				pBMSInfoStatus->temp_max_csc = (float)blobSiglBatt.nBAT_T_Max_CSC;

				pBMSInfoStatus->temp_min_csc = (float)blobSiglBatt.nBAT_T_Min_CSC;
				pBMSInfoStatus->remain_charge_energy = (float)blobSiglBatt.fChrg_Energy;
				pBMSInfoStatus->remain_discharge_energy = (float)blobSiglBatt.fDisChrging_Energy;
				pBMSInfoStatus->chrg_forbid = (BYTE)blobSiglBatt.bIsBatt_ChargeForbidden;
				pBMSInfoStatus->dischrg_forbid = (BYTE)blobSiglBatt.bIsBatt_DisChargeForbidden;

				pBMSInfoStatus->alarm_status = blobSiglBatt.emBattStatus_Alm;
				memcpy(pBMSInfoStatus->alarm_info, blobSiglBatt.szS2A_AlarmInfo, sizeof(pBMSInfoStatus->alarm_info));
				memcpy(pBMSInfoStatus->alarm2_info, blobSiglBatt.szInfy_AlarmInfo, sizeof(pBMSInfoStatus->alarm2_info));
				memcpy(pBMSInfoStatus->fault_info, blobSiglBatt.szInfy_FaultInfo, sizeof(pBMSInfoStatus->fault_info));
				memcpy(pBMSInfoStatus->gpio_info, blobSiglBatt.szInfy_GPIOInfo, sizeof(pBMSInfoStatus->gpio_info));

				pBMSInfoStatus->batt_ver = blobSiglBatt.fVersion;

				reverseBytes((BYTE*)pBMSInfoStatus, SIZE_103_DATA);
				nSendDataLen += SIZE_103_DATA;
				++pBMSInfoStatus;
			}
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}

// edit by KYA 20250520
void Send_SEMS_LDEV_C6_104(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_104 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	TRACEW("Send_SEMS_LDEV_C6_104 typeLC:%d", pSlave->lcCfg.lcType);
	int n = 0;
	if(pSlave->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		TRACEW("BMS Number Cfg is Invalid!");
		return;
	}
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_104_DATA = sizeof(SC6_LDEV_104_Data); // 38H
			USHORT usLenSingle = SIZE_104_DATA * pSlave->nCfgBMSNum;

			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        104;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  usLenSingle;
			pHead->byDevNum =     pSlave->nCfgBMSNum;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);

			A_BLOB_BRANCH_NODE_CALC_DATA nodeCalcData;
			valGet.nGetSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
			valGet.val.blobValue = (BYTE*)&nodeCalcData;

			SC6_LDEV_104_Data* pPowerDistInfo = (SC6_LDEV_104_Data*)(&sendDataBuff[SIZE_HEAD]);

			for(n = 0; n < pSlave->nCfgBMSNum; n++)
			{
				ZERO_POBJS(&nodeCalcData, 1);
				nodeCalcData.emBlobType = BLOB_TYPE_BRANCH_NODE_CALC_DATA;

				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 221 + n, 203, &valGet, sizeof(PARAM_VAL_GET));
			
				// 1.SC6_LDEV_104_Data
				pPowerDistInfo->branch_idx = (BYTE)n;
				pPowerDistInfo->batt_max_dischrg_pwr = nodeCalcData.fBatt_Max_DisChrg_Pwr;
				pPowerDistInfo->batt_max_chrg_pwr_req = nodeCalcData.fBatt_Max_Chrg_Pwr_Req;
				pPowerDistInfo->batt_distri4acdc_pwr = nodeCalcData.fBatt_Distri_4_ACDC_Pwr;

				pPowerDistInfo->batt_distri4dcdc_pwr = nodeCalcData.fDCDC_Max_Ability_Pwr;
				pPowerDistInfo->acdc_rate_pwr = nodeCalcData.fACDC_Max_Ability_Pwr;
				pPowerDistInfo->dcdc_rate_pwr = nodeCalcData.fDCDC_Max_Ability_Pwr;
				pPowerDistInfo->remain_batt_dischrg_pwr = nodeCalcData.fRemain_Batt_DisChrg_Pwr;
				pPowerDistInfo->remain_batt_chrg_pwr_req = nodeCalcData.fRemain_Batt_Chrg_Pwr_Req;
				pPowerDistInfo->acdc_distri4batt_chrg_pwr = nodeCalcData.fACDC_Distri_4_BattChrg_Pwr;

				reverseBytes((BYTE*)pPowerDistInfo, SIZE_104_DATA);

				++pPowerDistInfo;
			}

			nSendDataLen = SIZE_HEAD + usLenSingle;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}

// LJH250428 cmd155
void Send_SEMS_LDEV_C6_155(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_155 return is LC_TYPE_1st");
		return;
	}
	static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
	static const int SIZE_BRANCH = sizeof(SC6LDEV_CMD155_BranchData);
	static const int SIZE_SINGLE = sizeof(SC6LDEV_CMD155_ESCData);

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	TRACEW("Send_SEMS_LDEV_C6_155 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// // 二级主控
			// A_BLOB_CMD155 blobCmd105;
			// ZERO_POBJS(&blobCmd105, 1);
			// VAR_VALUE varV;
			// varV.blobValue = (BYTE*)&blobCmd105;
			// pCCU->pDPR->Get(hSamplerSelf, GETTYPE_SP_VAR, 1001, 3405, &varV, sizeof(A_BLOB_CMD155));
			// SC6LDEV_CMDHead* pHead = &blobCmd105.cmdHead;
			// SC6_LDEV_155* pCmds = &blobCmd105.cmds;


			// A_BLOB_CMD155 blobCmd105;
			// blobCmd105.emBlobType = BLOB_TYPE_CMD155;
			// valGet.nGetSize = sizeof(A_BLOB_CMD155);
			// valGet.val.blobValue = (BYTE*)&blobCmd105;
			// err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			// 	1001, 3405, &valGet, sizeof(PARAM_VAL_GET));
			// if(err != ERRCODE_DAI_OK)
			// {
			// 	TRACEE("Send_SEMS_LDEV_C6_155 Error: DAI Get A_BLOB_CMD155 Failed err:%d", err);
			// 	return;
			// }
			// SC6LDEV_CMDHead* pHead = &blobCmd105.cmdHead;
			// SC6_LDEV_155* pCmds = &blobCmd105.cmds;

			// SC6LDEV_CMDHead pHead = cmd105Sum.cmdHead;
			// // cmdHead.usCmd;
			// // cmdHead.usLenSingle;
			// int i;
			// for (i=0; i<cmdHead.byDevNum; i++)
			// {
			// 	cmd105Sum.cmd105s[i]

			// }
			// SC6_LDEV_155 cmd105;
			// cmd105.idxClient = pSlave->lcCfg.idxClientLC;
			return;
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
			return;
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			A_BLOB_ES_RUN_DATA esRunData;
			esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
			valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
			valGet.val.blobValue = (BYTE*)&esRunData;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				200, 80, &valGet, sizeof(PARAM_VAL_GET));
			if(err != ERRCODE_DAI_OK)
			{
				TRACEW("Send_SEMS_LDEV_C6_155 get A_BLOB_ES_RUN_DATA failed");
				return;
			}
			// UINT uIdx4Blob = GetIdx0_LC4cab(pSlave->lcCfg.idxClientLC);
			// esRunData.emAC_WorkStatus = 3;
			// esRunData.fACVolt_L1 =          221 + uIdx4Blob; // 43 5D 00 00
			// esRunData.fACVolt_L2 =          222 + uIdx4Blob;
			// esRunData.fACVolt_L3 =          223 + uIdx4Blob;
			// esRunData.fACCurr_L1 =           11 + uIdx4Blob; // 41 30 00 00
			// esRunData.fACCurr_L2 =           12 + uIdx4Blob;
			// esRunData.fACCurr_L3 =           13 + uIdx4Blob;
			// esRunData.f_PcabMeter =         220 + uIdx4Blob; // 43 5C 00 00
			// esRunData.fACDC_OutVolt[0] =     14 + uIdx4Blob; // 41 60 00 00
			// esRunData.fSTBatt_OutCurr[0] =   15 + uIdx4Blob;
			// esRunData.fACDCOutRelayVolt[0] = 16 + uIdx4Blob;
			// esRunData.nSTBatt_SOC[0] =       90 + uIdx4Blob; // 5A

			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        155;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_SINGLE;
			pHead->byDevNum =     1;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);
			
			// 2.SC6LDEV_CMD155_BranchData 1st没用到,先填充
			SC6LDEV_CMD155_BranchData* pBranchData = (SC6LDEV_CMD155_BranchData*)(&sendDataBuff[0 + SIZE_HEAD]);
			pBranchData->dev_id =   pSlave->lcCfg.idxClientLC;
			pBranchData->ac_workstat =  (BYTE)esRunData.emAC_WorkStatus;
			pBranchData->ac_volts[2] = esRunData.fACVolt_L1;
			pBranchData->ac_volts[1] = esRunData.fACVolt_L2;
			pBranchData->ac_volts[0] = esRunData.fACVolt_L3;

			pBranchData->ac_currs[2] = esRunData.fACCurr_L1;
			pBranchData->ac_currs[1] = esRunData.fACCurr_L2;
			pBranchData->ac_currs[0] = esRunData.fACCurr_L3;
			// pBranchData->position_auxmeter = (BYTE)esRunData.emAuxPos;
			pBranchData->kw_meter_grid = esRunData.f_PgridMeter;

			pBranchData->kw_meter_cab = esRunData.f_PcabMeter;
			pBranchData->kw_acload =    esRunData.fACLoad_Power;

			pBranchData->usLenSingle = (USHORT)SIZE_SINGLE;
			pBranchData->byESCNum = 1;
			reverseBytes((BYTE*)pBranchData, SIZE_BRANCH);
			// 3.SC6LDEV_CMD155_ESCData
			SC6LDEV_CMD155_ESCData* pESCData = (SC6LDEV_CMD155_ESCData*)(&sendDataBuff[0 + SIZE_HEAD + SIZE_BRANCH]);
			pESCData->dev_id =       pSlave->lcCfg.idxClientLC;
			pESCData->tmStamp =       GetCurrentBcdTimeStmp();

			pESCData->ac_workstat = (BYTE)esRunData.emAC_WorkStatus;
			pESCData->ac_volts[2] =   esRunData.fACVolt_L1;
			pESCData->ac_volts[1] =   esRunData.fACVolt_L2;
			pESCData->ac_volts[0] =   esRunData.fACVolt_L3;
			pESCData->ac_currs[2] =   esRunData.fACCurr_L1;
			pESCData->ac_currs[1] =   esRunData.fACCurr_L2;
			pESCData->ac_currs[0] =   esRunData.fACCurr_L3;
			pESCData->kw_meter_cab =  esRunData.f_PcabMeter;

			pESCData->acdc_workstat = esRunData.emACDC_WorkStatu[0];
			pESCData->acdc_volt =     esRunData.fACDC_OutVolt[0];
			pESCData->acdc_curr =     esRunData.fACDC_OutCurr[0];
			pESCData->acdc_ext_volt = esRunData.fACDCOutRelayVolt[0];

			int nSTBattStatu;
			if(esRunData.fSTBatt_OutCurr[0] > 1.0)
			{
				nSTBattStatu = 2;
			}
			else if(esRunData.fSTBatt_OutCurr[0] < -1.0)
			{
				nSTBattStatu = 3;
			}
			else
			{
				nSTBattStatu = 1;
			}
			pESCData->batt_workstat = (BYTE)nSTBattStatu;
			pESCData->batt_soc =      (BYTE)esRunData.nSTBatt_SOC[0];
			pESCData->batt_volt =     esRunData.fSTBatt_OutVolt[0];
			pESCData->batt_curr =     esRunData.fSTBatt_OutCurr[0];

			reverseBytes((BYTE*)pESCData, SIZE_SINGLE);
			nSendDataLen = SIZE_HEAD + SIZE_BRANCH + SIZE_SINGLE;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Send_SEMS_LDEV_C6_155 Err:Write LSYS Data To Station EMS Abnormal!");
	}
}

// LJH250520 cmd157
void Send_SEMS_LDEV_C6_157(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_157 return is LC_TYPE_1st");
		return;
	}
	static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
	static const int SIZE_SINGLE = sizeof(SC6LDEV_CMD157);

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;

	// 1.SC6LDEV_CMDHead
	SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
	pHead->usCmd =        157;
	pHead->idxClient =    pSlave->lcCfg.idxClientLC;
	pHead->usLenSingle =  (USHORT)SIZE_SINGLE;
	TRACEW("Send_SEMS_LDEV_C6_157 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			return;
		}
		break;

		case LC_TYPE_3rd:
		{
			return;
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			if (!pSlave->lcCfg.bCabMeter)
			{
				TRACEW("Send_SEMS_LDEV_C6_157 return; !bCabMeter");
				return;
			}
			// 1.SC6LDEV_CMDHead
			pHead->byDevNum =     1;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);
			// 2.SC6LDEV_CMD157
			SC6LDEV_CMD157* pSingle = (SC6LDEV_CMD157*)(&sendDataBuff[0 + SIZE_HEAD]);
			pSingle->dev_id =       pSlave->lcCfg.idxClientLC;
			pSingle->tmStamp =       GetCurrentBcdTimeStmp();
			pSingle->data_from = 0; // 0-储能柜交流电表
			GetMeterValue_C157(pReporter, 407, pSingle);
			reverseBytes((BYTE*)pSingle, SIZE_SINGLE);
			nSendDataLen = SIZE_HEAD + SIZE_SINGLE;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Send_SEMS_LDEV_C6_157 Err:Write LSYS Data To Station EMS Abnormal!");
	}
}

// edit by KYA 20250520
void Send_SEMS_LDEV_C6_106(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_106 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	int n = 0, nRectType = 0;
	int nOnlineACDCNum = 0, nOnlineDCDCNum = 0, nOnlineMPPTNum = 0;

	TRACEW("Send_SEMS_LDEV_C6_106 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
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
				return;
			}

			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_106_DATA = sizeof(A_BLOB_SINGLE_RECT_INFO_106); // 38H
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        106;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_106_DATA;
			pHead->byDevNum =     1;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);

			A_BLOB_SINGLE_RECT_INFO_106* pPCSStatusInfo = (A_BLOB_SINGLE_RECT_INFO_106*)(&sendDataBuff[0 + SIZE_HEAD]);			
			for(nRectType = 0; nRectType < MAX_RECT_TYPE; nRectType++)
			{
				UINT count = sigRect.nOnlineNum[nRectType];
				pPCSStatusInfo->nOnlineNum[nRectType] = sigRect.nOnlineNum[nRectType];
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
						// 3.SC6_LDEV_106_Data
						nOnlineACDCNum++;

						pPCSStatusInfo->arrRectPcs[nRectType][n].comm_statu = sigRect.arrRect[nRectType][n].byCommStatu;
						pPCSStatusInfo->arrRectPcs[nRectType][n].belong_grp = (BYTE)sigRect.arrRect[nRectType][n].nBelongGrp;
						pPCSStatusInfo->arrRectPcs[nRectType][n].work_mode = (BYTE)sigRect.arrRect[nRectType][n].emWorkMode;
						pPCSStatusInfo->arrRectPcs[nRectType][n].err_code = 0;
						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode, sigRect.arrRect[nRectType][n].byRectErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode));

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode, sigRect.arrRect[nRectType][n].byInvertErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode));
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_volt = sigRect.arrRect[nRectType][n].fRTOutputVolt;
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_curr = sigRect.arrRect[nRectType][n].fRTOutputCurr;
						pPCSStatusInfo->arrRectPcs[nRectType][n].dcdc_ver = sigRect.arrRect[nRectType][n].fRect_DCDCVer;
						pPCSStatusInfo->arrRectPcs[nRectType][n].pfc_var = sigRect.arrRect[nRectType][n].fRect_PFCVer;

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code, sigRect.arrRect[nRectType][n].byBarCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code));
						pPCSStatusInfo->arrRectPcs[nRectType][n].active_power = sigRect.arrRect[nRectType][n].fPtotalActive;
						pPCSStatusInfo->arrRectPcs[nRectType][n].reactive_power = sigRect.arrRect[nRectType][n].fPtotalReactive;

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
						pPCSStatusInfo->arrRectPcs[nRectType][n].comm_statu = sigRect.arrRect[nRectType][n].byCommStatu;
						pPCSStatusInfo->arrRectPcs[nRectType][n].belong_grp = (BYTE)sigRect.arrRect[nRectType][n].nBelongGrp;
						pPCSStatusInfo->arrRectPcs[nRectType][n].work_mode = (BYTE)sigRect.arrRect[nRectType][n].emWorkMode;
						pPCSStatusInfo->arrRectPcs[nRectType][n].err_code = 0;
						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode, sigRect.arrRect[nRectType][n].byRectErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode));

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode, sigRect.arrRect[nRectType][n].byInvertErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode));
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_volt = sigRect.arrRect[nRectType][n].fRTOutputVolt;
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_curr = sigRect.arrRect[nRectType][n].fRTOutputCurr;
						pPCSStatusInfo->arrRectPcs[nRectType][n].dcdc_ver = sigRect.arrRect[nRectType][n].fRect_DCDCVer;
						pPCSStatusInfo->arrRectPcs[nRectType][n].pfc_var = sigRect.arrRect[nRectType][n].fRect_PFCVer;

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code, sigRect.arrRect[nRectType][n].byBarCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code));

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
						pPCSStatusInfo->arrRectPcs[nRectType][n].comm_statu = sigRect.arrRect[nRectType][n].byCommStatu;
						pPCSStatusInfo->arrRectPcs[nRectType][n].belong_grp = (BYTE)sigRect.arrRect[nRectType][n].nBelongGrp;
						pPCSStatusInfo->arrRectPcs[nRectType][n].work_mode = (BYTE)sigRect.arrRect[nRectType][n].emWorkMode;
						pPCSStatusInfo->arrRectPcs[nRectType][n].err_code = 0;
						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode, sigRect.arrRect[nRectType][n].byRectErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].rect_mode_errcode));

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode, sigRect.arrRect[nRectType][n].byInvertErrCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].invrt_mode_errcode));
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_volt = sigRect.arrRect[nRectType][n].fRTOutputVolt;
						pPCSStatusInfo->arrRectPcs[nRectType][n].output_curr = sigRect.arrRect[nRectType][n].fRTOutputCurr;
						pPCSStatusInfo->arrRectPcs[nRectType][n].dcdc_ver = sigRect.arrRect[nRectType][n].fRect_DCDCVer;
						pPCSStatusInfo->arrRectPcs[nRectType][n].pfc_var = sigRect.arrRect[nRectType][n].fRect_PFCVer;

						memcpy(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code, sigRect.arrRect[nRectType][n].byBarCode, sizeof(pPCSStatusInfo->arrRectPcs[nRectType][n].bar_code));
					}
				}
			}

			pPCSStatusInfo->online_acdc_num = (BYTE)nOnlineACDCNum;
			pPCSStatusInfo->online_mppt_num = (BYTE)nOnlineMPPTNum;
			pPCSStatusInfo->online_dcdc_num = (BYTE)nOnlineDCDCNum;

			reverseBytes((BYTE*)pPCSStatusInfo, SIZE_106_DATA);
			nSendDataLen = SIZE_HEAD + SIZE_106_DATA;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}

// edit by KYA 20250520
void Send_SEMS_LDEV_C6_108(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_108 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };
	int nSendDataLen = 0;
	TRACEW("Send_SEMS_LDEV_C6_108 typeLC:%d", pSlave->lcCfg.lcType);

	char			szIMMU2_SWVer[MAX_LEN_MU2SWVER];
    char            szIMMU2_HWVer[8];
	memset(szIMMU2_SWVer, 0 ,sizeof(szIMMU2_SWVer));
	memset(szIMMU2_HWVer, 0 ,sizeof(szIMMU2_HWVer));

	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			valGet.nGetSize = 4;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
			if(err != ERRCODE_DAI_OK)
			{
				TRACEW("Send_SEMS_LDEV_C6_108 get A_BLOB_ES_RUN_DATA failed");
				return;
			}

			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_108_CDATA = sizeof(SC6_LDEV_108_Data); // 38H
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        108;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->usLenSingle =  (USHORT)SIZE_108_CDATA;
			pHead->byDevNum =     1;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);
			
			// 3.SC6_LDEV_108_Data
			SC6_LDEV_108_Data* pESSDeviceVer = (SC6_LDEV_108_Data*)(&sendDataBuff[0 + SIZE_HEAD]);

			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R, 0, 0, szIMMU2_SWVer, MAX_LEN_MU2SWVER);
			if(err == ERRCODE_DAI_OK)
			{
				strncpy((char *)pESSDeviceVer->sw_main_ver, (char *)szIMMU2_SWVer, sizeof(pESSDeviceVer->sw_main_ver) - 1);
				pESSDeviceVer->sw_main_ver[sizeof(pESSDeviceVer->sw_main_ver) - 1] = '\0';			
			}
			else
			{
				strcpy(pESSDeviceVer->sw_main_ver, "--");
			}

			strncpy((char *)pESSDeviceVer->hw_main_ver, (char *)GetIMMU2_HWVersion(), sizeof(pESSDeviceVer->hw_main_ver) - 1);
			pESSDeviceVer->hw_main_ver[sizeof(pESSDeviceVer->hw_main_ver) - 1] = '\0';		

			//IMSU-D version
			A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
			ZERO_POBJS(&blobCCUSett, 1);
			valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
			valGet.val.blobValue = (BYTE*)&blobCCUSett;
			err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					200, 200, &valGet, sizeof(PARAM_VAL_GET));
			if(err == ERRCODE_DAI_OK)
			{
				strncpy((char *)pESSDeviceVer->sw_lccu_ver, (char *)blobCCUSett.ccuSettings.szSWVer, sizeof(pESSDeviceVer->sw_lccu_ver) - 1);
				pESSDeviceVer->sw_lccu_ver[sizeof(pESSDeviceVer->sw_lccu_ver) - 1] = '\0';	
			}
			else
			{
				strcpy(pESSDeviceVer->sw_lccu_ver, "--");
			}
			strcpy(pESSDeviceVer->hw_lccu_ver, "--");

			reverseBytes((BYTE*)pESSDeviceVer, SIZE_108_CDATA);
			nSendDataLen = SIZE_HEAD + SIZE_108_CDATA;
		}
		break;

		default:
		return;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
}


static ALARM_INFO* GetCfgAlarm_ById(ROUGH_SLAVE_ROUGH* pSlave, int nAlarmId)
{
	ALARM_INFO* pAlarm = NULL;
 	int n = 0;
 	ALARM_INFO *pCfgAlarm = &pSlave->pAllAlarmInfo[0];
	if(pCfgAlarm == NULL)
	{
		return pAlarm;
	}
	for(n = 0; n < pSlave->nAlarmInfosTotal; n++, pCfgAlarm++)
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

BOOL CheckIfNeedShieldAlarms(int n, ROUGH_SLAVE_ROUGH* pSlave)
{
	ALARM_INFO *pCfgAlmInfo = NULL;
	int index = 0;

	pCfgAlmInfo = GetCfgAlarm_ById(pSlave,	pSlave->activeAlms[n].nAlarmId);
	if(pCfgAlmInfo == NULL)
	{
		TRACEE("IOCPP-Err: Get nDevId=[%d] nAlarmId=[%d] Failed\n", \
			pSlave->activeAlms[n].nDevId,pSlave->activeAlms[n].nAlarmId);
		return TRUE;
	}
	if (pCfgAlmInfo->emLevel == ALARM_LEVEL_OBSERVATION)  //OA level do not reporter
	{
		TRACEW("pSlave->activeAlms[n].nAlarmId[%d] == ALARM_LEVEL_OBSERVATION",pSlave->activeAlms[n].nAlarmId);
		return TRUE;
	}
	for (index = 0;index < pSlave->nShieldAlarmsNum;index++)
	{
		if (pSlave->activeAlms[n].nAlarmId == pSlave->ShieldAlarms[index])
		{
			TRACEW("pSlave->activeAlms[n].nAlarmId[%d] == pSlave->ShieldAlarms_id[%d]",pSlave->activeAlms[n].nAlarmId,pSlave->ShieldAlarms[index]);
			return TRUE;
		}
	}
	return FALSE;
}

BYTE sendDataBuff[TCP_BUFF_LEN] = { 0 };

void Send_SEMS_LDEV_C6_109(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_109 return is LC_TYPE_1st");
		return;
	}

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int nSendDataLen = 0;
	int n = 0;

	TRACEW("Send_SEMS_LDEV_C6_109 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			static const int SIZE_HEAD =    sizeof(SC6LDEV_CMDHead);
			static const int SIZE_109_DATA = sizeof(SC6_LDEV_109);
			  
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        109;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
		
			SC6_LDEV_109* pData = (SC6_LDEV_109*)&sendDataBuff[SIZE_HEAD];

			int nCount = 0;
			BOOL bRet = FALSE;
			for(n = 0; n < pSlave->nActiveAlmTotal; n++)
			{
				bRet = CheckIfNeedShieldAlarms(n, pSlave);
				if (bRet)
					continue;

				pData->nDevId = pSlave->activeAlms[n].nDevId;
				pData->nAlarmId = pSlave->activeAlms[n].nAlarmId;
				pData->status = pSlave->activeAlms[n].status;
				pData->fExParamVal = pSlave->activeAlms[n].fExParamVal;
				pData->tmStart = pSlave->activeAlms[n].tmStart;
				pData->tmEnd = pSlave->activeAlms[n].tmEnd;

				reverseBytes((BYTE*)pData, SIZE_109_DATA);
		
				++pData;
				++nCount;
			}

			pHead->byDevNum = nCount;
			pHead->usLenSingle =  nCount * SIZE_109_DATA;
			reverseBytes((BYTE*)pHead, SIZE_HEAD);

			nSendDataLen = SIZE_HEAD + pHead->usLenSingle;
		}
		break;

		default:
			break;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}

	return;
}

void Send_SEMS_LDEV_C6_110(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_110 return is LC_TYPE_1st");
		return;
	}

	if(pSlave->nCurrBMS >= MAX_SINGLE_STBATT_NUM)
	{
		TRACEW("Send_SEMS_LDEV_C6_110 pSlave->nCurrBMS  = %d error!", pSlave->nCurrBMS);
		return;
	}
	
	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int nSendDataLen = 0;
	int n = 0;

	TRACEW("Send_SEMS_LDEV_C6_110 typeLC:%d", pSlave->lcCfg.lcType);
	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			int iSizeHead = sizeof(SC6LDEV_CMDHead);
			int iSizeLDEV110 = sizeof(SC6_LDEV_110);
			int iSizeCell = sizeof(SC6_LDEV_CELL_110);
			int iDataLen = pSlave->nCfgBMSNum * (iSizeLDEV110 + pSlave->iBattCellNumPerGroup * sizeof(SC6_LDEV_CELL_110));

			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)sendDataBuff;
			pHead->usCmd =        110;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->byDevNum = pSlave->nCfgBMSNum;
			pHead->usLenSingle = iDataLen;
			

			if (iSizeHead + pHead->usLenSingle > sizeof(sendDataBuff))
			{
				TRACEW("length of data C6_110 to be send too long.\n");
				return;
			}
			
			reverseBytes((BYTE*)pHead, iSizeHead);

			char *pSendPos = &sendDataBuff[iSizeHead];
			int i = 0, n = 0;
			for(i = 0; i < pSlave->nCfgBMSNum; ++i)
			{
				BYTE *pSendPos = &sendDataBuff[iSizeHead] + i * (sizeof(SC6_LDEV_110) + pSlave->iBattCellNumPerGroup * sizeof(SC6_LDEV_CELL_110));

				SC6_LDEV_110* pSC6_LDEV_110 = (SC6_LDEV_110*)pSendPos;
				pSC6_LDEV_110->batt_idx = (BYTE)i;
				pSC6_LDEV_110->iCellNum = pSlave->iBattCellNumPerGroup;

				reverseBytes((BYTE*)pSC6_LDEV_110, iSizeLDEV110);

				A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
				ZERO_POBJS(&blobIfySiglBat, 1);
				blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
				valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
				valGet.val.blobValue = (BYTE*)&blobIfySiglBat;
				pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501 + i, 557, &valGet, sizeof(PARAM_VAL_GET));

				pSendPos += iSizeLDEV110;
				SC6_LDEV_CELL_110* pCellData = (SC6_LDEV_CELL_110*)pSendPos;

				for(n = 0; n < pSlave->iBattCellNumPerGroup; n++)
				{
					pCellData->cell_safe = blobIfySiglBat.byCellSafeStat[n];
					pCellData->cell_volt = blobIfySiglBat.usCellVolt[n];
					pCellData->cell_temp = blobIfySiglBat.cCellTemp[n];
					pCellData->cell_balance = blobIfySiglBat.byCellBalanceStat[n];

					reverseBytes((BYTE*)pCellData, iSizeCell);

					++pCellData;
				}
			}

			nSendDataLen = iSizeHead + iDataLen;
			
		}
		break;

		default:
			break;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}

	return;
}

void Send_SEMS_LDEV_C6_202(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		TRACEW("Send_SEMS_LDEV_C6_202 return is LC_TYPE_1st");
		return;
	}

	TRACEW("Send_SEMS_LDEV_C6_202 typeLC:%d", pSlave->lcCfg.lcType);

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int nSendDataLen = 0;
	// int n = 0;

	A_BLOB_ES_STATISTIC_DATA	blobStatData;
	ZERO_POBJS(&blobStatData ,	1);
	blobStatData.emBlobType = BLOB_TYPE_ES_STATISTICDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
	valGet.val.blobValue = (BYTE*)&blobStatData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 79, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q202 Failed err=[%d]!\n", err);
		return;
	}

	switch (pSlave->lcCfg.lcType)
	{
		// LC_TYPE_1st 一级主控上报到云平台
		case LC_TYPE_2nd:
		{
			// 一级主控
		}
		break;

		case LC_TYPE_3rd:
		{
			// 三级主控
		}
		break;

		case LC_TYPE_4cab:
		{
			// 储能柜
			int iSizeHead = sizeof(SC6LDEV_CMDHead);
			int iSizeLDEV202 = sizeof(SC6_LDEV_202);
			
			// 1.SC6LDEV_CMDHead
			SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
			pHead->usCmd =        202;
			pHead->idxClient =    pSlave->lcCfg.idxClientLC;
			pHead->byDevNum = 1;
			pHead->usLenSingle = iSizeLDEV202;

			if (iSizeHead + pHead->usLenSingle > sizeof(sendDataBuff))
			{
				TRACEW("length of data C6_202 to be send too long.\n");
				return;
			}

			reverseBytes((BYTE*)pHead, iSizeHead);

			int iDateSize = sizeof(float) * MAX_HOURS_SECTION_202;
			SC6_LDEV_202* pData = (SC6_LDEV_202*)&sendDataBuff[iSizeHead];

			pData->iBattGroupNum = pSlave->nCfgBMSNum;
			pData->data_from = 0;
			memcpy(pData->rmt_ac_realtime_kwh, blobStatData.es_Data.hours_Data.fAC_CurrKWH_Reading, iDateSize);
			memcpy(pData->rmt_ac_cost_kwh, blobStatData.es_Data.hours_Data.fAC_CostKWH_Sector, iDateSize);
			memcpy(pData->org_acload_cost_kwh, blobStatData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector, iDateSize);
			memcpy(pData->rmt_ac_revs_kwh, blobStatData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector, iDateSize);
			memcpy(pData->local_ac_input_kwh, blobStatData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector, iDateSize);
			memcpy(pData->local_ac_output_kwh, blobStatData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector, iDateSize);
			memcpy(pData->dcload_cost_kwh, blobStatData.es_Data.hours_Data.fDCLoad_CostKWH_Sector, iDateSize);

			memcpy(pData->value_mppt_output, blobStatData.es_Data.hours_Data.fMppt_GKWH_Sector, iDateSize * MAX_BATTERY_NUM);
			memcpy(pData->value_batt_chrg, blobStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector, iDateSize * MAX_BATTERY_NUM);
			memcpy(pData->value_batt_dischrg, blobStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector, iDateSize * MAX_BATTERY_NUM);

			reverseBytes((BYTE*)pData, iSizeLDEV202);
	
			nSendDataLen = iSizeHead + iSizeLDEV202;
		}
		break;

		default:
			break;
	}

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}

	return;
}

int Handler_SEMS_CFGG(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	UNUSED(pData);
	UNUSED(usLenID);
	// reverseBytes(pData, usLenID);
	// SEMS_61_CFGG_DATA_INFO* pCFGGServer = (SEMS_61_CFGG_DATA_INFO*)pData;
	// reverseBytes((BYTE*)pCFGGServer->szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	// if (ElapseMSBcdTimeStmp(&pCFGGServer->tmStamp) >= MAX_MS_DIFF_4CALIBRATETIME)
	// {
	// 	TRACEW("Parse_CFGG_SEMS Send_6202CFGS_RTC >= %d", MAX_MS_DIFF_4CALIBRATETIME);
	// 	Send_6202CFGS_RTC(n);
	// }
	// pCCU->byVerSEMSProt_Client = pCFGGServer->byVerSEMSProt; // LJH250630 SEMSProtVer
	// // CCU_BOOTUP_INFO* pBoot = &pCCU->blobBackendCfg blobChargerSpecificSett[n].ccuSettings;
	// // pCCU->blobChargerSpecificSett[nDispIdx]
	// s_MMClients[n].idxClientRecv = pCFGGServer->idxClient;
	// s_MMClients[n].emClient_RegisteredStatus = CCU_REGISTERED_OK;
	// s_MMClients[n].bCommFailSemsClient = FALSE;
	// s_MMClients[n].tmLastRcvHeatbeat = _NOW_;
	// pCCU->emCCUState = CCU_STATE_IDLE;


	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err = 0;

	TRACEW("Enter Parse_SEMS_CFGG--61 --------------------------");
	SEMS_61_CFGG_DATA_INFO stCFGGDataInfo;
	ZERO_POBJS(&stCFGGDataInfo, 1);
	//DataLen
	USHORT nSendDataLen = sizeof(SEMS_61_CFGG_DATA_INFO);
	stCFGGDataInfo.byVerSEMSProt = SEMS_PROTOCAL_VER; // LJH250630 SEMSProtVer
	stCFGGDataInfo.nSlaveIP = (UINT)pSlave->blobNetWorkInfo.netInfo_eht0.ulIpV4_IP;
	stCFGGDataInfo.idxClient = pSlave->lcCfg.idxClientLC;
	memcpy(stCFGGDataInfo.szIMMU2_SWVer, pSlave->szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	stCFGGDataInfo.tmStamp = GetCurrentBcdTimeStmp();
	
	stCFGGDataInfo.SEMS_LSYS_Period = pSlave->Sent_SEMS_LSYS_Period;
	valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3306, &valGet, sizeof(PARAM_VAL_GET));
	stCFGGDataInfo.nStopSOC_ChrgSet = (BYTE)((err==ERRCODE_DAI_OK) ? valGet.val.ulValue : 100);

	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3309, &valGet, sizeof(PARAM_VAL_GET));
	stCFGGDataInfo.nStopSOC_DiscSet = (BYTE)((err==ERRCODE_DAI_OK) ? valGet.val.ulValue : 0);
	stCFGGDataInfo.bSendCmdByLC1stOnly = pSlave->lcCfg.bSendCmdByLC1stOnly;
	TRACEI("Handler_SEMS_CFGG nSendDataLen = [%d] nSlaveIP:%04X idxClient:%u szIMMU2_SWVer:%s "
		"SEMS_LSYS_Period = [%d] StopSOC:%d-%d bSendCmdByLC1stOnly:%d",
		nSendDataLen, stCFGGDataInfo.nSlaveIP, stCFGGDataInfo.idxClient, stCFGGDataInfo.szIMMU2_SWVer,
		stCFGGDataInfo.SEMS_LSYS_Period, stCFGGDataInfo.nStopSOC_ChrgSet, stCFGGDataInfo.nStopSOC_DiscSet, stCFGGDataInfo.bSendCmdByLC1stOnly);

	reverseBytes((BYTE *)stCFGGDataInfo.szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	reverseBytes((BYTE *)&stCFGGDataInfo, nSendDataLen);
	if(Slave_WriteData(pSlave, (BYTE *)&stCFGGDataInfo, NLMSG_SEMS_CFGG_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write CFGG Data To Station EMS Abnormal! \n");
	}
	return 0;
}

static void SetDateTime(time_t* pTimeRet)
{
	char szTime[64];
	char szCommand[128];

	char szTime_Old[64];
	TimeToString(_NOW_, TIME_CHN_FMT, 
		szTime_Old, sizeof(szTime_Old));	

	TimeToString(*pTimeRet, TIME_CHN_FMT, 
		szTime, sizeof(szTime));

	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
			"Adj Sys Time[%s]->[%s]!", szTime_Old, szTime);

	sprintf(szCommand, "date -s \"%s\"", szTime);
	TRACE("\n***UpdateTime: [%s]\n", szCommand);
#ifdef _DEBUG
	system(szCommand);
	system("hwclock -w;/etc/init.d/save-rtc.sh;sync"); //added by Jimmy 20200910 for RTC saving problem
#else
	_SYSTEM(szCommand);
	_SYSTEM("hwclock -w;/etc/init.d/save-rtc.sh;sync &"); //added by Jimmy 20200910 for RTC saving problem
#endif
	stime(pTimeRet);		//2.Update OS time
}
// CFGS:62-XX
int Handler_SEMS_CFGS(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	UNUSED(usLenID);
	REPORTER* pReporter = pSlave->pReporter;
	USHORT nSendDataLen = 0;
	ERRCODE_DAI nErrCode;
	VAR_VALUE varV;
	UINT nRevSEMS_LSYS_Period;

	pSlave->bHadRecvCFGS = TRUE;
	BYTE sendDataBuff[TCP_BUFF_LEN - 9];
	memset(sendDataBuff, 0, TCP_BUFF_LEN - 9) ;

	sendDataBuff[nSendDataLen] = pData[0];
	nSendDataLen++;

	BYTE bySubCmd = pData[0];
	switch (bySubCmd)
	{
	case 0x02: //RTC Time
	{
		if (pSlave->lcCfg.idxClientLC != 0)
		{
			reverseBytes(&pData[1],9);
			time_t tRecvtime = BcdTimeStmp2time_t((BcdTimeStmp*)(&pData[1]));
			
			sendDataBuff[nSendDataLen] = 0x00;
			TRACEI("Receive SEMS Set time success !!");

			//if (ABS(tRecvtime - _NOW_) > 1) //sync RTC
			//if (ElapseMSBcdTimeStmp(&bcdTime) > 1000)
			{
				TRACEI("Now to set time .......");
				SetDateTime((time_t*)&tRecvtime);
				// VAR_VALUE varV;
				// varV.dtValue = tRecvtime;
				// nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0); 
				// if (nErrCode != ERRCODE_DAI_OK) //No Error
				// {
				// 	TRACEE("ERROR: Set RTC time Fail ! ");
				// 	sendDataBuff[nSendDataLen] = 0x01;
				// }
			}
		}
	}
		break;

	case 0x04: //设置SEMS_LSYS上报周期
	{
		memcpy(&nRevSEMS_LSYS_Period, reverseBytes(&pData[1], sizeof(UINT)), sizeof(UINT));
		// nRevSEMS_LSYS_Period = (UINT)((pData[1] << 24) | (pData[2] << 16) | (pData[3] << 8) | pData[4]);
		if (nRevSEMS_LSYS_Period > 0 && nRevSEMS_LSYS_Period < 999999)
		{			
			varV.emValue = nRevSEMS_LSYS_Period;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 84, &varV, 4, 1000);
			if (nErrCode == ERRCODE_DAI_OK) //No Error
			{
				pSlave->Sent_SEMS_LSYS_Period = nRevSEMS_LSYS_Period;
				TRACEI("Receive SEMS Set Sent_SEMS_LSYS_Period success = [%d]",pSlave->Sent_SEMS_LSYS_Period);
				sendDataBuff[nSendDataLen] = 0x00;
			}
			else	//Executed Error
			{
				TRACEE("ERROR: Set Sent_SEMS_LSYS_Period Fail ! ");
				sendDataBuff[nSendDataLen] = 0x01;
			}
		}
		else
		{
			//Data invalid
			sendDataBuff[nSendDataLen] = 0x04;
		}
	}
		break;

	case 0x05: // LJH251106 同步主从参数
	{
		ERRCODE_DAI nErrCode2;
		varV.ulValue = pData[1];
		nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1001, 3306, &varV, 4, 1000);
		varV.ulValue = pData[2];
		nErrCode2 = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1001, 3309, &varV, 4, 1000);
		TRACED("Handler_SEMS_CFGS nErrCode:%d nErrCode2:%d pData[1]:%d pData[2]:%d ", nErrCode, nErrCode2, pData[1], pData[2]);
		if (nErrCode == ERRCODE_DAI_OK && nErrCode2 == ERRCODE_DAI_OK)
		{
			TRACEI("Handler_SEMS_CFGS 61-05 CFGS:StopSOC success");
			sendDataBuff[nSendDataLen] = 0x00;
		}
		else
		{
			TRACEE("Handler_SEMS_CFGS 61-05 CFGS:StopSOC Fail nErrCode:%d nErrCode2:%d", nErrCode, nErrCode2);
			sendDataBuff[nSendDataLen] = 0x01;
		}
		sendDataBuff[nSendDataLen] = 0x00;
	}
		break;

	case 0x06:
	{
		varV.emValue = pData[1];
		nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1001, 3313, &varV, 4, 1000);
		if (nErrCode == ERRCODE_DAI_OK) //No Error
		{
			TRACEI("Receive SEMS Set bSendCmdByLC1stOnly success");
			sendDataBuff[nSendDataLen] = 0x00;
		}
		else	//Executed Error
		{
			TRACEE("ERROR: Set bSendCmdByLC1stOnly Fail ! ");
			sendDataBuff[nSendDataLen] = 0x01;
		}
	}
		break;

	default: //CMD invalid
	{
		sendDataBuff[nSendDataLen] = 0x02;
		TRACEE("now can not support this subcmd !!");
	}
		break;
	}
	
	nSendDataLen++;
	
	//nSendDataLen
	TRACEW(" nSendDataLen = [%d] ",nSendDataLen);

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_CFGS_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write CFGS Data To Station EMS Abnormal! \n");
	}
	return 0;
}

int Handler_SEMS_UCTL(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	// DebugTest(pSlave);
	// return 0;

	REPORTER* pReporter = pSlave->pReporter;
	USHORT nSendDataLen = 0;
	ERRCODE_DAI nErrCode;
	VAR_VALUE varV;
	Sems6330_CtrlWorkPwr_Response stSend6330;
	
	BYTE sendDataBuff[TCP_BUFF_LEN - 9];
	memset(sendDataBuff, 0, TCP_BUFF_LEN - 9) ;

	//subcmd
	BYTE bSubcmd = pData[0];

	reverseBytes(pData,usLenID);

	if (bSubcmd == 0x01)		//63-01并离网自动切换模式
	{
		TRACEW("Enter GridOnOffAutoSwitchMode--6301 --------------------------");
		S63Uctl_01GridOnOffAutoSwitchMode* stRev6301 =(S63Uctl_01GridOnOffAutoSwitchMode*)pData;
		if (stRev6301->idxClient == pSlave->lcCfg.idxClientLC)
		{
			TRACEI("stRev6301->GridOnOffAutoSwitchMode = [%d]",stRev6301->GridOnOffAutoSwitchMode);
			varV.emValue = (stRev6301->GridOnOffAutoSwitchMode == 0xA0) ? 0 : 1;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,  //0-auto   1-manual
				200, 218, &varV, 4, 1000);   
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffAutoSwitchMode successfully !!");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffAutoSwitchMode Fail errcode[%d]!!",nErrCode);
			}
			
		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if (bSubcmd == 0x02)	//63-02并离网模式(reserve)
	{
		TRACEW("Enter GridOnOffMode--6302 --------------------------");
		S63Uctl_02GridOnOffMode* stRev6302 =(S63Uctl_02GridOnOffMode*)pData;
		if (stRev6302->idxClient == pSlave->lcCfg.idxClientLC)
		{
			TRACEI("stRev6302->GridOnOffMode = [%d]",stRev6302->GridOnOffMode);
			varV.emValue = (stRev6302->GridOnOffMode == 0xA1) ? 0 : 1;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
				200, 219, &varV, 4, 1000);   
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffMode successfully !!");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  GridOnOffMode Fail errcode[%d]!!",nErrCode);
			}
		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if (bSubcmd == 0x14)	//63-14手动控制充放电功率
	{
		TRACEW("Enter ManualChargeDischargePower--6314 --------------------------");
		S63Uctl_14ManualChargeDischargePower* stRev6314 =(S63Uctl_14ManualChargeDischargePower*)pData;
		if (stRev6314->idxClient == pSlave->lcCfg.idxClientLC)
		{
			TRACEI("stRev6314->emACDC_WorkStatu = [%d]",stRev6314->emACDC_WorkStatu);
			TRACEI("stRev6314->StopSOC = [%d]",stRev6314->nStopSoc);
			TRACEI("stRev6314->fActv_Pwr = [%0.1f]",stRev6314->fActv_Pwr);
			TRACEI("stRev6314->fVolt = [%0.1f]",stRev6314->fVolt);
			TRACEI("stRev6314->fCurrent = [%0.1f]",stRev6314->fCurrent);
			TRACEI("stRev6314->usReActv_Pwr = [%d]",stRev6314->usReActv_Pwr);
			TRACEI("stRev6314->nEnergySchedulingMode = [%d]",stRev6314->nEnergySchedulingMode);

			A_BLOB_MANUALCTRL_RECT_CMD blobRev6314;
			ZERO_POBJS(&blobRev6314, 1);

			blobRev6314.emBlobType = BLOB_TYPE_MANUALCTRL_RECT;
			blobRev6314.emState = (STATE_MANUL_CTRL)stRev6314->emACDC_WorkStatu;   //枚举修改 pending 
			blobRev6314.nGrp = 0x00;//液冷一体柜  默认0x00；All Group  
			blobRev6314.fGrpVolt = stRev6314->fVolt;
			blobRev6314.fGrpCurr = stRev6314->fCurrent;
			blobRev6314.nStopSOC = (UINT)stRev6314->nStopSoc;
			blobRev6314.nAdjMode = (UINT)stRev6314->nEnergySchedulingMode;
			blobRev6314.fActv_Pwr = stRev6314->fActv_Pwr;
			blobRev6314.fReactv_Pwr = (float)stRev6314->usReActv_Pwr;

			varV.blobValue = (BYTE *)&blobRev6314;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW, 
					200, 214, &varV, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), 1000);
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEI("pDAI->Set A_BLOB_MANUALCTRL_RECT_CMD Set DAITYPE_CTRL_PARAMVALUE_RW Sucessfully\n");
				stSend6330.RTN = 0x00;
			}
			else
			{
				TRACEE("pDAI->Set A_BLOB_MANUALCTRL_RECT_CMD Set DAITYPE_CTRL_PARAMVALUE_RW Fail err[%d]!\n",nErrCode);
				stSend6330.RTN = 0x01;
			}
		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if (bSubcmd == 0x17)	// 63-17手动模式
	{
		TRACEW("Enter ManualMode--6317 --------------------------");
		S63Uctl_17ManualMode* stRev6317 =(S63Uctl_17ManualMode*)pData;
		if (stRev6317->idxClient == pSlave->lcCfg.idxClientLC)
		{
			TRACEI("stRev6317->ManualMode = [%d]",stRev6317->ManualMode);
			varV.emValue = (stRev6317->ManualMode == 0xA0) ? 0 : 1;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
				200, 217, &varV, 4, 1000);   
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  ManualMode successfully !!");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  ManualMode Fail errcode[%d]!!",nErrCode);
			}

		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if (bSubcmd == 0x30)	// 6330:自动控制充放电功率
	{
		S63Uctl_30WorkPwr* stRev6330 =(S63Uctl_30WorkPwr*)pData;
		TRACEI("Handler_SEMS_UCTL 6330 idxClient:%u bProtocol:%d emACDC_WorkStatuCtrl = [%d] fActv_PwrCtrl = [%0.1f]",
			stRev6330->idxClient, bProtocol, stRev6330->emACDC_WorkStatuCtrl, stRev6330->fActv_PwrCtrl);
		// if (bProtocol == PROTOCOL_UDP)
		// {
		// 	LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
		// 		"Client_UDP Received fActv_PwrCtrl[%.1f]",stRev6330->fActv_PwrCtrl);
		// }
		A_BLOB_SEMS2SLAVE_6330 blobData;
		blobData.emBlobType = BLOB_TYPE_SEMS2SlAVE_6330;
		blobData.stRev6330 = *stRev6330;
		varV.blobValue = (BYTE *)&blobData;
		nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
			200, 206, &varV, sizeof(A_BLOB_SEMS2SLAVE_6330), 1000);
		if (nErrCode == ERRCODE_DAI_OK)
		{
			stSend6330.RTN = 0x00;
		}
		else
		{
			TRACEE("Handler_SEMS_UCTL pDAI->Set A_BLOB_SEMS2SLAVE_6330 Set DAITYPE_VIRTUAL_PARAMVALUE_W Fail err[%d]!\n",nErrCode);
			stSend6330.RTN = 0x01;
		}
	}
	else if (bSubcmd == 0x60) // LJH250930 加快自动充放响应 //6360:热待机模式
	{
		S6360_Uctl_Standby* stRev6360 =(S6360_Uctl_Standby*)pData;
		TRACED("stRev6360->idxClient = [%d], pSlave->lcCfg.idxClientLC = [%d] bStandby = [%d]",
			stRev6360->idxClient, pSlave->lcCfg.idxClientLC, stRev6360->bStandby_BLL);
		if (stRev6360->idxClient == pSlave->lcCfg.idxClientLC)
		{
			varV.emValue = (_ENUM)stRev6360->bStandby_BLL;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1001, 3800, &varV, 4, 100);
			// nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			// 	1, 85, &varV, 4, 1000); // 0:enable  1:disable
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACED("pDAI->Set HotStandbyMode successfully !!");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEE("pDAI->Set HotStandbyMode Fail errcode[%d]!!",nErrCode);
			}
		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if (bSubcmd == 0x90)	// 6390:启用/禁用系统
	{
		TRACEW("Enter DisableEnableSys--6390 --------------------------");
		S63Uctl_90DisEnableSys* stRev6390 =(S63Uctl_90DisEnableSys*)pData;
		if (stRev6390->idxClient == pSlave->lcCfg.idxClientLC)
		{
			TRACEI("stRev6390->DisEnableSYS = [%d]",stRev6390->DisEnableSYS);

			varV.emValue = (stRev6390->DisEnableSYS == 0x01) ? 0 : 1;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
				200, 198, &varV, 4, 1000);   
			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEI("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  DisEnableSYS successfully !!");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEE("pDAI->Set  DAITYPE_SETT_PARAMVALUE_RW  DisEnableSYS Fail errcode[%d]!!",nErrCode);
			}

		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else if(bSubcmd == 0x91)
	{
		TRACEW("Enter-------6391 --------------------------");
		S63Uctl_EmsUpgradeCmd *UpgradeCmd = (S63Uctl_EmsUpgradeCmd*)pData;

		TRACEW("idxClient = %d, idxClientLC = %d\n", \
		UpgradeCmd->idxClient, pSlave->lcCfg.idxClientLC);

		if (UpgradeCmd->idxClient == pSlave->lcCfg.idxClientLC)
		{
			A_BLOB_UPDATEFIRMWARE_CMD cmd;
			ZERO_POBJS(&cmd, 1);
			cmd.emBlobType = BLOB_TYPE_UPDATE_FIRMWARE_CMD;
			size_t url_len = strlen(UpgradeCmd->szURL);
			if(url_len > 255)
			{
				printf("Err: URL too long !!\n");
				url_len = 255;
			}
			memcpy(cmd.szURL, UpgradeCmd->szURL, url_len);
			cmd.szURL[url_len] = '\0';
			cmd.dtRetrieveDate = UpgradeCmd->dtRetrieveDate;

			varV.blobValue = &cmd;
			nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 8, &varV, sizeof(A_BLOB_UPDATEFIRMWARE_CMD), 0);	

			if (nErrCode == ERRCODE_DAI_OK)
			{
				TRACEW("pDAI->Set  DAITYPE_STRL_PARAMVALUE_RW  Upgrade successfully !!\n");
				stSend6330.RTN = 0x00;
			}
			else
			{
				stSend6330.RTN = 0x01;
				TRACEW("pDAI->Set  DAITYPE_STRL_PARAMVALUE_RW  Upgrade Fail errcode[%d]!!\n",nErrCode);
			}
		}
		else
		{
			stSend6330.RTN = 0x01;
			TRACEE("rev idxClient no match !!");
		}
	}
	else
	{
		stSend6330.RTN = 0x01;
		TRACEE("now not support this subcmd !!");
	}

	stSend6330.subcmd = bSubcmd;
	nSendDataLen = (USHORT)(nSendDataLen + sizeof(Sems6330_CtrlWorkPwr_Response));
	memcpy(sendDataBuff, &stSend6330, nSendDataLen);
	reverseBytes(sendDataBuff,nSendDataLen);
	
	//nSendDataLen
	TRACEW(" nSendDataLen = [%d] ",nSendDataLen);

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_UCTL_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err:Write UCTL Data To Station EMS Abnormal! \n");
	}
	return 0;
}

int Handler_SEMS_RTIF(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	UNUSED(pSlave);
	UNUSED(pData);
	UNUSED(usLenID);
	UNUSED(bProtocol);

	TRACEW("Enter Parse_SEMS_RTIF--64 --------------------------");
	return 0;
}

int Handler_PCS_INFO(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	// printf("Enter Handler_PCS_INFO--65 --------------------------\n");
	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;

	int nSendDataLen = 0;
	int iSizeHead = sizeof(PCS_INFO_DATA);
	
	PCS_INFO_DATA* pHead = (PCS_INFO_DATA*)(&sendDataBuff[0]);
	pHead->subCmd =    0x65;
	pHead->idxClient =   pSlave->lcCfg.idxClientLC;

	A_BLOB_SINGLE_RECT_INFO allsingleRectData;
	ZERO_POBJS(&allsingleRectData, 	1);
	allsingleRectData.emBlobType = BLOB_TYPE_SINGLE_RECT_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
	valGet.val.blobValue = (BYTE*)&allsingleRectData;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 101, &valGet, sizeof(PARAM_VAL_GET));
    STU_SIGNLE_RECT_DATA singleRectData;
	int PCS_Num = (int)allsingleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; //法国客户目前只有一组PCS

	pHead->nPCSNum = PCS_Num;

	int n;
	for(n = 0; n < PCS_Num; n++)
	{
		if(n >= MAX_PSC_NUM)
		{
			TRACEE("Err: PCS_Num[%d] exceed MAX_PCS_NUM[%d]!");
			break;
		}
		singleRectData = allsingleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n];

		char szTemp[MAX_BUFFER_LNE] = "\0";
		snprintf(szTemp, MAX_BUFFER_LNE, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
		memcpy(pHead->szVersion[n], szTemp, MAX_BUFFER_LNE);
		pHead->szVersion[n][MAX_BUFFER_LNE - 1] = '\0';

		snprintf(szTemp, MAX_BUFFER_LNE, "%s", singleRectData.byBarCode);
		memcpy(pHead->szBar_code[n], szTemp, MAX_BUFFER_LNE);
		pHead->szBar_code[n][MAX_BUFFER_LNE - 1] = '\0';

	}

	reverseBytes((BYTE*)pHead, iSizeHead);

	nSendDataLen = iSizeHead ;

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_PCS_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		printf("Err:Write PCS INFO Data To Station EMS Abnormal! \n");
	}
	return 0;
}

int Parse_SEMS_USYS(ROUGH_SLAVE_ROUGH* pSlave, BYTE* pData, USHORT usLenID, PROTOCOL_TYPE bProtocol)
{
	UNUSED(bProtocol);
	REPORTER* pReporter = pSlave->pReporter;
	ERRCODE_DAI nErrCode;
	VAR_VALUE varV;
	
	reverseBytes(pData,usLenID);
	SEMS_A6_USYS_DATA_INFO * stRevUSYS = (SEMS_A6_USYS_DATA_INFO *)pData;
	// LJH250612 planSector
	SEMS_A6_USYS_DATA_INFO* pA6_USYS_Last = &pSlave->A6_USYS_Last;
	TRACED("Parse_SEMS_USYS idxClient = [%u] tmStamp=%s emUSYSStationEMSAvaiStatus = [%d] emCtrlRectMode = [%d] "
		"emAutoManuSwitchMode = [%d] emOnOffGridMode = [%d] "
		"emBatt_WorkModePlan:%d nStopSOCPlan:%d nActvPwrLmtPlan:%u nReActvPwrLmtPlan:%d nBattGrpPwrLmtPlan:%u Last:%u "
		"offset:%d memcmp:%d",
		stRevUSYS->idxClient, printableBcdTimeStmp(&stRevUSYS->tmStamp), stRevUSYS->emUSYSStationEMSAvaiStatus, stRevUSYS->emCtrlRectMode,
		stRevUSYS->emAutoManuSwitchMode, stRevUSYS->emOnOffGridMode, 
		stRevUSYS->emBatt_WorkModePlan, stRevUSYS->nStopSOCPlan, stRevUSYS->nActvPwrLmtPlan, stRevUSYS->nReActvPwrLmtPlan, stRevUSYS->nBattGrpPwrLmtPlan, pA6_USYS_Last->nBattGrpPwrLmtPlan,
		offsetof(SEMS_A6_USYS_DATA_INFO, tmStamp),
		memcmp(stRevUSYS, pA6_USYS_Last, offsetof(SEMS_A6_USYS_DATA_INFO, tmStamp)));
#ifdef LOG_FILEDEBUG
	static time_t stmLogFile = 0;
    if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE)
    {
        stmLogFile = _NOW_;
		LOGFD("Parse_SEMS_USYS idxClient:%u emCCUState:%d emCtrlRectMode:%d "
			"emBatt_WorkModePlan:%d nStopSOCPlan:%d nActvPwrLmtPlan:%u "
			"nReActvPwrLmtPlan:%d nBattGrpPwrLmtPlan:%u Last:%u",
			stRevUSYS->idxClient, stRevUSYS->emUSYSStationEMSAvaiStatus, stRevUSYS->emCtrlRectMode,
			stRevUSYS->emBatt_WorkModePlan, stRevUSYS->nStopSOCPlan, stRevUSYS->nActvPwrLmtPlan,
			stRevUSYS->nReActvPwrLmtPlan, stRevUSYS->nBattGrpPwrLmtPlan, pA6_USYS_Last->nBattGrpPwrLmtPlan);
	}
#endif
	if (memcmp(stRevUSYS, pA6_USYS_Last, offsetof(SEMS_A6_USYS_DATA_INFO, tmStamp)) != 0 )
	{
		A_BLOB_SEMS_A6_USYS blobData;
		blobData.emBlobType = BLOB_TYPE_SEMS_A6_USYS;
		blobData.A6_USYS = *stRevUSYS;
		varV.blobValue = (BYTE *)&blobData;
		nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W,
					200, 207, &varV, sizeof(A_BLOB_SEMS_A6_USYS), 1000);
		TRACEI("Parse_SEMS_USYS pDAI->Set 200, 207, A_BLOB_SEMS2SLAVE_6330 nErrCode:%d nBattGrpPwrLmtPlan:%u",
			nErrCode, blobData.A6_USYS.nBattGrpPwrLmtPlan);
		LOGFD("Parse_SEMS_USYS pDAI->Set 200, 207, A_BLOB_SEMS2SLAVE_6330 nErrCode:%d "
			"idxClient:%u emCCUState:%d emCtrlRectMode:%d "
			"emBatt_WorkModePlan:%d nStopSOCPlan:%d nActvPwrLmtPlan:%u "
			"nReActvPwrLmtPlan:%d nBattGrpPwrLmtPlan:%u Last BattGrpPwrLmt:%u",
			nErrCode,
			stRevUSYS->idxClient, stRevUSYS->emUSYSStationEMSAvaiStatus, stRevUSYS->emCtrlRectMode,
			stRevUSYS->emBatt_WorkModePlan, stRevUSYS->nStopSOCPlan, stRevUSYS->nActvPwrLmtPlan,
			stRevUSYS->nReActvPwrLmtPlan, stRevUSYS->nBattGrpPwrLmtPlan, pA6_USYS_Last->nBattGrpPwrLmtPlan);
		if (nErrCode == ERRCODE_DAI_OK)
		{
			*pA6_USYS_Last = *stRevUSYS;
		}
	}
	
	// valGet.nGetSize = 4;
	//上锁    上锁且有故障呢？
	// if (stRevUSYS->emUSYSStationEMSAvaiStatus == 1)
	// {
	// 	nTemp = 0;
	// }
	// else 
	// {
	// 	nTemp = 1;
	// }

	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,  //0-lock  1-normal
	// 	200, 198, &valGet, sizeof(PARAM_VAL_GET));
	// if (valGet.val.emValue != nTemp)
	// {
	// 	varV.emValue = nTemp;
	// 	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
	// 			200, 218, &varV, 4, 1000); 
	// }

	// LJH250220 Modify会反复出现故障、正常
	//sysstatus  状态同步
	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,//从机系统运行状态
	// 	1, 4, &valGet, sizeof(PARAM_VAL_GET));
	// if (valGet.val.emValue != stRevUSYS->emUSYSStationEMSAvaiStatus)
	// {
	// 	varV.emValue = stRevUSYS->emUSYSStationEMSAvaiStatus;
	// 	nErrCode = 	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
	// 	1, 4, &varV, 4, 100);
	// }

	// 63-17手动模式 LJH250220:先不同步
	// nTemp = (stRevUSYS->emCtrlRectMode == 0xA0) ? 0 : 1;
	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
	// 	200, 217, &valGet, sizeof(PARAM_VAL_GET));
	// if (valGet.val.emValue != nTemp)
	// {
	// 	varV.emValue = nTemp;
	// 	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
	// 			200, 217, &varV, 4, 1000); 
	// }
	
	//63-01并离网自动切换模式 LJH250220:先不同步
	// nTemp = (stRevUSYS->emAutoManuSwitchMode == 0xA0) ? 0 : 1;
	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
	// 	200, 218, &valGet, sizeof(PARAM_VAL_GET));
	// if (valGet.val.emValue != nTemp)
	// {
	// 	varV.emValue = nTemp;
	// 	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
	// 			200, 218, &varV, 4, 1000); 
	// }

	//63-02并离网模式(reserve 暂时不同步)
	// nTemp = (stRevUSYS->emOnOffGridMode == 0xA1) ? 0 : 1;
	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
	// 	200, 219, &valGet, sizeof(PARAM_VAL_GET));
	// if (valGet.val.emValue != nTemp)
	// {
	// 	varV.emValue = nTemp;
	// 	nErrCode = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
	// 			200, 219, &varV, 4, 1000); 
	// }
	
	return 0;
}
// edit by Kuangyue 20251112: Cable Alarm Light Status report to Station EMS
void Parse_AlarmLight_Status(ROUGH_SLAVE_ROUGH* pSlave, PROTOCOL_TYPE bProtocol)
{
	if(pSlave->lcCfg.lcType == LC_TYPE_1st)
		return;

	REPORTER* pReporter = pSlave->pReporter;
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3321, &valGet, sizeof(PARAM_VAL_GET));
	int nValue = valGet.val.emValue;
	if(nValue == 0)
	{
		TRACEW("Send_AlarmLight_Status Disabled");
		return;
	}
    static const int targetAlarmIds[] = {739, 813, 814, 815, 816};
    static const int targetCount = sizeof(targetAlarmIds) / sizeof(targetAlarmIds[0]);

    BYTE sendDataBuff[TCP_BUFF_LEN] = {0};
	int i = 0, j = 0;
	int nCount = 0;
    int nSendDataLen = 0;
    TRACEW("Parse_AlarmLight_Status Start, active alarm total=%d", pSlave->nActiveAlmTotal);

	SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)&sendDataBuff[0];
	SC6_LDEV_109* pData = (SC6_LDEV_109*)&sendDataBuff[sizeof(SC6LDEV_CMDHead)];

	for (i = 0; i < pSlave->nActiveAlmTotal; i++)
	{
		int alarmId = pSlave->activeAlms[i].nAlarmId;
		for (j = 0; j < targetCount; j++)
		{
			if (alarmId == targetAlarmIds[j])
			{
				TRACED("Target Alarm Found! DevID=%d, AlarmID=%d",
					pSlave->activeAlms[i].nDevId, alarmId);
				pData->nDevId      = pSlave->activeAlms[i].nDevId;
				pData->nAlarmId    = pSlave->activeAlms[i].nAlarmId;
				pData->status      = pSlave->activeAlms[i].status;
				pData->fExParamVal = pSlave->activeAlms[i].fExParamVal;
				pData->tmStart     = pSlave->activeAlms[i].tmStart;
				pData->tmEnd       = pSlave->activeAlms[i].tmEnd;
				reverseBytes((BYTE*)pData, sizeof(SC6_LDEV_109));
				pData++;
				nCount++;

				break;
			}
		}
	}
	if (nCount == 0)
	{
		return;
	}
	pHead->usCmd       = 902;
	pHead->idxClient   = pSlave->lcCfg.idxClientLC;
	pHead->byDevNum    = nCount;
	pHead->usLenSingle = sizeof(SC6_LDEV_109);
	reverseBytes((BYTE*)pHead, sizeof(SC6LDEV_CMDHead));

    // Step 4: 发送
    nSendDataLen = sizeof(SC6LDEV_CMDHead) + nCount * sizeof(SC6_LDEV_109);

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		TRACEE("Err: Write Target Alarm Data to EMS failed!\n");
	}
}

int Parse_Upgrade_Status(ROUGH_SLAVE_ROUGH* pSlave, FIRM_UPDATESTT UpgradeSTATUS, PROTOCOL_TYPE bProtocol)
{
	if (pSlave->lcCfg.lcType == LC_TYPE_1st)
	{
		//printf("Send_SEMS_LDEV_C6_901 return is LC_TYPE_1st");
		return;
	}
	//printf("Send_SEMS_LDEV_C6_901 typeLC:%d\n", pSlave->lcCfg.lcType);

	int nSendDataLen = 0;
	int iSizeHead = sizeof(SC6LDEV_CMDHead);
	int iSize901 = sizeof(BYTE);
	
	SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)(&sendDataBuff[0]);
	pHead->usCmd =        901;
	pHead->idxClient =    pSlave->lcCfg.idxClientLC;
	pHead->byDevNum = 1;
	pHead->usLenSingle = iSize901;
	reverseBytes((BYTE*)pHead, iSizeHead);

	sendDataBuff[iSizeHead] = (BYTE)UpgradeSTATUS;
	//printf("slave UpgradeSTATUS = %d, value = %d\n", UpgradeSTATUS, sendDataBuff[iSizeHead]);

	nSendDataLen = iSizeHead + iSize901;	

	if(Slave_WriteData(pSlave, sendDataBuff, NLMSG_SEMS_LDEV_CMD, nSendDataLen, bProtocol) != nSendDataLen + 9)
	{
		printf("Err:Write LSYS Data To Station EMS Abnormal! \n");
	}
	return 0;
}