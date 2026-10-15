#include "reporter_webapi.h"

int Get_VersionInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
	WEB_VERSION_INFO verInfo;
	ZERO_POBJS(&verInfo, 1);
	int nERR = ERRCODE_DAI_OK;	
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_APP_VRESION_R,
		0, 0, verInfo.szIMMU2_SWVer, MAX_LEN_MU2SWVER);

	//will handle other version info later
	// if(_SYSTEM_EX("hostname | grep -c a02") > 0)
	// {
	// 	sprintf(verInfo.szIMMU2_HWVer, "A02");
	// }
	// else if(_SYSTEM_EX("hostname | grep -c a01") > 0)
	// {
	// 	sprintf(verInfo.szIMMU2_HWVer, "A01");
	// }
	// else
	// {
	// 	sprintf(verInfo.szIMMU2_HWVer, "A00");
	// }
	snprintf(verInfo.szIMMU2_HWVer, 8, "%s", GetIMMU2_HWVersion());

	//IMSU-D version
	PARAM_VAL_GET valGet;
	ZERO_POBJS(&valGet, 1);
	A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
	ZERO_POBJS(&blobCCUSett, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS);
	valGet.val.blobValue = (BYTE*)&blobCCUSett;
	nERR = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			200, 200, &valGet, sizeof(PARAM_VAL_GET));
	if(nERR == ERRCODE_DAI_OK){
		
		snprintf(verInfo.szIMSU_D_Vers, 32, "%s", blobCCUSett.ccuSettings.szSWVer);	
	}
	else{
		sprintf(verInfo.szIMSU_D_Vers, "--");	
	}
	

	//STBattery Version
	ZERO_POBJS(&valGet, 1);
	A_BLOB_SINGLE_STBATT_RUNINFO stBattDataInfo;
	ZERO_POBJS(&stBattDataInfo, 1);
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
	valGet.val.blobValue = (BYTE*)&stBattDataInfo;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501, 551, &valGet, sizeof(PARAM_VAL_GET));
     if(stBattDataInfo.fVersion > 0)
    {
		snprintf(verInfo.szSTBatt_Ver, 32, "%.3f", stBattDataInfo.fVersion);//edit by Kuangyue 20250227
    }
	else
	{
		sprintf(verInfo.szSTBatt_Ver, "--");
	}	

	char szChargerID[64];
	valGet.nGetSize = 64;
	valGet.val.blobValue = (BYTE*)szChargerID;
	nERR = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 2, &valGet, sizeof(PARAM_VAL_GET));
	snprintf(verInfo.szDevID, 64, "%s", szChargerID);


	//IMEU2 Version
	/*
	ZERO_POBJS(&valGet, 1);
	A_BLOB_BATTCAB_IMEU2_V20   blobIMEU2;
	ZERO_POBJS(&blobIMEU2, 1);
	valGet.nGetSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);
	valGet.val.blobValue = (BYTE*)&blobIMEU2;
	nERR = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			601, 450, &valGet, sizeof(PARAM_VAL_GET));
    if((nERR == ERRCODE_DAI_OK) && (strlen(blobIMEU2.szSoftwareVer) > 1))
    {
  		snprintf(verInfo.szIMEU2_Ver, 32, "%s", blobIMEU2.szSoftwareVer);
    }
	else
	{
		sprintf(verInfo.szIMEU2_Ver, "--");
	}	
	*/
	//rectifier's version
	//edit by Kuangyue
	valGet.nGetSize = 4;
	nERR = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 12, &valGet, sizeof(PARAM_VAL_GET));
	if(nERR == ERRCODE_DAI_OK)
	{
		if(valGet.val.fValue > 0.001)
		{
			snprintf(verInfo.szRect_Ver_DC, 32, "%.3f", valGet.val.fValue);	
		}
		else
		{
			sprintf(verInfo.szRect_Ver_DC, "--");
		}
	}
	else
	{
		sprintf(verInfo.szRect_Ver_DC, "--");
	}
	//rectifier's version U1(PFC)版本
	valGet.nGetSize = 4;
	nERR = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			1, 13, &valGet, sizeof(PARAM_VAL_GET));
	if(nERR == ERRCODE_DAI_OK)
	{
		if(valGet.val.fValue > 0.001)
		{
			snprintf(verInfo.szRect_Ver_PFC, 32, "%.3f", valGet.val.fValue);	
		}
		else
		{
			sprintf(verInfo.szRect_Ver_PFC, "--");
		}		
	}
	else
	{
		sprintf(verInfo.szRect_Ver_PFC, "--");
	}
	
	//now send out all data
	memcpy(pRepDataPos, &verInfo, sizeof(WEB_VERSION_INFO));
    return sizeof(WEB_VERSION_INFO);
}

