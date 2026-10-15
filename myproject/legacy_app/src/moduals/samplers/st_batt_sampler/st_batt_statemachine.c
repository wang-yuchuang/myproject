// #define _TRACE_PRINT_ // batt
#include "st_batt_sampler.h"
// #define TRACE TRACED
// #define TRACEX TRACED

// LJH250408 CtrlByREMS
static BOOL IsMaster(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	return (pSTBatt_Rough->lcCfg.lcType == LC_TYPE_1st && !pSTBatt_Rough->enableREMSCtrl && !pSTBatt_Rough->executeChargingWhenReflux);}

static BOOL IsBatt_Plan2_StopCharge(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	if (IsMaster(pSTBatt_Rough))
	{
		A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

		int n = Get_EmsCfgPlan_Setctor(&pSTBatt_Rough->blobEmsCfgPlan, TRUE);
		if(n == -1)
		{
			return TRUE;
		}

		float fJudgeSOC = (float)pSingleRunInfo->nSOC + 0.05;
		A_BLOB_EMS_CONFIG_PLAN* pEmsCfgPlan = &pSTBatt_Rough->blobEmsCfgPlan;

		TRACE("-----Now judgeSOC->[%.1f], StopSOC->[%d]-------\n", fJudgeSOC, 
			pEmsCfgPlan->emsLmtSectors[n].nStopSOC);

		if(fJudgeSOC >= ((float)pEmsCfgPlan->emsLmtSectors[n].nStopSOC))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
static BOOL IsBatt_Plan2_Alternate(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	int n = Get_EmsCfgPlan_Setctor(&pSTBatt_Rough->blobEmsCfgPlan, nBattIdx);

	if(n == -1)
	{
		return FALSE;
	}

	//CCU will advice whether we need extra st-batt power
	if(!pSTBatt_Rough->branchGrpData[nBattIdx].bIsSTBatt_NeedDischarge)
	{
		return FALSE;
	}

	float fJudgeSOC = (float)pSingleRunInfo->nSOC + 0.01;
	A_BLOB_EMS_CONFIG_PLAN* pEmsCfgPlan = &pSTBatt_Rough->blobEmsCfgPlan;

	TRACEX("-----Now judgeSOC->[%.1f], AlterSOC->[%d]-------\n", fJudgeSOC, 
		pEmsCfgPlan->emsLmtSectors[n].nSOC_Alternate);

	if((fJudgeSOC > ((float)pEmsCfgPlan->emsLmtSectors[n].nSOC_Alternate)) &&
		(pEmsCfgPlan->emsLmtSectors[n].nSOC_Alternate > pEmsCfgPlan->emsLmtSectors[n].nSOC_Force_StartCharge))
	{
		return TRUE;
	}
	
	return FALSE;
}
*/

static BOOL IsBatt_Plan2_CutDischarge(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	float fJudgeSOC = (float)pSingleRunInfo->nSOC - 0.01;

	if(fJudgeSOC < ((float)pSTBatt_Rough->nProtectSOC))
	{
		// TRACE("IsBatt_Plan2_CutDischarge: fJudgeSOC =%f pSTBatt_Rough->nProtect_SOC=%d\n",\
		// 	fJudgeSOC , pSTBatt_Rough->nProtectSOC);
		return TRUE;
	}

	//John-Added 20240413. ZYF Req Need Added.
	if((IsMaster(pSTBatt_Rough)) &&
		(pSTBatt_Rough->emDisChgByStopSOC == TRUE) &&
		(pSTBatt_Rough->emPMCtrlMode == AUTO_CTRL_PM_MODE)
		)
	{
		A_BLOB_EMS_CONFIG_PLAN* pEmsCfgPlan = &pSTBatt_Rough->blobEmsCfgPlan;
		int n = Get_EmsCfgPlan_Setctor(&pSTBatt_Rough->blobEmsCfgPlan, TRUE);
		if(n == -1)
		{
			return FALSE;
		}

		if(fJudgeSOC <= ((float)pEmsCfgPlan->emsLmtSectors[n].nStopSOC))
		{
			TRACE("WArning Dischg n=[%d] fJudgeSOC=[%.1f] nStopSOC=[%d]\n", \
				n, fJudgeSOC, pEmsCfgPlan->emsLmtSectors[n].nStopSOC );
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL IsBatt_Plan2_StartCharge(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	if (IsMaster(pSTBatt_Rough))
	{
		A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

		//Contact With ZTZ,just Use Batt_0 Mgmt Plan
		int n = Get_EmsCfgPlan_Setctor(&pSTBatt_Rough->blobEmsCfgPlan, TRUE);
		if(n == -1)
		{
			return FALSE;
		}
		
		UINT nJudgeSOC = pSingleRunInfo->nSOC;
		A_BLOB_EMS_CONFIG_PLAN* pEmsCfgPlan = &pSTBatt_Rough->blobEmsCfgPlan;

		TRACEX("-----nBattIdx=[%d] Now sector[%d]judgeSOC->[%d], nStopSOC->[%d]  -------\n", nBattIdx, n, nJudgeSOC, 
			pEmsCfgPlan->emsLmtSectors[n].nStopSOC);

		if(nJudgeSOC < pEmsCfgPlan->emsLmtSectors[n].nStopSOC)
		{
			//EMS Mgmt Will Advice wether Need Start Charge

			TRACE("Plan Start Charge!!\n");
			return TRUE;
		}
		else
		{
			TRACE("Not Need Start Charge!\n");
		}
	}
	else
	{
		return (pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING);
	}

	return FALSE;
}

/*
static BOOL IsBatt_Plan2_StartCharge(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

 	//Contact With ZTZ,just Use Batt_0 Mgmt Plan
	int n = Get_EmsCfgPlan_Setctor(&pSTBatt_Rough->blobEmsCfgPlan);
	if(n == -1)
	{
		return FALSE;
	}
	
	UINT nJudgeSOC = pSingleRunInfo->nSOC;
	A_BLOB_EMS_CONFIG_PLAN* pEmsCfgPlan = &pSTBatt_Rough->blobEmsCfgPlan;

	TRACEX("-----nBattIdx=[%d] Now sector[%d]judgeSOC->[%d], nSOC_Force_StartCharge->[%d]  -------\n", nBattIdx, n, nJudgeSOC, 
		pEmsCfgPlan->emsLmtSectors[n].nSOC_Force_StartCharge);

	if(nJudgeSOC < pEmsCfgPlan->emsLmtSectors[n].nSOC_Force_StartCharge)
	{
		//EMS Mgmt Will Advice wether Need Start Charge

		TRACEX("Plan Start Charge!!\n");
		return TRUE;
	}
	else
	{
		TRACE("Not Need Start Charge!\n");
	}
	return FALSE;
}
*/
/* Not Have V2B Anymore
static BOOL Is_AnyGun_In_V2B(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	int n = 0;
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	for(n = 0; n < pSTBatt_Rough->branchGrpData[nBattIdx].nOnlineGun_Total; n++)
	{
		TRACE("Is_AnyGun_In_V2B: emDCDC_WorkStatu[%d]=%d\n", n ,pSTBatt_Rough->branchGrpData[nBattIdx].emDCDC_WorkStatu);
		if(pSTBatt_Rough->branchGrpData[nBattIdx].emDCDC_WorkStatu == DCDC_V2B)
		{
			return TRUE;
		}
	}

	return FALSE;
}
*/

/*
static BOOL Is_AnyDCDC_Chg2EV(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	int n = 0;
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	for(n = 0; n < pSTBatt_Rough->branchGrpData[nBattIdx].nOnlineGun_Total; n++)
	{
		TRACE("Is_AnyDCDC_Chg2EV:emDCDC_WorkStatu[%d]=%d nOnlineGun_Total=%d\n",\
		n, pSTBatt_Rough->branchGrpData[nBattIdx].emDCDC_WorkStatu,pSTBatt_Rough->branchGrpData[nBattIdx].nOnlineGun_Total);
		if(pSTBatt_Rough->branchGrpData[nBattIdx].emDCDC_WorkStatu == STATU_DCDC_RECTIFY)
		{
			return TRUE;
		}
	}

	return FALSE;
}
*/

static void Log_CtrlPcsEventStatus(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx,\
	IN STAT_BMSMGMT_STATUS emMgmtStatus,\
	IN CMD_BMSMGMT_PCSCTRL emMgmtPcsCtrlCmd,\
	IN REASON_BMSMGMT_EXEC emMgmtExecReason)
{
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	//Consider DataEvent had Too Many Element, Not Add This Function Yet.
	//if(pSingleRunInfo->emMgmtStatus != emMgmtStatus)
	//{
	//	EVENT_BMSMGMT_STATCHANGE evtBMSMgmt;
 	//	
	//	evtBMSMgmt.emLast_MgmtStatus = pSingleRunInfo->emMgmtStatus;
 
	//	evtBMSMgmt.emCurrtMgmtStatus = emMgmtStatus;
	//	evtBMSMgmt.emMgmtPcsCtrlCmd  = emMgmtPcsCtrlCmd;
	//	evtBMSMgmt.emMgmtExecReason  = emMgmtExecReason;
 
	//	SendMsg(pSTBatt_Rough->hSampler, EVENT_TYPE_GUN_RESERVE, EVENT_MSG_BEGIN,\
	//		&evtBMSMgmt, sizeof(EVENT_BMSMGMT_STATCHANGE));  
	//}

	pSingleRunInfo->emMgmtStatus = emMgmtStatus;
	pSingleRunInfo->emMgmtPcsCtrlCmd = emMgmtPcsCtrlCmd;
	pSingleRunInfo->emMgmtExecReason = emMgmtExecReason;

	TRACE("Log_CtrlPcsEventStatus:nBattIdx=[%d] [%d, %d, %d]\n",\
		nBattIdx, pSingleRunInfo->emMgmtStatus,
		pSingleRunInfo->emMgmtPcsCtrlCmd,
		pSingleRunInfo->emMgmtExecReason);
	return;
}


static REASON_CHARGE_START Judge_IfNeedChg_Start(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	// if(pSingleRunInfo->bIsBatt_ChargeForbidden == TRUE)
	// {
	// 	TRACEW("Judge_IfNeedChg_Start REASON_CHG_BMS_FORBIDDEN");
	// 	return REASON_CHG_BMS_FORBIDDEN;
	// }

	if(pSingleRunInfo->emIsoResult == STAT_ON_DETECTING)
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_BMS_ISOPENDING");
		return REASON_CHG_BMS_ISOPENDING;
	}
	
	if(fabs(tmNow - pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx]) < 1)
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_LASTSTARTTM_SHORT");
		return REASON_CHG_LASTSTARTTM_SHORT;
	}

	if(fabs(tmNow - pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx]) < 1) //HXM suggest 60S Period,But PCS V2 Not Need Wait anymore
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_LASTSTOPTM_SHORT");
		return REASON_CHG_LASTSTOPTM_SHORT;
	}
	
	if(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower < 1)
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_AVGPWR_BELOWALLOW");
		return 	REASON_CHG_AVGPWR_BELOWALLOW;
	}
	

	if(pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr < 1.0)
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_CALCPWR_LITTLE");
		return REASON_CHG_CALCPWR_LITTLE;
	}

	if(pSingleRunInfo->fAllow_MaxChargeCurr < 10.0)
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_MAXCURR_LITTLE");
		return REASON_CHG_MAXCURR_LITTLE;
	}

	if(pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode != BATT_WORKMODE_FIRST_CHRGING) //in case the charge started too frequently
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_NOTCHGMODE");
		return REASON_CHG_NOTCHGMODE;
	}
	
	if(!IsBatt_Plan2_StartCharge(pSTBatt_Rough, nBattIdx))
	{
		TRACEW("Judge_IfNeedChg_Start REASON_CHG_EMSPLAN_NOTALLOW");
		return REASON_CHG_EMSPLAN_NOTALLOW;
	}
	TRACEI("Judge_IfNeedChg_Start REASON_CHG_ALLOW_START");
	return REASON_CHG_ALLOW_START;
}

 
static REASON_DISCHGING_ON Judge_IfNeedDisChg_ON(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
 	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	// if(pSingleRunInfo->bIsBatt_DisChargeForbidden == TRUE)
	// {
	// 	TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_BMS_FORBIDDEN");
 	// 	return REASON_DISCHG_BMS_FORBIDDEN;
	// }

	if(pSingleRunInfo->emIsoResult == STAT_ON_DETECTING)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_BMS_ISOPENDING");
		return REASON_DISCHG_BMS_ISOPENDING;
	}

	if(fabs(tmNow - pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx]) < 1)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_LASTSTARTTM_SHORT");
		return REASON_DISCHG_LASTSTARTTM_SHORT;
	}
	//HXM suggest 90S Period,But PCS V2 Not Need Wait Anymore
	if(fabs(tmNow - pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx]) < 1)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_LASTSTOPTM_SHORT");
		return REASON_DISCHG_LASTSTOPTM_SHORT;
	}

	
	if(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower < 1)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_AVGPWR_BELOWALLOW");
		return  REASON_DISCHG_AVGPWR_BELOWALLOW;
	}
	
	if(pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr < 1.0)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_CALCPWR_LITTLE");
		return REASON_DISCHG_CALCPWR_LITTLE;
	}

	if(pSingleRunInfo->fAllow_MaxDisChargeCurr < 1.0)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_MAXCURR_LITTLE");
		return REASON_DISCHG_MAXCURR_LITTLE;
	}

	if(pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode != BATT_WORKMODE_FIRST_DISCHRG)
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_NOTDISCHGMODE");
		return REASON_DISCHG_NOTDISCHGMODE;
	}

	if(pSingleRunInfo->fPackTotalCurr < -5.0) //2021-10-12 ,If DisCharge below -5.0,means onDischarging Statu ,Not Send Ctrl DisCharge
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_HAD_ONDISCHG");
		return REASON_DISCHG_HAD_ONDISCHG;
	}

	if(IsBatt_Plan2_CutDischarge(pSTBatt_Rough, nBattIdx))
	{
		TRACEW("Judge_IfNeedDisChg_ON REASON_DISCHG_ENDBY_STOPSOC");
		return REASON_DISCHG_ENDBY_STOPSOC;
	}

	TRACEI("Judge_IfNeedDisChg_ON REASON_DISCHG_ALLOW_ON");
	return REASON_DISCHG_ALLOW_ON;
}

