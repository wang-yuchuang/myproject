// #define _TRACE_PRINT_ // AEMS
#include "aems_main.h"

#define MAX_MS_Backflow      (1000)
#define MAX_RATIO_RATEDPWR   (0.98f)
#define MAX_CtrlPwr_Diff     (5)

int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE		emEvType,
	_EVENT_MSG		emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int				nMaxBlockMSec)
{
	UNUSED(emMsgType); UNUSED(bBlockMode); UNUSED(nMaxBlockMSec);
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	ROUGH_AEMS* pAEMS = (ROUGH_AEMS*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = pAEMS->pDPR;
    VAR_VALUE varV;
    ERRCDDE_DPR err;
	TRACE("aems_main:Event_Handler got EVENT[%d] nEvDataSize=[%d]--------\n", emEvType, nEvDataSize);
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if (pCfgChange->parmPTR.nId == 1 && pCfgChange->parmPTR.nSubId == 26)
		{
 			varV.blobValue = (BYTE *)&pAEMS->blobBackendCfg;
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
			if(err != ERR_DPR_OK)
			{
				TRACEE("aems_main:Event_Handler A_BLOB_ES_BACKEND_CFG failed");
				return EVENT_HANDED_FAILED;
			}
			pAEMS->fProtBackflow_PV_AEMS = pAEMS->blobBackendCfg.fPreProt4RefluxAndOverLoad;
			TRACED("aems_main:Event_Handler A_BLOB_ES_BACKEND_CFG fProtBackflow_PV_AEMS:%.1f", pAEMS->fProtBackflow_PV_AEMS);
		}
		// // unit test
		// else if ((pCfgChange->parmPTR.nId == 200) &&
		// 	( (pCfgChange->parmPTR.nSubId >= 204 && pCfgChange->parmPTR.nSubId <= 207)
		// 	))
		// {
		// 	err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
		// 		pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
		// 	if(err != ERR_DPR_OK)
		// 	{
		// 		return EVENT_HANDED_FAILED;
		// 	}
		// 	switch (pCfgChange->parmPTR.nSubId)
		// 	{
		// 	case 204: // MPPT_1 Generate KWH 200, 204
		// 		varV.emValue = (varV.fValue>0 ? 1:0);
		// 		Push_VarValue(SETTYPE_INTERN_CTRL, hModuleSeif, 200, 301, varV); // bStandby_AEMS
		// 		break;
		// 	case 205: // MPPT_2 Generate KWH
		// 	{
		// 		A_BLOB_PV_CTRL_BY_AEMS pvCtrl;
		// 		A_BLOB_PV_CTRL_BY_AEMS* pPvCtrl = &pvCtrl;
		// 		ZERO_POBJS(&pvCtrl, 1);
		// 		pPvCtrl->CtrlMode = CTRL_BY_AEMS;
		// 		pPvCtrl->nPVStatus = 1;
		// 		pPvCtrl->fActivePower = varV.fValue;
		// 		pPvCtrl->fReActivePower = 0;
		// 		varV.blobValue = (BYTE*)pPvCtrl;
		// 		Push_BlobValue(SETTYPE_INTERN_CTRL, hModuleSeif,
		// 			1050, 534, (void*)varV.blobValue, sizeof(A_BLOB_PV_CTRL_BY_AEMS), BLOB_TYPE_PV_SRTTING_BY_AEMS);
		// 	}
		// 		break;

		// 	default:
		// 		break;
		// 	}
		// 	TRACED("aems_main:Event_Handler %d, %d %lu %.1f err:%d ",
		// 		pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, varV.ulValue, varV.fValue, err);
		// }
		else if ((pCfgChange->parmPTR.nId == 1001) &&
			(pCfgChange->parmPTR.nSubId == 3301))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 3301: // Automatic charging Enable 1001, 3301,
				pAEMS->bAutoChrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			// case 3302: // Automatic Discharging Enable 1001, 3302, 
			// 	pEMS->bAutoDischrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			// 	break;

			default:
				break;
			}
			TRACED("aems_main:Event_Handler nId=1001, %d %lu err:%d "
				"bAutoChrgEnable:%d",
				pCfgChange->parmPTR.nSubId, varV.ulValue, err,
				pAEMS->bAutoChrgEnable);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if ((pCfgChange->parmPTR.nId == 1050) &&
			(pCfgChange->parmPTR.nSubId == 440))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 440: // 1050, 440,
				pAEMS->bAntiBackflow_PV = (BOOL)((err==ERR_DPR_OK) ? (varV.emValue>0 ? TRUE:FALSE) : FALSE);
				break;
			default:
				break;
			}
			TRACED("aems_main:Event_Handler nId=1050, %d %lu err:%d "
				"bAntiBackflow_PV:%d",
				pCfgChange->parmPTR.nSubId, varV.ulValue, err,
				pAEMS->bAntiBackflow_PV);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
	}
	return EVENT_HANDED_OK;
}

DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
    pSampler->pDataHeap = (void*)NEW(ROUGH_AEMS, 1);
    if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
	ROUGH_AEMS* pAEMS = (ROUGH_AEMS*)pSampler->pDataHeap;
    ZERO_POBJS(pAEMS, 1);
    pAEMS->hSamplerLock = pSampler->hSamplerLock;
	pAEMS->hSampler = hSamplerSelf;
	pAEMS->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},

	};
	int n = 0;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pAEMS->pDPR->Register_EvHandler("AEMS_EvHandler",
			hSamplerSelf,
			Event_Handler,
			(UINT)tags[n].nEvTag,
			(UINT)tags[n].nMsgTag,
			FALSE);
	}

	DATA_PROCESSOR* pDPR = pAEMS->pDPR;
    VAR_VALUE varV;
    ERRCDDE_DPR err;
	varV.blobValue = (BYTE *)&pAEMS->blobBackendCfg;
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
	if(err != ERR_DPR_OK)
	{
		TRACEE("aems_main:Initialize Get A_BLOB_ES_BACKEND_CFG Failed");
		return FALSE;
	}
	pAEMS->fProtBackflow_PV_AEMS = pAEMS->blobBackendCfg.fPreProt4RefluxAndOverLoad;

	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 1001, 3301, &varV, 4);
	pAEMS->bAutoChrgEnable = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 1050, 440, &varV, 4);
	pAEMS->bAntiBackflow_PV = (BOOL)((err==ERR_DPR_OK) ? (varV.emValue>0?TRUE:FALSE) : FALSE);
	TRACED("aems_main:Initialize bAutoChrgEnable:%d bAntiBackflow_PV:%d",
		pAEMS->bAutoChrgEnable, pAEMS->bAntiBackflow_PV);
	
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 990, &varV, 4);
	pAEMS->ctrlPVMode = (CtrlPVMode)((err==ERR_DPR_OK) ? varV.emValue : CtrlPVMode_Auto);
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 991, &varV, 4);
	pAEMS->fProtBackflow_PV_DAQ =    (err==ERR_DPR_OK) ? varV.fValue : 100.0f;

	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 992, &varV, 4);
	pAEMS->uPStep_PV =               (err==ERR_DPR_OK) ? varV.ulValue : 0;
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 993, &varV, 4);
	pAEMS->uMsStep_PV =              (err==ERR_DPR_OK) ? varV.ulValue : 0;
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 994, &varV, 4);
	pAEMS->uMaxPBackflow =           (err==ERR_DPR_OK) ? varV.ulValue : 0;
	err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 991, 995, &varV, 4);
	pAEMS->uMaxMsBackflow =          (err==ERR_DPR_OK) ? varV.ulValue : 0;

	TRACED("aems_main:Initialize ctrlPVMode:%d fProtBackflow_PV_AEMS:%.1f fProtBackflow_PV_DAQ:%.1f "
		"uPStep_PV:%u uMsStep_PV:%u uMaxPBackflow:%u uMaxMsBackflow:%u",
		pAEMS->ctrlPVMode, pAEMS->fProtBackflow_PV_AEMS, pAEMS->fProtBackflow_PV_DAQ,
		pAEMS->uPStep_PV, pAEMS->uMsStep_PV, pAEMS->uMaxPBackflow, pAEMS->uMaxMsBackflow);
	return TRUE;
}

DLLExport int Control(HANDLE hSamplerThis , PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	UNUSED(nDevId); UNUSED(nValSize); 
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_AEMS* pAEMS = (ROUGH_AEMS*)pSampler->pDataHeap;
	HANDLE hSamplerLock = ((SAMPLER*)pAEMS->hSampler)->hSamplerLock;

	if(Mutex_Lock(hSamplerLock, (DWORD)nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)   //settings
	{
		switch (nParamId)
		{
		case 990: // 991, 990,
			pAEMS->ctrlPVMode = (CtrlPVMode)pVarVal->emValue;
			break;
		case 991: // 991, 991,
			pAEMS->fProtBackflow_PV_DAQ = pVarVal->fValue;
			break;

		case 992: // 991, 992,
			pAEMS->uPStep_PV = pVarVal->ulValue;
			break;
		case 993: // 991, 993,
			pAEMS->uMsStep_PV = pVarVal->ulValue;
			break;
		case 994: // 991, 994,
			pAEMS->uMaxPBackflow = pVarVal->ulValue;
			break;
		case 995: // 991, 995,
			pAEMS->uMaxMsBackflow = pVarVal->ulValue;
			break;
		
		default:
			break;
		}
		TRACED("aems_main:Control ctrlPVMode:%d fProtBackflow_PV_AEMS:%.1f fProtBackflow_PV_DAQ:%.1f "
		"uPStep_PV:%u uMsStep_PV:%u uMaxPBackflow:%u uMaxMsBackflow:%u",
		pAEMS->ctrlPVMode, pAEMS->fProtBackflow_PV_AEMS, pAEMS->fProtBackflow_PV_DAQ,
		pAEMS->uPStep_PV, pAEMS->uMsStep_PV, pAEMS->uMaxPBackflow, pAEMS->uMaxMsBackflow);
	}
	// else	//control
	// {
		
	// }
	
	Mutex_Unlock(hSamplerLock);
	return ERR_CONTROL_OK;
}


DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	UNUSED(hSamplerThis); UNUSED(pJigData);
	return ERR_CONTROL_OK;
}


