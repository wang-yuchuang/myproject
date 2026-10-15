// #define _TRACE_PRINT_ // batt
#include "st_batt_sampler.h"
// #define TRACE TRACED
// #define TRACEX TRACED

#define GET_BIT(num, n) (((num) >> (n)) & 1)
#define GET_VAL(num, shift, mask) (((num) >> (shift)) & (mask))
//For 285AH的CATL电芯 SOC校准
static const VoltageSOCMap ChargeSOCMap[] =
{
    {2884, 0}, 
	{3190, 500}, 
	{3224, 1000}, 
	{3251, 1500}, 
	{3277, 2000},
	{3295, 2500},
	// {3303, 3000},
	// {3304, 3500},
	// {3305, 4000},
	// {3306, 4500},
	// {3307, 5000},
	// {3309, 5500},
	// {3316, 6000},
	// {3338, 6500},
	// {3340, 7000},
	// {3340, 7500},
	// {3340, 8000},
	// {3340, 8500},
	// {3340, 9000},
    {3339, 9500}, 
	{3475, 10000}
};
static const VoltageSOCMap DisChargeSOCMap[] = 
{
    {2884, 0   }, 
	{3177, 500 }, 
	{3208, 1000}, 
	{3230, 1500}, 
	{3250, 2000},
	{3265, 2500},
	{3283, 3000},
	// {3289, 3500},
	// {3290, 4000},
	// {3291, 4500},
	// {3293, 5000},
	// {3295, 5500},
	// {3302, 6000},
	// {3326, 6500},
	// {3329, 7000},
	// {3330, 7500},
	// {3330, 8000},
	// {3331, 8500},
	// {3332, 9000},
    {3332, 9500}, 
	{3516, 10000}
};

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	TRACE("STBATT: emEvType=[%d] emMsgType=[%d] nEvDataSize=[%d]\n", emEvType, emMsgType, nEvDataSize);

	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = (STBATT_SAMPLER_ROUGH*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
 	ERRCDDE_DPR err;
	
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if (pCfgChange->parmPTR.nId == 1 && (pCfgChange->parmPTR.nSubId == 85))
		{
			// LJH250930 加快自动充放响应,standbySetReq
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 85: // bStandby_Set 1, 85,
				pSTBatt_Rough->bStandby_Set = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			default:
				break;
			}
			TRACED("st_batt_sampler::Event_Handler nId=1 nSubId:%d err:%d bStandby_Set:%d",
				pCfgChange->parmPTR.nSubId, err, pSTBatt_Rough->bStandby_Set);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 201))
		{
 			varV.blobValue = (BYTE *)&pSTBatt_Rough->blobEmsCfgPlan;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 218))
		{
			pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, 200, 218,  &varV, 4);
			pSTBatt_Rough->emSTSAutoManual = (STS_AUTOMANUAL_ONOFFGRID)varV.emValue;
			TRACE("EvtHandle:emSTSAutoManual=[%d]\n", pSTBatt_Rough->emSTSAutoManual);
		}
		else if ((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 300 || pCfgChange->parmPTR.nSubId == 301))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 300: // PID Control Enable
				pSTBatt_Rough->emPIDEnableState = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
				err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
						pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId + 1, &varV, 4);
				pSTBatt_Rough->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
				if (pSTBatt_Rough->emPIDEnableState>POWER_CTRL_DISABLE)
				{
					pSTBatt_Rough->emHighSensitivityEnable = pSTBatt_Rough->emPIDEnableState | pSTBatt_Rough->emHighSensitivityEnable;
				}
				break;
			case 301: // High Sensitivity Enable
				pSTBatt_Rough->emHighSensitivityEnable = (err==ERR_DPR_OK) ? varV.emValue : POWER_CTRL_DISABLE;
				pSTBatt_Rough->emHighSensitivityEnable = pSTBatt_Rough->emPIDEnableState | pSTBatt_Rough->emHighSensitivityEnable;
				break;
			default:
				break;
			}

			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if (pCfgChange->parmPTR.nId == 1001 &&
			(pCfgChange->parmPTR.nSubId == 3303 || pCfgChange->parmPTR.nSubId == 3306 || pCfgChange->parmPTR.nSubId == 3309))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 3303: // LJH250408 CtrlByREMS 1001, 3303,
				pSTBatt_Rough->enableREMSCtrl = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3306: // uChargeStopSoc(REMS) 1001, 3306, 
				pSTBatt_Rough->nStopSOC_ChrgSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 100);
				break;
			case 3309: // uDischargeStopSoc(REMS) 1001, 3309, 
				pSTBatt_Rough->nStopSOC_DiscSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 0);
				break;
			default:
				break;
			}
			TRACED("st_batt_sampler::Event_Handler nId=1001 nSubId:%d err:%d "
				"enableREMSCtrl:%d nStopSOC_ChrgSet_SEMS:%d %d",
				pCfgChange->parmPTR.nSubId, err,
				pSTBatt_Rough->enableREMSCtrl, pSTBatt_Rough->nStopSOC_ChrgSet_SEMS, pSTBatt_Rough->nStopSOC_DiscSet_SEMS);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 1 && (pCfgChange->parmPTR.nSubId == 93)))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			pSTBatt_Rough->nRunRecordInterval = (err==ERR_DPR_OK) ? (int)varV.fValue : -1;
		}
		/*
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			TRACE("stBatt update OCPP cfg\n");
			//OCPP setting has changed
			varV.blobValue = (BYTE *)&pSTBatt_Rough->blobESiOCPPCfgs;
			//copy setting to current data
			ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 26, &varV, sizeof(A_BLOB_ES_iOCPP_CONFIGURAION));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}*/
	}
	else if(emEvType == EVENT_TYPE_BMSRUNDATA_UPDATE) //restore Bms RealTime Runinfo
	{// EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_RESTORE
		TRACE("STBATT:--now received restore event for EVENT_TYPE_BMSRUNDATA_UPDATE\n");
		if((nEvDataSize != sizeof(EVENT_BMSRUNDATA_UPDATE)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			printf("nEvDataSize=%d EVENT_BMSRUNDATA_UPDATE=%d\n", nEvDataSize, sizeof(EVENT_BMSRUNDATA_UPDATE));
			return EVENT_HANDED_FAILED;
		}

		EVENT_BMSRUNDATA_UPDATE* pBmsSave = (EVENT_BMSRUNDATA_UPDATE*)pEvData;
		HIS_BMS_RUNTIMEDATA_REC* pBmsRunDataRec = pBmsSave->pBmsRunDataRec;

		printf("------now received restore event for EVENT_BMSRUNDATA_UPDATE-----\n");

		if(pBmsRunDataRec != NULL)
		{
			// memcpy(&pSTBatt_Rough->hisBmsRunDataRec, pBmsRunDataRec, sizeof(HIS_BMS_RUNTIMEDATA_REC));
			pSTBatt_Rough->hisBmsRunDataRec.bmsRunData.nBmsRuntimeDataTotal = 0;
			// printf("STBatt Sampler: Restore BMS RealTime RunInfo From DB\n");
			
			SAFELY_DELETE(pBmsRunDataRec);
		}
	}
	else if(emEvType == EVENT_TYPE_HEALTHDATA_UPDATE) //20220519 John Added For Health Restore
	{
		TRACE("STBATT:--now received restore event for EVENT_TYPE_HEALTHDATA_UPDATE\n");
		if((nEvDataSize != sizeof(EVENT_BMSHEALTH_UPDATE)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			printf("Err: nEvDataSize=%d EVENT_BMSHEALTH_UPDATE=%d\n", nEvDataSize, sizeof(EVENT_BMSHEALTH_UPDATE));
			return EVENT_HANDED_FAILED;
		}

		EVENT_BMSHEALTH_UPDATE* pHealthSave = (EVENT_BMSHEALTH_UPDATE*)pEvData;
		HIS_HEALTH_STATIC_DAILY* pHisHealthRec = pHealthSave->pHisBmsHealthRec;

		printf("------now received restore event for EVENT_BMSHEALTH_UPDATE-----\n");

		if(pHisHealthRec != NULL)
		{
			memcpy(&pSTBatt_Rough->hisHealthRec, pHisHealthRec, sizeof(HIS_HEALTH_STATIC_DAILY));

			printf("STBatt Sampler: Restore BMS Health Daily Data From DB Success!\n");
			
			DELETE(pHisHealthRec);
		}

	}

	return EVENT_HANDED_OK;
}


/*********************************************************************************
*  
*  FUNCTION NAME : isBMSRunAbnormal
*  CREATOR       : John
*  CREATED DATE  : 2021-11-1 11:13:53
*  DESCRIPTION   : According To CCU Comm Status And Insulation Status
*  
Step1:PCS System Fault->PCS Internal Disconnect AC-> EMS Ctrl Disconnect DC Relay->
	Waiting PCS System Recover Normal->Delay 10s  Connect DC Relay;
If After Step1,PCS Always Alarm，Repeat Step1, Try 1 times not Success, Disconnect DC Realy,
	Not Connect Anymore, Keep DC Realy Disconnect。

Leakage Current Detection Alarm Need Dsiconnect Relay

*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL isBMSRunAbnormal(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, float fPackTotalCurr) 
{
	BOOL bRet = FALSE;
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

	if((nBattIdx < 0) || (nBattIdx > MAX_SINGLE_STBATT_NUM))
	{
		TRACE("nBattIdx=[%d] is Out of Range !\n", nBattIdx);
		return bRet;
	}

	pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, 6, &varV, 4); //CCU Comm Status

	if(varV.emValue == 1)
	{
		if(fabs(fPackTotalCurr) > 2.0) //CCU CommFail, But PM Has Power Output
		{
			pSTBatt_Rough->nCCUAbnormalTimes++;
			if(pSTBatt_Rough->nCCUAbnormalTimes > MAX_BMS_ABNORMAL_TIMES)
			{
				pSTBatt_Rough->nCCUAbnormalTimes = MAX_BMS_ABNORMAL_TIMES;
				bRet = TRUE;
			}
		}
	}
	else
	{
		pSTBatt_Rough->nCCUAbnormalTimes = 0;
	}

	//Get IMIU Isolution Alarm
	UINT nSigID  =  273 + nBattIdx*2;
	pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, nSigID, &varV, 4); 
	if(varV.emValue == 1)
	{
 	
		pSTBatt_Rough->nInsolutionAlmTimes++;
		if(pSTBatt_Rough->nInsolutionAlmTimes > MAX_BMS_ABNORMAL_TIMES)
		{
			pSTBatt_Rough->nInsolutionAlmTimes = MAX_BMS_ABNORMAL_TIMES;
			bRet = TRUE;
		}
	}
	else
	{
		pSTBatt_Rough->nInsolutionAlmTimes = 0;
	}

	//Get PCS PM Fault Alarm
	nSigID  =  394 + nBattIdx;
	pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, nSigID, &varV, 4); 
	if(varV.emValue == 1)
	{
		pSTBatt_Rough->nPCSFaultTimes++;
		if(pSTBatt_Rough->nPCSFaultTimes > MAX_BMS_ABNORMAL_TIMES)
		{
			pSTBatt_Rough->nPCSFaultTimes = MAX_BMS_ABNORMAL_TIMES;
			bRet = TRUE;
		}
	}
	else
	{
		if(pSTBatt_Rough->nPCSFaultTimes > 0)
		{
			pSTBatt_Rough->nPCSFaultTimes--; //When PCS Fault Alm Accur, Need 10S Delay to Connect DC Realy.
			bRet = TRUE;
			TRACE("nBattIdx=[%d] PCSFaultTimes=[%d]\n", nBattIdx, pSTBatt_Rough->nPCSFaultTimes);
		}
 	}

	//Leakage Current Detection Alarm Need Disconnect DC Relay,Suggest By WFY
	//pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 1, 40, &varV, 4); //Leakage Current Abnormal
	//if(varV.emValue == 1)
	//{
	//	pSTBatt_Rough->nLeakageCurrAlmTimes++;
	//	if(pSTBatt_Rough->nLeakageCurrAlmTimes > MAX_BMS_ABNORMAL_TIMES)
	//	{
	//		pSTBatt_Rough->nLeakageCurrAlmTimes = MAX_BMS_ABNORMAL_TIMES;
	//		bRet = TRUE;
	//	}
	//}
	//else
	//{
	//	pSTBatt_Rough->nLeakageCurrAlmTimes = 0;
	//}

	return bRet;
}

//static void OnHandle_STSRunStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
//{
//	if(pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == STATE_OFFGRID)
//	{
//		
//	}
//	//Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 200, 220 + n, varV);
//
//}

BOOL JudgeIfPCS_OnHotWaiting(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	// BOOL bRet = FALSE;
	time_t 	tmNow = time(NULL);

	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

	// pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 85, &varV, 4); //Hot stand-by mode   0:enable  1:disable
	// if (1 == varV.emValue)
	// {
	// 	return FALSE;
	// }

	// LJH250930 加快自动充放响应,standbySetReq
	ERRCDDE_DPR err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1001, 3800, &varV, 4);
	BOOL bStandby_BLL = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	if ( !pSTBatt_Rough->bStandby_Set && !bStandby_BLL)
	{
		TRACED("JudgeIfPCS_OnHotWaiting return FALSE; bStandby_Set:%d bStandby_BLL:%d", pSTBatt_Rough->bStandby_Set, bStandby_BLL);
		return FALSE;
	}	

	// if(pSTBatt_Rough->emPMCtrlMode == MANUAL_CTRL_PM_MODE)
	// {
	// 	return bRet;
	// }
	
	if(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_Max_Output_Pwr_Ability < 1.0) 
	{
		//TRACE("nBattIdx=[%d] fACDC_Max_Output_Pwr_Ability=[%.1f]\n", \
		//	nBattIdx ,pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_Max_Output_Pwr_Ability);
		TRACED("JudgeIfPCS_OnHotWaiting return FALSE; fACDC_Max_Output_Pwr_Ability < 1.0");
		return FALSE;
	}
	
	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt < (pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtDisChargeVolt+10.0))
	{
		TRACED("JudgeIfPCS_OnHotWaiting return FALSE; fPackTotalVolt < %.1f", pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt);
		return FALSE;
	}
	
	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxDisChargeCurr < 1.0)
	{
		TRACED("JudgeIfPCS_OnHotWaiting return FALSE; fAllow_MaxDisChargeCurr < 1.0");
		return FALSE;
	}
	
	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].bIsBatt_DisChargeForbidden == TRUE)
	{
		TRACED("JudgeIfPCS_OnHotWaiting return FALSE; bIsBatt_DisChargeForbidden == TRUE");
		return FALSE;
	}

	//When STS is Off-Grid, But PCS is PowerOff, Need Power ON PCS.
	/*if(pSTBatt_Rough->emSTSAutoManual == MANUAL_ONOFFGRID_MODE)
	{
		//printf("emSTSAutoManual=[%d]\n", pSTBatt_Rough->emSTSAutoManual);
		if(pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == 0) //0-On-Grid, 1-Manual
		{
			return bRet;
		}
	}

	if(pSTBatt_Rough->esRunData.mapSTSCode.Status.FaultState == 1)
	{
		return bRet;
	}*/

	// LJH250930 加快自动充放响应,StopSOC_SetSEMS 截止SOC停止待机
	if(pSTBatt_Rough->emESSDevType <= LIQUID_COOLING)
	{
		if (pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING &&
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC >= pSTBatt_Rough->nStopSOC_ChrgSet_SEMS)
		{
			TRACEW("JudgeIfPCS_OnHotWaiting return FALSE; BATT_WORKMODE_FIRST_CHRGING nSOC:%d >= nStopSOC_ChrgSet_SEMS:%d",
				pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC, pSTBatt_Rough->nStopSOC_ChrgSet_SEMS);
			return FALSE;
		}
		if (pSTBatt_Rough->calcNodeData[nBattIdx].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG &&
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC <= pSTBatt_Rough->nStopSOC_DiscSet_SEMS)
		{
			TRACEI("JudgeIfPCS_OnHotWaiting return FALSE; BATT_WORKMODE_FIRST_CHRGING nSOC:%d >= nStopSOC_DiscSet_SEMS:%d",
				pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC, pSTBatt_Rough->nStopSOC_DiscSet_SEMS);
			return FALSE;
		}
	}
	TRACED("JudgeIfPCS_OnHotWaiting return TRUE");
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge If Need Enter OffGrid
*  CREATOR       : John
*  CREATED DATE  : 2023-04-11 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL JudgeIfNeedEnterOffGrid(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
    BOOL bRet = FALSE;

	//if(pSTBatt_Rough->emSTSAutoManual == MANUAL_ONOFFGRID_MODE)
	//{
	//	printf("emSTSAutoManual=[%d]\n", pSTBatt_Rough->emSTSAutoManual);
	//	return bRet;
	//}

    if(pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == STATE_STS_OFFGRID)
    {
		TRACE("emSTS_OnOffGridStatu=%d\n",\
        	pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State);
        return TRUE;
    }

    return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CtrlBattCabinetContactor
*  CREATOR       : John
*  CREATED DATE  : 2021-11-1 11:13:53
*  DESCRIPTION   : Change To IMSU Ctrl DC Input Contactor .0:FALSE DISCONNECTED, 1:TRUE CONNECTED
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CtrlBattCabinetContactor(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n, BOOL bCtrlDOClose)
{
	//TRACE("CtrlBattCabinetContactor: BattIDx=[%d] bCtrlOutRelayClose=%d\n",n, pSTBatt_Rough->bCtrlOutRelayClose[n]);
	time_t 		tmNow = time(NULL);
	VAR_VALUE	varV;
	//A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];

	//Added By John.22020602.Manual Mode && ON-Grid,When Ctrl Disconnect Not Auto Connnect.
	if((pSTBatt_Rough->emPMCtrlMode == MANUAL_CTRL_PM_MODE) 
		&& (pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_ONGRID)
		&& (bCtrlDOClose == TRUE))
	{
		return;
	}

	//John 2024-04-23 Added For EPO Pressed,Not CtrlDOClose,2~4 Relay powerON at some Time cause AuxPower PowerOff.
	if((pSTBatt_Rough->esRunData.emEPO == 1)
		&& (bCtrlDOClose == TRUE))
	{
		return;
	}

	if((n < 0) || (n > MAX_SINGLE_STBATT_NUM))
	{
		TRACE("Error BMS Index=[%d]\n", n);
		return;
	}

	if(tmNow - pSTBatt_Rough->ulLastCtrlTime[n] < 30)
	{
		pSTBatt_Rough->nCtrlTimes[n]++;
	}
	else
	{
		pSTBatt_Rough->nCtrlTimes[n] = 0;
	}
	
	if(pSTBatt_Rough->nCtrlTimes[n] >= BMS_RETRY_POWERON_TIMES)
	{
		pSTBatt_Rough->nCtrlTimes[n] = BMS_RETRY_POWERON_TIMES;
		return;
	}
	
	if((pSTBatt_Rough->emPMCtrlMode == MANUAL_CTRL_PM_MODE) && 
		(bCtrlDOClose == FALSE) &&
		(pSTBatt_Rough->branchGrpData[n].emACDC_WorkStatu != STATU_ACDC_IDLE) &&
		(fabs(pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr) > 10.0))
	{	
		TRACE("Info:Manaul Ctrl->Idx=[%d]fACDC_RealTime_Curr=[%.1f]\n", n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr);
		//Manual Ctlr Mode Protect Logic
		pSTBatt_Rough->blobManualCtrlPM.emState   = 0; 				//0-means PM Stop Work 
		pSTBatt_Rough->blobManualCtrlPM.nGrp = n + 1; 				//0-means All PM Grp
		pSTBatt_Rough->blobManualCtrlPM.fGrpVolt = 0;
		pSTBatt_Rough->blobManualCtrlPM.fGrpCurr = 0;
		pSTBatt_Rough->blobManualCtrlPM.nStopSOC = pSTBatt_Rough->nStopSOC[n];

		varV.blobValue = (BYTE *)&pSTBatt_Rough->blobManualCtrlPM;

		Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
					200, 214, (void*)varV.blobValue, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
		
		Sleep(2000); //Wait 2S
	}

	/*if(bCtrlDOClose == FALSE) //John , Before OFF,Need Stop PM Output 
	{
		if(pSTBatt_Rough->branchGrpData[n].emACDC_WorkStatu != STATU_ACDC_IDLE)
		{
			printf("Ctrl OFF Begin:tmNow=[%ld] nBattIdx=[%d] fACDC_OutVolt=[%.1f] fACDC_RealTime_Curr=[%.1f] fPackTotalCurr=[%.1f]\n", \
				tmNow, n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Volt, \
				pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr, \
				pSingleRunInfo->fPackTotalCurr );

			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, n);
			Sleep(2000); //Wait 2S

			printf("Ctrl OFF End: tmNow=[%ld] nBattIdx=[%d] fACDC_RealTime_Volt=[%.1f] fACDC_RealTime_Curr=[%.1f] fPackTotalCurr=[%.1f]\n", \
				tmNow, n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Volt, \
				pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr, \
				pSingleRunInfo->fPackTotalCurr );
		}
	}
	else
	{
		//John, Avoid 100A Suddenly Current Cause Contactor damage.
		if(ABS(pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Volt - pSingleRunInfo->fPackTotalVolt) > 50) //Offset 50V
		{
			printf("Ctrl ON Begin:tmNow=[%ld] nBattIdx=[%d] fACDC_OutVolt=[%.1f] fACDC_RealTime_Curr=[%.1f] fPackTotalCurr=[%.1f]\n", \
				tmNow, n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Volt, \
				pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr, \
				pSingleRunInfo->fPackTotalCurr );

			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_RECTIFY_ADJ_CURR, n); //For PreChrge

			Sleep(2000); //Wait 2S

			printf("Ctrl ON End:tmNow=[%ld] nBattIdx=[%d] fACDC_RealTime_Volt=[%.1f] fACDC_RealTime_Curr=[%.1f] fPackTotalCurr=[%.1f]\n", \
				tmNow, n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Volt, \
				pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr, \
				pSingleRunInfo->fPackTotalCurr );
			
			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, n);
		}
	}*/
	//printf("Info:->Idx=[%d]fACDC_RealTime_Curr=[%.1f]\n", n, pSTBatt_Rough->branchGrpData[n].fACDC_RealTime_Curr);

	varV.emValue = bCtrlDOClose;
	Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 200, 220 + n, varV);

	TRACE("Info: BattIDx=[%d] bCtrlOutRelayClose=%d ulLastCtrlTime=[%ld] tmNow=[%ld]\n", \
		n ,pSTBatt_Rough->bCtrlOutRelayClose[n], pSTBatt_Rough->ulLastCtrlTime[n], tmNow);

	pSTBatt_Rough->ulLastCtrlTime[n] = tmNow;
	 
	return;
} 

#define INIT_ST_STATE_MACHINE(p, _emState, _pfnOnAct) \
					((p)->emState = (_emState), \
					(p)->pfnAction = (_pfnOnAct), \
					(p)++)
void Initial_AutoMode_MachineState(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;

	//Note: do NOT chage the order!!!/**/
	STSTATE_MACHINE_ITEM *p = &pSTBatt_Rough->stateMachs[0];
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_IDLE,					OnIdle);
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_WAIT_CHARGE,			OnWaitCharge);
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_IN_CHARGING,			OnCharging);
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_IN_DISCHARGING,		OnDischarging);
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_FAULTED,				OnFaulted);
	INIT_ST_STATE_MACHINE(p,	STBATT_STATE_OFFGRID,				OnOffGrid);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->singleBattData[n].pCurrState = &pSTBatt_Rough->stateMachs[STBATT_STATE_IDLE];
		pSTBatt_Rough->singleBattData[n].pLastState = pSTBatt_Rough->singleBattData[n].pCurrState;

		pSTBatt_Rough->nStuffTimes[n] = 0;
		pSTBatt_Rough->emISOTriggerStatu[n] = TRIG_NO_TRIGGER;

		//----- BMS CAN Msg Data Save -----//
		pSTBatt_Rough->stuCanSave.stuStatMach[n].emSaveState = STAT_ON_START;
		pSTBatt_Rough->stuCanSave.stuStatMach[n].tm_LastStatEnter= _NOW_;
		pSTBatt_Rough->stuCanSave.emTrigMsgTp[n] = TP_TRIG_SUSPEND;
	}

	return;
}

static BOOL	Init_blobBmsAutoTestRange(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	printf("enter to init test param range!\n");
	// //Now get the bms test param range
	// int 					n = 0;
	// VAR_VALUE 				varV;
	// BOOL 					bNeedUpdtParamRange = FALSE;
	// //Now use the default plan
	// A_BLOB_BMS_TESTMODE_SET bmsRange;
	// ZERO_POBJS(&bmsRange,	1);

	// varV.blobValue = (BYTE *)&pSTBatt_Rough->bmsTestModeParm;
	// pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 202, &varV, sizeof(A_BLOB_BMS_TESTMODE_SET));
	// printf("after get data!\n");

	// -----Selfcheck-----
	// pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTotalVlot_Max = 0.0;
	// pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTotalVlot_Min = 0.0; // remove or not?
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTotalAddVolt_Gap = 1000;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolCell_Num = 240;// 风冷一体柜和液冷一体柜均为240
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num = 240;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMean_Max = 3600;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMean_Min = 2500;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMean_Max = 40;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMean_Min = 5;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMax_Max = 3600;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMax_Min = 2500;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMax_Max = 45;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMax_Min = 15;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMin_Max = 3600;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVolMin_Min = 2500;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMin_Max = 35;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempMin_Min = 5;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nVol_Gap = 1000;// unused
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTemp_Gap = 15;// unused

	// -----Charge-----
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndTemp_Max = 40;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndTemp_Min = -10;
    if(pSTBatt_Rough->lcCfg.emSystemType == SYSTYPE_ESS_AIR)//xsy
    {
        pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndTemp_Gap = 8;// 风冷8℃
    }
    else
    {
        pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndTemp_Gap = DTEST_TEMP_GAP;
    }	
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndVol_Max = 3595;////避免错过采集高于3600mv的数据
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEnd_TotalVolMax = 3.55*A_CABINET_CELL_MAX_NUM; // 3.55V * 240 = 852V
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndVol_Gap = 200;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEndPlateauVol_Gap = 50;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_fChargeEndRes_Gap = 0.05;
	// pSTBatt_Rough->bmsTestModeParm.initRange.TH_nChargeEnd_Energy = 0.0; // same

	// -----Discharge-----
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Max = 40;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Min = -20;
    if(pSTBatt_Rough->lcCfg.emSystemType == SYSTYPE_ESS_AIR)//xsy
    {
        pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Gap = 8;// 风冷8℃
    }
    else
    {
        pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndTemp_Gap = DTEST_TEMP_GAP;
    }	
	// pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min = 2900;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndVol_Min = 2905;//避免错过采集低于2900mv的数据
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEnd_TotalVolMin = 2.9*A_CABINET_CELL_MAX_NUM; // 2.9V * 240 = 696V
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndVol_Gap = 200;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEndPlateauVol_Gap = 50;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_fDischargeEndRes_Gap = 0.05;
	// pSTBatt_Rough->bmsTestModeParm.initRange.TH_nDischargeEnd_Energy = 0.0; // same

	// -----TotalTest-----
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nPower_Efficiency = DTEST_POW_EFF;
	pSTBatt_Rough->bmsTestModeParm.initRange.TH_nSystem_Efficiency = DTEST_SYS_EFF;

	// Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSTBatt_Rough->hSampler, 500, 202,
	// 		(void*)&bmsRange, sizeof(A_BLOB_BMS_TESTMODE_SET), BLOB_TYPE_BMS_TESTMODE_SET);
	// printf("after push data\n");
	// memcpy(&pSTBatt_Rough->bmsTestModeParm, &bmsRange, sizeof(A_BLOB_BMS_TESTMODE_SET));
	// printf("after memcpy\n");
	VAR_VALUE varV;
    varV.blobValue = (BYTE *)&pSTBatt_Rough->bmsTestModeParm;
    // printf("----------------before push----------------\n");
    int nRet = Push_BlobValue(SETTYPE_INTERN_SETT, pSTBatt_Rough->hSampler, 
		500, 202, (void*)varV.blobValue, sizeof(A_BLOB_BMS_TESTMODE_SET), BLOB_TYPE_BMS_TESTMODE_SET);
	
	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME : OnIdle
*  CREATOR       :  
*  CREATED DATE  : 2020-10-19 14:33:53
*  DESCRIPTION   :  
*
*  CHANGE HISTORY: 
*
***********************************************************************************/
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	printf("--------------------------------------------------------------------!!!!!!!!!!!!!!\n");
 	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	pSampler->pDataHeap = (void*)NEW(STBATT_SAMPLER_ROUGH, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
 
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = (STBATT_SAMPLER_ROUGH*)pSampler->pDataHeap;
	ZERO_POBJS(pSTBatt_Rough, 1);

	pSTBatt_Rough->hSampler = hSamplerSelf;
	pSTBatt_Rough->nSamplerId = pSampler->info.nSamplerID;
	pSTBatt_Rough->hSamplerLock = pSampler->hSamplerLock;
	pSTBatt_Rough->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	pSTBatt_Rough->hCAN1Write_Mutex = Mutex_Create(TRUE);

	//varInvalid.lValue = SAMP_INVALID_VALUE;
	pSTBatt_Rough->t_FirstBattBoot = _NOW_;

	VAR_VALUE varV;
	ERRCDDE_DPR err;

	varV.emValue = 0;
	Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 81, varV);

	Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 83, varV);


	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 500, &varV, 4);
	pSTBatt_Rough->emBattMFG = (BATT_MANUFACTURER)varV.emValue;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 502, &varV, 4);
	pSTBatt_Rough->nSystemCfgBattNum = varV.emValue;
	pSTBatt_Rough->nSystemCfgBattNum = MAX(0, pSTBatt_Rough->nSystemCfgBattNum);
	//printf("nSystemCfgBattNum=[%d]\n", pSTBatt_Rough->nSystemCfgBattNum);

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 503, &varV, 4);
	pSTBatt_Rough->nProtectSOC = varV.ulValue;
	pSTBatt_Rough->nBackupProtectSOC = varV.ulValue;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 508, &varV, 4);
	pSTBatt_Rough->fISO_PeriodMin_Sett = varV.fValue;
	
	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 509, &varV, 4);
	pSTBatt_Rough->emDisChgByStopSOC = varV.emValue;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 510, &varV, 4);
	pSTBatt_Rough->emFullyChgDischgMode = varV.emValue;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 200, 218,  &varV, 4);
	pSTBatt_Rough->emSTSAutoManual = (STS_AUTOMANUAL_ONOFFGRID)varV.emValue;

	// LJH250408 CtrlByREMS
	err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1001, 3303, &varV, 4);
	pSTBatt_Rough->enableREMSCtrl = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	pSTBatt_Rough->lcCfg = GetLCCfg_Sampler(pSTBatt_Rough->hSampler); // LJH250508 LCCfg
	// LJH250930 加快自动充放响应,StopSOC_SetSEMS
	err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1001, 3306, &varV, 4);
	pSTBatt_Rough->nStopSOC_ChrgSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 100);
	err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1001, 3309, &varV, 4);
	pSTBatt_Rough->nStopSOC_DiscSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 0);
	// LJH250930 加快自动充放响应,standbySetReq
	err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1, 85, &varV, 4);
	pSTBatt_Rough->bStandby_Set = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
	
	pSTBatt_Rough->executeChargingWhenReflux = FALSE;

	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 501 + n, 507, &varV, 4);
		pSTBatt_Rough->nStopSOC[n] = varV.ulValue;

		pSTBatt_Rough->t_LastTrigIsoStartCmd[n] = _NOW_;
		pSTBatt_Rough->tm_LastStartChg_4STBatt[n] = _NOW_;

		pSTBatt_Rough->fLastSendReqVolt_2CCU[n] = 0;
    	pSTBatt_Rough->fLastSendReqCurr_2CCU[n] = 0;
		pSTBatt_Rough->emLastCtrlPcsCmd[n] = GRP_ACDC_STOP;

		pSTBatt_Rough->fSingleChargeEnergy[n] = 0; // tmp solution 20240715
		pSTBatt_Rough->fSingleDischargeEnergy[n] = 0; // tmp solution 20240715
	}

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 501, &varV, 4);
	pSTBatt_Rough->nPVSysBattRetSOC = varV.ulValue;

	pSTBatt_Rough->nCCUAbnormalTimes = 0;
	
	pSTBatt_Rough->nInsolutionAlmTimes = 0;

	pSTBatt_Rough->nPCSFaultTimes = 0;

	pSTBatt_Rough->nLeakageCurrAlmTimes = 0;

	pSTBatt_Rough->nAirCondi_StartUp_Num = 0;
	pSTBatt_Rough->t_AirCondi_StartUp_Time = _NOW_ - 4;
	pSTBatt_Rough->nSmokeDetector_num = 0;
	pSTBatt_Rough->t_Smoke_time = _NOW_ - 10;
	pSTBatt_Rough->bSmokeFlag = TRUE;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 951, 446, &varV, 4);
	pSTBatt_Rough->f35KW_OpenTemp = varV.fValue;
	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 951, 447, &varV, 4);
	pSTBatt_Rough->f35KW_ShutdownTemp = varV.fValue;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 611, 452, &varV, 4);
	pSTBatt_Rough->QF3_Enable = varV.emValue;

	pSTBatt_Rough->bNeedRecBmsMu2CanLog = FALSE;
	pSTBatt_Rough->tm_fLastStartRecBmsMu2canlog = 0;
	
	memset(pSTBatt_Rough->bmuCanRecCirBuffer.buffer, 0, sizeof(pSTBatt_Rough->bmuCanRecCirBuffer.buffer));
	pSTBatt_Rough->bmuCanRecCirBuffer.count = 0;
	pSTBatt_Rough->bmuCanRecCirBuffer.head = 0;
	pSTBatt_Rough->bmuCanRecCirBuffer.tail = 0;
	
	memset(pSTBatt_Rough->bmuAlmCanRecCirBuffer.buffer, 0, sizeof(pSTBatt_Rough->bmuAlmCanRecCirBuffer.buffer));
	pSTBatt_Rough->bmuAlmCanRecCirBuffer.count = 0;
	pSTBatt_Rough->bmuAlmCanRecCirBuffer.isActive = FALSE;
	pSTBatt_Rough->tm_fLastRecBmsAlmcanlog = 0;
	pSTBatt_Rough->bHotWaitFlag = 0;

	memset(pSTBatt_Rough->szBsuVersion, 0, sizeof(pSTBatt_Rough->szBsuVersion));

	ZERO_POBJS(&pSTBatt_Rough->hisBmsRunDataRec, 	1);

	ZERO_POBJS(&pSTBatt_Rough->stAirStatusAlarm, 	1);
	ZERO_POBJS(&pSTBatt_Rough->stTongFeiAirStatusAlarm,   1);
	ZERO_POBJS(&pSTBatt_Rough->stIMEU3AirStatusFault,   1);//edit by kuangyue 20250217
	ZERO_POBJS(&pSTBatt_Rough->stXinMiaoAirStatusFault,   1);

	ZERO_POBJS(&pSTBatt_Rough->bmsAutoTestResult,   1);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 951 + n, 448,  &varV, 4);
		pSTBatt_Rough->emAirType[n] = (AIR_TYPE)varV.emValue;
	}

	pSTBatt_Rough->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT,
			12, NULL, &pSTBatt_Rough->Rpmsg_cycle_info, sizeof(COMMPORT));
	if(err != ERR_DPR_OK)
	{
		printf("Get Rpmsg_cycle_info commport failed\n");
	}

	pSTBatt_Rough->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT,
		12, NULL, &pSTBatt_Rough->Rpmsg_faq_info, sizeof(COMMPORT));
	if(err != ERR_DPR_OK)
	{
		printf("Get Rpmsg_faq_info commport failed\n");
	}


	ZERO_POBJS(&pSTBatt_Rough->stLiquidCoolingPack, 	1);

	ZERO_POBJS(&pSTBatt_Rough->StatusStat, 	1);
	ZERO_POBJS(&pSTBatt_Rough->EnergyStatus, 	1);
	ZERO_POBJS(&pSTBatt_Rough->DiagStat, 	1);
 
	/*varV.blobValue = (BYTE *)&pSTBatt_Rough->blobESiOCPPCfgs;
	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1, 26, 
		&varV, sizeof(A_BLOB_ES_iOCPP_CONFIGURAION));
	*/

	TRACE("pSTBatt_Rough->emBattMFG =%d \n", pSTBatt_Rough->emBattMFG);

	//Init_stESMgmtPlanParam(pSTBatt_Rough);
	
	ZERO_POBJS(&pSTBatt_Rough->blobEmsCfgPlan, 	1);
 	varV.blobValue = (BYTE *)&pSTBatt_Rough->blobEmsCfgPlan;
	pSTBatt_Rough->pDPR->Get((HANDLE)pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));

	TRACE("Info: nTotal_EmsLmt_Sectors=%d\n", pSTBatt_Rough->blobEmsCfgPlan.nTotal_EmsLmt_Sectors);
	// LJH240715 aircond
	pSTBatt_Rough->nSetTemp4Aircond     = 0;

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 617, &varV, 4);
	pSTBatt_Rough->bAntiOverChg = (BOOL)varV.emValue;
	pSTBatt_Rough->bBattProt = FALSE;
	
	pSTBatt_Rough->pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 300, &varV, 4);
	pSTBatt_Rough->emPIDEnableState = varV.emValue;
    pSTBatt_Rough->pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 301, &varV, 4);
	pSTBatt_Rough->emHighSensitivityEnable = varV.emValue;

	pSTBatt_Rough->pDPR->Get(pSampler, GETTYPE_ST_VAR, 1, 93, &varV, 4);
	pSTBatt_Rough->nRunRecordInterval = (int)varV.fValue;

	// printf("st_batt emPIDEnableState:[%d],emHighSensitivityEnable:[%d]", pSTBatt_Rough->emPIDEnableState, pSTBatt_Rough->emHighSensitivityEnable);

    if (pSTBatt_Rough->emPIDEnableState>POWER_CTRL_DISABLE)
    {
        pSTBatt_Rough->emHighSensitivityEnable = pSTBatt_Rough->emPIDEnableState | pSTBatt_Rough->emHighSensitivityEnable;
    }
    pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 500, 518, &varV, 4);
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->bKeepPowerMode[n] = varV.emValue;
	}

	for (n = 0; n < MAX_IMEU_NUM; ++n)
	{
		pSTBatt_Rough->IMEU2CANRoughData[n].hSampler = pSTBatt_Rough->hSampler;
		pSTBatt_Rough->IMEU2CANRoughData[n].nSamplerId = pSTBatt_Rough->nSamplerId;
		pSTBatt_Rough->IMEU2CANRoughData[n].hSamplerLock = pSTBatt_Rough->hSamplerLock;

		pSTBatt_Rough->IMEU3CANRoughData[n].hSampler = pSTBatt_Rough->hSampler;
		pSTBatt_Rough->IMEU3CANRoughData[n].nSamplerId = pSTBatt_Rough->nSamplerId;
		pSTBatt_Rough->IMEU3CANRoughData[n].hSamplerLock = pSTBatt_Rough->hSamplerLock;

		pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].hSampler = pSTBatt_Rough->hSampler;
		pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].nSamplerId = pSTBatt_Rough->nSamplerId;
		pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].hSamplerLock = pSTBatt_Rough->hSamplerLock;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 601 + n, 450, &varV, 4);
		pSTBatt_Rough->emIMEUType[n] = (IMEU_TYPE)varV.emValue;

