// #define _TRACE_PRINT_ // SEMSClient
#include "ess_sampler_main.h"

static void OnHandle_PowerCabLighting(ROUGH_CCU_DATA* pCCU)
{
	time_t tmNow = _NOW_;
	static BOOL bLast_Statu = FALSE;

	if(tmNow - pCCU->strLSYSInfo.tmLastSend_PwrCabLight <= 10)
	{
		return;
	}
	pCCU->strLSYSInfo.tmLastSend_PwrCabLight = tmNow;

	BOOL bRet =  (pCCU->strLSYSInfo.sysAlarmMap1.Alarm.DoorOpen == 1) ? TRUE : FALSE;

	if(bRet != bLast_Statu)
	{
		bLast_Statu = bRet;

		TRACEX("Ctrl: PwrCab Lighting bRet=[%d]\n", bRet);
		BYTE byData[3]={'\0'};
		byData[0] = 0x0C;
		byData[1] = (bRet == FALSE) ? 0xA0 : 0xA1; //0xA0:DisConnect
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 100);
	}
}

static BOOL Judge_BatCabFireProtAlm(ROUGH_CCU_DATA* pCCU)
{
	BOOL 		bRet = FALSE;
	int 		n = 0;
	VAR_VALUE 	varV;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + n, 739, &varV, 4);
		if(varV.emValue == 1)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}


static void OnHandle_BatCabFireProtAlm(ROUGH_CCU_DATA* pCCU)
{
	time_t tmNow = _NOW_;
	static BOOL bLast_Statu = FALSE;

	if(tmNow - pCCU->strLSYSInfo.tmLastSend_CtrlVoiceAndLight <= 10)
	{
		return;
	}
	pCCU->strLSYSInfo.tmLastSend_CtrlVoiceAndLight = tmNow;

	BOOL bRet = Judge_BatCabFireProtAlm(pCCU);

	if(bRet != bLast_Statu)
	{
		bLast_Statu = bRet;

		TRACEX("Ctrl: Voice and Visual Lighting bRet=[%d]\n", bRet);
		BYTE byData[3]={'\0'};
		byData[0] = 0x0B;
		byData[1] = (bRet == FALSE) ? 0xA0 : 0xA1; //0xA0:DisConnect
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 100);
	}

	return;
}


static void OnHandle_STS_OnOffStatu(ROUGH_CCU_DATA* pCCU)
{
	time_t tmNow = _NOW_;

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed == 0) //0-Disable
	{
		return;
	}

	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.STS1_CommFail == 1)//1-CommFail
	{
		return;
	}

	if(pCCU->strLSYSInfo.mapSTSCode.Status.Power_OnOff == 1) //1:Power-ON
	{
		return;
	}

	if(pCCU->strLSYSInfo.mapSTSCode.Status.FaultState == 1) //1:Fault.
	{
		return;
	}

	//John 20230910.All Mode Send PowerOn Cmd To STS.
	//if(pCCU->emPMCtrlMode != AUTO_CTRL_PM_MODE)
	//{
	//	return;
	//}

	if(tmNow - pCCU->strLSYSInfo.tmLastSend_PwrOnSTS <= MAX_ABNM_TIMES)
	{
		return;
	}

	pCCU->strLSYSInfo.tmLastSend_PwrOnSTS = tmNow;

	TRACE("Auto Ctrl STS: ON \n");
 	BYTE byData[3]={'\0'};
	byData[0] = 0x03;
	byData[1] = 0xA0 ;
	DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 100); 

	return;

}

//-------------------- 2023-12-24 John Added For On-Off Grid ------------------//
 
static void SendCmd2ChangeCCURunMode(IN ROUGH_CCU_DATA* pCCU, STAT_PM_CTRL_MODE emPCSRun)
{
	VAR_VALUE varV;
	varV.emValue = emPCSRun;
	Push_VarValue(SETTYPE_INTERN_SETT, pCCU->hSampler, 200, 217, varV);

	TRACE("SendCmd2ChangeCCURunMode: Enter Debug Mode emValue =%d\n",varV.emValue);

 	BYTE byData[5]={'\0'};
	byData[0] = 0x17;
	byData[1] = (varV.emValue == AUTO_CTRL_PM_MODE) ? 0xA0 : 0xA1;
	DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
 	pCCU->emPMCtrlMode = (STAT_PM_CTRL_MODE)varV.emValue;

	return;
}

static void SendCmd2AllPCS_ONorOFF(IN ROUGH_CCU_DATA* pCCU, PCS_RUN_STATU emPcsRun)
{
	BYTE byData[16];
	BYTE byCtrlCmd = (emPcsRun == STAT_PCS_OFF) ? 0 : 0x03; //0x03:Start DisChg.

	int nSetGrpVolt = 750;
	int nSetGrpCurr = 0; //To Ctrl PCS Handby

	byData[0] = 0x14;
	byData[1] = byCtrlCmd; //0-DC Adjust
	byData[2] = 0; //All Group
	byData[3] = (BYTE)(nSetGrpVolt >> 8);
	byData[4] = (BYTE)(nSetGrpVolt);
	byData[5] = (BYTE)(nSetGrpCurr >> 8);
	byData[6] = (BYTE)nSetGrpCurr;

	printf("Data[0]=%02X Data[1]=%02X Data[2]=%02X Data[3]=%02X Data[4]=%02X Data[5]=%02X Data[6]=%02X  Data[7]=%02X Data[8]=%02X Data[9]=%02X Data[10]=%02X Data[11]=%02X\n",\
			byData[0],byData[1],byData[2],byData[3],byData[4],byData[5],byData[6],byData[7],byData[8],byData[9],byData[10],byData[11]);
	DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 500) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;;			
		
	return;
}

static void SendCmd2ABU(IN ROUGH_CCU_DATA* pCCU,  ABU_ONOFFGIRD_STATU emABURun)
{
	TRACE("Manual Ctrl: ON/Off-Grid Mode Ctrl ABU=%d\n", emABURun);
 	if((emABURun == STAT_ABU_OFFGRID) //Not Allow ABU On2Off When AC Volt is Normal. 
		&& (pCCU->esRunData.fACVolt_L1 > 150)
		&& (pCCU->esRunData.fSTS_InputVolt[0] > 150))
		{
			TRACE("Warning: emABURun=[%d]fACVolt_L1=[%f]fSTS_InputVolt=[%f]\n", emABURun ,pCCU->esRunData.fACVolt_L1, pCCU->esRunData.fSTS_InputVolt[0]);
			return;
		}

	BYTE byData[3]={'\0'};
	byData[0] = 0x02;
	byData[1] = (emABURun == 0) ? 0xA1 : 0xA2;
	DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 

	return;
}

