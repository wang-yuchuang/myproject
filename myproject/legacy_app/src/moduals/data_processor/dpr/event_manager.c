// #define _TRACE_PRINT_
#include "dpr.h"

static int s_nCurrEventIdx = 1;
static int s_nCurrAlarmIdx = 1;
static int s_nCurrChargeRecIdx = 1;
static int s_nCurrChargeMeterValIdx = 1;
static int s_nCurrESStatDataIdx = 1;
static int s_nCurrBmsRunDataValIdx = 1;
static int s_nCurrHealthDataValIdx = 1;
static int s_nCurrMeterKwhValIdx = 1;
static int s_nCurrAccumulatedDataIdx = 1;

#define MAX_EVENT_HANDLER	256
static int s_nEventHandlerTotal = 0;
static EVENT_HANDLER_REGISTER s_TableEvHandlers[MAX_EVENT_HANDLER];
static HANDLE s_hEventHandMutex = NULL;

int Register_EventHandler(char* pszFuncName, HANDLE hModule, 
	EVENT_HANDLER pfnHandler, UINT nEventFlag, UINT nMsgFlag, BOOL bIsUnregister)
{
	int n;
	if(s_hEventHandMutex == NULL)
	{
		s_hEventHandMutex = Mutex_Create(TRUE);
		if(s_hEventHandMutex == NULL)
		{
			return ERR_DPR_MUTEX_ERR;
		}
	}

	if (Mutex_Lock(s_hEventHandMutex, 5000) != ERR_MUTEX_OK)
	{
		return ERR_DPR_MUTEX_ERR;
	}

	BOOL bFoundExisted = FALSE;
	int nErrCode = ERR_DPR_OK;
	EVENT_HANDLER_REGISTER *pItem = s_TableEvHandlers;
	for(n = 0; n < s_nEventHandlerTotal; n++, pItem++)
	{
		if((pItem->hModuleSelf == hModule) && 
			(pItem->pfnEvHandler == pfnHandler) &&
			(pItem->nEvTypeFlag == nEventFlag) &&
			(pItem->nMsgTypeFlag == nMsgFlag))
		{
			bFoundExisted = TRUE;
			break;
		}
	}
	if(!bIsUnregister)
	{
		if(bFoundExisted)
		{
			nErrCode = ERR_DPR_REPEATED_EVHAND;
		}
		else if(s_nEventHandlerTotal >= MAX_EVENT_HANDLER)
		{
			nErrCode = ERR_DPR_TOOMANY_EVHAND;
		}
		else
		{
			if(hModule == NULL)
			{
				nErrCode = ERR_DPR_INVALID_ARG;
			}
			else
			{
				pItem = &s_TableEvHandlers[s_nEventHandlerTotal];
				strncpy(pItem->szName, pszFuncName, strlen(pszFuncName));
				pItem->szName[sizeof(pItem->szName)-1] = 0;
				pItem->hModuleSelf = hModule;
				pItem->pfnEvHandler = pfnHandler;
				pItem->nEvTypeFlag = nEventFlag;
				pItem->nMsgTypeFlag = nMsgFlag;
				s_nEventHandlerTotal++;
			}

		}
	}
	else
	{
		if(!bFoundExisted)
		{
			nErrCode = ERR_DPR_INVALID_ARG;
		}
		else
		{
			pItem->pfnEvHandler = NULL;
			pItem->nEvTypeFlag = 0;
			pItem->nMsgTypeFlag = 0; 
			pItem->hModuleSelf =  NULL;
			if(n < s_nEventHandlerTotal - 1)
			{
				memmove(pItem, pItem+1, (s_nEventHandlerTotal- n - 1)*sizeof(EVENT_HANDLER_REGISTER));
			}
			s_nEventHandlerTotal--;
		}
	}
	Mutex_Unlock(s_hEventHandMutex);

	TRACEX("Now register EVHand[%s] for Module, TotalHandler = %d\n", pszFuncName, s_nEventHandlerTotal);

	return nErrCode;
}

/*
static char* MakeChgRecUpdate_SqlSubStr(IN UINT nUpdateFlag, HIS_CHARGE_RECORD* pRec, OUT char *ppszCond)
{
	char *p = ppszCond;
	memset(p, 0 , 256);
	char szSub[128];

	if((nUpdateFlag & CHARGE_REC_UPDATE_REPORTSTAT) == CHARGE_REC_UPDATE_REPORTSTAT)
	{
		sprintf(p, ", Report_Flag = %d", pRec->nReportFlag);
	}

	if((nUpdateFlag & CHARGE_REC_UPDATE_OFFPAYSTAT) == CHARGE_REC_UPDATE_OFFPAYSTAT)
	{
		memset(szSub, 0 , 128);
		sprintf(szSub, ", OfflinePay_Flag = %d", pRec->bIsOfflinePayedOK);
		strcat(p, szSub);
	}

	if((nUpdateFlag & CHARGE_REC_UPDATE_SESSID) == CHARGE_REC_UPDATE_SESSID)
	{
		memset(szSub, 0 , 128);
		sprintf(szSub, ", ChargeSession_ID = %d", pRec->nSessionID);
		strcat(p, szSub);
	}

	if((nUpdateFlag & CHARGE_REC_UPDATE_CHGSTAT) == CHARGE_REC_UPDATE_CHGSTAT)
	{
		memset(szSub, 0 , 128);
		sprintf(szSub, ", ChargingStatus = %d", pRec->emChargeStatus);
		strcat(p, szSub);
	}

	if((nUpdateFlag & CHARGE_REC_UPDATE_VIN) == CHARGE_REC_UPDATE_VIN)
	{
		memset(szSub, 0 , 128);
		sprintf(szSub, ", Car_VIN = %s", pRec->szVIN);
		strcat(p, szSub);
	}

	return ppszCond;
}
*/

// static pthread_mutex_t	s_hSqlOpenLock = PTHREAD_MUTEX_INITIALIZER;
// //#define LOCK_APP_LOG_INIT()	pthread_mutex_init(&s_hSqlOpenLock, NULL)
// #define LOCK_SQLOPENLOCK()		pthread_mutex_lock(&s_hSqlOpenLock)
// #define UNLOCK_SQLOPENLOCK()	pthread_mutex_unlock(&s_hSqlOpenLock)
// sqlite3* Reopen_AndLockSqliteDB(CHARGER* pCharger)
// {
// 	sqlite3* db = NULL; //= (sqlite3*)pCharger->hRunDB;

// 	char	szDBFile[256] = {0};
// 	MakeFullFileName(szDBFile, \
// 		getenv(ENV_VARIABLE_DATA_PATH), NULL, "nccu.data");

// 	LOCK_SQLOPENLOCK();

// 	int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
// 	if(rc == SQLITE_OK)
// 	{
// 		int nWaitCloseMax = 100; //max wait 5 sec for old db to close
// 		while(sqlite3_close((sqlite3*)pCharger->hRunDB) != SQLITE_OK)// ; //close old db handle
// 		{
// 			Sleep(50);
// 			nWaitCloseMax--;
// 			if(nWaitCloseMax <= 0)
// 			{
// 				break;
// 			}
// 		}
// 		sqlite3_busy_timeout(db, 20000); //set default BUSY_HANDLER for sql database
// 		pCharger->hRunDB = (HANDLE)db;
// 		//sqlite3_exec(db, "PRAGMA synchronous = OFF; ", NULL, NULL, NULL); //Jimmy added for improving performance test
// 		//Change_LogRepo((HANDLE)db); //need re-init the Log handle
// 		//Jimmy Note: when old handle not closed, still can open new handle, so need release old handle manually
// 		//LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING, 
// 		//	"DB is re-opened now!");
// 	}
// 	else
// 	{
// 		sqlite3_close(db); //still need to call close according to official doc
// 		//LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_INFO, 
// 		//	"Reused DB handle, NO need to reopen it!");
// 		db = (sqlite3*)pCharger->hRunDB;
// 	}

// 	//UNLOCK_SQLOPENLOCK();
// 	return db;
// }

// void UnLock_SqliteDB()
// {
// 	//int nMemUsed, nMemPeak;
// 	//sqlite3_status(SQLITE_STATUS_MEMORY_USED, &nMemUsed, &nMemPeak, TRUE);
// 	//LogOut(MAIN_MODULE_DPR, "DB_MEMSTAT", LOG_TYPE_INFO,
// 	//	"The Database memory used and peak values are:[%d, %d]!", nMemUsed, nMemPeak);
// 	sync(); //Jimmy added 20190529 for safety sync data to flash disk
// 	UNLOCK_SQLOPENLOCK();
// }

static BOOL Handle_AbilityChange_End(sqlite3* data_db, int nWhichGun)
{
	sqlite3_stmt * stat;
	char szSql[512];
	sprintf(szSql, "SELECT * FROM Data_Events where Event_Type = %d and Event_Status = %d", 
		EVENT_TYPE_ABILITY_CHANGE, EVENT_STATUS_OCCUR);
	sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
	EVENT_ABILITY_CHG evChg;
	int nEvSize;
	int nUpdateIdx = -1;
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		ZERO_POBJS(&evChg, 1);
		nEvSize = MIN(sqlite3_column_bytes(stat, 3), sizeof(EVENT_ABILITY_CHG));
		memcpy(&evChg, (EVENT_ABILITY_CHG*)sqlite3_column_text(stat, 3), nEvSize);
		if(evChg.nWhichGun == nWhichGun)
		{
			nUpdateIdx = sqlite3_column_int(stat, 0);
			break;
		}
	}
	sqlite3_finalize(stat);

	if(nUpdateIdx != -1)
	{
		sprintf(szSql, "update Data_Events set Event_Status = %d, End_Time = %d "
			"where Idx = %d", EVENT_STATUS_NOT_OCCUR, _NOW_,nUpdateIdx);
		TRACEX("Now execute the Handle_AbilityChange_End sql[%s]\n", szSql);
		sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
		sqlite3_step(stat);
		sqlite3_finalize(stat);
		return TRUE;
	}
	else
	{
		TRACEX("Cannot update Abitlchange end event\n");
	}

	return FALSE;
}
/*
static void Handle_GunReserve_End(sqlite3* data_db, int nWhichGun)
{
	sqlite3_stmt * stat;
	char szSql[512];
	sprintf(szSql, "SELECT * FROM Data_Events where Event_Type = %d and Event_Status = %d", 
		EVENT_TYPE_GUN_RESERVE, EVENT_STATUS_OCCUR);
	sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
	EVENT_GUN_RESERVE evGunRes;
	int nEvSize;
	int nUpdateIdx = -1;
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		ZERO_POBJS(&evGunRes, 1);
		nEvSize = MIN(sqlite3_column_bytes(stat, 3), sizeof(EVENT_GUN_RESERVE));
		memcpy(&evGunRes, (EVENT_GUN_RESERVE*)sqlite3_column_text(stat, 3), nEvSize);
		if(evGunRes.nGunIdx == nWhichGun)
		{
			nUpdateIdx = sqlite3_column_int(stat, 0);
			break;
		}
	}
	sqlite3_finalize(stat);
	if(nUpdateIdx != -1)
	{
		sprintf(szSql, "update Data_Events set Event_Status = %d, End_Time = %d "
			"where Idx = %d", EVENT_STATUS_NOT_OCCUR, _NOW_,nUpdateIdx);
		TRACEX("Now execute the Handle_GunReserve_End sql[%s]\n", szSql);
		sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
		sqlite3_step(stat);
		sqlite3_finalize(stat);
	}
	else
	{
		TRACEX("Cannot update GunReserve end event\n");
	}
}
*/

static int Get_MaxEventIdx(void* pInternalArg, int nColomn, char** ppszColumData, char** ppszColumnName)
{
	if(pInternalArg == NULL)
	{
		return 1;
	}
	UNUSED(ppszColumnName);
	if(nColomn < 1)
	{
		return 1;
	}
	//TRACEX("nColoum = %d, ppszColumData = %d\n", nColomn, ppszColumData);
	*(int*)pInternalArg = atoi(ppszColumData[0]) + 1;

	return 0;
}