static WEB_DEV_CLASS GetWebClass_ById(WEBAPI_REPORTER_ROUGH* pWebRough, int nDevId)
{
    if(nDevId == 1)
    {
        return DEV_CLASS_CHARGER_SYSTEM;
    }
    else if((nDevId >= 200) && (nDevId < 220))
    {
        return DEV_CLASS_CCU;
    }
	// else if((nDevId >= 221) && (nDevId < 230))
	// {
	// 	return DEV_CLASS_CALC_NODEPOWER;
	// }
    else if((nDevId >= 401) && (nDevId <= 411))
    {
        return DEV_CLASS_METERS;
    }
	else if((nDevId >= 500) && (nDevId <= 510))
	{
		return DEV_CLASS_BATTERY;
	}
	else if((nDevId >= 601) && (nDevId <= 604))
	{
		return DEV_CLASS_BATTCAB_IMEU2;
	}
	else if((nDevId >= 611) && (nDevId <= 615))
	{
		return DEV_CLASS_POWER_IMEU;
	}
	else if((nDevId >= 251) && (nDevId <= 275))
	{
		return DEV_CLASS_SINGLE_PM;
	}
	else if((nDevId >= 700) && (nDevId <= 705))
	{
		return DEV_CLASS_PCS_GROUP;
	}
	else if((nDevId >= 901) && (nDevId <= 909))  
	{
		return DEV_CLASS_UART_SERVER;
	}
	else if((nDevId >= 911) && (nDevId <= 914))   
	{
		return DEV_CLASS_TEMP_CONTROL;
	}
	else if((nDevId >= 951) && (nDevId <= 954))   
	{
		return DEV_CLASS_AIR_COND;
	}
	else if((nDevId >= 981) && (nDevId <= 985))
	{
		return DEV_CLASS_LIQUIDCOOLING_PACK;
	}
	else if((nDevId >= 1050) && (nDevId < 1070))
	{
		return DEV_CLASS_PV_INVERTER;
	}
	else if(nDevId == 1001)
	{
		return DEV_CLASS_MAX;
	}
    else
    {
        return DEV_CLASS_OTHER;
    }    
}

int Get_AllDevInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
    
    int errCode;
    int nDataSize = 0;
    int nTotalDev = 0;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_DEV_TOTAL_R,
			0, 0, &nTotalDev, 4);
    if((nTotalDev < 1) || (nTotalDev > 65535))
    {
        TRACEX("---the totalDev is %d----\n", nTotalDev);
        return 0;
    }
    int n;
    //total devices
    varV.lValue = nTotalDev;
	memcpy(pRepDataPos, varV.abyValue, 4);
	pRepDataPos += 4;
	nDataSize += 4;
    DEVICE_INFO devInfo;
    WEB_DEVICE_INFO webDevInfo;
    int nWebDevSize = sizeof(WEB_DEVICE_INFO);
	for(n = 0; n < nTotalDev; n++)
	{
		errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_DEV_INFO_R,
			n, 0, &devInfo, sizeof(DEVICE_INFO));
		if(errCode != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_REPORTER, "WebGetDevStru", LOG_TYPE_ERROR,
				"Get Dev_Idx[%d] info, err[%d]!", n, errCode);
			return 0; //do not reply
		}
        webDevInfo.nDevModelId = devInfo.pBelonged_DevModel->nDevModelId;
        webDevInfo.nDevId = devInfo.nDeviceId;
        webDevInfo.nDevName_ResId = devInfo.pRes_Name->nRes_Id;
        webDevInfo.emDevClass = GetWebClass_ById(pWebRough, devInfo.nDeviceId);
		memcpy(pRepDataPos, &webDevInfo, nWebDevSize);
		pRepDataPos += nWebDevSize;
		nDataSize += nWebDevSize;
	}

    return nDataSize;
}

