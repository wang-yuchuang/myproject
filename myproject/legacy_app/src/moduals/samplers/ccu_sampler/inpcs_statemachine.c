// #define _TRACE_PRINT_ // PSC
#include "inpcs_sampler_main.h"

#define MAX_INTV_MS_CHGWORK         (1000)
#define MAX_INTV_MS_CONTINUE        (10000)
#define MAX_INTV_MS_WAIT_PCS_WORK   (90000)
#define MAX_INTV_MS_IDLE            (10000)
#define MAX_VOLT_PCS_LMT_REQ        (1550.0f) // 限制请求PCS直流侧输出最大电压 1000V-1550V 275kw
#define MAX_CURR_PCS_LMT_REQ        (275.0f)  // 限制请求PCS直流侧输出最大电流
#define MIN_CURR_WORKING            (0.5f)    // 最小充放电电流，判断PCS是否正在充放电
#define MIN_DIFF_VALUE              (2.0f)
#define MIN_CURR_PCS_OUT_ABL        (5.0f)    // PCS最小输出电流能力阈值，判断PCS是否正常
#define MIN_PWR_PCS_OUT_ABL         (5.0f)    // PCS最小输出功率能力阈值，判断PCS是否正常

static void CtrlPcs_Manual(ROUGH_CCU_DATA* pCCU, int idx);
static ACDC_WORKSTATU RequestACDC_WorkStatu(ROUGH_CCU_DATA* pCCU, int idx);
static ACDC_WORKSTATU GetACDC_WorkingStatu(A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData);

static PcsSMState OnIdlePcs(ROUGH_CCU_DATA* pCCU, int idx);
static PcsSMState OnWaitChargePcs(ROUGH_CCU_DATA* pCCU, int idx);
static PcsSMState OnChargingPcs(ROUGH_CCU_DATA* pCCU, int idx);
static PcsSMState OnWaitDischargePcs(ROUGH_CCU_DATA* pCCU, int idx);
static PcsSMState OnDischargingPcs(ROUGH_CCU_DATA* pCCU, int idx);
static PcsSMState OnFaultedPcs(ROUGH_CCU_DATA* pCCU, int idx);

#define INIT_PCS_STATE_MACHINE(p, _name, _emState, _pfnOnAct) \
					(memcpy((p)->szStateNamePcs, (_name), sizeof((_name))), \
					(p)->emStatePcs = (_emState), \
					(p)->pfnActionPcs = (_pfnOnAct), \
					(p)++)

BOOL Initialize_INPCS_StateM(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;

	int n = 0;
	ZERO_POBJS(pCCU->pcsSMInfos, MAX_SCADA_PCS_NUM);
	for (n=0; n<MAX_SCADA_PCS_NUM; n++)
	{
		pCCU->pcsSMInfos[n].ctrlRectReq.emCtrl_Type = GRP_ACDC_STOP;
	}
	//Note: do NOT chage the order!!!/**/
	PcsSMItem *p = &pCCU->pcsSMItems[0];
	INIT_PCS_STATE_MACHINE(p, "Idle",			PCS_STATE_IDLE,					OnIdlePcs);
	INIT_PCS_STATE_MACHINE(p, "WaitCharge",		PCS_STATE_WAIT_CHARGE,			OnWaitChargePcs);
	INIT_PCS_STATE_MACHINE(p, "Charging",		PCS_STATE_IN_CHARGING,			OnChargingPcs);
	INIT_PCS_STATE_MACHINE(p, "WaitDischarge",	PCS_STATE_WAIT_DISCHARGE,		OnWaitDischargePcs);
	INIT_PCS_STATE_MACHINE(p, "Discharging",	PCS_STATE_IN_DISCHARGING,		OnDischargingPcs);
	INIT_PCS_STATE_MACHINE(p, "Faulted",		PCS_STATE_FAULTED,				OnFaultedPcs);

	PcsSMInfo* pPcsSMInfo;
	for(n = 0; n < MAX_SCADA_PCS_NUM; n++)
	{
		pPcsSMInfo = &pCCU->pcsSMInfos[n];
		pPcsSMInfo->pCurrSMItem = &pCCU->pcsSMItems[PCS_STATE_IDLE];
		pPcsSMInfo->pLastSMItem = pPcsSMInfo->pCurrSMItem;
	}
	
	TRACED("Initialize_INPCS_StateM OK");
	return TRUE;
}

int Sample_INPCS_StateM(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	int idx = 0;
	//now run the state-machine
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		if (pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE)
		{
			PcsSMState emState;
			PcsSMInfo* pPcsSMInfo;
			for (idx=0; idx<MAX_SCADA_PCS_NUM; idx++)
			{
				pPcsSMInfo = &pCCU->pcsSMInfos[idx];
				emState = pPcsSMInfo->pCurrSMItem->pfnActionPcs(pCCU, idx);
				pPcsSMInfo->pLastSMItem = pPcsSMInfo->pCurrSMItem;
				pPcsSMInfo->pCurrSMItem = &pCCU->pcsSMItems[emState];
				TRACED("Sample_INPCS_StateM AUTO_CTRL_PM_MODE idx:%d Now:%s Last:%s",
					idx, pPcsSMInfo->pCurrSMItem->szStateNamePcs, pPcsSMInfo->pLastSMItem->szStateNamePcs);
				if(pPcsSMInfo->pCurrSMItem != pPcsSMInfo->pLastSMItem)
				{
					TRACEW("Sample_INPCS_StateM AUTO_CTRL_PM_MODE Now:%s Last:%s", pPcsSMInfo->pCurrSMItem->szStateNamePcs, pPcsSMInfo->pLastSMItem->szStateNamePcs);
					pPcsSMInfo->pCurrSMItem->tm_EnterPcs = GetCurrentTime();
				}
			}
		}
		else if (pCCU->emPMCtrlMode == MANUAL_CTRL_PM_MODE)
		{
			// TRACED("Sample_INPCS_StateM MANUAL_CTRL_PM_MODE");
			for (idx=0; idx<MAX_SCADA_PCS_NUM; idx++)
			{
				CtrlPcs_Manual(pCCU, idx);
			}
		}
		
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	// TRACED("Sample_INPCS_StateM OK");

	return ERR_SAMPLE_OK;
}

