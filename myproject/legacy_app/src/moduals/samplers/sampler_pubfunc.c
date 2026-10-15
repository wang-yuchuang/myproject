// #define _TRACE_PRINT_
#include "std_sys.h"
#include "app_incs.h"
#include "sampler_base.h"

int SetCommStatus(HANDLE hSampler, int nDevId, BOOL bCommOK)
{
	EVENT_ARG eventArg;
	ZERO_POBJS(&eventArg, 1);
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	eventArg.emEvType = EVENT_TYPE_COMM_FAIL;
	eventArg.emMsgType = bCommOK ? EVENT_MSG_END : EVENT_MSG_BEGIN;
	eventArg.nEvDataSize = sizeof(OBJ_PTR);
	OBJ_PTR obj;
	obj.nId = pSampler->info.nSamplerID;
	obj.nSubId = nDevId; //nDevId = -1, means all sampler comm fail
	eventArg.pEvData = (void*)&obj;
	//no block
	eventArg.bBlockMode = FALSE;

	return pDPR->Set(hSampler, SETTYPE_SENDMSG,
		&eventArg, sizeof(EVENT_ARG));
}

int SendMsg(HANDLE hSampler, _EVENT_TYPE evTp, _EVENT_MSG msgTp, void* pMsgData, int nMsgDataSize)
{
	EVENT_ARG eventArg;
	ZERO_POBJS(&eventArg, 1);
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	eventArg.emEvType = evTp;
	eventArg.emMsgType = msgTp;
	eventArg.nEvDataSize = (UINT)nMsgDataSize;
	eventArg.pEvData = pMsgData;
	//no block
	eventArg.bBlockMode = FALSE;

	return pDPR->Set(hSampler, SETTYPE_SENDMSG,
		&eventArg, sizeof(EVENT_ARG));
}

int SendMsg_Urgent(HANDLE hSampler, _EVENT_TYPE evTp, _EVENT_MSG msgTp, void* pMsgData, int nMsgDataSize)
{
	EVENT_ARG eventArg;
	ZERO_POBJS(&eventArg, 1);
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	eventArg.emEvType = evTp;
	eventArg.emMsgType = msgTp;
	eventArg.nEvDataSize = (UINT)nMsgDataSize;
	eventArg.pEvData = pMsgData;
	//no block
	eventArg.bBlockMode = TRUE;

	return pDPR->Set(hSampler, SETTYPE_SENDMSG,
		&eventArg, sizeof(EVENT_ARG));
}


//for only 4-byte data
int Push_VarValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, VAR_VALUE varV)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = 4;
	setData.varValue = varV;
	setData.bRawInvalid = (varV.lValue == SAMP_INVALID_VALUE) ? TRUE: FALSE;
	
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	return (int)pDPR->Set(hSampler, setType,
		&setData, sizeof(INTERSET_DATA));
}

//for only Reread data and only 4-byte data
int Push_VarValue_RereadPCS(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, VAR_VALUE RereadvarV, int nWhichSingleModule)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = 4;
	setData.varValue = RereadvarV;
	setData.bRawInvalid = (RereadvarV.lValue == SAMP_INVALID_VALUE) ? TRUE: FALSE;
	setData.nWhichSingleModule = nWhichSingleModule;
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	return (int)pDPR->Set(hSampler, setType,
		&setData, sizeof(INTERSET_DATA));
}
////for only-4bytes
//int Pull_VarValue(INTERGET_TYPE getType, HANDLE hSampler, PARAM_TYPE emParamTp, int nDevId, int nParamId, VAR_VALUE *pVar)
//{
//	SAMPLER* pSampler = (SAMPLER*)hSampler;
//	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
//
//	if(emParamTp == PARAM_TYPE_STATUS)
//	{
//		ERRCDDE_DPR err = pDPR->Get(hSampler, 
//			bGetRaw ? GETTYPE_SP_RAW_VAL : GETTYPE_SP_VAR,
//			nDevId, nParamId,
//			pVar, 4);
//
//		return err;
//	}
//	else if(emParamTp == PARAM_TYPE_SETTING)
//	{
//		ERRCDDE_DPR err = pDPR->Get(hSampler, 
//			GETTYPE_ST_VAR, nDevId, nParamId,
//			pVar, 4);
//
//		return err;
//	}
//
//	return ERR_DPR_INVALID_ARG;
//}

int Push_BlobValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, void* pBlob, int nValSize, BLOB_TYPE tp)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	*((BLOB_TYPE*)pBlob) = tp;
	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = nValSize;
	setData.varValue.blobValue = pBlob;
	setData.bRawInvalid = FALSE;

	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int nRet =  (int)pDPR->Set(hSampler, setType,
		&setData, sizeof(INTERSET_DATA));

	TRACE("\n----------Now push[%d, %d] value, nSize = %d, nRet = %d----------\n",
		nDevId, nParamId, nValSize, nRet);

	return nRet;
}

BOOL Wait_BlobCtrlCmd(HANDLE hSampler, int nDevId, int nParamId, void* pBlob, int nValSize, BLOB_TYPE tp, int nTimeout_ms)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	*((BLOB_TYPE*)pBlob) = tp;
	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = nValSize;
	setData.varValue.blobValue = pBlob;
	setData.bRawInvalid = FALSE;
	setData.nTimeOut = nTimeout_ms;

	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int nRet =  (int)pDPR->Set(hSampler, SETTYPE_INTERN_CTRL,
		&setData, sizeof(INTERSET_DATA));

	TRACE("\n----------Now Ctrl[%d, %d] value, nSize = %d, nRet = %d----------\n",
		nDevId, nParamId, nValSize, nRet);

	return (ERR_DPR_OK == nRet) ? TRUE : FALSE;
}

