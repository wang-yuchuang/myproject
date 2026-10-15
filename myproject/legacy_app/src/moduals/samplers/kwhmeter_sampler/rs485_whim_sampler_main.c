// #define _TRACE_PRINT_
#include "rs485_whim_sampler_main.h"

COMMPORT * 	s_pRS485_2_Comm = NULL;
HANDLE      hRS485_2_Port;
COMMPORT * 	s_pUartServer_Comm = NULL;
HANDLE      hUartServer_Port;
static double LastSentPostTime;
static int count;

#define RS485_SAMPLER_PERIOD 100

//RS485_1_SAMPLER_DATA		g_RS485_1SamplerData;
//static		HANDLE				s_hPort485_2;
//static		SAMPLER_INFO			*s_pSamplerInfo_1;

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	ERRCDDE_DPR err;

	TRACEI("Event_Handler--got EVENT[%d] emMsgType=[%d]--------\n", emEvType, emMsgType);
	if(emEvType == EVENT_TYPE_CFG_CHANGED)
	{
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;
		if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 40 || pCfgChange->parmPTR.nSubId == 41))
		{
			// LJH241007 AntiPhase 1, 40, 1, 41
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, pCfgChange->parmPTR.nSubId, &varV, 4);
			if (pCfgChange->parmPTR.nSubId == 40)
			{
				pWhim_Rough->needAntiReflux_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			}
			else if (pCfgChange->parmPTR.nSubId == 41)
			{
				pWhim_Rough->needAntiOverLoad_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			}
			TRACED("rs485_whim_sampler_main::Event_Handler needAnti***_Phase Changed nId:%d nSubId:%d AntiReflux:%d %d err:%d",
				pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, pWhim_Rough->needAntiReflux_Phase, pWhim_Rough->needAntiOverLoad_Phase, err);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("A_BLOB_CHARGE_FEESET Changed success!!!\n");
		}
		else if ((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 26))
		{
			// printf("A_BLOB_ES_BACKEND_CFG blobBackendCfg Changed, Do Update!\n");

			A_BLOB_ES_BACKEND_CFG 			blobBackendCfg;
 			varV.blobValue = (BYTE *)&blobBackendCfg;
			//copy setting to current data
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			if (blobBackendCfg.bEnableQuickResponseChannel != pWhim_Rough->bEnableQuickResChannel)
			{
				pWhim_Rough->bEnableQuickResChannel = blobBackendCfg.bEnableQuickResponseChannel;
				LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_INFO,
					"bEnableQuickResChannel Changed --> [%d] !",pWhim_Rough->bEnableQuickResChannel);
			}
			// printf("pWhim_Rough->bEnableQuickResChannel = [%d]\n",pWhim_Rough->bEnableQuickResChannel);
			TRACE("A_BLOB_ES_BACKEND_CFG Changed success!!!\n");
		}
		else if ((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 107))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			pWhim_Rough->nMeterInterrAlarmNums = ((err==ERR_DPR_OK) ? varV.ulValue : 10);
			TRACED("nMeterInterrAlarmNums changed to [%d]\n",pWhim_Rough->nMeterInterrAlarmNums);
		}
		else if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 110))
		{
			err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR, pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId, &varV, 4);
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			pWhim_Rough->tagGridApplyScenery = (GRID_APPLY_SCENERY)varV.emValue;
			TRACED("tagGridApplyScenery changed to [%d]\n",pWhim_Rough->tagGridApplyScenery);
		}
	}
	else if(emEvType == EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW) //restore resere info
	{
		if((nEvDataSize != sizeof(EVENT_METER_SAVE_DATA)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			TRACEE("nEvDataSize=%d EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFWSize=%d\n", nEvDataSize, sizeof(EVENT_METER_SAVE_DATA));
			return EVENT_HANDED_FAILED;
		}

		EVENT_METER_SAVE_DATA* pMeterKwhSave = (EVENT_METER_SAVE_DATA*)pEvData;
		if(pMeterKwhSave != NULL)
		{
			if (pMeterKwhSave->ndevid == 407)  //only 407 need save current SPFW kwh
			{
				pWhim_Rough->meterData[CAB_FIRST].nLastYearToSaveDB = pMeterKwhSave->nYear;   
				pWhim_Rough->meterData[CAB_FIRST].nLastMonthToSaveDB = pMeterKwhSave->nMonth;
				pWhim_Rough->meterData[CAB_FIRST].nLastDayToSaveDB = pMeterKwhSave->nDay;
				TRACEI("Event_Handler-----received restore event DBLASTYear = [%d] DBLASTMonth = [%d] DBLASTDay = [%d] -----------\n",
					pWhim_Rough->meterData[CAB_FIRST].nLastYearToSaveDB,
					pWhim_Rough->meterData[CAB_FIRST].nLastMonthToSaveDB,
					pWhim_Rough->meterData[CAB_FIRST].nLastDayToSaveDB);
			}
		}
	}
	return EVENT_HANDED_OK;
}

//如果任意两相之间的功率差异超过一定的比例（1.5 或 0.5），就认为三相不平衡
static BOOL Judge_PhaseValue_Unbalance(float fPh_A, float fPh_B, float fPh_C)
{
    float fRate[3];
    fRate[0] = fPh_A;
    fRate[1] = fPh_B;
    fRate[2] = fPh_C;
    BOOL bRet = METERVALUE_OK;

    int i = 0, j = 0;
    for(i = 0; i < 3; i++)
    {
        TRACE("i=[%d] bRet=[%d]\n", i, bRet);
        for(j = i+1; j < 3; j++)
        {
            float fTemp = fRate[i]/(fRate[j]+0.1);
            
            TRACE("fTemp=[%.2f]\n", fTemp);

            if((fTemp >= 1.5) || (fTemp <= 0.5)) 
            {
                bRet = METERVALUE_ACTPower_UNBANLANCE;
                return bRet;
            }
        }
    }

    return bRet;
}

static BOOL Judge_ActvPower_Abnml(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{
	float fPwr_Sum = 0,  fPwr_A = 0,  fPwr_B = 0,  fPwr_C = 0;
	float fPhA_V = 0,  fPhB_V = 0,  fPhC_V = 0;

	fPhA_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_A].fValue;  
	fPhB_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_B].fValue;  
	fPhC_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_C].fValue;
	if((fPhA_V < 50.0) || (fPhB_V < 50.0) || (fPhC_V < 50.0)) //Lock Phase
	{
		return METERVALUE_VOLT_UNBANLANCE;
	}

	fPwr_Sum = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue;
	fPwr_A = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhA].fValue;
	fPwr_B = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhB].fValue;
	fPwr_C = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhC].fValue;

    float fActPower = fabs(fPwr_Sum);
    if(fActPower > 10.0)
    {
        return Judge_PhaseValue_Unbalance(fPwr_A, fPwr_B, fPwr_C);
    }

    return METERVALUE_OK;
}