static void OnHandle_DIStatus(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	return;
}

// static void OnHandle_ABUOffgrid(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
// {
// 	double tmNow = GetCurrentTime();
// 	//---Added By John 2023-04-13.For Auto On-Off Grid Mode.---//
// 	if(JudgeIfPCS_OnHotWaiting(pSTBatt_Rough, nBattIdx) == TRUE)
// 	{	
// 		TRACE("Idx=[%d] byPCSSysStatu=[%d]\n", nBattIdx , pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].byPCSSysStatu);

// 		BOOL bPCSAvaliable = ((pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].SysStatu.Byte0_Bit0to2 == 0) ||
// 						(pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].SysStatu.Byte0_Bit0to2 == 4)) ? TRUE : FALSE; //4-Alarm

// 		if((pSTBatt_Rough->esRunData.mapSTSCode.Status.FaultState == 0) && //0-Normal Status, 1-Fault.
// 			(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_IDLE) && //Need Judge PCS Fault Status
// 			(bPCSAvaliable == TRUE)) 
// 		{
// 			if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= MAX_PWRONPCS_SEC)
// 			{
// 				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_PENDING, nBattIdx);

// 				TRACE("GRP_ACDC_INVERT_PENDING: nBattIdx=[%d] emACDC_WorkStatu=[%d] FaultState=[%d]\n",\
// 					nBattIdx, pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu,\
// 					pSTBatt_Rough->esRunData.mapSTSCode.Status.FaultState);
// 			}
// 		}
// 	}