int Push_BlobValue_Ex(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, void* pBlob, int nValSize, BLOB_TYPE tp, int nTimeOut_MSec)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);
	*((BLOB_TYPE*)pBlob) = tp;
	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = nValSize;
	setData.varValue.blobValue = pBlob;
	setData.bRawInvalid = FALSE;
	setData.nTimeOut = nTimeOut_MSec;

	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int nRet =  (int)pDPR->Set(hSampler, setType,
		&setData, sizeof(INTERSET_DATA));

	TRACE("\n----------Now pushEx[%d, %d] value, nSize = %d, nTimeOut = %d, nRet = %d----------\n",
		nDevId, nParamId, nValSize, nTimeOut_MSec, nRet);

	return nRet;
}
int Push_TextValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, char* pString, int nValSize)
{
	INTERSET_DATA setData;
	ZERO_POBJS(&setData, 1);

	setData.nDevId = nDevId;
	setData.nParamId = nParamId;
	setData.nValueSize = nValSize;
	setData.varValue.blobValue = (void*)pString;
	setData.bRawInvalid = FALSE;

	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int nRet =  (int)pDPR->Set(hSampler, setType,
		&setData, sizeof(INTERSET_DATA));

	TRACE("\n----------Now pushString [%d, %d] value, nSize = %d, nRet = %d----------\n",
		nDevId, nParamId, nValSize, nRet);

	return nRet;
}

ESSYS_TYPE GetESSysType(HANDLE hSampler)
{
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ESSYS_TYPE emESSystemType;
	VAR_VALUE varV;
	
	pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 8, &varV, 4);
	
	emESSystemType = (ESSYS_TYPE)varV.emValue;
	
	/*if(varV.emValue == 0)
	{
		emESSystemType = ESSTYPE_UNINSTALL;
 	}
	else if(varV.emValue == 1)
	{	
		emESSystemType = ESSTYPE_UDP_SERVER;
	}
	else
	{
		emESSystemType = ESSTYPE_UDP_CLIENT1;
	}*/

	return emESSystemType;
}

void DecodeRectBarcode(const UINT* puiBarCode, char* pszDest, int nDestLen)
{
    if (!puiBarCode || !pszDest || nDestLen <= 0) {
        return;
    }

    UINT b1 = puiBarCode[0];
    UINT b2 = puiBarCode[1];
    UINT b3 = puiBarCode[2];
    UINT b4 = puiBarCode[3];

    BYTE factory = (b1 >> 20) & 0x0F;
    BYTE verLetter = (b1 >> 16) & 0x0F;
    BYTE verNum = (b1 >> 24) & 0xFF;

    BYTE yearMinus2000 = (b2 >> 0) & 0xFF;
    BYTE month = (b2 >> 12) & 0x0F;
    BYTE type = (b2 >> 8) & 0x0F;
    BYTE shortSerial = (USHORT)(((b2 >> 16) & 0xFF) << 8 | ((b2 >> 24) & 0xFF));

    char longSerial[9];
    memcpy(&longSerial[0], &b3, 4);
    memcpy(&longSerial[4], &b4, 4);
    longSerial[8] = '\0';

    snprintf(pszDest, nDestLen, "%u%02u%02u%05u%u%s %X%02X",
             factory, yearMinus2000, month, shortSerial, type, longSerial, verLetter, verNum);
}

UINT GetClientIndexLC_Sampler(HANDLE hSampler)
{
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	
	ERRCDDE_DPR err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 42, &varV, 4);
	UINT ret = (err==ERR_DPR_OK) ? varV.ulValue : 0;
	TRACED("sampler_pubfunc:GetClientIndexLC_Sampler %d", ret);
	return ret;
}

BOOL IsClientLC_Sampler(HANDLE hSampler)
{
	UINT idxClientLC = GetClientIndexLC_Sampler(hSampler);
	BOOL ret = (idxClientLC==0 ? FALSE : TRUE);
	TRACED("sampler_pubfunc:IsClientLC_Sampler:%d", ret);
	return ret;
}

LCCfg GetLCCfg_Sampler(HANDLE hSampler)
{
	LCCfg lcCfg;
	ZERO_POBJS(&lcCfg, 1);
	
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	
	ERRCDDE_DPR err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 100, &varV, 4);
	lcCfg.emSystemType = (SYSTEM_TYPE)(err==ERR_DPR_OK ? varV.emValue : SYSTYPE_ESS_AIR);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 7, &varV, 4);
	lcCfg.emApplyScenery = (POS_AUXMETER)(err==ERR_DPR_OK ? varV.emValue : AUXMETER_NOT_INSTALLED);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 42, &varV, 4);
	lcCfg.idxClientLC = (UINT)(err==ERR_DPR_OK ? varV.ulValue : 0);

	lcCfg.lcType = GetLCType(lcCfg.idxClientLC);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1001, 3313, &varV, 4);
	lcCfg.bSendCmdByLC1stOnly = (BOOL)(err==ERR_DPR_OK ? varV.emValue : FALSE);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1001, 3314, &varV, 4);
	lcCfg.nESCNum = (UINT)(err==ERR_DPR_OK ? varV.ulValue : 0);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1001, 3315, &varV, 4);
	lcCfg.nBranchNum = (UINT)(err==ERR_DPR_OK ? varV.ulValue : 0);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 407, 401, &varV, 4);
	lcCfg.bCabMeter = (BOOL)(err==ERR_DPR_OK ? (varV.emValue>0?TRUE:FALSE) : FALSE);

	err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 408, 401, &varV, 4);
	lcCfg.bGridMeter = (BOOL)(err==ERR_DPR_OK ? (varV.emValue>0?TRUE:FALSE) : FALSE);

	TRACED("sampler_pubfunc:GetLCCfg_Sampler emSystemType:%d emApplyScenery:%d idxClientLC:%u lcType:%d "
		"bSendCmdByLC1stOnly:%d nESCNum:%u nBranchNum:%u bCabMeter:%d bGridMeter:%d",
		lcCfg.emSystemType, lcCfg.emApplyScenery, lcCfg.idxClientLC, lcCfg.lcType,
		lcCfg.bSendCmdByLC1stOnly, lcCfg.nESCNum, lcCfg.nBranchNum, lcCfg.bCabMeter, lcCfg.bGridMeter);
	return lcCfg;
}