#ifdef IMMU3
		// memset(&pSTBatt_Rough->IMEU3CANRoughData, 0, MAX_IMEU_NUM*sizeof(IMEU3_CAN_ROUGH_DATA));

		varV.emValue = 1;//闭合DO3，给BMU上高压
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 601+n, 602, varV);
#endif

	}

	initIMEU2CAN(pSTBatt_Rough->IMEU2CANRoughData, MAX_IMEU_NUM);

	initIMEU3CAN(pSTBatt_Rough->IMEU3CANRoughData, MAX_IMEU_NUM);

	initIMEU3CAN(pSTBatt_Rough->PowerEU3_OtherEU3_Data, MAX_IMEU_NUM);

	pSTBatt_Rough->emESSDevType = GetESSDeviceType_Sampler(hSamplerSelf);

	//edit by Kuangyue 20241114
	int nPackNum, nSingleCellNum;
	if (SYSTYPE_ESS_AIR == pSTBatt_Rough->lcCfg.emSystemType)
	{
		pSTBatt_Rough->eBattCellTempPrecison = TEMP_PRECISION_ONE;
		nPackNum = SYSTYPE_ESS_AIR_PACK_NUM;
		varV.ulValue = nPackNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1944, varV);

		nSingleCellNum = SYSTYPE_ESS_AIR_SINGLE_CELL_NUM;
		varV.ulValue = nSingleCellNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1945, varV);
	}
	else if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
	{
		pSTBatt_Rough->eBattCellTempPrecison = TEMP_PRECISION_ONE_TENTH;
		nPackNum = LIQUID_COOLING_BACKUP_PACK_NUM;
		varV.ulValue = nPackNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1944, varV);

		nSingleCellNum = SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM;
		varV.ulValue = nSingleCellNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1945, varV);
	}
	else if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_836)
	{
		pSTBatt_Rough->eBattCellTempPrecison = TEMP_PRECISION_ONE_TENTH;
		nPackNum = LIQUIDED_836_PACK_NUM;
		varV.ulValue = nPackNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1944, varV);

		nSingleCellNum = SYSTYPE_ESS_LIQUIDED_836_SINGLE_CELL_NUM;
		varV.ulValue = nSingleCellNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1945, varV);
	}
	else
	{
		pSTBatt_Rough->eBattCellTempPrecison = TEMP_PRECISION_ONE_TENTH;
		nPackNum = SYSTYPE_ESS_LIQUIDED_PACK_NUM;
		varV.ulValue = nPackNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1944, varV);

		nSingleCellNum = SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM;
		varV.ulValue = nSingleCellNum;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 1945, varV);
	}

	YFY_Batt_Rpmsg_Init(pSTBatt_Rough);  // 测试用

	COMM_CAN_BAUDRATE	emLevel = COMM_CAN_BAUDRATE_250K;

	if(pSTBatt_Rough->emBattMFG == BATT_CFG_NO_BATT)
	{
		return TRUE;
	}
	/*else if(pSTBatt_Rough->emBattMFG == BATT_MFG_PL)
	{
		emLevel = COMM_CAN_BAUDRATE_500K;
		PL_Batt_Init(pSTBatt_Rough);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_ND)
	{
		emLevel = COMM_CAN_BAUDRATE_250K;
		ND_Batt_Init(pSTBatt_Rough);
	}*/
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_ND_V33)
	{
		emLevel = COMM_CAN_BAUDRATE_250K;
		ND_V33_Batt_Init(pSTBatt_Rough);
		TRACEX("emLevel=[%d] 250K emBattMFG=[%d]\n", emLevel, pSTBatt_Rough->emBattMFG);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_NIFY_V33_500K)
	{
		emLevel = COMM_CAN_BAUDRATE_500K;
		ND_V33_Batt_Init(pSTBatt_Rough);
		TRACEX("emLevel=[%d] emBattMFG=[%d] 500K\n", emLevel, pSTBatt_Rough->emBattMFG);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K || pSTBatt_Rough->emBattMFG == BATT_MFG_TEST_NO_BATT)
	{
		TRACEI("---Initialize----------YFY_Batt_Init---");
		emLevel = COMM_CAN_BAUDRATE_500K;
		YFY_Batt_Init(pSTBatt_Rough);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_IMTCU_V33_500K)
	{
		emLevel = COMM_CAN_BAUDRATE_500K;
		ND_V33_Batt_Init(pSTBatt_Rough);
		TRACEX("emLevel=[%d] emBattMFG=[%d] 500K\n", emLevel, pSTBatt_Rough->emBattMFG);	
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_FENGC_MODBUS)
	{
		FengCH_Batt_Init(pSTBatt_Rough);
		printf("INFO: Init Fengchao Modbus !!!\n");	
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_RPMSG)
	{
		YFY_Batt_Rpmsg_Init(pSTBatt_Rough);
		TRACEI("---Initialize----------YFY_Batt_Rpmsg_Init---");
	}
	else
	{
		TRACE("Not Found Batt MFG Config \n");
		return TRUE;
	}

// 	#define PortAttri "11|A0|500|cell_info"    // 单体核间通信端口

// 	// Infy自有核间通信协议 D9
// 	// if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K)
// 	{
// 		int nErrCode = 0;

// 		pSTBatt_Rough->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT, 12, NULL, pSTBatt_Rough->pRpmsgComm, sizeof(COMMPORT));
// 		if(pSTBatt_Rough->pRpmsgComm == NULL)
// 		{
// 			printf("Open RPMSG Failed !!! pRpmsgComm is NULL\n");
// 			goto STA_1;
// 		}

// 		pSTBatt_Rough->hRpmsgDev = pSTBatt_Rough->pRpmsgComm->Open(
// 			PortAttri,
// 			pSampler->info.szSamplerSetting,
// 			emLevel, RPMSG_READ_TIMEOUT, &nErrCode);
// 		if(pSTBatt_Rough->hRpmsgDev == NULL)
// 		{
// 			printf("Open RPMSG Failed !!! hRpmsgDev is NULL\n");
// 			return FALSE;
// 		}	

// 		if(ERR_COMM_OK != nErrCode)
// 		{
// 			printf("Open RPMSG Failed !!! nErrCode=[%d]\n", nErrCode);
// 			return FALSE;
// 		}

// 		printf("Open RPMSG Succeed!!!!!!!!!!!!!!!!!\n");
// 	}

// STA_1:
 	if(pSTBatt_Rough->emBattMFG != BATT_MFG_FENGC_MODBUS)
	{
		//Open comm port
		int nErrCode = 0;
		pSTBatt_Rough->pComm = pSampler->pUsedCommPort;
		pSTBatt_Rough->hCanDevHanle =  pSampler->pUsedCommPort->Open(
			pSampler->pUsedCommPort->info.szPortAttri,
			pSampler->info.szSamplerSetting,
			emLevel, CAN1_READ_TIMEOUT, &nErrCode); //50ms Timeout

		if(ERR_COMM_OK != nErrCode)
		{
			TRACE("Open CAN0 Failed !!! nErrCode=[%d]\n", nErrCode);
			return FALSE;
		}
	}

	//register event-handler
	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_BMSRUNDATA_UPDATE,	EVENT_MSG_RESTORE},
		{EVENT_TYPE_HEALTHDATA_UPDATE,	EVENT_MSG_RESTORE},

		//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END},
		//{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
	};

	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pSTBatt_Rough->pDPR->Register_EvHandler("stBatt_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	//Init the state machine
	TRACE("Ready to enter init AutoMode.\n");
	Initial_AutoMode_MachineState(pSTBatt_Rough);
	
	// TRACE("Ready to enter init TestMode.\n");
	// Initial_TestMode_MachineState(pSTBatt_Rough);

	// TRACE("Ready to enter prepare TestMode.\n");
	// Prepare_Test_StateMachine(pSTBatt_Rough);
	//Init PushData
	Init_blobBmsAutoTestRange(pSTBatt_Rough);


	//----- Create A Thread To Process Wave Error Frame -----//
	pSTBatt_Rough->stuCanSave.bNeedExitThread = FALSE;
	if(pthread_create(&pSTBatt_Rough->stuCanSave.hThread, 
		NULL, _Thread_BMSCanDataSave,
		(void *)pSTBatt_Rough) != 0) //0-Success
	{
		TRACE("----- Create _Thread_BMSCanDataSave Failed -----\n");
		return FALSE;  
	}
	pthread_detach(pSTBatt_Rough->stuCanSave.hThread);

	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}


static void Stuff_Catl_V33Alm(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n)
{
	VAR_VALUE varV;

	if(pSTBatt_Rough->nStuffTimes[n] == (4+n))
	{
		//TRACE("Info: 111 Stuff BMSIdx=[%d] StuffTimes=[%d] tmNow=[%ld]\n", n,pSTBatt_Rough->nStuffTimes[n], _NOW_);
		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_Ucell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 560, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Under_Ucell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 561, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_Tcell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 562, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Under_Tcell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 563, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_U_HVS;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 564, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Under_U_HVS;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 565, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_T_HVS;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 566, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Under_T_HVS;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 567, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_CC;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 568, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_DC;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 569, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Diff_Tcell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 570, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_PreChFailThree;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 571, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Under_SOC;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 572, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_HSD_Coil;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 573, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Diff_Ucell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 574, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Bal_Circuit;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 575, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.RESV1;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 576, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Relay_OverLoad;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 577, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Pre_Relay;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 578, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Main_Relay_Pos;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 579, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Main_Relay_Neg;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 580, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Relay_Stick_Pos;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 581, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Relay_Stick_Neg;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 582, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Over_Cur;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 583, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Mea_Ucell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 584, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Mea_Tcell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 585, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Diff_Tvol;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 586, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Mea_I_HVS;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 587, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_MCANComFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 588, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.RESV2;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 589, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.RESV3;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 590, varV);
	}
	else if(pSTBatt_Rough->nStuffTimes[n] == (8+n))
	{
		//TRACE("Info: 222 Stuff BMSIdx=[%d] StuffTimes=[%d] tmNow=[%ld]\n", n,pSTBatt_Rough->nStuffTimes[n], _NOW_);
		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.RESV4;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 591, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_MCU;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 592, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Extre_Tvol;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 593, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_CCAN;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 594, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_SCAN;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 595, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Mea_AllTcell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 596, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Extre_T;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 597, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Extre_Ucell;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 598, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_PowerSample_LEM;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 599, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_PowerSample_CSC;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 600, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_SBMU_PowerDown;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 601, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Fog;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 602, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_HvCrcuitBreak;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 603, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_HvMsdFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 604, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Fan1Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 605, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Fan2Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 606, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].unioBattGrpAlm.Alarm.AL_Err_Relay_STB;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 607, varV);
	}
	return;
}

static void Stuff_IFY_SelfProtAlm2Fault(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	int nByte = 0, nBit = 0;
	VAR_VALUE varV;
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	//pSingleRunInfo->szAlarm, 
	//pSingleRunInfo->szFault, 
	//pSingleRunInfo->szDIDOStatu

	if(pSTBatt_Rough->nStuffTimes[nBattIdx] == (4+nBattIdx))
	{
		if(pSTBatt_Rough->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED)
		{//液冷不用推送风冷的告警列表，否则会把液冷告警消除掉，第二个else if szFault故障列表风冷液冷共用
			return;
		}
		for(nByte = 0; nByte < CAN_DATA_AREA_LEN; nByte++)
		{
			for(nBit = 0; nBit < 8; nBit++)
			{
				varV.emValue = (_ENUM)((pSingleRunInfo->szAlarm[nByte] >> nBit) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 640 + nByte*8 + nBit, varV);
			}
		}
	}
	else if(pSTBatt_Rough->nStuffTimes[nBattIdx] == (8+nBattIdx))
	{
		for(nByte = 0; nByte < CAN_DATA_AREA_LEN; nByte++)
		{
			for(nBit = 0; nBit < 8; nBit++)
			{
				varV.emValue = (_ENUM)((pSingleRunInfo->szFault[nByte] >> nBit) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 704 + nByte*8 + nBit, varV);
			}
		}
	}
	else if(pSTBatt_Rough->nStuffTimes[nBattIdx] == (16+nBattIdx))
	{
		for(nByte = 0; nByte < 4; nByte++) //DIDO Only Use 4 Bytes, Reserve 4 Bytes
		{
			for(nBit = 0; nBit < 8; nBit++)
			{
				varV.emValue = (_ENUM)((pSingleRunInfo->szDIDOStatu[nByte] >> nBit) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + nBattIdx , 768 + nByte*8 + nBit, varV);
			}
		}
	}
	return;
}

static void Push_STBatt_AllData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	VAR_VALUE varV;
	varV.ulValue = pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt;
	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 500, varV);

	//varV.emValue = pSTBatt_Rough->blobGroupBattInfo.emBattGrpRunStatus;
	//Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 501, varV);

	//varV.emValue = pSTBatt_Rough->blobGroupBattInfo.emBattGrpAlmStatus;
	//Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 502, varV);

	varV.blobValue = (BYTE *)&pSTBatt_Rough->blobGroupBattInfo;
	Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
		500, 503, (void*)varV.blobValue, sizeof(A_BLOB_GROUP_STBATT_RUNINFO), BLOB_TYPE_GROUP_STBATT_RUNINFO);
	
	varV.ulValue = pSTBatt_Rough->nProtectSOC;
	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 508, varV);

	int n;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		varV.ulValue = pSTBatt_Rough->blobSingleBattInfo[n].uiBattCabDevAlm;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 505, varV);

		varV.ulValue = pSTBatt_Rough->stuCanSave.emTrigMsgTp[n];
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 506, varV);

		// varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run;
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 550, varV);

		varV.blobValue = (BYTE *)&pSTBatt_Rough->blobSingleBattInfo[n];
		Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
			501 + n, 551, (void*)varV.blobValue, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO), BLOB_TYPE_SINGLE_STBATT_RUNINFO);
		
		varV.blobValue = (BYTE *)&pSTBatt_Rough->blobIfyBMS[n];
		Push_BlobValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 
			501 + n, 557, (void*)varV.blobValue, sizeof(A_BLOB_IFY_BMS_DATAINFO), BLOB_TYPE_IFYBMS_DATAINFO);

		if(pSTBatt_Rough->lcCfg.emSystemType != SYSTYPE_ESS_AIR)//xsy 非风冷系统（液冷、站级等）由BMU上传漏电流值 (CAN)
		{
		varV.fValue = pSTBatt_Rough->blobIfyBMS[n].fLeakageCurrent; //xsy
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 1 + n, 40, varV);
		// printf("push:fLEAK : %lf\n", varV.fValue);
		
		varV.fValue = pSTBatt_Rough->blobIfyBMS[n].nSoftVer; //xsy
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 1 + n, 38, varV);
		}

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Alm;
		//TRACE("nBattIdx=[%d].emBattStatus_Alm=%d\n", n, pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Alm);
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 552, varV);

		// varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emPowerONStatu;
		//TRACE("nBattIdx=[%d].emPowerONStatu=%d\n", n, pSTBatt_Rough->blobSingleBattInfo[n].emPowerONStatu);
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 553, varV);

		varV.fValue = pSTBatt_Rough->blobSingleBattInfo[n].fMax_CellTemp;
		//TRACE("nBattIdx=[%d].fMax_CellTemp=%f\n", n, pSTBatt_Rough->blobSingleBattInfo[n].fMax_CellTemp);
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 558, varV);

		varV.fValue = pSTBatt_Rough->blobSingleBattInfo[n].fMin_CellTemp;
		//TRACE("nBattIdx=[%d].fMax_CellTemp=%f\n", n, pSTBatt_Rough->blobSingleBattInfo[n].fMax_CellTemp);
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 559, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emMgmtStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 530, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emMgmtPcsCtrlCmd;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 531, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emMgmtExecReason;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 532, varV);


		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emReason_ChgStart;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 545, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emReason_DisChgON;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 546, varV);

		varV.emValue = pSTBatt_Rough->blobSingleBattInfo[n].emIsoResult;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 547, varV);

		varV.fValue = pSTBatt_Rough->blobSingleBattInfo[n].fInsulation_P_R;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 548, varV);

		varV.fValue = pSTBatt_Rough->blobSingleBattInfo[n].fInsulation_N_R;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 549, varV);

		
		// if(pSTBatt_Rough->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED)
		// {//液冷设备的告警列表，分冷柜有另外的告警列表见 Stuff_IFY_SelfProtAlm2Fault
		// 	int nBit;
		// 	for( nBit = 0; nBit < 16; nBit++)
		// 	{
		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo1, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 640 + nBit, varV);

		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo2, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 656 + nBit, varV);

		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo4, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 804 + nBit, varV);

		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo5, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 820 + nBit, varV);

		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo6, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 836 + nBit, varV);

		// 		varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo7, nBit);
		// 		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 852 + nBit, varV);
		// 	}

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 0);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 672, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 1);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 673, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 2);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 674, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 3);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 675, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 4);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 676, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 5);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 677, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 6);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 678, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 7);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 679, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 8);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 680, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 9);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 681, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 10);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 682, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 11);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 683, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 12);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 800, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 13);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 801, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 14);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 802, varV);

		// 	varV.emValue = GET_BIT(pSTBatt_Rough->blobIfyBMS[n].nAlarmInfo3, 15);
		// 	Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 803, varV);
		// }
		
		varV.emValue = pSTBatt_Rough->blobIfyBMS[n].bAfeWorkModeAbnormal;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 3903, varV);


		//---- Stuff Alarm ----//
		if((pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run == STBATT_RUNSTAT_NOT_INSTALLED)
			|| (pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run == STBATT_RUNSTAT_COMM_FAIL))
		{
			continue;
		}

		//TRACE("nBattIdx=[%d] emBattStatus_Run=[%d]\n", n, pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run);

		if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K || pSTBatt_Rough->emBattMFG == BATT_MFG_TEST_NO_BATT)
		{
			// Stuff_IFY_SelfProtAlm2Fault(pSTBatt_Rough, n);
		}
		else 
		{
			// Stuff_Catl_V33Alm(pSTBatt_Rough, n);
		}
		
		//----- Push Alarm & Fault -----//
		// Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 540, (void*)&pSTBatt_Rough->blobSingleBattInfo[n].szS2A_AlarmInfo,  36);
		// Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 541, (void*)&pSTBatt_Rough->blobSingleBattInfo[n].szInfy_AlarmInfo, 36);
		// Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 542, (void*)&pSTBatt_Rough->blobSingleBattInfo[n].szInfy_FaultInfo, 36);
		// Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501 + n, 543, (void*)&pSTBatt_Rough->blobSingleBattInfo[n].szInfy_GPIOInfo,  36);
	
		pSTBatt_Rough->nStuffTimes[n]++;
		if(pSTBatt_Rough->nStuffTimes[n] > (16+n))
		{
			pSTBatt_Rough->nStuffTimes[n] = 0;
		}
	}

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++ )
	{
		if(fabs(pSTBatt_Rough->fCalc_ChargedEnergy[n] - pSTBatt_Rough->fLastCalc_ChargedEnergy[n]) > 1.0)
		{
			TRACE("nBattIDx=[%d]->fCalc_ChargedEnergy=%f ->fLastCalc_ChargedEnergy=%f\n", \
				n, pSTBatt_Rough->fCalc_ChargedEnergy[n] , pSTBatt_Rough->fLastCalc_ChargedEnergy[n]);
			
			pSTBatt_Rough->fLastCalc_ChargedEnergy[n] = pSTBatt_Rough->fCalc_ChargedEnergy[n];
			varV.fValue = (float)pSTBatt_Rough->fCalc_ChargedEnergy[n];
			Push_VarValue(SETTYPE_INTERN_SETT,  pSTBatt_Rough->hSampler, 501 + n, 504, varV);
		}

		if(fabs(pSTBatt_Rough->fCalc_DisChargedEnergy[n] - pSTBatt_Rough->fLastCalc_DisChargedEnergy[n]) > 1.0)
		{	
			TRACE("nBattIDx=[%d]->fCalc_DisChargedEnergy=%f ->fLastCalc_DisChargedEnergy=%f\n", \
				n, pSTBatt_Rough->fCalc_DisChargedEnergy[n] , pSTBatt_Rough->fLastCalc_DisChargedEnergy[n]);
			pSTBatt_Rough->fLastCalc_DisChargedEnergy[n] = pSTBatt_Rough->fCalc_DisChargedEnergy[n];
 			varV.fValue = (float)pSTBatt_Rough->fCalc_DisChargedEnergy[n];
			Push_VarValue(SETTYPE_INTERN_SETT,  pSTBatt_Rough->hSampler, 501 + n, 505, varV);
		}
	}
	return;
}

static void Push_STBatt_AirStatusData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	VAR_VALUE varV;
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("Push_STBatt_AirStatusData ----Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		//---liquid AIR status01---//
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nPowerStatus; // 开关机状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1691, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nWorkMode; // 工作模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1521, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fColdTemp; // 制冷温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1522, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nColdOpenDiff; // 制冷启动温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1523, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nColdStopDiff; // 制冷停止温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1524, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fHeatTemp; // 制热温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1525, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fHeatDiff; // 制热温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1526, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nRefrigerantType; // 冷媒类型
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1527, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nUpMachineBaud; // 上位机波特率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1528, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nContactAddr; // 通信地址
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1529, varV);

		//---liquid AIR status02---//
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nOutsideMachinAbility; // 室外机规格能力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1530, varV);

		// varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nSoftVer; // 软件版本
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1562, varV);    //pending 

		Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1562, (void*)&pSTBatt_Rough->stAirStatusAlarm[n].byBMSAirSoftVersion,  16);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nPLS; // 系统1主电子膨胀阀开度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1531, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1TargetCompFreq; // 系统1变频压缩机目标频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1532, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1CurrentCompFreq; // 系统1变频压缩机运行频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1533, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nFanTargetSpeed; // 风机目标转速
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1534, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fExhaustSuperheat; // 排气过热度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1535, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1AmbientTemp; // 系统1外环温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1536, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1ExhaustTemp; // 系统1排气温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1537, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1SuctionTemp; // 系统1吸气温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1538, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fModuleInletTemp; // 模块进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1539, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fModuleOutletTemp; // 模块出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1540, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fBatteryInletTemp; // 电池进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1541, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fBatteryOutletTemp; // 电池出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1542, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nPumpActualSpeed; // 水泵实际转速
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1543, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fModuleOutletPressure; // 模块出水压力 0.01
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1544, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fBatteryOutletPressure; // 电池出水压力 0.01
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1545, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1HighSatTemp; // 系统1高压饱和温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1546, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1LowSatTemp; // 系统1低压饱和温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1547, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSuctionSuperheat; // 吸气过热度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1548, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1LowPressure; // 系统1低压压力 0.01
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1549, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1HighPressure; // 系统1高压压力 0.01
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1550, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1ACInputVoltage; // 系统1AC输入电压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1551, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1ACInputCurrent; // 系统1AC输入电流
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1552, varV);

		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].nSys1CompressorCurrent; // 系统1压缩机相电流值
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1553, varV);

		varV.fValue = pSTBatt_Rough->stAirStatusAlarm[n].fSys1DCBusVoltage; // 系统1直流母线电压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1554, varV);

		//---liquid AIR status03---//
		// 推送电辅热状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.ElectricHeatingStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1555, varV);

		// 推送循环水泵状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.CirculationPumpStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1556, varV);

		// 推送冷凝风机状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.CondenserFanStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1557, varV);

		// 推送系统1压缩机状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.Comp1Status;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1558, varV);

		// 推送系统1曲轴加热带状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.CrankHeat1Status;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1559, varV);

		// 推送系统1运行状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.Sys1RunningStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1560, varV);

		// 推送系统1回油状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.OilReturn1Status;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1561, varV);

		// 推送系统总故障标志
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault1.Status.TotalFaultFlag;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1563, varV);

		//==========================================================================

		// 推送电池出水压力过高告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.HighBatteryOutletPressure;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1564, varV);

		// 推送模块出水压力过高告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.HighModuleOutletPressure;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1565, varV);

		// 推送PTC过载告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.PTCOverload;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1566, varV);

		// 推送回水温度过高过低告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.HighLowReturnWaterTemp;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1567, varV);

		// 推送出水温度过高过低告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.HighLowOutletWaterTemp;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1568, varV);

		// 推送风机1故障告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.Fan1Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1569, varV);

		// 推送风机2故障告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.Fan2Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1570, varV);

		// 推送风机3故障告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault2.Alarm.Fan3Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1571, varV);

		//==========================================================================
		// 推送风机4故障状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Fan4Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1572, varV);

		// 推送系统1故障报警状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Sys1AlarmStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1573, varV);

		// 推送系统1故障锁机状态
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Sys1LockStatus;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1574, varV);

		// 推送过冷温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.SubcoolingTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1575, varV);

		// 推送系统1外环温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Sys1AmbientTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1576, varV);

		// 推送系统1排气温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Sys1ExhaustTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1577, varV);

		// 推送系统1吸气温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.Sys1SuctionTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1578, varV);

		// 推送模块进水温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault3.Fault.ModuleInletTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1579, varV);

		//==========================================================================

		// 推送模块出水温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.ModuleOutletTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1580, varV);

		// 推送风机2故障(预留)
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.Fan2FaultReserved;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1581, varV);

		// 推送电池进水温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.BatteryInletTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1582, varV);

		// 推送电池出水温度传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.BatteryOutletTempSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1583, varV);

		// 推送过热度过低告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.LowSuperheatAlarm;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1584, varV);

		// 推送系统1高压压力传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.Sys1HighPressureSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1585, varV);

		// 推送系统1低压压力传感器故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.Sys1LowPressureSensorFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1586, varV);

		// 推送系统1与驱动器通讯故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault4.Fault.Sys1DriverCommFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1587, varV);

		//==========================================================================

		// 推送水箱缺液告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.WaterTankLowLevelAlarm;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1588, varV);

		// 推送模块系统缺液告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.ModuleSystemLowLevelAlarm;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1589, varV);

		// 推送风机告警故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.FanAlarmFault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1590, varV);

		// 推送系统1高压保护
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.Sys1HighPressureProtection;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1591, varV);

		// 推送系统1低压保护
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.Sys1LowPressureProtection;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1592, varV);

		// 推送电池系统缺液告警
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.BatterySystemLowLevelAlarm;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1593, varV);

		// 推送系统1冬季防冻结1
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.Sys1WinterAntiFreeze1;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1594, varV);

		// 推送系统1冬季防冻结2
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault5.Alarm.Sys1WinterAntiFreeze2;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1595, varV);

		//==========================================================================

		// 推送系统1水温异常保护
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.Sys1WaterTempAbnormalProtection;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1596, varV);

		// 推送系统1排气过高保护
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.Sys1ExhaustOverTempProtection;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1597, varV);

		// 推送系统1制冷防冻保护
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.Sys1CoolingAntiFreezeProtection;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1598, varV);

		// 推送HY1水泵故障反馈
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.HY1PumpFaultFeedback;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1599, varV);

		// 推送HY3水泵故障反馈
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.HY3PumpFaultFeedback;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1600, varV);

		// 推送直流风机1故障
		varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.DCFan1Fault;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1601, varV);

		// // 推送预留位6（不使用）
		// varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.Reserved6;
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1554, varV);

		// // 推送预留位7（不使用）
		// varV.emValue = pSTBatt_Rough->stAirStatusAlarm[n].AirSysFault6.Protection.Reserved7;
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1554, varV);

	}
}