static void CtrlPV(ROUGH_AEMS* pAEMS, BOOL bCtrlByAEMS, float fActivePower)
{
	PV* pPV = &pAEMS->pv;
	A_BLOB_PV_CTRL_BY_AEMS* pPvCtrl = &pPV->pvCtrl;
	VAR_VALUE varV;

	pPvCtrl->CtrlMode = bCtrlByAEMS ? CTRL_BY_AEMS : CTRL_BY_DATA_ACQUISITION;
	pPvCtrl->nPVStatus = 1;
	pPvCtrl->fActivePower = fActivePower;
	pPvCtrl->fReActivePower = 0;
	
	varV.blobValue = (BYTE*)pPvCtrl;
	Push_BlobValue(SETTYPE_INTERN_CTRL, pAEMS->hSampler,
		1050, 534, (void*)varV.blobValue, sizeof(A_BLOB_PV_CTRL_BY_AEMS), BLOB_TYPE_PV_SRTTING_BY_AEMS);

	pPV->tmCtrlPwr_Last = _NOW_;
}

static BOOL CtrlByDAQ(ROUGH_AEMS* pAEMS)
{
    A_BLOB_SEMS_CLIENTINFO_SUM* pBessInfo = &pAEMS->bess.bessInfo;
	PV* pPV = &pAEMS->pv;
	A_BLOB_PV_CTRL_BY_AEMS* pPvCtrl =        &pPV->pvCtrl;
	A_BLOB_PV_INVERTER_DATA_TOTAL* pPvInfo = &pPV->pvInfo;

	// pPvInfo->nAvailablePVNums > 0
	if (pAEMS->bAntiBackflow_PV && // PV有防逆流功能
		pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_INVERTER && pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_RECTIFY && pBessInfo->fACDC_RealTime_CurrSum < 2 &&
		pAEMS->fMax_Chrg_PwrAbility_AEMS < MIN_PWR_CHRG_4START // 储能没充放电 && BESS没有消纳能力
		// || (pBessInfo->f_PgridMeterSum > pAEMS->fProtBackflow_PV_DAQ && (pPvInfo->RecvCtrlMode == CTRL_BY_DATA_ACQUISITION // 防逆流表大于光伏防逆流保护值 && 本地控制
			// || pPvInfo->fTotalPVOutputActPower > pPvInfo->iTotalRatedPower*MAX_RATIO_RATEDPWR)))) // 防逆流表大于光伏防逆流保护值 && 当前光伏放电功率接近额定功率
	)
	{
		// 1.1. 光伏本地控制
		TRACEI("aems_main:CtrlByDAQ CTRL_BY_DATA_ACQUISITION");
		if (pPvCtrl->CtrlMode != CTRL_BY_DATA_ACQUISITION || pPvInfo->RecvCtrlMode != CTRL_BY_DATA_ACQUISITION || ElapseS(pPV->tmCtrlPwr_Last) > 5)
		{
			CtrlPV(pAEMS, FALSE, pPvInfo->fTotalPVOutputActPower);
		}
		return TRUE;
	}

	return FALSE;
}