static int Get_EventIdx(void* pInternalArg, int nColomn, char** ppszColumData, char** ppszColumnName)
{
	if(pInternalArg == NULL)
	{
		return 1;
	}
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

	int nWhichIdx = atoi(ppszColumData[0]);
	nWhichIdx += 1;
	if(nWhichIdx < 1)
	{
		nWhichIdx = 1;
	}
	if(pInternalArg == (&s_nCurrEventIdx))
	{
		if(nWhichIdx > MAX_EVENT_RECORDS)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrEventIdx = %d\n", s_nCurrEventIdx);
	}
	else if(pInternalArg == (&s_nCurrAlarmIdx))
	{
		if(nWhichIdx > MAX_ALARM_RECORDS)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrAlarmIdx = %d\n", s_nCurrAlarmIdx);
	}
	else if((pInternalArg == (&s_nCurrChargeRecIdx)))
	{
		if(nWhichIdx > MAX_CHARGE_RECORDS)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrChargeRecIdx = %d\n", s_nCurrChargeRecIdx);
	}
	else if((pInternalArg == (&s_nCurrChargeMeterValIdx)))
	{
		if(nWhichIdx > MAX_CHARGE_RECORDS)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrChargeMeterValIdx = %d\n", s_nCurrChargeMeterValIdx);
	}
	else if((pInternalArg == (&s_nCurrESStatDataIdx)))
	{
		if(nWhichIdx > MAX_ES_STATDAILY_DATA)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrESStatDataIdx = %d\n", s_nCurrESStatDataIdx);
	}
	else if((pInternalArg == (&s_nCurrBmsRunDataValIdx)))
	{
		if(nWhichIdx > MAX_BMS_RECORDS_NUM)
		{
			nWhichIdx = 1;
		}
		TRACEX("s_nCurrBmsRunDataValIdx = %d\n", s_nCurrBmsRunDataValIdx);
	}
	else
	{
		return 1;
	}

	*(int*)pInternalArg = nWhichIdx;

	return 0;
}

static int Get_CompressEvent(void* pInternalArg, int nColomn, char** ppszColumData, char** ppszColumnName)
{
	if(pInternalArg == NULL)
	{
		return 1;
	}
	UNUSED(ppszColumnName);
	if(nColomn < 1)
	{
		return 1;
	}
	//TRACEX("nColoum = %d, ppszColumData = %d\n", nColomn, ppszColumData);
	*(int*)pInternalArg = atoi(ppszColumData[0]);

	return 0;
}