ESS_DEVICE_TYPE GetESSDeviceType_Sampler(HANDLE hSampler)
{
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ESS_DEVICE_TYPE emDevType;
	VAR_VALUE varV;
	
	ERRCDDE_DPR err = pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 101, &varV, 4);
	emDevType = (err==ERR_DPR_OK) ? varV.emValue : DEFAULT_DEVICE;
	TRACED("sampler_pubfunc:GetESSDeviceType_Sampler %d", emDevType);
	return emDevType;
}

COMMPORT* GetCommPort_ById(HANDLE hSampler, int nPortId, COMMPORT* pComm)
{
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int err = pDPR->Get(hSampler, GETTYPE_COMMPORT,
		nPortId, NULL, pComm, sizeof(COMMPORT));

	if(err != ERR_DPR_OK)
	{
		return NULL;
	}
	TRACEI("sampler_pubfunc:GetCommPort_ById nPortId:%d GetCommPort_ById:<%s:%s>", nPortId, pComm->info.szPortAttri, pComm->info.szPortName);

	return pComm;
}

static BOOL IsMeterInstalled(DATA_PROCESSOR* pDPR, HANDLE hSampler, UINT devID)
{
    VAR_VALUE varV;
    pDPR->Get(hSampler, GETTYPE_ST_VAR, devID, 401, &varV, 4);
    return (varV.emValue != 0);
}

static BOOL IsMeterCommOk(DATA_PROCESSOR* pDPR, HANDLE hSampler, UINT devID)
{
    VAR_VALUE varV;
    pDPR->Get(hSampler, GETTYPE_ALMSTATE, devID, 401, &varV, 4);
    return (varV.emValue == ALARM_STATUS_CEASED);
}

static float GetMeterPower(DATA_PROCESSOR* pDPR, HANDLE hSampler, UINT devID)
{
    VAR_VALUE varV;
    return (pDPR->Get(hSampler, GETTYPE_SP_VAR, devID, 400, &varV, 4) == ERR_DPR_OK)
           ? varV.fValue : 0.0f;
}

float CalcMeterByApplyScenery(HANDLE hSampler, METER_TYPES MeterType)
{
    SAMPLER* pSampler = (SAMPLER*)hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    GRID_APPLY_SCENERY uApplyScenery = SCENERY_NOT_INSTALL;
    VAR_VALUE varV;
    UINT kkk;
    float RetValue = 0.0f;

    /* ---------------- 柜内表：直接累加 ---------------- */
    if (MeterType == METER_TYPES_CAB)
    {
        for (kkk = 0; kkk < METER_TYPE_NUM; kkk++)
        {
            UINT devID = All_TypesMetersID[MeterType][kkk];
            if (IsMeterInstalled(pDPR, hSampler, devID))
            {
                RetValue += GetMeterPower(pDPR, hSampler, devID);
            }
        }

        // printf("TIME[%0.3f] CalcMeterByApplyScenery CAB RetValue = [%0.1f]\n", GetCurrentTime(),RetValue);
        return RetValue;
    }

    /* ---------------- 网侧表：获取应用场景 ---------------- */
    pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 110, &varV, 4);
    uApplyScenery = (GRID_APPLY_SCENERY)varV.emValue;

    for (kkk = 0; kkk < METER_TYPE_NUM; kkk++)
    {
        UINT devID = All_TypesMetersID[MeterType][kkk];

        if (!IsMeterInstalled(pDPR, hSampler, devID))
            continue;

        /* 主备：找到第一个通信正常的直接返回 */
        if (uApplyScenery == SCENERY_ACTIVE_STANDBY)
        {
            if (IsMeterCommOk(pDPR, hSampler, devID))
            {
				UINT nSelectGridMeterId = 0;
				pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 111, &varV, 4); // Primary meter Select
				nSelectGridMeterId = varV.emValue;
				if(nSelectGridMeterId == 0) // 程序自动选择主备
				{
					float f_ActPower = 0.0f, f_ActPowerCheckThreshold = 10.0f;
					pDPR->Get(hSampler, GETTYPE_SP_VAR, devID, 400, &varV, 4); // RealTime Act Power
					f_ActPower = varV.fValue;
					pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 112, &varV, 4);
					f_ActPowerCheckThreshold = varV.fValue;
					TRACED("f_ActPowerCheckThreshold == [%f]\n", f_ActPowerCheckThreshold);
					if(fabs(f_ActPower) < f_ActPowerCheckThreshold) // 备
					{
						continue;
					}
					RetValue = GetMeterPower(pDPR, hSampler, devID);
				}
				else // 408,440,441,442
				{
					UINT meterDevId = 0;
					switch(nSelectGridMeterId)
					{
						case 1 : meterDevId = 408; break;
						case 2 : meterDevId = 440; break;
						case 3 : meterDevId = 441; break;
						case 4 : meterDevId = 442; break;
						default: break;
					}
					RetValue = GetMeterPower(pDPR, hSampler, meterDevId);
					TRACED("CurrMeterDevId=[%d],fGridPower=[%f]\n", meterDevId, RetValue);
				}

				pDPR->Get(hSampler, GETTYPE_SP_VAR, 1, 60, &varV, 4); // Curr Primary meterDevId
				TRACED("slectId=[%d], mPriMeterId=[%d], devId=[%d]\n", nSelectGridMeterId, varV.ulValue, devID);
				if(varV.ulValue != devID)
				{
					varV.ulValue = devID;
					Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 1, 60, varV);// push Primary Meter Id;
					TRACED("PUSH!\n");
				}


				TRACED("TIME[%0.3f] GRID ACTIVE_STANDBY devID[%d] RetValue = [%0.1f]\n", GetCurrentTime(), devID, RetValue);
                return RetValue;
            }
        }
        /* 多支路：累加 */
        else if (uApplyScenery == SCENERY_MULTI_BRANCH)
        {
            RetValue += GetMeterPower(pDPR, hSampler, devID);
        }
		else
		{
			RetValue = GetMeterPower(pDPR, hSampler, 408);
			printf("TIME[%0.3f] GRID SCENERY_NOT_INSTALL RetValue = [%0.1f]\n", GetCurrentTime(), RetValue);
			return RetValue;
		}
    }

    if (uApplyScenery == SCENERY_MULTI_BRANCH)
    {
        printf("TIME[%0.3f] GRID MULTI_BRANCH RetValue = [%0.1f]\n", GetCurrentTime(),RetValue);
    }

    return RetValue;
}