static BOOL Judge_IfAllPCSStatus_OFF(IN ROUGH_CCU_DATA* pCCU)
{
	BOOL bRet = TRUE;
	UINT n = 0; 

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pCCU->strLSYSInfo.branchNodeData[n].emACDC_WorkStatu != STATU_ACDC_IDLE)
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

static void On_HandleHMI_On2OffGridEvent(IN ROUGH_CCU_DATA* pCCU)
{
	HMI_ON2OFFGRID_PARAM* pHMIOn2Off = &pCCU->tagOn2OffParam; 
	time_t 	tmNow = _NOW_;
	UINT 	n = 0;
	
	double tm = GetCurrentTime();

	if(pCCU->pNewSet.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed == 0) //0-Disable, 1-Enable
	{
		return;
	}

	if(pHMIOn2Off->emHMITrig == STAT_TRIG_IDLE)//Shouid be Initial Idle at Func Initail()
	{
		return;
	}

	if((tmNow - pHMIOn2Off->tmLastHMITrig) > 10) //Over 10S Exit this Event.
	{
		pHMIOn2Off->emHMITrig = STAT_TRIG_IDLE;
		return;
	} 

	TRACE("tm=[%f] emHMITrig=[%d] emRunningMode=[%d] OnOffGrid_State=[%d]\n", \
		tm,\
		pHMIOn2Off->emHMITrig,\
		pCCU->strLSYSInfo.emRunningMode,\
		pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);

	switch(pHMIOn2Off->emHMITrig)
	{
		case STAT_TRIGABU_ON2OFF: 
		{
			if(pCCU->strLSYSInfo.emRunningMode == AUTO_CTRL_PM_MODE) // 0:AutoMode
			{
				SendCmd2ChangeCCURunMode(pCCU, MANUAL_CTRL_PM_MODE); //Wait 1000ms For -D Reply

				TRACE("tm=[%f]:SendCmd2ChangeCCURunMode:MANUAL_CTRL_PM_MODE\n", tm);
				break;
			}

			if(!Judge_IfAllPCSStatus_OFF(pCCU))
			{
				SendCmd2AllPCS_ONorOFF(pCCU, STAT_PCS_OFF);
				TRACE("tm=[%f]: SendCmd2AllPCS_ONorOFF:STAT_PCS_OFF\n", tm);
				break;
			}

			if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_ONGRID) //0-On-Grid, 1-Off-Grid
			{
				SendCmd2ABU(pCCU, STAT_ABU_OFFGRID);
				TRACE("tm=[%f]:SendCmd2ABU:STAT_ABU_OFFGRID\n", tm);
				break;
			}
			else
			{
				LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_WARING, "Click ON2OffGrid:RunMode=[%d]ABUStat[%d]\n", 
					pCCU->strLSYSInfo.emRunningMode, pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);
				pHMIOn2Off->emHMITrig = STAT_TRIG_IDLE;
				TRACE("tm=[%f]: ON2OffGrid Finished\n", tm);
			}
		} 
		break;
		
		case STAT_TRIG_PCSON:
		{
			//When User Grid QF is Not Disconnected, Forbid PCS ON.
			if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.LowVoltMCBBreaker == CONNECTED) //0-Connected
			{
				break;
			}

			if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_OFFGRID) //0-On-Grid, 1-Off-Grid
			{
				for(n = 0; n < MAX_SENDCMD2CCU_TIMES; n++)
				{
					SendCmd2AllPCS_ONorOFF(pCCU, STAT_PCS_ON); //Wait 100ms For -D Reply
					TRACE("tm=[%f]: SendCmd2AllPCS_ONorOFF:STAT_PCS_ON\n", tm);
				}
				pHMIOn2Off->emHMITrig = STAT_TRIG_IDLE;
			}
		} 
		break;
		
		case STAT_TRIG_PCSOFF:
		{
			if(pCCU->strLSYSInfo.emRunningMode == AUTO_CTRL_PM_MODE) // 0:AutoMode
			{
				SendCmd2ChangeCCURunMode(pCCU, MANUAL_CTRL_PM_MODE); //Wait 1000ms For -D Reply
				TRACE("tm=[%f]: SendCmd2ChangeCCURunMode:MANUAL_CTRL_PM_MODE\n", tm);
				break;
			}

			if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_OFFGRID) //0-On-Grid, 1-Off-Grid
			{
				for(n = 0; n < MAX_SENDCMD2CCU_TIMES; n++)
				{
					SendCmd2AllPCS_ONorOFF(pCCU, STAT_PCS_OFF); //Wait 100ms For -D Reply
					TRACE("tm=[%f]: SendCmd2AllPCS_ONorOFF:STAT_PCS_OFF\n", tm);
				}
				pHMIOn2Off->emHMITrig = STAT_TRIG_IDLE;
			}
		}
 		break;
		
		case  STAT_TRIGABU_OFF2ON:
		{
			if(!Judge_IfAllPCSStatus_OFF(pCCU))
			{
				SendCmd2AllPCS_ONorOFF(pCCU, STAT_PCS_OFF); //Wait 100ms For -D Reply
				TRACE("tm=[%f]: SendCmd2AllPCS_ONorOFF:STAT_PCS_OFF\n", tm);
 				break;
			}

			//When Grid is Error, ABU Always Send Off-Grid Status, Only Grid is normal, ABU will Auto Exchange To On-Grid.
			//if(pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State == STAT_ABU_OFFGRID) //0-On-Grid, 1-Off-Grid
			for(n = 0; n < MAX_SENDCMD2CCU_TIMES; n++)
			{
				SendCmd2ABU(pCCU, STAT_ABU_ONGRID);
				TRACE("tm=[%f]: SendCmd2ABU:STAT_ABU_ONGRID\n", tm);
			}

			if(pCCU->strLSYSInfo.emRunningMode == MANUAL_CTRL_PM_MODE) // 1:ManualMode
			{
				SendCmd2ChangeCCURunMode(pCCU, AUTO_CTRL_PM_MODE); //Wait 1000ms For -D Reply
				TRACE("tm=[%f]: SendCmd2ChangeCCURunMode:AUTO_CTRL_PM_MODE\n", tm);
 			}
			else
			{
				LogOut(MAIN_MODULE_SAMPLER, "CCU_SAMP", LOG_TYPE_WARING, "Click Off2OnGrid:RunMode=[%d]ABUStat[%d]\n", 
					pCCU->strLSYSInfo.emRunningMode, pCCU->strLSYSInfo.mapSTSCode.Status.OnOffGrid_State);
				pHMIOn2Off->emHMITrig = STAT_TRIG_IDLE;
				TRACE("tm=[%f]: Off2On:Finished\n", tm);
			}
		}	 
		break;
		
		default:
			break;
	}
	
	return ;
}
//-------------------- End ------------------//


