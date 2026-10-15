/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief  reporter's public function
* @history 2025-04-22 created
*******************************************************************************/

// #define _TRACE_PRINT_ // cmd
#include "std_sys.h"
#include "app_incs.h"
#include "reporters_base.h"
#include "../cJSON.h"

char* Make_TimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	sprintf(timeinstring, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

// LJH250508 LCCfg
LCCfg GetLCCfg_Reporter(REPORTER* pReporter)
{
	LCCfg lcCfg;
	ZERO_POBJS(&lcCfg, 1);

	HANDLE hReporter = (HANDLE)pReporter;
	DAI* pDAI =        (DAI*)pReporter->pDAI;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI   err;

	valGet.nGetSize = 4;
	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 100, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.emSystemType = (SYSTEM_TYPE)(err==ERRCODE_DAI_OK ? valGet.val.emValue : SYSTYPE_ESS_AIR);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 7, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.emApplyScenery = (POS_AUXMETER)(err==ERRCODE_DAI_OK ? valGet.val.emValue : AUXMETER_NOT_INSTALLED);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 42, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.idxClientLC = err==ERRCODE_DAI_OK ? valGet.val.ulValue : 0;

	lcCfg.lcType = GetLCType(lcCfg.idxClientLC);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3313, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.bSendCmdByLC1stOnly = (BOOL)(err==ERRCODE_DAI_OK ? valGet.val.emValue : FALSE);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3314, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.nESCNum = (err==ERRCODE_DAI_OK ? valGet.val.ulValue : 0);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1001, 3315, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.nBranchNum = (UINT)(err==ERRCODE_DAI_OK ? valGet.val.ulValue : 0);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		407, 401, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.bCabMeter = (BOOL)(err==ERRCODE_DAI_OK ? (valGet.val.emValue>0?TRUE:FALSE) : FALSE);

	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		408, 401, &valGet, sizeof(PARAM_VAL_GET));
	lcCfg.bGridMeter = (BOOL)(err==ERRCODE_DAI_OK ? (valGet.val.emValue>0?TRUE:FALSE) : FALSE);

	TRACED("reporters_pubfun:GetLCCfg_Reporter emSystemType:%d emApplyScenery:%d idxClientLC:%u lcType:%d "
		"bSendCmdByLC1stOnly:%d nESCNum:%u nBranchNum:%u bCabMeter:%d bGridMeter:%d",
		lcCfg.emSystemType, lcCfg.emApplyScenery, lcCfg.idxClientLC, lcCfg.lcType,
		lcCfg.bSendCmdByLC1stOnly, lcCfg.nESCNum, lcCfg.nBranchNum, lcCfg.bCabMeter, lcCfg.bGridMeter);
	return lcCfg;
}
// LJH250428 cmd155
cJSON* CreateJSON_C155(REPORTER* pReporter, LCCfg* pLCCfg, UINT nIntv,
	A_BLOB_ES_RUN_DATA* pRunData)
{
	// CreateJSON_C155 只由一级主控上报
	if (pLCCfg->lcType != LC_TYPE_1st || pLCCfg->emSystemType != SYSTYPE_EMS_STATION)
	{
		TRACEE("CreateJSON_C155 return NULL; !LC_TYPE_1st !SYSTYPE_EMS_STATION lcType:%d emSystemType:%d",
			pLCCfg->lcType, pLCCfg->emSystemType);
		return NULL;
	}
	HANDLE hReporter = (HANDLE)pReporter;
	DAI* pDAI =        (DAI*)pReporter->pDAI;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI   err;

	A_BLOB_CMD155 blobCmd;
	blobCmd.emBlobType = BLOB_TYPE_CMD155;
	valGet.nGetSize = sizeof(A_BLOB_CMD155);
	valGet.val.blobValue = (BYTE*)&blobCmd;
	err = pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3405, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEE("CreateJSON_C155 return NULL; Error: DAI Get A_BLOB_CMD155 Failed err:%d", err);
		return NULL;
	}
	int n, iBranch = 0, iEsc = 0;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pJArr_Brns = NULL, *pJObj_Brn = NULL;
	cJSON* pJArr_ESCs = NULL, *pJObj_ESC = NULL;
	cJSON* pJArr_ac_volts = NULL, *pJArr_ac_currs = NULL;

	SOURCE_AC_WORKSTATU ac_workstat_sum = SOURCE_AC_IDLE;
	float ac_volts_sum[3] = { 0 }, ac_currs_sum[3] = { 0 }, kw_meter_cab_sum = 0;
	switch (pLCCfg->emApplyScenery)
	{
	case AUXMETER_NOT_INSTALLED:
	{
		// BRANCH_DATA top#1 只有escDatas数据
		pJArr_Brns = cJSON_CreateArray();
		pJObj_Brn =  cJSON_CreateObject();
		pJArr_ESCs = cJSON_CreateArray();
		// 3. ESC_DATA
		for (iEsc=0; iEsc<(int)pLCCfg->nESCNum; iEsc++)
		{
			SC6LDEV_CMD155_ESCData* pESCData = &blobCmd.escDatas[iEsc];
			TRACED("CreateJSON_C155 pESCData dev_id:%u iEsc:%d "
				"ac_voltsA:%.1f B:%.1f C:%.1f ac_currs:%.1f %.1f %.1f batt_soc:%d",
				pESCData->dev_id, iEsc,
				pESCData->ac_volts[2], pESCData->ac_volts[1], pESCData->ac_volts[0],
				pESCData->ac_currs[2], pESCData->ac_currs[1], pESCData->ac_currs[0], pESCData->batt_soc);
			pJObj_ESC = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJObj_ESC, "dev_id",        pESCData->dev_id);
			BOOL commFault = FALSE;
			if (ElapseMSBcdTimeStmp(&pESCData->tmStamp) > nIntv * 1000)
			{
				TRACEW("CreateJSON_C155 LC_COMM_FAIL dev_id:%u iEsc:%d ElapseMSBcdTimeStmp:%u %s",
					pESCData->dev_id, iEsc, ElapseMSBcdTimeStmp(&pESCData->tmStamp), printableBcdTimeStmp(&pESCData->tmStamp));
				ZERO_POBJS(pESCData, 1);
				commFault = TRUE;
			}
			cJSON_AddNumberToObject(pJObj_ESC, "comm_fault",     commFault);
			cJSON_AddNumberToObject(pJObj_ESC, "ac_workstat",    pESCData->ac_workstat);
			pJArr_ac_volts = cJSON_CreateArray();
			pJArr_ac_currs = cJSON_CreateArray();
			for (n=0; n<3; n++)
			{
				int idxPhase = 3 - n - 1;
				ac_volts_sum[n] =  MAX(ac_volts_sum[n], pESCData->ac_volts[idxPhase]);
				ac_currs_sum[n] += pESCData->ac_currs[idxPhase];

				cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(pESCData->ac_volts[idxPhase], 1));
				cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(pESCData->ac_currs[idxPhase], 1));
			}
			TRACED("CreateJSON_C155 dev_id:%u iEsc:%d ac_volts_sum:%.1f %.1f %.1f ac_currs_sum:%.1f %.1f %.1f",
				pESCData->dev_id, iEsc, ac_volts_sum[0], ac_volts_sum[1], ac_volts_sum[2], ac_currs_sum[0], ac_currs_sum[1], ac_currs_sum[2]);
			cJSON_AddItemToObject(pJObj_ESC,   "ac_volts",       pJArr_ac_volts);
			cJSON_AddItemToObject(pJObj_ESC,   "ac_currs",       pJArr_ac_currs);
			cJSON_AddFloatToObject(pJObj_ESC,  "kw_meter_cab",   pESCData->kw_meter_cab, 1);

			cJSON_AddNumberToObject(pJObj_ESC, "acdc_workstat",  pESCData->acdc_workstat);
			cJSON_AddFloatToObject(pJObj_ESC,  "acdc_volt",      pESCData->acdc_volt, 1);
			cJSON_AddFloatToObject(pJObj_ESC,  "acdc_curr",      pESCData->acdc_curr, 1);
			cJSON_AddFloatToObject(pJObj_ESC,  "acdc_ext_volt",  pESCData->acdc_ext_volt, 1);

			cJSON_AddNumberToObject(pJObj_ESC, "batt_workstat",  pESCData->batt_workstat);
			cJSON_AddNumberToObject(pJObj_ESC, "batt_soc",       pESCData->batt_soc);
			cJSON_AddFloatToObject(pJObj_ESC,  "batt_volt",      pESCData->batt_volt, 1);
			cJSON_AddFloatToObject(pJObj_ESC,  "batt_curr",      pESCData->batt_curr, 1);
			cJSON_AddItemToArray(pJArr_ESCs, pJObj_ESC);
			// sum给Branch
			if (pESCData->ac_workstat == SOURCE_AC_PENDING_OUT)
			{
				ac_workstat_sum = SOURCE_AC_PENDING_OUT;
			}
			else if (pESCData->ac_workstat == SOURCE_AC_PENDING_IN)
			{
				ac_workstat_sum = SOURCE_AC_PENDING_IN;
			}
			kw_meter_cab_sum += pESCData->kw_meter_cab;
		}
		// 2. BranchAndESC
		cJSON_AddItemToObject(pJObj_Brn, "all_bess", pJArr_ESCs);
		cJSON_AddItemToArray(pJArr_Brns, pJObj_Brn);

		// 1. Root
		cJSON_AddNumberToObject(pRoot, "dev_id",           0);
		pJArr_ac_volts = cJSON_CreateArray();
		pJArr_ac_currs = cJSON_CreateArray();
		if (pLCCfg->bCabMeter)
		{
			ac_workstat_sum = pRunData->emAC_WorkStatus;

			cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(pRunData->fACVolt_L1, 1));
			cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(pRunData->fACVolt_L2, 1));
			cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(pRunData->fACVolt_L3, 1));

			cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(pRunData->fACCurr_L1, 1));
			cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(pRunData->fACCurr_L2, 1));
			cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(pRunData->fACCurr_L3, 1));

			kw_meter_cab_sum = pRunData->f_PcabMeter;
		}
		else
		{
			for (n=0; n<3; n++)
			{
				cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(ac_volts_sum[n], 1));
				cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(ac_currs_sum[n], 1));
			}
		}
		cJSON_AddNumberToObject(pRoot, "ac_workstat",      ac_workstat_sum);
		cJSON_AddItemToObject(pRoot,   "ac_volts",         pJArr_ac_volts);
		cJSON_AddItemToObject(pRoot,   "ac_currs",         pJArr_ac_currs);
		cJSON_AddFloatToObject(pRoot,  "kw_meter_grid",    pRunData->f_PgridMeter, 1);
		cJSON_AddFloatToObject(pRoot,  "kw_meter_cab",     kw_meter_cab_sum,       1);
		cJSON_AddFloatToObject(pRoot,  "kw_meter_aux1",    pRunData->f_PauxMeter,  1);
		cJSON_AddFloatToObject(pRoot,  "kw_meter_aux2",    pRunData->f_PauxMeter_3,1);
		cJSON_AddFloatToObject(pRoot,  "kw_acload",        pRunData->f_PgridMeter - kw_meter_cab_sum, 1);
		cJSON_AddFloatToObject(pRoot,  "kw_meter_offgrid", pRunData->fPoffgrid_LoadMeter, 1);
		
		cJSON_AddItemToObject(pRoot, "all_branch", pJArr_Brns);
	}
		break;

	default:
		TRACEW("CreateJSON_C155 return NULL default");
		return NULL;
	}

	return pRoot;
}

