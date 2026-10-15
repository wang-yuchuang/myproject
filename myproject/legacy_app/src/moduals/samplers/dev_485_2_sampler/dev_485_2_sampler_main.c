#include "dev_485_2_sampler_main.h"

extern void LED_RS485_Ctl_State(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough);
BYTE	HexToAscii(BYTE Input)
{
	if(Input > 9)
	{
		return Input + 0x37;					/*A - F*/
	}
	else
	{
		return Input + 0x30;					/*0 - 9*/
	}
}
 
BYTE	AsciiToHex(BYTE Input)
{
	if(Input > 0x39)
	{
		return Input - 0x37;					/*'A' - 'F'*/
	}
	else
	{
		return Input - 0x30;					/*'0' - '9'*/
	}
}

void	HexToTwoAsciiData(DWORD Input, BYTE *szOutData)
{
	*szOutData = HexToAscii(Input >> 4);
	*(szOutData + 1) = HexToAscii(Input & 0x0f);
	return;
}
 
BYTE	Ascii2ToHex(BYTE *pInput)
{
	return	((AsciiToHex(pInput[0])) << 4) + AsciiToHex(pInput[1]);
}

void	HexToFourAsciiData(DWORD Input, BYTE *szOutData)
{
	*szOutData = HexToAscii(Input >> 12);
	*(szOutData + 1) = HexToAscii((Input & 0x0f00) >> 8);
	*(szOutData + 2) = HexToAscii((Input & 0x00f0) >> 4);
	*(szOutData + 3) = HexToAscii(Input & 0x000f);
}

void	FloatTo8AsciiData(DWORD Input, BYTE *szOutData)
{
	//big endian to little endian	
	*(szOutData + 7) = HexToAscii((Input & 0x0f000000) >> 24);
	*(szOutData + 6)= HexToAscii(Input >> 28);
	*(szOutData + 5) = HexToAscii((Input & 0x000f0000) >> 16);
	*(szOutData + 4) = HexToAscii((Input & 0x00f00000) >> 20);
	*(szOutData + 3) = HexToAscii((Input & 0x00000f00) >> 8);
	*(szOutData + 2) = HexToAscii((Input & 0x0000f000) >> 12);
	*(szOutData + 1) = HexToAscii((Input & 0x0000000f));
	*(szOutData + 0) = HexToAscii((Input & 0x000000f0) >> 4);
}

BYTE LenCheckSum(UINT wLen)
{
	BYTE byLenCheckSum;

 	if (wLen == 0)
	{
		return 0;
	}

	byLenCheckSum = 0;
	byLenCheckSum += wLen & 0x000F;          
	byLenCheckSum += (wLen >> 4) & 0x000F;  
	byLenCheckSum += (wLen >> 8) & 0x000F;  
	byLenCheckSum %= 16;                    
	byLenCheckSum = (~byLenCheckSum) + 1;   
	byLenCheckSum &= 0x0F;                  

	return byLenCheckSum;
}

USHORT CalcSendCheckSum(BYTE* pSendBuf, USHORT dataLen)
{
	USHORT i = 0;
	USHORT IICRC;
	IICRC = 0;
	for (i = 0; i < dataLen; i++)
	{
		IICRC += pSendBuf[i];
	}
	IICRC = IICRC % 65536;

	return ((~IICRC)+1);
}

void Make_YDN_Lengh(IN UINT nLen, OUT BYTE* pbyOut)
{
	BYTE lenChk = LenCheckSum(nLen);
	pbyOut[0] = HexToAscii((lenChk & 0x0f));
	pbyOut[1] = HexToAscii(((nLen)>>8) & 0x0f);
	pbyOut[2] = HexToAscii(((nLen) & 0xf0)>>4);
	pbyOut[3] = HexToAscii((nLen) & 0x0f);
}

VAR_VALUE Ascii8ToVarV(BYTE *byInData)
{
	//little endian to big endian
	VAR_VALUE varV;
	varV.abyValue[0] = Ascii2ToHex(byInData + 0);
	varV.abyValue[1] = Ascii2ToHex(byInData + 2);
	varV.abyValue[2] = Ascii2ToHex(byInData + 4);
	varV.abyValue[3] = Ascii2ToHex(byInData + 6);

	//TRACEX("8 byte is[%02X-%02X-%02X-%02X], and converted fVal = %f, ulVal = %d.\n",
	//	varV.abyValue[0], varV.abyValue[1], varV.abyValue[2], varV.abyValue[3],
	//	varV.fValue, varV.ulValue);

	return varV;
}