// 	return;
// }

static void OnHandle_PCSHotWaiting(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	double tmNow = GetCurrentTime();
	//---Added By John 2023-04-13.For Auto On-Off Grid Mode.---//
	if(JudgeIfPCS_OnHotWaiting(pSTBatt_Rough, nBattIdx) == TRUE)
	{	
		TRACE("Idx=[%d] byPCSSysStatu=[%d]\n", nBattIdx , pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].byPCSSysStatu);

		BOOL bPCSAvaliable = ((pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].SysStatu.Byte0_Bit0to2 == 0) ||
						(pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].SysStatu.Byte0_Bit0to2 == 4)) ? TRUE : FALSE; //4-Alarm

		if((pSTBatt_Rough->esRunData.mapSTSCode.Status.FaultState == 0) && //0-Normal Status, 1-Fault.
			(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_IDLE) && //Need Judge PCS Fault Status
			(bPCSAvaliable == TRUE) &&
			(pSTBatt_Rough->bBattProt == FALSE)) 
		{
			if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= MAX_PWRONPCS_SEC)
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_PENDING, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_AUTOOFFGRID, CMD_MGMT_DISCHG_STANDBY, REASON_DISCHG_ALLOW);

				TRACE("GRP_ACDC_INVERT_PENDING: nBattIdx=[%d] emACDC_WorkStatu=[%d] FaultState=[%d]\n",\
					nBattIdx, pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu,\
					pSTBatt_Rough->esRunData.mapSTSCode.Status.FaultState);
			}
		}
	}
	else if(pSTBatt_Rough->bHotWaitFlag == 1) // 非热备状态且 热备flag==1，需要关闭pcs
	{
		pSTBatt_Rough->bHotWaitFlag = 0;
		Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
	}

	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : OnIdle
*  CREATOR       :  
*  CREATED DATE  : 2020-10-19 14:33:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 

*	GRP_ACDC_RECTIFY_START,
	GRP_ACDC_RECTIFY_ADJ_CURR,
	GRP_ACDC_INVERT_START,
	GRP_ACDC_INVERT_PENDING,
	GRP_ACDC_STOP,

	STAT_BMSMGMT_STATUS		pSingleRunInfo->emMgmtStatus;
	CMD_BMSMGMT_PCSCTRL		pSingleRunInfo->emMgmtPcsCtrlCmd;
	REASON_BMSMGMT_EXEC		pSingleRunInfo->emMgmtExecReason;

	//sometimes BMS Report SOC is 0.0,cause Auto Charge when CommFail->commOK
	TRACE("Idle nBattIdx=[%d] bIsBatt_DisChargeForbidden=[%d] fAllow_MaxDisChargeCurr=[%.1f] fBatt_Distri_4_ACDC_Pwr=[%.1f] \
		fPackTotalCurr=[%.1f] emBatt_WorkMode=[%d] fACDC_Distri_4_BattChrg_Pwr=[%.1f] fAllow_MaxChargeCurr=[%.1f]\n",\
		nBattIdx,
		pSingleRunInfo->bIsBatt_DisChargeForbidden,
		pSingleRunInfo->fAllow_MaxDisChargeCurr ,
		pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr,
		pSingleRunInfo->fPackTotalCurr,
		pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode,\
		pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr,\
		pSingleRunInfo->fAllow_MaxChargeCurr); 
