#include "dai_loader.h"
#include "../moduals/data_processor/dpr/dpr.h"
#include "3rd_party/sqlite/sqlite3.h"

CHARGER* g_pCharger;

static ERRCODE_DAI GetErrCode(ERRCDDE_DPR errCd)
{
	ERRCODE_DAI nErr = ERRCODE_DAI_OK;
	switch(errCd)
	{
	case ERR_DPR_INVALID_ARG:
	case ERR_DPR_WRONG_SIZE:
		nErr = ERRCODE_DAI_INVALID_ARG;
		break;
	case ERR_DPR_MUTEX_ERR:
	case ERR_DPR_TIMEOUT:
		nErr = ERRCODE_DAI_INVALID_ARG;
		break;
	case ERR_DPR_NOTSUPPORTED:
		nErr = ERRCODE_DAI_NOTSUPPORTED;
		break;
	default:
		break;
	}
	return nErr;
}

//static PARAM_BASIC_INFO* GetParamInfo_ById(int emParamType, int nParamId)
//{
//	PARAM_BASIC_INFO *pParamInfo = NULL;
//	int n;
//
//	if(emParamType == PARAM_TYPE_STATUS)
//	{
//		STATUS_PARAM_INFO *p = g_pCharger->info.statusParams_info.p;
//		for(n = 0; n < g_pCharger->info.statusParams_info.nTotal; n++, p++)
//		{
//			if(p->info.nParamId == nParamId)
//			{
//				pParamInfo = &p->info;
//				break;
//			}
//		}
//	}
//	else if(emParamType == PARAM_TYPE_CONTROL)
//	{
//		CONTROL_PARAM_INFO *p = g_pCharger->info.controlParams_info.p;
//		for(n = 0; n < g_pCharger->info.controlParams_info.nTotal; n++, p++)
//		{
//			if(p->info.nParamId == nParamId)
//			{
//				pParamInfo = &p->info;
//				break;
//			}
//		}
//	}
//	else if(emParamType == PARAM_TYPE_SETTING)
//	{
//		SETTING_PARAM_INFO *p = g_pCharger->info.settingParms_info.p;
//		for(n = 0; n < g_pCharger->info.settingParms_info.nTotal; n++, p++)
//		{
//			if(p->info.nParamId == nParamId)
//			{
//				pParamInfo = &p->info;
//				break;
//			}
//		}
//	}
//	return pParamInfo;
//}

//static ALARM_INFO* GetAlarmInfo_ById(int nAlarmId)
//{
//	ALARM_INFO *pAlarmInfo = NULL;
//	int n;
//
//	ALARM_INFO *p = g_pCharger->info.alarms_info.p;
//	for(n = 0; n < g_pCharger->info.alarms_info.nTotal; n++, p++)
//	{
//		if(p->nAlarmId == nAlarmId)
//		{
//			pAlarmInfo = p;
//			break;
//		}
//	}
//	
//	return pAlarmInfo;
//}

