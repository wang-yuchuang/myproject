// #define _TRACE_PRINT_ // batt
#include "imeu_sampler_main.h"
// #define TRACE TRACED
// #define TRACEX TRACED

#define GET_BIT(num, n) (((num) >> (n)) & 1)

void Make_SendCmd_4CANFrame(IN A_CAN_ND_BATT_FRAME *pFrm)
{
	UINT n = 0;
	UINT uiFrmID = pFrm->uiFrmID;
	BYTE *pbyFrame = pFrm->byRawData;
	UINT uiDataLen = MIN(pFrm->uiDataLen, CAN_DATA_AREA_LEN);

 	*(pbyFrame) = 	(BYTE)uiDataLen;
	*(pbyFrame + 1) = 	(BYTE)(uiFrmID >> 24);
	*(pbyFrame + 2) = 	(BYTE)(uiFrmID >> 16);
	*(pbyFrame + 3) = 	(BYTE)(uiFrmID >> 8);
	*(pbyFrame + 4) = 	(BYTE)(uiFrmID);

	for(n = 0; n < uiDataLen; n++)
	{
		*(pbyFrame + 5 + n) = pFrm->byData[n];
	}

	//TRACE("---------CAN-Data %02x %02x %02x %02x %02x %02x %02x %02x\n",\
	//	pbyFrame[5],pbyFrame[6],pbyFrame[7],pbyFrame[8], \
	//	pbyFrame[9],pbyFrame[10],pbyFrame[11],pbyFrame[12]);

	return;
}

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
	IMEU_SAMPLER_ROUGH *pIMEU_Rough = (IMEU_SAMPLER_ROUGH*)pSampler->pDataHeap;
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
				pIMEU_Rough->bStandby_Set = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			default:
				break;
			}
			TRACED("st_batt_sampler::Event_Handler nId=1 nSubId:%d err:%d bStandby_Set:%d",
				pCfgChange->parmPTR.nSubId, err, pIMEU_Rough->bStandby_Set);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		else if((pCfgChange->parmPTR.nId == 200) && (pCfgChange->parmPTR.nSubId == 201))
		{
 			varV.blobValue = (BYTE *)&pIMEU_Rough->blobEmsCfgPlan;
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
			pIMEU_Rough->emSTSAutoManual = (STS_AUTOMANUAL_ONOFFGRID)varV.emValue;
			TRACE("EvtHandle:emSTSAutoManual=[%d]\n", pIMEU_Rough->emSTSAutoManual);
		}
		else if (pCfgChange->parmPTR.nId == 1001 &&
			(pCfgChange->parmPTR.nSubId == 3303 || pCfgChange->parmPTR.nSubId == 3306 || pCfgChange->parmPTR.nSubId == 3309))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			switch (pCfgChange->parmPTR.nSubId)
			{
			case 3303: // LJH250408 CtrlByREMS 1001, 3303,
				pIMEU_Rough->enableREMSCtrl = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
				break;
			case 3306: // uChargeStopSoc(REMS) 1001, 3306, 
				pIMEU_Rough->nStopSOC_ChrgSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 100);
				break;
			case 3309: // uDischargeStopSoc(REMS) 1001, 3309, 
				pIMEU_Rough->nStopSOC_DiscSet_SEMS = (BYTE)((err==ERR_DPR_OK) ? varV.ulValue : 0);
				break;
			default:
				break;
			}
			TRACED("st_batt_sampler::Event_Handler nId=1001 nSubId:%d err:%d "
				"enableREMSCtrl:%d nStopSOC_ChrgSet_SEMS:%d %d",
				pCfgChange->parmPTR.nSubId, err,
				pIMEU_Rough->enableREMSCtrl, pIMEU_Rough->nStopSOC_ChrgSet_SEMS, pIMEU_Rough->nStopSOC_DiscSet_SEMS);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}
		/*
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			TRACE("stBatt update OCPP cfg\n");
			//OCPP setting has changed
			varV.blobValue = (BYTE *)&pIMEU_Rough->blobESiOCPPCfgs;
			//copy setting to current data
			ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 26, &varV, sizeof(A_BLOB_ES_iOCPP_CONFIGURAION));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
		}*/
	}
	else if(emEvType == EVENT_TYPE_ALARM)
	{
		//handle alarm event if needed
		IMEU3DoCtl_TargetAlarms(pIMEU_Rough);//根据下级储能柜告警状态控制断路器跳闸保护
	}

	return EVENT_HANDED_OK;
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
 	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;

	pSampler->pDataHeap = (void*)NEW(IMEU_SAMPLER_ROUGH, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}
 
	IMEU_SAMPLER_ROUGH *pIMEU_Rough = (IMEU_SAMPLER_ROUGH*)pSampler->pDataHeap;
	ZERO_POBJS(pIMEU_Rough, 1);

	pIMEU_Rough->hSampler = hSamplerSelf;
	pIMEU_Rough->nSamplerId = pSampler->info.nSamplerID;
	pIMEU_Rough->hSamplerLock = pSampler->hSamplerLock;
	pIMEU_Rough->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	pIMEU_Rough->hCAN1Write_Mutex = Mutex_Create(TRUE);

	pIMEU_Rough->t_FirstBattBoot = _NOW_;

	VAR_VALUE varV;
	ERRCDDE_DPR err;
	
	// ZERO_POBJS(&pIMEU_Rough->hisBmsRunDataRec, 1);
 
	pIMEU_Rough->lcCfg = GetLCCfg_Sampler(pIMEU_Rough->hSampler); // LJH250508 LCCfg
	
	ZERO_POBJS(&pIMEU_Rough->blobEmsCfgPlan, 1);
 	varV.blobValue = (BYTE *)&pIMEU_Rough->blobEmsCfgPlan;
	pIMEU_Rough->pDPR->Get((HANDLE)pIMEU_Rough->hSampler, GETTYPE_ST_VAR, 200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));

	int n;
	for (n = 0; n < MAX_IMEU_NUM; ++n)
	{
		pIMEU_Rough->IMEU2CANRoughData[n].hSampler = pIMEU_Rough->hSampler;
		pIMEU_Rough->IMEU2CANRoughData[n].nSamplerId = pIMEU_Rough->nSamplerId;
		pIMEU_Rough->IMEU2CANRoughData[n].hSamplerLock = pIMEU_Rough->hSamplerLock;

		pIMEU_Rough->IMEU3CANRoughData[n].hSampler = pIMEU_Rough->hSampler;
		pIMEU_Rough->IMEU3CANRoughData[n].nSamplerId = pIMEU_Rough->nSamplerId;
		pIMEU_Rough->IMEU3CANRoughData[n].hSamplerLock = pIMEU_Rough->hSamplerLock;

		pIMEU_Rough->PowerEU3_OtherEU3_Data[n].hSampler = pIMEU_Rough->hSampler;
		pIMEU_Rough->PowerEU3_OtherEU3_Data[n].nSamplerId = pIMEU_Rough->nSamplerId;
		pIMEU_Rough->PowerEU3_OtherEU3_Data[n].hSamplerLock = pIMEU_Rough->hSamplerLock;

		pIMEU_Rough->pDPR->Get(pIMEU_Rough->hSampler, GETTYPE_ST_VAR, 621 + n, 409, &varV, 4);
		pIMEU_Rough->emPOWER_IMEUType[n] = (IMEU_TYPE)varV.emValue;

#ifdef IMMU3
		// memset(&pIMEU_Rough->IMEU3CANRoughData, 0, MAX_IMEU_NUM*sizeof(IMEU3_CAN_ROUGH_DATA));

		varV.emValue = 1;//闭合DO3，给BMU上高压
		Push_VarValue(SETTYPE_INTERN_CTRL, pIMEU_Rough->hSampler, 601+n, 602, varV);
#endif

	}

	initIMEU2CAN(pIMEU_Rough->IMEU2CANRoughData, MAX_IMEU_NUM);

	initIMEU3CAN(pIMEU_Rough->IMEU3CANRoughData, MAX_IMEU_NUM);

	initIMEU3CAN(pIMEU_Rough->PowerEU3_OtherEU3_Data, MAX_IMEU_NUM);

	pIMEU_Rough->emESSDevType = GetESSDeviceType_Sampler(hSamplerSelf);

	COMM_CAN_BAUDRATE	emLevel = COMM_CAN_BAUDRATE_500K;// 站控EMSCAN1的波特率默认500K
	
 	if(pIMEU_Rough->lcCfg.emSystemType == SYSTYPE_EMS_STATION)//站控系统
	{
		//Open comm port
		int nErrCode = 0;
		pIMEU_Rough->pComm = pSampler->pUsedCommPort;
		pIMEU_Rough->hCanDevHanle =  pSampler->pUsedCommPort->Open(
			pSampler->pUsedCommPort->info.szPortAttri,
			pSampler->info.szSamplerSetting,
			emLevel, CAN1_READ_TIMEOUT, &nErrCode); //50ms Timeout

		if(ERR_COMM_OK != nErrCode)
		{
			TRACE("Open CAN1 Failed !!! nErrCode=[%d]\n", nErrCode);
			return FALSE;
		}
	}

	//register event-handler
	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		//{EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
		//{EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END},
		//{EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_BEGIN},
	};

	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pIMEU_Rough->pDPR->Register_EvHandler("stBatt_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	//Init the state machine
	TRACE("Ready to enter init AutoMode.\n");

	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}