static WEB_VALUE_TYPE Get_WebParamValType(int nParamId, _VALUE_TYPE emValType, int nPrecision)
{
    if((emValType == VALUE_TYPE_ULONG) || (emValType == VALUE_TYPE_LONG))
    {
        return WEB_VALTYPE_INT;
    }
    else if(emValType == VALUE_TYPE_DATETIME)
    {
        //need judge for DATA/DATETIME/TIME
        return WEB_VALTYPE_DATETIME;
    }
    else if(emValType == VALUE_TYPE_ENUM)
    {
        //need judge for enum/bool
        return WEB_VALTYPE_ENUM;
    }
    else if(emValType == VALUE_TYPE_STRING)
    {
        //need judge for string/password
        return WEB_VALTYPE_STRING;
    }
    else if(emValType == VALUE_TYPE_BLOB)
    {
        //need judge for string/password
        return WEB_VALTYPE_OBJECT;
    }
    else if(emValType == VALUE_TYPE_FLOAT)
    {        
        if(nPrecision >= 4)
        {
            return WEB_VALTYPE_FLOAT_4;
        }
        else if(nPrecision >= 3)
        {
            return WEB_VALTYPE_FLOAT_3;
        }
        else if(nPrecision >= 2)
        {
            return WEB_VALTYPE_FLOAT_2;
        }
        else if(nPrecision >= 1)
        {
            return WEB_VALTYPE_FLOAT_1;
        }
        else 
        {
            return WEB_VALTYPE_FLOAT_0;
        }
    }
    else //nothing else
    {
        return WEB_VALTYPE_INT;
    }    
}