short Ascii4ToShort(BYTE *byInData)
{
	//little endian to big endian
	SHORT_VAR var;
	var.byVal[1] = Ascii2ToHex(byInData + 0);
	var.byVal[0] = Ascii2ToHex(byInData + 2);
	 
	return var.sVal;
 }


#define  INIT_CAB_SAFE_STATE_MACHINE(p, _emState, _pfnOnAct) \
			((p)->emState = (_emState), \
			(p)->pfnAction = (_pfnOnAct), \
			(p)++)

/*********************************************************************************
*  
*  FUNCTION NAME : Event_Handler
*  CREATOR       :  
*  CREATED DATE  : 2020-11-14 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE		emEvType,
	_EVENT_MSG		emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int				nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	RS485_2_SAMPLER_DATA* pDev_485_2_Rough = (RS485_2_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	ERRCDDE_DPR err;
	VAR_VALUE 	varV;

	TRACE("-------485_2 got EVENT[%d] nEvDataSize=[%d]--------\n", emEvType, nEvDataSize);
	 
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
			varV.blobValue = (BYTE *)&pDev_485_2_Rough->blobEmsCfgPlan;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("Ems config Plan Changed success!!!\n");
		}
	}
	return EVENT_HANDED_OK;
}
 
		
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;

	pSampler->pDataHeap = (void*)NEW(RS485_2_SAMPLER_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

	RS485_2_SAMPLER_DATA  *pDev_485_2_Rough = (RS485_2_SAMPLER_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pDev_485_2_Rough, 1);

 	pDev_485_2_Rough->hSampler = hSamplerSelf;
	pDev_485_2_Rough->nSamplerId = pSampler->info.nSamplerID;
	pDev_485_2_Rough->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	pDev_485_2_Rough->hSamplerLock = pSampler->hSamplerLock;
	

	IMEU_InitRoughData_V20(pDev_485_2_Rough);//IMEU2 SW V20 Initial	

	//kwhMeter_InitRoughData(pDev_485_2_Rough);//init special rect signals

	//Init the fire state machine
    CAB_SAFE_STATE_MACHINE_ITEM *p = &pDev_485_2_Rough->StateMachineLoop[0];
    INIT_CAB_SAFE_STATE_MACHINE(p, 	CAB_SAFE_STATE_IDLE,	OnIdle_CabSafeStage);
    //INIT_CAB_SAFE_STATE_MACHINE(p, CAB_SAFE_STATE_FIRST,	OnFirstStage_CabSafeStage);
    INIT_CAB_SAFE_STATE_MACHINE(p, 	CAB_SAFE_STATE_SECOND,	OnSecond_CabSafeStage);
    INIT_CAB_SAFE_STATE_MACHINE(p, 	CAB_SAFE_STATE_THIRD,	OnThird_CabSafeStage);
    //INIT_CAB_SAFE_STATE_MACHINE(p, CAB_SAFE_STATE_FOURTH,	OnFourthStage_CabSafeStage);
     
	//Open comm port
	int nErrCode = 0;
	// pDev_485_2_Rough->hPort = pSampler->pUsedCommPort->Open(
	// 	pSampler->pUsedCommPort->info.szPortAttri, 
	// 	pSampler->info.szSamplerSetting,
	// 	COMM_CLIENT_MODE, 1000, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		return FALSE;
	}

	//Clear buffer
	// pSampler->pUsedCommPort->Control(pDev_485_2_Rough->hPort,
	// 	COMM_PURGE_RXCLEAR, NULL, 0);

	pDev_485_2_Rough->emESSDevType = GetESSDeviceType_Sampler(hSamplerSelf);
	pDev_485_2_Rough->emBattFaultStatu = STBATT_ALMSTAT_NOMAL;
	ZERO_POBJS(&pDev_485_2_Rough->esRunData, 1);
	pDev_485_2_Rough->t_Interval_time = _NOW_ - 10;
	pDev_485_2_Rough->emLED1LastState = GREEN;
	pDev_485_2_Rough->emLED2LastState = GREEN;

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},

	};
	int n = 0;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pDev_485_2_Rough->pDPR->Register_EvHandler("4852_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

void RefreshSystemRunState(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough)
{
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)((SAMPLER*)pDev_485_2_Rough->hSampler)->hDPR;
    VAR_VALUE varV;
	VAR_VALUE varVBat;
	A_BLOB_SINGLE_STBATT_RUNINFO blobRunInfo;
	ZERO_POBJS(&blobRunInfo, 1);

	STBATT_ALARM_STATU 	emBattStatu = STBATT_ALMSTAT_NOMAL;

	int n = 0;
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		varVBat.blobValue = (BYTE *)&blobRunInfo;
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 501 + n, 551, &varVBat, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		if(blobRunInfo.emBattStatus_Alm == STBATT_ALMSTAT_FAULT)
		{
			emBattStatu = STBATT_ALMSTAT_FAULT;
			break;
		}

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 501 + n, 553, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		if(varV.emValue == 3)
		{
			emBattStatu = STBATT_ALMSTAT_FAULT;
			break;
		}
	}
	pDev_485_2_Rough->emBattFaultStatu = emBattStatu;

	varV.blobValue = (BYTE *)&pDev_485_2_Rough->esRunData;
    pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 200, 80, &varV, sizeof(A_BLOB_ES_RUN_DATA));


	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 200, 103, &varV, 4);   //CCU系统状态
	pDev_485_2_Rough->emCCUState = varV.emValue;
	//TRACEI("pWhim_Rough->emCCUState  %d", pWhim_Rough->emCCUState);

	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 200, 161, &varV, 4);  //ABU STS 并离网状态
	pDev_485_2_Rough->bySTSOffGrid = varV.emValue;
	//TRACEI("pWhim_Rough->bySTSOffGrid  %d", pWhim_Rough->bySTSOffGrid);

	return;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	return;
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	RS485_2_SAMPLER_DATA *pDev_485_2_Rough = (RS485_2_SAMPLER_DATA*)pSampler->pDataHeap;

	/*Usr Load AC Meter Sampler John 
	pDev_485_2_Rough->nSampACMeterCycle++;
	if(pDev_485_2_Rough->nSampACMeterCycle < 2)
	{
		//sample IMEU2 V20 with RS485-2
		IMEU2_Sample_V20(pDev_485_2_Rough);	
	}
	else if(pDev_485_2_Rough->nSampACMeterCycle < 4)
	{
		ACMeter_Sample(pDev_485_2_Rough);
	}
	else if(pDev_485_2_Rough->nSampACMeterCycle >= MIN_SAMP_IMEU2_PERIOD)
	{
		pDev_485_2_Rough->nSampACMeterCycle = 0;
	}
	*/
	//sample IMEU2 V20 with RS485-2
	//printf("111111111111111111111111111111");
	if(pDev_485_2_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
	{
		RefreshSystemRunState(pDev_485_2_Rough);
		LED_RS485_Ctl_State(pDev_485_2_Rough);
	}

	VAR_VALUE varV;
	pDev_485_2_Rough->pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601, 450, &varV, 4);

	IMEU_TYPE devType  = (IMEU_TYPE)varV.emValue;
	if (IMEU_TYPE_IMEU2 == devType)
	{
		IMEU2_Sample_V20(pDev_485_2_Rough);
	}

	return ERR_SAMPLE_OK;
}
 