static ERRCODE_DAI DAI_Get(IN HANDLE hSender, IN DAI_TYPE emInterfaceType, IN int nArg1, IN int nArg2,
	IN OUT void* pBuffer, IN UINT nBuffSize)
{
	INTER_GET GET = g_pCharger->dataprocessor.Get;
	ERRCDDE_DPR nErrCode;
	ERRCODE_DAI nErr = ERRCODE_DAI_OK;
	PARAM_BASIC_INFO *pParamInfo = NULL;
	ALARM_INFO *pAlarmInfo = NULL;
	COMMPORT* pPort = g_pCharger->commports.p;
	int n;
	PARAMETER* pParam;
	PARAM_VAL_GET* pValGet;
	STATUS_PARAM_INFO* pStatusParamInfo;
	CONTROL_PARAM_INFO* pCtrlParamInfo;
	SETTING_PARAM_INFO* pSetParamInfo;

	if(pBuffer == NULL)
	{
		return ERRCODE_DAI_INVALID_ARG;
	}

	//TRACEX("---Now run DAI Get Interface = %d--\n",emInterfaceType);

	switch (emInterfaceType)
	{
	case DAITYPE_DEV_TOTAL_R:
		if(nBuffSize != 4)
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			*(int*)pBuffer = g_pCharger->devices.nTotal;
		}
		break;
	case DAITYPE_DEV_INFO_R:
		if(nBuffSize != sizeof(DEVICE_INFO))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			if((nArg1 < 0) || (nArg1 >= g_pCharger->devices.nTotal))
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
			else
			{
				memcpy(pBuffer, g_pCharger->devices.p + nArg1, sizeof(DEVICE_INFO));
			}
		}
		break;
	case DAITYPE_GETDEVNAME_BYID_R:
		{
			DEVICE* pDev = g_pCharger->devices.p;
			for(n = 0; n < g_pCharger->devices.nTotal; n++, pDev++)
			{
				if(nArg1 == pDev->info.nDeviceId)
				{
					snprintf(pBuffer, MIN(nBuffSize, 32), "%s", pDev->info.pRes_Name->szName);					
					break;
				}
			}
		}
		break;
	case DAITYPE_PARAM_TOTAL_R:
		if(nBuffSize != 4)
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			if(nArg1 == PARAM_TYPE_STATUS)
			{
				*(int*)pBuffer = g_pCharger->info.statusParams_info.nTotal;
			}
			else if(nArg1 == PARAM_TYPE_CONTROL)
			{
				*(int*)pBuffer = g_pCharger->info.controlParams_info.nTotal;
			}
			else if(nArg1 == PARAM_TYPE_SETTING)
			{
				*(int*)pBuffer = g_pCharger->info.settingParms_info.nTotal;
			}
			else
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
		}
		break;
	case DAITYPE_PARAM_INFO_R:
		// if(nBuffSize != sizeof(PARAM_BASIC_INFO))
		// {
		// 	nErr = ERRCODE_DAI_INVALID_ARG;
		// }
		// else
		// {
			if(nArg1 == PARAM_TYPE_STATUS)
			{
				if((nArg2 < 0) || (nArg2 >= g_pCharger->info.statusParams_info.nTotal))
				{
					nErr = ERRCODE_DAI_INVALID_ARG;
				}
				else
				{
					memcpy(pBuffer, g_pCharger->info.statusParams_info.p + nArg2, sizeof(PARAM_BASIC_INFO));
				}
			}
			else if(nArg1 == PARAM_TYPE_CONTROL)
			{
				if((nArg2 < 0) || (nArg2 >= g_pCharger->info.controlParams_info.nTotal))
				{
					nErr = ERRCODE_DAI_INVALID_ARG;
				}
				else
				{
					memcpy(pBuffer, g_pCharger->info.controlParams_info.p + nArg2, sizeof(PARAM_BASIC_INFO));
				}
			}
			else if(nArg1 == PARAM_TYPE_SETTING)
			{
				if((nArg2 < 0) || (nArg2 >= g_pCharger->info.settingParms_info.nTotal))
				{
					nErr = ERRCODE_DAI_INVALID_ARG;
				}
				else
				{
					memcpy(pBuffer, g_pCharger->info.settingParms_info.p + nArg2, sizeof(SETTING_PARAM_INFO));
					// printf("nBuffSize = [%d], sizeof(PARAM_BASIC_INFO) = [%d], sizeof(SETTING_PARAM_INFO) = [%d]\n", nBuffSize, sizeof(PARAM_BASIC_INFO), sizeof(SETTING_PARAM_INFO));
				}
			}
			else
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
		// }
		break;
	case DAITYPE_PARAM_INFO_BY_PARAMID_R:
		if(nBuffSize != sizeof(PARAM_BASIC_INFO))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			nErr = ERRCODE_DAI_INVALID_ARG; //not found
			if(nArg1 == PARAM_TYPE_STATUS)
			{
				pStatusParamInfo = g_pCharger->info.statusParams_info.p;
				for(n = 0; n < g_pCharger->info.statusParams_info.nTotal; n++, pStatusParamInfo++)
				{
					if(pStatusParamInfo->info.nParamId == nArg2)
					{
						nErr = ERRCODE_DAI_OK;
						memcpy(pBuffer, &pStatusParamInfo->info, sizeof(PARAM_BASIC_INFO));
						break;
					}					
				}
			}
			else if(nArg1 == PARAM_TYPE_CONTROL)
			{
				pCtrlParamInfo = g_pCharger->info.controlParams_info.p;
				for(n = 0; n < g_pCharger->info.controlParams_info.nTotal; n++, pCtrlParamInfo++)
				{
					if(pCtrlParamInfo->info.nParamId == nArg2)
					{
						nErr = ERRCODE_DAI_OK;
						memcpy(pBuffer, &pCtrlParamInfo->info, sizeof(PARAM_BASIC_INFO));
						break;
					}					
				}
			}
			else if(nArg1 == PARAM_TYPE_SETTING)
			{
				pSetParamInfo = g_pCharger->info.settingParms_info.p;
				for(n = 0; n < g_pCharger->info.settingParms_info.nTotal; n++, pSetParamInfo++)
				{
					if(pSetParamInfo->info.nParamId == nArg2)
					{
						nErr = ERRCODE_DAI_OK;
						memcpy(pBuffer, &pSetParamInfo->info, sizeof(PARAM_BASIC_INFO));
						break;
					}					
				}
			}
		}
		break;
	case DAITYPE_ALARM_TOTAL_R:
		if(nBuffSize != 4)
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			*(int*)pBuffer = g_pCharger->info.alarms_info.nTotal;
		}
		break;
	case DAITYPE_ALARM_INFO_R:
		if(nBuffSize != sizeof(ALARM_INFO))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			if((nArg1 < 0) || (nArg1 >= g_pCharger->info.alarms_info.nTotal))
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
			else
			{
				memcpy(pBuffer, g_pCharger->info.alarms_info.p + nArg1, sizeof(ALARM_INFO));
			}
		}
		break;
	case DAITYPE_GETAALARMNAME_BYID_R:
		{
			ALARM_INFO* pAlm = g_pCharger->info.alarms_info.p;
			for(n = 0; n < g_pCharger->info.alarms_info.nTotal; n++, pAlm++)
			{
				if(nArg1 == pAlm->nAlarmId)
				{
					snprintf(pBuffer, MIN(nBuffSize, 32), "%s", pAlm->pRes_Name->szName);
					break;
				}
			}
		}
		break;
	case DAITYPE_GETAALARM_LEVEL_BYID_R:
		{
			ALARM_INFO* pAlm = g_pCharger->info.alarms_info.p;
			for(n = 0; n < g_pCharger->info.alarms_info.nTotal; n++, pAlm++)
			{
				if(nArg1 == pAlm->nAlarmId)
				{
					*(int*)pBuffer = (int)pAlm->emLevel;
					break;
				}
			}
		}
		break;
	case DAITYPE_APP_VRESION_R:
		if(nBuffSize < 16)
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			ZERO_POBJS(pBuffer, nBuffSize);
			snprintf((char*)pBuffer, MAX_LEN_MU2SWVER, "%s", g_pCharger->info.szSWVer);
			TRACEX("---Now get app main_ver = %s---\n",pBuffer);
		}
		break;
	case DAITYPE_PKG_VRESION_R:
		if(nBuffSize != 4)
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			//ZERO_POBJS(pBuffer, nBuffSize);
			//snprintf((char*)pBuffer, 16, g_pCharger->info.nPkgVer);
			*(int*)pBuffer = g_pCharger->info.nPkgVer;
			TRACEX("---Now get pkg main_ver = %d---\n",g_pCharger->info.nPkgVer);
		}
		break;
	case DAITYPE_STAT_PARAMVALUE_R:
		if(nBuffSize != sizeof(PARAM_VAL_GET))
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		pValGet = (PARAM_VAL_GET*)pBuffer;
		pParam = GetParam_ByPR(PARAM_TYPE_STATUS, nArg1, nArg2);
		if(pParam == NULL)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}
		pValGet->tmRefresh = (pParam->bIsValueInvalid) ? pParam->tmLastValueChanged : 
			pParam->tmCurrValueRefreshed;
		pValGet->nValueAttribute = (!pParam->bIsValueInvalid) ? VALUE_ATTRIBUTE_VALID : 0;
		pValGet->nValueAttribute |= (pParam->bDisplayAble) ? VALUE_ATTRIBUTE_DISPLAY : 0;

		nErrCode = GET(hSender, GETTYPE_SP_VAR, 
			nArg1, nArg2, &pValGet->val, pValGet->nGetSize);
		nErr = GetErrCode(nErrCode);
		break;
	case DAITYPE_SETT_PARAMVALUE_RW:
		if(nBuffSize != sizeof(PARAM_VAL_GET))
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		pValGet = (PARAM_VAL_GET*)pBuffer;
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nArg1, nArg2);
		if(pParam == NULL)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}
		pValGet->tmRefresh = (pParam->bIsValueInvalid) ? pParam->tmLastValueChanged : 
			pParam->tmCurrValueRefreshed;
		pValGet->nValueAttribute = (!pParam->bIsValueInvalid) ? VALUE_ATTRIBUTE_VALID : 0;
		pValGet->nValueAttribute |= (pParam->bDisplayAble) ? VALUE_ATTRIBUTE_DISPLAY : 0;
		pValGet->nValueAttribute |= (pParam->bCtrlSetAble) ? VALUE_ATTRIBUTE_CTRLSETABLE: 0;

		nErrCode = GET(hSender, GETTYPE_ST_VAR, 
			nArg1, nArg2, &pValGet->val, pValGet->nGetSize);
		nErr = GetErrCode(nErrCode);
		break;
	case DAITYPE_READBACK_VAL:
		if(nBuffSize != sizeof(PARAM_VAL_GET))
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		pValGet = (PARAM_VAL_GET*)pBuffer;
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nArg1, nArg2);
		if(pParam == NULL)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}
		pValGet->tmRefresh = (pParam->bIsValueInvalid) ? pParam->tmLastValueChanged : 
			pParam->tmCurrValueRefreshed;
		pValGet->nValueAttribute = (!pParam->bIsValueInvalid) ? VALUE_ATTRIBUTE_VALID : 0;
		pValGet->nValueAttribute |= (pParam->bDisplayAble) ? VALUE_ATTRIBUTE_DISPLAY : 0;
		pValGet->nValueAttribute |= (pParam->bCtrlSetAble) ? VALUE_ATTRIBUTE_CTRLSETABLE: 0;

		nErrCode = GET(hSender, GETTYPE_READBACK_VAR, 
			nArg1, nArg2, &pValGet->val, pValGet->nGetSize);
		nErr = GetErrCode(nErrCode);
		break;
	case DAITYPE_CTRL_PARAMVALUE_RW:
		if(nBuffSize != sizeof(PARAM_VAL_GET))
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		pValGet = (PARAM_VAL_GET*)pBuffer;
		pParam = GetParam_ByPR(PARAM_TYPE_CONTROL, nArg1, nArg2);
		if(pParam == NULL)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}
		pValGet->tmRefresh = (pParam->bIsValueInvalid) ? pParam->tmLastValueChanged : 
			pParam->tmCurrValueRefreshed;
		pValGet->nValueAttribute = (!pParam->bIsValueInvalid) ? VALUE_ATTRIBUTE_VALID : 0;
		pValGet->nValueAttribute |= (pParam->bDisplayAble) ? VALUE_ATTRIBUTE_DISPLAY : 0;
		pValGet->nValueAttribute |= (pParam->bCtrlSetAble) ? VALUE_ATTRIBUTE_CTRLSETABLE: 0;

		nErrCode = GET(hSender, GETTYPE_CL_VAR, 
			nArg1, nArg2, &pValGet->val, pValGet->nGetSize);
		nErr = GetErrCode(nErrCode);
		break;
	case DAITYPE_ALARMSTAT_R:
		nErrCode = GET(hSender, GETTYPE_ALMSTATE, 
			nArg1, nArg2, pBuffer, nBuffSize);
		nErr = GetErrCode(nErrCode);
		break;
	case DAITYPE_COMMPORT_R:
		if(nBuffSize != sizeof(COMMPORT))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			for(n = 0; n < g_pCharger->commports.nTotal; n++,pPort++)
			{
				if(nArg1 == pPort->info.nCommPort)
				{
					memcpy(pBuffer, pPort, nBuffSize);
					break;
				}
			}
		}
		break;
	/*case DAITYPE_GETLOCALLIST_VERSION:
		if(nBuffSize != sizeof(int))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			*(int*)pBuffer = g_pCharger->pLocalList->nListVersion;
		}
		break;
	case DAITYPE_GETLOCALLIST_TOTAL:
		if((nBuffSize != sizeof(int)) || (g_pCharger->pLocalList == NULL))
		{
			nErr = ERRCODE_DAI_INVALID_ARG;
		}
		else
		{
			*(int*)pBuffer = g_pCharger->pLocalList->nAuthorizationDataNumber;
		}
		break;
	case DAITYPE_GETLOCALLIST_ITEMS:
		{
			if((nBuffSize != sizeof(A_BLOB_UPDATE_LOCALLIST)) || (g_pCharger->pLocalList == NULL))
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
			else if((nArg1 < 0) || (nArg1 >= g_pCharger->pLocalList->nAuthorizationDataNumber) || 
				(nArg2 < 0) || (nArg2 > MAX_LOCALLIST_UPDATE_ITEMS))
			{
				nErr = ERRCODE_DAI_INVALID_ARG;
			}
			else
			{
				A_BLOB_UPDATE_LOCALLIST* pOutList = (A_BLOB_UPDATE_LOCALLIST*)pBuffer;
				memset(pBuffer, 0, sizeof(A_BLOB_UPDATE_LOCALLIST));
				for(n = 0; n <= nArg2; n++)
				{
					if(n + nArg1 >= g_pCharger->pLocalList->nAuthorizationDataNumber)
					{
						break;
					}
					memcpy(&pOutList->authorizationDataList[n - nArg1], &g_pCharger->pLocalList->authorizationDataList[n + nArg1],
						sizeof(AUTHORIZATION_DATA));
					pOutList->nAuthorizationDataNumber++;
				}
			}
		}

		break;*/
	default:
		nErr = ERRCODE_DAI_INVALID_ARG;
		break;
	}
	
	return nErr;
}

