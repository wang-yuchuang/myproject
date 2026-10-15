#include "dpr.h"
#include "run_expression.h"

static UINT s_nCurrHisDataIdx = 1;
static void Alarm_Begin(CHARGER* pCharger, ALARM*pAlarm)
{
	if(pAlarm->emStatus != ALARM_STATUS_ACTIVE)
	{
		pAlarm->emStatus = ALARM_STATUS_ACTIVE;
		pAlarm->tmLastOccured = time(NULL);
		//Now raise alarm occur event and save/send out/**/
		EVENT_ARG arg;
		arg.bBlockMode = FALSE;
		arg.emEvType = EVENT_TYPE_ALARM;
		arg.emMsgType = EVENT_MSG_BEGIN;
		arg.nEvDataSize = sizeof(EVENT_ALARM); //use pointer
		arg.pEvData = (void*)NEW(EVENT_ALARM, 1);
		((EVENT_ALARM*)arg.pEvData)->nDevId = ((DEVICE*)pAlarm->hDevice)->info.nDeviceId;
		((EVENT_ALARM*)arg.pEvData)->nAlarmId = pAlarm->pAlarmInfo->nAlarmId;
		((EVENT_ALARM*)arg.pEvData)->status = pAlarm->emStatus;
		((EVENT_ALARM*)arg.pEvData)->tmStart = pAlarm->tmLastOccured;
		((EVENT_ALARM*)arg.pEvData)->tmEnd = 0;
		((EVENT_ALARM*)arg.pEvData)->fExParamVal = (pAlarm->pExParam == NULL) ? 0.0 : pAlarm->pExParam->currValue.fValue;
		arg.pfnReceipt = NULL;
		HandleEvent(pCharger, (HANDLE)&pCharger->dataprocessor, &arg);
	}
}

static void Alarm_Cease(CHARGER* pCharger, ALARM*pAlarm)
{
	//Jimmy changed 202100909: need cease those suppressed ALARM as well!!!
	//if(pAlarm->emStatus == ALARM_STATUS_ACTIVE)
	if(pAlarm->emStatus != ALARM_STATUS_CEASED)
	{
		pAlarm->emStatus = ALARM_STATUS_CEASED;
		pAlarm->tmLastCeased = time(NULL);
		//Now raise alarm occur event and save/send out/**/
		EVENT_ARG arg;
		arg.bBlockMode = FALSE;
		arg.emEvType = EVENT_TYPE_ALARM;
		arg.emMsgType = EVENT_MSG_END;
		arg.nEvDataSize = sizeof(EVENT_ALARM); //use pointer
		arg.pEvData = (void*)NEW(EVENT_ALARM, 1);
		((EVENT_ALARM*)arg.pEvData)->nDevId = ((DEVICE*)pAlarm->hDevice)->info.nDeviceId;
		((EVENT_ALARM*)arg.pEvData)->nAlarmId = pAlarm->pAlarmInfo->nAlarmId;
		((EVENT_ALARM*)arg.pEvData)->status = pAlarm->emStatus;
		((EVENT_ALARM*)arg.pEvData)->tmStart = pAlarm->tmLastOccured;
		((EVENT_ALARM*)arg.pEvData)->tmEnd = pAlarm->tmLastCeased;
		arg.pfnReceipt = NULL;
		HandleEvent(pCharger, (HANDLE)&pCharger->dataprocessor, &arg);
	}
	// else if(pAlarm->emStatus == ALARM_STATUS_SUPPRESSED)
	// {
	// 	pAlarm->emStatus = ALARM_STATUS_CEASED;
	// 	//pAlarm->tmLastCeased = time(NULL);
	// }
}