void* _thread_RS485_1_Sampler(void* pArgs)
{
	// printf("I AM _thread_RS485_1_Sampler !!\n");
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;

	VAR_VALUE varV;
	int nErrrCode;
	int nRet = 0;
	// int nMaxCommFailTimes;
	int n;
	int kkk;
	UINT nDeviceID = 0;
	int nCommFailMeterNum = 0;

#ifdef IMMU3
	int portId = 104; // BMU03 /dev/ttyO4
	static COMMPORT sMeter_UsedCommPort485;
	pSampler->pUsedCommPort = &sMeter_UsedCommPort485;
	if (GetCommPort_ById((HANDLE)pSampler, portId, pSampler->pUsedCommPort) == NULL)
	{
		pSampler->pUsedCommPort = NULL;
		printf("Open_CommPort_485:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}
#endif

	//Open comm port
	pWhim_Rough->hPort = pSampler->pUsedCommPort->Open(
		pSampler->pUsedCommPort->info.szPortAttri, 
		pSampler->info.szSamplerSetting,
		COMM_CLIENT_MODE, 1000, &nErrrCode);

	if(ERR_COMM_OK != nErrrCode)
	{
		return TRUE;
	}

#ifdef IMMU3
	pSampler->pUsedCommPort->Control(pWhim_Rough->hPort, 
		COMM_SET_SERIAL_485, NULL, 0);
#endif

	//Clear buffer
	pSampler->pUsedCommPort->Control(pWhim_Rough->hPort,
		COMM_PURGE_RXCLEAR, NULL, 0);
	pSampler->pUsedCommPort->Control(pWhim_Rough->hPort,
		COMM_PURGE_TXCLEAR, NULL, 0);

	while (!pSampler->bExit)
	{
		// if(pWhim_Rough->bEnableQuickResChannel)
		// {
		// 	nMaxCommFailTimes = 1;
		// }
		// else
		// {
		// 	nMaxCommFailTimes = 10;
		// }

		for(n = 0; n < MAX_WH_NUM; n++)
		{
			nDeviceID = All_TypesMetersID[n / METER_TYPE_NUM][n % METER_TYPE_NUM];
			if (nDeviceID == 0)
			{
				continue;
			}
			
			if( (pWhim_Rough->meterData[n].bPhysicalInterType != RS485_1) 
				||(pWhim_Rough->meterData[n].emMeterType == METER_TYPE_NotInstalled) 
				|| (pWhim_Rough->meterData[n].emMeterType == METER_TYPR_DataFromMaster))
			{
				continue;
			}
			nErrrCode = whMeter_Sample(pWhim_Rough, n);

			if(ERR_SAMPLE_OK != nErrrCode)
			{
				pWhim_Rough->meterData[n].nInterrupt_Times++;
				if(pWhim_Rough->meterData[n].nInterrupt_Times >= pWhim_Rough->nMeterInterrAlarmNums)//MAX_RS4851_TRY_TIMES
				{
					UINT nCurrGirdMeterId = 0;
					if(pWhim_Rough->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
					{
						pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 60, &varV, 4);
						nCurrGirdMeterId = varV.ulValue;
						if(nCurrGirdMeterId != nDeviceID)
						{
							SetCommStatus((HANDLE)pSampler, nDeviceID, TRUE); // 非主路表通信异常也不报
						}
					}

					pWhim_Rough->meterData[n].nInterrupt_Times = 0;
					if(!pWhim_Rough->meterData[n].bIsCommFail)
					{
						pWhim_Rough->meterData[n].bIsCommFail = TRUE;
						// pWhim_Rough->meterFaultRecBuffer.isActive = TRUE;
						TRACEX("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
						if(pWhim_Rough->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
						{

							if(nCurrGirdMeterId == nDeviceID)
							{
								SetCommStatus((HANDLE)pSampler, nDeviceID, FALSE);
							}
						}
						else
						{
							SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, FALSE);
						}
						// printf("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
						for(kkk = 0; kkk < _MAX_WH_ROUGH; kkk++)
						{
							pWhim_Rough->meterData[n].varWHMeterRough[kkk].lValue = 0;
						}
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
						//return nErrrCode;
					}
				}
			}
			else 
			{
				// Added By John For MeterValue SelfCheck.modify by lpl 240927 
				nRet = Judge_ActvPower_Abnml(pWhim_Rough, n);
				if(nRet == METERVALUE_VOLT_UNBANLANCE)  //Single-phase voltage below 50V
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes++;
					if(pWhim_Rough->meterData[n].nUnderVoltTimes > 30)
					{
						pWhim_Rough->meterData[n].nUnderVoltTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_VOLT_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
				}
				else if(nRet == METERVALUE_ACTPower_UNBANLANCE) //Three-phase active power imbalance
				{
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes++;
					if(pWhim_Rough->meterData[n].nPhaseUnbalanceTimes > 30)
					{
						pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_ACTPower_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;//证明检测电压已通过
				}
				else if(nRet == METERVALUE_OK)
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
					pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
				}

				pWhim_Rough->bLast_AllMeterCommOK = TRUE;
				if(pWhim_Rough->meterData[n].bIsCommFail)
				{
					pWhim_Rough->meterData[n].bIsCommFail = FALSE;
					SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, TRUE);
				}
				pWhim_Rough->meterData[n].nInterrupt_Times = 0;
			}
		}

		Sleep(RS485_SAMPLER_PERIOD); 
		
	}
}

void* _thread_RS485_2_Sampler(void* pArgs)
{
	// printf("I AM _thread_RS485_2_Sampler !!\n");
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;
	
	VAR_VALUE varV;
	int nErrrCode;
	int nRet = 0;
	// int nMaxCommFailTimes;
	int n;
	int kkk;

#ifdef IMMU3
	int portId = 101; // BMU03 /dev/ttyO1
#else
	int portId = 4;
#endif
	UINT nDeviceID = 0;

	//Open comm port
	s_pRS485_2_Comm = NEW(COMMPORT,1);
    if (s_pRS485_2_Comm == NULL) 
	{
        TRACEE("s_pRS485_2_Comm Memory allocation failed\n");
        return TRUE;
    }

	if (GetCommPort_ById((HANDLE)pSampler, portId, s_pRS485_2_Comm) == NULL)
	{
		TRACEE("_thread_RS485_2_Sampler:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}

#ifdef IMMU3
	hRS485_2_Port = s_pRS485_2_Comm->Open(
		"ttyO1", 
		"9600,N,8,1",
		COMM_CLIENT_MODE, 1000, &nErrrCode);	
#else
	hRS485_2_Port = s_pRS485_2_Comm->Open(
		"ttyS4", 
		"9600,N,8,1",
		COMM_CLIENT_MODE, 1000, &nErrrCode);
#endif

	if(ERR_COMM_OK != nErrrCode)
	{
		return TRUE;
	}

#ifdef IMMU3
	s_pRS485_2_Comm->Control(hRS485_2_Port, COMM_SET_SERIAL_485, NULL, 0);
#endif

	//Clear buffer
	s_pRS485_2_Comm->Control(hRS485_2_Port, COMM_PURGE_RXCLEAR, NULL, 0);
	s_pRS485_2_Comm->Control(hRS485_2_Port,	COMM_PURGE_TXCLEAR, NULL, 0);

	while (!pSampler->bExit)
	{
		// if(pWhim_Rough->bEnableQuickResChannel)
		// {
		// 	nMaxCommFailTimes = 1;
		// }
		// else
		// {
		// 	nMaxCommFailTimes = 10;
		// }

		for(n = 0; n < MAX_WH_NUM; n++)
		{
			nDeviceID = All_TypesMetersID[n / METER_TYPE_NUM][n % METER_TYPE_NUM];
			if (nDeviceID == 0)
			{
				continue;
			}
			
			if( (pWhim_Rough->meterData[n].bPhysicalInterType != RS485_2) 
				||(pWhim_Rough->meterData[n].emMeterType == METER_TYPE_NotInstalled) 
				|| (pWhim_Rough->meterData[n].emMeterType == METER_TYPR_DataFromMaster))
			{
				continue;
			}

			nErrrCode = whMeter_Sample(pWhim_Rough, n);

			if(ERR_SAMPLE_OK != nErrrCode)
			{
				pWhim_Rough->meterData[n].nInterrupt_Times++;
				if(pWhim_Rough->meterData[n].nInterrupt_Times >= pWhim_Rough->nMeterInterrAlarmNums)//MAX_RS4851_TRY_TIMES
				{
					UINT nCurrGirdMeterId = 0;
					if(pWhim_Rough->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
					{
						pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 60, &varV, 4);
						nCurrGirdMeterId = varV.ulValue;
						if(nCurrGirdMeterId != nDeviceID)
						{
							SetCommStatus((HANDLE)pSampler, nDeviceID, TRUE); // 非主路表通信异常也不报
						}
					}

					pWhim_Rough->meterData[n].nInterrupt_Times = 0;
					if(!pWhim_Rough->meterData[n].bIsCommFail)
					{
						pWhim_Rough->meterData[n].bIsCommFail = TRUE;
						// pWhim_Rough->meterFaultRecBuffer.isActive = TRUE;
						TRACEX("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
							UINT nCurrGirdMeterId = 0;
							pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 60, &varV, 4);
							nCurrGirdMeterId = varV.ulValue;

						if(pWhim_Rough->tagGridApplyScenery == SCENERY_ACTIVE_STANDBY)
						{
							if(nCurrGirdMeterId == nDeviceID)
							{
								SetCommStatus((HANDLE)pSampler, nDeviceID, FALSE);
							}
						}
						else
						{
							SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, FALSE);
						}
						// printf("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
						for(kkk = 0; kkk < _MAX_WH_ROUGH; kkk++)
						{
							pWhim_Rough->meterData[n].varWHMeterRough[kkk].lValue = 0;
						}
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
						//return nErrrCode;
					}
				}
			}
			else 
			{
				// Added By John For MeterValue SelfCheck.modify by lpl 240927 
				nRet = Judge_ActvPower_Abnml(pWhim_Rough, n);
				if(nRet == METERVALUE_VOLT_UNBANLANCE)  //Single-phase voltage below 50V
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes++;
					if(pWhim_Rough->meterData[n].nUnderVoltTimes > 30)
					{
						pWhim_Rough->meterData[n].nUnderVoltTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_VOLT_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
				}
				else if(nRet == METERVALUE_ACTPower_UNBANLANCE) //Three-phase active power imbalance
				{
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes++;
					if(pWhim_Rough->meterData[n].nPhaseUnbalanceTimes > 30)
					{
						pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_ACTPower_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;//证明检测电压已通过
				}
				else if(nRet == METERVALUE_OK)
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
					pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
				}

				pWhim_Rough->bLast_AllMeterCommOK = TRUE;
				if(pWhim_Rough->meterData[n].bIsCommFail)
				{
					pWhim_Rough->meterData[n].bIsCommFail = FALSE;
					SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, TRUE);
				}
				pWhim_Rough->meterData[n].nInterrupt_Times = 0;
			}
		}
		
		Sleep(RS485_SAMPLER_PERIOD); 
		
	}
}

int ReOpen_Comm_Port(RS485_SAMPLER_DATA * pWhim_Rough)
{
	int nErrrCode;
	SAMPLER* pSampler = (SAMPLER*)pWhim_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	int portId = 7;
	char szEndUrl[256] = "192.168.1.202:8899";
	
	//Open comm port
	if (GetCommPort_ById((HANDLE)pSampler, portId, s_pUartServer_Comm) == NULL)
	{
		TRACEE("_thread_UartServer_Sampler:GetCommPort_ById:%d failed", portId);
		return FALSE;
	}

	if(hUartServer_Port != NULL)
	{
		nErrrCode = s_pUartServer_Comm->Close(hUartServer_Port);
		TRACEE("Close The Comm Port errcode:%d\n", nErrrCode);

		hUartServer_Port = NULL;
	}
	else
	{
		TRACEW("hUartServer_Port == NULL!!!\n");
	}

	hUartServer_Port = s_pUartServer_Comm->Open(
		"kwhmeter_UartServer", 
		szEndUrl,
		COMM_CLIENT_MODE, 1000, &nErrrCode);
	
	// TRACEE("Open_Comm_Port uartServer Or Rs485_2 nErrrCode = %d !!!\n",nErrrCode);
	if(ERR_COMM_OK != nErrrCode)
	{
		pWhim_Rough->bUartServerCommOk = FALSE;
		// TRACEE("PV_INVERTER_SAMPLER :OpenCommPort:%d failed", nErrrCode);
		return FALSE;
	}

	pWhim_Rough->bUartServerCommOk = TRUE;

	//Clear buffer
	s_pUartServer_Comm->Control(hUartServer_Port, COMM_PURGE_RXCLEAR, NULL, 0);

	return TRUE;

}

void* _thread_UartServer_Sampler(void* pArgs)
{
	// printf("I AM _thread_UartServer_Sampler !!\n");
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;
	
	VAR_VALUE varV;
	int nErrrCode;
	int nRet = 0;
	// int nMaxCommFailTimes;
	int n;
	int kkk;
	UINT nDeviceID = 0;
	// int portId = 7;
	// char szEndUrl[256] = "192.168.1.202:8899";
	

	// //Open comm port
	// s_pUartServer_Comm = NEW(COMMPORT,1);
    // if (s_pUartServer_Comm == NULL) 
	// {
    //     TRACEE("s_pUartServer_Comm Memory allocation failed\n");
    //     return TRUE;
    // }

	// if (GetCommPort_ById((HANDLE)pSampler, portId, s_pUartServer_Comm) == NULL)
	// {
	// 	TRACEE("_thread_UartServer_Sampler:GetCommPort_ById:%d failed", portId);
	// 	return FALSE;
	// }

	// hUartServer_Port = s_pUartServer_Comm->Open(
	// 	"kwhmeter_UartServer", 
	// 	szEndUrl,
	// 	COMM_CLIENT_MODE, 1000, &nErrrCode);
	// if(ERR_COMM_OK != nErrrCode)
	// {
	// 	return TRUE;
	// }

	while (!pSampler->bExit)
	{
		if (!pWhim_Rough->bUartServerCommOk)
		{
			if (!ReOpen_Comm_Port(pWhim_Rough))
			{
				//alarm
				varV.emValue = 1;
				Push_VarValue(SETTYPE_RAW_SP, pSampler, 901, 3950, varV);

				Sleep(900); 	//Reopen every 1s
				return ERR_SAMPLE_FAILED;
			}
			else
			{
				varV.emValue = 0;
				Push_VarValue(SETTYPE_RAW_SP, pSampler, 901, 3950, varV);
			}
		}

		// if(pWhim_Rough->bEnableQuickResChannel)
		// {
		// 	nMaxCommFailTimes = 1;
		// }
		// else
		// {
		// 	nMaxCommFailTimes = 10;
		// }

		for(n = 0; n < MAX_WH_NUM; n++)
		{
			nDeviceID = All_TypesMetersID[n / METER_TYPE_NUM][n % METER_TYPE_NUM];
			if (nDeviceID == 0)
			{
				continue;
			}
			
			if( (pWhim_Rough->meterData[n].bPhysicalInterType != UART_SERVER) 
				||(pWhim_Rough->meterData[n].emMeterType == METER_TYPE_NotInstalled) 
				|| (pWhim_Rough->meterData[n].emMeterType == METER_TYPR_DataFromMaster))
			{
				continue;
			}

			nErrrCode = whMeter_Sample(pWhim_Rough, n);

			if(ERR_SAMPLE_OK != nErrrCode)
			{
				pWhim_Rough->meterData[n].nInterrupt_Times++;
				if(pWhim_Rough->meterData[n].nInterrupt_Times >= pWhim_Rough->nMeterInterrAlarmNums)//MAX_RS4851_TRY_TIMES
				{
					pWhim_Rough->meterData[n].nInterrupt_Times = 0;
					if(!pWhim_Rough->meterData[n].bIsCommFail)
					{
						pWhim_Rough->meterData[n].bIsCommFail = TRUE;
						// pWhim_Rough->meterFaultRecBuffer.isActive = TRUE;
						TRACEX("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
						SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, FALSE);
						// printf("Now set kwMeterSampler[%d] --> %d commFailed.\n", pSampler->info.nSamplerID, n);
						for(kkk = 0; kkk < _MAX_WH_ROUGH; kkk++)
						{
							pWhim_Rough->meterData[n].varWHMeterRough[kkk].lValue = 0;
						}
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
						//return nErrrCode;
					}
					pWhim_Rough->bUartServerCommOk = FALSE;
				}
			}
			else 
			{
				// Added By John For MeterValue SelfCheck.modify by lpl 240927 
				nRet = Judge_ActvPower_Abnml(pWhim_Rough, n);
				if(nRet == METERVALUE_VOLT_UNBANLANCE)  //Single-phase voltage below 50V
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes++;
					if(pWhim_Rough->meterData[n].nUnderVoltTimes > 30)
					{
						pWhim_Rough->meterData[n].nUnderVoltTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_VOLT_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
				}
				else if(nRet == METERVALUE_ACTPower_UNBANLANCE) //Three-phase active power imbalance
				{
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes++;
					if(pWhim_Rough->meterData[n].nPhaseUnbalanceTimes > 30)
					{
						pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 30;
						pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_ACTPower_UNBANLANCE;
					}
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;//证明检测电压已通过
				}
				else if(nRet == METERVALUE_OK)
				{
					pWhim_Rough->meterData[n].nUnderVoltTimes = 0;
					pWhim_Rough->meterData[n].nPhaseUnbalanceTimes = 0;
					pWhim_Rough->meterData[n].emSelfCheckCode = METERVALUE_OK;
				}

				pWhim_Rough->bLast_AllMeterCommOK = TRUE;
				if(pWhim_Rough->meterData[n].bIsCommFail)
				{
					pWhim_Rough->meterData[n].bIsCommFail = FALSE;
					SetCommStatus((HANDLE)pSampler, /*((pSampler->info.nSamplerID == 3) ? 408 : 410)*/ nDeviceID, TRUE);
				}
				pWhim_Rough->meterData[n].nInterrupt_Times = 0;
			}
		}
		
		Sleep(RS485_SAMPLER_PERIOD); 
		
	}
}


DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	int kkk = 0;

	pSampler->pDataHeap = (void*)NEW(RS485_SAMPLER_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pWhim_Rough, 1);

	pWhim_Rough->hSampler = hSamplerSelf;
	pWhim_Rough->nSamplerId = pSampler->info.nSamplerID;

	//Initialize rough data and some parameter. 
	whMeter_InitRoughData(pWhim_Rough);//init special rect signals

	LastSentPostTime = GetCurrentTime();

	memset(pWhim_Rough->abySendBuf, 0, sizeof(pWhim_Rough->abySendBuf));
	memset(pWhim_Rough->abyRcvBuf, 0, sizeof(pWhim_Rough->abyRcvBuf));
	memset(pWhim_Rough->nReadBuffBytes, 0, sizeof(pWhim_Rough->nReadBuffBytes));

	// // 故障上下报文记录功能
	// memset(pWhim_Rough->meterRecCirBuffer.buffer, 0, sizeof(pWhim_Rough->meterRecCirBuffer.buffer));
	// memset(pWhim_Rough->meterRecCirBuffer.dataLen, 0, sizeof(pWhim_Rough->meterRecCirBuffer.dataLen));
	// pWhim_Rough->meterRecCirBuffer.count = 0;
	// pWhim_Rough->meterRecCirBuffer.head = 0;
	// pWhim_Rough->meterRecCirBuffer.tail = 0;
	
	// memset(pWhim_Rough->meterFaultRecBuffer.buffer, 0, sizeof(pWhim_Rough->meterFaultRecBuffer.buffer));
	// memset(pWhim_Rough->meterFaultRecBuffer.dataLen, 0, sizeof(pWhim_Rough->meterFaultRecBuffer.dataLen));
	// pWhim_Rough->meterFaultRecBuffer.count = 0;
	// pWhim_Rough->meterFaultRecBuffer.isActive = FALSE;
	// pWhim_Rough->meterFaultRecBuffer.bTrigger = FALSE;
	// pWhim_Rough->tm_fLastRecMeterFaultLog = 0;

	// LJH241007 AntiPhase
	ERRCODE_DAI err;
	VAR_VALUE varV;
	err = pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR, 1, 40, &varV, 4);
	pWhim_Rough->needAntiReflux_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);

	err = pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR, 1, 41, &varV, 4);
	pWhim_Rough->needAntiOverLoad_Phase = ((err==ERR_DPR_OK) ? varV.emValue : FALSE);

	A_BLOB_ES_BACKEND_CFG 			blobBackendCfg;
	varV.blobValue = (BYTE *)&blobBackendCfg;
	//copy setting to current data
	err = pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
	if(err != ERR_DPR_OK)
	{
		return EVENT_HANDED_FAILED;
	}
	// if (blobBackendCfg.bEnableQuickResponseChannel != pWhim_Rough->bEnableQuickResChannel)
	{
		pWhim_Rough->bEnableQuickResChannel = blobBackendCfg.bEnableQuickResponseChannel;
		LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_INFO,
				"bEnableQuickResChannel Init --> [%d] !",pWhim_Rough->bEnableQuickResChannel);
	}
	// printf("init pWhim_Rough->bEnableQuickResChannel = [%d]\n",pWhim_Rough->bEnableQuickResChannel);

	err = pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR, 1, 107, &varV, 4);
	pWhim_Rough->nMeterInterrAlarmNums = ((err == ERR_DPR_OK) ? varV.emValue : 10);
	TRACED("Init nMeterInterrAlarmNums = [%d]\n",pWhim_Rough->nMeterInterrAlarmNums);

	pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR, 1, 110, &varV, 4);
	pWhim_Rough->tagGridApplyScenery = (GRID_APPLY_SCENERY)varV.emValue;

	/*
	pWhim_Rough->bNeedSampleIMEU = 
		(ESSYSTYPE_MULTIMAIN == GetESSysType(hSamplerSelf)) ? TRUE : FALSE;

	if((pWhim_Rough->bNeedSampleIMEU) && (pWhim_Rough->nSamplerId != 3))
	{
		IMEU_InitRoughData(pWhim_Rough);//init special rect signals		
	}
	*/

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN}, // LJH241007 AntiPhase
		{EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW,	EVENT_MSG_RESTORE},
	};
	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pDPR->Register_EvHandler("KWHMeter_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}
	

	pWhim_Rough->nSemCCUWith485Meter = Creat_SemID(SEM_CCU_485_PATH, SEM_CCU_485_PROJ_ID, 1, IPC_CREAT | 0666);
	TRACED("whMeterSampler semid = [%d]\n",pWhim_Rough->nSemCCUWith485Meter);
	LogOut(MAIN_MODULE_MAIN, "whMeterSampler", LOG_TYPE_INFO,
		"whMeterSampler semid --> [%d] !\n",pWhim_Rough->nSemCCUWith485Meter);
	if (pWhim_Rough->nSemCCUWith485Meter == -1)
	{
		LogOut(MAIN_MODULE_REPORTER, "whMeterSampler", LOG_TYPE_ERROR, 
			"Init SEMID FAILED!!!");
	}

	pthread_t hThread_RS485_1;
	if (pthread_create(&hThread_RS485_1, NULL, _thread_RS485_1_Sampler,(void *)pSampler) != 0)
	{
		TRACEE("\n----Init the _thread_RS485_1_Sampler error---\n");
		return FALSE;
	}
	
	pWhim_Rough->bUartServerCommOk = FALSE;

    BOOL needRS485_2 = FALSE;
    BOOL needUART_SERVER = FALSE;

	for(kkk = 0; kkk < MAX_WH_NUM; kkk++)
	{
		if (All_TypesMetersID[kkk / METER_TYPE_NUM][kkk % METER_TYPE_NUM] == 0)
		{
			continue;
		}
		if (pWhim_Rough->meterData[kkk].emMeterType == METER_TYPE_NotInstalled)
		{
			continue;
		}

		if (pWhim_Rough->meterData[kkk].bPhysicalInterType == RS485_2)
		{
			needRS485_2 = TRUE;
		}

		if (pWhim_Rough->meterData[kkk].bPhysicalInterType == UART_SERVER)
		{
			needUART_SERVER = TRUE;
		}
	}
	
	if (needRS485_2)
	{
		pthread_t hThread_RS485_2;
		if (pthread_create(&hThread_RS485_2, NULL, _thread_RS485_2_Sampler,(void *)pSampler) != 0)
		{
			TRACEE("\n----Init the _thread_RS485_2_Sampler error---\n");
			return FALSE;
		}
	}

	if (needUART_SERVER)
	{
		s_pUartServer_Comm = NEW(COMMPORT,1);
		if (s_pUartServer_Comm == NULL)
		{
			TRACEE("s_pUartServer_Comm Memory allocation failed\n");
			return FALSE;
		}

		pthread_t hThread_Uart_Server;
		if (pthread_create(&hThread_Uart_Server, NULL, _thread_UartServer_Sampler,(void *)pSampler) != 0)
		{
			TRACEE("\n----Init the _thread_UartServer_Sampler error---\n");
			return FALSE;
		}
	}
	
	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	 
	return 0;
}