ERRCODE_DAI DAI_Set(IN HANDLE hSender, IN DAI_TYPE emInterfaceType, IN int nArg1, IN int nArg2,
	IN OUT void* pBuffer, IN UINT nBuffSize, IN int nTimeout)
{
	ERRCODE_DAI nErr = ERRCODE_DAI_OK;
	BOOL bWriteable = FALSE;
	ERRCDDE_DPR errDpr;
	PARAMETER* pParam;
	//SETTING_PARAM_INFO *pSetParamInfo;
	//CONTROL_PARAM_INFO* pCtrlParamInfo;
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	if(emInterfaceType == DAITYPE_COMM_DATA_DUMP)
	{
		if(pBuffer == NULL)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		nErr = DumpOut(nArg1, (DUMP_TYPE)nArg2, pBuffer) ? ERRCODE_DAI_OK : ERRCODE_DAI_EXECUTE_ERR;
		return nErr;
	}

	switch(emInterfaceType)
	{
	case DAITYPE_SETT_PARAMVALUE_RW:
		pParam = GetParam_ByPR(PARAM_TYPE_SETTING, nArg1, nArg2);
		if(pParam == NULL)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "Can NOT find such set-param[%d, %d]!", nArg1, nArg2);
			return ERRCODE_DAI_INVALID_ARG;
		}
		//pSetParamInfo = (SETTING_PARAM_INFO *)pParam->pParamInfo;
		//errDpr = g_pCharger->dataprocessor.CalculateExp(pParam, 
		//	NULL,
		//	&pSetParamInfo->settableExpression, 
		//	FALSE, 
		//	(void*)&bWriteable);

		//if((errDpr != ERR_DPR_OK) || (!bWriteable))
		//{
		//	return ERRCODE_DAI_NOTSETTABLE;
		//}
		TRACEX("Now received setParam[%d, %d]-->%d, bSetAble = %d.\n", 
			((DEVICE*)pParam->hDevice)->info.nDeviceId, pParam->pParamInfo->nParamId, ((VAR_VALUE*)pBuffer)->lValue, pParam->bCtrlSetAble);
		if(!pParam->bCtrlSetAble)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "The set-param[%d, %d] is NOT settable!", nArg1, nArg2);
			return ERRCODE_DAI_NOTSETTABLE;
		}
		setData.nDevId = nArg1;
		setData.nParamId = nArg2;
		setData.varValue = *(VAR_VALUE*)pBuffer;
		setData.nValueSize = nBuffSize;
		setData.nTimeOut = nTimeout;
		errDpr = g_pCharger->dataprocessor.Set(hSender, SETTYPE_INTERN_SETT, 
					&setData, sizeof(setData));
		nErr = GetErrCode(errDpr);
		if(nErr != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "The set-param[%d, %d] is executed ERROR=>%d!", nArg1, nArg2, errDpr);
		}
		break;
	case DAITYPE_VIRTUAL_PARAMVALUE_W:
		pParam = GetParam_ByPR(PARAM_TYPE_STATUS, nArg1, nArg2);
		if(pParam == NULL)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "Can NOT find such virtual-status-param[%d, %d]!", nArg1, nArg2);
			return ERRCODE_DAI_INVALID_ARG;
		}
		//TRACEX("Now received virtalParam[%d, %d]-->%d, bSetAble = %d.\n", 
		//	((DEVICE*)pParam->hDevice)->info.nDeviceId, pParam->pParamInfo->nParamId, ((VAR_VALUE*)pBuffer)->lValue, pParam->bCtrlSetAble);
		//if(!pParam->bCtrlSetAble)
		//{
		//	return ERRCODE_DAI_NOTSETTABLE;
		//}
		setData.nDevId = nArg1;
		setData.nParamId = nArg2;
		setData.varValue = *(VAR_VALUE*)pBuffer;
		setData.nValueSize = nBuffSize;
		setData.nTimeOut = nTimeout;
		errDpr = g_pCharger->dataprocessor.Set(hSender, SETTYPE_LOGIC_SP, 
			&setData, sizeof(setData));
		nErr = GetErrCode(errDpr);
		TRACEX("---Now setVirtualParam[%d, %d], nErr = %d---\n", nArg1, nArg2, nErr);
		if(nErr != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "The virtual-status-param[%d, %d] is executed ERROR=>%d!", nArg1, nArg2, errDpr);
		}
		break;
	case DAITYPE_CTRL_PARAMVALUE_RW:
		pParam = GetParam_ByPR(PARAM_TYPE_CONTROL, nArg1, nArg2);
		if(pParam == NULL)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "Can NOT find such ctrl-param[%d, %d]!", nArg1, nArg2);
			return ERRCODE_DAI_INVALID_ARG;
		}
		//pCtrlParamInfo = (CONTROL_PARAM_INFO *)pParam->pParamInfo;
		//errDpr = g_pCharger->dataprocessor.CalculateExp(pParam,
		//	NULL,
		//	&pCtrlParamInfo->controlableExpression, 
		//	FALSE,
		//	(void*)&bWriteable);

		//if((errDpr != ERR_DPR_OK) || (!bWriteable))
		//{
		//	return ERRCODE_DAI_NOTCONTROABLE;
		//}
		if(!pParam->bCtrlSetAble)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "The ctrl-param[%d, %d] is NOT controllable!", nArg1, nArg2);
			return ERRCODE_DAI_NOTSETTABLE;
		}
		setData.nDevId = nArg1;
		setData.nParamId = nArg2;
		setData.varValue = *(VAR_VALUE*)pBuffer;
		setData.nValueSize = nBuffSize;
		setData.nTimeOut = nTimeout;
		errDpr = g_pCharger->dataprocessor.Set(hSender, SETTYPE_INTERN_CTRL, 
			&setData, sizeof(setData));
		nErr = GetErrCode(errDpr);
		if(nErr != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_DPR, "DAI_SET", LOG_TYPE_ERROR, "The ctrl-param[%d, %d] is executed ERROR=>%d!", nArg1, nArg2, errDpr);
		}
		break;
	//case DAITYPE_AUTHORIZE_CACHE_RW:
	//	//will implement later
	//	break;
	//case DAITYPE_NETWORK_RW:
	//	//DAI_SetNetworkParam();
	//	break;
	default:
		break;
	}
	return nErr;
}