int IMEU2SendSetCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	DATA_PROCESSOR* pDPR = pIMEU_Rough->pDPR;

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
		pIMEU_Rough->IMEU2CANRoughData[nWhichIMEU].nPT1000Num = pVarVal->lValue;
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
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, nBattIdx, frmSent.byRawData, 1);
		}
	}
	return 0;
}

//IMEU3  send set cmd
int IMEU3SendSetCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	DATA_PROCESSOR* pDPR = pIMEU_Rough->pDPR;

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
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, nBattIdx, frmSent.byRawData, 1);
		}
	}
	return 0;
}



int IMEU3SendCtlCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	if (NULL == pVarVal)
		return  ERR_CONTROL_FAILED;

	// if ((nDevId < 611 || nDevId > 614) && (nDevId < 601 || nDevId > 604))
	// {
	// 	return;
	// }

	DATA_PROCESSOR* pDPR = pIMEU_Rough->pDPR;

	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));
	

	int iDevIdx = nDevId - 621;
	

	if(621 == nDevId)
	{
		if (nParamId >= 530 && nParamId <= 531)
		{
			frmSent.uiFrmID = 0x05F20000;
			frmSent.byData[0] = 0xC1;// 控制第一路DO 1~7
			frmSent.byData[1] = (BYTE)(nParamId - 525); // DO 5~6
			frmSent.byData[2] = (BYTE)pVarVal->emValue;
		}
		else
		{
			TRACE("Error: Not Found IMEU Control Cmd\n");
			return  ERR_CONTROL_FAILED;
		}

	}
	else if(622 == nDevId)
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
	else if (623 == nDevId)
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
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, 0, frmSent.byRawData, 1);
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