int Get_AllParamInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
    
    int errCode;
    int nDataSize = 0;
    int n;		
    int nType;
	int kk;
	int nParmTotal = 0;
    PARAM_BASIC_INFO paramInfo;
    WEB_PARAM_INFO  webParamInfo;
    int nWebParamInfoSize = sizeof(WEB_PARAM_INFO);
    //first get total params
    int nParamTotal_4Types[3] = {0, 0, 0};
    for(nType = PARAM_TYPE_STATUS; nType <= PARAM_TYPE_SETTING; nType++)
    {
		errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PARAM_TOTAL_R,
				nType, 0, &nParamTotal_4Types[nType - PARAM_TYPE_STATUS], 4);
        
		TRACEX("Get paramType[%d] Total:%d\n", nType, 
            nParamTotal_4Types[nType - PARAM_TYPE_STATUS]);
		if(errCode != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_REPORTER, "WebGetParamStru", LOG_TYPE_ERROR,
					"Get ParamType[%d] Total, err[%d]!", nType, errCode);
				return 0; //do not reply
		}

        nParmTotal += nParamTotal_4Types[nType - PARAM_TYPE_STATUS];
    }
    //now write the total
	varV.lValue = nParmTotal;
    memcpy(pRepDataPos, varV.abyValue, 4);
    pRepDataPos += 4;
    nDataSize += 4;

	//3-type param info
	for(nType = PARAM_TYPE_STATUS; nType <= PARAM_TYPE_SETTING; nType++)
	{
        nParmTotal = nParamTotal_4Types[nType - PARAM_TYPE_STATUS];
        //then write paramInfo one by one
		for(n = 0; n < nParmTotal; n++)
		{
			SETTING_PARAM_INFO paramSetInfo;
			if (nType == PARAM_TYPE_SETTING)
			{
				errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PARAM_INFO_R,
					nType, n, &paramSetInfo, sizeof(SETTING_PARAM_INFO));
				memcpy(&paramInfo, &paramSetInfo.info, sizeof(PARAM_BASIC_INFO));
			}
			else
			{
				errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_PARAM_INFO_R,
					nType, n, &paramInfo, sizeof(PARAM_BASIC_INFO));
			}
			if(errCode != ERRCODE_DAI_OK)
			{
				LogOut(MAIN_MODULE_REPORTER, "WebGetParamStru", LOG_TYPE_ERROR,
					"Get ParamType[%d] info, Idx[%d], err[%d]!", nType, n, errCode);
				return 0; //do not reply
			}
			TRACE("param %s, paramInfo.nEnumTotal: %d\n", paramInfo.pRes_Name->szName, paramInfo.nEnumTotal);
			if(paramInfo.bDisableDisplay)
			{
				continue; //if not Display ON UI, just skip it!
			}
            ZERO_POBJS(&webParamInfo, 1);
            webParamInfo.nDevModelId = paramInfo.pBelonged_DevModel->nDevModelId;
            webParamInfo.emParamType = (PARAM_TYPE)nType;
            webParamInfo.nParamId = paramInfo.nParamId;
            webParamInfo.nParamName_ResId = (paramInfo.pRes_Name != NULL) ? paramInfo.pRes_Name->nRes_Id : 0;//name
			webParamInfo.nParamUnit_ResId = (paramInfo.pRes_Unit != NULL) ? paramInfo.pRes_Unit->nRes_Id : 0;//unit
            webParamInfo.nValueSize = paramInfo.nValueSize; //value size
            webParamInfo.emValType = Get_WebParamValType(webParamInfo.nParamId, paramInfo.emValueType, paramInfo.nPrecision); //value size
            webParamInfo.fMaxLimit = paramInfo.fMaxLimit; //max limit
            webParamInfo.fMinLimit = paramInfo.fMinLimit; //min limit
            webParamInfo.nDisplayPriority = paramInfo.nDisplayPriority; //disp priority
            webParamInfo.uMask_AuthorityLevel = paramInfo.uMask_AuthorityLevel; //Authority Level
			webParamInfo.Web_1 = paramInfo.Web_1;
			webParamInfo.Web_2 = paramInfo.Web_2;

			if (nType == PARAM_TYPE_SETTING)
			{
				SETTING_PARAM_INFO* pSettParam = (SETTING_PARAM_INFO*)&paramInfo;
				webParamInfo.varValDefault = paramSetInfo.varValDefault;
				// webParamInfo.varValDefault.fValue = 5;
			}
            if(paramInfo.emValueType == VALUE_TYPE_BLOB)
            {
                webParamInfo.nBlobId = paramInfo.nPrecision; //blobId
            }
            webParamInfo.nEnumTotal = MIN(paramInfo.nEnumTotal, MAX_SUPPORTED_WEB_ENUMITEM);
			for(kk = 0; kk < webParamInfo.nEnumTotal; kk++)
			{
                webParamInfo.enumList[kk].emVal = paramInfo.pEnumList[kk].nEnumVal;
                webParamInfo.enumList[kk].nEnumName_ResId = paramInfo.pEnumList[kk].pEnumRes->nRes_Id;
			}//enum list

            memcpy(pRepDataPos, &webParamInfo, nWebParamInfoSize);
            pRepDataPos += nWebParamInfoSize;
            nDataSize += nWebParamInfoSize;

		}//single param

	}//3 type

    return nDataSize;
}

int Get_AllAlarmInfos(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
    
    int errCode;
    int nDataSize = 0;
    int n;	
    int nAlarmInfoTotal = 0;
	//now get alarm total
	errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_TOTAL_R,
		0, 0, &nAlarmInfoTotal, 4);
	TRACEX("AlarmTotal = %d, nDataSize:%d\n", nAlarmInfoTotal, nDataSize);
	if(errCode != ERRCODE_DAI_OK)
	{
		LogOut(MAIN_MODULE_REPORTER, "WebGetAlmInfoStru", LOG_TYPE_ERROR,
			"Get Alarm Total, err[%d]!", errCode);
		return 0; //do not reply
	}

    //first write out the alarm INfo total
	varV.lValue = nAlarmInfoTotal;
    memcpy(pRepDataPos, varV.abyValue, 4);
    pRepDataPos += 4;
    nDataSize += 4;

	ALARM_INFO alarmInfo;
    WEB_ALARM_INFO webAlmInfo;
    int nWebAlmInfoSize = sizeof(WEB_ALARM_INFO);
	for(n = 0; n < nAlarmInfoTotal; n++)
	{
		errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARM_INFO_R,
			n, 0, &alarmInfo, sizeof(ALARM_INFO));
		if(errCode != ERRCODE_DAI_OK)
		{
			LogOut(MAIN_MODULE_REPORTER, "GUIGetDataStru", LOG_TYPE_ERROR,
				"Get Alarm info, Idx[%d], err[%d]!", n, errCode);
			return 0; //do not reply
		}
		webAlmInfo.nAlarmId = alarmInfo.nAlarmId; //alarmId
		webAlmInfo.nAlmName_ResId = (alarmInfo.pRes_Name != NULL) ? alarmInfo.pRes_Name->nRes_Id : 0;//name
		webAlmInfo.emLevel = alarmInfo.emLevel;//alarm level

        memcpy(pRepDataPos, &webAlmInfo, nWebAlmInfoSize);
        pRepDataPos += nWebAlmInfoSize;
        nDataSize += nWebAlmInfoSize;
	}

    return nDataSize;	
}