static BOOL CtrlByAEMS(ROUGH_AEMS* pAEMS)
{
	// DATA_PROCESSOR* pDPR = pAEMS->pDPR;
    // VAR_VALUE varV;
    // ERRCDDE_DPR err;

	// BESS* pBESS = &pAEMS->bess;
    A_BLOB_SEMS_CLIENTINFO_SUM* pBessInfo = &pAEMS->bess.bessInfo;
	PV* pPV = &pAEMS->pv;
	A_BLOB_PV_CTRL_BY_AEMS* pPvCtrl =        &pPV->pvCtrl;
	A_BLOB_PV_INVERTER_DATA_TOTAL* pPvInfo = &pPV->pvInfo;
	pAEMS->bPwrUpStep_PV = FALSE;
	// 光伏AEMS控制
	pAEMS->fPBackflow =    (pBessInfo->f_PgridMeterSum<0 ? ABS(pBessInfo->f_PgridMeterSum):0);
	pAEMS->bChrgAbl_Bess = (pAEMS->fRemainChrgAbl_Bess > pAEMS->fPBackflow); // BESS是否有消纳逆流功率的能力
	// 没有放电能力
	// PV已经通信上且没有放电能力
	// err = pDPR->Get(pAEMS->hSampler, GETTYPE_ALMSTATE, 1051, 3400, &varV, 4);
	// BOOL bCommFailed = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);

	// 3400
	// 3750

	// pDPR->Get(pSampler, GETTYPE_ALMSTATE, 406, 401, &varV, 4);
	// BOOL bAlarmStatus = varV.emValue;

	// pDPR->Get(pSampler, GETTYPE_ST_VAR, 406, 401, &varV, 4);
	// BOOL bInstalled = varV.emValue;

	// if (bInstalled == METER_TYPE_NotInstalled || bAlarmStatus != ALARM_STATUS_CEASED)
	// {
	// 	pPvInverterSample->blobPVTotalinfo.fTotalPVOutputActPower = 		fTotalPVOutputActPowerTemp * 0.001; //kw
	// }
	// else
	// {
	// 	pDPR->Get(pSampler, GETTYPE_SP_VAR, 406, 400, &varV, 4); 		//PV Meter Active Power
    //     pPvInverterSample->blobPVTotalinfo.fTotalPVOutputActPower = varV.fValue;   //KW
	// }
	// 2.1:shutdown
	if (pAEMS->fPLoad_AndBess < pAEMS->fProtBackflow_PV_AEMS) // 没有负载和BESS充电能力,关机
	{
		TRACEW("aems_main:CtrlByAEMS 2.1 shutdown fPLoad_AndBess:%.1f < fProtBackflow_PV_AEMS:%.1f",
			pAEMS->fPLoad_AndBess, pAEMS->fProtBackflow_PV_AEMS);
		if (pPvInfo->fRecvActivePower > 0 || pPvInfo->fTotalPVOutputActPower > 1)
		{
			TRACEW("aems_main:CtrlByAEMS 2.1 shutdown fActivePower:%.1f", pPvCtrl->fActivePower);
			CtrlPV(pAEMS, TRUE, 0);
		}
		return TRUE;
	}
	// 2.2:PwrDown
	if (pBessInfo->f_PgridMeterSum < pAEMS->fProtBackflow_PV_AEMS)
	{
		// 2.2 PwrDown 逆流
		pAEMS->msBackflow = ElapseMS(pAEMS->dtmBackflow_Start);
		pAEMS->bPwrDown_PV = ((pAEMS->msBackflow > MAX_MS_Backflow && (pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_IDLE // BESS停止
			|| !pAEMS->bChrgAbl_Bess // BESS无法消纳逆流功率
			|| pAEMS->fPBackflow > pAEMS->uMaxPBackflow)) // 逆流太大
			|| (pAEMS->fPBackflow > 0 && pAEMS->msBackflow > pAEMS->uMaxMsBackflow) // 逆流太久
			);
		TRACEW("aems_main:CtrlByAEMS 2.2 backflow f_PgridMeterSum:%.1f < fProtBackflow_PV_AEMS:%.1f fPBackflow:%.1f msBackflow:%u "
			"bPwrDown:%d [%d && (%d || %d)] || (%d && %d)",
			pBessInfo->f_PgridMeterSum, pAEMS->fProtBackflow_PV_AEMS, pAEMS->fPBackflow, pAEMS->msBackflow,
			pAEMS->bPwrDown_PV, pAEMS->msBackflow > MAX_MS_Backflow, pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_IDLE, !pAEMS->bChrgAbl_Bess,
			pAEMS->fPBackflow>0, pAEMS->msBackflow > pAEMS->uMaxMsBackflow);
		if (pAEMS->bPwrDown_PV)
		{
			// 降功率
			pPvCtrl->fActivePower = pPvInfo->fTotalPVOutputActPower - pAEMS->fPBackflow - pAEMS->fProtBackflow_PV_AEMS;
			pPvCtrl->fActivePower = MAX(0, pPvCtrl->fActivePower);
			if (pAEMS->msBackflow > pAEMS->uMaxMsBackflow * 2)
			{// 一直逆流，先降到0
				pPvCtrl->fActivePower = 0;
			}
			if (pPvCtrl->CtrlMode != CTRL_BY_AEMS || pPvInfo->RecvCtrlMode != CTRL_BY_AEMS)
			{
				CtrlPV(pAEMS, TRUE, pPvCtrl->fActivePower);
				TRACEW("aems_main:CtrlByAEMS 2.2 pwrDn fActivePower:%.1f", pPvCtrl->fActivePower);
			}
			return TRUE;
		}
	}
	else
	{
		pAEMS->dtmBackflow_Start = GetCurrentTime();
	}
	// 3.1 PwrUpAll 全额放电
	if ((float)pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AC // PV额定功率 < 负载
		|| (pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_IDLE && pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AndBess) // 已经开机并且PV额定功率 < 总消纳功率
	)
	{
		pPvCtrl->fActivePower = (float)(pPvInfo->iTotalRatedPower);
		TRACED("aems_main:CtrlByAEMS 3.1 pwrAll fActivePower:%.1f iTotalRatedPower:%d fPLoad_AC:%.1f "
			"emACDC_WorkStatuSum:%d fPLoad_AndBess:%.1f",
			pPvCtrl->fActivePower, pPvInfo->iTotalRatedPower, pAEMS->fPLoad_AC,
			pBessInfo->emACDC_WorkStatuSum, pAEMS->fPLoad_AndBess);
	}
	// 3.2 PwrUpStep 往上步调
	else
	{
		pAEMS->bPwrUpStep_PV = (ElapseMS(pPV->dtmRetry_Last) > pAEMS->uMsStep_PV &&
			(pBessInfo->f_PgridMeterSum > (pAEMS->fProtBackflow_PV_AEMS + (float)pAEMS->uPStep_PV/2))); // 防逆流保护值内
		if (pAEMS->bPwrUpStep_PV)
		{
			if (pPvInfo->fRecvActivePower > (((float)pPvInfo->iTotalRatedPower) * MAX_RATIO_RATEDPWR))
			{
				TRACED("aems_main:CtrlByAEMS 3.2 pwrUp all fRecvActivePower:%.1f", pPvInfo->fRecvActivePower);
				pPvCtrl->fActivePower = (float)pPvInfo->iTotalRatedPower;
			}
			else
			{
				TRACED("aems_main:CtrlByAEMS 3.2 uPStep_PV fRecvActivePower:%.1f + uPStep_PV:%u",
					pPvInfo->fRecvActivePower, pAEMS->uPStep_PV);
				pPvCtrl->fActivePower = pPvInfo->fRecvActivePower + (float)pAEMS->uPStep_PV;
			}
			TRACEI("aems_main:CtrlByAEMS 3.2 pwrUp fActivePower:%.1f ElapseMS:%u uMsStep_PV:%u",
				pPvCtrl->fActivePower, ElapseMS(pPV->dtmRetry_Last), pAEMS->uMsStep_PV);
			pPV->dtmRetry_Last = GetCurrentTime();
		}
	}
	
	pPvCtrl->fActivePower = MIN((float)pPvInfo->iTotalRatedPower, pPvCtrl->fActivePower);
	float fCtrlPwr_Diff = ABS(pPvCtrl->fActivePower - pPvInfo->fRecvActivePower);
	if (pPvCtrl->CtrlMode != CTRL_BY_AEMS || pPvInfo->RecvCtrlMode != CTRL_BY_AEMS ||
		fCtrlPwr_Diff>MAX_CtrlPwr_Diff || ElapseS(pPV->tmCtrlPwr_Last) > 1)
	{
		CtrlPV(pAEMS, TRUE, pPvCtrl->fActivePower);
		TRACED("aems_main:CtrlByAEMS Push_BlobValue fActivePower:%.1f", pPvCtrl->fActivePower);
	}
	// PID算法:
	// 其他，逐步加大放电功率，让储能自动充电

	// 2.2 PV额定功率 < 负载 + ESS可充功率, 等待储能待机后PV逐步加大放电

	// 2.2 PV额定功率 > 负载 先按负载放电，等待储能待机后PV逐步加大放电
	return TRUE;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	TRACEW(">>> aems_main:Sample >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_AEMS* pAEMS = (ROUGH_AEMS*)pSampler->pDataHeap;
	// HANDLE hSamplerLock = ((SAMPLER*)pAEMS->hSampler)->hSamplerLock;

	// TRACED("aems_main:Sample lcType:%d", pAEMS->lcCfg.lcType);
    DATA_PROCESSOR* pDPR = pAEMS->pDPR;
    VAR_VALUE varV;
    // ERRCDDE_DPR err;
    BOOL bNeedStandby;
	// 1. 计算负载
	BESS* pBESS = &pAEMS->bess;
    A_BLOB_SEMS_CLIENTINFO_SUM* pBessInfo = &pAEMS->bess.bessInfo;
    ZERO_POBJS(pBessInfo, 1);
	varV.blobValue = (BYTE*)pBessInfo;
	pDPR->Get(pAEMS->hSampler, GETTYPE_SP_VAR, 1001, 3900, &varV, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM));

	PV* pPV = &pAEMS->pv;
	A_BLOB_PV_CTRL_BY_AEMS* pPvCtrl =        &pPV->pvCtrl;
	A_BLOB_PV_INVERTER_DATA_TOTAL* pPvInfo = &pPV->pvInfo;
    ZERO_POBJS(pPvInfo, 1);
	varV.blobValue = (BYTE*)pPvInfo;
	pDPR->Get(pAEMS->hSampler, GETTYPE_SP_VAR, 1050, 3697, &varV, sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL));

	// unit test
	// err = pDPR->Get(pAEMS->hSampler, GETTYPE_ST_VAR, 200, 207, &varV, 4); // MPPT_4 Generate KWH
    // int ctrlByAEMS = (int)((err == ERR_DPR_OK) ? varV.fValue : 0);
	pAEMS->fMax_Chrg_PwrAbility_AEMS = (pAEMS->bAutoChrgEnable ? pBessInfo->fMax_Chrg_PwrAbilitySumA :
		(pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_RECTIFY ? pBessInfo->fMax_Chrg_PwrAbilitySum : 0));
	pAEMS->fRemainChrgAbl_Bess = (pAEMS->fMax_Chrg_PwrAbility_AEMS - pBessInfo->f_PcabMeterSum);
	pAEMS->fPLoad_AC =      pPvInfo->fTotalPVOutputActPower + pBessInfo->fPacload4DischgSum;
	pAEMS->fPLoad_AndBess = pAEMS->fPLoad_AC + pAEMS->fMax_Chrg_PwrAbility_AEMS;
    TRACED("aems_main:Sample f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f fMax_Chrg_PwrAbility_AEMS:%.1f "
		"bAutoChrgEnable:%d fMax_Chrg_PwrAbilitySumA:%.1f %.1f bAntiBackflow_PV:%d "
		"fPLoad_AC:%.1f=fTotalPVOutputActPower:%.1f+fPacload4DischgSum:%.1f bPwrUpStep_PV:%d",
		pBessInfo->f_PgridMeterSum, pBessInfo->f_PcabMeterSum, pAEMS->fMax_Chrg_PwrAbility_AEMS,
		pAEMS->bAutoChrgEnable, pBessInfo->fMax_Chrg_PwrAbilitySumA, pBessInfo->fMax_Chrg_PwrAbilitySum, pAEMS->bAntiBackflow_PV,
		pAEMS->fPLoad_AC, pPvInfo->fTotalPVOutputActPower, pBessInfo->fPacload4DischgSum, pAEMS->bPwrUpStep_PV);
    // ？？？ 是否需要保护值+跟随比
    // (pEMS->fPacload4Dischg - pAEMS->blobBackendCfg.fPreProt4RefluxAndOverLoad) * pAEMS->blobBackendCfg.fOutput2AC_PwrRate;

	// 光伏自动控制模式:储能充满了且没有放电计划
	// 上级控制模式:其他:储能未充满或者有放电计划

	// 光伏没有放电能力
	// if (pPvInfo->fTotalPVPowerAbility < MIN_PWR_LMT ||
	// 	(pAEMS->bAntiBackflow_PV && // PV有防逆流功能
	// 		((pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_INVERTER && pBessInfo->fACDC_RealTime_CurrSum>2
	// 			&& pAEMS->fMax_Chrg_PwrAbility_AEMS < MIN_PWR_CHRG_4START) // 储能没放电 && BESS没有消纳能力
	// 		|| (pBessInfo->f_PgridMeterSum > pAEMS->fProtBackflow_PV_DAQ && (pPvInfo->RecvCtrlMode == CTRL_BY_DATA_ACQUISITION // 防逆流表大于光伏防逆流保护值 && 本地控制
	// 			|| pPvInfo->fTotalPVOutputActPower > pPvInfo->iTotalRatedPower*MAX_RATIO_RATEDPWR)))) // 防逆流表大于光伏防逆流保护值 && 当前光伏放电功率接近额定功率
	// )
	TRACED("aems_main:Sample fTotalPVPowerAbility:%.1f fMax_Chrg_PwrAbility_AEMS:%.1f "
		"%d (bAntiBackflow_PV:%d && emACDC_WorkStatuSum:%d && fACDC_RealTime_CurrSum:%.1f && fMax_Chrg_PwrAbility_AEMS:%d)",
		pPvInfo->fTotalPVPowerAbility, pAEMS->fMax_Chrg_PwrAbility_AEMS,
		(pAEMS->bAntiBackflow_PV && // PV有防逆流功能
			pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_INVERTER && pBessInfo->fACDC_RealTime_CurrSum>2
			&& pAEMS->fMax_Chrg_PwrAbility_AEMS < MIN_PWR_CHRG_4START),
			
		pAEMS->bAntiBackflow_PV, pBessInfo->emACDC_WorkStatuSum, pBessInfo->fACDC_RealTime_CurrSum,
		pAEMS->fMax_Chrg_PwrAbility_AEMS);

	switch (pAEMS->ctrlPVMode)
	{
	case CtrlPVMode_DAQ:
		CtrlByDAQ(pAEMS);
		break;

	case CtrlPVMode_AEMS:
		CtrlByAEMS(pAEMS);
		break;
	
	default:
	{
		if (!CtrlByDAQ(pAEMS))
		{
			CtrlByAEMS(pAEMS);
		}
	}
		break;
	}
	// 11.bStandby_AEMS
	bNeedStandby =
		(pPvInfo->fTotalPVPowerAbility > MIN_PWR_LMT && pAEMS->fMax_Chrg_PwrAbility_AEMS > MIN_PWR_CHRG_4START &&
			(pPvInfo->fTotalPVOutputActPower >= pAEMS->fPLoad_AC
				|| pBessInfo->fPacload4DischgSum < pAEMS->fLtPwr2Standby_4Chrg || pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_INVERTER)
		);
	if (bNeedStandby)
	{
		// 开启待机
		if (!pBessInfo->bStandby_AEMS)
		{
			varV.emValue = TRUE;
			Push_VarValue(SETTYPE_INTERN_CTRL, pAEMS->hSampler, 200, 301, varV);
		}
		pBESS->tmCtrlStandby = _NOW_;
	}
	else
	{
		// 停止待机
		if (ElapseS(pBESS->tmCtrlStandby) > pAEMS->uDelayStandby_4Chrg)
		{
			if (pBessInfo->bStandby_AEMS)
			{
				varV.emValue = FALSE;
				Push_VarValue(SETTYPE_INTERN_CTRL, pAEMS->hSampler, 200, 301, varV);
			}
		}
	}

