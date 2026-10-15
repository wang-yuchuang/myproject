#include "dpr_loader.h"
#include "3rd_party/sqlite/sqlite3.h"
#include "dpr/dpr.h"
#include "dpr/run_expression.h"

static CHARGER* s_pCharger;


static char* GetModuleName(MODULE_INFO* pModual, OUT char* pszName)
{
	switch(pModual->emModuleType)
	{
	case MODULE_TYPE_SAMPLER:
		memcpy(pszName, ((SAMPLER_INFO*)pModual)->szSamplerName, 32);
		break;
	case MODULE_TYPE_REPORTER:
		memcpy(pszName, ((REPORTER_INFO*)pModual)->szReporterName, 32);
		break;
	default:
		memcpy(pszName, ((DATAPROC_INFO*)pModual)->szDataProcName, 32);
		break;
	}
	return pszName;
}

static ERRCDDE_DPR Intern_Get(IN HANDLE hSender,		//which module send this
	IN INTERGET_TYPE emGetTp, 
	IN int nDevId, 
	IN int nParamId,
	IN OUT VAR_VALUE* pBuff,
	IN int nBuffSize)
{
	PARAMETER* pParam;
	ALARM* pAlarm;
	//SAMPLER* pSampler = (SAMPLER*)hSender;
	MODULE_INFO* pModual = (MODULE_INFO*)hSender;
	char szName[32];
	int n;

	if((pModual == NULL) || (pBuff == NULL))
	{
		return ERR_DPR_INVALID_ARG;
	}

	if(emGetTp == GETTYPE_APPVER)
	{
		if(nBuffSize < MAX_LEN_MU2SWVER)
		{
			return ERR_DPR_WRONG_SIZE;
		}
		memcpy(pBuff, s_pCharger->info.szSWVer, MAX_LEN_MU2SWVER);
		return ERR_DPR_OK;
	}
	else if(emGetTp == GETTYPE_COMMPORT)
	{
		TRACE("\n---Received GetCommport:{%d}, nBuffSize = %d---\n", nDevId, nBuffSize);

		if(nBuffSize != sizeof(COMMPORT))
		{
			return ERR_DPR_WRONG_SIZE;
		}
		COMMPORT* pPort = s_pCharger->commports.p;
		
		for(n = 0; n < s_pCharger->commports.nTotal; n++,pPort++)
		{
			if(nDevId == pPort->info.nCommPort)
			{
				//pBuff = (VAR_VALUE*)pPort;
				memcpy(pBuff, pPort, sizeof(COMMPORT));
				return ERR_DPR_OK;
			}
		}

		return ERR_DPR_INVALID_ARG;
	}
	else if(emGetTp == GETTYPE_METERVALUE_REC)
	{
		if(nBuffSize != sizeof(HIS_METER_VALUE_REC))
		{
			return ERR_DPR_WRONG_SIZE;
		}

		HIS_METER_VALUE_REC rec;
		char szSql[256];
		sqlite3_stmt * stat;

		sprintf(szSql, "SELECT * FROM Data_ChargeMeterValues where Gun_Idx = %d and Charge_Begin_Time = %d", 
			nDevId, nParamId);

		sqlite3 *db = Open_Lock_SqliteDB();

		sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

		int nSqlRet;
		if((nSqlRet = sqlite3_step(stat)) == SQLITE_ROW)
		{
			rec.nGunNum = sqlite3_column_int(stat, 1);
			rec.tmChargeStart = sqlite3_column_int(stat, 2);
			//memcpy(&rec.meterData, sqlite3_column_blob(stat, 3), 
			//	MIN(sizeof(CHARGE_METERITEM_LIST), sqlite3_column_bytes(stat, 3)));
			sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&rec.meterData, sizeof(CHARGE_METERITEM_LIST));
			memcpy(pBuff->blobValue, &rec, sizeof(HIS_METER_VALUE_REC));
		}
		else
		{
			sqlite3_finalize(stat);
			Close_UnLock_SqliteDB();
			if(nSqlRet != SQLITE_DONE)
			{
				//TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg((sqlite3*)s_pCharger->hRunDB));
				LogOut(MAIN_MODULE_DPR, "GET", LOG_TYPE_ERROR, "Get charge MeterValues rec FAILED!");
				//system("reboot");
			}
			return ERR_DPR_NO_DATA;
		}
		
		sqlite3_finalize(stat);
		Close_UnLock_SqliteDB();
		return ERR_DPR_OK;
	}
	else if(emGetTp == GETTYPE_ALMSTATE)
	{
		pAlarm = GetAlarm_ByPR(nDevId, nParamId);
		if(pAlarm == NULL)
		{
			TRACE("[%s], invalid InternGet Alarm PTR: [%d, %d, %d]", GetModuleName(pModual, szName),
				emGetTp, nDevId, nParamId);
			return ERR_DPR_INVALID_ARG;
		}
		pBuff->emValue = pAlarm->emStatus;
		return ERR_DPR_OK;
	}
	else if(emGetTp == GETTYPE_READBACK_VAR)
	{
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nDevId, nParamId);
		VAR_VALUE* pVarV = &pParam->ReadValue;
		
		memcpy(pBuff->abyValue, (void*)pVarV->abyValue, 4);
		return ERR_DPR_OK;
	}
	else if((emGetTp == GETTYPE_SP_RAW_VAL) ||
		(emGetTp == GETTYPE_SP_VAR))
	{
		pParam = GetParam_ByPR(PARAM_TYPE_STATUS, nDevId, nParamId);
	}
	else if(emGetTp == GETTYPE_CL_VAR)
	{
		pParam = GetParam_ByPR(PARAM_TYPE_CONTROL, nDevId, nParamId);
	}
	else
	{
		// GETTYPE_ST_VAR
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nDevId, nParamId);
	}
	if(pParam == NULL)
	{
		TRACE("[%s], invalid InternGet Param PTR: [%d, %d, %d]", GetModuleName(pModual, szName),
			emGetTp, nDevId, nParamId);
		return ERR_DPR_INVALID_ARG;
	}

	VAR_VALUE* pVarV = &pParam->currValue;
	//if((emGetTp == GETTYPE_SP_VAR) && (pParam->bIsValueInvalid))
	//{
	//	return ERR_DPR_INVALID_ARG; //invalid value can not be gotten
	//}

	int nValSize = pParam->pParamInfo->nValueSize;

	if(nValSize > nBuffSize) //buffer size can be larger but not smaller
	{
		TRACEX("Wrong param size for[%d, %d, %d], nValsize:%d, nBuffSize:%d\n", 
			emGetTp, nDevId, nParamId, nValSize, nBuffSize);
		return ERR_DPR_WRONG_SIZE;
	}

	if(/*(emGetTp == GETTYPE_SP_RAW_VAL) &&*/ (pParam->bIsValueInvalid))
	{
		if(pParam->pParamInfo->emValueType != VALUE_TYPE_BLOB)
		{
			pVarV = &pParam->lastValue;
		}
	}

	if((pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB) ||
		(pParam->pParamInfo->emValueType == VALUE_TYPE_STRING))
	{
		//TRACE("[%s], InternGet Param, [%d, %d, %d], size[%d-->%d], isValInvalid = %d, pCurrSzVal = [%s]\n", GetModuleName(pModual, szName),
		//	emGetTp, nDevId, nParamId, nBuffSize, nValSize, pParam->bIsValueInvalid, (char*)pVarV->blobValue);
		memcpy(pBuff->blobValue, (void*)pVarV->blobValue, nValSize);
	}
	else
	{
		memcpy(pBuff->abyValue, (void*)pVarV->abyValue, 4);
	}

	return ERR_DPR_OK;
}