static BOOL Send_Ctrl_PCS(ROUGH_CCU_DATA* pCCU, GRP_RECT_CTRL_CMDTYPE emCtrlSend, int idx, int msElapse)
{
	double tmNow = GetCurrentTime();
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->ctrlRectSend = pPcsSMInfo->ctrlRectReq;
	A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectSend = &pPcsSMInfo->ctrlRectSend;

	VAR_VALUE 	varV;
	varV.blobValue = (BYTE *)(&pPcsSMInfo->ctrlRectSend);
	pCtrlRectSend->emCtrl_Type = emCtrlSend;
	pCtrlRectSend->nGrp = idx;

	{// unit test 自动模式测试按设置值直接发送
		if (emCtrlSend <= GRP_ACDC_STOP && (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) > 10000))
		{
			//ACDC Running Rectfier Mode
			if(emCtrlSend == GRP_ACDC_RECTIFY_START || emCtrlSend == GRP_ACDC_RECTIFY_ADJ_CURR)
			{
				pCtrlRectSend->fGrpVoltLmt = MIN(pCtrlRectReq->fGrpVoltLmt, MAX_VOLT_PCS_LMT_REQ);
				// unit test
				// pCtrlRectSend->fGrpVoltLmt = 861.6f;
				// pCtrlRectSend->fGrpCurrLmt = 6.2f;
				TRACED("Send_Ctrl_PCS Rectify idx:%d emCtrlSend:%d %d fGrpVoltLmtSend:%.1f %.1f fGrpCurrLmtSend:%.1f %.1f",
					idx, emCtrlSend, pCtrlRectReq->emCtrl_Type, pCtrlRectSend->fGrpVoltLmt, pCtrlRectReq->fGrpVoltLmt, pCtrlRectSend->fGrpCurrLmt, pCtrlRectReq->fGrpCurrLmt);
			}
			else if(emCtrlSend == GRP_ACDC_INVERT_START || emCtrlSend == GRP_ACDC_INVERT_PENDING)
			{
				pCtrlRectSend->fGrpVoltLmt = MIN(pCtrlRectReq->fGrpVoltLmt, MAX_VOLT_PCS_LMT_REQ);
				pCtrlRectSend->fGrpCurrLmt = MIN(pCtrlRectReq->fGrpCurrLmt, fMaxCurrPcs);
				TRACED("Send_Ctrl_PCS Invert idx:%d emCtrlSend:%d %d fGrpVoltLmtSend:%.1f %.1f fGrpCurrLmtSend:%.1f %.1f",
					idx, emCtrlSend, pCtrlRectReq->emCtrl_Type, pCtrlRectSend->fGrpVoltLmt, pCtrlRectReq->fGrpVoltLmt, pCtrlRectSend->fGrpCurrLmt, pCtrlRectReq->fGrpCurrLmt);
			}
			else if(emCtrlSend == GRP_ACDC_STOP)
			{
				pCtrlRectSend->fGrpVoltLmt = MIN(pCtrlRectReq->fGrpVoltLmt, MAX_VOLT_PCS_LMT_REQ);
				pCtrlRectSend->fGrpCurrLmt = 0.0;
				pPcsSMInfo->tm_LastStopWork = tmNow;
			}
			// pPcsSMInfo->emLastCtrlCmdPcs      = pCtrlRectSend->emCtrl_Type;
			pPcsSMInfo->tm_LastSendCmd_2Pcs = tmNow;
			Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
				200, 232, (void*)varV.blobValue, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD), BLOB_TYPE_RECTGRP_AUTOCTRLCMD);
		}
		return TRUE;
	}

	/*// TRACED("idx=[%d] cmdType=[%d, %.1f, %.1f] \
	// 	fAllow_LmtChargeVolt=%.1f fAllow_MaxChargeCurr=%.1f  \
	// 	fLimitCurrent=%.1f fACDC_Distri_4_BattChrg_Pwr =%.1f \
	// 	fAllow_MaxDisChargeCurr=%.1f fBatt_Distri_4_ACDC_Pwr=[%.1f]to CCU\n", \
	// 		idx, 	pCtrlRectReq->emCtrl_Type,\
	// 		pCtrlRectReq->fGrpVoltLmt, pCtrlRectReq->fGrpCurrLmt,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_LmtChargeVolt,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_MaxChargeCurr, fLimitCurrent,\
	// 		pCCU->calcNodeData[idx].fACDC_Distri_4_BattChrg_Pwr,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_MaxDisChargeCurr,\
	// 		pCCU->calcNodeData[idx].fBatt_Distri_4_ACDC_Pwr);*/

	float fVoltReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqVolt_2Pcs - pCtrlRectReq->fGrpVoltLmt);
	float fVoltOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Volt - pCtrlRectReq->fGrpVoltLmt);
	float fCurrReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqCurr_2Pcs - pCtrlRectReq->fGrpCurrLmt);
	float fCurrOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Curr - pCtrlRectReq->fGrpCurrLmt);
	// TRACED("Send_Ctrl_PCS idx:%d fVoltReqDiff:%.1f fVoltOutDiff:%.1f fCurrReqDiff:%.1f fCurrOutDiff:%.1f "
	// 	"OutUI:%.1f %.1f ReqUI:%.1f %.1f fLastSendReqUI:%.1f %.1f fMaxCurrPcs:%.1f",
	// 	idx, fVoltReqDiff, fVoltOutDiff, fCurrReqDiff, fCurrOutDiff,
	// 	pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
	// 	pCtrlRectReq->fGrpVoltLmt, pCtrlRectReq->fGrpCurrLmt,
	// 	pPcsSMInfo->fLastSendReqVolt_2Pcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, fMaxCurrPcs);
	// if ((fVoltReqDiff>MIN_DIFF_VALUE) || (fVoltOutDiff>MIN_DIFF_VALUE) ||
	// 	(fCurrReqDiff>MIN_DIFF_VALUE) || (fCurrOutDiff>MIN_DIFF_VALUE))
	if ((fVoltReqDiff>MIN_DIFF_VALUE) || (fCurrReqDiff>MIN_DIFF_VALUE) ||
		(ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= msElapse))
	{
		TRACEI("Send_Ctrl_PCS Push_BlobValue idx:%d WorkStatuReq:%d %d[1:rect 2:invt] "
			"fVoltReqDiff:%.1f fVoltOutDiff:%.1f fCurrReqDiff:%.1f fCurrOutDiff:%.1f "
			"OutUI:%.1f %.1f ReqUI:%.1f %.1f fLastSendReqUI:%.1f %.1f fMaxCurrPcs:%.1f ElapseMS:%d %d",
			idx, RequestACDC_WorkStatu(pCCU, idx), GetACDC_WorkingStatu(pBranchGrpData), fVoltReqDiff, fVoltOutDiff, fCurrReqDiff, fCurrOutDiff,
			pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
			pCtrlRectReq->fGrpVoltLmt, pCtrlRectReq->fGrpCurrLmt,
			pPcsSMInfo->fLastSendReqVolt_2Pcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, fMaxCurrPcs,
			ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs), msElapse);
			
		pPcsSMInfo->fLastSendReqVolt_2Pcs = pCtrlRectSend->fGrpVoltLmt;
		pPcsSMInfo->fLastSendReqCurr_2Pcs = pCtrlRectSend->fGrpCurrLmt;
		pPcsSMInfo->tm_LastSendCmd_2Pcs = tmNow;
		
		BYTE byData[16]={'\0'};
		memset(byData, 0, 16);
		int nSendLen = 0;
		if(pCtrlRectSend->emCtrl_Type <= GRP_ACDC_STOP)
		{
			byData[0] = 0x31 + pCtrlRectSend->nGrp;
			nSendLen = 6;
			if(pCtrlRectSend->emCtrl_Type == GRP_ACDC_STOP)
			{
				byData[1] = 0x00;
			}
			else if(pCtrlRectSend->emCtrl_Type == GRP_ACDC_RECTIFY_START)
			{
				byData[1] = 0x01;
			}
			else if(pCtrlRectSend->emCtrl_Type == GRP_ACDC_RECTIFY_ADJ_CURR)
			{
				byData[1] = 0x02;
			}
			else if(pCtrlRectSend->emCtrl_Type == GRP_ACDC_INVERT_START)
			{
				byData[1] = 0x03;
			}
			else if(pCtrlRectSend->emCtrl_Type == GRP_ACDC_INVERT_PENDING)
			{
				byData[1] = 0x04;
			}
			TRACE("Send_Ctrl_PCS emCtrl_Type=%d ,nVoltReq=%.1f, nCurrReq=%.1f\n",\
				pCtrlRectSend->emCtrl_Type,\
				pCtrlRectSend->fGrpVoltLmt,\
				pCtrlRectSend->fGrpCurrLmt);
			
			byData[2] = (BYTE)((UINT)(pCtrlRectSend->fGrpVoltLmt*10) >> 8);
			byData[3] = (BYTE)(pCtrlRectSend->fGrpVoltLmt*10);
			byData[4] = (BYTE)((UINT)(pCtrlRectSend->fGrpCurrLmt*10) >> 8);
			byData[5] = (BYTE)(pCtrlRectSend->fGrpCurrLmt*10);
		}
		
		TRACEI("Send_Ctrl_PCS:%d(0:rect 2:invt 4:stop) nGrp:%d UI:%.1f %.1f byData<%s>",
			pCtrlRectSend->emCtrl_Type, pCtrlRectSend->nGrp,
			pCtrlRectSend->fGrpVoltLmt,
			pCtrlRectSend->fGrpCurrLmt, printableBytes(byData, nSendLen));
		//2020-01-04 Do Not Wait Adjust Current Cmd
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 0) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
	
		// Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
		// 	200, 232, (void*)varV.blobValue, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD), BLOB_TYPE_RECTGRP_AUTOCTRLCMD);
	}
	
	return TRUE;
}