static void Push_STBatt_PackDataInfo(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	VAR_VALUE varV;
	int n = 0;
	int PackDevId = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("Push_STBatt_AirStatusData ----Skip Single Battery Idx=[%d]\n",n);
			continue;
		}

		int i = 0;
		for(i = 0; i < MAX_COOLINGPACK_NUM; i++)
		{
		    //PACK #1 - #5 push 液冷参数数组 目前有10（MAX_COOLINGPACK_NUM）个  其中有两个用作预留参数
			
		    varV.fValue = pSTBatt_Rough->stLiquidCoolingPack[n].fPositiveTemp[i]; //  正极铜排温度
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1700 + COOLINGPACK_PARAMETER_NUM*i, varV);

		    varV.fValue = pSTBatt_Rough->stLiquidCoolingPack[n].fNegativeTemp[i];  //   负极铜排温度
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1701 + COOLINGPACK_PARAMETER_NUM*i, varV); 

		    varV.ulValue = pSTBatt_Rough->stLiquidCoolingPack[n].uImmersionFreq[i];  // 水浸频率
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1702 + COOLINGPACK_PARAMETER_NUM*i, varV); 
		
		    varV.fValue = pSTBatt_Rough->stLiquidCoolingPack[n].fHighVoltageValue[i];  // 高压互锁幅值
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1703 + COOLINGPACK_PARAMETER_NUM*i, varV); 

	    	varV.ulValue = pSTBatt_Rough->stLiquidCoolingPack[n].uHighVoltageDuty[i];  // 高压互锁占空比
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1704 + COOLINGPACK_PARAMETER_NUM*i, varV); 

		    varV.ulValue = pSTBatt_Rough->stLiquidCoolingPack[n].uSol[i];  // 气溶胶浓度
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1705 + COOLINGPACK_PARAMETER_NUM*i, varV);

		    varV.ulValue = pSTBatt_Rough->stLiquidCoolingPack[n].uH2[i];  // H2浓度
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1706 + COOLINGPACK_PARAMETER_NUM*i, varV);

		    varV.ulValue = pSTBatt_Rough->stLiquidCoolingPack[n].uAerosolStatusAndFault[i];  // 气溶胶状态及故障信息
		    Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 +n, 1707 + COOLINGPACK_PARAMETER_NUM*i, varV);

		}
			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu1BotNum;// PACK1 AFE BOT端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3708, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu1TopNum;// PACK1 AFE TOP端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3709, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu2BotNum;// PACK2 AFE BOT端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3710, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu2TopNum;// PACK2 AFE TOP端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3711, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu3BotNum;// PACK3 AFE BOT端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3712, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu3TopNum;// PACK3 AFE TOP端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3713, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu4BotNum;// PACK4 AFE BOT端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3714, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu4TopNum;// PACK4 AFE TOP端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3715, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu5BotNum;// PACK5 AFE BOT端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3716, varV);

			varV.lValue = pSTBatt_Rough->stLiquidCoolingPack[n].usBsu5TopNum;// PACK5 AFE TOP端枚举数量
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 981 + n, 3717, varV);
	}

}

static void Push_TongFei_AirStatusData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{

	VAR_VALUE varV;
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("Push_TongFei_AirStatusData ----Skip Single Battery Idx=[%d]\n",n);
			continue;
		}

		varV.emValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nStartupMode; // 启动模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1770, varV);

		varV.emValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nWorkingMode; // 工作模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1771, varV);

		varV.emValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSuplyWater; // 供回水控制目标
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1772, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fRefrigerationPresetTemperature; // 制冷预设温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1773, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fCoolingTemperatureTifference; // 制冷温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1774, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fHeatingTresetTemperature; // 制热预设温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1775, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fHeatingTemperatureTifference; // 制热温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1776, varV);

		varV.emValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nUnitOperatingStatus; // 机组运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1777, varV);

		varV.emValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nUnitOperatingMode; // 机组运行模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1778, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fLiquidSupplyTemperature; // 供液温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1779, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fReturnLiquidTemperature; // 回液温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1780, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fSystemSteamingIn; // 系统蒸进
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1781, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fSteamOutTemperature; // 蒸出温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1782, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fAmbientTemperature; // 环境温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1783, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].fSystemCondensationTemperature; // 系统冷凝温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1784, varV);

		varV.ulValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nCurrentOpeningDegree; // 节流电膨当前开度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1785, varV);

		varV.ulValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nCurrentSpeedWaterPump; // 水泵当前转速
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1786, varV);

		varV.ulValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nCurrentSpeedCompressor; // 压缩机当前转速
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1787, varV);

		varV.ulValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFanSpeed; // 风机转速
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1788, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nWaterSupplyPressure; // 供水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1789, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nReturnWaterPressure; // 回水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1790, varV);

		varV.fValue = pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nExhaustPressure; // 排气压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1791, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 0); // Bit0:系统高压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1792, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 2); // Bit2:系统低压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1793, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 5); // Bit5:液位开关
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1794, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 6); // Bit6:供水泵过载
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1795, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 9); // Bit9:加热管过载
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1796, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nSwitchingInput, 10); // Bit10:风机过载
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1797, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nRelayInput, 5); // Bit5:加热管
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1799, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nRelayInput, 6); // Bit6:报警继电器
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1800, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nRelayInput, 7); // Bit7:加热带
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1801, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 0); // Bit0:供水泵故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1802, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 1); // Bit1:泵空转保护
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1803, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 2); // Bit2:泵堵转保护
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1804, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 3); // Bit3:泵过流故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1805, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 4); // Bit4:泵欠压故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1806, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 5); // Bit5:泵过压故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1807, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 6); // Bit6:泵缺相/电流采样偏移/失步/预充故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1808, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 7); // Bit7:泵驱动过热保护故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1809, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 8); // Bit8:供水压力传感器故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1810, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 9); // Bit9:主板IIC故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1811, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 10); // Bit10:回水压力低故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1812, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 11); // Bit11:供水压力过高
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1813, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 12); // Bit12:电压过高
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1814, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus1, 13); // Bit13:电压过低
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1815, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 0); // Bit0:系统低压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1816, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 1); // Bit1:电加热保护
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1817, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 2); // Bit2:风机过载
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1818, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 3); // Bit3:系统高压故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1819, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 4); // Bit4:蒸发进口探头故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1820, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 5); // Bit5: 蒸发出口探头故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1821, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 6); // Bit6:系统高压报警锁定
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1822, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 7); // Bit7:排气压力传感器故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1823, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus2, 8); // Bit8:压缩机变频器通信故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1824, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 0); // Bit0:液温探头故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1825, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 1); // Bit1:回液探头故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1826, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 2); // Bit2:环境探头故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1827, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 3); // Bit3:液温过高
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1828, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 4); // Bit4:回水压力传感器故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1829, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 5); // Bit5:屏幕与主板通信超时
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1830, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 6); // Bit6:液温过低
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1831, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 7); // Bit7:供水压力过高预警
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1832, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 8); // Bit8:掉电报警	
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1833, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus3, 9); // Bit9:从板通讯故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1834, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 0); // Bit0:压机启动瞬间过流
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1835, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 1); // 	Bit1:压机加速运行过流
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1836, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 2); // Bit2:压机减速运行过流
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1837, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 3); // Bit3: 压机恒速运行过流
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1838, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 4); //  Bit4: 压机加速运行过压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1839, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 5); //   Bit5: 压机减速运行过压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1840, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 6); //   Bit6: 压机恒速运行过压
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1841, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 7); // Bit7: 压机待机时过压   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1842, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 8); //  Bit8:压机运行中欠  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1843, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 9); //   Bit9:压机输出缺相 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1844, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 10); //   Bit10:压机功率器件保护 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1845, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 11); //  Bit11:压机过热  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1846, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 12); //   Bit12:压机过载 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1847, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 13); // Bit13:压机检测压机过载   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1848, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 14); // Bit14:压机负载过重   		
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1849, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus4, 15); //  Bit15:压机速度过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1850, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 0); //  Bit0:压机D轴电流过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1851, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 1); //  Bit1:压机Q轴电流过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1852, varV);		

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 2); //  Bit2:压机参数存储失败  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1853, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 3); //  Bit3:压机通信故障  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1854, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 4); //   Bit4:压机电流检测故障 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1855, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 5); //  Bit5:压机启动中电机堵转  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1856, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 6); // Bit6: 压机运行中电机堵转   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1857, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 7); // Bit7:压机失速故障   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1858, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 8); //    Bit8:压机中断溢出1
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1859, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 9); // Bit9:压机中断溢出2   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1860, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 10); //   Bit10:压机启动中转子抖动过大 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1861, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 11); //  Bit11:压机运行中转子抖动过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1862, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 12); //   Bit12:PFC过流 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1863, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 13); //  Bit13:PFC峰值电流过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1864, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nFaultStatus5, 14); //  Bit14:PFC有效值电流过大  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1865, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 0); // Bit0:风机启动瞬间过流   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1869, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 1); // Bit1: 风机加速运行过流   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1870, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 2); // Bit2: 风机减速运行过流   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1871, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 3); //  Bit3: 风机恒速运行过流  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1872, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 4); //  Bit4: 风机加速运行过压  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1873, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 5); //   Bit5: 风机减速运行过压 
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1874, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 6); //  Bit6: 风机恒速运行过压  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1875, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 7); //  Bit7: 风机待机时过压    
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1876, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 8); // Bit8: 风机运行中欠   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1877, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 9); //  Bit9:风机输入缺相  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1878, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 10); // Bit10:风机输出缺相   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1879, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 11); // Bit11:风机功率器件保护   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1880, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 12); //  Bit12:风机过热  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1881, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 13); // Bit13:变频器过载   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1882, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 14); //  Bit14:风机过载  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1883, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault1, 15); // Bit15:外部故障   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1884, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 0); // Bit0:电机负载过重   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1885, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 1); //  Bit1:变频器欠载  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1886, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 2); // Bit2:风扇故障   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1887, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 3); //  Bit3:用户使用期限禁止时间已到  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1888, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 4); //  Bit4:参数存储失败  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1889, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 5); //  Bit5:通讯异常  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1890, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 6); //  Bit6:电流检测故障  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1891, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 7); // Bit7:自整定不良   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1892, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 8); //  Bit8:模拟输入掉线  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1893, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 9); //  Bit9:PG断线  
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1894, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 10); // Bit10:热敏电阻开路   
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1895, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stTongFeiAirStatusAlarm[n].nVariableFrequencyFault2, 11); //   Bit11:异常停机故障
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 1896, varV);

	}


}

static void Push_IMEU3_AirStatusData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	//edit by kuangyue 20250217
	VAR_VALUE varV;
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("Push_TongFei_AirStatusData ----Skip Single Battery Idx=[%d]\n",n);
			continue;
		}

		//判断开关机、故障状态，清除手动控压缩机频率标志
		int nDevId = 951 + n; 
		
		if (pSTBatt_Rough->stIMEU3AirStatusFault[n].uPowerStat == 0)
		{
			ClearAir537Param(pSTBatt_Rough, PARAM_TYPE_SETTING, nDevId, 537, NULL, sizeof(VAR_VALUE), 1000);
		}

		if (pSTBatt_Rough->stIMEU3AirStatusFault[n].uErrCode != 0)
		{
			ClearAir537Param(pSTBatt_Rough, PARAM_TYPE_SETTING, nDevId, 537, NULL, sizeof(VAR_VALUE), 1000);
		}

		char buffer[16];
 		snprintf(buffer, 16, "V%d.%02d", (pSTBatt_Rough->stIMEU3AirStatusFault[n].bSoftVer) >> 8, (pSTBatt_Rough->stIMEU3AirStatusFault[n].bSoftVer) & 0xFF);
		Push_TextValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3500, buffer,  16);

		varV.emValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uPowerStat; // 开机状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3501, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uOpMode; // 运行模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3502, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uOpStep; // 运行阶段
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3503, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uErrCode; // 故障码
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3504, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fTgtTemp; // 目标温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3505, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fRetTemp; // 反馈水温
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3506, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fOverheatTemp; // 吸气过热温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3507, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fSatTemp; // 饱和温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3508, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uExpValveOpen; // 电子膨胀阀当前开度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3509, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uTotalCap; // 能力总输出
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3510, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uCompTgtHz; // 压缩机目标频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3511, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uCompOpHz; // 压缩机运行频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3512, varV);

		varV.ulValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].uCompOpPhase; // 压缩机运行阶段
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3513, varV);

		// 故障等级 (bit0~bit15) 
		USHORT  rawData = pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv;
		uint8_t battStatus = GET_VAL(rawData, 0, 0x1F);
		uint8_t modStatus  = GET_VAL(rawData, 5, 0x07);

		varV.emValue = (battStatus == 1); // 电池液路1 告警不停机 电池液路1可运行
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3515, varV);

		varV.emValue = (battStatus == 2); // 电池液路1 告警需停机 电池液路1不运行
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3516, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 0); // Bit0: 电池液路1 无故障
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3514, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 1); // Bit1: 电池液路1 告警不停机
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3515, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 2); // Bit2: 电池液路1 告警需停机 (低位)
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3516, varV);

		varV.emValue = (modStatus == 1); // 告警不停机，模块液路2可运行
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3520, varV);

		varV.emValue = (modStatus == 2); // 告警需停机，模块液路2不运行
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3521, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 3); // Bit3: 电池液路1 告警需停机 (高位)
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3517, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 4); // Bit4: 电池液路1 保留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3518, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 5); // Bit5: 模块液路2 无故障
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3519, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 6); // Bit6: 模块液路2 告警不停机
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3520, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 7); // Bit7: 模块液路2 告警需停机
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3521, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 8); // Bit8: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3522, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 9); // Bit9: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3523, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 10); // Bit10: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3524, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 11); // Bit11: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3525, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 12); // Bit12: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3526, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 13); // Bit13: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3527, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 14); // Bit14: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3528, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLv, 15); // Bit15: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3529, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fexhaustTempFault; // 排气温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3530, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fEvapInTemp; // 吸气温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3531, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fCoolInTemp; // 空调冷却水进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3532, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fModuleOutTemp; // 模块出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3533, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fModuleInTemp; // 模块进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3534, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fBattInTemp; // 电池进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3535, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fBattOutTemp; // 电池出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3536, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fCoolModOutTemp; // 空调冷却水模块出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3537, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fCoolBattOutTemp; // 空调冷却水电池出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3538, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fAmbTemp; // 环境温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3539, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fHighPress; // 高压压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3540, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fLowPress; // 低压压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3541, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fBattOutPress; // 电池出水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3542, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fModuleOutPress; // 模块出水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3543, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fCoolInPress; // 空调冷却水进水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3544, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fCoolOutPress; // 空调冷却水出水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3545, varV);

		// 传感器故障1 (bit0~bit15) 
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 0); // Bit0: NTC1 排气温度传感器异常（A1）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3546, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 1); // Bit1: NTC2 吸气温度传感器异常（A0）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3547, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 2); // Bit2: NTC3 空调冷却水进水温度传感器异常（A2）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3548, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 3); // Bit3: NTC4 模块出水温度传感器异常（A4）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3549, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 4); // Bit4: NTC5 模块进水温度传感器异常（A3）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3550, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 5); // Bit5: NTC6 电池进水温度传感器异常（A5）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3551, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 6); // Bit6: NTC7 电池出水温度传感器异常（A6）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3552, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 7); // Bit7: NTC8 空调冷却水模块侧出水温度传感器故障（A7）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3553, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 8); // Bit8: NTC9 空调冷却水电池侧出水温度传感器故障（A8）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3554, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 9); // Bit9: NTC10 环境温度传感器异常（A9）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3555, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 10); // Bit10: NTC11 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3556, varV);
		
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 11); // Bit11: NTC12 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3557, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 12); // Bit12: PS1 排气压力传感器故障（AA）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3558, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 13); // Bit13: PS2 吸气压力传感器故障（AB）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3559, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 14); // Bit14: PS3 电池出水压力传感器故障（AC）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3560, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nFaultLvl, 15); // Bit15: PS4 模块出水压力传感器故障（AD）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3561, varV);

		// 传感器故障2 (bit0~bit15)
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 0); // Bit0: PS5 空调冷却水进水压力传感器故障（AE）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3562, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 1); // Bit1: PS6 空调冷却水出水压力传感器故障（AF）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3563, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 2); // Bit2: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3564, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 3); // Bit3: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3565, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 4); // Bit4: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3566, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 5); // Bit5: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3567, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 6); // Bit6: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3568, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 7); // Bit7: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3569, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 8); // Bit8: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3570, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 9); // Bit9: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3571, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 10); // Bit10: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3572, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 11); // Bit11: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3573, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 12); // Bit12: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3574, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 13); // Bit13: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3575, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 14); // Bit14: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3576, varV);
		
		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nVfFault2, 15); // Bit15: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3577, varV);

		// 系统部件故障 (bit0~bit15)
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 0); // Bit0: DI1 风机1故障（B1）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3578, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 1); // Bit1: DI2 风机2故障（B2）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3579, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 2); // Bit2: DI3 风机3故障（B3）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3580, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 3); // Bit3: DI4 风机4故障（B4）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3581, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 4); // Bit4: DI5 电加热故障（B7）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3582, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 5); // Bit5: DI6 水箱缺液故障（D7）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3583, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 6); // Bit6: DI7 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3584, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 7); // Bit7: DI8 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3585, varV);
		
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 8); // Bit8: DI17 EH1电池水泵故障（B5）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3586, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 9); // Bit9: DI18 EH3模块水泵故障（B6）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3587, varV);
		
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 10); // Bit10: 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3588, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 11); // Bit11: 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3589, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 12); // Bit12: 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3590, varV);
		
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 13); // Bit13: 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3591, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 14); // Bit14: 预留
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3592, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nPfcFault, 15); // Bit15: 风机全故障（B0）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3593, varV);

		// 压缩机故障1 (bit0~bit15)
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 0); // Bit0: 压缩机通信故障（C0）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3594, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 1); // Bit1: 交流电流（PFC电流）硬件过流故障（C1）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3595, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 2); // Bit2: AC输入瞬时过流故障（C2）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3596, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 3); // Bit3: PFC直流瞬时过压故障（C3）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3597, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 4); // Bit4: 压机逆变器硬件故障（C4）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3598, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 5); // Bit5: 压机瞬时过流故障（C5）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3599, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 6); // Bit6: 压机失步故障（C6）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3600, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 7); // Bit7: 压机缺相故障（C7）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3601, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 8); // Bit8: 压机逆变器过温故障/PCB过温故障（C8）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3602, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 9); // Bit9: IGBT外壳过温故障（C9）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3603, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 10); // Bit10: 功率过载故障（CA）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3604, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 11); // Bit11: 交流电流（PFC电流）过流故障（CB）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3605, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 12); // Bit12: 交流电压过压故障（CC）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3606, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 13); // Bit13: 直流电压（PFC）过压故障（CD）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3607, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 14); // Bit14: 压机相电流过流故障（CE）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3608, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault1, 15); // Bit15: 交流电压欠压故障（CF）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3609, varV);

		// 压缩机故障2 (bit0~bit15)
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 0); // Bit0: 直流电压（PFC）欠压故障（CG）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3610, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 1); // Bit1: （三相）交流电压缺相故障（CH）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3611, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 2); // Bit2: 压缩机总故障（CI）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3612, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 3); // Bit3: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3613, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 4); // Bit4: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3614, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 5); // Bit5: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3615, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 6); // Bit6: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3616, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 7); // Bit7: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3617, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 8); // Bit8: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3618, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 9); // Bit9: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3619, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 10); // Bit10: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3620, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 11); // Bit11: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3621, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 12); // Bit12: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3622, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 13); // Bit13: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3623, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 14); // Bit14: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3624, varV);

		// varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nCompFault2, 15); // Bit15: 预留
		// Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3625, varV);

		// 系统状态故障 (bit0~bit15)
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 0); // Bit0: 电池出水压力过高告警（D0）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3626, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 1); // Bit1: 模块出水压力过高告警（D1）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3627, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 2); // Bit2: 电池进/出水温度过高过低（D2）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3628, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 3); // Bit3: 模块进/出水温度过高过低（D3）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3629, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 4); // Bit4: 过热度过低告警（D4）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3630, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 5); // Bit5: 系统高压保护（D5）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3631, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 6); // Bit6: 系统低压保护（D6）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3632, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 7); // Bit7: 水箱缺液告警（D7）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3633, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 8); // Bit8: 模块系统缺液告警（D8）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3634, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 9); // Bit9: 电池系统缺液告警（D9）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3635, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 10); // Bit10: 电池冷却水温差告警（DA）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3636, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 11); // Bit11: 模块冷却水温差告警（DB）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3637, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 12); // Bit12: 冷却水流量告警（DC）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3638, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 13); // Bit13: 排气高温告警（DD）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3639, varV);

		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 14); // Bit14: 空调冷却水温过高过低（DE）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3640, varV);
		
		varV.emValue = GET_BIT(pSTBatt_Rough->stIMEU3AirStatusFault[n].nSysStatFault, 15); // Bit15: 电池侧进出水温差保护（DF）
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3641, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fBattPumpHz;    // 电池水泵频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3700, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fBattPumpDuty;  // 电池水泵占空比
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3701, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fModulePumpHz;   // 模块水泵频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3702, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fModulePumpDuty; // 模块水泵占空比
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3703, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fDrvFanHz;   // 驱动器散热风机输出频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3704, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fDrvFanDuty; // 驱动器散热风机输出占空比
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3705, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fFanHz;       // 风机频率
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3706, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirStatusFault[n].fFanDuty;     // 风机占空比
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3707, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].byCellPwrState; // 电池测开关机 //edit by kuangyue 20250811
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3718, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].byPcsPwrState; // 模块侧开关机
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3719, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].usBatteryMode; // 电池侧工作模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3720, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].byControlObj; // 进出水控制目标：0：控制出水温度，1：控制进水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3721, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fCoolingTemp; // 设置制冷温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3722, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fHeatingTemp; // 设置制热温度	
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3723, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fCoolingTempDiff; // 设置制冷启动温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3724, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fHeatingTempDiff; // 设置制冷停止温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3725, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fCoolingStopTempDiff; // 设置制热启动温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3726, varV);

		varV.fValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].fHeatingStopTempDiff; // 设置制热停止温差
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3727, varV);

		varV.emValue = pSTBatt_Rough->stIMEU3AirControlInfo[n].byNoiseReductionMode; // 降噪模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3728, varV);
	}
}

static void Push_XinMiao_AirStatusData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	//edit by kuangyue 20250217
	VAR_VALUE varV;
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("Push_XinMiao_AirStatusData ----Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		
		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nPumpStat;   // 水泵运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3642, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nComp1Stat; // 1#压缩机运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3643, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nComp2Stat; // 2#压缩机运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3644, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nHeatStat; // 电加热运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3645, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nCoolFanStat; // 冷凝风机运行状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3646, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fOutTemp; // 机组出水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3647, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fRetTemp; // 机组回水温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3648, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fOutPress; // 机组出水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3649, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fRetPress; // 机组回水压力
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3650, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fenvTempFault; // 机组外环境温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3651, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nMakeUpPump; // 补液水泵状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3652, varV);

		varV.ulValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nFaultCode; // 故障告警代码
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3653, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nWorkMode; // 液冷机组控制模式
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3654, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].nPowerStat; // 开机状态
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3655, varV);

		varV.fValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].fTgtTemp; // 机组设定温度
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3656, varV);

		varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.paramErr;//edit by kuangyue 20250306
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3657, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.phaseSeqFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3658, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.phaseLoss;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3659, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.overSpeed;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3660, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.retTempFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3661, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.outTempFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3662, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.envTempFaultFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3663, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.exhaustTempFaultFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3664, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.suctionTempFaultFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3665, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.comm485FaultFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3666, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.boardCommFaultFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3667, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.outPressFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3668, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.inPressFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3669, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.compOverCurr;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3670, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.pressDiffLow;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3671, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.coolTempLow;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3672, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.outTempHigh;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3673, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.exhaustTempHigh;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3674, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.highPressFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3675, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.lowPressFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3676, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.highPressProt;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3677, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.lowPressProt;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3678, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.waterLow;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3679, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.levelSwitch;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3680, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.fan1Fault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3681, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.outPressHigh;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3682, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.drvModuleFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3683, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.busVoltAbn;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3684, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.drvTempHigh;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3685, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.compStartFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3686, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.compStepLoss;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3687, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.freqOverCurr;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3688, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.rectSoftOver;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3689, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.rectHardOver;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3690, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.fan2Fault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3691, varV);

        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.pumpFault;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3692, varV);
		
        varV.emValue = pSTBatt_Rough->stXinMiaoAirStatusFault[n].faultBits.Faults.freqInstOver;
        Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 951 + n, 3693, varV);

	}
}

BOOL Send_Ctrl_STBatt(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nBattIdx)
{
	/*if(pSTBatt_Rough->emBattMFG == BATT_MFG_PL)
	{
		return PL_Batt_Control(pSTBatt_Rough, emCmdTp, nBattIdx);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_ND)
	{
		return ND_Batt_Control(pSTBatt_Rough, emCmdTp, nBattIdx);
	}
	else*/ if((pSTBatt_Rough->emBattMFG == BATT_MFG_ND_V33) ||
	(pSTBatt_Rough->emBattMFG == BATT_MFG_NIFY_V33_500K)||
	(pSTBatt_Rough->emBattMFG == BATT_MFG_IMTCU_V33_500K))
	{
		return ND_V33_Batt_Control(pSTBatt_Rough, emCmdTp, nBattIdx);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K|| pSTBatt_Rough->emBattMFG == BATT_MFG_TEST_NO_BATT)
	{
		return YFY_Batt_Control(pSTBatt_Rough, emCmdTp, nBattIdx);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_FENGC_MODBUS)
	{
		return FengCH_Batt_Control(pSTBatt_Rough, emCmdTp, nBattIdx);
		TRACEX("Ctrl: Fengchao Modbus PROTOCAL !!!\n");
	}

	return FALSE;
}

BOOL Send_Ctrl_CCU(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, GRP_RECT_CTRL_CMDTYPE emCmdType, int nBattIdx)
{
	float 		fLimitCurrent = 0.0;
	VAR_VALUE 	varV;

	pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx] = GetCurrentTime();

	A_BLOB_GRP_RECT_AUTOCTRL_CMD	blobGrpRectCtrl;
	ZERO_POBJS(&blobGrpRectCtrl, 1);
	varV.blobValue = (BYTE *)&blobGrpRectCtrl;
	blobGrpRectCtrl.emCtrl_Type = emCmdType;
	blobGrpRectCtrl.nGrp = nBattIdx;
 	if(emCmdType <= GRP_ACDC_STOP)
	{
		//ACDC Running Rectfier Mode
		if(emCmdType == GRP_ACDC_RECTIFY_START)
		{
			blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
			
			if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt > 50.0)
			{
				if(pSTBatt_Rough->bBattProt == TRUE)
				{
					fLimitCurrent = 1.5;
				}
				else
				{
					fLimitCurrent = (pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr*1000)/(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt+2);
				}
			}
			blobGrpRectCtrl.fCtrlPower = pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr;
			blobGrpRectCtrl.fGrpCurrLmt = MIN(fLimitCurrent, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxChargeCurr);
			TRACE("Rectify Mode Ctrl nBattIdx=[%d]:fLimitCurrent=%.1f fAllow_MaxChargeCurr=%.1f\n", \
				nBattIdx, fLimitCurrent, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxChargeCurr);
		}
		else if(emCmdType == GRP_ACDC_RECTIFY_ADJ_CURR)//John Added For PreCharge Function.
		{
			blobGrpRectCtrl.fGrpVoltLmt = pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt - 10;
			blobGrpRectCtrl.fGrpCurrLmt  = 0.0;
		}
		else if((emCmdType == GRP_ACDC_INVERT_START))		//ACDC Running Invert Mode
		{
			if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt > 50.0)
			{	
				if(pSTBatt_Rough->bBattProt == TRUE)
				{
					fLimitCurrent = 0.5;
				}
				else
				{	
					//20230228.  due to 62.5 is allow overload. +1~5 is avoid Overload;
					fLimitCurrent = (pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr*1000)/(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt+10);
				}
			}
			blobGrpRectCtrl.fCtrlPower = pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr;
			blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
			blobGrpRectCtrl.fGrpCurrLmt = MIN(fLimitCurrent, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxDisChargeCurr);
			TRACE("Invert Mode Ctrl nBattIdx=[%d]:emCmdType=[%d] fLimitCurrent=%.1f fAllow_MaxDisChargeCurr=%.1f\n", \
				nBattIdx, emCmdType, fLimitCurrent,pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxDisChargeCurr);
		}
		else if(emCmdType == GRP_ACDC_INVERT_PENDING)//Used For On-Grid to Ensure PCS is On_WaitingRun Status;
		{
			if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
			{
				blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt - 20);
				if(pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == 1) //1 离网逆变、0 并网整流(空闲状态会进入)
				{
					blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_INVERT_START;
				}
				else  
				{
					blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_RECTIFY_START; 
				}
			}
			else
			{
				blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
				blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_INVERT_START;//Specical Process.IMSU Not Start Dischg if PCS is Off State.
			}
			blobGrpRectCtrl.fGrpCurrLmt = 0.0;
		}
		else if(emCmdType == GRP_ACDC_STOP)
		{
			// if((pSTBatt_Rough->emSTSAutoManual == AUTO_ONOFFGRID_MODE) &&
			if(JudgeIfPCS_OnHotWaiting(pSTBatt_Rough, nBattIdx) == TRUE)
			{
				TRACE("Auto:nBattIdx=[%d] emSTSAutoManual=[%d] \n", nBattIdx,pSTBatt_Rough->emSTSAutoManual);
				blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
				pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx] = _NOW_;
				if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
				{
					blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalVolt - 20);
					if(pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == 1) //1 离网逆变、0 并网整流（满充后会进入）
					{
						blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_INVERT_START;
					}
					else  
					{
						blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_RECTIFY_START; 
					}					
				}
				else
				{
					blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
					blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_INVERT_START;//Specical Process.IMSU Not Start Dischg if PCS is Off State.
					pSTBatt_Rough->bHotWaitFlag = 1;
				}
				if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC <= pSTBatt_Rough->nProtectSOC)
				{ //电池SOC小于等于保护SOC时，关闭模块
					blobGrpRectCtrl.emCtrl_Type = GRP_ACDC_STOP;
				}
			}
			else
			{
				blobGrpRectCtrl.fGrpVoltLmt = MIN(1500, pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt);
				blobGrpRectCtrl.fGrpCurrLmt = 0.0;
				pSTBatt_Rough->tm_LastStopChg_4STBatt[nBattIdx] = _NOW_;
			}
		}
		
		pSTBatt_Rough->fLastSendReqVolt_2CCU[nBattIdx] = blobGrpRectCtrl.fGrpVoltLmt;
		pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx] = blobGrpRectCtrl.fGrpCurrLmt;
		pSTBatt_Rough->emLastCtrlPcsCmd[nBattIdx] = blobGrpRectCtrl.emCtrl_Type;
	}
	else
	{
		
	}

	TRACEX("nBattIdx=[%d] cmdType=[%d, %.1f, %.1f] \
		fAllow_LmtChargeVolt=%.1f fAllow_MaxChargeCurr=%.1f  \
		fLimitCurrent=%.1f fACDC_Distri_4_BattChrg_Pwr =%.1f \
		fAllow_MaxDisChargeCurr=%.1f fBatt_Distri_4_ACDC_Pwr=[%.1f]to CCU\n", \
			nBattIdx, 	blobGrpRectCtrl.emCtrl_Type,\
			blobGrpRectCtrl.fGrpVoltLmt, blobGrpRectCtrl.fGrpCurrLmt,\
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_LmtChargeVolt,\
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxChargeCurr, fLimitCurrent,\
			pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr,\
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fAllow_MaxDisChargeCurr,\
			pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr);

	Push_STBatt_AllData(pSTBatt_Rough);//need update data before sending out the cmd
	//if((GRP_ACDC_RECTIFY_START == emCmdType) || (GRP_ACDC_STOP == emCmdType))
	//{
	//	//return Wait_BlobCtrlCmd(pSTBatt_Rough->hSampler, 200, 231, (void*)varV.blobValue, 
	//	//	sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD), BLOB_TYPE_RECTGRP_AUTOCTRLCMD, 500);
	//	Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
	//		200, 232, (void*)varV.blobValue, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD), BLOB_TYPE_RECTGRP_AUTOCTRLCMD);
	//
	//}

	Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 
			200, 232, (void*)varV.blobValue, sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD), BLOB_TYPE_RECTGRP_AUTOCTRLCMD);
	
	return TRUE;
}