static void OnHandle_TrigCANMsg(IN ROUGH_CCU_DATA* pCCU)
{
	time_t tmNow = _NOW_;
 
	if((tmNow - pCCU->tagPM2CCU_GCan.tm_LastStatTrig > 120) &&
		(pCCU->tagPM2CCU_GCan.emTrigMsgTp != PM2CCU_TRIG_OFF))
	{
		printf("OnHandle_CtrlCmd4CANMsgTrig emTrigMsgTp=[%d] \n", pCCU->tagPM2CCU_GCan.emTrigMsgTp);
		
		pCCU->tagPM2CCU_GCan.tm_LastStatTrig = tmNow;

		pCCU->tagPM2CCU_GCan.emTrigMsgTp = PM2CCU_TRIG_OFF;

		OnHandle_CtrlCmd4CANMsgTrig(pCCU, PM2CCU_TRIG_OFF);
	}
	return;
}


static void Onhandle_GrpInputContactor1_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 308, &varV, 4);

	if(pCCU->emGrpContactor1_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emGrpContactor1_CtrlVal)
	{
		if(pCCU->nGrp1_StickingAlarmTimes < 3)
		{
			pCCU->nGrp1_StickingAlarmTimes += 1;
		}
		else if(pCCU->nGrp1_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;

			if(pCCU->pNewSet.ccuSettings.sysAtributesMap3.Set.BattGrp1_InputContactor == 1)
			{
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1962, varV_alarm);
			}
			else
			{
				varV_alarm.emValue = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1962, varV_alarm);
				return;
			}
			

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x01;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nGrp1_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nGrp1_StickingAlarmTimes > 0)
		{
			pCCU->nGrp1_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1962, varV_alarm);
		}
	}

	return;
}


static void Onhandle_GrpInputContactor2_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 309, &varV, 4);

	if(pCCU->emGrpContactor2_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emGrpContactor2_CtrlVal)
	{
		if(pCCU->nGrp2_StickingAlarmTimes < 3)
		{
			pCCU->nGrp2_StickingAlarmTimes += 1;
		}
		else if(pCCU->nGrp2_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			if(pCCU->pNewSet.ccuSettings.sysAtributesMap3.Set.BattGrp2_InputContactor == 1)
			{
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1963, varV_alarm);
			}
			else
			{
				varV_alarm.emValue = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1963, varV_alarm);
				return;
			}

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x02;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nGrp2_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nGrp2_StickingAlarmTimes > 0)
		{
			pCCU->nGrp2_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1963, varV_alarm);
		}
	}

	return;
}


static void Onhandle_GrpInputContactor3_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 310, &varV, 4);

	if(pCCU->emGrpContactor3_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emGrpContactor3_CtrlVal)
	{
		if(pCCU->nGrp3_StickingAlarmTimes < 3)
		{
			pCCU->nGrp3_StickingAlarmTimes += 1;
		}
		else if(pCCU->nGrp3_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			if(pCCU->pNewSet.ccuSettings.sysAtributesMap3.Set.BattGrp3_InputContactor == 1)
			{
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1964, varV_alarm);
			}
			else
			{
				varV_alarm.emValue = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1964, varV_alarm);
				return;
			}

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x03;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nGrp3_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nGrp3_StickingAlarmTimes > 0)
		{
			pCCU->nGrp3_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1964, varV_alarm);
		}
	}

	return;
}


static void Onhandle_GrpInputContactor4_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 311, &varV, 4);

	if(pCCU->emGrpContactor4_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emGrpContactor4_CtrlVal)
	{
		if(pCCU->nGrp4_StickingAlarmTimes < 30)
		{
			pCCU->nGrp4_StickingAlarmTimes += 1;
		}
		else if(pCCU->nGrp4_StickingAlarmTimes == 30)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			if(pCCU->pNewSet.ccuSettings.sysAtributesMap3.Set.BattGrp3_InputContactor == 1)
			{
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1965, varV_alarm);
			}
			else
			{
				varV_alarm.emValue = 0;
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1965, varV_alarm);
				return;
			}

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x04;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nGrp4_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nGrp4_StickingAlarmTimes > 0)
		{
			pCCU->nGrp4_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1965, varV_alarm);
		}
	}

	return;
}


static void Onhandle_PCS_RelayAlarm(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV1;
	int n;
	int m;
	int nFlag;

	if(pCCU->emIsGrpContactor1_DisconnNeeded == 0)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 251, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 252, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag > 0)
		{
			pCCU->emIsGrpContactor1_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsGrpContactor2_DisconnNeeded == 0)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 256, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 257, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag > 0)
		{
			pCCU->emIsGrpContactor2_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsGrpContactor3_DisconnNeeded == 0)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 261, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 262, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag > 0)
		{
			pCCU->emIsGrpContactor3_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsGrpContactor4_DisconnNeeded == 0)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 266, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 267, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag > 0)
		{
			pCCU->emIsGrpContactor4_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsGrpContactor1_DisconnNeeded == 2)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 251, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 252, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag == 0)
		{
			pCCU->emIsGrpContactor1_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsGrpContactor2_DisconnNeeded == 2)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 256, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 257, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag == 0)
		{
			pCCU->emIsGrpContactor2_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsGrpContactor3_DisconnNeeded == 2)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 261, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 262, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag == 0)
		{
			pCCU->emIsGrpContactor3_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsGrpContactor4_DisconnNeeded == 2)
	{
		nFlag = 0;
		for(n = 0; n < 4; n++)
		{
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 266, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 267, 1275 + n, &varV, 4);
			nFlag += varV.emValue;
		}
		if(nFlag == 0)
		{
			pCCU->emIsGrpContactor4_DisconnNeeded = 0;
		}
	}
	
	if(pCCU->emIsGrpContactor1_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x20;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----Grp Input Contactor 1 off!!!!!-----");
		pCCU->emGrpContactor1_CtrlVal = 1;
		pCCU->emIsGrpContactor1_DisconnNeeded = 2;
	}

	if(pCCU->emIsGrpContactor2_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x21;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----Grp Input Contactor 2 off!!!!!-----");
		pCCU->emGrpContactor2_CtrlVal = 1;
		pCCU->emIsGrpContactor2_DisconnNeeded = 2;
	}

	if(pCCU->emIsGrpContactor3_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x22;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----Grp Input Contactor 3 off!!!!!-----");
		pCCU->emGrpContactor3_CtrlVal = 1;
		pCCU->emIsGrpContactor3_DisconnNeeded = 2;
	}

	if(pCCU->emIsGrpContactor4_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x23;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----Grp Input Contactor 4 off!!!!!-----");
		pCCU->emGrpContactor4_CtrlVal = 1;
		pCCU->emIsGrpContactor4_DisconnNeeded = 2;
	}

	return;
}