static ACDC_WORKSTATU RequestACDC_WorkStatu(ROUGH_CCU_DATA* pCCU, int idx)
{
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx]; // LSYS
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData; // RTIF
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];

	{// unit test 自动模式测试按设置值直接发送
	if (pBranchGrpData->fACDC_Max_Output_Pwr_Ability < MIN_PWR_PCS_OUT_ABL)
	{
		TRACEW("RequestACDC_WorkStatu return STATU_ACDC_IDLE fACDC_Max_Output_Pwr_Ability:%.1f", pBranchGrpData->fACDC_Max_Output_Pwr_Ability);
		return STATU_ACDC_IDLE;
	}
	if (fMaxCurrPcs < MIN_CURR_PCS_OUT_ABL)
	{
		TRACEW("RequestACDC_WorkStatu return STATU_ACDC_IDLE fMaxCurrPcs:%.1f", fMaxCurrPcs);
		return STATU_ACDC_IDLE;
	}
	}

	if (pCtrlRectReq->emCtrl_Type == GRP_ACDC_RECTIFY_START || pCtrlRectReq->emCtrl_Type==GRP_ACDC_RECTIFY_ADJ_CURR)
	{
		TRACED("RequestACDC_WorkStatu return STATU_ACDC_RECTIFY emCtrl_Type:%d", pCtrlRectReq->emCtrl_Type);
		return STATU_ACDC_RECTIFY;
	}

	if (pCtrlRectReq->emCtrl_Type == GRP_ACDC_INVERT_START || pCtrlRectReq->emCtrl_Type==GRP_ACDC_INVERT_PENDING)
	{
		TRACED("RequestACDC_WorkStatu return STATU_ACDC_INVERTER emCtrl_Type:%d", pCtrlRectReq->emCtrl_Type);
		return STATU_ACDC_INVERTER;
	}

	return STATU_ACDC_IDLE;
}

static ACDC_WORKSTATU GetACDC_WorkingStatu(A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData)
{
	if (pBranchGrpData->fACDC_RealTime_Curr > MIN_CURR_WORKING)
	{
		if (pBranchGrpData->emACDC_WorkStatu == STATU_ACDC_RECTIFY ||
			pBranchGrpData->emACDC_WorkStatu == STATU_ACDC_INVERTER)
		{
			return pBranchGrpData->emACDC_WorkStatu;
		}
	}

	return STATU_ACDC_IDLE;
}