//edit by Kuangyue 20251130
int IMEU3CtlPWM_MutifunCab(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	int Value = 0;
	IMEU3_CAN_DataInfo* pIMEU3_Power = &pIMEU_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo;
	float MaxTemp = 0;
	MaxTemp = MAX(MAX(MAX(pIMEU3_Power->fNTC[0], pIMEU3_Power->fNTC[1]), pIMEU3_Power->fNTC[2]), pIMEU3_Power->fNTC[3]);
	if(MaxTemp >= 40)
	{
		Value = MAX(Value, 50);
	}
	Value = MAX(Value, 15); //系统运作至少按15转风扇
	frmSent.uiFrmID = 0x05F10000;
	frmSent.byData[0] = 0xC4;//四路PWM控制
	frmSent.byData[1] = 0x00; 
	frmSent.byData[2] = (BYTE)Value;

	BYTE byEMSAddr = 0x5A;
	BYTE byIMEU3Addr = 0x1E;
	
	frmSent.uiFrmID |= (byIMEU3Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;
	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, 0, frmSent.byRawData, 1);
		}
	}

	return 0;
}

int IMEU3CtlPWM(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	int Value = 0;
	int nBattIdx = 0;
	float MPPTAllPwr = 0;
	for(nBattIdx = 0; nBattIdx < MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
		MPPTAllPwr +=(pIMEU_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Volt*pIMEU_Rough->branchGrpData[nBattIdx].fMPPT_RealTime_Curr*0.001);
	}
	if(MPPTAllPwr >= 80)
	{
		Value = 100;
	}
	else
	{
		Value = MPPTAllPwr/80 * 100;
	}
	IMEU3_CAN_DataInfo* pIMEU3_Power = &pIMEU_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo;
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
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, 0, frmSent.byRawData, 1);
		}
	}

	return 0;
}