static void Alarm_Suppress(CHARGER* pCharger, ALARM*pAlarm)
{
	if(pAlarm->emStatus == ALARM_STATUS_ACTIVE)
	{
		pAlarm->emStatus = ALARM_STATUS_SUPPRESSED;
		pAlarm->tmLastCeased = time(NULL); //suppress also consider as 'ceased'
		//Now raise alarm occur event and save/send out/**/
		EVENT_ARG arg;
		arg.bBlockMode = FALSE;
		arg.emEvType = EVENT_TYPE_ALARM;
		arg.emMsgType = EVENT_MSG_STATUS_CHANGED; //means need remove this from db
		arg.nEvDataSize = sizeof(EVENT_ALARM); //use pointer
		arg.pEvData = (void*)NEW(EVENT_ALARM, 1);
		((EVENT_ALARM*)arg.pEvData)->nDevId = ((DEVICE*)pAlarm->hDevice)->info.nDeviceId;
		((EVENT_ALARM*)arg.pEvData)->nAlarmId = pAlarm->pAlarmInfo->nAlarmId;
		((EVENT_ALARM*)arg.pEvData)->status = pAlarm->emStatus;
		((EVENT_ALARM*)arg.pEvData)->tmStart = pAlarm->tmLastOccured;
		((EVENT_ALARM*)arg.pEvData)->tmEnd = _NOW_;
		//((EVENT_ALARM*)arg.pEvData)->fExParamVal = (pAlarm->pExParam == NULL) ? 0.0 : pAlarm->pExParam->currValue.fValue;
		arg.pfnReceipt = NULL;
		HandleEvent(pCharger, (HANDLE)&pCharger->dataprocessor, &arg);
	}
}

static void Refresh_StatusParam(DEVICE* pDev, PARAMETER* pParam)
{
	STATUS_PARAM_INFO* pStatusParamInfo;
	EXP_CONST result;
	pStatusParamInfo = (STATUS_PARAM_INFO*)pParam->pParamInfo;
	char *szParamName = (pParam->pParamInfo->pRes_Name == NULL) ? "NA" : pParam->pParamInfo->pRes_Name->szName;
	if(pStatusParamInfo->emDataSrc == PARAM_SRC_CA)
	{
		//calculate parameter value
		pParam->bIsValueInvalid = FALSE;
		if( Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pStatusParamInfo->equalExpression.p, 
			pStatusParamInfo->equalExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			if(pParam->pParamInfo->emValueType == VALUE_TYPE_FLOAT)
			{
				pParam->lastValue.fValue = pParam->currValue.fValue;
				pParam->tmLastValueChanged = pParam->tmCurrValueRefreshed;
				pParam->tmCurrValueRefreshed = _NOW_;
				pParam->currValue.fValue = (float)result;
			}
			else if((pParam->pParamInfo->emValueType == VALUE_TYPE_ULONG) ||
				(pParam->pParamInfo->emValueType == VALUE_TYPE_ENUM))
			{
				pParam->lastValue.lValue = pParam->currValue.lValue;
				pParam->tmLastValueChanged = pParam->tmCurrValueRefreshed;
				pParam->tmCurrValueRefreshed = _NOW_;
				pParam->currValue.ulValue = (LONG)result;
			}
			else if(pParam->pParamInfo->emValueType == VALUE_TYPE_LONG)
			{
				pParam->lastValue.lValue = pParam->currValue.lValue;
				pParam->tmLastValueChanged = pParam->tmCurrValueRefreshed;
				pParam->tmCurrValueRefreshed = _NOW_;
				pParam->currValue.lValue = (LONG)result;
			}
			else
			{
				pParam->bIsValueInvalid = TRUE;
			}
		}
		else
		{
			pParam->bIsValueInvalid = TRUE;
		}
	}

	//Now refresh display expression
	if((pParam->pParamInfo->displayExpression.nTotal > 0) && (!pParam->pParamInfo->bDisableDisplay))
	{
		if(Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pParam->pParamInfo->displayExpression.p, 
			pParam->pParamInfo->displayExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			pParam->bDisplayAble = (BOOL)result;
		}
		else
		{
			pParam->bDisplayAble = FALSE;
		}
	}
	else
	{
		pParam->bDisplayAble = (pParam->pParamInfo->bDisableDisplay) ? FALSE : TRUE;
	}
}

