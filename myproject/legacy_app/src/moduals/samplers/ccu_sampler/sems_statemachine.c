// #define _TRACE_PRINT_ // SEMS
#include "sems_sampler_main.h"

static void CtrlPcs_Manual(ROUGH_CCU_DATA* pCCU, int n);
static ACDC_WORKSTATU RequestACDC_WorkStatu(ROUGH_CCU_DATA* pCCU, int n);
static ACDC_WORKSTATU GetACDC_WorkingStatu(A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData);

static PcsSMState OnIdle(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnWaitCharge(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnCharging(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnWaitDischarge(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnDischarging(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnFaulted(ROUGH_CCU_DATA* pCCU, int n);
static PcsSMState OnOffGrid(ROUGH_CCU_DATA* pCCU, int n);

static BOOL JudgeIfNeedEnterOffGrid(ROUGH_CCU_DATA* pCCU, int n);

#define INIT_PCS_STATE_MACHINE(p, _name, _emState, _pfnOnAct) \
					(memcpy((p)->szStateNamePcs, (_name), sizeof((_name))), \
					(p)->emStatePcs = (_emState), \
					(p)->pfnActionPcs = (_pfnOnAct), \
					(p)++)

BOOL Initialize_SEMS_StateM(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pCCU->semsSMInfos, MAX_SEMS_CLIENTS_NUM);

	//Note: do NOT chage the order!!!/**/
	PcsSMItem *p = &pCCU->semsSMItems[0];
	INIT_PCS_STATE_MACHINE(p, "Idle",			PCS_STATE_IDLE,					OnIdle);
	INIT_PCS_STATE_MACHINE(p, "WaitCharge",		PCS_STATE_WAIT_CHARGE,			OnWaitCharge);
	INIT_PCS_STATE_MACHINE(p, "Charging",		PCS_STATE_IN_CHARGING,			OnCharging);
	INIT_PCS_STATE_MACHINE(p, "WaitDischarge",	PCS_STATE_WAIT_DISCHARGE,		OnWaitDischarge);
	INIT_PCS_STATE_MACHINE(p, "Discharging",	PCS_STATE_IN_DISCHARGING,		OnDischarging);
	INIT_PCS_STATE_MACHINE(p, "Faulted",		PCS_STATE_FAULTED,				OnFaulted);
	INIT_PCS_STATE_MACHINE(p, "OnOffGrid",      PCS_STATE_OFFGRID,				OnOffGrid);

	int n = 0;
	PcsSMInfo* pSMInfo;
	for(n = 0; n < pCCU->emsMgmtData.nNodeNum; n++)
	{
		pSMInfo = &pCCU->semsSMInfos[n];
		pSMInfo->pCurrSMItem = &pCCU->semsSMItems[PCS_STATE_IDLE];
		pSMInfo->pLastSMItem = pSMInfo->pCurrSMItem;
	}
	TRACED("Initialize_SEMS_StateM OK");
	return TRUE;
}

int Sample_SEMS_StateM(HANDLE hSamplerThis)
{
	TRACE(">>> Sample_SEMS_StateM");
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	int n = 0;
	//now run the state-machine
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 2000);
	if(nErr == ERR_MUTEX_OK)
	{
		if (pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE)
		{
			PcsSMState emState;
			PcsSMInfo* pSMInfo;
			for (n=0; n<pCCU->emsMgmtData.nNodeNum; n++)
			{
				if (s_MMClients[n].bNeedRegister)
				{
					TRACE("Sample_SEMS_StateM continue n:%d bNeedRegister", n);
					continue;
				}
				pSMInfo = &pCCU->semsSMInfos[n];
				emState = pSMInfo->pCurrSMItem->pfnActionPcs(pCCU, n);
				pSMInfo->pLastSMItem = pSMInfo->pCurrSMItem;
				pSMInfo->pCurrSMItem = &pCCU->semsSMItems[emState];
				// TRACED("Sample_SEMS_StateM AUTO_CTRL_PM_MODE n:%d Now:%s Last:%s",
				// 	n, pSMInfo->pCurrSMItem->szStateNamePcs, pSMInfo->pLastSMItem->szStateNamePcs);
				if(pSMInfo->pCurrSMItem != pSMInfo->pLastSMItem)
				{
					// TRACEW("Sample_SEMS_StateM AUTO_CTRL_PM_MODE Now:%s Last:%s", pSMInfo->pCurrSMItem->szStateNamePcs, pSMInfo->pLastSMItem->szStateNamePcs);
					pSMInfo->pCurrSMItem->tm_EnterPcs = GetCurrentTime();
				}
			}
		}
		else if (pCCU->emPMCtrlMode == MANUAL_CTRL_PM_MODE)
		{
			// TRACED("Sample_SEMS_StateM MANUAL_CTRL_PM_MODE");
			for (n=0; n<pCCU->emsMgmtData.nNodeNum; n++)
			{
				CtrlPcs_Manual(pCCU, n);
			}
		}
		
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	TRACE("Sample_SEMS_StateM OK");

	return ERR_SAMPLE_OK;
}

#define INIT_ONWORK_VAR(OnAction, printLog) \
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node =   &pCCU->emsMgmtData.calc_Node[n]; \
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[n]; \
	PcsSMInfo* pSMInfo =                         &pCCU->semsSMInfos[n]; \
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS =   &pBranchGrpData->b6LSYSLast_SEMS; \
	ACDC_WORKSTATU emACDC_WorkStatuNow =          GetACDC_WorkingStatu(pBranchGrpData); \
	ACDC_WORKSTATU emACDC_WorkStatuReq =          RequestACDC_WorkStatu(pCCU, n); \
	float fMaxCurrAbilitySum =                    pB6LSYSLast_SEMS->fRealTime_MaxCurrSum; \
	if (printLog) TRACEI("sems_statemachine %s n:%d emACDC_WorkStatuNow:%d req:%d [1:rect 2:invt] " \
		"UI:[%.1f %.1f] fMaxCurrAbilitySum=[%.1f]", OnAction, \
		n, emACDC_WorkStatuNow, emACDC_WorkStatuReq, \
		pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum, pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum, \
		fMaxCurrAbilitySum);

static void LogOut_WorkingStatus(ROUGH_CCU_DATA* pCCU, int n, const char* pszAction)
{
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[n];
	PcsSMInfo* pSMInfo =                         &pCCU->semsSMInfos[n];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS =   &pBranchGrpData->b6LSYSLast_SEMS;
	float fMaxCurrAbilitySum =                    pB6LSYSLast_SEMS->fRealTime_MaxCurrSum;
	TRACEI("%s:Idx[%d]Imax[%.1f]Pwrlast[%.1f]UIrect[%.1f %.1f]", pszAction,
		n, fMaxCurrAbilitySum, pSMInfo->fActv_PwrCtrl_Last, pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum, pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum);
	LOGFD( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
		"%s:Idx[%d]Imax[%.1f]Pwrlast[%.1f]UIrect[%.1f %.1f]", pszAction,
		n, fMaxCurrAbilitySum, pSMInfo->fActv_PwrCtrl_Last, pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum, pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum);
}

static ACDC_WORKSTATU RequestACDC_WorkStatu(ROUGH_CCU_DATA* pCCU, int n)
{
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node =   &pCCU->emsMgmtData.calc_Node[n];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS =   &pCCU->strLSYSInfo.branchNodeData[n].b6LSYSLast_SEMS;
	float fMaxCurrAbilitySum =                    pB6LSYSLast_SEMS->fRealTime_MaxCurrSum;

	if (JudgeIfNeedEnterOffGrid(pCCU, n))
	{
		return STATU_ACDC_IDLE;
	}

	ACDC_WORKSTATU emACDC_WorkStatuReq = MapBattWorkMode2ACDC_WorkStatu(pCalc_Node->emBatt_WorkMode);
	TRACED("RequestACDC_WorkStatu n:%d emACDC_WorkStatuReq:%d emBatt_WorkMode:%d 5:idle", n, emACDC_WorkStatuReq, pCalc_Node->emBatt_WorkMode);
	
	// if (ElapseMS(pB6LSYSLast_SEMS->llTimeMs) > MAX_MS_VALIDDATA_SEMS)
	// {
	// 	TRACED("RequestACDC_WorkStatu return STATU_ACDC_RECTIFY ElapseMS:%d", ElapseMS(pB6LSYSLast_SEMS->llTimeMs));
	// 	return STATU_ACDC_IDLE;
	// }
	if (emACDC_WorkStatuReq == STATU_ACDC_RECTIFY)
	{
		if (pB6LSYSLast_SEMS->emCCUStateSum != CCU_STATE_IDLE ||
			pB6LSYSLast_SEMS->emPMCtrlModeSum != AUTO_CTRL_PM_MODE ||
			pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum0 < MIN_PWR_PCS_OUT_ABL ||
			fMaxCurrAbilitySum < MIN_CURR_PCS_OUT_ABL
		)
		{
			TRACEW("RequestACDC_WorkStatu STATU_ACDC_RECTIFY return STATU_ACDC_IDLE n:%d emBatt_WorkMode:%d 5:idle emCCUStateSum:%d emPMCtrlModeSum:%d 0:auto 1:manual "
				"fMax_Chrg_PwrAbilitySum0:%.1f fMaxCurrAbilitySum:%.1f "
				"fACDC_Distri_4_BattChrg_Pwr:%.1f fAvg_ChrgLmtPower:%.1f %.1f nSOC:%d stop:%d",
				n, pCalc_Node->emBatt_WorkMode,
				pB6LSYSLast_SEMS->emCCUStateSum, pB6LSYSLast_SEMS->emPMCtrlModeSum,
				pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum0, fMaxCurrAbilitySum,
				pCalc_Node->fACDC_Distri_4_BattChrg_Pwr, pCalc_Node->fAvg_ChrgLmtPower, pCalc_Node->fACDC_Max_Ability_Pwr,
				pCalc_Node->nSOC, pCalc_Node->nStopSOCReq);
			return STATU_ACDC_IDLE;
		}
		TRACED("RequestACDC_WorkStatu return STATU_ACDC_RECTIFY");
		return STATU_ACDC_RECTIFY;
	}

	if (emACDC_WorkStatuReq == STATU_ACDC_INVERTER)
	{
		if (pB6LSYSLast_SEMS->emCCUStateSum != CCU_STATE_IDLE ||
			pB6LSYSLast_SEMS->emPMCtrlModeSum != AUTO_CTRL_PM_MODE ||
			pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum0 < MIN_PWR_PCS_OUT_ABL ||
			fMaxCurrAbilitySum < MIN_CURR_PCS_OUT_ABL
		)
		{
			TRACEW("RequestACDC_WorkStatu STATU_ACDC_INVERTER return STATU_ACDC_IDLE n:%d emBatt_WorkMode:%d 5:idle emCCUStateSum:%d emPMCtrlModeSum:%d "
				"fMax_DisChrg_PwrAbilitySum0:%.1f fMaxCurrAbilitySum:%.1f "
				"fBatt_Distri_4_ACDC_Pwr:%.1f fAvg_DisChrgingLmtPower:%.1f %.1f nSOC:%d stop:%d",
				n, pCalc_Node->emBatt_WorkMode,
				pB6LSYSLast_SEMS->emCCUStateSum, pB6LSYSLast_SEMS->emPMCtrlModeSum,
				pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum0, fMaxCurrAbilitySum,
				pCalc_Node->fBatt_Distri_4_ACDC_Pwr, pCalc_Node->fAvg_DisChrgingLmtPower, pCalc_Node->fACDC_Max_Ability_Pwr,
				pCalc_Node->nSOC, pCalc_Node->nStopSOCReq);
			return STATU_ACDC_IDLE;
		}
		TRACED("RequestACDC_WorkStatu return STATU_ACDC_INVERTER");
		return STATU_ACDC_INVERTER;
	}

	TRACED("RequestACDC_WorkStatu return STATU_ACDC_IDLE emACDC_WorkStatuReq:%d", emACDC_WorkStatuReq);
	return STATU_ACDC_IDLE;
}

static ACDC_WORKSTATU GetACDC_WorkingStatu(A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData)
{
	SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS = &pBranchGrpData->b6LSYSLast_SEMS;
	if (pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum > MIN_CURR_WORKING)
	{
		if (pB6LSYSLast_SEMS->emACDC_WorkStatuSum == STATU_ACDC_RECTIFY ||
			pB6LSYSLast_SEMS->emACDC_WorkStatuSum == STATU_ACDC_INVERTER)
		{
			return pB6LSYSLast_SEMS->emACDC_WorkStatuSum;
		}
	}

	return STATU_ACDC_IDLE;
}

static PcsSMState OnIdle(ROUGH_CCU_DATA* pCCU, int n)
{
	INIT_ONWORK_VAR("OnIdle", TRUE);
	UNUSED(pCalc_Node);
	
	if (pB6LSYSLast_SEMS->emCCUStateSum != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	// Handle_PCSHotWaiting(pCCU, n);
	if (JudgeIfNeedEnterOffGrid(pCCU, n))
	{
		TRACEW("OnIdle return PCS_STATE_OFFGRID n:%d emACDC_WorkStatuNow:%d req:%d [1:rect 2:invt] "
			"UI:[%.1f %.1f] fMaxCurrAbilitySum=[%.1f]",
			n, emACDC_WorkStatuNow, emACDC_WorkStatuReq,
			pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum, pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum,
			fMaxCurrAbilitySum);
		return PCS_STATE_OFFGRID;
	}

	switch (emACDC_WorkStatuReq)
	{
		case STATU_ACDC_IDLE:
		{
			if (emACDC_WorkStatuNow != STATU_ACDC_IDLE ||
				pCCU->emsMgmtData.byWorkAsap != pB6LSYSLast_SEMS->byWorkAsapSum)
			{
				TRACEE("OnIdle Send_6330UctlWorkPwrAuto STATU_ACDC_IDLE emACDC_WorkStatuNow != STATU_ACDC_IDLE %d || "
					"byWorkAsapData!=pB6LSYSLast_SEMS->byWorkAsap my:%d client:%d n:%d",
					emACDC_WorkStatuNow, pCCU->emsMgmtData.byWorkAsap, pB6LSYSLast_SEMS->byWorkAsapSum, n);
				Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_IDLE, n, MAX_INTV_MS_CHGWORK);
			}
			else
			{
				Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_IDLE, n, MAX_INTV_MS_CONTINUE_WORK);
			}
		}
			return PCS_STATE_IDLE;
		case STATU_ACDC_RECTIFY:
		{
			pSMInfo->tm_LastStartWork = GetCurrentTime();
			TRACEI("OnIdle Send_6330UctlWorkPwrAuto STATU_ACDC_RECTIFY n=[%d]", n);
			Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_RECTIFY, n, MAX_INTV_MS_CHGWORK);
			LogOut_WorkingStatus(pCCU, n, "Wait Chrg");
		}
			return PCS_STATE_WAIT_CHARGE;
		case STATU_ACDC_INVERTER:
		{
			pSMInfo->tm_LastStartWork = GetCurrentTime();
			TRACEI("OnIdle Send_6330UctlWorkPwrAuto STATU_ACDC_INVERTER n=[%d]", n);
			Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_INVERTER, n, MAX_INTV_MS_CHGWORK);
			LogOut_WorkingStatus(pCCU, n, "Wait Dischrg");
		}
			return PCS_STATE_WAIT_DISCHARGE;

		default:
			break;
	}

	PcsSMState retState;
	switch (emACDC_WorkStatuNow)
	{
		case STATU_ACDC_RECTIFY:
		{
			TRACED("OnIdle now:STATU_ACDC_RECTIFY return PCS_STATE_IN_CHARGING n:%d", n);
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
		case STATU_ACDC_INVERTER:
		{
			TRACED("OnIdle now:STATU_ACDC_INVERTER return PCS_STATE_IN_DISCHARGING n:%d", n);
			retState = PCS_STATE_IN_DISCHARGING;
		}
			break;
		default:
		{
			retState = PCS_STATE_IDLE;
		}
			break;
	}

	return retState;
}

static PcsSMState OnWaitCharge(ROUGH_CCU_DATA* pCCU, int n)
{// idle->chrg:28S dischg->chrg:5S
	INIT_ONWORK_VAR("OnWaitCharge", TRUE);
	UNUSED(pCalc_Node);
	if (emACDC_WorkStatuReq != STATU_ACDC_RECTIFY)
	{
		TRACEE("OnWaitCharge return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_RECTIFY");
		return PCS_STATE_IDLE;
	}

	if (emACDC_WorkStatuNow == STATU_ACDC_RECTIFY)
	{
		LogOut_WorkingStatus(pCCU, n, "Start Chrg");
		return PCS_STATE_IN_CHARGING;
	}

	if (ElapseMS(pSMInfo->tm_LastStartWork) >= MAX_INTV_MS_WAIT_PCS_WORK)//PCS Need Over 30S Wait Power Output
	{
		//means start FAIL
		LogOut_WorkingStatus(pCCU, n, "Start Chrg Fail");
		return PCS_STATE_IDLE;
	}
	Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_RECTIFY, n, MAX_INTV_MS_CONTINUE_WORK);

	return PCS_STATE_WAIT_CHARGE;
}

static PcsSMState OnCharging(ROUGH_CCU_DATA* pCCU, int n)
{
	INIT_ONWORK_VAR("OnCharging", TRUE);
	UNUSED(pCalc_Node);
	UNUSED(pSMInfo);
	if (emACDC_WorkStatuReq != STATU_ACDC_RECTIFY)
	{
		TRACEE("OnCharging return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_RECTIFY");
		LogOut_WorkingStatus(pCCU, n, "Stop Chrg");
		return PCS_STATE_IDLE;
	}

	PcsSMState retState;
	switch (emACDC_WorkStatuNow)
	{
		case STATU_ACDC_IDLE:
		{
			TRACED("OnCharging return PCS_STATE_IDLE");
			retState = PCS_STATE_IDLE;
		}
			break;
		case STATU_ACDC_INVERTER:
		{
			TRACED("OnCharging return PCS_STATE_IN_DISCHARGING");
			retState = PCS_STATE_IN_DISCHARGING;
		}
			break;
		default:
		{
			Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_RECTIFY, n, MAX_INTV_MS_CONTINUE_WORK);
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
	}

	return retState; 
}

static PcsSMState OnWaitDischarge(ROUGH_CCU_DATA* pCCU, int n)
{// 26S
	INIT_ONWORK_VAR("OnWaitDischarge", TRUE);
	UNUSED(pCalc_Node);
	if (emACDC_WorkStatuReq != STATU_ACDC_INVERTER)
	{
		TRACEE("OnWaitDischarge return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_INVERTER");
		return PCS_STATE_IDLE;
	}
	
	if (emACDC_WorkStatuNow == STATU_ACDC_INVERTER)
	{
		LogOut_WorkingStatus(pCCU, n, "Start Dischrg");
		return PCS_STATE_IN_DISCHARGING;
	}

	if (ElapseMS(pSMInfo->tm_LastStartWork) >= MAX_INTV_MS_WAIT_PCS_WORK)//PCS Need Over 30S Wait Power Output
	{
		//means start FAIL
		LogOut_WorkingStatus(pCCU, n, "Start Dischrg Fail");
		return PCS_STATE_IDLE;
	}
	
	Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_INVERTER, n, MAX_INTV_MS_CONTINUE_WORK);
	return PCS_STATE_WAIT_DISCHARGE;
}