int IMEU3SendDOCtlCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, int BatIdx, BYTE WhichDo, BYTE Value)
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
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, 0, frmSent.byRawData, 1);
		}
	}


	return 0;
}
//edit by Kuangyue 20251130
static int IMEU3SendImluCAN(IMEU_SAMPLER_ROUGH *pIMEU_Rough, int BatIdx, BYTE Value)
{
	BYTE szIMEU3MapIdx[]={0x1E, 0x1F};  // 汇流柜EU3
	BYTE byCRC = 0;
	A_CAN_ND_BATT_FRAME frmSent;
	memset(frmSent.byData, 0, sizeof(frmSent.byData));

	frmSent.uiFrmID = 0x05F60000;
	frmSent.byData[0] = 0xC1;
	frmSent.byData[1] = 0x00; 
	frmSent.byData[2] = (BYTE)Value;

	BYTE byEMSAddr = 0x5A;
	BYTE byIMEU3Addr = szIMEU3MapIdx[BatIdx];
	
	frmSent.uiFrmID |= (byIMEU3Addr << 8);
	frmSent.uiFrmID |= byEMSAddr;
	frmSent.uiDataLen = CAN_DATA_AREA_LEN;

	byCRC = Crc_ComputeCRC8((frmSent.uiFrmID & 0xffff), &frmSent.byData[0], 7);
 	frmSent.byData[7] = byCRC;
	Make_SendCmd_4CANFrame(&frmSent);
	ssize_t nWriteSize = pIMEU_Rough->pComm->Write(pIMEU_Rough->hCanDevHanle, (void *)frmSent.byRawData, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pIMEU_Rough->stuCanSave.stuStatMach[0].emSaveState == STAT_ON_PENDING) &&
			(pIMEU_Rough->stuCanSave.emTrigMsgTp[0] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pIMEU_Rough, 0, frmSent.byRawData, 1);
		}
	}
	return 0;
}

