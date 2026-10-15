#include "dpr.h"
#include <values.h>
#include <math.h>
#include "run_expression.h"
#include "../../../version_patch.h"

static CHARGER* s_pCharger = NULL;

BOOL InitCfgMagt(HANDLE hCharger)
{
	s_pCharger = (CHARGER*)hCharger;
	//TRACE("CHARGERDDD = %d.\n", s_pCharger);
	return TRUE;
}

PARAMETER* GetParam_ByPR(PARAM_TYPE emType, int nDevId, int nParamId)
{
	PARAMETER* pParam = NULL;
	CHARGER *pCharger = s_pCharger;
	int n;
	int k;
	DEVICE * pDevice = pCharger->devices.p;
	PARAMETER *pPaa = NULL;
	int nTotal = 0;

	for(n = 0; n < pCharger->devices.nTotal; n ++, pDevice++)
	{
		if(pDevice->info.nDeviceId == nDevId)
		{
			pPaa = (emType == PARAM_TYPE_STATUS) ?
				pDevice->params_Status.p : 
				((emType == PARAM_TYPE_CONTROL) ? 
					pDevice->params_Control.p : pDevice->params_Setting.p);

			nTotal = (emType == PARAM_TYPE_STATUS) ?
					pDevice->params_Status.nTotal : 
				((emType == PARAM_TYPE_CONTROL) ? 
					pDevice->params_Control.nTotal : pDevice->params_Setting.nTotal);

			for(k = 0; k < nTotal; k++, pPaa++)
			{
				if(pPaa->pParamInfo->nParamId == nParamId)
				{
					pParam = pPaa;
					break;
				}
			}
			break;
		}
	}
	return pParam;
}

ALARM* GetAlarm_ByPR(int nDevId, int nAlarmId)
{
	ALARM* pAlarm = NULL;
	CHARGER *pCharger = s_pCharger;
	int n;
	int k;
	DEVICE * pDevice = pCharger->devices.p;
	ALARM *pPaa = NULL;
	for(n = 0; n < pCharger->devices.nTotal; n ++, pDevice++)
	{
		if(pDevice->info.nDeviceId == nDevId)
		{
			pPaa = pDevice->alarms.p;
			for(k = 0; k < pDevice->alarms.nTotal; k++, pPaa++)
			{
				if(pPaa->pAlarmInfo->nAlarmId== nAlarmId)
				{
					pAlarm = pPaa;
					break;
				}
			}
			break;
		}
	}
	return pAlarm;
}

DEVICE_MODEL_INFO* GetDevModel_ById(int nDevModelId)
{
	DEVICE_MODEL_INFO* pDevModel = NULL;
	int n;
	//TRACE("DevModelTotal = %d.\n", s_pCharger->info.devModels_info.nTotal);

	if(s_pCharger->info.devModels_info.nTotal == 0)
	{
		return NULL;
	}
	
	DEVICE_MODEL_INFO* pTmp = s_pCharger->info.devModels_info.p;
	//TRACE("The pTmp = %d.\n", pTmp);
	for(n = 0; n < s_pCharger->info.devModels_info.nTotal; n++, pTmp++)
	{
		//TRACE("%d. %d, [%s].\n", n + 1, pTmp->nDevModelId, pTmp->szDevModelName);
		if(pTmp->nDevModelId == nDevModelId)
		{
			pDevModel = pTmp;
			break;
		}
	}
	return pDevModel;
}

static void MakeErrorMsg(char **ppszErr, IN char *pszFormat, ...)
{
	char szBuf[128];

	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szBuf, sizeof(szBuf), pszFormat, arglist);
	va_end(arglist);

	int nLen = strlen(szBuf);
	if(nLen < 1)
	{
		return;
	}

	*ppszErr = NEW(char, nLen + 1);
	if(*ppszErr != NULL)
	{
		TRACE("ErrorInfo: %s\n", szBuf);
		memcpy(*ppszErr, szBuf, nLen);
	}
}

DEVICE* GetDevce_ById(int nDeviceId)
{
	DEVICE* pDev = NULL;
	int n;
	if(s_pCharger->devices.nTotal < 1)
	{
		return NULL;
	}
	DEVICE* pTmp = s_pCharger->devices.p;
	for(n = 0; n < s_pCharger->devices.nTotal; n++, pTmp++)
	{
		if(pTmp->info.nDeviceId == nDeviceId)
		{
			pDev = pTmp;
			break;
		}
	}
	return pDev;
}

int Check_ParamValue(PARAMETER* pParam, VAR_VALUE* pValue, int nSizeValue, BOOL bBackwardCompatible_4Blob)
{
	int nRealValSize = pParam->pParamInfo->nValueSize;
	//if(pParam->pParamInfo->emValueType == VALUE_TYPE_ENUM)
	//{
	//	nRealValSize = 4;
	//}
	BOOL bException = FALSE;
	if(nRealValSize!= nSizeValue)
	{
		TRACE("ERRRRVal[%d,%d]\n", nRealValSize, nSizeValue);
		if((pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB) && bBackwardCompatible_4Blob/* &&
			(nRealValSize > nSizeValue)*/) //20200530: blob data need to be back-compitible for version rolling back
		{
			bException = TRUE;
		}
		//Jimmy added 20190928: when blob is expanded, the length can be bigger than old one
		if(!bException)
		{
			return ERR_PARAMVAL_SIZE;
		}
	}
	else if(pValue == NULL)
	{
		return ERR_PARAMVAL_RANGE;
	}
	//now check range
	if(pParam->pParamInfo->emValueType == VALUE_TYPE_BLOB)
	{
		int tp = *(int*)pValue->blobValue;
		//TRACE("\n----------now check a blob param[%d, %d, %d], blobType = %d-------\n", pParam->pParamInfo->emParamType,
		//	((DEVICE*)pParam->hDevice)->info.nDeviceId, 
		//	pParam->pParamInfo->nParamId, tp);
		if((tp < BLOB_TYPE_UPGRADE_CCU) || (tp >= BLOB_TYPE_END))
		{
			TRACE("vvvvv[%d\n", tp);
			return ERR_PARAMVAL_RANGE;
		}
	}
	else if(pParam->pParamInfo->emValueType != VALUE_TYPE_STRING)
	{
		switch (pParam->pParamInfo->emValueType)
		{
		case VALUE_TYPE_LONG:
			if((pValue->lValue> (LONG)pParam->pParamInfo->fMaxLimit) ||
				(pValue->lValue < (LONG)pParam->pParamInfo->fMinLimit))
			{
				return ERR_PARAMVAL_RANGE;
			}
			break;
		case VALUE_TYPE_ULONG:
		case VALUE_TYPE_DATETIME:
		case VALUE_TYPE_ENUM:
			if((pValue->ulValue > (ULONG)pParam->pParamInfo->fMaxLimit) ||
				(pValue->ulValue < (ULONG)pParam->pParamInfo->fMinLimit))
			{
				TRACE("----ulVal = %d, fMax = %d, fMin = %d-----\n",
					pValue->ulValue, (ULONG)pParam->pParamInfo->fMaxLimit, (ULONG)pParam->pParamInfo->fMinLimit);
				return ERR_PARAMVAL_RANGE;
			}
			break;
		case VALUE_TYPE_FLOAT:
		default:
			if((pValue->fValue > pParam->pParamInfo->fMaxLimit) ||
				(pValue->fValue < pParam->pParamInfo->fMinLimit))
			{
				return ERR_PARAMVAL_RANGE;
			}
			break;
		}
	}
	return bException ? ERR_PARAMVAL_OK_BUT_HAS_EXEPT : ERR_PARAMVAL_OK;
}
// #undef TRACE 

void Dump_ParamValue(PARAMETER* pParm, VAR_VALUE* pNewVal, int nSizeValue, time_t tmStamp)
{
	pParm->tmLastValueChanged = pParm->tmCurrValueRefreshed;
	pParm->tmCurrValueRefreshed = tmStamp;
	//pParm->emValueStatus = VALUE_STATUS_NORMAL;
	pParm->bIsValueInvalid = FALSE;
	
	//added by Jimmy 20200909
	nSizeValue = MIN(pParm->pParamInfo->nValueSize, nSizeValue);
	nSizeValue = MAX(0, nSizeValue);

	if(pParm->pParamInfo->emValueType == VALUE_TYPE_STRING)
	{
		memcpy(pParm->lastValue.blobValue, pParm->currValue.blobValue, (size_t)pParm->pParamInfo->nValueSize);
		memcpy(pParm->currValue.blobValue, pNewVal->blobValue, (size_t)nSizeValue);
	}
	else if(pParm->pParamInfo->emValueType == VALUE_TYPE_BLOB)
	{
		memcpy(pParm->currValue.blobValue, pNewVal->blobValue, (size_t)nSizeValue);
	}
	else
	{
		memcpy(pParm->lastValue.abyValue, pParm->currValue.abyValue, 4);
		memcpy(&pParm->currValue, pNewVal, 4);
	}
}

void Dump_ReadParamValue(PARAMETER* pParm, VAR_VALUE* pNewVal, int nSizeValue, time_t tmStamp)
{
	
	nSizeValue = MIN(pParm->pParamInfo->nValueSize, nSizeValue);
	nSizeValue = MAX(0, nSizeValue);

	if(pParm->pParamInfo->emValueType == VALUE_TYPE_STRING)
	{
		memcpy(pParm->ReadValue.blobValue, pNewVal->blobValue, (size_t)nSizeValue);
	}
	else if(pParm->pParamInfo->emValueType == VALUE_TYPE_BLOB)
	{
		memcpy(pParm->ReadValue.blobValue, pNewVal->blobValue, (size_t)nSizeValue);
	}
	else
	{
		memcpy(&pParm->ReadValue, pNewVal, 4);
	}
}