static void Refresh_CtrlParam(DEVICE* pDev, PARAMETER* pParam)
{
	CONTROL_PARAM_INFO* pCtrlParamInfo;
	EXP_CONST result;
	pCtrlParamInfo = (CONTROL_PARAM_INFO*)pParam->pParamInfo;
	char *szParamName = (pParam->pParamInfo->pRes_Name == NULL) ? "NA" : pParam->pParamInfo->pRes_Name->szName;
	//Now refresh display expression
	if((pParam->pParamInfo->displayExpression.nTotal > 0) && (!pParam->pParamInfo->bDisableDisplay))
	{
		if(Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pParam->pParamInfo->displayExpression.p, 
			pParam->pParamInfo->displayExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			pParam->bDisplayAble = (BOOL)result;
		}
		else
		{
			pParam->bDisplayAble = FALSE;
		}
	}
	else
	{
		pParam->bDisplayAble = (pParam->pParamInfo->bDisableDisplay) ? FALSE : TRUE;
	}
	
	//Now refresh controllable expression
	if(pCtrlParamInfo->controlableExpression.nTotal > 0)
	{
		if(Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pCtrlParamInfo->controlableExpression.p, 
			pCtrlParamInfo->controlableExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			pParam->bCtrlSetAble = (BOOL)result;
		}
		else
		{
			pParam->bCtrlSetAble = FALSE;
		}
	}
	else
	{
		pParam->bCtrlSetAble = TRUE;
	}
}

static void Refresh_SettParam(DEVICE* pDev, PARAMETER* pParam)
{
	SETTING_PARAM_INFO* pSettParamInfo;
	EXP_CONST result;
	pSettParamInfo = (SETTING_PARAM_INFO*)pParam->pParamInfo;
	char *szParamName = (pParam->pParamInfo->pRes_Name == NULL) ? "NA" : pParam->pParamInfo->pRes_Name->szName;
	//Now refresh display expression
	if((pParam->pParamInfo->displayExpression.nTotal > 0) && (!pParam->pParamInfo->bDisableDisplay))
	{
		if(Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pParam->pParamInfo->displayExpression.p, 
			pParam->pParamInfo->displayExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			pParam->bDisplayAble = (BOOL)result;
		}
		else
		{
			pParam->bDisplayAble = FALSE;
		}
	}
	else
	{
		pParam->bDisplayAble = (pParam->pParamInfo->bDisableDisplay) ? FALSE : TRUE;
	}

	//Now refresh settable expression
	if(pSettParamInfo->settableExpression.nTotal > 0)
	{
		if(Exp_Calculate(pDev->info.nDeviceId, szParamName, 
			pSettParamInfo->settableExpression.p, 
			pSettParamInfo->settableExpression.nTotal,
			&result) == EXP_ETYPE_CONST)
		{
			pParam->bCtrlSetAble = (BOOL)result;
		}
		else
		{
			pParam->bCtrlSetAble = FALSE;
		}
	}
	else
	{
		pParam->bCtrlSetAble = TRUE;
	}

	//if(pParam->pParamInfo->nParamId == 9)
	//{
	//	TRACE("----Now set[%d] param .settable = %d ,setable ExpTotal = %d-----.\n", pParam->pParamInfo->nParamId, pParam->bCtrlSetAble, pSettParamInfo->settableExpression.nTotal);
	//}
}