typedef struct tagSampValMapper
{
	int		iRawValIdx; //Sample raw float data, if not a float , think it as float
	int		iSigID; //only sample signal, so no need to assign sig type
}SAMPVAL_MAP;

static void RS485_StuffChanl(RS485_SAMPLER_DATA *pWhim_Rough)
{
	//int nSamplerId = ((SAMPLER*)pWhim_Rough->hSampler)->info.nSamplerID;
	SAMPVAL_MAP whStuff[] = 
	{
		{WHMETER_ROUGH_RTACTPower,				400},
		{WHMETER_ROUGH_ACTPower_PhA,			401},
		{WHMETER_ROUGH_ACTPower_PhB,			402},
		{WHMETER_ROUGH_ACTPower_PhC,			403},

		{WHMETER_ROUGH_RTReActPower,			404},
		{WHMETER_ROUGH_ReActPower_PhA,			405},
		{WHMETER_ROUGH_ReActPower_PhB,			406},
		{WHMETER_ROUGH_ReActPower_PhC,			407},

		{WHMETER_ROUGH_RTkWh_AC,				411},
		{WHMETER_ROUGH_RTkvar,					412},
		{WHMETER_ROUGH_RT_VOLT_A,				413},
		{WHMETER_ROUGH_RT_CURR_A,				414},
		{WHMETER_ROUGH_RT_VOLT_B,				415},
		{WHMETER_ROUGH_RT_CURR_B,				416},
		{WHMETER_ROUGH_RT_VOLT_C,				417},
		{WHMETER_ROUGH_RT_CURR_C,				418},
		{WHMETER_ROUGH_RTkvar2,					419},
		//{WHMETER_ROUGH_RTReActPower,			420},
		{WHMETER_ROUGH_RT_RVSkWh_AC,			422},
		//{WHMETER_ROUGH_RT_PowerFactor,		423},
		{WHMETER_ROUGH_RT_Freq,					424},
		{WHMETER_ROUGH_PMAXDEMAND,				425},

		{WHMETER_ROUGH_RT_PowerFactor,			426},
		{WHMETER_ROUGH_PowerFactor_PhA,			427},
		{WHMETER_ROUGH_PowerFactor_PhB,			428},
		{WHMETER_ROUGH_PowerFactor_PhC,			429},

		{WHMETER_ROUGH_LAST1_POS_SUM,			1501},		
		{WHMETER_ROUGH_LAST1_POS_SHARP,			1502},		
		{WHMETER_ROUGH_LAST1_POS_PEAK,			1503},	
		{WHMETER_ROUGH_LAST1_POS_FLAT,			1504},	
		{WHMETER_ROUGH_LAST1_POS_WAVE,			1505},
		{WHMETER_ROUGH_LAST1_NEG_SUM,			1506},
		{WHMETER_ROUGH_LAST1_NEG_SHARP,			1507},
		{WHMETER_ROUGH_LAST1_NEG_PEAK,			1508},
		{WHMETER_ROUGH_LAST1_NEG_FLAT,			1509},
		{WHMETER_ROUGH_LAST1_NEG_WAVE,			1510},
		{WHMETER_ROUGH_LAST2_POS_SUM,			1511},
		{WHMETER_ROUGH_LAST2_POS_SHARP,			1512},
		{WHMETER_ROUGH_LAST2_POS_PEAK,			1513},
		{WHMETER_ROUGH_LAST2_POS_FLAT,			1514},
		{WHMETER_ROUGH_LAST2_POS_WAVE,			1515},
		{WHMETER_ROUGH_LAST2_NEG_SUM,			1516},
		{WHMETER_ROUGH_LAST2_NEG_SHARP,			1517},
		{WHMETER_ROUGH_LAST2_NEG_PEAK,			1518},
		{WHMETER_ROUGH_LAST2_NEG_FLAT,			1519},
		{WHMETER_ROUGH_LAST2_NEG_WAVE,			1520},
		{WHMETER_ROUGH_SUM_RTkWh_AC,			1491}, 
		{WHMETER_ROUGH_SHARP_RTkWh_AC,			1492},
		{WHMETER_ROUGH_PEAK_RTkWh_AC,			1493},
		{WHMETER_ROUGH_FLAT_RTkWh_AC,			1494},
		{WHMETER_ROUGH_WAVE_RTkWh_AC,			1495},
		{WHMETER_ROUGH_RT_SUM_RVSkWh_AC,		1496}, 
		{WHMETER_ROUGH_RT_SHARP_RVSkWh_AC,		1497},
		{WHMETER_ROUGH_RT_PEAK_RVSkWh_AC,		1498},
		{WHMETER_ROUGH_RT_FLAT_RVSkWh_AC,		1499},
		{WHMETER_ROUGH_RT_WAVE_RVSkWh_AC,		1500},

		{WHMETER_ROUGH_CURRENTPOSDEMAND,		432},

	};

	int n = 0;
	int kkk = 0;
	UINT nDeviceID = 0;
	for(kkk = 0 ; kkk < MAX_WH_NUM; kkk++)
	// for(kkk = MAX_WH_NUM - 1; kkk >= 0; kkk--)
	{
		nDeviceID = All_TypesMetersID[kkk / METER_TYPE_NUM][kkk % METER_TYPE_NUM];
		if (nDeviceID == 0)
		{
			continue;
		}
		if((pWhim_Rough->meterData[kkk].emMeterType == METER_TYPE_NotInstalled)
			|| (pWhim_Rough->meterData[kkk].emMeterType == METER_TYPR_DataFromMaster))
		{
			continue;
		}

		for(n = 0; n < ITEM_OF(whStuff); n++)
		{
			//if((401 + kkk) == 407) //Skip 407 AC Meter For Local
			//{
			//	continue;
			//}
			Push_VarValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler,
				nDeviceID,
				whStuff[n].iSigID,
				pWhim_Rough->meterData[kkk].varWHMeterRough[whStuff[n].iRawValIdx]);

			//TRACE("-------kkk=[%d] nSigNal=[%d] iRawValIdx=[%d] The whRawLVal = %ld fValue=%f-------------\n",\
			//	kkk, n, whStuff[n].iRawValIdx, pWhim_Rough->meterData[kkk].varWHMeterRough[whStuff[n].iRawValIdx].ulValue,\
			//	pWhim_Rough->meterData[kkk].varWHMeterRough[whStuff[n].iRawValIdx].fValue);
		}

		VAR_VALUE varV;
		varV.ulValue = pWhim_Rough->meterData[kkk].emSelfCheckCode;
		Push_VarValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler,
			nDeviceID, 423, varV);
	}
	return;
}

