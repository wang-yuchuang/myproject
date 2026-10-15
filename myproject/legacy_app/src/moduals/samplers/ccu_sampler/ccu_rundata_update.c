// #define _TRACE_PRINT_ // SEMSRundata
#include "ccu_sampler_main.h"


void Update_SlvMeterValue(ROUGH_CCU_DATA* pCCU)
{
	UINT n = 0; 
	for( n = 0; n < MAX_SLVMETER_NUM; n++)
	{
		ERRCDDE_DPR err;
		VAR_VALUE varV;
		A_BLOB_METER_VALUES 	blobMeter;
		ZERO_POBJS(&blobMeter, 	1);

		varV.blobValue = (BYTE *)&blobMeter;
		err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 800 + n, 430, &varV, sizeof(A_BLOB_METER_VALUES));
		if(err != ERR_DPR_OK)
		{
			printf("Error: n=[%d] Get MeterValue Failed\n", n);
			continue;
		}
		pCCU->esRunData.f_PMeter[n] = blobMeter.fRealTime_ActivePower;
		pCCU->esRunData.f_QMeter[n] = blobMeter.fReactivePower;
		pCCU->esRunData.emDataFrom[n] = blobMeter.emDataFrom;

		if(blobMeter.szIP != NULL)
		{
			memcpy(pCCU->esRunData.szIP[n], blobMeter.szIP, 32);
		}

		if(blobMeter.tmRefresh != 0)
		{
			TimeToString(blobMeter.tmRefresh, "%H:%M:%S", pCCU->esRunData.szRefreshTM[n], 32);
		}

		// printf("P=[%.1f] Q=[%.1f] Vabc=[%.1f %.1f %.1f] Iabc=[%.1f %.1f %.1f] szIP=[%s] blobIP=[%s] TM=[%s]\n",
		// 	blobMeter.fRealTime_ActivePower, blobMeter.fReactivePower,
		// 	blobMeter.fPhA_Volt, blobMeter.fPhA_Curr,
		// 	blobMeter.fPhB_Volt, blobMeter.fPhB_Curr,
		// 	blobMeter.fPhC_Volt, blobMeter.fPhC_Curr,
		// 	pCCU->esRunData.szIP[n], blobMeter.szIP, pCCU->esRunData.szRefreshTM[n]);
	}
	return;
}