// 407:Cabinet AC 408:Grid 409:Aux1 410:Aux3 411:OffGridLoad
BOOL GetMeterValue_C157(REPORTER* pReporter, int nDev, SC6LDEV_CMD157* pCMD157)
{
	HANDLE hReporter = (HANDLE)pReporter;
	DAI* pDAI =        (DAI*)pReporter->pDAI;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI   err;

	valGet.nGetSize = 4;
	err = pDAI->Get(hReporter, DAITYPE_SETT_PARAMVALUE_RW,
		nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
	BOOL bInst = (BOOL)(err==ERRCODE_DAI_OK ? (valGet.val.emValue>0?TRUE:FALSE) : FALSE);
	if (!bInst)
	{
		TRACEW("GetMeterValue_C157 return !bInst nDev:%d", nDev);
		return FALSE;
	}

	// phase_volts
	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 413, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_volts[2] = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 415, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_volts[1] = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 417, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_volts[0] = valGet.val.fValue;
	// phase_currs
	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 414, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_currs[2] = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 416, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_currs[1] = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 418, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->phase_currs[0] = valGet.val.fValue;

	// phase_pf kwh_reading
	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 411, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->kwh_reading = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 422, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->neg_kwh_reading = valGet.val.fValue;
	
	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 412, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->kvarh_reading = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 400, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->active_power = valGet.val.fValue;
	
	// reactive_power
	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 404, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->reactive_power = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 426, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->power_factor = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 424, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->ac_freq = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 425, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->ac_pmaxdemand = valGet.val.fValue;

	pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDev, 432, &valGet, sizeof(PARAM_VAL_GET));
	pCMD157->ac_currentdemand = valGet.val.fValue;

	return TRUE;
}