static IMEU_LED_STATE GetCurrentLedState(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
    VAR_VALUE varV;
    ERRCDDE_DPR err = ERR_DPR_OK;
    A_BLOB_SEMS_CLIENTINFO_SUM clientSum;
    A_BLOB_SEMS_CLIENTINFO_SUM *pClientSum = &clientSum;
    ZERO_POBJS(pClientSum, 1);
    varV.blobValue = (BYTE*)pClientSum;
    
    // 1. 获取单个柜运行数据
    err = pIMEU_Rough->pDPR->Get(pIMEU_Rough->hSampler, GETTYPE_SP_VAR, 1001, 3900, &varV, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM));
    
    if (err != ERR_DPR_OK)
    {
        TRACEE("Get client summary data failed. err:%d", err);
        return IMEU_STATE_STANDBY;
    }
    
    IMEU3_CAN_DataInfo* pIMEU3_Multifun = &pIMEU_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo;
    BYTE emergency_value = GET_BIT(pIMEU3_Multifun->uDIStatus, 4); 

    if (emergency_value == 1)
    {
        return IMEU_STATE_FAULT;
    }
    if ((pClientSum->fACDC_RealTime_CurrSum > 1.0f) && (pClientSum->emACDC_WorkStatuSum == STATU_ACDC_RECTIFY))
    {
        return IMEU_STATE_CHARGE;
    }
    
    if ((pClientSum->fACDC_RealTime_CurrSum > 1.0f) && (pClientSum->emACDC_WorkStatuSum == STATU_ACDC_INVERTER))
    {
        return IMEU_STATE_DISCHARGE;
    }
    return IMEU_STATE_STANDBY;
}

static void bIMEU3LEDCtl_MultifunCube(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
    IMEU_LED_STATE state = GetCurrentLedState(pIMEU_Rough);
    
    switch (state)
    {
        case IMEU_STATE_FAULT:
            IMEU3SendImluCAN(pIMEU_Rough, 0, RED); // 汇流柜告警
            TRACE("MultifunCab IMEU3 LED Ctl: Status=FAULT, Sent RED\n");
            break;
            
        case IMEU_STATE_CHARGE:
            IMEU3SendImluCAN(pIMEU_Rough, 0, YELLOW); // 汇流柜充电
            TRACE("MultifunCab IMEU3 LED Ctl: Status=CHARGE, Sent YELLOW\n");
            break;
        case IMEU_STATE_DISCHARGE:
            IMEU3SendImluCAN(pIMEU_Rough, 0, BLUE); // 汇流柜放电
            TRACE("MultifunCab IMEU3 LED Ctl: Status=DISCHARGE, Sent BLUE\n");
            break; 
        case IMEU_STATE_OFFGRID:
			IMEU3SendImluCAN(pIMEU_Rough, 0, ORANGE); // 汇流柜离网
			TRACE("MultifunCab IMEU3 LED Ctl: Status=OFFGRID, Sent ORANGE\n");
			break;
        default:
            IMEU3SendImluCAN(pIMEU_Rough, 0, GREEN); // 汇流柜待机
            TRACE("MultifunCab IMEU3 LED Ctl: Status=STANDBY, Sent GREEN\n");
            break;
    }
    
    TRACED("MutifunCabIMEU3LEDCtl: Status update complete for state: %d", state);
}