void Prepare_ESRunData(ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE 	varV;
	int 		n = 0;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

	//------------------ System Param -------------------------//
	pCCU->esRunData.emEPO = pCCU->strLSYSInfo.sysAlarmMap1.Alarm.EPO;
	pCCU->esRunData.emCCUState = pCCU->emCCUState;
	pCCU->esRunData.emSystemFailReason = pCCU->emSystemFailReason;
	pCCU->esRunData.mapSTSCode.byRunStatus = pCCU->strLSYSInfo.mapSTSCode.byRunStatus;
 	pCCU->esRunData.emCCUAutoOrManual = pCCU->strLSYSInfo.emRunningMode;
	pCCU->esRunData.emABUInstalled = pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed;

 	//---20230226.added For MeterValue Topology---//
	pCCU->esRunData.f_QgridMeter = pEMS->f_QgridMeter;
	pCCU->esRunData.f_QcabMeter = pEMS->f_QcabMeter;

	pCCU->esRunData.f_PgridMeter = pEMS->f_PgridMeter;

	float f_OffPcabMeter = 0, f_OffQcabMeter = 0;
	if((pCCU->emESSDevType == LIQUID_COOLING_BACKUP) && (pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == 1))
	{
		//WLD01206 2025/3/29 有几组电池就加多少功率  离网后直接加支路功率
		for(n = 0; n < pCCU->esRunData.nBattCfgNum; n++) 
		{
 			f_OffPcabMeter += pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp;
			pCCU->esRunData.f_PcabMeter = f_OffPcabMeter;

			f_OffQcabMeter += pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp;
			pCCU->esRunData.f_QcabMeter = f_OffQcabMeter;
		}
	} 
	else if((pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == 1)
		&& (fabs(pEMS->f_PcabMeter) < 0.1)) //John 20240124 Added. Due To HT Off-Grid Cabinet AC Meter Work is Normal.
	{
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
 			f_OffPcabMeter += pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp;
			pCCU->esRunData.f_PcabMeter = f_OffPcabMeter;

			f_OffQcabMeter += pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp;
			pCCU->esRunData.f_QcabMeter = f_OffQcabMeter;
		}
	}
	else
	{
		pCCU->esRunData.f_PcabMeter = pEMS->f_PcabMeter;
	}
	
	pCCU->esRunData.fLocalAC_Power = pEMS->f_PcabMeter;

	pCCU->esRunData.f_PauxMeter = pEMS->f_PauxMeter;
	pCCU->esRunData.fPacload4Chg = pEMS->fPacload4Chg;  
	pCCU->esRunData.fPacload4Dischg = pEMS->fPacload4Dischg;
	pCCU->esRunData.f_PauxMeter_3 = pEMS->f_PauxMeter_3;
	pCCU->esRunData.f_PmaxGridDemand = pEMS->f_PmaxGridDemand;

 	pCCU->esRunData.fAvg_Pacload4Dischg = pEMS->fAvg_Pacload4Dischg; 
	pCCU->esRunData.fTotal_PessChgAvaliable = pEMS->fTotal_PessChgAvaliable;
	pCCU->esRunData.fTotal_PessDischgLmt = pEMS->fTotal_PessDischgLmt;
	pCCU->esRunData.nTMSector = pEMS->nTMSector;
	pCCU->esRunData.emAuxPos = pCCU->emAuxMeterPos;


	pCCU->esRunData.mapDIStatus.Status.DI_PwrCabQF1 		= pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACGridInputMCBQF1;
	pCCU->esRunData.mapDIStatus.Status.DI_UserGridQF 		= pCCU->strLSYSInfo.sysAlarmMap1.Alarm.LowVoltMCBBreaker;
	pCCU->esRunData.mapDIStatus.Status.DI_PhaN_Contactor 	= pCCU->strLSYSInfo.sysAlarmMap1.Alarm.N_LineContactor;
	pCCU->esRunData.mapDIStatus.Status.DI_ACLoadOutputMCBQF2= pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACLoadOutputMCBQF2;
	pCCU->esRunData.mapDIStatus.Status.DI_PCSGrp1_Contactor = pCCU->strLSYSInfo.sysAlarmMap3.Alarm.BattGrp1_InputContactor;
	pCCU->esRunData.mapDIStatus.Status.DI_PCSGrp2_Contactor = pCCU->strLSYSInfo.sysAlarmMap3.Alarm.BattGrp2_InputContactor;
	pCCU->esRunData.mapDIStatus.Status.DI_PCSGrp3_Contactor = pCCU->strLSYSInfo.sysAlarmMap3.Alarm.BattGrp3_InputContactor;
	pCCU->esRunData.mapDIStatus.Status.DI_PCSGrp4_Contactor = pCCU->strLSYSInfo.sysAlarmMap3.Alarm.BattGrp4_InputContactor;
	
	//------Get Off-Grid ACLoad Meter Value------//
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 413, &varV, 4);
	pCCU->esRunData.fVoffLoad[0] = varV.fValue;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 415, &varV, 4);
	pCCU->esRunData.fVoffLoad[1] = varV.fValue;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 417, &varV, 4);
	pCCU->esRunData.fVoffLoad[2] = varV.fValue;

    pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 400, &varV, 4); //PoffLoad AC Meter 
	pCCU->esRunData.fPoffgrid_LoadMeter = varV.fValue;

    pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 404, &varV, 4); //QoffLoad AC Meter
	pCCU->esRunData.fQoff_LoadMeter = varV.fValue;

    pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 411, 424, &varV, 4); //FreqoffLoad AC Meter 
	pCCU->esRunData.fFreqoff_LoadMeter = varV.fValue;


	//------------Grid AC Meter Status--------------//
	pCCU->esRunData.emGridAC_WorkStatus = SOURCE_AC_IDLE;
	//get ac meter reading and judge
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 413, &varV, 4);
	pCCU->esRunData.fGrid_ACVolt_L1 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 414, &varV, 4);
	pCCU->esRunData.fGrid_ACCurr_L1 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 415, &varV, 4);
	pCCU->esRunData.fGrid_ACVolt_L2 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 416, &varV, 4);
	pCCU->esRunData.fGrid_ACCurr_L2 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 417, &varV, 4);
	pCCU->esRunData.fGrid_ACVolt_L3 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 408, 418, &varV, 4);
	pCCU->esRunData.fGrid_ACCurr_L3 = varV.fValue;

	if(pCCU->esRunData.f_PgridMeter > 0.5)
    {
		pCCU->esRunData.emGridAC_WorkStatus = SOURCE_AC_PENDING_OUT;
    }
    else if(pCCU->esRunData.f_PgridMeter < -0.5)
    {
		pCCU->esRunData.emGridAC_WorkStatus = SOURCE_AC_PENDING_IN;
		pCCU->esRunData.fGrid_ACCurr_L1 = -1*pCCU->esRunData.fGrid_ACCurr_L1;
		pCCU->esRunData.fGrid_ACCurr_L2 = -1*pCCU->esRunData.fGrid_ACCurr_L2;
		pCCU->esRunData.fGrid_ACCurr_L3 = -1*pCCU->esRunData.fGrid_ACCurr_L3;
     }
    else
    {	if((pCCU->esRunData.fACVolt_L1 < 50.0)&&(pCCU->esRunData.fACVolt_L2 < 50.0)&&(pCCU->esRunData.fACVolt_L3 < 50.0))
		{
			pCCU->esRunData.emGridAC_WorkStatus = SOURCE_AC_POWEROFF;
		}
		else
		{
			pCCU->esRunData.emGridAC_WorkStatus = SOURCE_AC_IDLE;
		}
	}

 	//------------------ Local AC Meter DataInfo-----------------//
	pCCU->esRunData.emAC_WorkStatus = SOURCE_AC_IDLE;

	//get Local ac meter reading and judge
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 413, &varV, 4);
	pCCU->esRunData.fACVolt_L1 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 414, &varV, 4);
	pCCU->esRunData.fACCurr_L1 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 415, &varV, 4);
	pCCU->esRunData.fACVolt_L2 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 416, &varV, 4);
	pCCU->esRunData.fACCurr_L2 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 417, &varV, 4);
	pCCU->esRunData.fACVolt_L3 = varV.fValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 407, 418, &varV, 4);
	pCCU->esRunData.fACCurr_L3 = varV.fValue;

	if(pCCU->esRunData.f_PcabMeter > 1.0) //2021-08-30 Found when Battery Dischrge Curr is <= 3A,the Local AC Meter Power Value is Positive
    {
		pCCU->esRunData.emAC_WorkStatus = SOURCE_AC_PENDING_OUT;
    }
    else if(pCCU->esRunData.f_PcabMeter < -0.5)
    {
		pCCU->esRunData.emAC_WorkStatus = SOURCE_AC_PENDING_IN;
		pCCU->esRunData.fACCurr_L1 = -1*pCCU->esRunData.fACCurr_L1;
		pCCU->esRunData.fACCurr_L2 = -1*pCCU->esRunData.fACCurr_L2;
		pCCU->esRunData.fACCurr_L3 = -1*pCCU->esRunData.fACCurr_L3;
     }
    else
    {	if((pCCU->esRunData.fACVolt_L1 < 50.0)&&(pCCU->esRunData.fACVolt_L2 < 50.0)&&(pCCU->esRunData.fACVolt_L3 < 50.0))
		{
			pCCU->esRunData.emAC_WorkStatus = SOURCE_AC_POWEROFF;
		}
		else
		{
			pCCU->esRunData.emAC_WorkStatus = SOURCE_AC_IDLE;
		}
    }

	A_BLOB_PV_INVERTER_DATA_TOTAL pvInverterDataTotal;
	varV.blobValue = (BYTE *)&pvInverterDataTotal;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1050, 3697, &varV, sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL));

	int F_Meter_Enable = 0; //电表F如果安装就同步到光伏逆变器功率
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 406, 401, &varV, 4);
	F_Meter_Enable = varV.emValue;
    
	float fPvPower = 0.0f;
	if(F_Meter_Enable == 1) //安装F电表
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 406, 400, &varV, 4);
		fPvPower = varV.fValue;
	}
	else
	{
		fPvPower = pvInverterDataTotal.fTotalPVOutputActPower;
	}

	//ACLoad DataInfo
	pCCU->esRunData.emACLoad_WorkStatus = SOURCE_AC_IDLE;
	
	if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == 1) //1:Off-Grid
	{
		pCCU->esRunData.fACLoad_Power = pCCU->esRunData.fPoffgrid_LoadMeter + fPvPower;
	}
	else
	{
		pCCU->esRunData.fACLoad_Power = pCCU->esRunData.f_PgridMeter - pCCU->esRunData.f_PcabMeter + pCCU->esRunData.fPoffgrid_LoadMeter + fPvPower;
	}

	if(pCCU->esRunData.fACLoad_Power > 0.5)
    {
		pCCU->esRunData.emACLoad_WorkStatus = SOURCE_AC_PENDING_OUT;
    }
    else if(pCCU->esRunData.fACLoad_Power < -5.0)
    {
		//pCCU->esRunData.emACLoad_WorkStatus = SOURCE_AC_PENDING_IN;
		pCCU->esRunData.emACLoad_WorkStatus = SOURCE_AC_IDLE; //John. 2022-01-03, ACLoad  Dispaly Pending_IN Status.
	}
	else
	{
		pCCU->esRunData.emACLoad_WorkStatus = SOURCE_AC_IDLE;
	}
	pCCU->esRunData.fACLoad_ACVolt_L1 = pCCU->esRunData.fGrid_ACVolt_L1;
	pCCU->esRunData.fACLoad_ACVolt_L2 = pCCU->esRunData.fGrid_ACVolt_L2;
	pCCU->esRunData.fACLoad_ACVolt_L3 = pCCU->esRunData.fGrid_ACVolt_L3;
	pCCU->esRunData.fACLoad_ACCurr_L1 = pCCU->esRunData.fGrid_ACCurr_L1 - pCCU->esRunData.fACCurr_L1;
	pCCU->esRunData.fACLoad_ACCurr_L2 = pCCU->esRunData.fGrid_ACCurr_L2 - pCCU->esRunData.fACCurr_L2;
	pCCU->esRunData.fACLoad_ACCurr_L3 = pCCU->esRunData.fGrid_ACCurr_L3 - pCCU->esRunData.fACCurr_L3;

 	//------------------ END -----------------//

	//TRACEX("nClientTotal=%d pCCU->esRunData.emAC_WorkStatus=%d pCCU->esRunData.fACCurr_L1=%f pCCU->esRunData.fACCurr_L2=%f pCCU->esRunData.fACCurr_L3=%f\n",
	//	nClientTotal, pCCU->esRunData.emAC_WorkStatus, pCCU->esRunData.fACCurr_L1, pCCU->esRunData.fACCurr_L2, pCCU->esRunData.fACCurr_L3);
	
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 500, 502, &varV, 4);
	pCCU->esRunData.nBattCfgNum = varV.ulValue;
 
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + n, 550, &varV, 4);
		pCCU->esRunData.emSTBatt_WorkStatu[n] = varV.emValue;
		if(pCCU->esRunData.emSTBatt_WorkStatu[n] <= STBATT_RUNSTAT_COMM_FAIL)
		{
			pCCU->esRunData.nSTBatt_SOC[n] = 0;
			pCCU->esRunData.fSTBatt_OutVolt[n] = 0;
			pCCU->esRunData.fSTBatt_OutCurr[n] = 0;
		}
		else
		{
			pCCU->esRunData.nSTBatt_SOC[n] = pCCU->strLSYSInfo.branchNodeData[n].nSOC;
			pCCU->esRunData.fSTBatt_OutVolt[n] = pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Volt;
			pCCU->esRunData.fSTBatt_OutCurr[n] = pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Curr;	
		}	

		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + n, 558, &varV, 4);
		pCCU->esRunData.fSTBatt_Tmax[n] = varV.fValue;
	}
 
	//----------------- ACDC info ----------------//
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->esRunData.emACDC_WorkStatu[n] 	= pCCU->strLSYSInfo.branchNodeData[n].emACDC_WorkStatu;
		pCCU->esRunData.fACDC_OutVolt[n] 		= pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Volt;
		pCCU->esRunData.fACDC_OutCurr[n] 		= pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Curr;
		pCCU->esRunData.fACDCOutRelayVolt[n] 	= pCCU->strLSYSInfo.branchNodeData[n].fACDCOutRelayVolt;
		pCCU->esRunData.fActvPwr_PCSGrp[n] 		= pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp;
		pCCU->esRunData.fQ_PCSGrp[n] 			= pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp;

		int m = 0;
		for(m = 0; m < NUM_GRID_PHASE; m++)
		{
			pCCU->esRunData.fV_PCS3Phase[n][m] = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputVolt[m];
			pCCU->esRunData.fI_PCS3Phase[n][m] = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputCurr[m];
		}
 	}

	//DCDC info
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->esRunData.emDCDC_WorkStatu[n] = pCCU->strLSYSInfo.branchNodeData[n].emDCDC_WorkStatu;
		pCCU->esRunData.fDCDC_OutVolt[n] = pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt;
		pCCU->esRunData.fDCDC_OutCurr[n] = pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr;
	}

	//MPPT Info
	pCCU->esRunData.fTotalMPPT_Power = 0.0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->esRunData.emMPPT_WorkStatu[n] = pCCU->strLSYSInfo.branchNodeData[n].emMPPT_WorkStatu;
		pCCU->esRunData.fMPPT_OutVolt[n] = pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt; 
		pCCU->esRunData.fMPPT_OutCurr[n] = pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr;
		if(pCCU->blobBackendCfg.isMPPT_Grp_Exist[n] == TRUE)
		{
			pCCU->esRunData.fTotalMPPT_Power += pCCU->esRunData.fMPPT_OutVolt[n]*pCCU->esRunData.fMPPT_OutCurr[n]*0.001;
		}
		//TRACE("branchNodeData[%d].emMPPT_WorkStatu=[%d]\n",
		//	n, pCCU->strLSYSInfo.branchNodeData[n].emMPPT_WorkStatu); 
	}

	//DCLoad info
	float fMAX_DCLoadVolt = 0, fACLoadCurr = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		fMAX_DCLoadVolt = (fMAX_DCLoadVolt > pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt) ? 
			fMAX_DCLoadVolt : pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt;
		fACLoadCurr = fMAX_DCLoadVolt > pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr;
	}
 
	pCCU->esRunData.fDCLoad_Volt = fMAX_DCLoadVolt;
	pCCU->esRunData.fDCLoad_Curr = fACLoadCurr;
	
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->esRunData.isACDC_Grp_Exist[n] = pCCU->blobBackendCfg.isACDC_Grp_Exist[n];
		pCCU->esRunData.isDCDC_Grp_Exist[n] = pCCU->blobBackendCfg.isDCDC_Grp_Exist[n];
		pCCU->esRunData.isMPPT_Grp_Exist[n] = pCCU->blobBackendCfg.isMPPT_Grp_Exist[n];
	}

	for(n = 0 ; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->esRunData.mapPCSSysStatu[n].byPCSSysStatu = pCCU->strLSYSInfo.mapPCSSysStatus[n].byPCSSysStatu;
		pCCU->esRunData.mapPCSSysStatuBelong2[n].byPCSSysStatu = pCCU->strLSYSInfo.mapPCSSysStatusBelong2[n].byPCSSysStatu;
	}
	//--- Calc Daily Statistic Data ---//
	time_t 	tmNow = _NOW_;
    struct 	tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    int  	nCurr_MDay = gmTime.tm_mday - 1;
	if((nCurr_MDay < 0) || (nCurr_MDay > MAX_DAYS_SECTOR))
	{
		pCCU->esRunData.fCabAC_InputKWH_D 		= 0;
		pCCU->esRunData.fCabAC_OutputKWH_D 		= 0;
		pCCU->esRunData.fACLoad_InputKWH_D 		= 0;
		pCCU->esRunData.fBattGrpChrgKWH_D 		= 0;
		pCCU->esRunData.fBattGrpDisChrgKWH_D 	= 0;
		pCCU->esRunData.fPVGrp_OutPutKWH_D 		= 0;
		pCCU->esRunData.fGridLoad_InputKWH_D    = 0;
	}
	else
	{
		pCCU->esRunData.fCabAC_InputKWH_D 		= pCCU->hisESData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_MDay]; 
		pCCU->esRunData.fCabAC_OutputKWH_D 		= pCCU->hisESData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_MDay];
		pCCU->esRunData.fACLoad_InputKWH_D 		= pCCU->hisESData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nCurr_MDay];
		pCCU->esRunData.fGridLoad_InputKWH_D	= pCCU->hisESData.es_Data.days_Data.fOffGridLoad_CostKWH_D[nCurr_MDay];

		pCCU->esRunData.fBattGrpChrgKWH_D 		= 0;
		pCCU->esRunData.fBattGrpDisChrgKWH_D 	= 0;
		pCCU->esRunData.fPVGrp_OutPutKWH_D 		= 0;

		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			pCCU->esRunData.fBattGrpChrgKWH_D 		+= pCCU->hisESData.es_Data.days_Data.fBattTotalChrgKWH_D[n][nCurr_MDay];
			pCCU->esRunData.fBattGrpDisChrgKWH_D 	+= pCCU->hisESData.es_Data.days_Data.fBattTotalDisChrgKWH_D[n][nCurr_MDay];
			pCCU->esRunData.fPVGrp_OutPutKWH_D 		+= pCCU->hisESData.es_Data.days_Data.fMpptTotalGKWH_D[n][nCurr_MDay];
		}
	}
	
	//Added By John 20230401,For ABU
	for(n = 0; n < NUM_GRID_PHASE; n++)
	{
		pCCU->esRunData.fSTS_InputVolt[n] = 
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPh_InputVolt[n];

		pCCU->esRunData.fSTS_InputCurt[n] =
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPh_InputCurr[n];

		pCCU->esRunData.fSTS_OutputVolt[n] =
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPh_OutputVolt[n];

		
	}

	pCCU->esRunData.fAC_Freq = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fAC_Freq;
	pCCU->esRunData.fAmbit_Temp = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fAmbit_Temp;
	pCCU->esRunData.fPtotalActive = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPtotalActive;
	pCCU->esRunData.fPtotalReactive = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPtotalReactive;
	pCCU->esRunData.fPtotalFactor = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][0].fPtotalFactor;
 
	// LJH240925 RunData
	pCCU->esRunData.bEnterAnti_Reflux = pEMS->bEnterAnti_Reflux;
    pCCU->esRunData.bEnter_OverLoad =   pEMS->bEnter_OverLoad;
	pCCU->esRunData.bEnter_OverLoadHV = pEMS->bEnter_OverLoadHV;
	// LJH241007 AntiPhase RunData
	pCCU->esRunData.fGridRemainACPwr2Chrg = pEMS->fGridRemainACPwr2Chrg;
	pCCU->esRunData.fRemainPwr4Chrg =       pEMS->fRemainPwr4Chrg;
	pCCU->esRunData.fRemainDemand4Chrg =    pEMS->fRemainDemand4Chrg;
	for(n = 0; n < MAX_PHASE_NUM; n++)
	{
		pCCU->esRunData.fAtRflxMtr_kWPhs[n] = pEMS->fAtRflxMtr_kWPhs[n];
		pCCU->esRunData.fAtOvlMtr_APhs[n] =   pEMS->fAtOvlMtr_APhs[n];
	}
	pCCU->esRunData.fAtRflxMtr_Factor =       pEMS->fAtRflxMtr_Factor;
	pCCU->esRunData.fDiffPwr4Dischrg_SumPhs = pEMS->fDiffPwr4Dischrg_SumPhs;
	pCCU->esRunData.fAtOvlMtr_Factor =        pEMS->fAtOvlMtr_Factor;
	pCCU->esRunData.fMaxPwr4Chrg_SumPhs =     pEMS->fMaxPwr4Chrg_SumPhs;
	pCCU->esRunData.bEnterAnti_Reflux_Phase = pEMS->bEnterAnti_Reflux_Phase;
	pCCU->esRunData.bEnter_OverLoad_Phase =   pEMS->bEnter_OverLoad_Phase;
	pCCU->esRunData.fAtRflx_kW =              pEMS->fAtRflx_kW;
	pCCU->esRunData.fAtOvl_kW =               pEMS->fAtOvl_kW;
	
	pCCU->esRunData.bEnter_AutoChrg_SEMS = pEMS->bEnter_AutoChrg_SEMS;
	pCCU->esRunData.bEnter_AutoDisc_SEMS = pEMS->bEnter_AutoDisc_SEMS;
	// LJH250408 CtrlByREMS
	pCCU->esRunData.emEMSCtrlMode_REMS =    pEMS->clientInfoSum.emEMSCtrlMode_REMS;
	pCCU->esRunData.emACDC_WorkStatu_REMS = pEMS->clientInfoSum.emACDC_WorkStatu_REMS;
	pCCU->esRunData.fActvPwrReq_REMS =      pEMS->clientInfoSum.fActvPwrReq_REMS;
	// LJH250726 东博上报MaxChg|DisChg都为0
	pCCU->esRunData.fRatedPower =    pCCU->strLSYSInfo.blobGrpRectData.fRatedPower[0] ; // 额定功率
	pCCU->esRunData.nBattGrpPwrLmt = pEMS->planSectorNow.nBattGrpPwrLmt; // 单组电池组功率限制

	//John 2024-03-14.Added For Slave EMS MeterValue Display.
	if(pCCU->emESSystemType == ESSTYPE_UDP_SERVER)
	{
		Update_SlvMeterValue(pCCU);
	}
	Push_BlobValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 80, (void*)&pCCU->esRunData,
			sizeof(A_BLOB_ES_RUN_DATA), BLOB_TYPE_ES_RUNDATA);

	return;
}