static cJSON* SCreateJSON_C157_Single(UINT nIntv, cJSON* pJArrs, SC6LDEV_CMD157* pCMD157)
{
	cJSON* pJObj_Single = cJSON_CreateObject();
	cJSON_AddNumberToObject(pJObj_Single, "data_from",        pCMD157->data_from);
	BOOL commFault = FALSE; // 电表的通信状态后面加
	cJSON_AddNumberToObject(pJObj_Single, "comm_fault",     0);
	cJSON* pJArr_ac_volts = cJSON_CreateArray();
	cJSON* pJArr_ac_currs = cJSON_CreateArray();
	int n;
	for (n=0; n<3; n++)
	{
		int idxPhase = 3 - n - 1;
		cJSON_AddItemToArray(pJArr_ac_volts, cJSON_CreateFloat(pCMD157->phase_volts[idxPhase], 1));
		cJSON_AddItemToArray(pJArr_ac_currs, cJSON_CreateFloat(pCMD157->phase_currs[idxPhase], 1));
	}
	TRACED("SCreateJSON_C157_Single dev_id:%u phase_volts:%.1f %.1f %.1f ac_currentdemand:%.1f",
		pCMD157->dev_id, pCMD157->phase_volts[0], pCMD157->phase_volts[1], pCMD157->phase_volts[2], pCMD157->ac_currentdemand);
	cJSON_AddItemToObject(pJObj_Single, "phase_volts", pJArr_ac_volts);
	cJSON_AddItemToObject(pJObj_Single, "phase_currs", pJArr_ac_currs);

	cJSON_AddFloatToObject(pJObj_Single, "kwh_reading",     pCMD157->kwh_reading, 1);
	cJSON_AddFloatToObject(pJObj_Single, "neg_kwh_reading", pCMD157->neg_kwh_reading, 1);
	cJSON_AddFloatToObject(pJObj_Single, "kvarh_reading",   pCMD157->kvarh_reading, 1);
	cJSON_AddFloatToObject(pJObj_Single, "active_power",    pCMD157->active_power, 1);

	cJSON_AddFloatToObject(pJObj_Single, "reactive_power",  pCMD157->reactive_power, 1);
	cJSON_AddFloatToObject(pJObj_Single, "power_factor",    pCMD157->power_factor, 1);
	cJSON_AddFloatToObject(pJObj_Single, "ac_freq",         pCMD157->ac_freq, 1);
	cJSON_AddFloatToObject(pJObj_Single, "ac_pmaxdemand",   pCMD157->ac_pmaxdemand, 1);
	cJSON_AddFloatToObject(pJObj_Single, "ac_currentdemand",pCMD157->ac_currentdemand, 1);
	
	cJSON_AddItemToArray(pJArrs, pJObj_Single);
	return pJArrs;
}
// LJH250520 cmd157
cJSON* CreateJSON_C157(REPORTER* pReporter, LCCfg* pLCCfg, UINT nIntv)
{
	if (pLCCfg->lcType != LC_TYPE_1st || pLCCfg->emSystemType != SYSTYPE_EMS_STATION)
	{
		TRACEE("CreateJSON_C157 return NULL; !LC_TYPE_1st !SYSTYPE_EMS_STATION lcType:%d emSystemType:%d",
			pLCCfg->lcType, pLCCfg->emSystemType);
		return NULL;
	}
	HANDLE hReporter = (HANDLE)pReporter;
	DAI* pDAI =        (DAI*)pReporter->pDAI;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI   err;

	A_BLOB_CMD157 blobCmd;
	blobCmd.emBlobType = BLOB_TYPE_CMD157;
	valGet.nGetSize = sizeof(A_BLOB_CMD157);
	valGet.val.blobValue = (BYTE*)&blobCmd;
	err = pDAI->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3407, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEE("CreateJSON_C157 return NULL; Error: DAI Get A_BLOB_CMD157 Failed err:%d", err);
		return NULL;
	}
	int n, iBranch = 0, iEsc = 0;
	cJSON* pRoot = cJSON_CreateObject();
	cJSON* pJArr_Brns = NULL, *pJObj_Brn = NULL;
	cJSON* pJArr_ESCs = NULL, *pJObj_ESC = NULL;
	cJSON* pJArr_Singles = NULL, *pJObj_Single = NULL;
	cJSON* pJArr_ac_volts = NULL, *pJArr_ac_currs = NULL;

	SC6LDEV_CMD157 cmd157;
	switch (pLCCfg->emApplyScenery)
	{
	case AUXMETER_NOT_INSTALLED:
	{
		// BRANCH_DATA top#1 只有escDatas数据
		pJArr_Brns = cJSON_CreateArray();
		pJObj_Brn =  cJSON_CreateObject();
		pJArr_ESCs = cJSON_CreateArray();
		// 3. ESC_DATA
		for (iEsc=0; iEsc<(int)pLCCfg->nESCNum; iEsc++)
		{
			SC6LDEV_CMD157* pESCData = &blobCmd.escDatas[iEsc];
			TRACED("CreateJSON_C157 pESCData dev_id:%u iEsc:%d "
				"ac_voltsA:%.1f B:%.1f C:%.1f ac_currentdemand:%.1f",
				pESCData->dev_id, iEsc,
				pESCData->phase_volts[2], pESCData->phase_volts[1], pESCData->phase_volts[0],
				pESCData->ac_currentdemand);
			pJObj_ESC = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJObj_ESC, "dev_id",        pESCData->dev_id);
			BOOL commFault = FALSE;
			if (ElapseMSBcdTimeStmp(&pESCData->tmStamp) > nIntv * 1000)
			{
				TRACEW("CreateJSON_C157 LC_COMM_FAIL dev_id:%u iEsc:%d ElapseMSBcdTimeStmp:%u %s",
					pESCData->dev_id, iEsc, ElapseMSBcdTimeStmp(&pESCData->tmStamp), printableBcdTimeStmp(&pESCData->tmStamp));
				ZERO_POBJS(pESCData, 1);
				commFault = TRUE;
			}
			cJSON_AddNumberToObject(pJObj_ESC, "comm_fault",     commFault);
			pJArr_Singles = cJSON_CreateArray();
			SCreateJSON_C157_Single(nIntv, pJArr_Singles, pESCData);
			cJSON_AddItemToObject(pJObj_ESC, "meters", pJArr_Singles);
			cJSON_AddItemToArray(pJArr_ESCs, pJObj_ESC);
		}
		// 2. BranchAndESC
		cJSON_AddItemToObject(pJObj_Brn, "all_bess", pJArr_ESCs);
		cJSON_AddItemToArray(pJArr_Brns, pJObj_Brn);
		// 1. Root top#1 主控最多只有407 408
		cJSON_AddNumberToObject(pRoot, "dev_id",           0);
		pJArr_Singles = cJSON_CreateArray();
		for (n=0; n<2; n++)
		{
			ZERO_POBJS(&cmd157, 1);
			if (GetMeterValue_C157(pReporter, 407 + n, &cmd157))
			{
				SCreateJSON_C157_Single(nIntv, pJArr_Singles, &cmd157);
			}
		}
		cJSON_AddItemToObject(pRoot, "meters",  pJArr_Singles);
		cJSON_AddItemToObject(pRoot, "all_branch", pJArr_Brns);
	}
		break;

	default:
		TRACEW("CreateJSON_C157 return NULL default");
		return NULL;
	}

	return pRoot;
}


