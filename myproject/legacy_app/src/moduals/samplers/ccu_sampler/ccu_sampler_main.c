// #define _TRACE_PRINT_ // SEMS
#include "ccu_sampler_main.h"
#include "sems_sampler_main.h"

/***************************************************
* 1、适用对象:普通工业及大量用电以及高需求用户
* 2、峰谷时段
*高峰时段（9:00—11:30、14:00—16:30、19:00—21:00）
*平时段（7:00—9:00、11:30—14:00、16:30—19:00、21:00—23:00）
*低谷时段（23:00—次日7:00）
*峰值电价：1.09元/kwh，
*平值电价：0.6648元/kwh，
*谷值电价：0.337元/kwh
****************************************************/
BOOL Init_blobEmsCfgPlanParam(ROUGH_CCU_DATA *pCCU)
{
	//now get the batt management plan
	int 					n = 0;
	VAR_VALUE 				varV, varV_GH;
	BOOL 					bNeedUpdtMgmtPlan = FALSE;
	//now use the default plan
	A_BLOB_EMS_CONFIG_PLAN 	emsPlan;
	ZERO_POBJS(&emsPlan, 	1);

	//ZERO_POBJS(&pCCU->blobEmsCfgPlan, 1);
	varV.blobValue = (BYTE *)&pCCU->blobEmsCfgPlan;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
	Save_EmsMgmtPlan(&pCCU->blobEmsCfgPlan);

	TRACEX("nTotal_EmsLmt_Sectors=%d\n", pCCU->blobEmsCfgPlan.nTotal_EmsLmt_Sectors);

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR,  1 , 87, &varV_GH, 4);
	if((varV_GH.emValue == GH_CUSTOM) && (pCCU->blobEmsCfgPlan.nEMSMgmt_Ver == 2102))
	{
		//远程控制版本时不要初始化
		return TRUE;
	}
 	//judge if the plan need to be initialized
	if(pCCU->blobEmsCfgPlan.nTotal_EmsLmt_Sectors < 1)
	{
		//default plan has 2 sectors:
		emsPlan.nTotal_EmsLmt_Sectors = 9;

		//1: 00:00—07:00
		emsPlan.emsLmtSectors[0].tmStart = StringToTime("2000-01-01 00:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[0].nInterval = 3600 * 7;
		emsPlan.emsLmtSectors[0].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[0].emFeeRate = FEERATE_VALLEY;
		emsPlan.emsLmtSectors[0].nStopSOC = 50;
		emsPlan.emsLmtSectors[0].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[0].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[0].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[0].nGridInputPwrLmt = 0;
		
		//2: 7:00—9:00 平
		emsPlan.emsLmtSectors[1].tmStart = StringToTime("2000-01-01 07:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[1].nInterval = 3600 * 2;
		emsPlan.emsLmtSectors[1].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[1].emFeeRate = FEERATE_FLAT;
		emsPlan.emsLmtSectors[1].nStopSOC = 50;
		emsPlan.emsLmtSectors[1].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[1].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[1].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[1].nGridInputPwrLmt = 0;

		//3:9:00—11:30 峰
		emsPlan.emsLmtSectors[2].tmStart = StringToTime("2000-01-01 09:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[2].nInterval = 3600 * 2.5;
		emsPlan.emsLmtSectors[2].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;
		emsPlan.emsLmtSectors[2].emFeeRate = FEERATE_PEAK;
		emsPlan.emsLmtSectors[2].nStopSOC = 50;
		emsPlan.emsLmtSectors[2].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[2].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[2].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[2].nGridInputPwrLmt = 0;

		//4. 11:30—14:00 平
 		emsPlan.emsLmtSectors[3].tmStart = StringToTime("2000-01-01 11:30:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[3].nInterval = 3600 * 2.5;
		emsPlan.emsLmtSectors[3].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[3].emFeeRate = FEERATE_FLAT;
		emsPlan.emsLmtSectors[3].nStopSOC = 50;
		emsPlan.emsLmtSectors[3].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[3].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[3].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[3].nGridInputPwrLmt = 0;

		//5:14:00—16:30 峰
		emsPlan.emsLmtSectors[4].tmStart = StringToTime("2000-01-01 14:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[4].nInterval = 3600 * 2.5;
		emsPlan.emsLmtSectors[4].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;
		emsPlan.emsLmtSectors[4].emFeeRate = FEERATE_PEAK;
		emsPlan.emsLmtSectors[4].nStopSOC = 50;
		emsPlan.emsLmtSectors[4].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[4].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[4].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[4].nGridInputPwrLmt = 0;

		//6. 16:30—19:00 平
 		emsPlan.emsLmtSectors[5].tmStart = StringToTime("2000-01-01 16:30:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[5].nInterval = 3600 * 2.5;
		emsPlan.emsLmtSectors[5].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[5].emFeeRate = FEERATE_FLAT;
		emsPlan.emsLmtSectors[5].nStopSOC = 50;
		emsPlan.emsLmtSectors[5].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[5].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[5].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[5].nGridInputPwrLmt = 0;

		//7:19:00—21:00 峰
 		emsPlan.emsLmtSectors[6].tmStart = StringToTime("2000-01-01 19:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[6].nInterval = 3600 * 2;
		emsPlan.emsLmtSectors[6].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;
		emsPlan.emsLmtSectors[6].emFeeRate = FEERATE_PEAK;
		emsPlan.emsLmtSectors[6].nStopSOC = 50;
		emsPlan.emsLmtSectors[6].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[6].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[6].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[6].nGridInputPwrLmt = 0;

		//8:21:00-23:00 平
 		emsPlan.emsLmtSectors[7].tmStart = StringToTime("2000-01-01 21:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[7].nInterval = 3600 * 2;
		emsPlan.emsLmtSectors[7].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[7].emFeeRate = FEERATE_FLAT;
		emsPlan.emsLmtSectors[7].nStopSOC = 50;
		emsPlan.emsLmtSectors[7].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[7].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[7].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[7].nGridInputPwrLmt = 0;

		//9: 23:00—00:00 谷
		emsPlan.emsLmtSectors[8].tmStart = StringToTime("2000-01-01 23:00:00", TIME_CHN_FMT);
		emsPlan.emsLmtSectors[8].nInterval = 3600 * 1;
		emsPlan.emsLmtSectors[8].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING;
		emsPlan.emsLmtSectors[8].emFeeRate = FEERATE_VALLEY;
		emsPlan.emsLmtSectors[8].nStopSOC = 50;
		emsPlan.emsLmtSectors[8].nActvPwrLmt = 0;
		emsPlan.emsLmtSectors[8].nReActvPwrLmt = 0;
		emsPlan.emsLmtSectors[8].nBattGrpPwrLmt = 0;
		emsPlan.emsLmtSectors_ext[8].nGridInputPwrLmt = 0;

		bNeedUpdtMgmtPlan = TRUE;
		TRACE("Need Update EMS Config Plan!\n");
	}

	if(bNeedUpdtMgmtPlan)
	{
		Push_BlobValue(SETTYPE_RAW_ST, (HANDLE)pCCU->hSampler, 200, 201, 
				(void*)&emsPlan, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
		memcpy(&pCCU->blobEmsCfgPlan, &emsPlan, sizeof(A_BLOB_EMS_CONFIG_PLAN));
	}
	return TRUE;
}


void Init_EmsMgmtData(ROUGH_CCU_DATA* pCCU)
{
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	time_t tm_Now = _NOW_;
	int n;
	VAR_VALUE varV;
    ERRCDDE_DPR err;

	pEMS->bEnterUrgetDischrgMode = FALSE;
	
 	pEMS->tm_LastEnterUrgetMode = tm_Now;
	
	pEMS->nChrg_NodeNum = 0;
	pEMS->nDisChrging_NodeNum = 0;

	pEMS->nLast_Chrg_NodeNum = 0;
	pEMS->nLast_DisChrging_NodeNum = 0;

	pEMS->nTMSector = 0;
    pEMS->bEnterAnti_Reflux = FALSE;
    pEMS->tm_LastEnterReflux = tm_Now;

    pEMS->bEnter_OverLoad = FALSE;
    pEMS->tm_LastEnterOverLoad = tm_Now;
	pEMS->fRemainPwr4Chrg = 0.0f;
	pEMS->fRemainDemand4Chrg = 0.0f;
	// LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
	pEMS->bEnter_OverLoadHV = FALSE;
    pEMS->tm_LastEnterOverLoadHV = tm_Now;
	// LJH241007 AntiPhase
	pEMS->isAtRflxMtrHV = (pCCU->emAuxMeterPos == AUXMETER_POS_BRANCH_TRANSFER);
	pEMS->bEnterAnti_Reflux_Phase =    FALSE;
	pEMS->tm_LastEnterReflux_Phase =   tm_Now;
	pEMS->bEnter_OverLoad_Phase =      FALSE;
	pEMS->tm_LastEnterOverLoad_Phase = tm_Now;
	// LJH241202 SEMS 优化Sort_NodeOrderByChrgedEnergy
	pEMS->tm_LastSort =   _NOW_;
	pEMS->bSwapLastSort =     FALSE;
	for (n=0; n<MAX_SEMS_CLIENTS_NUM; n++)
	{
		pEMS->idxsLastSort[n] = n;
	}
	pEMS->nTargetStableCount = 0;
	pEMS->nBattGrpPwrLmt = 0;

	// LJH250408 CtrlByREMS
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3303, &varV, 4);
	pEMS->enableREMSCtrl = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3304, &varV, 4);
	pEMS->enableAntiRflx_REMS = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3305, &varV, 4);
	pEMS->enableAntiOvld_REMS = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3306, &varV, 4);
	pEMS->nStopSOC_ChrgSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 100);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3309, &varV, 4);
	pEMS->nStopSOC_DiscSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 0);

	// LJH241202 SEMS ClientInfo
	pEMS->tmStart0sec = StringToTime("2000-01-01 00:00:00", TIME_CHN_FMT);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3301, &varV, 4);
	pEMS->bAutoChrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3302, &varV, 4);
	pEMS->bAutoDischrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3307, &varV, 4);
	pEMS->fTargetPwr_4Chrg_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
	pEMS->fTargetPwr_4Chrg_SEMS = MIN(pEMS->fTargetPwr_4Chrg_SEMS, pCCU->blobBackendCfg.fSysACInput_PwrLmt);

	
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3308, &varV, 4);
	pEMS->fTargetPwr_4Dischrg_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3311, &varV, 4);
	pEMS->fTargetPwr_4ChrgOffset_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3312, &varV, 4);
	pEMS->fTargetPwr_4DischrgOffset_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);

	// LJH250930 加快自动充放响应 1001, 3317,
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3317, &varV, 4);
    pEMS->fLtPwr2Standby_4Chrg = ((err==ERR_DPR_OK) ? varV.fValue : -99999);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3318, &varV, 4);
	pEMS->uDelayStandby_4Chrg = ((err==ERR_DPR_OK) ? varV.ulValue : 0);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3319, &varV, 4);
    pEMS->fGtPwr2Standby_4Disc = ((err==ERR_DPR_OK) ? varV.fValue : 99999);
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1001, 3320, &varV, 4);
    pEMS->uDelayStandby_4Disc = ((err==ERR_DPR_OK) ? varV.ulValue : 0);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 85, &varV, 4);
    pEMS->bStandby_Set = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	TRACED("Init_EmsMgmtData bStandby_Set:%d", pEMS->bStandby_Set);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 99, &varV, 4);
    pEMS->emSortMethod = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : SORT_BY_SOC);

    pEMS->emLastSocSortWm = WORKMODE_MAX;

	// Init PID Params
	PID_Init(pCCU);
}

void InitCCUStuffData(ROUGH_CCU_DATA* pCCU)
{
	int n = 0;
	for(n = 0; n <MAX_SINGLE_STBATT_NUM; n++)
	{
		ZERO_POBJS(&pCCU->strLSYSInfo.branchNodeData[n], 1);
		ZERO_POBJS(&pCCU->emsMgmtData.calc_Node[n],1);
	}

	ZERO_POBJS(&pCCU->strLSYSInfo.singleRectData,1);
	return;
}

BOOL Send_CCU_Ctrl(ROUGH_CCU_DATA* pCCU, A_BLOB_GRP_RECT_AUTOCTRL_CMD CCUCtrlCmd)
{
	int nRet = 0;
	BYTE byData[16]={'\0'};
	memset(byData, 0, 16);
	int nSendLen = 0;
	
	A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCmd = &CCUCtrlCmd;
	
	//get battGrp info
	A_BLOB_SINGLE_STBATT_RUNINFO  blobSingleBattData;
	VAR_VALUE varV;
	varV.blobValue = (BYTE *)&blobSingleBattData;
	// LJH241111 PcsSCADA
	if ( HasEMSFeature(pCCU->lcCfg.emSystemType) )
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
			501 + pCmd->nGrp, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
	}
	
	TRACEI("-------Send_CCU_Ctrl cmd A_BLOB_GRP_RECT_AUTOCTRL_CMD[%d] nGrpType=[%d] , fReq[%.1f, %.1f], fLmt[%.1f, %.1f]-----\n", 
		pCmd->nGrp, pCmd->emCtrl_Type, pCmd->fGrpVoltLmt, pCmd->fGrpCurrLmt, blobSingleBattData.fAllow_LmtChargeVolt, blobSingleBattData.fAllow_MaxChargeCurr);

	memcpy(&pCCU->blobGrpRectCtrl, pCmd, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD));
	
	TRACEI("Send_CCU_Ctrl emCharge_Statu=%d,nVoltReq=%.1f, nCurrReq=%.1f\n",\
	pCCU->blobGrpRectCtrl.emCtrl_Type,\
	pCCU->blobGrpRectCtrl.fGrpVoltLmt,\
	pCCU->blobGrpRectCtrl.fGrpCurrLmt);
	
	if(pCCU->blobGrpRectCtrl.emCtrl_Type <= GRP_ACDC_STOP)
	{
		byData[0] = 0x31 + pCmd->nGrp;
		nSendLen = 6;
		if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_STOP)
		{
			byData[1] = 0x00;
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_START)
		{
			byData[1] = 0x01;
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR)
		{
			byData[1] = 0x02;
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_INVERT_START)
		{
			byData[1] = 0x03;
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_INVERT_PENDING)
		{
			byData[1] = 0x04;
		}
		if (HasEMSFeature(pCCU->lcCfg.emSystemType))
		{
			TRACE("Send_CCU_Ctrl:Control HasEMSFeature fGrpVoltLmt:%.1f %.1f fGrpCurrLmt:%.1f %.1f %.1f\n",
				pCCU->blobGrpRectCtrl.fGrpVoltLmt, blobSingleBattData.fAllow_LmtChargeVolt,
				pCCU->blobGrpRectCtrl.fGrpCurrLmt, blobSingleBattData.fAllow_MaxChargeCurr, blobSingleBattData.fAllow_MaxDisChargeCurr);

			pCCU->blobGrpRectCtrl.fGrpVoltLmt = MIN(pCCU->blobGrpRectCtrl.fGrpVoltLmt,
				blobSingleBattData.fAllow_LmtChargeVolt);

			if((pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_START) ||
			(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR))
			{
				pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
					blobSingleBattData.fAllow_MaxChargeCurr);
			}
			else
			{
				pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
					blobSingleBattData.fAllow_MaxDisChargeCurr);
			}
		}
		TRACE("Send_CCU_Ctrl emCtrl_Type=%d ,nVoltReq=%.1f, nCurrReq=%.1f\n",\
			pCCU->blobGrpRectCtrl.emCtrl_Type,\
			pCCU->blobGrpRectCtrl.fGrpVoltLmt,\
			pCCU->blobGrpRectCtrl.fGrpCurrLmt);
		// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
		// 		"Sent to IMSU-D ---emCtrl_Type[%d] fActivePower[%.1f] fGrpVoltLmt[%.1f] fGrpCurrLmt[%.1f]",
		// 		pCCU->blobGrpRectCtrl.emCtrl_Type,
		// 		pCCU->blobGrpRectCtrl.fGrpVoltLmt * pCCU->blobGrpRectCtrl.fGrpCurrLmt,
		// 		pCCU->blobGrpRectCtrl.fGrpVoltLmt,
		// 		pCCU->blobGrpRectCtrl.fGrpCurrLmt);
		
		byData[2] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10) >> 8);
		byData[3] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10);
		byData[4] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10) >> 8);
		byData[5] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10);
	}
	else if(pCCU->blobGrpRectCtrl.emCtrl_Type <= GRP_DCDC_STOP)
	{
		byData[0] = 0x51 + pCmd->nGrp;
		nSendLen = 4;
		if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_STOP)
		{
			byData[1] = 0x00;
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_START)
		{
			byData[1] = 0x01; 
		}
		else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_PENDING)
		{
			byData[1] = 0x02; 
		}

		if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_START && HasEMSFeature(pCCU->lcCfg.emSystemType))
		{
			pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
				blobSingleBattData.fAllow_MaxDisChargeCurr);
		}
		byData[2] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10) >> 8);  
		byData[3] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10);  
		
		byData[4] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10) >> 8);
		byData[5] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10);
	}
	TRACE("Send_CCU_Ctrl:Control emCtrl_Type:%d(0:rect 2:invt 4:stop) nGrp:%d UI:%.1f %.1f byData<%s>\n",
		pCCU->blobGrpRectCtrl.emCtrl_Type, pCCU->blobGrpRectCtrl.nGrp,
		pCCU->blobGrpRectCtrl.fGrpVoltLmt,
		pCCU->blobGrpRectCtrl.fGrpCurrLmt, printableBytes(byData, nSendLen));
	//2020-01-04 Do Not Wait Adjust Current Cmd
	nRet = DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 0) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;

	return nRet;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Initialize
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
// #define INIT_NL_MSGINFO(p, _msgType, _pfnParser, _byCmd, _usMinDataLen) \
// 					((p)->msgTp = (_msgType), \
// 					(p)->byCmd = (_byCmd), \
// 					(p)->pfnParser = (_pfnParser), \
// 					(p)->usMin_ExpectDataLen = (_usMinDataLen), \
// 					(p)++)

DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	TRACE("CCU_TCP: 1\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	pSampler->pDataHeap = (void*)NEW(ROUGH_CCU_DATA, 1);
	int n = 0;
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

	TRACE("CCU_TCP: 2\n");

	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pCCU, 1);
	pCCU->hSamplerLock = pSampler->hSamplerLock; 
	pCCU->hSampler = hSamplerSelf;
	pCCU->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
#ifdef IMMU2
	pCCU->hDataComm = NULL;
#endif
	
	pCCU->tmSysBoot = _NOW_;
 	pCCU->emCCUState = CCU_STATE_IDLE;
	pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
    pCCU->tmLastCalcESStoreData  = _NOW_;
	pCCU->strLSYSInfo.tmLastSend_PwrOnSTS =  _NOW_;
	pCCU->strLSYSInfo.tmLastSend_CtrlVoiceAndLight =  _NOW_;
	pCCU->strLSYSInfo.tmLastSend_PwrCabLight =  _NOW_;
	pCCU->strHEARTInfo.bySWRunLocation =  0;
	pCCU->strHEARTInfo.usSWVer =  0;
	pCCU->strHEARTInfo.tmLastRecv_HEART =  _NOW_;
	pCCU->nStuffWaitTimes = 0;
	pCCU->bCommFail = FALSE;

    pCCU->nLast_Hour     = 0;
    pCCU->nLast_MDay     = 0;
    pCCU->nLast_tmYear   = 0;
	pCCU->emLast_FeeRate = FEERATE_SHARP;
	pCCU->nAbnormalCurr_Count  = 0;
	
	pCCU->tagPM2CCU_GCan.emTrigMsgTp = PM2CCU_TRIG_OFF;

	pCCU->emGrpContactor1_CtrlVal = 2;
	pCCU->emGrpContactor2_CtrlVal = 2;
	pCCU->emGrpContactor3_CtrlVal = 2;
	pCCU->emGrpContactor4_CtrlVal = 2;
	pCCU->nGrp1_StickingAlarmTimes = 0;
	pCCU->nGrp2_StickingAlarmTimes = 0;
	pCCU->nGrp3_StickingAlarmTimes = 0;
	pCCU->nGrp4_StickingAlarmTimes = 0;
	pCCU->emIsGrpContactor1_DisconnNeeded = 0;
	pCCU->emIsGrpContactor2_DisconnNeeded = 0;
	pCCU->emIsGrpContactor3_DisconnNeeded = 0;
	pCCU->emIsGrpContactor4_DisconnNeeded = 0;

	pCCU->emPVContactor1_CtrlVal = 2;
	pCCU->emPVContactor2_CtrlVal = 2;
	pCCU->emPVContactor3_CtrlVal = 2;
	pCCU->emPVContactor4_CtrlVal = 2;
	pCCU->nPV1_StickingAlarmTimes = 0;
	pCCU->nPV2_StickingAlarmTimes = 0;
	pCCU->nPV3_StickingAlarmTimes = 0;
	pCCU->nPV4_StickingAlarmTimes = 0;
	pCCU->emIsPVContactor1_DisconnNeeded = 0;
	pCCU->emIsPVContactor2_DisconnNeeded = 0;
	pCCU->emIsPVContactor3_DisconnNeeded = 0;
	pCCU->emIsPVContactor4_DisconnNeeded = 0;

	pCCU->emCurrDiffAbnormal = 0;
	
	pCCU->nPCSNum = 0;
	pCCU->tmLastSend_Cmd = _NOW_;
#ifdef IMMU3
	pCCU->bIsReceivedCFGG_FromLCCU = FALSE;