static int GetDAITp_ByParTp(int emParamTp)
{
	if(emParamTp == PARAM_TYPE_STATUS)
	{
		return DAITYPE_STAT_PARAMVALUE_R;
	}
	else if(emParamTp == PARAM_TYPE_SETTING)
	{
		return DAITYPE_SETT_PARAMVALUE_RW;
	}
	else if(emParamTp == PARAM_TYPE_CONTROL)
	{
		return DAITYPE_CTRL_PARAMVALUE_RW;
	}

    return 0;
}


int Get_AllParmsVals(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
    
    int errCode;
    int nDataSize = 0;
    int n;	
    int nParamTpTotal = 0;
    int nType;
    PARAM_VAL_GET valGet;
	OBJ_PTR arg;
    int nGetValSize;
    int nGetParamTp;
    int nReqqqqqDataLen = pWebRough->nParsed_InDataLen;
    BYTE* pReqqqqDataPos =  pWebRough->pbyParsed_InDataPos;
    int nHandledParams = 0;
	while(nReqqqqqDataLen > 0)
	{
        nGetParamTp = *(BYTE*)pReqqqqDataPos;
		memcpy(varV.abyValue, pReqqqqDataPos + 1, 4);
		arg.nId = varV.lValue;
		memcpy(varV.abyValue, pReqqqqDataPos + 5, 4);
		arg.nSubId = varV.lValue;
		memcpy(varV.abyValue, pReqqqqDataPos + 9, 4);
		nGetValSize = varV.lValue;
        //now send back the same thing for paramInfo
        // memcpy(pRepDataPos, pReqqqqDataPos, 13);
		// pRepDataPos += 13;
		// nDataSize += 13;
        //now get the value
		pReqqqqDataPos += 13;
		nReqqqqqDataLen -= 13;
		ZERO_POBJS(&valGet, 1);
		valGet.nGetSize = nGetValSize;
        TRACEX("---Now try to get the param[%d, %d, %d] value, size->%d---\n",
            arg.nId, nGetParamTp, arg.nSubId, nGetValSize);
		if((nGetValSize > 4) && (nGetValSize < 1024*1024*2)) //max support 2M for single blob
		{
			valGet.val.blobValue = NEW(BYTE, nGetValSize);
		}
        else
        {
           valGet.nGetSize = 4;
           nGetValSize = 4;
        }
        
		errCode = pReporter->pDAI->Get((HANDLE)pReporter, GetDAITp_ByParTp(nGetParamTp),
			arg.nId, arg.nSubId, &valGet, sizeof(PARAM_VAL_GET));
		if(errCode != ERRCODE_DAI_OK)
		{
			TRACE("jjjjjjj arg.Id = %d, arg.nSubId = %d, nDataLen = %d, tp = %d, nValSize = %d, and ErrCode = %d\n", 
				arg.nId, arg.nSubId, nReqqqqqDataLen, nGetParamTp, nGetValSize, errCode);
			pWebRough->nParseHandleRTN = WEB_ERRCODE_WRONG_FORMAT;
			// varV.lValue = nHandledParams; //notice qt which param not correct
			// memcpy(pRepDataPos, varV.abyValue, 4);
			nDataSize = 0;
			if(nGetValSize > 4)
			{
				DELETE(valGet.val.blobValue);
			}
			return 0;
		}
		//now copy result
		varV.ulValue = valGet.nValueAttribute;
		memcpy(pRepDataPos, varV.abyValue, 4);
		pRepDataPos += 4;
		nDataSize += 4;
		if(nGetValSize > 4)
		{
			memcpy(pRepDataPos, valGet.val.blobValue, (size_t)nGetValSize);
            DELETE(valGet.val.blobValue);
		}
		else
		{
			memcpy(pRepDataPos, valGet.val.abyValue, 4);
		}
		pRepDataPos += nGetValSize;
		nDataSize += nGetValSize;
        //handled params
		nHandledParams++;
	} 

    //TRACE("----Now handled paramvLue total->%d---\n", nHandledParams);

	if((nDataSize > 0) && (!pWebRough->bIsRegistered))
	{
		pWebRough->bIsRegistered = TRUE;
		pWebRough->tmRegiseterd = _NOW_;
	}
	
    return nDataSize;
}