static PcsSMState OnDischarging(ROUGH_CCU_DATA* pCCU, int n)
{
	INIT_ONWORK_VAR("OnDischarging", TRUE);
	UNUSED(pCalc_Node);
	UNUSED(pSMInfo);
	if (emACDC_WorkStatuReq != STATU_ACDC_INVERTER)
	{
		TRACEE("OnDischarging return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_INVERTER");
		LogOut_WorkingStatus(pCCU, n, "Stop Dischrg");
		return PCS_STATE_IDLE;
	}

	PcsSMState retState;
	switch (emACDC_WorkStatuNow)
	{
		case STATU_ACDC_IDLE:
		{
			TRACED("OnDischarging return PCS_STATE_IDLE");
			retState = PCS_STATE_IDLE;
		}
			break;
		case STATU_ACDC_RECTIFY:
		{
			TRACED("OnDischarging return PCS_STATE_IN_CHARGING");
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
		default:
		{
			Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_INVERTER, n, MAX_INTV_MS_CONTINUE_WORK);
			retState = PCS_STATE_IN_DISCHARGING;
		}
		break;
	}

	return retState; 
}

static PcsSMState OnFaulted(ROUGH_CCU_DATA* pCCU, int n)
{
	INIT_ONWORK_VAR("OnFaulted", TRUE);
	UNUSED(pCalc_Node);
	UNUSED(pSMInfo);
	if (pB6LSYSLast_SEMS->emCCUStateSum == CCU_STATE_IDLE)
	{
		TRACED("OnFaulted return PCS_STATE_IDLE");
		return PCS_STATE_IDLE; 
	}
	
	if(emACDC_WorkStatuNow != STATU_ACDC_IDLE)
	{
		TRACEW("OnFaulted Send_6330UctlWorkPwrAuto STATU_ACDC_IDLE");
		Send_6330UctlWorkPwrAuto(pCCU, STATU_ACDC_IDLE, n, MAX_INTV_MS_CHGWORK);
	}
	return PCS_STATE_FAULTED;
}