static void Onhandle_PVContactor1_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 336, &varV, 4);

	if(pCCU->emPVContactor1_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emPVContactor1_CtrlVal)
	{
		if(pCCU->nPV1_StickingAlarmTimes < 3)
		{
			pCCU->nPV1_StickingAlarmTimes += 1;
		}
		else if(pCCU->nPV1_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1958, varV_alarm);

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x05;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nPV1_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nPV1_StickingAlarmTimes > 0)
		{
			pCCU->nPV1_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1958, varV_alarm);
		}
	}

	return;
}


static void Onhandle_PVContactor2_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 337, &varV, 4);

	if(pCCU->emPVContactor2_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emPVContactor2_CtrlVal)
	{
		if(pCCU->nPV2_StickingAlarmTimes < 3)
		{
			pCCU->nPV2_StickingAlarmTimes += 1;
		}
		else if(pCCU->nPV2_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1959, varV_alarm);

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x06;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nPV2_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nPV2_StickingAlarmTimes > 0)
		{
			pCCU->nPV2_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1959, varV_alarm);
		}
	}

	return;
}


static void Onhandle_PVContactor3_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 338, &varV, 4);

	if(pCCU->emPVContactor3_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emPVContactor3_CtrlVal)
	{
		if(pCCU->nPV3_StickingAlarmTimes < 3)
		{
			pCCU->nPV3_StickingAlarmTimes += 1;
		}
		else if(pCCU->nPV3_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1960, varV_alarm);

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x07;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nPV3_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nPV3_StickingAlarmTimes > 0)
		{
			pCCU->nPV3_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1960, varV_alarm);
		}
	}

	return;
}


static void Onhandle_PVContactor4_CtrlAndExcuteStatus(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	VAR_VALUE varV_alarm;

	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 339, &varV, 4);

	if(pCCU->emPVContactor4_CtrlVal == 2){return;}

	else if(varV.emValue != pCCU->emPVContactor4_CtrlVal)
	{
		if(pCCU->nPV4_StickingAlarmTimes < 3)
		{
			pCCU->nPV4_StickingAlarmTimes += 1;
		}
		else if(pCCU->nPV4_StickingAlarmTimes == 3)
		{
			BYTE byData[16];

			varV_alarm.emValue = 1;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1961, varV_alarm);

			byData[0] = 0x14;
			byData[1] = 0x00;
			byData[2] = 0x08;
			byData[3] = 0x00;
			byData[4] = 0x00;
		 	byData[5] = 0x00;
			byData[6] = 0x00;

			DoTCPSession(pCCU, NL_MSG_UCTL, byData, 7, 1000);
			
			pCCU->nPV4_StickingAlarmTimes += 1;
		}
	}
	else
	{
		if(pCCU->nPV4_StickingAlarmTimes > 0)
		{
			pCCU->nPV4_StickingAlarmTimes = 0;
			varV_alarm.emValue = 0;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1961, varV_alarm);
		}
	}

	return;
}


static void Onhandle_MPPT_SCAlarm(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;

	if(pCCU->emIsPVContactor1_DisconnNeeded == 0)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 296, &varV, 4);
		if(varV.emValue == 1)
		{
			pCCU->emIsPVContactor1_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsPVContactor2_DisconnNeeded == 0)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 297, &varV, 4);
		if(varV.emValue == 1)
		{
			pCCU->emIsPVContactor2_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsPVContactor3_DisconnNeeded == 0)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 298, &varV, 4);
		if(varV.emValue == 1)
		{
			pCCU->emIsPVContactor3_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsPVContactor4_DisconnNeeded == 0)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 299, &varV, 4);
		if(varV.emValue == 1)
		{
			pCCU->emIsPVContactor4_DisconnNeeded = 1;
		}
	}

	if(pCCU->emIsPVContactor1_DisconnNeeded == 2)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 296, &varV, 4);
		if(varV.emValue == 0)
		{
			pCCU->emIsPVContactor1_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsPVContactor2_DisconnNeeded == 2)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 297, &varV, 4);
		if(varV.emValue == 0)
		{
			pCCU->emIsPVContactor2_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsPVContactor3_DisconnNeeded == 2)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 298, &varV, 4);
		if(varV.emValue == 0)
		{
			pCCU->emIsPVContactor3_DisconnNeeded = 0;
		}
	}

	if(pCCU->emIsPVContactor4_DisconnNeeded == 2)
	{
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 200, 299, &varV, 4);
		if(varV.emValue == 0)
		{
			pCCU->emIsPVContactor4_DisconnNeeded = 0;
		}
	}
	
	if(pCCU->emIsPVContactor1_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x24;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----PV Contactor 1 off!!!!!-----");
		pCCU->emPVContactor1_CtrlVal = 1;
		pCCU->emIsPVContactor1_DisconnNeeded = 2;
	}

	if(pCCU->emIsPVContactor2_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x25;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----PV Contactor 2 off!!!!!-----");
		pCCU->emPVContactor2_CtrlVal = 1;
		pCCU->emIsPVContactor2_DisconnNeeded = 2;
	}

	if(pCCU->emIsPVContactor3_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x26;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----PV Contactor 3 off!!!!!-----");
		pCCU->emPVContactor3_CtrlVal = 1;
		pCCU->emIsPVContactor3_DisconnNeeded = 2;
	}

	if(pCCU->emIsPVContactor4_DisconnNeeded == 1)
	{
		BYTE byData[5]={'\0'};
		byData[0] = 0x27;
		byData[1] = 0xA0;
		DoTCPSession(pCCU, NL_MSG_UCTL, byData, 2, 500); 
		printf("----PV Contactor 4 off!!!!!-----");
		pCCU->emPVContactor4_CtrlVal = 1;
		pCCU->emIsPVContactor4_DisconnNeeded = 2;
	}

	return;
}


static void OverloadAlarm(IN ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	if(pCCU->esRunData.f_PcabMeter < (pCCU->fTotalPowerLimit*1.1))
	{
		pCCU->tLoadLimitTime_over110 = _NOW_;
	}

	if(pCCU->esRunData.f_PcabMeter < (pCCU->fTotalPowerLimit*1.2))
	{
		pCCU->tLoadLimitTime_over120 = _NOW_;
	}
	int val_110 = 0;
	int val_120 = 0;
	if((_NOW_ - pCCU->tLoadLimitTime_over110) > 60*10)
	{
		val_110 = 1;
	}
	else
	{
		val_110 = 0;
	}
	if((_NOW_ - pCCU->tLoadLimitTime_over120) > 60)
	{
		val_120 = 1;
	}
	else
	{
		val_120 = 0;
	}
	varV.emValue = val_110 | val_120;

	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 2293, varV);
	return;
}