static void Refresh_Alarm(CHARGER* pCharger, DEVICE* pDev, ALARM* pAlarm)
{
	EXP_CONST result;
	BOOL bAlarm, bAlarmMsk;
	if(pAlarm->pAlarmInfo->alarmCondExpression.nTotal <= 0)
	{
		return;
	}

	char *szAlarmName = (pAlarm->pAlarmInfo->pRes_Name == NULL) ? "NA" : pAlarm->pAlarmInfo->pRes_Name->szName;
	if(Exp_Calculate(pDev->info.nDeviceId, szAlarmName, 
		pAlarm->pAlarmInfo->alarmCondExpression.p, 
		pAlarm->pAlarmInfo->alarmCondExpression.nTotal,
		&result) == EXP_ETYPE_CONST)
	{
		bAlarm = (BOOL)result;
		if(!bAlarm)
		{
			Alarm_Cease(pCharger, pAlarm);
		}
		else
		{
			if(pAlarm->pAlarmInfo->alarmMaskExpression.nTotal > 0)
			{
				if(Exp_Calculate(pDev->info.nDeviceId, szAlarmName, 
					pAlarm->pAlarmInfo->alarmMaskExpression.p, 
					pAlarm->pAlarmInfo->alarmMaskExpression.nTotal,
					&result) == EXP_ETYPE_CONST)
				{
					//printf("111 Alarm_Suppress: pDev->info.nDeviceId=%d, szAlarmName=%s\n",\
					//	pDev->info.nDeviceId, szAlarmName);

					bAlarmMsk = (BOOL)result;
					if(bAlarmMsk)
					{
						//printf("222 Alarm_Suppress: pDev->info.nDeviceId=%d, szAlarmName=%s\n",\
						//pDev->info.nDeviceId, szAlarmName);
						Alarm_Suppress(pCharger, pAlarm);
						return;
					}
				}
			}

			Alarm_Begin(pCharger, pAlarm);
		}
	}
}

static int Get_HisParamIdx(void* pInternalArdg, int nColomn, char** ppszColumData, char** ppszColumnName)
{
	UNUSED(pInternalArdg);
	UNUSED(ppszColumnName);
	if(nColomn < 1)
	{
		return 1;
	}
	//int n = 0;
	//for(n = 0; n < nColomn -1;n++)
	//{
	//	ppszColumData += sizeof(UINT);
	//}
	//s_nCurrHisDataIdx = *(UINT*)ppszColumData;
	s_nCurrHisDataIdx = atoi(ppszColumData[0]) + 1;
	if(s_nCurrHisDataIdx > MAX_HISDATA_RECORDS)
	{
		s_nCurrHisDataIdx = 1;
	}
	return 0;
}