static PcsSMState OnOffGrid(ROUGH_CCU_DATA* pCCU, int n)
{
	// A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
	// double tmNow = GetCurrentTime();

	// TRACE("n=[%d]: fPackTotalCurr=%f\n", n, pSingleRunInfo->fPackTotalCurr);

	// if((pSingleRunInfo->bIsBatt_DisChargeForbidden == TRUE) ||
	// 	(pSingleRunInfo->bIsBatt_ChargeForbidden == TRUE) ||
	// 	(pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) ||
	// 	(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) ||
	// 	(pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
	// {
	// 	//now wait charge to be ended
	// 	if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[n]) >= URGET_STBATT_PUSH2CCU_PERIOD)
	// 	{
	// 		if(pSTBatt_Rough->branchGrpData[n].emACDC_WorkStatu != STATU_ACDC_IDLE)
	// 		{
	// 			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, n);

	// 			Log_CtrlPcsEventStatus(pSTBatt_Rough, n, STAT_MGMT_AUTOOFFGRID, CMD_MGMT_STOP, REASON_FAULT_ALARM);
	// 		}
			
	// 		if(pCCU->strLSYSInfo.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 1) //1-Run
	// 		{
	// 			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, n);

	// 			Log_CtrlPcsEventStatus(pSTBatt_Rough, n, STAT_MGMT_AUTOOFFGRID, CMD_MGMT_STOP, REASON_FAULT_ALARM);
	// 		}
	// 	}
	// }
	
	// if(JudgeIfNeedEnterOffGrid(pSTBatt_Rough, n) == FALSE) //STS ON-Grid Status,Exit Off-Grid StateMachine.
    // {
    //     TRACE("Exit: n=[%d] emSTS_OnOffGridStatu=[%d]\n",
	// 		n,  pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);
    //     return STBATT_STATE_IDLE; 
    // }
	
	// Handle_PCSHotWaiting(pSTBatt_Rough, n);

    // TRACE("Off-Grid: n=[%d] emSTS_OnOffGridStatu=[%d]\n",\
	// 	n,  pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);
	return PCS_STATE_OFFGRID;
}