/*
信号来源解释：
一体机：来源于485电表Sampler
站级EMS-从设备：来源于slave接收到站级EMS-UDP下发指令
*/
void* _thread_ESS_thread_Process_Sem(void* pArgs)
{
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

	double	tmNow;
	ERRCDDE_DPR err;
	VAR_VALUE varV;
	float fLimitCurrent = 0.0;
	A_BLOB_GRP_RECT_AUTOCTRL_CMD  CCUCtrlCmd;
	A_BLOB_SINGLE_STBATT_RUNINFO  blobSingleBattData;

	// float f_PgridMeter = 0,f_PgridMeter_Pa = 0,f_PgridMeter_Pb = 0,f_PgridMeter_Pc = 0;

	if (pCCU->nEssSemCCUWith485Meter == -1)
	{
		pthread_exit(NULL);
		LogOut(MAIN_MODULE_REPORTER, "_thread_Process_SemFrom485", LOG_TYPE_ERROR, 
			"SEMID FAILED!!!");
	}

	while (!pSampler->bExit)
	{
		//Sleep_Ex(200); 
		// printf("_thread_Process_SemFrom485 waiting sem...\n");
        sem_wait_op(pCCU->nEssSemCCUWith485Meter);   //block wait

		TRACEI("_thread_Process_SemFrom485 it is EMS or slave ~~\n");
		// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
		// 	"it is EMS or client send to IMSU-D !");
		if (pCCU->emPMCtrlMode != AUTO_CTRL_PM_MODE)	//Not executed in non automatic mode
		{
			continue;
		}
		pCCU->bQuickChannle = TRUE;
		Ems_Mgmt_Main_ESS(pCCU);

		//get battGrp info
		//if (HasEMSFeature(pCCU->emSystemType))
		{
			varV.blobValue = (BYTE *)&blobSingleBattData;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 
				501 + 0, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		}

		CCUCtrlCmd.emCtrl_Type = GRP_ACDC_INVERT_START;

		if(blobSingleBattData.fPackTotalVolt > 50.0)
		{	
			//20230228.  due to 62.5 is allow overload. +1~5 is avoid Overload;
			fLimitCurrent = (pEMS->calc_Node[0].fBatt_Distri_4_ACDC_Pwr*1000)/(blobSingleBattData.fPackTotalVolt+10);
		}
		CCUCtrlCmd.fGrpVoltLmt = MIN(1000, blobSingleBattData.fAllow_LmtChargeVolt);
		CCUCtrlCmd.fGrpCurrLmt = MIN(fLimitCurrent,blobSingleBattData.fAllow_MaxDisChargeCurr);

		TRACED("_thread_Process_SemFrom485 :fLimitCurrent=%0.1f fBatt_Distri_4_ACDC_Pwr=%0.1f fPackTotalVolt=%0.1f"
		"fGrpVoltLmt=%0.1f  fAllow_LmtChargeVolt=%0.1f  fGrpCurrLmt=%0.1f fAllow_MaxDisChargeCurr=%0.1f\n",
		fLimitCurrent,pEMS->calc_Node[0].fBatt_Distri_4_ACDC_Pwr,blobSingleBattData.fPackTotalVolt,
		CCUCtrlCmd.fGrpVoltLmt,blobSingleBattData.fAllow_LmtChargeVolt,CCUCtrlCmd.fGrpCurrLmt,blobSingleBattData.fAllow_MaxDisChargeCurr);

		Send_CCU_Ctrl(pCCU,CCUCtrlCmd);

		tmNow = GetCurrentTime();	// current time.
		pCCU->tSentCCUDisChargeCmdTime = tmNow;
		TRACE("============================_thread_Process_SemFrom485 Received signal time [%0.3f]============================",tmNow);

	}
}
BOOL Initialize_ESS(HANDLE hSamplerSelf)
{
	TRACEI("Initialize_ESS");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	ERRCDDE_DPR err;
	VAR_VALUE varV;
	
	if (pCCU->lcCfg.emSystemType == SYSTYPE_ESS_LIQUIDED)
	{// LJH250607 优化
		err = pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_ST_VAR, 500, 502, &varV, 4);
		pEMS->nNodeNum = (int)((err==ERR_DPR_OK) ? varV.lValue : 0);
		pEMS->nNodeNum = MIN(pEMS->nNodeNum, MAX_SINGLE_STBATT_NUM);
		pEMS->nNodeNum = MAX(pEMS->nNodeNum, 1);
	}
	else
	{
		pEMS->nNodeNum = MAX_SINGLE_STBATT_NUM;
	}

	// // unit test
	// pEMS->nNodeNum = 1;
	
	if(!CCU_Check_And_ReOpen_CommPort(pSampler, FALSE))
	{
		return FALSE;
	}
	//init sem
	pCCU->nEssSemCCUWith485Meter = Creat_SemID(SEM_CCU_485_PATH, SEM_CCU_485_PROJ_ID, 1, IPC_CREAT | 0666);
	TRACED("ESSccuSampler Init semid = [%d]\n",pCCU->nEssSemCCUWith485Meter);
	LogOut(MAIN_MODULE_MAIN, "ESS_CCU_SAMP", LOG_TYPE_INFO,
		"ESS_CCU_SAMP semid Init --> [%d] !\n",pCCU->nEssSemCCUWith485Meter);
	if (pCCU->nEssSemCCUWith485Meter != -1)
	{
		Init_Sem(pCCU->nEssSemCCUWith485Meter, 0, SETVAL, 0);
	}

	//----- Create A Thread To Process the sem-----//
	struct sched_param param;
	pthread_attr_t attr;
	pthread_t hThread;

	pthread_attr_init(&attr);
	param.sched_priority = 98;	//数值越大，优先级越高。
	pthread_attr_setschedpolicy(&attr,SCHED_RR);//实时调度策略
	pthread_attr_setschedparam(&attr,&param);//设置优先级
	pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);//要使优先级其作用必须要有这句话

	if(pthread_create(&hThread, &attr, _thread_ESS_thread_Process_Sem,
		(void *)pSampler) != 0) //0-Success
	{
		TRACE("----- Create _thread_ESS_thread_Process_Sem Failed -----\n");
		LogOut(MAIN_MODULE_REPORTER, "_thread_ESS_thread_Process_Sem", LOG_TYPE_ERROR, 
			"Init _thread_ESS_thread_Process_Sem FAILED!!!");
		pthread_attr_destroy(&attr);
		return FALSE;  
	}
	return TRUE;
}