*  
***********************************************************************************/
STBATT_STATEMACHINE_STATE OnIdle(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	if((pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	{
		TRACE("nBattIdx=[%d]:STBATT_STATE_FAULTED->emBattStatus_Run =%d\n", nBattIdx, pSingleRunInfo->emBattStatus_Run);
		
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
		}
		return STBATT_STATE_FAULTED;
	}

	//---Added By John 2023-04-13.For Auto On-Off Grid Mode.
	OnHandle_PCSHotWaiting(pSTBatt_Rough, nBattIdx);

	if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, nBattIdx) == TRUE)
	{
		return  STBATT_STATE_OFFGRID;
	}
	//--------End 2023-04-13 --------//

	pSingleRunInfo->emReason_ChgStart = Judge_IfNeedChg_Start(pSTBatt_Rough, nBattIdx);
	pSingleRunInfo->emReason_DisChgON = Judge_IfNeedDisChg_ON(pSTBatt_Rough, nBattIdx); 

	if(pSingleRunInfo->emReason_ChgStart == REASON_CHG_ALLOW_START)
	{
		pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx] = tmNow;
		if(Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_RECTIFY_START, nBattIdx))
		{
			pSTBatt_Rough->bBattProt = FALSE;
 			if(pSingleRunInfo->nSOC >= 95)
			{
				LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Ctrl Chrg Start:Idx[%d]IPack[%.1f]Imax[%.1f]Ilast[%.1f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr, pSingleRunInfo->nSOC);
			}
			pSingleRunInfo->fChg_Start_MaxCellTemp = pSingleRunInfo->fMax_CellTemp;
			pSingleRunInfo->fChg_Start_MinCellTemp = pSingleRunInfo->fMin_CellTemp;
			
			Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_CHRG_ON, REASON_CHG_ALLOW);

			return STBATT_STATE_WAIT_CHARGE;
		}
		else
		{
			TRACE("nBattIdx=[%d] Start charge Cmd Send Ctrl CCU Failed!\n", nBattIdx);
		}
	}
	else if(pSingleRunInfo->emReason_DisChgON == REASON_DISCHG_ALLOW_ON)
	{
		pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx] = tmNow;
		if(Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_START, nBattIdx))
		{
			pSTBatt_Rough->bBattProt = FALSE;
			pSingleRunInfo->fDischg_Start_MaxCellTemp = pSingleRunInfo->fMax_CellTemp;
			pSingleRunInfo->fDischg_Start_MinCellTemp = pSingleRunInfo->fMin_CellTemp;

			Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_DISCHG_ON, REASON_DISCHG_ALLOW);

			if(pSingleRunInfo->nSOC <= 10)
			{
				LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"Ctrl DisChrg Start:Idx[%d]IPack[%.1f]Imax[%.1f]Ilast[%.1f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxDisChargeCurr,\
					pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC);
			}
			return STBATT_STATE_IN_DISCHARGING;
		}
		else
		{
			TRACEX("nBattIdx=[%d] Start Discharge Cmd Send Ctrl CCU Failed!\n", nBattIdx);
		}
	}
	else if(pSingleRunInfo->emBattStatus_Run == STBATT_RUNSTAT_SLEEP)
	{
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= STBATT_PUSH2CCU_PERIOD)
		{
			 if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
			{
				TRACE("nBattIdx=[%d]Stop ACDC Invert Mode\n", nBattIdx);
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_BMS_SLEEP);
			}
		}
	}

	if(pSTBatt_Rough->lcCfg.emSystemType < SYSTYPE_EMS_STATION)
	{
		BattProtect(pSTBatt_Rough, nBattIdx);
	}
	if(pSTBatt_Rough->bBattProt == TRUE)
	{
		return STBATT_STATE_IDLE; 
	}
	
	//should Excute STBatt Mgmt First,Judge If need Plan To Charge 
	if((pSingleRunInfo->fPackTotalCurr > 2.0)/* || 
			Is_AnyGun_In_V2B(pSTBatt_Rough, nBattIdx) ||
			(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STBATT_ACDC_CHRG2STBATT)*/)
	{
		TRACEX("nBattIdx=[%d] STBATT_STATE_IN_CHARGING:->fPackTotalCurr =%.1f\n", nBattIdx, pSingleRunInfo->fPackTotalCurr);
		//it is possible that CCU started the charge via V2B
		//LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
		//	 	"STBattery Idx=[%d]charge ALREADY started!, nSOC = [%d]!", nBattIdx, pSingleRunInfo->nSOC);
		return STBATT_STATE_IN_CHARGING;
	}
	//LYX Recommand 1.0A as a statu to judge Charge/Discharge Statu
	//SBK Merge Recommand merge statu have a offset current, MAx will be 2A
	else if((pSingleRunInfo->fPackTotalCurr < -2.0) /*|| 
			Is_AnyDCDC_Chg2EV(pSTBatt_Rough, nBattIdx) ||
			(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STBATT_ACDC_B2G)*/)
	{
		TRACE("nBattIdx=[%d] STBATT_STATE_IN_DISCHARGING->fPackTotalCurr =%.1f\n", nBattIdx, pSingleRunInfo->fPackTotalCurr);
		return STBATT_STATE_IN_DISCHARGING;
	}
	else 
	{
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= 300) //Waiting 5Min,Avoid Frequency On/Off,CB Suggest Wait 15Min;But Idle Cost is Too Higher(300W)
		{
			if(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower < 1.0)
			{
				if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY) ||
					(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
				{ 
					TRACEX("111 Rectify Stop!!!! nBattIdx=[%d] fAvg_ChrgLmtPower=[%.1f] emACDC_WorkStatu=[%d]\n", \
						nBattIdx, pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower, pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
					
					Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx); 
					
					Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_PCHRGAVG_LOW);

					LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
						"Ctrl Chrg-Stop:Idx[%d]", nBattIdx);
				}
			}

			if((pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower < 1.0) && 
				(pSTBatt_Rough->emSTSAutoManual == MANUAL_ONOFFGRID_MODE))
			{ 
				if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER) ||
					(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
				{
					Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);  
					
					TRACE("222 Rectify Stop!!!! nBattIdx=[%d] fAvg_ChrgLmtPower=[%.1f]\n", \
						nBattIdx, pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower);
					
					Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_PDISCHGAVG_LOW);

					LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
						"Ctrl DisChrg-Stop:Idx[%d]", nBattIdx);
				}
			}
		}

		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= 5) //John Added. PCS Pending Status hava a samll Current 0.1A.
		{
			if(pSingleRunInfo->bIsBatt_ChargeForbidden == TRUE)
			{
				if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY) &&
					(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
				{ 
					Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx); 
					
					Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_CHRG);

					LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
						"Forbid Chrg: Ctrl Chrg-Stop:Idx[%d]ACDC_WorkStatu=[%d]fACDC_RealTime_Volt=[%.1f]", \
						nBattIdx,pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu, pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt);
				}
			}


			if(pSingleRunInfo->bIsBatt_DisChargeForbidden == TRUE)
			{
				if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER) &&
					(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
				{ 
					Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx); 
					
					Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_IDLE, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_DISCHG);
			
					LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
						"Forbid DisChrg: Ctrl DisChrg-Stop:Idx[%d]", nBattIdx);
				}
			} 
		}

	}

	return STBATT_STATE_IDLE; 
}

STBATT_STATEMACHINE_STATE OnWaitCharge(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	if((pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	{
		TRACEX("emBattStatus_Run=%d  GrpAlmStatus=%d\n", pSingleRunInfo->emBattStatus_Run,pSingleRunInfo->emBattStatus_Alm);
		
		Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
		
		Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_WAITCHG, CMD_MGMT_STOP, REASON_FAULT_ALARM);

		return STBATT_STATE_IDLE;
	}
	
	//Added By John For Off-Grid Func. 2023-04-11.
	if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, nBattIdx) == TRUE)
	{
		return  STBATT_STATE_OFFGRID;
	}

	if((pSingleRunInfo->fPackTotalCurr > 2.0) &&
		(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY))
	{
		TRACEX("emACDC_WorkStatu=%d fPackTotalCurr=%.1f\n",\
			pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu, pSingleRunInfo->fPackTotalCurr);
		LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
			"Start Chrg Ok:Idx[%d]IPack[%.1f]Imax[%.1f]Ilast[%.1f]Irect[%.1f]SOC[%d]!", \
			nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
			pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
			pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC);
		return STBATT_STATE_IN_CHARGING;
	}
	
	if(fabs(tmNow - pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx]) >= 90)//PCS Need Over 30S Wait Power Output
	{
		//means start FAIL
		LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
			"Start Chrg Fail:Idx[%d]IPack[%.1f]Imax[%.1f]Ilast[%.1f]Irect[%.1f]SOC[%d]!", \
			nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
			pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
			pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC);
		return STBATT_STATE_IDLE;
	}
	else if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= STBATT_PUSH2CCU_PERIOD)
	{
		Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_RECTIFY_START, nBattIdx);

		Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_WAITCHG, CMD_MGMT_CHRG_ON, REASON_CHG_ALLOW);
	}

	return STBATT_STATE_WAIT_CHARGE; 
}