#endif
	// LJH241202 SEMS
	pCCU->emESSystemType = GetESSysType(hSamplerSelf);

	pCCU->emESSDevType = GetESSDeviceType_Sampler(hSamplerSelf);
	

	int nErrCode;
	nErrCode = pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT,
		5, NULL, &pCCU->pComm, sizeof(COMMPORT));

	if(nErrCode != ERR_DPR_OK)
	{
		printf("ccu get can fail\n");
	}
	else
	{
		COMM_CAN_BAUDRATE	emLevel = COMM_CAN_BAUDRATE_500K;

		pCCU->hCanDevHanle = pCCU->pComm.Open(
			CANPORT,
			pSampler->info.szSamplerSetting,
			emLevel, 50, &nErrCode);

		if(nErrCode != ERR_COMM_OK)
		{
			printf("Open CAN1 Failed !!! nErrCode=[%d]\n", nErrCode);
		}
	}
	pCCU->bAllConfirmed = FALSE;

	pCCU->pfnInitialize[SYSTYPE_ESS_AIR] =         Initialize_ESS;
	pCCU->pfnInitialize[SYSTYPE_ESS_LIQUIDED] =    Initialize_ESS;
	pCCU->pfnInitialize[SYSTYPE_EMS_STATION] =     Initialize_EMS_STATION;
	// pCCU->pfnInitialize[SYSTYPE_SCADA_LOCALCTRL] = Initialize_EMS_STATION; //Initialize_SCADA_LOCALCTR;
	// pCCU->pfnInitialize[SYSTYPE_SCADA_INPCS] =     Initialize_SCADA_INPCS;
	// pCCU->pfnInitialize[SYSTYPE_SCADA_INPCS235KW] =Initialize_SCADA_INPCS;

	pCCU->pfnSamples[SYSTYPE_ESS_AIR] =         Sample_ESS;
	pCCU->pfnSamples[SYSTYPE_ESS_LIQUIDED] =    Sample_ESS;
	pCCU->pfnSamples[SYSTYPE_EMS_STATION] =     Sample_EMS_STATION;

	pCCU->pfn_can_Samples = CAN_Samples;
	// pCCU->pfnSamples[SYSTYPE_SCADA_LOCALCTRL] = Sample_EMS_STATION; //Sample_SCADA_LOCALCTR;
	// pCCU->pfnSamples[SYSTYPE_SCADA_INPCS] =     Sample_SCADA_INPCS;
	// pCCU->pfnSamples[SYSTYPE_SCADA_INPCS235KW] =Sample_SCADA_INPCS;

	if (pCCU->lcCfg.emSystemType != SYSTYPE_EMS_STATION)
	{
		NLMSG_FRAME_INFO *pp = &pCCU->nlFrameInfo[0];
		INIT_NL_MSGINFO(pp, NL_MSG_CFGG,	Parse_CFGG,		0x01,	40);
		INIT_NL_MSGINFO(pp, NL_MSG_CFGS,	Parse_CFGS,		0x02,	2);
		INIT_NL_MSGINFO(pp, NL_MSG_UCTL,	Parse_UCTL,		0x03,	2);
		INIT_NL_MSGINFO(pp, NL_MSG_RTIF,	Parse_RTIF,		0x04,	18);
		INIT_NL_MSGINFO(pp, NL_MSG_GCAN,	Parse_GCAN,		0x05,	1);

		INIT_NL_MSGINFO(pp, NL_MSG_PCSGET,	Parse_PCSGet,	0x23,	8); //Added By John.20220715
		INIT_NL_MSGINFO(pp, NL_MSG_PCSSET,	Parse_PCSSet,	0x24,	8);
		INIT_NL_MSGINFO(pp, NL_MSG_READ_PCSSET,	Parse_Read_PCSSett,	0x25,	8);//Use Cmd25 To Read Sett.BYTE2 is Defined to ReadPCS Sett.

		INIT_NL_MSGINFO(pp, NL_MSG_USIZ,	Parse_USIZ,		0x12,	1);
		INIT_NL_MSGINFO(pp, NL_MSG_UDTA,	Parse_UDTA,		0x13,	3);

		INIT_NL_MSGINFO(pp, NL_MSG_LSYS,	Parse_LSYS,		0xB0,	15);
		
		INIT_NL_MSGINFO(pp, NL_MSG_LDEV,	Parse_LDEV,		0x0C,	2);

		INIT_NL_MSGINFO(pp, NL_MSG_HEART,	Parse_HEART,	0xD0,	3); //2524 增加心跳帧
	}

	if (pCCU->lcCfg.emSystemType != SYSTYPE_EMS_STATION)
	{
		RPMSG_FRAME_INFO *pp = &pCCU->rpFrameInfo[0];
		INIT_RP_MSGINFO(pp, RP_MSG_ALL_MODULE_INFO, Parse_AllModulesInfo, 	0x50,	 5);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_GRP_1_INFO, Parse_ACDC_Grp_Info,  	0x51, 	12);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_GRP_2_INFO, Parse_ACDC_Grp_Info, 	0x52, 	12);
		INIT_RP_MSGINFO(pp, RP_MSG_DCDC_GRP_1_INFO,	Parse_DCDC_Grp_Info,	0x53,	13);
		INIT_RP_MSGINFO(pp, RP_MSG_DCDC_GRP_2_INFO,	Parse_DCDC_Grp_Info,	0x54,	13);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_SIG_1_INFO,	Parse_ACDC_Sig_Info,	0x55,	46);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_SIG_2_INFO,	Parse_ACDC_Sig_Info,	0x56,	46);
		INIT_RP_MSGINFO(pp, RP_MSG_DCDC_SIG_1_INFO,	Parse_DCDC_Sig_Info,	0x57,	21);
		INIT_RP_MSGINFO(pp, RP_MSG_DCDC_SIG_2_INFO,	Parse_DCDC_Sig_Info,	0x58,	21);

		// Write/Read
		INIT_RP_MSGINFO(pp, RP_MSG_MODULE_TYPE_CFG,   		Parse_Module_Type_Cfg,  		0x6E, 3);
		INIT_RP_MSGINFO(pp, RP_MSG_MODULE_NUMBER_CFG,   	Parse_Module_Num_Cfg,  			0x6F, 6);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_CHRG_DISCHRG_CTRL,	Parse_ACDC_ChrgDischarge_Ctrl,  0x70, 7);
		INIT_RP_MSGINFO(pp, RP_MSG_DCDC_CHRG_DISCHRG_CTRL,	Parse_DCDC_ChrgDischarge_Ctrl,  0x71, 6);
		INIT_RP_MSGINFO(pp, RP_MSG_MAUNL_AUTO_MODE,			Parse_Manul_Auto_Ctrl,  		0x72, 1);
		INIT_RP_MSGINFO(pp, RP_MSG_ON_OFF_GRID_CTRL,		Parse_N2002_OnOffGrid_Ctrl,  	0x73, 4);
		INIT_RP_MSGINFO(pp, RP_MSG_ACDC_PARAMS_SET,			Parse_ACDC_Params_Set,			0x74, 9);
	}

	//Initialize DOs
	Set_IMMU_DO(IOC_SET_DO1, 0);
	Set_IMMU_DO(IOC_SET_DO2, 0);
	Set_IMMU_DO(IOC_SET_DO3, 0);
	Set_IMMU_DO(IOC_SET_DO4, 0);
	Set_IMMU_DO(IOC_SET_DO6, 0);

	InitCCUStuffData(pCCU);

 	ZERO_POBJS(&pCCU->hisESData, 1);
 
 
	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_UI_NOTICE,			EVENT_MSG_BEGIN },
		{EVENT_TYPE_SAVE_ES_STATDATA,	EVENT_MSG_RESTORE},
		{EVENT_TYPE_BMSRUNDATA_UPDATE,	EVENT_MSG_RESTORE},
	};
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pCCU->pDPR->Register_EvHandler("CCUTcp_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
	
	//Get Specific Setting 
	VAR_VALUE varV;
	varV.blobValue = (BYTE *)&pCCU->pNewSet;
	pCCU->pDPR->Get(hSamplerSelf, GETTYPE_ST_VAR, 200, 200, &varV, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
	
	varV.blobValue = (BYTE *)&pCCU->blobBackendCfg;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));

	varV.blobValue = (BYTE *)&pCCU->blobFeeRate;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 12, &varV, sizeof(A_BLOB_CHARGE_FEESET));

 	//varV.blobValue = (BYTE *)&pCCU->battPlan;
	//pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 500, 501, &varV, sizeof(A_BLOB_BATT_MGMT_PLAN));

  	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 20, &varV, 4);
	pCCU->emPmaxDemandMode = varV.emValue;
			
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 23, &varV, 4);
	pCCU->fBAT_Max_T_PwrLmtRate_Set = varV.fValue;
	
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 7, &varV, 4); // Apply Scenery Config
	pCCU->emAuxMeterPos =(POS_AUXMETER)varV.ulValue;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 217, &varV, 4);
	pCCU->emPMCtrlMode = (STAT_PM_CTRL_MODE)varV.emValue;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 218, &varV, 4);
	pCCU->emSTSAutoManual = (STS_AUTOMANUAL_ONOFFGRID)varV.emValue;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 219, &varV, 4);
	pCCU->emSTSOnOffGridState = (STS_ONOFFGRID_STATE)varV.emValue;
	
 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 500, 509, &varV, 4);
	pCCU->emDisChgByStopSOC = varV.emValue;//0-No, 1-Yes
	//TRACE("Init:emDisChgByStopSOC=[%d]\n", pCCU->emDisChgByStopSOC);

	pCCU->tagOn2OffParam.emHMITrig 		= STAT_TRIG_IDLE;
	pCCU->tagOn2OffParam.tmLastHMITrig 	= _NOW_;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 198, &varV, 4);
	TRACE("ccu-init Get 200 198 = [%d]",varV.emValue);
	if(varV.emValue == 0) //lock need check
	{
		if(pCCU->emCCUState != CCU_STATE_LOCKED)
		{
			pCCU->emCCUState = CCU_STATE_LOCKED;
		}
	}
	else
	{
		pCCU->emCCUState = CCU_STATE_IDLE;
		//Refresh_SysAvailState(pCCU);
	}
	// LJH241007 AntiPhase
	ERRCDDE_DPR err;
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 40, &varV, 4);
	pCCU->needAntiReflux_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 41, &varV, 4);
	pCCU->needAntiOverLoad_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	pCCU->lcCfg = GetLCCfg_Sampler(pCCU->hSampler); // LJH250508 LCCfg

	// LJH250703
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 84, &varV, 4);
	pCCU->msPeriodLSYS = ((err==ERR_DPR_OK) ? varV.ulValue : 1000);
	// LJH250703 移到SystemSample 200, 222, 改为 1, 102,
	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 102, &varV, 4);
	pCCU->fTotalPowerLimit = ((err==ERR_DPR_OK) ? varV.fValue : MAX_PWR_LMT);
	TRACED("Initialize msPeriodLSYS:%u fTotalPowerLimit:%.1f",
		pCCU->msPeriodLSYS, pCCU->fTotalPowerLimit);

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 106, &varV, 4);  //放电滤波点数
	pCCU->nDisChrgFilterPoint = ((err==ERR_DPR_OK) ? varV.ulValue : 20);
	TRACED("Init nDisChrgFilterPoint = [%d]\n",pCCU->nDisChrgFilterPoint);

	pCCU->tLoadLimitTime_over110 = _NOW_;
	pCCU->tLoadLimitTime_over120 = _NOW_;
	pCCU->fcabPowerConsump = 0;
	pCCU->tPowerUpdataTime = _NOW_;

	err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 200, 216, &varV, 4);
	pCCU->emOnOffGridCtlNLineGround = ((err==ERR_DPR_OK) ? varV.emValue : 0);

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 110, &varV, 4);
	pCCU->tagGridApplyScenery = (GRID_APPLY_SCENERY)varV.emValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 1, 111, &varV, 4);
	pCCU->nSetPriGridMeter = (UINT)varV.emValue;

	varV.emValue = 0;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 86, varV);
	//now send msg to other modual
	//PushAllData(pCCU);

	//Init PushData
	
	Init_blobEmsCfgPlanParam(pCCU);
 
 	Init_EmsMgmtData(pCCU);

	//Push_BlobValue(SETTYPE_RAW_ST, hSamplerSelf, 
	//	200, 200, (void*)&pCCU->pOldSet, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), BLOB_TYPE_CHARGER_SPECIFIC_SETTING);
	
	// LJH241202 SEMS
	pCCU->pfnInitialize[pCCU->lcCfg.emSystemType](hSamplerSelf);

	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : JigTest
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   : Only Used For Hardware interface Test
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

/*
static int DoTCPFrmParsing(ROUGH_CCU_DATA* pCCU, NLMSG_FRAME_INFO* pFrm,  BYTE* pData, USHORT usLenID)
{
	Mutex_Lock(pCCU->hSamplerLock, WAIT_INFINITE);
	TRACEX("------Now DoTCPFrmParsing msg ----\n");


	Mutex_Unlock(pCCU->hSamplerLock);

	return nRet;
}
*/

static USHORT MakeChkSum(BYTE byCmd, USHORT usDataLen, BYTE* byData)
{
	USHORT ulCalcSum = (USHORT)byCmd + (BYTE)(usDataLen >> 8) + (BYTE)usDataLen;

	// TRACE("ulCalcSum=%d byCmd=%02X usDataLen=%d\n",ulCalcSum , byCmd , usDataLen);
	int n;
	for(n = 0; n < usDataLen; n++)
	{
		ulCalcSum += byData[n];
	}

	ulCalcSum &= 0xFFFF;
	//ulCalcSum = (~ulCalcSum) + 1;

	return ulCalcSum;
}

int MakeNLMSG_SendFrame(BYTE byCmd, IN USHORT usSendDataLen, IN BYTE* pData, OUT BYTE *sendBuff)
{
	int n;
#ifdef IMMU3
	memset(sendBuff, 0, CCU_BUFF_LEN_232);
#else
	memset(sendBuff, 0, TCP_BUFF_LEN);
#endif
	int nSendBuffLen = 6;
	//SOI
	sendBuff[0] = (BYTE)(NLMSG_SOI >> 16);
	sendBuff[1] = (BYTE)(NLMSG_SOI >> 8);
	sendBuff[2] = (BYTE)(NLMSG_SOI & 0xFF);
	//CMD
	sendBuff[3] = byCmd;
	//DataLen
	sendBuff[4] = (BYTE)(usSendDataLen >> 8);
	sendBuff[5] = (BYTE)(usSendDataLen & 0xFF);
	//Data
	for(n = 0; n < usSendDataLen; n++)
	{
		sendBuff[6 + n] = pData[n];
	}
	nSendBuffLen += usSendDataLen;
	//Calc chksum
	USHORT ulCalcSum = MakeChkSum(byCmd, usSendDataLen, pData);
	sendBuff[nSendBuffLen] = (BYTE)(ulCalcSum >> 8);
	sendBuff[nSendBuffLen + 1] = (BYTE)(ulCalcSum & 0xFF);
	sendBuff[nSendBuffLen + 2] = (BYTE)(NLMSG_EOI);
	TRACE("MakeNLMSG_SendFrame byCmd:%X sendBuff<%s>", byCmd, printableBytes(sendBuff, nSendBuffLen + 3));
	
	return nSendBuffLen + 3;
}

 
/*********************************************************************************
*  
*  FUNCTION NAME : Find_NLMsgFrame
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
NLMSG_FRAME_INFO* Find_NLMsgFrame(IN ROUGH_CCU_DATA* pCCU, IN BYTE* pRaw, IN int nReadLen, 
	OUT BYTE** ppDataPos, OUT USHORT* pusDataLen)
{
	int n, m;
	BYTE* byPos = pRaw;
	int nSOIPos = -1;
	int nFindLen = 0;
	USHORT nParsedDataLen = 0;
	NLMSG_FRAME_INFO* pFoundMsg = NULL;
	//Notice: only find the first frame, if have multi frames, ignore the followed others
	for(n = 0; n < nReadLen - NLMSG_MIN_LEN + 1; n++)
	{
		if((byPos[n] == (BYTE)(NLMSG_SOI >> 16)) && 
			(byPos[n + 1] == (BYTE)(NLMSG_SOI >> 8)) &&
			(byPos[n + 2] == (BYTE)(NLMSG_SOI & 0xFF)))
		{
			//CMD;
			for(m = 0; m < NL_MSG_Max; m++)
			{
				if(pCCU->nlFrameInfo[m].byCmd == byPos[n + 3])
				{
					pFoundMsg = &pCCU->nlFrameInfo[m];
					// TRACED("Had Found nlFrameInfo[%d].byCmd=[%02X]\n", n,pCCU->nlFrameInfo[m].byCmd);
					break;
				}
			}
			
			if(pFoundMsg == NULL)
			{
				// TRACED("Maybe CCU Transfer Error Cmd,Cause System reboot\n");
				continue;
			}	
			//now check data length;
			nParsedDataLen = ((USHORT)byPos[n + 4] << 8) + byPos[n + 5];
#ifdef IMMU3
		if((nParsedDataLen > CCU_BUFF_LEN_232) || (nParsedDataLen < pFoundMsg->usMin_ExpectDataLen))
#else
		if((nParsedDataLen > TCP_BUFF_LEN) || (nParsedDataLen < pFoundMsg->usMin_ExpectDataLen))
#endif
			{
				TRACE("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 3], nParsedDataLen);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}

			//now check chksum
			USHORT ulCalcSum = MakeChkSum(pFoundMsg->byCmd, nParsedDataLen, byPos + n + 6);
			USHORT ulRealSum = ((USHORT)byPos[n + 6 + nParsedDataLen] << 8) + byPos[n + 7 + nParsedDataLen];
			if(ulCalcSum != ulRealSum)
			{
				TRACE("Found Msg[%02X] nParsedDataLen =%d but the chkSum[%02X, %02X] is DIFFERENT\n", byPos[n + 3], 
					nParsedDataLen, ulCalcSum, ulRealSum);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}
			//EOI
			if(byPos[n + 8 + nParsedDataLen] != NLMSG_EOI)
			{
				TRACE("Found Msg[%02X] but the EOI[%02X] is INVALID\n", byPos[n + 3], 
					byPos[n + 8 + nParsedDataLen]);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}
			//now succeccfully get the frame
			*ppDataPos = byPos + n + 6;
			*pusDataLen = nParsedDataLen;
			break;
		}
	}	
	return pFoundMsg;
}

/*********************************************************************************
*  
*  FUNCTION NAME : GetSTBattInfo_4USYS
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int GetSTBattInfo_4USYS(ROUGH_CCU_DATA* pCCU, BYTE* pData)
{
	int 		n = 0;
	int 		nLen = 0;

	UINT 		uiVal;
	VAR_VALUE 	varV;
	USHORT 		usVal;
	SHORT       sVal;
	BYTE byBattStatu = 0x00;

	// --- Process Battery Run Statu ----//
	uiVal = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		switch (pCCU->blobSingleBattData[n].emBattStatus_Alm)
		{
			case STBATT_ALMSTAT_NOMAL:		
			case STBATT_ALMSTAT_ALARMING:
			case STBATT_ALMSTAT_PROTECT:
			{
				if(pCCU->blobSingleBattData[n].emBattStatus_Run == STBATT_RUNSTAT_CHARGE)
				{
					byBattStatu = 0x01;
				}
				else if(pCCU->blobSingleBattData[n].emBattStatus_Run == STBATT_RUNSTAT_DISCHARGE)
				{
					byBattStatu = 0x02;
				}
				else
				{
					byBattStatu = 0x00;
				}
			}
			break;
			case STBATT_ALMSTAT_FAULT:
				byBattStatu = 0x03;
			break;
			default:
				break;
		}

		//Judge Battery CommFail Status
		if(pCCU->blobSingleBattData[n].emBattStatus_Run == STBATT_RUNSTAT_COMM_FAIL) 
		{
			byBattStatu = 0x03;	
		}

		uiVal += (byBattStatu << (n*4));

		//TRACE("BMS=[%d].emBattStatus_Alm=%d\n",n,pCCU->blobSingleBattData[n].emBattStatus_Alm);
	}

	//4.Battery Status
	pData[nLen++] = (BYTE)(uiVal >> 24);
	pData[nLen++] = (BYTE)(uiVal >> 16);
	pData[nLen++] = (BYTE)(uiVal >> 8);
	pData[nLen++] = (BYTE)(uiVal & 0xFF);


	//5 ~ 28 Battery Group DataInfo
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//TRACE("nBattGrp=[%d] emBattStatus_Run=[%d] \
		//	fPackTotalVolt=[%f] fPackTotalCurr[%f] bySOC=[%d] fCharge_Cutoff_Volt=[%f] fAllow_MaxChargeCurr=[%f]\n", \
		//	n,pCCU->blobSingleBattData[n].emBattStatus_Run,\
		//	pCCU->blobSingleBattData[n].fPackTotalVolt,\
		//	pCCU->blobSingleBattData[n].fPackTotalCurr,\
		//	pCCU->blobSingleBattData[n].nSOC,\
		//	pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt,\
		//	pCCU->blobSingleBattData[n].fAllow_MaxChargeCurr);

		usVal = (USHORT)(pCCU->blobSingleBattData[n].fPackTotalVolt *10);
		pData[nLen++] = (BYTE)(usVal >> 8);
		pData[nLen++] = (BYTE)(usVal & 0xFF);

		sVal = (short)(pCCU->blobSingleBattData[n].fPackTotalCurr *10);
		pData[nLen++] = (BYTE)(sVal >> 8);
		pData[nLen++] = (BYTE)(sVal & 0xFF);

		pData[nLen++] = (BYTE)pCCU->blobSingleBattData[n].nSOC;
	
		usVal = (USHORT)(pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt *10);
		pData[nLen++] = (BYTE)(usVal >> 8);
		pData[nLen++] = (BYTE)(usVal & 0xFF);

		sVal = (short)(pCCU->blobSingleBattData[n].fAllow_MaxChargeCurr *10);
		pData[nLen++] = (BYTE)(sVal >> 8);
		pData[nLen++] = (BYTE)(sVal & 0xFF);

		//Resved
		nLen += 4;
	}
 
	//29~44. Get Node Calc Data
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{	
		//TRACE("fBatt_Distri_4_ACDC_Pwr=[%f] fBatt_Distri_4_DCDC_Pwr=[%f] fMPPT_Distri_4_ACDC_Pwr=[%f]\
		//	fMPPT_Distri_4_DCDC_Pwr=[%f] fACDC_Distri_4_BattChrg_Pwr=[%f]\n",\
		//	pCCU->emsMgmtData.calc_Node[n].fBatt_Distri_4_ACDC_Pwr,\
		//	pCCU->emsMgmtData.calc_Node[n].fBatt_Distri_4_DCDC_Pwr,\
		//	pCCU->emsMgmtData.calc_Node[n].fMPPT_Distri_4_ACDC_Pwr,\
		//	pCCU->emsMgmtData.calc_Node[n].fMPPT_Distri_4_DCDC_Pwr,\
		//	pCCU->emsMgmtData.calc_Node[n].fACDC_Distri_4_BattChrg_Pwr);

		usVal = (USHORT)(pCCU->emsMgmtData.calc_Node[n].fBatt_Distri_4_ACDC_Pwr *10);
		pData[nLen++] = (BYTE)(usVal >> 8);
		pData[nLen++] = (BYTE)(usVal & 0xFF);

		usVal = (USHORT)(pCCU->emsMgmtData.calc_Node[n].fBatt_Distri_4_DCDC_Pwr *10);
		pData[nLen++] = (BYTE)(usVal >> 8);
		pData[nLen++] = (BYTE)(usVal & 0xFF);

		usVal = (USHORT)(pCCU->emsMgmtData.calc_Node[n].fACDC_Distri_4_BattChrg_Pwr *10);
		pData[nLen++] = (BYTE)(usVal >> 8);
		pData[nLen++] = (BYTE)(usVal & 0xFF);

		sVal = (short)(pCCU->emsMgmtData.calc_Node[n].fAvg_QoutputLmt *10);
		pData[nLen++] = (BYTE)(sVal >> 8);
		pData[nLen++] = (BYTE)(sVal & 0xFF);
		
		//TRACE("!!!--- nBattIdx=[%d] sVal =[%d] \n", n, sVal);

		nLen += 2;
	}

	//TRACE("Get BattInfo And Calc Node Data nLen = %d\n", nLen);
 	return nLen;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Send_USYS
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Send_USYS(ROUGH_CCU_DATA* pCCU)
{
	int n = 0;
	VAR_VALUE varV;
#ifdef IMMU3
	BYTE sendBuff[CCU_BUFF_LEN_232];
	memset(sendBuff, 0, CCU_BUFF_LEN_232);
#else
	BYTE sendBuff[TCP_BUFF_LEN];
	memset(sendBuff, 0, TCP_BUFF_LEN);
#endif
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	//SOI
	sendBuff[0] = (BYTE)(NLMSG_SOI >> 16);
	sendBuff[1] = (BYTE)(NLMSG_SOI >> 8);
	sendBuff[2] = (BYTE)(NLMSG_SOI & 0xFF);
	//CMD
	sendBuff[3] = NLMSG_USYS_CMD;
	//Now make the Data
	int nSendDataLen = 0;
	BYTE* pData = sendBuff + 6;
	//CP status
	if((pCCU->emCCUState != CCU_STATE_IDLE) /*&& 
		(pCCU->emSystemFailReason != SYSFAIL_REASON_CCU_FAULTED)*/)
	{
		pData[0] = 0x01;
	}
	else
	{
		pData[0] = 0x00;  
	}

	//1. CP Faulted Status
	nSendDataLen += 1;

	//2.System Alarm Bit Map
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 1, 38, &varV, 4);
	pCCU->usysAlmMap1.Alarm.PV_SDP_Alm = varV.emValue;
	
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 601, 453, &varV, 4);
	pCCU->usysAlmMap1.Alarm.DoorOpen = varV.emValue;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 611, 3096, &varV, 4);
	pCCU->usysAlmMap1.Alarm.DoorOpen = varV.emValue | pCCU->usysAlmMap1.Alarm.DoorOpen;

	pData[nSendDataLen] = (BYTE)(pCCU->usysAlmMap1.usFlag >> 8);
	nSendDataLen++;
	pData[nSendDataLen] = (BYTE)(pCCU->usysAlmMap1.usFlag & 0xFF);
	nSendDataLen++;

	//3. Resved - TBD
	nSendDataLen += 32;

	//Now get the ST-Batt Info
	nSendDataLen += GetSTBattInfo_4USYS(pCCU, pData + nSendDataLen);

	//DataLen
	sendBuff[4] = (BYTE)(nSendDataLen >> 8);
	sendBuff[5] = (BYTE)(nSendDataLen & 0xFF);
	//Calc chksum
	USHORT ulCalcSum = MakeChkSum(NLMSG_USYS_CMD, (USHORT)nSendDataLen, pData);
	sendBuff[nSendDataLen + 6] = (BYTE)(ulCalcSum >> 8);
	sendBuff[nSendDataLen + 7] = (BYTE)(ulCalcSum & 0xFF);
	sendBuff[nSendDataLen + 8] = (BYTE)(NLMSG_EOI);
	//all frame len
	int nSendFrmLen = nSendDataLen + 9;
	//TRACE("\n---USYS---Now begin sending msg to CCU nSendDataLen=[%d] nSendFrmLen: [%d] time_t[%lf]-------\n", nSendDataLen, nSendFrmLen, GetCurrentTime());
	//Sleep_Ex(50); // if send to frequently data can get lost
	
	//int i;
	//for(i =0; i < nSendFrmLen; i++)
	//{
	//	TRACE("%02X-",sendBuff[i]);
	//}
	//TRACE("\n");
	
	// if(pSampler->pUsedCommPort->Write(pCCU->hDataComm,
	// 	(char*)sendBuff, nSendFrmLen) != nSendFrmLen)
	if(CCU_WriteData(pCCU, sendBuff, nSendFrmLen) != nSendFrmLen)
	{
		//do nothing?
		TRACE("Err:Write Data To LCCU Abnormal! \n");
	}
}