int HandleEvent(CHARGER* pCharger, HANDLE hSender, EVENT_ARG* pEventArg)
{
	sqlite3_stmt * stat;
	//sqlite3* data_db = (sqlite3*)pCharger->hRunDB;

	int n;
	if(s_hEventHandMutex == NULL)
	{
		s_hEventHandMutex = Mutex_Create(TRUE);
		if(s_hEventHandMutex == NULL)
		{
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_MUTEX_ERR;
		}
	}

	TRACEX("Now begin handling eventType ->[%d] emMsgType=[%d].\n", pEventArg->emEvType, pEventArg->emMsgType);

	if(pEventArg->emEvType == EVENT_TYPE_ALARM)
	{
		EVENT_ALARM *pActAlarm = (EVENT_ALARM*)pEventArg->pEvData;

		TRACEX(" EVENT_TYPE_ALARM[%d, %d]-->%d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",
			pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->status);

		if(pEventArg->emMsgType == EVENT_MSG_BEGIN)
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			//------------added by Jimmy 20201228-should end the old alarms before start new alarm
			char szSql[256];
			sprintf(szSql, "update Data_Alarms set Alarm_Status = 0, Alarm_EndTime = %d "
				"where Device_Id = %d and Alarm_Id = %d and Alarm_Status > 0", 
				_NOW_, pActAlarm->nDevId, pActAlarm->nAlarmId);
			//sqlite3_prepare(data_db,"update Data_Alarms values (?,?,?,?,?,?);",-1,&stat,NULL);
			//char *szErrMsg;
			/*int nErrCode = */sqlite3_exec(data_db, szSql, NULL, NULL, NULL/*&szErrMsg*/);
			//------------------------end of adding
			sqlite3_prepare_v2(data_db,"replace into Data_Alarms values (?,?,?,?,?,?,?);",-1,&stat,NULL);
			sqlite3_bind_int(stat,1, (int)s_nCurrAlarmIdx);
			sqlite3_bind_int(stat,2, pActAlarm->nDevId);
			sqlite3_bind_int(stat,3, pActAlarm->nAlarmId);
			sqlite3_bind_int(stat,4, pActAlarm->status);
			sqlite3_bind_double(stat,5, (double)pActAlarm->fExParamVal);
			sqlite3_bind_int(stat,6, pActAlarm->tmStart);
			sqlite3_bind_int(stat,7, 0);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat
			Close_UnLock_SqliteDB();
			s_nCurrAlarmIdx++;
			if(s_nCurrAlarmIdx > MAX_ALARM_RECORDS)
			{
				s_nCurrAlarmIdx = 1;
			}

			if(nSqlRet != SQLITE_DONE)
			{
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_Alarms values (?,?,?,?,?,?,?);", 
					nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
			}
			else
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING,
					"Alarm[%d, %d, %.1f] has OCCURED!", pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->fExParamVal);
			}
		}
		//else if(pEventArg->emMsgType == EVENT_MSG_END)
		else if(pEventArg->emMsgType == EVENT_MSG_STATUS_CHANGED)
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			char szSql[256];
			sprintf(szSql, "update Data_Alarms set Alarm_Status = %d, Alarm_EndTime = %d "
				"where Device_Id = %d and Alarm_Id = %d and Alarm_StartTime = %d", 
				pActAlarm->status, pActAlarm->tmEnd,
				pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->tmStart);
			//sqlite3_prepare_v2(data_db,"update Data_Alarms values (?,?,?,?,?,?);",-1,&stat,NULL);
			char *szErrMsg;
			int nErrCode = sqlite3_exec(data_db, szSql, NULL, NULL, &szErrMsg);
			Close_UnLock_SqliteDB();
			if(nErrCode != SQLITE_OK)
			{
				LogOut(MAIN_MODULE_DPR, "HandleAlarm", LOG_TYPE_ERROR,
					"Can not update alarm[%d, %d, %d]!", 
					pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->tmStart);
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, 
					nErrCode, szErrMsg);
				//system("reboot");
				//SAFELY_DELETE(pEventArg->pEvData);
				//return ERR_DPR_INVALID_ARG;
			}
			else
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING,
					"Alarm[%d, %d, %.1f] has CHANGED status->%d!", pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->fExParamVal, pActAlarm->status);
			}
		}
		else if(pEventArg->emMsgType == EVENT_MSG_END)
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			char szSql[256];
			sprintf(szSql, "update Data_Alarms set Alarm_Status = 0, Alarm_EndTime = %d "
				"where Device_Id = %d and Alarm_Id = %d and Alarm_Status > 0", 
				pActAlarm->tmEnd, pActAlarm->nDevId, pActAlarm->nAlarmId);
			//sqlite3_prepare(data_db,"update Data_Alarms values (?,?,?,?,?,?);",-1,&stat,NULL);
			char *szErrMsg;
			int nErrCode = sqlite3_exec(data_db, szSql, NULL, NULL, &szErrMsg);
			Close_UnLock_SqliteDB();
			if(nErrCode != SQLITE_OK)
			{
				LogOut(MAIN_MODULE_DPR, "HandleAlarm", LOG_TYPE_ERROR,
					"Can not end alarm[%d, %d, %d]!", 
					pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->tmStart);
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, 
					nErrCode, szErrMsg);
				//system("reboot");
				//SAFELY_DELETE(pEventArg->pEvData);
				//return ERR_DPR_INVALID_ARG;
			}
			else
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING,
					"Alarm[%d, %d, %.1f] has CEASED!", pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->fExParamVal);
			}
		}
		//else if(pEventArg->emMsgType == EVENT_MSG_STATUS_CHANGED)
		//{
		//	char szSql[256];
		//	sprintf(szSql, "delete from Data_Alarms"
		//		"where Device_Id = %d and Alarm_Id = %d and Alarm_StartTime = %d", 
		//		pActAlarm->nDevId, pActAlarm->nAlarmId, pActAlarm->tmStart);
		//	sqlite3_exec(data_db, szSql, NULL, NULL, NULL);
		//}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_BMS_HISTFAULT)
	{
		EVENT_BMS_HISTFAULT *pFault = (EVENT_BMS_HISTFAULT*)pEventArg->pEvData;
		if(pEventArg->emMsgType == EVENT_MSG_BEGIN)
		{
			// 状态：产生(1/3)=1未恢复，消除(2/4)=0已恢复
			int nStatus = (pFault->nFaultType == 1 || pFault->nFaultType == 3) ? 1 : 0;

			sqlite3* data_db = Open_Lock_SqliteDB();
			sqlite3_stmt *stat;
			sqlite3_prepare_v2(data_db,
				"INSERT INTO Data_BmsHistFault (BattGrp,FaultType,FaultCode,"
				"FaultTime,"
				"ProtectState,WorkState,Current,Custom3,Custom4,Custom5,SaveTime,Status) "
				"VALUES (?,?,?,?,?,?,?,?,?,?,?,?);",
				-1, &stat, NULL);
			sqlite3_bind_int(stat, 1, pFault->nBattGrp);
			sqlite3_bind_int(stat, 2, pFault->nFaultType);
			sqlite3_bind_int(stat, 3, pFault->nFaultCode);
			sqlite3_bind_int(stat, 4, (int)pFault->tmFault);
			sqlite3_bind_int(stat, 5, pFault->nProtectState);
			sqlite3_bind_int(stat, 6, pFault->nWorkState);
			sqlite3_bind_int(stat, 7, pFault->nCurrent);
			sqlite3_bind_int(stat, 8, pFault->nCustom3);
			sqlite3_bind_int(stat, 9, pFault->nCustom4);
			sqlite3_bind_int(stat, 10, pFault->nCustom5);
			sqlite3_bind_int(stat, 11, (int)_NOW_);
			sqlite3_bind_int(stat, 12, nStatus);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);

			// 消除记录：把对应的「产生」记录也标记为已恢复
			if(nStatus == 0)
			{
				char szSql[256];
				snprintf(szSql, sizeof(szSql),
					"UPDATE Data_BmsHistFault SET Status = 0 "
					"WHERE BattGrp = %d AND FaultCode = %d AND Status = 1",
					pFault->nBattGrp, pFault->nFaultCode);
				sqlite3_exec(data_db, szSql, NULL, NULL, NULL);
			}

			Close_UnLock_SqliteDB();

			if(nSqlRet == SQLITE_DONE)
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING,
					"BMS HistFault[Grp%d, Code%d, Type%d, Status%d] saved!",
					pFault->nBattGrp, pFault->nFaultCode, pFault->nFaultType, nStatus);
			}
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_BMS_HISTEVENT)
	{
		EVENT_BMS_HISTEVENT *pEvent = (EVENT_BMS_HISTEVENT*)pEventArg->pEvData;
		if(pEventArg->emMsgType == EVENT_MSG_BEGIN)
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			sqlite3_stmt *stat;
			sqlite3_prepare_v2(data_db,
				"INSERT INTO Data_BmsHistEvent (BattGrp,EventType,EventCode,"
				"EventTime,"
				"ProtectState,WorkState,Current,Custom3,Custom4,Custom5,SaveTime) "
				"VALUES (?,?,?,?,?,?,?,?,?,?,?);",
				-1, &stat, NULL);
			sqlite3_bind_int(stat, 1, pEvent->nBattGrp);
			sqlite3_bind_int(stat, 2, pEvent->nEventType);
			sqlite3_bind_int(stat, 3, pEvent->nEventCode);
			sqlite3_bind_int(stat, 4, (int)pEvent->tmEvent);
			sqlite3_bind_int(stat, 5, pEvent->nProtectState);
			sqlite3_bind_int(stat, 6, pEvent->nWorkState);
			sqlite3_bind_int(stat, 7, pEvent->nCurrent);
			sqlite3_bind_int(stat, 8, pEvent->nCustom3);
			sqlite3_bind_int(stat, 9, pEvent->nCustom4);
			sqlite3_bind_int(stat, 10, pEvent->nCustom5);
			sqlite3_bind_int(stat, 11, (int)_NOW_);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);

			Close_UnLock_SqliteDB();

			if(nSqlRet == SQLITE_DONE)
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_WARING,
					"BMS HistEvent[Grp%d, Type%d, Code%d] saved!",
					pEvent->nBattGrp, pEvent->nEventType, pEvent->nEventCode);
			}
		}
	}
	/*
	else if(pEventArg->emEvType == EVENT_TYPE_CHARGING)
	{
		EVENT_CHARGING *pEvChg = (EVENT_CHARGING*)pEventArg->pEvData;

		TRACEX("EVENT CHARGING for CONN[%d], msgType = %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 
			pEvChg->emGunNum, pEventArg->emMsgType);

		A_BLOB_RUN_CHARGE_INFO chargeData;
		CHARGE_REC_EXTRADATA recExtraData;
		ZERO_POBJS(&recExtraData, 1);
		VAR_VALUE varV;
		//recExtraData.emChargeType = 1;
		BOOL bHasRTData = TRUE;
		if(pEvChg->emGunNum != EVENT_FOR_CONN_0)
		{
			MAKE_BLOB(chargeData, BLOB_TYPE_RUN_CHARGE_INFO);
			OBJ_PTR arg;
			varV.blobValue = (BYTE *)&chargeData;
			arg.nId = (pEvChg->emGunNum - 1) + 201;
			arg.nSubId = 107;
			//get charge real time data
			ERRCDDE_DPR nErrCode = pCharger->dataprocessor.Get((HANDLE)&pCharger->dataprocessor, GETTYPE_SP_VAR, 
				arg.nId, arg.nSubId, (void*)&varV, sizeof(A_BLOB_RUN_CHARGE_INFO));
			if(nErrCode != ERR_DPR_OK)
			{
				//SAFELY_DELETE(pEventArg->pEvData);
				//return ERR_DPR_INVALID_ARG;
				TRACEX("-----------CanNOT get REAL RUNCHARGE data----------\n");
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
					"Write charge record for Gun[%d] FAILED because runtime data can NOT be obtained!", 
					pEvChg->emGunNum);
				bHasRTData = FALSE;
			}
		}
		else
		{
			bHasRTData = FALSE;
			//do nothing
		}

		if(pEventArg->emMsgType == EVENT_MSG_BEGIN)
		{
			if(!bHasRTData)
			{
				TRACEX("-----------DO not record charge startRec----------\n");
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_INVALID_ARG;
			}

			sqlite3* data_db = Open_Lock_SqliteDB();
			TRACEX("----------Now make charge BEGIN record[%d]----------\n", s_nCurrChargeRecIdx);
			memcpy(&recExtraData.runData, &chargeData.extraData, sizeof(RUN_CHARGE_EXTRA_DATA));
			sqlite3_prepare(data_db,"replace into Data_ChargeRecords values (?,?,?,?,?,?,?,?,?,?,?);",-1,&stat,NULL);
			sqlite3_bind_int(stat,1, (int)s_nCurrChargeRecIdx);
			sqlite3_bind_int(stat,2, pEvChg->emGunNum);
			sqlite3_bind_int(stat,3, chargeData.tmChargeStart);
			sqlite3_bind_int(stat,4, chargeData.nSessionID);
			sqlite3_bind_text(stat,5, chargeData.szUserID, 64, NULL);
			sqlite3_bind_text(stat,6, chargeData.szVIN, 64, NULL);
			sqlite3_bind_int(stat,7, chargeData.emChargeStartMethod);
			sqlite3_bind_int(stat,8, chargeData.emChargeStatus);
			sqlite3_bind_int(stat,9, chargeData.nReportFlag);
			sqlite3_bind_int(stat,10, chargeData.bIsOfflinePayedOK);
			sqlite3_bind_blob(stat,11, (void*)&recExtraData, sizeof(CHARGE_REC_EXTRADATA), NULL);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat
			Close_UnLock_SqliteDB();
			s_nCurrChargeRecIdx++;
			if(s_nCurrChargeRecIdx > MAX_CHARGE_RECORDS)
			{
				s_nCurrChargeRecIdx = 1;
			}

			if(SQLITE_DONE == nSqlRet)
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_INFO,
					"Charge session for Gun[%d] has been started!", 
					pEvChg->emGunNum);
			}
			else
			{
				//ReOpen_SqliteDB(pCharger);
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
					"Charge session for Gun[%d] is started but record CANNOT be written, errcode = %d, errMsg= [%s]!", 
					pEvChg->emGunNum, nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
			}
		}
		else if(pEventArg->emMsgType == EVENT_MSG_END)
		{
			if(!bHasRTData)
			{
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_INVALID_ARG;
			}
			sqlite3* data_db = Open_Lock_SqliteDB();
			memcpy(&recExtraData.runData, &chargeData.extraData, sizeof(RUN_CHARGE_EXTRA_DATA));
			char szSql[512];
			sprintf(szSql, "update Data_ChargeRecords set ChargingStatus = %d, "
				"ChargeSession_ID = %d, OfflinePay_Flag = %d, "
				"Report_Flag = %d, ChargeData = (?), Car_VIN = (?) "
				"where WhichGun = %d and Charge_BeginTime = %d", 
				(int)chargeData.emChargeStatus, chargeData.nSessionID, 
				(int)chargeData.bIsOfflinePayedOK, (int)chargeData.nReportFlag,
				(int)pEvChg->emGunNum, (int)chargeData.tmChargeStart);
			TRACEX("-----Now update charge record for tmStart = %d, sql = [%s]-----\n",
				(int)chargeData.tmChargeStart, szSql);
			sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
			sqlite3_bind_blob(stat, 1, (void*)&recExtraData, sizeof(CHARGE_REC_EXTRADATA), NULL);
			sqlite3_bind_text(stat, 2, chargeData.szVIN, 64, NULL);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat
			Close_UnLock_SqliteDB();
			//LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_INFO,
			//	"Charge session for Gun[%d] has ENDED, StopReason[%d]!", 
			//	pEvChg->emGunNum, recExtraData.runData.emChargeEndReason);
			if(SQLITE_DONE == nSqlRet)
			{
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_INFO,
					"Charge session for Gun[%d] has ENDED, StopReason[%d]!", 
					pEvChg->emGunNum, recExtraData.runData.emChargeEndReason);
			}
			else
			{
				//ReOpen_SqliteDB(pCharger);
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_INFO,
					"Charge session for Gun[%d] has ENDED, StopReason[%d], but FAILED SQL[%s]!", 
					pEvChg->emGunNum, recExtraData.runData.emChargeEndReason, sqlite3_errmsg(data_db));
				//system("reboot");
			}
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_CHARGEREC_UPDATE)
	{
		EVENT_CHARGEREC_UPDATE *pEvRec = (EVENT_CHARGEREC_UPDATE*)pEventArg->pEvData;
		HIS_CHARGE_RECORD* pRec = pEvRec->pRecData;
		sqlite3* data_db = Open_Lock_SqliteDB();
		char szSql[512];
		char szSqlSub[256];
		sprintf(szSql, "update Data_ChargeRecords set ChargeData = (?)%s "
			"where WhichGun = %d and Charge_BeginTime = %d", 
			MakeChgRecUpdate_SqlSubStr(pEvRec->nUpdateFlag, pRec, szSqlSub),
			(int)pRec->emGunNum, (int)pRec->tmChargeStart);
		sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
		sqlite3_bind_blob(stat, 1, (void*)&pRec->extraRecData, sizeof(CHARGE_REC_EXTRADATA), NULL);
		int nSqlRet = sqlite3_step(stat);
		sqlite3_finalize(stat);//delete stat
		Close_UnLock_SqliteDB();

		if(nSqlRet != SQLITE_DONE)
		{
			TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
			//system("reboot");
		}

		TRACEX("EVENT CHARGERREC UPDATE for ID[%d], sql[%s] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", pRec->nSessionID, szSql);
		//other modual do not handle this 
		SAFELY_DELETE(pEventArg->pEvData);
		return ERR_DPR_OK;
	}
	else if(pEventArg->emEvType == EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING)
	{
		EVENT_METERVALUE_UPDATE *pEvMeterUpdate = (EVENT_METERVALUE_UPDATE*)pEventArg->pEvData;
		HIS_METER_VALUE_REC *pRec = pEvMeterUpdate->pMeterRecord;
		if(pEvMeterUpdate->emMeteVal_UpdateType == METER_VAL_UPDATETYPE_BEGIN)
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			TRACEX("----------Now make metervalues BEGIN record----------\n");
			sqlite3_prepare(data_db,"replace into Data_ChargeMeterValues values (?,?,?,?);",-1,&stat,NULL);
			sqlite3_bind_int(stat,1, (int)s_nCurrChargeMeterValIdx);
			sqlite3_bind_int(stat,2, pRec->nGunNum);
			sqlite3_bind_int(stat,3, pRec->tmChargeStart);
			sqlite3_bind_blob(stat,4, (void*)&pRec->meterData, sizeof(HIS_METER_VALUE_REC), NULL);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat
			
			if(nSqlRet != SQLITE_DONE)
			{
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_ChargeMeterValues values (?,?,?,?);", 
					nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
					"---Execute sql[%s], errCode = %d, errMsg = [%s]---","replace into Data_ChargeMeterValues values (?,?,?,?);", 
					nSqlRet, sqlite3_errmsg(data_db));
			}

			Close_UnLock_SqliteDB();
			s_nCurrChargeMeterValIdx++;
			if(s_nCurrChargeMeterValIdx > MAX_CHARGE_RECORDS)
			{
				s_nCurrChargeMeterValIdx = 1;
			}
		}
		else
		{
			sqlite3* data_db = Open_Lock_SqliteDB();
			char szSql[512];
			char szSqlSub[256];
			sprintf(szSql, "update Data_ChargeMeterValues set Meter_Data = (?) "
				"where Gun_Idx = %d and Charge_Begin_Time = %d", 
				(int)pRec->nGunNum, (int)pRec->tmChargeStart);
			sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
			sqlite3_bind_blob(stat, 1, (void*)&pRec->meterData, sizeof(HIS_METER_VALUE_REC), NULL);
			int nSqlRet = sqlite3_step(stat);
			sqlite3_finalize(stat);//delete stat
			Close_UnLock_SqliteDB();
			if(nSqlRet != SQLITE_DONE)
			{
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
					"---Execute sql[%s], errCode = %d, errMsg = [%s]---",
					szSql, nSqlRet, sqlite3_errmsg(data_db));
			}

			TRACEX("EVENT METERVALUE UPDATE for ID[%d, %d], sql[%s] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", pRec->nGunNum, pRec->tmChargeStart, szSql);
		}

		//msg end should be only hanlded here
		if(pEventArg->emMsgType == EVENT_MSG_END)
		{
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;
		}
		
	}*/
	else if(pEventArg->emEvType == EVENT_TYPE_BMSRUNDATA_UPDATE)
	{
		//if restore, just ignore it here but send it to other moduals for handling
		if(pEventArg->emMsgType != EVENT_MSG_RESTORE)
		{// EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_STATUS_CHANGED
			// printf("HandleEvent EVENT_TYPE_BMSRUNDATA_UPDATE != EVENT_MSG_RESTORE");
			void* pOut = NULL;
			EVENT_BMSRUNDATA_UPDATE *pBmsRunDataUpdate = (EVENT_BMSRUNDATA_UPDATE*)pEventArg->pEvData;
			HIS_BMS_RUNTIMEDATA_REC *pRec = pBmsRunDataUpdate->pBmsRunDataRec;

			if(pRec == NULL)
			{	
				LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
					"---BMS RunData Record is NULL pointer---");
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_INVALID_ARG;
			}

			sqlite3 *data_db = Open_Lock_Database(DB_TYPE_BMSDATA);

			int nDay =  (int)ConvDateTime_2DateOnly(pRec->tmLogTime) ;
			char szSql[512];
			
			// sprintf(szSql, "SELECT * FROM Data_RealTimeBmsData WHERE Log_Time = %d", pRec->tmLogTime); //唯一的ID
			// int rc = sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			// if(rc != SQLITE_OK)
			// {
			// 	TRACEI("SQL prepare failed: %s\n", sqlite3_errmsg(data_db));
			// 	sqlite3_finalize(stat); // 即使失败也要释放
			// 	Close_UnLock_Database(DB_TYPE_BMSDATA); 
			// 	return ERR_DPR_INVALID_ARG;
			// }

			// if(sqlite3_step(stat) == SQLITE_ROW)
			// {
			// 	//now update the record
			// 	sqlite3_finalize(stat);//delete stat	

			// 	printf("----------Now Update Data_RealTimeBmsData BEGIN record----------\n");
			// 	sprintf(szSql, "update Data_RealTimeBmsData set Runtime_Data = (?) where Log_Time = %d", pRec->tmLogTime);
			// 	sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);

			// 	sqlite3_bind_blob_compress(stat, 1, (void*)&pRec->bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST), &pOut, NULL);
			// 	int nSqlRet = sqlite3_step(stat);
			// 	//sqlite3_finalize(stat);//delete stat
			// 	sqlite3_finalize_compressed(stat, pOut);//delete stat
				
			// 	if(nSqlRet != SQLITE_DONE)
			// 	{
			// 		TRACEI("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "update Data_RealTimeBmsData values (?,?,?,?);", 
			// 			nSqlRet, sqlite3_errmsg(data_db));

			// 		LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
			// 			"---Execute sql[%s], errCode = %d, errMsg = [%s]---","update Data_RealTimeBmsData values (?,?,?,?);", 
			// 			nSqlRet, sqlite3_errmsg(data_db));
			// 	}
			// }
			// else
			// {
				TRACEI("--- Replace Data_RealTimeBmsData s_nCurrBmsRunDataValIdx=[%d] nDay=[%d] tmLogTime=[%ld] nBmsRuntimeDataTotal = [%d]---\n",\
					s_nCurrBmsRunDataValIdx, nDay, pRec->tmLogTime, pRec->bmsRunData.nBmsRuntimeDataTotal);
				
				// sqlite3_finalize(stat);//delete stat
				sqlite3_prepare_v2(data_db,"replace into Data_RealTimeBmsData values (?,?,?,?);",-1,&stat,NULL);
				sqlite3_bind_int(stat, 	1, s_nCurrBmsRunDataValIdx);
				sqlite3_bind_int(stat, 	2, nDay);
				sqlite3_bind_int(stat,	3, pRec->tmLogTime);
				sqlite3_bind_blob_compress(stat, 4, (void*)&pRec->bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST), &pOut, NULL);
				int nSqlRet = sqlite3_step(stat);
				//sqlite3_finalize(stat);//delete stat
				sqlite3_finalize_compressed(stat, pOut);//delete stat
				//Close_UnLock_SqliteDB();				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEI("---Execute replace into Data_RealTimeBmsData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
					LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
						"---Execute sql[%s], errCode = %d, errMsg = [%s]---","replace into Data_RealTimeBmsData values (?,?,?,?);", 
						nSqlRet, sqlite3_errmsg(data_db));
				}
				s_nCurrBmsRunDataValIdx++;
				if(s_nCurrBmsRunDataValIdx > MAX_BMS_RECORDS_NUM)
				{
					s_nCurrBmsRunDataValIdx = 1;
				}
			// }
			SAFELY_DELETE(pRec);//释放申请的堆内存，防止内存泄漏！！！！！！！
			
			Close_UnLock_Database(DB_TYPE_BMSDATA);
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;	
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_HEALTHDATA_UPDATE)
	{
		//if restore, just ignore it here but send it to other moduals for handling
		if(pEventArg->emMsgType != EVENT_MSG_RESTORE)
		{
			void* pOut = NULL;
			EVENT_BMSHEALTH_UPDATE *pEvtBmsHealthUpdate = (EVENT_BMSHEALTH_UPDATE*)pEventArg->pEvData;
			HIS_HEALTH_STATIC_DAILY *pRec = pEvtBmsHealthUpdate->pHisBmsHealthRec;
			
			//TRACEX("nInstalledNum=[%d] fKey_Chg_TmsvDif_CellVolt=[%.1f] fKey_Chg_TmsvDif_CellTemp=[%.1f]\
			//		fKey_Chg_Max_CellTemp=[%.1f] fKey_Chg_Min_CellTemp=[%.1f]\n",\
			//		pRec->bmsHealthData.nInstalledNum,\
			//		pRec->bmsHealthData.healthItem[2].fKey_Chg_TmsvDif_CellVolt,\
			//		pRec->bmsHealthData.healthItem[2].fKey_Chg_TmsvDif_CellTemp,\
			//		pRec->bmsHealthData.healthItem[2].fKey_Chg_Max_CellTemp,\
			//		pRec->bmsHealthData.healthItem[2].fKey_Chg_Min_CellTemp);

			sqlite3* data_db = Open_Lock_SqliteDB();
			int nDay = (int)ConvDateTime_2DateOnly(_NOW_);
			char szSql[512];
			
			sprintf(szSql, "SELECT * FROM Data_BMSHealthData WHERE Day = %d", nDay);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			if(sqlite3_step(stat) == SQLITE_ROW)
			{
				//now update the record
				sqlite3_finalize(stat);//delete stat	

				//printf("----------Now Update Data_BMSHealthData BEGIN record--nDay=[%d]--------\n", nDay);

				sprintf(szSql, "update Data_BMSHealthData set Health_Data = (?) where Day = %d", nDay);
 
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				//sqlite3_bind_int(stat,1, (int)s_nCurrHealthDataValIdx);
				//sqlite3_bind_int(stat,2, nDay);
				//sqlite3_bind_int(stat,3, pRec->tmLogTime);
				//sqlite3_bind_blob(stat,4, (void*)&pRec->meterData, sizeof(BMS_RUNTIMEDATA_LIST), NULL);
				sqlite3_bind_blob_compress(stat, 1, (void*)&pRec->bmsHealthData, sizeof(A_BMS_HEALTH_LIST), &pOut, NULL);
				int nSqlRet = sqlite3_step(stat);
				//sqlite3_finalize(stat);//delete stat
				sqlite3_finalize_compressed(stat, pOut);//delete stat
				//Close_UnLock_SqliteDB();
				if(nSqlRet != SQLITE_DONE)
				{
					printf("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "update Data_BMSHealthData values (?,?,?,?);", 
						nSqlRet, sqlite3_errmsg(data_db));
					//system("reboot");
					LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
						"---Execute sql[%s], errCode = %d, errMsg = [%s]---","update Data_BMSHealthData values (?,?,?,?);", 
						nSqlRet, sqlite3_errmsg(data_db));
				}
			}
			else
			{
				printf("--- Replace Data_BMSHealthData s_nCurrHealthDataValIdx=[%d] nDay=[%d] tmLogTime=[%ld]---\n",\
					s_nCurrHealthDataValIdx, nDay, pRec->tmLogTime);

				sqlite3_finalize(stat);//delete stat
				sqlite3_prepare_v2(data_db,"replace into Data_BMSHealthData values (?,?,?,?);", -1, &stat, NULL);
				sqlite3_bind_int(stat, 	1, s_nCurrHealthDataValIdx);
				sqlite3_bind_int(stat, 	2, nDay);
				sqlite3_bind_int(stat,	3, pRec->tmLogTime);
				sqlite3_bind_blob_compress(stat, 4, (void*)&pRec->bmsHealthData, sizeof(A_BMS_HEALTH_LIST), &pOut, NULL);
				int nSqlRet = sqlite3_step(stat);
				//sqlite3_finalize(stat);//delete stat
				sqlite3_finalize_compressed(stat, pOut);//delete stat
				//Close_UnLock_SqliteDB();				
				if(nSqlRet != SQLITE_DONE)
				{
					printf("---Execute replace into Data_BMSHealthData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				s_nCurrHealthDataValIdx++;
				if(s_nCurrHealthDataValIdx > MAX_HEALTH_RECORDS_DAY)
				{
					s_nCurrHealthDataValIdx = 1;
				}
			}
			
			Close_UnLock_SqliteDB();
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;	
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_COMM_FAIL)
	{
		//use -1/-2 paramId to calc comm fail alarm
		//-1 means dev fail, -2 means sampler fail
		OBJ_PTR* pCommPtr = (OBJ_PTR*)pEventArg->pEvData;
		SAMPLER* pSampler = GetSamplerById(pCommPtr->nId);
		if(pSampler == NULL)
		{
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_INVALID_ARG;
		}
		if(pCommPtr->nSubId == -1)
		{
			//sampler disconnect status
			pSampler->emWorkStatus = 
				(pEventArg->emMsgType == EVENT_MSG_BEGIN) ? SAMPLER_IS_DISCON : SAMPLER_IS_WORK;
		}
		else
		{
			DEVICE* pDev = GetDevce_ById(pCommPtr->nSubId);
			if(pDev == NULL)
			{
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_INVALID_ARG;
			}
			else if(pDev->info.pBelonged_Sampler != pSampler)
			{
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_INVALID_ARG;
			}

			TRACEX("-------------now set device[%s] commFail[%d]--------\n", pDev->info.pRes_Name->szName, 
				pEventArg->emMsgType );

			pDev->emStatus = 
				(pEventArg->emMsgType == EVENT_MSG_BEGIN) ? DEVICE_STATUS_COMM_FAIL : DEVICE_STATUS_NORMAL;
			if(pDev->emStatus == DEVICE_STATUS_NORMAL)
			{
				pSampler->emWorkStatus = SAMPLER_IS_WORK;
			}
		}
		//other modual do not handle this 
		SAFELY_DELETE(pEventArg->pEvData);
		return ERR_DPR_OK;
	}
	else if(pEventArg->emEvType == EVENT_TYPE_SAVE_ES_STATDATA)
	{
		//if restore, just ignore it here but send it to other moduals for handling
		if(pEventArg->emMsgType != EVENT_MSG_RESTORE)
		{
			TRACE("---Update EVENT_ES_SAVE_STATDATA Sqlite Data---\n");
			EVENT_ES_SAVE_STATDATA *pEvSaveESStat = (EVENT_ES_SAVE_STATDATA*)pEventArg->pEvData;
			HIS_ES_STATISTIC_DAILY* pRec = pEvSaveESStat->pDailyData;
			sqlite3* data_db = Open_Lock_SqliteDB();
			int nDay = (int)ConvDateTime_2DateOnly(_NOW_);
			char szSql[512];
			int nIdx = 1;
			sprintf(szSql, "SELECT * FROM Data_ESStats_Daily WHERE Idx = %d", nIdx);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			if(sqlite3_step(stat) == SQLITE_ROW)
			{
				//now update the record
				sqlite3_finalize(stat);//delete stat				
				sprintf(szSql, "update Data_ESStats_Daily set Data = (?) where Idx = %d", nIdx);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				sqlite3_bind_blob(stat, 1, (void*)pRec, sizeof(HIS_ES_STATISTIC_DAILY), NULL);
				int nSqlRet = sqlite3_step(stat);				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute replace into Data_ESStatDaily failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
			}
			else
			{
				TRACE("---Replace EVENT_ES_SAVE_STATDATA Sqlite Data---s_nCurrESStatDataIdx=[%d]\n", s_nCurrESStatDataIdx);

				sqlite3_finalize(stat);//delete stat
				sqlite3_prepare_v2(data_db,"replace into Data_ESStats_Daily values (?,?,?);",-1,&stat,NULL);
				sqlite3_bind_int(stat, 1, 1/*s_nCurrESStatDataIdx*/);
				sqlite3_bind_int(stat, 2, nDay);
				sqlite3_bind_blob(stat, 3, (void*)pRec, sizeof(HIS_ES_STATISTIC_DAILY), NULL);
				int nSqlRet = sqlite3_step(stat);				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute replace into Data_ESStats_Daily failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
				//s_nCurrESStatDataIdx++;
				if(s_nCurrESStatDataIdx > MAX_ES_STATDAILY_DATA)
				{
					s_nCurrESStatDataIdx = 1;
				}
			}

			Close_UnLock_SqliteDB();
			
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;	
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW)//add by lpl  240731 
	{
		int nSqlRet;
		//if restore, just ignore it here but send it to other moduals for handling
		if(pEventArg->emMsgType != EVENT_MSG_RESTORE)
		{
			EVENT_METER_SAVE_DATA *pEventSaveMeterData = (EVENT_METER_SAVE_DATA*)pEventArg->pEvData;

			time_t tmNow = _NOW_;
			char szSql[1024];
			sqlite3* data_db = Open_Lock_SqliteDB();
			int nDay = (int)ConvDateTime_2DateOnly(tmNow);

			sprintf(szSql, "SELECT * FROM Data_CurrentSPFW_kwh WHERE Device_Id = %d AND Day = %d", pEventSaveMeterData->ndevid, nDay);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			if(sqlite3_step(stat) == SQLITE_ROW)    //one meter one day one record
			{
				sqlite3_finalize(stat);//delete stat
				Close_UnLock_SqliteDB();
				SAFELY_DELETE(pEventArg->pEvData);
				return ERR_DPR_OK;	
			}

			TRACEI("HandleEvent---EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW------ REPLACE INTO");
			sqlite3_finalize(stat);//delete stat
			sqlite3_prepare_v2(data_db,"replace into Data_CurrentSPFW_kwh values (?,?,?,?,?,?,?,?,?,?,?,?,?,?);",-1,&stat,NULL);

			sqlite3_bind_int(stat, 1, s_nCurrMeterKwhValIdx);
			sqlite3_bind_int(stat, 2, pEventSaveMeterData->ndevid);  
			sqlite3_bind_int(stat, 3, nDay);
			sqlite3_bind_int(stat, 4, nDay);
			sqlite3_bind_double(stat,5, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_SUM_RTkWh_AC].fValue);
			sqlite3_bind_double(stat,6, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_SHARP_RTkWh_AC].fValue);
			sqlite3_bind_double(stat,7, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_PEAK_RTkWh_AC].fValue);
			sqlite3_bind_double(stat,8, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_FLAT_RTkWh_AC].fValue);
			sqlite3_bind_double(stat,9, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_WAVE_RTkWh_AC].fValue);

			sqlite3_bind_double(stat,10, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_RT_SUM_RVSkWh_AC].fValue);
			sqlite3_bind_double(stat,11, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_RT_SHARP_RVSkWh_AC].fValue);
			sqlite3_bind_double(stat,12, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_RT_PEAK_RVSkWh_AC].fValue);
			sqlite3_bind_double(stat,13, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_RT_FLAT_RVSkWh_AC].fValue);
			sqlite3_bind_double(stat,14, (double)pEventSaveMeterData->varEventWHMeterRough[WHMETER_ROUGH_RT_WAVE_RVSkWh_AC].fValue);

			nSqlRet = sqlite3_step(stat);				
			if(nSqlRet != SQLITE_DONE)
			{
				TRACEE("---Execute replace into Data_CurrentSPFW_kwh failed, errCode = %d, errMsg = [%s]---\n",
					nSqlRet, sqlite3_errmsg(data_db));
			}
			sqlite3_finalize(stat);//delete stat

			sprintf(szSql, "UPDATE Data_CurrentSPFW_kwh SET Log_Time = CURRENT_TIMESTAMP WHERE Device_Id = %d AND Day = %d",pEventSaveMeterData->ndevid, nDay);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			nSqlRet = sqlite3_step(stat);
			if(nSqlRet != SQLITE_DONE)
			{
				TRACEE("---Execute UPDATE Data_CurrentSPFW_kwh (Log_Time) failed, errCode = %d, errMsg = [%s]---\n",
					nSqlRet, sqlite3_errmsg(data_db));
			}
			sqlite3_finalize(stat);//delete stat

			s_nCurrMeterKwhValIdx++;
			if(s_nCurrMeterKwhValIdx > MAX_METER_DATA_RECORDS_DAY)
			{
				s_nCurrMeterKwhValIdx = 1;
			}
			
			Close_UnLock_SqliteDB();
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;	
		}
	}
	else if(pEventArg->emEvType == EVENT_TYPE_SAVE_ACCUMULATED_DATA)
	{
		if(pEventArg->emMsgType != EVENT_MSG_RESTORE)
		{
			TRACE("---Update EVENT_TYPE_SAVE_ACCUMULATED_DATA Sqlite Data\n");
			EVENT_ACCUMULATED_DATA *pEvSaveAccumulatedStat = (EVENT_ACCUMULATED_DATA*)pEventArg->pEvData;
			EFFICIENCY_DATA_DAILY* pRec = pEvSaveAccumulatedStat->pEfficiency_data;
			sqlite3* data_db = Open_Lock_SqliteDB();
			time_t nDay = ConvDateTime_2DateOnly(pRec->tmTimeSaved);
			// printf("5 time now is:%d\n", nDay);
			// printf("5 daily input kwh is: %f\n", pRec->fTotalACInputKWH_Daily);
			// printf("5 daily output kwh is: %f\n", pRec->fTotalACOutputKWH_Daily);
			char szSql[512];
			char szStringVal[32] = "\0";
			char* szErrMsg = NULL;

			//daily input kwh
			sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", nDay, TYPE_INPUT_KWH_D);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			if(sqlite3_step(stat) == SQLITE_ROW)
			{
				//now update the record
				sqlite3_finalize(stat);//delete stat
				sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", nDay, TYPE_INPUT_KWH_D);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Daily);
				//printf("input kwh string is: %s\n", szStringVal);
				sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
				int nSqlRet = sqlite3_step(stat);			
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
			}
			else
			{
				sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
				TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA InputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

				sqlite3_finalize(stat);
				sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
				sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
				sqlite3_bind_int(stat, 2, TYPE_INPUT_KWH_D);
				sqlite3_bind_int64(stat, 3, nDay);
				sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Daily);
				//printf("input kwh string is: %s\n", szStringVal);
				sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
				int nSqlRet = sqlite3_step(stat);				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
			}

			//daily output kwh
			sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", nDay, TYPE_OUTPUT_KWH_D);
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
			if(sqlite3_step(stat) == SQLITE_ROW)
			{
				//now update the record
				sqlite3_finalize(stat);//delete stat
				sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", nDay, TYPE_OUTPUT_KWH_D);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Daily);
				//printf("output kwh string is: %s\n", szStringVal);
				sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
				int nSqlRet = sqlite3_step(stat);				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
			}
			else
			{
				sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
				TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA OutputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

				sqlite3_finalize(stat);
				sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
				sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
				sqlite3_bind_int(stat, 2, TYPE_OUTPUT_KWH_D);
				sqlite3_bind_int64(stat, 3, nDay);
				sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Daily);
				//printf("output kwh string is: %s\n", szStringVal);
				sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
				int nSqlRet = sqlite3_step(stat);				
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
						nSqlRet, sqlite3_errmsg(data_db));
				}
				sqlite3_finalize(stat);//delete stat
			}

			if(pRec->nFlagIsMonthDataRecordNeeded == 1)
			{
				struct tm gmTimeLast;
				time_t tmLast = nDay;
        		gmtime_r(&tmLast, &gmTimeLast);
				gmTimeLast.tm_mday = 1;
				time_t tmLastMonthStart = mktime(&gmTimeLast);
				// time_t tmLastMonthEnd;
				// if(gmTimeLast.tm_mon == 11)
				// {
				// 	gmTimeLast.tm_year += 1;
				// 	gmTimeLast.tm_mon = 0;
				// 	tmLastMonthEnd = mktime(&gmTimeLast);
				// }
				// else
				// {
				// 	gmTimeLast.tm_mon += 1;
				// 	tmLastMonthEnd = mktime(&gmTimeLast);
				// }

				//monthly input kwh
				sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", tmLastMonthStart, TYPE_INPUT_KWH_M);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				if(sqlite3_step(stat) == SQLITE_ROW)
				{
					//now update the record
					sqlite3_finalize(stat);//delete stat
					sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", tmLastMonthStart, TYPE_INPUT_KWH_M);
					sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
					sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Monthly);
					//printf("input kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);			
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
				else
				{
					sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
					TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA InputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

					sqlite3_finalize(stat);
					sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
					sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
					sqlite3_bind_int(stat, 2, TYPE_INPUT_KWH_M);
					sqlite3_bind_int64(stat, 3, tmLastMonthStart);
					sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Monthly);
					//printf("input kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}

				//monthly output kwh
				sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", tmLastMonthStart, TYPE_OUTPUT_KWH_M);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				if(sqlite3_step(stat) == SQLITE_ROW)
				{
					//now update the record
					sqlite3_finalize(stat);//delete stat
					sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", tmLastMonthStart, TYPE_OUTPUT_KWH_M);
					sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
					sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Monthly);
					//printf("output kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
				else
				{
					sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
					TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA OutputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

					sqlite3_finalize(stat);
					sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
					sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
					sqlite3_bind_int(stat, 2, TYPE_OUTPUT_KWH_M);
					sqlite3_bind_int64(stat, 3, tmLastMonthStart);
					sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Monthly);
					//printf("output kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
			}

			if(pRec->nFlagIsYearDataRecordNeeded == 1)
			{
				struct tm gmTimeLast;
				time_t tmLast = nDay;
        		gmtime_r(&tmLast, &gmTimeLast);
				gmTimeLast.tm_mday = 1;
				gmTimeLast.tm_mon = 0;
				time_t tmLastYearStart = mktime(&gmTimeLast);

				//yearly input kwh
				sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", tmLastYearStart, TYPE_INPUT_KWH_Y);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				if(sqlite3_step(stat) == SQLITE_ROW)
				{
					//now update the record
					sqlite3_finalize(stat);//delete stat
					sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", tmLastYearStart, TYPE_INPUT_KWH_Y);
					sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
					sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Yearly);
					//printf("input kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);			
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
				else
				{
					sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
					TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA InputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

					sqlite3_finalize(stat);
					sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
					sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
					sqlite3_bind_int(stat, 2, TYPE_INPUT_KWH_Y);
					sqlite3_bind_int64(stat, 3, tmLastYearStart);
					sprintf(szStringVal, "%f", pRec->fTotalACInputKWH_Yearly);
					//printf("input kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}

				//yearly output kwh
				sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE Time = %d AND Type = %d", tmLastYearStart, TYPE_OUTPUT_KWH_Y);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				if(sqlite3_step(stat) == SQLITE_ROW)
				{
					//now update the record
					sqlite3_finalize(stat);//delete stat
					sprintf(szSql, "update Data_AccumulatedData set Data = (?) where Time = %d and Type = %d", tmLastYearStart, TYPE_OUTPUT_KWH_Y);
					sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
					sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Yearly);
					//printf("output kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 1, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute Updating Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
				else
				{
					sqlite3_exec(data_db,"SELECT count(*) FROM Data_AccumulatedData", Get_MaxEventIdx, &s_nCurrAccumulatedDataIdx, &szErrMsg);
					TRACE("---Replace EVENT_TYPE_SAVE_ACCUMULATED_DATA OutputKWH Sqlite Data---s_nCurrAccumulatedDataIdx=[%d]\n", s_nCurrAccumulatedDataIdx);

					sqlite3_finalize(stat);
					sqlite3_prepare_v2(data_db, "replace into Data_AccumulatedData values (?,?,?,?);",-1,&stat,NULL);
					sqlite3_bind_int(stat, 1, s_nCurrAccumulatedDataIdx);
					sqlite3_bind_int(stat, 2, TYPE_OUTPUT_KWH_Y);
					sqlite3_bind_int64(stat, 3, tmLastYearStart);
					sprintf(szStringVal, "%f", pRec->fTotalACOutputKWH_Yearly);
					//printf("output kwh string is: %s\n", szStringVal);
					sqlite3_bind_blob(stat, 4, (void*)szStringVal, strlen(szStringVal) + 1, NULL);
					int nSqlRet = sqlite3_step(stat);				
					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute replace into Data_AccumulatedData failed, errCode = %d, errMsg = [%s]---\n",
							nSqlRet, sqlite3_errmsg(data_db));
					}
					sqlite3_finalize(stat);//delete stat
				}
			}

			Close_UnLock_SqliteDB();
			
			SAFELY_DELETE(pEventArg->pEvData);
			return ERR_DPR_OK;	
		}
		
	}
	else //other event
	{
		if((pEventArg->emEvType == EVENT_TYPE_SYSTEM_EXIT) || 
			(pEventArg->emEvType == EVENT_TYPE_FIRMWARE_UPDATE) ||
			(pEventArg->emEvType == EVENT_TYPE_IN_MAINTAINING) ||
			(pEventArg->emEvType == EVENT_TYPE_ABILITY_CHANGE) ||
			(pEventArg->emEvType == EVENT_TYPE_GUN_RESERVE) ||
			(pEventArg->emEvType == EVENT_TYPE_DIAGNOSTIC_UPLOAD))
		{
			TRACEX(" EVENT_TYPE_GENERAL_EVENT, msgType[%d]!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", pEventArg->emMsgType);
			if(pEventArg->emMsgType == EVENT_MSG_BEGIN)
			{
				sqlite3* data_db = Open_Lock_SqliteDB();
				sqlite3_prepare_v2(data_db,"replace into Data_Events values (?,?,?,?,?,?);",-1,&stat,NULL);
				sqlite3_bind_int(stat,1, (int)s_nCurrEventIdx);
				sqlite3_bind_int(stat,2, pEventArg->emEvType);
				sqlite3_bind_int(stat,3, EVENT_STATUS_OCCUR);
				sqlite3_bind_blob(stat, 4, (void*)pEventArg->pEvData, pEventArg->nEvDataSize, NULL);
				sqlite3_bind_int(stat,5, _NOW_);
				sqlite3_bind_int(stat,6, 0);
				int nSqlRet = sqlite3_step(stat);
				sqlite3_finalize(stat);//delete stat
				Close_UnLock_SqliteDB();
				s_nCurrEventIdx++;
				if(s_nCurrEventIdx > MAX_EVENT_RECORDS)
				{
					s_nCurrEventIdx = 1;
				}

				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_Events values (?,?,?,?,?,?);", 
						nSqlRet, sqlite3_errmsg(data_db));
					//system("reboot");
					LogOut(MAIN_MODULE_DPR, "DPR_EvHand", LOG_TYPE_ERROR,
						"---Execute sql[%s], errCode = %d, errMsg = [%s]---", "replace into Data_Events values (?,?,?,?,?,?);", 
						nSqlRet, sqlite3_errmsg(data_db));
				}

			}
			else if(pEventArg->emMsgType == EVENT_MSG_END)
			{
				sqlite3* data_db = Open_Lock_SqliteDB();
				//there should only be one instance for each type of EVENT!
				char szSql[256];
				if(pEventArg->emEvType == EVENT_TYPE_ABILITY_CHANGE)
				{
					//handle ability change end
					int kk;
					for(kk =0 ; kk < 10; kk++)
					{
						//to delete all the gun records
						if(!Handle_AbilityChange_End(data_db, ((EVENT_ABILITY_CHG*)pEventArg->pEvData)->nWhichGun))
						{
							break;
						}
					}
				}
				else
				{
					sprintf(szSql, "update Data_Events set Event_Status = %d, End_Time = %d, Event_Data = (?) "
						"where Event_Type = %d and Event_Status = %d", 
						EVENT_STATUS_NOT_OCCUR, _NOW_,
						pEventArg->emEvType, EVENT_STATUS_OCCUR);
					TRACEX("Now execute the sql[%s]\n", szSql);
					sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
					sqlite3_bind_blob(stat, 1, (void*)pEventArg->pEvData, pEventArg->nEvDataSize, NULL);
					int nSqlRet = sqlite3_step(stat);
					sqlite3_finalize(stat);//delete stat

					if(nSqlRet != SQLITE_DONE)
					{
						TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
						//system("reboot");
					}
				}
				Close_UnLock_SqliteDB();
			}
			else if(pEventArg->emMsgType == EVENT_MSG_STATUS_CHANGED)
			{
				sqlite3* data_db = Open_Lock_SqliteDB();
				char szSql[256];
				sprintf(szSql, "update Data_Events set Event_Data = (?) "
					"where Event_Type = %d and Event_Status = %d", 
					pEventArg->emEvType, EVENT_STATUS_OCCUR);
				TRACEX("Now execute the sql[%s]\n", szSql);
				sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
				sqlite3_bind_blob(stat, 1, (void*)pEventArg->pEvData, pEventArg->nEvDataSize, NULL);
				int nSqlRet = sqlite3_step(stat);
				sqlite3_finalize(stat);//delete stat
				Close_UnLock_SqliteDB();
				if(nSqlRet != SQLITE_DONE)
				{
					TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
					//system("reboot");
				}
			}
		}
	}

	//now execute the call back for every registered caller
	//if (Mutex_Lock(s_hEventHandMutex, 1000) != ERR_MUTEX_OK)
	//{
	//	SAFELY_DELETE(pEventArg->pEvData);
	//	return ERR_DPR_MUTEX_ERR;
	//}
	Mutex_Lock(s_hEventHandMutex, WAIT_INFINITE);

	EVENT_HANDLER_REGISTER *pItem = s_TableEvHandlers;
	TRACEX("total msgHandler:%d pEventArg->emEvType=[%d]\n", s_nEventHandlerTotal,pEventArg->emEvType);
	for(n = 0; n < s_nEventHandlerTotal ; n++, pItem++)
	{
		if ((pItem->pfnEvHandler != NULL) && 
			((pItem->nEvTypeFlag) == pEventArg->emEvType) &&
			((pItem->nMsgTypeFlag & pEventArg->emMsgType) == pEventArg->emMsgType))
		{
			//if(pEventArg->ptrReciver.emObjType != EVENT_OBJ_NOT_SPECIFIED)
			//{
			//	MODULE_TYPE* pModTy = (MODULE_TYPE*)pItem->hModuleSelf;
			//	if(((*pModTy == MODULE_TYPE_SAMPLER) && (pEventArg->ptrReciver.emObjType != EVENT_OBJ_SAMPLER)) ||
			//		((*pModTy == MODULE_TYPE_REPORTER) && (pEventArg->ptrReciver.emObjType != EVENT_OBJ_REPORTER))||
			//		((*pModTy == MODULE_TYPE_DATAPROC) && (pEventArg->ptrReciver.emObjType != EVENT_OBJ_DATAPROCESSOR)))
			//	{
			//		continue;
			//	}
			//}
			pItem->pfnEvHandler(pItem->hModuleSelf,
				pEventArg->emEvType,
				pEventArg->emMsgType,
				pEventArg->nEvDataSize,
				pEventArg->pEvData,
				pEventArg->bBlockMode,
				pEventArg->nMaxBlockMSec);

			//call back to sender for noticfaication!
			if(pEventArg->pfnReceipt != NULL)
			{
				pEventArg->pfnReceipt(hSender, 
					pEventArg->pSenderCallBackArg);
			}
		}
	}

	Mutex_Unlock(s_hEventHandMutex);

	if((pEventArg->emEvType == EVENT_TYPE_SYSTEM_EXIT)&&
		(pEventArg->emMsgType == EVENT_MSG_BEGIN))
	{
		TRACEX("------------------------System will exit now-----------------------\n");
		//pCharger->bExit = TRUE;
		//Sleep(3000);
		//_SYSTEM("reboot");
		pCharger->bResetCmdPending = TRUE;
	}

	SAFELY_DELETE(pEventArg->pEvData);

	return ERR_DPR_OK;
}