static void Test_RPMsg_Reply(ROUGH_CCU_DATA* pCCU);	// TEST

int Sample_ESS(HANDLE hSamplerThis)
{
	TRACEW(">>> Sample_ESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;

	BuildRpmsgCommChannel(pCCU);

	if(pCCU->bRpmsgPortReady && pCCU->bRpmsgPCS_InfoReady)
	{
		// 模块信息
		if(pCCU->bRpmsgPCS_InfoReady)
		{
			Recv_RpMsg_CCU_Msg(pCCU);
		}
		// 模块配置信息
		if(pCCU->bRpmsgPCS_CfgReady)
		{
			Recv_RpMsg_CCU_Reply_Msg(pCCU);
			// Test_RPMsg_Reply(pCCU);	// TEST: 模拟回复数据
		}
	}

	//if is now exiting, do nothing here...
	/*if(pCCU->emCCUState == CCU_STATE_EXITING)
	{
		TRACE("Waitting Exist, Do Nothing!\n");
		return ERR_SAMPLE_OK;
	}*/

	BYTE byData[5] = {'\0'};

	static time_t tLastSendCFGG;
#ifdef IMMU3

	if(pCCU->bCommFail)
	{
		if(pCCU->emCCUState != CCU_STATE_LOCKED)
		{
			pCCU->emCCUState = CCU_STATE_FAULTED;
			pCCU->emSystemFailReason = SYSFAIL_REASON_CCU_COMFAIL;
		}
	}
	//---Step1. Shoule Get LCCU Config ,then Process Data Communication---//
	// printf("loc:%X, bIsReceivedCFGG_FromLCCU=%d", pCCU->strHEARTInfo.bySWRunLocation, pCCU->bIsReceivedCFGG_FromLCCU);
	if (!pCCU->bIsReceivedCFGG_FromLCCU)
	{
		if(pCCU->strHEARTInfo.bySWRunLocation == 0xA0) // IN BOOT
		{
			printf("loc:%02X, IN BOOT~~~\n", pCCU->strHEARTInfo.bySWRunLocation);
			pCCU->nCommFailTimes = 0;
			pCCU->bCommFail = FALSE;
			pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;

		}
		else
		{
			if(!CCU_Check_And_ReOpen_CommPort(pSampler, FALSE))
			{
				CCU_Close_CommPort(pSampler);
				// printf("\nXSY ::: Can't OpenPort\n\n");
				// goto stateM;
			}
		// TRACED("ccu_sampler_main_232:CCU_Sample_RS232 nCommFailTimes:%d", pCCU->nCommFailTimes);
			else
			{
				if(!DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500))
				{
					// printf("XSY::Send CFGG FAIL\n");
					pCCU->nCommFailTimes++;
					if(pCCU->nCommFailTimes >= 10)
					{
					TRACED("-----------CFGG not Responded, CCU_232 has comm fail---------\n");
					pCCU->nCommFailTimes = 10;
					pCCU->bCommFail = TRUE;
					CCU_Close_CommPort(pSampler);
					pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
					}
					// goto stateM;
				}
				else
				{
					// printf("XSY::Send CFGG SUCCESS\n");
					tLastSendCFGG = _NOW_;
					pCCU->nCommFailTimes = 0;
					pCCU->bCommFail = FALSE;
					pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
					TRACE("Info:Here Get CFGG ok!!!\n");
				}
			}

		}

	}
	else
	{
		// TRACEW("XSY::elasps(tLastSendCFGG) == %d\n", ElapseS(tLastSendCFGG));
		if(ElapseS(tLastSendCFGG) >= 60*60)// 5s,test //2*60*60) // 60*60, sample cfgg every 1 hour
		{
			pCCU->bIsReceivedCFGG_FromLCCU = FALSE;
			// TRACEW("XSY:::sample CFGG\n");
		}
	}
	//TRACE("nCommFailTimes =%d emCommStatus=%d\n", pCCU->nCommFailTimes, pCCU->emCommStatus);
	//Judge if comm fail
	time_t tmNow = _NOW_;
	static int s_nCommFaileResetCounter = 0;
	// if(tmNow - pCCU->strLSYSInfo.tmLastRecv_LSYS >= LSYS_BROADCAST_PERIOD
		// && 
	if(tmNow - pCCU->strHEARTInfo.tmLastRecv_HEART > (CCU_HEART_PERIOD+0.5))
	{
		// printf("XSY:: CCU CommFail---\n");

		pCCU->nCommFailTimes++;
		// printf("CCU_Sample_RS232 We have got CCU fail-times[%d]", pCCU->nCommFailTimes);

		if(pCCU->nCommFailTimes >= 10) // 10s for commFail judge
		{
			// 通信中断处理
			TRACED("CCU_Sample_RS232 Sample Failed!");
			pCCU->emCommStatus = CCU_COMMSTAT_FAILED;

			if(!pCCU->bCommFail)
			{
				LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, "CCU communication FAILED, please check comm-port cable!!!");	
				InitCCUStuffData(pCCU);
			}		
			pCCU->nCommFailTimes = 10;
			pCCU->bIsReceivedCFGG_FromLCCU = FALSE; // 在下次采样时 发送 CFGG
			pCCU->bCommFail = TRUE;

			// 尝试重新打开端口
			static time_t s_tmLastReopenComm = 0;
			if (ElapseS(s_tmLastReopenComm) >= 30) //30*4s timeout for reset when no CCU comm
			{
				s_tmLastReopenComm = _NOW_;
				BOOL bReOpenOK = CCU_Check_And_ReOpen_CommPort(pSampler, FALSE);
				LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, "Reopen the CCU-RS232 port => %s",
					bReOpenOK ? "SUCCESS" : "FAILED");
				s_nCommFaileResetCounter++;
				// if(s_nCommFaileResetCounter > 4)
				// keep-app-alive(2026-09): 只有"曾经成功通信过"才允许触发系统重启。
				// 从未建立通信时仅告警（否则无 CCU 的现场/台架每 ~5.5 分钟被无条件重启）。
				if(!pCCU->bEverRecvHeart)
				{
					if(s_nCommFaileResetCounter == 11) // 只打印一次，避免 30s 周期刷屏
					{
						LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR,
							"CCU-RS232 never established communication; skip system reset (alarm only)");
					}
				}
				else if(s_nCommFaileResetCounter > 10) // 10s*10
				{
					//_SYSTEM("reboot &"); //need reboot system
					LogOut(MAIN_MODULE_SAMPLER, "CCU232Samp", LOG_TYPE_ERROR, 
						"!!!Now we will RESET the system because of NO response from CCU-RS232!!!");
					//gracefully reboot system
					EVENT_SYSTEM_EXIT evExit;
					evExit.nExitType = SYS_INTERNAL_ERROR;
					SendMsg(pCCU->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
						&evExit, sizeof(EVENT_SYSTEM_EXIT));
				}	
			}
			goto stateM;
		}		
	}
	else
	{
		// printf("XSY:: CCU Sucess---\n");
		s_nCommFaileResetCounter = 0;
		pCCU->nCommFailTimes = 0;
		pCCU->bCommFail = FALSE;
		pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
	}
	
	if(pCCU->emCommStatus != CCU_COMMSTAT_NORMAL)
	{
		TRACEE("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
		goto stateM;
	}
	//Trying Recv LCCU Data LSYS & RTIF
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		// TRACEX("Now trying to receive CCU_Wait_ACCU_Reply_RS232");
		CCU_Wait_ACCU_Reply_RS232(pCCU, NULL, 0, NULL, 0);
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	
	//John-2021-08-26 ,Step Up Send USYS To CCU,Ensure CCU recv Battery Data ASAP
	// if(ElapseS(pCCU->strLSYSInfo.tmLast_Sent_USYS) >= 1)
	{
		if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			Send_USYS(pCCU);
			pCCU->strLSYSInfo.tmLast_Sent_USYS = tmNow;
			Mutex_Unlock(pCCU->hSamplerLock);
		}
	}