BOOL Try_SyncCfgSetting(ROUGH_CCU_DATA* pCCU)
{
	// if(pCCU->emCommStatus == CCU_COMMSTAT_FAILED)
	// {
	// 	TRACE("Err: emCommStatus=%d\n", pCCU->emCommStatus);
	// 	return FALSE;
	// }

	TRACE("Now try sync settings for emCommStatus = %d\n", pCCU->emCommStatus);
	//SYS_ATTRIBUTES			sysAtributes;
	int n = 0;					
	//BRANCH_CFG_ATTRIBUTES	branchCfgAttri;

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap1.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap1.usFlag)
	{
		TRACE("pCCU->pNewSet.ccuSettings.sysAtributesMap1= %d\n",pCCU->pNewSet.ccuSettings.sysAtributesMap1);
		BYTE byData[5];
		byData[0] = 0x03;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap1.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap1.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->aaaaaaaaaaa-----\n");
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap1.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap1.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap2.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap2.usFlag)
	{
 		BYTE byData[5];
		byData[0] = 0x04;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap2.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap2.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap2.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap2.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap3.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap3.usFlag)
	{
 		BYTE byData[5];
		byData[0] = 0x05;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap3.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap3.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap3.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap3.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap4.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap4.usFlag)
	{
 		BYTE byData[5];
		byData[0] = 0x06;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap4.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap4.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap4.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap4.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap5.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap5.usFlag)
	{
 		BYTE byData[5];
		byData[0] = 0x07;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap5.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap5.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap5.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap5.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap6.usFlag != pCCU->pOldSet.ccuSettings.sysAtributesMap6.usFlag)
	{
 		BYTE byData[5];
		byData[0] = 0x08;
		byData[1] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap6.usFlag >> 8);
		byData[2] = (BYTE)(pCCU->pNewSet.ccuSettings.sysAtributesMap6.usFlag);
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.sysAtributesMap6.usFlag = pCCU->pNewSet.ccuSettings.sysAtributesMap6.usFlag;
	}

	if(pCCU->pNewSet.ccuSettings.byFan1_SpdLevel != pCCU->pOldSet.ccuSettings.byFan1_SpdLevel)
	{
		TRACEX("pCCU->pNewSet.ccuSettings.byFan1_SpdLevel= %d\n",pCCU->pNewSet.ccuSettings.byFan1_SpdLevel);
		BYTE byData[5];
		byData[0] = 0x09;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan1_SpdLevel;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->bbbbbbbbbbbb-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan1_SpdLevel = pCCU->pNewSet.ccuSettings.byFan1_SpdLevel;
	}

	if(pCCU->pNewSet.ccuSettings.byFan2_SpdLevel != pCCU->pOldSet.ccuSettings.byFan2_SpdLevel)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byFan2_SpdLevel= %d\n",pCCU->pNewSet.ccuSettings.byFan2_SpdLevel);
		BYTE byData[5];
		byData[0] = 0x0A;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan2_SpdLevel;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->bbbbbbbbbbbb-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan2_SpdLevel = pCCU->pNewSet.ccuSettings.byFan2_SpdLevel;
	}

	if(pCCU->pNewSet.ccuSettings.byFan3_SpdLevel != pCCU->pOldSet.ccuSettings.byFan3_SpdLevel)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byFan3_SpdLevel= %d\n",pCCU->pNewSet.ccuSettings.byFan3_SpdLevel);
		BYTE byData[5];
		byData[0] = 0x0B;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan3_SpdLevel;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->bbbbbbbbbbbb-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan3_SpdLevel = pCCU->pNewSet.ccuSettings.byFan3_SpdLevel;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_1 != pCCU->pOldSet.ccuSettings.byNumGrp_1)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_1=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_1);
		BYTE byData[5];
		byData[0] = 0x11;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_1;
		if(DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_1 = pCCU->pNewSet.ccuSettings.byNumGrp_1;
	}
	
	if(pCCU->pNewSet.ccuSettings.byNumGrp_2 != pCCU->pOldSet.ccuSettings.byNumGrp_2)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_2=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_2);
		BYTE byData[5];
		byData[0] = 0x12;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_2;
		if(DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_2 = pCCU->pNewSet.ccuSettings.byNumGrp_2;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_3 != pCCU->pOldSet.ccuSettings.byNumGrp_3)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_3=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_3);
		BYTE byData[5];
		byData[0] = 0x13;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_3;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_3 = pCCU->pNewSet.ccuSettings.byNumGrp_3;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_4 != pCCU->pOldSet.ccuSettings.byNumGrp_4)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_4=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_4);
		BYTE byData[5];
		byData[0] = 0x14;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_4;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_4 = pCCU->pNewSet.ccuSettings.byNumGrp_4;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_5 != pCCU->pOldSet.ccuSettings.byNumGrp_5)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_5=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_5);
		BYTE byData[5];
		byData[0] = 0x15;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_5;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_5 = pCCU->pNewSet.ccuSettings.byNumGrp_5;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_6 != pCCU->pOldSet.ccuSettings.byNumGrp_6)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_6=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_6);
		BYTE byData[5];
		byData[0] = 0x16;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_6;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_6 = pCCU->pNewSet.ccuSettings.byNumGrp_6;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_7 != pCCU->pOldSet.ccuSettings.byNumGrp_7)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_7=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_7);
		BYTE byData[5];
		byData[0] = 0x17;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_7;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_7 = pCCU->pNewSet.ccuSettings.byNumGrp_7;
	}

		if(pCCU->pNewSet.ccuSettings.byNumGrp_8 != pCCU->pOldSet.ccuSettings.byNumGrp_8)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_8=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_8);
		BYTE byData[5];
		byData[0] = 0x18;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_8;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_8 = pCCU->pNewSet.ccuSettings.byNumGrp_8;
	}


	if(pCCU->pNewSet.ccuSettings.byNumGrp_9 != pCCU->pOldSet.ccuSettings.byNumGrp_9)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_9=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_9);
		BYTE byData[5];
		byData[0] = 0x19;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_9;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_9 = pCCU->pNewSet.ccuSettings.byNumGrp_9;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_10 != pCCU->pOldSet.ccuSettings.byNumGrp_10)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_10=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_10);
		BYTE byData[5];
		byData[0] = 0x1A;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_10;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_10 = pCCU->pNewSet.ccuSettings.byNumGrp_10;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_11 != pCCU->pOldSet.ccuSettings.byNumGrp_11)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_11=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_11);
		BYTE byData[5];
		byData[0] = 0x1B;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_11;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_11 = pCCU->pNewSet.ccuSettings.byNumGrp_11;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_12 != pCCU->pOldSet.ccuSettings.byNumGrp_12)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byNumGrp_12=[%d]\n", n,pCCU->pNewSet.ccuSettings.byNumGrp_12);
		BYTE byData[5];
		byData[0] = 0x1C;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byNumGrp_12;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			TRACE("\n---------CCU SyncFail [%d]--->ccccccc-----\n" );
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byNumGrp_12 = pCCU->pNewSet.ccuSettings.byNumGrp_12;
	}
	//----- PWM1----- //
	if((pCCU->pNewSet.ccuSettings.byFan1_WorkMode_M != pCCU->pOldSet.ccuSettings.byFan1_WorkMode_M) || 
		(pCCU->pNewSet.ccuSettings.byFan1_FanWorkMode_N != pCCU->pOldSet.ccuSettings.byFan1_FanWorkMode_N))
	{
		BYTE byData[5];
		byData[0] = 0x20;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan1_WorkMode_M;
		byData[2] = (BYTE)pCCU->pNewSet.ccuSettings.byFan1_FanWorkMode_N;
		if(pCCU->pNewSet.ccuSettings.byFan1_WorkMode_M == 0)
		{
			byData[2] = 0;
		}
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan1_WorkMode_M = pCCU->pNewSet.ccuSettings.byFan1_WorkMode_M;
		pCCU->pOldSet.ccuSettings.byFan1_FanWorkMode_N = pCCU->pNewSet.ccuSettings.byFan1_FanWorkMode_N; 
	}

	if(pCCU->pNewSet.ccuSettings.nFan1_OverTemp4Cool != pCCU->pOldSet.ccuSettings.nFan1_OverTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan1_OverTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan1_OverTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x21;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan1_OverTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2 , 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan1_OverTemp4Cool = pCCU->pNewSet.ccuSettings.nFan1_OverTemp4Cool;
	}

	if(pCCU->pNewSet.ccuSettings.nFan1_UltraTemp4Cool != pCCU->pOldSet.ccuSettings.nFan1_UltraTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan1_UltraTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan1_UltraTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x22;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan1_UltraTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan1_UltraTemp4Cool = pCCU->pNewSet.ccuSettings.nFan1_UltraTemp4Cool;
	}
	//----- PWM2----- //
	if((pCCU->pNewSet.ccuSettings.byFan2_WorkMode_M != pCCU->pOldSet.ccuSettings.byFan2_WorkMode_M) || 
		(pCCU->pNewSet.ccuSettings.byFan2_FanWorkMode_N != pCCU->pOldSet.ccuSettings.byFan2_FanWorkMode_N))
	{
		BYTE byData[5];
		byData[0] = 0x23;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan2_WorkMode_M;
		byData[2] = (BYTE)pCCU->pNewSet.ccuSettings.byFan2_FanWorkMode_N;
		if(pCCU->pNewSet.ccuSettings.byFan2_WorkMode_M == 0)
		{
			byData[2] = 0;
		}
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan2_WorkMode_M = pCCU->pNewSet.ccuSettings.byFan2_WorkMode_M;
		pCCU->pOldSet.ccuSettings.byFan2_FanWorkMode_N = pCCU->pNewSet.ccuSettings.byFan2_FanWorkMode_N; 
	}

	if(pCCU->pNewSet.ccuSettings.nFan2_OverTemp4Cool != pCCU->pOldSet.ccuSettings.nFan2_OverTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan2_OverTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan2_OverTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x24;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan2_OverTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2 , 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan2_OverTemp4Cool = pCCU->pNewSet.ccuSettings.nFan2_OverTemp4Cool;
	}

	if(pCCU->pNewSet.ccuSettings.nFan2_UltraTemp4Cool != pCCU->pOldSet.ccuSettings.nFan2_UltraTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan2_UltraTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan2_UltraTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x25;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan2_UltraTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan2_UltraTemp4Cool = pCCU->pNewSet.ccuSettings.nFan2_UltraTemp4Cool;
	}
	//----- PWM3----- //
	if((pCCU->pNewSet.ccuSettings.byFan3_WorkMode_M != pCCU->pOldSet.ccuSettings.byFan3_WorkMode_M) || 
		(pCCU->pNewSet.ccuSettings.byFan3_FanWorkMode_N != pCCU->pOldSet.ccuSettings.byFan3_FanWorkMode_N))
	{
		BYTE byData[5];
		byData[0] = 0x26;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byFan3_WorkMode_M;
		byData[2] = (BYTE)pCCU->pNewSet.ccuSettings.byFan3_FanWorkMode_N;
		if(pCCU->pNewSet.ccuSettings.byFan3_WorkMode_M == 0)
		{
			byData[2] = 0;
		}
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 3, 1000))
		{
 			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byFan3_WorkMode_M = pCCU->pNewSet.ccuSettings.byFan3_WorkMode_M;
		pCCU->pOldSet.ccuSettings.byFan3_FanWorkMode_N = pCCU->pNewSet.ccuSettings.byFan3_FanWorkMode_N; 
	}

	if(pCCU->pNewSet.ccuSettings.nFan3_OverTemp4Cool != pCCU->pOldSet.ccuSettings.nFan3_OverTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan3_OverTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan3_OverTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x27;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan3_OverTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2 , 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan3_OverTemp4Cool = pCCU->pNewSet.ccuSettings.nFan3_OverTemp4Cool;
	}

	if(pCCU->pNewSet.ccuSettings.nFan3_UltraTemp4Cool != pCCU->pOldSet.ccuSettings.nFan3_UltraTemp4Cool)
	{
		TRACE("pCCU->pNewSet.ccuSettings.nFan3_UltraTemp4Cool= %d\n",pCCU->pNewSet.ccuSettings.nFan3_UltraTemp4Cool);
		BYTE byData[5];
		byData[0] = 0x28;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.nFan3_UltraTemp4Cool;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.nFan3_UltraTemp4Cool = pCCU->pNewSet.ccuSettings.nFan3_UltraTemp4Cool;
	}

	if(pCCU->pNewSet.ccuSettings.byPCSType != pCCU->pOldSet.ccuSettings.byPCSType)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byPCSType= %d\n",pCCU->pNewSet.ccuSettings.byPCSType);
		BYTE byData[5];
		byData[0] = 0x29;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byPCSType;
		if(DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byPCSType = pCCU->pNewSet.ccuSettings.byPCSType;
	}

	if(pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode != pCCU->pOldSet.ccuSettings.byDCorAC_AdjMode)
	{
		TRACE("pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode= %d\n",pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode);
		BYTE byData[5];
		byData[0] = 0x2A;
		byData[1] = (BYTE)pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode;
		if(!DoTCPSession(pCCU, NL_MSG_CFGS, byData, 2, 1000))
		{
			return FALSE;
		}
		pCCU->pOldSet.ccuSettings.byDCorAC_AdjMode = pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode;
	}

	return TRUE;
}
 
 

/*********************************************************************************
*  
*  FUNCTION NAME : MangeLEDStatus
*  CREATOR       :  
*  CREATED DATE  : 2021-8-31 15:13:53
*  DESCRIPTION   : MLXY  Power | DO2:Battery Charging | DO4:Battery Discharging | DO1:PV | DO3:Error
*  
*  CHANGE HISTORY: A2105 Power | DO1:Battery Charging | DO2:Battery Discharging | DO4:OFFLine | DO3:Error
*  
***********************************************************************************/
void MangeLEDStatus(ROUGH_CCU_DATA* pCCU)
{
	int 	n = 0;
	static 	BOOL bFirstRun = TRUE;
	static 	BOOL s_bFailLEDStatus = FALSE;

	static 	BOOL s_bLEDCharge_stBatt = FALSE;
	static 	BOOL s_bLEDDisCharge_stBatt = FALSE;

	static 	BOOL s_bPVWorkStatu = FALSE;
	static 	BOOL s_bOffGridStatu = FALSE;

	double tmNow = GetCurrentTime();

	if(fabs(tmNow - pCCU->tm_LastCtrlLED < 2.0))
	{
		return;
	}
	pCCU->tm_LastCtrlLED = tmNow;

	MPPT_WORKSTATU emMpptStatu = STATU_MPPT_IDLE;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->esRunData.emMPPT_WorkStatu[n] == STATU_MPPT_RECTIFY)
		{
			emMpptStatu = STATU_MPPT_RECTIFY;
			break;
		}
	}


	STBATT_RUN_STATU emSTBatt_ChgWorkStatu = STBATT_RUNSTAT_IDLE;
	STBATT_RUN_STATU emSTBatt_DischgWorkStatu = STBATT_RUNSTAT_IDLE;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->esRunData.fSTBatt_OutCurr[n] >= 2.0) //2021-09-07,When Sleep Mode, battery Also Incharging Statu
		{
			emSTBatt_ChgWorkStatu = STBATT_RUNSTAT_CHARGE;
			continue;
		}

		if(pCCU->esRunData.fSTBatt_OutCurr[n] <= -2.0)
		{
			emSTBatt_DischgWorkStatu = STBATT_RUNSTAT_DISCHARGE;
			continue;
		}
	}
 
	STBATT_ALARM_STATU emBattFaultStatu = STBATT_ALMSTAT_NOMAL;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->blobSingleBattData[n].emBattStatus_Alm == STBATT_ALMSTAT_FAULT)
		{
			emBattFaultStatu = STBATT_ALMSTAT_FAULT;
			break;
		}
	}

	//TRACE("emMpptStatu=[%d]  emSTBatt_ChgWorkStatu=[%d] emSTBatt_DischgWorkStatu=[%d]\n", \
	//	emMpptStatu, emSTBatt_ChgWorkStatu, emSTBatt_DischgWorkStatu);

	//2. stBatt Charging LED 
	if(emSTBatt_ChgWorkStatu == STBATT_RUNSTAT_CHARGE)
	{
		if(!s_bLEDCharge_stBatt)
		{
			s_bLEDCharge_stBatt = Set_IMMU_DO(IOC_SET_DO1, 1) ? TRUE : FALSE;
		}
	}
	else 
	{
		if(s_bLEDCharge_stBatt)
		{
			s_bLEDCharge_stBatt = Set_IMMU_DO(IOC_SET_DO1, 0) ? FALSE : TRUE;
		}
 	}

	//3. stBatt DisCharging LED 
	if(emSTBatt_DischgWorkStatu == STBATT_RUNSTAT_DISCHARGE)
	{
		if(!s_bLEDDisCharge_stBatt)
		{
			s_bLEDDisCharge_stBatt = Set_IMMU_DO(IOC_SET_DO2, 1) ? TRUE : FALSE;
		}
	}
	else  
	{
		if(s_bLEDDisCharge_stBatt)
		{
			s_bLEDDisCharge_stBatt = Set_IMMU_DO(IOC_SET_DO2, 0) ? FALSE : TRUE;
		}
	}
	
	if(pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed == 1) //0-Disable, 1-Enable
	{
		if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_OFFGRID) 
		{
			if(!s_bOffGridStatu)
			{
				s_bOffGridStatu = Set_IMMU_DO(IOC_SET_DO4, 1) ? TRUE : FALSE;
			}
		}
		else
		{
			if(s_bOffGridStatu)
			{
				s_bOffGridStatu = Set_IMMU_DO(IOC_SET_DO4, 0) ? FALSE : TRUE;
			}	
		}
	}
	else
	{
		//4. PV LED Statu 
		if(emMpptStatu == STATU_MPPT_RECTIFY)
		{
			if(!s_bPVWorkStatu)
			{
				s_bPVWorkStatu = Set_IMMU_DO(IOC_SET_DO4, 1) ? TRUE : FALSE;
			}
		}
		else 
		{
			if(s_bPVWorkStatu)
			{
				s_bPVWorkStatu = Set_IMMU_DO(IOC_SET_DO4, 0) ? FALSE : TRUE;
			}	
		}
	}

	//5. System Alarm LED
	if((pCCU->emCCUState == CCU_STATE_FAULTED)||(emBattFaultStatu == STBATT_ALMSTAT_FAULT))
	{
		if(!s_bFailLEDStatus)
		{
			s_bFailLEDStatus = Set_IMMU_DO(IOC_SET_DO3, 1) ? TRUE : FALSE;
		}
	}
	else 
	{
		if(s_bFailLEDStatus)
		{
			s_bFailLEDStatus = Set_IMMU_DO(IOC_SET_DO3, 0) ? FALSE : TRUE;
		}
	}

	return;
}