static char* MakeQueryCondString(const IN QUERY_TYPE emQueryType, const IN QUERY_COND *pCond, OUT char *pszCond)
{
	char *p = pszCond;
	memset(p, 0 , 256);
	char szSub[128];
	BOOL bNeedAnd = FALSE;
	/*if(emQueryType == QUERYTYPE_CHARGEREC)
	{
		QUERYCOND_CHARGERECORD *pChgRecCond = (QUERYCOND_CHARGERECORD *)pCond->pCondArg;

		if((pCond->nQueryCondType & QUERY_COND_CHARGE_STARTTIME) == QUERY_COND_CHARGE_STARTTIME)
		{
			sprintf(p, "AND Charge_BeginTime BETWWEN %d AND %d",
				pChgRecCond->tmChargeTime_Begin, pChgRecCond->tmChargeTime_End);
		}

		if((pCond->nQueryCondType & QUERY_COND_CHARGE_USER) == QUERY_COND_CHARGE_USER)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "AND UserID = %s", pChgRecCond->szUserId);
			strcat(p, szSub);
		}

		if((pCond->nQueryCondType & QUERY_COND_CHARGE_GUN) == QUERY_COND_CHARGE_GUN)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "AND WhichGun = %d", pChgRecCond->nWhichGun);
			strcat(p, szSub);
		}

		if((pCond->nQueryCondType & QUERY_COND_CHARGE_CARVIN) == QUERY_COND_CHARGE_CARVIN)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "AND Car_VIN = %s", pChgRecCond->szCarVIN);
			strcat(p, szSub);
		}

		if((pCond->nQueryCondType & QUERY_COND_REPORTED_FLAG) == QUERY_COND_REPORTED_FLAG)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "AND Report_Flag = %d", pChgRecCond->nReportFlag);
			strcat(p, szSub);
		}

		if((pCond->nQueryCondType & QUERY_COND_CHARGE_STARTMETHOD) == QUERY_COND_CHARGE_STARTMETHOD)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "AND ChargeStart_Mothod = %d", pChgRecCond->nChargeStartMethod);
			strcat(p, szSub);
		}
	}
	else*/ if(emQueryType == QUERYTYPE_ALARM)
	{
		QUERYCOND_ALARM *pAlarmCond = (QUERYCOND_ALARM *)pCond->pCondArg;
		if((pCond->nQueryCondType &QUERY_COND_ALARM_STARTTIME) == QUERY_COND_ALARM_STARTTIME)
		{
			sprintf(p, "Alarm_StartTime BETWEEN %d AND %d",
				pAlarmCond->tmAlarmStartTime_Begin, pAlarmCond->tmAlarmStartTime_End);
			bNeedAnd = TRUE;
		}
		else 
		{
			bNeedAnd = FALSE;
		}
		if((pCond->nQueryCondType & QUERY_COND_ALARM_PTR) == QUERY_COND_ALARM_PTR)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "%s Device_Id = %d AND Alarm_Id = %d", bNeedAnd ? "AND" : "", 
				pAlarmCond->alarmPTR.nId, pAlarmCond->alarmPTR.nSubId);
			strcat(p, szSub);
			bNeedAnd = TRUE;
		}
		else
		{
			bNeedAnd = FALSE;
		}
		if((pCond->nQueryCondType & QUERY_COND_ALARM_STATUS) == QUERY_COND_ALARM_STATUS)
		{
			memset(szSub, 0 , 128);
			sprintf(szSub, "%s Alarm_Status = %d", bNeedAnd ? "AND" : "", (int)pAlarmCond->almStatus);
			strcat(p, szSub);
		}
	}

	return p;
}