//re-compress related table for old software db
static void ReCompressData_4OldSWVer(sqlite3* data_db)
{
	//first check the EVENT_TYPE_DB_COMPRESSED,
	//if existed, that means the compress is already done
	int nEventCount_CompEvt = 0;
	char szSql[256] = {0};
	snprintf(szSql, 256, "SELECT count(*) FROM Data_Events where Event_Type = %d", EVENT_TYPE_DB_COMPRESSED);	
	sqlite3_exec(data_db,szSql, Get_CompressEvent, &nEventCount_CompEvt, NULL);	
	if(nEventCount_CompEvt > 0)
	{
		TRACEX("The compress already done! so we skip this step!");
		return;
	}

	LogOut_A(MAIN_MODULE_DPR, "RecompressDB", LOG_TYPE_WARING,
					"-------DB is being compressed!------");
	
	//now begin the transaction for DB
	sqlite3_exec(data_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);	

	sqlite3_stmt * stat_query;
	sqlite3_stmt * stat_update;
	int nIdx;
	
	int nBlobLen;
	void* pBlobData;
	void* pOut = NULL;

	//----------Step 1: Compresseing for Data_ChargeMeterValues----------
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_ChargeMeterValues", -1, &stat_query, NULL);
	while(sqlite3_step(stat_query) == SQLITE_ROW)
	{
		//idx(column #1)
		nIdx = sqlite3_column_int(stat_query, 0);
		//meter_data(column #3)
		pBlobData = sqlite3_column_blob(stat_query, 3);
		nBlobLen = sqlite3_column_bytes(stat_query, 3);		
		snprintf(szSql, 256, "update Data_ChargeMeterValues set Meter_Data = (?) "
			"where Idx = %d", nIdx);
		sqlite3_prepare_v2(data_db, szSql, -1, &stat_update, NULL);
		sqlite3_bind_blob_compress(stat_update, 1, pBlobData, nBlobLen, &pOut, NULL);
		sqlite3_step(stat_update);
		sqlite3_finalize_compressed(stat_update, pOut);//delete stat
	}
	sqlite3_finalize(stat_query);

	//now insert the EVENT_TYPE_DB_COMPRESSED into event table in case for next time identification
	//Jimmy changed status 0 => 1 so that for new software no need re-translate the meterValues
	snprintf(szSql, 256, "replace into Data_Events values (%d, %d, 1, 0, %d, %d)",
			 s_nCurrEventIdx, EVENT_TYPE_DB_COMPRESSED, _NOW_, _NOW_);	
	//char* szErrMsg = NULL;
	int nRetS = sqlite3_exec(data_db, szSql, NULL, NULL, NULL/*,&szErrMsg*/);	
	//if(szErrMsg != NULL)
	if(nRetS == SQLITE_OK)
	{
		sqlite3_exec(data_db, "COMMIT;VACUUM;", NULL, NULL, NULL);	
		s_nCurrEventIdx++;
		if(s_nCurrEventIdx > MAX_EVENT_RECORDS)
		{
			s_nCurrEventIdx = 1;
		}
	}
	else
	{
		sqlite3_exec(data_db, "ROLLBACK;", NULL, NULL, NULL);
		//sqlite3_free(szErrMsg);	
		TRACEX("---now has error[%d] ths sql[%s], so need rollback the transaction for SQlite\n",
			nRetS, szSql);
	}	

	LogOut_A(MAIN_MODULE_DPR, "RecompressDB", LOG_TYPE_WARING,
				"-------DB Compressed done!------");
}
void RestoreHisEvent(CHARGER* pCharger)
{
	sqlite3 *data_db = (sqlite3*)pCharger->hRunDB;
	char* szErrMsg = NULL;
	char szSqlKwh[1024]={0};
	UINT nNeedaSveSPFWDevMap[2]={401,407};
	UINT i = 0;
	//init get HisEventIdx
	sqlite3_exec(data_db,"SELECT count(*) FROM Data_Events", Get_MaxEventIdx, &s_nCurrEventIdx, &szErrMsg);
	if(s_nCurrEventIdx > MAX_EVENT_RECORDS)
	{
		sqlite3_exec(data_db,"SELECT Idx FROM Data_Events WHERE Begin_Time IN(SELECT max(Begin_Time) FROM Data_Events)", Get_EventIdx, &s_nCurrEventIdx, &szErrMsg);
	}

	sqlite3_exec(data_db,"SELECT count(*) FROM Data_Alarms", Get_MaxEventIdx, &s_nCurrAlarmIdx, &szErrMsg);
	if(s_nCurrAlarmIdx > MAX_ALARM_RECORDS)
	{
		sqlite3_exec(data_db,"SELECT Idx FROM Data_Alarms WHERE Alarm_StartTime IN(SELECT max(Alarm_StartTime) FROM Data_Alarms)", Get_EventIdx, &s_nCurrAlarmIdx, &szErrMsg);
	}

	//sqlite3_exec(data_db,"SELECT count(*) FROM Data_ChargeRecords", Get_MaxEventIdx, &s_nCurrChargeRecIdx, &szErrMsg);
	//if(s_nCurrChargeRecIdx > MAX_CHARGE_RECORDS)
	//{
	//	sqlite3_exec(data_db,"SELECT Idx FROM Data_ChargeRecords WHERE Charge_BeginTime IN(SELECT max(Charge_BeginTime) FROM Data_ChargeRecords)", Get_EventIdx, &s_nCurrChargeRecIdx, &szErrMsg);
	//}

	//sqlite3_exec(data_db,"SELECT count(*) FROM Data_ChargeMeterValues", Get_MaxEventIdx, &s_nCurrChargeMeterValIdx, &szErrMsg);
	//if(s_nCurrChargeMeterValIdx > MAX_CHARGE_RECORDS)
	//{
	//	sqlite3_exec(data_db,"SELECT Idx FROM Data_ChargeMeterValues WHERE Charge_Begin_Time IN(SELECT max(Charge_BeginTime) FROM Data_ChargeMeterValues)", Get_EventIdx, &s_nCurrChargeMeterValIdx, &szErrMsg);
	//}
	sqlite3_exec(data_db,"SELECT count(*) FROM Data_ChargeRecords", Get_MaxEventIdx, &s_nCurrChargeRecIdx, &szErrMsg);
	if(s_nCurrChargeRecIdx > MAX_CHARGE_RECORDS)
	{
		//added by Jimmy 20200921 for avoding the over-date overwrite problem
		int nGetBelowCurrMaxId = 0;
		char szSql[256] = {0};
		snprintf(szSql, 256, "SELECT Idx FROM Data_ChargeRecords WHERE Charge_BeginTime "
			"IN(SELECT max(Charge_BeginTime) FROM Data_ChargeRecords WHERE Charge_BeginTime < %d)", _NOW_);
		sqlite3_exec(data_db, szSql,  Get_MaxEventIdx, &nGetBelowCurrMaxId, &szErrMsg);
		if(nGetBelowCurrMaxId <= 0)
		{
			sqlite3_exec(data_db,"SELECT Idx FROM Data_ChargeRecords WHERE Charge_BeginTime IN(SELECT max(Charge_BeginTime) FROM Data_ChargeRecords)", Get_EventIdx, &s_nCurrChargeRecIdx, &szErrMsg);
		}
		else
		{
			s_nCurrChargeRecIdx = nGetBelowCurrMaxId;
		}		
	}

	sqlite3_exec(data_db,"SELECT count(*) FROM Data_ESStats_Daily", Get_MaxEventIdx, &s_nCurrESStatDataIdx, &szErrMsg);
	if(s_nCurrESStatDataIdx > MAX_ES_STATDAILY_DATA)
	{
		sqlite3_exec(data_db,"SELECT Idx FROM Data_ESStats_Daily WHERE Day IN(SELECT max(Day) FROM Data_ESStats_Daily)", Get_EventIdx, &s_nCurrESStatDataIdx, &szErrMsg);
	}

	sqlite3 *data_bms = Open_Lock_Database(DB_TYPE_BMSDATA);
	sqlite3_exec(data_bms,"SELECT count(*) FROM Data_RealTimeBmsData", Get_MaxEventIdx, &s_nCurrBmsRunDataValIdx, &szErrMsg);
	if(s_nCurrBmsRunDataValIdx > MAX_BMS_RECORDS_NUM)
	{
		sqlite3_exec(data_bms,"SELECT Idx FROM Data_RealTimeBmsData WHERE Day IN(SELECT max(Day) FROM Data_RealTimeBmsData)", Get_EventIdx, &s_nCurrBmsRunDataValIdx, &szErrMsg);
	}
	Close_UnLock_Database(DB_TYPE_BMSDATA);

	sqlite3_exec(data_db,"SELECT count(*) FROM Data_BMSHealthData", Get_MaxEventIdx, &s_nCurrHealthDataValIdx, &szErrMsg);
	if(s_nCurrHealthDataValIdx > MAX_HEALTH_RECORDS_DAY)
	{
		sqlite3_exec(data_db,"SELECT Idx FROM Data_BMSHealthData WHERE Day IN(SELECT max(Day) FROM Data_BMSHealthData)", Get_EventIdx, &s_nCurrHealthDataValIdx, &szErrMsg);
	}

	TRACEW("RestoreHisEvent--------");
	sqlite3_exec(data_db,"SELECT count(*) FROM Data_CurrentSPFW_kwh", Get_MaxEventIdx, &s_nCurrMeterKwhValIdx, &szErrMsg);
	if(s_nCurrMeterKwhValIdx > MAX_METER_DATA_RECORDS_DAY)
	{
		sqlite3_exec(data_db,"SELECT Idx FROM Data_CurrentSPFW_kwh WHERE Day IN(SELECT max(Day) FROM Data_CurrentSPFW_kwh)", Get_EventIdx, &s_nCurrMeterKwhValIdx, &szErrMsg);
	}

	TRACEX("----Now get back event IDs[%d, %d, %d, %d, %d, %d, %d]----\n", \
		s_nCurrEventIdx, s_nCurrAlarmIdx, s_nCurrChargeRecIdx, s_nCurrChargeMeterValIdx, \
		s_nCurrESStatDataIdx,s_nCurrBmsRunDataValIdx, s_nCurrHealthDataValIdx);

	//restore alarms
	//now begin the transaction for DB
	sqlite3_exec(data_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);	//Jimmy added 20210528 to delte thoee wrong/repeated alarms
	sqlite3_stmt * stat;
	//sqlite3_prepare_v2(data_db, "SELECT * FROM Data_Alarms WHERE Alarm_Status = 1 ORDER BY Alarm_StartTime DESC", -1, &stat, NULL);
	//changed by Jimmy: even for suppressed ALARM, we should restore it so as to manage it!!!
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_Alarms WHERE Alarm_Status > 0 ORDER BY Alarm_StartTime DESC", -1, &stat, NULL);
	char szSqlUPPPP[256];
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		int nIdx = sqlite3_column_int(stat, 0);
		int nDevId = sqlite3_column_int(stat, 1);
		int nAlarmId = sqlite3_column_int(stat, 2);
		ALARM* pAlarm = GetAlarm_ByPR(nDevId, nAlarmId);
		if(pAlarm == NULL)
		{
			LogOut(MAIN_MODULE_DPR, "RestoreAlarm", LOG_TYPE_WARING,
				"Found an INVALID alarm[%d, %d] and will set it CEASED!", nDevId, nAlarmId);
			sprintf(szSqlUPPPP, "update Data_Alarms set Alarm_Status = 0, Alarm_EndTime = %d "
				"where Idx = %d",_NOW_, nIdx);
			sqlite3_exec(data_db, szSqlUPPPP, NULL, NULL, NULL);
			continue;
		}
		//now restore the alarm
		if(pAlarm->emStatus != ALARM_STATUS_ACTIVE) //if already restored ,then skip!
		{
			float fExVal = (float)sqlite3_column_double(stat, 4);
			time_t tmStart = (time_t)sqlite3_column_int(stat, 5);

			pAlarm->emStatus = ALARM_STATUS_ACTIVE;
			pAlarm->tmLastOccured = tmStart;
			pAlarm->tmLastRestored = _NOW_;

			//printf("Now try to restore alarm for [%d, %d]\n", nDevId, nAlarmId);
			LogOut(MAIN_MODULE_DPR, "RestoreAlarm", LOG_TYPE_WARING,
				"RESTORED a hist alarm[%d, %d]!!!", nDevId, nAlarmId);
			EVENT_ALARM evAlarm;
			evAlarm.status = ALARM_STATUS_ACTIVE;
			evAlarm.nDevId = nDevId;
			evAlarm.nAlarmId = nAlarmId;
			evAlarm.tmStart = tmStart;
			evAlarm.fExParamVal = fExVal;
			//Now raise alarm occur event and save/send out/**/
			EVENT_ARG arg;
			ZERO_POBJS(&arg, 1);
			arg.hSender = (HANDLE)&pCharger->dataprocessor;
			arg.emEvType = EVENT_TYPE_ALARM;
			arg.emMsgType = EVENT_MSG_RESTORE;
			arg.nEvDataSize = sizeof(EVENT_ALARM); //use pointer
			arg.pEvData = (void*)NEW(EVENT_ALARM, 1);
			memcpy(arg.pEvData, &evAlarm, arg.nEvDataSize);
			Queue_Put(pCharger->hqEvent, &arg, FALSE);
		}
		else
		{
			LogOut(MAIN_MODULE_DPR, "RestoreAlarm", LOG_TYPE_WARING,
				"Found a REPEATED alarm[%d, %d] and will set it CEASED!", nDevId, nAlarmId);
			sprintf(szSqlUPPPP, "update Data_Alarms set Alarm_Status = 0, Alarm_EndTime = %d "
				"where Idx = %d",_NOW_, nIdx);
			sqlite3_exec(data_db, szSqlUPPPP, NULL, NULL, NULL);
		}
	}
	sqlite3_finalize(stat);
	sqlite3_exec(data_db, "COMMIT;VACUUM;", NULL, NULL, NULL);	//now commit the delete
	//restore charge records
	//sqlite3_prepare(data_db, "SELECT * FROM Data_ChargeRecords WHERE ChargingStatus != 2 ORDER BY Idx DESC", -1, &stat, NULL);
	//BOOL bGunRestored[MAX_SUPPORT_GUN_NUM] = {FALSE};
	//int nRecCount = 0;
	//while(sqlite3_step(stat) == SQLITE_ROW)
	//{
	//	nRecCount++;
	//
	//	EVENT_CHARGING evRestore;
	//	//A_BLOB_RUN_CHARGE_INFO chargeData;
	//	//MAKE_BLOB(chargeData, BLOB_TYPE_RUN_CHARGE_INFO);
	//	A_BLOB_RUN_CHARGE_INFO* pResChgData = NEW(A_BLOB_RUN_CHARGE_INFO, 1); //will be destroyed by CCU sampler
	//	evRestore.emGunNum = (BYTE)sqlite3_column_int(stat, 1);
	//	int nCurrIdx = evRestore.emGunNum -1;
	//	
	//	TRACEX("-------Found a recorrr for Gun[%d], Total found = [%d]----------\n", evRestore.emGunNum, nRecCount);
	//
	//	//every time we just restore only one record for each gun
	//	if((nCurrIdx >= 0) && (nCurrIdx < MAX_SUPPORT_GUN_NUM))
	//	{
	//		if(!bGunRestored[nCurrIdx])
	//		{
	//			bGunRestored[nCurrIdx] = TRUE;
	//		}
	//		else
	//		{
	//			DELETE(pResChgData);
	//			continue;
	//		}
	//	}
	//	else
	//	{
	//		DELETE(pResChgData);
	//		continue;
	//	}
	//
	//	evRestore.tmChargeStart = (time_t)sqlite3_column_int(stat, 2);
	//	evRestore.nSessionID = sqlite3_column_int(stat, 3);
	//	pResChgData->tmChargeStart = evRestore.tmChargeStart;
	//	pResChgData->nSessionID = evRestore.nSessionID;
	//	memcpy(pResChgData->szUserID, sqlite3_column_text(stat, 4), MIN(64, sqlite3_column_bytes(stat, 4)));
	//	memcpy(pResChgData->szVIN, sqlite3_column_text(stat, 5), MIN(64, sqlite3_column_bytes(stat, 5)));
	//	pResChgData->emChargeStartMethod = (CHARGE_START_MOTHOD)sqlite3_column_int(stat, 6);
	//	pResChgData->emChargeStatus = (BYTE)sqlite3_column_int(stat, 7);
	//	pResChgData->nReportFlag = (BYTE)sqlite3_column_int(stat, 8);
	//	pResChgData->bIsOfflinePayedOK = (BYTE)sqlite3_column_int(stat, 9);
	//	int nRecSize = sqlite3_column_bytes(stat, 10);
	//	if(nRecSize != sizeof(CHARGE_REC_EXTRADATA))
	//	{
	//		DELETE(pResChgData);
	//		continue;
	//	}
	//	memcpy(&pResChgData->extraData, (RUN_CHARGE_EXTRA_DATA*)sqlite3_column_text(stat, 10), /*nRecSize*/ sizeof(RUN_CHARGE_EXTRA_DATA));
	//	evRestore.pRestoreChgData = (void*)pResChgData;
	//	EVENT_ARG arg;
	//	ZERO_POBJS(&arg, 1);
	//	arg.hSender = (HANDLE)&pCharger->dataprocessor;
	//	arg.emEvType = EVENT_TYPE_CHARGING;
	//	arg.emMsgType = EVENT_MSG_RESTORE;
	//	arg.nEvDataSize = sizeof(EVENT_CHARGING); //use pointer
	//	arg.pEvData = (void*)NEW(EVENT_CHARGING, 1);
	//	if(arg.pEvData == NULL)
	//	{
	//		DELETE(pResChgData);
	//		LogOut(MAIN_MODULE_DPR, "RestoreChargeRec", LOG_TYPE_ERROR,
	//			"Can NOT allocate memory for a charge record[%d, %d]", evRestore.emGunNum , evRestore.tmChargeStart);
	//		continue;
	//	}
	//	memcpy(arg.pEvData, &evRestore, arg.nEvDataSize);
	//	Queue_Put(pCharger->hqEvent, &arg, FALSE);
	//}
	//sqlite3_finalize(stat);
	

	//handle other event
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_Events WHERE Event_Status != 0", -1, &stat, NULL);
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		EVENT_ARG arg;
		ZERO_POBJS(&arg, 1);
		arg.emEvType = (_EVENT_TYPE)sqlite3_column_int(stat, 1);
		arg.emMsgType = EVENT_MSG_RESTORE;//sqlite3_column_int(stat, 2);
		if(arg.emEvType == EVENT_TYPE_SYSTEM_EXIT)
		{
			arg.emMsgType = EVENT_MSG_END;
		}
		size_t nBlobSize = sqlite3_column_bytes(stat, 3);