void CalcMPPTGenerateKWH(ROUGH_CCU_DATA* pCCU)
{
	int 		n = 0;
	VAR_VALUE 	varV;
	ERRCDDE_DPR err;

	double 	tm_fNow = GetCurrentTime();
	double 	tmDiff = (tm_fNow - pCCU->strLSYSInfo.tm_fLastCalcMPPT);
	float 	fMPPT_DischrgkWh_Diff = 0.0;
	static  BOOL 	bFirstRun = TRUE;

	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	if(bFirstRun)
	{
		bFirstRun = FALSE;
		//Get MPPt His Generate KWH
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			err = pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 204 + n, &varV, 4);
			if(err == ERR_DPR_OK)
			{
				pCCU->strLSYSInfo.fMPPTGenerateKWH[n] = varV.fValue;
				pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n] = varV.fValue;

				TRACE("Node=[%d] MPPT_Curr_KWH = %f fLast_MPPTGenerateKWH=[%.1f]\n",\
					n, pCCU->strLSYSInfo.fMPPTGenerateKWH[n], pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n]);
			}
		}
		pCCU->strLSYSInfo.tm_fLastCalcMPPT = tm_fNow; //The First Time Enter
		return;
	}

 	if((tmDiff > 0.5) && (tmDiff < 5.0))
	{
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			fMPPT_DischrgkWh_Diff = fabs(pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt * pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr * 0.001 * tmDiff/3600); 
			//TRACE("fMPPT_DischrgkWh_Diff=%f fMPPT_RealTime_Volt=%f fMPPT_RealTime_Curr=%f\n", \
			//fMPPT_DischrgkWh_Diff,\
			//pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt,\
			//pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr);
			if((fMPPT_DischrgkWh_Diff > 0.00002) && (fMPPT_DischrgkWh_Diff < 5.0))
			{
				pCCU->strLSYSInfo.fMPPTGenerateKWH[n] += fMPPT_DischrgkWh_Diff;
			}
			
			//TRACE("Node=[%d] tmDiff =%f Chrged_kWhCost_Diff=%f fAll_ChargedEnergy=%f fExportVolt=%f, fExportCurr=%f\n",\
			//	n, tmDiff, fMPPT_DischrgkWh_Diff, pCCU->strLSYSInfo.fMPPTGenerateKWH[n],\
			//	pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt, pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr);
		}

		pCCU->strLSYSInfo.tm_fLastCalcMPPT = tm_fNow;
	}
	else if(tmDiff > 5.0)
	{
		pCCU->strLSYSInfo.tm_fLastCalcMPPT = tm_fNow; //The Time Changed
	}

 	return;
}

void CalcDCLoadCostKWH(ROUGH_CCU_DATA* pCCU)
{
	int 		n = 0;
	VAR_VALUE 	varV;
	ERRCDDE_DPR err;

	double 	tm_fNow = GetCurrentTime();
	double 	tmDiff = (tm_fNow - pCCU->strLSYSInfo.tm_fLastCalcDCLoad);
	float 	fDCLoadCost_Diff = 0.0;
	static  BOOL 	bFirstRun = TRUE;

	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

	if(bFirstRun)
	{
		bFirstRun = FALSE;
		//Get DCLoad His Cost KWH
		err = pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 208, &varV, 4);
		if(err == ERR_DPR_OK)
		{
			pCCU->strLSYSInfo.fDCLoad_CostKWH = varV.fValue;
			pCCU->strLSYSInfo.fLast_DCLoad_CostKWH = varV.fValue;
			TRACEX("Node=[%d] fDCLoad_CostKWH KWH=[%.1f] fLast_DCLoad_CostKWH=[%.1f]\n", \
				n, pCCU->strLSYSInfo.fDCLoad_CostKWH, pCCU->strLSYSInfo.fLast_DCLoad_CostKWH);
		}
		
		pCCU->strLSYSInfo.tm_fLastCalcDCLoad = tm_fNow; //The First Time Enter
		return;
	}

 	if((tmDiff > 0.5) && (tmDiff < 5.0))
	{
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			fDCLoadCost_Diff = fabs(pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt * pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr * 0.001 * tmDiff/3600); 
	 
			if((fDCLoadCost_Diff > 0.00002) && (fDCLoadCost_Diff < 5.0))
			{
				pCCU->strLSYSInfo.fDCLoad_CostKWH += fDCLoadCost_Diff;
			}
			
			//TRACE("Node=[%d] tmDiff =%f fDCLoadCost_Diff=%f fAll_ChargedEnergy=%f fExportVolt=%f, fExportCurr=%f\n",\
			//	n, tmDiff, fDCLoadCost_Diff, pCCU->strLSYSInfo.fDCLoad_CostKWH,\
			//	pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt, pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr);
		}

		pCCU->strLSYSInfo.tm_fLastCalcDCLoad = tm_fNow;
	}
	else if(tmDiff > 5.0)
	{
		pCCU->strLSYSInfo.tm_fLastCalcDCLoad = tm_fNow; //Maybe The Time Changed
	}

 	return;
}

static BOOL JudgeBattCabinetStatu(ROUGH_CCU_DATA* pCCU)
{
	int 		n = 0;
	BOOL 		bRet = FALSE;
	VAR_VALUE 	varV;
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	//Get Cfg Battery Number
	pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR,  500 , 502, &varV, 4);
	UINT nCfgBattNumber = varV.ulValue;
	TRACE("nCfgBattNumber =%d\n", nCfgBattNumber);

	//Battery Merge Mode,Only Config 1 Battery ,but has 2 Battery Cabinet
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 601 + n, 456, &varV, 4);
  		if(varV.emValue != 0)
		{
			bRet = TRUE;
			break;
		}

		pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 601 + n, 451, &varV, 4);
		if(varV.emValue != 0)
		{
			bRet = TRUE;
			break;
		}
		//pCCU->mapSysFault.Fault.Flood_Alarm = varV.emValue;	
		//TRACEX("Flood Alm:varV.emValue=%d\n",varV.emValue);

		//pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 601 + n, 452, &varV, 4);
		//pCCU->mapSysFault.Fault.DoorOpen_Alarm = varV.emValue;	
		//TRACEX("DoorOpen Alm:varV.emValue=%d\n",varV.emValue);

		pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 601 + n, 453, &varV, 4);
		if(varV.emValue != 0)
		{
			bRet = TRUE;
			break;
		}
		//pCCU->mapSysFault.Fault.Smoke_Alarm = varV.emValue;	
		//TRACEX("Smoke Alm:varV.emValue=%d\n",varV.emValue);

		pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 601 + n, 454, &varV, 4);
		if(varV.emValue != 0)
		{
			bRet = TRUE;
			break;
		}
		//pCCU->mapSysFault.Fault.Fire_Alarm = varV.emValue;	
		//TRACEX("Fire Alm:varV.emValue=%d\n",varV.emValue);

		//pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 701 + n, 457, &varV, 4);
		//pCCU->mapSysFault.Fault.AirCond_CommFail = varV.emValue;

		//pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 701 + n, 450, &varV, 4);
		//pCCU->mapSysFault.Fault.AirCond_Alarm = varV.emValue;
	}

	return bRet;
}
 
static SYSTEM_FAILREASON GetExecptionFaultCode(ROUGH_CCU_DATA* pCCU)
{
	SYSTEM_FAILREASON	errCode = SYSFAIL_REASON_NOFAIL;
	int n = 0;
	
	if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == 1)//0:On-Grid, 1:Off-Grid
	{
		return errCode;
	}

	if(pCCU->pNewSet.ccuSettings.byNumGrp_5 > 0 || pCCU->pNewSet.ccuSettings.byNumGrp_6 > 0) //MPPT exist
	{
		return errCode;
	}

 	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->blobSingleBattData[n].emBattStatus_Run == STBATT_RUNSTAT_NOT_INSTALLED)
		{
			continue;
		}

		if((TYPE_SIG_RECT_ACDC_GRP1+n) >= MAX_RECT_TYPE)
		{
			break;
		}

		float fBatRackCurr = fabs(pCCU->blobSingleBattData[n].fPackTotalCurr);
		float fPMRealTimeCurr = fabs(pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Curr);

		//float fBatRackVolt = fabs(pCCU->blobSingleBattData[n].fPackTotalVolt);
		//float fAllow_LmtChargeVolt =  fabs(pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt);
		//float fAllow_LmtDisChargeVolt =  fabs(pCCU->blobSingleBattData[n].fAllow_LmtDisChargeVolt);

		//OverCurrent 20A, Abnormal Current,need Stop.
		if((fabs(fBatRackCurr - fPMRealTimeCurr)) > 20.0) 
		{
			TRACE("Idx=[%d] fBatRackCurr=[%.1f] fPMRealTimeCurr=[%.1f] nAbnormalCurr_Count=[%d]\n", \
				n, fBatRackCurr, fPMRealTimeCurr, pCCU->nAbnormalCurr_Count);

			pCCU->nAbnormalCurr_Count++;
			if(pCCU->nAbnormalCurr_Count >= MAX_ABNM_TIMES)
			{
				pCCU->emCurrDiffAbnormal = 1;//0 NORMAL 1 ABNORMAL
				pCCU->nAbnormalCurr_Count = MAX_ABNM_TIMES;
				errCode =  SYSFAIL_REASON_MASTER_FAULT;

				//LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_ERROR, "BMSAbn:Idx[%d]Ipm[%.1f]Irack[%.1f]Vrack[%.1f]VchgLmt[%.1f]VdischLmt[%.1f]",
				//	n ,fPMRealTimeCurr,fBatRackCurr, fBatRackVolt, fAllow_LmtChargeVolt, fAllow_LmtDisChargeVolt);
				break;
			}
		}
		else
		{
			pCCU->emCurrDiffAbnormal = 0;//0 NORMAL 1 ABNORMAL
			pCCU->nAbnormalCurr_Count = 0;
		}
	}

	return errCode;
}