static void RS485_StuffMeterValues(RS485_SAMPLER_DATA *pWhim_Rough)
{
	UINT n = 0;
	UINT nDevID = 0;

	for(n = 0 ; n < MAX_WH_NUM; n++)
	{
		if((pWhim_Rough->meterData[n].emMeterType == METER_TYPE_NotInstalled)
			|| (pWhim_Rough->meterData[n].emMeterType == METER_TYPR_DataFromMaster))
		{
			continue;
		}

		TRACE("RS485_StuffMeterValues: n=[%d] emMeterType=[%d]\n", \
			n, pWhim_Rough->meterData[n].emMeterType);

		METER_DATA *pMeterData = &pWhim_Rough->meterData[n];
		
		pWhim_Rough->blobMeterVal[n].tmRefresh = time(NULL);
		pWhim_Rough->blobMeterVal[n].emDataFrom = ESSTYPE_UDP_SERVER;  
 		memcpy(pWhim_Rough->blobMeterVal[n].szIP, "192.168.1.100", 32);

		pWhim_Rough->blobMeterVal[n].fRealTime_ActivePower	= pMeterData->varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue;
		pWhim_Rough->blobMeterVal[n].fPhA_ActivePower 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_ACTPower_PhA].fValue;
		pWhim_Rough->blobMeterVal[n].fPhB_ActivePower 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_ACTPower_PhB].fValue;
		pWhim_Rough->blobMeterVal[n].fPhC_ActivePower 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_ACTPower_PhC].fValue;

		pWhim_Rough->blobMeterVal[n].fReactivePower 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_RTReActPower].fValue;	
		pWhim_Rough->blobMeterVal[n].fPhA_ReactivePower 	= pMeterData->varWHMeterRough[WHMETER_ROUGH_ReActPower_PhA].fValue;
		pWhim_Rough->blobMeterVal[n].fPhB_ReactivePower 	= pMeterData->varWHMeterRough[WHMETER_ROUGH_ReActPower_PhB].fValue;
		pWhim_Rough->blobMeterVal[n].fPhC_ReactivePower 	= pMeterData->varWHMeterRough[WHMETER_ROUGH_ReActPower_PhC].fValue;

		pWhim_Rough->blobMeterVal[n].fRealTime_kWh			= pMeterData->varWHMeterRough[WHMETER_ROUGH_RTkWh_AC].fValue;	
		pWhim_Rough->blobMeterVal[n].fRealTime_kVarh1 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_RTkvar].fValue;			

		pWhim_Rough->blobMeterVal[n].fPhA_Volt 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_VOLT_A].fValue;
		pWhim_Rough->blobMeterVal[n].fPhA_Curr 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_CURR_A].fValue;
		pWhim_Rough->blobMeterVal[n].fPhB_Volt 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_VOLT_B].fValue;
		pWhim_Rough->blobMeterVal[n].fPhB_Curr 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_CURR_B].fValue;
		pWhim_Rough->blobMeterVal[n].fPhC_Volt 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_VOLT_C].fValue;
		pWhim_Rough->blobMeterVal[n].fPhC_Curr 				= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_CURR_C].fValue;

		pWhim_Rough->blobMeterVal[n].fRealTime_kVarh2 		= pMeterData->varWHMeterRough[WHMETER_ROUGH_RTkvar2].fValue;
		pWhim_Rough->blobMeterVal[n].fReverse_kWh			= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_RVSkWh_AC].fValue;
		pWhim_Rough->blobMeterVal[n].fAC_frequency			= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_Freq].fValue;		
		pWhim_Rough->blobMeterVal[n].fPmaxDemand			= pMeterData->varWHMeterRough[WHMETER_ROUGH_PMAXDEMAND].fValue;
		pWhim_Rough->blobMeterVal[n].fPower_Factor			= pMeterData->varWHMeterRough[WHMETER_ROUGH_RT_PowerFactor].fValue;	

		pWhim_Rough->blobMeterVal[n].fPhA_PowerFactor		= pMeterData->varWHMeterRough[WHMETER_ROUGH_PowerFactor_PhA].fValue;
		pWhim_Rough->blobMeterVal[n].fPhB_PowerFactor		= pMeterData->varWHMeterRough[WHMETER_ROUGH_PowerFactor_PhB].fValue;
		pWhim_Rough->blobMeterVal[n].fPhC_PowerFactor		= pMeterData->varWHMeterRough[WHMETER_ROUGH_PowerFactor_PhC].fValue;

		nDevID = 790 + n;
		Push_BlobValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler, 790 + n, 430,
			(void*)&pWhim_Rough->blobMeterVal[n], sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
		if(nDevID == 790)
		{
			Push_BlobValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler, 800, 430,
				(void*)&pWhim_Rough->blobMeterVal[n], sizeof(A_BLOB_METER_VALUES), BLOB_TYPE_METERVALUES);
			
			VAR_VALUE varV;	
			varV.ulValue = 1;//Master Priority Default is 1. Need Client Adjust First.
			Push_VarValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler, 1, 47, varV);
		}
	}

	return;
}