static BOOL Save_SettingParam(int nDevId, int nParamId, BYTE* pbyVal, int nValSize, sqlite3* data_db)
{
	sqlite3_stmt * stat;
	int nSqlRet = -1;
	const char* sql = "INSERT INTO Data_SavedSettings (Device_Id, Param_Id, Save_Time, Current_Value) "
                      "VALUES (?, ?, ?, ?) "
                      "ON CONFLICT(Device_Id, Param_Id) DO UPDATE SET "
                      "Save_Time = excluded.Save_Time, "
                      "Current_Value = excluded.Current_Value;";       //防止Replace覆盖掉上传的回读值

    // 预处理SQL语句
    nSqlRet = sqlite3_prepare_v2(data_db, sql, -1, &stat, NULL);
    if (nSqlRet != SQLITE_OK)
	{
        TRACEX("Save_SettingParam prepare failed: errCode=%d, errMsg=[%s]\n", 
               nSqlRet, sqlite3_errmsg(data_db));
        return FALSE;
    }	sqlite3_bind_int(stat, 1, nDevId);
	sqlite3_bind_int(stat, 2, nParamId);
	sqlite3_bind_int(stat, 3, _NOW_);
	//For LocalList and LocalCache, we need handle it specially
	if(/*(nParamId == 6) || (nParamId == 7)*/0)
	{
		TRACE("------Now add compressed %s into SavedSetting table of DB----\n", 
			(nParamId == 6) ? "LocalList" : "LocalCache");
		void* pOut;
		sqlite3_bind_blob_compress(stat, 4, pbyVal, nValSize, &pOut, NULL);
		nSqlRet = sqlite3_step(stat);
		sqlite3_finalize_compressed(stat, pOut);//delete stat
	}
	else
	{
		sqlite3_bind_blob(stat, 4, pbyVal, nValSize, NULL);
		nSqlRet = sqlite3_step(stat);
		sqlite3_finalize(stat);//delete stat
	}
	
	if(nSqlRet != SQLITE_DONE)
	{
		TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", sql, 
			nSqlRet, sqlite3_errmsg(data_db));
		LogOut_A(MAIN_MODULE_DPR, "SAVE SET", LOG_TYPE_ERROR, "CAN NOT save the param[%d,%d] into DB! errCode=%d", nDevId, nParamId, nSqlRet);
		//system("reboot");
		return FALSE;
	}

	if((nParamId != 504)&&(nParamId != 505)&&(nParamId != 204)&&(nParamId != 205)&&(nParamId != 206)&&(nParamId != 207)&&(nParamId != 208))
	{
		LogOut_A(MAIN_MODULE_DPR, "SAVE SET", LOG_TYPE_INFO, "Successfully saved the param[%d,%d] into DB!", nDevId, nParamId);
	}
	return TRUE;
	//TRACE("\n-----Save setting. lasterr= %s-------\n", sqlite3_errmsg(data_db));
}

// 更新单个模块的回读值
static BOOL Update_RereadData(int nDevId, int nParamId, BYTE* pbyVal, int nValSize, int nWhichModule, sqlite3* data_db)
{
    if (data_db == NULL || 
        ((pbyVal == NULL && nValSize != 0) || (pbyVal != NULL && nValSize <= 0)) || 
        nWhichModule < 0 || nWhichModule > 7)
    {
        TRACEX("Invalid parameter! nDevId=%d, nParamId=%d, pbyVal=%p, nValSize=%d, nWhichModule=%d\n",
                nDevId, nParamId, pbyVal, nValSize, nWhichModule);
        return FALSE;
    }

    sqlite3_stmt *stat = NULL;
    int nSqlRet = -1;
    char sql[512] = {0};

    snprintf(sql, sizeof(sql),
            "UPDATE Data_SavedSettings SET RereadValue_SingleModule_%d = ?, Save_Time = ? WHERE Device_Id = ? AND Param_Id = ?",
            nWhichModule + 1);

    nSqlRet = sqlite3_prepare_v2(data_db, sql, -1, &stat, NULL);
    if (nSqlRet != SQLITE_OK)
    {
        TRACEX("Prepare SQL failed! errCode=%d, errMsg=%s, SQL=%s\n",
                nSqlRet, sqlite3_errmsg(data_db), sql);
        return FALSE;
    }

    if (pbyVal == NULL)
    {
        sqlite3_bind_null(stat, 1);
    }
    else
    {
        nSqlRet = sqlite3_bind_blob(stat, 1, pbyVal, nValSize, SQLITE_TRANSIENT);
        if (nSqlRet != SQLITE_OK)
        {
            TRACEX("Bind BLOB failed! errCode=%d, errMsg=%s\n",
                    nSqlRet, sqlite3_errmsg(data_db));
            sqlite3_finalize(stat);
            return FALSE;
        }
    }

    sqlite3_bind_int(stat, 2, _NOW_);    // 时间
    sqlite3_bind_int(stat, 3, nDevId);   // 设备ID
    sqlite3_bind_int(stat, 4, nParamId); // 参数ID

    nSqlRet = sqlite3_step(stat);
    int nChangedRows = sqlite3_changes(data_db);

    sqlite3_finalize(stat);

    if (nSqlRet != SQLITE_DONE)
    {
        TRACEX("Execute SQL failed! errCode=%d, errMsg=%s, SQL=%s\n",
                nSqlRet, sqlite3_errmsg(data_db), sql);
        return FALSE;
    }

    if (nChangedRows == 0)
    {
        return TRUE;
    }

    TRACEX("[INFO] Update success! nDevId=%d, nParamId=%d, module=%d, changed rows=%d\n",
           nDevId, nParamId, nWhichModule + 1, nChangedRows);
    return TRUE;
}