BOOL BattProtect(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMax_CellVolt < 1 ||
		pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMin_CellVolt < 1)
	{
		return TRUE; // 刚上电 值为0
	}

	double tmNow = GetCurrentTime();
	// 无需判断系统是否可用，之前已检查过

	// 防欠压（必需） 防过充（可选按钮）

	if(ElapseS(pSTBatt_Rough->singleBattData[nBattIdx].pCurrState->tm_Enter[nBattIdx]) >= 15*60
		&& (fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= MAX_PWRONPCS_SEC))
	{
		if(pSTBatt_Rough->bAntiOverChg == 1
			&& pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMax_CellVolt > 3.5) //即将过充
		{
			pSTBatt_Rough->bBattProt = TRUE;
			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_START, nBattIdx);
		}
		else if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMin_CellVolt < 3.0) // 即将欠压
		{
			pSTBatt_Rough->bBattProt = TRUE;
			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_RECTIFY_START, nBattIdx);
		}
		else if((pSTBatt_Rough->bBattProt == TRUE)
			&& ((pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMax_CellVolt <= 3.335 && pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC > 90)
				|| (pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMin_CellVolt >= 3.18 && pSTBatt_Rough->blobSingleBattInfo[nBattIdx].nSOC < 10)))
		{
			pSTBatt_Rough->bBattProt = FALSE;
			Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_INVERT_PENDING, nBattIdx);
		}
	}

	return TRUE;
}

int IMEU2SendSetCAN(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	DATA_PROCESSOR* pDPR = pSTBatt_Rough->pDPR;

	int nBattIdx = nDevId - 601;
	int nWhichIMEU = nBattIdx;
	BYTE byCRC = 0;

	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	BYTE byEMSAddr = 0x5A + nBattIdx;
	BYTE byIMEU2Addr = 0x28 + nBattIdx;

	frmSent.uiFrmID = 0x05DE0000;
	frmSent.uiFrmID |= (byIMEU2Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;

	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	if(459 == nParamId) //Fire Protect Type
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x05;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(480 == nParamId) //Humi Temp Sensor Number
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x01;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(481 == nParamId) //Comb Gas Sensor Cfg, enable or disable
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x02;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(482 == nParamId) //Comb Gas Sensor Number 
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x04;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(483 == nParamId) //Gas Sensor Maunfactuer
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x03;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(484 == nParamId) //Hydrogen Discharge Interval Without Host 
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x06;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(485 == nParamId) //Hydrogen Discharge Interval Without Host 
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x07;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(489 == nParamId) //Ctrl Shutter Action Time
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x08;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(490 == nParamId) //Gas PPM Alarm Threshold
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x08;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;	
	}
	else if(494 == nParamId) //PT1000 Number Config
	{
		pSTBatt_Rough->IMEU2CANRoughData[nWhichIMEU].nPT1000Num = pVarVal->lValue;
		return  ERR_CONTROL_OK;
	}
	else
	{
		TRACE("Error: Not Found IMEU Setting Cmd\n");
		return  ERR_CONTROL_FAILED;
	}

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;

	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, frmSent.byRawData, 1);
		}
	}
	return 0;
}

//IMEU3  send set cmd
int IMEU3SendSetCAN(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	DATA_PROCESSOR* pDPR = pSTBatt_Rough->pDPR;

	int nBattIdx = nDevId - 601;
	int nWhichIMEU = nBattIdx;
	BYTE byCRC = 0;

	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	BYTE byEMSAddr = 0x5A + nBattIdx;
	BYTE byIMEU2Addr = 0x1E + nBattIdx;

	frmSent.uiFrmID = 0x05F00000;
	frmSent.uiFrmID |= (byIMEU2Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;

	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	if(610 == nParamId) // Temperature parameter configuration
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x00;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(611 == nParamId) //DI channel parameter configuration
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x01;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(612 == nParamId)  //PWM parameter configuration
	{  
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x02;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(613 == nParamId)  //Pressure sensor configuration
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x03;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(614 == nParamId) //Combustible gas sensor configuration
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x04;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(615 == nParamId) //Configuration of 485 temperature and humidity sensors
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x05;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else if(616 == nParamId) //Dehumidifier configuration
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x06;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
	}
	else
	{
		TRACE("Error: Not Found IMEU Setting Cmd\n");
		return  ERR_CONTROL_FAILED;
	}

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;

	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, frmSent.byRawData, 1);
		}
	}
	return 0;
}



int IMEU3SendCtlCAN(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	if ((nDevId < 611 || nDevId > 614) && (nDevId < 601 || nDevId > 604))
	{
		return;
	}

	DATA_PROCESSOR* pDPR = pSTBatt_Rough->pDPR;

	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));
	

	int iDevIdx = nDevId - 611;
	

	if(601 == nDevId)
	{
		if (nParamId == 602)// 600 601 2524不需要控制DO1、2口
		{
			frmSent.uiFrmID = 0x05F20000;
			frmSent.byData[0] = 0xC1;
			frmSent.byData[1] = (BYTE)(nParamId - 599); // DO 1~6
			frmSent.byData[2] = (BYTE)pVarVal->emValue;
		}
		else
		{
			TRACE("Error: Not Found IMEU Control Cmd\n");
			return  ERR_CONTROL_FAILED;
		}

	}
	else if(611 == nDevId)
	{
		if (nParamId >= 38 && nParamId <= 48)
		{
			frmSent.uiFrmID = 0x05F20000;
			frmSent.byData[0] = 0xC1;
			frmSent.byData[1] = (BYTE)(nParamId - 37); // DO 1~6
			frmSent.byData[2] = (BYTE)pVarVal->emValue;
		}
		else
		{
			TRACE("Error: Not Found IMEU Control Cmd\n");
			return  ERR_CONTROL_FAILED;
		}
		
	}
	else if (612 == nDevId)
	{
		if (nParamId >= 509 && nParamId <= 519)
		{
			frmSent.uiFrmID = 0x05F20000;
			frmSent.byData[0] = 0xC1;
			frmSent.byData[1] = (BYTE)(nParamId - 508); // DO 1~6
			frmSent.byData[2] = (BYTE)pVarVal->emValue;
		}
		else if (525 == nParamId)
		{
			frmSent.uiFrmID = 0x05F10000;
			frmSent.byData[0] = 0xC1;
			frmSent.byData[1] = 0;
			frmSent.byData[2] = (BYTE)pVarVal->lValue;
		}
		else
		{
			TRACE("Error: Not Found IMEU Control Cmd\n");
			return  ERR_CONTROL_FAILED;
		}
		
	}
	else
	{
		TRACE("Error: Not Found IMEU Control Cmd\n");
		return  ERR_CONTROL_FAILED;
	}

	BYTE byEMSAddr = 0x5A;
#ifdef IMMU3
	BYTE byIMEU3Addr = 0x1F;
#else
	BYTE byIMEU3Addr = getIMEU3Addr(iDevIdx);
#endif
	
	frmSent.uiFrmID |= (byIMEU3Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;
	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, 0, frmSent.byRawData, 1);
		}
	}


	int k = 0;
	TRACE("IMEU3SendCtlCAN  write: nDevId = %d nParamId = %d\n", nDevId, nParamId);
	for (k = 0; k < CAN_FRAME_LEN; ++k)
	{
		TRACE("%02x ", frmSent.byRawData[k]);
	}

	TRACE("\n");

	return 0;
}

int IMEU3CtlPWM(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	int Value = 0;
	int nBattIdx = 0;
	float MPPTAllPwr = 0;
	for(nBattIdx = 0; nBattIdx < MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
		MPPTAllPwr +=(pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Volt*pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Curr*0.001);
	}
	if(MPPTAllPwr >= 80)
	{
		Value = 100;
	}
	else
	{
		Value = MPPTAllPwr/80 * 100;
	}
	IMEU3_CAN_DataInfo* pIMEU3_Power = &pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo;
	float MaxTemp = 0;
	MaxTemp = MAX(MAX(MAX(pIMEU3_Power->fNTC[0], pIMEU3_Power->fNTC[1]), pIMEU3_Power->fNTC[2]), pIMEU3_Power->fNTC[3]);
	if(MaxTemp > 30)
	{
		Value = MAX(Value, 30);
	}
	Value = MAX(Value, 15); //系统运作至少按15转风扇
	frmSent.uiFrmID = 0x05F10000;
	frmSent.byData[0] = 0xC4;
	frmSent.byData[1] = 0x00; 
	frmSent.byData[2] = 0x00;
	frmSent.byData[3] = (BYTE)Value;

	BYTE byEMSAddr = 0x5A;
	BYTE byIMEU3Addr = 0x1E;
	
	frmSent.uiFrmID |= (byIMEU3Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;
	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, 0, frmSent.byRawData, 1);
		}
	}

	return 0;
}

int IMEU3SendDOCtlCAN(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int BatIdx, BYTE WhichDo, BYTE Value)
{
	BYTE szIMEU3MapIdx[]={0x1E, 0x1F};  // 电源、 电池EU3
	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	frmSent.uiFrmID = 0x05F20000;
	frmSent.byData[0] = 0xC1;
	frmSent.byData[1] = (BYTE)WhichDo; 
	frmSent.byData[2] = (BYTE)Value;

	BYTE byEMSAddr = 0x5A;
	BYTE byIMEU3Addr = szIMEU3MapIdx[BatIdx];
	
	frmSent.uiFrmID |= (byIMEU3Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;
	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, 0, frmSent.byRawData, 1);
		}
	}


	return 0;
}

static void IMEU3DoCtl(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	IMEU3_CAN_DataInfo* pIMEU3_Bat = &pSTBatt_Rough->IMEU3CANRoughData[0].stIMEU3DataInfo;  //电池仓EU3数据
	IMEU3_CAN_DataInfo* pIMEU3_Power = &pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo; //电源仓EU3数据

	BYTE value = 0;
	VAR_VALUE varV;

	int i = 0;
	for(i = 0; i < MAX_SINGLE_STBATT_NUM; i++) //电池组二级和三级消防告警，触发EU3声光报警
	{
		value += GET_BIT(pSTBatt_Rough->blobIfyBMS[i].nAlarmInfo4, 11);
		value += GET_BIT(pSTBatt_Rough->blobIfyBMS[i].nAlarmInfo4, 12);
	}
	if(value > 0)
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 1, 1);  //声光报警DO控制
	}
	else
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 1, 0);  //告警消失，关闭报警器
	}

	pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 161, &varV, 4); //0:并网 1：离网
	if(varV.lValue == 1) //edit by kuangyue 20251029
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 4, 1);   
	}
	else
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 4, 0);
	}

	if(pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].bIsCommFail == FALSE) //电源EU3通信正常 才下发空调开关
	{

		if((pSTBatt_Rough->nAirCondi_StartUp_Num < 3) && (ABS(_NOW_ - pSTBatt_Rough->t_AirCondi_StartUp_Time) > AirCondi_Interval_Time))
		{
			pSTBatt_Rough->nAirCondi_StartUp_Num++;
			pSTBatt_Rough->t_AirCondi_StartUp_Time = _NOW_;

			IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 5, 1);   
			IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 6, 1);   
		}

		if((UINT)pSTBatt_Rough->stAirStatusAlarm[0].fBatteryInletTemp == 0) //第一组电池空调，没开机就强制闭合接触器
		{
			IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 5, 1);   
		}

		if((UINT)pSTBatt_Rough->stAirStatusAlarm[1].fBatteryInletTemp == 0) //第二组电池空调，没开机就强制闭合接触器
		{
			IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 6, 1);   
		}

		if((pSTBatt_Rough->nSmokeDetector_num < 10) && (ABS(_NOW_ - pSTBatt_Rough->t_Smoke_time) > 8)) //MU2上电后，给烟感上电，多上几次电防止控不住
		{
			pSTBatt_Rough->nSmokeDetector_num++;
			pSTBatt_Rough->t_Smoke_time = _NOW_;

			IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 8, 1); 
		}
	}

    value = GET_BIT(pIMEU3_Power->uDIStatus, 7);
	if((value == 0) && (pSTBatt_Rough->bSmokeFlag == TRUE))
	{
		pSTBatt_Rough->bSmokeFlag = FALSE;
		pSTBatt_Rough->t_Smoke_time = _NOW_;
		
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 8, 0);   //烟感SS电源DO控制  下电
	}
	else if(pSTBatt_Rough->bSmokeFlag == FALSE)  
	{
		pSTBatt_Rough->t_Smoke_time = _NOW_;
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 0, 8, 1); 
	}

	if((pSTBatt_Rough->bSmokeFlag == FALSE) && (ABS(_NOW_ - pSTBatt_Rough->t_Smoke_time) > 5*60)) //过五分钟可以重置检测
	{
		pSTBatt_Rough->bSmokeFlag = TRUE;
		pSTBatt_Rough->t_Smoke_time = _NOW_;
	}

	value = GET_BIT(pIMEU3_Bat->uDIStatus, 0);    //电池舱前门
	if(value > 0)
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 1, 1, 1);   //打开DO1 前门照明灯
	}
	else
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 1, 1, 0);   //关门，关灯
	}

	value = GET_BIT(pIMEU3_Bat->uDIStatus, 1);    //电池舱后门
	if(value > 0)
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 1, 2, 1);   //打开DO2 后门照明灯
	}
	else
	{
		IMEU3SendDOCtlCAN(pSTBatt_Rough, 1, 2, 0);   //关门，关灯
	}

	return;
}

static int BatSendSetCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
	{
		return  ERR_CONTROL_FAILED;
	}
	
	int nBattIdx = nDevId - 951;
	BYTE byCRC = 0;
	if(nBattIdx >= MAX_SINGLE_STBATT_NUM)
	{
		return  ERR_CONTROL_FAILED;
	}
	A_CAN_ND_BATT_FRAME frmSent;
	ZERO_POBJS(&frmSent, 1);
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	BYTE byEMSAddr = 0x5A;
	BYTE byBatAddr = 0x32 + nBattIdx;

	frmSent.uiFrmID = 0x05560000;
	frmSent.uiFrmID |= (byBatAddr << 8);
	frmSent.uiFrmID |= byEMSAddr;

	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	if(445 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x09;
		switch(pVarVal->emValue)
		{
			case 0: 
				frmSent.byData[2] = 0x01;
				break;
			case 1: 
				frmSent.byData[2] = 0x02;
				break;
			case 2: 
				frmSent.byData[2] = 0x04;
				break;
			default:
				frmSent.byData[2] = 0x00;
				break;
		}
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(444 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		if(pSTBatt_Rough->emAirType[nBattIdx] == TONGFEI_AIR)
		{
			frmSent.byData[1] = 0x0A;
		}
		else if(pSTBatt_Rough->emAirType[nBattIdx] == XINMIAO_AIR)
		{
			frmSent.byData[1] = 0x0B;
		}
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	//EU3 Air
	else if(529 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x0D;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(530 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x0E;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(531 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x0F;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(532 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x10;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(533 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x11;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(534 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x12;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(535 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x13;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(536 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x14;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(537 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x15;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;

		// 标记待清除状态
		if (frmSent.byData[2] == 0x01) 
		{
			pSTBatt_Rough->bAir537NeedClear[nBattIdx] = TRUE; 
		} else {
			pSTBatt_Rough->bAir537NeedClear[nBattIdx] = FALSE;
		}

	}
	else if(538 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x16;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(539 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x17;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(540 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x18;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(541 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x19;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(542 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x1A;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(543 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x1B;
		frmSent.byData[2] = (BYTE)pVarVal->emValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}
	else if(544 == nParamId)
	{
		frmSent.byData[0] = 0xC1;
		frmSent.byData[1] = 0x1C;
		frmSent.byData[2] = (BYTE)pVarVal->ulValue;
		frmSent.byData[3] = 0x00;
		frmSent.byData[4] = 0x00;
		frmSent.byData[5] = 0x00;
		frmSent.byData[6] = 0x00;
	}

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;

	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		printf("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, frmSent.byRawData, 1);
		}
	}
	return 0;
}

// 封装537参数清除函数
void ClearAir537Param(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,
		PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	int nBattIdx = nDevId - 951;
    if (!pSTBatt_Rough->bAir537NeedClear[nBattIdx]) {
        return;
    }

    VAR_VALUE var537 = {0};
    var537.emValue = 0x00; // 清除手动控压缩机频率标志为0
	BatSendSetCmd(pSTBatt_Rough, emParaType, nDevId, 537, &var537, sizeof(var537), nTimeOut);
    // 清除标记（避免重复清除）
    pSTBatt_Rough->bAir537NeedClear[nBattIdx] = FALSE;
}

static void CalcGroupBatt_RunInfo(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n;
	A_BLOB_GROUP_STBATT_RUNINFO* pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	pGrpRunInfo->fBattBusCurr = 0.0;
	pGrpRunInfo->fBattBusVolt = 0.0;
	pGrpRunInfo->fMaxSOC = 0.0;
	pGrpRunInfo->fMinSOC = 100.0;
	pGrpRunInfo->fAvgSOC = 0.0;
	pGrpRunInfo->fMaxSOH = 0.0;
	pGrpRunInfo->fMinSOH = 100.0;
	pGrpRunInfo->fAvgSOH = 0.0;
	pGrpRunInfo->fAll_Allow_LmtChargeVolt = 9999.0;
	pGrpRunInfo->fAll_Allow_LmtDisChargeVolt = 0.0;
	pGrpRunInfo->fAll_Allow_MaxChargeCurr = 0.0;
	pGrpRunInfo->fAll_Allow_MaxDisChargeCurr = 0.0;
	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = NULL;
	for(n = 0; n < pGrpRunInfo->nTotalOnlineBatt; n++)
	{
		pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
		//calc bus volt/curr
		pGrpRunInfo->fBattBusCurr += pSingleRunInfo->fPackTotalCurr;
		pGrpRunInfo->fBattBusVolt = MAX(pGrpRunInfo->fBattBusVolt, pSingleRunInfo->fPackTotalVolt);
		
		pGrpRunInfo->fMaxSOC = MAX(pGrpRunInfo->fMaxSOC, (float)pSingleRunInfo->nSOC);
		pGrpRunInfo->fMinSOC = MIN(pGrpRunInfo->fMinSOC, (float)pSingleRunInfo->nSOC);
		pGrpRunInfo->fAvgSOC += (float)pSingleRunInfo->nSOC;
		
		pGrpRunInfo->fMaxSOH = MAX(pGrpRunInfo->fMaxSOH, (float)pSingleRunInfo->nSOH);
		pGrpRunInfo->fMinSOH = MIN(pGrpRunInfo->fMinSOH, (float)pSingleRunInfo->nSOH);
		pGrpRunInfo->fAvgSOH += (float)pSingleRunInfo->nSOH;

		pGrpRunInfo->fAll_Allow_LmtChargeVolt = MIN(pGrpRunInfo->fAll_Allow_LmtChargeVolt, pSingleRunInfo->fAllow_LmtChargeVolt);
		pGrpRunInfo->fAll_Allow_LmtDisChargeVolt = MIN(pGrpRunInfo->fAll_Allow_LmtChargeVolt, pSingleRunInfo->fAllow_LmtDisChargeVolt);
		pGrpRunInfo->fAll_Allow_MaxChargeCurr += pSingleRunInfo->fAllow_MaxChargeCurr;
		pGrpRunInfo->fAll_Allow_MaxDisChargeCurr += pSingleRunInfo->fAllow_MaxDisChargeCurr;
	}

	pGrpRunInfo->fAvgSOC /= pGrpRunInfo->nTotalOnlineBatt;
	pGrpRunInfo->fAvgSOH /= pGrpRunInfo->nTotalOnlineBatt;

	//do not allow charge if the batt is in sleep or charging
	if(pGrpRunInfo->emBattGrpRunStatus <= STBATT_RUNSTAT_SLEEP)
	{
		pGrpRunInfo->fAll_Allow_MaxDisChargeCurr = 0.0;
		pGrpRunInfo->fAll_Allow_MaxChargeCurr = 0.0;
		//pGrpRunInfo->fAll_Allow_MaxChargeCurr_V2B = 0.0;
	}
	
 }

#define STBATT_STOP_TYPE_CHG	0x01
#define STBATT_STOP_TYPE_DISCHG	0x02

static void Stop_STBatt_ChargeOrDischarge(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, A_BLOB_ES_RUN_DATA* pESRunData, int nStopTy, int nBattIdx)
{
	pSTBatt_Rough->blobGroupBattInfo.emBattGrpRunStatus = STBATT_RUNSTAT_SLEEP;
	pSTBatt_Rough->blobGroupBattInfo.fAll_Allow_LmtChargeVolt = 0.0;
	pSTBatt_Rough->blobGroupBattInfo.fAll_Allow_LmtDisChargeVolt = 0.0;
	pSTBatt_Rough->blobGroupBattInfo.fAll_Allow_MaxChargeCurr = 0.0;
	pSTBatt_Rough->blobGroupBattInfo.fAll_Allow_MaxDisChargeCurr = 0.0;

	if((nStopTy & STBATT_STOP_TYPE_CHG) == STBATT_STOP_TYPE_CHG)
	{
		// if((pESRunData->emSTBatt_ACDC_WorkStatu == STBATT_ACDC_CHRG2STBATT) ||
		// 	(pESRunData->emDCDC_WorkStatu[0] == DCDC_V2B) || 
		// 	(pESRunData->emDCDC_WorkStatu[1] == DCDC_V2B))
		{
			Send_Ctrl_CCU(pSTBatt_Rough, CTRL_CCU_STOP_CHARGE_4STBatt, nBattIdx);
		}
	}

	// if((nStopTy & STBATT_STOP_TYPE_DISCHG) == STBATT_STOP_TYPE_DISCHG)
	// {
	// 	// if((pESRunData->emDCDC_WorkStatu[0] == DCDC_CHRG2EV) ||
	// 	// 	(pESRunData->emDCDC_WorkStatu[1] == DCDC_CHRG2EV))
	// 	{
	// 		Send_Ctrl_CCU(pSTBatt_Rough, CTRL_CCU_STOP_DISCHARGE_4STBatt, nBattIdx);
	// 	}
	// }
}


/*********************************************************************************
*  
*  FUNCTION NAME :  Judge_BMSIsoTriggerStatus
*  CREATOR       :  
*  CREATED DATE  : 2022-08-13 11:13:53
*  DESCRIPTION   :  
*	YFY_ISOLATION_RESULT	emIsoResult;
	TRIGGER_ISO_STATUS		emTrigIsoStatus;
*  
    TRIG_AUTO_DETECTION = 0,
    TRIG_START_DETECTING,
    TRIG_STOP_DETECTION,
*  CHANGE HISTORY: 
*  
***********************************************************************************/
TRIGGER_ISO_STATUS Judge_BMSIsoTriggerStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
    double tmNow = GetCurrentTime(); 
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
    A_BLOB_IFY_BMS_DATAINFO       *pIfyBms = &pSTBatt_Rough->blobIfyBMS[nBattIdx];      

	if(pSTBatt_Rough->fISO_PeriodMin_Sett < 1.0) 
	{ 
		//printf(" Disable ISO Detection,Do Nothing!!! nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_NO_TRIGGER; 
	}

	if(pSTBatt_Rough->emPMCtrlMode != AUTO_CTRL_PM_MODE)
	{
		return TRIG_NO_TRIGGER; 
	}

	if(pSingleRunInfo->emIsoResult == STAT_AUTO_DETECTION) 
	{ 
		//printf("111 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_STOP_DETECTION;
	}
	else if(pSingleRunInfo->emIsoResult == STAT_ON_DETECTING)
	{
		//if Batt Start Charge Or DisCharge, But BMS is Iso Detecting, So Need Emergency Stop !!! 
		if(fabs(tmNow - pSTBatt_Rough->tm_LastStartChg_4STBatt[nBattIdx]) < 60) 
		{
			//printf("222 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
			return TRIG_STOP_DETECTION; 
		}

		//add 20230420, SBK suggests that DETECT should be stop if DETECTING state over 300s
		if(fabs(tmNow - pSTBatt_Rough->t_LastTrigIsoStartCmd[nBattIdx]) > 300)
		{
			LogOut4c(MAIN_MODULE_SAMPLER, "STBattSamp", LOG_TYPE_ERROR, "DETECTING state over 300S should be stop");
			return TRIG_STOP_DETECTION;
		}

		//printf("333 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_NO_TRIGGER; 
	}
	else if(pSingleRunInfo->emIsoResult == STAT_ON_FINISHED) //After BMS Finished ISO Detection, Need Send Stop Cmd To BMS.
	{
		//printf("444 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_STOP_DETECTION; 
	}

	if(pIfyBms->unioBattAlm.Alarm.BattInsulationAlmLv1 == 1)
	{
		return TRIG_NO_TRIGGER; 
	}

	if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) < 360)
	{
		//printf("555 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_NO_TRIGGER; 
	}

	if(fabs(tmNow - pSTBatt_Rough->t_LastTrigIsoStartCmd[nBattIdx]) < (60*pSTBatt_Rough->fISO_PeriodMin_Sett)) 
	{ 
		//printf("666 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);
		return TRIG_NO_TRIGGER; 
	} 

	if(pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr >= 1.0) 
	{
		//printf("888 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);

		return TRIG_NO_TRIGGER; 
	}

	if(pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr >= 1.0) 
	{
		//printf("999 nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);

		return TRIG_NO_TRIGGER; 
	}

	if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE) 
	{
		//printf("aaa nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);

		return TRIG_NO_TRIGGER; 
	}

	//John 20230827 added. SomeTimes PCS Currt is 0, but Still Running,Upload PCS Status is Idle. 
	if(pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt > 50.0) 
	{
		return TRIG_NO_TRIGGER; 
	}

	//LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,\
	//	"Trig Iso Start:Idx[%d]ISO_Res[%d]!", \
	//	nBattIdx, pSingleRunInfo->emIsoResult);

	pSTBatt_Rough->t_LastTrigIsoStartCmd[nBattIdx] = tmNow;

	//printf("Trigger Start : bbb nBattIdx=[%d] emIsoResult=[%d]\n", nBattIdx, pSingleRunInfo->emIsoResult);

	return TRIG_START_DETECTING;
}


/*********************************************************************************
*  
*  FUNCTION NAME : JudgeIfSkipAutoRun4HT_OnOffGrid
*  CREATOR       :  
*  CREATED DATE  : 2023-12-26 11:13:53
*  DESCRIPTION   : Added By John.Added For Avoid AutoMode Ctrl PCS ON But ABU is Off-Grid,
*				   Will Cause PCS AC Fault.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL JudgeIfSkipAutoRun4HT_OnOffGrid(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	BOOL bRet = FALSE;
	double tmNow = GetCurrentTime();
	A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	if((pSTBatt_Rough->esRunData.emABUInstalled == 1) //1-Installed.
		&& (pSTBatt_Rough->esRunData.mapSTSCode.Status.ManualAuto_Mode == MANUAL_ONOFFGRID_MODE)
		&& (pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_OFFGRID)
		&& (pSTBatt_Rough->esRunData.mapDIStatus.Status.DI_UserGridQF == NOT_CONNECTED)) //1-Disconnected
		{
			bRet = TRUE;

			if((pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL) 
				|| (pSingleRunInfo->emBattStatus_Alm != STBATT_ALMSTAT_NOMAL) 
				|| (pSTBatt_Rough->ccuStatu != CCU_STATE_IDLE))
			{
				TRACE("nBattIdx=[%d]:STBATT_STATE_FAULTED->emBattStatus_Run =%d\n", nBattIdx, pSingleRunInfo->emBattStatus_Run);
				
				if(fabs(tmNow - pSTBatt_Rough->tm_fLastSendCmd_2CCU[nBattIdx]) >= URGET_STBATT_PUSH2CCU_PERIOD)
				{
					if(pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu != STATU_ACDC_IDLE)
					{
						Send_Ctrl_CCU(pSTBatt_Rough, GRP_ACDC_STOP, nBattIdx);
					}
				}
			}
		}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : STBatt_Management
*  CREATOR       :  
*  CREATED DATE  : 2023-12-26 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void STBatt_Management(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	STBATT_STATEMACHINE_STATE 	emState = STBATT_STATE_IDLE;
	int 						nErr = ERR_MUTEX_OK;
	int 						n = 0;
	ERRCDDE_DPR 				err;

	//get Branch Rect Grp Data
	VAR_VALUE varV;
	err =  pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 4, &varV, 4);//ccu状态
	pSTBatt_Rough->ccuStatu = (CCU_STATE)varV.emValue;

	//---John 2021-09-02,Manual Ctrl Mode, If Battery Not Avaliable,Should Stop PM
	pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 200, 217,  &varV, 4);//控制模式
	pSTBatt_Rough->emPMCtrlMode = (STAT_PM_CTRL_MODE)varV.emValue;
	/* if(pSTBatt_Rough->emPMCtrlMode !=  pSTBatt_Rough->emLast_PMCtrlMode)
	{
		pSTBatt_Rough->emLast_PMCtrlMode = pSTBatt_Rough->emPMCtrlMode;
		Prepare_Test_StateMachine(pSTBatt_Rough);
	} */

	varV.blobValue = (BYTE *)&pSTBatt_Rough->esRunData;
	err = pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 80, &varV, sizeof(A_BLOB_ES_RUN_DATA));//实时储能系统运行数据

	for( n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{	
		varV.blobValue = (BYTE *)&pSTBatt_Rough->branchGrpData[n];
 		err = pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 
			211 + n, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA));//支路模块组数据
		if(err != ERR_DPR_OK)
		{
			TRACE("Get Branch=[%d] err = [%d]\n", n, err);
			//return;
		}
	}	
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
 		varV.blobValue = (BYTE*)&pSTBatt_Rough->calcNodeData[n];
		err = pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 
			221 + n, 203, &varV, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA));//支路节点计算数据
		if(err != ERR_DPR_OK)
		{
			TRACEX("Get calcNodeData=[%d] err = [%d]\n", n, err);
			//return;
		}

		//TRACEX("NodeIdx=[%d] fAvg_ChrgLmtPower=[%.1f]fAvg_DisChrgingLmtPower=[%.1f]\
		//	fACDC_Distri_4_BattChrg_Pwr=[%.1f]fBatt_Distri_4_ACDC_Pwr=[%.1f]fACDC_Max_Ability_Pwr=[%.1f]\n", \
		//	n,pSTBatt_Rough->calcNodeData[n].fAvg_ChrgLmtPower,\
		//	pSTBatt_Rough->calcNodeData[n].fAvg_DisChrgingLmtPower,\
		//	pSTBatt_Rough->calcNodeData[n].fACDC_Distri_4_BattChrg_Pwr,\
		//	pSTBatt_Rough->calcNodeData[n].fBatt_Distri_4_ACDC_Pwr,\
		//	pSTBatt_Rough->calcNodeData[n].fACDC_Max_Ability_Pwr);
 
	}
	
	//now run the state-machine
	nErr = Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			if(pSTBatt_Rough->emPMCtrlMode == AUTO_CTRL_PM_MODE)
			{
				if(JudgeIfSkipAutoRun4HT_OnOffGrid(pSTBatt_Rough, n))
				{
					continue;
				}
				emState = pSTBatt_Rough->singleBattData[n].pCurrState->pfnAction(pSTBatt_Rough, n);
				pSTBatt_Rough->singleBattData[n].pLastState = pSTBatt_Rough->singleBattData[n].pCurrState;
				pSTBatt_Rough->singleBattData[n].pCurrState = &pSTBatt_Rough->stateMachs[emState];
				if(pSTBatt_Rough->singleBattData[n].pCurrState != pSTBatt_Rough->singleBattData[n].pLastState)
				{
					pSTBatt_Rough->singleBattData[n].pCurrState->tm_Enter[n] = GetCurrentTime();
				}
				// printf("AutoMode.\n");
			}
			else if(pSTBatt_Rough->emPMCtrlMode == MANUAL_CTRL_PM_MODE)
			{	
				// printf("temp num: %.1f\n", pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num);
				Enter_ManualCtrlPMMode(pSTBatt_Rough, n);
				// printf("ManualMode.\n");
			}
			else if(n > 0)
			{
				continue;
			}
			else if(pSTBatt_Rough->emPMCtrlMode == TESTMODE_CTRL_PM_MODE) //单簇测试模式下 先放电
			{	
				// printf("-----------------------------------------------------------------------------\n");
				Enter_BMSTestMode(pSTBatt_Rough, n);
				// printf("temp num: %.1f\n", pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num);
				// printf("-----------------------------------------------------------------------------\n");
			}
			else if(pSTBatt_Rough->emPMCtrlMode == TESTMODE_CTRL_PM_MODE_FIRST_CHARGE) //单簇测试模式下 先充电
			{
				// printf("-----------------------------------------------------------------------------\n");
				Enter_BMSTestMode_FirstCharge(pSTBatt_Rough, n);
				// printf("temp num: %.1f\n", pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num);
				// printf("-----------------------------------------------------------------------------\n");
			}
			else if(pSTBatt_Rough->emPMCtrlMode == TESTMODE_TOW_CTRL_PM_MODE) // 对托测试
			{        		
				// printf("1. before, temp num: %.1f\n", pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num);
				Enter_BMSTestMode_Tow(pSTBatt_Rough, n);
				// printf("2. after, temp num: %.1f\n", pSTBatt_Rough->bmsTestModeParm.initRange.TH_nTempCell_Num);

			}
			else if(pSTBatt_Rough->emPMCtrlMode == TESTMODE_ShortTime_CTRL_PM_MODE) // 短时测试
			{
				Enter_BMSTestMode_ShortTime(pSTBatt_Rough, n);
			}

		}
	
		Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
	}
	return;
}
 

static void CalcBatt_EnergyCost(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	VAR_VALUE varV;
	ERRCDDE_DPR err;

	int n = 0; 
	static BOOL s_bIsFirstCalc = TRUE;
	
	//&pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	double tm_fNow = GetCurrentTime();

	if(s_bIsFirstCalc)
	{
		s_bIsFirstCalc = FALSE;
		pSTBatt_Rough->tm_fLastCalcBattEnergyCost = tm_fNow;

		//Get Battery His Generate KWH
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 501 + n, 504 , &varV, 4);
			if(err == ERR_DPR_OK)
			{
				pSTBatt_Rough->fCalc_ChargedEnergy[n] = varV.fValue;
				pSTBatt_Rough->fLastCalc_ChargedEnergy[n] = varV.fValue;
			}

			err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 501 + n, 505 , &varV, 4);
			if(err == ERR_DPR_OK)
			{
				pSTBatt_Rough->fCalc_DisChargedEnergy[n] = varV.fValue;
				pSTBatt_Rough->fLastCalc_DisChargedEnergy[n] = varV.fValue;
			}

			TRACE("nBattIDx=[%d] fCalc_ChargedEnergy, fCalc_DisChargedEnergy=%f\n", \
				n, pSTBatt_Rough->fCalc_ChargedEnergy[n], pSTBatt_Rough->fCalc_DisChargedEnergy[n]);
		}
		
		return;
	}

	double tmDiff = (tm_fNow - pSTBatt_Rough->tm_fLastCalcBattEnergyCost);
	if((tmDiff > 0.5) && (tmDiff < 5.0))
	{
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			A_BLOB_SINGLE_STBATT_RUNINFO  *pSingleRunInfo ;
			pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
			double fChrged_kWhCost_Diff = 0.0, fDisChrged_kWhCost_Diff = 0.0;

			if(pSingleRunInfo->fPackTotalCurr > 0.5)
			{
				fChrged_kWhCost_Diff = fabs(pSingleRunInfo->fPackTotalVolt * pSingleRunInfo->fPackTotalCurr * 0.001 * tmDiff/3600); 
				
				if((fChrged_kWhCost_Diff > 0.0005) && (fChrged_kWhCost_Diff < 5.0))
				{
					pSTBatt_Rough->fCalc_ChargedEnergy[n] += fChrged_kWhCost_Diff;
				}
				//TRACE("Chrged_kWhCost_Diff=%f pGrpRunInfo->fAll_ChargedEnergy=%f\n",\
				//	fChrged_kWhCost_Diff , pSTBatt_Rough->fCalc_ChargedEnergy);
			}
			else if(pSingleRunInfo->fPackTotalCurr < -0.5)
			{
				fDisChrged_kWhCost_Diff = fabs(pSingleRunInfo->fPackTotalVolt * pSingleRunInfo->fPackTotalCurr * 0.001 * tmDiff/3600); 
				if((fDisChrged_kWhCost_Diff > 0.0005) && (fDisChrged_kWhCost_Diff < 5.0))
				{
					pSTBatt_Rough->fCalc_DisChargedEnergy[n] += fDisChrged_kWhCost_Diff;
				}	
				//TRACE("DisChrged_kWhCost_Diff=%f pGrpRunInfo->fAll_DisChargedEnergy=%f\n",\
				//	fDisChrged_kWhCost_Diff, pSTBatt_Rough->fCalc_DisChargedEnergy);
			}
			else
			{
				//TRACE("Idle Statu, Do Not Calculate!\n");
 			}
			
			//TRACE("nBattIDx=[%d] tmDiff=%f fChrged_kWhCost_Diff =%f fAll_ChargedEnergy=%f fDisChrged_kWhCost_Diff =%f fAll_DisChargedEnergy =%f\n",\
			//	n,\
			//	tmDiff, \
			//	fChrged_kWhCost_Diff, \
			//	pSTBatt_Rough->fCalc_ChargedEnergy[n], \
			//	fDisChrged_kWhCost_Diff, \
			//	pSTBatt_Rough->fCalc_DisChargedEnergy[n]);
		}	
		pSTBatt_Rough->tm_fLastCalcBattEnergyCost = tm_fNow;

	}
	else if(tmDiff > 5.0)
	{
		pSTBatt_Rough->tm_fLastCalcBattEnergyCost = tm_fNow;
	}	

	return;
}