/*********************************************************************************
*  
*  FUNCTION NAME : PushAllData
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void PushAllData(ROUGH_CCU_DATA* pCCU)
{
	// EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

	//TRACE("---CCU Tcp Sampler Main PushAllData---\n");
	int n = 0; // m = 0;
	VAR_VALUE varV;
	char szSingleRectName[64] = "\0";

	varV.emValue = pCCU->emCCUState;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 4, varV);

	varV.emValue = pCCU->emSystemFailReason;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 6, varV);

	if(ABS(_NOW_ - pCCU->tmSysBoot) > 120) //John-20220610,Wait 120S,then Judge CCU Comm Status.
	{
		varV.emValue = pCCU->bCommFail;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 100, varV);
	}

	varV.ulValue = pCCU->strLSYSInfo.uiCCUFaultCode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 106, varV);

	varV.ulValue = pCCU->strLSYSInfo.uiPCS1CtrlFailCode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 107, varV);

	varV.ulValue = pCCU->strLSYSInfo.uiPCS2CtrlFailCode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 108, varV);

	varV.ulValue = pCCU->strLSYSInfo.uiPCS3CtrlFailCode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 109, varV);

	varV.ulValue = pCCU->strLSYSInfo.uiPCS4CtrlFailCode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 110, varV);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		varV.ulValue = pCCU->strLSYSInfo.mapPCSSysStatus[n].byPCSSysStatu;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 111 + n, varV);
	}

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		varV.ulValue = pCCU->strLSYSInfo.mapPCSSysStatusBelong2[n].byPCSSysStatu;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 115 + n, varV);
	}

	if(pCCU->lcCfg.emSystemType == SYSTYPE_ESS_AIR)
	{
		varV.fValue = pCCU->strLSYSInfo.fLeakageCurr;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1, 40, varV);
	}
 
	varV.emValue = pCCU->emsMgmtData.bEnterUrgetDischrgMode;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 9, varV);

	varV.emValue = pCCU->emsMgmtData.bEnterAnti_Reflux;
 	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 25, varV);

    varV.emValue = pCCU->emsMgmtData.bEnter_OverLoad;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 26, varV);
	// LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
	varV.emValue = pCCU->emsMgmtData.bEnter_OverLoadHV;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 28, varV);

	varV.emValue = pCCU->emErrCode_MgmtCfg;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 102, varV);

	varV.emValue =  pCCU->emCCUState;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 103, varV);
 
	//Push Single Battery DataInfo To Branch Struct
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + n, 551, &varV, 
		//		sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));

		pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Volt = pCCU->blobSingleBattData[n].fPackTotalVolt;
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Curr = pCCU->blobSingleBattData[n].fPackTotalCurr;
		pCCU->strLSYSInfo.branchNodeData[n].nSOC = pCCU->blobSingleBattData[n].nSOC;
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_DisChrg_Pwr = fabs(pCCU->blobSingleBattData[n].fPackTotalVolt*pCCU->blobSingleBattData[n].fAllow_MaxDisChargeCurr*0.001);
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_Chrg_Pwr_Req = fabs(pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt*pCCU->blobSingleBattData[n].fAllow_MaxChargeCurr*0.001);	

		pCCU->strLSYSInfo.branchNodeData[n].nOnlineACDCNum = pCCU->strLSYSInfo.singleRectData.nOnlineNum[(TYPE_SIGNLE_RECT)n];
		pCCU->strLSYSInfo.branchNodeData[n].nOnlineMPPTNum = pCCU->strLSYSInfo.singleRectData.nOnlineNum[(TYPE_SIGNLE_RECT)(n + 4)];
		//TRACEX("idx=[%d] fBatt_RealTime_Volt=[%.1f] fBatt_RealTime_Curr=[%.1f] \
		//		fBatt_Max_DisChrg_Pwr=[%.1f] fBatt_Max_Chrg_Pwr_Req=[%.1f] nSOC=[%d]\n",\
		//	n, pCCU->blobSingleBattData[n].fPackTotalVolt, pCCU->blobSingleBattData[n].fPackTotalCurr, \
		//	pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_DisChrg_Pwr, \
		//	pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_Chrg_Pwr_Req, \
		//	pCCU->strLSYSInfo.branchNodeData[n].nSOC);
		
		//Stuff Branch Node Data - 2021-10-11
		Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 211 + n, 201, (void*)&pCCU->strLSYSInfo.branchNodeData[n], sizeof(A_BLOB_BRANCH_GRP_RECTDATA), BLOB_TYPE_BRANCH_GRP_RECT_DATA);
	}

	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 101, (void*)&pCCU->strLSYSInfo.singleRectData, sizeof(A_BLOB_SINGLE_RECT_INFO), BLOB_TYPE_SINGLE_RECT_DATAINFO);
	
	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 204, (void*)&pCCU->strLSYSInfo.blobGrpRectData, sizeof(A_BLOB_RECT_GRP_DATA), BLOB_TYPE_GRP_PM_DATAINFO);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		Push_BlobValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 221 + n, 203, (void*)&pCCU->emsMgmtData.calc_Node[n], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA), BLOB_TYPE_BRANCH_NODE_CALC_DATA);
	}
	
 	for(n = 0; n < 8; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.mapSTSCode.byRunStatus >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 160 + n, varV);
	}
	
	varV.ulValue = (ULONG)pCCU->strLSYSInfo.mapAbuStatu1.byRunStatus;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 169, varV);

	varV.emValue = (_ENUM)pCCU->strLSYSInfo.mapAbuStatu1.Status.Slave0rMaster ? 0 : 1;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1, 13, varV);

	//Attention Alarm Bit Map: Stuff Status Parameter Signal should be continuously 2021-10-26
	//----- SigID 240~271 -----//
	for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap1.uiFlag >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 240 + n, varV);
	}
	 
	//----- SigID 304~335 -----//
	for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap3.uiFlag >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 304 + n, varV);
	}
	
	//----- SigID 336~367 -----// 20230905 John Modify DCLoad Alarm.
	
	for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap4.uiFlag >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 336 + n, varV);
	}
	

	//----- SigID 368~399 -----//
	for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.pmGrpAlmMap.uiFlag >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 368 + n, varV);
	}
	
	//John 20230905. added For Judge Need Single PCS Alarm.
	BOOL bNeedShieldSinglePcsAlm = JudgeisNeedSkipSinglePCSAlarm(pCCU);

	//---- SigID 1201~1232 ----//
	UINT nOnline = 0;
	if(pCCU->nStuffWaitTimes == 0)
	{
 		BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP1);

 		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || bOnIsoPending|| (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].mapPCSCode.uiPCSStatus = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1201 + n, varV);
				continue;
			}

			for(n = 8; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1201 + n, varV);
			}
		}
	}
 
	if(pCCU->nStuffWaitTimes == 1)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
		{
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1150, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byRectErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1151, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byInvertErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1152, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultCode,  16);
			
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1153, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultCode_1113,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1154, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultCode_1114,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1155, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultCode_1115,  16);
		}

		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
		{
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1150, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byRectErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1151, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byInvertErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1152, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultCode,  16);
			
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1153, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultCode_1113,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1154, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultCode_1114,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1155, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultCode_1115,  16);
		}

		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP3]; nOnline++)
		{
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1150, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byRectErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1151, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byInvertErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1152, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byPCSFaultCode,  16);
			
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1153, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byPCSFaultCode_1113,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1154, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byPCSFaultCode_1114,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1155, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].byPCSFaultCode_1115,  16);

		}

		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP4]; nOnline++)
		{
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1150, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byRectErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1151, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byInvertErrCode,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1152, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byPCSFaultCode,  16);
			
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1153, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byPCSFaultCode_1113,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1154, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byPCSFaultCode_1114,  16);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1155, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].byPCSFaultCode_1115,  16);
		}
	}
 
	if(pCCU->nStuffWaitTimes == 2)
	{
 		BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP2);

 		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].mapPCSCode.uiPCSStatus = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1201 + n, varV);
				continue;
			}
			for(n = 8; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1201 + n, varV);
			}
		}
	}


	if(pCCU->nStuffWaitTimes == 3)
	{
 		BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP3);

 		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP3]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].mapPCSCode.uiPCSStatus = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1201 + n, varV);
				continue;
			}
			for(n = 8; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1201 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 4)
	{
 		BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP4);

 		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP4]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].mapPCSCode.uiPCSStatus = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1201 + n, varV);
				continue;
			}
			for(n = 8; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1201 + n, varV);
			}
		}
	}
 
	//Adde By John 2022-09-08 For Single PCS Detail AlarmInfo.
	if(pCCU->nStuffWaitTimes == 5)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 = 0;
				continue;
			}
			// 2025-02-17 George Add 235kWPCS Alarm
			// if (pCCU->lcCfg.emSystemType == SYSTYPE_SCADA_INPCS235KW)
			// {
			// 	// printf("parse 235kWPCS [1113] Alarm\n");
			// 	int i = 0;
			// 	// for (n = 0; n < 32; n++)
			// 	// {
			// 	// 	printf("%d", (pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
			// 	// 	if (n % 8 == 7)
			// 	// 	{
			// 	// 		printf(" ");
			// 	// 	}
			// 	// }
			// 	// printf("\n");

			// 	// 2025-02-17 George add 逆变状态表 0
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	// printf("varV.emValue before = %d\n", varV.emValue);
			// 	//varV.emValue = nUnified_Variable;
			// 	// printf("varV.emValue after= %d\n", varV.emValue);
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1298, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1299, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2104, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2103, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2102, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2101, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2100, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1301, varV);

			// 	// 2025-02-17 George add 逆变状态表 1 		
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1302, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1240, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1237, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2108, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2107, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2245, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2106, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2105, varV);

			// 	// 2025-02-17 George add 逆变状态表 2
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1244, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2112, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2110, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2111, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1248, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2250, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1259, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2109, varV);

			// 	// 2025-02-17 George add 逆变状态表 3
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2113, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1289, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1275, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1276, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1277, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1278, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1279, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1296, varV);

			// }
			// else
			{
				for(n = 0; n < MAX_ALARMBIT_MAP; n++)
				{
					varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1233 + n, varV);
				}	
			}
		}
	}
 
 	if(pCCU->nStuffWaitTimes == 6)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 = 0;
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 = 0;
				continue;
			}
			// if (pCCU->lcCfg.emSystemType == SYSTYPE_SCADA_INPCS235KW)
			// {
			// 	// printf("parse 235kWPCS [1114, 1115] Alarm\n");
			// 	int i = 0;

			// 	// 2025-02-18 George add 1114 逆变状态表 0
			// 	i = 0;
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1288, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1287, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1273, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1274, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2114, varV);

			// 	// 2025-02-18 George add 1115 逆变状态表 0
			// 	i = 0;
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1297, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2120, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2119, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2118, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2117, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2116, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2115, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1304, varV);

			// 	// 2025-02-18 George add 1115 逆变状态表 1
			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1305, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2123, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2122, varV);

			// 	i += 2;
			// 	// varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	// //varV.emValue = nUnified_Variable;
			// 	// Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1308, varV);

			// 	// varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	// //varV.emValue = nUnified_Variable;
			// 	// Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1309, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1310, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1218, varV);

			// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> i++) & 0x01);
			// 	//varV.emValue = nUnified_Variable;
			// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2121, varV);

			// }
			// else
			{
				for(n = 0; n < MAX_ALARMBIT_MAP; n++)
				{
					varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1114 >> n) & 0x01);
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1265 + n, varV);
				}			
				
				for(n = 0; n < 16; n++)
				{
					varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiPCSFaultCode_1115 >> n) & 0x01);
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1297 + n, varV);
				}
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 7)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1113 = 0;
				continue;
			}

			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1233 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 8)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1114 = 0;
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1115 = 0;
				continue;
			}
			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1114 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1265 + n, varV);
			}

			for(n = 0; n < 16; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].uiPCSFaultCode_1115 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1297 + n, varV);
			}

		}
	}

	if(pCCU->nStuffWaitTimes == 9)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP3]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1113 = 0;
				continue;
			}

			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1233 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 10)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP3]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1114 = 0;
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1115 = 0;
				continue;
			}
			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1114 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1265 + n, varV);
			}

			for(n = 0; n < 16; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].uiPCSFaultCode_1115 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1297 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 11)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP4]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1113 = 0;
				continue;
			}

			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1233 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 12)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP4]; nOnline++)
		{
			if(bNeedShieldSinglePcsAlm || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1114 = 0;
				pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1115 = 0;
				continue;
			}
			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1114 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1265 + n, varV);
			}

			for(n = 0; n < 16; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].uiPCSFaultCode_1115 >> n) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1297 + n, varV);
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 13)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_STS]; nOnline++)
		{
			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].uiPCSFaultCode_1113 >> n) & 0x01);
				if((pCCU->strLSYSInfo.mapSTSCode.byRunStatus & 0x02) && (n == 1))
				{//WLD250509 Off-grid  disable GridUVP 1330(bit1).
					varV.emValue = 0;
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1329 + n, varV);
				}
				else
				{
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1329 + n, varV);
				}
			}
		
			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].uiPCSFaultCode_1114 >> n) & 0x01);
				if((pCCU->strLSYSInfo.mapSTSCode.byRunStatus & 0x02) && (n == 24))
				{//WLD250509 Off-grid  disable GridFault alarm 1385(bit24).
					varV.emValue = 0;
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1361 + n, varV);
				}
				else
				{
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1361 + n, varV);
				}
			}

			for(n = 0; n < MAX_ALARMBIT_MAP; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].uiPCSFaultCode_1115 >> n) & 0x01);
				if((n == 13)||(n == 16)||(n == 17)||(n == 18)||(n == 19))
				{//WLD250509 Off-grid and manual ctrl disable STS Fan alarm 1406; 1409-1412.(bit13, bit16-bit19)
					if((pCCU->strLSYSInfo.mapSTSCode.byRunStatus & 0x02) && (pCCU->emPMCtrlMode == 1))
					{
						varV.emValue = 0;
						Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1393 + n, varV);
					}
					else
					{
						Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1393 + n, varV);
					}
				}
				else
				{
					Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1393 + n, varV);
				}
			}
		}
	}

	if(pCCU->nStuffWaitTimes == 14)
	{
		float  fMax_SysTemp = 0.0;
		for(n = 0; n < MAX_AMBINENT_TEMP; n++)
		{
			varV.fValue = pCCU->strLSYSInfo.fAmbientTemp[n];
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1, 41 + n, varV);
			//WWJ Suggest Normal Samp Temp Range:[-40,125];
			if((pCCU->strLSYSInfo.fAmbientTemp[n] < 125) && (pCCU->strLSYSInfo.fAmbientTemp[n] > -40))
			{
				fMax_SysTemp = MAX(fMax_SysTemp, pCCU->strLSYSInfo.fAmbientTemp[n]);
			}
		}
		//Stuff System Max Ambinent Temperature
		varV.fValue = fMax_SysTemp;
		Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 3, varV);

		Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 99, 
			(void*)&pCCU->strLSYSInfo.szPowerCube_IMEU2,  9);

		//------------20230316 Need Stuff STS Data--------------//
		if(pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_STS] > 0)
		{
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 275, 1152, 
					(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].byPCSFaultCode,  16);
				
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 275, 1153, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].byPCSFaultCode_1113,  16);

			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 275, 1154, 
				(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].byPCSFaultCode_1114,  16);

			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 275, 1155, 
					(void*)&pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_STS][nOnline].byPCSFaultCode_1115,  16);
		}

		//----- SigID 180 ~ 199 -----//
		for(n = 0; n < MAX_SUPPORT_IMIU_NUM; n++)
		{
			varV.fValue = pCCU->strLSYSInfo.fIMIU_P_IsoVal[n];
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 180 + n*2, varV);

			varV.fValue = pCCU->strLSYSInfo.fIMIU_N_IsoVal[n];
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 180 + n*2 + 1, varV);
		}

		for(n = 0; n < MAX_ALARMBIT_MAP; n++)
		{
			varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap2.uiFlag >> n) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 272 + n, varV);
		}

		varV.emValue = pCCU->strLSYSInfo.emAPPUpgradingType;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 120, varV);
 
		varV.ulValue = pCCU->strLSYSInfo.byUpgradingSubID;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 121, varV);

		varV.emValue = pCCU->strLSYSInfo.emUpgradingStatus;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 122, varV);
		
		varV.ulValue = pCCU->strLSYSInfo.byUpgradingSubIDSchedule;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 123, varV);

		varV.emValue = pCCU->strLSYSInfo.emUpgradingTotSchedule;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 124, varV);

		varV.ulValue = pCCU->strLSYSInfo.byUpgradingFailReason;
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 125, varV);	
	}
	if(pCCU->nStuffWaitTimes == 15)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
		{
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1110[0] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2200 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1110[1] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2208 + n, varV);

			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1110[2] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2216 + n, varV);
			}
			for(n = 0; n < BYTE_NUM - 2; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1110[3] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2224 + n, varV);
			}
			varV.emValue = (_ENUM)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1110[3] & 0x03);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2230, varV);

			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1111[0] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2231 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1111[1] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2239 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1111[2] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2247 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byPCSFaultcode_1111[3] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2255 + n, varV);
			}

			varV.fValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].f35KW_PlateTemperature;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2263, varV);

			varV.fValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].f35KW_PowerDevicesperature;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2264, varV);

#if 0
			// 模块当前能量调度模式
			varV.emValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byCurrent_Schedulingmode;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5288, varV);

			// 模块当前并离网状态
			varV.emValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byCurrent_Onandoffstatus;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5289, varV);

			// 当前系统状态
			varV.emValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byCurrent_Systemstatus;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5290, varV);

			// 当前充电模式
			varV.emValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].byCurrent_Chargingmode;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5291, varV);
#endif
			// 组号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].nBelongGrp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5292, varV);

			// 单模块号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uModuID;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5293, varV);

			// 状态信息
			STU_SIGNLE_RECT_DATA *pArrRect = &pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline];
			// switch (pCCU->pOldSet.ccuSettings.emACDCType)   // ACDC设备类型
			switch (pCCU->pOldSet.ccuSettings.byPCSType)
			{
				case ACDC_NONE:
					break;
				
				// case ACDC_N2101_35KW:
				case 2:
					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuCommIntrpAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bWALK_IN,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bOutputOverVoltAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bOverTempAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bFanFaultAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuProtectAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuFaultAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuDCSideOff,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bReserved5,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bReserved4,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuDischargeAbnormal,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bModuSleep,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bReserved3,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bReserved2,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bReserved1,
						pArrRect->unioInverterStatus_1_1.ACDC_N2101.bOutputSC
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5312, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.Reserved,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.emSeriesParallelMode,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bPfcShutdown,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bInputOverVolt,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bInputUnderVolt,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bPhaseUnbalance,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bPhaseLoss,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bCurrUnbalance,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.IDRepeat,
						pArrRect->unioInverterStatus_1_2.ACDC_N2101.bPowerLimit
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5313, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.Reserved7_15,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.bIslandingProtect,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.bPhaseLockErr,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.bAcFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.Reserved2_3,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.bInvBattUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2101.bInverterMode
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5314, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.Reserved5_15,
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.bEStopAlarm,
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.bGridConnInOffGrid,
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.Reserved2,
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.bOffGridAcOverload,
						pArrRect->unioInverterStatus_2_2.ACDC_N2101.bOffGridMode
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5315, szSingleRectName, 64);

					break;

				// case ACDC_N2002_62_5KW:
				case 1:
					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved2,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bAuxFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bRelayErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bUserSetErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattEodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattDodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bVrtFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOverBusVolt,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved1,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOnOff,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bCharge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bDischarge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emGridStatus,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emRunningStatus
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5312, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bSysFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bAcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bDcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bBusFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bInnerOverCurr,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutShortCircuit,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutOverload,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutDerating,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCommFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bIslandingProtect,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bPhaseLockFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOverTempFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bFanFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCurrUnbalance,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.IDRepeat,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bEOPFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5313, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bRevGridTransOverVolt1,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bRevGridTransOverVolt2,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bRevGridUnbalance,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bPhaseLockFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.Reserved,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bHvrtFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bLvrtFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bIslandingProtect,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridOverVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridOverFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridUnderFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridPhaseLoss,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridLossN,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridRevSeq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2002.bGridLoss
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5314, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBattOverVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBattUnderVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bRevBattReverseConn,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBattOverCurr,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBattChargeOverVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.Reserved10,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBattEodShutdown,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.Reserved8,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusTransUnbalance,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusSlowUnbalance,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusTransOverVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusSlowOverVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusTransUnderVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bBusSlowUnderVolt,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bRevBusShortCircuit,
						pArrRect->unioInverterStatus_2_2.ACDC_N2002.bAcHigherThanDc
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5315, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bBusSoftStartFail,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bInvSoftStartFail,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bGridRelayOpen,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bGridRelayShort,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bInvRelayOpen,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bInvRelayShort,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bDcRelayFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bDcSideFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bAcTransOverCurr,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bOutOverload,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.Reserved5,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bPhaseACbcLimit,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bPhaseBCbcLimit,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bPhaseCCbcLimit,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bOutCurrUnbalance,
						pArrRect->unioInverterStatus_3_1.ACDC_N2002.bEfficiencyFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5316, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bAcOutShortCircuit,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bDspFaultRev,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bAbuFaultRev,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bCanCommFaultRev,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bEepromFaultRev,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bEpoFault,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bCarrierAbnormal,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bFreqSyncAbnormal,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bDcCurrSampleFault,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bInductorCurrSampleFault,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bOutCurrSampleFault,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.Reserved3_4,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bDcSoftStartOverload,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bDcSoftStartRelayFault,
						pArrRect->unioInverterStatus_3_2.ACDC_N2002.bParallelCommFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5317, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bIdRepeat,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bDcShortCircuit,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bVoSetTooLow,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bVoSetOverProtect,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bFanFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bCpldShootThrough,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bDspShootThrough,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bAuxPowerFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bAmbientOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bHeatsinkOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bBusUnderVolt,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bBusOverVolt,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bBalanceBridgeOverCurr,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bSpiCommFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bSciCommFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2002.bCanCommFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5318, szSingleRectName, 64);
					break;

				// case ACDC_N2402_235KW:
				case 3:
					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved2,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bAuxFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bRelayErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bUserSetErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattEodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattDodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bVrtFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOverBusVolt,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved1,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOnOff,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bCharge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bDischarge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emGridStatus,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emRunningStatus
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5312, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bSysFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bAcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bDcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bBusFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bInnerOverCurr,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutShortCircuit,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutOverload,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutDerating,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCommFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bIslandingProtect,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bPhaseLockFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOverTempFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bFanFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCurrUnbalance,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.IDRepeat,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bEOPFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5313, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputNoPower,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputPhaseLoss,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputTransientDrop,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSwSampleOverCurr,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bActzOverCurr,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputVoltUnbalance,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bPhaseSeqError,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSciCommFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSpiCommFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcOverVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcOverFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcUnderFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusAbnormal,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusOverVolt
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5314, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutUnderVoltProtect,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutShortAlarm,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bGridRelayOpen,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bGridRelayShort,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bInvRelayOpen,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bInvRelayShort,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bDcRelayFault,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bFreqSyncAbnormal,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bPhaseLockErr2,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bPhaseLockErr1,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bAcRelayFault,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bAcShortCircuit,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bIslandingProtect,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bEStop,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bBattReverseConn,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutOverVolt
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5315, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.Reserved8_15,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bParallelCommFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bDcSoftStartRelayFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bBusSoftStartFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bInvSoftStartFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bOutCurrUnbalance,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.Reserved0_2
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5316, szSingleRectName, 64);


					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bAuxPowerFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bInsulationFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bLeakageCurrFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bFanFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bVoSetOverProtect,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bVoSetTooLow,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIdRepeat,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bSoftStartResOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bCanCommFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bWaterInletOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bWaterOutletOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmAOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmBOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmCOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmDOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bAmbientOverTemp
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5318, szSingleRectName, 64);
					break;

				default:
					break;
			}

			// DC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fRect_DCDCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1425, varV);
			
			// PFC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fRect_PFCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1426, varV);
			
			// 条码信息
			DecodeRectBarcode(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].uiBarCode, szSingleRectName, 64);
			TRACED("barcode: %s\n", szSingleRectName);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5294, szSingleRectName, 64);

			// 模块直流侧电压
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fDCsideVoltage;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5295, varV);

			// 模块直流侧电流
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fDCsideCurrent;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5296, varV);

			// 模块环境温度
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fAmbit_Temp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5297, varV);

			// 交流线电压
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fphase_LineVolt[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fphase_LineVolt[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fphase_LineVolt[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5298, szSingleRectName, 64);

			// 交流相电压
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputVolt[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputVolt[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputVolt[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5299, szSingleRectName, 64);

			// 交流相电流
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputCurr[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputCurr[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPh_InputCurr[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5300, szSingleRectName, 64);

			// 交流频率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fAC_Freq;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5301, varV);
			
			// 总有功功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPtotalActive;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5302, varV);

			// 三相有功功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Active[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Active[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Active[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5303, szSingleRectName, 64);

			// 总无功功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPtotalReactive;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5304, varV);

			// 三相无功功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Reactive[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Reactive[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Reactive[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5305, szSingleRectName, 64);

			// 总视在功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPTotalApparent;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5306, varV);

			// 三相视在功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Apparent[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Apparent[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Apparent[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5307, szSingleRectName, 64);

			// 总功率因数
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPtotalFactor;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5308, varV);

			// 三相功率因数
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f",
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Factor[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Factor[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].fPphase_Factor[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5309, szSingleRectName, 64);
		}

		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
		{
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1110[0] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2200 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1110[1] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2208 + n, varV);

			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1110[2] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2216 + n, varV);
			}
			varV.emValue = (_ENUM)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1110[3] & 0x03);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2230, varV);

			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1111[0] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2231 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1111[1] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2239 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1111[2] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2247 + n, varV);
			}
			for(n = 0; n < BYTE_NUM; n++)
			{
				varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].byPCSFaultcode_1111[3] >> (BYTE_NUM - n - 1)) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2255 + n, varV);
			}

			varV.fValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].f35KW_PlateTemperature;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2263, varV);

			varV.fValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].f35KW_PowerDevicesperature;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2264, varV);

		}
	}
#if 0
	if(pCCU->nStuffWaitTimes == 16)
	{
		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_MPPT_GRP1]; nOnline++)
		{
#if 0
			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bPFC_Off;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2265, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bAC_OverVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2266, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bDC_LowVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2267, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bAC_NoBalance;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2268, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bAC_PhaseLack;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2269, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bCurr_NoBalace;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2270, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bID_Conflict;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2271, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bPowerLimited;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2272, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bRectCommFail;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2273, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bWalkIn_Enabled;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2274, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bDC_OverVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2275, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bOverTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2276, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bFanFault;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2277, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bProtected;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2278, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bRectFail;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2279, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bDC_Off;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2280, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bDisCh_Err;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2283, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bOn_Sleep;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2284, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bPFC_Err;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2285, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bInternalCommErr;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2286, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bInternalDC_LowVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2287, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].bDC_Shorted;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2288, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fMPPT_DCTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2289, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fMPPT_PFCTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2290, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fMPPT_DCTransTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2291, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fMPPT_PFCTransTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 2292, varV);
#endif
			// 组号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].nBelongGrp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5292, varV);

			// 单模块号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].uModuID;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5293, varV);

			// TODO:状态信息

			// DC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fRect_DCDCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1425, varV);
			
			// PFC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fRect_PFCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1426, varV);
			
			// TODO:条码信息显示格式
			BYTE buffer[8];
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].usBarCode[0] = 12;
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].usBarCode[1] = 34;
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].usBarCode[2] = 56;
			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].usBarCode[3] = 78;
			memcpy(buffer, (BYTE*)pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].usBarCode, sizeof(buffer));
			snprintf(szSingleRectName, 64, "%s", buffer);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5294, szSingleRectName, 8);

			// 模块电压
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fModuVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5310, varV);

			// 模块电流
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fModuCurr;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5311, varV);

			// 模块环境温度
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP1][nOnline].fAmbit_Temp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 5297, varV);

		}

		for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_MPPT_GRP2]; nOnline++)
		{
			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bPFC_Off;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2265, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bAC_OverVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2266, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bDC_LowVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2267, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bAC_NoBalance;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2268, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bAC_PhaseLack;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2269, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bCurr_NoBalace;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2270, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bID_Conflict;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2271, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bPowerLimited;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2272, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bRectCommFail;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2273, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bWalkIn_Enabled;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2274, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bDC_OverVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2275, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bOverTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2276, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bFanFault;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2277, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bProtected;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2278, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bRectFail;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2279, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bDC_Off;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2280, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bDisCh_Err;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2283, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bOn_Sleep;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2284, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bPFC_Err;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2285, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bInternalCommErr;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2286, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bInternalDC_LowVolt;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2287, varV);

			varV.emValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].bDC_Shorted;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2288, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].fMPPT_DCTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2289, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].fMPPT_PFCTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2290, varV);

			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].fMPPT_DCTransTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2291, varV);
			
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_MPPT_GRP2][nOnline].fMPPT_PFCTransTemp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 2292, varV);
		}

	}
#endif

	pCCU->nStuffWaitTimes++; //Added by John 2022-03-31.In Order To Improve CPU Capbility
	if(pCCU->nStuffWaitTimes > 16)
	{
		pCCU->nStuffWaitTimes = 0;
	}

	varV.emValue = pCCU->strLSYSInfo.emRunningMode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 152, varV);
 
	//Stuff MPPT KWH
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(fabs(pCCU->strLSYSInfo.fMPPTGenerateKWH[n] - pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n]) > 0.5)
		{
			pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n] = pCCU->strLSYSInfo.fMPPTGenerateKWH[n];
			varV.fValue = pCCU->strLSYSInfo.fMPPTGenerateKWH[n];
			Push_VarValue(SETTYPE_INTERN_SETT,  pCCU->hSampler, 200, 204 + n, varV);
		}
	}
 
	if(fabs(pCCU->strLSYSInfo.fDCLoad_CostKWH - pCCU->strLSYSInfo.fLast_DCLoad_CostKWH) > 1.0)
	{
		pCCU->strLSYSInfo.fLast_DCLoad_CostKWH = pCCU->strLSYSInfo.fDCLoad_CostKWH;
		varV.fValue = pCCU->strLSYSInfo.fDCLoad_CostKWH;
		Push_VarValue(SETTYPE_INTERN_SETT,  pCCU->hSampler, 200, 208, varV);
	}
	
	varV.emValue = pCCU->emCurrDiffAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1910, varV);
	return;
}

void PushAllRereadPCSData(ROUGH_CCU_DATA* pCCU, UINT nDevIdx, UINT nParamId, VAR_VALUE* pVarVal, BOOL IsSetCmd)
{
    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
	ERRCDDE_DPR err;
	VAR_VALUE varV;
	VAR_VALUE varVAuth;

	int i, j;
	i = nDevIdx - 701;
	int nSingleRectNum = pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[i];
	for(j = 0; j < nSingleRectNum; j++)
	{
		if(nParamId >= 996 && nParamId <= 999)
		{
			if(IsSetCmd)
			{
				switch(nParamId)
				{
					case 996:
						varV.fValue = pVarVal->fValue;
						break;
					case 997:
						varV.fValue = pVarVal->fValue;
						break;
					case 998:
						varV.emValue = pVarVal->emValue;
						break;
					case 999:
						varV.fValue = pVarVal->fValue;
						break;
					default:
						continue;
				}
			}
			else
			{
				pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, nDevIdx, nParamId, &varVAuth, 4);
				switch(nParamId)
				{
					case 996:
						varV.fValue = varVAuth.fValue;
						break;
					case 997:
						varV.fValue = varVAuth.fValue;
						break;
					case 998:
						varV.emValue = varVAuth.emValue;
						break;
					case 999:
						varV.fValue = varVAuth.fValue;
						break;
					default:
						continue;
				}
			}
			Push_VarValue_RereadPCS(SETTYPE_Reread_PCS_Param, pCCU->hSampler, nDevIdx, nParamId, varV, j);
		}
		else
		{
			int paramIndex = j;
			BOOL bParamMatched = TRUE;
			
			switch(nParamId)
			{
				// 0x10 段参数
				case 1001:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam1001;
					break;
				case 1002:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam1002;
					break;

				// 0x11 段参数
				case 1110:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam1110;
					break;
				case 1127:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam1127;
					break;
				case 1133:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].fParam1133;
					break;

				// 0x20 段参数
				case 2003:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2003;
					break;

				// 0x21 段参数
				case 2101:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2101;
					break;
				case 2102:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2102;
					break;
				case 2103:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2103;
					break;
				case 2105:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2105;
					break;
				case 2106:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam2106;
					break;
				case 2107:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam2107;
					break;
				case 2110:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2110;
					break;
				case 2111:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2111;
					break;
				case 2114:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2114;
					break;
				case 2115:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2115;
					break;
				case 2117:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2117;
					break;
				case 2118:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2118;
					break;
				case 2120:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2120;
					break;
				case 2121:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2121;
					break;
				case 2122:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2122;
					break;
				case 2123:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2123;
					break;
				case 2124:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2124;
					break;
				case 2125:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2125;
					break;
				case 2126:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2126;
					break;
				case 2127:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2127;
					break;
				case 2128:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2128;
					break;
				case 2129:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2129;
					break;
				case 21210:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21210;
					break;
				case 21211:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21211;
					break;
				case 21212:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21212;
					break;
				case 21213:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21213;
					break;
				case 21214:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21214;
					break;
				case 21215:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam21215;
					break;
				case 2130:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam2130;
					break;
				case 2140:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam2140;
					break;
				case 2141:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam2141;
					break;
				case 2142:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam2142;
					break;
				case 2143:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam2143;
					break;
				case 2144:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam2144;
					break;
				case 2145:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2145;
					break;
				case 2146:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam2146;
					break;

				// 0x31 段参数
				case 3111:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam3111;
					break;
				case 3112:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam3112;
					break;
				case 3113:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3113;
					break;
				case 3114:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam3114;
					break;
				case 3115:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3115;
					break;
				case 3116:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam3116;
					break;
				case 3117:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam3117;
					break;
				case 3118:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3118;
					break;
				case 3119:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3119;
					break;
				case 31181:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3118;
					break;
				case 31191:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3119;
					break;
				case 3121:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3121;
					break;
				case 3122:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3122;
					break;
				case 3123:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3123;
					break;
				case 3124:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3124;
					break;
				case 3125:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3125;
					break;
				case 3126:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3126;
					break;
				case 3127:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3127;
					break;
				case 3128:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3128;
					break;
				case 31281:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31281;
					break;
				case 3129:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3129;
					break;
				case 31211:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31211;
					break;
				case 3131:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3131;
					break;
				case 3132:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3132;
					break;
				case 3133:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3133;
					break;
				case 3135:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3135;
					break;
				case 3136:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3136;
					break;
				case 3138:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3138;
					break;
				case 3150:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3150;
					break;
				case 3151:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3151;
					break;
				case 3152:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3152;
					break;
				case 3153:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3153;
					break;
				case 3154:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3154;
					break;
				case 3155:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3155;
					break;
				case 3156:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3156;
					break;
				case 3157:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3157;
					break;
				case 3160:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3160;
					break;
				case 3161:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam3161;
					break;
				case 3200:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam3161;
					break;
				case 3163:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3163;
					break;
				case 3164:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3164;
					break;
				case 3165:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3165;
					break;
				case 3166:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3166;
					break;
				case 3167:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3167;
					break;
				case 3168:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3168;
					break;
				case 3169:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3169;
					break;
				case 31610:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31610;
					break;
				case 31611:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31611;
					break;
				case 31612:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31612;
					break;
				case 31613:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31613;
					break;
				case 31614:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31614;
					break;
				case 31615:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31615;
					break;
				case 3173:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3173;
					break;
				case 3175:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3175;
					break;
				case 31751:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3175 * 10;
					break;
				case 3174:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3174;
					break;
				case 31741:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3174 * 10;
					break;
				case 3176:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3176;
					break;
				case 3177:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3177;
					break;
				case 3178:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3178;
					break;
				case 31781:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3178;
					break;
				case 3179:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3179;
					break;
				case 31710:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31710;
					break;
				case 31711:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31711;
					break;
				case 31712:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31712;
					break;
				case 31713:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31713;
					break;
				case 31714:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31714;
					break;
				case 317141:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31714;
					break;
				case 31110:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31110;
					break;
				case 31111:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31111;
					break;
				case 311101:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31110;
					break;
				case 311111:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31111;
					break;
				case 31113:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31113;
					break;
				case 31121:
					varV.ulValue = pCCU->PCSRereadParam[paramIndex].uParam31121;
					break;
				case 31130:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31130;
					break;
				case 31131:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31131;
					break;
				case 31132:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31132;
					break;
				case 31133:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31133;
					break;
				case 31136:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31136;
					break;
				case 31138:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31138;
					break;
				case 31139:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31139;
					break;
				case 31156:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31156;
					break;
				case 311561:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam311561;
					break;
				case 31157:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31157;
					break;
				case 31158:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam31158;
					break;
				case 31159:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].fParam31159;
					break;
				case 311512:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam311512;
					break;
				case 31310:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31310;
					break;
				case 31315:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31315;
					break;
				case 313151:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam313151;
					break;
				case 31914:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31914;
					break;
				case 31915:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31915;
					break;
				case 3111111:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3111111;
					break;
				case 31243:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31234;
					break;
				case 31250:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31250;
					break;
				case 31251:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31251;
					break;
				case 311313:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam311313;
					break;
				case 311315:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam311315;
					break;

				// 0x51 段参数
				case 5101:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam5101;
					break;
				case 5102:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam5102;
					break;
				case 5103:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam5103;
					break;
				case 5104:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam5104;
					break;
				case 5105:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam5105;
					break;

				// 0x71 段参数
				case 7140:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam7140;
					break;
				case 71411:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam71411;
					break;
				case 71412:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam71412;
					break;

				// 0x31 补充段参数
				case 3180:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3180;
					break;
				case 3181:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3181;
					break;
				case 3182:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3182;
					break;
				case 3183:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3183;
					break;
				case 3184:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3184;
					break;
				case 3185:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3185;
					break;
				case 3186:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3186;
					break;
				case 3187:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3187;
					break;
				case 3188:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3188;
					break;
				case 3189:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3189;
					break;
				case 31810:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31810;
					break;
				case 31811:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam31811;
					break;
				case 31812:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam31812;
					break;
				case 31813:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31813;
					break;
				case 31814:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31814;
					break;
				case 3190:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3190;
					break;
				case 3191:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3191;
					break;
				case 3192:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3192;
					break;
				case 3193:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3193;
					break;
				case 3194:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3194;
					break;
				case 3195:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3195;
					break;
				case 3196:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3196;
					break;
				case 3197:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3197;
					break;
				case 3198:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3198;
					break;
				case 3199:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3199;
					break;
				case 31910:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31910;
					break;
				case 31911:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31911;
					break;
				case 31912:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam31912;
					break;
				case 311011:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].bParam311011;
					break;

				// 0x32 段参数
				case 32000:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam32001;
					break;
				case 3201:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3201;
					break;
				case 3202:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3202;
					break;
				case 3203:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3203;
					break;
				case 3204:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3204;
					break;
				case 3205:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3205;
					break;
				case 3206:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3206;
					break;
				case 3207:
					varV.fValue = pCCU->PCSRereadParam[paramIndex].fParam3207;
					break;
				
				// 0x91 段参数
				case 91011:
					varV.emValue = pCCU->PCSRereadParam[paramIndex].nParam91011;
					break;

				// 未匹配到的参数
				default:
					TRACEX("Unsupported param ID: %d for device %d\n", nParamId, nDevIdx);
					bParamMatched = FALSE;
					break;
			}

			if(bParamMatched)
			{
				Push_VarValue_RereadPCS(SETTYPE_Reread_PCS_Param, pCCU->hSampler, nDevIdx, nParamId, varV, paramIndex);
			}
		}
	}
	return;
}

// LJH241202 SEMS rundata
static int Find_LastSavedClientIdx_SEMS(ROUGH_CCU_DATA* pCCU, int idx)
{
	HIS_BMS_RUNTIMEDATA_REC* pRec = &pCCU->hisBmsRunDataRec;
	int n = 0;
	int nCurr_Position = (int)(pRec->bmsRunData.nBmsRuntimeDataTotal - 1);
	nCurr_Position = MAX(0, nCurr_Position);
	for(n = nCurr_Position; n > 0; n--)
	{
		A_BMS_RUNTIME_DATA_ITEM* pOldItem = &pRec->bmsRunData.stBmsRuntimeDataList[n];
		if(idx == pOldItem->nBmsIdx)
		{
			break;
		}
	}
	return n;
}
// LJH241202 SEMS rundata
static BOOL IsNeedUpdate_RunDataList_SEMS(ROUGH_CCU_DATA* pCCU, int idx)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
 	HIS_BMS_RUNTIMEDATA_REC* pRec = &pCCU->hisBmsRunDataRec;
 	A_BLOB_ES_RUN_DATA *pESRunData = &pCCU->esRunData;
	RunDataExt_SEMS* pRunDataExt_SEMS = &pCCU->runDataExt_SEMS;
	A_BLOB_SEMS_CLIENTINFO* pClientInfo = &s_MMClients[idx];
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData_SEMS = &pClientInfo->branchNodeData_SEMS;
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node = &pEMS->calc_Node[idx];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS = &pBranchNodeData_SEMS->b6LSYSLast_SEMS;
	A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pEMS->clientInfoSum;

	pRunDataExt_SEMS->fGridMeter_Volt[0] = pESRunData->fGrid_ACVolt_L1;
 	pRunDataExt_SEMS->fGridMeter_Volt[1] = pESRunData->fGrid_ACVolt_L2;
	pRunDataExt_SEMS->fGridMeter_Volt[2] = pESRunData->fGrid_ACVolt_L3;

	pRunDataExt_SEMS->fGridMeter_Curr[0] = pESRunData->fGrid_ACCurr_L1;
	pRunDataExt_SEMS->fGridMeter_Curr[1] = pESRunData->fGrid_ACCurr_L2;
	pRunDataExt_SEMS->fGridMeter_Curr[2] = pESRunData->fGrid_ACCurr_L3;

	pRunDataExt_SEMS->f_QgridMeter = pESRunData->f_QgridMeter;
	pRunDataExt_SEMS->f_QcabMeter = pESRunData->f_QcabMeter;   

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 411, &varV, 4);
	pRunDataExt_SEMS->fGridMeter_Power[0] = varV.fValue;
	
	pRunDataExt_SEMS->fGridMeter_Power[1] = pESRunData->f_PgridMeter;

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 422, &varV, 4);//Revs KWH
	pRunDataExt_SEMS->fGridMeter_Power[2] = varV.fValue;

	pRunDataExt_SEMS->fCabMeter_Volt[0] = pESRunData->fACVolt_L1; 
	pRunDataExt_SEMS->fCabMeter_Volt[1] = pESRunData->fACVolt_L2; 
	pRunDataExt_SEMS->fCabMeter_Volt[2] = pESRunData->fACVolt_L3; 

	pRunDataExt_SEMS->fCabMeter_Curr[0] = pESRunData->fACCurr_L1; 
	pRunDataExt_SEMS->fCabMeter_Curr[1] = pESRunData->fACCurr_L2; 
	pRunDataExt_SEMS->fCabMeter_Curr[2] = pESRunData->fACCurr_L3; 

	pRunDataExt_SEMS->fCabMeter_Power[1] = pESRunData->f_PcabMeter;

 	// if(pRec->bmsRunData.nBmsRuntimeDataTotal < 1)
 	// {
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = 0;
 	// 	return TRUE;
 	// }

	// if(pRec->bmsRunData.nBmsRuntimeDataTotal > MAX_HIS_BMS_RUNTIME_DATALIST)
	// {
	// 	TRACEW("IsNeedUpdate_RunDataList_SEMS MAX_HIS_BMS_RUNTIME_DATALIST idx=[%d] nBmsRuntimeDataTotal=[%d]\n", idx, pRec->bmsRunData.nBmsRuntimeDataTotal);
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = MAX_HIS_BMS_RUNTIME_DATALIST;
	// }

	int nLastSavePosition = Find_LastSavedClientIdx_SEMS(pCCU, idx);
 	A_BMS_RUNTIME_DATA_ITEM* pOldItem = &pRec->bmsRunData.stBmsRuntimeDataList[nLastSavePosition];
	
	if (pOldItem->bCommFailSemsClient != pClientInfo->bCommFailSemsClient)
	{
		return TRUE;
	}
	// if(ConvDateTime_2DateOnly(_NOW_) != ConvDateTime_2DateOnly(pOldItem->tmSaved))
	// {
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = 0; //If Day Changed,Need Init nBmsRuntimeDataTotal
	// 	return TRUE;
	// }
	if (pOldItem->emCtrlPcsCmd == GRP_ACDC_STOP && pCalc_Node->emBatt_WorkMode == BATT_WORKMODE_IDLE &&
		ABS(_NOW_ - pOldItem->tmSaved)<180) // 空闲时每x秒记录一次 3′
	{
		TRACEW("IsNeedUpdate_RunDataList_SEMS return FALSE emBatt_WorkMode:%d tm:%d idx:%d",
			pCalc_Node->emBatt_WorkMode, ABS(_NOW_ - pOldItem->tmSaved), idx);
		return FALSE;
	}

	//----- 3-pahse volt -----//
 	float fDiff_A = pRunDataExt_SEMS->fCabMeter_Volt[0] - pOldItem->fCabMeter_Volt[0];
 	float fDiff_B = pRunDataExt_SEMS->fCabMeter_Volt[1] - pOldItem->fCabMeter_Volt[1];
 	float fDiff_C = pRunDataExt_SEMS->fCabMeter_Volt[2] - pOldItem->fCabMeter_Volt[2];
 	if((fabs(fDiff_A) >= 50) || (fabs(fDiff_B) >= 50) || (fabs(fDiff_C) >= 50))
 	{
		TRACEI("IsNeedUpdate_RunDataList_SEMS return TRUE Info-VoltDiff: fDiff_A=[%.1f],fDiff_B=[%.1f],fDiff_C=[%.1f] idx:%d",
			fDiff_A,fDiff_B,fDiff_C, idx);
 		return TRUE;
 	}

 	//----- 3-pahse curr -----//
 	fDiff_A = pRunDataExt_SEMS->fCabMeter_Curr[0] - pOldItem->fCabMeter_Curr[0];
 	fDiff_B = pRunDataExt_SEMS->fCabMeter_Curr[1] - pOldItem->fCabMeter_Curr[1];
 	fDiff_C = pRunDataExt_SEMS->fCabMeter_Curr[2] - pOldItem->fCabMeter_Curr[2];
 	if((fabs(fDiff_A) >= 50) || (fabs(fDiff_B) >= 50) || (fabs(fDiff_C) >= 50))
 	{
		TRACEI("IsNeedUpdate_RunDataList_SEMS return TRUE Info-CurrentDiff: fDiff_A=[%.1f],fDiff_B=[%.1f],fDiff_C=[%.1f] idx:%d",
			fDiff_A,fDiff_B,fDiff_C, idx);
 		return TRUE;
 	}
	//------ 4-judge if BMS data update-----//
	// UINT nSaveCycle = MAX(10, pEMS->nNodeNum*10);
	int nSaveCycle = 60;
	TRACED("IsNeedUpdate_RunDataList_SEMS idx:%d emEMSCtrlMode_REMS:%d fActvPwrReq_REMS:%d f_PcabMeterSum:%d emBatt_WorkMode:%d nSOC:%d fMax_Chrg_PwrAbility:%d "
			"fMax_DisChrg_PwrAbility:%d fChrg_Energy:%d fAllow_MaxChargeCurr:%d fPMOutput_Volt:%d fPMOutput_Curr:%d tm:%d",
		idx, pOldItem->emEMSCtrlMode_REMS != pClientSum->emEMSCtrlMode_REMS,
		fabs(pOldItem->fActvPwrReq_REMS - pClientSum->fActvPwrReq_REMS) > 10,
		fabs(pOldItem->f_PcabMeter_Client - pB6LSYSLast_SEMS->f_PcabMeterSum) > 10,
		pOldItem->emCtrlPcsCmd != MapBattWorkMode2GrpRectCtrlCmdType(pCalc_Node->emBatt_WorkMode),
		ABS(pOldItem->nSOC - (BYTE)pB6LSYSLast_SEMS->fSOCSum) >= 1,
		fabs(pOldItem->fMax_Chrg_PwrAbility - pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum) > 20,
		
		fabs(pOldItem->fMax_DisChrg_PwrAbility - pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum) > 20,
		fabs(pOldItem->fChrg_Energy - pB6LSYSLast_SEMS->fChrgedEnergySum) > 10,
		fabs(pOldItem->fAllow_MaxChargeCurr - pB6LSYSLast_SEMS->fRealTime_MaxCurrSum) > 20,
		fabs(pOldItem->fPMOutput_Volt - pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum) > 10,
		fabs(pOldItem->fPMOutput_Curr - pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum) > 20,
		ABS(_NOW_ - pOldItem->tmSaved) >= nSaveCycle);

 	if (pOldItem->emEMSCtrlMode_REMS != pClientSum->emEMSCtrlMode_REMS ||
		fabs(pOldItem->fActvPwrReq_REMS - pClientSum->fActvPwrReq_REMS) > 10 ||
		fabs(pOldItem->f_PcabMeter_Client - pB6LSYSLast_SEMS->f_PcabMeterSum) > 10 ||
		pOldItem->emCtrlPcsCmd != MapBattWorkMode2GrpRectCtrlCmdType(pCalc_Node->emBatt_WorkMode) ||
		ABS(pOldItem->nSOC - (BYTE)pB6LSYSLast_SEMS->fSOCSum) >= 1 ||
		fabs(pOldItem->fMax_Chrg_PwrAbility - pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum) > 20 ||
		
		fabs(pOldItem->fMax_DisChrg_PwrAbility - pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum) > 20 ||
		fabs(pOldItem->fChrg_Energy - pB6LSYSLast_SEMS->fChrgedEnergySum) > 10 ||
		fabs(pOldItem->fAllow_MaxChargeCurr - pB6LSYSLast_SEMS->fRealTime_MaxCurrSum) > 20 ||
		fabs(pOldItem->fActvPwr_PCSGrp - pB6LSYSLast_SEMS->fActvPwr_PCSGrpSum) > 10 ||
		fabs(pOldItem->fPMOutput_Volt - pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum) > 10 ||
		fabs(pOldItem->fPMOutput_Curr - pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum) > 20 ||
		ABS(_NOW_ - pOldItem->tmSaved) >= nSaveCycle)
 	{
		TRACEI("IsNeedUpdate_RunDataList_SEMS return TRUE idx:%d emBattStatus_Run=[%d],fPMOutput_Volt=[%.1f],fACDC_RealTime_CurrSum=[%.1f]\n",
			idx, MapACDC_WorkStatu2BattWorkMode(pB6LSYSLast_SEMS->emACDC_WorkStatuSum),
			pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum, pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum);
 		return TRUE;
 	}
 	return FALSE;
}

// LJH241202 SEMS rundata Update_BmsRuntimeData_2DB
void Update_RuntimeData_2DB_SEMS(ROUGH_CCU_DATA* pCCU, int idx)
{
	BOOL bSaveFlag = FALSE;
	static 	BOOL bFirstRun = TRUE;
	int 	n = 0;

	//John, Sometimes After Reboot,BmsRunDataRec Lost.
	if(bFirstRun == TRUE)
	{
		if(ABS(_NOW_ - pCCU->tmSysBoot) > 30)//should wait 30S 
		{
			//---Need Wait Event_Handler Restore BmsStaticData---//
			bFirstRun = FALSE;
		}
		return;
	}
	if (pCCU->bCommFail)
	{
		return;
	}
	if(!IsNeedUpdate_RunDataList_SEMS(pCCU, idx))
 	{
		//printf("Warning: No NeedUpdate_bmsRunDataList!\n");
 		return;
 	}
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
 	HIS_BMS_RUNTIMEDATA_REC* pRec = &pCCU->hisBmsRunDataRec;
 	A_BMS_RUNTIME_DATA_ITEM* pItem = NULL;//pRec->bmsRunData.stBmsRuntimeDataList[pRec->bmsRunData.nBmsRuntimeDataTotal];
 	// A_BLOB_ES_RUN_DATA *pESRunData = &pCCU->esRunData;
	RunDataExt_SEMS* pRunDataExt_SEMS = &pCCU->runDataExt_SEMS;

	A_BLOB_SEMS_CLIENTINFO* pClientInfo = &s_MMClients[idx];
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData_SEMS = &pClientInfo->branchNodeData_SEMS;
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node = &pEMS->calc_Node[idx];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS = &pBranchNodeData_SEMS->b6LSYSLast_SEMS;

	int nIdx = (int)pRec->bmsRunData.nBmsRuntimeDataTotal;
	if(nIdx >= MAX_HIS_BMS_RUNTIME_DATALIST - 1)
 	{
 		bSaveFlag = TRUE;
 	}
 	else
 	{
 		if(nIdx <= 0)
 		{
			pRec->tmLogTime = _NOW_; //当前运行记录保存开始时间是多少
			pRec->bmsRunData.nBmsRuntimeDataTotal = 1;	
 			nIdx = 0;
 		}
		else
		{
			pRec->bmsRunData.nBmsRuntimeDataTotal++;	
		}	
 	}
 	pItem = &pRec->bmsRunData.stBmsRuntimeDataList[nIdx];

 	ZERO_POBJS(pItem, 1);
 	//--------------------Step1. Cabinet AC Meter Values-------------------//
	pItem->nBmsIdx = (BYTE)idx;
	pItem->bCommFailSemsClient = (BYTE)pClientInfo->bCommFailSemsClient;
	pItem->emEMSCtrlMode_REMS =    (BYTE)pEMS->clientInfoSum.emEMSCtrlMode_REMS;
	pItem->emACDC_WorkStatu_REMS = (BYTE)pEMS->clientInfoSum.emACDC_WorkStatu_REMS;
	pItem->fActvPwrReq_REMS =      pEMS->clientInfoSum.fActvPwrReq_REMS;
	pItem->f_PcabMeter_Client =  pB6LSYSLast_SEMS->f_PcabMeterSum;
	//---------------User Grid AC Meter RealTime DataInfo------------------//
	pItem->fGridMeter_Volt[0] = pRunDataExt_SEMS->fGridMeter_Volt[0];
 	pItem->fGridMeter_Volt[1] = pRunDataExt_SEMS->fGridMeter_Volt[1];
 	pItem->fGridMeter_Volt[2] = pRunDataExt_SEMS->fGridMeter_Volt[2];
 	
   	pItem->fGridMeter_Curr[0] = pRunDataExt_SEMS->fGridMeter_Curr[0];
 	pItem->fGridMeter_Curr[1] = pRunDataExt_SEMS->fGridMeter_Curr[1];
 	pItem->fGridMeter_Curr[2] = pRunDataExt_SEMS->fGridMeter_Curr[2];
	
   	pItem->fGridMeter_Power[0] = pRunDataExt_SEMS->fGridMeter_Power[0];
 	pItem->fGridMeter_Power[1] = pRunDataExt_SEMS->fGridMeter_Power[1];
 	pItem->fGridMeter_Power[2] = pRunDataExt_SEMS->fGridMeter_Power[2];
 	//------------------Cabinet AC Meter RealTime DataInfo-----------------//
  	pItem->fCabMeter_Volt[0] = pRunDataExt_SEMS->fCabMeter_Volt[0];
 	pItem->fCabMeter_Volt[1] = pRunDataExt_SEMS->fCabMeter_Volt[1];
 	pItem->fCabMeter_Volt[2] = pRunDataExt_SEMS->fCabMeter_Volt[2];
 	
  	pItem->fCabMeter_Curr[0] = pRunDataExt_SEMS->fCabMeter_Curr[0];
 	pItem->fCabMeter_Curr[1] = pRunDataExt_SEMS->fCabMeter_Curr[1];
 	pItem->fCabMeter_Curr[2] = pRunDataExt_SEMS->fCabMeter_Curr[2];

	pItem->fCabMeter_Power[0] = pRunDataExt_SEMS->fCabMeter_Power[0];
 	pItem->fCabMeter_Power[1] = pRunDataExt_SEMS->fCabMeter_Power[1];
 	pItem->fCabMeter_Power[2] = pRunDataExt_SEMS->fCabMeter_Power[2];
	//-----------------------Battery RealTime Data-------------------------//
	pItem->emBattStatus_Run =     MapBattWorkMode2GrpRectCtrlCmdType(pB6LSYSLast_SEMS->emACDC_WorkStatuSum);
	pItem->nSOC =                 (BYTE)pB6LSYSLast_SEMS->fSOCSum;
    pItem->fAllow_MaxChargeCurr = pB6LSYSLast_SEMS->fRealTime_MaxCurrSum;
	pItem->fChrg_Energy = pB6LSYSLast_SEMS->fChrgedEnergySum;
	pItem->fQ_PCSGrp =    pB6LSYSLast_SEMS->fQ_PCSGrpSum;
	pItem->f_QgridMeter = pRunDataExt_SEMS->f_QgridMeter;
	pItem->f_QcabMeter =  pRunDataExt_SEMS->f_QcabMeter; 
	//---------- PM Relevant DataInfo ----------//
	pItem->emCtrlPcsCmd =    (BYTE)MapBattWorkMode2GrpRectCtrlCmdType(pCalc_Node->emBatt_WorkMode);
	pItem->fActvPwr_PCSGrp = pB6LSYSLast_SEMS->fActvPwr_PCSGrpSum;
 	pItem->fPMOutput_Volt =  pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum;
    pItem->fPMOutput_Curr =  pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum;
	pItem->byPCSStatu = pB6LSYSLast_SEMS->emACDC_WorkStatuSum;
	pItem->fAvg_Calc_Chg_Power = pCalc_Node->fAvg_ChrgLmtPower;
	pItem->fCalc_Chg_Power = pCalc_Node->fACDC_Distri_4_BattChrg_Pwr;
	pItem->fAvg_Calc_DisChg_Power = pCalc_Node->fAvg_DisChrgingLmtPower;
	pItem->fCalc_DisChg_Power = pCalc_Node->fBatt_Distri_4_ACDC_Pwr;
	pItem->f_PauxMeter = pCCU->esRunData.f_PauxMeter;
	pItem->fPacload4Chg = pCCU->esRunData.fPacload4Chg;  
	pItem->fPacload4Dischg = pCCU->esRunData.fPacload4Dischg; 
	pItem->fTotal_PessDischgLmt = pCCU->esRunData.fTotal_PessDischgLmt;
	pItem->fTotal_PessChgAvaliable = pCCU->esRunData.fTotal_PessChgAvaliable;
	pItem->f_PmaxGridDemand = pCCU->esRunData.f_PmaxGridDemand;
	pItem->f_PauxMeter_3 = pCCU->esRunData.f_PauxMeter_3;
	pItem->bySTSSysStatu = pCCU->esRunData.mapSTSCode.byRunStatus;
	// LJH240925 RunData // LJH250610 fPwrAbility0
	pItem->fMax_Chrg_PwrAbility =     pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum;
	pItem->fMax_DisChrg_PwrAbility =  pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum;
	pItem->fMax_Chrg_PwrAbility0 =    pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum0;
	pItem->fMax_DisChrg_PwrAbility0 = pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum0;
	// pItem->fACDC_Max_Output_Pwr_Ability =
	// 	MAX(pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum);
	pItem->fACDC_Max_Output_Pwr_Ability = 0;
	pItem->antiMode[0] = (BYTE)pCCU->esRunData.bEnterAnti_Reflux;
	pItem->antiMode[1] = (BYTE)pCCU->esRunData.bEnter_OverLoad;
	pItem->antiMode[2] = (BYTE)pCCU->esRunData.bEnter_OverLoadHV;
	// LJH241007 AntiPhase RunData
	pItem->fGridRemainACPwr2Chrg = pCCU->esRunData.fGridRemainACPwr2Chrg;
	pItem->fRemainPwr4Chrg =       pCCU->esRunData.fRemainPwr4Chrg;
	pItem->fRemainDemand4Chrg =    pCCU->esRunData.fRemainDemand4Chrg;
	for(n = 0; n < MAX_PHASE_NUM; n++)
	{
		pItem->fAtRflxMtr_kWPhs[n] = pCCU->esRunData.fAtRflxMtr_kWPhs[n];
		pItem->fAtOvlMtr_APhs[n] =   pCCU->esRunData.fAtOvlMtr_APhs[n];
	}
	pItem->fAtRflxMtr_Factor =       pCCU->esRunData.fAtRflxMtr_Factor;
	pItem->fDiffPwr4Dischrg_SumPhs = pCCU->esRunData.fDiffPwr4Dischrg_SumPhs;
	pItem->fAtOvlMtr_Factor =        pCCU->esRunData.fAtOvlMtr_Factor;
	pItem->fMaxPwr4Chrg_SumPhs =     pCCU->esRunData.fMaxPwr4Chrg_SumPhs;
	pItem->antiMode_Phase[0] =       (BYTE)pCCU->esRunData.bEnterAnti_Reflux_Phase;
	pItem->antiMode_Phase[1] =       (BYTE)pCCU->esRunData.bEnter_OverLoad_Phase;
	pItem->autoChrgDischrg[0] =      (BYTE)pCCU->esRunData.bEnter_AutoChrg_SEMS;
	pItem->autoChrgDischrg[1] =      (BYTE)pCCU->esRunData.bEnter_AutoDisc_SEMS;
	pItem->fAtRflx_kW =              pCCU->esRunData.fAtRflx_kW;
	pItem->fAtOvl_kW =               pCCU->esRunData.fAtOvl_kW;
 	//now calculate the tmSampled
  	pItem->tmSaved = _NOW_; 
	//now sendmsg => db

	BOOL  bisCrossDay = FALSE;
	if(ConvDateTime_2DateOnly(_NOW_) != ConvDateTime_2DateOnly(pRec->tmLogTime) && (pRec->bmsRunData.nBmsRuntimeDataTotal > 1))
	{
		bisCrossDay = TRUE;
	}

	//换天 或者  当前保存的 到达了限制总数  保存一次
	if(bisCrossDay || bSaveFlag)
	{
		if(bisCrossDay && (pRec->bmsRunData.nBmsRuntimeDataTotal > 1))
		{
			pRec->bmsRunData.nBmsRuntimeDataTotal -= 1; //换天最后会残留第二天的一条数据，去除掉
		}
		TRACEI("----Update_RuntimeData_2DB_SEMS Save to DB nBmsRuntimeDataTotal=[%d] bSaveFlag=[%d] bisCrossDay=[%d]----\n",
			pRec->bmsRunData.nBmsRuntimeDataTotal, bSaveFlag,
			bisCrossDay);
			
 		EVENT_BMSRUNDATA_UPDATE bmsRunDataUpdt;

		//使用深拷贝，防止发送给事件处理后，没有及时处理导致数据被篡改
		//!!!!!!! 在事件处理后需要释放内存 !!!!!!!  
		bmsRunDataUpdt.pBmsRunDataRec = (HIS_BMS_RUNTIMEDATA_REC*) malloc(sizeof(HIS_BMS_RUNTIMEDATA_REC));
		if(bmsRunDataUpdt.pBmsRunDataRec == NULL)
		{
			TRACEI("Failed to allocate memory for BMS runtime data record\n");
			return;
		}
		memcpy(bmsRunDataUpdt.pBmsRunDataRec, &pCCU->hisBmsRunDataRec, sizeof(HIS_BMS_RUNTIMEDATA_REC));

		// EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_STATUS_CHANGED
 		SendMsg(pCCU->hSampler, EVENT_TYPE_BMSRUNDATA_UPDATE, EVENT_MSG_STATUS_CHANGED,
 			&bmsRunDataUpdt, sizeof(EVENT_BMSRUNDATA_UPDATE));
		TRACEI("---SendMsg EVENT_TYPE_BMSRUNDATA_UPDATE---tmLastWriteRec=[%ld]\n",pRec->tmLastWriteRec);	
		pRec->bmsRunData.nBmsRuntimeDataTotal = 0;  
		pRec->tmLastWriteRec = _NOW_;
	}

	if(bisCrossDay)
	{
		pRec->tmLogTime = _NOW_; //当前运行记录保存开始时间是多少
		pRec->bmsRunData.nBmsRuntimeDataTotal = 1;	
		memcpy(&pRec->bmsRunData.stBmsRuntimeDataList[0], pItem, sizeof(A_BMS_RUNTIME_DATA_ITEM));
	}

	return;
}