static BOOL IsSettVal_SameAsOld(PARAMETER* pParam, VAR_VALUE valSet)
{
	if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) ||
		(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
	{
		//can not use char compare!!!!!!
		//return (strncmp(pParam->currValue.blobValue, valSet.blobValue, pParam->pParamInfo->nValueSize) == 0);
		int n;
		for(n = 0; n < pParam->pParamInfo->nValueSize; n++)
		{
			if(pParam->currValue.blobValue[n] != valSet.blobValue[n])
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	else
	{
		return (pParam->currValue.ulValue == valSet.ulValue);
	}
}



static UINT GetTotal_SettParam_Number(void)
{
	UINT nTotal = 0;
	int n;
	DEVICE * pDevice = s_pCharger->devices.p;

	for(n = 0; n < s_pCharger->devices.nTotal; n++, pDevice++)
	{
		nTotal += pDevice->params_Setting.nTotal;
	}

	return nTotal;
}

#define MAX_DORESTORE_FROMDB		1024
static void AddParam2_RestoredList(int nParamId, OUT int *pnRestortedParamLst, OUT int* pnRestoredParamTotal)
{
	if((pnRestortedParamLst == NULL) || 
		(pnRestoredParamTotal == NULL))
	{
		return;
	}
	//now count++
	pnRestortedParamLst[*pnRestoredParamTotal] = nParamId;
	*pnRestoredParamTotal += 1;
	if(*pnRestoredParamTotal >= MAX_DORESTORE_FROMDB)
	{
		*pnRestoredParamTotal = MAX_DORESTORE_FROMDB - 1;
	}
}

static int Do_SettParam_Restoring(BOOL bIsFromWeb, BOOL bIsClone, OUT int *pnRestortedParamLst, OUT int* pnRestoredParamTotal)
{
	char* szErrMsg = NULL;
	if(pnRestoredParamTotal != NULL)
	{
		*pnRestoredParamTotal = 0;	
	}	
	TRACEX("Now begin restoring param for [%s]\n", bIsClone? "CLONE" : "NORMAL");
	sqlite3* data_db = Open_Lock_SqliteDB();//(sqlite3*)s_pCharger->hRunDB;
	if(bIsFromWeb && (access(SYSDB_WEB_IMPORT_FILENAME, F_OK) == 0))
	{
		char szSql[128];
		snprintf(szSql, 128, "ATTACH '%s' AS Backup", SYSDB_WEB_IMPORT_FILENAME);
		sqlite3_exec(data_db, szSql, NULL, NULL, &szErrMsg);
	}
	// else if((access("/media/mmcblk0p1/nccu.data", F_OK) == 0) && (!bIsFromWeb)) 
	// {
	// 	sqlite3_exec(data_db,"ATTACH '/media/mmcblk0p1/nccu.data' AS Backup", NULL, NULL, &szErrMsg);
	// }
	else if(!bIsFromWeb)
	{
		char szPath[64] = {0};			
		if(FindFilePath_In_USB_OrSD_MountedDir(TRUE, "nccu.data", szPath, 64) == NULL)
		{
			if(FindFilePath_In_USB_OrSD_MountedDir(FALSE, "nccu.data", szPath, 64) == NULL)
			{
				Close_UnLock_SqliteDB();
				return ERR_DPR_INVALID_ARG;
			}		
		}
		char szSql[128];
		snprintf(szSql, 128, "ATTACH '%s/nccu.data' AS Backup", szPath);
		sqlite3_exec(data_db, szSql, NULL, NULL, &szErrMsg);
	}
	//Jimmy: temporyly disabled the USB disk 20200820
	//Jimmy 20200722: diabled this temproyaly due to kernel problem
	// else if(access("/dev/sda1", F_OK) == 0) 
	// {
	// 	sqlite3_exec(data_db,"ATTACH '/media/sda1/nccu.data' AS Backup", NULL, NULL, &szErrMsg);
	// }
	else
	{
		TRACE("\nCan not attach Backup DB because NO USB dev found!\n");
		Close_UnLock_SqliteDB();
		return ERR_DPR_INVALID_ARG;
	}
		
	if(szErrMsg != NULL)
	{
		TRACE("Can not attach Backup DB, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		Close_UnLock_SqliteDB();
		return ERR_DPR_INVALID_ARG;
	}

	sqlite3_stmt * stat;
	sqlite3_prepare_v2(data_db, "SELECT * FROM Backup.Data_SavedSettings", -1, &stat, NULL);
	BOOL bRestoreOK = TRUE;
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		int nDevId = sqlite3_column_int(stat, 0);
		int nParamId = sqlite3_column_int(stat, 1);
		TRACEX("--Begin restoring for Param[%d]\n", nParamId);
		PARAMETER* pParam = NULL;
		VAR_VALUE varV;
		if((pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nDevId, nParamId)) == NULL)
		{
			TRACEX("OVERVVVV: Found an INVALID setting param[%d, %d]!", nDevId, nParamId);
			continue;
		}

		if(bIsClone)
		{
			//only reset the setting for CCU/network settings currently
			if((nParamId != 200)/* && (nParamId != 3)*/) //network should be skipped for web
			{
				TRACE("Now skipEEEEE the parameter[%d]\n", nParamId);
				AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
				continue;
			}		
		}
		else
		{
			//Jimmy revised 20200615 to judge it using parameter attribute
			// if((nParamId == 401) || (nParamId == 402) || (nParamId == 403) || (nParamId == 404) || (nParamId == 405) || 
			// 	(nParamId == 13) || (nParamId == 17) ||  (nParamId == 200) || (nParamId == 301)) //do not restore Meter Settings
			// {
			// 	continue;
			// }
			//Jimmy revised 20200615: 11 means for root account only
			//do not handle the network
			//else 
			if((pParam->pParamInfo->uMask_AuthorityLevel >= 11) || (nParamId == 3))
			{
				TRACE("Now skipFFFFF the parameter[%d]\n", nParamId);
				AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
				continue;
			}
		}
		

			//value and value length
		time_t tmSet = (time_t)sqlite3_column_int(stat, 2);
		void* pData = sqlite3_column_blob(stat, 3);
		int nVaLen = sqlite3_column_bytes(stat, 3);
		if((pParam->pParamInfo->emValueType != VALUE_TYPE_STRING) &&
			(pParam->pParamInfo->emValueType != VALUE_TYPE_BLOB) &&
			(nVaLen > 0))
		{
			memcpy(varV.abyValue, pData, MIN(nVaLen, 4));
		}
		else
		{
			varV.blobValue = pData;
		}

		//now check the param
		int nErr = Check_ParamValue(pParam, &varV, nVaLen, TRUE);
		if(nErr != ERR_PARAMVAL_OK)
		{
			if(nErr == ERR_PARAMVAL_OK_BUT_HAS_EXEPT)
			{
				LogOut_A(MAIN_MODULE_DPR, bIsClone ? "OverRestorFromDB" : "RestoreFromDB",
					 LOG_TYPE_WARING,
					"Param[%d, %d] value size is Accepted but from old SW!", nDevId, nParamId);
			}
			else
			{
				LogOut_A(MAIN_MODULE_DPR, bIsClone ? "OverRestorFromDB" : "RestoreFromDB",
					 LOG_TYPE_WARING,
					"Param[%d, %d] value %s is INVALID!", nDevId, nParamId,
					(nErr == ERR_PARAMVAL_SIZE) ? "size" : "range");
				AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
				continue;
			}
		}
		else if(IsSettVal_SameAsOld(pParam, varV)) //same value, no need to update
		{
			//still need add one
			TRACE("\n-----SSSSSAMEVAL for parm[%d]-------\n", nParamId);
			AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
			continue;
		}
		DEVICE* pDev = GetDevce_ById(nDevId);
		if(pDev == NULL)
		{
			TRACEX("can not find such device[%d]\n", nDevId);
			AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
			continue;
		}
		SETTING_PARAM_INFO* pSettParam = (SETTING_PARAM_INFO *)pParam->pParamInfo;
		
		BOOL bIsNeedNew = FALSE;
		if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) || 
			(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
		{
			TRACEX("--Now renew the Param for Blob---\n");
			//BYTE* pNew = NEW(BYTE, nVaLen);	
			BYTE* pNew = NEW(BYTE, pParam->pParamInfo->nValueSize);				
			//the pointer will be destroyed by sampler
			if(pNew == NULL)
			{
				bRestoreOK = FALSE; 
				AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
				continue;
			}
			memcpy(pNew, varV.blobValue, MIN(nVaLen, pParam->pParamInfo->nValueSize));
			varV.blobValue = pNew;
			bIsNeedNew = TRUE;
		}
		
		if((pSettParam->bNeedSyncSampler) && (pDev->info.pBelonged_Sampler->Control != NULL))
		{
			TRACEX("Now begin run CONTROL for Backup Set[%d, %d]\n", nDevId, nParamId);
			// INTER_QUEUE_CMD cmd;
			// cmd.emType = SETTYPE_INTERN_SETT;
			// cmd.data.nDevId = nDevId;
			// cmd.data.nParamId = nParamId;
			// cmd.data.nValueSize = nVaLen;
			// cmd.data.varValue = varV;
			// if(ERR_QUEUE_OK != Queue_Put(pDev->info.pBelonged_Sampler->hqControlCommand, 
			// 	&cmd, FALSE))
			// {
			// 	continue;
			// }
			//sync to sampler directly
			ERRCODE_SAMPLER errCode = pDev->info.pBelonged_Sampler->Control(
				(HANDLE)pDev->info.pBelonged_Sampler,
				PARAM_TYPE_SETTING, 
				nDevId, nParamId,
				&varV, pParam->pParamInfo->nValueSize,
				5000);
			if(ERR_CONTROL_OK != errCode)
			{
				TRACE("\n-----ERRRRRRRRRR for control-------\n");
				LogOut_A(MAIN_MODULE_DPR, bIsClone ? "OverRestorFromDB" : "RestoreFromDB",
					 LOG_TYPE_ERROR,
					"Param[%d, %d] value Overwrite ERROR, we skip it!",
					 nDevId, nParamId);
				bRestoreOK = FALSE; 
				if(bIsNeedNew)
				{
					DELETE(varV.blobValue);
				}

				//still need to add 
				AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
				//break;
				continue; //Jimmy note, even re-write error, we will still continue it
				//return ERR_DPR_TIMEOUT; //only timeout?
			}

			TRACEX("Now EXE the overwrite for Set[%d, %d] successfully\n", nDevId, nParamId);
		}
		//Jimmy: need to handle the copy process here
		//else
		{
			if(!bIsClone)
			{
				TRACEX("Now copy NEW value from Backup[%d, %d]\n", nDevId, nParamId);
				//now set value
				Dump_ParamValue(pParam, &varV, nVaLen, tmSet);
				if(pSettParam->bNeedSave)
				{
					BYTE* pbySetData = varV.abyValue;
					if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) || 
						(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
					{
						pbySetData = varV.blobValue;
					}
					Save_SettingParam(nDevId, 
						nParamId,
						pbySetData,
						nVaLen, data_db);
				}
			}
		}
		
		if(bIsNeedNew)
		{
			DELETE(varV.blobValue);
		}

		AddParam2_RestoredList(nParamId, pnRestortedParamLst, pnRestoredParamTotal);
	}
	sqlite3_finalize(stat);
	//do not forget to detach the DB
	sqlite3_exec(data_db,"DETACH Backup", NULL, NULL, &szErrMsg);
	Close_UnLock_SqliteDB();
	TRACEX("NOW DETACH the BackUP DB [%s] \n", (szErrMsg == NULL) ? "Successfully" : szErrMsg);
	//for DB error, we can continue
	return bRestoreOK ? ERR_DPR_OK : ERR_DPR_DB_ERR;
}

static BOOL IsFoundParam_FromIdlst(int nParamId, int *pRestoredParLst, int nRestoreParmTotal)
{
	int n;
	for(n = 0; n < nRestoreParmTotal; n++)
	{
		if(pRestoredParLst[n] == nParamId)
		{
			return TRUE;
		}
	}

	return FALSE;
}
static ERRCDDE_DPR Intern_Set(IN HANDLE hSender,		//which sampler send this
	IN INTERSET_TYPE emSetTp, 
	IN OUT void* pBuff,
	IN int nBuffSize)
{
	MODULE_INFO* pModual = (MODULE_INFO*)hSender;
	int nCheckRet;
	char szName[32];
	if((pModual == NULL) || ((pBuff == NULL) && 
		(emSetTp != SETTYPE_RESTORE_SETTINGS_FROMDB) && 
		(emSetTp != SETTYPE_CLEAR_FACTOY_DEBUG_DATA) &&
		(emSetTp != SETTYPE_OVERWRITE_SET_BEFORE_CLONE)))
	{
		return ERR_DPR_INVALID_ARG;
	}

	if(emSetTp == SETTYPE_CLEAR_FACTOY_DEBUG_DATA)
	{
		sqlite3* data_db = Open_Lock_SqliteDB();//(sqlite3*)s_pCharger->hRunDB;
		//Jimmy note: just assume no Failure for deleting
		sqlite3_exec(data_db,"DELETE FROM Data_BMSHealthData", NULL, NULL, NULL);
		sqlite3_exec(data_db,"DELETE FROM Data_RealTimeBmsData", NULL, NULL, NULL);
		sqlite3_exec(data_db,"DELETE FROM Data_ESStats_Daily", NULL, NULL, NULL);
		sqlite3_exec(data_db,"DELETE FROM Data_Alarms", NULL, NULL, NULL);
		Close_UnLock_SqliteDB();
		return ERR_DPR_OK;
	}

	if(emSetTp == SETTYPE_RESTORE_SETTINGS_FROMDB)
	{
		//int *pnRestoredParamIdLst = NEW(int , GetTotal_SettParam_Number());
		int nSettParamTotal = GetTotal_SettParam_Number();
		int nRestoredParamIdLst[MAX_DORESTORE_FROMDB];//max 1024
		int nRestoredParamsCount = 0;
		int nDoRestoreResult = Do_SettParam_Restoring((nBuffSize == 99) ? TRUE : FALSE,
			FALSE, nRestoredParamIdLst, &nRestoredParamsCount);
		TRACEX("The restored sett paramterTotal=%d, nDoRestoreResult=%d\n",
			 nRestoredParamsCount, nDoRestoreResult);
		if(nRestoredParamsCount != ERR_DPR_INVALID_ARG)		
		{			
			int kk, mm;
			DEVICE * pDevice = s_pCharger->devices.p;
			sqlite3* data_db = Open_Lock_SqliteDB();//(sqlite3*)s_pCharger->hRunDB;
			sqlite3_exec(data_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);	
			char szSql[64];
			for(kk = 0; kk < s_pCharger->devices.nTotal; kk++, pDevice++)
			{
				PARAMETER* pParam = pDevice->params_Setting.p;
				for(mm = 0; mm < pDevice->params_Setting.nTotal; mm++, pParam++)
				{
					if(!IsFoundParam_FromIdlst(pParam->pParamInfo->nParamId, nRestoredParamIdLst, nRestoredParamsCount))
					{
						TRACEX("Now revert back the values for set-param[%d]\n", pParam->pParamInfo->nParamId);
						snprintf(szSql, 64, "delete from Data_SavedSettings where Param_Id = %d;", 
							pParam->pParamInfo->nParamId);
						sqlite3_exec(data_db, szSql, NULL, NULL, NULL);	
					}
				}
			}
			sqlite3_exec(data_db, "COMMIT;", NULL, NULL, NULL);	
			Close_UnLock_SqliteDB();			
		}
		return nDoRestoreResult;
	}
	//added by Jimmy 20200613 only for overwrite the current setting which needs to sync to samplers
	//for 'data-clone' function
	else if(emSetTp == SETTYPE_OVERWRITE_SET_BEFORE_CLONE)
	{
		return Do_SettParam_Restoring((nBuffSize == 99) ? TRUE : FALSE, TRUE, NULL, NULL);
	}
	
	if(emSetTp != SETTYPE_SENDMSG)
	{
		INTERSET_DATA *pSetData = (INTERSET_DATA*)pBuff;
		if(nBuffSize != sizeof(INTERSET_DATA))
		{
			TRACE("invalid arg\n");
			return ERR_DPR_INVALID_ARG;
		}
		DEVICE* pDev = GetDevce_ById(pSetData->nDevId);
		if(pDev == NULL)
		{
			TRACE("can not find such device[%d]\n",pSetData->nDevId);
			return ERR_DPR_INVALID_ARG;
		}
		PARAMETER* pParam = NULL;
		int nValueSize = 4;
		STATUS_PARAM_INFO *pStatParam = NULL;
		SETTING_PARAM_INFO *pSettParam = NULL;
		switch(emSetTp)
		{
		case SETTYPE_RAW_SP:
		case SETTYPE_LOGIC_SP:
			pParam = GetParam_ByPR(PARAM_TYPE_STATUS, pSetData->nDevId, pSetData->nParamId);

			if(pParam == NULL)
			{
				TRACE("can not find such param\n");
				return ERR_DPR_INVALID_ARG;
			}
			if(SETTYPE_LOGIC_SP == emSetTp)
			{
				pStatParam = (STATUS_PARAM_INFO *)pParam->pParamInfo;
				if(pStatParam->emDataSrc != PARAM_SRC_LS)
				{
					return ERR_DPR_INVALID_ARG;
				}
			}
			nValueSize = pParam->pParamInfo->nValueSize;
			//if(pParam->pParamInfo->emValueType == VALUE_TYPE_ENUM)
			//{
			//	nValueSize = 4;
			//}

		
			if(pSetData->bRawInvalid)
			{
				pParam->bIsValueInvalid = TRUE;
			}
			else if(Check_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, FALSE) != ERR_PARAMVAL_OK)
			{
				pParam->bIsValueInvalid = TRUE;
			}
			else
			{
				if(nValueSize != pSetData->nValueSize)
				{
					return ERR_DPR_WRONG_SIZE;
				}
				pParam->bIsValueInvalid = FALSE;
				//memcpy(&pParam->currValue, &pSetData->varValue, nValueSize);
				Dump_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);
			}

			//TRACEX("Set RawVal[%d,%d], bValid = %d, fVal = %.3f. pParam->bIsValueInvalid = %d\n", 
			//	pSetData->nDevId, pSetData->nParamId, 
			//	pSetData->bRawInvalid, pSetData->varValue.fValue, pParam->bIsValueInvalid);
			break;
		case SETTYPE_INTERN_CTRL:
		case SETTYPE_RAW_BACK_CTRL:
			pParam = GetParam_ByPR(PARAM_TYPE_CONTROL, pSetData->nDevId, pSetData->nParamId);
			TRACE("[%s], Ctrl Param, [%d, %d], size[%d-->%d]\n", GetModuleName(pModual, szName),
				pSetData->nDevId, pSetData->nParamId, nBuffSize, pSetData->nValueSize);
			if(pParam == NULL)
			{
				TRACE("--ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR1111111--\n");
				return ERR_DPR_INVALID_ARG;
			}
			else if(Check_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, FALSE) != ERR_PARAMVAL_OK)
			{
				TRACE("--ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR222222--\n");
				return ERR_DPR_INVALID_ARG;
			}
			else if(pDev->info.pBelonged_Sampler->emWorkStatus != SAMPLER_IS_WORK)
			{
				TRACE("--ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR33333--\n");
				return ERR_DPR_SAMPLER_COMMFAIL;
			}
			else if(emSetTp == SETTYPE_RAW_BACK_CTRL)
			{
				//ctrl param also dump data
				Dump_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);
				return ERR_DPR_OK;
			}
			else if((pModual->emModuleType == MODULE_TYPE_SAMPLER) && (pDev->info.pBelonged_Sampler->info.nSamplerID == pModual->nModualId))
			{
				//TRACEX("Issued by the same module, no need sync\n");
				//return ERR_DPR_OK;
				pSetData->nTimeOut = 0; //Jimmy changed 20211105: if issued by same sampler, shall execute it later in Q
			}
			if(pDev->info.pBelonged_Sampler->Control != NULL)
			{
				if(pSetData->nTimeOut != 0)
				{
					//sync to sampler directly
					double fTimeBeforeExec = GetCurrentTime();
					ERRCODE_SAMPLER errCode = pDev->info.pBelonged_Sampler->Control(
						(HANDLE)pDev->info.pBelonged_Sampler,
						PARAM_TYPE_CONTROL, 
						pSetData->nDevId, 
						pSetData->nParamId,
						&pSetData->varValue,
						pSetData->nValueSize,
						pSetData->nTimeOut);

					if(ERR_CONTROL_NOTSUPPORTED == errCode)
					{
						return ERR_DPR_NOTSUPPORTED;
					}
					else if(ERR_CONTROL_OK != errCode)
					{
						LogOut(MAIN_MODULE_DPR, "User Ctrl", LOG_TYPE_ERROR, 
							"Executed the param[%d,%d]->%d FAILED[errcode: %d],nTimeOut[%d],tmCost[%.3f]!", 
							pSetData->nDevId, pSetData->nParamId,
							pSetData->varValue.emValue, errCode, pSetData->nTimeOut,
							GetCurrentTime() - fTimeBeforeExec);
						return ERR_DPR_TIMEOUT; //only timeout?
					}
					else
					{
						LogOut(MAIN_MODULE_DPR, "User Ctrl", LOG_TYPE_INFO, 
							"Executed the param[%d,%d]->%d Successfully!", 
							pSetData->nDevId, pSetData->nParamId, pSetData->varValue.emValue);
					}
				}
				else
				{
					INTER_QUEUE_CMD cmd;
					cmd.emType = emSetTp;
					memcpy(&cmd.data, pSetData, sizeof(INTERSET_DATA));
					if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) || 
						(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
					{
						BYTE* pNew = NEW(BYTE, pSetData->nValueSize);
						memcpy(pNew, pSetData->varValue.blobValue, pSetData->nValueSize);
						cmd.data.varValue.blobValue = pNew;
						//the pointer will be destroyed by sampler
						if(cmd.data.varValue.blobValue == NULL)
						{
							return ERR_DPR_NO_MEMORY;
						}
					}
					if(ERR_QUEUE_OK != Queue_Put(pDev->info.pBelonged_Sampler->hqControlCommand, 
						&cmd, FALSE))
					{
						return ERR_DPR_QUEUE_FULL;
					}

					// LogOut(MAIN_MODULE_DPR, "Ctrl Q", LOG_TYPE_INFO, 
					// 		"Issued cmd[%d,%d]->%d been put 2Q and it will be executed later!", 
					// 		pSetData->nDevId, pSetData->nParamId, pSetData->varValue.emValue);

					//execute later. and will be sync by sampler itself
					return ERR_DPR_OK;
				}
			}

			//ctrl param also dump data
			Dump_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);
			break;
		case SETTYPE_READ_VALUE:
			pParam = GetParam_ByPR(PARAM_TYPE_SETTING, pSetData->nDevId, pSetData->nParamId);
			if(pParam == NULL)
			{
				TRACE("Cannot find suchppppp : [%d,%d]\n",pSetData->nDevId, pSetData->nParamId);
				return ERR_DPR_INVALID_ARG;
			}
			nCheckRet = Check_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, FALSE);
			if(nCheckRet != ERR_PARAMVAL_OK)
			{
				TRACE("CheckParamFail : %d\n",nCheckRet);
				return ERR_DPR_INVALID_ARG;
			}
			pSettParam = (SETTING_PARAM_INFO *)pParam->pParamInfo;

			Dump_ReadParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);
			
			break;
		case SETTYPE_INTERN_SETT:
		case SETTYPE_RAW_ST:
		case SETTYPE_RAW_BACK_SETT:
		case SETTYPE_Reread_PCS_Param:
		default:
			pParam = GetParam_ByPR(PARAM_TYPE_SETTING, pSetData->nDevId, pSetData->nParamId);
			if(pParam == NULL)
			{
				TRACE("Cannot find suchppppp : [%d,%d]\n",pSetData->nDevId, pSetData->nParamId);
				return ERR_DPR_INVALID_ARG;
			}
			nCheckRet = Check_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, FALSE);
			if(nCheckRet != ERR_PARAMVAL_OK)
			{
				TRACE("CheckParamFail : %d\n",nCheckRet);
				return ERR_DPR_INVALID_ARG;
			}
			else if(IsSettVal_SameAsOld(pParam, pSetData->varValue)) //same value, no need to update
			{
				//John 2023-12-13 Added. Special Process CCU ABU PCS#1~#4 Sett.
				if((pSetData->nDevId != 200) &&
					(pSetData->nDevId != 700) &&
					(pSetData->nDevId != 701) &&
					(pSetData->nDevId != 702) &&
					(pSetData->nDevId != 703) &&
					(pSetData->nDevId != 704) &&
					(pSetData->nDevId != 705)&&
					(pSetData->nDevId != 500))
				{
					TRACE("SettVal is same as old pSetData->nDevId=[%d]\n", pSetData->nDevId);
					return ERR_DPR_OK;
				}
				else{
					//TRACE("Special Dev same as old pSetData->nDevId=[%d]\n", pSetData->nDevId);
				}
			}

			pSettParam = (SETTING_PARAM_INFO *)pParam->pParamInfo;

			if(SETTYPE_INTERN_SETT != emSetTp && SETTYPE_Reread_PCS_Param != emSetTp)
			{
				//just refresh value
				Dump_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);
			}
			else
			{
				TRACE("--------Run hearrrrr, bNeedSync = %d, bNeedSave = %d---------\n", pSettParam->bNeedSyncSampler, pSettParam->bNeedSave);
				if(pSettParam->bNeedSyncSampler)
				{
					if((pModual->emModuleType == MODULE_TYPE_SAMPLER) && (pDev->info.pBelonged_Sampler->info.nSamplerID == pModual->nModualId))
					{
						TRACE("Issued by the same module, no need sync\n");
						//return ERR_DPR_OK;
					}
					else if(pDev->info.pBelonged_Sampler->emWorkStatus != SAMPLER_IS_WORK)
					{
						//return ERR_DPR_SAMPLER_COMMFAIL;
					}
					else if(pDev->info.pBelonged_Sampler->Control != NULL)
					{
						if(pSetData->nTimeOut != 0)
						{
							//sync to sampler directly
							double fTimeBeforeExec = GetCurrentTime();
							ERRCODE_SAMPLER errCode = pDev->info.pBelonged_Sampler->Control(
								(HANDLE)pDev->info.pBelonged_Sampler,
								PARAM_TYPE_SETTING, 
								pSetData->nDevId, 
								pSetData->nParamId,
								&pSetData->varValue,
								pSetData->nValueSize,
								pSetData->nTimeOut);
							if(ERR_CONTROL_OK != errCode)
							{
								LogOut(MAIN_MODULE_DPR, "User Set", LOG_TYPE_ERROR, 
									"Executed the param[%d,%d]->%d FAILED[errcode: %d],nTimeOut[%d],tmCost[%.3f]!", 
									pSetData->nDevId, pSetData->nParamId,
									pSetData->varValue.emValue, errCode, pSetData->nTimeOut,
									GetCurrentTime() - fTimeBeforeExec);
								return ERR_DPR_TIMEOUT; //only timeout?
							}
						}
						else
						{
							INTER_QUEUE_CMD cmd;
							cmd.emType = emSetTp;
							memcpy(&cmd.data, pSetData, sizeof(INTERSET_DATA));
							if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) || 
								(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
							{
								BYTE* pNew = NEW(BYTE, pSetData->nValueSize);
								memcpy(pNew, pSetData->varValue.blobValue, pSetData->nValueSize);
								cmd.data.varValue.blobValue = pNew;
								//the pointer will be destroyed by sampler
								if(cmd.data.varValue.blobValue == NULL)
								{
									return ERR_DPR_NO_MEMORY;
								}
							}
							if(ERR_QUEUE_OK != Queue_Put(pDev->info.pBelonged_Sampler->hqControlCommand, 
								&cmd, FALSE))
							{
								return ERR_DPR_QUEUE_FULL;
							}

							//execute later. and will be sync by sampler itself
							return ERR_DPR_OK;
						}
					}
				}

				//Now copy and save data
				Dump_ParamValue(pParam, &pSetData->varValue, pSetData->nValueSize, _NOW_);

				// //now make the cfg change event msg
				// EVENT_PARAM_CHG evCfgChange;
				// evCfgChange.hSender = hSender;
				// evCfgChange.parmPTR.nId = pSetData->nDevId;
				// evCfgChange.parmPTR.nSubId = pSetData->nParamId;

				// EVENT_ARG eventArg;
				// ZERO_POBJS(&eventArg, 1);
				// eventArg.emEvType = EVENT_TYPE_CFG_CHANGED;
				// eventArg.emMsgType = EVENT_MSG_BEGIN;
				// eventArg.nEvDataSize = sizeof(EVENT_PARAM_CHG);
				// eventArg.pEvData = (void*)&evCfgChange;
				// //now send out the msg
				// Intern_Set(hSender, SETTYPE_SENDMSG, &eventArg, sizeof(EVENT_ARG));
			}

			if(pSettParam->bNeedSave)
			{
				if((SETTYPE_INTERN_SETT != emSetTp) &&
				 (pSetData->nParamId == 3))
				{
					//Jimmy note20190529: if it is Network setting, do not save unless user change it on UI
					return ERR_DPR_OK;
				}

				//Jimmy: put the event here as it shall always been sent(for web)
				//now make the cfg change event msg
				int nWhichSingleModule;
				EVENT_PARAM_CHG evCfgChange;
				evCfgChange.hSender = hSender;
				evCfgChange.parmPTR.nId = pSetData->nDevId;
				evCfgChange.parmPTR.nSubId = pSetData->nParamId;

				EVENT_ARG eventArg;
				ZERO_POBJS(&eventArg, 1);
				eventArg.emEvType = EVENT_TYPE_CFG_CHANGED;
				eventArg.emMsgType = EVENT_MSG_BEGIN;
				eventArg.nEvDataSize = sizeof(EVENT_PARAM_CHG);
				eventArg.pEvData = (void*)&evCfgChange;
				//now send out the msg
				Intern_Set(hSender, SETTYPE_SENDMSG, &eventArg, sizeof(EVENT_ARG));

				BYTE* pbySetData = pSetData->varValue.abyValue;
				if((pParam->pParamInfo->emValueType == VALUE_TYPE_STRING) || 
					(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB))
				{
					pbySetData = pSetData->varValue.blobValue;
				}
				sqlite3* data_db = Open_Lock_SqliteDB();
				if(SETTYPE_Reread_PCS_Param != emSetTp)
				{
					sqlite3* data_db = Open_Lock_SqliteDB();
					if(!Save_SettingParam(pSetData->nDevId, 
					pSetData->nParamId,
					pbySetData,
					pSetData->nValueSize, data_db))
					{
						Close_UnLock_SqliteDB();
						return ERR_DPR_DB_ERR;
					}
				}
				else if(SETTYPE_Reread_PCS_Param == emSetTp)
				{
					sqlite3* data_db = Open_Lock_SqliteDB();
					nWhichSingleModule = pSetData->nWhichSingleModule;
					if(!Update_RereadData(pSetData->nDevId, 
						pSetData->nParamId, pbySetData, 
						pSetData->nValueSize, nWhichSingleModule, data_db))
					{
						Close_UnLock_SqliteDB();
						return ERR_DPR_DB_ERR;
					}
				}
				Close_UnLock_SqliteDB();
			}

			break;
		}
	}
	else
	{
		if(nBuffSize != sizeof(EVENT_ARG))
		{
			return ERR_DPR_INVALID_ARG;
		}
		//handle event or put it to queue
		EVENT_ARG* pEventArg = (EVENT_ARG*)pBuff;
		//memory move to New heap and will be destroy in handling process
		if(pEventArg->nEvDataSize > 0)
		{
			BYTE* pbyNewData = NEW(BYTE, pEventArg->nEvDataSize);
			if(pbyNewData == NULL)
			{
				return ERR_DPR_NO_MEMORY;
			}
			memmove(pbyNewData, pEventArg->pEvData, pEventArg->nEvDataSize);
			pEventArg->pEvData = pbyNewData;
		}
		else
		{
			pEventArg->pEvData = NULL;
		}

		if(pEventArg->bBlockMode)
		{
			return (ERRCDDE_DPR)HandleEvent(s_pCharger, hSender, pEventArg);
		}
		else
		{
			pEventArg->hSender = hSender;
			//put it to queue and handle it later
			Queue_Put(s_pCharger->hqEvent, pEventArg, FALSE);
			static time_t s_tmLstWarn = 0;
			int nEventQueueCount = Queue_GetCount(s_pCharger->hqEvent, NULL);
			if((nEventQueueCount > 5) && (ABS(_NOW_ - s_tmLstWarn) > 30))
			{			
				s_tmLstWarn = _NOW_;
				LogOut(MAIN_MODULE_DPR, "Event Hand", LOG_TYPE_WARING, 
					"The event queue is a BIT heavy, queue count = %d!", nEventQueueCount);			
			}		
		}
	}

	return ERR_DPR_OK;
}