cJSON* CreateJSON_SEMS_C103(A_BLOB_SEMS_CMD103* pBlobSemsCMD103Data, CTREAT_SEMS_JSON_INFO* pCreateInfo)
{
	int iClientIdx = pCreateInfo->iClientIdx;

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pCreateInfo->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "dev_id", pBlobSemsCMD103Data->clientBMS[pCreateInfo->iClientIdx].iDevID);

	int iBattGroupNum = pBlobSemsCMD103Data->clientBMS[pCreateInfo->iClientIdx].iBattGroupNum;
	cJSON_AddNumberToObject(pRoot, "online_batts", iBattGroupNum);

	if (iBattGroupNum > 4)
		return pRoot;

	cJSON 	*pBMS_Array = cJSON_CreateArray();

	int i = 0;
	for (i = 0; i < iBattGroupNum; ++i)
	{
		cJSON *pBMS_Single = cJSON_CreateObject();
		cJSON_AddNumberToObject(pBMS_Single, "batt_idx", i);

		SC6_LDEV_103_Data* pBmsData = &pBlobSemsCMD103Data->clientBMS[pCreateInfo->iClientIdx].bmsData[i];

		cJSON_AddNumberToObject(pBMS_Single, "run_statu", pBmsData->run_statu);

		cJSON_AddFloatToObject(pBMS_Single, "pack_volt", pBmsData->pack_volt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "pack_curr", pBmsData->pack_curr, 1);
		
		cJSON_AddNumberToObject(pBMS_Single, "soh", pBmsData->soh);
		cJSON_AddNumberToObject(pBMS_Single, "soc", pBmsData->soc);

		cJSON_AddNumberToObject(pBMS_Single, "cycle", pBmsData->cycle);

		cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_volt", pBmsData->max_allow_chg_volt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_chg_curr", pBmsData->max_allow_chg_curr, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_volt", pBmsData->max_allow_dischg_volt, 1);
		cJSON_AddFloatToObject(pBMS_Single, "max_allow_dischg_curr", pBmsData->max_allow_dischg_curr, 1);

		cJSON_AddFloatToObject(pBMS_Single, "cell_max_volt", pBmsData->cell_max_volt, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_min_volt", pBmsData->cell_min_volt, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_avg_volt", pBmsData->cell_avg_volt, 3);
		
		cJSON_AddFloatToObject(pBMS_Single, "cell_max_temp", pBmsData->cell_max_temp, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_min_temp", pBmsData->cell_min_temp, 3);
		cJSON_AddFloatToObject(pBMS_Single, "cell_avg_temp", pBmsData->cell_avg_temp, 1);

		cJSON_AddFloatToObject(pBMS_Single, "ucell_max_csc",  (float)pBmsData->ucell_max_csc, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_max_cell", (float)pBmsData->ucell_max_cell, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_min_csc",  (float)pBmsData->ucell_min_csc, 1);
		cJSON_AddFloatToObject(pBMS_Single, "ucell_min_cell", (float)pBmsData->ucell_min_cell, 1);
		
		cJSON_AddFloatToObject(pBMS_Single, "temp_max_csc",  (float)pBmsData->temp_max_csc, 1);
		cJSON_AddFloatToObject(pBMS_Single, "temp_min_csc", (float)pBmsData->temp_min_csc, 1);

		cJSON_AddFloatToObject(pBMS_Single, "remain_charge_energy", (float)pBmsData->remain_charge_energy, 2);
		cJSON_AddFloatToObject(pBMS_Single, "remain_discharge_energy", (float)pBmsData->remain_discharge_energy, 2);

		cJSON_AddNumberToObject(pBMS_Single, "chrg_forbid", pBmsData->chrg_forbid);
		cJSON_AddNumberToObject(pBMS_Single, "dischrg_forbid", pBmsData->dischrg_forbid);
 		
		cJSON_AddNumberToObject(pBMS_Single, "alarm_status", pBmsData->alarm_status); 
		cJSON_AddStringToObject(pBMS_Single, "alarm_info", pBmsData->alarm_info);

		cJSON_AddStringToObject(pBMS_Single, "alarm2_info", pBmsData->alarm2_info);
		cJSON_AddStringToObject(pBMS_Single, "fault_info", pBmsData->fault_info); 
		cJSON_AddStringToObject(pBMS_Single, "gpio_info", pBmsData->gpio_info); 

		cJSON_AddFloatToObject(pBMS_Single, "batt_ver", pBmsData->batt_ver, 3);

		cJSON_AddItemToArray(pBMS_Array, pBMS_Single);
	}

	cJSON_AddItemToObject(pRoot, "batt_grp_data", pBMS_Array);

	return pRoot;
}