static void CtrlPcs_Manual(ROUGH_CCU_DATA* pCCU, int n)
{
	return;
// 	// SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
// 	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	

// 	// PcsSMState retState;
// 	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[n];
// 	PcsSMInfo* pSMInfo =                      &pCCU->semsSMInfos[n];
// 	//A_BLOB_CTRL_SEMS_CLIENT* pCtrlReq = &pSMInfo->ctrlSemsClientReq;
// 	A_BLOB_MANUALCTRL_RECT_CMD* ctrlRectReq_Manual = &pSMInfo->ctrlRectReq_Manual;
// 	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
// 	float fMaxCurrAbilitySum = pGrpRectData->fRealTime_MaxCurr[n];

// 	if (ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs) < MAX_INTV_MS_CHGWORK)
// 	{
// 		// TRACEW("CtrlPcs_Manual too short ElapseMS:%d", ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs));
// 		return;
// 	}

// 	// 测试按设置值直接发送
// 	{
// 		double tmNow = GetCurrentTime();
// 		A_BLOB_MANUALCTRL_RECT_CMD ctrlRectSend_Manual = pSMInfo->ctrlRectReq_Manual;
// 		VAR_VALUE 	varV;
// 		varV.blobValue = (BYTE *)(&ctrlRectSend_Manual);
// 		ctrlRectSend_Manual.nGrp = (UINT)n;
		
// 		pSMInfo->tm_LastSendCmd_2Pcs = tmNow;
// 		Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
// 						200, 217, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT_SCADA);
// 		return;
// 	}

// 	emACDC_WorkStatuNow = STATU_ACDC_IDLE;
// 	if (pBranchGrpData->fACDC_RealTime_Curr > MIN_CURR_WORKING)
// 	{
// 		if (pBranchGrpData->emACDC_WorkStatuSum == STATU_ACDC_RECTIFY ||
// 			pBranchGrpData->emACDC_WorkStatuSum == STATU_ACDC_INVERTER)
// 		{
// 			emACDC_WorkStatuNow = pBranchGrpData->emACDC_WorkStatuSum;
// 		}
// 	}
	
// 	// system abnormal stop
// 	VAR_VALUE 	varV;
// 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 1, 12, &varV, 4);
// 	BOOL bSysOverTemp = (varV.emValue == 1) ? TRUE : FALSE;
// 	TRACED("CtrlPcs_Manual bSysOverTemp:%d emCCUState:%d emACDC_WorkStatuNow:%d fACDC_Max_Output_Pwr_Ability:%.1f fMaxCurrAbilitySum:%.1f",
// 		bSysOverTemp, pB6LSYSLast_SEMS->emCCUStateSum, emACDC_WorkStatuNow, pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrAbilitySum);
// 	if (bSysOverTemp || (pB6LSYSLast_SEMS->emCCUStateSum!=CCU_STATE_IDLE) ||
// 		pBranchGrpData->fACDC_Max_Output_Pwr_Ability < MIN_PWR_PCS_OUT_ABL ||
// 		fMaxCurrAbilitySum < MIN_CURR_PCS_OUT_ABL)
// 	{
// 		if(emACDC_WorkStatuNow != STATU_ACDC_IDLE)
// 		{
// 			TRACEW("CtrlPcs_Manual return Send_Ctrl_PCS_Manual STAT_MANUAL_STOP");
// 			Send_Ctrl_PCS_Manual(pCCU, STAT_MANUAL_STOP, n);
// 			LogOut(MAIN_MODULE_SAMPLER, "Manual Stop", LOG_TYPE_INFO,
// 				"Idx[%d]SysOverTemp[%d]ccuStatu[%d]"
// 				"Pmax[%.1f]Imax[%.1f]Irect[%.1f]Vrect[%.1f]ACDC[%d]",
// 				n, bSysOverTemp, pB6LSYSLast_SEMS->emCCUStateSum,
// 				pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrAbilitySum,
// 				pBranchGrpData->fACDC_RealTime_Curr, pBranchGrpData->fACDC_RealTime_Volt,
// 				emACDC_WorkStatuNow);
// 			return;
// 		}
// 	}

// 	// ACDC_WorkStatu
// 	emACDC_WorkStatuReq = STATU_ACDC_IDLE;
// 	if (ctrlRectReq_Manual->emState==STAT_MANUAL_START_CHG ||
// 		ctrlRectReq_Manual->emState==STAT_MANUAL_ADJ_CHG)
// 	{
// 		TRACED("CtrlPcs_Manual STATU_ACDC_RECTIFY emState:%d", ctrlRectReq_Manual->emState);
// 		emACDC_WorkStatuReq = STATU_ACDC_RECTIFY;
// 	}
// 	else if (ctrlRectReq_Manual->emState==STAT_MANUAL_START_DISCHG ||
// 		ctrlRectReq_Manual->emState==STAT_MANUAL_ADJ_DISCHG)
// 	{
// 		TRACED("CtrlPcs_Manual STATU_ACDC_INVERTER emState:%d", ctrlRectReq_Manual->emState);
// 		emACDC_WorkStatuReq = STATU_ACDC_INVERTER;
// 	}

// 	// 工作中的WorkStatu不是请求的状态 stop
// 	if (emACDC_WorkStatuNow != STATU_ACDC_IDLE &&
// 		emACDC_WorkStatuNow != emACDC_WorkStatuReq)
// 	{
// 		TRACEE("CtrlPcs_Manual return Send_Ctrl_PCS_Manual STAT_MANUAL_STOP emACDC_WorkStatuNow!=emACDC_WorkStatuReq %d %d",
// 			emACDC_WorkStatuNow, emACDC_WorkStatuReq);
// 		Send_Ctrl_PCS_Manual(pCCU, STAT_MANUAL_STOP, n);
// 		LogOut(MAIN_MODULE_SAMPLER, "Manual Stop", LOG_TYPE_INFO,
// 			"Idx[%d]WorkStatuNow[%d]WorkStatuReq[%d]"
// 			"Pmax[%.1f]Imax[%.1f]Irect[%.1f]Vrect[%.1f]",
// 			n, emACDC_WorkStatuNow, emACDC_WorkStatuReq,
// 			pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrAbilitySum,
// 			pBranchGrpData->fACDC_RealTime_Curr, pBranchGrpData->fACDC_RealTime_Volt);
// 		return;
// 	}

// 	// 正常，处理请求
// 	Send_Ctrl_PCS_Manual(pCCU, ctrlRectReq_Manual->emState, n);
}

static BOOL JudgeIfNeedEnterOffGrid(ROUGH_CCU_DATA* pCCU, int n)
{
    BOOL bRet = FALSE;

	//if(pSTBatt_Rough->emSTSAutoManual == MANUAL_ONOFFGRID_MODE)
	//{
	//	printf("emSTSAutoManual=[%d]\n", pSTBatt_Rough->emSTSAutoManual);
	//	return bRet;
	//}
	

    // if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STATE_STS_OFFGRID)
    // {
	// 	TRACE("emSTS_OnOffGridStatu=%d\n",\
    //     	pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);
    //     return TRUE;
    // }

    return bRet;
}