static BOOL DoDataProc(BOOL bStart)
{
	if(!bStart)
	{
		s_pCharger->dataprocessor.bExit = TRUE;
		return TRUE;
	}

	//RestoreHisEvent(s_pCharger);
	DATA_PROCESSOR* pDpr = &s_pCharger->dataprocessor;
	pDpr->bExit = FALSE;

	pDpr->hThread = RunThread_Create("DataProcess",
		(RUN_THREAD_START_PROC)_ThreadEntry_DataProcessing,
		(void *)(s_pCharger),
		(DWORD*)&pDpr->bExit);
	if(pDpr->hThread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

static void RestoreSettings(HANDLE hCharger, HANDLE hDB)
{
	CHARGER *pCharger = (CHARGER*)hCharger;
	sqlite3* data_db = (sqlite3*)hDB;
	sqlite3_stmt * stat;
	PARAMETER* pParam;
	VAR_VALUE varV;
	int nErr = 0;
 	sqlite3_prepare_v2(data_db, "SELECT * FROM Data_SavedSettings", -1, &stat, NULL);
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		int nDevId = sqlite3_column_int(stat, 0);
		int nParamId = sqlite3_column_int(stat, 1);
		TRACEX("---Now begin restoring the param[%d, %d]---\n", nDevId, nParamId);
		if((pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nDevId, nParamId)) == NULL)
		{
			LogOut_A(MAIN_MODULE_DPR, "RestoreHis", LOG_TYPE_WARING,
				"Found an INVALID setting param[%d, %d]!", nDevId, nParamId);
			continue;
		}
 
		//value and value length
		time_t tmSet = (time_t)sqlite3_column_int(stat, 2);
		void* pData = sqlite3_column_blob(stat, 3);
		int nVaLen = sqlite3_column_bytes(stat, 3);
		BOOL bNeedNewBlod = FALSE;
		if((pParam->pParamInfo->emValueType != VALUE_TYPE_STRING) &&
			(pParam->pParamInfo->emValueType != VALUE_TYPE_BLOB) &&
			(nVaLen > 0))
		{
			memcpy(varV.abyValue, pData, MIN(nVaLen, 4));
		}
		else
		{
			varV.blobValue = pData;
			//for LocalList/Cache, we need uncompress first
			if(/*(nParamId == 6) || (nParamId == 7)*/0)
			{
				int nVaLen_Ex = sqlite3_sizeof_blob_uncompressed(stat, 3);
				if(nVaLen_Ex > 0)
				{
					pData = NEW(BYTE, nVaLen_Ex);
					if(pData == NULL)
					{
						LogOut_A(MAIN_MODULE_DPR, "RestoreHis", LOG_TYPE_ERROR,
							"Param[%d, %d] value can NOT be restored as NEW failed!",
							 nDevId, nParamId);
						continue;
					}
					TRACEX("---Now restore the uncompressed %s for DataSavedSetting table of DB---\n",
						(nParamId == 6) ? "LocalList" : "LocalCache");
					sqlite3_parse_blob_uncompressed(stat, 3, pData, nVaLen_Ex);
					varV.blobValue = pData;
					nVaLen = nVaLen_Ex;
					bNeedNewBlod = TRUE;
				}
			}
		}

		//now check the param
		nErr = Check_ParamValue(pParam, &varV, nVaLen, TRUE);
		if(nErr != ERR_PARAMVAL_OK)
		{
			if(nErr == ERR_PARAMVAL_OK_BUT_HAS_EXEPT)
			{
				LogOut_A(MAIN_MODULE_DPR, "RestoreFromDB", LOG_TYPE_WARING,
					"Param[%d, %d] value size is Accepted but from old SW!", nDevId, nParamId);
			}
			else
			{
				LogOut_A(MAIN_MODULE_DPR, "RestoreHis", LOG_TYPE_WARING,
					"Param[%d, %d] value %s is INVALID!", nDevId, nParamId,
					(nErr == ERR_PARAMVAL_SIZE) ? "size" : "range");
				if(bNeedNewBlod)
				{
					DELETE(pData);
				}
				continue;
			}
		}
 
		if(pParam->pParamInfo->emValueType == VALUE_TYPE_STRING)
		{
			TRACE("Now successfully restored a STRING param[%d, %d]-->[%s]---\n", nDevId, nParamId, (char*)varV.blobValue);
		}
		else
		{
			TRACE("Now successfully restored an OTHER param[%d, %d], val_int[%d]---\n", nDevId, nParamId, varV.lValue);
		}
		//now set value
		Dump_ParamValue(pParam, &varV, nVaLen, tmSet);
		if(bNeedNewBlod)
		{
			DELETE(pData);
		}
	}
 
	sqlite3_finalize(stat);
}