STBATT_STATEMACHINE_STATE OnFaulted(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	//TRACE("ONFault:nBattIdx=[%d]  fPackTotalCurr=%.1f emBattStatus_Run=%d emBattStatus_Alm=%d\n",\
	//	nBattIdx, pSingleRunInfo->fPackTotalCurr,\
	//	pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm);

	if((pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
	(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
	(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	{
		//TRACEX("nBattIdx=[%d]:STBATT_STATE_FAULTED->emBattStatus_Run =%d ccuStatu=%d\n", \
		nBattIdx, pSingleRunInfo->emBattStatus_Run, pSTBatt_Rough->ccuStatu);
		
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_FAULT, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
		}
		return 	STBATT_STATE_FAULTED;
	}

	if((pSTBatt_Rough->ccuStatu == CCU_STATE_IDLE) && 
		(STBATT_ALMSTAT_NOMAL == pSingleRunInfo->emBattStatus_Alm))
	{
		TRACE("on_Fault ccuStatu=%d,emBattStatus_Alm = %d: 111\n", \
			pSTBatt_Rough->ccuStatu,pSingleRunInfo->emBattStatus_Alm);
			
		//if has sleep batt, need wake it up first
		//BOOL bHasSleep = FALSE;

		if(pSingleRunInfo->emBattStatus_Run == STBATT_RUNSTAT_SLEEP)
		{
			TRACE("on_Fault :222\n");
			//bHasSleep = TRUE;
			Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_WAKE_UP, nBattIdx);
		}
		
		//no -error, now go back to idle
		return STBATT_STATE_IDLE; 
	}
	else
	{
		TRACE("Fault : Has Alarm 444\n");
	}

	TRACEX("return STBATT_STATE_FAULTED\n");
	return STBATT_STATE_FAULTED; 
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnIdle
*  CREATOR       :  
*  CREATED DATE  : 2020-10-19 14:33:53
*  DESCRIPTION   :  
*  
*	GRP_ACDC_RECTIFY_START,
	GRP_ACDC_RECTIFY_ADJ_CURR,
	GRP_ACDC_INVERT_START,
	GRP_ACDC_INVERT_PENDING,
	GRP_ACDC_STOP,
*
*  CHANGE HISTORY: 
 
	电池充电：
	1.ACDC模块给电池充电，当满足停止充电条件，关ACDC模块；
	2.MPPT模块给电池充电，当电池需求充电功率为0时，IMSU_D需要根据MPPT分配给电池充电功率进行调节输出电压；
*
***********************************************************************************/
STBATT_STATEMACHINE_STATE OnCharging(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{	
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	float fLimitCurrent = 0.0;
	double tmNow = GetCurrentTime();
	float fPackTotalCurr = fabs(pSingleRunInfo->fPackTotalCurr);

	TRACE("time=[%f] nBattIdx=[%d]: fPackTotalCurr=%f bIsBatt_ChargeForbidden=[%d] fAllow_MaxChargeCurr=[%.1f] fACDC_RealTime_Curr=[%.1f] emACDC_WorkStatu=[%d] fACDC_RealTime_Volt=[%.1f]\n", \
		tmNow, nBattIdx, pSingleRunInfo->fPackTotalCurr,\
		pSingleRunInfo->bIsBatt_ChargeForbidden,\
		pSingleRunInfo->fAllow_MaxChargeCurr,\
		pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,\
		pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu,\
		pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt);

	//John. 2024-04-23 Added For MPPT is Chging, EMS is On_DisChgMode.
	TRACE("nBattIdx=[%d] emMPPT_WorkStatu=[%d] fMPPT_RealTime_Volt=[%.1f]\n", \
		nBattIdx, pSTBatt_Rough->branchGrpData[nBattIdx].emMPPT_WorkStatu,\
		pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Volt);
	if((pSTBatt_Rough->branchGrpData[nBattIdx].emMPPT_WorkStatu == STATU_MPPT_RECTIFY)
		&& (pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Volt > 100.0))
	{
		pSingleRunInfo->emReason_DisChgON = Judge_IfNeedDisChg_ON(pSTBatt_Rough, nBattIdx); 
		if(pSingleRunInfo->emReason_DisChgON == REASON_DISCHG_ALLOW_ON)
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= STBATT_PUSH2CCU_PERIOD))  
			{	
				//pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx] = tmNow;
		
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_START, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_DISCHG, CMD_MGMT_DISCHG_ADJ, REASON_DISCHG_ALLOW);

				TRACEX("Chg2DisCharg Adjust Cmd: nBattIdx=[%d] Adjust fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=[%.1f]\n",\
					nBattIdx, pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx], pSingleRunInfo->fPackTotalCurr);
			}
			return STBATT_STATE_IN_DISCHARGING;
		}
	}
	
	if(pSingleRunInfo->bIsBatt_ChargeForbidden ||
		IsBatt_Plan2_StopCharge(pSTBatt_Rough, nBattIdx) ||
		(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
		(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE) ||
		(pSingleRunInfo->fAllow_MaxChargeCurr < 5.0) ||
		((pSingleRunInfo->fPackTotalVolt - pSingleRunInfo->fAllow_LmtChargeVolt) > 10) ||
		// LJH241120 fixChrgAndDisTogether
		(pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode != BATT_WORKMODE_FIRST_CHRGING)
		)
	{
		//now wait charge to be ended
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY) 
			/*(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt >= 50.0))//PCS is Idle,But Output Volt is 800V.*/
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_CHGING, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
		}
		// LJH241202 Added
		return STBATT_STATE_IDLE;
	}

	//Added By John. Stop PCS again. SomeTimes PCS Status is Abnormal.
	if((pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
		(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	{
		//now wait charge to be ended
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= STBATT_PUSH2CCU_PERIOD)
		{
			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
			Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_CHGING, CMD_MGMT_STOP, REASON_FAULT_ALARM);
		}
	}


	//Added By John For Off-Grid Func. 2023-04-11.
	if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, nBattIdx) == TRUE)
	{
		return  STBATT_STATE_OFFGRID;
	}

	if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_IDLE) || 
		(fabs(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr) < 1.0))
	{
		//Only MPPT Running,should goto Idle,judge Charge Plan 
		//pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx] = _NOW_;

			TRACEX("STBattery charge ENDED OK!, AvgSOC = [%.1f]\n", (float)pSingleRunInfo->nSOC);
			//LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
			//	"STBattery IDx=[%d] charge ENDED OK!, AvgSOC = [%.1f] fACDCRTCurr=[%.1f]!", nBattIdx, (float)pSingleRunInfo->nSOC, pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr);
		return STBATT_STATE_IDLE; 
	}
	else if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY)
	{
		if(pSingleRunInfo->fPackTotalVolt > 50.0)
		{
			fLimitCurrent = (pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr*1000)/pSingleRunInfo->fPackTotalVolt;
		}

		float fVoltReqDiff = fabs(pSTBatt_Rough->fLastSendReqVolt_2CCU[nBattIdx] - MIN(pSingleRunInfo->fAllow_LmtChargeVolt, 1000));
		float fCurrReqDiff = fabs(pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx] - MIN(pSingleRunInfo->fAllow_MaxChargeCurr, fLimitCurrent));
		
		float fCurrOutDiff = pSingleRunInfo->fPackTotalCurr - pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Curr - MIN(pSingleRunInfo->fAllow_MaxChargeCurr, fLimitCurrent);
		
		//float fCurrOutDiff = pSingleRunInfo->fPackTotalCurr - pSingleRunInfo->fAllow_MaxChargeCurr;
		//2021-11-17,MPPt Output Current will Cause PackCurr Over MAx_AllowCurr

		TRACEX("tmNow=%f nBattIdx=[%d] fVoltReqDiff=[%.1f], fCurrReqDiff=[%.1f]fCurrOutDiff=%f,\
			fPackTotalCurr=%f,fLastSendReqCurr_2CCU=[%.1f] fAllow_MaxChargeCurr=[%.1f] fACDC_Distri_4_BattChrg_Pwr=[%.1f],\
			fMPPT_RealTime_Curr=[%.1f]\n",\
			tmNow, nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, pSingleRunInfo->fPackTotalCurr,\
		pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx], pSingleRunInfo->fAllow_MaxChargeCurr,\
		pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr, pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Curr);

		//20220927-Remove By John.Avoid On|Off Frequently.
		if(/*(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower <= 1.0) ||*/ (pSingleRunInfo->fAllow_MaxChargeCurr < 10.0))//20220415,John Modify,Samll Current,Stop Charging.
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))
			{	
				TRACE("Stop Charging:nBattIdx=[%d]fAvg_LmtCurrent=[%.1f] fLimitCurrent=[%.1f]\n", \
					nBattIdx, pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower, fLimitCurrent);
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_CHGING, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_CHRG);
			}
		}
		else if(((pSTBatt_Rough->emHighSensitivityEnable == POWER_CTRL_CHARGE) || (pSTBatt_Rough->emHighSensitivityEnable == POWER_CTRL_CAHARGE_DISCHARGE)) ||
				(fabs(fVoltReqDiff) > 2.0) || (fabs(fCurrReqDiff) > 2.0) || (fCurrOutDiff > 3.0)) 
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
			{	
				TRACEX("Charge Adjust Cmd:tmNow=%f - pSTBatt_Rough->tm_fLastSendCmd_2CCU[%d]=%lf\n", \
				tmNow , nBattIdx, pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]);
				if((pSingleRunInfo->fAllow_MaxChargeCurr < 20.0) || (fabs(fCurrReqDiff) > 10))
				{
					//LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					//	"Adj:Idx=[%d]IPack[%.1f]Imax[%.0f]Ilmt[%.0f]Ilast[%.0f]Irect[%.1f]SOC[%d]!", \
					//	nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					//	fLimitCurrent,pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
					//	pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC);
				}
				// printf("OnCharging: %d\n", pSTBatt_Rough->emHighSensitivityEnable);
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_RECTIFY_START, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_CHGING, CMD_MGMT_CHRG_ADJ, REASON_CHG_ALLOW);
			}		
		}
		else if(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower <= 1.0) //John 20230824,Long Time No Power Dischg,PowerOff PCS.
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= 120))
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_CHGING, CMD_MGMT_STOP, REASON_PCHRGAVG_LOW);
			}
		}
		
		TRACEX("777 nBattIdx=[%d] Adjust ChargePower fVoltReqDiff=%f fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%f\n",\
				nBattIdx, fVoltReqDiff, fCurrReqDiff, fCurrOutDiff, fLimitCurrent);
	}

	return STBATT_STATE_IN_CHARGING; 
}