static BOOL IsTime_InRange(time_t tmStart, UINT nInterv, ES_MGMT_REPEATTYPE emRepeatTp, BOOL bMatchmDayOK)
{
	time_t tmNow = _NOW_;
	struct tm gmTime_Now;
	gmtime_r(&tmNow, &gmTime_Now);

	struct tm gmTime_Sec_Start;
	struct tm gmTime_Sec_Stop;
	time_t tmStop = tmStart + nInterv;
	gmtime_r(&tmStart, &gmTime_Sec_Start);
	gmtime_r(&tmStop, &gmTime_Sec_Stop);

	if(emRepeatTp == ESMGMT_REPEAT_DAILY) //daily data should only from 0~24, no over-night
	{
		//do nothing
	}
	else if(emRepeatTp == ESMGMT_REPEAT_WEEKLY) //daily data should only from week 0-7, no over-week
	{
		if((gmTime_Now.tm_wday < gmTime_Sec_Start.tm_wday) || 
				(gmTime_Now.tm_wday > gmTime_Sec_Stop.tm_wday))
		{
			return FALSE;
		}		
	}
	else if(emRepeatTp == ESMGMT_REPEAT_MONTHLY) //daily data should only from month 0-30, no over-month
	{
		if((gmTime_Now.tm_mday < gmTime_Sec_Start.tm_mday) || 
				(gmTime_Now.tm_mday > gmTime_Sec_Stop.tm_mday))
		{
			return FALSE;
		}
	}

	gmTime_Sec_Start.tm_year = gmTime_Now.tm_year;
	gmTime_Sec_Start.tm_mon = gmTime_Now.tm_mon;
	//John 2022-12-10 Added For Match mDay to Support MultiDay Sector EMS Plan.
	if(bMatchmDayOK == FALSE)
	{
		gmTime_Sec_Start.tm_mday = gmTime_Now.tm_mday;
		//printf("111 bMatchmDayOK=[%d] tm_mday=[%ld], tm_mday=[%ld]\n", bMatchmDayOK, gmTime_Sec_Start.tm_mday, gmTime_Now.tm_mday);
	}
	time_t tmNewStart = mktime(&gmTime_Sec_Start);		
	if(tmNow >= tmNewStart)
	{
		UINT nDiff = tmNow - tmNewStart;
		if(nDiff < nInterv) // 20220219 John Modify <= To <,Avoid **:**:00 Or **:30:00 is In Sector 1,But Reality is Sector 2
		{
			//printf("222 bMatchmDayOK=[%d] nDiff=[%ld],tmNow=[%ld] tmNewStart=[%ld] nInterv=[%d]\n", \
			//	bMatchmDayOK, nDiff,tmNow, tmNewStart, nInterv);

			return TRUE;
		}
	}

	return FALSE;
}