typedef struct tagCfgReader
{
	char				szTable[64];
	CFG_ROW_PARSER	pfnLoader;
	void**			ppRowHeader;
	size_t			nRowSize;
	UINT*			pnRowTotal;
	BOOL				bIsNullable; // can be null table or not

}CFG_READER;

static BOOL Intern_LoadCfgAndData(HANDLE pDataDB)
{
	char	szDBFile[256] = {0};
	sqlite3 *cfg_db;
	MakeFullFileName(szDBFile, \
		getenv(ENV_VARIABLE_APP_PATH), "config", CHARGER_CONFIG_DB);
	int rc = sqlite3_open_v2( szDBFile, &cfg_db, SQLITE_OPEN_READONLY, NULL);
	if(rc != SQLITE_OK)
	{
		TRACE("Can not create cfg database, so exit system!");
		LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
			"Thd CfgDB [%s] can NOT be opened, please check!", szDBFile);
		return FALSE;
	}
	//Jimmy Note: the order of the following table can NOT be CHANGED!!!
	CFG_READER table_readers[] = 
	{
		{"Revisions",							Cfg_Parse_ChargerRevision,(void**)&s_pCharger->info.charger_revision_list.p,	sizeof(CHARGER_REVISION),	&s_pCharger->info.charger_revision_list.nTotal,	FALSE},
		{"Resources",							Cfg_Parse_Resources,		(void**)&s_pCharger->info.resource_list.p,			sizeof(RESOURCE_TEXT),		&s_pCharger->info.resource_list.nTotal,		FALSE},
		{"Charger_DeviceModel",					Cfg_Parse_DevModel,		(void**)&s_pCharger->info.devModels_info.p,		sizeof(DEVICE_MODEL_INFO),	&s_pCharger->info.devModels_info.nTotal,		FALSE},
		{"Charger_DeviceModel_StatusParameters",		Cfg_Parse_STUParam,		(void**)&s_pCharger->info.statusParams_info.p,		sizeof(STATUS_PARAM_INFO),	&s_pCharger->info.statusParams_info.nTotal,	FALSE},
		{"Charger_DeviceModel_ControlParameters",	Cfg_Parse_CTRParam,		(void**)&s_pCharger->info.controlParams_info.p,	sizeof(CONTROL_PARAM_INFO),	&s_pCharger->info.controlParams_info.nTotal,	FALSE},
		{"Charger_DeviceModel_SettingParameters",	Cfg_Parse_SETParam,		(void**)&s_pCharger->info.settingParms_info.p,		sizeof(SETTING_PARAM_INFO),	&s_pCharger->info.settingParms_info.nTotal,	FALSE},
		{"Charger_DeviceModel_Alarms",			Cfg_Parse_Alarm,		(void**)&s_pCharger->info.alarms_info.p,			sizeof(ALARM_INFO),			&s_pCharger->info.alarms_info.nTotal,		FALSE},
		{"Charger_HistoryParameter",				Cfg_Parse_HisParam,		(void**)&s_pCharger->info.hisParams_info.p,		sizeof(HISPARAM_INFO),		&s_pCharger->info.hisParams_info.nTotal,		TRUE},
		
		{"Charger_CommPort",					Cfg_Parse_CommPort,	(void**)&s_pCharger->commports.p,				sizeof(COMMPORT),			&s_pCharger->commports.nTotal,	FALSE},
		{"Charger_Sampler",						Cfg_Parse_Sampler,		(void**)&s_pCharger->samplers.p,				sizeof(SAMPLER),			&s_pCharger->samplers.nTotal,		FALSE},
		{"Charger_Reporter",						Cfg_Parse_Reporter,		(void**)&s_pCharger->reporters.p,				sizeof(REPORTER),			&s_pCharger->reporters.nTotal,		FALSE},
		{"Charger_Devices",						Cfg_Parse_Device,		(void**)&s_pCharger->devices.p,				sizeof(DEVICE),				&s_pCharger->devices.nTotal,		FALSE},
	};
	sqlite3_stmt * stat;
	char sql[256];
	int n;
	int nTableCount = ITEM_OF(table_readers);
	int nTableRows = 0;
	//first parse table total items for each
	//Reset information first
	memset(&s_pCharger->info, 0, sizeof(CHARGER_INFO));

	for(n = 0; n < nTableCount; n++)
	{
		ZERO_POBJS(sql, 256);
		sprintf(sql, "SELECT COUNT(*) FROM %s", table_readers[n].szTable);
		sqlite3_prepare_v2(cfg_db, sql, -1, &stat, NULL);
		if(sqlite3_step(stat) != SQLITE_ROW)
		{
			LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
				"The sql [%s] can NOT be executed, please check!", sql);
			sqlite3_close(cfg_db);
			return FALSE;
		}
		nTableRows = sqlite3_column_int(stat, 0);
		sqlite3_finalize(stat);
		if(nTableRows == 0)
		{
			if(table_readers[n].bIsNullable)
			{
				continue;
			}
			else
			{
				LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
					"The table [%s] can NOT be NULL, please check!", table_readers[n].szTable);
				sqlite3_close(cfg_db);
				return FALSE;
			}
		}
		else if(table_readers[n].pnRowTotal != NULL)
		{
			*(table_readers[n].pnRowTotal) = nTableRows;
			//Now create memory for table
			*table_readers[n].ppRowHeader = (void*)NEW(BYTE, nTableRows*table_readers[n].nRowSize);
			TRACE("Create table[%s] items total %d bytes.\n", table_readers[n].szTable, nTableRows*table_readers[n].nRowSize);
			if(*table_readers[n].ppRowHeader == NULL)
			{
				LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
					"The table [%s] can NOT be created: Memory FULL!", table_readers[n].szTable);
				sqlite3_close(cfg_db);
				return FALSE;
			}
		}
	}
	//now parse each row for a single table
	int k;
	char *szErrMsg = NULL;
	CFG_ERRCODE errCode;
	void* pRowItem = NULL;
	for(n = 0; n < nTableCount; n++)
	{
		ZERO_POBJS(sql, 256);
		sprintf(sql, "SELECT * FROM %s", table_readers[n].szTable);
		sqlite3_prepare_v2(cfg_db, sql, -1, &stat, NULL);
		nTableRows = (table_readers[n].pnRowTotal == NULL) ? 1:
			(*(table_readers[n].pnRowTotal));

		TRACE("Now Begin to Parse Table [%s], RowSize:%d, Total Row:%d.\n", table_readers[n].szTable,
			table_readers[n].nRowSize, nTableRows);

		for(k = 0; k < nTableRows; k++)
		{
			if(sqlite3_step(stat) != SQLITE_ROW)
			{
				LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
					"The sql [%s] can NOT be executed on Row[%d], please check!", sql, k);
				sqlite3_finalize(stat);
				sqlite3_close(cfg_db);
				return FALSE;
			}

			if(table_readers[n].pnRowTotal != NULL)
			{
				pRowItem = (*table_readers[n].ppRowHeader)+ (k * table_readers[n].nRowSize);
			}
			szErrMsg = NULL;
			errCode = table_readers[n].pfnLoader(stat, pRowItem, &szErrMsg);
			if(errCode != CFG_ERR_OK)
			{
				LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
					"The table[%s] parsed ERROR: %s", table_readers[n].szTable, szErrMsg);
				sqlite3_finalize(stat);
				sqlite3_close(cfg_db);
				SAFELY_DELETE(szErrMsg);
				return FALSE;
			}
		}
		sqlite3_finalize(stat);
	}

	sqlite3_close(cfg_db);
	//now restore value from dataDB
	if(pDataDB != NULL)
	{
		// printf("pDataDB != NULL : RestoreHisEvent\n");
		sqlite3* data_db = (sqlite3*)pDataDB;
		RestoreSettings((HANDLE)s_pCharger, (HANDLE)data_db);
		RestoreHisEvent(s_pCharger);
	}