STBATT_STATEMACHINE_STATE OnDischarging(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	double tmNow = GetCurrentTime();
	float fLimitCurrent = 0.0;
	if(pSingleRunInfo->bIsBatt_DisChargeForbidden ||
	 	IsBatt_Plan2_CutDischarge(pSTBatt_Rough, nBattIdx) ||
	 	(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
	 	(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE) ||
		((pSingleRunInfo->fAllow_LmtDisChargeVolt - pSingleRunInfo->fPackTotalVolt) > 10) ||
		// LJH241120 fixChrgAndDisTogether
		(pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode != BATT_WORKMODE_FIRST_DISCHRG)
	)
	{
	 	//now wait charge to be ended
	 	//send Battery Distribute DCDC Power to -D,-D Judge then Close DCDC 
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
	 	{
			//John Modify 2021-10-13,When Need Stop Discharge,Not Judge ACDC Statu,Stop Directly.
			//if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
			{
	 			Send_Ctrl_CCU(pSTBatt_Rough,  GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_DISCHG, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
	 	}
		// LJH241202 Added
		return STBATT_STATE_IDLE;
	}

	//Added By John For Off-Grid Func. 2023-04-11.
	if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, nBattIdx) == TRUE)
	{
		return  STBATT_STATE_OFFGRID;
	}

	if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_IDLE) || 
		(fabs(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr) < 1.0))
	{
		//Only MPPT Running,should goto Idle,judge Charge Plan 
		//pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx] = _NOW_;
		if(pSingleRunInfo->fPackTotalCurr > -1.0) 
 		{
			TRACE("STBattery Discharge ENDED OK!, AvgSOC = [%d]\n", pSingleRunInfo->nSOC);
			if((pSingleRunInfo->nSOC <= 10) && (pSingleRunInfo->fAllow_MaxChargeCurr < 30))
			{
				LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
					"DisChrg End:Idx[%d]IPack[%.1f]Imax[%.0f]Ilast[%.1f]Irect[%.1f]SOC[%d]!", \
					nBattIdx,pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx],\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC);
			}
		}
		return STBATT_STATE_IDLE; 
	}
	else if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
	{
		if(pSingleRunInfo->fPackTotalVolt > 0)
		{
			//2021-01-03 Do Not Add Distri4DCDC Power,IMSU_D will Adjust the Output Power According To Dsitri4DCDC Power 
			//fLimitCurrent = ((pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr+pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_DCDC_Pwr)*1000)/pSingleRunInfo->fPackTotalVolt;
			fLimitCurrent = (pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr*1000)/pSingleRunInfo->fPackTotalVolt;

		}

		float fCurrReqDiff = fabs(fabs(pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx]) -
			MIN(fLimitCurrent, fabs(pSingleRunInfo->fAllow_MaxDisChargeCurr)));
		float fCurrOutDiff = fabs(fabs(pSingleRunInfo->fPackTotalCurr) -
			MIN(fLimitCurrent, fabs(pSingleRunInfo->fAllow_MaxDisChargeCurr)));
		
	 	if(fabs(pSingleRunInfo->fAllow_MaxDisChargeCurr) < 10.0)  
		 	/*(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower <= 1.0))//Below 5KW, OFF PCS John 20220927 Remove.Avoid On|Off Frequently*/
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD) )
			{	
				TRACEX("Stop DisCharging:fAvg_LmtCurrent=[%.1f] fLimitCurrent=[%.1f]\n", \
					pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower, pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr);
		
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_DISCHG, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_DISCHG);

			}	
		}
		else if((pSTBatt_Rough->emHighSensitivityEnable>POWER_CTRL_CHARGE) || (fabs(fCurrReqDiff) > 2.0) || ((fCurrOutDiff > 3.0) && (pSingleRunInfo->fPackTotalCurr < -3.0)))
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD))//Step Speed to 1.0 Cycle to Adjust the Power 
			{			
				// printf("OnDischarging: %d\n", pSTBatt_Rough->emHighSensitivityEnable);

				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_START, nBattIdx);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_DISCHG, CMD_MGMT_DISCHG_ADJ, REASON_DISCHG_ALLOW);

				TRACEX("DisCharg Adjust Cmd: nBattIdx=[%d] Adjust ChargePower fCurrReqDiff=%f fCurrOutDiff=%f \
					fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f fPackTotalCurr=%.1f\n",\
					nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
					pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx], pSingleRunInfo->fPackTotalCurr);
			}
		}
		else if(pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower <= 1.0)//2022-12-5:John Found KYC Not Stop DisChg when Battery Curr is 2.1A
		{
			if((fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= 180)) 
			{			
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_DISCHG, CMD_MGMT_STOP, REASON_PDISCHGAVG_LOW);
			}	
		}
	
		TRACE("999 nBattIdx=[%d] Adjust DisChargePower fCurrReqDiff=%f fCurrOutDiff=%f fLimitCurrent=%.1f fLastSendReqCurr_2CCU=%.1f\n",\
			nBattIdx, fCurrReqDiff, fCurrOutDiff ,fLimitCurrent ,\
			pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx]);
	}

	return STBATT_STATE_IN_DISCHARGING; 
}