#define MAX_EVENT_ARG_BLOBSIZE	10240
		if((nBlobSize > MAX_EVENT_ARG_BLOBSIZE) || (nBlobSize < 1))
		{
			TRACEX("-----Event[%d], Blobsize not correct[%d]------\n",arg. emEvType, nBlobSize);
			continue;
		}
		arg.nEvDataSize = (UINT)nBlobSize;
		arg.pEvData = (void*)NEW(BYTE, nBlobSize);
		if(arg.pEvData == NULL)
		{
			LogOut(MAIN_MODULE_DPR, "RestoreEvent", LOG_TYPE_ERROR,
				"Can NOT allocate memory for a EventType[%d]", arg.emEvType);
			continue;
		}
		memcpy(arg.pEvData, sqlite3_column_blob(stat, 3), nBlobSize);
		arg.hSender = (HANDLE)&pCharger->dataprocessor;
		Queue_Put(pCharger->hqEvent, &arg, FALSE);
		TRACEX("------Now restore EVENT[%d]-------\n", arg.emEvType);
	}
	sqlite3_finalize(stat);
	
	//restore his es_statistic data
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_ESStats_Daily WHERE Idx = (?)", -1, &stat, NULL);
	sqlite3_bind_int(stat, 1, 1);
	if(sqlite3_step(stat) == SQLITE_ROW)
	{
		// printf("Now restore a daily data of es_statistic\n");
		size_t nBlobSize = sqlite3_column_bytes(stat, 2);
		if(nBlobSize != sizeof(HIS_ES_STATISTIC_DAILY))
		{
			LogOut(MAIN_MODULE_DPR, "RestoreEvent", LOG_TYPE_ERROR,
				"Found an INVAID record for ES statistic!");
			
			sqlite3_finalize(stat);
			
			return;
		}

		EVENT_ES_SAVE_STATDATA evSavedEsData;
		HIS_ES_STATISTIC_DAILY* pDailyData = NEW(HIS_ES_STATISTIC_DAILY, 1); //will be destroyed by CCU sampler
		memcpy(pDailyData, sqlite3_column_blob(stat, 2), nBlobSize);
		evSavedEsData.pDailyData = pDailyData;
		EVENT_ARG arg;
		ZERO_POBJS(&arg, 1);
		arg.hSender = (HANDLE)&pCharger->dataprocessor;
		arg.emEvType = EVENT_TYPE_SAVE_ES_STATDATA;
		arg.emMsgType = EVENT_MSG_RESTORE;
		arg.nEvDataSize = sizeof(EVENT_ES_SAVE_STATDATA); //use pointer
		arg.pEvData = (void*)NEW(EVENT_ES_SAVE_STATDATA, 1);
		memcpy(arg.pEvData, &evSavedEsData, arg.nEvDataSize);
		Queue_Put(pCharger->hqEvent, &arg, TRUE);
		
		// printf("---EVENT_TYPE_SAVE_ES_STATDATA---\n");
	}
	
	sqlite3_finalize(stat);
	
	//---------------------- restore  BMS RealTime Statistic data ---------------------//
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_RealTimeBmsData WHERE Day = (?)", -1, &stat, NULL);
	sqlite3_bind_int(stat, 1, (int)ConvDateTime_2DateOnly(_NOW_));
	if(sqlite3_step(stat) == SQLITE_ROW)
	{
		//size_t nBlobSize = sqlite3_column_bytes(stat, 3);
		size_t nBlobSize = sqlite3_sizeof_blob_uncompressed(stat, 3); // sizeof(BMS_RUNTIMEDATA_LIST_V1) 5280020

		// printf("Now restore a Data_RealTimeBmsData nBlobSize=[%d] old[%d] BMS_RUNTIMEDATA_LIST[%d] HIS_BMS_REC_SIZE=[%d]\n",
		// 	nBlobSize, sizeof(BMS_RUNTIMEDATA_LIST_V1), sizeof(BMS_RUNTIMEDATA_LIST), sizeof(HIS_BMS_RUNTIMEDATA_REC));

		//if(nBlobSize != sizeof(HIS_BMS_RUNTIMEDATA_REC))
		if(nBlobSize <= 0)
		{
			LogOut(MAIN_MODULE_DPR, "RestoreEvent", LOG_TYPE_ERROR,
				"Found an INVAID record for RealTime Bms Data!");
			sqlite3_finalize(stat);
			return;
		}

		EVENT_BMSRUNDATA_UPDATE bmsSavedData;
		HIS_BMS_RUNTIMEDATA_REC* pBmsRunDataRec = NEW(HIS_BMS_RUNTIMEDATA_REC, 1); //will be destroyed by STBATT sampler
		//memcpy(pBmsRunDataRec, sqlite3_column_blob(stat, 3), nBlobSize);
		pBmsRunDataRec->nDay = sqlite3_column_int(stat, 1);
		pBmsRunDataRec->tmLogTime = sqlite3_column_int(stat, 2);
		// LJH240925 RunData LJH250303 delete BMS_RUNTIMEDATA_LIST_V1
		// if (nBlobSize == sizeof(BMS_RUNTIMEDATA_LIST_V1))
		// {
		// 	BMS_RUNTIMEDATA_LIST_V1 bmsRunData_V1;
		// 	ZERO_POBJS(&bmsRunData_V1, 1);
		// 	sqlite3_parse_blob_uncompressed(stat, 3, 
		// 			(void*)&bmsRunData_V1, sizeof(BMS_RUNTIMEDATA_LIST_V1));

		// 	int n = 0;
		// 	pBmsRunDataRec->bmsRunData.nBmsRuntimeDataTotal = bmsRunData_V1.nBmsRuntimeDataTotal;
		// 	for (n=0; n<bmsRunData_V1.nBmsRuntimeDataTotal; ++n)
		// 	{
		// 		memcpy(&pBmsRunDataRec->bmsRunData.stBmsRuntimeDataList[n],
		// 			&bmsRunData_V1.stBmsRuntimeDataList[n], sizeof(A_BMS_RUNTIME_DATA_ITEM_V1));
		// 	}
		// }
		// else
		if (nBlobSize == sizeof(BMS_RUNTIMEDATA_LIST))
		{
			sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pBmsRunDataRec->bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST));
		}
		// old
		// sqlite3_parse_blob_uncompressed(stat, 3, 
		// 			(void*)&pBmsRunDataRec->bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST));
		bmsSavedData.pBmsRunDataRec = pBmsRunDataRec;
		TRACED("RestoreHisEvent nDay:%d nBmsRuntimeDataTotal:%d", pBmsRunDataRec->nDay, pBmsRunDataRec->bmsRunData.nBmsRuntimeDataTotal);
		EVENT_ARG arg;
		ZERO_POBJS(&arg, 1);
		arg.hSender = (HANDLE)&pCharger->dataprocessor;
		arg.emEvType = EVENT_TYPE_BMSRUNDATA_UPDATE; // EVENT_TYPE_BMSRUNDATA_UPDATE && EVENT_MSG_RESTORE
		arg.emMsgType = EVENT_MSG_RESTORE;
		arg.nEvDataSize = sizeof(EVENT_BMSRUNDATA_UPDATE); //use pointer
		arg.pEvData = (void*)NEW(EVENT_BMSRUNDATA_UPDATE, 1);
		memcpy(arg.pEvData, &bmsSavedData, arg.nEvDataSize);
		int nRet = Queue_Put(pCharger->hqEvent, &arg, TRUE);
		
		printf("Event_Manager:---EVENT_TYPE_BMSRUNDATA_UPDATE---nRet=[%d]\n", nRet);
	}
	
	sqlite3_finalize(stat);



	//---------------------- restore  BMS Health data 20220519 John---------------------//
	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_BMSHealthData WHERE Day = (?)", -1, &stat, NULL);
	sqlite3_bind_int(stat, 1, (int)ConvDateTime_2DateOnly(_NOW_));
	if(sqlite3_step(stat) == SQLITE_ROW)
	{
		//size_t nBlobSize = sqlite3_column_bytes(stat, 3);
		size_t nBlobSize = sqlite3_sizeof_blob_uncompressed(stat, 3);
		
		printf("Now restore a Data_BMSHealthData nBlobSize=[%d]HIS_BMS_HEALTH_SIZE=[%d]\n", nBlobSize, sizeof(HIS_HEALTH_STATIC_DAILY));

		//if(nBlobSize != sizeof(HIS_HEALTH_STATIC_DAILY))
		if(nBlobSize <= 0)
		{
			LogOut(MAIN_MODULE_DPR, "RestoreEvent", LOG_TYPE_ERROR,
				"Found an INVAID Bms Health Daily Data!");
			sqlite3_finalize(stat);
			return;
		}

		EVENT_BMSHEALTH_UPDATE evtHealthData;
		HIS_HEALTH_STATIC_DAILY* pHisHealthRec = NEW(HIS_HEALTH_STATIC_DAILY, 1); //will be destroyed by STBATT sampler
		//memcpy(pBmsRunDataRec, sqlite3_column_blob(stat, 3), nBlobSize);
		pHisHealthRec->nDay = sqlite3_column_int(stat, 1);
		pHisHealthRec->tmLogTime = sqlite3_column_int(stat, 2);
		sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pHisHealthRec->bmsHealthData, sizeof(A_BMS_HEALTH_LIST));
 
		evtHealthData.pHisBmsHealthRec = pHisHealthRec;
		EVENT_ARG arg;
		ZERO_POBJS(&arg, 1);
		arg.hSender = (HANDLE)&pCharger->dataprocessor;
		arg.emEvType = EVENT_TYPE_HEALTHDATA_UPDATE;
		arg.emMsgType = EVENT_MSG_RESTORE;
		arg.nEvDataSize = sizeof(EVENT_BMSHEALTH_UPDATE); //use pointer
		arg.pEvData = (void*)NEW(EVENT_BMSHEALTH_UPDATE, 1);
		memcpy(arg.pEvData, &evtHealthData, arg.nEvDataSize);
		int nRet = Queue_Put(pCharger->hqEvent, &arg, TRUE);
		
		printf("Event_Manager:---EVENT_BMSHEALTH_UPDATE---nRet=[%d]\n", nRet);
	}
	
	sqlite3_finalize(stat);

	//---------------------- restore  Data_CurrentSPFW_kwh 20240801 LPL---------------------//
	for (i = 0; i < sizeof(nNeedaSveSPFWDevMap) / sizeof(nNeedaSveSPFWDevMap[0]); i++)
	{
		memset(szSqlKwh, 0, sizeof(szSqlKwh));
		sprintf(szSqlKwh, "SELECT * FROM Data_CurrentSPFW_kwh WHERE Device_Id = %d ORDER BY Log_Time DESC LIMIT 1;",nNeedaSveSPFWDevMap[i]);
		sqlite3_prepare_v2(data_db, szSqlKwh, -1, &stat, NULL);
		TRACEW("restoreHISeVENT---now  I AM DEV [%d]",nNeedaSveSPFWDevMap[i]);
		if(sqlite3_step(stat) == SQLITE_ROW)
		{
			TRACEW("5--------");
			// EVENT_METER_SAVE_DATA* pMeterSPFWKwhRec = NEW(EVENT_METER_SAVE_DATA, 1); 

			// pMeterSPFWKwhRec->ndevid = sqlite3_column_int(stat, 1); 
			// int nDayReceive = sqlite3_column_int(stat, 2);

			// time_t tt = (time_t)nDayReceive;
			// struct tm gmTime;
			// gmtime_r(&tt, &gmTime);
			// pMeterSPFWKwhRec->nYear = gmTime.tm_year;
			// pMeterSPFWKwhRec->nMonth = gmTime.tm_mon;
			// pMeterSPFWKwhRec->nDay = gmTime.tm_mday;

			// EVENT_ARG arg;
			// ZERO_POBJS(&arg, 1);
			// arg.hSender = (HANDLE)&pCharger->dataprocessor;
			// arg.emEvType = EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW;
			// arg.emMsgType = EVENT_MSG_RESTORE;
			// arg.nEvDataSize = sizeof(EVENT_METER_SAVE_DATA); //use pointer
			// arg.pEvData = (void*)NEW(EVENT_METER_SAVE_DATA, 1);
			// memcpy(arg.pEvData, pMeterSPFWKwhRec, arg.nEvDataSize);
			// int nRet = Queue_Put(pCharger->hqEvent, &arg, TRUE);

			EVENT_METER_SAVE_DATA MeterSPFWKwhRec ; 

			MeterSPFWKwhRec.ndevid = sqlite3_column_int(stat, 1); 
			int nDayReceive = sqlite3_column_int(stat, 2);

			time_t tt = (time_t)nDayReceive;
			struct tm gmTime;
			gmtime_r(&tt, &gmTime);
			MeterSPFWKwhRec.nYear = gmTime.tm_year;
			MeterSPFWKwhRec.nMonth = gmTime.tm_mon;
			MeterSPFWKwhRec.nDay = gmTime.tm_mday;

			EVENT_ARG arg;
			ZERO_POBJS(&arg, 1);
			arg.hSender = (HANDLE)&pCharger->dataprocessor;
			arg.emEvType = EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW;
			arg.emMsgType = EVENT_MSG_RESTORE;
			arg.nEvDataSize = sizeof(EVENT_METER_SAVE_DATA); //use pointer
			arg.pEvData = (void*)NEW(EVENT_METER_SAVE_DATA, 1);
			memcpy(arg.pEvData, &MeterSPFWKwhRec, arg.nEvDataSize);
			int nRet = Queue_Put(pCharger->hqEvent, &arg, TRUE);
			TRACEI("RestoreHisEvent[%d]---nRet=[%d] nDayReceive = [%d] DBLASTyear =[%d] DBLASTmonth =[%d] DBLASTday =[%d] \n", 
				nNeedaSveSPFWDevMap[i],nRet,nDayReceive,MeterSPFWKwhRec.nYear,MeterSPFWKwhRec.nMonth,MeterSPFWKwhRec.nDay);
		}
		sqlite3_finalize(stat);
	}
	//now recompress DB
	//ReCompressData_4OldSWVer(data_db);
}