static PcsSMState OnIdlePcs(ROUGH_CCU_DATA* pCCU, int idx)
{
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	pPcsSMInfo->emACDC_WorkStatuReq = RequestACDC_WorkStatu(pCCU, idx);
	// TRACEI("OnIdlePcs idx:%d emACDC_WorkStatu:%d req:%d [1:rect 2:invt] "
	// 	"UI:[%.1f %.1f] fMaxCurrPcs=[%.1f] fMaxPwr_Ability:%.1f",
	// 	idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
	// 	pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
	// 	fMaxCurrPcs, pBranchGrpData->fACDC_Max_Output_Pwr_Ability);

	switch (pPcsSMInfo->emACDC_WorkStatuReq)
	{
		case STATU_ACDC_IDLE:
		{
			if (pPcsSMInfo->emACDC_WorkStatuNow != STATU_ACDC_IDLE)
			{
				TRACEE("OnIdlePcs Send_Ctrl_PCS GRP_ACDC_STOP emACDC_WorkStatuNow != STATU_ACDC_IDLE %d", pPcsSMInfo->emACDC_WorkStatuNow);
				Send_Ctrl_PCS(pCCU, GRP_ACDC_STOP, idx, MAX_INTV_MS_CHGWORK);
				return PCS_STATE_IDLE;
			}
		}
			break;
		case STATU_ACDC_RECTIFY:
		{
			if (ElapseMS(pPcsSMInfo->tm_LastStartWork) >= MAX_INTV_MS_CHGWORK)
			{
				pPcsSMInfo->tm_LastStartWork = GetCurrentTime();
				TRACEI("OnIdlePcs Send_Ctrl_PCS GRP_ACDC_RECTIFY_START idx=[%d]", idx);
				Send_Ctrl_PCS(pCCU, GRP_ACDC_RECTIFY_START, idx, MAX_INTV_MS_CHGWORK);
				return PCS_STATE_WAIT_CHARGE;
			}
		}
			break;
		case STATU_ACDC_INVERTER:
		{
			if (ElapseMS(pPcsSMInfo->tm_LastStartWork) >= MAX_INTV_MS_CHGWORK)
			{
				pPcsSMInfo->tm_LastStartWork = GetCurrentTime();
				TRACEI("OnIdlePcs Send_Ctrl_PCS GRP_ACDC_INVERT_START idx=[%d]", idx);
				Send_Ctrl_PCS(pCCU, GRP_ACDC_INVERT_START, idx, MAX_INTV_MS_CHGWORK);
				return PCS_STATE_WAIT_DISCHARGE;
			}
		}
			break;
		default:
			break;
	}
	
	switch (pPcsSMInfo->emACDC_WorkStatuNow)
	{
		case STATU_ACDC_RECTIFY:
		{
			TRACED("OnIdlePcs return PCS_STATE_IN_CHARGING");
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
		case STATU_ACDC_INVERTER:
		{
			TRACED("OnIdlePcs return PCS_STATE_IN_DISCHARGING");
			retState = PCS_STATE_IN_DISCHARGING;
		}
			break;
		default:
		{
			// if (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= MAX_INTV_MS_SEND2PCS)
			// {
			// 	TRACED("OnIdlePcs Send_Ctrl_PCS GRP_ACDC_STOP >10s ElapseMS:%d", ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs));
			// 	Send_Ctrl_PCS(pCCU, GRP_ACDC_STOP, idx);
			// }
			Send_Ctrl_PCS(pCCU, GRP_ACDC_STOP, idx, MAX_INTV_MS_IDLE);
			retState = PCS_STATE_IDLE;
		}
			break;
	}

	return retState;
}

static PcsSMState OnWaitChargePcs(ROUGH_CCU_DATA* pCCU, int idx)
{// idle->chrg:28S dischg->chrg:5S
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	// PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	pPcsSMInfo->emACDC_WorkStatuReq = RequestACDC_WorkStatu(pCCU, idx);
	TRACEI("OnWaitChargePcs idx:%d emACDC_WorkStatu:%d req:%d [1:rect 2:invt] "
		"UI:[%.1f %.1f] fMaxCurrPcs=[%.1f] fMaxPwr_Ability:%.1f",
		idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
		pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
		fMaxCurrPcs, pBranchGrpData->fACDC_Max_Output_Pwr_Ability);
	if (pPcsSMInfo->emACDC_WorkStatuReq != STATU_ACDC_RECTIFY)
	{
		TRACEE("OnWaitChargePcs return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_RECTIFY");
		return PCS_STATE_IDLE;
	}

	if (pPcsSMInfo->emACDC_WorkStatuNow == STATU_ACDC_RECTIFY)
	{
		TRACEI("OnWaitChargePcs Start Chrg Ok:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		LogOut( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
			"Start Chrg Ok:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		return PCS_STATE_IN_CHARGING;
	}

	if (ElapseMS(pPcsSMInfo->tm_LastStartWork) >= MAX_INTV_MS_WAIT_PCS_WORK)//PCS Need Over 30S Wait Power Output
	{
		//means start FAIL
		LogOut( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
			"Start Chrg Fail:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		return PCS_STATE_IDLE;
	}
	
	// if (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= MAX_INTV_MS_SEND2PCS)
	// {
	// 	TRACED("OnWaitChargePcs Send_Ctrl_PCS GRP_ACDC_RECTIFY_START ElapseMS:%d", ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs));
	// 	Send_Ctrl_PCS(pCCU, GRP_ACDC_RECTIFY_START, idx);
	// }
	Send_Ctrl_PCS(pCCU, GRP_ACDC_RECTIFY_START, idx, MAX_INTV_MS_CONTINUE);

	return PCS_STATE_WAIT_CHARGE;
}

static PcsSMState OnChargingPcs(ROUGH_CCU_DATA* pCCU, int idx)
{
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	pPcsSMInfo->emACDC_WorkStatuReq = RequestACDC_WorkStatu(pCCU, idx);
	TRACEI("OnChargingPcs idx:%d emACDC_WorkStatu:%d req:%d [1:rect 2:invt] "
		"UI:[%.1f %.1f] fMaxCurrPcs=[%.1f] fMaxPwr_Ability:%.1f",
		idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
		pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
		fMaxCurrPcs, pBranchGrpData->fACDC_Max_Output_Pwr_Ability);
	if (pPcsSMInfo->emACDC_WorkStatuReq != STATU_ACDC_RECTIFY)
	{
		TRACEE("OnChargingPcs return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_RECTIFY");
		LogOut( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
			"Stop Chrg Ok:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		return PCS_STATE_IDLE;
	}

	switch (pPcsSMInfo->emACDC_WorkStatuNow)
	{
		case STATU_ACDC_IDLE:
		{
			TRACED("OnChargingPcs return PCS_STATE_IDLE");
			retState = PCS_STATE_IDLE;
		}
			break;
		case STATU_ACDC_INVERTER:
		{
			TRACED("OnChargingPcs return PCS_STATE_IN_DISCHARGING");
			retState = PCS_STATE_IN_DISCHARGING;
		}
			break;
		default:
		{
			// float fVoltReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqVolt_2Pcs - MIN(pCtrlRectReq->fGrpVoltLmt, MAX_VOLT_PCS_LMT_REQ));
			// float fCurrReq = MIN(pCtrlRectReq->fGrpCurrLmt, fMaxCurrPcs);
			// float fCurrReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqCurr_2Pcs - fCurrReq);
			// float fCurrOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Curr - fCurrReq);
			// TRACED("OnChargingPcs STATU_ACDC_RECTIFY idx:%d fVoltReqDiff:%.1f fCurrReq:%.1f fCurrReqDiff:%.1f fCurrOutDiff:%.1f "
			// 	"fACDC_RealTime_Curr:%.1f fLastSendReqCurr_2CCU:%.1f fMaxCurrPcs:%.1f",
			// 	idx, fVoltReqDiff, fCurrReq, fCurrReqDiff, fCurrOutDiff,
			// 	pBranchGrpData->fACDC_RealTime_Curr, pPcsSMInfo->fLastSendReqCurr_2Pcs, fMaxCurrPcs);
			// //if ((fVoltReqDiff>MIN_DIFF_VALUE) || (fCurrReqDiff>MIN_DIFF_VALUE) || (fCurrOutDiff>MIN_DIFF_VALUE))
			// if (fCurrOutDiff > MIN_DIFF_VALUE)
			// {
			// 	if((ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= MAX_INTV_MS_CHGWORK))//CATL Need 3S Adjust to 0A,Otherwise BMS will Be OverCurrent Alarm
			// 	{	
			// 		TRACEI("OnChargingPcs Send_Ctrl_PCS GRP_ACDC_RECTIFY_START");
			// 		Send_Ctrl_PCS(pCCU, GRP_ACDC_RECTIFY_START, idx);
			// 	}		
			// }

			Send_Ctrl_PCS(pCCU, GRP_ACDC_RECTIFY_START, idx, MAX_INTV_MS_CONTINUE);
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
	}

	return retState; 
}

static PcsSMState OnWaitDischargePcs(ROUGH_CCU_DATA* pCCU, int idx)
{// 26S
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	// PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	pPcsSMInfo->emACDC_WorkStatuReq = RequestACDC_WorkStatu(pCCU, idx);
	TRACEI("OnWaitDischargePcs idx:%d emACDC_WorkStatu:%d req:%d [1:rect 2:invt] "
		"UI:[%.1f %.1f] fMaxCurrPcs=[%.1f] fMaxPwr_Ability:%.1f",
		idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
		pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
		fMaxCurrPcs, pBranchGrpData->fACDC_Max_Output_Pwr_Ability);
	if (pPcsSMInfo->emACDC_WorkStatuReq != STATU_ACDC_INVERTER)
	{
		TRACEE("OnWaitDischargePcs return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_INVERTER");
		return PCS_STATE_IDLE;
	}
	
	if (pPcsSMInfo->emACDC_WorkStatuNow == STATU_ACDC_INVERTER)
	{
		TRACEI("OnWaitDischargePcs Start Dischrg Ok:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		LogOut( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
			"Start Dischrg Ok:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		return PCS_STATE_IN_DISCHARGING;
	}

	if (ElapseMS(pPcsSMInfo->tm_LastStartWork) >= MAX_INTV_MS_WAIT_PCS_WORK)//PCS Need Over 30S Wait Power Output
	{
		//means start FAIL
		TRACEE("OnWaitDischargePcs Start DisChrg Fail:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		LogOut( MAIN_MODULE_SAMPLER, "PCS", LOG_TYPE_INFO,
			"Start Dischrg Fail:Idx[%d]Imax[%.1f]Ilast[%.1f]Irect[%.1f]!",
			idx, fMaxCurrPcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, pBranchGrpData->fACDC_RealTime_Curr );
		return PCS_STATE_IDLE;
	}

	// if (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= MAX_INTV_MS_SEND2PCS)
	// {
	// 	TRACED("OnWaitDischargePcs Send_Ctrl_PCS GRP_ACDC_INVERT_START ElapseMS:%d", ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs));
	// 	Send_Ctrl_PCS(pCCU, GRP_ACDC_INVERT_START, idx);
	// }

	Send_Ctrl_PCS(pCCU, GRP_ACDC_INVERT_START, idx, MAX_INTV_MS_CONTINUE);
	return PCS_STATE_WAIT_DISCHARGE;
}

static PcsSMState OnDischargingPcs(ROUGH_CCU_DATA* pCCU, int idx)
{
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		return PCS_STATE_FAULTED;
	}

	PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	pPcsSMInfo->emACDC_WorkStatuReq = RequestACDC_WorkStatu(pCCU, idx);
	TRACEI("OnDischargingPcs idx:%d emACDC_WorkStatu:%d req:%d [1:rect 2:invt] "
		"UI:[%.1f %.1f] fMaxCurrPcs=[%.1f] fMaxPwr_Ability:%.1f",
		idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
		pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
		fMaxCurrPcs, pBranchGrpData->fACDC_Max_Output_Pwr_Ability);
	if (pPcsSMInfo->emACDC_WorkStatuReq != STATU_ACDC_INVERTER)
	{
		TRACEE("OnDischargingPcs return PCS_STATE_IDLE Wrong emACDC_WorkStatuReq != STATU_ACDC_INVERTER");
		return PCS_STATE_IDLE;
	}

	switch (pPcsSMInfo->emACDC_WorkStatuNow)
	{
		case STATU_ACDC_IDLE:
		{
			TRACED("OnDischargingPcs return PCS_STATE_IDLE");
			retState = PCS_STATE_IDLE;
		}
			break;
		case STATU_ACDC_RECTIFY:
		{
			TRACED("OnDischargingPcs return PCS_STATE_IN_CHARGING");
			retState = PCS_STATE_IN_CHARGING;
		}
			break;
		default:
		{
			// float fVoltReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqVolt_2Pcs - MIN(pCtrlRectReq->fGrpVoltLmt, MAX_VOLT_PCS_LMT_REQ));
			// float fCurrReq = MIN(pCtrlRectReq->fGrpCurrLmt, fMaxCurrPcs);
			// float fCurrReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqCurr_2Pcs - fCurrReq);
			// float fCurrOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Curr - fCurrReq);
			// TRACED("OnDischargingPcs STATU_ACDC_INVERTER idx:%d fVoltReqDiff:%.1f fCurrReq:%.1f fCurrReqDiff:%.1f fCurrOutDiff:%.1f "
			// 	"fACDC_RealTime_Curr:%.1f fLastSendReqCurr_2CCU:%.1f fMaxCurrPcs:%.1f",
			// 	idx, fVoltReqDiff, fCurrReq, fCurrReqDiff, fCurrOutDiff,
			// 	pBranchGrpData->fACDC_RealTime_Curr, pPcsSMInfo->fLastSendReqCurr_2Pcs, fMaxCurrPcs);
			// //if ((fVoltReqDiff>MIN_DIFF_VALUE) || (fCurrReqDiff>MIN_DIFF_VALUE) || (fCurrOutDiff>MIN_DIFF_VALUE))
			// if (fCurrOutDiff > MIN_DIFF_VALUE)
			// {
			// 	if((ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) >= MAX_INTV_MS_CHGWORK))//Step Speed to 1.0 Cycle to Adjust the Power 
			// 	{
			// 		TRACEI("OnDischargingPcs Send_Ctrl_PCS GRP_ACDC_INVERT_START");
			// 		Send_Ctrl_PCS(pCCU, GRP_ACDC_INVERT_START, idx);
			// 	}		
			// }
			Send_Ctrl_PCS(pCCU, GRP_ACDC_INVERT_START, idx, MAX_INTV_MS_CONTINUE);
			retState = PCS_STATE_IN_DISCHARGING;
		}
		break;
	}

	return retState; 
}

static PcsSMState OnFaultedPcs(ROUGH_CCU_DATA* pCCU, int idx)
{
	// TRACEW("OnFaultedPcs idx:%d", idx);
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	pPcsSMInfo->emACDC_WorkStatuNow = GetACDC_WorkingStatu(pBranchGrpData);
	if (pCCU->emCCUState != CCU_STATE_IDLE)
	{
		if(pPcsSMInfo->emACDC_WorkStatuNow != STATU_ACDC_IDLE)
		{
			TRACEW("OnFaultedPcs Send_Ctrl_PCS GRP_ACDC_STOP");
			Send_Ctrl_PCS(pCCU, GRP_ACDC_STOP, idx, MAX_INTV_MS_CHGWORK);
		}
		return PCS_STATE_FAULTED;
	}

	if (pCCU->emCCUState == CCU_STATE_IDLE)
	{
		TRACED("OnFaultedPcs return PCS_STATE_IDLE");
		return PCS_STATE_IDLE; 
	}
	
	TRACED("OnFaultedPcs return PCS_STATE_FAULTED");
	return PCS_STATE_FAULTED; 
}

static BOOL Send_Ctrl_PCS_Manual(ROUGH_CCU_DATA* pCCU, STATE_MANUL_CTRL emCtrlSendManual, int idx)
{
	double tmNow = GetCurrentTime();
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	// A_BLOB_MANUALCTRL_RECT_CMD* ctrlRectReq_Manual =   &pPcsSMInfo->ctrlRectReq_Manual;
	A_BLOB_MANUALCTRL_RECT_CMD ctrlRectSend_Manual = pPcsSMInfo->ctrlRectReq_Manual;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];

	VAR_VALUE 	varV;
	varV.blobValue = (BYTE *)(&ctrlRectSend_Manual);
	ctrlRectSend_Manual.emState = emCtrlSendManual;
	ctrlRectSend_Manual.nGrp = (UINT)idx;
 	// if(emCtrlSendManual > STAT_MANUAL_STOP)
	{
		//ACDC Running Rectfier Mode
		if(emCtrlSendManual == STAT_MANUAL_START_CHG)
		{
			ctrlRectSend_Manual.fGrpVolt = MIN(ctrlRectSend_Manual.fGrpVolt, MAX_VOLT_PCS_LMT_REQ);
			ctrlRectSend_Manual.fGrpCurr = MIN(ctrlRectSend_Manual.fGrpCurr, fMaxCurrPcs);
			TRACED("Send_Ctrl_PCS_Manual Rectify idx:%d emCtrlSendManual:%d fGrpVolt:%.1f fGrpCurrLmt:%.1f",
				idx, emCtrlSendManual, ctrlRectSend_Manual.fGrpVolt, ctrlRectSend_Manual.fGrpCurr);
		}
		// else if(emCtrlSendManual == GRP_ACDC_RECTIFY_ADJ_CURR)//John Added For PreCharge Function.
		// {
		// 	ctrlRectSend_Manual.fGrpVolt = pCCU->blobSingleBattInfo[idx].fPackTotalVolt - 10;
		// 	ctrlRectSend_Manual.fGrpCurr  = 0.0;
		// }
		else if(emCtrlSendManual == STAT_MANUAL_START_DISCHG)		//ACDC Running Invert Mode
		{
			ctrlRectSend_Manual.fGrpVolt = MIN(ctrlRectSend_Manual.fGrpVolt, MAX_VOLT_PCS_LMT_REQ);
			ctrlRectSend_Manual.fGrpCurr = MIN(ctrlRectSend_Manual.fGrpCurr, fMaxCurrPcs);
			TRACED("Send_Ctrl_PCS_Manual Invert idx:%d emCtrlSendManual:%d fGrpVolt:%.1f fGrpCurrLmt:%.1f",
				idx, emCtrlSendManual, ctrlRectSend_Manual.fGrpVolt, ctrlRectSend_Manual.fGrpCurr);
		}
		// else if(emCtrlSendManual == GRP_ACDC_INVERT_PENDING)//Used For On-Grid to Ensure PCS is On_WaitingRun Status;
		// {
		// 	ctrlRectSend_Manual.fGrpVolt = MIN(1000, pCCU->blobSingleBattInfo[idx].fAllow_LmtChargeVolt);
		// 	ctrlRectSend_Manual.fGrpCurr = 0.0;
		// 	ctrlRectSend_Manual.emCtrl_Type = GRP_ACDC_INVERT_START;//Specical Process.IMSU Not Start Dischg if PCS is Off State.
		// }
		else if(emCtrlSendManual == STAT_MANUAL_STOP)
		{
			ctrlRectSend_Manual.fGrpVolt = MIN(ctrlRectSend_Manual.fGrpVolt, MAX_VOLT_PCS_LMT_REQ);
			ctrlRectSend_Manual.fGrpCurr = 0.0;
			pPcsSMInfo->tm_LastStopWork = tmNow;
		}
		// pPcsSMInfo->emLastCtrlCmdPcs      = ctrlRectSend_Manual.emCtrl_Type;
	}

	/*// TRACED("idx=[%d] cmdType=[%d, %.1f, %.1f] \
	// 	fAllow_LmtChargeVolt=%.1f fAllow_MaxChargeCurr=%.1f  \
	// 	fLimitCurrent=%.1f fACDC_Distri_4_BattChrg_Pwr =%.1f \
	// 	fAllow_MaxDisChargeCurr=%.1f fBatt_Distri_4_ACDC_Pwr=[%.1f]to CCU\n", \
	// 		idx, 	ctrlRectSend_Manual.emCtrl_Type,\
	// 		ctrlRectSend_Manual.fGrpVolt, ctrlRectSend_Manual.fGrpCurr,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_LmtChargeVolt,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_MaxChargeCurr, fLimitCurrent,\
	// 		pCCU->calcNodeData[idx].fACDC_Distri_4_BattChrg_Pwr,\
	// 		pCCU->blobSingleBattInfo[idx].fAllow_MaxDisChargeCurr,\
	// 		pCCU->calcNodeData[idx].fBatt_Distri_4_ACDC_Pwr);*/

	// float fCurrReq = MIN(ctrlRectReq_Manual->fGrpCurr, fMaxCurrPcs);
	// float fCurrOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Curr - fCurrReq);

	float fVoltReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqVolt_2Pcs - ctrlRectSend_Manual.fGrpVolt);
	float fVoltOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Volt - ctrlRectSend_Manual.fGrpVolt);
	float fCurrReqDiff = (float)fabs(pPcsSMInfo->fLastSendReqCurr_2Pcs - ctrlRectSend_Manual.fGrpCurr);
	float fCurrOutDiff = (float)fabs(pBranchGrpData->fACDC_RealTime_Curr - ctrlRectSend_Manual.fGrpCurr);
	
	TRACED("Send_Ctrl_PCS_Manual Push_BlobValue idx:%d fVoltReqDiff:%.1f fVoltOutDiff:%.1f fCurrReqDiff:%.1f fCurrOutDiff:%.1f "
			"OutUI:%.1f %.1f ReqUI:%.1f %.1f fLastSendReqUI:%.1f %.1f fMaxCurrPcs:%.1f ElapseMS:%d %d",
			idx, fVoltReqDiff, fVoltOutDiff, fCurrReqDiff, fCurrOutDiff,
			pBranchGrpData->fACDC_RealTime_Volt, pBranchGrpData->fACDC_RealTime_Curr,
			ctrlRectSend_Manual.fGrpVolt, ctrlRectSend_Manual.fGrpCurr,
			pPcsSMInfo->fLastSendReqVolt_2Pcs, pPcsSMInfo->fLastSendReqCurr_2Pcs, fMaxCurrPcs);
			// ,ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs), msElapse);
			
	pPcsSMInfo->fLastSendReqVolt_2Pcs = ctrlRectSend_Manual.fGrpVolt;
	pPcsSMInfo->fLastSendReqCurr_2Pcs = ctrlRectSend_Manual.fGrpCurr;
	pPcsSMInfo->tm_LastSendCmd_2Pcs = tmNow;

	A_BLOB_MANUALCTRL_RECT_CMD* pCmd = &ctrlRectSend_Manual;
	//Now make the cmd
	BYTE byData[16];
	int nSetGrpVolt = (int)(pCmd->fGrpVolt*10);
	int nSetGrpCurr = (int)(pCmd->fGrpCurr*10);
	int	nSetActv_Pwr = (int)(pCmd->fActv_Pwr*10);
	int	nSetReactv_Pwr = (int)(pCmd->fReactv_Pwr*10);
	TRACEI("ccu_tcp_sampler_main:Control DoTCPSession emState:%d nGrp:%d nSetGrpVolt:%d nSetGrpCurr:%d nSetActv_Pwr:%d nSetReactv_Pwr:%d",
		pCmd->emState, pCmd->nGrp, nSetGrpVolt, nSetGrpCurr, nSetActv_Pwr, nSetReactv_Pwr);
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
	// if(pCmd->nGrp == 9)
	// {
	// 	nRet = ERR_CONTROL_OK;
	// 	int i;
	// 	int nErrFlag = 0;
	// 	for(i = 0; i < MAX_MPPT_GROUP_NUM; i++)
	// 	{
	// 		byData[2] = (BYTE) (5 + i);
	// 		TRACE("nGrp==9 nSendLen=[%d] Data[0]=%02X Data[1]=%02X Data[2]=%02X Data[3]=%02X Data[4]=%02X Data[5]=%02X Data[6]=%02X  Data[7]=%02X Data[8]=%02X Data[9]=%02X Data[10]=%02X Data[11]=%02X\n",\
	// 			nSendLen, byData[0],byData[1],byData[2],byData[3],byData[4],byData[5],byData[6],byData[7],byData[8],byData[9],byData[10],byData[11]);
	// 		nErrFlag +=	DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 1000) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
	// 	}
	// 	if(nErrFlag > 0){nRet = ERR_CONTROL_FAILED;}
	// }
	// else
	{
		TRACE("nGrp!=9 nSendLen=[%d] Data[0]=%02X Data[1]=%02X Data[2]=%02X Data[3]=%02X Data[4]=%02X Data[5]=%02X Data[6]=%02X  Data[7]=%02X Data[8]=%02X Data[9]=%02X Data[10]=%02X Data[11]=%02X\n",\
				nSendLen, byData[0],byData[1],byData[2],byData[3],byData[4],byData[5],byData[6],byData[7],byData[8],byData[9],byData[10],byData[11]);
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, nSendLen, 1000) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
	}

	// Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
	// 				200, 217, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT_SCADA);

	return TRUE;
}

static void CtrlPcs_Manual(ROUGH_CCU_DATA* pCCU, int idx)
{
	// SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	

	// PcsSMState retState;
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[idx];
	PcsSMInfo* pPcsSMInfo =                      &pCCU->pcsSMInfos[idx];
	//A_BLOB_GRP_RECT_AUTOCTRL_CMD* pCtrlRectReq = &pPcsSMInfo->ctrlRectReq;
	A_BLOB_MANUALCTRL_RECT_CMD* ctrlRectReq_Manual = &pPcsSMInfo->ctrlRectReq_Manual;
	A_BLOB_RECT_GRP_DATA* pGrpRectData =         &pCCU->strLSYSInfo.blobGrpRectData;
	float fMaxCurrPcs = pGrpRectData->fRealTime_MaxCurr[idx];

	if (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) < MAX_INTV_MS_CHGWORK)
	{
		// TRACEW("CtrlPcs_Manual too short ElapseMS:%d", ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs));
		return;
	}

	{// unit test 手动模式测试按设置值直接发送
		if (ElapseMS(pPcsSMInfo->tm_LastSendCmd_2Pcs) > 10000)
		{
			double tmNow = GetCurrentTime();
			A_BLOB_MANUALCTRL_RECT_CMD ctrlRectSend_Manual = pPcsSMInfo->ctrlRectReq_Manual;
			VAR_VALUE 	varV;
			varV.blobValue = (BYTE *)(&ctrlRectSend_Manual);
			ctrlRectSend_Manual.nGrp = (UINT)idx;
			
			pPcsSMInfo->tm_LastSendCmd_2Pcs = tmNow;
			Push_BlobValue(SETTYPE_INTERN_CTRL, pCCU->hSampler, 
							200, 217, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT_SCADA);
		}
		return;
	}

	pPcsSMInfo->emACDC_WorkStatuNow = STATU_ACDC_IDLE;
	if (pBranchGrpData->fACDC_RealTime_Curr > MIN_CURR_WORKING)
	{
		if (pBranchGrpData->emACDC_WorkStatu == STATU_ACDC_RECTIFY ||
			pBranchGrpData->emACDC_WorkStatu == STATU_ACDC_INVERTER)
		{
			pPcsSMInfo->emACDC_WorkStatuNow = pBranchGrpData->emACDC_WorkStatu;
		}
	}
	
	// system abnormal stop
	VAR_VALUE 	varV;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ALMSTATE, 1, 12, &varV, 4);
	BOOL bSysOverTemp = (varV.emValue == 1) ? TRUE : FALSE;
	TRACED("CtrlPcs_Manual bSysOverTemp:%d emCCUState:%d emACDC_WorkStatuNow:%d fACDC_Max_Output_Pwr_Ability:%.1f fMaxCurrPcs:%.1f",
		bSysOverTemp, pCCU->emCCUState, pPcsSMInfo->emACDC_WorkStatuNow, pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrPcs);
	if (bSysOverTemp || (pCCU->emCCUState!=CCU_STATE_IDLE) ||
		pBranchGrpData->fACDC_Max_Output_Pwr_Ability < MIN_PWR_PCS_OUT_ABL ||
		fMaxCurrPcs < MIN_CURR_PCS_OUT_ABL)
	{
		if(pPcsSMInfo->emACDC_WorkStatuNow != STATU_ACDC_IDLE)
		{
			TRACEW("CtrlPcs_Manual return Send_Ctrl_PCS_Manual STAT_MANUAL_STOP");
			Send_Ctrl_PCS_Manual(pCCU, STAT_MANUAL_STOP, idx);
			LogOut(MAIN_MODULE_SAMPLER, "Manual Stop", LOG_TYPE_INFO,
				"Idx[%d]SysOverTemp[%d]ccuStatu[%d]"
				"Pmax[%.1f]Imax[%.1f]Irect[%.1f]Vrect[%.1f]ACDC[%d]",
				idx, bSysOverTemp, pCCU->emCCUState,
				pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrPcs,
				pBranchGrpData->fACDC_RealTime_Curr, pBranchGrpData->fACDC_RealTime_Volt,
				pPcsSMInfo->emACDC_WorkStatuNow);
			return;
		}
	}

	// ACDC_WorkStatu
	pPcsSMInfo->emACDC_WorkStatuReq = STATU_ACDC_IDLE;
	if (ctrlRectReq_Manual->emState==STAT_MANUAL_START_CHG ||
		ctrlRectReq_Manual->emState==STAT_MANUAL_ADJ_CHG)
	{
		TRACED("CtrlPcs_Manual STATU_ACDC_RECTIFY emState:%d", ctrlRectReq_Manual->emState);
		pPcsSMInfo->emACDC_WorkStatuReq = STATU_ACDC_RECTIFY;
	}
	else if (ctrlRectReq_Manual->emState==STAT_MANUAL_START_DISCHG ||
		ctrlRectReq_Manual->emState==STAT_MANUAL_ADJ_DISCHG)
	{
		TRACED("CtrlPcs_Manual STATU_ACDC_INVERTER emState:%d", ctrlRectReq_Manual->emState);
		pPcsSMInfo->emACDC_WorkStatuReq = STATU_ACDC_INVERTER;
	}

	// 工作中的WorkStatu不是请求的状态 stop
	if (pPcsSMInfo->emACDC_WorkStatuNow != STATU_ACDC_IDLE &&
		pPcsSMInfo->emACDC_WorkStatuNow != pPcsSMInfo->emACDC_WorkStatuReq)
	{
		TRACEE("CtrlPcs_Manual return Send_Ctrl_PCS_Manual STAT_MANUAL_STOP emACDC_WorkStatuNow!=emACDC_WorkStatuReq %d %d",
			pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq);
		Send_Ctrl_PCS_Manual(pCCU, STAT_MANUAL_STOP, idx);
		LogOut(MAIN_MODULE_SAMPLER, "Manual Stop", LOG_TYPE_INFO,
			"Idx[%d]WorkStatuNow[%d]WorkStatuReq[%d]"
			"Pmax[%.1f]Imax[%.1f]Irect[%.1f]Vrect[%.1f]",
			idx, pPcsSMInfo->emACDC_WorkStatuNow, pPcsSMInfo->emACDC_WorkStatuReq,
			pBranchGrpData->fACDC_Max_Output_Pwr_Ability, fMaxCurrPcs,
			pBranchGrpData->fACDC_RealTime_Curr, pBranchGrpData->fACDC_RealTime_Volt);
		return;
	}

	// 正常，处理请求
	Send_Ctrl_PCS_Manual(pCCU, ctrlRectReq_Manual->emState, idx);
}