int Get_ReadbackVals(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
    
    int errCode;
    int nDataSize = 0;
    int n;	
    int nParamTpTotal = 0;
    int nType;
    PARAM_VAL_GET valGet;
	OBJ_PTR arg;
    int nGetValSize;
    int nGetParamTp;
    int nReqqqqqDataLen = pWebRough->nParsed_InDataLen;
    BYTE* pReqqqqDataPos =  pWebRough->pbyParsed_InDataPos;
    int nHandledParams = 0;
	while(nReqqqqqDataLen > 0)
	{
        nGetParamTp = *(BYTE*)pReqqqqDataPos;
		memcpy(varV.abyValue, pReqqqqDataPos + 1, 4);
		arg.nId = varV.lValue;
		memcpy(varV.abyValue, pReqqqqDataPos + 5, 4);
		arg.nSubId = varV.lValue;
		memcpy(varV.abyValue, pReqqqqDataPos + 9, 4);
		nGetValSize = varV.lValue;

		pReqqqqDataPos += 13;
		nReqqqqqDataLen -= 13;
		ZERO_POBJS(&valGet, 1);
		valGet.nGetSize = nGetValSize;
        TRACEX("---Now try to get the param[%d, %d, %d] value, size->%d---\n",
            arg.nId, nGetParamTp, arg.nSubId, nGetValSize);
		if((nGetValSize > 4) && (nGetValSize < 1024*1024*2)) //max support 2M for single blob
		{
			valGet.val.blobValue = NEW(BYTE, nGetValSize);
		}
        else
        {
           valGet.nGetSize = 4;
           nGetValSize = 4;
        }
        
		errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_READBACK_VAL,
			arg.nId, arg.nSubId, &valGet, sizeof(PARAM_VAL_GET));
		if(errCode != ERRCODE_DAI_OK)
		{
			TRACE("jjjjjjj arg.Id = %d, arg.nSubId = %d, nDataLen = %d, tp = %d, nValSize = %d, and ErrCode = %d\n", 
				arg.nId, arg.nSubId, nReqqqqqDataLen, nGetParamTp, nGetValSize, errCode);
			pWebRough->nParseHandleRTN = WEB_ERRCODE_WRONG_FORMAT;
			// varV.lValue = nHandledParams; //notice qt which param not correct
			// memcpy(pRepDataPos, varV.abyValue, 4);
			nDataSize = 0;
			if(nGetValSize > 4)
			{
				DELETE(valGet.val.blobValue);
			}
			return 0;
		}
		//now copy result
		varV.ulValue = valGet.nValueAttribute;
		memcpy(pRepDataPos, varV.abyValue, 4);
		pRepDataPos += 4;
		nDataSize += 4;
		if(nGetValSize > 4)
		{
			memcpy(pRepDataPos, valGet.val.blobValue, (size_t)nGetValSize);
            DELETE(valGet.val.blobValue);
		}
		else
		{
			memcpy(pRepDataPos, valGet.val.abyValue, 4);
		}
		pRepDataPos += nGetValSize;
		nDataSize += nGetValSize;
        //handled params
		nHandledParams++;
	} 

	if((nDataSize > 0) && (!pWebRough->bIsRegistered))
	{
		pWebRough->bIsRegistered = TRUE;
		pWebRough->tmRegiseterd = _NOW_;
	}
	
    return nDataSize;
}