/*
Setting : 46
7E323142383731343436 30 30 41 38 31 30 31 31 30 00 00 00 00 46 43 35 31 0D
*/
DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	RS485_2_SAMPLER_DATA *pDev_485_2_Rough = (RS485_2_SAMPLER_DATA*)pSampler->pDataHeap;
	//double 	tmNow = GetCurrentTime();
	ERRCODE_SAMPLER nRet = ERR_CONTROL_OK;
	//int 	nWhichIMEU = 0;
 
	TRACE("nDevId =%d nParamId=%d \n", nDevId, nParamId);

	//if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	//{
	//	TRACEX("Mutex_Lock Error\n");
	//	return ERR_SAMPLE_NEED_RESAMP;
	//}

	TRACEX("Info: IMEU Sett Cmd emParaType=[%d], nDevId=[%d], nParamId=[%d], nValSize=[%d], emValue=[%d]\n", \
		emParaType , nDevId, nParamId, nValSize, pVarVal->emValue);
	if((nDevId == 601) || (nDevId == 602)||(nDevId == 603)||(nDevId == 604))
	{
		nRet = IMEU_SendCtrl_V20(pDev_485_2_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
		//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
 	}
	/*else if((nDevId >= 401) && (nDevId <= 406))  //John 2021-12-18 Sample UserLoad ACMeter DataInfo
	{
		nRet = kwhMeter_SendCtrl(pDev_485_2_Rough, emParaType, nDevId, nParamId, pVarVal, nValSize, nTimeOut);
	}*/
	//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	return nRet;
 }