#else

	if(pCCU->hDataComm == NULL)
	{
		pCCU->hDataComm = pSampler->pUsedCommPort->Accept(pSampler->hCommPort);
		if(pCCU->hDataComm != NULL)
		{
			pCCU->emCommStatus = CCU_COMMSTAT_ACCEPTED;
			pCCU->nCommFailTimes = 0;
			TRACE("pCCU->hDataComm != NULL\n");
		}
		else
		{
			TRACEX("---pCCU->hDataComm is NULL, Should Continue Wait Accept---\n");
 			Sleep(500);
			
			pCCU->nCommFailTimes++;
			if(pCCU->nCommFailTimes >= 20)
			{
				TRACEX("-----------Accept is NULL, CCU is comm fail---------\n");
				pCCU->nCommFailTimes = 20;
				pCCU->bCommFail = TRUE;
			}

			goto stateM;
		}
	}

	//---Step1. Shoule Get LCCU Config ,then Process Data Communication---//
	if(pCCU->emCommStatus == CCU_COMMSTAT_ACCEPTED)
	{
		if(!DoTCPSession(pCCU, NL_MSG_CFGG, byData, 0, 500))
		{
			pCCU->nCommFailTimes++;
			if(pCCU->nCommFailTimes >= 20)
			{
				TRACED("-----------CFGG not Responded, CCU has comm fail---------\n");
				pCCU->nCommFailTimes = 20;
				pCCU->bCommFail = TRUE;
				if(pCCU->hDataComm != NULL)
				{
					TRACE("---Close hDataComm---\n");
					//pSampler->pUsedCommPort->Close(pCCU->hDataComm);
					CCU_TCP_Close_Client(pCCU);
				}
				pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
			}
			goto stateM;
		}
		else
		{
			pCCU->nCommFailTimes = 0;
			pCCU->bCommFail = FALSE;
			pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
			TRACE("Info:Here Get CFGG ok!!!\n");
		}
	}

	//TRACE("nCommFailTimes =%d emCommStatus=%d\n", pCCU->nCommFailTimes, pCCU->emCommStatus);

	//Judge if comm fail
	time_t tmNow = _NOW_;
	if(tmNow - pCCU->strLSYSInfo.tmLastRecv_LSYS >= LSYS_BROADCAST_PERIOD)
	{
		pCCU->nCommFailTimes++;
		if(pCCU->nCommFailTimes >= 30) //30s
		{
			TRACEX("-----------CCU has comm fail[%ld, %d, %d]---------\n", 
				tmNow, pCCU->strLSYSInfo.tmLastRecv_LSYS,
				pCCU->nCommFailTimes);
			pCCU->nCommFailTimes = 30;
		 	
			if(pCCU->hDataComm != NULL)
			{
				InitCCUStuffData(pCCU);
				pSampler->pUsedCommPort->Close(pCCU->hDataComm);
				pCCU->hDataComm = NULL;
			}
			pCCU->emCommStatus = CCU_COMMSTAT_FAILED;
			goto stateM;
		}
	}
	else
	{
		pCCU->nCommFailTimes = 0;
		pCCU->emCommStatus = CCU_COMMSTAT_NORMAL;
		//TRACE("nCommFailTimes=%d, emCommStatus=%d\n",pCCU->nCommFailTimes,pCCU->emCommStatus);
	}
	if(pCCU->emCommStatus != CCU_COMMSTAT_NORMAL)
	{
		TRACEE("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
		goto stateM;
	}
	//Trying Recv LCCU Data LSYS & RTIF
	int nErr = Mutex_Lock(pCCU->hSamplerLock, 100);
	if(nErr == ERR_MUTEX_OK)
	{
		//TRACEX("Now trying to receive LSYS_LCHG for Client\n");
		// Recv_Tcp_CCU_Msg(pCCU);
		// Recv_RpMsg_CCU_Msg(pCCU);
		Mutex_Unlock(pCCU->hSamplerLock);
	}
	
	//John-2021-08-26 ,Step Up Send USYS To CCU,Ensure CCU recv Battery Data ASAP
	//if(ABS(tmNow - pCCU->strLSYSInfo.tmLast_Sent_USYS) >= 1.0)
	{
		if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			Send_USYS(pCCU);
			pCCU->strLSYSInfo.tmLast_Sent_USYS = tmNow;
			Mutex_Unlock(pCCU->hSamplerLock);
		}
	}

	/*if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
	{
		//Judge_IfNeedDisConnectACLoadMCB(pCCU);
		Judge_IfNeedQueryPCS_Settings(pCCU);
		Mutex_Unlock(pCCU->hSamplerLock);	
	}*/