static void CalcBatt_EnergyCostbyBMS(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{	
	int n = 0; 
	// double tm_fNow = GetCurrentTime();
	// double tmDiff = (tm_fNow - pSTBatt_Rough->tm_fLastCalcBattEnergyCostbyBMS);
	// if (tmDiff > 5.0)
	// {	
	for (n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{	
		STBATT_STATEMACHINE_STATE lastState = pSTBatt_Rough->singleBattData[n].pLastState->emState;
		STBATT_STATEMACHINE_STATE currState = pSTBatt_Rough->singleBattData[n].pCurrState->emState;
		float soc = pSTBatt_Rough->blobIfyBMS[n].fSOC;

		if (currState == STBATT_STATE_IN_CHARGING)
		{
			if (lastState != STBATT_STATE_IN_CHARGING) 
			{
				// Start charging
				pSTBatt_Rough->fSingleStartChargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuChgEnergy;
				pSTBatt_Rough->emFullyChgDischgState[n] = (soc == 0) ? FULLY_CHG_START : FULLY_TEST_IDLE;
			}
			else
			{
				// Continue charging
				pSTBatt_Rough->fSingleChargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuChgEnergy - pSTBatt_Rough->fSingleStartChargeEnergy[n];
			}
		}
		else if (lastState == STBATT_STATE_IN_CHARGING && currState == STBATT_STATE_IDLE)
		{
			// Stop charging
			pSTBatt_Rough->fSingleChargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuChgEnergy - pSTBatt_Rough->fSingleStartChargeEnergy[n];
			pSTBatt_Rough->emFullyChgDischgState[n] = (soc == 100 && pSTBatt_Rough->emFullyChgDischgState[n] == FULLY_CHG_START) ? FULLY_CHG_COMPLETE : FULLY_TEST_IDLE;
			if (pSTBatt_Rough->emFullyChgDischgState[n] == FULLY_CHG_COMPLETE)
			{	
				char szText[128];
				snprintf(szText, 128, "[Batt:%d][0->100][Energy:%.1fkWh]", n, pSTBatt_Rough->fSingleChargeEnergy[n]);
				char szTime[32];
				memset(szTime, 0, 32);
				double fLogTime = GetCurrentTime();
				LogOut_BattDegr(LOG_FULLY_CHG, AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), szText);
			}
		}
		else if (currState == STBATT_STATE_IN_DISCHARGING)
		{
			if (lastState != STBATT_STATE_IN_DISCHARGING)
			{
				// Start discharging
				pSTBatt_Rough->fSingleStartDischargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuDischgEnergy;
				pSTBatt_Rough->emFullyChgDischgState[n] = (soc == 100) ? FULLY_DISCHG_START : FULLY_TEST_IDLE;
			}
			else
			{
				// Continue discharging
				pSTBatt_Rough->fSingleDischargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuDischgEnergy - pSTBatt_Rough->fSingleStartDischargeEnergy[n];
			}
		}
		else if (lastState == STBATT_STATE_IN_DISCHARGING && currState == STBATT_STATE_IDLE)
		{
			// Stop discharging
			pSTBatt_Rough->fSingleDischargeEnergy[n] = pSTBatt_Rough->blobIfyBMS[n].fAccuDischgEnergy - pSTBatt_Rough->fSingleStartDischargeEnergy[n];
			pSTBatt_Rough->emFullyChgDischgState[n] = (soc == 0 && pSTBatt_Rough->emFullyChgDischgState[n] == FULLY_DISCHG_START) ? FULLY_DISCHG_COMPLETE : FULLY_TEST_IDLE;
			if (pSTBatt_Rough->emFullyChgDischgState[n] == FULLY_DISCHG_COMPLETE)
			{	
				char szText[128];
				snprintf(szText, 128, "[Batt:%d][100->0][Energy:%.1fkWh]", n, pSTBatt_Rough->fSingleDischargeEnergy[n]);
				char szTime[32];
				memset(szTime, 0, 32);
				double fLogTime = GetCurrentTime();
				LogOut_BattDegr(LOG_FULLY_DISCHG, AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32), "Fully Discharged!!!");
			}
		}
		
	}
			// pSTBatt_Rough->tm_fLastCalcBattEnergyCostbyBMS = tm_fNow;	 
	// }
	return;
}

static UINT Find_LastSavedBattIdx(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	HIS_BMS_RUNTIMEDATA_REC* pRec = &pSTBatt_Rough->hisBmsRunDataRec;
	
	int n = 0;
	int nCurr_Position = pRec->bmsRunData.nBmsRuntimeDataTotal - 1;
	nCurr_Position = MAX(0, nCurr_Position);

	for(n = nCurr_Position; n > 0; n--)
	{
		A_BMS_RUNTIME_DATA_ITEM* pOldItem = &pRec->bmsRunData.stBmsRuntimeDataList[n];
		if(nBattIdx == pOldItem->nBmsIdx)
		{
			break;
		}
	}

	return n;
}

static BOOL IsNeedUpdate_BmsRunDataList(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	A_BLOB_ES_RUN_DATA *pESRunData = &pSTBatt_Rough->esRunData;
	//A_BLOB_SINGLE_STBATT_RUNINFO *pSigBatt = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];

	VAR_VALUE varV;
	pSTBatt_Rough->fGridMeter_Volt[0] = pESRunData->fGrid_ACVolt_L1;
 	pSTBatt_Rough->fGridMeter_Volt[1] = pESRunData->fGrid_ACVolt_L2;
	pSTBatt_Rough->fGridMeter_Volt[2] = pESRunData->fGrid_ACVolt_L3;

	pSTBatt_Rough->fGridMeter_Curr[0] = pESRunData->fGrid_ACCurr_L1;
	pSTBatt_Rough->fGridMeter_Curr[1] = pESRunData->fGrid_ACCurr_L2;
	pSTBatt_Rough->fGridMeter_Curr[2] = pESRunData->fGrid_ACCurr_L3;

	pSTBatt_Rough->f_QgridMeter = pESRunData->f_QgridMeter;
	pSTBatt_Rough->f_QcabMeter 	= pESRunData->f_QcabMeter;   

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 411, &varV, 4);
	pSTBatt_Rough->fGridMeter_Power[0] = varV.fValue;
	
	//pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 419, &varV, 4);
	pSTBatt_Rough->fGridMeter_Power[1] = pESRunData->f_PgridMeter;

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 408, 422, &varV, 4);//Revs KWH
	pSTBatt_Rough->fGridMeter_Power[2] = varV.fValue;

	pSTBatt_Rough->fCabMeter_Volt[0] = pESRunData->fACVolt_L1; 
	pSTBatt_Rough->fCabMeter_Volt[1] = pESRunData->fACVolt_L2; 
	pSTBatt_Rough->fCabMeter_Volt[2] = pESRunData->fACVolt_L3; 

	pSTBatt_Rough->fCabMeter_Curr[0] = pESRunData->fACCurr_L1; 
	pSTBatt_Rough->fCabMeter_Curr[1] = pESRunData->fACCurr_L2; 
	pSTBatt_Rough->fCabMeter_Curr[2] = pESRunData->fACCurr_L3; 

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 411, &varV, 4);
	pSTBatt_Rough->fCabMeter_Power[0] = varV.fValue;
	
	//pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 419, &varV, 4);
	pSTBatt_Rough->fCabMeter_Power[1] = pESRunData->f_PcabMeter;

	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 407, 422, &varV, 4);
	pSTBatt_Rough->fCabMeter_Power[2] = varV.fValue;

	//----- Get Power Cube Temp -----//
	int n = 0;
	for(n = 0; n < 3; n++)
	{
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 41+n, &varV, 4);
		pSTBatt_Rough->fPowerCubeTemp[n] = varV.fValue;
	}

	//----- Get IMEU2 V2 DataInfo -----//
	varV.blobValue = (BYTE *)&pSTBatt_Rough->blobIMEU2Data[nBattIdx];
 	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 601 + nBattIdx, 450, &varV, sizeof(A_BLOB_BATTCAB_IMEU2_V20));
 
	HIS_BMS_RUNTIMEDATA_REC* pRec = &pSTBatt_Rough->hisBmsRunDataRec;
 	// if(pRec->bmsRunData.nBmsRuntimeDataTotal < 1)
 	// {
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = 0;
 	// 	return TRUE;
 	// }

	// if(pRec->bmsRunData.nBmsRuntimeDataTotal > MAX_HIS_BMS_RUNTIME_DATALIST)
	// {
	// 	TRACEE("IsNeedUpdate_BmsRunDataList nBattIdx=[%d] nBmsRuntimeDataTotal=[%d]", nBattIdx, pRec->bmsRunData.nBmsRuntimeDataTotal);
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = MAX_HIS_BMS_RUNTIME_DATALIST;
	// }

	int nLastSavePosition = Find_LastSavedBattIdx(pSTBatt_Rough, nBattIdx);
 	A_BMS_RUNTIME_DATA_ITEM* pOldItem = &pRec->bmsRunData.stBmsRuntimeDataList[nLastSavePosition];
	
	// if(ConvDateTime_2DateOnly(_NOW_) != ConvDateTime_2DateOnly(pOldItem->tmSaved))
	// {
	// 	pRec->bmsRunData.nBmsRuntimeDataTotal = 0; //If Day Changed,Need Init nBmsRuntimeDataTotal
	// 	return TRUE;
	// }

	//----- 3-pahse volt -----//
 	float fDiff_A = pSTBatt_Rough->fCabMeter_Volt[0] - pOldItem->fCabMeter_Volt[0];
 	float fDiff_B = pSTBatt_Rough->fCabMeter_Volt[1] - pOldItem->fCabMeter_Volt[1];
 	float fDiff_C = pSTBatt_Rough->fCabMeter_Volt[2] - pOldItem->fCabMeter_Volt[2];
 	if((fabs(fDiff_A) >= 50) || (fabs(fDiff_B) >= 50) || (fabs(fDiff_C) >= 50))
 	{
		TRACE("Info-VoltDiff: fDiff_A=[%.1f],fDiff_B=[%.1f],fDiff_C=[%.1f]\n", fDiff_A,fDiff_B,fDiff_C);
 		return TRUE;
 	}

 	//----- 3-pahse curr -----//
 	fDiff_A = pSTBatt_Rough->fCabMeter_Curr[0] - pOldItem->fCabMeter_Curr[0];
 	fDiff_B = pSTBatt_Rough->fCabMeter_Curr[1] - pOldItem->fCabMeter_Curr[1];
 	fDiff_C = pSTBatt_Rough->fCabMeter_Curr[2] - pOldItem->fCabMeter_Curr[2];
 	if((fabs(fDiff_A) >= 50) || (fabs(fDiff_B) >= 50) || (fabs(fDiff_C) >= 50))
 	{
		TRACE("Info-CurrentDiff: fDiff_A=[%.1f],fDiff_B=[%.1f],fDiff_C=[%.1f]\n", fDiff_A,fDiff_B,fDiff_C);
 		return TRUE;
 	}					
 	
 	//------ 4-judge if BMS data update-----//
	// UINT nSaveCycle = MAX(10, pSTBatt_Rough->nSystemCfgBattNum*10);
	int nSaveCycle;
	if(pSTBatt_Rough->nRunRecordInterval < 0)
	{
		nSaveCycle = MAX(10, pSTBatt_Rough->nSystemCfgBattNum*10);
	}
	else
	{
		nSaveCycle = pSTBatt_Rough->nRunRecordInterval;
	}

 	A_BLOB_SINGLE_STBATT_RUNINFO* pBMS = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
 	if((fabs(pOldItem->fPackTotalVolt - pBMS->fPackTotalVolt) >= 5.0) ||
 		(fabs(pOldItem->fPackTotalCurr - pBMS->fPackTotalCurr) >= 20.0) ||
 		(fabs(pOldItem->fAllow_LmtChargeVolt - pBMS->fAllow_LmtChargeVolt) > 10.0) ||
 		(fabs(pOldItem->fAllow_LmtDisChargeVolt - pBMS->fAllow_LmtDisChargeVolt) > 10.0) ||
		(fabs(pOldItem->fAllow_MaxChargeCurr - pBMS->fAllow_MaxChargeCurr) > 5.0) ||
 		(fabs(pOldItem->fAllow_MaxDisChargeCurr - pBMS->fAllow_MaxDisChargeCurr) > 5.0) ||
 		(ABS(pOldItem->nSOC - pBMS->nSOC) >= 1) ||
 		(fabs(pOldItem->fChrg_Energy - pBMS->fChrg_Energy) > 5) ||
 		(fabs(pOldItem->fDisChrging_Energy - pBMS->fDisChrging_Energy) > 5) ||
		(fabs(pOldItem->fMax_CellVolt - pBMS->fMax_CellVolt) >= 0.1) ||
 		(fabs(pOldItem->fMin_CellVolt - pBMS->fMin_CellVolt) >= 0.1) ||
		(fabs(pOldItem->fMax_CellTemp - pBMS->fMax_CellTemp) >= 2) ||
 		(fabs(pOldItem->fMin_CellTemp - pBMS->fMin_CellTemp) >= 2) || 
 		(pOldItem->emBattStatus_Run != pBMS->emBattStatus_Run) || 
		(pOldItem->emBattStatus_Alm != pBMS->emBattStatus_Alm) ||
		(pOldItem->emEMSCtrlMode_REMS != pSTBatt_Rough->esRunData.emEMSCtrlMode_REMS) || // LJH250408 CtrlByREMS
		(pOldItem->emACDC_WorkStatu_REMS != pSTBatt_Rough->esRunData.emACDC_WorkStatu_REMS) ||
		(fabs(pOldItem->fActvPwrReq_REMS - pSTBatt_Rough->esRunData.fActvPwrReq_REMS > 10)) ||
		(ABS(_NOW_ - pOldItem->tmSaved) >= nSaveCycle))  
 	{
		TRACE("emBattStatus_Run=[%d],fPackTotalVolt=[%.1f],fPackTotalCurr=[%.1f]\n", pBMS->emBattStatus_Run, pBMS->fPackTotalVolt, pBMS->fPackTotalCurr);
 		return TRUE;
 	}

 	return FALSE;
}
 