static ERRCODE_DAI DAI_Query(IN HANDLE hSender, IN QUERY_TYPE emQueryType, IN QUERY_COND cond,
		IN OUT void* pBuffer, IN OUT UINT *pGetNum)
{
	UNUSED(hSender); //no need at right now
	ERRCODE_DAI nErr = ERRCODE_DAI_OK;
	//sqlite3* data_db = (sqlite3*)g_pCharger->hRunDB;
	HIS_METER_VALUE_REC *pMeterValRec;
	EVENT_ALARM *pHisAlm;
	EVENT_ALARM *pActvAlm;
	char szSql[512];
	char szCond[256];
	sqlite3_stmt * stat;
	sqlite3 *data_db;
	UINT nLimit = (*pGetNum);
	UINT nGetCount = 0;
	int nSqlRet = 0;
	switch(emQueryType)
	{
	/*case QUERYTYPE_CHARGEREC:
		if(sizeof(QUERYCOND_CHARGERECORD) != cond.nArgSize)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}
		sprintf(szSql, "SELECT * FROM Data_ChargeRecords Where ChargingStatus = 2 %s ORDER BY Charge_BeginTime DESC LIMIT %d",
			MakeQueryCondString(emQueryType, &cond, szCond), nLimit);
		data_db = Open_Lock_SqliteDB();
		sqlite3_prepare(data_db, szSql, -1, &stat, NULL);
		nGetCount = 0;
		pChgRec = (HIS_CHARGE_RECORD *)pBuffer;
		nSqlRet = 0;
		while ((nSqlRet = sqlite3_step(stat)) == SQLITE_ROW)
		{
			pChgRec->emGunNum = sqlite3_column_int(stat, 1);
			pChgRec->tmChargeStart = sqlite3_column_int(stat, 2);
			pChgRec->nSessionID = sqlite3_column_int(stat, 3);
			memcpy(pChgRec->szUserID, sqlite3_column_text(stat, 4), 
				MIN(64, sqlite3_column_bytes(stat, 4)));
			memcpy(pChgRec->szVIN, sqlite3_column_text(stat, 5), 
				MIN(64, sqlite3_column_bytes(stat, 5)));
			pChgRec->emChargeStartMethod = sqlite3_column_int(stat, 6);
			pChgRec->emChargeStatus = sqlite3_column_int(stat, 7);
			pChgRec->nReportFlag = sqlite3_column_int(stat, 8);
			pChgRec->bIsOfflinePayedOK = sqlite3_column_int(stat, 9);
			memcpy(&pChgRec->extraRecData, sqlite3_column_blob(stat, 10), 
				MIN(sizeof(CHARGE_REC_EXTRADATA), sqlite3_column_bytes(stat, 10)));

			//pChgRec++;
			nGetCount++;
			if(nGetCount >= nLimit)
			{
				break;
			}
			pChgRec = (HIS_CHARGE_RECORD *)(pBuffer + sizeof(HIS_CHARGE_RECORD)*nGetCount);
		}

		*pGetNum = nGetCount;

		if((nSqlRet != SQLITE_DONE) && (nSqlRet != SQLITE_ROW))
		{
			TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
			//system("reboot");
		}
		else
		{
			TRACEX("------Execute query chargeRec sql[%s] SUSCCCCCC, nGetCount = %d------\n", szSql, nGetCount);
		}
		
		sqlite3_finalize(stat);
		Close_UnLock_SqliteDB();
		break;
		*/
	case QUERYTYPE_METERVALUS:
		if(sizeof(QUERYCOND_HISMETERVAL) != cond.nArgSize)
		{
			return ERRCODE_DAI_INVALID_ARG;
		}

		QUERYCOND_HISMETERVAL *pChgMeterValCond = (QUERYCOND_HISMETERVAL *)cond.pCondArg;
		sprintf(szSql, "SELECT * FROM Data_ChargeMeterValues Where Gun_Idx = %d AND Charge_Begin_Time = %d",
			pChgMeterValCond->nGunIdx, pChgMeterValCond->tmChargeStart);
		TRACEX("Now begin query metarValus, sql[%s]\n", szSql);
		data_db = Open_Lock_SqliteDB();
		sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
		nGetCount = 0;
		pMeterValRec = (HIS_METER_VALUE_REC *)pBuffer;
		nSqlRet = 0;
		while ((nSqlRet = sqlite3_step(stat)) == SQLITE_ROW)
		{
			pMeterValRec->nGunNum = sqlite3_column_int(stat, 1);
			pMeterValRec->tmChargeStart = sqlite3_column_int(stat, 2);
			//memcpy(&pMeterValRec->meterData, sqlite3_column_blob(stat, 3), 
			//	MIN(sizeof(CHARGE_METERITEM_LIST), sqlite3_column_bytes(stat, 3)));
			sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pMeterValRec->meterData, sizeof(CHARGE_METERITEM_LIST));
			//pChgRec++;
			nGetCount++;
			if(nGetCount >= nLimit)
			{
				break;
			}
			pMeterValRec = (CHARGE_METERITEM_LIST *)((BYTE*)pBuffer + sizeof(CHARGE_METERITEM_LIST)*nGetCount);
		}

		*pGetNum = nGetCount;

		if((nSqlRet != SQLITE_DONE) && (nSqlRet != SQLITE_ROW))
		{
			TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
			//system("reboot");
		}
		else
		{
			TRACEX("------Execute query meterVals sql[%s] SUSCCCCCC, nGetCount = %d------\n", szSql, nGetCount);
		}

		sqlite3_finalize(stat);
		Close_UnLock_SqliteDB();
		break;
	
	case QUERYTYPE_ALARM:
		{
			if(sizeof(EVENT_ALARM) != cond.nArgSize)
			{
				return ERRCODE_DAI_INVALID_ARG;
			}
			sprintf(szSql, "SELECT * FROM Data_Alarms");
			sqlite3 *data_db = Open_Lock_SqliteDB();
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);	
			UINT nGetCount = 0;
			pHisAlm = (EVENT_ALARM *)pBuffer;
			int nSqlRet;
			while ((nSqlRet = sqlite3_step(stat)) == SQLITE_ROW)
			{
				pHisAlm->nDevId = sqlite3_column_int(stat, 1);
				pHisAlm->nAlarmId = sqlite3_column_int(stat, 2);
				pHisAlm->status = sqlite3_column_int(stat, 3);
				pHisAlm->fExParamVal = (float)sqlite3_column_double(stat, 4);
				pHisAlm->tmStart = sqlite3_column_int(stat, 5);
				pHisAlm->tmEnd = sqlite3_column_int(stat, 6);

				nGetCount++;
				if(nGetCount >= nLimit)
				{
					break;
				}
				
				TRACEX("nDevId=%d nAlarmId=%d status=%d fExParamVal=%f tmStart=%ld tmEnd=%ld\n",\
					pHisAlm->nDevId,pHisAlm->nAlarmId,pHisAlm->status,pHisAlm->fExParamVal,pHisAlm->tmStart,pHisAlm->tmEnd);

				pHisAlm = (EVENT_ALARM *)((BYTE*)pBuffer + sizeof(EVENT_ALARM)*nGetCount);
			}

			*pGetNum = nGetCount;

			if((nSqlRet != SQLITE_DONE) && (nSqlRet != SQLITE_ROW))
			{
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
			}

			sqlite3_finalize(stat);
			Close_UnLock_SqliteDB();
			break;
		}
	case QUERYTYPE_ACTV_ALARM:
		{
			if(sizeof(EVENT_ALARM) != cond.nArgSize)
			{
				return ERRCODE_DAI_INVALID_ARG;
			}
			sprintf(szSql, "select * from Data_Alarms where Alarm_Status = 1 order by Alarm_StartTime DESC");
			sqlite3 *data_db = Open_Lock_SqliteDB();
			sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);	
			UINT nGetCount = 0;
			pActvAlm = (EVENT_ALARM *)pBuffer;
			int nSqlRet;
			while ((nSqlRet = sqlite3_step(stat)) == SQLITE_ROW)
			{
				pActvAlm->nDevId = sqlite3_column_int(stat, 1);
				pActvAlm->nAlarmId = sqlite3_column_int(stat, 2);
				pActvAlm->status = sqlite3_column_int(stat, 3);
				pActvAlm->fExParamVal = (float)sqlite3_column_double(stat, 4);
				pActvAlm->tmStart = sqlite3_column_int(stat, 5);
				pActvAlm->tmEnd = sqlite3_column_int(stat, 6);

				nGetCount++;
				if(nGetCount >= nLimit)
				{
					break;
				}
				
				TRACEX("nDevId=%d nAlarmId=%d status=%d fExParamVal=%f tmStart=%ld tmEnd=%ld\n",\
					pActvAlm->nDevId,pActvAlm->nAlarmId,pActvAlm->status,pActvAlm->fExParamVal,pActvAlm->tmStart,pActvAlm->tmEnd);

				pActvAlm = (EVENT_ALARM *)((BYTE*)pBuffer + sizeof(EVENT_ALARM)*nGetCount);
			}

			*pGetNum = nGetCount;

			if((nSqlRet != SQLITE_DONE) && (nSqlRet != SQLITE_ROW))
			{
				TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", szSql, nSqlRet, sqlite3_errmsg(data_db));
				//system("reboot");
			}

			sqlite3_finalize(stat);
			Close_UnLock_SqliteDB();
			break;
		}

 
	default:
		nErr = ERRCODE_DAI_INVALID_ARG;
		break;
	}
	
	return nErr;
}