//edit by Kuangyue 20251130
static void bIMEU3DoCtl_MultifunChecks(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	IMEU3_CAN_DataInfo* pIMEU3_Multifun = &pIMEU_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo; // 多功能汇流柜内EU3数据

	int idx;
	int cond = 0;
	VAR_VALUE varV;
	ERRCDDE_DPR err = ERR_DPR_OK;

	typedef enum
	{
		CMP_EQ_INT = 0,
		CMP_GE_NUM = 1
	} CMP_TYPE;

	struct DOCheck
	{
		int getType;
		int nDevId;
		int nParamId;
		int isFloat; /* 0 = use lValue, 1 = use fValue */
		CMP_TYPE cmp;
		double threshold;
		BYTE whichDo;
		BYTE trueVal;  /* value to send when condition is true */
		BYTE falseVal; /* value to send when condition is false */
	};

	struct DOCheck checks[] =
	{
		{ GETTYPE_SP_VAR, 621, 2315, 0, CMP_EQ_INT, 1.0, 5, 0, 1 },   // 水浸告警：1 异常 -> 发送 trueVal（断闸） 
		{ GETTYPE_SP_VAR, 621, 2305, 0, CMP_EQ_INT, 1.0, 5, 0, 1 },   // 急停告警：1 异常 -> 发送 trueVal（断闸） 
		{ GETTYPE_SP_VAR, 621, 2300, 1, CMP_GE_NUM, 75.0, 5, 0, 1 },  // 箱体环境温度 >=75 -> 发送 trueVal（断闸） 
		{ GETTYPE_SP_VAR, 621, 2301, 1, CMP_GE_NUM, 75.0, 5, 0, 1 },  // 箱体环境温度 >=75 -> 发送 trueVal（断闸） 
		{ GETTYPE_SP_VAR, 621, 2302, 1, CMP_GE_NUM, 125.0, 5, 0, 1 }, // 母排温度 >=125 -> 发送 trueVal（断闸） 
		{ GETTYPE_SP_VAR, 621, 2303, 1, CMP_GE_NUM, 125.0, 5, 0, 1 }, // 母排温度 >=125 -> 发送 trueVal（断闸）
	};

	const int nChecks = (int)(sizeof(checks) / sizeof(checks[0]));
	for ( idx = 0; idx < nChecks; ++idx)
	{
		err = pIMEU_Rough->pDPR->Get(pIMEU_Rough->hSampler, checks[idx].getType,checks[idx].nDevId, checks[idx].nParamId, &varV, 4);
		if (err != ERR_DPR_OK)
		{
			TRACED("MutifunCabIMEU3DoCtl: Get failed for dev=%d param=%d, err=%d", checks[idx].nDevId, checks[idx].nParamId, err);
			continue;
		}
		double val = checks[idx].isFloat ? varV.fValue : (double)varV.emValue;
		if (checks[idx].cmp == CMP_GE_NUM)
		{
			cond = (val >= checks[idx].threshold) ? 1 : 0;
		}
		else
		{
			cond = (val == checks[idx].threshold) ? 1 : 0;
		}
		if (cond)
		{
			IMEU3SendDOCtlCAN(pIMEU_Rough, 0, checks[idx].whichDo, checks[idx].trueVal);
			TRACED("MutifunCabIMEU3DoCtl: sent DO ctl dev=%d param=%d DO=%d val=%.2f",
				   checks[idx].nDevId, checks[idx].nParamId, checks[idx].whichDo, val);
		}
	}
}


// void IMEU3DoCtl_TargetAlarms(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
// {
// 	int i, j;
// 	int iClientIdx;
// 	VAR_VALUE varV;

// 	static const int targetAlarmIds[] = {553, 739, 813, 814, 815, 816}; // 目标告警 ID

// 	ERRCDDE_DPR err = pIMEU_Rough->pDPR->Get(pIMEU_Rough->hSampler, GETTYPE_ST_VAR, 1001, 3314, &varV, 4); /* ESC Num 储能柜数量 */
// 	int nESCNum = (err == ERR_DPR_OK) ? (int)varV.ulValue : 0;
// 	printf("MutifunCabIMEU3DoCtl: nESCNum=%d\n", nESCNum);
// 	A_BLOB_SEMS_CMD109 dataBloCmd902;
// 	varV.blobValue = (BYTE*)&dataBloCmd902;