static void Update_BmsRuntimeData_2DB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	BOOL bSaveFlag = FALSE;
	static 	BOOL bFirstRun = TRUE;
	int 	n = 0;
	//John, Sometimes After Reboot,BmsRunDataRec Lost.
	if(bFirstRun == TRUE)
	{
		if(ABS(_NOW_ - pSTBatt_Rough->t_FirstBattBoot) > 30)//should wait 30S 
		{
			//---Need Wait Event_Handler Restore BmsStaticData---//
			bFirstRun = FALSE;
		}
		return;
	}

	A_BLOB_SINGLE_STBATT_RUNINFO *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[nBattIdx];
	if(pSingleRunInfo->emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

 	if(!IsNeedUpdate_BmsRunDataList(pSTBatt_Rough, nBattIdx))
 	{
		//printf("Warning: No NeedUpdate_bmsRunDataList!\n");
 		return;
 	}
	
	A_BLOB_BATTCAB_IMEU2_V20 *pblobIMEU2 = &pSTBatt_Rough->blobIMEU2Data[nBattIdx];

	IMEU3_CAN_DataInfo* pIMEU3_Bat = &pSTBatt_Rough->IMEU3CANRoughData[0].stIMEU3DataInfo;  //电池仓EU3数据
	IMEU3_CAN_DataInfo* pIMEU3_Power = &pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo; //电源仓EU3数据

 	HIS_BMS_RUNTIMEDATA_REC* pRec = &pSTBatt_Rough->hisBmsRunDataRec;
 	A_BMS_RUNTIME_DATA_ITEM* pItem = NULL;//pRec->bmsRunData.stBmsRuntimeDataList[pRec->bmsRunData.nBmsRuntimeDataTotal];
 	int nIdx = pRec->bmsRunData.nBmsRuntimeDataTotal;
	 
	TRACEI("nBattIdx=[%d] nBmsRuntimeDataTotal=[%d]\n", nBattIdx, pRec->bmsRunData.nBmsRuntimeDataTotal);	
	//要超出数组大小了，该把数据保存下来
	if(nIdx >= MAX_HIS_BMS_RUNTIME_DATALIST - 1)
 	{
 		bSaveFlag = TRUE;
 	}
 	else
 	{
 		if(nIdx <= 0)
 		{	
			pRec->tmLogTime = _NOW_; //当前运行记录保存起始时间
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
	pItem->nBmsIdx = nBattIdx;

	//---------------User Grid AC Meter RealTime DataInfo------------------//
  	pItem->fGridMeter_Volt[0] = pSTBatt_Rough->fGridMeter_Volt[0];
 	pItem->fGridMeter_Volt[1] = pSTBatt_Rough->fGridMeter_Volt[1];
 	pItem->fGridMeter_Volt[2] = pSTBatt_Rough->fGridMeter_Volt[2];
 	
   	pItem->fGridMeter_Curr[0] = pSTBatt_Rough->fGridMeter_Curr[0];
 	pItem->fGridMeter_Curr[1] = pSTBatt_Rough->fGridMeter_Curr[1];
 	pItem->fGridMeter_Curr[2] = pSTBatt_Rough->fGridMeter_Curr[2];
	
   	pItem->fGridMeter_Power[0] = pSTBatt_Rough->fGridMeter_Power[0];
 	pItem->fGridMeter_Power[1] = pSTBatt_Rough->fGridMeter_Power[1];
 	pItem->fGridMeter_Power[2] = pSTBatt_Rough->fGridMeter_Power[2];
	
 	//------------------Cabinet AC Meter RealTime DataInfo-----------------//
  	pItem->fCabMeter_Volt[0] = pSTBatt_Rough->fCabMeter_Volt[0];
 	pItem->fCabMeter_Volt[1] = pSTBatt_Rough->fCabMeter_Volt[1];
 	pItem->fCabMeter_Volt[2] = pSTBatt_Rough->fCabMeter_Volt[2];
 	
  	pItem->fCabMeter_Curr[0] = pSTBatt_Rough->fCabMeter_Curr[0];
 	pItem->fCabMeter_Curr[1] = pSTBatt_Rough->fCabMeter_Curr[1];
 	pItem->fCabMeter_Curr[2] = pSTBatt_Rough->fCabMeter_Curr[2];

	pItem->fCabMeter_Power[0] = pSTBatt_Rough->fCabMeter_Power[0];
 	pItem->fCabMeter_Power[1] = pSTBatt_Rough->fCabMeter_Power[1];
 	pItem->fCabMeter_Power[2] = pSTBatt_Rough->fCabMeter_Power[2];

	for(n = 0; n < 3; n++)
	{
 		pItem->fPowerCubeTemp[n] = pSTBatt_Rough->fPowerCubeTemp[n];
	}
 
	//-----------------------Battery RealTime Data-------------------------//
	pItem->emBattStatus_Run 		= pSingleRunInfo->emBattStatus_Run;
	pItem->emBattStatus_Alm 		= pSingleRunInfo->emBattStatus_Alm;

	pItem->nSOC 					= pSingleRunInfo->nSOC;
	pItem->nSOH 					= pSingleRunInfo->nSOH;
	pItem->nLifeCycleTimes 			= pSingleRunInfo->nLifeCycleTimes;

	pItem->fInsulation_P_R			= pSingleRunInfo->fInsulation_P_R;
	pItem->fInsulation_N_R			= pSingleRunInfo->fInsulation_N_R;
	pItem->fOutsideVolt				= pSingleRunInfo->fOutsideVolt;
	pItem->fNegativeBusVolt			= pSingleRunInfo->fNegativeBusVolt;
	pItem->usBattWorkState			= pSingleRunInfo->usBattWorkState;

	memcpy(pItem->szFault, pSingleRunInfo->szFault, CAN_DATA_AREA_LEN);
	memcpy(pItem->szAlarm, pSingleRunInfo->szAlarm, CAN_DATA_AREA_LEN);
	memcpy(pItem->szDIDOStatu, pSingleRunInfo->szDIDOStatu, CAN_DATA_AREA_LEN);

	pItem->szLONGALARM[0] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo1 >> 8 & 0xFF;
	pItem->szLONGALARM[1] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo1 & 0xFF;
	pItem->szLONGALARM[2] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo2 >> 8 & 0xFF;
	pItem->szLONGALARM[3] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo2 & 0xFF;
	pItem->szLONGALARM[4] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo3 >> 8 & 0xFF;
	pItem->szLONGALARM[5] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo3 & 0xFF;
	pItem->szLONGALARM[6] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo4 >> 8 & 0xFF;
	pItem->szLONGALARM[7] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo4 & 0xFF;
	pItem->szLONGALARM[8] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo5 >> 8 & 0xFF;
	pItem->szLONGALARM[9] =   pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo5 & 0xFF;
	pItem->szLONGALARM[10] =  pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo6 >> 8 & 0xFF;
	pItem->szLONGALARM[11] =  pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo6 & 0xFF;
	pItem->szLONGALARM[12] =  pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo7 >> 8 & 0xFF;
	pItem->szLONGALARM[13] =  pSTBatt_Rough->blobIfyBMS[nBattIdx].nAlarmInfo7 & 0xFF;

	pItem->fPackTotalVolt 			= pSingleRunInfo->fPackTotalVolt;
    pItem->fPackTotalCurr 			= pSingleRunInfo->fPackTotalCurr;

    pItem->fAllow_LmtChargeVolt 	= pSingleRunInfo->fAllow_LmtChargeVolt ;
    pItem->fAllow_LmtDisChargeVolt 	= pSingleRunInfo->fAllow_LmtDisChargeVolt;
    pItem->fAllow_MaxChargeCurr 	= pSingleRunInfo->fAllow_MaxChargeCurr ;
    pItem->fAllow_MaxDisChargeCurr 	= pSingleRunInfo->fAllow_MaxDisChargeCurr;

 	pItem->uiBAT_Ucell_Max_CSC 		= pSingleRunInfo->uiBAT_Ucell_Max_CSC ;
	pItem->uiBAT_Ucell_Max_CELL 	= pSingleRunInfo->uiBAT_Ucell_Max_CELL;
	pItem->uiBAT_Ucell_Min_CSC 		= pSingleRunInfo->uiBAT_Ucell_Min_CSC ;
	pItem->uiBAT_Ucell_Min_CELL 	= pSingleRunInfo->uiBAT_Ucell_Min_CELL;

	pItem->fMax_CellVolt 			= pSingleRunInfo->fMax_CellVolt;
	pItem->fMin_CellVolt 			= pSingleRunInfo->fMin_CellVolt;
 	pItem->fAvg_CellVolt 			= pSingleRunInfo->fAvg_CellVolt;

	pItem->fMax_CellTemp 			= pSingleRunInfo->fMax_CellTemp;
	pItem->fMin_CellTemp 			= pSingleRunInfo->fMin_CellTemp;
	pItem->fAvg_CellTemp 			= pSingleRunInfo->fAvg_CellTemp;
	
	pItem->nBAT_T_Max_CSC 			= pSingleRunInfo->nBAT_T_Max_CSC;
	pItem->nBAT_T_Min_CSC 			= pSingleRunInfo->nBAT_T_Min_CSC;
	
	pItem->fChrg_Energy				= pSingleRunInfo->fChrg_Energy;
	pItem->fDisChrging_Energy		= pSingleRunInfo->fDisChrging_Energy;
	pItem->fSingleChrgCapacity 	    = pSingleRunInfo->fSingleChrgCapacity;
	pItem->fSingleDischrgCapacity   = pSingleRunInfo->fSingleDischrgCapacity;
	pItem->AccuChrgCapacity			= pSingleRunInfo->AccuChrgCapacity;
	pItem->fAccuDischrgCapacity		= pSingleRunInfo->fAccuDischrgCapacity;
	pItem->fAccuChrgEnergy		    = pSingleRunInfo->fAccuChrgEnergy;	
	pItem->fAccuDischrgEnergy		= pSingleRunInfo->fAccuDischrgEnergy;

	//20220510 Added John.
	pItem->fACDCOutRelayVolt		= pSTBatt_Rough->branchGrpData[nBattIdx].fACDCOutRelayVolt;
 
	//pItem->fAdd_ChgCapacity			= pSingleRunInfo->fAdd_ChgCapacity; 
	//pItem->fAdd_DischgCapacity		= pSingleRunInfo->fAdd_DischgCapacity;
	pItem->fQ_PCSGrp = pSTBatt_Rough->branchGrpData[nBattIdx].fQ_PCSGrp;
	pItem->f_QgridMeter = pSTBatt_Rough->f_QgridMeter;
	pItem->f_QcabMeter  = pSTBatt_Rough->f_QcabMeter; 

 	pItem->fQ_PCSGrpCtrl		= pSTBatt_Rough->branchGrpData[nBattIdx].fCtrl_RealTime_Q;

	//---------- PM Relevant DataInfo ----------//
	pItem->fPMCtrl_Volt 			= pSTBatt_Rough->fLastSendReqVolt_2CCU[nBattIdx];
    pItem->fPMCtrl_Curr 			= pSTBatt_Rough->fLastSendReqCurr_2CCU[nBattIdx];
	pItem->emCtrlPcsCmd 			= (BYTE)pSTBatt_Rough->emLastCtrlPcsCmd[nBattIdx];
 
	pItem->fActvPwr_PCSGrp			= pSTBatt_Rough->branchGrpData[nBattIdx].fActvPwr_PCSGrp;
	pItem->fPMOutput_Volt 			= pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Volt;
    pItem->fPMOutput_Curr 			= pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_RealTime_Curr;

	pItem->fMPPTOutPut_Volt			= pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Volt;
	pItem->fMPPTOutPut_Curr			= pSTBatt_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Curr;	

	pItem->byPCSStatu = (BYTE)pSTBatt_Rough->branchGrpData[nBattIdx].emACDC_WorkStatu;
	//fAvg_ChrgLmtPower

	pItem->fAvg_Calc_Chg_Power = pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_ChrgLmtPower;
	pItem->fCalc_Chg_Power = pSTBatt_Rough->calcNodeData[nBattIdx].fACDC_Distri_4_BattChrg_Pwr;

	pItem->fAvg_Calc_DisChg_Power = pSTBatt_Rough->calcNodeData[nBattIdx].fAvg_DisChrgingLmtPower;
	pItem->fCalc_DisChg_Power = pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Distri_4_ACDC_Pwr;
	

		//Cabinet DataInfo
	pItem->fAir_Temp1 = pblobIMEU2->fRealTimeCabTemp;
	pItem->fAir_Temp2 = pblobIMEU2->fCoolSetPoint;
	pItem->fAir_Humidity = pblobIMEU2->fRealTimeCabHumi;
	pItem->byOnOrOffd	= (BYTE)pblobIMEU2->emOnOrOffd;

	//for(n = 0; n < MIN(2, MAX_HUMITEMP_NUM); n++)
	//{
	//	pItem->fSensor_Temp[n] = pblobIMEU2->fTemperature[n];
	//	pItem->fSensor_Humi[n] = pblobIMEU2->fHumidity[n];
	//}

	pItem->f_PauxMeter = pSTBatt_Rough->esRunData.f_PauxMeter;
	pItem->fPacload4Chg = pSTBatt_Rough->esRunData.fPacload4Chg;  
	pItem->fPacload4Dischg = pSTBatt_Rough->esRunData.fPacload4Dischg; 
	pItem->fTotal_PessDischgLmt = pSTBatt_Rough->esRunData.fTotal_PessDischgLmt;
	pItem->fTotal_PessChgAvaliable = pSTBatt_Rough->esRunData.fTotal_PessChgAvaliable;
	
	pItem->f_PmaxGridDemand = pSTBatt_Rough->esRunData.f_PmaxGridDemand;

	pItem->f_PauxMeter_3 = pSTBatt_Rough->esRunData.f_PauxMeter_3;
	pItem->bySTSSysStatu = pSTBatt_Rough->esRunData.mapSTSCode.byRunStatus;
	//for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pItem->byPCSSysStatu[0] = pSTBatt_Rough->esRunData.mapPCSSysStatu[nBattIdx].byPCSSysStatu;
		pItem->byPCSSysStatu[1] = pSTBatt_Rough->esRunData.mapPCSSysStatuBelong2[nBattIdx].byPCSSysStatu;
	}
	/*for(n = 0; n < MIN(5, MAX_PT1000_NUM); n++)
	{
		pItem->fPT1000_Value[n] = pblobIMEU2->fPT1000Temperature[n];
	}*/

	//Air Conditioner DataInfo
	pItem->fBattInWaterTemp = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fBatteryInletTemp;
	pItem->fBattOutWaterTemp = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fBatteryOutletTemp;
	pItem->fSysAmbientTemp = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fSys1AmbientTemp;

	pItem->fSingleChargeEnergy = pSTBatt_Rough->fSingleChargeEnergy[nBattIdx];
	pItem->fSingleDischargeEnergy = pSTBatt_Rough->fSingleDischargeEnergy[nBattIdx];

	// LJH240925 RunData LJH250610 fPwrAbility0
	pItem->fMax_Chrg_PwrAbility0 =        pSTBatt_Rough->calcNodeData[nBattIdx].fMax_Chrg_PwrAbility0;
	pItem->fMax_DisChrg_PwrAbility0 =     pSTBatt_Rough->calcNodeData[nBattIdx].fMax_DisChrg_PwrAbility0;
	pItem->fMax_Chrg_PwrAbility =         pSTBatt_Rough->calcNodeData[nBattIdx].fMax_Chrg_PwrAbility;
	pItem->fMax_DisChrg_PwrAbility =      pSTBatt_Rough->calcNodeData[nBattIdx].fMax_DisChrg_PwrAbility;

	pItem->fBatt_Max_Chrg_Pwr_Req =       pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Max_Chrg_Pwr_Req;
	pItem->fBatt_Max_DisChrg_Pwr =        pSTBatt_Rough->calcNodeData[nBattIdx].fBatt_Max_DisChrg_Pwr;
	pItem->fACDC_Max_Output_Pwr_Ability = pSTBatt_Rough->branchGrpData[nBattIdx].fACDC_Max_Output_Pwr_Ability;
	pItem->antiMode[0] = pSTBatt_Rough->esRunData.bEnterAnti_Reflux;
	pItem->antiMode[1] = pSTBatt_Rough->esRunData.bEnter_OverLoad;
	pItem->antiMode[2] = pSTBatt_Rough->esRunData.bEnter_OverLoadHV;
	// LJH241007 AntiPhase RunData
	pItem->fGridRemainACPwr2Chrg = pSTBatt_Rough->esRunData.fGridRemainACPwr2Chrg;
	pItem->fRemainPwr4Chrg =       pSTBatt_Rough->esRunData.fRemainPwr4Chrg;
	pItem->fRemainDemand4Chrg =    pSTBatt_Rough->esRunData.fRemainDemand4Chrg;
	for(n = 0; n < MAX_PHASE_NUM; n++)
	{
		pItem->fAtRflxMtr_kWPhs[n] = pSTBatt_Rough->esRunData.fAtRflxMtr_kWPhs[n];
		pItem->fAtOvlMtr_APhs[n] =   pSTBatt_Rough->esRunData.fAtOvlMtr_APhs[n];
	}
	pItem->fAtRflxMtr_Factor =       pSTBatt_Rough->esRunData.fAtRflxMtr_Factor;
	pItem->fDiffPwr4Dischrg_SumPhs = pSTBatt_Rough->esRunData.fDiffPwr4Dischrg_SumPhs;
	pItem->fAtOvlMtr_Factor =        pSTBatt_Rough->esRunData.fAtOvlMtr_Factor;
	pItem->fMaxPwr4Chrg_SumPhs =     pSTBatt_Rough->esRunData.fMaxPwr4Chrg_SumPhs;
	pItem->antiMode_Phase[0] =       pSTBatt_Rough->esRunData.bEnterAnti_Reflux_Phase;
	pItem->antiMode_Phase[1] =       pSTBatt_Rough->esRunData.bEnter_OverLoad_Phase;
	pItem->fAtRflx_kW =              pSTBatt_Rough->esRunData.fAtRflx_kW;
	pItem->fAtOvl_kW =               pSTBatt_Rough->esRunData.fAtOvl_kW;
	// LJH250408 CtrlByREMS
	pItem->emEMSCtrlMode_REMS =    (BYTE)pSTBatt_Rough->esRunData.emEMSCtrlMode_REMS;
	pItem->emACDC_WorkStatu_REMS = (BYTE)pSTBatt_Rough->esRunData.emACDC_WorkStatu_REMS;
	pItem->fActvPwrReq_REMS =      pSTBatt_Rough->esRunData.fActvPwrReq_REMS;
	// LJH250726 东博上报MaxChg|DisChg都为0
	pItem->fRatedPower =    pSTBatt_Rough->esRunData.fRatedPower; // 额定功率
	pItem->nBattGrpPwrLmt = pSTBatt_Rough->esRunData.nBattGrpPwrLmt; // 单组电池组功率限制
	TRACED("Update_BmsRuntimeData_2DB emEMSCtrlMode_REMS:%d emACDC_WorkStatu_REMS:%d %.1f fRatedPower:%.1f nBattGrpPwrLmt:%u",
		pItem->emEMSCtrlMode_REMS, pItem->emACDC_WorkStatu_REMS, pItem->fActvPwrReq_REMS, pItem->fRatedPower, pItem->nBattGrpPwrLmt);	
	
	// XSY250807 工厂上报液冷空调信息
	memset(pItem->usLCAC_FaultCode, 0, sizeof(pItem->usLCAC_FaultCode));
	pItem->byLCAC_Type = pSTBatt_Rough->emAirType[nBattIdx];
	switch(pItem->byLCAC_Type)
	{
		case INFYPOWER_AIR: 
			// pItem->byLCAC_WorkMode = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].nWorkMode;
			// pItem->byLCAC_BmsSetWM = 0;//---
			// pItem->byLCAC_CompState
			// pItem->fLCAC_CompSpeed = 0; // 未上传
			// pItem->fLCAC_ColTemp = STBatt_Rough->stAirStatusAlarm[nBattIdx].nColdOpenDiff;
			// pItem->fLCAC_ColDiff = STBatt_Rough->stAirStatusAlarm[nBattIdx].
			// pItem->fLCAC_HeatTemp = STBatt_Rough->stAirStatusAlarm[nBattIdx].
			// pItem->fLCAC_HeatDiff = STBatt_Rough->stAirStatusAlarm[nBattIdx].
			// pItem->fLCAC_XmSetTemp = 0;
			// pItem->fLCAC_AmbiT = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fSys1AmbientTemp;
			// pItem->fLCAC_FeedT = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fModuleInletTemp;
			// pItem->fLCAC_ReturnT = pSTBatt_Rough->stAirStatusAlarm[nBattIdx].fModuleOutletTemp;
			// pItem->fLCAC_ModFeedT
			// pItem->fLCAC_ModReturn
			// pItem->fLCAC_ExPre = 0; // 未上传
			// pItem->fLCAC_FeedPre
			// pItem->fLCAC_ReturnPre = 0;
			// pItem->usLCAC_FaultCode[0] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault1;
			// pItem->usLCAC_FaultCode[1] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault2;
			// pItem->usLCAC_FaultCode[2] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault3;
			// pItem->usLCAC_FaultCode[3] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault4;
			// pItem->usLCAC_FaultCode[4] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault5;
			// pItem->usLCAC_FaultCode[5] =  pSTBatt_Rough->stAirStatusAlarm[nBattIdx].AirSysFault6;
			break;
		case TONGFEI_AIR: 
			pItem->byLCAC_WorkMode = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nWorkingMode;
			pItem->byLCAC_BmsSetWM = ((pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nUnitOperatingStatus & 0x00FF) << 8 
									| (pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nUnitOperatingMode & 0x00FF));
			pItem->byLCAC_CompState = (pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nUnitOperatingMode == 1) ? 1 : 0;
			pItem->fLCAC_CompSpeed = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nCurrentSpeedCompressor;
			pItem->fLCAC_ColTemp = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fRefrigerationPresetTemperature;
			pItem->fLCAC_ColOpenDiff = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fCoolingTemperatureTifference;
			pItem->fLCAC_ColStopDiff = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fCoolingTemperatureTifference;
			pItem->fLCAC_HeatTemp = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fHeatingTresetTemperature;
			pItem->fLCAC_HeatOpenDiff = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fHeatingTemperatureTifference;
			pItem->fLCAC_HeatStopDiff = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fHeatingTemperatureTifference;
			pItem->fLCAC_XmSetTemp = 0;
			pItem->fLCAC_AmbiT = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fAmbientTemperature;
			pItem->fLCAC_FeedT = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fLiquidSupplyTemperature;
			pItem->fLCAC_ReturnT = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].fReturnLiquidTemperature;
			pItem->fLCAC_ModFeedT = 0;
			pItem->fLCAC_ModReturnT = 0;
			pItem->fLCAC_ExPre = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nExhaustPressure;
			pItem->fLCAC_FeedPre = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nWaterSupplyPressure;
			pItem->fLCAC_ReturnPre = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nReturnWaterPressure;
			
			pItem->usLCAC_FaultCode[0] = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nFaultStatus1;
			pItem->usLCAC_FaultCode[1] = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nFaultStatus2;
			pItem->usLCAC_FaultCode[2] = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nFaultStatus3;
			pItem->usLCAC_FaultCode[3] = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nFaultStatus4;
			pItem->usLCAC_FaultCode[4] = pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx].nFaultStatus5;
			break;
		case IMEU3_AIR: 
			pItem->byLCAC_WorkMode = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].uOpMode;
			pItem->byLCAC_BmsSetWM = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].usBatteryMode;
			pItem->byLCAC_CompState = (pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].uCompOpHz > 0) ? 1 : 0;
			pItem->fLCAC_CompSpeed = 0; // 未上传
			pItem->fLCAC_ColTemp = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fCoolingTemp;
			pItem->fLCAC_ColOpenDiff = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fCoolingTempDiff;
			pItem->fLCAC_ColStopDiff  = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fCoolingStopTempDiff;
			pItem->fLCAC_HeatTemp = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fHeatingTemp;
			pItem->fLCAC_HeatOpenDiff = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fHeatingTempDiff;
			pItem->fLCAC_HeatStopDiff = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].fHeatingStopTempDiff;
			pItem->byLCAC_NoiseReductionMode = pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx].byNoiseReductionMode;
			pItem->fLCAC_XmSetTemp = 0;
			pItem->fLCAC_AmbiT = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fAmbTemp;
			pItem->fLCAC_FeedT = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fCoolInTemp;
			pItem->fLCAC_ReturnT = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fCoolBattOutTemp;
			pItem->fLCAC_ModFeedT = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fModuleInTemp;
			pItem->fLCAC_ModReturnT = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fModuleOutTemp;
			pItem->fLCAC_ExPre = 0; // 未上传
			pItem->fLCAC_FeedPre = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fCoolInPress;
			pItem->fLCAC_ReturnPre = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].fCoolOutPress;
			
			pItem->usLCAC_FaultCode[0] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nFaultLvl;
			pItem->usLCAC_FaultCode[1] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nVfFault2;
			pItem->usLCAC_FaultCode[2] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nPfcFault;
			pItem->usLCAC_FaultCode[3] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nCompFault1;
			pItem->usLCAC_FaultCode[4] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nCompFault2;
			pItem->usLCAC_FaultCode[5] = pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx].nSysStatFault;
			break;
		case XINMIAO_AIR: 
			pItem->byLCAC_WorkMode = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].nPowerStat;
			pItem->byLCAC_BmsSetWM = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].nWorkMode;
			pItem->byLCAC_CompState = (pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].nComp1Stat 
									|| pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].nComp2Stat);
			pItem->fLCAC_CompSpeed = 0; // 未上传
			pItem->fLCAC_ColTemp = 0; // 未上传
			pItem->fLCAC_ColOpenDiff = 0; // 未上传
			pItem->fLCAC_ColStopDiff  = 0; // 未上传
			pItem->fLCAC_HeatTemp = 0; // 未上传
			pItem->fLCAC_HeatOpenDiff = 0; // 未上传
			pItem->fLCAC_HeatStopDiff  = 0; // 未上传
			pItem->fLCAC_XmSetTemp = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fTgtTemp;
			pItem->fLCAC_AmbiT = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fenvTempFault;
			pItem->fLCAC_FeedT = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fOutTemp;
			pItem->fLCAC_ReturnT = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fRetTemp;
			pItem->fLCAC_ModFeedT = 0; // 未上传
			pItem->fLCAC_ModReturnT = 0; // 未上传
			pItem->fLCAC_ExPre = 0; // 未上传
			pItem->fLCAC_FeedPre = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fOutPress;
			pItem->fLCAC_ReturnPre = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].fRetPress;
			
			pItem->usLCAC_FaultCode[0] = pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx].nFaultCode;
			break;
		case AIR_NOT_INSTALLED:
		default: break;
	}
	
	int i = 0;
	for(i = 0; i < 3; ++i)
	{
		pItem->by_BatEu3_DIStatus[i] = pIMEU3_Bat->uDIStatus >> (i*8) & 0xFF;
		pItem->by_BatEu3_DOStatus[i] = pIMEU3_Bat->uDOStatus >> (i*8) & 0xFF;
	
	}
	pItem->fNTC_BatEu3_Temp[0] = (pIMEU3_Bat->fNTC[0] == -50)? 0 : pIMEU3_Bat->fNTC[0];
	pItem->fNTC_BatEu3_Temp[1] = (pIMEU3_Bat->fNTC[1] == -50)? 0 : pIMEU3_Bat->fNTC[1]; 
	pItem->fNTC_BatEu3_Temp[2] = (pIMEU3_Bat->fNTC[2] == -50)? 0 : pIMEU3_Bat->fNTC[2]; 
	pItem->fNTC_BatEu3_Temp[3] = (pIMEU3_Bat->fNTC[3] == -50)? 0 : pIMEU3_Bat->fNTC[3];
	pItem->fTemperatureSen = pIMEU3_Bat->fTemperatureValue[0];
	pItem->fHumiditySen = pIMEU3_Bat->fhumidityValue[0];
	pItem->fGasConcent1H2 = pIMEU3_Bat->fGasConcent1H2;
	pItem->fGasConcent2C0 = pIMEU3_Bat->fGasConcent2C0;
	pItem->fHumidity = pIMEU3_Bat->fHumidity;

	for(i = 0; i < 3; ++i)
	{
		pItem->by_PowerEu3_DIStatus[i] = pIMEU3_Power->uDIStatus >> (i*8) & 0xFF;
		pItem->by_PowerEu3_DOStatus[i] = pIMEU3_Power->uDOStatus >> (i*8) & 0xFF;
	}
	pItem->fNTC_PowerEu3_Temp[0] = (pIMEU3_Power->fNTC[0] == -50)? 0 : pIMEU3_Power->fNTC[0];
	pItem->fNTC_PowerEu3_Temp[1] = (pIMEU3_Power->fNTC[1] == -50)? 0 : pIMEU3_Power->fNTC[1];
	pItem->fNTC_PowerEu3_Temp[2] = (pIMEU3_Power->fNTC[2] == -50)? 0 : pIMEU3_Power->fNTC[2];
	pItem->fNTC_PowerEu3_Temp[3] = (pIMEU3_Power->fNTC[3] == -50)? 0 : pIMEU3_Power->fNTC[3];

		TRACED(" %d, %d, %d, %d, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f\n", 
			pItem->byLCAC_Type,
			pItem->byLCAC_WorkMode,
			pItem->byLCAC_BmsSetWM,
			pItem->byLCAC_CompState,
			pItem->fLCAC_CompSpeed,
			pItem->fLCAC_ColTemp,
			pItem->fLCAC_ColOpenDiff,
			pItem->fLCAC_ColStopDiff,
			pItem->fLCAC_HeatTemp,
			pItem->fLCAC_HeatOpenDiff,
			pItem->fLCAC_HeatStopDiff,
			pItem->fLCAC_XmSetTemp,
			pItem->fLCAC_AmbiT,
			pItem->fLCAC_FeedT,
			pItem->fLCAC_ReturnT,
			pItem->fLCAC_ModFeedT,
			pItem->fLCAC_ModReturnT,
			pItem->fLCAC_ExPre,
			pItem->fLCAC_FeedPre,
			pItem->fLCAC_ReturnPre);
 	//now calculate the tmSampled
  	pItem->tmSaved = _NOW_; 
	
 	//now sendmsg => db

	BOOL  bisCrossDay = FALSE;
	if(ConvDateTime_2DateOnly(_NOW_) != ConvDateTime_2DateOnly(pRec->tmLogTime) && (pRec->bmsRunData.nBmsRuntimeDataTotal > 1))
	{
		bisCrossDay = TRUE;
	}

	//换天 或者 当前保存到达了限制总数  保存一次
	if(bisCrossDay || bSaveFlag)
	{
		if(bisCrossDay && (pRec->bmsRunData.nBmsRuntimeDataTotal > 1))
		{
			pRec->bmsRunData.nBmsRuntimeDataTotal -= 1; //换天最后会残留第二天的一条数据，去除掉
		}

 		EVENT_BMSRUNDATA_UPDATE bmsRunDataUpdt;
		//使用深拷贝，防止发送给事件处理后，没有及时处理导致数据被篡改
		//!!!!!!! 在事件处理后需要释放内存 !!!!!!!  
		bmsRunDataUpdt.pBmsRunDataRec = (HIS_BMS_RUNTIMEDATA_REC*) malloc(sizeof(HIS_BMS_RUNTIMEDATA_REC));
		if(bmsRunDataUpdt.pBmsRunDataRec == NULL)
		{
			TRACEI("Failed to allocate memory for BMS runtime data record\n");
			return;
		}
		memcpy(bmsRunDataUpdt.pBmsRunDataRec, &pSTBatt_Rough->hisBmsRunDataRec, sizeof(HIS_BMS_RUNTIMEDATA_REC));
		// EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_STATUS_CHANGED
 		SendMsg(pSTBatt_Rough->hSampler, EVENT_TYPE_BMSRUNDATA_UPDATE, EVENT_MSG_STATUS_CHANGED,
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

/*********************************************************************************
*  
*  FUNCTION NAME :  Rec_CANMsg_StateMachine
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 11:13:53
*  DESCRIPTION   :  
*				    
*	Start_WriteMsg2USB(pSTBatt_Rough, nBMSIdx, TP_TRIGGER_MSG emMsgType);
*	Pending_WriteMsg2USB(pSTBatt_Rough, nBMSIdx, BYTE* pMsgData, UINT nMsgNum);
*	End_WriteMsg2USB(pSTBatt_Rough, nBMSIdx);
*
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Rec_CANMsg_StateMachine(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx)
{
	STU_BMS_CANMSG_STATMACH *pStatMach = &pSTBatt_Rough->stuCanSave.stuStatMach[nBMSIdx];
	
	switch (pStatMach->emSaveState)
	{
		case STAT_ON_START:
		{
			//Judge If Need Start Write CAN Msg
			if(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] != TP_TRIG_SUSPEND)
			{
				LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,
					"BMS[%d] Start Write Msg State[%d] TrigMsgTp[%d].\n", nBMSIdx, pStatMach->emSaveState,pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx]);
	
				//pSTBatt_Rough->stuCanSave.bTriggerMsgRec[nBMSIdx] = FALSE;
				pStatMach->tm_LastStatEnter = _NOW_;
				pStatMach->emSaveState = STAT_ON_PENDING;
				Start_WriteMsg2USB(pSTBatt_Rough, nBMSIdx, pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx]);

				TRACEX("ON_START: nBMSIdx=[%d] ->tm_LastStatEnter=[%ld]->emSaveState=[%d]\n", \
					nBMSIdx ,pStatMach->tm_LastStatEnter, pStatMach->emSaveState);
				break;
			}
			break;
		}
		case STAT_ON_PENDING:
		{
			//Note: John Over 300S ,Need End Write,FileSize About 100KB
			if((ABS(_NOW_ - pStatMach->tm_LastStatEnter) > 300) ||
				(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] == TP_TRIG_SUSPEND))
			{
				pStatMach->tm_LastStatEnter = _NOW_;
				pStatMach->emSaveState = STAT_ON_END;
				TRACEX("ON_PENDING: nBMSIdx=[%d] ->tm_LastStatEnter=[%ld]->emSaveState=[%d]\n", \
					nBMSIdx ,pStatMach->tm_LastStatEnter, pStatMach->emSaveState);
				break;
			}
			//if Have ErrMsg Over 5Min,Maybe CommFail, GoTo End;
			break;
		}
		case STAT_ON_END:
		{
			LogOut4c(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_INFO,
				"BMS[%d] End Write Msg State[%d] TrigMsgTp[%d].\n", nBMSIdx, pStatMach->emSaveState,pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx]);
	
			End_WriteMsg2USB(pSTBatt_Rough, nBMSIdx);
			pStatMach->emSaveState = STAT_ON_START;
			pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND;

			TRACEX("ON_END: nBMSIdx=[%d] ->tm_LastStatEnter=[%ld]->emSaveState=[%d]\n", \
					nBMSIdx ,pStatMach->tm_LastStatEnter, pStatMach->emSaveState);

			break;
		}
		
		default:
		{
			pStatMach->emSaveState = STAT_ON_START;
			pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND;
			break;
		}
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Save_BMSCanMsg2Log
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Save_BMSCanMsg2Log(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	UINT n = 0;

 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		if((pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run == STBATT_RUNSTAT_NOT_INSTALLED)
			&& (pSTBatt_Rough->blobSingleBattInfo[n].emBattStatus_Run == STBATT_RUNSTAT_COMM_FAIL))
		{
			continue;
		}

		Rec_CANMsg_StateMachine(pSTBatt_Rough, n);

	}
	return ;
}


static void OnHandle_STS_Cabinet(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	static time_t lastSend = -4;
	time_t nowTime = _NOW_;

	if (ABS(nowTime - lastSend) >= 2)
	{
		VAR_VALUE varV;
		VAR_VALUE varVCtrl;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 160, &varV, 4);
		//DO2 指示灯(绿灯）/运行
		varVCtrl.emValue =  (varV.emValue == 0 ? 0 : 1);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 510, varVCtrl);

		//DO3 指示灯(红灯）//故障
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 163, &varV, 4);
		varVCtrl.emValue =  (varV.emValue == 1 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 511, varVCtrl);


		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 161, &varV, 4); //0:并网 1：离网

		//DO1 指示灯(黄灯）/并网
		varVCtrl.emValue =  (varV.emValue == 0 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 509, varVCtrl);

		//DO4 指示灯(黄灯）/离网
		varVCtrl.emValue =  (varV.emValue == 0 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 512, varVCtrl);

		//DO5 N线接触器KM控制 （离网控制断开）	
		varVCtrl.emValue =  (varV.emValue == 0 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 513, varVCtrl);

		//DO6 N接地接触器KM控制（离网控制闭合）
		varVCtrl.emValue = (varV.emValue == 0 ? 0 : 1);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 514, varVCtrl);

		//DO7 主回路接触器2#KM控制(离网控制断开）	
		varVCtrl.emValue = (varV.emValue == 0 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 515, varVCtrl);

		//电源柜IMEU3 DO3 N线接触器控制， 一直给高电平（闭合）
		varVCtrl.emValue =  1;
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 611, 40, varVCtrl);

		//电源柜IMEU3 DO4 N线接地触器控制， 一直给给低电平（断开）
		varVCtrl.emValue =  0;
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 611, 41, varVCtrl);

		lastSend = nowTime;
	}
}

static void OnHandleSTSCabinetFan(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	static time_t lastSend = -4;
	time_t nowTime = _NOW_;
	
	if (ABS(nowTime - lastSend) >= 2)
	{
		
		VAR_VALUE varV;
		
		//STS柜内温度
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 612, 1983, &varV, 4);
		float fCabinetInternalTemp = varV.fValue;

		int iFanSpeed = 0;
		if (fCabinetInternalTemp < 40)
		{
			iFanSpeed = 0;
		}
		else if (fCabinetInternalTemp >= 50)
		{
			iFanSpeed = 100;
		}
		else
		{
			iFanSpeed = ((int)(fCabinetInternalTemp - 39)) * 10;
		}

		VAR_VALUE varVCtrl;
		varVCtrl.lValue = iFanSpeed;
		
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 612, 525, varVCtrl);

		lastSend = nowTime;
	}


	return;
}

static void OnHandleLiquidBackupAutoOnOffGrid(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	static time_t lastSend = -4;
	time_t nowTime = _NOW_;

	if (ABS(nowTime - lastSend) >= 2)
	{
		VAR_VALUE varV;
		VAR_VALUE varVCtrl;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 161, &varV, 4); //0:离网 1：并网
		//DO11 储能主交流接触器KM1控制(离网控制断开）	
		varVCtrl.emValue = (varV.emValue == 0 ? 1 : 0);
		Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 611, 48, varVCtrl);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 300, &varV, 4); //IMIU检测N线状态
		//DO4 N线接地接触器KM4控制（IMIU检测N线状态，异常就闭合N线）
		if(varV.emValue == 1)
		{
			varVCtrl.emValue = 1;
			Push_VarValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 611, 41, varVCtrl);
		}

		lastSend = nowTime;
	}
}

// 认证功能
static void Authentication(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	IMEU3_CAN_DataInfo* pIMEU3_Bat = &pSTBatt_Rough->IMEU3CANRoughData[0].stIMEU3DataInfo;  //电池仓EU3数据
	VAR_VALUE varV;
	VAR_VALUE varV2;

	ERRCDDE_DPR err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ST_VAR, 1, 102, &varV, 4);
	float fTotalPowerLimit = varV.fValue;
	// printf("fTotalPowerLimit == %.1f\n", fTotalPowerLimit);
	int nFlag = 0;
	int needCtl = 0;
	
	A_BLOB_MANUALCTRL_RECT_CMD blobManualCtrlRect;
	ZERO_POBJS(&blobManualCtrlRect, 1);
	varV.blobValue = (BYTE *)&blobManualCtrlRect;
	 err = pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_CL_VAR, 
		200, 214, &varV, sizeof(A_BLOB_MANUALCTRL_RECT_CMD));
	
	static A_BLOB_MANUALCTRL_RECT_CMD blobRawManuCtl;
	static int nDelayButton = 0;

	int ButtonDown = (!GET_BIT(pIMEU3_Bat->uDIStatus, 6) 
					|| !GET_BIT(pIMEU3_Bat->uDIStatus, 5) 
					|| !GET_BIT(pIMEU3_Bat->uDIStatus, 4) 
					|| !GET_BIT(pIMEU3_Bat->uDIStatus, 3));// 有按钮按下为1,否则为0

	// printf("ButtonDown=%d | 7=%d | 6=%d | 5=%d | 4=%d\n", 
	// 	ButtonDown,
	// 	GET_BIT(pIMEU3_Bat->uDIStatus, 6),
	// 	GET_BIT(pIMEU3_Bat->uDIStatus, 5),
	// 	GET_BIT(pIMEU3_Bat->uDIStatus, 4),
	// 	GET_BIT(pIMEU3_Bat->uDIStatus, 3));


	if(ButtonDown)
	{
		if(GET_BIT(pIMEU3_Bat->uDIStatus, 6) == 0)
		{
			//模块关机
			if(blobManualCtrlRect.emState != STAT_MANUAL_STOP)
			{
				blobManualCtrlRect.emState = STAT_MANUAL_STOP;
				needCtl = 1;
			}
			nDelayButton = 4;
		}
		else if(GET_BIT(pIMEU3_Bat->uDIStatus, 5) == 0)
		{
			//限制在0%power
			//printf("%d\n", (int)fabs(blobManualCtrlRect.fActv_Pwr));
			if((int)fabs(blobManualCtrlRect.fActv_Pwr) > 0)
			{
				needCtl = 1;
				blobManualCtrlRect.fActv_Pwr = 0.0;
				// varV2.emValue = 1;
				// Push_VarValue(SETTYPE_INTERN_SETT, pSTBatt_Rough->hSampler, 701, 91011, varV2);//奥地利认证：增加进入PCS硬件降载功能
				//printf("%f == 0.0?\n", blobManualCtrlRect.fActv_Pwr);
			}
			nDelayButton = 4;
		}
		else if(GET_BIT(pIMEU3_Bat->uDIStatus, 4) == 0)
		{
			//限制在30%power
			// printf("%d\n", (int)fabs(blobManualCtrlRect.fActv_Pwr));
			if((int)fabs(blobManualCtrlRect.fActv_Pwr) > (int)(fTotalPowerLimit*0.3))
			{
				needCtl = 1;
				blobManualCtrlRect.fActv_Pwr = fTotalPowerLimit*0.3;
				// varV2.emValue = 1;
				// Push_VarValue(SETTYPE_INTERN_SETT, pSTBatt_Rough->hSampler, 701, 91011, varV2);//进入硬件降载
				// printf("%f == %f?\n", blobManualCtrlRect.fActv_Pwr, fTotalPowerLimit*0.3);
			}
			nDelayButton = 4;
		}
		else if(GET_BIT(pIMEU3_Bat->uDIStatus, 3) == 0)
		{
			//限制在60%power
			// printf("%d\n", (int)fabs(blobManualCtrlRect.fActv_Pwr));
			if((int)fabs(blobManualCtrlRect.fActv_Pwr) > (int)(fTotalPowerLimit*0.6))
			{
				needCtl = 1;
				blobManualCtrlRect.fActv_Pwr = fTotalPowerLimit*0.6;
				// varV2.emValue = 1;
				// Push_VarValue(SETTYPE_INTERN_SETT, pSTBatt_Rough->hSampler, 701, 91011, varV2);//进入硬件降载
				// printf("%f == %f?\n", blobManualCtrlRect.fActv_Pwr, fTotalPowerLimit*0.6);
			}
			nDelayButton = 4;
		}
	}
	else//按钮未按下
	{
		if(nDelayButton > 0)
		{
			// varV2.emValue = 0;
			// Push_VarValue(SETTYPE_INTERN_SETT, pSTBatt_Rough->hSampler, 701, 91011, varV2);//退出硬件降载
			memcpy(&blobManualCtrlRect, &blobRawManuCtl, sizeof(A_BLOB_MANUALCTRL_RECT_CMD));
			needCtl = 1;
			nDelayButton=0;
		}
		else
		{
			memcpy(&blobRawManuCtl, &blobManualCtrlRect, sizeof(A_BLOB_MANUALCTRL_RECT_CMD));
		}
	}

	if(needCtl)
	{
		Push_BlobValue(SETTYPE_INTERN_CTRL, pSTBatt_Rough->hSampler, 200, 214, 
			(void*)&blobManualCtrlRect, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);

	}

}

static void Update_OCVCalibrationData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
    static time_t tLastWriteTime = 0;
    static int nIdx = 0;
    int i;
	static int j = 0;
    time_t tCurrentTime = time(NULL);
    
    if (tCurrentTime - tLastWriteTime >= 10)
    {
        for(i = 0; i < pSTBatt_Rough->nSystemCfgBattNum; i++)
        {
            if(nIdx >= MAX_OCV_REC_NUM)
            {
                nIdx = 0;
                pSTBatt_Rough->nToTalRecNum = 0;
            }
            
            pSTBatt_Rough->stCalibratedSOCData[nIdx].tmUTCTime 			= tCurrentTime;
			pSTBatt_Rough->stCalibratedSOCData[nIdx].emBattStatus_Run 	= pSTBatt_Rough->blobSingleBattInfo[0].emBattStatus_Run;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].fPackMaxVolt 		= pSTBatt_Rough->blobSingleBattInfo[0].fMax_CellVolt;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].fPackMinVolt 		= pSTBatt_Rough->blobSingleBattInfo[0].fMin_CellVolt;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].fPackVolt 			= pSTBatt_Rough->blobSingleBattInfo[0].fPackTotalVolt;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].fPackCurr 			= pSTBatt_Rough->blobSingleBattInfo[0].fPackTotalCurr;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].nSOC 				= pSTBatt_Rough->blobSingleBattInfo[0].nSOC;
            pSTBatt_Rough->stCalibratedSOCData[nIdx].fPackTemp 			= pSTBatt_Rough->blobSingleBattInfo[0].fAvg_CellTemp; 
            pSTBatt_Rough->stCalibratedSOCData[nIdx].nOCVResult 		= pSTBatt_Rough->bOCVFlag; 
            pSTBatt_Rough->stCalibratedSOCData[nIdx].nCalibratedSOC 	= pSTBatt_Rough->nCalibratedSOC; 
            
            pSTBatt_Rough->nToTalRecNum = nIdx + 1;
            nIdx++;
        }
        
        tLastWriteTime = tCurrentTime;
    }
}

static char* Make_OCVCalibrationData_2StrLine(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nIdx, char* pOutStr, int nOutStrLen)
{
    if (pSTBatt_Rough == NULL || pOutStr == NULL || nOutStrLen <= 0 || nIdx < 0)
    {
        return NULL;
    }
    
    CalibratedSOCData* pItem = &pSTBatt_Rough->stCalibratedSOCData[nIdx];
    
    memset(pOutStr, 0, nOutStrLen);
    char szIdx[8] = {0};
    char szTime[64] = {0};
    char szTTTT[64] = {0};
    char szBmsStatus[64] = {0}; 
    
    snprintf(szIdx, sizeof(szIdx), "%d,", nIdx + 1);
    strcat(pOutStr, szIdx);
    
    snprintf(szTTTT, 64, "%s,", TimeToString(pItem->tmUTCTime, "%H:%M:%S", szTime, sizeof(szTime)));
    strcat(pOutStr, szTTTT);

	switch (pItem->emBattStatus_Run)
	{
		case STBATT_RUNSTAT_CHARGE:
			strcpy(szBmsStatus, "CHARGE");
			break;
		case STBATT_RUNSTAT_DISCHARGE:
			strcpy(szBmsStatus, "DISCHARGE");
			break;
		case STBATT_RUNSTAT_FULL_CHARGE:
			strcpy(szBmsStatus, "FULL_CHARGE");
			break;
		case STBATT_RUNSTAT_FULL_DISCHARGE:
			strcpy(szBmsStatus, "FULL_DISCHARGE");
			break;
		case STBATT_RUNSTAT_IDLE:
			strcpy(szBmsStatus, "IDLE");
			break;
		case STBATT_RUNSTAT_NOT_INSTALLED:
			strcpy(szBmsStatus, "NOT_INSTALLED");
			break;
		case STBATT_RUNSTAT_COMM_FAIL:
			strcpy(szBmsStatus, "COMM_FAIL");
			break;
		case STBATT_RUNSTAT_SLEEP:
			strcpy(szBmsStatus, "SLEEP");
			break;
		default:
			strcpy(szBmsStatus, "UNKNOWN");
			break;
	}
	
    snprintf(szTTTT, 64, "%s,", szBmsStatus);
    strcat(pOutStr, szTTTT);
	        
    snprintf(szTTTT, 64, "%d,", (int)(pItem->fPackMaxVolt* 1000));
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%d,", (int)(pItem->fPackMinVolt* 1000));
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%.2f,", pItem->fPackVolt);
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%.2f,", pItem->fPackCurr);
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%.2f,", pItem->fPackTemp);
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%d,", pItem->nSOC);
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%d,", pItem->nOCVResult);
    strcat(pOutStr, szTTTT);
    
    snprintf(szTTTT, 64, "%d\n", pItem->nCalibratedSOC);
    strcat(pOutStr, szTTTT);

    return pOutStr;
}

static void Save_OCVCalibrationData(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
    int nIdx;
    FILE* fp = fopen("/data/OCVCalibrationData.csv", "w+");
    if(!fp)
    {
        LogOut(MAIN_MODULE_MAIN, "OCVCalibrationData DATA SAVE", LOG_TYPE_ERROR, "Can NOT fopen OCVCalibrationData.csv, please check!");
        return;
    }
    
    char szRecLine[2048] = {0};
    
    snprintf(szRecLine, sizeof(szRecLine), 
            "Id,UTC Time,BattStatus,PackMaxVolt(mV),PackMinVolt(mV),PackVolt(V),PackCurr(A),PackTemp,SOC,OCVResult,CalibratedSOC\n");
    fputs(szRecLine, fp); 
    memset(szRecLine, 0, sizeof(szRecLine)); 
    
    int nMaxRecNum = (pSTBatt_Rough->nToTalRecNum > MAX_OCV_REC_NUM -1) ? MAX_OCV_REC_NUM : pSTBatt_Rough->nToTalRecNum;
    for(nIdx=0; nIdx < nMaxRecNum; nIdx++)
    {
        memset(szRecLine, 0, sizeof(szRecLine)); 
        if(Make_OCVCalibrationData_2StrLine(pSTBatt_Rough, nIdx, szRecLine, sizeof(szRecLine)) != NULL)
        {
            fputs(szRecLine, fp);
        }
    }
    fflush(fp);
    fclose(fp);
}

//------Only for Electra----//
static int MapVoltageToSOC(BOOL bIsChargeOCVType, int nVoltage)
{
    int nSOC = 0, i;
	int VoltageDiff = 0;
	int SOCDiff = 0;
	int VoltageOffset = 0;

	const VoltageSOCMap* Volt_Soc_Map = NULL;
    int mapLength = 0;
	if(bIsChargeOCVType == 1)//充电校准
	{
		Volt_Soc_Map = ChargeSOCMap;
		mapLength = ARRAY_LEN(ChargeSOCMap);
	}
	else//放电校准
	{
		Volt_Soc_Map = DisChargeSOCMap;
		mapLength = ARRAY_LEN(DisChargeSOCMap);
	}

	// 电压低于最低值，返回0%
	if(nVoltage <= Volt_Soc_Map[0].nVoltage)
    {
        return Volt_Soc_Map[0].nSOC;
    }

    // 电压高于最高值，返回100% 
    if(nVoltage >= Volt_Soc_Map[mapLength - 1].nVoltage)
    {
        return Volt_Soc_Map[mapLength - 1].nSOC;
    }

    for(i = 0; i < mapLength - 1; i++)
    {
		if(bIsChargeOCVType == 1)//充电校准
		{
			TRACED("Charging nVoltage = %d", nVoltage);
			if(nVoltage == 3295)
			{
				nSOC = 2500;
				break;
			}
			else if (nVoltage == 3339)
			{
				nSOC = 9500;
				break;
			}
			else if(nVoltage >= 2884 && nVoltage < 3295 || nVoltage > 3339 && nVoltage <= 3475)
			{
				if(nVoltage >= Volt_Soc_Map[i].nVoltage && nVoltage < Volt_Soc_Map[i + 1].nVoltage)
				{
					VoltageDiff = Volt_Soc_Map[i + 1].nVoltage - Volt_Soc_Map[i].nVoltage;
					SOCDiff = Volt_Soc_Map[i + 1].nSOC - Volt_Soc_Map[i].nSOC;
					VoltageOffset = nVoltage - Volt_Soc_Map[i].nVoltage;        
					nSOC = Volt_Soc_Map[i].nSOC + (int)((SOCDiff * VoltageOffset) / VoltageDiff);
					//TRACED("nSOC = %d", nSOC);
					break;
				}
			}
		}

		if(bIsChargeOCVType == 0)//放电校准
		{
			if(nVoltage == 3283)
			{
				nSOC = 3000;
				break;
			}
			else if (nVoltage == 3332)
			{
				nSOC = 9500;
				break;
			}
			else if(nVoltage >= 2884 && nVoltage < 3283 || nVoltage > 3332 && nVoltage <= 3516) 
			{
				if(nVoltage >= Volt_Soc_Map[i].nVoltage && nVoltage < Volt_Soc_Map[i + 1].nVoltage)
				{
					VoltageDiff = Volt_Soc_Map[i + 1].nVoltage - Volt_Soc_Map[i].nVoltage;
					SOCDiff = Volt_Soc_Map[i + 1].nSOC - Volt_Soc_Map[i].nSOC;
					VoltageOffset = nVoltage - Volt_Soc_Map[i].nVoltage;        
					nSOC = Volt_Soc_Map[i].nSOC + (int)((SOCDiff * VoltageOffset) / VoltageDiff);
					//TRACED("nSOC = %d", nSOC);
					break;
				}
			}
		}
    }
    return nSOC;
}

//OCV校准前置条件检查函数
static void Prerequisites4OCVCalibration(STBATT_SAMPLER_ROUGH* pSTBatt_Rough, int nBattIdx)
{
    static OCV_CAL_STAGE emCurStage = STAGE_CHARGE_DISCHARGE;
    static time_t tmStageStart = 0;  // 当前阶段的开始时间
    time_t tmNow = time(NULL);
    float fCurrent = fabs(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fPackTotalCurr);
    STBATT_STATEMACHINE_STATE emBatteryState = pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emBattStatus_Run;
    switch (emCurStage)
    {
        case STAGE_CHARGE_DISCHARGE:
        {
			pSTBatt_Rough->bOCVFlag = FALSE;  //进入充电或者放电模式下  OCV标志位置为0
			//大电流OCV充放校准时间20min
            if (fCurrent >= CURRENT_THRESHOLD)
            {
				//根据状态判断OCV校准类型
                if(emBatteryState == STBATT_RUNSTAT_CHARGE || emBatteryState == STBATT_RUNSTAT_FULL_CHARGE)
				{
                    pSTBatt_Rough->emSOCCalibration = SOCChargingCalibration;
                }
				else if(emBatteryState == STBATT_RUNSTAT_DISCHARGE || emBatteryState == STBATT_RUNSTAT_FULL_DISCHARGE)
				{
                    pSTBatt_Rough->emSOCCalibration = SOCDischargingCalibration;
                }

                if (tmStageStart == 0)
				{
					//进入充电或者放电模式下，记录当前时间
                    tmStageStart = tmNow;
                }

                // 计算该阶段持续时长，达到预设时间则进入下一个阶段
                double dDuration = difftime(tmNow, tmStageStart);
                // TRACED("Charge_DisCharg Time: [%.1f]s (Need [%d]s)\n", dDuration, CHARGE_DISCHARGE_TIME2);
                if (dDuration >= CHARGE_DISCHARGE_TIME2)
				{
                    emCurStage = STAGE_WAIT_IDLE;      
                    tmStageStart = 0;             
                }
            }
			//小电流OCV充放校准时间40min
			else if(fCurrent < CURRENT_THRESHOLD && fCurrent >= CURRENT_THRESHOLD2)
            {
				//根据状态判断OCV校准类型
                if(emBatteryState == STBATT_RUNSTAT_CHARGE || emBatteryState == STBATT_RUNSTAT_FULL_CHARGE)
				{
                    pSTBatt_Rough->emSOCCalibration = SOCChargingCalibration;
                }
				else if(emBatteryState == STBATT_RUNSTAT_DISCHARGE || emBatteryState == STBATT_RUNSTAT_FULL_DISCHARGE)
				{
                    pSTBatt_Rough->emSOCCalibration = SOCDischargingCalibration;
                }

                if (tmStageStart == 0)
				{
					//进入充电或者放电模式下，记录当前时间
                    tmStageStart = tmNow;
                }

                // 计算该阶段持续时长，达到预设时间则进入下一个阶段
                double dDuration = difftime(tmNow, tmStageStart);
                TRACED("Wait Idle: [%.1f]s (Need [%d]s)\n", dDuration, CHARGE_DISCHARGE_TIME);
                if (dDuration >= CHARGE_DISCHARGE_TIME)
				{
                    emCurStage = STAGE_WAIT_IDLE;      
                    tmStageStart = 0;             
                }
            }
            else
            {
                // 电流不满足/状态不对，重置充放电计时
                // TRACED("Charge/Discharge Curr/State not match, reset time\n");
                tmStageStart = 0;
            }
            break;
        }
		case STAGE_WAIT_IDLE:
		{
			//满足前序充放电阶段时间后进入该阶段，如果为空闲状态则进入空闲等待阶段计算时间
			TRACED("Wait Idle Stage\n");
			if (emBatteryState == STBATT_RUNSTAT_IDLE || 
			   (fCurrent < PCS_HOTSTANDBY_MAX_CURRENT && 
			   (emBatteryState == STBATT_RUNSTAT_FULL_CHARGE || emBatteryState == STBATT_RUNSTAT_FULL_DISCHARGE)))
			{
				TRACED("Goto STAGE_IDLE\n");
				emCurStage = STAGE_IDLE;      
                tmStageStart = 0; 
			}
			else
			{
				if(pSTBatt_Rough->emSOCCalibration == SOCChargingCalibration)
				{
					if(emBatteryState == STBATT_RUNSTAT_DISCHARGE)
					{
						//空闲等待期间充放电转换，重新计时
						// TRACED("Wait Idle Stage  Charging State Change 2 DisCharging Statt----Reset\n");
						tmStageStart = 0; 
						emCurStage = STAGE_CHARGE_DISCHARGE;
					}
					else if(emBatteryState == STBATT_RUNSTAT_CHARGE && fCurrent >= PCS_HOTSTANDBY_MAX_CURRENT)
					{
						//空闲等待期间一直充电还没转换到下一阶段，超过等待时间重新计时
						if(tmStageStart == 0)
						{
							tmStageStart = tmNow;
							TRACED("WaitIdle Stage Start at %ld\n", tmStageStart);
						}
						double dDuration = difftime(tmNow, tmStageStart);
						TRACED("WaitIdle Duration: [%.1f]s (Need [%d]s)\n", dDuration, IDLE_WAIT_TIMEOUT_TIME);
						if (dDuration >= IDLE_WAIT_TIMEOUT_TIME)
						{
							TRACED("WaitIdle Last [%.2f], Beyond MAX Idle Time :[%d]s\n", dDuration, IDLE_WAIT_TIMEOUT_TIME);
							emCurStage = STAGE_CHARGE_DISCHARGE;
							tmStageStart = 0;
						}
					}
				}
				else
				{
					if(emBatteryState == STBATT_RUNSTAT_CHARGE)
					{
						//空闲等待期间充放电转换，重新计时
						// TRACED("Wait Idle Stage  Discharging State Change 2 Charging Statt----Reset\n");
						tmStageStart = 0; 
						emCurStage = STAGE_CHARGE_DISCHARGE;
					}
					else if(emBatteryState == STBATT_RUNSTAT_DISCHARGE && fCurrent >= PCS_HOTSTANDBY_MAX_CURRENT)
					{
						//空闲等待期间一直放电还没转换到下一阶段，超过等待时间重新计时
						if(tmStageStart == 0)
						{
							tmStageStart = tmNow;
							// TRACED("WaitIdle Stage Start at %ld\n", tmStageStart);
						}
						double dDuration = difftime(tmNow, tmStageStart);
						// TRACED("WaitIdle Duration: [%.1f]s (Need [%d]s)\n", dDuration, IDLE_WAIT_TIMEOUT_TIME);
						if (dDuration >= IDLE_WAIT_TIMEOUT_TIME)
						{
							// TRACED("WaitIdle Last [%.2f], Beyond MAX Idle Time :[%d]s\n", dDuration, IDLE_WAIT_TIMEOUT_TIME);
							emCurStage = STAGE_CHARGE_DISCHARGE;
							tmStageStart = 0;
						}
					}
				}
				// TRACED("Charging Or DisCharing Time is Enough, Please Set to Idle\n");
			}
			break;
		}

        case STAGE_IDLE:
        {
            if (emBatteryState == STBATT_RUNSTAT_IDLE || 
			   (fCurrent < PCS_HOTSTANDBY_MAX_CURRENT && 
			   (emBatteryState == STBATT_RUNSTAT_FULL_CHARGE || emBatteryState == STBATT_RUNSTAT_FULL_DISCHARGE)))
			{
                if(tmStageStart == 0)
				{
                    tmStageStart = tmNow;
                }

                double dDuration = difftime(tmNow, tmStageStart);
                if (dDuration >= IDLE_TIME)
				{
                    //TRACED("Idle State Last [%.1f], All Prerequisites Done\n", dDuration);
					emCurStage = STAGE_COMPLETED;
                    tmStageStart = 0;
                }				
            }
            else
            {
                // TRACED("Battery not in Idle State, Back to Charge/Discharge Stage\n");
                tmStageStart = 0; 
				emCurStage = STAGE_CHARGE_DISCHARGE;
            }
            break;
        }

        case STAGE_COMPLETED:
        {
			if(emBatteryState == STBATT_RUNSTAT_IDLE)
			{
				if(tmStageStart == 0)
				{
                    tmStageStart = tmNow;
                }

				double dDuration = difftime(tmNow, tmStageStart);
                if (dDuration >= OCVCOMPLETE_DURATION_TIME)
				{
                    // TRACED("Completed Complete Last [%.2f], Beyond MAX Idle Time :[%d]s\n", dDuration, OCVCOMPLETE_DURATION_TIME);
					emCurStage = STAGE_CHARGE_DISCHARGE;
                    tmStageStart = 0;
                }
				else
				{
					// TRACED("Completed Stage Last [%.2f], All Prerequisites Done\n", dDuration);
					pSTBatt_Rough->bOCVFlag = TRUE;   //完成OCV前序条件，置为1
				}
			}
			else if(emBatteryState == STBATT_RUNSTAT_CHARGE || emBatteryState == STBATT_RUNSTAT_DISCHARGE)
			{
				// TRACED("Now is Charging or DisCharging, Reset Stage\n");
				emCurStage = STAGE_CHARGE_DISCHARGE;
				tmStageStart = 0;
			}
            break;
        }
        default:
            // TRACED("Invalid Stage, reset to Charge/Discharge\n");
            emCurStage = STAGE_CHARGE_DISCHARGE;
            tmStageStart = 0;
            break;
    }
}

static void Rpmsg(STBATT_SAMPLER_ROUGH* pSTBatt_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ERRCDDE_DPR err;
	int nErrCode = 0;

	static time_t t2 = 0;
	if((pSTBatt_Rough->RpmsgHanle_cycle == NULL) && (ABS(_NOW_ - t2) >= 60))
	{
		t2 = _NOW_;
		printf("Open Rpmsg Handle cycle...\n");
		pSTBatt_Rough->RpmsgHanle_cycle = pSTBatt_Rough->Rpmsg_cycle_info.Open(RPMSG_CYCLE_INFO,
					LOCAL_UPGRADE_FIRMWARE_NAME, 
					2, 1800, &nErrCode); 	
	}

	static time_t t3 = 0;
	if((pSTBatt_Rough->RpmsgHanle_faq == NULL) && (ABS(_NOW_ - t3) >= 60))
	{
		t3 = _NOW_;
		printf("Open Rpmsg Handle faq...\n");
		pSTBatt_Rough->RpmsgHanle_faq = pSTBatt_Rough->Rpmsg_faq_info.Open(RPMSG_FAO_INFO,
					LOCAL_UPGRADE_FIRMWARE_NAME, 
					3, 1800, &nErrCode); 	
	}

	return;
}

#define MAX_GROUPS      30
#define MAX_PARAMS      1024
#define MAX_TYPE_LEN    32

// ---- 数据基础结构 ----
typedef struct {
    char type[MAX_TYPE_LEN];
    int cfg_val;
} Param;

typedef struct {
    int index;            // 组索引号（如 100, 101）
    int param_count;
    Param params[MAX_PARAMS];
} ParamGroup;

// 🎯【核心重构】将数据和计数器打包成“上下文”，彻底消灭全局变量
typedef struct {
    ParamGroup groups[MAX_GROUPS];
    int group_count;
} ParamContext;


// ---- 内部私有工具函数 ----

// 查找分组（必须传入上下文指针）
static int find_group(const ParamContext *ctx, int idx) {
    for (int i = 0; i < ctx->group_count; i++) {
        if (ctx->groups[i].index == idx) return i;
    }
    return -1;
}

// 获取或创建组（必须传入上下文指针）
static ParamGroup* get_or_create_group(ParamContext *ctx, int idx) {
    int pos = find_group(ctx, idx);
    if (pos != -1) return &ctx->groups[pos];
    
    if (ctx->group_count >= MAX_GROUPS) {
        fprintf(stderr, "【CSV 错误】超出最大分组数 %d\n", MAX_GROUPS);
        return NULL;
    }
    
    ParamGroup *grp = &ctx->groups[ctx->group_count];
    grp->index = idx;
    grp->param_count = 0;
    ctx->group_count++; // 计数器自增
    return grp;
}

// 零拷贝 trim
static char* trim(char *str) {
    if (!str) return NULL;
    char *start = str;
    while (*start == ' ' || *start == '\t') start++;
    if (*start == '\0') return start;

    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = '\0';
    return start;
}

static double to_double(const char *str) {
    if (!str || *str == '\0') return 0.0;
    char *end;
    double val = strtod(str, &end);
    return (end == str) ? 0.0 : val;
}

// 解析函数：不再依赖全局变量，数据直接写进传入的 ctx 句柄中
int load_csv(const char *filename, ParamContext *ctx) {
    if (!filename || !ctx) return -1;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("【CSV 错误】无法打开文件");
        return -1;
    }

    // 初始化/清空上下文（防备重用同一个上下文未清空的情况）
    ctx->group_count = 0;
    memset(ctx->groups, 0, sizeof(ctx->groups));

    char line[2048];
    int is_header = 1;
    int rows = 0;

    // 高速局部缓存（线程安全，O(1) 块状连续行秒杀）
    int last_idx = -1;
    ParamGroup *last_grp = NULL;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue; 
        if (is_header) {
            is_header = 0;
            continue;
        }

        char *fields[6] = {NULL};
        int cnt = 0;
        char *running = line;
        char *token;

        // 按需截断，填满 6 列立刻 break
        while ((token = strsep(&running, ",")) != NULL) {
            fields[cnt++] = trim(token);
            if (cnt >= 6) break; 
        }

        if (cnt < 6) continue;  

        int idx = atoi(fields[0]);            
        char *type = fields[3];               
        double cfg_val = to_double(fields[5]); 

        // 缓存定位逻辑
        ParamGroup *grp = NULL;
        if (idx == last_idx && last_grp != NULL) {
            grp = last_grp; 
        } else {
            grp = get_or_create_group(ctx, idx); // 换组时传入 ctx 句柄进行操作
            if (!grp) break;
            last_idx = idx; 
            last_grp = grp;
        }

        if (grp->param_count >= MAX_PARAMS) {
            fprintf(stderr, "【CSV 警告】索引 %d 参数行数超限\n", idx);
            continue;
        }

        Param *p = &grp->params[grp->param_count];
        strncpy(p->type, type, MAX_TYPE_LEN - 1);
        p->type[MAX_TYPE_LEN - 1] = '\0'; 
        p->cfg_val = cfg_val;
        grp->param_count++;
        rows++;
    }

    fclose(fp);
    return rows; // 返回总解析行数
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = (STBATT_SAMPLER_ROUGH*)pSampler->pDataHeap;
	VAR_VALUE varV;

	// Infy自有核间通信协议 D9
	// if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K)
	if(pSTBatt_Rough->bRpmsgPortReady == FALSE)
	{
		int nErrCode = 0;

		nErrCode = pSTBatt_Rough->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT, 12, NULL, &pSTBatt_Rough->pRpmsgComm, sizeof(COMMPORT));

		if(ERR_COMM_OK != nErrCode)
		{
			TRACED("Open RPMSGComm Failed !!! nErrCode=[%d]\n", nErrCode);
			goto STA_1;
		}

		pSTBatt_Rough->hRpmsgDev = pSTBatt_Rough->pRpmsgComm.Open(
			RPMSG_CELL_INFO,
			pSampler->info.szSamplerSetting,
			3, RPMSG_READ_TIMEOUT, &nErrCode);
		if(pSTBatt_Rough->hRpmsgDev == NULL)
		{
			TRACED("Open RPMSG Failed !!! hRpmsgDev is NULL\n");
			goto STA_1;
		}

		if(ERR_COMM_OK != nErrCode)
		{
			TRACED("Open RPMSG Failed !!! nErrCode=[%d]\n", nErrCode);
			goto STA_1;
		}

		pSTBatt_Rough->bRpmsgPortReady = TRUE;

		TRACED("Open RPMSG Succeed!!!!!!!!!!!!!!!!!\n");
	}

	YFY_Batt_Rpmsg_Sample(pSTBatt_Rough);
	MapRawData_2_InfyBMSBlob(pSTBatt_Rough);

	Rpmsg(pSTBatt_Rough);
	if(pSTBatt_Rough->RpmsgHanle_cycle != NULL)
	{
		ReadAndParseBmsStatus(pSTBatt_Rough);
	}

	if(pSTBatt_Rough->RpmsgHanle_faq != NULL)
	{
		ReadAndParseConfigParameters(pSTBatt_Rough);
	}
			// if((!s_infy_BattRunData[n].bBattInfo_Regisited) ||
        	// s_infy_BattRunData[n].bIsCommFail || 
	Bat_Rpmsg_ExchangeData(pSTBatt_Rough, 0);
	Bat_Rpmsg_ExchangeData(pSTBatt_Rough, 1);
	Push_BmsRunStatus(pSTBatt_Rough);

	// test 临时
	// printf("sample push1\n");
	Push_STBatt_AllData(pSTBatt_Rough);


STA_1:

	if(pSTBatt_Rough->emBattMFG == BATT_CFG_NO_BATT)
	{
 		Sleep(1000);
		return ERR_SAMPLE_OK;
	}
	/*
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_PL)
	{	
		PL_Batt_Sample(pSTBatt_Rough);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_ND)
	{
		ND_Batt_Sample(pSTBatt_Rough);
	}*/
	else if((pSTBatt_Rough->emBattMFG == BATT_MFG_ND_V33)||
	(pSTBatt_Rough->emBattMFG == BATT_MFG_NIFY_V33_500K)||
	(pSTBatt_Rough->emBattMFG == BATT_MFG_IMTCU_V33_500K))
	{
		// ND_V33_Batt_Sample(pSTBatt_Rough);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K || pSTBatt_Rough->emBattMFG == BATT_MFG_TEST_NO_BATT)
	{	

		//TRACEI("---Sample----------YFY_Batt_Sample---");
		// YFY_Batt_Sample(pSTBatt_Rough);
	}
	else if(pSTBatt_Rough->emBattMFG == BATT_MFG_FENGC_MODBUS)
	{
		//FengC_Batt_Modbus_Sample(pSTBatt_Rough);
		// FengCH_Batt_Sample(pSTBatt_Rough);
		TRACEX("INFO: FengChao Modbus Sampler!!!\n");
	}
	else
	{
		Sleep(1000);
		return ERR_SAMPLE_OK;
		/* code */
	}

	//---更新OCV校准数据---//
	Update_OCVCalibrationData(pSTBatt_Rough);
	//---保存OCV校准数据---//
	Save_OCVCalibrationData(pSTBatt_Rough);
	//---检查OCV校准前置条件---//
	int nBattIdx = 0;
	Prerequisites4OCVCalibration(pSTBatt_Rough, nBattIdx);
	int nSOCCalibrationVlot;
	if(pSTBatt_Rough->bOCVFlag == TRUE)
	{
		//判断OCV校准后电压是否在可校准的范围内
		if(pSTBatt_Rough->emSOCCalibration == SOCChargingCalibration)
		{
			nSOCCalibrationVlot = (int)(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMax_CellVolt * 1000);
			if(nSOCCalibrationVlot > 3295 && nSOCCalibrationVlot < 3339)
			{
				pSTBatt_Rough->bOCVFlag = FALSE;
				varV.emValue = FALSE;
			}
			else
			{
				pSTBatt_Rough->bOCVFlag = TRUE;
				varV.emValue = TRUE;
			}
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501, 3911, varV);
		}
		else if(pSTBatt_Rough->emSOCCalibration == SOCDischargingCalibration)
		{
			nSOCCalibrationVlot = (int)(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMin_CellVolt * 1000);
			if(nSOCCalibrationVlot > 3283 && nSOCCalibrationVlot < 3332)
			{
				pSTBatt_Rough->bOCVFlag = FALSE;
				varV.emValue = FALSE;
			}
			else
			{
				pSTBatt_Rough->bOCVFlag = TRUE;
				varV.emValue = TRUE;
			}
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501, 3911, varV);
		}
	}
	else
	{
		varV.emValue = FALSE;
		Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501, 3911, varV);
	}
	// TRACED("Prerequisites4OCVCalibration Flag = %d\n", pSTBatt_Rough->bOCVFlag);
	
	CalcBatt_EnergyCost(pSTBatt_Rough);

	//Now push data into db
	Push_STBatt_AllData(pSTBatt_Rough);

	//Now push Air Status & Alarm & fault data into db
	//TRACEI("-------------------TONGFEI_AIR  =   %d-------------", pSTBatt_Rough->emAirType[0]);
	/* NJX 2026-07-06: 同飞(TF)空调数据改由独立 sampler_aircon.so 通过 RS485(ttyS9) Modbus 直接读取。
	   st_batt_sampler 走 CAN 总线获取空调数据, 但 D9 平台 CAN 上无空调数据,
	   会用 0 覆盖 sampler_aircon 推送的真实值, 导致网页空调数据大部分显示空。
	   故跳过同飞空调参数推送, 由 sampler_aircon 独占 dev 951 / params 1770-1810。 */
	/* NJX 2026-08-13: EU3/英飞源空调数据同样由 sampler_aircon.so 通过 RS485(ttyS9) 直接读取,
	   st_batt CAN 上无 EU3 空调数据, 推送会用 0 覆盖 sampler_aircon 的真实值。 */
	/* if(INFYPOWER_AIR == pSTBatt_Rough->emAirType[0])
	{
		Push_STBatt_AirStatusData(pSTBatt_Rough);
	} */
	/* TF 空调由 sampler_aircon.so 独占, 跳过 CAN 推送
	else if(TONGFEI_AIR == pSTBatt_Rough->emAirType[0])
	{
		Push_TongFei_AirStatusData(pSTBatt_Rough);
	} */
	if(IMEU3_AIR == pSTBatt_Rough->emAirType[0])//edit by kuangyue 20250217
	{
		Push_IMEU3_AirStatusData(pSTBatt_Rough);
	}
	else if(XINMIAO_AIR == pSTBatt_Rough->emAirType[0])
	{
		Push_XinMiao_AirStatusData(pSTBatt_Rough);
	}
	
	//Now push Pack Data Info into db
	Push_STBatt_PackDataInfo(pSTBatt_Rough);

	if (IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[0])
	{
#ifdef IMMU3
		push2524IMEU3CanData(pSTBatt_Rough->IMEU3CANRoughData, pSTBatt_Rough->nSystemCfgBattNum);
#else
		pushBattCabinetIMEU3CANData(pSTBatt_Rough->IMEU3CANRoughData, MAX_IMEU_NUM);  //电池仓内IMEU3

		if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)   //液冷备电中电源柜IMEU3 
		{
			pushLiquidCoolBackupPowerCabinetIMEU3(&pSTBatt_Rough->PowerEU3_OtherEU3_Data[0]);
			pushBackupBattIMEU3Data(pSTBatt_Rough->IMEU3CANRoughData, MAX_IMEU_NUM);  //备电柜新增电池仓数据
			IMEU3DoCtl(pSTBatt_Rough);
			IMEU3CtlPWM(pSTBatt_Rough);
		}
		else
		{
			//液冷一体储能机型电源仓中IMEU3
			pushLiquidCoolPowerCabinetIMEU3(&pSTBatt_Rough->PowerEU3_OtherEU3_Data[0]);
		}
		if(LIQUID_COOLING_ADD_STS == pSTBatt_Rough->emESSDevType) //液冷一体储能机+STS柜中IMEU3
		{
			pushSTSCabinetIMEU3CanData(&pSTBatt_Rough->PowerEU3_OtherEU3_Data[1]);
		}