ERRCODE_DAI DAI_Signal(IN HANDLE hSender, EVENT_ARG* pArg)
{
	ERRCODE_DAI nErr = ERRCODE_DAI_INVALID_ARG;

	//Jimmy note: for right now framework, we do not support msg.send

	//ERRCDDE_DPR errDpr = 
	//	g_pCharger->dataprocessor.Set(hSender, SETTYPE_SENDMSG, 
	//		pArg, sizeof(EVENT_ARG));

	//nErr = GetErrCode(errDpr);

	return nErr;
}

ERRCODE_DAI DAI_RegisterHandler(char* pszFuncName, 
	HANDLE hModule, 
	EVENT_HANDLER pfnHandler, 
	UINT nEventFlag, 
	UINT nMsgFlag, 
	BOOL bIsUnregister)
{
	ERRCODE_DAI nErr = ERRCODE_DAI_OK;

	//TRACEX("Now load Evhand %s for modual, rrrrrhand = %d\n", pszFuncName, g_pCharger->dataprocessor.Register_EvHandler);

	ERRCDDE_DPR errDpr = 
		g_pCharger->dataprocessor.Register_EvHandler(pszFuncName, 
		hModule, 
		pfnHandler,
		nEventFlag,
		nMsgFlag,
		bIsUnregister);

	nErr = GetErrCode(errDpr);
	return nErr;
}