void Refresh_SysAvailState(ROUGH_CCU_DATA* pCCU)
{
	int 		n = 0 ;
	VAR_VALUE 	varV;

	if(pCCU->emCCUState == CCU_STATE_LOCKED)
	{
		TRACE("System Is Locked emCCUState=[%d]!!!\n", pCCU->emCCUState);
		return;
	}

	// //1. Judge CCU Comm Statu
	// if(pCCU->bCommFail)
	// {
	// 	pCCU->emCCUState = CCU_STATE_FAULTED;
	// 	pCCU->emSystemFailReason = SYSFAIL_REASON_CCU_COMFAIL;
	// 	return;
	// }
	
	if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == 0)//0-On-Grid State
	{
		//20230228 John.added Grid/Aux/Cab AC Meter CommFail Alarm,Need Dsiable System.
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 407, 401, &varV, 4); //Cab Side
		if(varV.emValue != ALARM_STATUS_CEASED)
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_METER_COMFAIL;
			return;
		}

		if(pCCU->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1, 60, &varV, 4); //Pri Grid Meter Side
			pCCU->nCurrPriGridMeterId = varV.ulValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, pCCU->nCurrPriGridMeterId, 401, &varV, 4); //Pri Grid Meter Side
		}
		else
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 408, 401, &varV, 4); //Grid Side
		}
		if(varV.emValue != ALARM_STATUS_CEASED)
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_METER_COMFAIL;
			return;
		}

		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 409, 401, &varV, 4); //Aux Side
		if(varV.emValue != ALARM_STATUS_CEASED)
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_METER_COMFAIL;
			return;
		}

		//----- To Protect HT ESS#2 will Run when ESS#1 is Off-Grid RunStatus 2023-12-24---//
		if((pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed == 1) //0-Disable, 1-Enable
			&& (pCCU->strLSYSInfo.sysAlarmMap1.Alarm.LowVoltMCBBreaker == 1) 
			&& (pCCU->esRunData.fACVolt_L1 > 150)
			&& (pCCU->esRunData.fSTS_InputVolt[0] > 150))
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_INPUTQF_DISCNECT;
			return;
		}
	}
	else
	{
		if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.STS1_CommFail == 1)//1-CommFail
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_ABU_COMMFAIL;
			return;
		}
	}

	//---20230303. GetExecptionFaultCode By Abnormal Value---//
	pCCU->emSystemFailReason = GetExecptionFaultCode(pCCU);
	if(pCCU->emSystemFailReason != SYSFAIL_REASON_NOFAIL)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		return;
	}

	//----- Leakage Current Detection Alarm -----//
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 1, 40, &varV, 4);
	if(varV.emValue != ALARM_STATUS_CEASED)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_LEAKAGECURR_ALM;
		return;
	}

	//Power Cab Smoke Sensor
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 200, 261, &varV, 4);
	if(varV.emValue != ALARM_STATUS_CEASED)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_OTHER;
		return;
	}
	//Power Cab Flooding DI
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 200, 262, &varV, 4);
	if(varV.emValue != ALARM_STATUS_CEASED)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_OTHER;
		return;
	}
	//Power Cab Topple DI
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 200, 263, &varV, 4);
	if(varV.emValue != ALARM_STATUS_CEASED)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_OTHER;
		return;
	}

	// added by LJH 20240704 ccu fault // LJH240919 added:认证测试不需要安装电池柜
	STBATT_ALARM_STATU emBattFaultStatu = STBATT_ALMSTAT_FAULT;
	int nBattInstalled = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->blobSingleBattData[n].emBattStatus_Run == STBATT_RUNSTAT_NOT_INSTALLED)
		{
			continue;
		}
		nBattInstalled++;
		if(pCCU->blobSingleBattData[n].emBattStatus_Alm != STBATT_ALMSTAT_FAULT)
		{
			// 多组电池柜，只要一组电池柜正常就算系统正常
			emBattFaultStatu = pCCU->blobSingleBattData[n].emBattStatus_Alm;
			break;
		}
	}
	// TRACED("Refresh_SysAvailState emLSysAvaiStatus:%d emBattFaultStatu:%d nBattInstalled:%d", pCCU->strLSYSInfo.emLSysAvaiStatus, emBattFaultStatu, nBattInstalled);
	if (emBattFaultStatu==STBATT_ALMSTAT_FAULT && nBattInstalled>0)
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_BMS_FAULT;
		return;
	}
	
	if(pCCU->strLSYSInfo.pmGrpAlmMap.uiFlag & 0x01) // 所有模块组失效
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;
		pCCU->emSystemFailReason = SYSFAIL_REASON_CCU_FAULTED;
		return;
	}

	//4. sys state from LSYS
	if (pCCU->strLSYSInfo.emLSysAvaiStatus != SYS_STATUS_Faulted)
	{
		pCCU->emCCUState = CCU_STATE_IDLE;
		pCCU->emSystemFailReason = SYSFAIL_REASON_NOFAIL;
		return;
	}
	else
	{
		pCCU->emCCUState = CCU_STATE_FAULTED;

		if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.DoorOpen)
		{
			TRACE("Jufge SYSALARM: DoorOpen! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_DOOR_OPENED;
		}
		else if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.EPO)
		{
			//TRACEX("Jufge SYSALARM: EPO! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_EPO_PRESSED;
		}
		else if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.SPD)
		{
			TRACEX("Jufge SYSALARM: SPD ! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_SPD_ALARM;
		}
		else if(pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectFail)
		{
			TRACEX("SYSALARM: All Rect Fail ! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_ALLRECT_FAIL;
		}
		else if(pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectCommFail)
		{
			TRACEX("SYSALARM: All Rect Comm Fail ! \n");
 			pCCU->emSystemFailReason = SYSFAIL_REASON_ALLRECT_COMMFAIL;
		}
		else if((pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation1_CommFail)||
		(pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation2_CommFail)||
		(pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation3_CommFail)||
		((pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation4_CommFail)))
		{
			TRACEX("SYSALARM: Insulation Comm Fail ! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_INSULATION_COMMFAIL;
		}
		else if((pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation1_detectAlm)||
				(pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation2_detectAlm)||
				(pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation3_detectAlm)||
				(pCCU->strLSYSInfo.sysAlarmMap2.Alarm.Insulation4_detectAlm))
		{
			TRACE("SYSALARM: Insulation Detect Alarm ! \n");
			pCCU->emSystemFailReason = SYSFAIL_REASON_INSULATION_DETECTALM;
		}
 		else
		{
			pCCU->emSystemFailReason = SYSFAIL_REASON_CCU_FAULTED;
		}
	}
	return;
}
 

/*********************************************************************************
*  
*  FUNCTION NAME :  Manage_ECOMode
*  CREATOR       :  
*  CREATED DATE  : 2022-03-13 11:13:53
*  DESCRIPTION   :  
*				ECO模式使能&&
*				非离网模式&&
*				无MPPT模块&&
*				非紧急放电模式&&
*				EMS计划本时段为待机状态&&
*				EMS计划待机时段持续时长超30Min &&
*				非手动模式
*				柜内电表功率＜10KW
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
 
static void Judge_IfNeedDisConnectACLoadMCB(ROUGH_CCU_DATA* pCCU)
{
	BYTE byData[5]={'\0'};
	time_t tmNow = _NOW_;

	if(ABS(tmNow - pCCU->tmLastCtrlACLoadMCB) < 30)
	{
		return;
	}

	TRACE("f_PgridMeter=%.1f fACLoadPwrProtcPoint=%.1f\n", \
		pCCU->esRunData.f_PgridMeter, pCCU->blobBackendCfg.fACLoadPwrProtcPoint);

	if(pCCU->esRunData.f_PgridMeter > pCCU->blobBackendCfg.fACLoadPwrProtcPoint)
	{
		if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACLoadOutputMCBQF2 == 0) //0-Connect Status
		{
			byData[0] = 0x19;
			byData[1] = 0xA0; //DisConnect
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			pCCU->tmLastCtrlACLoadMCB = tmNow;
		}
	}
	else
	{
		if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACLoadOutputMCBQF2 == 1) //0-DisConnect Status
		{
			byData[0] = 0x19;
			byData[1] = 0xA1; //Connect
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			pCCU->tmLastCtrlACLoadMCB = tmNow;
		}
	}
	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : Check_MgmtPlanCfg
*  CREATOR       :  
*  CREATED DATE  : 2021-08-30 11:36:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_MGMTCFG Check_MgmtPlanCfg(IN ROUGH_CCU_DATA* pCCU)
{
	ERR_CODE_MGMTCFG err = ERR_MGMT_NO_ERR;
	int n = 0, m = 0;
	UINT nTotalInterval = 0;

	if (pCCU->lcCfg.lcType != LC_TYPE_1st) //Slave don't need to alarm
	{
		err = ERR_MGMT_NO_ERR;
		return err;
	}

	A_BLOB_EMS_CONFIG_PLAN* pEmsMgmtPlan = &pCCU->blobEmsCfgPlan;
	if((pEmsMgmtPlan->nTotal_EmsLmt_Sectors <= 0) || (pEmsMgmtPlan->nTotal_EmsLmt_Sectors > MAX_EMSMGMT_LMT_SETOR))
	{
		err = ERR_MGMT_SECTORNUM;
		return err;
	}

	for(n = 0; n < pEmsMgmtPlan->nTotal_EmsLmt_Sectors; n++)
	{
		if((pEmsMgmtPlan->emsLmtSectors[n].nStopSOC > 100)||(pEmsMgmtPlan->emsLmtSectors[n].nStopSOC < 0))
		{
			err = ERR_MGMT_SOC_ABNORMAL;
			return err;
		}
	}

	for(n = 0; n < pEmsMgmtPlan->nTotal_EmsLmt_Sectors; n++)
	{
		nTotalInterval += pEmsMgmtPlan->emsLmtSectors[n].nInterval;
	}

	if(nTotalInterval % SEC_PER_DAY != 0)
	{
		err = ERR_MGMT_SUMINTERVAL;
		return err;
	}

	for(n = 0; n < pEmsMgmtPlan->nTotal_EmsLmt_Sectors; n++)
	{
		m = n + 1;
		if(m >= pEmsMgmtPlan->nTotal_EmsLmt_Sectors)
		{
			break;
		}
		//TRACE("nCurrt=[%d] tmStart=[%ld] mNext=[%d] tmStart=[%ld]\n",\
		//		n, (pEmsMgmtPlan->emsLmtSectors[n].tmStart + pEmsMgmtPlan->emsLmtSectors[n].nInterval),\
		//		m, pEmsMgmtPlan->emsLmtSectors[m].tmStart);
		
		if(pEmsMgmtPlan->emsLmtSectors[m].tmStart < (pEmsMgmtPlan->emsLmtSectors[n].tmStart + pEmsMgmtPlan->emsLmtSectors[n].nInterval))
		{
			err = ERR_MGMT_STATTM_OVERLAP;
			return err;	
		}
 	}

	return err;
}

void Update_SysDataInfo(IN ROUGH_CCU_DATA* pCCU)
{
	int n = 0;
	VAR_VALUE varV;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		ZERO_POBJS(&pCCU->blobSingleBattData[n], 1);
		varV.blobValue = (BYTE *)&pCCU->blobSingleBattData[n];
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
			501 + n, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		//TRACE("nBattGrp=[%d] emBattStatus_Run=[%d] \
		//	fPackTotalVolt=[%f] fPackTotalCurr[%f] bySOC=[%d] fCharge_Cutoff_Volt=[%f] fAllow_MaxChargeCurr=[%f]\n", \
		//	n,pCCU->blobSingleBattData[n].emBattStatus_Run,\
		//	pCCU->blobSingleBattData[n].fPackTotalVolt,\
		//	pCCU->blobSingleBattData[n].fPackTotalCurr,\
		//	pCCU->blobSingleBattData[n].nSOC,\
		//	pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt,\
		//	pCCU->blobSingleBattData[n].fAllow_MaxChargeCurr);
	}
}

/*********************************************************************************
*  
*  FUNCTION NAME : Sample
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;

	if(pCCU->emESSDevType == LIQUID_COOLING_836)
	{
		return pCCU->pfn_can_Samples(hSamplerThis);
	}
	
	return pCCU->pfnSamples[pCCU->lcCfg.emSystemType](hSamplerThis);
}

static void AutoConfigCCUNewSetByBatNum(ROUGH_CCU_DATA* pCCU, _ENUM emValue)
{
	CCU_BOOTUP_INFO* pNew = &pCCU->pNewSet.ccuSettings;

	BYTE byOffset = 0;
	switch (emValue)
	{
		case 0:
			{
				pNew->sysAtributesMap2.usFlag = 0;
				pNew->sysAtributesMap2.Set.IMEU2_Grp1_Installed = 1;//At least 1 IMEU2

				pNew->sysAtributesMap3.usFlag = 0;
				pNew->sysAtributesMap4.usFlag = 0;
				pNew->byNumGrp_1 = 0;
				pNew->byNumGrp_2 = 0;
				pNew->byNumGrp_3 = 0;
				pNew->byNumGrp_4 = 0;
				pNew->byNumGrp_5 = 0;
				pNew->byNumGrp_6 = 0;
				pNew->byNumGrp_7 = 0;
				pNew->byNumGrp_8 = 0;
				pNew->byNumGrp_9 = 0;
				pNew->byNumGrp_10 = 0;
				pNew->byNumGrp_11 = 0;	
				pNew->byNumGrp_12 = 0;
			}
 			break;
		case 1:
			{
				pNew->sysAtributesMap2.Set.isBattGrp1_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp2_Installed = 0;
				pNew->sysAtributesMap2.Set.isBattGrp3_Installed = 0;
 				pNew->sysAtributesMap2.Set.isBattGrp4_Installed = 0;

				pNew->sysAtributesMap2.Set.IMEU2_Grp1_Installed = 1;
				pNew->sysAtributesMap2.Set.IMEU2_Grp2_Installed = 0;

				pNew->sysAtributesMap2.Set.BmsGrp1_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp2_CANInstalled = 0;
				pNew->sysAtributesMap2.Set.BmsGrp3_CANInstalled = 0;
				pNew->sysAtributesMap2.Set.BmsGrp4_CANInstalled = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_ForBidRun = 0;
				pNew->sysAtributesMap3.Set.BattGrp3_ForBidRun = 0;
				pNew->sysAtributesMap3.Set.BattGrp4_ForBidRun = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_InputContactor = 0;
				pNew->sysAtributesMap3.Set.BattGrp3_InputContactor = 0;
				pNew->sysAtributesMap3.Set.BattGrp4_InputContactor = 0;
			
				pNew->byNumGrp_1 = 2;
				pNew->byNumGrp_2 = 0;
				pNew->byNumGrp_3 = 0;
				pNew->byNumGrp_4 = 0;
			}
 			break;
		case 2:
 			{
				pNew->sysAtributesMap2.Set.isBattGrp1_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp2_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp3_Installed = 0;
 				pNew->sysAtributesMap2.Set.isBattGrp4_Installed = 0;

				pNew->sysAtributesMap2.Set.IMEU2_Grp1_Installed = 1;
				pNew->sysAtributesMap2.Set.IMEU2_Grp2_Installed = 0;

				pNew->sysAtributesMap2.Set.BmsGrp1_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp2_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp3_CANInstalled = 0;
				pNew->sysAtributesMap2.Set.BmsGrp4_CANInstalled = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_ForBidRun = 0;
				pNew->sysAtributesMap3.Set.BattGrp4_ForBidRun = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_InputContactor = 0;
				pNew->sysAtributesMap3.Set.BattGrp4_InputContactor = 0;
	
				pNew->byNumGrp_1 = 2;
				pNew->byNumGrp_2 = 2;
				pNew->byNumGrp_3 = 0;
				pNew->byNumGrp_4 = 0;
			}
			break;
		case 3:
 			{
				pNew->sysAtributesMap2.Set.isBattGrp1_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp2_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp3_Installed = 1;
 				pNew->sysAtributesMap2.Set.isBattGrp4_Installed = 0;

				pNew->sysAtributesMap2.Set.IMEU2_Grp1_Installed = 1;
				pNew->sysAtributesMap2.Set.IMEU2_Grp2_Installed = 0;

				pNew->sysAtributesMap2.Set.BmsGrp1_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp2_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp3_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp4_CANInstalled = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp4_ForBidRun = 0;

				pNew->sysAtributesMap3.Set.BattGrp1_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp4_InputContactor = 0;
	
				pNew->byNumGrp_1 = 2;
				pNew->byNumGrp_2 = 2;
				pNew->byNumGrp_3 = 2;
				pNew->byNumGrp_4 = 0;
			}			
			break;
		case 4:
			{
				pNew->sysAtributesMap2.Set.isBattGrp1_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp2_Installed = 1;
				pNew->sysAtributesMap2.Set.isBattGrp3_Installed = 1;
 				pNew->sysAtributesMap2.Set.isBattGrp4_Installed = 1;

				pNew->sysAtributesMap2.Set.IMEU2_Grp1_Installed = 1;
				pNew->sysAtributesMap2.Set.IMEU2_Grp2_Installed = 1;

				pNew->sysAtributesMap2.Set.BmsGrp1_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp2_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp3_CANInstalled = 1;
				pNew->sysAtributesMap2.Set.BmsGrp4_CANInstalled = 1;

				pNew->sysAtributesMap3.Set.BattGrp1_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_ForBidRun = 1;
				pNew->sysAtributesMap3.Set.BattGrp4_ForBidRun = 1;

				pNew->sysAtributesMap3.Set.BattGrp1_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp2_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp3_InputContactor = 1;
				pNew->sysAtributesMap3.Set.BattGrp4_InputContactor = 1;
				
				pNew->byNumGrp_1 = 2;
				pNew->byNumGrp_2 = 2;
				pNew->byNumGrp_3 = 2;
				pNew->byNumGrp_4 = 2;
			}
			break;
		default:
 			break;
	}


	pNew->sysAtributesMap1.Set.EPOVoltLevel 		= 0; //0-High
	pNew->sysAtributesMap1.Set.DoorOpenEnable 	= 1;//1-Enable
	pNew->sysAtributesMap1.Set.SPDEnable 		= 1;//1-Enable
 	pNew->sysAtributesMap1.Set.ACGridInputMCB 	= 1;//1-Enable
  	pNew->sysAtributesMap1.Set.AC_InputContactor = 1;//1-Enable
 	pNew->sysAtributesMap1.Set.Smoke_Sensor  	= 1;//1-Enable
	pNew->sysAtributesMap1.Set.Flooding_DI 		= 1;//1-Enable
 
 	pNew->nFan3_OverTemp4Cool = 40;
	pNew->nFan3_UltraTemp4Cool = 45;
	pNew->byPCSType = 1; //62.5kW
	pNew->byDCorAC_AdjMode = 0;//0-DC Adjust.
	return;
}
 
int OnHandle_CtrlCmd4CANMsgTrig(IN ROUGH_CCU_DATA *pCCU, IN _ENUM emValue)
{
	static BYTE bySendData[] = {0xA0, 0x00, 0x01, 0x02, 0x03, 0x0C, 0xFF};
	BYTE byData[5]={'0'};

	pCCU->tagPM2CCU_GCan.emTrigMsgTp =  (PM2CCU_TRIGGER_MSG)emValue;
	
	if(pCCU->tagPM2CCU_GCan.emTrigMsgTp != PM2CCU_TRIG_OFF)
	{
		ZERO_POBJS(&pCCU->blobCanMsg, 1);
	}

	byData[0] = bySendData[emValue];

	pCCU->tagPM2CCU_GCan.tm_LastStatTrig = _NOW_;

	DoTCPSession(pCCU, NL_MSG_GCAN, byData, 4, 500); 

	TRACE("OnHandle_CtrlCmd4CANMsgTrig:byData[0]=[%02X]\n", byData[0]);

	return ERR_CONTROL_OK;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Control
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
	int			nDevId,
	int			nParamId,
	VAR_VALUE	*pVarVal,
	int			nValSize,
	int			nTimeOut)
{
	TRACE("ccu_sampler_main:Control emParaType =%d nDevId=%d nParamId=%d ulValue=%lu %.1f nValSize=%d \n",
		emParaType, nDevId,nParamId, pVarVal->ulValue, pVarVal->fValue, nValSize);
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA *pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	int nOnlineSingleNum = pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[nDevId - 701];
	BYTE pbySendBuf[32]={'\0'};
	BYTE byPCS_SetData[10]={'\0'};
	int  nSetValue = 0;

// #ifdef IMMU2
// 	if(pCCU->hDataComm == NULL && pCCU->lcCfg.emSystemType < SYSTYPE_EMS_STATION) // LJH250625 fix:SEMS使用 s_MMClients[nInComingCCU_Idx].hClientPort
// 	{
// 		TRACEE("hDataComm == NULL, Retrun Back\n");
// 		return ERR_CONTROL_FAILED;
// 	}
// #endif
// 	if (pCCU->bCommFail)
// 	{
// 		TRACEE("ccu_sampler_main:Control return ERR_CONTROL_FAILED bCommFail");
// 		return ERR_CONTROL_FAILED;
// 	}

	double tmNow = GetCurrentTime();
	int nRet = ERR_CONTROL_OK;
	int n;
	VAR_VALUE varV;
	if(Mutex_Lock(pCCU->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		TRACEE("ccu_sampler_main:Control ERR_CONTROL_TIMEOUT");
		return ERR_CONTROL_TIMEOUT;
	}
	
	if(emParaType == PARAM_TYPE_SETTING)
	{
		if(nParamId == 216)
		{
			pCCU->emOnOffGridCtlNLineGround = pVarVal->emValue;
		}
		else if(nParamId == 217) //John 20220718 .Mode Change Use Set Cmd.
		{
			TRACED("Ctrl: Enter Debug Mode emValue =%d\n",pVarVal->emValue);
			if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
			{
				// LJH250606 deleted:SEMS直接返回成功
				// for (n = 0; n < pEMS->nNodeNum; n++)
				// {
				// 	Send_6317UCTL_CtrlRectMode(n, (pVarVal->emValue == 0) ? CtrlRectMode_Auto : CtrlRectMode_Manu);
				// }
			}
			else
			{
				BYTE byData[5]={'\0'};
				byData[0] = 0x17;
				byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
				DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			}
			nRet = ERR_CONTROL_OK;
			pCCU->emPMCtrlMode = (STAT_PM_CTRL_MODE)pVarVal->emValue;

			if(pCCU->emPMCtrlMode == MANUAL_CTRL_PM_MODE) //不是自动   那就是自动切手动 需要停机
			{	
				t_ACDCControlData ctrlData;
				ctrlData.eWorkMode = eCDMode_Stop;
				for(n = 1; n <= 2; n++)
				{
					Set235kWPCS_WrokMode(pCCU, n, ctrlData);
				}	
			}
			// if(pCCU->emPMCtrlMode != AUTO_CTRL_PM_MODE) //不是自动   那就是自动切手动 需要停机
			// {
			// 	for(n = 0; n < 2; ++n)
			// 	{
			// 		A_BLOB_MANUALCTRL_RECT_CMD blobCont;
			// 		ZERO_POBJS(&blobCont, 1);

			// 		blobCont.emBlobType = BLOB_TYPE_MANUALCTRL_RECT;
			// 		blobCont.emState = STAT_MANUAL_STOP;   //枚举修改 pending 
			// 		blobCont.nGrp = n;//液冷一体柜  默认0x00；All Group  
			// 		blobCont.fGrpVolt = 0;
			// 		blobCont.fGrpCurr = 0;
			// 		blobCont.nStopSOC = 0;
			// 		blobCont.nAdjMode = 0;
			// 		blobCont.fActv_Pwr = 0;
			// 		blobCont.fReactv_Pwr = 0;
			// 		varV.blobValue = (BYTE *)&blobCont;
			// 		Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
			// 		200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
			// 	}	
			// }
 		}
		else if(nParamId == 219) //On/Off Grid Mode 
		{
			TRACE("Manual Ctrl: ON/Off-Grid Mode Ctrl=%d\n",pVarVal->emValue);
			if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
			{
				// LJH250606 暂时关闭
				return ERR_CONTROL_FAILED;
				// for (n = 0; n < pEMS->nNodeNum; n++)
				// {
				// 	Send_6302UCTL_OnOffGridMode(n, (pVarVal->emValue == 0) ? OnOffGridMode_On_Invert : OnOffGridMode_Off);
				// }
			}
			else if(pCCU->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED)
			{
				int N=2,NG=2,RetryTime=5;
				while(RetryTime--)
				{
					varV.emValue = (pVarVal->emValue == 0) ? 0 : 1;
					Push_VarValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 611, 40, varV);
					if(pCCU->emOnOffGridCtlNLineGround==0)
					{
						Push_VarValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 611, 41, varV);
					}

					Sleep(1000);

			 		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 611, 1676, &varV, 4);
					N = (int)varV.emValue;
			 		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 611, 1677, &varV, 4);
					NG = (int)varV.emValue;
					// TRACED("N=%d | NG=%d\n", N, NG);
					if(pCCU->emOnOffGridCtlNLineGround==0)
					{
						if((pVarVal->emValue == 0 && !N && !NG) || (pVarVal->emValue == 1 && N==1 && NG==1))
						{
							BYTE byData[3]={'\0'};
							byData[0] = 0x02;
							byData[1] = (pVarVal->emValue == 0) ? 0xA1 : 0xA2;
							DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
	
							Mutex_Unlock(pCCU->hSamplerLock);
							return ERR_CONTROL_OK;
						}
					}
					else
					{
						if((pVarVal->emValue == 0 && !N) || (pVarVal->emValue == 1 && N==1))
						{
							BYTE byData[3]={'\0'};
							byData[0] = 0x02;
							byData[1] = (pVarVal->emValue == 0) ? 0xA1 : 0xA2;
							DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
	
							Mutex_Unlock(pCCU->hSamplerLock);
							return ERR_CONTROL_OK;
						}
					}
				}
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			else
			{
				BYTE byData[3]={'\0'};
				byData[0] = 0x02;
				byData[1] = (pVarVal->emValue == 0) ? 0xA1 : 0xA2;
				DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			}
			nRet = ERR_CONTROL_OK;
			pCCU->emSTSOnOffGridState = (STS_ONOFFGRID_STATE)pVarVal->emValue;
 		}
		else if(nParamId == 218)
		{	
			TRACE("Auto/Manual : ON/Off-Grid Mode Ctrl=%d\n",pVarVal->emValue);
			if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
			{
				// LJH250606 暂时关闭
				return ERR_CONTROL_FAILED;
				// for (n = 0; n < pEMS->nNodeNum; n++)
				// {
				// 	Send_6301UCTL_AutoManuSwitchMode(n, (pVarVal->emValue == 0) ? AutoManuSwitchMode_Auto : AutoManuSwitchMode_Manu);
				// }
			}
			else
			{
				BYTE byData[3]={'\0'};
			
				//Step1.Need Power Off First!!!
				byData[0] = 0x03;
				byData[1] = 0xA1;
				DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
				
				//Step2:Change Mode!!!
				byData[0] = 0x01;
				byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
				BOOL bRet = DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
			}
			nRet = ERR_CONTROL_OK; 
			pCCU->emSTSAutoManual = (STS_AUTOMANUAL_ONOFFGRID)pVarVal->emValue;
		}
		else if(nParamId == 199)
		{	
			if(pVarVal->emValue > MAX_SINGLE_STBATT_NUM)
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			
			//pCCU->pNewSet = *((A_BLOB_CHARGERSPECIFIC_SETTINGS*)pVarVal->blobValue);
			
			AutoConfigCCUNewSetByBatNum(pCCU, pVarVal->emValue);

			//use client 0 to try
			if(!Try_SyncCfgSetting(pCCU))
			{
				TRACE("Auto Config CCU Specific Cfg Failed!\n");
				LogOut(MAIN_MODULE_SAMPLER, "CCU_TCP_SAMP", LOG_TYPE_ERROR, "Auto Config CCU Specific Cfg Failed!");
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			//now OK
			nRet = ERR_CONTROL_OK;
			//now triggle the boot cmd again
			if(DoTCPSession(pCCU, NL_MSG_CFGG, NULL, 0, 500) == FALSE)
			{
				DoTCPSession(pCCU, NL_MSG_CFGG, NULL, 0, 500);//If Get CFGG Param Fail,Try Again.	
			}	

		}
 		else if(nParamId == 200)
		{
			TRACE("---------nValSize=%d sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS=%d---------\n", 
					nValSize,sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
			if(nValSize != sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pCCU->pNewSet = *((A_BLOB_CHARGERSPECIFIC_SETTINGS*)pVarVal->blobValue);

			//use client 0 to try
			// if(!Try_SyncCfgSetting(pCCU))
			// {
			// 	TRACE("Sync CCU Specific Cfg Failed!\n");
			// 	LogOut(MAIN_MODULE_SAMPLER, "CCU_TCP_SAMP", LOG_TYPE_ERROR, "Sync Specific Cfg Failed!");
			// 	Mutex_Unlock(pCCU->hSamplerLock);
			// 	return ERR_CONTROL_FAILED;
			// }

			//now OK
			nRet = ERR_CONTROL_OK;
			//now triggle the boot cmd again
			// DoTCPSession(pCCU, NL_MSG_CFGG, NULL, 0, 100);	
		}
		else if (nParamId == 220)	// CJJ 20250106 for 235kW PCS OnGrid
		{
			TRACE("Control PARAM_TYPE_SETTING 220 %d %lu\n", nValSize, sizeof(A_BLOB_CTRL_PCS_ONGRID_CMD));

			if(nValSize != sizeof(A_BLOB_CTRL_PCS_ONGRID_CMD))
			{
				TRACE("Setting ONGRID data size ERR====================\n");
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_CTRL_PCS_ONGRID_CMD* pCtrlCmd = (A_BLOB_CTRL_PCS_ONGRID_CMD*)pVarVal->blobValue;

			// goto 235kW OnGrid State Machine
			CTRL_PCS_CMDTYPE emCtrlType = pCtrlCmd->emCtrlType;
			// TRACE("emCtrlType = [%d]\n", emCtrlType);
			if (emCtrlType >= CTRL_PCS_STOP && emCtrlType <= CTRL_PCS_ONGRID_INVERT)
			{
				pCCU->nCntSendOnOffGridCmd2CCU = 0;
				pCCU->tmLastSendCmd2CCU += 10000;
				pCtrlCmd->nPMIndex = 0;
				pCCU->pcsSMInfos[pCtrlCmd->nPMIndex].ctrlPcsOnGridReq = *pCtrlCmd;
				// memcpy(&pCCU->pcsSMInfos[pCtrlCmd->nPMIndex].ctrlPcsOnGridReq, pCtrlCmd, sizeof(A_BLOB_CTRL_PCS_ONGRID_CMD));
				// pCCU->pcsSMInfos[pCtrlCmd->nPMIndex].ctrlPcsOnGridReq.emCtrlType = emCtrlType;

				nRet = ERR_CONTROL_OK;
			}
			else
			{
				nRet = ERR_CONTROL_FAILED;
			}
		}
		else if (nParamId == 221)	// CJJ 20250106 for 235kW PCS OffGrid
		{
			TRACE("Control PARAM_TYPE_SETTING 221 %d %lu\n", nValSize, sizeof(A_BLOB_CTRL_PCS_OFFGRID_CMD));
			if(nValSize != sizeof(A_BLOB_CTRL_PCS_OFFGRID_CMD))
			{
				TRACE("Setting OFFGRID data size ERR====================\n");
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_CTRL_PCS_OFFGRID_CMD* pCtrlCmd = (A_BLOB_CTRL_PCS_OFFGRID_CMD*)pVarVal->blobValue;
			TRACE("nGrp:[%d]\n", pCtrlCmd->nPMIndex);
			TRACE("emCtrlType:[%d]\n", pCtrlCmd->emCtrlType);
			TRACE("fGrpVoltLmt:[%.3f]\n", pCtrlCmd->fGrpVoltLmt);
			TRACE("pCtrlCmd->fGrpFreqLmt:[%.3f]\n", pCtrlCmd->fGrpFreqLmt);
			// goto 235kW OnGrid State Machine
			CTRL_PCS_CMDTYPE emCtrlType = pCtrlCmd->emCtrlType;
			if (emCtrlType == 2 || emCtrlType == CTRL_PCS_STOP)
			{
				pCCU->nCntSendOnOffGridCmd2CCU = 0;
				pCCU->tmLastSendCmd2CCU += 10000;
				pCtrlCmd->nPMIndex = 0;
				if (emCtrlType == 2)
				{
					pCtrlCmd->emCtrlType = CTRL_PCS_OFFGRID_INVERT;
				}
				pCCU->pcsSMInfos[pCtrlCmd->nPMIndex].ctrlPcsOffGridReq = *pCtrlCmd;
				nRet = ERR_CONTROL_OK;
			}
			else
			{
				nRet = ERR_CONTROL_FAILED;
			}

 			// BYTE byData[16]={'\0'};
			// int i = 0;
			// byData[i] = 0x61;									++i;
			// byData[i] = (BYTE)pCtrlCmd->emCtrlType;				++i;
			// byData[i] = (BYTE)pCtrlCmd->nPMIndex;				++i;
			// nSetValue = (int)(pCtrlCmd->fGrpVoltLmt * 100);		
			// byData[i] = (BYTE)(nSetValue >> 8);					++i;
			// byData[i] = (BYTE)(nSetValue);						++i;
			// nSetValue = (int)(pCtrlCmd->fGrpFreqLmt * 1000);
			// byData[i] = (BYTE)(nSetValue >> 8);					++i;
			// byData[i] = (BYTE)(nSetValue);
			
			// DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 500);
			// nRet = ERR_CONTROL_OK;
		}
		else if((nDevId >= 700) && (nDevId <= 705)) // LJH241210 modify DevId >= 700 INPCS need addr=0
		{
			// if (nDevId == 700 && pCCU->lcCfg.emSystemType < SYSTYPE_SCADA_INPCS)
			// {// LJH241210 add DevId:700 INPCS need addr=0 LJH250318 1.ESS也可以支持addr=0 2.return ERR_CONTROL_NOTSUPPORTED会导致错误
			// 	return ERR_CONTROL_NOTSUPPORTED;
			// }
			if(nParamId == 1099)
			{
				nRet = ERR_CONTROL_OK;		
			}
			else if((nParamId >= 1001) && (nParamId <=1010))
			{
				if(On_Handle_PCSGettCmd(pCCU, nDevId, nParamId, pVarVal) == FALSE)
				{
					Mutex_Unlock(pCCU->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
else
			{
				int nRet = On_Handle_PCSSettingCmd(pCCU, nDevId, nParamId, pVarVal, TRUE, TRUE, 0);
				if(nRet == FALSE)//BYTE 3 = 0x00 to set the single rect info param
				{
					Mutex_Unlock(pCCU->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
				PushAllRereadPCSData(pCCU, nDevId, nParamId, pVarVal, TRUE);
			}
		}
		else if(nParamId == 198)  //lock/unlock system
		{	
			TRACEI("receive ccu--setting Lock/unlock system 0--lock 1--unlock [%d]",pVarVal->emValue);
			if(pVarVal->emValue == 0) //lock need check
			{
				if(pCCU->emCCUState != CCU_STATE_LOCKED)
				{
					pCCU->emCCUState = CCU_STATE_LOCKED;
				}
			}
			else
			{
				pCCU->emCCUState = CCU_STATE_IDLE;
				Refresh_SysAvailState(pCCU);
			}

			//now send msg to other modual
			PushAllData(pCCU);

			// LJH250606 deleted:SEMS直接返回成功，锁定不会再往下级分配功率
			// if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION)
			// {
			// 	for (n = 0; n < pEMS->nNodeNum; n++)
			// 	{
			// 		Send_6390UCTL_EnableDisableSystem(n, pCCU->emCCUState);
			// 	}
			// }
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 210) // LJH241111 PcsSCADA Auto Ctrl PCS
		{
			// TRACEI("ccu_sampler_main:Control PARAM_TYPE_SETTING 210 %d %lu", nValSize, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD));
			if(nValSize != sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_GRP_RECT_AUTOCTRL_CMD* pRectCtrl = (A_BLOB_GRP_RECT_AUTOCTRL_CMD*)pVarVal->blobValue;
			GRP_RECT_CTRL_CMDTYPE emCtrl_TypeOld = pRectCtrl->emCtrl_Type;
			// 0:stop 1:电池充电_启动 2:电池充电_调节 3:电池放电_启动 4:电池放电_调节
			GRP_RECT_CTRL_CMDTYPE ctrlTypes[GRP_CTRL_MAX] = {
																GRP_ACDC_STOP,
																GRP_ACDC_RECTIFY_START,
																GRP_ACDC_RECTIFY_ADJ_CURR,
																GRP_ACDC_INVERT_START,
																GRP_ACDC_INVERT_PENDING };
			if (emCtrl_TypeOld <= GRP_ACDC_STOP)
			{
				pRectCtrl->nGrp = 0;
				pCCU->pcsSMInfos[pRectCtrl->nGrp].ctrlRectReq = *pRectCtrl;
				pCCU->pcsSMInfos[pRectCtrl->nGrp].ctrlRectReq.emCtrl_Type = ctrlTypes[pRectCtrl->emCtrl_Type];
				
				pRectCtrl = &pCCU->pcsSMInfos[pRectCtrl->nGrp].ctrlRectReq;
				TRACEI("ccu_sampler_main:Control PARAM_TYPE_SETTING emCtrl_Type:%d nGrp:%d UI:%.1f %.1f emCtrl_TypeOld:%d",
					pRectCtrl->emCtrl_Type, pRectCtrl->nGrp, pRectCtrl->fGrpVoltLmt, pRectCtrl->fGrpCurrLmt, emCtrl_TypeOld);
				nRet = ERR_CONTROL_OK;
			}
			else
			{
				nRet = ERR_CONTROL_FAILED;
			}
		}
		else if(nParamId == 211) // LJH241111 PcsSCADA Manual Ctrl PCS(SCADA)
		{
			// TRACEI("Control Manual Ctrl PCS(SCADA) nValSize=%d sizeof(A_BLOB_MANUALCTRL_RECT_CMD) =%d nParamId:%d",
			// 	nValSize,sizeof(A_BLOB_MANUALCTRL_RECT_CMD), nParamId);
			if(nValSize != sizeof(A_BLOB_MANUALCTRL_RECT_CMD))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_MANUALCTRL_RECT_CMD* pRectCtrl_Manual = (A_BLOB_MANUALCTRL_RECT_CMD*)pVarVal->blobValue;
			pCCU->pcsSMInfos[pRectCtrl_Manual->nGrp].ctrlRectReq_Manual = *pRectCtrl_Manual;
			TRACEI("ccu_tcp_sampler_main:Control PARAM_TYPE_SETTING ManualCtrl emState:%d nGrp:%d UI:%.1f %.1f "
				"nAdjMode:%d fActv_Pwr:%.1f %.1f",
				pRectCtrl_Manual->emState, pRectCtrl_Manual->nGrp, pRectCtrl_Manual->fGrpVolt, pRectCtrl_Manual->fGrpCurr,
				pRectCtrl_Manual->nAdjMode, pRectCtrl_Manual->fActv_Pwr, pRectCtrl_Manual->fReactv_Pwr);
			nRet = ERR_CONTROL_OK;
		}
		// else if(nParamId == 222) // LJH250703 移到SystemSample
		// {
		// 	pCCU->fTotalPowerLimit = (float)pVarVal->fValue;
		// 	TRACE("pCCU->fTotalPowerLimit = [%f]\n", pCCU->fTotalPowerLimit);
		// }
		//John 20220716-Added For PCS 0x23/24 .Test
		/*else if(nParamId == 1001)
		{
			//nSetValue = (int)pVarVal->fValue*1000;
			byPCS_SetData[0] = 0x21;
			byPCS_SetData[1] = 0x42;
		 	//byPCS_SetData[4] = (BYTE)(nSetValue >> 24);
			//byPCS_SetData[5] = (BYTE)(nSetValue >> 16);
			//byPCS_SetData[6] = (BYTE)(nSetValue >> 8);
			//byPCS_SetData[7] = (BYTE)(nSetValue);
			byPCS_SetData[7] = 0xA1;

			CAN_StuffHead(0x0B, 0x24, 0x01,0xF0, pbySendBuf);

			memcpy(pbySendBuf + 4, byPCS_SetData, 8);

			DoTCPSession(pCCU, NL_MSG_PCSSET, pbySendBuf, 12, 1000);
			int n;
			TRACE("\n\t Set 2115 To CCU:");
			for(n = 0; n < 12; n++)
			{
				TRACE("%02X-", pbySendBuf[n]);

			}
			TRACE("\n");

		}
		else if(nParamId == 1002)
		{
			byPCS_SetData[0] = 0x10;
			byPCS_SetData[1] = 0x01;
 

			CAN_StuffHead(0x0B, 0x23, 0x02, 0xF0, pbySendBuf); 

			memcpy(pbySendBuf + 4, byPCS_SetData, 8);

			DoTCPSession(pCCU, NL_MSG_PCSGET, pbySendBuf, 12, 1000);
			int n;
			TRACE("\n\t Get 1001 From CCU:");
			for(n = 0; n < 12; n++)
			{
				TRACE("%02X-", pbySendBuf[n]);

			}
			TRACE("\n");
		}*/
	}
	else //control
	{ 
		// if(nParamId == 1) //lock/unlock charger
		// {
		// 	if(pVarVal->emValue == 0) //lock need check
		// 	{
		// 		if(pCCU->emCCUState != CCU_STATE_LOCKED)
		// 		{
		// 			pCCU->emCCUState = CCU_STATE_LOCKED;
		// 		}
		// 	}
		// 	else
		// 	{
		// 		pCCU->emCCUState = CCU_STATE_IDLE;
		// 		Refresh_SysAvailState(pCCU);
		// 	}

		// 	//now send msg to other modual
		// 	PushAllData(pCCU);

		// 	nRet = ERR_CONTROL_OK;
		// }
		if(nParamId == 15)  
		{
			TRACED("Ctrl: Clear PCS Fault =%d\n",pVarVal->emValue);
			if(pVarVal->emValue == 0) // Not clear
			{
				nRet = ERR_CONTROL_OK;
				Mutex_Unlock(pCCU->hSamplerLock);
				return nRet;
			}
			else if(pVarVal->emValue < 0 || pVarVal->emValue > 11) // Grp0~10->var:1~11
			{
				nRet = ERR_CONTROL_FAILED;
				Mutex_Unlock(pCCU->hSamplerLock);
				return nRet;

			}
			else
			{
				BYTE byData[3]={'\0'};
				byData[0] = 0x55;
				byData[1] = (BYTE)pVarVal->emValue;
				DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
				nRet = ERR_CONTROL_OK; 

			}
 		}
		else if(nParamId == 201)  
		{
			TRACE("Ctrl: Auto/Manual ON/Off-Grid Mode Sett =%d\n",pVarVal->emValue);
 			BYTE byData[3]={'\0'};
			byData[0] = 0x01;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
 		}
		// else if(nParamId == 202) //On/Off Grid Mode 
		// {
		// 	TRACE("Manual Ctrl: ON/Off-Grid Mode Ctrl=%d\n",pVarVal->emValue);
 		// 	BYTE byData[3]={'\0'};
		// 	byData[0] = 0x02;
		// 	byData[1] = (pVarVal->emValue == 0) ? 0xA1 : 0xA2;
		// 	DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		// 	nRet = ERR_CONTROL_OK; 
 		// }
		else if(nParamId == 203)
		{
			TRACE("Manual Ctrl STS: ON/Off%d\n",pVarVal->emValue);
 			BYTE byData[3]={'\0'};
			byData[0] = 0x03;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
 		}
		else if(nParamId == 204)
		{
			TRACE("Manual Ctrl STS: N-Line Relay%d\n",pVarVal->emValue);
 			BYTE byData[3]={'\0'};
			byData[0] = 0x04;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK;
 		}
		else if(nParamId == 205)
		{
			TRACE("Manual Ctrl CabinetAC Input QF1 %d\n",pVarVal->emValue);
 			BYTE byData[3]={'\0'};
			byData[0] = 0x0D;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK;
 		}
		else if(nParamId == 206)
		{
			TRACE("Manual Ctrl ACLoad QF2 %d\n",pVarVal->emValue);
 			BYTE byData[3]={'\0'};
			byData[0] = 0x0E;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
 		}

		else if(nParamId == 208)
		{
			TRACEX("Ctrl: Sys AC Contactor emValue=[%d]\n", pVarVal->emValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x0E;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 209)
		{
			TRACEX("Ctrl: Sys Fan#1 Run ulValue=[%d]\n", pVarVal->ulValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x0F;
			byData[1] = (BYTE)pVarVal->ulValue;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 210)
		{
			TRACEX("Ctrl: Sys Fan#2 Run ulValue=[%d]\n", pVarVal->ulValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x10;
			byData[1] = (BYTE)pVarVal->ulValue;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 211)
		{
			TRACEX("Ctrl: Sys Fan#3 Run ulValue=[%d]\n", pVarVal->ulValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x11;
			byData[1] = (BYTE)pVarVal->ulValue;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 212) //Reboot LCCU
		{
			TRACE("Ctrl: Reboot LCCU \n");
			BYTE byData[5]={'\0'};
			byData[0] = 0x12;
			byData[1] = 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK;
		}		
		else if(nParamId == 213) //Clear LCCU KWH
		{
			TRACE("Ctrl: Clear LCCU KWH\n");
			BYTE byData[5]={'\0'};
			byData[0] = 0x13;
			byData[1] = 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 254)//reread pcs param
		{
			int i;
			int nFreshParamId = pVarVal->ulValue;
			for(i = 0; i< nOnlineSingleNum; i++)
			{

				int nRet = On_Handle_PCSSettingCmd(pCCU, nDevId, nFreshParamId, pVarVal, FALSE, FALSE, i);//刷新回读PCS组下的每一个模块的值 
				if(nRet == FALSE)//BYTE 3 = 0x01 to query the single rect info param
				{
					Mutex_Unlock(pCCU->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			int nRefreshParmId = pVarVal->ulValue;
			PushAllRereadPCSData(pCCU, nDevId, nRefreshParmId, pVarVal, FALSE);
		}
		else if(nParamId == 214)// || nParamId == 217) // LJH241111 PcsSCADA add 217
		{
 			// TRACEI("Ctrl : LCCU Grp Manual Ctrl Rect nValSize=%d sizeof(A_BLOB_MANUALCTRL_RECT_CMD) =%d nParamId:%d",
			// 	nValSize,sizeof(A_BLOB_MANUALCTRL_RECT_CMD), nParamId);
			if(nValSize != sizeof(A_BLOB_MANUALCTRL_RECT_CMD))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

 			A_BLOB_MANUALCTRL_RECT_CMD* pCmd = (A_BLOB_MANUALCTRL_RECT_CMD*)pVarVal->blobValue;
			//Now make the cmd
			BYTE byData[16];

			if(pCCU->emESSDevType == LIQUID_COOLING_836)
			{
				t_ACDCControlData ctrlData;
				ctrlData.u32CtrlVolt = pCmd->fGrpVolt*1000;
				TRACEI("emState = %d\n", pCmd->emState);

				if((pCmd->emState == STAT_MANUAL_START_DISCHG) || (pCmd->emState == STAT_MANUAL_ADJ_DISCHG))
				{
					printf("放电\n");
					ctrlData.eWorkMode = eCDMode_Discharge;
					ctrlData.u32CtrlCurr = pCmd->fGrpCurr*1000;
					if(pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 1)
					{//交流调度
						ctrlData.u32CtrlPower = ABS(pCmd->fReactv_Pwr*1000000);
					}
					else
					{
						ctrlData.u32CtrlPower = 235000000;
					}
					 
				}
				else if((pCmd->emState == STAT_MANUAL_START_CHG) || (pCmd->emState == STAT_MANUAL_ADJ_CHG))
				{
					printf("充电\n");
					ctrlData.eWorkMode = eCDMode_Charge;
					ctrlData.u32CtrlCurr = (UINT)(-(int)(pCmd->fGrpCurr*1000));
					if(pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 1)
					{
						ctrlData.u32CtrlPower = (UINT)(-(int)(ABS(pCmd->fReactv_Pwr*1000000)));
					}
					else
					{
						ctrlData.u32CtrlPower = (UINT)(-(int)(235000000));
					}
				}
				else
				{
					printf("关机\n");
					ctrlData.eWorkMode = eCDMode_Stop;
					ctrlData.u32CtrlVolt = 0;
					ctrlData.u32CtrlCurr = 0;
					ctrlData.u32CtrlPower = 0;
				}

				UINT SGNo = pCmd->nGrp;
				if(SGNo == 0)
				{
					// ctrlData.eWorkMode = 0;
					for(n = 1; n <= 2; n++)
					{
						Set235kWPCS_WrokMode(pCCU, n, ctrlData);
					}
				}
				else
				{
					Set235kWPCS_WrokMode(pCCU, SGNo, ctrlData);
				}

				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_OK;
			}

			if(ABS(pCmd->fActv_Pwr) > pCCU->fTotalPowerLimit)
			{
				if (pCmd->fActv_Pwr < 0)
				{
					pCmd->fActv_Pwr = -pCCU->fTotalPowerLimit;
				}
				else
				{
					pCmd->fActv_Pwr = pCCU->fTotalPowerLimit;
				}
			}
			Set_IMMU_DO(IOC_SET_DO6, 1);
			Sleep(50);
			Set_IMMU_DO(IOC_SET_DO6, 0);

			int nSetGrpVolt = (int)(pCmd->fGrpVolt*10);
			int nSetGrpCurr = (int)(pCmd->fGrpCurr*10);
			int	nSetActv_Pwr = (int)(pCmd->fActv_Pwr*10);
			int	nSetReactv_Pwr = (int)(pCmd->fReactv_Pwr*10);
			TRACEI("ccu_tcp_sampler_main:Control DoTCPSession nParamId:%d emState:%d nGrp:%d nSetGrpVolt:%d nSetGrpCurr:%d nSetActv_Pwr:%d nSetReactv_Pwr:%d",
				nParamId, pCmd->emState, pCmd->nGrp, nSetGrpVolt, nSetGrpCurr, nSetActv_Pwr, nSetReactv_Pwr);
			if (nParamId == 214)
			{
				UINT nStopSOC 	= pCmd->nStopSOC;
				if((pCmd->nGrp > 0) && (pCmd->nGrp < MAX_SINGLE_STBATT_NUM))
				{	
					int nDevID = 501 + pCmd->nGrp - 1; //0-means Stop,1-2-3-4:ACDC Grp1-2-3-4.
					VAR_VALUE varV_SOC;
					varV_SOC.ulValue = nStopSOC;
					Push_VarValue(SETTYPE_RAW_ST, pCCU->hSampler, nDevID, 507, varV_SOC);

					TRACE("nSetGrpVolt=[%d] nSetGrpCurr=[%d] nStopSOC=[%d] nDevID=[%d]\n", \
						nSetGrpVolt, nSetGrpCurr, nStopSOC, nDevID);
				}
			}
			
			if(pCmd->nAdjMode == 1) // 1-AC // 交流调度按照充放指令下发，不再依赖正负号
			{
				if(pCmd->emState == STAT_MANUAL_START_CHG
					|| pCmd->emState == STAT_MANUAL_ADJ_CHG)
				{
					nSetActv_Pwr = -ABS(nSetActv_Pwr);
					nSetGrpCurr = 0; // 交流调度时，下发电流需为0，避免 STAT_MANUAL_ADJ_CHG 时误操作
				}
				else if(pCmd->emState == STAT_MANUAL_START_DISCHG
					|| pCmd->emState == STAT_MANUAL_ADJ_DISCHG)
				{
					nSetActv_Pwr = ABS(nSetActv_Pwr);
					nSetGrpCurr = 0; // 交流调度时，下发电流需为0 
				}
			}

			byData[0] = 0x14;
			byData[1] = (BYTE)pCmd->emState;
			byData[2] = (BYTE)pCmd->nGrp;
			byData[3] = (BYTE)(nSetGrpVolt >> 8);
			byData[4] = (BYTE)(nSetGrpVolt);
		 	byData[5] = (BYTE)(nSetGrpCurr >> 8);
			byData[6] = (BYTE)nSetGrpCurr;

			byData[7] = (BYTE)pCmd->nAdjMode;

			byData[8] = (BYTE)(nSetActv_Pwr >> 8);
			byData[9] = (BYTE)nSetActv_Pwr;
			byData[10] =  (BYTE)(nSetReactv_Pwr >> 8);
			byData[11] = (BYTE)nSetReactv_Pwr;
			int nSendLen = (pCmd->nAdjMode == 0) ? 7 : 12; //In Order compatible For -D old Version
			// TRACED("Control:emState:%d nGrp:%d", pCmd->emState, pCmd->nGrp);
			if(pCmd->nGrp == 9)
			{
				nRet = ERR_CONTROL_OK;
				int i;
				int nErrFlag = 0;
				for(i = 0; i < MAX_MPPT_GROUP_NUM; i++)
				{
					byData[2] = (BYTE) (5 + i);
					TRACE("nGrp==9 nSendLen=[%d] Data[0]=%02X Data[1]=%02X Data[2]=%02X Data[3]=%02X Data[4]=%02X Data[5]=%02X Data[6]=%02X  Data[7]=%02X Data[8]=%02X Data[9]=%02X Data[10]=%02X Data[11]=%02X\n",\
						nSendLen, byData[0],byData[1],byData[2],byData[3],byData[4],byData[5],byData[6],byData[7],byData[8],byData[9],byData[10],byData[11]);
					nErrFlag +=	DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 1000) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
				}
				if(nErrFlag > 0){nRet = ERR_CONTROL_FAILED;}
			}
			else
			{
				varV.blobValue = pCmd;
				Push_BlobValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 
					200, 50, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);

				TRACE("nGrp!=9 nSendLen=[%d] Data[0]=%02X Data[1]=%02X Data[2]=%02X Data[3]=%02X Data[4]=%02X Data[5]=%02X Data[6]=%02X  Data[7]=%02X Data[8]=%02X Data[9]=%02X Data[10]=%02X Data[11]=%02X\n",\
						nSendLen, byData[0],byData[1],byData[2],byData[3],byData[4],byData[5],byData[6],byData[7],byData[8],byData[9],byData[10],byData[11]);
				nRet =	DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 1000) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;;		
			}
		} 
		else if(nParamId == 215)//Voice and Visual Lighting
		{
			TRACEX("Ctrl: Voice and Visual Lighting emValue=[%d]\n", pVarVal->emValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x0B;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 216)//Power Cabinet Lighting
		{
			TRACEX("Ctrl: Power Cabinet Lighting emValue=[%d]\n", pVarVal->emValue);
			BYTE byData[5]={'\0'};
			byData[0] = 0x0C;
			byData[1] = (pVarVal->emValue == 0) ? 0xA0 : 0xA1;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500);
			nRet = ERR_CONTROL_OK;
		}
		else if(nParamId == 218)
		{
			TRACE("Ctrl : AC Grid Input MCB emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x18;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
 		}	
		else if(nParamId == 219)
		{
			TRACE("Ctrl : AC Load Output MCB emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x19;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
 		}
		else if(nParamId == 220)
		{
			TRACE("Ctrl : Battery Grp1 Contactor emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x20;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
			pCCU->emGrpContactor1_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
 		}
		else if(nParamId == 221)
		{
			TRACE("Ctrl : Battery Grp2 Contactor emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x21;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
			pCCU->emGrpContactor2_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
 		}	
		else if(nParamId == 222)
		{
			TRACE("Ctrl : Battery Grp3 Contactor emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x22;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
			pCCU->emGrpContactor3_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
 		}	
		else if(nParamId == 223)
		{
			TRACE("Ctrl : Battery Grp4 Contactor emValue =%d\n",pVarVal->emValue);
 			BYTE byData[5]={'\0'};
			byData[0] = 0x23;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 1000); 
			pCCU->emGrpContactor4_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
 		}
		else if(nParamId == 224)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x24;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			pCCU->emPVContactor1_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
		}
  		else if(nParamId == 225)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x25;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			pCCU->emPVContactor2_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 226)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x26;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			pCCU->emPVContactor3_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 227)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x27;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			pCCU->emPVContactor4_CtrlVal = (pVarVal->emValue == 1) ? 0 : 1;
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 228)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x28;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 229)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x29;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 230)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x2A;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 231)
		{
			BYTE byData[5]={'\0'};
			byData[0] = 0x2B;
			byData[1] = (pVarVal->emValue == 1) ? 0xA1 : 0xA0;
			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 232)
		{
			TRACE("Ctrl : Rect Group For AC/DC DC/DC nParamId == 232 \n");
			if(nValSize != sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			BYTE byData[16]={'\0'};
			memset(byData, 0, 16);
			int nSendLen = 0;
			
			A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCmd = (A_BLOB_GRP_RECT_AUTOCTRL_CMD*)pVarVal->blobValue;

			if(pCCU->emESSDevType == LIQUID_COOLING_836)
			{
				// printf("fGrpVoltLmt = %f, fGrpCurrLmt = %f, power = %f\n",\
				// 	 pCmd->fGrpVoltLmt, pCmd->fGrpCurrLmt, pCmd->fCtrlPower);

				t_ACDCControlData ctrlData;
				
				// printf("emCtrl_Type = %d\n", pCmd->emCtrl_Type);

				ctrlData.u32CtrlVolt = pCmd->fGrpVoltLmt*1000;
				if((pCmd->emCtrl_Type == GRP_ACDC_INVERT_START) || (pCmd->emCtrl_Type == GRP_ACDC_INVERT_PENDING))
				{
					pCCU->strLSYSInfo.blobGrpRectData.emCtrlStatus[pCmd->nGrp] = CTRL_STATUS_INVERTER_REVERSE;

					// printf("放电\n");
					ctrlData.eWorkMode = eCDMode_Discharge;
					ctrlData.u32CtrlCurr = pCmd->fGrpCurrLmt*1000;
					if(pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 1)
					{
						ctrlData.u32CtrlPower = pCmd->fCtrlPower * 1000000;
					}
					else
					{
						ctrlData.u32CtrlPower = 235000000;
					}
					 
				}
				else if((pCmd->emCtrl_Type == GRP_ACDC_RECTIFY_START) || (pCmd->emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR))
				{
					pCCU->strLSYSInfo.blobGrpRectData.emCtrlStatus[pCmd->nGrp] = CTRL_STATUS_RECTIFY_POSITIVE;

					// printf("充电\n");
					ctrlData.eWorkMode = eCDMode_Charge;
					ctrlData.u32CtrlCurr = (UINT)(-(int)(pCmd->fGrpCurrLmt*1000));				
					if(pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 1)
					{
						ctrlData.u32CtrlPower = (UINT)(-(int)(ABS(pCmd->fCtrlPower*1000000)));
					}
					else
					{
						ctrlData.u32CtrlPower = (UINT)(-(int)(235000000));
					}
				}
				else
				{
					pCCU->strLSYSInfo.blobGrpRectData.emCtrlStatus[pCmd->nGrp] = CTRL_STATUS_OFF;

					// printf("关机\n");
					ctrlData.eWorkMode = eCDMode_Stop;
					ctrlData.u32CtrlPower = 0;
				}

				pCCU->strLSYSInfo.blobGrpRectData.fCtrlVolt[pCmd->nGrp] = ctrlData.u32CtrlVolt*0.001f;
				pCCU->strLSYSInfo.blobGrpRectData.fCtrlCurr[pCmd->nGrp] = (float)(ABS((int)ctrlData.u32CtrlCurr)) * 0.001f;
				pCCU->strLSYSInfo.blobGrpRectData.fCtrlPower[pCmd->nGrp] = (float)(ABS((int)ctrlData.u32CtrlPower)) * 0.000001f;

				// printf("idx = %d, u32CtrlVolt = %d, u32CtrlCurr = %d, u32CtrlPower = %d\n",\
				// pCmd->nGrp, ctrlData.u32CtrlVolt, ctrlData.u32CtrlCurr, ctrlData.u32CtrlPower);

				// printf("pCmd->nGrp = %d\n", pCmd->nGrp);
				
				int SGNo = pCmd->nGrp + 1;
				Set235kWPCS_WrokMode(pCCU, SGNo, ctrlData);

				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_OK;

			}

			
			if ((pCmd->emCtrl_Type == GRP_ACDC_INVERT_START) && (ElapseS(pCCU->tSentCCUDisChargeCmdTime) < 5))
			{
				TRACEE("control 232 ElapseS < 5 return !!!!\n");
				//return ERR_CONTROL_OK;
			}
			else
			{
				//get battGrp info
				A_BLOB_SINGLE_STBATT_RUNINFO  blobSingleBattData;
				VAR_VALUE varV;
				varV.blobValue = (BYTE *)&blobSingleBattData;
				// LJH241111 PcsSCADA
				if ( HasEMSFeature(pCCU->lcCfg.emSystemType) )
				{
					pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
						501 + pCmd->nGrp, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
				}
				
				TRACE("-------Received cmd A_BLOB_GRP_RECT_AUTOCTRL_CMD[%d] nGrpType=[%d] nValSize=[%d], fReq[%.1f, %.1f], fLmt[%.1f, %.1f]-----\n", 
					pCmd->nGrp, pCmd->emCtrl_Type, nValSize, pCmd->fGrpVoltLmt, pCmd->fGrpCurrLmt, blobSingleBattData.fAllow_LmtChargeVolt, blobSingleBattData.fAllow_MaxChargeCurr);

				memcpy(&pCCU->blobGrpRectCtrl, pCmd, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD));
				
				TRACEX("emCharge_Statu=%d,nVoltReq=%.1f, nCurrReq=%.1f\n",\
					pCCU->blobGrpRectCtrl.emCtrl_Type,\
					pCCU->blobGrpRectCtrl.fGrpVoltLmt,\
					pCCU->blobGrpRectCtrl.fGrpCurrLmt);
				
				if(pCCU->blobGrpRectCtrl.emCtrl_Type <= GRP_ACDC_STOP)
				{
					byData[0] = 0x31 + pCmd->nGrp;
					nSendLen = 6;
					if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_STOP)
					{
						byData[1] = 0x00;
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_START)
					{
						byData[1] = 0x01;
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR)
					{
						byData[1] = 0x02;
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_INVERT_START)
					{
						byData[1] = 0x03;
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_INVERT_PENDING)
					{
						byData[1] = 0x04;
					}
					if (HasEMSFeature(pCCU->lcCfg.emSystemType))
					{
						TRACED("ccu_sampler_main:Control HasEMSFeature fGrpVoltLmt:%.1f %.1f fGrpCurrLmt:%.1f %.1f %.1f",
							pCCU->blobGrpRectCtrl.fGrpVoltLmt, blobSingleBattData.fAllow_LmtChargeVolt,
							pCCU->blobGrpRectCtrl.fGrpCurrLmt, blobSingleBattData.fAllow_MaxChargeCurr, blobSingleBattData.fAllow_MaxDisChargeCurr);

						pCCU->blobGrpRectCtrl.fGrpVoltLmt = MIN(pCCU->blobGrpRectCtrl.fGrpVoltLmt,
							blobSingleBattData.fAllow_LmtChargeVolt);

						if((pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_START) ||
						(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR))
						{
							pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
								blobSingleBattData.fAllow_MaxChargeCurr);
						}
						else
						{
							pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
								blobSingleBattData.fAllow_MaxDisChargeCurr);
						}
					}
					TRACE("emCtrl_Type=%d ,nVoltReq=%.1f, nCurrReq=%.1f\n",\
						pCCU->blobGrpRectCtrl.emCtrl_Type,\
						pCCU->blobGrpRectCtrl.fGrpVoltLmt,\
						pCCU->blobGrpRectCtrl.fGrpCurrLmt);
					
					byData[2] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10) >> 8);
					byData[3] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10);
					byData[4] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10) >> 8);
					byData[5] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10);
				}
				else if(pCCU->blobGrpRectCtrl.emCtrl_Type <= GRP_DCDC_STOP)
				{
					byData[0] = 0x51 + pCmd->nGrp;
					nSendLen = 4;
					if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_STOP)
					{
						byData[1] = 0x00;
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_START)
					{
						byData[1] = 0x01; 
					}
					else if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_PENDING)
					{
						byData[1] = 0x02; 
					}

					if(pCCU->blobGrpRectCtrl.emCtrl_Type == GRP_DCDC_RECTIFY_START && HasEMSFeature(pCCU->lcCfg.emSystemType))
					{
						pCCU->blobGrpRectCtrl.fGrpCurrLmt = MIN(pCCU->blobGrpRectCtrl.fGrpCurrLmt,
							blobSingleBattData.fAllow_MaxDisChargeCurr);
					}
					byData[2] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10) >> 8);  
					byData[3] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpVoltLmt*10);  
					
					byData[4] = (BYTE)((UINT)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10) >> 8);
					byData[5] = (BYTE)(pCCU->blobGrpRectCtrl.fGrpCurrLmt*10);
				}
				TRACEI("ccu_sampler_main:Control emCtrl_Type:%d(0:rect 2:invt 4:stop) nGrp:%d UI:%.1f %.1f byData<%s>",
					pCCU->blobGrpRectCtrl.emCtrl_Type, pCCU->blobGrpRectCtrl.nGrp,
					pCCU->blobGrpRectCtrl.fGrpVoltLmt,
					pCCU->blobGrpRectCtrl.fGrpCurrLmt, printableBytes(byData, nSendLen));
				//2020-01-04 Do Not Wait Adjust Current Cmd
				nRet = DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 0) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;			
				nRet = ERR_CONTROL_OK;
			}
		}
		else if(nParamId == 233)
		{
			if(pVarVal->emValue >= PM2CCU_TRIG_MAX)
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			OnHandle_CtrlCmd4CANMsgTrig(pCCU, pVarVal->emValue);
			nRet = ERR_CONTROL_OK; 
		}
		else if(nParamId == 151) //enter Update rect firmware process
		{
			TRACE("---151 A_BLOB_UPDT_INFO_4CCU---\n");
			if(nValSize != sizeof(A_BLOB_UPDT_INFO_4CCU))
			{
				TRACE("----Size mismatch for CCU-updt-ctrl[%d,%d]----\n", nValSize, sizeof(A_BLOB_UPDT_INFO_4CCU));
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			
			A_BLOB_UPDT_INFO_4CCU *pUpdtRectCmd = (A_BLOB_UPDT_INFO_4CCU*)pVarVal->blobValue;
			memcpy(&pCCU->blobUpdCmd, pUpdtRectCmd, sizeof(A_BLOB_UPDT_INFO_4CCU));

			if(pCCU->blobUpdCmd.bIs4Web)
			{
				BOOL bRet = Prepare_CCUBin_Updte(pCCU, 2000);
				if(bRet == FALSE)
				{
					TRACE("----Prepare_CCUBin_Updte Failed!!!---\n");
					Mutex_Unlock(pCCU->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}

			//now notice sampler to goto the update process
			pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.tm_StartUpgrade = _NOW_;

			pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus = (pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus >= UPGRADE_READBINFILE) ? 
				pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus : UPGRADE_READBINFILE;   

#ifdef IMMU3
			pCCU->tagCCUUpgradeParam.strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SLIENCE; 
#endif

			TRACEX("tagMstUpgradeParam.emUpgradeStatus=%d\n", pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus);
			
		}
		else if(nParamId == 250) //Trigger On/Off Grid State(Manual)
		{
			pCCU->tagOn2OffParam.emHMITrig = (STAT_HMITRIG_ON2OFFGRID)pVarVal->emValue;
			pCCU->tagOn2OffParam.tmLastHMITrig = _NOW_;
		}
		else if(nParamId == 251)
		{
			if(nValSize != sizeof(A_BLOB_UPDATEFIRMWARE_CMD))
			{
				Mutex_Unlock(pCCU->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			A_BLOB_UPDATEFIRMWARE_CMD *cmd = (A_BLOB_UPDATEFIRMWARE_CMD*)pVarVal->blobValue;

			size_t url_len = strlen(cmd->szURL);
			if((url_len > 255) || (cmd->nSemsIdx > MAX_SEMS_CLIENTS_NUM))
			{
				return ERR_CONTROL_FAILED;
			}
			S63Uctl_EmsUpgradeCmd Upgradecmd;
			memcpy(Upgradecmd.szURL, cmd->szURL, url_len);
			Upgradecmd.szURL[url_len] = '\0';
			Upgradecmd.dtRetrieveDate = cmd->dtRetrieveDate;
			int i;

			if(cmd->nSemsIdx == -1) //升级所有从控
			{
				for(i = 0; i <  pCCU->emsMgmtData.nNodeNum; ++i)
				{
					Send_6391_SlaveUpgradeCmd(i + 1, Upgradecmd);
				}
			}
			else //升级指定的从控设备
			{
				Send_6391_SlaveUpgradeCmd(cmd->nSemsIdx, Upgradecmd);
			}
		}
		else if (nParamId == 301) // 200, 301,
		{
			pEMS->bStandby_AEMS = (BOOL)pVarVal->emValue;
			TRACED("ccu_sampler_main:Control 200, 301, bStandby_AEMS:%d", pEMS->bStandby_AEMS);
		}
		else if(nParamId == 252)
		{
			pCCU->nPCSNum = 0;
		}
		else if(nParamId == 253)
		{
			int i;
			for(i = 0; i <  pCCU->emsMgmtData.nNodeNum; ++i)
			{
				Send_65Query_PCSInfo(i);
			}
		}
	}

	Mutex_Unlock(pCCU->hSamplerLock);
	return nRet;	
}
 
BOOL JudgeisNeedSkipSinglePCSAlarm(ROUGH_CCU_DATA* pCCU)
 {	
	if(ABS(_NOW_ - pCCU->tmSysBoot) < 90)  
	{
		//TRACE("_NOW_=[%ld] , pCCU->tmSysBoot=[%ld]\n", _NOW_ , pCCU->tmSysBoot);
		return TRUE;
	}

	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.MainAC_Contactor == TRUE)
	{
		//TRACE("MainAC_Contactor=[%d]\n", pCCU->strLSYSInfo.sysAlarmMap1.Alarm.MainAC_Contactor);
		return TRUE;
	}

	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACGridInputMCBQF1 == TRUE)
	{
		//TRACE("ACGridInputMCBQF1=[%d]\n", pCCU->strLSYSInfo.sysAlarmMap1.Alarm.ACGridInputMCBQF1);
		return TRUE;
	}

	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.EPO == TRUE)
	{
		//TRACE("EPO=[%d]\n", pCCU->strLSYSInfo.sysAlarmMap1.Alarm.EPO);	
		return TRUE;
	}

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed == 1) //0-Disable, 1-Enable
	{
		if((pCCU->strLSYSInfo.sysAlarmMap1.Alarm.STS1_CommFail == 0) &&  	//0-CommOk, 1-CommFail.
			(pCCU->strLSYSInfo.mapSTSCode.Status.Power_OnOff == 0)) 		//0-Power_Off, 1-Power-On.
		{
			//TRACE("STS1_CommFail=[%d] Power_OnOff=[%d]\n",\
			//	pCCU->strLSYSInfo.sysAlarmMap1.Alarm.STS1_CommFail,\
			//	pCCU->strLSYSInfo.mapSTSCode.Status.Power_OnOff);

			return TRUE;
		}
	}

	return FALSE;	 
}