static BOOL Judge_IfNeedRebootSys(RS485_SAMPLER_DATA *pWhim_Rough)
{
	VAR_VALUE varV;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)((SAMPLER*)pWhim_Rough->hSampler)->hDPR;

	if(pWhim_Rough->bLast_AllMeterCommOK == FALSE)
	{
		return FALSE;
	}

	// LJH250523 fix:站控方案的储能柜只装了柜内表，主控只装了网侧表或辅助表
	int n;
	for(n = 0 ; n < MAX_WH_NUM; n++)
	{	
		if (All_TypesMetersID[n / METER_TYPE_NUM][n % METER_TYPE_NUM] == 0)
		{
			continue;
		}
			
		if (pWhim_Rough->meterData[n].emMeterType != METER_TYPE_NotInstalled && pWhim_Rough->meterData[n].bIsCommFail == FALSE)
		{
			return FALSE;
		}
	}
	// old if(pWhim_Rough->meterData[0].emMeterType == METER_TYPE_NotInstalled)
	// {
	// 	return FALSE;
	// }

	// if(pWhim_Rough->meterData[1].emMeterType == METER_TYPE_NotInstalled)
	// {
	// 	return FALSE;
	// }

	// if(pWhim_Rough->meterData[0].bIsCommFail == FALSE)
	// {
	// 	return FALSE;
	// }

	// if(pWhim_Rough->meterData[1].bIsCommFail == FALSE)
	// {
	// 	return FALSE;
	// }

	//pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR, 1, 5, &varV, 4);
	//if(varV.emValue == 0) //1-Server CommFial
	//{
	//	return FALSE;
	//}
	//printf("Warning: Cloud Comm Statu=[%d]\n", varV.emValue);


	return TRUE;
}




