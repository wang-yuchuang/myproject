#include "dev_485_2_sampler_main.h"
/*********************************************************************************
*  
*  FUNCTION NAME : Dev_PT1000_Mgmt
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void BubbleSort(float *pData, int nLen)
{
	int i = 0, j = 0;
	float fTemp = 0.0;
	for(i = 0; i< nLen-1; i++)
	{
		for(j = 0; j < (nLen-i-1); j++)
		{
			if(pData[j] > pData[j+1])
			{
				fTemp = pData[j];
				pData[j] = pData[j+1];
				pData[j+1] = fTemp;
			}
		}
	}
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Dev_PT1000_Mgmt
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Dev_PT1000_Mgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	UINT n = 0;
	UINT nPT1000Num = pIMEU2_RoughData->nPT1000Num;
	float fPT1000_Temp[MAX_PT1000_NUM]={0.0};
	
	if(nPT1000Num > MAX_PT1000_NUM)
	{
		return;
	}
	
	for(n = 0; n < nPT1000Num; n++)
	{
		fPT1000_Temp[n] = pIMEU2_RoughData->blobIMEU2_V20.fPT1000Temperature[n];
	}

	BubbleSort(fPT1000_Temp, nPT1000Num);

	if(fabs(fPT1000_Temp[0] - fPT1000_Temp[nPT1000Num-1]) > 10.0)//LYX Suggest 10°.2021-11-10
	{
		pIMEU2_RoughData->emTempImbalance = 1;  //Alarm
	}
	else
	{
		pIMEU2_RoughData->emTempImbalance = 0;  //Normal
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Dev_ShutterFan_Mgmt
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Dev_ShutterFan_Mgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU)
{
 	time_t           tmNow = _NOW_;
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	
	if(pIMEU2_RoughData->bMonitorShutter == FALSE) 
	{
		return;
	}

	if(ABS(tmNow - pIMEU2_RoughData->tmShutterFanCtrl) < 30) 
	{
		return;
	}

	if(pIMEU2_RoughData->nShutterCtrlCmd == STAT_DEV_ON)  //Open Cmd
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emShutterOpenedOver == STAT_Shutter_NotOpenTotally) //Check Shutter Opened Status
		{
			//Failed, Try again ctrl
			pIMEU2_RoughData->tmShutterFanCtrl = tmNow;
			if((--pIMEU2_RoughData->nShutterCtrlTimes) <= 0)
			{
				//Try 3 Times, Not Yet Success, Believe Cmd Ctrl Failed Finally, Need Alarm, End of Process
				pIMEU2_RoughData->bMonitorShutter = FALSE;
				pIMEU2_RoughData->nShutterOpenResult = 1;
				TRACE("IMEU2: Open ShutterFan Cmd Try 3 Times, Not Yet Success\n");
			}
			TRACE("IMEU2: Open Cmd Retry\n");
			//Retry Ctrl Cmd
			SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_SUB_SHUTTER, STAT_DEV_ON);
		}
		else
		{
			//Success, End of Process
			pIMEU2_RoughData->bMonitorShutter = FALSE;
			pIMEU2_RoughData->nShutterOpenResult = 0;
			TRACE("IMEU2: Open ShutterFan Cmd Send Success\n");
		}
	}
	else  //Closed Cmd
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emDI6_ShutterClosedOver == STAT_Shutter_NotCloseTotally) 
		{
			//Failed, Try again ctrl
			pIMEU2_RoughData->tmShutterFanCtrl = tmNow;
			if((--pIMEU2_RoughData->nShutterCtrlTimes) <= 0)
			{
				//Try 3 Times, Not Yet Success, Believe Cmd Ctrl Failed Finally, Need Alarm, End of Process
				pIMEU2_RoughData->bMonitorShutter = FALSE;
				pIMEU2_RoughData->nShutterClosedResult = 1;
				TRACE("IMEU2: Closed ShutterFan Cmd Try 3 Times, Not Yet Success\n");
			}
			//Retry Ctrl Cmd
			SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_SUB_SHUTTER, STAT_DEV_OFF);
		}
		else
		{
			//Success, End of Process
			pIMEU2_RoughData->bMonitorShutter = FALSE;
			pIMEU2_RoughData->nShutterClosedResult = 0;
			TRACE("IMEU2: Closed ShutterFan Cmd Send Success\n");
		}
	}
	return;
}
 
/*********************************************************************************
*  
*  FUNCTION NAME : OnIdle_CabSafeStage
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
CAB_SAFE_MANAGE_STATE   OnIdle_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{
    IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
 
    time_t tmNow = time(NULL);
    if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) < 30)
    {
        //TRACE("Warning: GoBack OnIdle-nWhichIMEU=[%d] tmNow=[%ld] tm_Enter=[%ld]\n", \
        //        nWhichIMEU, tmNow, pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter);  
        return;
    }
    TRACEX("Info: Excute Idle Logic-nWhichIMEU=[%d] tmNow=[%ld] tm_Enter=[%ld]\n", \
            nWhichIMEU, tmNow, pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter);

    //0-Open OK, 1-Not Opened Totally
	if((pIMEU2_RoughData->blobIMEU2_V20.emShutterOpenedOver == STAT_Shutter_OpenOK) || (pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed > 5))
	{
		SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_SHUTTER, STAT_DEV_OFF);
		if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) > SEC_PER_HOUR)
    	{
        	LogOut(MAIN_MODULE_SAMPLER, "BattCab_Mgmt", LOG_TYPE_INFO, "3nd:IMEU2=[%d]OpenedOver=[%d]FanSpeed=[%d]", \
            	nWhichIMEU, pIMEU2_RoughData->blobIMEU2_V20.emShutterOpenedOver, pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed);
		}
	}

    //if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON
    //{  
    //    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
    //}

	//ON: When Force Send The Following 4 Cmd, AirCond Recv Then Excute it.
	//OFF: When Send OFF Cmd it will be stopped, Then Auto run according to its own logic.
 	if(pIMEU2_RoughData->blobIMEU2_V20.emStartRefrigMode == STAT_DEV_ON) // 1-ON -> 0-OFF
    {  
        SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_COOL_MODE, STAT_DEV_OFF, TRUE);
    }

	if(pIMEU2_RoughData->blobIMEU2_V20.emStartSupplyAirMode == STAT_DEV_ON) // 1-ON -> 0-OFF
    {  
        SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_SUPPLY_AIR, STAT_DEV_OFF, TRUE);
    }

	if(pIMEU2_RoughData->blobIMEU2_V20.emStartStandbyMode == STAT_DEV_ON) // 1-ON -> 0-OFF
    {  
        SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_STANDBY, STAT_DEV_OFF, TRUE);
    }

	if(pIMEU2_RoughData->blobIMEU2_V20.emStartHeatMode == STAT_DEV_ON) // 1-ON -> 0-OFF
    {  
        SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_HEAT_MODE, STAT_DEV_OFF, TRUE);
    }

	pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter = tmNow;
    return CAB_SAFE_STATE_IDLE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnSecond_CabSafeStage
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
CAB_SAFE_MANAGE_STATE   OnSecond_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{  
    //TRACE("Info: OnSecond_-nWhichIMEU=[%d]\n", nWhichIMEU);
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

    time_t tmNow = time(NULL);
    if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) < 5)
    {
        //TRACE("Warning: GoBack OnIdle-nWhichIMEU=[%d] tmNow=[%ld] tm_Enter=[%ld]\n", \
        //        nWhichIMEU, tmNow, pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter);  
        return;
    }

    //0-Close ok Totally, 1-Not Close Totally
	if((pIMEU2_RoughData->blobIMEU2_V20.emDI6_ShutterClosedOver == STAT_Shutter_CloseOK) 
		||(pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed < 5))
	{
		SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_SHUTTER, STAT_DEV_ON);
		if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) > SEC_PER_HOUR)
    	{
        	LogOut(MAIN_MODULE_SAMPLER, "BattCab_Mgmt", LOG_TYPE_INFO, "2nd:IMEU2=[%d]DI6ClosedOver=[%d]FanSpeed=[%d]", \
            	nWhichIMEU, pIMEU2_RoughData->blobIMEU2_V20.emDI6_ShutterClosedOver, pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed);
		}
	}

	pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter = tmNow;
    return CAB_SAFE_STATE_SECOND;
}

/*********************************************************************************
*  
*  FUNCTION NAME : OnThird_CabSafeStage
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : if ShutterFan Opened,Close it. 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
CAB_SAFE_MANAGE_STATE   OnThird_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

    time_t tmNow = time(NULL);
    if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) < 5)
    {
        //TRACE("Warning: GoBack OnIdle-nWhichIMEU=[%d] tmNow=[%ld] tm_Enter=[%ld]\n", \
        //        nWhichIMEU, tmNow, pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter);  
        return;
    }
        
    //0-Open OK, 1-Not Opened Totally
	if((pIMEU2_RoughData->blobIMEU2_V20.emShutterOpenedOver == STAT_Shutter_OpenOK) 
		||(pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed > 5))
	{
		SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_SHUTTER, STAT_DEV_OFF); 
		if(ABS(tmNow - pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter) > SEC_PER_HOUR)
    	{
			LogOut(MAIN_MODULE_SAMPLER, "BattCab_Mgmt", LOG_TYPE_INFO, "3nd:IMEU2=[%d]OpenedOver=[%d]FanSpeed=[%d]", \
            	nWhichIMEU, pIMEU2_RoughData->blobIMEU2_V20.emShutterOpenedOver, pIMEU2_RoughData->blobIMEU2_V20.nExhaustFanSpeed);
		}
	}

	pIMEU2_RoughData->pCurr_StateMachineItem->tm_Enter = tmNow;
    return CAB_SAFE_STATE_SECOND;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfNeedOffAir2TempAndECO
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfNeedOffAir2TempAndECO(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	time_t tmNow = _NOW_;

	TRACEX("nWhichIMEU=[%d]fBattTmax_Samp=[%.1f] fBatt_Tmax_cooloffSett=[%.1f] fRealTimeCabTemp=[%.1f] \
		fAmbit_TcoolonSett=[%.1f] emECORunStatu=[%d] fCoolSetPoint=[%.1f] fECOModeTempOffset=[%.1f]\n", \
		nWhichIMEU, pIMEU2_RoughData->fBattTmax_Samp,pIMEU2_RoughData->fBatt_Tmax_cooloffSett,\
		pBlobCabV2->fRealTimeCabTemp, pIMEU2_RoughData->fAmbit_TcoolonSett,\
		pIMEU2_RoughData->emECORunStatu, pBlobCabV2->fCoolSetPoint ,pIMEU2_RoughData->fECOModeTempOffset);
	
	if(pIMEU2_RoughData->emECORunStatu != STAT_ECO_MODE)
	{
		return FALSE;
	}

	if((pIMEU2_RoughData->fBattTmax_Samp < (pIMEU2_RoughData->fBatt_Tmax_cooloffSett + pIMEU2_RoughData->fECOModeTempOffset)) &&
		(pBlobCabV2->fRealTimeCabTemp < pIMEU2_RoughData->fAmbit_TcoolonSett) &&
		(pBlobCabV2->fCoolSetPoint >= AIRCOND_STOP_COOLPOINT))//John,Need Wait AirCond Stop Cool First.
		{
			return TRUE;
		}
	
	if(ABS(tmNow - pIMEU2_RoughData->tm_LastChangeECOMode) >= MAX_ENTERECO_SEC)//Enter ECO Mode Over 30Min,Off AirCond.
	{
		return TRUE;
	}

	return FALSE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfNeedStopAirCondition
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfNeedStopAirCondition(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL bRet = FALSE;
	BOOL bAllHumiTempCommFail = TRUE;
	int	 n = 0;
	time_t tmNow = _NOW_;

	float fAmbit_TcooloffSett = 0.0;
	float fBatt_Tmax_cooloffSett = 0.0;

	if(ABS(tmNow - pIMEU2_RoughData->tmLastStartCool_AirCond) <= MAX_AIR_COOLONOFF_INTERVAL)
	{
		return bRet;
	}

	//if(pIMEU2_RoughData->emECORunStatu == STAT_ECO_MODE)
	//{
	//	fAmbit_TcooloffSett = pIMEU2_RoughData->fAmbit_TcooloffSett + pIMEU2_RoughData->fECOModeTempOffset;
	//	fBatt_Tmax_cooloffSett = pIMEU2_RoughData->fBatt_Tmax_cooloffSett + pIMEU2_RoughData->fECOModeTempOffset;
	//}
	//else
	{
		fAmbit_TcooloffSett = pIMEU2_RoughData->fAmbit_TcooloffSett ;
		fBatt_Tmax_cooloffSett = pIMEU2_RoughData->fBatt_Tmax_cooloffSett ;
	}
	
	TRACE("Stop AirCond: fAmbit_TcooloffSett=[%.1f] fBatt_Tmax_cooloffSett=[%.1f]\n", fAmbit_TcooloffSett, fBatt_Tmax_cooloffSett);
	if((pBlobCabV2->fRealTimeCabTemp <= fAmbit_TcooloffSett) && //Default 21
 		(pIMEU2_RoughData->fBattTmax_Samp <= fBatt_Tmax_cooloffSett) && //Default 30
		(ABS(pIMEU2_RoughData->blobSingleBattData.fPackTotalCurr) < 20.0))  //John 2022-03-23,When Battery Running, Do not Adjust Param
	{	
		bRet = TRUE;
		/*for(n = 0; n < pBlobCabV2->nHumiTempSensorNumber; n++)
		{
			if(pBlobCabV2->emTempHumi_CommFail[n] == FALSE)
			{
				if(pBlobCabV2->fTemperature[n] <= (pIMEU2_RoughData->fAmbit_TcooloffSett))//Default 22
				{
					bRet = TRUE;
					break;
				}
			}
		}

		for(n = 0; n < pBlobCabV2->nHumiTempSensorNumber; n++)
		{
			if(pBlobCabV2->emTempHumi_CommFail[n] == FALSE)
			{
				bAllHumiTempCommFail = FALSE;
				break;
			}
		}

		if((bAllHumiTempCommFail == TRUE) || (pBlobCabV2->nHumiTempSensorNumber == 0))
		{
			bRet = TRUE;
		}*/
 	}

	if((pBlobCabV2->fRealTimeCabTemp <= pIMEU2_RoughData->fAirEnTran_TlmtSett) &&  //Default 20
		(pIMEU2_RoughData->fBattTmax_Samp <= (pIMEU2_RoughData->fAirEnTran_TlmtSett + 6)))
	{
		bRet = TRUE;
	}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStartCoolAccording2AmbitTemp
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  Tambinet ≥ Tcfg1_AmbitCoolOn[Default:28'C]，&& Tair ≥ TlimOff
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfStartCoolAccording2AmbitTemp(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	BOOL bRet = FALSE;
	int n = 0;

	float fAmbit_TcoolonSett = 0.0;

	if(pIMEU2_RoughData->emECORunStatu == STAT_ECO_MODE)
	{
		fAmbit_TcoolonSett = pIMEU2_RoughData->fAmbit_TcoolonSett + pIMEU2_RoughData->fECOModeTempOffset;
 	}
	else
	{
		fAmbit_TcoolonSett = pIMEU2_RoughData->fAmbit_TcoolonSett;
	}


 	if(pBlobCabV2->fRealTimeCabTemp <= pIMEU2_RoughData->fAirEnTran_TlmtSett)
	{
		return bRet;
	}

	for(n = 0; n < pBlobCabV2->nHumiTempSensorNumber; n++)
	{
		if(pBlobCabV2->emTempHumi_CommFail[n] == FALSE)
		{
			if(pBlobCabV2->fTemperature[n] >= fAmbit_TcoolonSett)
			{
				bRet = TRUE;
				break;
			}
		}
	}
 	
	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStartCoolAccording2AirCondTemp
*  CREATOR       :  
*  CREATED DATE  : 2021-1-14 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfStartCoolAccording2AirCondTemp(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	BOOL bRet = FALSE;

	float fAmbit_TcoolonSett = 0.0;

	if(pIMEU2_RoughData->emECORunStatu == STAT_ECO_MODE)
	{
		fAmbit_TcoolonSett = pIMEU2_RoughData->fAmbit_TcoolonSett + pIMEU2_RoughData->fECOModeTempOffset;
 	}
	else
	{
		fAmbit_TcoolonSett = pIMEU2_RoughData->fAmbit_TcoolonSett;
	}

	TRACE("nWhichIMEU=[%d]Start Cool According To AirCondTemp=[%.1f]\n", nWhichIMEU, fAmbit_TcoolonSett);
	if((pBlobCabV2->fRealTimeCabTemp >= fAmbit_TcoolonSett) &&  //Default 28
		(pIMEU2_RoughData->fBattTmax_Samp >= pIMEU2_RoughData->fAmbit_TcooloffSett)) //Default 22
	{
		bRet = TRUE;
	}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStartCoolAccording2BmsTmax
*  CREATOR       :  
*  CREATED DATE  : 2021-1-14 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfStartCoolAccording2BmsTmax(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	BOOL bRet = FALSE;

	float fBatt_Tmax_coolonSett = 0.0;

	if(pIMEU2_RoughData->emECORunStatu == STAT_ECO_MODE)
	{
		fBatt_Tmax_coolonSett = pIMEU2_RoughData->fBatt_Tmax_coolonSett + pIMEU2_RoughData->fECOModeTempOffset;
 	}
	else
	{
		fBatt_Tmax_coolonSett = pIMEU2_RoughData->fBatt_Tmax_coolonSett;
	}

	TRACE("nWhichIMEU=[%d] Start Cool Accroding To Batt Temp:fBatt_Tmax_coolonSett=[%f]\n", nWhichIMEU, fBatt_Tmax_coolonSett);
	if((pIMEU2_RoughData->fBattTmax_Samp >= fBatt_Tmax_coolonSett) && //Default 31
		(pBlobCabV2->fRealTimeCabTemp >= pIMEU2_RoughData->fAirEnTran_TlmtSett)) //Default 20
	{
		bRet = TRUE;
	}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Sync_CoolParam2AirCondition
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Sync_CoolParam2AirCondition(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU, BOOL bNeedWait)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	if(pBlobCabV2->fCoolSetPoint != pIMEU2_RoughData->fCoolPoint_Sync)
	{
		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_CRYOGEN_POINT, (int)pIMEU2_RoughData->fCoolPoint_Sync, bNeedWait);
		//2022-01-14 IMEU2 Return OK,But Send To AirCondtion Failed,So Used CoolSetPoint To Judge
	}

	if(pBlobCabV2->fCoolDeviation != pIMEU2_RoughData->fCoolRet_Sync)
	{
		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_COOLRET_POINT, (int)pIMEU2_RoughData->fCoolRet_Sync, FALSE);
	}
	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : AirCond_CoolingMgmt
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				Sync IdleCoolPoint.
*				First Stop Cooling Then Ctrl Off. Fisrt Ctrl ON Then Start Cooling.
*				Manufactuer Suggest Period Need Over 30Min.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void AirCond_CoolingMgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL bRet = FALSE;
  	time_t tmNow = _NOW_;

	//---Step1. Judge If Need Stop AirCondtion---//
	bRet = Judge_IfNeedStopAirCondition(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		pBlobCabV2->emAirMachineState = STAT_AIR_IDLE;
		pIMEU2_RoughData->fCoolPoint_Sync = AIRCOND_STOP_COOLPOINT;
	    pIMEU2_RoughData->fCoolRet_Sync = AIRCOND_ON_COOLOFFSET;
		Sync_CoolParam2AirCondition(pDev_485_2_Rough, nWhichIMEU, FALSE);

		return;
	}

	//---Step2. Judge AirCond Entrance Temperature To Cool Start---//
	bRet = Judge_IfStartCoolAccording2AirCondTemp(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON , Should Ctrl ON First
		{  
		    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
		}

		pBlobCabV2->emAirMachineState = STAT_AIR_COOLON;
		pIMEU2_RoughData->fCoolPoint_Sync = AIRCOND_ON_COOLPOINT;
	    pIMEU2_RoughData->fCoolRet_Sync = AIRCOND_ON_COOLOFFSET;
		if(pBlobCabV2->fCoolSetPoint != pIMEU2_RoughData->fCoolPoint_Sync)
		{
 			pIMEU2_RoughData->tmLastStartCool_AirCond = _NOW_;
		}

		Sync_CoolParam2AirCondition(pDev_485_2_Rough, nWhichIMEU, FALSE);

		return;
	}

	//John 2022-02-19 Temp&Humi Sensor Value is unreliable 
	//---Step3. Judge Cab Door Temperature To Cool Start---//
	//bRet = Judge_IfStartCoolAccording2AmbitTemp(pDev_485_2_Rough, nWhichIMEU);
	//if(bRet == TRUE)
	//{
	//	if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON , Should Ctrl ON First
	//	{  
	//	    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
	//	}
	//
	//	pBlobCabV2->emAirMachineState = STAT_AIR_COOLON;
	//	pIMEU2_RoughData->fCoolPoint_Sync = 21;
	//    pIMEU2_RoughData->fCoolRet_Sync = 7;
	//	Sync_CoolParam2AirCondition(pDev_485_2_Rough, nWhichIMEU, FALSE);
	// 
	//	return;
	//}

	//---Step4. Judge Battery Max Temperature To Cool Start---//
	bRet = Judge_IfStartCoolAccording2BmsTmax(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON , Should Ctrl ON First
		{  
		    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
		}

		//---2022-02-23 John Found Battery Max Temp Over 35,But AirTemp is Lower 27,SO Adjust CoolSet_Point 
		pBlobCabV2->emAirMachineState = STAT_AIR_COOLON;
 		pIMEU2_RoughData->fCoolPoint_Sync = AIRCOND_ON_COOLPOINT;
		pIMEU2_RoughData->fCoolRet_Sync = AIRCOND_ON_COOLOFFSET;
		if(pBlobCabV2->fCoolSetPoint != pIMEU2_RoughData->fCoolPoint_Sync)
		{
 			pIMEU2_RoughData->tmLastStartCool_AirCond = _NOW_;
		}

		Sync_CoolParam2AirCondition(pDev_485_2_Rough, nWhichIMEU, FALSE);

		return;
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStartHeating2Temp
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				   Sync IdleCoolPoint.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfStartHeating2Temp(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL bRet = FALSE;
  
	if((pBlobCabV2->fRealTimeCabTemp <= pIMEU2_RoughData->fAmbit_TheatonSett) && //Default 10
 		(pIMEU2_RoughData->fBattTmin_Samp <= pIMEU2_RoughData->fAmbit_TheatonSett)) //Default 10
	{	
		bRet = TRUE;
 	}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStopHeating2Temp
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				   Sync IdleCoolPoint.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
//static BOOL Judge_IfStopHeating2Temp(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
//{
//	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
//	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
//	
//	BOOL bRet = FALSE;
//  
//	if((pBlobCabV2->fRealTimeCabTemp > pIMEU2_RoughData->fAirEnTran_TlmtSett) //Default 20
// 		&& (pIMEU2_RoughData->fBattTmax_Samp > pIMEU2_RoughData->fAirEnTran_TlmtSett)) //Default 20
//	{	
//		bRet = TRUE;
// 	}
//
//	return bRet;
//}

/*********************************************************************************
*  
*  FUNCTION NAME : Sync_HeatParam2AirCondition
*  CREATOR       :  
*  CREATED DATE  : 2021-01-14 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Sync_HeatParam2AirCondition(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	if(pBlobCabV2->fHeatSetPoint != pIMEU2_RoughData->fHeatPoint_Sync)
	{
		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_HEATING_POINT, (int)pIMEU2_RoughData->fHeatPoint_Sync, TRUE);
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : AirCond_HeatingMgmt
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void AirCond_HeatingMgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL 	bRet = FALSE;
	bRet = Judge_IfStartHeating2Temp(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON , Should Ctrl ON First
		{  
		    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
		}

		pBlobCabV2->emAirMachineState = STAT_AIR_HEATON;
 		pIMEU2_RoughData->fHeatPoint_Sync = 10;
 		Sync_HeatParam2AirCondition(pDev_485_2_Rough, nWhichIMEU);
		return;
	}

	//bRet = Judge_IfStopHeating2Temp(pDev_485_2_Rough, nWhichIMEU);
	//if(bRet == TRUE)
	//{
	//	pIMEU2_RoughData->fHeatPoint_Sync = 5;
 	//	Sync_HeatParam2AirCondition(pDev_485_2_Rough, nWhichIMEU);
	//	return;
	//}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfStartHumidity2Temp
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Judge_IfStartHumidity2Temp(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL bRet = FALSE;
    BOOL bAllHumiTempCommFail = TRUE;
	int  n = 0;

	if(pBlobCabV2->fRealTimeCabHumi >= pIMEU2_RoughData->fHumidity_StartSett) //Default 80
	{	
		for(n = 0; n < pBlobCabV2->nHumiTempSensorNumber; n++)
		{
			if(pBlobCabV2->emTempHumi_CommFail[n] == FALSE)
			{
				if(pBlobCabV2->fHumidity[n] >= (pIMEU2_RoughData->fHumidity_StartSett))//Default 80
				{
					bRet = TRUE;
					break;
				}
			}
		}

		for(n = 0; n < pBlobCabV2->nHumiTempSensorNumber; n++)
		{
			if(pBlobCabV2->emTempHumi_CommFail[n] == FALSE)
			{
				bAllHumiTempCommFail = FALSE;
				break;
			}
		}

		if((bAllHumiTempCommFail == TRUE) || (pBlobCabV2->nHumiTempSensorNumber == 0))
		{
			bRet = TRUE;
		}
  	}

	return bRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Sync_HumidityParam2AirCondition
*  CREATOR       :  
*  CREATED DATE  : 2021-1-14 11:13:53
*  DESCRIPTION   :  DehumiStopTemp_Sync Should Lower fAmbit_TcoolonSett. 
*  Otherwise will Cause AirCond Heating->Cooling->Heating->Cooling
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Sync_HumidityParam2AirCondition(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	//---Sync DeHumi Temp Value ---//
	if(pBlobCabV2->fDehumiStartTemp != pIMEU2_RoughData->fDehumiStartTemp_Sync) 
	{
 		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_DEHUMI_STARTTEMP, (int)pIMEU2_RoughData->fDehumiStartTemp_Sync, FALSE);
	}

	if(pBlobCabV2->fDehumiStopTemp != pIMEU2_RoughData->fDehumiStopTemp_Sync) 
	{
 		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_DEHUMI_STOPTEMP, (int)pIMEU2_RoughData->fDehumiStopTemp_Sync, FALSE);
	}

	//--- Sync De Humidity Value ---//
	if(pBlobCabV2->fDehumiStopHumi != pIMEU2_RoughData->fHumidityStop_Sync)
	{
		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_DEHUMI_STOPHUMIDITY, (int)pIMEU2_RoughData->fHumidityStop_Sync, FALSE);
	}

	if(pBlobCabV2->fDehumiStartHumi != pIMEU2_RoughData->fHumidityStart_Sync)
	{
		SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_DEHUMI_STARTHUMIDITY, (int)pIMEU2_RoughData->fHumidityStart_Sync, FALSE);
	}

	return;	
}

/*********************************************************************************
*  
*  FUNCTION NAME : AirCond_HumidityMgmt
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				   Sync IdleCoolPoint.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void AirCond_HumidityMgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	BOOL 	bRet = FALSE;
 	bRet = Judge_IfStartHumidity2Temp(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_OFF) //0-OFF-> 1-ON , Should Ctrl ON First
		{  
		    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_ON, TRUE);
		}

		pBlobCabV2->emAirMachineState = STAT_AIR_DEHUMIMITY;

		pIMEU2_RoughData->fHumidityStart_Sync 	= 95;
		pIMEU2_RoughData->fHumidityStop_Sync 	= 85;

		pIMEU2_RoughData->fDehumiStartTemp_Sync = 20;
		pIMEU2_RoughData->fDehumiStopTemp_Sync =  25;

 		Sync_HumidityParam2AirCondition(pDev_485_2_Rough, nWhichIMEU);
		return;
	}

	return;
}

static void Judge_IfNeedOffAirCond(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	//A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	BOOL 		bRet = FALSE;
 
	//---Step1. If Need Start Heating
	bRet = Judge_IfStartHeating2Temp(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		return ;
	}

	//---Step2. If Need Start DeHumity
	bRet = Judge_IfStartHumidity2Temp(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		return;
	}

	//---Step3. Judge If Need Stop AirCondtion Cooling---!!! Should be at Last
	bRet = Judge_IfNeedOffAir2TempAndECO(pDev_485_2_Rough, nWhichIMEU);
	if(bRet == TRUE)
	{
		if(pIMEU2_RoughData->blobIMEU2_V20.emOnOrOffd == STAT_DEV_ON) //0-OFF-> 1-ON
		{  
		    SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, STAT_DEV_OFF, TRUE);
		}
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Judge_IfAirMachine_OnIdle
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*				   Only For Refrence, Not A Correct Statu.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Judge_AirStateAccording2LastState(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	
	if(pBlobCabV2->emLast_AirMachineState != pBlobCabV2->emAirMachineState)
	{
		return;
	}

	switch (pBlobCabV2->emLast_AirMachineState)
	{
		case STAT_AIR_IDLE:
			{
				if(pBlobCabV2->fRealTimeCabTemp >= (pBlobCabV2->fCoolSetPoint + pBlobCabV2->fCoolDeviation - 0.5))
				{
					pBlobCabV2->emAirMachineState = STAT_AIR_COOLON;
				}
				else if(pBlobCabV2->fRealTimeCabTemp <= (pBlobCabV2->fHeatSetPoint + 0.5))
				{
					pBlobCabV2->emAirMachineState = STAT_AIR_HEATON;
				}
				else if((pBlobCabV2->fRealTimeCabTemp <= (pBlobCabV2->fDehumiStartTemp + 0.5)) && 
					(pBlobCabV2->fRealTimeCabHumi >= (pBlobCabV2->fDehumiStartHumi - 0.5)))
				{
					pBlobCabV2->emAirMachineState = STAT_AIR_DEHUMIMITY;
				}
				else
				{
					pBlobCabV2->emAirMachineState = STAT_AIR_IDLE;
				}
			}
		case STAT_AIR_COOLON:
			if(pBlobCabV2->fRealTimeCabTemp <= (pBlobCabV2->fCoolSetPoint + 2.0))
			{
				pBlobCabV2->emAirMachineState = STAT_AIR_IDLE;
			}
			break;
		case STAT_AIR_HEATON:
			if(pBlobCabV2->fRealTimeCabTemp >= (pBlobCabV2->fHeatSetPoint+pBlobCabV2->fHeatDeviation - 2.0))
			{
				pBlobCabV2->emAirMachineState = STAT_AIR_IDLE;
			}
			break;
		case STAT_AIR_DEHUMIMITY:
			if((pBlobCabV2->fRealTimeCabTemp >= (pBlobCabV2->fDehumiStopTemp - 2.0)) || 
				(pBlobCabV2->fRealTimeCabHumi <= (pBlobCabV2->fDehumiStopHumi + 2.0)))
			{
				pBlobCabV2->emAirMachineState = STAT_AIR_IDLE;
			}
			break;
		
		default:
			break;
	}
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : AirCond_Mgmt
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : Charging/DisCharging Sync ChrgCoolPoint.
*				   Idle Sync IdleCoolPoint.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void AirCond_Mgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	SAMPLER* pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	if((pBlobCabV2->emAirCond_CommFail == TRUE)||(pBlobCabV2->emAirCondTotalAlm == TRUE))
	{
		TRACEX("emAirCond_CommFail=[%d],emAirCondTotalAlm=[%d]\n",pBlobCabV2->emAirCond_CommFail, pBlobCabV2->emAirCondTotalAlm);
		return ;
	}
	
	BOOL 		bRet = FALSE;
	VAR_VALUE 	varV;
	
 	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_RAW_VAL, 501 + nWhichIMEU, 558, &varV, 4);
	pIMEU2_RoughData->fBattTmax_Samp = varV.fValue;

	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_RAW_VAL, 501 + nWhichIMEU, 559, &varV, 4);
	pIMEU2_RoughData->fBattTmin_Samp = varV.fValue;

	TRACE("SP:Idx[%d]Stat[%d]ECO[%d]Tair[%.1f]HUMIair[%.0f] Tambit[0][1][%.1f][%.1f]BAT_Tmax|Tmin[%.1f][%.1f]\
		HUMIambit[0][1][%.0f][%.0f]Heat[%.0f]HumiON|OFF[%.0f][%.0f]\n",\
		nWhichIMEU, pBlobCabV2->emAirMachineState, pIMEU2_RoughData->emECORunStatu,pBlobCabV2->fRealTimeCabTemp, pBlobCabV2->fRealTimeCabHumi, pBlobCabV2->fTemperature[0], \
		pBlobCabV2->fTemperature[1], pIMEU2_RoughData->fBattTmax_Samp, pIMEU2_RoughData->fBattTmin_Samp,\
		pBlobCabV2->fHumidity[0], pBlobCabV2->fHumidity[1], pBlobCabV2->fHeatSetPoint, pBlobCabV2->fDehumiStartHumi, pBlobCabV2->fDehumiStopHumi);
 
	TRACE("ST:Ambit_Tcoolon[%.0f]Bat_Tmax_coolon[%.0f]Ambit_Tcooloff[%.0f]\
		Bat_Tmax_cooloff[%.0f]AirEnTran_Tlmt[%.0f]CoolSetPoint=[%.0f]CoolDeviation[%.0f]\n",\
		pIMEU2_RoughData->fAmbit_TcoolonSett,pIMEU2_RoughData->fBatt_Tmax_coolonSett,pIMEU2_RoughData->fAmbit_TcooloffSett,\
		pIMEU2_RoughData->fBatt_Tmax_cooloffSett,pIMEU2_RoughData->fAirEnTran_TlmtSett,pBlobCabV2->fCoolSetPoint, pBlobCabV2->fCoolDeviation);
	
	AirCond_CoolingMgmt(pDev_485_2_Rough, nWhichIMEU);

	AirCond_HeatingMgmt(pDev_485_2_Rough, nWhichIMEU);

	AirCond_HumidityMgmt(pDev_485_2_Rough, nWhichIMEU);

	Judge_IfNeedOffAirCond(pDev_485_2_Rough, nWhichIMEU);

 	Judge_AirStateAccording2LastState(pDev_485_2_Rough, nWhichIMEU);

	if(pBlobCabV2->emLast_AirMachineState != pBlobCabV2->emAirMachineState)
	{
		//LogOut(MAIN_MODULE_SAMPLER, "IMEU2", LOG_TYPE_INFO, "Idx[%d]StateMachine[%d]LastStateMachine[%d]\n", \
		//	nWhichIMEU, pBlobCabV2->emAirMachineState, pBlobCabV2->emLast_AirMachineState);

		pBlobCabV2->emLast_AirMachineState = pBlobCabV2->emAirMachineState;
	}

 	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Sync_CoolPoint2IMEU2
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : Charging/DisCharging Sync ChrgCoolPoint.
*				   Idle Sync IdleCoolPoint.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
/*void Sync_CoolPoint2IMEU2(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	SAMPLER* pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	VAR_VALUE varV;
	STBATT_RUN_STATU emBattStatus; 
	ERRCODE_SAMPLER bRet = ERR_CONTROL_OK;

	//Batt Charge Status
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_RAW_VAL, 501 + nWhichIMEU, 550, &varV, 4);
	emBattStatus = (STBATT_RUN_STATU)varV.emValue;
	TRACE("nWhichIMEU=[%d] Curr BattStatus:[%d] Last BattStatus:[%f] fChrgCoolPoint=[%f] fIdleCoolPoint=[%.f]\n", \
		nWhichIMEU, emBattStatus, \
		pIMEU2_RoughData->fLast_SetCoolPoint2AirCond,\
		pIMEU2_RoughData->fChrgCoolPoint,\
		pIMEU2_RoughData->fIdleCoolPoint);

	if((emBattStatus == STBATT_RUNSTAT_CHARGE) || (emBattStatus == STBATT_RUNSTAT_DISCHARGE))
	{
		if(pIMEU2_RoughData->fLast_SetCoolPoint2AirCond != pIMEU2_RoughData->fChrgCoolPoint)
		{
			TRACE("nWhichIMEU=[%d] Send Chrg Cool Point Value[%d]\n", nWhichIMEU, (int)pIMEU2_RoughData->fChrgCoolPoint);
			//SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_CRYOGEN_POINT, (int)pIMEU2_RoughData->fChrgCoolPoint);
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_CRYOGEN_POINT, (int)pIMEU2_RoughData->fChrgCoolPoint);
			if(bRet == ERR_CONTROL_OK)
			{
				pIMEU2_RoughData->fLast_SetCoolPoint2AirCond = pIMEU2_RoughData->fChrgCoolPoint;
			}
		}
	}
	else
	{
		if(pIMEU2_RoughData->fLast_SetCoolPoint2AirCond != pIMEU2_RoughData->fIdleCoolPoint)
		{
			TRACE("nWhichIMEU=[%d] Send Idle Cool Point Value[%d]\n", nWhichIMEU, (int)pIMEU2_RoughData->fIdleCoolPoint);
			//SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_CRYOGEN_POINT, (int)pIMEU2_RoughData->fIdleCoolPoint);
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_SET_AIRCOND_CRYOGEN_POINT, (int)pIMEU2_RoughData->fIdleCoolPoint);
			if(bRet == ERR_CONTROL_OK)
			{
				pIMEU2_RoughData->fLast_SetCoolPoint2AirCond = pIMEU2_RoughData->fIdleCoolPoint;
			}
		}
	}
	
	return;
}
*/
/*********************************************************************************
*  
*  FUNCTION NAME : Judge_CabSafeRunState
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
CAB_SAFE_MANAGE_STATE Judge_CabSafeRunState(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{   
	SAMPLER	*pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	
	VAR_VALUE varV;
	UINT emAlmValue_DI1 = 0, emAlmValue_DI2 = 0, emAlmValue_DI4 = 0;
	
	//	DI1-Fire Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 460, &varV, 4);
	emAlmValue_DI1 = (pIMEU2_RoughData->blobIMEU2_V20.emDI1_Fire == varV.emValue) ? 1 : 0;

   	//	DI2-Smoke Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 461, &varV, 4);
	emAlmValue_DI2 = (pIMEU2_RoughData->blobIMEU2_V20.emDI2_Smoke == varV.emValue) ? 1 : 0;

    if(emAlmValue_DI1 || emAlmValue_DI2)   
	{	
		TRACE("Warning: 3rd-> emAlmValue_DI1=[%d] emAlmValue_DI2=[%d] emDI1_Fire=[%d] emDI2_Smoke=[%d]\n",\
			pIMEU2_RoughData->blobIMEU2_V20.emDI1_Fire, pIMEU2_RoughData->blobIMEU2_V20.emDI2_Smoke);
		return CAB_SAFE_STATE_THIRD;
	}

	//Level.1 Alarm, Judge Condition Potency Over 20%.
	//DI4-Flammable Gas Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 463, &varV, 4);
	emAlmValue_DI4 = (pIMEU2_RoughData->blobIMEU2_V20.emDI4_FlammableGas == varV.emValue) ? 1 : 0;
	if(emAlmValue_DI4)  
	{	
		TRACE("Warning: 2nd->emDI4_FlammableGas=[%d]\n", pIMEU2_RoughData->blobIMEU2_V20.emDI4_FlammableGas);
		return CAB_SAFE_STATE_SECOND;
	}
 	
	//TRACE("Warning: Idle->emDI1_Fire=[%d] emDI2_Smoke=[%d] emDI4_FlammableGas=[%d]\n",\
	//		pIMEU2_RoughData->blobIMEU2_V20.emDI1_Fire, \
	//		pIMEU2_RoughData->blobIMEU2_V20.emDI2_Smoke,\
	//		pIMEU2_RoughData->blobIMEU2_V20.emDI4_FlammableGas);

	return CAB_SAFE_STATE_IDLE;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  AirCond_ECOMode
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

static STATU_ECO_CODE Judge_IfNeedExitECOMode(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	int nSector = 0;
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

	//--- Step1. ECO Enable Setting  ---//

	if(pDev_485_2_Rough->emECOModeSetting == SETT_NORMAL_RUN) //Not ECO Mode
	{
		return CODE_DISABLE_ECO; 
	}

	//--- Step4. EMS Mgmt Plan ---//
	nSector = Get_EmsCfgPlan_Setctor(&pDev_485_2_Rough->blobEmsCfgPlan, TRUE);
    if(nSector == -1)
    {
		return CODE_WRONG_EMSPLAN;
	}

	EMS_LIMIT_SECTOR *pEMSSector = &pDev_485_2_Rough->blobEmsCfgPlan.emsLmtSectors[nSector];
	if((pEMSSector->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG) &&
		(pIMEU2_RoughData->blobSingleBattData.nSOC > pDev_485_2_Rough->nProtectSOC) &&
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run != STBATT_RUNSTAT_SLEEP) &&
		(pEMSSector->nActvPwrLmt != 0) &&
		(pEMSSector->nBattGrpPwrLmt != 0))
	{
		return CODE_DISCHRG_MODE;
	}

	if((pEMSSector->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING) &&
		(pIMEU2_RoughData->blobSingleBattData.nSOC < pEMSSector->nStopSOC) &&
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run != STBATT_RUNSTAT_SLEEP) &&
		(pEMSSector->nStopSOC > pDev_485_2_Rough->nProtectSOC) &&
		(pEMSSector->nActvPwrLmt != 0) &&
		(pEMSSector->nBattGrpPwrLmt != 0))
	{
		return CODE_NEED_CHRG;
	}

	if((pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run == STBATT_RUNSTAT_CHARGE) ||
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run == STBATT_RUNSTAT_DISCHARGE) ||
		(ABS(pIMEU2_RoughData->blobSingleBattData.fPackTotalCurr) > 20.0))
	{
		return CODE_BATT_RUNNING;
	}

	return CODE_NONEED_EXIT;
}


static STATU_ECO_CODE Judge_IfNeedEnterECOMode(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	int 	nSector = 0;
	time_t 	tm_Now = _NOW_;
	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

	
	if(pDev_485_2_Rough->emECOModeSetting == SETT_NORMAL_RUN)  
	{
		return CODE_DISABLE_ECO;
	}

	if(ABS(tm_Now - pIMEU2_RoughData->tm_LastChangeECOMode) < ECO_CHANGE_TIMEOUT) //10Min
	{
		return CODE_ENTERECO_BELOW5MIN;
	}

	nSector = Get_EmsCfgPlan_Setctor(&pDev_485_2_Rough->blobEmsCfgPlan, TRUE);
    if(nSector == -1)
    {
		return CODE_WRONG_EMSPLAN;
	}

	EMS_LIMIT_SECTOR *pEMSSector = &pDev_485_2_Rough->blobEmsCfgPlan.emsLmtSectors[nSector];
	TRACE("emBatt_WorkMode=[%d]nInterval=[%d]nStopSOC=[%d]nRealTimeSOC=[%d]emBattStatus_Run=[%d]fPackTotalCurr=[%.1f]nProtectSOC[%d]\n", \
		pEMSSector->emBatt_WorkMode,\
		pEMSSector->nInterval,\
		pEMSSector->nStopSOC,\
		pIMEU2_RoughData->blobSingleBattData.nSOC,\
		pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run,\
		ABS(pIMEU2_RoughData->blobSingleBattData.fPackTotalCurr),\
		pDev_485_2_Rough->nProtectSOC);
	
	if((pEMSSector->emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG) &&
		(pIMEU2_RoughData->blobSingleBattData.nSOC > pDev_485_2_Rough->nProtectSOC) &&
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run != STBATT_RUNSTAT_SLEEP) &&
		(pEMSSector->nActvPwrLmt != 0) &&
		(pEMSSector->nBattGrpPwrLmt != 0))
	{
		return CODE_DISCHRG_MODE;
	}

	if((pEMSSector->emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING) &&
		(pIMEU2_RoughData->blobSingleBattData.nSOC < pEMSSector->nStopSOC) &&
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run != STBATT_RUNSTAT_SLEEP) &&
		(pEMSSector->nStopSOC > pDev_485_2_Rough->nProtectSOC) &&
		(pEMSSector->nActvPwrLmt != 0) &&
		(pEMSSector->nBattGrpPwrLmt != 0))
	{
		return CODE_NEED_CHRG;
	}

	if((pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run == STBATT_RUNSTAT_CHARGE) ||
		(pIMEU2_RoughData->blobSingleBattData.emBattStatus_Run == STBATT_RUNSTAT_DISCHARGE) ||
		(ABS(pIMEU2_RoughData->blobSingleBattData.fPackTotalCurr) > 20.0))
	{
		return CODE_BATT_RUNNING;
	}

	if(pEMSSector->nInterval < 1800)
	{
		return CODE_EMS_INTRVL_SHORT;
	}

	return CODE_ENTER_ECO;
}

static void AirCond_ECOMode(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU)
{
	time_t tm_Now = _NOW_;

	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

	if(pIMEU2_RoughData->emECORunStatu == STAT_ECO_MODE)
	{
		pIMEU2_RoughData->emStatCode = Judge_IfNeedExitECOMode(pDev_485_2_Rough, nWhichIMEU);
		if(pIMEU2_RoughData->emStatCode != CODE_NONEED_EXIT)
		{
			pIMEU2_RoughData->emECORunStatu = STAT_NORMAL_RUN;
			pIMEU2_RoughData->tm_LastChangeECOMode = tm_Now;

			LogOut(MAIN_MODULE_SAMPLER, "485_2_Samp", LOG_TYPE_ERROR, "Exit ECO:emStatCode[%d]ECOSetting[%d]ECORunStat[%d]",
            	pIMEU2_RoughData->emStatCode, pDev_485_2_Rough->emECOModeSetting, pIMEU2_RoughData->emECORunStatu);
		}
	}
	else
	{
		pIMEU2_RoughData->emStatCode = Judge_IfNeedEnterECOMode(pDev_485_2_Rough, nWhichIMEU);
		if(pIMEU2_RoughData->emStatCode == CODE_ENTER_ECO)
		{
			pIMEU2_RoughData->emECORunStatu = STAT_ECO_MODE;
			pIMEU2_RoughData->tm_LastChangeECOMode = tm_Now;

			LogOut(MAIN_MODULE_SAMPLER, "485_2_Samp", LOG_TYPE_ERROR, "Enter ECO:emStatCode[%d]ECOSetting[%d]ECORunStat[%d]",
			pIMEU2_RoughData->emStatCode, pDev_485_2_Rough->emECOModeSetting, pIMEU2_RoughData->emECORunStatu);
		}
	}

	TRACE("tm_Now=[%ld] tm_LastChangeECOMode=[%ld] emStatCode=[%d] emECOModeSetting=[%d] emECORunStatu=[%d]\n", \
		tm_Now, pIMEU2_RoughData->tm_LastChangeECOMode, pIMEU2_RoughData->emStatCode, \
		pDev_485_2_Rough->emECOModeSetting,\
		pIMEU2_RoughData->emECORunStatu);

	return TRUE;
}

static void Refresh_RealTimeDataInfo(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{
	SAMPLER	*pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	VAR_VALUE varV;

	varV.blobValue = (BYTE *)&pIMEU2_RoughData->blobSingleBattData;
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_VAR, 
		501 + nWhichIMEU, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));

	
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 1, 6, &varV, 4);
	pDev_485_2_Rough->emECOModeSetting = varV.emValue;
 	
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 500, 503, &varV, 4);
	pDev_485_2_Rough->nProtectSOC = varV.ulValue;


	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : State_Machine Entrance
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CabSafe_StateMachine(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
{
    IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];

	//Step.1 
    pIMEU2_RoughData->emCabSafeState = Judge_CabSafeRunState(pDev_485_2_Rough, nWhichIMEU);
    if((pIMEU2_RoughData->emCabSafeState < CAB_SAFE_STATE_IDLE)||(pIMEU2_RoughData->emCabSafeState >= CAB_SAFE_STATE_MAX))
    {
        TRACE("Error: emCabSafeState=[%d]\n", pIMEU2_RoughData->emCabSafeState);
        return;
    }
    
	if((pIMEU2_RoughData->emManageCtrlMode == MANAGE_CTRL_MODE_MANUAL) || 
		(pIMEU2_RoughData->blobIMEU2_V20.emIMEU2BMSCommState == 0))  //0-Normal,1-CommFail.Manual Mode or BMS And IMEU Comm Mormal
	{
		pIMEU2_RoughData->emCabSafeState = CAB_SAFE_STATE_IDLE;
		pIMEU2_RoughData->emECORunStatu = STAT_NORMAL_RUN;
		//if IMEU2 Comm2BMS OK,means BMS is Infy BMS, BMS Do Cab Safety Function. 
		return;
	}
	else  //Auto Mode
	{
		//Sync_CoolPoint2IMEU2(pDev_485_2_Rough, nWhichIMEU);
		Refresh_RealTimeDataInfo(pDev_485_2_Rough, nWhichIMEU);

		AirCond_ECOMode(pDev_485_2_Rough, nWhichIMEU);

		AirCond_Mgmt(pDev_485_2_Rough, nWhichIMEU);
	 
		pIMEU2_RoughData->pCurr_StateMachineItem = &pDev_485_2_Rough->StateMachineLoop[pIMEU2_RoughData->emCabSafeState];

		pIMEU2_RoughData->pCurr_StateMachineItem->pfnAction(pDev_485_2_Rough, nWhichIMEU);
		if(pIMEU2_RoughData->emCabSafeState != pIMEU2_RoughData->emLast_CabSafeState)
		{
			pIMEU2_RoughData->emLast_CabSafeState = pIMEU2_RoughData->emCabSafeState;
			pIMEU2_RoughData->bCabSafeStateChanged = TRUE;
			TRACE("emLast_CabSafeState=[%d] emCabSafeState=[%d]\n", pIMEU2_RoughData->emLast_CabSafeState, pIMEU2_RoughData->emCabSafeState);
		}
		else
		{
			pIMEU2_RoughData->bCabSafeStateChanged = FALSE;
		}
	}
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : BattCab_Safety_Management
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   : Battery Cabinet Management
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void BattCab_Safety_Management(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU)
{
	if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail == TRUE)
	{
		return;
	}

	Dev_PT1000_Mgmt(pDev_485_2_Rough, nWhichIMEU);

	Dev_ShutterFan_Mgmt(pDev_485_2_Rough, nWhichIMEU);   

	CabSafe_StateMachine(pDev_485_2_Rough, nWhichIMEU);

	return;
}