static void Refresh_HisParam(CHARGER* pCharger, HISPARAM_INFO* pParam)
{
	double tmNow = GetCurrentTime();
	//BOOL bTimeOut = FALSE;
	PARAMETER* pParamInfo = GetParam_ByPR(PARAM_TYPE_STATUS, pParam->nDevId, pParam->nParamId);
	if(pParamInfo == NULL)
	{
		TRACEX("RRRRRCan not find such param[%d, %d]\n", pParam->nDevId, pParam->nParamId);
		return;
	}
	else if(pParamInfo->bIsValueInvalid)
	{
		TRACEX("The param[%d, %d] value is INVALID so can not handle history data\n", 
			pParam->nDevId, pParam->nParamId);
		return;
	}
	//char *szParamName = (pParam->pParam->pParamInfo->pRes_Name == NULL) ? "NA" : pParam->pParam->pParamInfo->pRes_Name->szName;
	
	//if(pParam->emTriggleType == HISTRIGGLE_PERIODLY)
	// {
	// 	bTimeOut = (fabs(tmNow - pParam->tmLastRecord) >= pParam->fPeriod_mSec) ? TRUE : FALSE;
	// }

	//if(bTimeOut/* && (fabs(pParamInfo->currValue.fValue - pParam->fLastValue_Recorded) > 0.2)*/)
	{
		int nCurrIdx = 0;
		BOOL bNeedWrite= FALSE;
		float fLastValue = -999999.0;
		if(pParam->nHisValsTotal <= 0)
		{
			pParam->nHisValsTotal = 0;
			fLastValue = pParam->hisValus[0].fValue; //use the first value
		}
		else if(pParam->nHisValsTotal >= MAX_TEMP_STORED_HISVALS)
		{
			pParam->nHisValsTotal = MAX_TEMP_STORED_HISVALS;
			bNeedWrite = TRUE;
		}
		else 
		{
			nCurrIdx = pParam->nHisValsTotal - 1;
			fLastValue = pParam->hisValus[nCurrIdx].fValue;
		}

		if(!bNeedWrite)
		{			
			//Jimmy changed 0.4=>0.9 20200922 to shorten the write interval
			//if(fabs(pParamInfo->currValue.fValue - fLastValue) > 0.9)		
			//Jimmy changed 20210317: use the configDB threashold instead of fixed value
			//!!!Jimmy note: min-resolution is 1.0
			if(fabs(pParamInfo->currValue.fValue - fLastValue) > MAX(1.0, pParam->fTriggle_GreaterThan))
			{		
				nCurrIdx = pParam->nHisValsTotal;		
				pParam->hisValus[nCurrIdx].fValue = pParamInfo->currValue.fValue;
				pParam->hisValus[nCurrIdx].tmSampled = tmNow;
				pParam->nHisValsTotal++;	
				TRACEX("--Now added new value[%.2f] to param[%d,%d[, total = %d--\n",
					pParam->hisValus[nCurrIdx].fValue, pParam->nDevId, pParam->nParamId,
					pParam->nHisValsTotal);
			}

			if(pParam->nHisValsTotal >= 1) //Jimmy changed 20210318 from >1 to >=1
			{
				float fDiff = fabs(tmNow - pParam->tmLastRecord);
				//Jimmy 20210318: change interv 15 => 60
				if(fDiff > 60.0) //write it in every 60 sec
				{
					bNeedWrite = TRUE;
				}
			}
			
			if(!bNeedWrite)
			{
				return;
			}			
		}
		
		//now write data to db (10 sec or 200items)
		TRACEX("Now write hisData for param[%d, %d], timeDiff = %.2f, total = %d, s_nCurrHisDataIdx=%d", pParam->nDevId, pParam->nParamId,
			tmNow - pParam->tmLastRecord, pParam->nHisValsTotal, s_nCurrHisDataIdx);
		sqlite3 *data_db = Open_Lock_SqliteDB();
		sqlite3_exec(data_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);	
		int n;
		int nSqlRet;
		for(n = 0; n < pParam->nHisValsTotal; n++)
		{
			sqlite3_stmt * stat;
			sqlite3_prepare_v2(data_db,"replace into Data_HisStatusData values (?,?,?,?,?);",-1,&stat,NULL);
			sqlite3_bind_int(stat,1, (int)s_nCurrHisDataIdx);
			sqlite3_bind_double(stat, 2, pParam->hisValus[n].tmSampled);
			sqlite3_bind_int(stat,3, pParam->nDevId);
			sqlite3_bind_int(stat,4, pParam->nParamId);
			sqlite3_bind_double(stat, 5, (double)pParam->hisValus[n].fValue);			
			nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat	
			s_nCurrHisDataIdx++;
			if(s_nCurrHisDataIdx > MAX_HISDATA_RECORDS)
			{
				s_nCurrHisDataIdx = 1;
			}
		}

		if(nSqlRet != SQLITE_DONE)
		{
			TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_HistoricalData values (?,?,?,?,?);", 
				nSqlRet, sqlite3_errmsg(data_db));
			//system("reboot");
		}
		sqlite3_exec(data_db, "COMMIT;", NULL, NULL, NULL);	
		Close_UnLock_SqliteDB();

		//now needs to record the write and reset the varList
		pParam->tmLastRecord = tmNow;
		//use the last value as first value for next round
		pParam->hisValus[0].fValue = pParam->hisValus[pParam->nHisValsTotal - 1].fValue;
		pParam->nHisValsTotal = 0;
	}
}