static BOOL Judge_ActvPower_Reflux(IN RS485_SAMPLER_DATA* pWhim_Rough)
{
	//如果没有安装柜内表或者网侧表，就不处理。例如站级EMS-从设备没有安装网侧表，只安装了柜内表；不需要处理
	//站级EMS 快速防逆流必须接柜内表和网侧表
	//从设备  只安装了柜内表
	//一体机  安装了柜内表和网侧表
	if((pWhim_Rough->meterData[CAB_FIRST].emMeterType == METER_TYPE_NotInstalled) 
		|| (pWhim_Rough->meterData[GRID_FIRST].emMeterType == METER_TYPE_NotInstalled))
	{
		return 0;
	}
	char TempBuf[512];
	float fGridPwr_Sum = 0,  fGridPwr_A = 0,  fGridPwr_B = 0,  fGridPwr_C = 0, fCabPwr_Sum = 0;
	double tmNow = GetCurrentTime();	// current time.

	fCabPwr_Sum = pWhim_Rough->meterData[CAB_FIRST].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue;  //cab

	fGridPwr_Sum = pWhim_Rough->meterData[GRID_FIRST].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue; //grid meter
	fGridPwr_A = pWhim_Rough->meterData[GRID_FIRST].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhA].fValue;
	fGridPwr_B = pWhim_Rough->meterData[GRID_FIRST].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhB].fValue;
	fGridPwr_C = pWhim_Rough->meterData[GRID_FIRST].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhC].fValue;

	int n;
	for(n = 0 ; n < MAX_WH_NUM; n++)
	{	
		if(pWhim_Rough->meterData[n].bIsCommFail == FALSE)
		{
			continue;
		
		}
		if(ElapseMS(LastSentPostTime) > 10000)
		{
			LastSentPostTime = tmNow;
			TRACEW("Meter Alarm [%d] time [ %0.3f ]", n, tmNow);
			// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
			// 	"Meter Alarm [%d]",n);
			sem_post_op(pWhim_Rough->nSemCCUWith485Meter);	//send sem to ccusampler
			return TRUE;
		}
	}

	if ((fCabPwr_Sum < 0) && (fGridPwr_Sum < 0))
	{
		if (pWhim_Rough->nSemCCUWith485Meter != -1)
		{
			TRACEW("Start to Adjust ---Grid[ %0.3f ] Cab[ %0.3f ] time [ %0.3f ]",fGridPwr_Sum,fCabPwr_Sum,tmNow);
			// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
			// 	"Start to Adjust ---Grid[ %0.3f ] Cab[ %0.3f ]",fGridPwr_Sum,fCabPwr_Sum);
			LastSentPostTime = GetCurrentTime();
			sem_post_op(pWhim_Rough->nSemCCUWith485Meter);	//send sem to ccusampler
		}
	}
	return TRUE;
}
BOOL JudgeAndSaveToDB(RS485_SAMPLER_DATA *pWhim_Rough,int nWhichWh)
{
	if (nWhichWh != CAB_FIRST)  //only cab meter
	{
		return 0;
	}

	time_t tmNow = _NOW_;
	struct tm gmTime;
	gmtime_r(&tmNow, &gmTime);
	TRACEW("JudgeAndSaveToDB---CURRENTLASTYear = [%d] CURRENTLASTMonth = [%d] CURRENTLASTDay = [%d]\n",
		pWhim_Rough->meterData[nWhichWh].nLastYearToSaveDB,pWhim_Rough->meterData[nWhichWh].nLastMonthToSaveDB,pWhim_Rough->meterData[nWhichWh].nLastDayToSaveDB);

	if (pWhim_Rough->meterData[nWhichWh].nLastYearToSaveDB != gmTime.tm_year || 
		pWhim_Rough->meterData[nWhichWh].nLastMonthToSaveDB != gmTime.tm_mon || 
		pWhim_Rough->meterData[nWhichWh].nLastDayToSaveDB != gmTime.tm_mday )    //one day one record
	{

		EVENT_METER_SAVE_DATA Event_KwhMeterVar;
		memset(&Event_KwhMeterVar, 0, sizeof(EVENT_METER_SAVE_DATA));

		Event_KwhMeterVar.ndevid = All_TypesMetersID[nWhichWh / METER_TYPE_NUM][nWhichWh % METER_TYPE_NUM];
		if (sizeof(Event_KwhMeterVar.varEventWHMeterRough) == sizeof(pWhim_Rough->meterData[nWhichWh].varWHMeterRough))
		{		
			memcpy(Event_KwhMeterVar.varEventWHMeterRough,  pWhim_Rough->meterData[nWhichWh].varWHMeterRough, 
				sizeof(Event_KwhMeterVar.varEventWHMeterRough));
		}

		SendMsg(pWhim_Rough->hSampler, EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW, EVENT_MSG_STATUS_CHANGED,
			&Event_KwhMeterVar, sizeof(EVENT_METER_SAVE_DATA));

		pWhim_Rough->meterData[nWhichWh].nLastYearToSaveDB = gmTime.tm_year;
		pWhim_Rough->meterData[nWhichWh].nLastMonthToSaveDB = gmTime.tm_mon;
		pWhim_Rough->meterData[nWhichWh].nLastDayToSaveDB = gmTime.tm_mday;

		TRACEW("JudgeAndSaveToDB---tm_year=[%d] tm_mon=[%d] tm_mday=[%d]  tm_hour=[%d] tm_min=[%d]",
			gmTime.tm_year,gmTime.tm_mon,gmTime.tm_mday,gmTime.tm_hour,gmTime.tm_min);
	}
	return 0;
}