int Set_ParamCtrlSet(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
	REPORTER* pReporter = pWebRough->pReporter;
    VAR_VALUE varV;
	OBJ_PTR arg;
	int errCode;

	pWebRough->nParseHandleRTN = WEB_ERRCODE_OK;
    int nValSize;
	int nReqqqqqDataLen = pWebRough->nParsed_InDataLen;
    BYTE* pReqqqqDataPos =  pWebRough->pbyParsed_InDataPos;
	DAI_TYPE tp = (pReqqqqDataPos[0] == 1) ? DAITYPE_CTRL_PARAMVALUE_RW : DAITYPE_SETT_PARAMVALUE_RW;
	memcpy(varV.abyValue, pReqqqqDataPos + 1, 4);
	arg.nId = varV.lValue;
	memcpy(varV.abyValue, pReqqqqDataPos + 5, 4);
	arg.nSubId = varV.lValue;
	memcpy(varV.abyValue, pReqqqqDataPos + 9, 4);
	nValSize = varV.lValue;
	//now go to the datapos
	pReqqqqDataPos += 13;
	nReqqqqqDataLen -= 13;
	if((nReqqqqqDataLen - nValSize < 0) || (nValSize < 4))
	{
		pWebRough->nParseHandleRTN = WEB_ERRCODE_WRONG_LEN;
	}
	else
	{
		//Jimmy added 20200605 specially handle the SYSTEM cmd from Qt
		//Jimmy removed it 20200610 since it semms not nessessary here
		// if((tp == DAITYPE_CTRL_PARAMVALUE_RW) && (arg.nSubId == 9999))
		// {				
		// 	char szSysCmd[256];
		// 	snprintf(szSysCmd, 256, (char*)pReqestPos);		
		// 	TRACEX("---Now begin run the system cmd[%s]---", szSysCmd);
		// 	int nRet = _SYSTEM(szSysCmd);		
		// 	errCode = (nRet == /*_SYSTEM_ERR_OK*/ 0) ? ERRCODE_DAI_OK:
		// 				ERRCODE_DAI_EXECUTE_ERR;
		// 	TRACEX("---Now begin run the system cmd[%s] for Qt, ret=%d---\n", 
		// 		szSysCmd, nRet);
		// }
		// else
		{
			memset(&varV, 0, sizeof(VAR_VALUE));
			memcpy(varV.abyValue, pReqqqqDataPos, 4);
			if(nValSize > 4)
			{
				varV.blobValue = NEW(BYTE, nValSize);
				memcpy(varV.blobValue, pReqqqqDataPos, nValSize);
			}
			errCode = pReporter->pDAI->Set((HANDLE)pReporter, tp, 
				arg.nId, arg.nSubId, &varV, nValSize, MAX_WAIT_TIMEOUT_SEC_4SETCTRL);
			TRACEX("-----Web set [%d, %d, %d], nTimeOut:%d, valSize : %d, to [%d], err = %d-----\n", 
				tp, arg.nId, arg.nSubId, MAX_WAIT_TIMEOUT_SEC_4SETCTRL, nValSize, varV.lValue, errCode);	
		
			if(nValSize > 4)
			{
				DELETE(varV.blobValue);
			}			
		}
		if(errCode != ERRCODE_DAI_OK)
		{
			TRACEX("-eeeeeeeeeeeeeeeeeeeeerrrrr-\n");
			pWebRough->nParseHandleRTN = WEB_ERRCODE_EXECUTE_FAIL;
		}
	}

    return 0;
}