BOOL JudgeIsoPendingStatus(ROUGH_CCU_DATA* pCCU, TYPE_SIGNLE_RECT emGrouId)
{
	VAR_VALUE varV;
	time_t tmNow = _NOW_;
	static time_t tmLastIsoPending[4];

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + emGrouId, 547, &varV, 4);
	if(varV.emValue == 1)
	{
		tmLastIsoPending[emGrouId] = tmNow;
		return TRUE;
	}
	
	if(ABS(tmNow - tmLastIsoPending[emGrouId]) < 60) //Wait ISO Finished At Least 60S. Single PM Sample Too Slowly.
	{
		return TRUE;
	}

	return FALSE;
}

void Save_EmsMgmtPlan(A_BLOB_EMS_CONFIG_PLAN *pEmsMgmtPlan)
{
	char log[32768];
	char tmp[256];
	char strTime[32];
	memset(log, 0, sizeof(log));
	memset(tmp, 0, sizeof(tmp));
	memset(strTime, 0, sizeof(strTime));

	static const char* workMode[] = {
		[BATT_WORKMODE_FIRST_DISCHRG] = "FirstDischg",
		[BATT_WORKMODE_FIRST_CHRGING] = "FirstChg",
		[BATT_WORKMODE_DISCHRG_ONLY] = "OnlyDischg",
		[BATT_WORKMODE_CHRGING_ONLY] = "OnlyChg",
		[BATT_WORKMODE_STANDBY] = "Standby",
		[BATT_WORKMODE_IDLE] = "Idle"
	};

	snprintf(tmp, 256, "Set mgmtPlan SectorNum:%d\n", pEmsMgmtPlan->nTotal_EmsLmt_Sectors);
	strcat(log, tmp);
	int i=0;
	for(i=0; i<pEmsMgmtPlan->nTotal_EmsLmt_Sectors; ++i)
	{
		TimeToString(pEmsMgmtPlan->emsLmtSectors[i].tmStart, TIME_CHN_FMT, strTime, sizeof(strTime));

		snprintf(tmp, 256, "%d:start-%s mode-%s\tstopSoc-%d\tgrpLmt-%d\ttotalLmt-%d\trectLmt-%d\n", 
						i, 
						strTime, 
						workMode[pEmsMgmtPlan->emsLmtSectors[i].emBatt_WorkMode], 
						pEmsMgmtPlan->emsLmtSectors[i].nStopSOC, 
						pEmsMgmtPlan->emsLmtSectors[i].nBattGrpPwrLmt, 
						pEmsMgmtPlan->emsLmtSectors[i].nActvPwrLmt, 
						pEmsMgmtPlan->emsLmtSectors[i].nReActvPwrLmt);
		strcat(log, tmp);
	}
		
	LOGFD("%s", log);
}


 
/*********************************************************************************
*  
*  FUNCTION NAME : Event_Handler
*  CREATOR       :  
*  CREATED DATE  : 2020-9-19 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	ROUGH_CCU_DATA *pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	ERRCDDE_DPR err;
	int n;

	TRACE("-------CCU_tcp_Sampler got EVENT[%d] emMsgType=[%d]--------\n", emEvType, emMsgType);
	 
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
	 	if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 201))
		{
			TRACE("Ems config Plan Changed, Do Update!\n");
			//EMS Config Plan Changed
			varV.blobValue = (BYTE *)&pCCU->blobEmsCfgPlan;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}

			Save_EmsMgmtPlan(&pCCU->blobEmsCfgPlan);

			TRACE("Ems config Plan Changed success!!!\n");
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 7))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 7, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->emAuxMeterPos =(POS_AUXMETER)varV.ulValue;
			// LJH241007 AntiPhase
			pCCU->emsMgmtData.isAtRflxMtrHV = (pCCU->emAuxMeterPos == AUXMETER_POS_BRANCH_TRANSFER);
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 20))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 20, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->emPmaxDemandMode = varV.emValue;
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 23))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 23, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->fBAT_Max_T_PwrLmtRate_Set = MIN(1.0, varV.fValue);
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			TRACE("A_BLOB_ES_BACKEND_CFG blobBackendCfg Changed, Do Update!\n");
 			varV.blobValue = (BYTE *)&pCCU->blobBackendCfg;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("A_BLOB_ES_BACKEND_CFG Changed success!!!\n");
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 12))
		{
			TRACE("A_BLOB_CHARGE_FEESET blobFeeRate  Changed, Do Update!\n");
 			varV.blobValue = (BYTE *)&pCCU->blobFeeRate;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 12, &varV, sizeof(A_BLOB_CHARGE_FEESET));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("A_BLOB_CHARGE_FEESET Changed success!!!\n");
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 40 || pCfgChange->parmPTR.nSubId == 41))
		{
			// LJH241007 AntiPhase 1, 40, 1, 41,
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, pCfgChange->parmPTR.nSubId, &varV, 4);
			if (pCfgChange->parmPTR.nSubId == 40)
			{
				pCCU->needAntiReflux_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			}
			else if (pCfgChange->parmPTR.nSubId == 41)
			{
				pCCU->needAntiOverLoad_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			}
			TRACED("ccu_sampler_main::Event_Handler needAnti***_Phase Changed nId:%d nSubId:%d "
				"AntiReflux:%d %d err:%d",
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId,
				pCCU->needAntiReflux_Phase, pCCU->needAntiOverLoad_Phase, err);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("A_BLOB_CHARGE_FEESET Changed success!!!\n");
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 44))
		{
			// LJH250316 1, 44,
			char szFirstIP_EMSClient[MAX_LEN_SZIP] = "192.168.1.101";
			varV.blobValue = (BYTE *)szFirstIP_EMSClient;
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, sizeof(szFirstIP_EMSClient));
			s_SemsInfo.ulIP_FirstClient = ntohl(inet_addr(szFirstIP_EMSClient));
			TRACED("ccu_sampler_main::Event_Handler szFirstIP_EMSClient:%s %04X",
				szFirstIP_EMSClient, s_SemsInfo.ulIP_FirstClient);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 99))
		{
			// XSY251224 1, 99,
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 99, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->emsMgmtData.emSortMethod = varV.emValue;			
		}
		else if((pCfgChange->parmPTR.nId == 500) && (pCfgChange->parmPTR.nSubId == 509))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 500, 509, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->emDisChgByStopSOC = varV.emValue;//0-No, 1-Yes
			TRACE("Event_Handler:emDisChgByStopSOC=[%d]\n", pCCU->emDisChgByStopSOC);
		}
		else if (pCfgChange->parmPTR.nId == 1 &&
			(pCfgChange->parmPTR.nSubId == 84 || pCfgChange->parmPTR.nSubId == 85 || pCfgChange->parmPTR.nSubId == 102))
		{
			// LJH241202 SEMS
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 84: // msPeriodLSYS 1, 84,
				pCCU->msPeriodLSYS = ((err==ERR_DPR_OK) ? varV.ulValue : 1000);
				if (pCCU->lcCfg.emSystemType == SYSTYPE_EMS_STATION && pCCU->lcCfg.lcType == LC_TYPE_1st)
				{
					for (n = 0; n < pEMS->nNodeNum; n++)
					{
						TRACED("ccu_sampler_main::Event_Handler Send_6204CFGS_LSYSPeriod n:%d msPeriodLSYS:%u", n, pCCU->msPeriodLSYS);
						Send_6204CFGS_LSYSPeriod(n);
					}
				}
				break;
			case 85: // bStandby_Set 1, 85,
    			pEMS->bStandby_Set = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 102: // LJH250703 移到SystemSample fTotalPowerLimit 1, 102,
				pCCU->fTotalPowerLimit = ((err==ERR_DPR_OK) ? varV.fValue : MAX_PWR_LMT);
				break;
			default:
				break;
			}
			TRACED("ccu_sampler_main::Event_Handler nId=1 nSubId:%d err:%d msPeriodLSYS:%u fTotalPowerLimit:%.1f bStandby_Set:%d",
				pCfgChange->parmPTR.nSubId, err, pCCU->msPeriodLSYS, pCCU->fTotalPowerLimit, pEMS->bStandby_Set);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 1001) &&
			((pCfgChange->parmPTR.nSubId >= 3301 && pCfgChange->parmPTR.nSubId <= 3309) ||
			(pCfgChange->parmPTR.nSubId >= 3311 && pCfgChange->parmPTR.nSubId <= 3313) ||
			(pCfgChange->parmPTR.nSubId >= 3317 && pCfgChange->parmPTR.nSubId <= 3320)
			))
		{
			// LJH250205 REMS
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 3301: // Automatic charging Enable 1001, 3301,
				pEMS->bAutoChrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3302: // Automatic Discharging Enable 1001, 3302, 
				pEMS->bAutoDischrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3303: // EMS Control Mode 1001, 3303,
				pEMS->enableREMSCtrl = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3304: // Anti-Reflux(REMS) 1001, 3304,
				pEMS->enableAntiRflx_REMS = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3305: // Anti-OverLoad(REMS) 1001, 3305, 
				pEMS->enableAntiOvld_REMS = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3306: // uChargeStopSoc(REMS) 1001, 3306, // LJH251106 同步主从参数
			{
				pEMS->nStopSOC_ChrgSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 100);
				for (n = 0; n < pEMS->nNodeNum; n++)
				{
					pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_ChrgSet = 1;
				}
			}
				break;
			case 3307: // Automatic charging Enable 1001, 3307, 
				pEMS->fTargetPwr_4Chrg_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
				// LJH250703 移到AutoCharging_IfReflux
				// pEMS->fTargetPwr_4Chrg_SEMS = MIN(pEMS->fTargetPwr_4Chrg_SEMS, pCCU->blobBackendCfg.fSysACInput_PwrLmt);
				break;
			case 3308: // Automatic Discharging Enable 1001, 3308,
				pEMS->fTargetPwr_4Dischrg_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
				break;
			case 3309: // uDischargeStopSoc(REMS) 1001, 3309, 
			{
				pEMS->nStopSOC_DiscSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 0);
				for (n = 0; n < pEMS->nNodeNum; n++)
				{
					pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_DiscSet = 1;
				}
			}
				break;
			case 3311: // Automatic charging Target Power Offset 1001, 3311,
				pEMS->fTargetPwr_4ChrgOffset_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
				break;
			case 3312: // Automatic Discharging Target Power Offset 1001, 3312,
				pEMS->fTargetPwr_4DischrgOffset_SEMS = ((err==ERR_DPR_OK) ? varV.fValue : 0);
				break;
			case 3313: // 只由一级主控上报cmd 1001, 3313,
			{
				pCCU->lcCfg.bSendCmdByLC1stOnly = (BOOL)(err==ERR_DPR_OK ? varV.emValue : FALSE);
				for (n = 0; n < pEMS->nNodeNum; n++)
				{
					pCCU->semsSMInfos[n].hasParamSetCFGS.bSendCmdByLC1stOnlySet = 1;
				}
			}
				break;
			case 3317: // LJH250930 加快自动充放响应 1001, 3317,
				pEMS->fLtPwr2Standby_4Chrg = ((err==ERR_DPR_OK) ? varV.fValue : -99999);
				break;
			case 3318: // Automatic Discharging Target Power Offset 1001, 3318,
				pEMS->uDelayStandby_4Chrg = ((err==ERR_DPR_OK) ? varV.ulValue : 0);
				break;
			case 3319: // 1001, 3319,
				pEMS->fGtPwr2Standby_4Disc = ((err==ERR_DPR_OK) ? varV.fValue : 99999);
				break;
			case 3320: // Automatic Discharging Target Power Offset 1001, 3320,
				pEMS->uDelayStandby_4Disc = ((err==ERR_DPR_OK) ? varV.ulValue : 0);
				break;

			default:
				break;
			}
			TRACED("ccu_sampler_main::Event_Handler nId=1001, %d %lu err:%d "
				"bAutoChrgEnable:%d %d fTargetPwr_4Chrg_SEMS:%.1f %.1f Offset:%.1f %.1f "
				"enableREMSCtrl:%d enableAntiRflx_REMS:%d %d nStopSOC_ChrgSet_SEMS:%d-%d",
				pCfgChange->parmPTR.nSubId, varV.ulValue, err,
				pEMS->bAutoChrgEnable, pEMS->bAutoDischrgEnable,
				pEMS->fTargetPwr_4Chrg_SEMS, pEMS->fTargetPwr_4Dischrg_SEMS, pEMS->fTargetPwr_4ChrgOffset_SEMS, pEMS->fTargetPwr_4DischrgOffset_SEMS,
				pEMS->enableREMSCtrl, pEMS->enableAntiRflx_REMS, pEMS->enableAntiOvld_REMS, pEMS->nStopSOC_ChrgSet_SEMS, pEMS->nStopSOC_DiscSet_SEMS);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if ((pCfgChange->parmPTR.nId == 407 || pCfgChange->parmPTR.nId == 408) && pCfgChange->parmPTR.nSubId == 401)
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nId)
			{
			case 407: // LJH241202 SEMS 407, 401, 
				pEMS->bCabMeter_SEMS = ((err==ERR_DPR_OK) ? (varV.emValue>0?TRUE:FALSE) : FALSE);
				break;
			case 408: // LJH250401 topology#6#7 408, 401, 
				pEMS->bGMeter_SEMS = ((err==ERR_DPR_OK) ? (varV.emValue>0?TRUE:FALSE) : FALSE);
				break;
			default:
				break;
			}
			// LJH250401 topology#6#7
			TRACED("ccu_sampler_main::Event_Handler SEMS bCabMeter_SEMS:%d bGMeter_SEMS:%d", pEMS->bCabMeter_SEMS, pEMS->bGMeter_SEMS);
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 106)) 	//nDisChrgFilterPoint
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->nDisChrgFilterPoint = varV.ulValue;
			TRACED("nDisChrgFilterPoint Changed to [%d]\n",pCCU->nDisChrgFilterPoint);

			LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_INFO,
					"nDisChrgFilterPoint Changed --> [%d] !",pCCU->nDisChrgFilterPoint);
		}
		else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId >= 300 && pCfgChange->parmPTR.nSubId <= 318)) 
		{
			PID_Ctx *pPidCtx = &pEMS->stPidCtx;
			PID_Params *pPidChgParams = &pPidCtx->stPIDChgParams;
			PID_Params *pPidDischgParams = &pPidCtx->stPIDDischgParams;

			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 300: // PID Control Enable
				pEMS->emPIDEnableState = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
				err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
						pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId + 1, &varV, 4);
				pEMS->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;

				if (pEMS->emPIDEnableState>POWER_CTRL_DISABLE)
				{
					pEMS->emHighSensitivityEnable = pEMS->emPIDEnableState | pEMS->emHighSensitivityEnable;
				}
				
				break;
			case 301: // High Sensitivity Enable
				pEMS->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
				pEMS->emHighSensitivityEnable = pEMS->emPIDEnableState | pEMS->emHighSensitivityEnable;
				break;
			// 充电参数
			case 302:
				pPidChgParams->Kp = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 303:
				pPidChgParams->Ki = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 304:
				pPidChgParams->fDead_Band = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 305:
				pPidChgParams->fProportional_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 306:
				pPidChgParams->fIntegral_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			// 放电参数
			case 307:
				pPidDischgParams->Kp = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 308:
				pPidDischgParams->Ki = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 309:
				pPidDischgParams->fDead_Band = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 310:
				pPidDischgParams->fProportional_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 311:
				pPidDischgParams->fIntegral_Term_Sepa_Thres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			// 其他
			case 312:
				pPidCtx->fClearCompensationThres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 313:
				pPidCtx->fMaxOutput_Lmt = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 314:
				pPidCtx->fMinOutput_Lmt = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 315:
				pPidCtx->fCompensation_Lmt = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 316:
				pPidCtx->fInitCompCoef = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			case 317:
				pPidCtx->nTargetStableCount_Thres = (UINT)(err==ERR_DPR_OK) ? varV.ulValue : 5;
				break;
			case 318:
				pPidCtx->fTargetStableThres = (err==ERR_DPR_OK) ? varV.fValue : 0.0f;
				break;
			default:
				break;
			}
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 110))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->tagGridApplyScenery = (GRID_APPLY_SCENERY)varV.emValue;
			TRACED("tagGridApplyScenery changed to [%d]\n",pCCU->tagGridApplyScenery);
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 111))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pCCU->nSetPriGridMeter = varV.ulValue;
			TRACED("nSetPriGridMeter changed to [%d]\n",pCCU->nSetPriGridMeter);
		}
		//else if((pCfgChange->parmPTR.nId >= 501) && (pCfgChange->parmPTR.nId < (501+MAX_SINGLE_STBATT_NUM)))
		//{
		//	TRACE("nDevId=[%d] nSubId=[%d]\n", pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId);
		//	if(pCfgChange->parmPTR.nSubId == 506)
		//	{
		//		int nBattIdx = pCfgChange->parmPTR.nId - 501;
		//		
		//	}
		//}
		//else if((pCfgChange->parmPTR.nId == 500) && (pCfgChange->parmPTR.nSubId == 501))
		//{
		//	//Battery Mgmt Plan
		//	varV.blobValue = (BYTE *)&pCCU->battPlan;
		//	//copy setting to current data
		//	ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
		//		500, 501, &varV, sizeof(A_BLOB_BATT_MGMT_PLAN));
		//	if(err != ERR_DPR_OK)
		//	{
		//		return EVENT_HANDED_FAILED;
		//	}
		//}
	}
	else if(EVENT_TYPE_SAVE_ES_STATDATA == emEvType) //restore resere info
	{
		if((nEvDataSize != sizeof(EVENT_ES_SAVE_STATDATA)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			TRACE("nEvDataSize=%d EVENT_ES_SAVE_STATDATA=%d\n", nEvDataSize, sizeof(EVENT_ES_SAVE_STATDATA));
			return EVENT_HANDED_FAILED;
		}

		EVENT_ES_SAVE_STATDATA* pESSave = (EVENT_ES_SAVE_STATDATA*)pEvData;
		HIS_ES_STATISTIC_DAILY* pDailyData = pESSave->pDailyData;
		TRACE("-----------now received restore event for EVENT_ES_SAVE_STATDATA-----------\n");

		if(pDailyData != NULL)
		{
			memcpy(&pCCU->hisESData, pDailyData, sizeof(HIS_ES_STATISTIC_DAILY));

			//do not forget to delete it here
			//TRACE("\n",pCCU->hisESStaticData->es_Data.hours_data.);
			
			time_t tmNow = _NOW_;
			struct tm gmTime;
			gmtime_r(&tmNow, &gmTime);
		    int   nCurr_MDay = gmTime.tm_mday - 1;

			time_t tmSave = pCCU->hisESData.es_Data.tmTimeSave;
			struct tm saveTime;
			gmtime_r(&tmSave, &saveTime);
			int	nSave_Hour = 2*saveTime.tm_hour;
			if((saveTime.tm_min >= 30))
			{
				nSave_Hour += 1;
			}
			int	nSave_MDay = saveTime.tm_mday - 1;
    		int	nSave_tmYear = saveTime.tm_year - 120; //2020-1900=120, offset From 120
		    int nFeeRate_Sector = 0;
            nFeeRate_Sector = Get_FeeRate_Setctor_Accroding2Time(&pCCU->blobFeeRate, tmSave);
            if(nFeeRate_Sector != -1)
            {
                pCCU->emLast_FeeRate = pCCU->blobFeeRate.feeRateSectors[nFeeRate_Sector].emFeeRate;
            }
			// TRACE("CCU Sampler:Restore ESData nCurr_Hour=[%d] nCurr_MDay=[%d],nCurr_tmYear=[%d]emLast_FeeRate=[%d]\n",\
			// 	nSave_Hour,nSave_MDay,nSave_tmYear, pCCU->emLast_FeeRate);

			pCCU->nLast_Hour    = nSave_Hour;
            pCCU->nLast_MDay    = nSave_MDay;
            pCCU->nLast_tmYear  = nSave_tmYear;
       		LogOut(MAIN_MODULE_SAMPLER, "CCU_Event", LOG_TYPE_INFO, "Restore SaveTime.tmYear[%d]-MDay[%d] Hour[%d]:tm_min[%d]FeeRate[%d]",
                pCCU->nLast_tmYear,pCCU->nLast_MDay,pCCU->nLast_Hour, saveTime.tm_min,pCCU->emLast_FeeRate);

			pCCU->tmLastCalcESStoreData = tmNow;
			pCCU->tmLastSaveESData2DB = tmNow;

			if(gmTime.tm_year != saveTime.tm_year)
			{
 				//ZERO_POBJS(&pCCU->hisESData.es_Data.days_Data, 1);
				ZERO_POBJS(&pCCU->hisESData.es_Data.hours_Data, 1);
				TRACE("CCU:gmTime.tm_Year=%d -- saveTime.tm_Year=%d\n", gmTime.tm_year , saveTime.tm_year);
			}
	
			if(gmTime.tm_mday != saveTime.tm_mday)
			{
 				ZERO_POBJS(&pCCU->hisESData.es_Data.hours_Data, 1);
        		Init_CurrDayKWH(pCCU, nCurr_MDay); //Added By John 2021-10-16,To Solve KWH Dubble Added Bug.

				LogOut(MAIN_MODULE_SAMPLER, "CCU_TCP_SAMP", LOG_TYPE_INFO, "Curr_MDay=[%d]gm.tm_mday=[%d]save.tm_mday=[%d]",nCurr_MDay, gmTime.tm_mday, saveTime.tm_mday);

				TRACE("CCU: gmTime.tm_mday=%d, saveTime.tm_mday=%d\n",  gmTime.tm_mday , saveTime.tm_mday);
  			}
			
			//int n = 0;
			//for(n = 0; n < MAX_HOURS_SECTOR; n++)
			//{
			//	TRACE("CCU:hours_Data.fAC_CostKWH_Sector[%d]=%f\n",n,pCCU->hisESData.es_Data.hours_Data.fAC_CostKWH_Sector[n]);
			//}

			DELETE(pDailyData);
		}
	}
	else if(emEvType == EVENT_TYPE_BMSRUNDATA_UPDATE) //restore Bms RealTime Runinfo
	{ // EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_RESTORE
		if (HasEMSFeature(pCCU->lcCfg.emSystemType))
		{
			return EVENT_HANDED_OK;
		}
		if((nEvDataSize != sizeof(EVENT_BMSRUNDATA_UPDATE)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			TRACEW("ccu: nEvDataSize=%d EVENT_BMSRUNDATA_UPDATE=%d emMsgType:%d",
				nEvDataSize, sizeof(EVENT_BMSRUNDATA_UPDATE), emMsgType);
			return EVENT_HANDED_FAILED;
		}

		EVENT_BMSRUNDATA_UPDATE* pBmsSave = (EVENT_BMSRUNDATA_UPDATE*)pEvData;
		HIS_BMS_RUNTIMEDATA_REC* pBmsRunDataRec = pBmsSave->pBmsRunDataRec;

		TRACEI("ccu:------now received restore event for EVENT_BMSRUNDATA_UPDATE-----\n");

		if(pBmsRunDataRec != NULL)
		{
			// memcpy(&pCCU->hisBmsRunDataRec, pBmsRunDataRec, sizeof(HIS_BMS_RUNTIMEDATA_REC));
			pCCU->hisBmsRunDataRec.bmsRunData.nBmsRuntimeDataTotal = 0;
			TRACEI("ccu Sampler: Restore BMS RealTime RunInfo From DB\n");
			
			SAFELY_DELETE(pBmsRunDataRec);
		}
	}
	return EVENT_HANDED_OK;
}

// LJH241101 MU3
void CCU_Close_CommPort(SAMPLER* pSampler)
{
	if(pSampler->hCommPort != NULL)
	{
		TRACEW("ccu_sampler_main:CCU_Close_CommPort");
		pSampler->pUsedCommPort->Close(pSampler->hCommPort);
		pSampler->hCommPort = NULL;
	}
}

int CCU_WriteData(ROUGH_CCU_DATA* pCCU, BYTE *szBuff, int nSendLen)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
#ifdef IMMU3
	int nWrite = pSampler->pUsedCommPort->Write(pSampler->hCommPort, (char*)szBuff, nSendLen);
#else
	if (pCCU->hDataComm == NULL)
	{
		TRACEE("CCU_WriteData IMMU2 pCCU->hDataComm == NULL");
		return 0;
	}
	int nWrite = pSampler->pUsedCommPort->Write(pCCU->hDataComm, (char*)szBuff, nSendLen);
#endif
#ifdef _TRACE_PRINT_
	BYTE byCmd = szBuff[3];
	switch (byCmd)
	{
		case 0x03: // UCTL
			TRACEI(">>>CCU_WriteData byCmd<%02X> UCTL sub:%02X nSendLen<%d %d> byBuff<%s>",
					byCmd, szBuff[6], nWrite, nSendLen, printableBytes(szBuff, nWrite));
			break;
		case 0xA0: // USYS
			TRACE(">>>CCU_WriteData byCmd<%02X> USYS nSendLen<%d %d> bySendBuff<%s>", byCmd, nWrite, nSendLen, printableBytes(szBuff, nWrite));
			break;

		default:
			TRACEI(">>>CCU_WriteData byCmd<%02X> nSendLen<%d %d> bySendBuff<%s>", byCmd, nWrite, nSendLen, printableBytes(szBuff, nWrite));
			break;
	}
#endif
	return nWrite;
}

int CCU_ReadData(ROUGH_CCU_DATA* pCCU, BYTE *rdBuff, int nReadLen)
{
	SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
#ifdef IMMU3
	int nReadBytes = pSampler->pUsedCommPort->Read(pSampler->hCommPort, (char*)rdBuff, nReadLen);
#else
	// if(pCCU->hDataComm == NULL)
	// {
	// 	TRACE("CCU_ReadData Error: 444 pCCU->hDataComm == NULL\n");
	// }
	int nReadBytes = pSampler->pUsedCommPort->Read(pCCU->hDataComm, (char*)rdBuff, nReadLen);
#endif
// #ifdef _TRACE_PRINT_
// 	if (nReadBytes > 0)
// 	{
// 		BYTE byCmd = rdBuff[3];
// 		// UCTL
// 		// if (byCmd==0x03 && rdBuff[6]>=0x31 && rdBuff[6]<=0x34)
// 		// {
// 		// 	TRACEI("<<<CCU_ReadData UCTL cmd:%02X len:%d rdBuff<%s>", byCmd, nReadBytes, printableBytes(rdBuff, nReadBytes));
// 		// }
// 		// //else if (byCmd == 0x04) // RTIF
// 		// // else if (byCmd == 0xB0) // LSYS
// 		// // {
// 		// // 	TRACEW("<<<CCU_ReadData LSYS cmd:%02X len:%d rdBuff<%s>", byCmd, nReadBytes, printableBytes(rdBuff, nReadBytes));
// 		// // }
// 		// else
// 		{
// 			TRACEI("<<<CCU_ReadData cmd:%02X len:%d rdBuff<%s>",
// 				byCmd, nReadBytes, printableBytes(rdBuff, nReadBytes));
// 		}
// 	}
// #endif
	return nReadBytes;
}