#ifdef LOG_FILEDEBUG
    static time_t stmLogFile;
	static PV_CRTL_MODE CtrlMode;
	static BOOL bBackflow;
	static float fTotalPVOutputActPower;
	float fCtrlPwr_Diff = ABS(pPvCtrl->fActivePower - pPvInfo->fRecvActivePower);
	float fOutPwr_Diff  = ABS(fTotalPVOutputActPower - pPvInfo->fTotalPVOutputActPower);
	// unit test
	// LOGFD("aems_main:Sample writelog:%d %d %d %d %d ElapseS:%u fCtrlPwr_Diff:%.1f fOutPwr_Diff:%.1f",
	// 	ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE, CtrlMode != pPvCtrl->CtrlMode, bBackflow != (pBessInfo->f_PgridMeterSum<0),
	// 	fCtrlPwr_Diff>MAX_CtrlPwr_Diff, fOutPwr_Diff>pAEMS->uPStep_PV,
	// 	ElapseS(stmLogFile), fCtrlPwr_Diff, fOutPwr_Diff);
	if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE || CtrlMode != pPvCtrl->CtrlMode
		|| bBackflow != (pBessInfo->f_PgridMeterSum<0) || fCtrlPwr_Diff>MAX_CtrlPwr_Diff || fOutPwr_Diff>pAEMS->uPStep_PV)
    {
        stmLogFile = _NOW_;
		CtrlMode = pPvCtrl->CtrlMode;
		bBackflow = (pBessInfo->f_PgridMeterSum<0);
		fTotalPVOutputActPower = pPvInfo->fTotalPVOutputActPower;
		LOGFD("pv aems_main:Sample CtrlMode:%d fActivePower:%.1f %.1f fTotalPVOutputActPower:%.1f "
			"f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f fMax_Chrg_PwrAbility_AEMS:%.1f "
			"bAutoChrgEnable:%d fMax_Chrg_PwrAbilitySumA:%.1f | %d %.1f "
			"fPBackflow:%.1f fPLoad_AndBess:%.1f fPLoad_AC:%.1f=fTotalPVOutputActPower:%.1f+fPacload4DischgSum:%.1f "
			"emACDC_WorkStatuSum:%d fRemainChrgAbl_Bess:%.1f fProtBackflow_PV_AEMS:%.1f "

			// 2.1 shutdown // 2.2 PwrDown 逆流
			"2.1:shutdown:%d 2.2:PwrDown:%d <ProtBackflow:%d && bPwrDown_PV:%d "
			"[msBackflow:%d && (BESS停止:%d || BESS无法消纳逆流功率:%d || 逆流太大:%d)] || 逆流太久:%d "
			// 3.1 PwrUpAll 全额放电 // 3.2 PwrUpStep 往上步调
			"3.1:PwrUpAll:%d [负载很大:%d || 能消纳(emACDC_WorkStatuSum!IDLE:%d && %d)] 3.2:bPwrUpStep_PV:%d=(ElapseMS:%d && 防逆流保护值内:%d) "
			// 11.bStandby_AEMS
			"bStandby_AEMS:%d bNeedStandby:%d=[%d && %d && (%d || %d || %d)] "
			// others
			" fTotalPVPowerAbility:%.1f bEnableManualMode:%d RecvCtrlMode:%d nRecvPVStatus:%u "
			"nAvailablePVNums:%u fTotalPVInput:%.1f fDailyPowerGenerationSum:%.1f fTotalPowerGenerationSum:%.1f "
			"fPVDCTotalPowerGenerationSum:%.1f iTotalRatedPower:%d emPvRunStatus:%d 2:normal ",

			pPvCtrl->CtrlMode, pPvCtrl->fActivePower, pPvInfo->fRecvActivePower, pPvInfo->fTotalPVOutputActPower,
			pBessInfo->f_PgridMeterSum, pBessInfo->f_PcabMeterSum, pAEMS->fMax_Chrg_PwrAbility_AEMS,
			pAEMS->bAutoChrgEnable, pBessInfo->fMax_Chrg_PwrAbilitySumA, pBessInfo->emACDC_WorkStatuSum, pBessInfo->fMax_Chrg_PwrAbilitySum,
			pAEMS->fPBackflow, pAEMS->fPLoad_AndBess, pAEMS->fPLoad_AC, pPvInfo->fTotalPVOutputActPower, pBessInfo->fPacload4DischgSum,
			pBessInfo->emACDC_WorkStatuSum, pAEMS->fRemainChrgAbl_Bess, pAEMS->fProtBackflow_PV_AEMS,

			// 2.1 shutdown
			pAEMS->fPLoad_AndBess < pAEMS->fProtBackflow_PV_AEMS,
			// 2.2 PwrDown 逆流
			(pBessInfo->f_PgridMeterSum < pAEMS->fProtBackflow_PV_AEMS) && pAEMS->bPwrDown_PV,
			pBessInfo->f_PgridMeterSum < pAEMS->fProtBackflow_PV_AEMS, pAEMS->bPwrDown_PV,
			pAEMS->msBackflow > MAX_MS_Backflow, pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_IDLE // BESS停止
			, !pAEMS->bChrgAbl_Bess // BESS无法消纳逆流功率
			, pAEMS->fPBackflow > pAEMS->uMaxPBackflow // 逆流太大
			, (pAEMS->fPBackflow > 0 && pAEMS->msBackflow > pAEMS->uMaxMsBackflow), // 逆流太久
			// 3.1 PwrUpAll 全额放电
			((float)pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AC || (pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_IDLE && pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AndBess)),
			(float)pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AC, // 负载很大 PV额定功率 < 负载
			pBessInfo->emACDC_WorkStatuSum != STATU_ACDC_IDLE, pPvInfo->iTotalRatedPower < pAEMS->fPLoad_AndBess, // 能消纳 已经开机并且PV额定功率 < 总消纳功率
			// 3.2 PwrUpStep 往上步调
			pAEMS->bPwrUpStep_PV,
			ElapseMS(pPV->dtmRetry_Last) > pAEMS->uMsStep_PV,
			(pBessInfo->f_PgridMeterSum > (pAEMS->fProtBackflow_PV_AEMS + (float)pAEMS->uPStep_PV/2)), // 防逆流保护值内
			// 11.bStandby_AEMS
			pBessInfo->bStandby_AEMS, bNeedStandby, pPvInfo->fTotalPVPowerAbility > MIN_PWR_LMT, pAEMS->fMax_Chrg_PwrAbility_AEMS > MIN_PWR_CHRG_4START,
			pPvInfo->fTotalPVOutputActPower >= pAEMS->fPLoad_AC, pBessInfo->fPacload4DischgSum < pAEMS->fLtPwr2Standby_4Chrg,
			pBessInfo->emACDC_WorkStatuSum == STATU_ACDC_INVERTER,
			// others
			pPvInfo->fTotalPVPowerAbility, pPvInfo->bEnableManualMode, pPvInfo->RecvCtrlMode, pPvInfo->nRecvPVStatus,
			pPvInfo->nAvailablePVNums, pPvInfo->fTotalPVInput, pPvInfo->fDailyPowerGenerationSum, pPvInfo->fTotalPowerGenerationSum,
			pPvInfo->fPVDCTotalPowerGenerationSum, pPvInfo->iTotalRatedPower, pPvInfo->emPvRunStatus
		);
	}
#endif
	return ERR_SAMPLE_OK;

}