/*********************************************************************************
*  
*  FUNCTION NAME : PrePare_StateMachine
*  CREATOR       :  
*  CREATED DATE  : 2022-07-20 11:13:53
*  DESCRIPTION   :  
*  			0x01-Start Charging
			0x02-Pending (Adjust Charging Current)
			0x03-Start DisCharge
			0x04-Pending (Adjust DisCharging Current)
			0x00-Stop Charging
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Enter_ManualCtrlPMMode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	//DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	VAR_VALUE varV;
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	VAR_VALUE varVSett;
 	A_BLOB_MANUALCTRL_RECT_CMD blobManuCtl;
    ZERO_POBJS(&blobManuCtl, 1);
    varVSett.blobValue = (BYTE*)&blobManuCtl;
    pSTBatt_Rough->pDPR->Get(pSampler, GETTYPE_SP_VAR, 200, 50, &varVSett, sizeof(A_BLOB_MANUALCTRL_RECT_CMD));
   
	//----John 20240513 Added For EN50549 Auth.OverTemp Stop.----//
	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE, 1, 12, &varV, 4);
	BOOL bSysOverTemp = (varV.emValue == 1) ? TRUE : FALSE;
	if(bSysOverTemp == TRUE)
	{
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
			{
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				//Manual Ctlr Mode Protect Logic
				pSTBatt_Rough->blobManualCtrlPM.emState   = 0; 				//0-means PM Stop Work 
				pSTBatt_Rough->blobManualCtrlPM.nGrp = nBattIdx + 1; 		//0-means All PM Grp
				pSTBatt_Rough->blobManualCtrlPM.fGrpVolt = 0;
				pSTBatt_Rough->blobManualCtrlPM.fGrpCurr = 0;
				pSTBatt_Rough->blobManualCtrlPM.nStopSOC = pSTBatt_Rough->nStopSOC[nBattIdx];
				pSTBatt_Rough->blobManualCtrlPM.nAdjMode = blobManuCtl.nAdjMode;
				varV.blobValue = (BYTE *)&pSTBatt_Rough->blobManualCtrlPM;

				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_STOP, REASON_FAULT_ALARM);

				LogOut4c(MAIN_MODULE_SAMPLER, "SysFault|OverTemp Manual Stop", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
				
				return;
			}
		}
	}
	//--- End ---//
	
	//20220624-John Added For No Battey Inst But need PM Output
	if(pSingleRunInfo->emBattStatus_Run == STBATT_RUNSTAT_NOT_INSTALLED)
	{
		return;
	}
 
	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 
		501 + nBattIdx, 507, &varV, 4);
	pSTBatt_Rough->nStopSOC[nBattIdx] = varV.ulValue;

	//Manual Ctlr Mode Protect Logic
	pSTBatt_Rough->blobManualCtrlPM.emState   = 0; 				//0-means PM Stop Work 
	pSTBatt_Rough->blobManualCtrlPM.nGrp = nBattIdx + 1; 		//0-means All PM Grp
	pSTBatt_Rough->blobManualCtrlPM.fGrpVolt = 0;
	pSTBatt_Rough->blobManualCtrlPM.fGrpCurr = 0;
	pSTBatt_Rough->blobManualCtrlPM.nStopSOC = pSTBatt_Rough->nStopSOC[nBattIdx];

	pSTBatt_Rough->blobManualCtrlPM.nAdjMode = blobManuCtl.nAdjMode;
	

	varV.blobValue = (BYTE *)&pSTBatt_Rough->blobManualCtrlPM;

	if((pSingleRunInfo->emBattStatus_Run == STBATT_RUNSTAT_COMM_FAIL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE) ||
		(bSysOverTemp == TRUE))
	{
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
			{
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_STOP, REASON_FAULT_ALARM);

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_1", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
				
				return;
			}
		}
	}
	else if((pSingleRunInfo->bIsBatt_ChargeForbidden == TRUE) || (pSingleRunInfo->fAllow_MaxChargeCurr <= 5.0))
	{
		if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY) 
			&& (pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
		{
			TRACEX("nBattIdx=[%d] emACDC_WorkStatu=[%d] bIsBatt_ChargeForbidden=[%d] fAllow_MaxChargeCurr=[%.1f]\n", \
				nBattIdx,pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu,\
				pSingleRunInfo->bIsBatt_ChargeForbidden, pSingleRunInfo->fAllow_MaxChargeCurr);

			if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
			{	
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_CHRG);

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_2", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
				
				return;
			}
		}
	}
	else if((pSingleRunInfo->bIsBatt_DisChargeForbidden == TRUE) || 
			IsBatt_Plan2_CutDischarge(pSTBatt_Rough, nBattIdx) ||
			(pSingleRunInfo->fAllow_MaxDisChargeCurr <= 5.0))
	{
		if((pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
			&& (pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0))
		{
			if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
			{
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_STOP, REASON_BMSFORBIDDEN_DISCHG);

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_3", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
					
				return;
			}
		}
	}
	
	//When Charging Mode ,Need Adjust Current Follow the Max Charging Limit
	//TRACE("nBattIdx=[%d] emACDC_WorkStatu=[%d]\n",nBattIdx ,pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
	if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
	{
		if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY)
		{
			float fVoltReqDiff = pSingleRunInfo->fPackTotalVolt - pSingleRunInfo->fAllow_LmtChargeVolt;
			float fCurrOutDiff = pSingleRunInfo->fPackTotalCurr - pSingleRunInfo->fAllow_MaxChargeCurr;
		
			if((fVoltReqDiff > 2.0) || (fCurrOutDiff > 0.5)) //Over 0.5A,Adjust Current
			{
				pSTBatt_Rough->blobManualCtrlPM.emState = 2; 				//1-Adjust Current When Charging
				pSTBatt_Rough->blobManualCtrlPM.nGrp = nBattIdx + 1; 		//Which Battery Group Index
				pSTBatt_Rough->blobManualCtrlPM.fGrpVolt = pSingleRunInfo->fPackTotalVolt;
				pSTBatt_Rough->blobManualCtrlPM.fGrpCurr = pSingleRunInfo->fAllow_MaxChargeCurr;

				pSTBatt_Rough->blobManualCtrlPM.nAdjMode = blobManuCtl.nAdjMode;
				//交流调度充电功率是负的
				float fActv_Pwr = pSTBatt_Rough->blobManualCtrlPM.fGrpVolt*pSTBatt_Rough->blobManualCtrlPM.fGrpCurr*0.001;

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_4", LOG_TYPE_INFO,\
					"fGrpVolt[%.1f]fGrpCurr[%.1f]fActv_Pwr[%.1f]real fActv_Pwr[%.1f]", \
						pSTBatt_Rough->blobManualCtrlPM.fGrpVolt, pSTBatt_Rough->blobManualCtrlPM.fGrpCurr, \
					fActv_Pwr, blobManuCtl.fActv_Pwr);
				
				pSTBatt_Rough->blobManualCtrlPM.fActv_Pwr = -MIN(fActv_Pwr, ABS(blobManuCtl.fActv_Pwr)); //充电为负

				varV.blobValue = (BYTE *)&pSTBatt_Rough->blobManualCtrlPM;
				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_CHRG_ADJ, REASON_CHG_ALLOW);

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_4", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
			}
		}
		else if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
		{
			float fCurrOutDiff = fabs(pSingleRunInfo->fPackTotalCurr) - pSingleRunInfo->fAllow_MaxDisChargeCurr;
		
			if(fCurrOutDiff > 1.0) //Over 1.0A, Adjust Current
			{
				pSTBatt_Rough->blobManualCtrlPM.emState = 4; 				//4-Adjust Current When DisCharging
				pSTBatt_Rough->blobManualCtrlPM.nGrp = nBattIdx + 1; 		//Which Battery Group Index
				pSTBatt_Rough->blobManualCtrlPM.fGrpVolt = pSingleRunInfo->fPackTotalVolt;
				pSTBatt_Rough->blobManualCtrlPM.fGrpCurr = pSingleRunInfo->fAllow_MaxDisChargeCurr;

				pSTBatt_Rough->blobManualCtrlPM.nAdjMode = blobManuCtl.nAdjMode;
				float fActv_Pwr = pSTBatt_Rough->blobManualCtrlPM.fGrpVolt*pSTBatt_Rough->blobManualCtrlPM.fGrpCurr*0.001;
				pSTBatt_Rough->blobManualCtrlPM.fActv_Pwr = MIN(fActv_Pwr, ABS(blobManuCtl.fActv_Pwr)); 
				
				varV.blobValue = (BYTE *)&pSTBatt_Rough->blobManualCtrlPM;
				Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
				
				pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = tmNow;
				
				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_MANUALCTRL, CMD_MGMT_DISCHG_ADJ, REASON_DISCHG_ALLOW);

				LogOut4c(MAIN_MODULE_SAMPLER, "Batt_Manaul_5", LOG_TYPE_INFO,\
					"Idx[%d]Run[%d]Alm[%d]IPack[%.1f]Imax[%.1f]Irect[%.1f]Soc[%d]ProtSOC[%d]ACDC[%d]", \
					nBattIdx,pSingleRunInfo->emBattStatus_Run, pSingleRunInfo->emBattStatus_Alm, \
					pSingleRunInfo->fPackTotalCurr,pSingleRunInfo->fAllow_MaxDisChargeCurr,\
					pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr,pSingleRunInfo->nSOC,pSTBatt_Rough->nProtectSOC,\
					pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu);
			}
		} 
	}
	
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnOffGrid
*  CREATOR       :  
*  CREATED DATE  : 2023-04-11 11:13:53
*  DESCRIPTION   :  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
STBATT_STATEMACHINE_STATE OnOffGrid(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{	
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	double tmNow = GetCurrentTime();

	TRACE("nBattIdx=[%d]: fPackTotalCurr=%f\n", nBattIdx, pSingleRunInfo->fPackTotalCurr);

	if((pSingleRunInfo->bIsBatt_DisChargeForbidden == TRUE) ||
		(pSingleRunInfo->bIsBatt_ChargeForbidden == TRUE) ||
		(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
		(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
		(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	{
		//now wait charge to be ended
		if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
		{
			if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_AUTOOFFGRID, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
			
			if(pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].SysStatu.Byte0_Bit0to2 == 1) //1-Run
			{
				Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);

				Log_CtrlPcsEventStatus(pSTBatt_Rough, nBattIdx, STAT_MGMT_AUTOOFFGRID, CMD_MGMT_STOP, REASON_FAULT_ALARM);
			}
		}
	}
	
	if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, nBattIdx) == FALSE) //STS ON-Grid Status,Exit Off-Grid StateMachine.
    {
        TRACE("Exit: nBattIdx=[%d] emSTS_OnOffGridStatu=[%d]\n",\
			nBattIdx,  pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State);
        return STBATT_STATE_IDLE; 
    }
	
	if((pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC <= pSTBatt_Rough->nProtectSOC) && (pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE))
	{ //离网时电池SOC小于等于保护SOC时，停止逆变
		Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
	}
	else
	{
		OnHandle_PCSHotWaiting(pSTBatt_Rough, nBattIdx);
	}
	
    TRACE("Off-Grid: nBattIdx=[%d] emSTS_OnOffGridStatu=[%d]\n",\
		nBattIdx,  pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State);
	return STBATT_STATE_OFFGRID; 
}

 
 
 
 
 
 
 
 