CFG_ERRCODE Cfg_Parse_ChargerRevision(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	CHARGER_REVISION* pRevis = (CHARGER_REVISION*)pRowItem;
	memset(pRevis, 0, sizeof(CHARGER_REVISION));
	UNUSED(ppszErrMsg);
	static time_t dtNewestEdit = 0;
	//_DATE_TIME dtCurrEdit = (_DATE_TIME)sqlite3_column_int(pStmt, 2);
	char* szDate = (char*)sqlite3_column_text(pStmt, 4);
	time_t dtCurrEdit = StringToTime(szDate, SQLITE_DATE_FMT);
	char szTime[32];
	TRACEX("The ReleaseDate = %d, [%s]---\n", dtCurrEdit, TimeToString(dtCurrEdit, TIME_CHN_FMT, szTime, 32));
	pRevis->dtReleaseDate = dtCurrEdit;

	pRevis->nPkgVersion = sqlite3_column_int(pStmt, 1);

	UCHAR* szSWVer = sqlite3_column_text(pStmt, 2);
	if(szSWVer != NULL)
	{
		memcpy(pRevis->szSWVersion, szSWVer, MIN(MAX_LEN_MU2SWVER, sqlite3_column_bytes(pStmt, 2)));
	}

	UCHAR* szPkgVer = sqlite3_column_text(pStmt, 3);
	if(szPkgVer != NULL)
	{
		memcpy(pRevis->szLCCUVersion, szPkgVer, MIN(16, sqlite3_column_bytes(pStmt, 3)));
	}

	UCHAR* szAuthor = sqlite3_column_text(pStmt, 5);
	if(szAuthor != NULL)
	{
		memcpy(pRevis->szConfAuthor, szAuthor, MIN(32, sqlite3_column_bytes(pStmt, 5)));
	}
	UCHAR* szDetail = sqlite3_column_text(pStmt, 6);
	if(szDetail != NULL)
	{
		memcpy(pRevis->szUpdateDetails, szDetail, MIN(128, sqlite3_column_bytes(pStmt, 6)));
	}
		if(dtCurrEdit > dtNewestEdit) //get newest date as now version
	{
		dtNewestEdit = dtCurrEdit;
		s_pCharger->info.nPkgVer = pRevis->nPkgVersion;
		memcpy(s_pCharger->info.szSWVer, pRevis->szSWVersion, MAX_LEN_MU2SWVER);
		// LJH241101 MU3
#ifdef IMMU3
		strncat(s_pCharger->info.szSWVer, ".3", MAX_LEN_MU2SWVER);
#else
		strncat(s_pCharger->info.szSWVer, ".2", MAX_LEN_MU2SWVER);
#endif
		strncat(s_pCharger->info.szSWVer, SWVERSION_PATCH_NUMBER, MAX_LEN_MU2SWVER);
		TRACED("Cfg_Parse_ChargerRevision szSWVer:%s %s", s_pCharger->info.szSWVer, SWVERSION_PATCH_NUMBER);
	}

	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_Resources(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	RESOURCE_TEXT* pRes = (RESOURCE_TEXT*)pRowItem;
	memset(pRes, 0, sizeof(RESOURCE_TEXT));
	pRes->nRes_Id = sqlite3_column_int(pStmt, 0);
	UCHAR *szName = sqlite3_column_text(pStmt, 1);
	if(szName != NULL)
	{
		memcpy(pRes->szName, szName, MIN(32, sqlite3_column_bytes(pStmt, 1)));
	}
	else
	{
		MakeErrorMsg(ppszErrMsg, "ResID[%d]-->Text Name CANNOT be NULL!", pRes->nRes_Id);
		return CFG_ERR_BADCONFIG;
	}
	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_DevModel(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	UNUSED(ppszErrMsg);
	DEVICE_MODEL_INFO* pDevInfo = (DEVICE_MODEL_INFO*)pRowItem;
	memset(pDevInfo, 0, sizeof(DEVICE_MODEL_INFO));

	pDevInfo->nDevModelId = sqlite3_column_int(pStmt, 0);
	UCHAR *szDevName = sqlite3_column_text(pStmt, 1);
	if(szDevName != NULL)
	{
		memcpy(pDevInfo->szDevModelName, szDevName, MIN(32, sqlite3_column_bytes(pStmt, 1)));
	}
	return CFG_ERR_OK;
}

static BOOL ParseValueType_FromString(IN const UCHAR* szSrc, OUT PARAM_BASIC_INFO* pInfo)
{
	if(strcmp(szSrc, "INT") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_LONG;
		return TRUE;
	}
	else if(strcmp(szSrc, "UINT") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_ULONG;
		return TRUE;
	}
	else if(strcmp(szSrc, "FLOAT") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_FLOAT;
		return TRUE;
	}
	else if(strcmp(szSrc, "DATETIME") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_DATETIME;
		return TRUE;
	}
	else if(strcmp(szSrc, "ENUM") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_ENUM;
		return TRUE;
	}
	else if(strcmp(szSrc, "TEXT") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_STRING;
		return TRUE;
	}
	else if(strcmp(szSrc, "BLOB") == 0)
	{
		pInfo->emValueType = VALUE_TYPE_BLOB;
		return TRUE;
	}
	return FALSE;
}

static RESOURCE_TEXT* GetRes_ById(int nResId)
{
	int n;
	RESOURCE_TEXT* pRes = NULL;
	RESOURCE_TEXT* pRes1 = s_pCharger->info.resource_list.p;
	for(n = 0; n < s_pCharger->info.resource_list.nTotal; n++, pRes1++)
	{
		if(pRes1->nRes_Id == nResId)
		{
			pRes = pRes1;
			break;
		}
	}
	return pRes;
}

static BOOL PargeValueRange_FromString(IN const UCHAR* szSrc, OUT PARAM_BASIC_INFO* pInfo, OUT char** ppszErrMsg)
{
	pInfo->nValueSize = 4;
	if(szSrc == NULL)
	{
		switch(pInfo->emValueType)
		{
		case VALUE_TYPE_LONG:
			pInfo->fMaxLimit = (float)MAXLONG;
			pInfo->fMinLimit = (float)MINLONG;
			break;
		case VALUE_TYPE_ULONG:
		case VALUE_TYPE_DATETIME:
			pInfo->fMaxLimit = (float)MAX_ULONG;
			pInfo->fMinLimit = 0;
			break;
		case VALUE_TYPE_FLOAT:
			pInfo->fMaxLimit = MAXFLOAT;
			pInfo->fMinLimit = MIN_FLOAT;
			break;
		case VALUE_TYPE_ENUM:
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->ENUM value range can NOT be NULL!", pInfo->nParamId);
			return FALSE; //enum can not have empty range
		case VALUE_TYPE_STRING:
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->STRING value range can NOT be NULL!", pInfo->nParamId);
			return FALSE; //String can not have empty range
		case VALUE_TYPE_BLOB:
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->BLOB value range can NOT be NULL!", pInfo->nParamId);
			return FALSE; //Blob can not have empty range
		default:
			break;
		}
		return TRUE;
	}
	//Now parse range
	if((pInfo->emValueType == VALUE_TYPE_STRING) ||
		(pInfo->emValueType == VALUE_TYPE_BLOB))
	{
		int nParseSize = 0;
		if(!CheckInt_AndParse(szSrc, &nParseSize))
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Value Range INVALID", pInfo->nParamId);
			return FALSE;
		}
		else if(nParseSize< 1)
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Value Size not correct!", pInfo->nParamId);
			return FALSE;
		}
		if(pInfo->emValueType == VALUE_TYPE_STRING)
		{
			if((nParseSize < 2) || (nParseSize > 4096))
			{
				MakeErrorMsg(ppszErrMsg, "Param[%d]-->Text size over range(2~4096)!", pInfo->nParamId);
				return FALSE;
			}
			pInfo->nValueSize = nParseSize;
		}
		else
		{
			switch(nParseSize)
			{
			case BLOB_TYPE_UPGRADE_CCU:
				pInfo->nValueSize = sizeof(A_BLOB_UPDT_INFO_4CCU);
				break;

			case BLOB_TYPE_NETWORKPARAMS:
				pInfo->nValueSize = sizeof(A_BLOB_NETWORKINFO);
				break;

			case BLOB_TYPE_CHARGE_FEE_SET:
				pInfo->nValueSize = sizeof(A_BLOB_CHARGE_FEESET);
				break;

			case BLOB_TYPE_SINGLE_CHARGEPROFILE:
				pInfo->nValueSize = sizeof(A_BLOB_SINGLE_CHARGINGPROFILE);
				break;
			case BLOB_TYPE_METERVALUES:
				pInfo->nValueSize = sizeof(A_BLOB_METER_VALUES);
				break;
			case BLOB_TYPE_UPDATE_CHARG_STAT_METER:
				pInfo->nValueSize = sizeof(A_BLOB_UPDATE_CHARGEMETER_STAT);
				break;
			case BLOB_TYPE_CHARGER_BOOTINFO:
				pInfo->nValueSize = sizeof(A_BLOB_CHARGER_BOOTINFO);
				break;
			case BLOB_TYPE_UPDATE_FIRMWARE_CMD:
				pInfo->nValueSize = sizeof(A_BLOB_UPDATEFIRMWARE_CMD);
				break;
			case BLOB_TYPE_CHARGER_SPECIFIC_SETTING:
				pInfo->nValueSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
				break;
			case BLOB_TYPE_UPLOAD_DIAGNOSTIC:
				pInfo->nValueSize = sizeof(A_BLOB_UPLOADDIAGN_CMD);
				break;
			case BLOB_TYPE_MANUALCTRL_RECT: case BLOB_TYPE_MANUALCTRL_RECT_SCADA:
				pInfo->nValueSize = sizeof(A_BLOB_MANUALCTRL_RECT_CMD);
				break;
			//case BLOB_TYPE_BATT_MGMT_PLAN:
			//	pInfo->nValueSize = sizeof(A_BLOB_BATT_MGMT_PLAN);
			//	break;
			case BLOB_TYPE_ES_RUNDATA:
				pInfo->nValueSize = sizeof(A_BLOB_ES_RUN_DATA);
				break;
			case BLOB_TYPE_ES_BACKEND_CFG:
				pInfo->nValueSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				break;
			case BLOB_TYPE_SINGLE_STBATT_RUNINFO:
				pInfo->nValueSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
				break;	
			case BLOB_TYPE_GRP_PM_DATAINFO:
				pInfo->nValueSize = sizeof(A_BLOB_RECT_GRP_DATA);
				break;
			case BLOB_TYPE_GROUP_STBATT_RUNINFO:
				pInfo->nValueSize = sizeof(A_BLOB_GROUP_STBATT_RUNINFO);
				break;
			case BLOB_TYPE_ES_STATISTICDATA:
				pInfo->nValueSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
				break;
			case BLOB_TYPE_BRANCH_GRP_RECT_DATA:
				pInfo->nValueSize = sizeof(A_BLOB_BRANCH_GRP_RECTDATA);
				break;
			case BLOB_TYPE_SINGLE_RECT_DATAINFO:
				pInfo->nValueSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
				break;
			case BLOB_TYPE_BRANCH_NODE_CALC_DATA:
				pInfo->nValueSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
				break;
			case BLOB_TYPE_RECTGRP_AUTOCTRLCMD:
				pInfo->nValueSize = sizeof(A_BLOB_GRP_RECT_AUTOCTRL_CMD);
				break;
			case BLOB_TYPE_EMS_CONFIG_PLAN:
				pInfo->nValueSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
				break;
			case BLOB_TYPE_IFYBMS_DATAINFO:
				pInfo->nValueSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
				break;
			case BLOB_TYPE_BATTCAB_IMEU2_V20:
				pInfo->nValueSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);
				break;
			case BLOB_TYPE_CCU2PCSCAN_MSG:
				pInfo->nValueSize = sizeof(A_BLOB_CCU2PCSCAN_MSG);
				break;
			case BLOB_TYPE_BMS_TESTMODE_SET:
				pInfo->nValueSize = sizeof(A_BLOB_BMS_TESTMODE_SET);
				break;
			case BLOB_TYPE_SEMS_CLIENTNFO:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CLIENTINFO);
				break;
			case BLOB_TYPE_SEMS_CLIENTNFO_SUM:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
				break;
			case BLOB_TYPE_SEMS2SlAVE_6330:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS2SLAVE_6330);
				break;
			case BLOB_TYPE_PCS_ONGRID_CTRL:
				pInfo->nValueSize = sizeof(A_BLOB_CTRL_PCS_ONGRID_CMD);
				break;
			case BLOB_TYPE_PCS_OFFGRID_CTRL:
				pInfo->nValueSize = sizeof(A_BLOB_CTRL_PCS_OFFGRID_CMD);
				break;
			case BLOB_TYPE_KEEPALIVEDINFO:
				pInfo->nValueSize = sizeof(A_BLOB_KEEPALIVEDINFO);
				break;
			case BLOB_TYPE_SEMS_A6_USYS:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_A6_USYS);
				break;
			case BLOB_TYPE_PV_INVERTER_DATA:
				pInfo->nValueSize = sizeof(A_BLOB_PV_INVERTER_DATA);
				break;
			case BLOB_TYPE_PV_INVERTER_DATA_TOTAL:
				pInfo->nValueSize = sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL);
				break;
			case BLOB_TYPE_PV_SRTTING_BY_AEMS:
				pInfo->nValueSize = sizeof(A_BLOB_PV_CTRL_BY_AEMS);
				break;
			case BLOB_TYPE_CMD155: // LJH250428 cmd155
				pInfo->nValueSize = sizeof(A_BLOB_CMD155);
				break;
			case BLOB_TYPE_CMD157: // LJH250520 cmd157
				pInfo->nValueSize = sizeof(A_BLOB_CMD157);
				break;
			case BLOB_TYPE_SEMS_CMD101:// KYA20250802
				pInfo->nValueSize = sizeof(A_BLOB_CMD101);
				break;
			case BLOB_TYPE_SEMS_CMD102:
				pInfo->nValueSize = sizeof(A_BLOB_CMD102);
				break;
			case BLOB_TYPE_SEMS_CMD103:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD103);
				break;
			case BLOB_TYPE_SEMS_CMD106:
				pInfo->nValueSize = sizeof(A_BLOB_CMD106);
				break;
			case BLOB_TYPE_SEMS_CMD108:
				pInfo->nValueSize = sizeof(A_BLOB_CMD108);
				break;
			case BLOB_TYPE_SEMS_CMD104:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD104);
				break;
			case BLOB_TYPE_SEMS_CMD109:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD109);
				break;
			case BLOB_TYPE_SEMS_CMD110:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD110);
				break;
			case BLOB_TYPE_SEMS_CMD202:
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD202);
				break;
			case BLOB_TYPE_DEVINSTALLCONFIG:
				pInfo->nValueSize = sizeof(A_BLOB_DEVINSTALLCONFIG);
				break;
			// edit by Kuangyue 20251112
			case BLOB_TYPE_SEMS_CMD902:	
				pInfo->nValueSize = sizeof(A_BLOB_SEMS_CMD109);
				break;
			case BLOB_TYPE_CELL_BARCODE:	
				pInfo->nValueSize = sizeof(A_BLOB_CELL_BARCODE);
				break;
			case BLOB_TYPE_ENERGY_STAT:
				pInfo->nValueSize = sizeof(A_BLOB_ENERGY_STAT);
				break;
			case BLOB_TYPE_STATUS_STAT:
				pInfo->nValueSize = sizeof(A_BLOB_STATUS_STAT);
				break;
			case BLOB_TYPE_DIAG_STAT:
				pInfo->nValueSize = sizeof(A_BLOB_DIAG_STAT);
				break;
			default:
				MakeErrorMsg(ppszErrMsg, "Param[%d]-->Wrong Blob type!", pInfo->nParamId);
				return FALSE;
			}
			pInfo->nPrecision = nParseSize; //reuse nPrecision as type
		}
		TRACE("Parsed Blob or string size = %d.\n", pInfo->nValueSize);
	}
	else if(pInfo->emValueType == VALUE_TYPE_ENUM)
	{
		pInfo->nEnumTotal = GetSplitStringCount(szSrc, COMMA);
		if(pInfo->nEnumTotal < 1)
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Enum Value Range < 1!", pInfo->nParamId);
			return FALSE;
		}
		pInfo->pEnumList = NEW(PARAM_ENUM_ITEM, pInfo->nEnumTotal);
		int iCurIndex = 0;
		char *pField;
		int nMaxVal = -99999;
		int nMinVal = 99999;
		int nVal = 0;
		int nRes = 0;
		char* pSubField;
		char* pSubSubFeild;
		do 
		{
			szSrc = SplitString(szSrc, &pField, COMMA);
			if ((*pField) < '0' || (*pField) > '9')
			{
				MakeErrorMsg(ppszErrMsg, "Param[%d]-->Enum[%d] ValueText[%s] INVALID!", pInfo->nParamId, iCurIndex + 1, szSrc);
				return FALSE;
			}
			//TRACE("\npField = [%s]", pField);
			nVal = atoi(pField);
			pInfo->pEnumList[iCurIndex].nEnumVal = nVal;
			pSubField = SplitString(pField, &pSubSubFeild, '(');
			if((pSubSubFeild == pSubField) || (((*pSubField) < '0' || (*pSubField) > '9')))
			{
				//TRACE("\npSubSubFeild:%s, pSubField:%s, pField:%s\n", pSubSubFeild, pSubField, pField);
				MakeErrorMsg(ppszErrMsg, "Param[%d]-->Enum[%d] ResText[%s] INVALID!", pInfo->nParamId, iCurIndex + 1, pSubField);
				return FALSE;
			}
			nRes = atoi(pSubField);
			pInfo->pEnumList[iCurIndex].pEnumRes = GetRes_ById(nRes);
			if(pInfo->pEnumList[iCurIndex].pEnumRes == NULL)
			{
				MakeErrorMsg(ppszErrMsg, "Param[%d]-->Enum[%d] ResVal[%d] INVALID!", pInfo->nParamId, iCurIndex + 1, nRes);
				return FALSE;
			}
			//else
			//{
			//	TRACE("\n----Found EnumItem[%d, %s]------\n", pInfo->pEnumList[iCurIndex].nEnumVal,
			//		pInfo->pEnumList[iCurIndex].pEnumRes->szName);
			//}

			if(nMaxVal < nVal)
			{
				nMaxVal = nVal;
			}
			if(nMinVal > nVal)
			{
				nMinVal = nVal;
			}

			iCurIndex++;

		} while (iCurIndex < pInfo->nEnumTotal);

		pInfo->fMaxLimit = (float)nMaxVal;
		pInfo->fMinLimit = (float)nMinVal;
	}
	else
	{
		int nCount = GetSplitStringCount(szSrc, SLASH);
		if(nCount != 2)
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->VarValue Range Length must be 2!", pInfo->nParamId);
			return FALSE;
		}

		char *pField;
		szSrc = SplitString(szSrc, &pField, SLASH);
		if(((*pField) < '0' || (*pField) > '9') && ((*pField) != '-'))
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Min Range INVALID!", pInfo->nParamId);
			return FALSE;
		}
		pInfo->fMinLimit = (float)atof(pField);
		SplitString(szSrc, &pField, SLASH);
		if(((*pField) < '0' || (*pField) > '9') && ((*pField) != '-'))
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Max Range INVALID!", pInfo->nParamId);
			return FALSE;
		}
		pInfo->fMaxLimit = (float)atof(pField);
		if(pInfo->fMinLimit > pInfo->fMaxLimit)
		{
			MakeErrorMsg(ppszErrMsg, "Param[%d]-->Max Range < Min Range!", pInfo->nParamId);
			return FALSE;
		}
	}
	//TRACE("Parsed fMin = %.1f, fMax = %.1f \n", pInfo->fMinLimit, pInfo->fMaxLimit);
	return TRUE;
}