//ERRCODE_DAI DAI_CalcExpression(IN HANDLE hSender,
//	IN PARAMETER* pParam,
//	IN char* szExpression, 
//	CLUSTER_EXP_ELEMENT *pExpSet, 
//	BOOL bParseExpSet, //first time parse, then no need
//	OUT void* pResult)
//{
//	ERRCODE_DAI nErr = ERRCODE_DAI_OK;
//
//	ERRCDDE_DPR errDpr = 
//		g_pCharger->dataprocessor.CalculateExp(pParam, 
//		szExpression, 
//		pExpSet,
//		bParseExpSet,
//		pResult);
//
//	nErr = GetErrCode(errDpr);
//	if(nErr != ERRCODE_DAI_OK)
//	{
//		LogOut(MAIN_MODULE_MAIN, "DAI_CalcExp", LOG_TYPE_ERROR,
//			"Thd expressopn[%s] from Sender[%s] canNOT be calc, Err = %d", szExpression, 
//			((REPORTER_INFO*)hSender)->szReporterName, nErr);
//	}
//	return nErr;
//}
//
//void DAI_FreeExpression(IN CLUSTER_EXP_ELEMENT *pExpSet)
//{
//	if(pExpSet == NULL)
//	{
//		return;
//	}
//	int n;
//	EXP_ELEMENT* p = (EXP_ELEMENT*)pExpSet->p;
//	for(n = 0; n < pExpSet->nTotal; n++, p++)
//	{
//		SAFELY_DELETE(p);
//	}
//	pExpSet->nTotal = 0;
//}

BOOL Init_LoadDAI(CHARGER* pCharger)
{
	g_pCharger = pCharger;
	DAI *pThis = &g_pCharger->dai;
	pThis->Get = DAI_Get;
	pThis->Set = DAI_Set;
	pThis->Query = DAI_Query;
	pThis->Signal = DAI_Signal;
	pThis->Regsiter_EvHandler = DAI_RegisterHandler;
	TRACEX("Now load DAI successfully!\n");
	//pThis->CalculateExp = DAI_CalcExpression;
	//pThis->FreeExp = DAI_FreeExpression;
	return TRUE;
}