//#define  MAX_RS4851_TRY_TIMES	10
DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	RS485_SAMPLER_DATA *pWhim_Rough = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;

	// printf("time[%0.3f] Grid[%0.3f]  Cab[%0.3f] count[%d]\n",
	// 	GetCurrentTime(),
	// 	pWhim_Rough->meterData[1].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue, 
	// 	pWhim_Rough->meterData[0].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue,
	// 	count );
	if (pWhim_Rough->bEnableQuickResChannel) //快速响应通道 Enable
	{
		if (pWhim_Rough->meterData[GRID_FIRST].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue < 0 && count < 20)	//等待网侧表稳定，等500ms
		{
			count += 1;
		}
		else
		{
			count = 0;
		}

		if ((count > 0 && count < 5) || (count >= 10))
		{
			return ERR_SAMPLE_OK;
		}
		// 当等于5的时候，就push数据
		if (count > 5 && count < 10)//最多连续发5包
		{
			// printf("continue to adjust ....\n");
			Judge_ActvPower_Reflux(pWhim_Rough);

			return ERR_SAMPLE_OK;
		}
	}

	//Now do the stuff chanel
	RS485_StuffChanl(pWhim_Rough);

	// RS485_StuffMeterValues(pWhim_Rough);

	if (pWhim_Rough->bEnableQuickResChannel) //快速响应通道 Enable
	{
		Judge_ActvPower_Reflux(pWhim_Rough);
	}

	if(Judge_IfNeedRebootSys(pWhim_Rough) == TRUE)
	{
		pWhim_Rough->nAllCommFailCounter++;
		if(pWhim_Rough->nAllCommFailCounter >= 1200) //通信中断4min后重启
		{
			pWhim_Rough->nAllCommFailCounter = 0;
			LogOut(MAIN_MODULE_SAMPLER, "whMeterSampler", LOG_TYPE_ERROR, 
				"!!!Now we will RESET the system.NO response from kWhMeter-RS485!!!");
			//gracefully reboot system
			EVENT_SYSTEM_EXIT evExit;
			evExit.nExitType = SYS_INTERNAL_ERROR;
			SendMsg(pWhim_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
				&evExit, sizeof(EVENT_SYSTEM_EXIT));	
		}
	}
	else
	{
		pWhim_Rough->nAllCommFailCounter = 0;
	}

	return ERR_SAMPLE_OK;
}