static BOOL IsTime_InRange_DayOrMonth(time_t tmStart, UINT nInterv, time_t tmEdn)
{
	time_t tmNow = _NOW_;
	struct tm gmTime_Now;
	gmtime_r(&tmNow, &gmTime_Now);

	struct tm gmTime_Sec_Start;
	struct tm gmTime_Sec_Stop;
	time_t tmStop = tmStart + nInterv;
	gmtime_r(&tmStart, &gmTime_Sec_Start);
	gmtime_r(&tmStop, &gmTime_Sec_Stop);

	//by WLD01206 2025/7/2
	//7月2号到7月8号这段时间需要运行(10点开始放电的话)
	//把时间更新到现在，再去判断启动时间是否大于7月8号10点，小于等于表示现在可以运行这个时段
	gmTime_Sec_Start.tm_year = gmTime_Now.tm_year;
	gmTime_Sec_Start.tm_mon = gmTime_Now.tm_mon;
	gmTime_Sec_Start.tm_mday = gmTime_Now.tm_mday;

	
	time_t tmNewStart = mktime(&gmTime_Sec_Start);	
	if(tmNewStart > tmEdn)
	{//大于开始时间直接返回
		return FALSE;
	}

	if(tmNow >= tmNewStart)
	{
		UINT nDiff = tmNow - tmNewStart;
		if(nDiff < nInterv)
		{
/* 			printf("222  nDiff=[%d] nInterv=[%d] month=[%d] day=[%d] hour=[%d] min=[%d] sec=[%d]\n", \
				nDiff, nInterv, gmTime_Sec_Start.tm_mon, gmTime_Sec_Start.tm_mday, 
				gmTime_Sec_Start.tm_hour, gmTime_Sec_Start.tm_min, gmTime_Sec_Start.tm_sec); */
			return TRUE;
		}
	}

	return FALSE;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Get_EmsCfgPlan_Setctor
*  CREATOR       :  
*  CREATED DATE  : 2020-11-19 14:33:53
*  DESCRIPTION   :  EMS Config Plan 
*  
*  CHANGE HISTORY: 

*	 contact with ZTZ, Just Use one Plan for multi group Battery Mgmt
*    // LJH250808 从控只取管理计划第一段 added:bFromOrg 是否从原始数据获取
***********************************************************************************/
int Get_EmsCfgPlan_Setctor(A_BLOB_EMS_CONFIG_PLAN *pEmsCfgPlan, BOOL bFromOrg)
{
	if (!bFromOrg)
	{
		// LJH250808 从控只取管理计划第一段
		return 0;
	}

	int		nFoundSec = -1;
	int		n = 0;
	time_t	tmNow = _NOW_;
 	struct	tm gmTime_Now;
	gmtime_r(&tmNow, &gmTime_Now);

	time_t	tmStart;
	struct tm gmTime_Sec_Start;
	BOOL bMatchmDayOK = FALSE;

	for(n = 0; n < pEmsCfgPlan->nTotal_EmsLmt_Sectors; n++)
	{
		tmStart = pEmsCfgPlan->emsLmtSectors[n].tmStart;
		gmtime_r(&tmStart, &gmTime_Sec_Start);
		
		//printf("111 n=[%d] tmStart=[%ld] emsLmtSectors[n].tmStart=[%ld]\n", \
		//	n, tmStart, pEmsCfgPlan->emsLmtSectors[n].tmStart);

		if(gmTime_Sec_Start.tm_mday == gmTime_Now.tm_mday)
		{
			bMatchmDayOK = TRUE;
			break;
		}

	}

	for(n = 0; n < pEmsCfgPlan->nTotal_EmsLmt_Sectors; n++)
	{
		/*if((pEmsCfgPlan->emsLmtSectors[n].tmStart > tmNow) ||
			(pEmsCfgPlan->emsLmtSectors[n].nInterval < 60)) //at least 60 sec
		{
			continue;
		}*/
		//John Modify. If current is 2023-09-07,But EMS plan is 2023-09-08,EMS plan will Return -1.
		if(pEmsCfgPlan->emsLmtSectors[n].nInterval < 60) //at least 60 sec
		{
			continue;
		}

		if(IsTime_InRange(pEmsCfgPlan->emsLmtSectors[n].tmStart, 
				pEmsCfgPlan->emsLmtSectors[n].nInterval,
				ESMGMT_REPEAT_DAILY,
				bMatchmDayOK))
		{
			nFoundSec = n;
			break;
		}
	}

	return nFoundSec;
}


/*********************************************************************************
*  
*  FUNCTION NAME : Get_FeeRate_Setctor
*  CREATOR       :  
*  CREATED DATE  : 2020-11-19 14:33:53
*  DESCRIPTION   :  EMS Config Plan 
*  
*  CHANGE HISTORY: 

*	 contact with ZTZ, Just Use one Plan for multi group Battery Mgmt
*  
***********************************************************************************/
int Get_FeeRate_Setctor(A_BLOB_CHARGE_FEESET *pFeeRate)
{
	int 	nFoundSec = -1;
	int 	n = 0;
	time_t  tmNow = _NOW_;

	for(n = 0; n < pFeeRate->nTotal_FeeRate_Sectors; n++)
	{
		if((pFeeRate->feeRateSectors[n].tmStart > tmNow) ||
			(pFeeRate->feeRateSectors[n].nInterval < 60)) //at least 60 sec
		{
			continue;
		}

		if(IsTime_InRange(pFeeRate->feeRateSectors[n].tmStart, 
				pFeeRate->feeRateSectors[n].nInterval,
				ESMGMT_REPEAT_DAILY,
				FALSE))
		{
			nFoundSec = n;
			break;
		}
	}

	return nFoundSec;
}

static BOOL IsTimeNow_InRange(time_t tmStart, UINT nInterv, time_t tm_Now, ES_MGMT_REPEATTYPE emRepeatTp)
{
	time_t tmNow = tm_Now;
	struct tm gmTime_Now;
	gmtime_r(&tmNow, &gmTime_Now);

	struct tm gmTime_Sec_Start;
	struct tm gmTime_Sec_Stop;
	time_t tmStop = tmStart + nInterv;
	gmtime_r(&tmStart, &gmTime_Sec_Start);
	gmtime_r(&tmStop, &gmTime_Sec_Stop);

	if(emRepeatTp == ESMGMT_REPEAT_DAILY) //daily data should only from 0~24, no over-night
	{
		//do nothing
	}
	else if(emRepeatTp == ESMGMT_REPEAT_WEEKLY) //daily data should only from week 0-7, no over-week
	{
		if((gmTime_Now.tm_wday < gmTime_Sec_Start.tm_wday) || 
				(gmTime_Now.tm_wday > gmTime_Sec_Stop.tm_wday))
		{
			return FALSE;
		}		
	}
	else if(emRepeatTp == ESMGMT_REPEAT_MONTHLY) //daily data should only from month 0-30, no over-month
	{
		if((gmTime_Now.tm_mday < gmTime_Sec_Start.tm_mday) || 
				(gmTime_Now.tm_mday > gmTime_Sec_Stop.tm_mday))
		{
			return FALSE;
		}
	}

	gmTime_Sec_Start.tm_year = gmTime_Now.tm_year;
	gmTime_Sec_Start.tm_mon = gmTime_Now.tm_mon;
	gmTime_Sec_Start.tm_mday = gmTime_Now.tm_mday;
	time_t tmNewStart = mktime(&gmTime_Sec_Start);		
	if(tmNow >= tmNewStart)
	{
		UINT nDiff = tmNow - tmNewStart;
		if(nDiff < nInterv) // 20220219 John Modify <= To <,Avoid **:**:00 Or **:30:00 is In Sector 1,But Reality is Sector 2
		{
			return TRUE;
		}
	}

	return FALSE;
}

int Get_FeeRate_Setctor_Accroding2Time(A_BLOB_CHARGE_FEESET *pFeeRate, time_t  tm_Now)
{
	int 	nFoundSec = -1;
	int 	n = 0;
	time_t  tmNow = tm_Now;

	for(n = 0; n < pFeeRate->nTotal_FeeRate_Sectors; n++)
	{
		if((pFeeRate->feeRateSectors[n].tmStart > tmNow) ||
			(pFeeRate->feeRateSectors[n].nInterval < 60)) //at least 60 sec
		{
			continue;
		}

		if(IsTimeNow_InRange(pFeeRate->feeRateSectors[n].tmStart, 
				pFeeRate->feeRateSectors[n].nInterval,
				tmNow,
				ESMGMT_REPEAT_DAILY))
		{
			nFoundSec = n;
			break;
		}
	}

	return nFoundSec;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Get_ESPlan_Sector
*  CREATOR       :  
*  CREATED DATE  : 2020-11-19 14:33:53
*  DESCRIPTION   :  Battery Mgmt Plan 
*  
*  CHANGE HISTORY: 2021-6-21 Remove Battery Magmt,Use EMS Plan Instead.

*	 contact with ZTZ, Just Use one Plan for multi group Battery Mgmt
*  
***********************************************************************************/
//int Get_BattMgmt_Sector(A_BLOB_BATT_MGMT_PLAN * pBattMgmtPlan)
//{
//	int nFoundSec = -1; //-1 means not found
//	int n;
//	time_t tmNow = _NOW_;
//
//	for(n = 0; n < pBattMgmtPlan->nTotal_STBattLmt_Sectors; n++)
//	{
//		if((pBattMgmtPlan->stBattLmtSectors[n].tmStart > tmNow) ||
//			(pBattMgmtPlan->stBattLmtSectors[n].nInterval < 60)) //at least 60 sec
//		{
//			continue;
//		}
//
//		if(IsTime_InRange(pBattMgmtPlan->stBattLmtSectors[n].tmStart, 
//				pBattMgmtPlan->stBattLmtSectors[n].nInterval,
//				pBattMgmtPlan->emBattChgPlan_Repeat))
//		{
//			nFoundSec = n;
//			break;
//		}
//	}
//
//	return nFoundSec;
//}

BOOL DoMD5Chk_ForRemotePkg(BOOL bIs4Web)
{
	FILE *fPkg = fopen(bIs4Web ? "/opt/appdata/data/firm_tmp_extract/pkg.sum" : "/opt/appdata/data/pkg.sum", "r");
	if(fPkg == NULL)
	{
		TRACEX("Failed reason 1--\n");
		return FALSE;
	}

	char szPkgSum[33] = {0};
	if(fread(szPkgSum, sizeof(char), 32, fPkg) != 32)
	{
		TRACEX("Failed reason 2 [%s]--\n", szPkgSum);
		fclose(fPkg);
		return FALSE;
	}

	//added by Jimmy 20210531 to avoid wrongly upgrade for N3
	char szFlag_4N3xx[5] = {0};
	memset(szFlag_4N3xx, 0, 5);
	if(fread(szFlag_4N3xx, sizeof(char), 4, fPkg) != 4)
	{
		TRACEX("Failed reason 3 [%s]--\n", szFlag_4N3xx);
		fclose(fPkg);
		return FALSE;
	}
// LJH241101 MU3
#ifdef IMMU3
	// if(strcmp(szFlag_4N3xx, "mu03") != 0)
	if(strcmp(szFlag_4N3xx, "imu3") != 0)
	{
		TRACEX("Failed reason 4 [%s]--\n", szFlag_4N3xx);
		fclose(fPkg);
		return FALSE;
	}
#else
	if(strcmp(szFlag_4N3xx, "n3ge") != 0)
	{
		if(strcmp(szFlag_4N3xx, "n3xx") != 0) //Added By John 2022-11-14.For V3 BSP Only Support n3xx.
		{
			TRACEX("Failed reason 4 [%s]--\n", szFlag_4N3xx);
			fclose(fPkg);
			return FALSE;
		}
	}
#endif
	fclose(fPkg);

	char szSubTemp1[7] = {0};
	char szSubTemp2[11] = {0};
	char szSubTemp3[9] = {0};
	char szID1[5]= {0};
	char szID2[5]= {0};
	memcpy(szSubTemp2, szPkgSum, 10);
	memcpy(szID1, szPkgSum + 10, 4);
	memcpy(szSubTemp3, szPkgSum + 14, 8);
	memcpy(szID2, szPkgSum + 22, 4);
	memcpy(szSubTemp1, szPkgSum + 26, 6);

	TRACEX("-------szPkgSum = [%s], szSubTemp1 = [%s],"
		"szSubTemp2 = [%s], szSubTemp3 = [%s], szID1 = [%s], szID2 = [%s]------\n",
		szPkgSum, szSubTemp1, szSubTemp2, szSubTemp3, szID1, szID2);

	if((strcmp(szID1, "4da1") != 0) ||
		(strcmp(szID2, "f2c5") != 0))
	{
		TRACEX("Failed reason 3--\n");
		return FALSE;
	}
	
	_SYSTEM(bIs4Web ? "md5sum -b /opt/appdata/data/firm_tmp_extract/app_nccu.tar.gz > /opt/appdata/data/firm_tmp_extract/md5_calc" :
			"md5sum -b /opt/appdata/data/app_nccu.tar.gz > /opt/appdata/data/md5_calc");
	FILE *fCalc= fopen(bIs4Web ? "/opt/appdata/data/firm_tmp_extract/md5_calc" : "/opt/appdata/data/md5_calc", "r");
	if(fCalc == NULL)
	{
		TRACEX("Failed reason 4--\n");
		return FALSE;
	}

	char szCalcChk[33] = {0};
	if(fread(szCalcChk, sizeof(char), 32, fCalc) != 32)
	{
		TRACEX("Failed reason 5 [%s]--\n", szCalcChk);
		fclose(fCalc);
		return FALSE;
	}

	fclose(fCalc);

	char szCalcTemp1[7] = {0};
	char szCalcTemp2[11] = {0};
	char szCalcTemp3[9] = {0};
	memcpy(szCalcTemp1, szCalcChk + 1, 6);
	memcpy(szCalcTemp2, szCalcChk + 13, 10);
	memcpy(szCalcTemp3, szCalcChk + 20, 8);
	TRACEX("-------szCalcChk = [%s], szCalcTemp1 = [%s],"
		"szCalcTemp2 = [%s], szCalcTemp3 = [%s]------\n",
		szCalcChk, szCalcTemp1, szCalcTemp2, szCalcTemp3);

	if((strncmp(szSubTemp1, szCalcTemp1, 7) != 0) ||
		(strncmp(szSubTemp2, szCalcTemp2, 11) != 0) ||
		(strncmp(szSubTemp3, szCalcTemp3, 9) != 0))
	{
		TRACEX("Failed reason 6--\n");
		return FALSE;
	}

	return TRUE;
}


float GetTimeZone_OffSetHrs(HANDLE hSampler)
{
    float fOffset = 0.0;
	BYTE  byTimeZone[8] = {0};
	BYTE  bySubStr[2][3];
	BYTE  bySubStrNum = 0;
	char  *p, *ptr, cc;
	BOOL bMinus;
	int i, j, nHrs, nMin;
	SAMPLER* pSampler = (SAMPLER*)hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	varV.blobValue = byTimeZone;
	pDPR->Get(hSampler, GETTYPE_ST_VAR, 1, 66, &varV, 8);
	
    if(strlen(byTimeZone) > 0)
    {
		//printf("GetTimeZone_OffSetHrs: byTimeZone:%s\n", byTimeZone);
		if(strlen(byTimeZone) != 6)
		{
			//printf("GetTimeZone_OffSetHrs: byTimeZone Len Is Not 6\n");
			return fOffset;
		}

        cc = byTimeZone[0];
        if((cc != '+') && (cc != '-'))
        {
        	//printf("GetTimeZone_OffSetHrs: + - Error\n");
            return fOffset;
        }
		
        bMinus = (cc == '-') ? TRUE: FALSE;

		//remove first elem
		for(i=0; i<7; i++)
		{
			byTimeZone[i] = byTimeZone[i+1];
		}
		byTimeZone[7] = 0;

		//"08:00", According To ":", Cut Sub String
		ptr = strtok_r(byTimeZone, ":", &p);
		if((ptr != NULL) && (strlen(ptr) == 2))
		{
			memcpy(bySubStr[0], ptr, 2);
			bySubStrNum++;
		}
		else
		{	
			//printf("GetTimeZone_OffSetHrs: 111\n");
			return fOffset;
		}

		ptr = strtok_r(NULL, ":", &p);
		if((ptr != NULL) && (strlen(ptr) == 2))
		{
			memcpy(bySubStr[1], ptr, 2);
			bySubStrNum++;
		}
		else
		{
			//printf("GetTimeZone_OffSetHrs: 222\n");
			return fOffset;
		}

		if(bySubStrNum != 2)
		{
			//printf("GetTimeZone_OffSetHrs: 333\n");
			return fOffset;
		}

		for(i=0; i<2; i++)
		{
			for(j=0; j<2; j++)
			{
				if((bySubStr[i][j] < '0') || (bySubStr[i][j] > '9'))
				{
					//printf("GetTimeZone_OffSetHrs: 444\n");
					return fOffset;
				}
			}
		}
		
        nHrs = (bySubStr[0][0] - 0x30) * 10 + (bySubStr[0][1] - 0x30);
        nMin = (bySubStr[1][0] - 0x30) * 10 + (bySubStr[1][1] - 0x30);
		
		//printf("GetTimeZone_OffSetHrs: nHrs:%d, nMin:%d\n", nHrs ,nMin);
        if( (nHrs < 0) || (nHrs > 23) || ((nMin != 0) && (nMin != 30)) )
        {
        	//printf("GetTimeZone_OffSetHrs: 555\n");
            return fOffset;
        }
        fOffset = (float)nHrs + ((nMin == 30) ? 0.5 : 0.0);
        if(bMinus)
        {
           fOffset *= -1;
        }
    }
    return fOffset;
}

float Samp_Avg_Filter(FILTER_TYPE emType, float fRawVal, int nPointNum)
{
	float fNewVal = fRawVal;
	static float fData[FILTERTYPE_MAX][MAX_FILTER_POINT];
	int nIdx = emType;//(pBMSRough->nSamplerID == 2) ? 0 : 1;
	static int nEnterCounts[FILTERTYPE_MAX] = {0};
	float fResult = 0.0;
	int n;
	int nCount = MIN(MAX_FILTER_POINT, nPointNum);
		nCount = MAX(1, nCount);

	if(nEnterCounts[nIdx] < nCount)
	{
		fData[nIdx][nEnterCounts[nIdx]] = fNewVal;
		nEnterCounts[nIdx]++;
		fResult = fNewVal/nCount;
	}
	else
	{
		for(n = 0; n < nCount - 1; n++)
		{
			fData[nIdx][n] = fData[nIdx][n + 1];
			fResult += fData[nIdx][n];
		}
		fData[nIdx][nCount - 1] = fNewVal;
		fResult += fNewVal;
		fResult /= nCount;
		//TRACEX("-------------Filtered fVal = %.2f-------------------", fResult);
	}
	return fResult;
}

static USHORT AR_ParseUint16(BYTE *buf)
{
    return (USHORT)(buf[0] | (buf[1] << 8));
}

static UINT AR_ParseUint32(BYTE *buf)
{
    return (UINT)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

static int SafeCopyData(BYTE* pDst, const BYTE* pSrc, int nStartIdx, int nCopyLen, int nTotalLen)
{
	// 1. Check
	if (pDst       ==  NULL || pSrc      ==  NULL ||
		nStartIdx  <   0    || nCopyLen  <=  0
	)
	{
		TRACED("SafeCopyData nStartIdx:[%d], nCopyLen:[%d]\n", nStartIdx, nCopyLen);
		return FALSE;
	}

	// 2. Boundary Protect
	if (nCopyLen > nTotalLen)
	{
		TRACED("SafeCopyData Beyond the boundary. nTotalLen:[%d], nCopyLen:[%d]\n",
				nTotalLen, nCopyLen);
		return FALSE;
	}

	// 3. Clear Dstdata
	memset(pDst, 0, nTotalLen);

	// 4. Quick Copy
	memcpy(pDst, pSrc + nStartIdx, nCopyLen);

#ifdef DEBUG
	for (int i = 0; i < nCopyLen; i++)
	{
		TRACED("pSrc[%d]=[%02X]\n", i, pSrc[nStartIdx+i]);
		TRACED("pDst[%d]=[%02X]\n", i, pDst[i]);
	}
#endif

	return TRUE;
}

static UINT AR_ParseID(BYTE * rxBuf, AR_FRAME *frame)
{
    frame->ID_Datalen = AR_ParseUint32(&rxBuf[0]);

	TRACED("AR_ParseID: ID_Datalen:[%d]，rxBuf[0-3]:%02X %02X %02X %02X\n", frame->ID_Datalen, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3]);

    frame->ID_FunCode  = rxBuf[4];
    frame->ID_MainIndex = rxBuf[5];
    frame->ID_DstAddr   = rxBuf[6];
    frame->ID_SrcAddr   = rxBuf[7];

#ifdef DEBUG
	TRACED("ID_Datalen:[%d], ID_FunCode:[%d], ID_MainIndex:[%d], ID_DstAddr:[%d], ID_SrcAddr:[%d]\n",
			frame->ID_Datalen, frame->ID_FunCode, frame->ID_MainIndex, frame->ID_DstAddr, frame->ID_SrcAddr);
#endif

    return TRUE;
}

static int AR_ParseDATA(BYTE *rxBuf, AR_FRAME *frame)
{
    UINT i;
    BYTE *dataPtr;

    if((frame->ID_Datalen == 0) || (frame->ID_Datalen > AR_MAX_DATA_LEN))
	{
		TRACED("Invalid data length! ID_Datalen：[%d]\n", frame->ID_Datalen);
		return FALSE;
	}

    if(!SafeCopyData(&frame->Data, rxBuf, 8, frame->ID_Datalen-8, AR_MAX_DATA_LEN))
	{
		TRACED("AR_ParseDATA Data copy failed.\n");
		return FALSE;
	}

    return TRUE;
}

static UINT AR_ParseFrame(BYTE *rxBuf, AR_FRAME *frame)
{
    if(rxBuf == NULL || frame == NULL)
	{
		TRACED("The input or output is empty.\n");
		return FALSE;
	}

    if(!AR_ParseID(rxBuf, frame))
	{
		TRACED("ID domain resolution failed.\n");
		return FALSE;
	}

    if(!AR_ParseDATA(rxBuf, frame))
	{
		TRACED("DATA domain resolution failed.\n");
		return FALSE;
	}

    return  TRUE;
}

UINT AR_ProcessFrame(BYTE *rxBuf, AR_FRAME *frame)
{
	// UINT dataLen;

    // if(rxLen < 8)
	// {
	// 	return FALSE;
	// }
        
    // dataLen = AR_ParseUint32(&rxBuf[0]);

    // if(rxLen != dataLen)
	// {
	// 	TRACED("Invalid data length received!\n");
	// 	return FALSE;
	// }
    
    if(!AR_ParseFrame(rxBuf, frame))
	{	
		return FALSE;
	}

	return TRUE;
}

// LJH241101 MU3
BOOL Set_IMMU_DO(UINT ulWhichDO, BOOL bOn)
{
#ifdef IMMU3
	return Set_IMMU3_DO(ulWhichDO, bOn);
#else
	return Set_IMMU2_DO(ulWhichDO, bOn);
#endif
}

#ifdef IMMU3
BOOL Set_IMMU3_DO(UINT ulWhichDO, BOOL bOn)
{
	// int fd = open(DIDO_NAME, O_RDWR | O_NONBLOCK);
	int fd = open(DIDO_NAME, O_RDWR);
	if(fd < 0)
	{
		TRACEX("\nCan't open DIDO device \n");
		perror("Can't open DIDO device");
		return FALSE;
	}

	//ioctl(fd, ulWhichDO, bOn ? 1 : 0);
	int nVal = bOn ? 1 : 0;
	ioctl(fd, IOC_SET_DIDO(ulWhichDO), &nVal);

	TRACEX("\n---Set DO[%d] -> %d---\n", ulWhichDO, bOn);

	close(fd);

	Sleep_Ex(50); //aded by Jimmy 20220802

	return TRUE;
}

void Set_IMMU3_Beep(UINT nMSec)
{
	int fd = open(IMMU3_DEV_BEEP_PATH, O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		TRACEX("Can't open IMMU3_DEV_BEEP_PATH device \n");
		return;
	}

	nMSec = (nMSec > 10000) ? 10000 : nMSec; //max 10sec each time
	char szVal[16];
	snprintf(szVal, 16, "%d\n", nMSec);	//timeout_ms有效时间
	write(fd, szVal, strlen(szVal));	//写入beep时长

	//printf("---------Received BEEP ctrl -> [%d, %s] \n", nMSec, szVal);

	close(fd);
}
#else
BOOL Set_IMMU2_DO(UINT ulWhichDO, BOOL bOn)
{
	int fd = open(DIDO_NAME, O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		TRACEX("\nCan't open DIDO device \n");
		return FALSE;
	}

	ioctl(fd, ulWhichDO, bOn ? 1 : 0);

	TRACEX("\n---Set DO[%d] -> %d---\n", ulWhichDO, bOn);

	close(fd);

	Sleep_Ex(50); //aded by Jimmy 20220802

	return TRUE;
}

BOOL Get_IMMU2_DI(int nWhichDI)
{
	//added by Jimmy 20210623: if get FAILED, should return val from last time sampling
	static USHORT	s_usDIData = 0;
	BOOL bOn = FALSE;
	bOn = (((s_usDIData >> nWhichDI) & 0x01) == 0x01) ? TRUE : FALSE;

	int fd = open(DIDO_NAME, O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		TRACEX("Can't open DIDO device \n");
		return bOn;
	}

	if((nWhichDI < IMMU2_DI1) || (nWhichDI > IMMU2_DI4))
	{
		close(fd);
		return bOn;
	}

	int iRtn = read(fd, &s_usDIData, 2);
	if(iRtn > 0)
	{
		bOn = (((s_usDIData >> nWhichDI) & 0x01) == 0x01) ? TRUE : FALSE;
	}

	close(fd);

	return bOn;
}

BOOL Set_LCDPWR(BOOL bOn)
{
	int fd = open(DIDO_NAME, O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		TRACEX("\nCan't open DIDO device \n");
		return FALSE;
	}

	ioctl(fd, IOC_SET_LCDPWR, bOn ? 1 : 0);
	TRACEX("\nNow SET_LCD_PWR=>%d for A02 \n", bOn);
	
	close(fd);

	return TRUE;
}

#endif