DWORD _ThreadEntry_DataProcessing(void* pArgs)
{
	CHARGER* pCharger = (CHARGER*)pArgs;
	PARAMETER* pParam;
	DEVICE* pDev;
	ALARM* pAlarm;
	HISPARAM_INFO* pHisParam;
	int n;
	int i;
	BOOL bHasHisParam = (pCharger->info.hisParams_info.nTotal > 0) ? TRUE : FALSE;
	TRACEX("---HisParam Total :%d, bHasHisParam = %d---", pCharger->info.hisParams_info.nTotal, bHasHisParam);
	if(bHasHisParam)
	{
		sqlite3 *data_db = Open_Lock_SqliteDB();//(sqlite3*)pCharger->hRunDB;
		//char* szErrMsg = NULL;
		//init get HisalarmIdx
		//sqlite3_exec(data_db,"SELECT Idx FROM Data_HisStatusData WHERE Record_Time IN(SELECT max(Record_Time) FROM Data_HisStatusData)",
		//	Get_HisParamIdx, NULL, NULL/*&szErrMsg*/);
		sqlite3_exec(data_db,"SELECT count(*) FROM Data_HisStatusData",	Get_HisParamIdx, NULL, NULL/*&szErrMsg*/);
		Close_UnLock_SqliteDB();
	}

	SAMPLER* pSampler = pCharger->samplers.p;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)&pCharger->dataprocessor;

	while(!pCharger->dataprocessor.bExit)
	{
		Sleep(200);
		RUN_THREAD_HEARTBEAT();
		
		//Wait_QtCmd_andExecute(10);

		time_t tmNow = _NOW_;

		VAR_VALUE varV;
		// VAR_VALUE varV_upd;

		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 83, &varV, 4);

		time_t tmUpgradeFinishedTime = (time_t)varV.ulValue; 

		// pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 85, &varV_upd, 4);

		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 81, &varV, 4);
		//printf("upgrade status varV.emValue = %d\n", (int)varV.emValue);
		//printf("tmUpgradeFinishedTime = %u tmNow = %u tmNow - tmUpgradeFinishedTime = %u\n", tmUpgradeFinishedTime, tmNow, tmNow - tmUpgradeFinishedTime);
		
		pDev = pCharger->devices.p;
		for(n = 0; n < pCharger->devices.nTotal; n++, pDev++)
		{
			//refresh status param value
			pParam = pDev->params_Status.p;
			for(i = 0 ; i < pDev->params_Status.nTotal; i++, pParam++)
			{
				Refresh_StatusParam(pDev, pParam);
			}

			//refresh control param value
			pParam = pDev->params_Control.p;
			for(i = 0 ; i < pDev->params_Control.nTotal; i++, pParam++)
			{
				Refresh_CtrlParam(pDev, pParam);
			}

			//refresh setting param value
			pParam = pDev->params_Setting.p;
			for(i = 0 ; i < pDev->params_Setting.nTotal; i++, pParam++)
			{
				Refresh_SettParam(pDev, pParam);
			}

			//not process alarm while upgrading, avoid triggering unexpected alarms
			// if (UPGRADE_IDLE != (SW_UPGRADE_STATUS)varV.emValue || tmNow - tmUpgradeFinishedTime < 60 || UPGRADE_IDLE != (SW_UPGRADE_STATUS)varV_upd.emValue)
			// if (UPGRADE_IDLE != (SW_UPGRADE_STATUS)varV.emValue || tmNow - tmUpgradeFinishedTime < 30)// || UPGRADE_IDLE != (SW_UPGRADE_STATUS)varV_upd.emValue)
			// {
			// 	continue;
			// }
			//calculate alarm status
			pAlarm = pDev->alarms.p;
			for(i = 0; i < pDev->alarms.nTotal; i++, pAlarm++)
			{
				if(ABS(tmNow - pAlarm->tmLastRestored) < 10)
				{
					TRACE("-------This alarm[%d, %d] is restored, so Skip this[%d]!-----\n",
						((DEVICE*)pAlarm->hDevice)->info.nDeviceId, pAlarm->pAlarmInfo->nAlarmId, tmNow - pAlarm->tmLastRestored);
					continue;
				}
				Refresh_Alarm(pCharger, pDev, pAlarm);
			}
		}

		//Now handle historical data
		if(bHasHisParam)
		{
			pHisParam = pCharger->info.hisParams_info.p;
			for(n = 0; n < pCharger->info.hisParams_info.nTotal; n++, pHisParam++)
			{
				// if(pHisParam->pParam->bIsValueInvalid)
				// {
				// 	continue;
				// }
				Refresh_HisParam(pCharger, pHisParam);
				RUN_THREAD_HEARTBEAT();
			}
		}
	}

	//do clean job
	//THREAD_CANCEL_ALL means must system will reboot
	return THREAD_CANCEL_ALL;
}