#endif


	//if(pCCU->emCommStatus == CCU_COMMSTAT_NORMAL)
	{
		OnHandle_STS_OnOffStatu(pCCU);

#ifdef IMMU2
		if(Mutex_Lock(pCCU->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
	 		On_HandleHMI_On2OffGridEvent(pCCU);
			Mutex_Unlock(pCCU->hSamplerLock);
		}
#endif

		
		Refresh_PCSVersion(pCCU);
		
		OnHandle_BatCabFireProtAlm(pCCU);

		OnHandle_PowerCabLighting(pCCU);

		OnHandle_TrigCANMsg(pCCU);
		
		//Dealing with PCS relay and Batt Grp Input Contactor Linkage Errors
		Onhandle_GrpInputContactor1_CtrlAndExcuteStatus(pCCU);
		Onhandle_GrpInputContactor2_CtrlAndExcuteStatus(pCCU);
		Onhandle_GrpInputContactor3_CtrlAndExcuteStatus(pCCU);
		Onhandle_GrpInputContactor4_CtrlAndExcuteStatus(pCCU);
		Onhandle_PCS_RelayAlarm(pCCU);

		//Dealing with MPPT and PV Contactor Errors
		Onhandle_PVContactor1_CtrlAndExcuteStatus(pCCU);
		Onhandle_PVContactor2_CtrlAndExcuteStatus(pCCU);
		Onhandle_PVContactor3_CtrlAndExcuteStatus(pCCU);
		Onhandle_PVContactor4_CtrlAndExcuteStatus(pCCU);
		Onhandle_MPPT_SCAlarm(pCCU);
	
		if(pCCU->emESSDevType == LIQUID_COOLING_BACKUP)
		{
			OverloadAlarm(pCCU); 
		}

		if(pCCU->bNeedSyncTime2IMSU == TRUE)
		{
			//printf("\n---------IMSU-D:%d Sync Time to CCU --------\n", pClient->nReal_CCUAddr);
			// BYTE byData[5];
 			UINT nTimeSecond = (UINT)_NOW_;
			byData[0] = 0x02;
			byData[1] = (BYTE)(nTimeSecond >> 24);
			byData[2] = (BYTE)(nTimeSecond >> 16);
			byData[3] = (BYTE)(nTimeSecond >> 8);
			byData[4] = (BYTE)nTimeSecond;
			DoTCPSession(pCCU, NL_MSG_CFGS, byData, 5, 1000);
			pCCU->bNeedSyncTime2IMSU = FALSE;
		}

		//Enter Upgrade Mode
		if(pCCU->tagCCUUpgradeParam.tagMstUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
		{
			tLastSendCFGG = _NOW_;
			pCCU->bIsReceivedCFGG_FromLCCU = TRUE;
			//if(fabs(tmNow - pCCU->tagCCUUpgradeParam[pClient->nReal_CCUAddr].tmLastEnterUpgrade >= 0.3)) //Consider IMSU_D process Speed
			{
				//pCCU->tagCCUUpgradeParam[pClient->nReal_CCUAddr].tmLastEnterUpgrade = tmNow;
				HandleIMSUUpgrading(pCCU);
			}
		}
	}
	// else
	// {
	// 	TRACEE("ERR: emCommStatus=%d Not Normal, this should Not happened", pCCU->emCommStatus);
	// 	goto stateM;
	// }
	
	//TRACE("Now Running Here ,Do some Cycle Data Process!\n");
stateM:

	//TRACE("Info:---Running Here stateM---\n");
	Refresh_SysAvailState(pCCU);
	
	//TRACE("emCCUState=%d, emSystemFailReason=%d \n", pCCU->emCCUState, pCCU->emSystemFailReason );
 
	// A_BLOB_SINGLE_STBATT_RUNINFO
	Update_SysDataInfo(pCCU);
	pCCU->bQuickChannle = FALSE;
	Ems_Mgmt_Main_ESS(pCCU);
	pCCU->emErrCode_MgmtCfg = Check_MgmtPlanCfg(pCCU);
	
// LJH241101 MU3
#ifdef IMMU2
	//now Manage the LED status
	MangeLEDStatus(pCCU);
#endif

 	CalcMPPTGenerateKWH(pCCU);
	
	CalcDCLoadCostKWH(pCCU);
 
	PushAllData(pCCU);
 
	Prepare_ESRunData(pCCU);
 
	Manage_ESStoreData(pCCU);
 
	//Manage_ECOMode(pCCU);
	// printf("fACDC_RealTime_V[%0.1f]_A[%0.1f]_P[%0.1f]\n",pCCU->strLSYSInfo.branchNodeData[0].fACDC_RealTime_Volt,
	// 	pCCU->strLSYSInfo.branchNodeData[0].fACDC_RealTime_Curr,pCCU->strLSYSInfo.branchNodeData[0].fActvPwr_PCSGrp);
		
	TRACEW(">>> Sample_ESS END>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

	return ERR_SAMPLE_OK;
}

/*===============================================================================
 *  TEST: 模拟 RPMSG 回复数据，压入 DPR 状态参数 5320~5333 验证数据流
 *  验证后删除 Test_RPMsg_Reply(pCCU); 调用即可
 *===============================================================================*/
static void Test_RPMsg_Reply(ROUGH_CCU_DATA* pCCU)
{
	VAR_VALUE varV;
	memset(&varV, 0, sizeof(varV));
	TRACED("[TEST-RPMSG] injecting fake reply data...");

	/* 0x6E 模块类型 → 5320~5322 */
	pCCU->strModuleTypeCfg.byACDC_Module = 1;
	pCCU->strModuleTypeCfg.byDCDC_Module = 1;
	pCCU->strModuleTypeCfg.byMPPT_Module = 1;
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5320, varV);
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5321, varV);
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5322, varV);

	/* 0x6F 模块数 → 5323~5328 */
	pCCU->strModuleNumCfg.byACDC_GRP1_SigNum = 10;
	pCCU->strModuleNumCfg.byACDC_GRP2_SigNum = 20;
	pCCU->strModuleNumCfg.byDCDC_GRP1_SigNum = 30;
	pCCU->strModuleNumCfg.byDCDC_GRP2_SigNum = 40;
	pCCU->strModuleNumCfg.byMPPT_GRP1_SigNum = 50;
	pCCU->strModuleNumCfg.byMPPT_GRP2_SigNum = 60;
	varV.lValue = 10; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5323, varV);
	varV.lValue = 20; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5324, varV);
	varV.lValue = 30; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5325, varV);
	varV.lValue = 40; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5326, varV);
	varV.lValue = 50; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5327, varV);
	varV.lValue = 60; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5328, varV);

	/* 0x72 自动/手动 → 5329 */
	pCCU->strManulAutoCtrl.byManulAutoMode = 1;
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5329, varV);

	/* 0x72 并离网 → 5330~5333 */
	pCCU->strN2002_OnOffGridCtrl.byABU_Enable = 1;
	pCCU->strN2002_OnOffGridCtrl.byOnOffGridAutoSwitchMode = 1;
	pCCU->strN2002_OnOffGridCtrl.byOnOffGridMode = 1;
	pCCU->strN2002_OnOffGridCtrl.byABU_OnOff = 1;
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5330, varV);
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5331, varV);
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5332, varV);
	varV.emValue = 1; Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5333, varV);

	TRACED("[TEST-RPMSG] done!");
}