// 	err = pIMEU_Rough->pDPR->Get(pIMEU_Rough->hSampler, GETTYPE_SP_VAR, 1001, 3412, &varV, sizeof(A_BLOB_SEMS_CMD109)); /* 储能柜总告警信息 */
// 	if (err != ERRCODE_DAI_OK)
// 	{
// 		TRACED("MutifunCabIMEU3DoCtl: failed to get alarm blob, err=%d", err);
// 		return;
// 	}
// 	// printf("MutifunCabIMEU3DoCtl: dataBlobCmd902.clientActiveAlarms[iClientIdx].iAlarmCount=%d\n, ", dataBloCmd902.clientActiveAlarms[iClientIdx].iAlarmCount);
// 	for ( iClientIdx = 0; iClientIdx < nESCNum; ++iClientIdx)
// 	{
// 		int iAlarmCount = dataBloCmd902.clientActiveAlarms[iClientIdx].iAlarmCount;
// 		for ( j = 0; j < iAlarmCount && j < MAX_QUERY_ACTV_ALM; ++j)
// 		{
// 			SC6_LDEV_109* pAlarm = &dataBloCmd902.clientActiveAlarms[iClientIdx].activeAlms[j];
// 			printf("dataBloCmd902.clientActiveAlarms[%d].activeAlms[%d]: AlarmId=%d\n", iClientIdx, j, pAlarm->nAlarmId);
// 			for ( i = 0; i < sizeof(targetAlarmIds)/sizeof(targetAlarmIds[0]); ++i)
// 			{
// 				if (pAlarm->nAlarmId == targetAlarmIds[i])
// 				{
// 					IMEU3SendDOCtlCAN(pIMEU_Rough, 0, 5, 0);
// 					IMEU3SendImluCAN(pIMEU_Rough, 0, RED); // 汇流柜告警
// 					printf("MutifunCabIMEU3DoCtl: Set alm_status to TRUE (0) for AlarmId=%d", pAlarm->nAlarmId);
// 					break;
// 				}
// 			}
// 		}
// 	}
// }

DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	IMEU_SAMPLER_ROUGH *pIMEU_Rough = (IMEU_SAMPLER_ROUGH*)pSampler->pDataHeap;
	if(SYSTYPE_EMS_STATION == pIMEU_Rough->lcCfg.emSystemType)
	{
		YFY_IMEU_Sample(pIMEU_Rough);
	}
	if(IMEU_TYPE_IMEU3 == pIMEU_Rough->emPOWER_IMEUType[0])//edit by Kuangyue 20251130 多功能汇流柜IMEU3数据
	{
		pushMultifunctionCombinerCabinetIMEU3(&pIMEU_Rough->PowerEU3_OtherEU3_Data[0]);
		bIMEU3DoCtl_MultifunChecks(pIMEU_Rough);//控制断路器跳闸保护
		IMEU3CtlPWM_MutifunCab(pIMEU_Rough);//风扇PWM控制
		bIMEU3LEDCtl_MultifunCube(pIMEU_Rough);// 汇流柜灯带控制
		// IMEU3DoCtl_TargetAlarms(pIMEU_Rough);
	}

	// --- 20220428 John Added This -----//
	// Save_BMSCanMsg2Log(pIMEU_Rough);

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
	IMEU_SAMPLER_ROUGH *pIMEU_Rough = (IMEU_SAMPLER_ROUGH*)pSampler->pDataHeap;
	
	int nRet = ERR_CONTROL_OK;
	// int nBattIdx;

	//屏蔽代码，设置成无电池后，其它设置项都会失效
/*
	if(pIMEU_Rough->emBattMFG == BATT_CFG_NO_BATT)
	{
		TRACE("Control: BATT_CFG_NO_BATT !!!\n");
		return ERR_CONTROL_FAILED;
	}
*/
	if(nDevId >= 621 && nDevId <= 624)
	{
		if (PARAM_TYPE_CONTROL == emParaType)
		{
			nRet = IMEU3SendCtlCAN(pIMEU_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
		}	
		else if(PARAM_TYPE_SETTING == emParaType)
		{
			int nWhichPowerIMEU = nDevId - 621;//edit by Kuangyue 20251130
			if(409 == nParamId)   //汇流柜内IMEU3和IMEU2配置选择
			{
				pIMEU_Rough->emPOWER_IMEUType[nWhichPowerIMEU] = (IMEU_TYPE)pVarVal->emValue;
				return ERR_CONTROL_OK;
			}
		}	
		else
		{
			return ERR_CONTROL_FAILED;
		}
		return nRet;
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
ssize_t Write_CAN1(IN IMEU_SAMPLER_ROUGH *pBattData, IN void * buf,IN size_t count)
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
void CAN_Filter_Conf(IMEU_SAMPLER_ROUGH *pBattData)
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
void CAN_ClearReadBuf(IMEU_SAMPLER_ROUGH *pBattData)
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