#ifdef _DEBUG
	Debug_PrintCfg();
#endif

	return TRUE;
}

static int EventProcess(HANDLE hEvent)
{
	EVENT_ARG* pEventArg = (EVENT_ARG*)hEvent;
	if(pEventArg != NULL)
	{
		char szName[32];
		TRACEX("*******HANDLE queued event, sender = [%s], eventType ->[%d].\n", GetModuleName((MODULE_INFO*)pEventArg->hSender, szName),
			pEventArg->emEvType);
		return HandleEvent(s_pCharger, pEventArg->hSender, pEventArg);
	}

	return ERR_DPR_INVALID_ARG;
}

static ERRCDDE_DPR Calculate_Expression(IN PARAMETER* pParam,
	IN char* szExpression, CLUSTER_EXP_ELEMENT *pExpSet, 
	BOOL bParseExpSet, OUT void* pResult)
{
	EXP_CONST result;
	DEVICE* pDev = (DEVICE*)pParam->hDevice;
	int n;
	if(pExpSet == NULL)
	{
		return ERR_DPR_INVALID_ARG;
	}
	if(bParseExpSet)
	{
		if(pExpSet->nTotal > 0)
		{
			//free memory first in case memory leak
			EXP_ELEMENT *p = (EXP_ELEMENT*)pExpSet->p;
			for (n = 0; n < pExpSet->nTotal; n++, p++)
			{
				SAFELY_DELETE(p);
			}
			pExpSet->nTotal = 0;
		}
		if(ExpressionParser(szExpression, pExpSet) != CFG_ERR_OK)
		{
			return ERR_DPR_INVALID_ARG;
		}
	}
	char *szParamName = (pParam->pParamInfo->pRes_Name == NULL) ? "NA" : pParam->pParamInfo->pRes_Name->szName;
	int nErrCode = Exp_Calculate(pDev->info.nDeviceId, szParamName, 
		pExpSet->p, pExpSet->nTotal, &result);
	if(nErrCode == EXP_ETYPE_CONST)
	{
		*(EXP_CONST*)pResult = result;
		return ERR_DPR_OK;
	}
	return ERR_DPR_INVALID_ARG;
}