cJSON* CreateJSON_SEMS_C110(A_BLOB_SEMS_CMD110* pBlobSemsCMD110Data, CTREAT_SEMS_JSON_INFO* pCreateInfo)
{
	int iClientIdx = pCreateInfo->iClientIdx;
	int iBattIdx = pCreateInfo->iBattIdx;

	cJSON 	*pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "data_from", pCreateInfo->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "dev_id", pBlobSemsCMD110Data->clientBattCell[iClientIdx].iDevID);
	cJSON_AddNumberToObject(pRoot, "batt_idx", 	iBattIdx);
	cJSON_AddNumberToObject(pRoot, "data_type", 0);

	cJSON* pVolt_Array = cJSON_CreateArray();
	cJSON* pTemp_Array = cJSON_CreateArray();
	cJSON* pUnblance_Array = cJSON_CreateArray();
	cJSON* pUnsafe_Array = cJSON_CreateArray();

	int iCellNum =  pBlobSemsCMD110Data->clientBattCell[iClientIdx].battGroupCell[iBattIdx].usCellNumPerGroup;
	
	int j = 0;
	for(j = 0; j < iCellNum; ++j)
	{
		SC6_LDEV_CELL_110* pCellInfo = &pBlobSemsCMD110Data->clientBattCell[iClientIdx].battGroupCell[iBattIdx].battCellInfo[j];

		cJSON_AddItemToArray(pUnsafe_Array, cJSON_CreateNumber(pCellInfo->cell_safe));
		cJSON_AddItemToArray(pVolt_Array, cJSON_CreateFloat(pCellInfo->cell_volt, 1));
		cJSON_AddItemToArray(pTemp_Array, cJSON_CreateFloat(pCellInfo->cell_temp, 1));
		cJSON_AddItemToArray(pUnblance_Array, cJSON_CreateNumber(pCellInfo->cell_balance));
	}

	cJSON_AddItemToObject(pRoot, "sigl_cell_volt_data", pVolt_Array);
	cJSON_AddItemToObject(pRoot, "sigl_cell_temp_data", pTemp_Array);
	cJSON_AddItemToObject(pRoot, "sigl_cell_balance_data", pUnblance_Array);
	cJSON_AddItemToObject(pRoot, "sigl_cell_safe_data", pUnsafe_Array);

	return pRoot;

}