// int Query_FirmwUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
// {
//     return 0;
// }
/*
int Get_ChgProfiles(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
	PARAM_VAL_GET valGet;
    ZERO_POBJS(&valGet, 1);
	valGet.nGetSize = sizeof(A_BLOB_CHARGINGPROFILE);
	valGet.val.blobValue = NEW(A_BLOB_CHARGINGPROFILE, 1);
    int errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 15, &valGet, sizeof(PARAM_VAL_GET));
	if(errCode != ERRCODE_DAI_OK)
	{
		TRACEX("----the chgprofiles can NOTTTTTT be fetched %d----\n", errCode);
		pWebRough->nParseHandleRTN = WEB_ERRCODE_EXECUTE_FAIL;
		DELETE(valGet.val.blobValue);
		return 0;		
	}
	A_BLOB_CHARGINGPROFILE* pChgProfiles = (A_BLOB_CHARGINGPROFILE*)valGet.val.blobValue;
	TRACEX("----The get-chgfProfils,maxCP:%d, nGetSize%d---\n", 
		pChgProfiles->nInstalled_MaxCP, valGet.nGetSize);
	memcpy(pRepDataPos, valGet.val.blobValue, sizeof(A_BLOB_CHARGINGPROFILE));
	DELETE(valGet.val.blobValue);
    return valGet.nGetSize;
}
*/
// int Handle_LoacalListUpdt(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
// {
//     return 0;
// }
/*
int Get_LocalList(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
    REPORTER* pReporter = pWebRough->pReporter;
	VAR_VALUE varV;
	//first get locallist Version
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETLOCALLIST_VERSION,
		0, 0, &varV.lValue, 4);
	int nDataSize = 0;
	memcpy(pRepDataPos, varV.abyValue, 4);
	pRepDataPos += 4;
	nDataSize += 4;
	//second the locallist item total
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETLOCALLIST_TOTAL,
		0, 0, &varV.lValue, 4);
	if(varV.lValue < 0 || (varV.lValue > MAX_LOCALLIST_ITEMS))
	{
		pWebRough->nParseHandleRTN = WEB_ERRCODE_OUT_OF_RANGE;
		return 0;
	}
	varV.lValue = MIN(varV.lValue, WEB_SUPPORTED_MAX_LOCALLIST); //max get total should not exceed
	memcpy(pRepDataPos, varV.abyValue, 4);
	pRepDataPos += 4;
	nDataSize += 4;
	//now get the items
	int nGetItem = varV.lValue;
	if(nGetItem > 0)
	{
		A_BLOB_UPDATE_LOCALLIST items;
		ZERO_POBJS(&items, 1);
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_GETLOCALLIST_ITEMS,
				0, nGetItem - 1, &items, sizeof(A_BLOB_UPDATE_LOCALLIST));
		if(items.nAuthorizationDataNumber != nGetItem)
		{
			TRACEX("----the items does NNNNNNOT match----\n");
			pWebRough->nParseHandleRTN = WEB_ERRCODE_WRONG_LEN;
			return 0;
		}
		//not copy the authdata
		int n;
		AUTHORIZATION_DATA* pAtuh = items.authorizationDataList;
		int nAuthSize = sizeof(AUTHORIZATION_DATA);
		for(n = 0; n < items.nAuthorizationDataNumber; n++, pAtuh++)
		{
			memcpy(pRepDataPos, pAtuh, nAuthSize);
			pRepDataPos += nAuthSize;
			nDataSize += nAuthSize;
		}
	}

    return nDataSize;
}
*/
/*
int Get_LocalCache(WEBAPI_REPORTER_ROUGH* pWebRough, BYTE* pRepDataPos)
{
	REPORTER* pReporter = pWebRough->pReporter;
	PARAM_VAL_GET valGet;
    ZERO_POBJS(&valGet, 1);
	valGet.nGetSize = sizeof(A_BLOB_AUTHORIZECACHE);
	valGet.val.blobValue = NEW(A_BLOB_AUTHORIZECACHE, 1);
    int errCode = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			301, 7, &valGet, sizeof(PARAM_VAL_GET));
	if(errCode != ERRCODE_DAI_OK)
	{
		TRACEX("----the cache can NOTTTTTT be fetched %d----\n", errCode);
		pWebRough->nParseHandleRTN = WEB_ERRCODE_EXECUTE_FAIL;
		DELETE(valGet.val.blobValue);
		return 0;		
	}
	A_BLOB_AUTHORIZECACHE* pCache = (A_BLOB_AUTHORIZECACHE*)valGet.val.blobValue;
	TRACEX("----The send-localccaeee%d, nGetSize%d---\n", 
		pCache->nAuthorizationDataNumber, valGet.nGetSize);
	memcpy(pRepDataPos, valGet.val.blobValue, sizeof(A_BLOB_AUTHORIZECACHE));
	DELETE(valGet.val.blobValue);
    return valGet.nGetSize;
}*/