void GetMeterSN(RS485_SAMPLER_DATA *pWhim_Rough, IN int nWhichWh, char* szSrc)
{
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)((SAMPLER*)pWhim_Rough->hSampler)->hDPR;
	char szSN[13];
	char* psz = szSrc;
	if(szSrc == NULL)
	{
		VAR_VALUE varV;
		varV.blobValue = szSN;
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ All_TypesMetersID[nWhichWh/METER_TYPE_NUM][nWhichWh%METER_TYPE_NUM], 403, &varV, 13);
		psz = szSN;
	}

	//sscanf(psz, "%02X%02X%02X%02X%02X%02X",
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[5],
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[4],
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[3],
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[2],
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[1],
	//	&pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[0]);
	BYTE byConvt[6];
	sscanf(psz, "%02X%02X%02X%02X%02X%02X",
		&byConvt[0], &byConvt[1], &byConvt[2],
		&byConvt[3], &byConvt[4], &byConvt[5]);
	int n;
	for(n = 0 ; n < 6; n++)
	{
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[n] = byConvt[5 - n];
	}

	TRACEX(" now update meterSN:[%s], [%02X%02X%02X%02X%02X%02X]\n ", psz,
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[5],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[4],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[3],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[2],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[1],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[0]);

	if(strlen(psz) > 0)
	{
		pWhim_Rough->meterData[nWhichWh].byModbusAddress = (BYTE)atoi(psz);
	}
	else
	{
		pWhim_Rough->meterData[nWhichWh].byModbusAddress = 0x01; //default is 1
	}
}

DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	RS485_SAMPLER_DATA *pWhm = (RS485_SAMPLER_DATA*)pSampler->pDataHeap;
 	//int nRet = ERR_CONTROL_FAILED;

	/*if(nDevId == 421) //for IMEU
	{
		if(!pWhm->bNeedSampleIMEU)
		{
			return ERR_CONTROL_FAILED;
		}
		return IMEU_SendCtrl(pWhm, nParamId, *pVarVal);
	}*/

	// HANDLE hSamplerLock = ((SAMPLER*)pWhm->hSampler)->hSamplerLock;
	// if(Mutex_Lock(hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	// {
	// 	return ERR_CONTROL_TIMEOUT;
	// }

	int iii , kkk;
	int row = -1, col = -1;
	int found = 0;
	int nWhichWh = 0;
	for( iii = 0; iii < METER_TYPES_MAX && !found; iii++)
	{
		for( kkk = 0; kkk < METER_TYPE_NUM; kkk++)
		{
			if (nDevId == All_TypesMetersID[iii][kkk])
			{
				row = iii;
                col = kkk;
                found = 1;
				break;
			}
		}
	}

	if (found) 
	{
        nWhichWh = row * METER_TYPE_NUM + col;
        // printf("nDevId %d corresponds to 485buf[%d], which is All_TypesMetersID[%d][%d] = %d\n", 
		// 	nDevId, nWhichWh, row, col, All_TypesMetersID[row][col]);
    }
	else
	{
        // printf("nDevId %d not found in All_TypesMetersID array\n", nDevId);
		return ERR_CONTROL_FAILED;
    }
	if(emParaType == PARAM_TYPE_SETTING)
	{		
		//settings
		if(nParamId == 403)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 13)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			GetMeterSN(pWhm, nWhichWh, (char*)pVarVal->blobValue);
		}
		else if(nParamId == 401)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pWhm->meterData[nWhichWh].emMeterType = (METER_TYPE)pVarVal->emValue;
			
			int nDeviceID = All_TypesMetersID[nWhichWh / METER_TYPE_NUM][nWhichWh % METER_TYPE_NUM];
			if(nDeviceID != 0 && pWhm->meterData[nWhichWh].emMeterType == METER_TYPE_NotInstalled)
			{
				SetCommStatus((HANDLE)pSampler, nDeviceID, TRUE);
			}
		}
		else if(nParamId == 402)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pWhm->meterData[nWhichWh].fACMeter_PowerFactor = pVarVal->fValue;
		}
		else if(nParamId == 404)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pWhm->meterData[nWhichWh].emACMeter_ProtocolType = (ACMETER_PROTOTP)pVarVal->emValue;
		}
		else if(nParamId == 405)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			//Baudrate of the kwhMeter, newly added 20190226
			if(pVarVal->emValue == 0) //auto set by DC/AC, to be compatible with marchet exsiting kwhMeter
			{
				if((pWhm->meterData[nWhichWh].emMeterType == METER_TYPR_ac) &&
					(pWhm->meterData[nWhichWh].emACMeter_ProtocolType  == ACMETER_PROTOCOL_MODBUS_EM737))
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
				}
				else if((pWhm->meterData[nWhichWh].emMeterType == METER_TYPR_ac) &&
					(pWhm->meterData[nWhichWh].emACMeter_ProtocolType  == ACMETER_PROTOCOL_MODBUS_RM_PRIVATE))
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
				}
				else if((pWhm->meterData[nWhichWh].emMeterType == METER_TYPR_ac) &&
					(pWhm->meterData[nWhichWh].emACMeter_ProtocolType  == ACMETER_PROTOCOL_MODBUS_RTU_YD))
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "9600,N,8,1");
				}
				else
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
				}
			}
			else
			{
				if(pVarVal->emValue == 3)
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "9600,N,8,1");
				}
				else if(pVarVal->emValue == 1)
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
				}
				else
				{
					snprintf(pWhm->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
				}
			}

			TRACE("The kwhMeter[%d] baudrate is[%s]\n", nWhichWh, pWhm->meterData[nWhichWh].szBaudrateSett);
		}
		else if(nParamId == 407)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pWhm->meterData[nWhichWh].fACMeter_VoltPTFactor = pVarVal->fValue;
		}
		else if(nParamId == 408)
		{
			// int nWhichWh = (nDevId - /*((pWhm->nSamplerId == 3) ? 408 : 410)*/ KWHMETER_INDEX_START);
			if((nWhichWh < 0) || (nWhichWh >= MAX_WH_NUM))
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			if(nValSize != 4)
			{
				// Mutex_Unlock(hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pWhm->meterData[nWhichWh].bPhysicalInterType = (PHY_INTER_TYPE)pVarVal->emValue;
		}
	}

	// Mutex_Unlock(hSamplerLock);
	return ERR_CONTROL_OK;
}