cJSON* CreateJSON_SEMS_C202(A_BLOB_SEMS_CMD202* pBlobSemsCMD202Data, CTREAT_SEMS_JSON_INFO* pCreateInfo)
{
	int iClientIdx = pCreateInfo->iClientIdx;

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, 	"data_from", pCreateInfo->nCurrDataFrom);
	cJSON_AddNumberToObject(pRoot, "dev_id", pBlobSemsCMD202Data->semsEleStatist[iClientIdx].iDevID);


	cJSON* pACRealTime_Array = cJSON_CreateArray();
	cJSON* pACCost_Array = cJSON_CreateArray();
	cJSON* pOrgLoadCost_Array = cJSON_CreateArray();
	cJSON* pRmtACRevs_Array = cJSON_CreateArray();
	cJSON* pLocalACIN_Array = cJSON_CreateArray();
	cJSON* pLocalACOut_Array = cJSON_CreateArray();
	cJSON* pDCLoadCost_Array = cJSON_CreateArray();
	
	cJSON* pMpptOut_Array = cJSON_CreateArray();
	cJSON* pBattGrp_Array = cJSON_CreateArray();

	int n = 0;
	for(n = 0; n < MAX_HOURS_SECTION_202; n++)
	{
		SC6_LDEV_202* pEleStatistv = &pBlobSemsCMD202Data->semsEleStatist[iClientIdx].eleStatist;

		cJSON_AddItemToArray(pACRealTime_Array, 	cJSON_CreateFloat(pEleStatistv->rmt_ac_realtime_kwh[n], 1));
		cJSON_AddItemToArray(pACCost_Array, 		cJSON_CreateFloat(pEleStatistv->rmt_ac_cost_kwh[n], 1));
		cJSON_AddItemToArray(pOrgLoadCost_Array, 	cJSON_CreateFloat(pEleStatistv->org_acload_cost_kwh[n], 1));
		cJSON_AddItemToArray(pRmtACRevs_Array, 		cJSON_CreateFloat(pEleStatistv->rmt_ac_revs_kwh[n], 1));		
		cJSON_AddItemToArray(pLocalACIN_Array, 		cJSON_CreateFloat(pEleStatistv->local_ac_input_kwh[n], 1));
		cJSON_AddItemToArray(pLocalACOut_Array, 	cJSON_CreateFloat(pEleStatistv->local_ac_output_kwh[n], 1));
		cJSON_AddItemToArray(pDCLoadCost_Array, 	cJSON_CreateFloat(pEleStatistv->dcload_cost_kwh[n], 1));
	}

	int nGrp = 0;
	for(nGrp = 0; nGrp < pCreateInfo->iBattGroupNum && nGrp < MAX_BATTERY_NUM; nGrp++)
	{
		//value_mppt_output[MAX_BATTERY_NUM][MAX_HOURS_SECTION_202];

		cJSON *pMppt_Obj = cJSON_CreateObject();
		cJSON* pMpptValue_Array = cJSON_CreateArray();

		cJSON *pBatt_Obj = cJSON_CreateObject();
		cJSON* pBattChrg_Array = cJSON_CreateArray();
		cJSON* pBattDisChrg_Array = cJSON_CreateArray();

		for(n = 0; n < MAX_HOURS_SECTION_202; n++)
		{
			cJSON_AddItemToArray(pMpptValue_Array, 	cJSON_CreateFloat(pBlobSemsCMD202Data->semsEleStatist[iClientIdx].eleStatist.value_mppt_output[nGrp][n], 1));

			cJSON_AddItemToArray(pBattChrg_Array, 		cJSON_CreateFloat(pBlobSemsCMD202Data->semsEleStatist[iClientIdx].eleStatist.value_batt_chrg[nGrp][n], 	1));
			cJSON_AddItemToArray(pBattDisChrg_Array, 	cJSON_CreateFloat(pBlobSemsCMD202Data->semsEleStatist[iClientIdx].eleStatist.value_batt_dischrg[nGrp][n], 1));
		}
		
		cJSON_AddItemToObject(pMppt_Obj, "value_mppt_output", 	pMpptValue_Array);
		cJSON_AddItemToArray(pMpptOut_Array, pMppt_Obj);

		cJSON_AddItemToObject(pBatt_Obj, "value_batt_chrg", 	pBattChrg_Array);
		cJSON_AddItemToObject(pBatt_Obj, "value_batt_dischrg", 	pBattDisChrg_Array);
		cJSON_AddItemToArray(pBattGrp_Array, pBatt_Obj);
	}

	cJSON_AddItemToObject(pRoot, "rmt_ac_realtime_kwh", 	pACRealTime_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_cost_kwh", 		pACCost_Array);
	cJSON_AddItemToObject(pRoot, "org_acload_cost_kwh", 	pOrgLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_revs_kwh", 		pRmtACRevs_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_input_kwh", 		pLocalACIN_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_output_kwh", 	pLocalACOut_Array);
	cJSON_AddItemToObject(pRoot, "dcload_cost_kwh", 		pDCLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "mppt_grp_output_kwh", 	pMpptOut_Array);
	cJSON_AddItemToObject(pRoot, "batt_grp_chrgdischrgkwh", pBattGrp_Array);


	return pRoot;
}

cJSON* CreateJSON_C202(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage, UINT nCfgBMSNum)
{
	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pACRealTime_Array = cJSON_CreateArray();
	cJSON* pACCost_Array = cJSON_CreateArray();
	cJSON* pOrgLoadCost_Array = cJSON_CreateArray();
	cJSON* pRmtACRevs_Array = cJSON_CreateArray();
	cJSON* pLocalACIN_Array = cJSON_CreateArray();
	cJSON* pLocalACOut_Array = cJSON_CreateArray();
	cJSON* pDCLoadCost_Array = cJSON_CreateArray();
	
	cJSON* pMpptOut_Array = cJSON_CreateArray();
	cJSON* pBattGrp_Array = cJSON_CreateArray();
 
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	0);

	int n = 0;
	for(n = 0; n < 48; n++)
	{
		cJSON_AddItemToArray(pACRealTime_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fAC_CurrKWH_Reading[n], 1));
		cJSON_AddItemToArray(pACCost_Array, 		cJSON_CreateFloat(blobDataStorage->hours_Data.fAC_CostKWH_Sector[n], 1));
		cJSON_AddItemToArray(pOrgLoadCost_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fOrgACLoad_CostKWH_Sector[n], 1));
		cJSON_AddItemToArray(pRmtACRevs_Array, 		cJSON_CreateFloat(blobDataStorage->hours_Data.fAC_RevrtOutPutKWH_Sector[n], 1));		
		cJSON_AddItemToArray(pLocalACIN_Array, 		cJSON_CreateFloat(blobDataStorage->hours_Data.fLocalAC_INPUTKWH_Sector[n], 1));
		cJSON_AddItemToArray(pLocalACOut_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fLocalAC_OUTPUTKWH_Sector[n], 1));
		cJSON_AddItemToArray(pDCLoadCost_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fDCLoad_CostKWH_Sector[n], 1));
	}

	int nGrp = 0;
	for(nGrp = 0; nGrp < nCfgBMSNum; nGrp++)
	{
		cJSON *pMppt_Obj = cJSON_CreateObject();
		cJSON* pMpptValue_Array = cJSON_CreateArray();
		for(n = 0; n < 48; n++)
		{
			cJSON_AddItemToArray(pMpptValue_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fMppt_GKWH_Sector[nGrp][n], 1));
		}
		
		cJSON_AddItemToObject(pMppt_Obj, "value_mppt_output", 	pMpptValue_Array);
		cJSON_AddItemToArray(pMpptOut_Array, pMppt_Obj);
 	}

	for(nGrp = 0; nGrp < nCfgBMSNum; nGrp++)
	{
		cJSON *pBatt_Obj = cJSON_CreateObject();
		cJSON* pBattChrg_Array = cJSON_CreateArray();
		cJSON* pBattDisChrg_Array = cJSON_CreateArray();

		for(n = 0; n < 48; n++)
		{
			cJSON_AddItemToArray(pBattChrg_Array, 		cJSON_CreateFloat(blobDataStorage->hours_Data.fBatt_ChrgKWH_Sector[nGrp][n], 	1));
			cJSON_AddItemToArray(pBattDisChrg_Array, 	cJSON_CreateFloat(blobDataStorage->hours_Data.fBatt_DisChrgKWH_Sector[nGrp][n], 1));
		}
		
		cJSON_AddItemToObject(pBatt_Obj, "value_batt_chrg", 	pBattChrg_Array);
		cJSON_AddItemToObject(pBatt_Obj, "value_batt_dischrg", 	pBattDisChrg_Array);

		cJSON_AddItemToArray(pBattGrp_Array, pBatt_Obj);
 	}

	cJSON_AddItemToObject(pRoot, "rmt_ac_realtime_kwh", 	pACRealTime_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_cost_kwh", 		pACCost_Array);
	cJSON_AddItemToObject(pRoot, "org_acload_cost_kwh", 	pOrgLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "rmt_ac_revs_kwh", 		pRmtACRevs_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_input_kwh", 		pLocalACIN_Array);
	cJSON_AddItemToObject(pRoot, "local_ac_output_kwh", 	pLocalACOut_Array);
	cJSON_AddItemToObject(pRoot, "dcload_cost_kwh", 		pDCLoadCost_Array);
	cJSON_AddItemToObject(pRoot, "mppt_grp_output_kwh", 	pMpptOut_Array);
	cJSON_AddItemToObject(pRoot, "batt_grp_chrgdischrgkwh", pBattGrp_Array);

	char szTime[32];
	cJSON_AddStringToObject(pRoot, "time_stamp", Make_TimeString(blobDataStorage->t_SaveTime, szTime));

	return pRoot;
}

cJSON* CreateJSON_C204(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage, UINT nCfgBMSNum)
{
	int nGrp = 0;
	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, 	"data_from", 	0);
	char szTime[32];

	cJSON_AddStringToObject(pRoot, "time_stamp", Make_TimeString(blobDataStorage->t_SaveTime, szTime));

	cJSON_AddFloatToObject(pRoot, "rmt_ac_realtime_kwh",		blobDataStorage->days_Data.fAC_CurrKWH_Reading,  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_cost_kwh", 			blobDataStorage->days_Data.fACTotalCostKWH_D,  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_cost_kwh", 		blobDataStorage->days_Data.fOrgACLoad_CostKWH_D,  1);
	cJSON_AddFloatToObject(pRoot, "rmt_ac_revs_kwh", 			blobDataStorage->days_Data.fAC_RevrtOutPutKWH_D,  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_input_kwh", 		blobDataStorage->days_Data.fLocalAC_INPUTKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_output_kwh", 		blobDataStorage->days_Data.fLocalAC_OUTPUTKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "dcload_cost_kwh", 			blobDataStorage->days_Data.fDCLoad_CostKWH_D ,  1);
	
	cJSON* pMppt_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < nCfgBMSNum; nGrp++)
	{
		cJSON *pMppt_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_total_output", 	blobDataStorage->days_Data.fMpptTotalGKWH_D[nGrp] ,  1);
		
		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_sharp_output", 	blobDataStorage->days_Data.fMppt_Sharp_GKWH_D[nGrp] ,  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_peak_output", 	blobDataStorage->days_Data.fMppt_Peak_GKWH_D[nGrp] ,  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_flat_output", 	blobDataStorage->days_Data.fMppt_Flat_GKWH_D[nGrp] ,  1);
 		cJSON_AddFloatToObject(pMppt_Obj, "value_mppt_valley_output", 	blobDataStorage->days_Data.fMppt_Valley_GKWH_D[nGrp] ,  1);


		cJSON_AddItemToArray(pMppt_Array, pMppt_Obj);
	}
	cJSON_AddItemToObject(pRoot, "mppt_grp_output_kwh", pMppt_Array);

	cJSON* pBattGrp_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < nCfgBMSNum; nGrp++)
	{
		cJSON *pBattGrp_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_chrg", 	blobDataStorage->days_Data.fBattTotalChrgKWH_D[nGrp] ,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_dischrg", 	blobDataStorage->days_Data.fBattTotalDisChrgKWH_D[nGrp] ,  1);
		
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_sharp_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_peak_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_flat_chrg", 		0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_valley_chrg", 		0.0,  1);

		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_sharp_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_peak_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_flat_dischrg", 	0.0,  1);
		cJSON_AddFloatToObject(pBattGrp_Obj, "value_batt_valley_dischrg", 	0.0,  1);

		cJSON_AddItemToArray(pBattGrp_Array, pBattGrp_Obj);
	}
	cJSON_AddItemToObject(pRoot, "batt_grp_chrgdischrgkwh", 		pBattGrp_Array);
 
	cJSON_AddFloatToObject(pRoot, "local_ac_sharpinput_kwh", 	blobDataStorage->days_Data.fLocalAC_Sharp_InputKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_peakinput_kwh", 	blobDataStorage->days_Data.fLocalAC_Peak_InputKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_flatinput_kwh", 	blobDataStorage->days_Data.fLocalAC_Flat_InputKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_valleyinput_kwh", 	blobDataStorage->days_Data.fLocalAC_Valley_InputKWH_D ,  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_sharpoutput_kwh", 	blobDataStorage->days_Data.fLocalAC_Sharp_OutPutKWH_D ,  1);

	cJSON_AddFloatToObject(pRoot, "local_ac_peakoutput_kwh", 	blobDataStorage->days_Data.fLocalAC_Peak_OutPutKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_flatoutput_kwh", 	blobDataStorage->days_Data.fLocalAC_Flat_OutputKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "local_ac_valleyoutput_kwh", 	blobDataStorage->days_Data.fLocalAC_Valley_OutputKWH_D ,  1);
	
	cJSON_AddFloatToObject(pRoot, "org_acload_sharpcost_kwh", 	blobDataStorage->days_Data.fOrgACLoad_Sharp_CostKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_peakcost_kwh", 	blobDataStorage->days_Data.fOrgACLoad_Peak_CostKWH_D ,  1);

	cJSON_AddFloatToObject(pRoot, "org_acload_flatcost_kwh", 	blobDataStorage->days_Data.fOrgACLoad_Flat_CostKWH_D ,  1);
	cJSON_AddFloatToObject(pRoot, "org_acload_valleycost_kwh", 	blobDataStorage->days_Data.fOrgACLoad_Valley_CostKWH_D ,  1);
	
 	cJSON* pExcTimes_Array = cJSON_CreateArray();
	for(nGrp = 0; nGrp < nCfgBMSNum; nGrp++)
	{
		cJSON *pExcTimes_Obj = cJSON_CreateObject();
 		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_chrg_times", 		blobDataStorage->days_Data.fBattChrgTimes_D[nGrp] ,  2);
		cJSON_AddFloatToObject(pExcTimes_Obj, "value_batt_dischrg_times", 	blobDataStorage->days_Data.fBattDisChrgTimes_D[nGrp] ,  2);
		cJSON_AddItemToArray(pExcTimes_Array, pExcTimes_Obj);
	}
	cJSON_AddItemToObject(pRoot, "batt_chrgdischrg_times", pExcTimes_Array);

	return pRoot;
}

#define GetFeeRateKWHDevNum 2
static UINT s_GetFeeRateKWHDevMap[GetFeeRateKWHDevNum]={407,401};
cJSON* CreateJSON_C206(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage)
{
	cJSON *pRoot = cJSON_CreateObject();
	cJSON* pFEERATE_KWH_ARRAY = cJSON_CreateArray();
	PARAM_VAL_GET valGet;

	UINT nWhichWh, nDev, nTimeNum;
	for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	{
		nDev = s_GetFeeRateKWHDevMap[nWhichWh]; //

		valGet.nGetSize = 4;
		VAR_VALUE varV;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				nDev, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Not Installed
		{
			TRACEX("Warning: nDev=[%d] Not Installed!\n", nDev);
			continue;
		}
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
				nDev, 401, &varV, 4);
		if (varV.emValue != 0)
		{
			TRACEX("Warning: nDev=[%d] kWhMeter CommFail!\n", nDev);
			continue;
		}

		cJSON *pAllFeeRateKwh_Obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(pAllFeeRateKwh_Obj, "data_from", 			nDev); 

		cJSON* pTime_Array = cJSON_CreateArray();
		for (nTimeNum = 0; nTimeNum < GetFeeRateKWHTimeNum; nTimeNum++) //current,last1,last2
		{
			cJSON *pFeeRateKwh_Obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(pFeeRateKwh_Obj, "data_from", 			nTimeNum);  
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sum_pos_kwh", 			blobDataStorage->feeRateKWH[nWhichWh].fPosSumKwh[nTimeNum][0],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sharp_pos_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fPosSharpKwh[nTimeNum][1],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "peak_pos_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fPosPeakKwh[nTimeNum][2],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "flat_pos_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fPosFlatKwh[nTimeNum][3],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "valley_pos_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fPosWaveKwh[nTimeNum][4],  3);
			
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sum_neg_kwh", 			blobDataStorage->feeRateKWH[nWhichWh].fNegSumKwh[nTimeNum][5],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "sharp_neg_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fNegSharpKwh[nTimeNum][6],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "peak_neg_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fNegPeakKwh[nTimeNum][7],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "flat_neg_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fNegFlatKwh[nTimeNum][8],  3);
			cJSON_AddFloatToObject(pFeeRateKwh_Obj, "valley_neg_kwh", 		blobDataStorage->feeRateKWH[nWhichWh].fNegWaveKwh[nTimeNum][9],  3);

			cJSON_AddItemToArray(pTime_Array, pFeeRateKwh_Obj);
		}
		cJSON_AddItemToObject(pAllFeeRateKwh_Obj, "time_feerate_kwh", pTime_Array);

		cJSON_AddItemToArray(pFEERATE_KWH_ARRAY, pAllFeeRateKwh_Obj);
		
	}
	cJSON_AddItemToObject(pRoot, "feerate_kwh", pFEERATE_KWH_ARRAY);
	
	char szTime[32];
	cJSON_AddStringToObject(pRoot, "time", Make_TimeString(blobDataStorage->t_SaveTime, szTime));

	return pRoot;
}

COMMPORT* Reporter_GetCommPort_ById(HANDLE hReporter, int nPortId, COMMPORT* pComm)
{
	REPORTER* pReporter = (REPORTER*)hReporter;

	int err = pReporter->pDAI->Get(hReporter, DAITYPE_COMMPORT_R,
		nPortId, NULL, pComm, sizeof(COMMPORT));
	
	if(err != ERR_DPR_OK)
	{
		return NULL;
	}
	TRACE("reporters_pubfunc:GetCommPort_ById nPortId:%d GetCommPort_ById:<%s:%s>\n", nPortId, pComm->info.szPortAttri, pComm->info.szPortName);

	return pComm;
}