CFG_ERRCODE Cfg_Parse_STUParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	STATUS_PARAM_INFO* pParam = (STATUS_PARAM_INFO*)pRowItem;
	memset(pParam, 0, sizeof(STATUS_PARAM_INFO));

	pParam->info.emParamType = PARAM_TYPE_STATUS;
	pParam->info.nParamId = sqlite3_column_int(pStmt, 0);

	int nDevModelID = sqlite3_column_int(pStmt, 5);
	//TRACE("Now begin devModel.\n");
	DEVICE_MODEL_INFO* pDevModel = GetDevModel_ById(nDevModelID);
	if(pDevModel == NULL)
	{
		TRACE("Can not find such devModel %d.\n", nDevModelID);
		MakeErrorMsg(ppszErrMsg, "Found an INVALID DevModel[%d] for Param[%d]!", nDevModelID, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}
	pParam->info.pBelonged_DevModel = pDevModel;

	//name
	if(sqlite3_column_type(pStmt, 2) == SQLITE_NULL)
	{
		pParam->info.pRes_Name = NULL;
	}
	else
	{
		pParam->info.pRes_Name = GetRes_ById(sqlite3_column_int(pStmt, 2));
	}

	//unit
	if(sqlite3_column_type(pStmt, 4) == SQLITE_NULL)
	{
		pParam->info.pRes_Unit = NULL;
	}
	else
	{
		pParam->info.pRes_Unit = GetRes_ById(sqlite3_column_int(pStmt, 4));
	}

	//Data Source
	const UCHAR* szDataSrc = sqlite3_column_text(pStmt, 6);
	if(strcmp(szDataSrc, "SA") == 0)
	{
		pParam->emDataSrc = PARAM_SRC_SA;
	}
	else if(strcmp(szDataSrc, "CA") == 0)
	{
		pParam->emDataSrc = PARAM_SRC_CA;
	}
	else if(strcmp(szDataSrc, "LS") == 0)
	{
		pParam->emDataSrc = PARAM_SRC_LS;
	}
	else
	{
		MakeErrorMsg(ppszErrMsg, "DataSrc[%s] is INVALID for Param[%d]!", szDataSrc, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}
	//Value Type
	const UCHAR* szValTp = sqlite3_column_text(pStmt, 7);
	if(!ParseValueType_FromString(szValTp, &pParam->info))
	{
		MakeErrorMsg(ppszErrMsg, "ValueType[%s] is INVALID for Param[%d]!", szValTp, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}

	pParam->info.nPrecision = 0;
	if(sqlite3_column_type(pStmt, 8) != SQLITE_NULL)
	{
		pParam->info.nPrecision = sqlite3_column_int(pStmt, 8);
	}

	//Value Range
	const UCHAR* szValRange = sqlite3_column_text(pStmt, 9);
	if(!PargeValueRange_FromString(szValRange, &pParam->info, ppszErrMsg))
	{
		//MakeErrorMsg(ppszErrMsg, "ValueRange[%s] is INVALID for STUParam[%d]!", szValRange, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}
	
	//Equal Expression
	//TRACE("Now begin parse EqualExpressions.\n");
	if(pParam->emDataSrc == PARAM_SRC_CA)
	{
		const UCHAR* szEqualExp = sqlite3_column_text(pStmt, 10);
		if(szEqualExp == NULL)
		{
			MakeErrorMsg(ppszErrMsg, "EqualExpression CANNOT be empty for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
		else
		{
			if(ExpressionParser(szEqualExp, &pParam->equalExpression) != CFG_ERR_OK)
			{
				MakeErrorMsg(ppszErrMsg, "EqualExpression is INVALID for Param[%d]!", pParam->info.nParamId);
				return CFG_ERR_PARSER;
			}
		}
	}

	//Display Expression
	//TRACE("Now begin parse DispExpressions.\n");
	const UCHAR* szDispExp = sqlite3_column_text(pStmt, 11);
	pParam->info.bDisableDisplay = FALSE;
	if(szDispExp == NULL)
	{
		pParam->info.displayExpression.nTotal = 0;
	}
	else
	{
		if(strcmp(szDispExp, "NOT") == 0)
		{
			TRACE("The param will NEVER be displayed! \n");
			pParam->info.bDisableDisplay = TRUE;
		}
		else if(ExpressionParser(szDispExp, &pParam->info.displayExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "DisplayExpression is INVALID for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}

	pParam->info.nDisplayPriority = 255; //Jimmy 20190803:max means lower prio
	if(sqlite3_column_type(pStmt, 12) != SQLITE_NULL)
	{
		pParam->info.nDisplayPriority = sqlite3_column_int(pStmt, 12);
	}

	pParam->info.uMask_AuthorityLevel = 0; //Jimmy 20190814:0x00 means R/W for all level
	if(sqlite3_column_type(pStmt, 13) != SQLITE_NULL)
	{
		pParam->info.uMask_AuthorityLevel = (UINT)sqlite3_column_int(pStmt, 13);
	}
	//TRACEX("----AuthLevel = %d----\n", pParam->info.uMask_AuthorityLevel);
	if(sqlite3_column_type(pStmt, 14) != SQLITE_NULL)
	{
		pParam->info.Web_1 = (UINT)sqlite3_column_int(pStmt, 14);
	}
	if(sqlite3_column_type(pStmt, 15) != SQLITE_NULL)
	{
		pParam->info.Web_2 = (UINT)sqlite3_column_int(pStmt, 15);
	}

	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_CTRParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	CONTROL_PARAM_INFO* pParam = (CONTROL_PARAM_INFO*)pRowItem;
	memset(pParam, 0, sizeof(CONTROL_PARAM_INFO));

	pParam->info.emParamType = PARAM_TYPE_CONTROL;
	pParam->info.nParamId = sqlite3_column_int(pStmt, 0);
	int nDevModelID = sqlite3_column_int(pStmt, 5);

	//TRACE("Now begin devModel.\n");
	DEVICE_MODEL_INFO* pDevModel = GetDevModel_ById(nDevModelID);
	if(pDevModel == NULL)
	{
		TRACE("Can not find such devModel %d.\n", nDevModelID);
		MakeErrorMsg(ppszErrMsg, "Found an INVALID DevModel[%d] for Param[%d]!", nDevModelID, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}
	pParam->info.pBelonged_DevModel = pDevModel;

	//name
	if(sqlite3_column_type(pStmt, 2) == SQLITE_NULL)
	{
		pParam->info.pRes_Name = NULL;
	}
	else
	{
		pParam->info.pRes_Name = GetRes_ById(sqlite3_column_int(pStmt, 2));
	}

	//unit
	if(sqlite3_column_type(pStmt, 4) == SQLITE_NULL)
	{
		pParam->info.pRes_Unit = NULL;
	}
	else
	{
		pParam->info.pRes_Unit = GetRes_ById(sqlite3_column_int(pStmt, 4));
	}

	//Value Type
	const UCHAR* szValTp = sqlite3_column_text(pStmt, 6);
	if(!ParseValueType_FromString(szValTp, &pParam->info))
	{
		MakeErrorMsg(ppszErrMsg, "ValueType[%s] is INVALID for Param[%d]!", szValTp, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}

	pParam->info.nPrecision = 0;
	if(sqlite3_column_type(pStmt, 7) != SQLITE_NULL)
	{
		pParam->info.nPrecision = sqlite3_column_int(pStmt, 7);
	}

	//Value Range
	const UCHAR* szValRange = sqlite3_column_text(pStmt, 8);
	if(!PargeValueRange_FromString(szValRange, &pParam->info, ppszErrMsg))
	{
		//MakeErrorMsg(ppszErrMsg, "ValueRange[%s] is INVALID for Param[%d]!", szValRange, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}

	//Controllable Expression
	//TRACE("Now begin parse CtrlExpressions.\n");
	const UCHAR* szCtrlExp = sqlite3_column_text(pStmt, 9);
	if(szCtrlExp == NULL)
	{
		pParam->controlableExpression.nTotal = 0;
	}
	else
	{
		if(ExpressionParser(szCtrlExp, &pParam->controlableExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "CtrlExpression is INVALID for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}

	//Display Expression
	//TRACE("Now begin parse DispExpressions.\n");
	const UCHAR* szDispExp = sqlite3_column_text(pStmt, 10);
	pParam->info.bDisableDisplay = FALSE;
	if(szDispExp == NULL)
	{
		pParam->info.displayExpression.nTotal = 0;
	}
	else
	{
		if(strcmp(szDispExp, "NOT") == 0)
		{
			TRACE("The param will NEVER be displayed! \n");
			pParam->info.bDisableDisplay = TRUE;
		}
		else if(ExpressionParser(szDispExp, &pParam->info.displayExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "DisplayExpression is INVALID for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}

	pParam->info.nDisplayPriority = 255; //Jimmy 20190803:max means lower prio
	if(sqlite3_column_type(pStmt, 11) != SQLITE_NULL)
	{
		pParam->info.nDisplayPriority = sqlite3_column_int(pStmt, 11);
	}

	pParam->info.uMask_AuthorityLevel = 0; //Jimmy 20190814:0x00 means R/W for all level
	if(sqlite3_column_type(pStmt, 12) != SQLITE_NULL)
	{
		pParam->info.uMask_AuthorityLevel = (UINT)sqlite3_column_int(pStmt, 12);
	}
	if(sqlite3_column_type(pStmt, 13) != SQLITE_NULL)
	{
		pParam->info.Web_1 = (UINT)sqlite3_column_int(pStmt, 13);
	}
	if(sqlite3_column_type(pStmt, 14) != SQLITE_NULL)
	{
		pParam->info.Web_2 = (UINT)sqlite3_column_int(pStmt, 14);
	}
	return CFG_ERR_OK;
}

static BOOL Convert2Float(UCHAR* szSrc, float* pfOutVal)
{
	if(szSrc == NULL)
	{
		return FALSE;
	}
	int nLen = strlen(szSrc);
	if(nLen < 1)
	{
		return FALSE;
	}

	if(((szSrc[0] > '9') || (szSrc[0] < '0')) && (szSrc[0] != '-'))
	{
		return FALSE;
	}

	int n;
	for(n = 1; n < nLen; n++)
	{
		if(((szSrc[n] > '9') || (szSrc[n] < '0')) && (szSrc[n] != '.'))
		{
			return FALSE;
		}
	}
	*pfOutVal = atof(szSrc);
	return TRUE;
}

static BOOL Convert2Int(UCHAR* szSrc, ULONG* pIntVal)
{
	if(szSrc == NULL)
	{
		return FALSE;
	}
	int nLen = strlen(szSrc);
	if(nLen < 1)
	{
		return FALSE;
	}

	if(((szSrc[0] > '9') || (szSrc[0] < '0')) && (szSrc[0] != '-'))
	{
		return FALSE;
	}

	int n;
	for(n = 1; n < nLen; n++)
	{
		if(((szSrc[n] > '9') || (szSrc[n] < '0')))
		{
			return FALSE;
		}
	}
	*pIntVal = atoi(szSrc);
	return TRUE;
}

CFG_ERRCODE Cfg_Parse_SETParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	SETTING_PARAM_INFO* pParam = (SETTING_PARAM_INFO*)pRowItem;
	memset(pParam, 0, sizeof(SETTING_PARAM_INFO));

	pParam->info.emParamType = PARAM_TYPE_SETTING;
	pParam->info.nParamId = sqlite3_column_int(pStmt, 0);

	int nDevModelID = sqlite3_column_int(pStmt, 5);
	//TRACE("Now begin devModel.\n");
	DEVICE_MODEL_INFO* pDevModel = GetDevModel_ById(nDevModelID);
	if(pDevModel == NULL)
	{
		TRACE("Can not find such devModel %d.\n", nDevModelID);
		MakeErrorMsg(ppszErrMsg, "Found an INVALID DevModel[%d] for Param[%d]!", nDevModelID, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}
	pParam->info.pBelonged_DevModel = pDevModel;

	//name
	if(sqlite3_column_type(pStmt, 2) == SQLITE_NULL)
	{
		pParam->info.pRes_Name = NULL;
	}
	else
	{
		pParam->info.pRes_Name = GetRes_ById(sqlite3_column_int(pStmt, 2));
	}

	//unit
	if(sqlite3_column_type(pStmt, 4) == SQLITE_NULL)
	{
		pParam->info.pRes_Unit = NULL;
	}
	else
	{
		pParam->info.pRes_Unit = GetRes_ById(sqlite3_column_int(pStmt, 4));
	}

	//Value Type
	const UCHAR* szValTp = sqlite3_column_text(pStmt, 6);
	if(!ParseValueType_FromString(szValTp, &pParam->info))
	{
		MakeErrorMsg(ppszErrMsg, "ValueType[%s] is INVALID for Param[%d]!", szValTp, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}

	pParam->info.nPrecision = 0;
	if(sqlite3_column_type(pStmt, 7) != SQLITE_NULL)
	{
		pParam->info.nPrecision = sqlite3_column_int(pStmt, 7);
	}

	//Value Range
	const UCHAR* szValRange = sqlite3_column_text(pStmt, 8);
	if(!PargeValueRange_FromString(szValRange, &pParam->info, ppszErrMsg))
	{
		//MakeErrorMsg(ppszErrMsg, "ValueRange[%s] is INVALID for Param[%d]!", szValRange, pParam->info.nParamId);
		return CFG_ERR_PARSER;
	}

	//Default Value
	const UCHAR* szDefaultValue = sqlite3_column_text(pStmt, 9);
	if(pParam->info.emValueType == VALUE_TYPE_FLOAT)
	{
		if(!Convert2Float(szDefaultValue, &pParam->varValDefault.fValue))
		{
			MakeErrorMsg(ppszErrMsg, "Default value[%s] is INVALID for float Param[%d]!", szDefaultValue, pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}
	else if((pParam->info.emValueType != VALUE_TYPE_STRING) && (pParam->info.emValueType != VALUE_TYPE_BLOB))
	{
		if(!Convert2Int(szDefaultValue, &pParam->varValDefault.lValue))
		{
			MakeErrorMsg(ppszErrMsg, "Default value[%s] is INVALID for int Param[%d]!", szDefaultValue, pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}
	else if(pParam->info.emValueType == VALUE_TYPE_STRING) //blob param has no default
	{
		pParam->varValDefault.blobValue = NEW(BYTE, pParam->info.nValueSize);
		if(pParam->varValDefault.blobValue == NULL)
		{
			MakeErrorMsg(ppszErrMsg, "Can NOT allocate memory for default text value of Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}

		memset(pParam->varValDefault.blobValue, 0, pParam->info.nValueSize);
		if(szDefaultValue != NULL)
		{
			memcpy(pParam->varValDefault.blobValue, szDefaultValue, MIN(strlen(szDefaultValue), pParam->info.nValueSize - 1));
		}
	}

	//Is need sync Sampler
	pParam->bNeedSyncSampler = (BOOL)sqlite3_column_int(pStmt, 10);
	//TRACE("NeedSync = %d.\n", pParam->bNeedSyncSampler);

	//Is need Save
	pParam->bNeedSave = (BOOL)sqlite3_column_int(pStmt, 11);
	//TRACE("NeedSave = %d.\n", pParam->bNeedSave);

	//Settable Expression
	//TRACE("Now begin parse SetExpressions.\n");
	const UCHAR* szSetExp = sqlite3_column_text(pStmt, 12);
	if(szSetExp == NULL)
	{
		pParam->settableExpression.nTotal = 0;
	}
	else
	{
		if(ExpressionParser(szSetExp, &pParam->settableExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "SetExpression is INVALID for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}

	//Display Expression
	//TRACE("Now begin parse DispExpressions.\n");
	const UCHAR* szDispExp = sqlite3_column_text(pStmt, 13);
	pParam->info.bDisableDisplay = FALSE;
	if(szDispExp == NULL)
	{
		pParam->info.displayExpression.nTotal = 0;
	}
	else
	{
		if(strcmp(szDispExp, "NOT") == 0)
		{
			TRACE("The param will NEVER be displayed! \n");
			pParam->info.bDisableDisplay = TRUE;
		}
		else if(ExpressionParser(szDispExp, &pParam->info.displayExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "DisplayExpression is INVALID for Param[%d]!", pParam->info.nParamId);
			return CFG_ERR_PARSER;
		}
	}

	pParam->info.nDisplayPriority = 255; //Jimmy 20190803:max means lower prio
	if(sqlite3_column_type(pStmt, 14) != SQLITE_NULL)
	{
		pParam->info.nDisplayPriority = sqlite3_column_int(pStmt, 14);
	}

	pParam->info.uMask_AuthorityLevel = 0; //Jimmy 20190814:0x00 means R/W for all level
	if(sqlite3_column_type(pStmt, 15) != SQLITE_NULL)
	{
		pParam->info.uMask_AuthorityLevel = (UINT)sqlite3_column_int(pStmt, 15);
	}
	if(sqlite3_column_type(pStmt, 16) != SQLITE_NULL)
	{
		pParam->info.Web_1 = (UINT)sqlite3_column_int(pStmt, 16);
	}
	if(sqlite3_column_type(pStmt, 17) != SQLITE_NULL)
	{
		pParam->info.Web_2 = (UINT)sqlite3_column_int(pStmt, 17);
	}

	return CFG_ERR_OK;
}

STATUS_PARAM_INFO* GetSTParamInfo_ById(int nParamId)
{
	int n;
	STATUS_PARAM_INFO* p = s_pCharger->info.statusParams_info.p;
	for (n = 0; n < s_pCharger->info.statusParams_info.nTotal; n++, p++)
	{
		if(p->info.nParamId == nParamId)
		{
			return p;
		}
	}

	return NULL;
}


CFG_ERRCODE Cfg_Parse_Alarm(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	ALARM_INFO* pAlarm = (ALARM_INFO*)pRowItem;
	memset(pAlarm, 0, sizeof(ALARM_INFO));

	pAlarm->nAlarmId = sqlite3_column_int(pStmt, 0);

	int nDevModelID = sqlite3_column_int(pStmt, 3);
	//TRACE("Now begin devModel.\n");
	DEVICE_MODEL_INFO* pDevModel = GetDevModel_ById(nDevModelID);
	if(pDevModel == NULL)
	{
		TRACE("Can not find such devModel %d.\n", nDevModelID);
		MakeErrorMsg(ppszErrMsg, "Found an INVALID DevModel[%d] for Alarm[%d]!", nDevModelID, pAlarm->nAlarmId);
		return CFG_ERR_PARSER;
	}
	pAlarm->pBelonged_DevModel = pDevModel;

	//name
	if(sqlite3_column_type(pStmt, 2) == SQLITE_NULL)
	{
		pAlarm->pRes_Name = NULL;
	}
	else
	{
		pAlarm->pRes_Name = GetRes_ById(sqlite3_column_int(pStmt, 2));
	}
	
	//Alarm related status param
	pAlarm->nExValue_ParamId = -1;
	if(sqlite3_column_type(pStmt, 4) != SQLITE_NULL)
	{
		int nRelParm = sqlite3_column_int(pStmt, 4);
		STATUS_PARAM_INFO* pPar = GetSTParamInfo_ById(nRelParm);
		if(pPar == NULL)
		{
			MakeErrorMsg(ppszErrMsg, "Can NOT find such RelatedParam for Alarm[%d]!", pAlarm->nAlarmId);
			return CFG_ERR_PARSER;
		}
		else if(pPar->info.pBelonged_DevModel != pDevModel)
		{
			MakeErrorMsg(ppszErrMsg, "RelatedParam shall belonged to same device, for Alarm[%d]!", pAlarm->nAlarmId);
			return CFG_ERR_PARSER;
		}
		pAlarm->nExValue_ParamId = nRelParm;
	}

	//Alarm Level
	const UCHAR* szAlarmLevel = sqlite3_column_text(pStmt, 5);
	if(strcmp(szAlarmLevel, "NA") == 0)
	{
		pAlarm->emLevel = ALARM_LEVEL_NONE;
	}
	else if(strcmp(szAlarmLevel, "OA") == 0)
	{
		pAlarm->emLevel = ALARM_LEVEL_OBSERVATION;
	}
	else if(strcmp(szAlarmLevel, "MA") == 0)
	{
		pAlarm->emLevel = ALARM_LEVEL_MAJOR;
	}
	else if(strcmp(szAlarmLevel, "CA") == 0)
	{
		pAlarm->emLevel = ALARM_LEVEL_CRITICAL;
	}
	else
	{
		MakeErrorMsg(ppszErrMsg, "Alarm Level INVALID for Alarm[%d]!", pAlarm->nAlarmId);
		return CFG_ERR_PARSER;
	}

	//Alarm Expression
	//TRACE("Now begin parse AlarmExpression.\n");
	const UCHAR* szAlarmExp = sqlite3_column_text(pStmt, 6);
	if(szAlarmExp == NULL)
	{
		pAlarm->alarmCondExpression.nTotal = 0;
	}
	else
	{
		if(ExpressionParser(szAlarmExp, &pAlarm->alarmCondExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "AlarmExpression is INVALID for Alarm[%d]!", pAlarm->nAlarmId);
			return CFG_ERR_PARSER;
		}
	}

	//AlarmSuppression Expression
	//TRACE("Now begin parse AlarmMaskExpression.\n");
	const UCHAR* szAlarmMskExp = sqlite3_column_text(pStmt, 7);
	if(szAlarmMskExp == NULL)
	{
		pAlarm->alarmMaskExpression.nTotal = 0;
	}
	else
	{
		if(ExpressionParser(szAlarmMskExp, &pAlarm->alarmMaskExpression) != CFG_ERR_OK)
		{
			MakeErrorMsg(ppszErrMsg, "AlarmMaskExpression is INVALID for Alarm[%d]!", pAlarm->nAlarmId);
			return CFG_ERR_PARSER;
		}
	}

	return CFG_ERR_OK;
}

static COMMPORT* GetCommPortById(int nPortId)
{
	int n;
	COMMPORT* p = s_pCharger->commports.p;
	for(n = 0; n < s_pCharger->commports.nTotal; n++, p++)
	{
		//TRACE("Total = %d, port = %s,%d, nFound = %d.\n",s_pCharger->commports.nTotal, 
		//	p->info.szPortName,
		//	p->info.nCommPort, nPortId);
		if(p->info.nCommPort == nPortId)
		{
			return p;
		}
	}
	return NULL;
}

CFG_ERRCODE Cfg_Parse_Sampler(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	SAMPLER* pSampler = (SAMPLER*)pRowItem;
	memset(pSampler, 0, sizeof(SAMPLER));

	pSampler->info.emModuleType = MODULE_TYPE_SAMPLER;
	pSampler->info.nSamplerID = sqlite3_column_int(pStmt, 0);

	const UCHAR *szName = sqlite3_column_text(pStmt, 1);
	if(szName != NULL)
	{
		//TRACE("Now begin szName[%s].\n", szName);
		memcpy(pSampler->info.szSamplerName, szName, MIN(32, sqlite3_column_bytes(pStmt, 1)));
	}

	COMMPORT* pPort = GetCommPortById(sqlite3_column_int(pStmt, 2));
	if(pPort == NULL)
	{
		MakeErrorMsg(ppszErrMsg, "INVALID Comport for Sampler[%d]!", pSampler->info.nSamplerID);
		return CFG_ERR_PARSER;
	}
	pSampler->pUsedCommPort = pPort;

	const UCHAR *szCommSetting = sqlite3_column_text(pStmt, 3);
	if(szCommSetting != NULL)
	{
		memcpy(pSampler->info.szSamplerSetting, szCommSetting, MIN(64, sqlite3_column_bytes(pStmt, 3)));
	}

	pSampler->info.nSamplePeriod_mSec = sqlite3_column_int(pStmt, 4);

	const UCHAR *szDriver = sqlite3_column_text(pStmt, 5);
	if(szDriver != NULL)
	{
		memcpy(pSampler->info.szSamplerDriver, szDriver, MIN(32, sqlite3_column_bytes(pStmt, 5)));
	}

	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_Reporter(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	REPORTER* pReporter = (REPORTER*)pRowItem;
	memset(pReporter, 0, sizeof(REPORTER));

	pReporter->info.emModuleType = MODULE_TYPE_REPORTER;
	pReporter->info.nReporterID= sqlite3_column_int(pStmt, 0);

	const UCHAR *szName = sqlite3_column_text(pStmt, 1);
	if(szName != NULL)
	{
		//TRACE("Now begin szName[%s].\n", szName);
		memcpy(pReporter->info.szReporterName, szName, MIN(32, sqlite3_column_bytes(pStmt, 1)));
	}

	COMMPORT* pPort = GetCommPortById(sqlite3_column_int(pStmt, 2));
	if(pPort == NULL)
	{
		MakeErrorMsg(ppszErrMsg, "INVALID Comport for Reporter[%d]!", pReporter->info.nReporterID);
		return CFG_ERR_PARSER;
	}
	pReporter->pUsedCommPort= pPort;

	const UCHAR *szCommSetting = sqlite3_column_text(pStmt, 3);
	if(szCommSetting != NULL)
	{
		memcpy(pReporter->info.szReporterSetting, szCommSetting, MIN(64, sqlite3_column_bytes(pStmt, 3)));
	}

	const UCHAR *szDriver = sqlite3_column_text(pStmt, 4);
	if(szDriver != NULL)
	{
		memcpy(pReporter->info.szReporterDriver, szDriver, MIN(32, sqlite3_column_bytes(pStmt, 4)));
	}

	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_CommPort(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	COMMPORT* pComport = (COMMPORT*)pRowItem;
	memset(pComport, 0, sizeof(COMMPORT));

	pComport->info.nCommPort = sqlite3_column_int(pStmt, 0);

	const UCHAR *szName = sqlite3_column_text(pStmt, 1);
	if(szName != NULL)
	{
		//TRACE("Now begin szName[%s], port = %d.\n", szName, pComport->info.nCommPort);
		memcpy(pComport->info.szPortName, szName, MIN(31, sqlite3_column_bytes(pStmt, 1)));
	}

	pComport->info.bIsExlusive = (BOOL)sqlite3_column_int(pStmt, 2);

	const UCHAR *szPortAttri = sqlite3_column_text(pStmt, 3);
	if(szPortAttri != NULL)
	{
		memcpy(pComport->info.szPortAttri, szPortAttri, MIN(63, sqlite3_column_bytes(pStmt, 3)));
	}

	const UCHAR *szDriver = sqlite3_column_text(pStmt, 4);
	if(szDriver != NULL)
	{
		//TRACE("driver = %s, leng = %d.\n", szDriver, sqlite3_column_bytes(pStmt, 4));
		memcpy(pComport->info.szPortDriver, szDriver, MIN(63, sqlite3_column_bytes(pStmt, 4)));
	}

	return CFG_ERR_OK;
}

SAMPLER* GetSamplerById(int nSamplerId)
{
	int n;
	//TRACE("\n-----------Begin Finding SamplerId%d, Total = %d--------\n", nSamplerId, s_pCharger->samplers.nTotal);
	SAMPLER* p = s_pCharger->samplers.p;
	for(n = 0; n < s_pCharger->samplers.nTotal; n++, p++)
	{
		if(p->info.nSamplerID == nSamplerId)
		{
			//TRACE("-------------End of Finding, FOUND!-------\n");
			return p;
		}
	}
	//TRACE("-------------End of Finding, NOT Found!-------\n");
	return NULL;
}

static UINT GetParamNum_ByBelongedDevModel(PARAM_TYPE tp, DEVICE_MODEL_INFO *pDevMod)
{
	int n;
	UINT nTotal = 0;
	if(tp == PARAM_TYPE_STATUS)
	{
		STATUS_PARAM_INFO* p = s_pCharger->info.statusParams_info.p;
		for(n = 0; n < s_pCharger->info.statusParams_info.nTotal; n++, p++)
		{
			if(p->info.pBelonged_DevModel == pDevMod)
			{
				nTotal++;
			}
		}
	}
	else if(tp == PARAM_TYPE_CONTROL)
	{
		CONTROL_PARAM_INFO* p = s_pCharger->info.controlParams_info.p;
		for(n = 0; n < s_pCharger->info.controlParams_info.nTotal; n++, p++)
		{
			if(p->info.pBelonged_DevModel == pDevMod)
			{
				nTotal++;
			}
		}
	}
	else if(tp == PARAM_TYPE_SETTING)
	{
		SETTING_PARAM_INFO* p = s_pCharger->info.settingParms_info.p;
		for(n = 0; n < s_pCharger->info.settingParms_info.nTotal; n++, p++)
		{
			if(p->info.pBelonged_DevModel == pDevMod)
			{
				nTotal++;
			}
		}
	}

	return nTotal;
}

static BOOL CreateParams_4Device(DEVICE* pDev)
{
	int n;
	PARAMETER* pParam;
	DEVICE_MODEL_INFO *pDevMod = pDev->info.pBelonged_DevModel;

	pDev->params_Status.nTotal =
		GetParamNum_ByBelongedDevModel(PARAM_TYPE_STATUS, pDevMod);

	pDev->params_Control.nTotal =
		GetParamNum_ByBelongedDevModel(PARAM_TYPE_CONTROL, pDevMod);

	pDev->params_Setting.nTotal =
		GetParamNum_ByBelongedDevModel(PARAM_TYPE_SETTING, pDevMod);

	//Create status param
	if(pDev->params_Status.nTotal > 0)
	{
		pDev->params_Status.p = NEW(PARAMETER, pDev->params_Status.nTotal);
		if(pDev->params_Status.p == NULL)
		{
			TRACE("Can not allocate status_params for Device.\n");
			return FALSE;
		}

		STATUS_PARAM_INFO* p = s_pCharger->info.statusParams_info.p;
		pParam = pDev->params_Status.p;
		for(n = 0; n < s_pCharger->info.statusParams_info.nTotal; n++, p++)
		{
			if(p->info.pBelonged_DevModel != pDevMod)
			{
				continue;
			}
			ZERO_POBJS(pParam, 1);
			pParam->pParamInfo = (PARAM_BASIC_INFO*)p;
			pParam->hDevice = (HANDLE)pDev;
			pParam->bIsValueInvalid = TRUE;
			if(p->info.emValueType == VALUE_TYPE_STRING)
			{
				TRACE("Now Create String SAParam[%s], size = %d.\n", (p->info.pRes_Name == NULL) ? "(NA)" : p->info.pRes_Name->szName, p->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p->info.nValueSize);
				pParam->lastValue.blobValue = NEW(BYTE, p->info.nValueSize);
				if((pParam->currValue.blobValue == NULL) || 
					(pParam->lastValue.blobValue == NULL))
				{
					TRACE("Can not create status_params  blob or string value  for Device.\n");
					return FALSE;
				}
				memset(pParam->currValue.blobValue, 0, p->info.nValueSize);
				memset(pParam->lastValue.blobValue, 0, p->info.nValueSize);
			}
			else if(p->info.emValueType == VALUE_TYPE_BLOB)
			{
				TRACE("Now Create Blob SAParam[%s], size = %d.\n", (p->info.pRes_Name == NULL) ? "(NA)" : p->info.pRes_Name->szName, p->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p->info.nValueSize);
				if(pParam->currValue.blobValue == NULL)
				{
					TRACE("Can not create status_params  blob or string value  for Device.\n");
					return FALSE;
				}
				pParam->lastValue.blobValue = NULL; //do not need save last value for blobType
				memset(pParam->currValue.blobValue, 0, p->info.nValueSize);
				*(BLOB_TYPE*)pParam->currValue.blobValue = (BLOB_TYPE)pParam->pParamInfo->nPrecision;
			}
			pParam++;
		}
	}


	//Create control param
	if(pDev->params_Control.nTotal > 0)
	{
		pDev->params_Control.p = NEW(PARAMETER, pDev->params_Control.nTotal);
		if(pDev->params_Control.p == NULL)
		{
			TRACE("Can not allocate status_params for Device.\n");
			return FALSE;
		}

		pParam = pDev->params_Control.p;
		CONTROL_PARAM_INFO* p1 = s_pCharger->info.controlParams_info.p;
		for(n = 0; n < s_pCharger->info.controlParams_info.nTotal; n++, p1++)
		{
			if(p1->info.pBelonged_DevModel != pDevMod)
			{
				continue;
			}
			ZERO_POBJS(pParam, 1);
			pParam->pParamInfo = (PARAM_BASIC_INFO*)p1;
			pParam->hDevice = (HANDLE)pDev;
			pParam->bCtrlSetAble = TRUE;
			pParam->bIsValueInvalid = FALSE;
			if(p1->info.emValueType == VALUE_TYPE_STRING)
			{
				TRACE("Now Create String CLParam[%s], size = %d.\n", (p1->info.pRes_Name == NULL) ? "(NA)" : p1->info.pRes_Name->szName, p1->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p1->info.nValueSize);
				pParam->lastValue.blobValue = NEW(BYTE, p1->info.nValueSize);
				if((pParam->currValue.blobValue == NULL) || 
					(pParam->lastValue.blobValue == NULL))
				{
					TRACE("Can not create control_params  blob or string value  for Device.\n");
					return FALSE;
				}
				memset(pParam->currValue.blobValue, 0, p1->info.nValueSize);
				memset(pParam->lastValue.blobValue, 0, p1->info.nValueSize);
			}
			else if(p1->info.emValueType == VALUE_TYPE_BLOB)
			{
				TRACE("Now Create Blob CLParam[%s], size = %d.\n", (p1->info.pRes_Name == NULL) ? "(NA)" : p1->info.pRes_Name->szName, p1->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p1->info.nValueSize);
				if(pParam->currValue.blobValue == NULL)
				{
					TRACE("Can not create control_params  blob or string value  for Device.\n");
					return FALSE;
				}
				pParam->lastValue.blobValue = NULL;
				memset(pParam->currValue.blobValue, 0, p1->info.nValueSize);
				*(BLOB_TYPE*)pParam->currValue.blobValue = (BLOB_TYPE)pParam->pParamInfo->nPrecision;
			}

			pParam++;
		}
	}

	//Create setting param
	if(pDev->params_Setting.nTotal > 0)
	{
		pDev->params_Setting.p = NEW(PARAMETER, pDev->params_Setting.nTotal);
		if(pDev->params_Setting.p == NULL)
		{
			TRACE("Can not allocate status_params for Device.\n");
			return FALSE;
		}

		pParam = pDev->params_Setting.p;
		SETTING_PARAM_INFO* p2 = s_pCharger->info.settingParms_info.p;
		for(n = 0; n < s_pCharger->info.settingParms_info.nTotal; n++, p2++)
		{
			if(p2->info.pBelonged_DevModel != pDevMod)
			{
				continue;
			}
			ZERO_POBJS(pParam, 1);
			pParam->pParamInfo = (PARAM_BASIC_INFO*)p2;
			pParam->hDevice = (HANDLE)pDev;
			pParam->bCtrlSetAble = TRUE;
			pParam->bIsValueInvalid = FALSE;
			if(p2->info.emValueType == VALUE_TYPE_STRING)
			{
				TRACE("Now Create String STParam[%s], size = %d.\n", (p2->info.pRes_Name == NULL) ? "(NA)" : p2->info.pRes_Name->szName, p2->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p2->info.nValueSize);
				pParam->lastValue.blobValue = NEW(BYTE, p2->info.nValueSize);
				if((pParam->currValue.blobValue == NULL) || 
					(pParam->lastValue.blobValue == NULL))
				{
					TRACE("Can not create setting_params  blob or string value  for Device.\n");
					return FALSE;
				}

				memcpy(pParam->currValue.blobValue, p2->varValDefault.blobValue, p2->info.nValueSize);
				memcpy(pParam->lastValue.blobValue, pParam->currValue.blobValue, p2->info.nValueSize);
			}
			else if(p2->info.emValueType == VALUE_TYPE_BLOB)
			{
				TRACE("Now Create Blob STParam[%s], size = %d.\n", (p2->info.pRes_Name == NULL) ? "(NA)" : p2->info.pRes_Name->szName, p2->info.nValueSize);
				pParam->currValue.blobValue = NEW(BYTE, p2->info.nValueSize);
				if(pParam->currValue.blobValue == NULL)
				{
					TRACE("Can not create setting_params  blob or string value  for Device.\n");
					return FALSE;
				}

				memset(pParam->currValue.blobValue, 0, p2->info.nValueSize);
				pParam->lastValue.blobValue = NULL;
				pParam->bIsValueInvalid = TRUE; //blob has no default value, so shall be set invalid
				*(BLOB_TYPE*)pParam->currValue.blobValue = (BLOB_TYPE)pParam->pParamInfo->nPrecision;
			}
			else
			{
				//restore default value
				pParam->currValue = p2->varValDefault;
				pParam->lastValue = pParam->currValue;
			}

			pParam++;
		}
	}

	return TRUE;
}

static UINT GetAlarmNum_ByBelongedDevModel(DEVICE_MODEL_INFO *pDevMod)
{
	ALARM_INFO* p = s_pCharger->info.alarms_info.p;
	int n;
	UINT nTotal = 0;

	for(n = 0; n < s_pCharger->info.alarms_info.nTotal; n++, p++)
	{
		if(p->pBelonged_DevModel == pDevMod)
		{
			nTotal++;
		}
	}

	return nTotal;
}

static BOOL CreateAlarms_4Device(DEVICE* pDev)
{
	int n;
	DEVICE_MODEL_INFO* pDevMod = pDev->info.pBelonged_DevModel;
	pDev->alarms.nTotal = GetAlarmNum_ByBelongedDevModel(pDevMod);
	pDev->alarms.p = NEW(ALARM, pDev->alarms.nTotal);
	if(pDev->alarms.p == NULL)
	{
		TRACE("Can not create alarms for Device.\n");
		return FALSE;
	}

	ALARM* pAlarm = pDev->alarms.p;
	ALARM_INFO* p = s_pCharger->info.alarms_info.p;
	for(n = 0; n < s_pCharger->info.alarms_info.nTotal; n++, p++)
	{
		if(p->pBelonged_DevModel != pDevMod)
		{
			continue;
		}
		ZERO_POBJS(pAlarm, 1);
		pAlarm->pAlarmInfo = p;
		pAlarm->hDevice = (HANDLE)pDev;
		if(p->nExValue_ParamId != -1)
		{
			pAlarm->pExParam = GetParam_ByPR(PARAM_TYPE_STATUS,
				pDev->info.nDeviceId, p->nExValue_ParamId);
		}
		else
		{
			pAlarm->pExParam = NULL;
		}
		pAlarm++;
	}

	return TRUE;
}

CFG_ERRCODE Cfg_Parse_Device(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	DEVICE* pDev = (DEVICE*)pRowItem;
	memset(pDev, 0, sizeof(DEVICE));

	pDev->info.nDeviceId = sqlite3_column_int(pStmt, 0);

	//name
	if(sqlite3_column_type(pStmt, 2) == SQLITE_NULL)
	{
		//pDev->info.pRes_Name = NULL;
		MakeErrorMsg(ppszErrMsg, "Device[%d] name can not be null!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}
	else
	{
		pDev->info.pRes_Name = GetRes_ById(sqlite3_column_int(pStmt, 2));
	}

	if(pDev->info.pRes_Name == NULL)
	{
		MakeErrorMsg(ppszErrMsg, "Device[%d] name can NOT be null!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}

	DEVICE_MODEL_INFO* pDevMode = GetDevModel_ById(sqlite3_column_int(pStmt, 3));
	if(pDevMode == NULL)
	{
		MakeErrorMsg(ppszErrMsg, "INVALID DevModel for Device[%d]!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}
	pDev->info.pBelonged_DevModel = pDevMode;

	SAMPLER* pSampler = GetSamplerById(sqlite3_column_int(pStmt, 4));
	
#ifdef IMMU3
	if(pDev->info.nDeviceId == 601) //2524 使用601 imeu3 通过can控制急停, 不使用 485
	{
		pSampler = GetSamplerById(5);
	}
	#endif
	if(pSampler == NULL)
	{
		MakeErrorMsg(ppszErrMsg, "INVALID Sampler for Device[%d]!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}
	pDev->info.pBelonged_Sampler = pSampler;

	//Now create params
	if(!CreateParams_4Device(pDev))
	{
		MakeErrorMsg(ppszErrMsg, "Create params FAILED for Device[%d]!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}
	
	//Now create alarms
	if(!CreateAlarms_4Device(pDev))
	{
		MakeErrorMsg(ppszErrMsg, "Create alarms FAILED for Device[%d]!", pDev->info.nDeviceId);
		return CFG_ERR_PARSER;
	}

	return CFG_ERR_OK;
}

CFG_ERRCODE Cfg_Parse_HisParam(sqlite3_stmt *pStmt, void* pRowItem, char** ppszErrMsg)
{
	HISPARAM_INFO* pHisParam = (HISPARAM_INFO*)pRowItem;
	memset(pHisParam, 0, sizeof(HISPARAM_INFO));

	pHisParam->nDevId = sqlite3_column_int(pStmt, 0);
	pHisParam->nParamId = sqlite3_column_int(pStmt, 1);
	pHisParam->emTriggleType = sqlite3_column_int(pStmt, 2);
	pHisParam->fPeriod_mSec = (float)sqlite3_column_int(pStmt, 3) / 1000.0;
	pHisParam->fTriggle_GreaterThan = (float)sqlite3_column_double(pStmt, 4);

	TRACEX("--FOUND a HISPARAM:[%d, %d, %d, %.1f, %.1f]--\n", 
		pHisParam->nDevId, pHisParam->nParamId,
		pHisParam->emTriggleType, pHisParam->fPeriod_mSec,
		pHisParam->fTriggle_GreaterThan);

	return CFG_ERR_OK;
}

static char* PrintExpression(BOOL bDisable, CLUSTER_EXP_ELEMENT *pExpSet, char* pBuff, int nSize)
{
	ZERO_POBJS(pBuff, nSize);
	if(bDisable)
	{
		sprintf(pBuff, "NOT");
	}
	else if(pExpSet->nTotal == 0)
	{
		sprintf(pBuff, "NA");
	}
	else
	{
		Debug_PringExpression(pExpSet, pBuff, nSize);
	}

	return pBuff;
}

static char* Debug_PrintEnumList(PARAM_BASIC_INFO* pParamInfo, char* szBuff)
{
	ZERO_POBJS(szBuff, 1024);
	if((pParamInfo->nEnumTotal > 0) && (pParamInfo->emValueType == VALUE_TYPE_ENUM))
	{
		char szTmp[64];
		int n;
		sprintf(szBuff, "Enum: %d, ", pParamInfo->nEnumTotal);
		for(n = 0; n < pParamInfo->nEnumTotal; n++)
		{
			sprintf(szTmp, "[%d, %s],", pParamInfo->pEnumList[n].nEnumVal, pParamInfo->pEnumList[n].pEnumRes->szName);
			strcat(szBuff, szTmp);
		}
	}
	return szBuff;
}

void Debug_PrintCfg()
{
	int n, k;
	DEVICE* pDev = s_pCharger->devices.p;
	char szBuff[256];
	char szBuff1[256];
	char szEnumLst[1024];
	TRACE("\n-------------------------Now Begin Printing Cfg Parsed Info----------------------------\n");
	RESOURCE_TEXT* pRes = s_pCharger->info.resource_list.p;
	for(n = 0; n < s_pCharger->info.resource_list.nTotal; n++, pRes++)
	{
		TRACE("%d. [%s]\n", pRes->nRes_Id, pRes->szName);
	}

	for(n = 0; n < s_pCharger->devices.nTotal; n++, pDev++)
	{
		TRACE("\n[%s(type-%s)Parmeters]\n", (pDev->info.pRes_Name == NULL) ? "(NA)" : pDev->info.pRes_Name->szName, 
			pDev->info.pBelonged_DevModel->szDevModelName);

		//Status params
		PARAMETER* p = pDev->params_Status.p;
		STATUS_PARAM_INFO* pSA;
		for(k = 0; k < pDev->params_Status.nTotal; k++, p++)
		{
			pSA = (STATUS_PARAM_INFO*)p->pParamInfo;
			TRACE("SA%d. %d\t [%s] \t[%s] \t %d \t %d \t %s \\%s %d \t%s\n", 
				pSA->info.nParamId, 
				pSA->info.nPrecision,
				(pSA->info.pRes_Name == NULL) ? "(NA)" : pSA->info.pRes_Name->szName,
				(pSA->info.pRes_Unit == NULL) ? "(NA)" : pSA->info.pRes_Unit->szName,
				pSA->info.emValueType,
				pSA->emDataSrc,
				PrintExpression((pSA->emDataSrc == PARAM_SRC_CA), &pSA->equalExpression, szBuff, 256),
				PrintExpression(pSA->info.bDisableDisplay, &pSA->info.displayExpression, szBuff1, 256),
				pSA->info.displayExpression.nTotal,
				Debug_PrintEnumList(p->pParamInfo, szEnumLst));
		}

		//Control params
		p = pDev->params_Control.p;
		CONTROL_PARAM_INFO* pCL;
		for(k = 0; k < pDev->params_Control.nTotal; k++, p++)
		{
			pCL = (CONTROL_PARAM_INFO*)p->pParamInfo;
			TRACE("CL%d. \t [%s] \t %d \t %s \\%s\n", 
				pCL->info.nParamId, 
				(pCL->info.pRes_Name == NULL) ? "(NA)" : pCL->info.pRes_Name->szName,
				pCL->info.emValueType,
				PrintExpression(FALSE, &pCL->controlableExpression, szBuff, 256),
				PrintExpression(pCL->info.bDisableDisplay, &pCL->info.displayExpression, szBuff1, 256));
		}

		//Setting params
		p = pDev->params_Setting.p;
		SETTING_PARAM_INFO* pST;
		for(k = 0; k < pDev->params_Setting.nTotal; k++, p++)
		{
			pST = (SETTING_PARAM_INFO*)p->pParamInfo;
			if(pST->info.emValueType == VALUE_TYPE_FLOAT)
			{
				TRACE("ST%d. %d\t [%s] \t [%s] \t %d \t %s \\%s %d\\, default[%.1f] \t %s\n", 
					pST->info.nParamId, 
					pST->info.nPrecision,
					(pST->info.pRes_Name == NULL) ? "(NA)" : pST->info.pRes_Name->szName,
					(pST->info.pRes_Unit == NULL) ? "(NA)" : pST->info.pRes_Unit->szName,
					pST->info.emValueType,
					PrintExpression(FALSE, &pST->settableExpression, szBuff, 256),
					PrintExpression(pST->info.bDisableDisplay, &pST->info.displayExpression, szBuff1, 256),
					pST->info.displayExpression.nTotal, pST->varValDefault.fValue,
					Debug_PrintEnumList(p->pParamInfo, szEnumLst));
			}
			else if((pST->info.emValueType != VALUE_TYPE_STRING) && (pST->info.emValueType != VALUE_TYPE_BLOB))
			{
				TRACE("ST%d. \t [%s] \t %d \t %s \\%s %d\\, default[%d]\n", 
					pST->info.nParamId, 
					(pST->info.pRes_Name == NULL) ? "(NA)" : pST->info.pRes_Name->szName,
					pST->info.emValueType,
					PrintExpression(FALSE, &pST->settableExpression, szBuff, 256),
					PrintExpression(pST->info.bDisableDisplay, &pST->info.displayExpression, szBuff1, 256),
					pST->info.displayExpression.nTotal, pST->varValDefault.lValue);
			}
			else if(pST->info.emValueType == VALUE_TYPE_STRING)
			{
				TRACE("ST%d. \t [%s] \t %d \t %s \\%s %d\\, default[%s]\n", 
					pST->info.nParamId, 
					(pST->info.pRes_Name == NULL) ? "(NA)" : pST->info.pRes_Name->szName,
					pST->info.emValueType,
					PrintExpression(FALSE, &pST->settableExpression, szBuff, 256),
					PrintExpression(pST->info.bDisableDisplay, &pST->info.displayExpression, szBuff1, 256),
					pST->info.displayExpression.nTotal, pST->varValDefault.blobValue);
			}
			else
			{
				TRACE("ST%d. \t [%s] \t %d \t %s \\%s %d\\, default[NULL]\n", 
					pST->info.nParamId, 
					(pST->info.pRes_Name == NULL) ? "(NA)" : pST->info.pRes_Name->szName,
					pST->info.emValueType,
					PrintExpression(FALSE, &pST->settableExpression, szBuff, 256),
					PrintExpression(pST->info.bDisableDisplay, &pST->info.displayExpression, szBuff1, 256),
					pST->info.displayExpression.nTotal);
			}

		}
	}

	TRACE("\n-------------------------------------End of Printing-------------------------------------\n");
}