BOOL Init_CreateDataProcessor(CHARGER* pCharger)
{
	s_pCharger = pCharger;
	DATA_PROCESSOR *pThis = &pCharger->dataprocessor;
	pThis->info.emModuleType = MODULE_TYPE_DATAPROC;
	pThis->info.nDataProcID = 1;
	sprintf(pThis->info.szDataProcName, "DATA_PROCESSOR");
	pThis->Get = Intern_Get;
	pThis->Set = Intern_Set;
	//init for loading cfg
	pCharger->info.devModels_info.nTotal = 0;
	pCharger->info.statusParams_info.nTotal = 0;
	pCharger->info.controlParams_info.nTotal = 0;
	pCharger->info.settingParms_info.nTotal = 0;
	pCharger->info.alarms_info.nTotal = 0;
	pCharger->info.hisParams_info.nTotal = 0;

	pCharger->devices.nTotal = 0;
	pCharger->samplers.nTotal = 0;
	pCharger->reporters.nTotal = 0;
	pCharger->commports.nTotal = 0;
	pThis->LoadConfigAndData = Intern_LoadCfgAndData;
	pThis->DoDataProcessing = DoDataProc;
	pThis->DoEvent = EventProcess;
	pThis->Register_EvHandler = Register_EventHandler;
	pThis->CalculateExp = Calculate_Expression;
	if(!InitCfgMagt((HANDLE)pCharger))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL Init_CreadDataDB(HANDLE hDB)
{
	sqlite3* db = (sqlite3*)hDB;
	char* szErrMsg = NULL;
	sqlite3_stmt* stmt = NULL;

	//1. Data_SavedSetting
	//according to official doc, 'create table if not exits...' statement will still return error but has no error msg
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data_SavedSettings("
		"Device_Id INTEGER NOT NULL,"
		"Param_Id INTEGER NOT NULL,"
		"Save_Time DATETIME, "
		"Current_Value BLOB,"
		"PRIMARY KEY (Device_Id,Param_Id))",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_SavedSettings table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}
	const char* arrColumns[] = {
        "RereadValue_SingleModule_1",
        "RereadValue_SingleModule_2",
        "RereadValue_SingleModule_3",
        "RereadValue_SingleModule_4",
        "RereadValue_SingleModule_5",
        "RereadValue_SingleModule_6",
        "RereadValue_SingleModule_7",
        "RereadValue_SingleModule_8"
    };

    int nColumnCount = sizeof(arrColumns) / sizeof(arrColumns[0]);
	int i, nRet;
    for(i = 0; i < nColumnCount; i++)
    {
        const char* szColName = arrColumns[i];
        char sql_check_col[256] = {0};
        snprintf(sql_check_col, sizeof(sql_check_col), 
                 "PRAGMA table_info(Data_SavedSettings) WHERE name='%s'", szColName);
        
        stmt = NULL;
        nRet = sqlite3_prepare_v2(db, sql_check_col, -1, &stmt, NULL);
        BOOL bColExist = FALSE;
        if(nRet == SQLITE_OK)
        {
            if(sqlite3_step(stmt) == SQLITE_ROW)
            {
                bColExist = TRUE;
            }
        }
        sqlite3_finalize(stmt);

        if(!bColExist)
        {
            char sql_add_col[256] = {0};
            snprintf(sql_add_col, sizeof(sql_add_col), 
                     "ALTER TABLE Data_SavedSettings ADD COLUMN %s BLOB DEFAULT NULL", szColName);
            
            nRet = sqlite3_exec(db, sql_add_col, NULL, NULL, &szErrMsg);
            if(nRet != SQLITE_OK)
            {
                TRACEX("Add Column %s Failed:%s\n", szColName, szErrMsg);
                sqlite3_free(szErrMsg);
                continue; 
            }
    	}
	}
	//2. Data_Alarms
	//according to official doc, 'create table if not exits...' statement will still return error but has no error msg
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data_Alarms("
		"Idx INTEGER PRIMARY KEY,"
		"Device_Id INTEGER NOT NULL,"
		"Alarm_Id INTEGER NOT NULL,"
		"Alarm_Status INTEGER NOT NULL,"
		"Extra_ParamVal DOUBLE,"
		"Alarm_StartTime DATETIME,"
		"Alarm_EndTime DATETIME)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_Alarms table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	//3. Data_ChargeRecords
	//sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_ChargeRecords ("
	//	"Idx                INTEGER  UNIQUE,"
	//	"WhichGun           INTEGER,"
	//	"Charge_BeginTime   DATETIME,"
	//	"ChargeSession_ID           INTEGER,"
	//	"UserID             STRING,"
	//	"Car_VIN            STRING,"
	//	"ChargeStart_Mothod INTEGER,"
	//	"ChargingStatus     INTEGER,"
	//	"Report_Flag      INTERGER,"
	//	"OfflinePay_Flag      INTERGER,"
	//	"ChargeData         BLOB,"
	//	"PRIMARY KEY (WhichGun, Charge_BeginTime, ChargeSession_ID))",
	//	NULL, NULL, &szErrMsg);
	//sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_ChargeRecords ("
	//	"Idx                INTEGER  PRIMARY KEY,"
	//	"WhichGun           INTEGER,"
	//	"Charge_BeginTime   DATETIME,"
	//	"ChargeSession_ID           INTEGER,"
	//	"UserID             STRING,"
	//	"Car_VIN            STRING,"
	//	"ChargeStart_Mothod INTEGER,"
	//	"ChargingStatus     INTEGER,"
	//	"Report_Flag      INTERGER,"
	//	"OfflinePay_Flag      INTERGER,"
	//	"ChargeData         BLOB)",
	//	NULL, NULL, &szErrMsg);
	//if(szErrMsg != NULL)
	//{
	//	TRACE("Can not create Data_ChargeRecords table, reason = [%s].\n", szErrMsg);
	//	sqlite3_free(szErrMsg);
	//	return FALSE;
	//}

	//4. Data_Events
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_Events ("
		"Idx          INTEGER  PRIMARY KEY,"
		"Event_Type   INTEGER,"
		"Event_Status INTEGER  DEFAULT (0) NOT NULL,"
		"Event_Data   BLOB     NOT NULL,"
		"Begin_Time   DATETIME NOT NULL,"
		"End_Time     DATETIME)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_Events table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	//5. Data_HistoricalData
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_HistoricalData ("
		"Idx          INTEGER  PRIMARY KEY,"
		"Device_Id    INTEGER  NOT NULL,"
		"Param_Id     INTEGER  NOT NULL,"
		"CurrentValue BLOB     NOT NULL,"
		"Record_Time  DATETIME NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_HistoricalData table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	//6. Data_BMSHealthData
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_BMSHealthData ("
		"Idx          INTEGER  PRIMARY KEY,"
		"Day   			INTEGER  NOT NULL,"
		"Log_Time     DATETIME NOT NULL,"
		"Health_Data   BLOB     NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_BMSHealthData table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	//7. Data_ESStatDaily
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_ESStats_Daily ("
		"Idx INTEGER PRIMARY KEY, Day DATETIME NOT NULL, Data BLOB)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_ESStats_Daily table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}
	
	//8. Data_RealTimeChargeData (for diagnostic)
	// sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_RealTimeBmsData ("
	// 	"Idx          	INTEGER  PRIMARY KEY,"
	// 	"Day   			INTEGER  NOT NULL,"
	// 	"Log_Time  		DATETIME NOT NULL,"
	// 	"Runtime_Data   BLOB     NOT NULL)",
	// 	NULL, NULL, &szErrMsg);
	// if(szErrMsg != NULL)
	// {
	// 	TRACE("Can not create Data_RealTimeBmsData table, reason = [%s].\n", szErrMsg);
	// 	sqlite3_free(szErrMsg);
	// 	return FALSE;
	// }

	//9. Data_CurrentSPFW_kwh (Record CurrentSPFW_kwh at 0:00 every day for reporter )
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_CurrentSPFW_kwh ("
		"Idx          	INTEGER  PRIMARY KEY,"
		"Device_Id    	INTEGER  NOT NULL,"
		"Day   		    INTEGER  NOT NULL,"
		"Log_Time  		DATETIME NOT NULL,"
		"Current_Pos_Sum   		DOUBLE,"
		"Current_Pos_Sharp   	DOUBLE,"
		"Current_Pos_Peak   	DOUBLE,"
		"Current_Pos_Flat   	DOUBLE,"
		"Current_Pos_Wave   	DOUBLE,"
		"Current_Neg_Sum   		DOUBLE,"
		"Current_Neg_Sharp   	DOUBLE,"
		"Current_Neg_Peak   	DOUBLE,"
		"Current_Neg_Flat   	DOUBLE,"
		"Current_Neg_Wave   	DOUBLE)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		printf("Can not create Data_CurrentSPFW_kwh table, reason = [%s].\n", szErrMsg);
		TRACE("Can not create Data_CurrentSPFW_kwh table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	
	//10. Data_AccumulatedData (for storing efficiency data)
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Data_AccumulatedData ("
		"Idx          		INTEGER  PRIMARY KEY,"
		"Type   			INTEGER  NOT NULL,"
		"Time  				DATETIME NOT NULL,"
		"Accumulated_Data   BLOB     NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_AccumulatedData table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}
	
	//11. Data_BmsHistFault
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data_BmsHistFault("
		"Idx INTEGER PRIMARY KEY AUTOINCREMENT,"
		"BattGrp INTEGER, FaultType INTEGER, FaultCode INTEGER,"
		"FaultTime INTEGER,"
		"ProtectState INTEGER, WorkState INTEGER,"
		"Current INTEGER, Custom3 INTEGER, Custom4 INTEGER, Custom5 INTEGER,"
		"SaveTime INTEGER,"
		"Status INTEGER DEFAULT 0)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_BmsHistFault table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}
	
	//12. Data_BmsHistEvent
	sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS Data_BmsHistEvent("
		"Idx INTEGER PRIMARY KEY AUTOINCREMENT,"
		"BattGrp INTEGER, EventType INTEGER, EventCode INTEGER,"
		"EventTime INTEGER,"
		"ProtectState INTEGER, WorkState INTEGER,"
		"Current INTEGER, Custom3 INTEGER, Custom4 INTEGER, Custom5 INTEGER,"
		"SaveTime INTEGER)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_BmsHistEvent table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	return TRUE;
}