#endif

		pushIMEU3toCloudPlatform(pSTBatt_Rough->IMEU3CANRoughData, pSTBatt_Rough->PowerEU3_OtherEU3_Data, pSTBatt_Rough->emESSDevType, pSTBatt_Rough->blobIMEU2Data);
	}

	if (LIQUID_COOLING_ADD_STS == pSTBatt_Rough->emESSDevType)
	{
		VAR_VALUE varV;	
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 166, &varV, 4); //0:自动并网 1：手动并网
		if (0 == varV.emValue)
		{
			OnHandle_STS_Cabinet(pSTBatt_Rough);
		}
		//控制STS柜风扇转速
		OnHandleSTSCabinetFan(pSTBatt_Rough);
	}
	else if (LIQUID_COOLING_BACKUP == pSTBatt_Rough->emESSDevType)
	{
		VAR_VALUE varV;	
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_SP_VAR, 200, 166, &varV, 4); //0:自动并网 1：手动并网
		if (0 == varV.emValue)
		{
			OnHandleLiquidBackupAutoOnOffGrid(pSTBatt_Rough);
		}
	}
	// 瑞士认证 LJH250718 fix:风冷手动模式失效
	// if(pSTBatt_Rough->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED)
	// {
	// 	Authentication(pSTBatt_Rough);
	// }
	//Now do the battChargeManagement
    STBatt_Management(pSTBatt_Rough);

	//Calculate Charge/Discharge Energy using BMS upload 144a info. Added by Jeffery 20240715
	CalcBatt_EnergyCostbyBMS(pSTBatt_Rough);

	//--- 2021-10-8 John added this ---//
	for(nBattIdx = 0; nBattIdx < MAX_SINGLE_STBATT_NUM; nBattIdx++)
	{
		Update_BmsRuntimeData_2DB(pSTBatt_Rough, nBattIdx);
	}

	//--- 20220428 John Added This -----//
	Save_BMSCanMsg2Log(pSTBatt_Rough);

	Sleep_Ex(100);

	return ERR_SAMPLE_OK;
}


DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = (STBATT_SAMPLER_ROUGH*)pSampler->pDataHeap;
	
	int nRet = ERR_CONTROL_OK;
	int nBattIdx;

	//屏蔽代码，设置成无电池后，其它设置项都会失效
/*
	if(pSTBatt_Rough->emBattMFG == BATT_CFG_NO_BATT)
	{
		TRACE("Control: BATT_CFG_NO_BATT !!!\n");
		return ERR_CONTROL_FAILED;
	}
*/
	if((nDevId >= 601) && (nDevId <= 604)) //ctrl  IMEU2 and IMEU3   根据设置参数选择使用IMEU2还是IMEU3
	{
		int nWhichIMEU = nDevId - 601;
		
		if (nWhichIMEU >= MAX_IMEU_NUM)
			return ERR_CONTROL_FAILED;

		if (PARAM_TYPE_CONTROL == emParaType)
		{
			nRet = IMEU3SendCtlCAN(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
		}
		else if (PARAM_TYPE_SETTING == emParaType)
		{
			if (450 == nParamId) //IMEU2 Config Type
			{	
				pSTBatt_Rough->emIMEUType[nWhichIMEU] = (IMEU_TYPE)pVarVal->emValue;

				//IMEU2 communicates via RS485, IMEU3 communicates via CAN
				int iSampleID = (IMEU_TYPE_IMEU3 == (IMEU_TYPE)pVarVal->emValue ? 5 : 4);
				SAMPLER* pSampler = GetSamplerById(iSampleID);
				DEVICE* pDev = GetDevce_ById(nDevId);
				pDev->info.pBelonged_Sampler = pSampler;
				
			
				return ERR_CONTROL_OK;
			}
/*			if (IMEU_TYPE_IMEU2 == pSTBatt_Rough->emIMEUType[nWhichIMEU])
			{
				nRet = IMEU2SendSetCAN(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
			}
			else 
			*/
			if (IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[nWhichIMEU])
			{
				nRet = IMEU3SendSetCAN(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
			}
			else
			{
				return ERR_CONTROL_FAILED;
			}

			return nRet;
		}
			//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	}
	else if(nDevId >= 611 && nDevId <= 614)
	{
		if (PARAM_TYPE_CONTROL == emParaType)
		{
			nRet = IMEU3SendCtlCAN(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
		}	
		else if(PARAM_TYPE_SETTING == emParaType)
		{
			int nWhichPowerIMEU = nDevId - 611;
			if(451 == nParamId)   //电源柜IMEU3和IMEU2配置选择
			{
				pSTBatt_Rough->emPOWER_IMEUType[nWhichPowerIMEU] = (IMEU_TYPE)pVarVal->emValue;
				return ERR_CONTROL_OK;
			}
			else if(452 == nParamId)
			{
				pSTBatt_Rough->QF3_Enable = pVarVal->emValue;
				return ERR_CONTROL_OK;
			}
		}	
		else
		{
			return ERR_CONTROL_FAILED;
		}
		return nRet;
	}
	else if(nDevId >= 951 && nDevId <= 954)
	{
		if(PARAM_TYPE_SETTING == emParaType)
		{
			int nAir = nDevId - 951;
			if(448 == nParamId)
			{
				pSTBatt_Rough->emAirType[nAir] = (AIR_TYPE)pVarVal->emValue;
			}
			else if(446 == nParamId)
			{
				pSTBatt_Rough->f35KW_OpenTemp = pVarVal->fValue;
			}
			else if(447 == nParamId)
			{
				pSTBatt_Rough->f35KW_ShutdownTemp = pVarVal->fValue;
			}
			else if(445 == nParamId)
			{
				BatSendSetCmd(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
			}
			else if(444 == nParamId)
			{
				BatSendSetCmd(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
			}
			//EU3 Air
			if (nParamId >= 529 && nParamId <= 544) {
				BatSendSetCmd(pSTBatt_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
				TRACED("nParamId:%d \n", nParamId);
			}
		}
	}

	//TRACE("STBATT Control: emParaType=%d nParamId=%d\n",emParaType, nParamId);
	if(emParaType == PARAM_TYPE_SETTING)
	{	
		if(nParamId == 502)
		{
			pSTBatt_Rough->nSystemCfgBattNum = pVarVal->emValue;
	
			pSTBatt_Rough->nSystemCfgBattNum = MAX(0, pSTBatt_Rough->nSystemCfgBattNum);

			TRACE("Setting:nSystemCfgBattNum =%d pVarVal->emValue=%d\n",\
				pSTBatt_Rough->nSystemCfgBattNum, pVarVal->emValue);
		}
		else if((nParamId >= 120 && nParamId <= 166) || (nParamId >= 620 && nParamId <= 853))
		{
						DATA_INFO data_info;
			if(!GetDataInfo(nParamId, &data_info))
			{
				printf("444");
			}
			int len = 2;
			if(data_info.datalen == 1)
			{
				len = 1;
			}

			BYTE tx_data[2];
			if(data_info.dataType == 1) //枚举
			{
				tx_data[0] = (BYTE)(pVarVal->emValue & 0xFF);  // 低字节：枚举值
    			tx_data[1] = 0x00;                             // 高字节：补0
			}
			else
			{
				uint16_t float_val = (uint16_t)pVarVal->fValue;
				// 小端序（和枚举保持一致，推荐）
				tx_data[0] = (BYTE)(float_val & 0xFF);        // 低8位
				tx_data[1] = (BYTE)((float_val >> 8) & 0xFF); // 高8位
			}

			AR_FRAME ar_Frame;
			memset(&ar_Frame, 0, sizeof(AR_FRAME));
			if(GetSetParameter(&ar_Frame, FUNCODE_REQ_MODIFY, nParamId, tx_data, len))
			{
				nRet = BMU02_SendFrameByArFrame(pSTBatt_Rough, &ar_Frame);
			}

			if(nRet == 0)
			{
				return ERR_CONTROL_OK;
			}
			else
			{
				return ERR_CONTROL_FAILED;
			}
		}
		else if(nParamId == 519)
		{	
			printf("nParamId 519  = [%d]\n", pVarVal->ulValue);

			DATA_INFO data_info;
			int len = 0;
			BYTE tx_data[2] = {0};
			AR_FRAME ar_Frame;
			memset(&ar_Frame, 0, sizeof(AR_FRAME));

			if(GetSetParameter(&ar_Frame, FUNCODE_REQ_QUERY, pVarVal->ulValue, tx_data, len))
			{
				BMU02_SendFrameByArFrame(pSTBatt_Rough, &ar_Frame);
			}

		}
		else if(nParamId == 503)
		{
			pSTBatt_Rough->nProtectSOC = pVarVal->ulValue;
			pSTBatt_Rough->nBackupProtectSOC = pVarVal->ulValue;
			TRACE("nProtectSOC =[%d]\n", pSTBatt_Rough->nProtectSOC);
		}
		else if(nParamId == 501)
		{
			pSTBatt_Rough->nPVSysBattRetSOC = pVarVal->ulValue;
			TRACE("nPVSysBattRetSOC=[%d]\n", pSTBatt_Rough->nPVSysBattRetSOC);
		}
		else if(nParamId == 508)
		{
			pSTBatt_Rough->fISO_PeriodMin_Sett = pVarVal->fValue;
			TRACE("fISO_PeriodMin_Sett=[%d]\n", pSTBatt_Rough->fISO_PeriodMin_Sett);
		}
		else if(nParamId == 509)
		{
			pSTBatt_Rough->emDisChgByStopSOC = pVarVal->emValue;
			TRACE("pSTBatt_Rough->emDisChgByStopSOC=[%d]\n", pSTBatt_Rough->emDisChgByStopSOC);
		}
		else if(nParamId == 510)
		{
			pSTBatt_Rough->emFullyChgDischgMode = pVarVal->emValue;
			TRACE("pSTBatt_Rough->emFullyChgDischgMode=[%d]\n", pSTBatt_Rough->emFullyChgDischgMode);
			// if(pSTBatt_Rough->emFullyChgDischgMode == TRUE && pSTBatt_Rough->emDisChgByStopSOC == FALSE)
			// if(pSTBatt_Rough->emFullyChgDischgMode == TRUE)
			// {	
			// 	pSTBatt_Rough->nBackupProtectSOC = pSTBatt_Rough->nProtectSOC;
			// 	pSTBatt_Rough->nProtectSOC = 0;
			// 	TRACE("Set Protect Soc to 0\n");
			// }
			// else
			// {
			// 	pSTBatt_Rough->nProtectSOC = pSTBatt_Rough->nBackupProtectSOC;
			// }
			
		}
		else if(nParamId == 202)
		{
			// printf("--------------before--------------\n");
			// float pwrlmtstep1 = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep1;
    		// float pwrlmtstep3 = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep3;
    		// float pwrlmtstep5 = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep5;
    		// float pwrlmtstep7 = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep7;
    		// printf("pwrlmtstep1:%f !!!\n", pwrlmtstep1);
    		// printf("pwrlmtstep3:%f !!!\n", pwrlmtstep3);
    		// printf("pwrlmtstep5:%f !!!\n", pwrlmtstep5);
    		// printf("pwrlmtstep7:%f !!!\n", pwrlmtstep7);
			// pSTBatt_Rough->bmsTestModeParm.autoTestParm[0] = pVarVal->blobValue;
			// pSTBatt_Rough->bmsTestModeParm.autoTestParm[0] = (BMS_TESTMODE_PARM)(pVarVal->blobValue);
			A_BLOB_BMS_TESTMODE_SET *pTestParam = (A_BLOB_BMS_TESTMODE_SET*)pVarVal->blobValue;
			// nBattIdx = nDevId - 500;
			// memcpy(&pSTBatt_Rough->bmsTestModeParm, pTestParam, sizeof(A_BLOB_BMS_TESTMODE_SET));
			// memcpy(&pSTBatt_Rough->bmsTestModeParm.autoTestParm[nBattIdx], &pTestParam->autoTestParm[nBattIdx], sizeof(BMS_TESTMODE_PARM));
			memcpy(&pSTBatt_Rough->bmsTestModeParm.autoTestParm, &pTestParam->autoTestParm, 4*sizeof(BMS_TESTMODE_PARM));
			// printf("--------------after--------------\n");
			// float pwrlmtstep1_a = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep1;
    		// float pwrlmtstep3_a = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep3;
    		// float pwrlmtstep5_a = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep5;
    		// float pwrlmtstep7_a = pSTBatt_Rough->bmsTestModeParm.autoTestParm[1].nPwrLmtStep7;
    		// printf("pwrlmtstep1:%f !!!\n", pwrlmtstep1_a);
    		// printf("pwrlmtstep3:%f !!!\n", pwrlmtstep3_a);
    		// printf("pwrlmtstep5:%f !!!\n", pwrlmtstep5_a);
    		// printf("pwrlmtstep7:%f !!!\n", pwrlmtstep7_a);
		}
		// else if (511 == nParamId)
		// {
		// 	pSTBatt_Rough->eBattCellTempPrecison = pVarVal->emValue;
		// 	TRACE("pSTBatt_Rough->eBattCellTempPrecison=[%d]\n", pSTBatt_Rough->eBattCellTempPrecison);
		// }
		else if(nParamId >= 8000 && nParamId <= 8007) // LJH240715 aircond
		{
			nBattIdx = nDevId- 501;
			int offset = nParamId - 8000;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum) || (offset < 0) || (offset > 7))
			{
				TRACEE("st_batt_sampler::Control nParamId:%d nBattIdx:%d offset:%d nSystemCfgBattNum:%d return ERR_CONTROL_FAILED",
					nParamId, nBattIdx, offset, pSTBatt_Rough->nSystemCfgBattNum);
				return ERR_CONTROL_FAILED;
			}
			pSTBatt_Rough->nSetTemp4Aircond     = (int)pVarVal->fValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_SETAIRCOND_BATT_COOLON+offset, nBattIdx) ? 
				ERR_CONTROL_OK : ERR_CONTROL_FAILED;
			TRACEI("st_batt_sampler::Control nParamId:%d nBattIdx:%d offset:%d nSystemCfgBattNum:%d nRet:%d",
				nParamId, nBattIdx, offset, pSTBatt_Rough->nSystemCfgBattNum, nRet);
		}
		else if(nParamId == 617) // Anti Over Charge
		{
			pSTBatt_Rough->bAntiOverChg = (BOOL)pVarVal->emValue;
		}
		else if(nParamId == 550)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			pSTBatt_Rough->emAirNoise[nBattIdx] = pVarVal->emValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_EU3_AIRNOISE, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 518)
		{	
			int n = 0;
			nRet = ERR_CONTROL_OK;

			for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
			{
				pSTBatt_Rough->bKeepPowerMode[n] = (BOOL)pVarVal->emValue;
				int tempRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_KEEP_POWER_MODE, n) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;

				if(tempRet != ERR_CONTROL_OK)
				{
					TRACEE("Set battery group %d keep power mode failed", n);
					nRet = ERR_CONTROL_FAILED;
				}
			}
		}
	}
	else
	{
		if(nParamId == 500)
		{
			TRACE("Info: Enter BMS Upgrading...:emUpdownStatus=[%d]\n", pSTBatt_Rough->stUpgradeParam.emUpgradeStatus);
			//now notice sampler to goto the update process
			/*if(pVarVal->emValue == 0)
			{
				pSTBatt_Rough->stUpgradeParam.tm_StartUpgrade = _NOW_;
				pSTBatt_Rough->stUpgradeParam.emUpgradeStatus = ( pSTBatt_Rough->stUpgradeParam.emUpgradeStatus >= UPGRADE_READBINFILE) ? 
					pSTBatt_Rough->stUpgradeParam.emUpgradeStatus : UPGRADE_READBINFILE;   	
			}
			else
			{
				pSTBatt_Rough->stUpgradeParam.emUpgradeStatus = UPGRADE_FINISHED;
			}*/

			if (pSTBatt_Rough->stUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
			{
				LogOut4c(MAIN_MODULE_SAMPLER, "BAT-Updt", LOG_TYPE_INFO, "emUpgradeStatus != UPGRADE_IDLE, upgrade not restart");
				return ERR_CONTROL_OK;
			}
				

			TRACE("---BMS Upgrade A_BLOB_UPDT_INFO_4CCU---\n");
			if(nValSize != sizeof(A_BLOB_UPDT_INFO_4CCU))
			{
				TRACE("----Size mismatch for BMS-updt-ctrl[%d,%d]----\n", nValSize, sizeof(A_BLOB_UPDT_INFO_4CCU));
				return ERR_CONTROL_FAILED;
			}
			
			A_BLOB_UPDT_INFO_4CCU *pUpdtRectCmd = (A_BLOB_UPDT_INFO_4CCU*)pVarVal->blobValue;
			memcpy(&pSTBatt_Rough->blobUpdCmd, pUpdtRectCmd, sizeof(A_BLOB_UPDT_INFO_4CCU));

			if(pSTBatt_Rough->blobUpdCmd.bIs4Web)
			{
				printf("!!! blobUpdCmd.bIs4Web !!!\n");
				BOOL bRet = Prepare_BMSBin_Updte(pSTBatt_Rough, 2000);
				if(bRet == FALSE)
				{
					TRACE("----Prepare_CCUBin_Updte Failed!!!---\n");
 					return ERR_CONTROL_FAILED;
				}
			}
			
			//now notice sampler to goto the update process
			pSTBatt_Rough->stUpgradeParam.tm_StartUpgrade = _NOW_;
			int nProcess = 253;
			VAR_VALUE varV;
			varV.lValue = nProcess;
			Push_VarValue(SETTYPE_LOGIC_SP, pSTBatt_Rough->hSampler, 200, 82, varV);
			pSTBatt_Rough->stUpgradeParam.emUpgradeStatus = (pSTBatt_Rough->stUpgradeParam.emUpgradeStatus >= UPGRADE_READBINFILE) ? 
				pSTBatt_Rough->stUpgradeParam.emUpgradeStatus : UPGRADE_READBINFILE;
		}
		else if(nParamId == 501)
		{
			nBattIdx = nDevId- 501;
			TRACE("------------Received cmd Sleep/Awake for Battery[%d] bEnterSleepMode=[%d]---------\n", nBattIdx,pVarVal->emValue);	
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, (pVarVal->emValue == 1) ? CTRL_STBATT_GO_SLEEP : CTRL_STBATT_WAKE_UP, nBattIdx) ? 
				ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 502)
		{
			nBattIdx = nDevId- 501;
			TRACE("------------Received cmd Masking Error for Battery[%d]---------\n", nBattIdx);
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_MASK_ERR, nBattIdx) ? 
				ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 503)
		{
			nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			if((pVarVal->emValue != TP_TRIG_SUSPEND) &&
				(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] != TP_TRIG_SUSPEND))
			{
				TRACE("--- Trigger Some Msg Type nBattIdx=[%d]->emValue=[%d] emTrigMsgTp=[%d]\n", \
					nBattIdx, pVarVal->emValue, pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx]);
				return ERR_CONTROL_FAILED;
			}
			
			pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] = (TP_TRIGGER_MSG)pVarVal->emValue;
		}
		else if(nParamId == 504)
		{
			//if() ??? Need Process Later
			//pSTBatt_Rough->stuTestMode.emTrigState[nBattIdx] = (STATE_MACHINE_STAT_TEST)pVarVal->emValue;
			//TRACE("nBattIdx=[%d] emValue=[%d]\n", nBattIdx, pVarVal->emValue);
			//if() ??? Need Process Later
			// nBattIdx = nDevId- 501;
			// printf("nBattIdx: %d\n", nBattIdx);
			// pSTBatt_Rough->stuTestMode.emTrigState[nBattIdx] = (STATE_MACHINE_STAT_TEST)pVarVal->emValue;
			// printf("nBattIdx=[%d] emValue=[%d]\n", nBattIdx, pVarVal->emValue);
			// if(pVarVal->emValue==10)//暂停退出测试模式
			// {
			// 	pSTBatt_Rough->emPMCtrlMode=0;//退出TestMode
			// 	TRACE("504, Auto emPMCtrlMode: [%d]\n", pSTBatt_Rough->emPMCtrlMode);
			// }
			// else
			// {
			// 	pSTBatt_Rough->emPMCtrlMode=2;//测试进入TestMode
			// 	TRACE("504, Test emPMCtrlMode: [%d]\n", pSTBatt_Rough->emPMCtrlMode);
			// }
			nBattIdx = nDevId - 501;	
			pSTBatt_Rough->bmsAutoTestResult[nBattIdx].testState = pVarVal->emValue;
			// printf("after 504 setting, n: %d, testState: %d\n", nBattIdx, pSTBatt_Rough->bmsAutoTestResult[nBattIdx].testState);
			// pSTBatt_Rough->bmsAutoTestResult[1].testState = pVarVal->emValue;
			// pSTBatt_Rough->bmsAutoTestResult[2].testState = pVarVal->emValue;
			// pSTBatt_Rough->bmsAutoTestResult[3].testState = pVarVal->emValue;
		}
		else if(nParamId == 505)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}
			//pSTBatt_Rough->bEnterReboot[nBattIdx] = 
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_REBOOT, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 506)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			if((pVarVal->emValue < 0) || (pVarVal->emValue > 1))
			{
				return ERR_CONTROL_FAILED;
			}

			pSTBatt_Rough->emEnterSuperuser[nBattIdx] = pVarVal->emValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_SUPER_USER, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 507)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			if((pVarVal->emValue < 0) || (pVarVal->emValue > 1))
			{
				return ERR_CONTROL_FAILED;
			}

			pSTBatt_Rough->emClearFaultType[nBattIdx] = pVarVal->emValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_CLEAR_FAULT, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 545)
		{
			/* PACK number 1~5 from web input, CAN byData[3] = PACK, byData[2] = 0x01(reset) */
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}
			int nPackNum = (int)pVarVal->ulValue;
			if((nPackNum < 0) || (nPackNum >= 5))
			{
				return ERR_CONTROL_FAILED;
			}
			pSTBatt_Rough->Afe_Reset_PackNum[nBattIdx] = nPackNum;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_AFE_RESET, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 546)
		{
			/* PACK number 1~5 from web input, CAN byData[3] = PACK, byData[2] = 0x01(reset) */
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}
			int nPackNum = (int)pVarVal->ulValue;
			if((nPackNum < 0) || (nPackNum >= 5))
			{
				return ERR_CONTROL_FAILED;
			}
			pSTBatt_Rough->Bus_Reset_PackNum[nBattIdx] = nPackNum;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_BSU_RESET, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 508)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			pSTBatt_Rough->emWorkMode[nBattIdx] = pVarVal->emValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_WORK_MODE, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
		}
		else if(nParamId == 526)
		{
			int nBattIdx = nDevId- 501;
			if((nBattIdx < 0) || (nBattIdx >= pSTBatt_Rough->nSystemCfgBattNum))
			{
				return ERR_CONTROL_FAILED;
			}

			pSTBatt_Rough->nRintFreqIdx[nBattIdx] = pVarVal->ulValue;
			nRet = Send_Ctrl_STBatt(pSTBatt_Rough, CTRL_STBATT_RINT_POINT, nBattIdx) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
			VAR_VALUE varV;
			varV.emValue = RUNNING;
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 501, 3904, varV);
			TRACED("-----------send success, flag: %d-------------\n", varV.emValue);
		}
		else if(nParamId == 499)
		{
			QueryBSUVersion(pSTBatt_Rough);
		}
		else if (527 == nParamId) // 501, 527,
		{
			pSTBatt_Rough->executeChargingWhenReflux = (BOOL)pVarVal->emValue;
		}
		else if(nParamId == 528)
		{
			pSTBatt_Rough->bNeedRecBmsMu2CanLog = (pVarVal->emValue == 0) ? TRUE : FALSE;
			if(pSTBatt_Rough->bNeedRecBmsMu2CanLog == TRUE)
			{
				pSTBatt_Rough->tm_fLastStartRecBmsMu2canlog = _NOW_;
			}
		}
		else if(nParamId >= 666 && nParamId <= 668)  // 条码查询 Cell/Pack/BSU
		{
			int nPackIdx, nCellIdx = 0;
			int queryType = nParamId - 666;
			
			if(queryType == 0)
			{
				int encoded = pVarVal->lValue;
				nPackIdx = encoded / 1000;
				nCellIdx = encoded % 1000;
			}
			else
			{
				nPackIdx = pVarVal->lValue;
			}
			
			YFY_C2S_Query_Barcode(pSTBatt_Rough, queryType, nPackIdx, nCellIdx);
		}
		else if(nParamId == 669)  // 写入条码
		{
			if(nValSize > 0)
			{
				char* pData = (char*)pVarVal->blobValue;
				char* token = strtok(pData, ":");
				if(token)
				{
					int nType = atoi(token);
					token = strtok(NULL, ":");
					if(token)
					{
						int nPackIdx = atoi(token);
						token = strtok(NULL, ":");
						if(token)
						{
							int nCellIdx = atoi(token);
							token = strtok(NULL, ":");
							if(token)
							{
								char* szBarcode = token;
								YFY_C2S_Write_Barcode(pSTBatt_Rough, nType, nPackIdx, nCellIdx, szBarcode);
							}
						}
					}
				}
			}
		}
		else if(nParamId == 604)//法国客户手动SOC校准
		{
			int nBattIdx = nDevId- 501;
			int nSOCCalibrationVlot = 0;
			if(pSTBatt_Rough->bOCVFlag == FALSE)
			{
				pSTBatt_Rough->bIsCalibrated = FALSE;
				return ERR_CONTROL_FAILED;
			}
			else
			{
				if(pVarVal->emValue == 1)
				{
					if(pSTBatt_Rough->emSOCCalibration == SOCChargingCalibration)
					{
						// 校准SOC充电状态
						nSOCCalibrationVlot = (int)(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMax_CellVolt * 1000);
						if(nSOCCalibrationVlot > 3295 && nSOCCalibrationVlot < 3339)
						{
							pSTBatt_Rough->bIsCalibrated = FALSE;
							return ERR_CONTROL_FAILED;
						}
						else
						{
							pSTBatt_Rough->nCalibratedSOC = MapVoltageToSOC(TRUE, nSOCCalibrationVlot);
							pSTBatt_Rough->bIsCalibrated = TRUE;
						}
					}
					else if(pSTBatt_Rough->emSOCCalibration == SOCDischargingCalibration)
					{
						// 校准SOC放电状态
						nSOCCalibrationVlot = (int)(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].fMin_CellVolt * 1000);
						if(nSOCCalibrationVlot > 3283 && nSOCCalibrationVlot < 3332)
						{
							pSTBatt_Rough->bIsCalibrated = FALSE;
							return ERR_CONTROL_FAILED;
						}
						else
						{
							pSTBatt_Rough->nCalibratedSOC = MapVoltageToSOC(FALSE, nSOCCalibrationVlot);
							pSTBatt_Rough->bIsCalibrated = TRUE;
						}
					}
				}
				else
				{
					pSTBatt_Rough->bIsCalibrated = FALSE;
				}
			}
		}
 	}
	
	TRACE("emParaType =%d nDevId =%d nParamId=%d pVarVal=%d nRet=[%d]\n",\
		emParaType,nDevId,nParamId, pVarVal->emValue, nRet);
	
	return nRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Write_CAN1
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ssize_t Write_CAN1(IN STBATT_SAMPLER_ROUGH *pBattData, IN void * buf,IN size_t count)
{
	if (Mutex_Lock(pBattData->hCAN1Write_Mutex, 100) != ERR_MUTEX_OK)
	{
		return -1;
	}

	ssize_t nWriteSize = pBattData->pComm->Write(pBattData->hCanDevHanle, buf, count);

	Mutex_Unlock(pBattData->hCAN1Write_Mutex);
	
	//TRACE("nWriteSize = %d \n",nWriteSize);

	return nWriteSize;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CAN_Filter_Conf
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CAN_Filter_Conf(STBATT_SAMPLER_ROUGH *pBattData)
{
	#define _CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */

	COMM_CAN_FILTER rfilter;

	//Filter matching rule:<received_can_id> & mask == can_id & mask

	rfilter.ulCANId   = 0x00000000 | _CAN_EFF_FLAG; 
	rfilter.ulCANMask =  0xFFFF0000; // 0- represents the value of this bit is arbitrary, 1- represents that the bit must be consistent

	int nErrCode = pBattData->pComm->Control(pBattData->hCanDevHanle, COMM_SET_CAN_FILTER, &rfilter,sizeof(COMM_CAN_FILTER));

	TRACE("CAN Control nErrCode = %d sizeof(COMM_CAN_FILTER) =%d\n",nErrCode,sizeof(COMM_CAN_FILTER));

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CAN_ClearReadBuf
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CAN_ClearReadBuf(STBATT_SAMPLER_ROUGH *pBattData)
{
	int		iReadLen = 0;
	BYTE*	pbyRcvBuf = pBattData->abyRcvBuf;

	while((pBattData->pComm->Read(pBattData->hCanDevHanle, (void *)pbyRcvBuf, MAX_READ_BUFF_SIZE*CAN_FRAME_LEN) == MAX_READ_BUFF_SIZE*CAN_FRAME_LEN) && 
		(iReadLen < MAX_READ_BUFF_SIZE * CAN_FRAME_LEN))
	{
		TRACE("ClearReadBuff iReadLen=%d\n",iReadLen);
		iReadLen += CAN_FRAME_LEN;
		pbyRcvBuf += CAN_FRAME_LEN;
	}

	return;
}

