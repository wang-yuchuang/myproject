#include "iec_104.h"

#define GET_BIT(num, n) (((num) >> (n)) & 1)
#define SET_BIT(num, n) ((num) | (1 << (n)))


/*********************************************************************************
*  
*  FUNCTION NAME :  Init_SysParamConfig()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Init_SysParamConfig(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET valGet;
	//--------- Get Battery Rack Number ---------//

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			500, 502, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->nCfgBMSNum = valGet.val.ulValue;
	if(pIEC104->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pIEC104->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}

	//-------------Get Backend Config-------------// 
	ZERO_POBJS(&pIEC104->blobBackendSet, 1);
	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pIEC104->blobBackendSet;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}

	ZERO_POBJS(&pIEC104->emsPlan, 1);
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	pIEC104->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.val.blobValue = (BYTE*)&pIEC104->emsPlan;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		200, 201, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return FALSE;
	}

	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  Init_IEC104_SigMap()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Init_IEC104_SigMap(IEC104_DATA_ROUGH* pIEC104)
{
	pIEC104->em104ConnStatus 	= STAT_DisConnected;
	pIEC104->bTrigger 			= TRUE;
	pIEC104->bOn_Interrogating 	= FALSE;
	pIEC104->bNeedUploadMppt 	= FALSE;

	//------------------------ Yao Xin SigMap ------------------------//
	IEC104_CONFIG_SIGMAP cfgYaoXin[] =
	{
		{0 ,	0x01, IEC60870_QUALITY_GOOD ,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bMainAC_Breaker) },
		{1 ,	0x02, IEC60870_QUALITY_GOOD ,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bBAT1_Contactor) },
		{2 ,	0x03, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bBAT2_Contactor) },
		{3 ,	0x04, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bBAT3_Contactor) },
		{4 ,	0x05, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bBAT4_Contactor) },
		{5 ,	0x06, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strYX_SigMap.bLiqAirWorkMode) },

	};

	pIEC104->pYXEle = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYaoXin));
	if(pIEC104->pYXEle == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYXEle, cfgYaoXin, sizeof(cfgYaoXin));
	pIEC104->nYXNum = ITEM_OF(cfgYaoXin);

	// //------------------------ Yao Xin BMS SigMap ------------------------//
	UINT nBattIdx;
	for(nBattIdx=0; nBattIdx<MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
		IEC104_CONFIG_SIGMAP cfgBmsPart1YaoXin[] =
		{
			{0 ,	0x1001 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBmsNotPowerOn		) },
			{1 ,	0x1002 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBmsHVPowerOnFail	) },
			{2 ,	0x1003 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBmsHVPowerOffFail	) },
			{3 ,	0x1004 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_Ucell		) },
			{4 ,	0x1005 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Under_Ucell		) },
			{5 ,	0x1006 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_Tcell		) },
			{6 ,	0x1007 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Under_Tcell		) },
			{7 ,	0x1008 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_U_HVS		) },
			{8 ,	0x1009 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Under_U_HVS		) },
			{9 ,	0x100A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_T_HVS		) },
			{10 ,	0x100B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Under_T_HVS		) },
			{11 ,	0x100C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_CC			) },
			{12 ,	0x100D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_DC			) },
			{13 ,	0x100E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Diff_Tcell		) },
			{14 ,	0x100F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_PreChFailThree	) },
			{15 ,	0x1010 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_HSD_Coil			) },
			{16 ,	0x1011 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Diff_Ucell		) },
			{17 ,	0x1012 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Bal_Circuit	) },
			{18 ,	0x1013 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Rly_OverLoad		) },
			{19 ,	0x1014 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Pre_Rly			) },
			{20 ,	0x1015 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Main_Rly_Pos		) },
			{21 ,	0x1016 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Main_Rly_Neg		) },
			{22 ,	0x1017 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Rly_Stick_Pos	) },
			{23 ,	0x1018 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Rly_Stick_Neg	) },
			{24 ,	0x1019 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Over_Cur			) },
			{25 ,	0x101A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Mea_Ucell	) },
			{26 ,	0x101B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Mea_Tcell	) },
			{27 ,	0x101C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Diff_Tvol		) },
			{28 ,	0x101D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Mea_I_HVS	) },
			{29 ,	0x101E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_MCANComFault		) },
			{30 ,	0x101F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_MCU			) },
			{31 ,	0x1020 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Extre_Tvol		) },
			{32 ,	0x1021 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_CCAN			) },
			{33 ,	0x1022 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_SCAN			) },
			{34 ,	0x1023 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Mea_AllTcell) },
			{35 ,	0x1024 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Extre_T			) },
			{36 ,	0x1025 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Extre_Ucell		) },
			{37 ,	0x1026 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_PowerSample_LEM	) },
			{38 ,	0x1027 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_PowerSample_CSC	) },
			{39 ,	0x1028 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_SBMU_PowerDown	) },
			{40 ,	0x1029 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Fog			) },
			{41 ,	0x102A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_HvCrcuitBreak) },
			{42 ,	0x102B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_HvMsdFault	) },
			{43 ,	0x102C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Fan1Fault	) },
			{44 ,	0x102D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Fan2Fault	) },
			{45 ,	0x102E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAL_Err_Rly_STB		) },
			{46 ,	0x102F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bCellV_HV1			) },
			{47 ,	0x1030 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bCellV_LV1			) },
			{48 ,	0x1031 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bSumV_HV1			) },
			{49 ,	0x1032 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bSumV_LV1			) },
			{50 ,	0x1033 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bChg_HT1				) },
			{51 ,	0x1034 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bChg_LT1				) },
			{52 ,	0x1035 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bDischg_HT1			) },
			{53 ,	0x1036 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bDischg_LT1			) },
			{54 ,	0x1037 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bChg_OC1				) },
			{55 ,	0x1038 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bDischg_OC1			) },
			{56 ,	0x1039 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bExtT_HT1			) },
			{57 ,	0x103A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBattery_insul1		) },
			{58 ,	0x103B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bCellV_diff1			) },
			{59 ,	0x103C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bCellT_diff1			) },
			{60 ,	0x103D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBattBox_fan_recheck	) },
			{61 ,	0x103E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBattCab_ShuttFan_or_AirCondFail) },
			{62 ,	0x103F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBattCab_Aux_Dev_CommFail) },
			{63 ,	0x1040 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBattCube_SPD		) },
			{64 ,	0x1041 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bBatt_Cube_Tip		) },
			{65 ,	0x1042 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPower_IMEU3_CommFail) },
			{66 ,	0x1043 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bCab_IMEU3_CommFail	) },
			{67 ,	0x1044 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bLeakCurrCommFail	) },
			{68 ,	0x1045 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bAir_Cond_CommFail	) },
			{69 ,	0x1046 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bDehumid_CommFail	) },
			{70 ,	0x1047 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bGasSensor_CommFail	) },
			{71 ,	0x1048 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPDU_Flow_Fan_Recheck) },
			{72 ,	0x1049 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPCS_Fan_Recheck		) },
			{73 ,	0x104A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bExhaust_Fan_Recheck) },
			{74 ,	0x104B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bExplos_Recheck		) },
			{75 ,	0x104C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bLeakCurrAbnormal	) },
			{76 ,	0x104D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bFireAlm				) },
			{77 ,	0x104E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bFire_Lv1_Alm		) },
			{78 ,	0x104F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bFire_Lv2_Alm		) },
			{79 ,	0x1050 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bFire_Lv3_Alm		) },
			{80 ,	0x1051 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPDU_High_EnvirTemp_Lv1_Alm) },
			{81 ,	0x1052 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPDU_High_CableTemp_Lv1_Alm) },
			{82 ,	0x1053 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPCS_High_OutletTemp_Lv1_Alm) },
			{83 ,	0x1054 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_Water_Alm		) },
			{84 ,	0x1055 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_MSD_Alm		) },
			{85 ,	0x1056 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_Pos_HT_Alm	) },
			{86 ,	0x1057 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_Neg_HT_Alm	) },
			{87 ,	0x1058 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_Aerosol_Alm	) },
			{88 ,	0x1059 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_CO_Alm		) },
			{99 ,	0x105A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK1_H2_Alm		) },
			{90 ,	0x105B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_Water_Alm		) },
			{91 ,	0x105C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_MSD_Alm		) },
			{92 ,	0x105D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_Pos_HT_Alm	) },
			{93 ,	0x105E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_Neg_HT_Alm	) },
			{94 ,	0x105F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_Aerosol_Alm	) },
			{95 ,	0x1060 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_CO_Alm		) },
			{96 ,	0x1061 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK2_H2_Alm		) },
			{97 ,	0x1062 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_Water_Alm		) },
			{98 ,	0x1063 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_MSD_Alm		) },
			{99 ,	0x1064 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_Pos_HT_Alm	) },
			{100 ,	0x1065 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_Neg_HT_Alm	) },
			{101 ,	0x1066 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_Aerosol_Alm	) },
			{102 ,	0x1067 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_CO_Alm		) },
			{103 ,	0x1068 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK3_H2_Alm		) },
			{104 ,	0x1069 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_Water_Alm		) },
			{105 ,	0x106A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_MSD_Alm		) },
			{106 ,	0x106B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_Pos_HT_Alm	) },
			{107 ,	0x106C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_Neg_HT_Alm	) },
			{108 ,	0x106D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_Aerosol_Alm	) },
			{109 ,	0x106E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_CO_Alm		) },
			{110 ,	0x106F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK4_H2_Alm		) },
			{111 ,	0x1070 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_Water_Alm		) },
			{112 ,	0x1071 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_MSD_Alm		) },
			{113 ,	0x1072 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_Pos_HT_Alm	) },
			{114 ,	0x1073 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_Neg_HT_Alm	) },
			{115 ,	0x1074 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_Aerosol_Alm	) },
			{116 ,	0x1075 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_CO_Alm		) },
			{117 ,	0x1076 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPACK5_H2_Alm		) },
			{118 ,	0x1077 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bSlv_Temp_Alm		) },
			{119 ,	0x1078 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bSlv_Inner_Fault		) },
			{120 ,	0x1079 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bMas_Slv_CommFail	) },
			{121 ,	0x107A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bMas_Software_Alm	) },
			{122 ,	0x107B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bMas_Inner_Fault		) },
			{123 ,	0x107C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bHV_Contactor		) },
			{124 ,	0x107D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bPeriph_backInspec	) },
			{125 ,	0x107E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bHVPowOnFail			) },
			{126 ,	0x107F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart1YX_SigMap[nBattIdx].bHVPowOffFail		) },
		};
		
		pIEC104->pYX_BmsPart1[nBattIdx] = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgBmsPart1YaoXin));
		if(pIEC104->pYX_BmsPart1[nBattIdx] == NULL)
		{
			return FALSE;
		}
		memcpy(pIEC104->pYX_BmsPart1[nBattIdx], cfgBmsPart1YaoXin, sizeof(cfgBmsPart1YaoXin));
		pIEC104->nYX_BmsPart1Num = ITEM_OF(cfgBmsPart1YaoXin);

		IEC104_CONFIG_SIGMAP cfgBmsPart2YaoXin[] =
		{
			{0 ,	0x1081 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bPeriph_Stat_NotReady	) },
			{1 ,	0x1082 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bGrpContactor_Change_OnLoad	) },
			{2 ,	0x1083 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bSOS_Alm_Occur			) },
			{3 ,	0x1084 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bUcell_Over				) },
			{4 ,	0x1085 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bUcell_diff				) },
			{5 ,	0x1086 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bUcell_Drop				) },
			{6 ,	0x1087 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bTcell_Over				) },
			{7 ,	0x1088 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bTcell_diff				) },
			{8 ,	0x1089 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bTcell_Rise				) },
			{9 ,	0x108A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bEqualChip_OT			) },
			{10 ,	0x108B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bPowerChip_OT			) },
			{11 ,	0x108C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bInstant_OC				) },
			{12 ,	0x108D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bStandbyCurrAlm			) },
			{13 ,	0x108E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bTotalCapacityDecay		) },
			{14 ,	0x108F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bCellChgDischgProtFail	) },
			{15 ,	0x1090 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bCellColFail				) },
			{16 ,	0x1091 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bLongDODLow				) },
			{17 ,	0x1092 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bChgDisChgEffLow			) },
			// {18 ,	0x1083 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strBmsPart2YX_SigMap[nBattIdx].bDI_1_AuxPower			) },
		};
		
		pIEC104->pYX_BmsPart2[nBattIdx] = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgBmsPart2YaoXin));
		if(pIEC104->pYX_BmsPart2[nBattIdx] == NULL)
		{
			return FALSE;
		}
		memcpy(pIEC104->pYX_BmsPart2[nBattIdx], cfgBmsPart2YaoXin, sizeof(cfgBmsPart2YaoXin));
		pIEC104->nYX_BmsPart2Num = ITEM_OF(cfgBmsPart2YaoXin);

	}

	// //------------------------ Yao Xin PcsTotalSigMap ------------------------//
	IEC104_CONFIG_SIGMAP cfgPcsTotalYaoXin[] =
	{
		{0 ,	0x2001, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bCcu_CommFail		) },
		{1 ,	0x2002, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bAC_OV				) },
		{2 ,	0x2003, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bAC_UV				) },
		{3 ,	0x2004, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bACGrid_Input_MCB_QF1	) },
		{4 ,	0x2005, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bACLoad_Output_MCB_QF2	) },
		{5 ,	0x2006, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bAC_Contactor		) },
		{6 ,	0x2007, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bABU1_CommFail		) },
		{7 ,	0x2008, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bABU2_CommFail		) },
		{8 ,	0x2009, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bABU3_CommFail		) },
		{9 ,	0x200A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bABU4_CommFail		) },
		{10 ,	0x200B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bLV_MCBBBreaker	) },
		{11 ,	0x200C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bN_LineContactor	) },
		{12 ,	0x200D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula1_CommFail	) },
		{13 ,	0x200E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula1_DeteAlm	) },
		{14 ,	0x200F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula2_CommFail	) },
		{15 ,	0x2010, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula2_DeteAlm	) },
		{16 ,	0x2011, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula3_CommFail	) },
		{17 ,	0x2012, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula3_DeteAlm	) },
		{18 ,	0x2013, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula4_CommFail	) },
		{19 ,	0x2014, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula4_DeteAlm	) },
		{20 ,	0x2015, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula5_CommFail	) },
		{21 ,	0x2016, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula5_DeteAlm	) },
		{22 ,	0x2017, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula6_CommFail	) },
		{23 ,	0x2018, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula6_DeteAlm	) },
		{24 ,	0x2019, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula7_CommFail	) },
		{25 ,	0x201A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula7_DeteAlm	) },
		{26 ,	0x201B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula8_CommFail	) },
		{27 ,	0x201C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula8_DeteAlm	) },
		{28 ,	0x201D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula9_CommFail	) },
		{29 ,	0x201E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula9_DeteAlm	) },
		{30 ,	0x201F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula10_CommFai	) },
		{31 ,	0x2020, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bInsula10_DeteAlm	) },
		{32 ,	0x2021, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp1_ForbidRun	) },
		{33 ,	0x2022, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp2_ForbidRun	) },
		{34 ,	0x2023, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp3_ForbidRun	) },
		{35 ,	0x2024, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp4_ForbidRun	) },
		{36 ,	0x2025, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp1_InputContact	) },
		{37 ,	0x2026, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp2_InputContact	) },
		{38 ,	0x2027, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp3_InputContact	) },
		{39 ,	0x2028, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp4_InputContact	) },
		{40 ,	0x2029, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp1_IsoSwitch	) },
		{41 ,	0x202A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp2_IsoSwitch	) },
		{42 ,	0x202B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp3_IsoSwitch	) },
		{43 ,	0x202C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBattGrp4_IsoSwitch	) },
		{44 ,	0x202D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBms1Can_CommFail	) },
		{45 ,	0x202E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBms2Can_CommFail	) },
		{46 ,	0x202F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBms3Can_CommFail	) },
		{47 ,	0x2030, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bBms4Can_CommFail	) },
		{48 ,	0x2031, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bAllPMGrp_Fail		) },
		{49 ,	0x2032, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bAllPMGrp_CommFail	) },
		{50 ,	0x2033, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM1_Fail			) },
		{51 ,	0x2034, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM1_CommFail		) },
		{52 ,	0x2035, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM2_Fail			) },
		{53 ,	0x2036, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM2_CommFail		) },
		{54 ,	0x2037, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM3_Fail			) },
		{55 ,	0x2038, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM3_CommFail		) },
		{56 ,	0x2039, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM4_Fail			) },
		{57 ,	0x203A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM4_CommFail		) },
		{58 ,	0x203B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM5_Fail			) },
		{59 ,	0x203C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM5_CommFail		) },
		{60 ,	0x203D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM6_Fail			) },
		{61 ,	0x203E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM6_CommFail		) },
		{62 ,	0x203F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM7_Fail			) },
		{63 ,	0x2040, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM7_CommFail		) },
		{64 ,	0x2041, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM8_Fail			) },
		{65 ,	0x2042, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM8_CommFail		) },
		{66 ,	0x2043, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM9_Fail			) },
		{67 ,	0x2044, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM9_CommFail		) },
		{68 ,	0x2045, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM10_Fail			) },
		{69 ,	0x2046, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM10_CommFail		) },
		{70 ,	0x2047, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM11_Fail			) },
		{71 ,	0x2048, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM11_CommFail		) },
		{72 ,	0x2049, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM12_Fail			) },
		{73 ,	0x204A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPM12_CommFail		) },
		{74 ,	0x204B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPCSGrp1_Fail		) },
		{75 ,	0x204C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPCSGrp2_Fail		) },
		{76 ,	0x204D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPCSGrp3_Fail		) },
		{77 ,	0x204E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsTotalYX_SigMap.bPCSGrp4_Fail		) },

	};

	pIEC104->pYX_PcsTotal = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgPcsTotalYaoXin));
	if(pIEC104->pYX_PcsTotal == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYX_PcsTotal, cfgPcsTotalYaoXin, sizeof(cfgPcsTotalYaoXin));
	pIEC104->nYX_PcsTotalNum = ITEM_OF(cfgPcsTotalYaoXin);

	//------------------------ Yao Xin PcsGrp SigMap ------------------------//
	for(nBattIdx=0; nBattIdx<MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
		IEC104_CONFIG_SIGMAP cfgPcsGrpYaoXin[] =
		{
			{0 ,	0x2101 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDischgErr		) },
			{1 ,	0x2102 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBusFail		) },
			{2 ,	0x2103 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSciFail		) },
			{3 ,	0x2104 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bUVdc			) },
			{4 ,	0x2105 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bShort			) },
			{5 ,	0x2106 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bMCCFail		) },
			{6 ,	0x2107 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOvo			) },
			{7 ,	0x2108 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOT			) },
			{8 ,	0x2109 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bFault			) },
			{9 ,	0x210A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOVin			) },
			{10 ,	0x210B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bUVin			) },
			{11 ,	0x210C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInUnBal		) },
			{12 ,	0x210D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInLessPhase	) },
			{13 ,	0x210E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOutUnBal		) },
			{14 ,	0x210F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bIDDup			) },
			{15 ,	0x2110 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGridFault		) },
			{16 ,	0x2111 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPllFault		) },
			{17 ,	0x2112 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAcFail		) },
			{18 ,	0x2113 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBusErr		) },
			{19 ,	0x2114 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bVinOff		) },
			{20 ,	0x2115 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrpUVdc		) },
			{21 ,	0x2116 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bVGridFault	) },
			{22 ,	0x2117 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bFGridFault	) },
			{23 ,	0x2118 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bACLessPhase	) },
			{24 ,	0x2119 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhaseErr		) },
			{25 ,	0x211A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDCTZ			) },
			{26 ,	0x211B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOIin			) },
			{27 ,	0x211C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bACTZ			) },
			{28 ,	0x211D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOVBUS			) },
			{29 ,	0x211E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOP			) },
			{30 ,	0x211F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGridVolt_OverBus) },
			{31 ,	0x2120 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bHighOrLowFault) },
			{32 ,	0x2121 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBAT_EOD		) },
			{33 ,	0x2122 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRly_Fault		) },
			{34 ,	0x2123 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSoftStart_Fault) },
			{35 ,	0x2124 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAuxPower_Fault) },
			{36 ,	0x2125 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bEPO_Fault		) },
			{37 ,	0x2126 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDuplicate_ID	) },
			{38 ,	0x2127 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bUnEven_Curr	) },
			{39 ,	0x2128 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bFan_Fail		) },
			{40 ,	0x2129 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOTemp			) },
			{41 ,	0x212A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhase_Lock	) },
			{42 ,	0x212B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bIsland_Prot	) },
			{43 ,	0x212C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bCommFail_CanSciSpi) },
			{44 ,	0x212D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOutput_Derating) },
			{45 ,	0x212E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOutput_OL		) },
			{46 ,	0x212F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOutput_Short	) },
			{47 ,	0x2130 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInter_OC		) },
			{48 ,	0x2131 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBus_Fault		) },
			{49 ,	0x2132 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_Fault		) },
			{50 ,	0x2133 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_Fault		) },
			{51 ,	0x2134 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSys_Fail		) },
			{52 ,	0x2135 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_OV		) },
			{53 ,	0x2136 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_UV		) },
			{54 ,	0x2137 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_OF		) },
			{55 ,	0x2138 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_UF		) },
			{56 ,	0x2139 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhase_Loss	) },
			{57 ,	0x213A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRevGrid_lackN	) },
			{58 ,	0x213B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_ReverPhase) },
			{59 ,	0x213C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_PowerOff	) },
			{60 ,	0x213D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRevGrid_Fast_OV1) },
			{61 ,	0x213E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRevGrid_Fast_OV2) },
			{62 ,	0x213F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRevGrid_Unbal	) },
			{63 ,	0x2140 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhase_Lock_Fault) },
			{64 ,	0x2141 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bHVRT_Fault	) },
			{65 ,	0x2142 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bLVRT_Fault	) },
			{66 ,	0x2143 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bIslandProt	) },
			{67 ,	0x2144 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_FastUnbal) },
			{68 ,	0x2145 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_SlowUnbal) },
			{69 ,	0x2146 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_FastOV) },
			{70 ,	0x2147 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_SlowOV) },
			{71 ,	0x2148 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_FastUV) },
			{72 ,	0x2149 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInterBus_SlowUV) },
			{73 ,	0x214A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRevBus_Short	) },
			{74 ,	0x214B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_High_DC	) },
			{75 ,	0x214C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_OV			) },
			{76 ,	0x214D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_UV			) },
			{77 ,	0x214E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRev_DC_Reversed) },
			{78 ,	0x214F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_OC			) },
			{79 ,	0x2150 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_ChgOV		) },
			{80 ,	0x2151 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_EOD_Shutdown) },
			{81 ,	0x2152 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_FastOC		) },
			{82 ,	0x2153 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bOutput_OLoad	) },
			{83 ,	0x2154 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhA_PBPFault	) },
			{84 ,	0x2155 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhB_PBPFault	) },
			{85 ,	0x2156 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bPhC_PBPFault	) },
			{86 ,	0x2157 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_CurrUnbal	) },
			{87 ,	0x2158 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bEff_Fail		) },
			{88 ,	0x2159 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBus_SoftStart_Fault) },
			{99 ,	0x215A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInv_SoftStart_Fault) },
			{90 ,	0x215B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_RlyOpen	) },
			{91 ,	0x215C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bGrid_RlyShort	) },
			{92 ,	0x215D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInv_RlyOpen	) },
			{93 ,	0x215E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInv_RlyShort	) },
			{94 ,	0x215F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_RlyFault	) },
			{95 ,	0x2160 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_Side_FaultSum) },
			{96 ,	0x2161 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_Curr_SampFail) },
			{97 ,	0x2162 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bInductor_Curr_SampFail) },
			{98 ,	0x2163 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_Curr_SampFail) },
			{99 ,	0x2164 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_SoftStart_OL) },
			{100 ,	0x2165 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_SoftStart_RlyFail	) },
			{101 ,	0x2166 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bParallel_CommFail	) },
			{102 ,	0x2167 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_ShortCircuit	) },
			{103 ,	0x2168 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDSP_FaultRev		) },
			{104 ,	0x2169 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSTS_FaultRev		) },
			{105 ,	0x216A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bCAN_CommFailRev	) },
			{106 ,	0x216B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bEEPROM_faultRev	) },
			{107 ,	0x216C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bEPO_Fail			) },
			{108 ,	0x216D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bCarrier_SyncFail	) },
			{109 ,	0x216E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAC_SyncFail		) },
			{110 ,	0x216F + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bCAN_CommFail		) },
			{111 ,	0x2170 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSCI_CommFail		) },
			{112 ,	0x2171 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bSPI_CommFail		) },
			{113 ,	0x2172 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBalBridge_OC		) },
			{114 ,	0x2173 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBus_OV			) },
			{115 ,	0x2174 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bBus_UV			) },
			{116 ,	0x2175 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bRadiator_OT		) },
			{117 ,	0x2176 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAmbient_OT		) },
			{118 ,	0x2177 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bAux_PowerFault	) },
			{119 ,	0x2178 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDSP_PWM_Fault		) },
			{120 ,	0x2179 + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bCPLD_PWM_Fault	) },
			{121 ,	0x217A + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bFan_Fault			) },
			{122 ,	0x217B + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bVo_Set_OverProtPoint	) },
			{123 ,	0x217C + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bVo_Setting_Low	) },
			{124 ,	0x217D + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDC_Short_Circuit	) },
			{125 ,	0x217E + nBattIdx*0x100, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strPcsGrpYX_SigMap[nBattIdx].bDuplicated_ID		) },
		};
		
		pIEC104->pYX_PcsGrp[nBattIdx] = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgPcsGrpYaoXin));
		if(pIEC104->pYX_PcsGrp[nBattIdx] == NULL)
		{
			return FALSE;
		}
		memcpy(pIEC104->pYX_PcsGrp[nBattIdx], cfgPcsGrpYaoXin, sizeof(cfgPcsGrpYaoXin));
		pIEC104->nYX_PcsGrpNum = ITEM_OF(cfgPcsGrpYaoXin);

	}

	//------------------------ Yao Xin STSSigMap ------------------------//
	IEC104_CONFIG_SIGMAP cfgSTSYaoXin[] =
	{
		{0 ,	0x2501, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridOVP		) },
		{1 ,	0x2502, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridUVP		) },
		{2 ,	0x2503, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridOFP		) },
		{3 ,	0x2504, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridUFP		) },
		{4 ,	0x2505, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridFreqLose	) },
		{5 ,	0x2506, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridLosePhase	) },
		{6 ,	0x2507, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridPhaseSeqInv) },
		{7 ,	0x2508, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridLoseN		) },
		{8 ,	0x2509, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridUnbal		) },
		{9 ,	0x250A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_VGridLineFault	) },
		{10 ,	0x250B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_PllFail		) },
		{11 ,	0x250C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_VgridCrossZeroLose	) },
		{12 ,	0x250D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridDQFastFault) },
		{13 ,	0x250E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridFreErr		) },
		{14 ,	0x250F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridFreChg		) },
		{15 ,	0x2510, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OutputOL		) },
		{16 ,	0x2511, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridOCP		) },
		{17 ,	0x2512, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_IacUnbal		) },
		{18 ,	0x2513, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OVRT			) },
		{19 ,	0x2514, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_LVRT			) },
		{20 ,	0x2515, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_IslandFault	) },
		{21 ,	0x2516, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_RefluxFault	) },
		{22 ,	0x2517, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OutputShort	) },
		{23 ,	0x2518, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_VoutCrossZeroLose	) },
		{24 ,	0x2519, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bGri_UaSampErr		) },
		{25 ,	0x251A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bGri_UbSampErr		) },
		{26 ,	0x251B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bGri_UcSampErr		) },
		{27 ,	0x251C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_RlyErr			) },
		{28 ,	0x251D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OnGridStartFault) },
		{29 ,	0x251E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_CanErr			) },
		{30 ,	0x251F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_SpiErr			) },
		{31 ,	0x2520, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_E2PROMErr		) },
		{32 ,	0x2521, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OTSink			) },
		{33 ,	0x2522, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OTAmb			) },
		{34 ,	0x2523, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OTCabinet		) },
		{35 ,	0x2524, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_FanERR			) },
		{36 ,	0x2525, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_GridFault		) },
		{37 ,	0x2526, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_SysFault_1114	) },
		{38 ,	0x2527, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_SCRShort		) },
		{39 ,	0x2528, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_SCROpen		) },
		{40 ,	0x2529, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_U2CanErr		) },
		{41 ,	0x252A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_U2SpiErr		) },
		{42 ,	0x252B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_U2E2PROMErr	) },
		{43 ,	0x252C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_EPO			) },
		{44 ,	0x252D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_OTempTrsfm		) },
		{45 ,	0x252E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_AuxPowerErr	) },
		{46 ,	0x252F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_InvContctErr	) },
		{47 ,	0x2530, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_ByContctErr	) },
		{48 ,	0x2531, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_FanErr			) },
		{49 ,	0x2532, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_IoReadyErr		) },
		{50 ,	0x2533, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_U2SysFault		) },
		{51 ,	0x2534, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_Fan1Err		) },
		{52 ,	0x2535, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_Fan2Err		) },
		{53 ,	0x2536, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_Fan3Err		) },
		{54 ,	0x2537, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_Fan4Err		) },
		{55 ,	0x2538, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strSTSYX_SigMap.bSTS_N_RlyErr		) },

	};

	pIEC104->pYX_STS = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgSTSYaoXin));
	if(pIEC104->pYX_STS == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYX_STS, cfgSTSYaoXin, sizeof(cfgSTSYaoXin));
	pIEC104->nYX_STSNum = ITEM_OF(cfgSTSYaoXin);

	//------------------------ Yao Xin FireSigMap ------------------------//
	IEC104_CONFIG_SIGMAP cfgFireYaoXin[] =
	{
		{0 ,	0x2601, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bPowerCab_Smoke_Sensor		) },
		{1 ,	0x2602, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bCombGas_Sensor1_CommFail	) },
		{2 ,	0x2603, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bCombGas_Sensor2_CommFail	) },
		{3 ,	0x2604, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bCombGas_Sensor3_CommFail	) },
		{4 ,	0x2605, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bCombGas_Sensor4_CommFail	) },
		{5 ,	0x2606, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bFireTube_Alm				) },
		{6 ,	0x2607, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bSmoke_Alm					) },
		{7 ,	0x2608, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bFlamGas_Alm				) },
		{8 ,	0x2609, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bElecShut_FullCls			) },
		{9 ,	0x260A, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bElecShut_FullOpen			) },
		{10 ,	0x260B, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bEnterProt_Lv1				) },
		{11 ,	0x260C, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bEnterProt_Lv2				) },
		{12 ,	0x260D, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bEnterProt_Lv3				) },
		{13 ,	0x260E, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bEnterProt_Lv4				) },
		{14 ,	0x260F, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bSlvAerosol_DensityAlm		) },
		{15 ,	0x2610, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bCombGas_concentraAlm		) },
		{16 ,	0x2611, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bBatGrp_FirepProtAlm		) },
		{17 ,	0x2612, IEC60870_QUALITY_GOOD,	OBJVALUE_BOOL, 1, (void*)&(pIEC104->strFireYX_SigMap.bGrp_SmokeAlm				) },

	};

	pIEC104->pYX_Fire = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgFireYaoXin));
	if(pIEC104->pYX_Fire == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYX_Fire, cfgFireYaoXin, sizeof(cfgFireYaoXin));
	pIEC104->nYX_FireNum = ITEM_OF(cfgFireYaoXin);



	//--------------------- Yao Ce EMS SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_Ems[] =
	{
		{0 ,	0x4001,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.nEmsSectorNum) },
		{1 ,	0x4002,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[0].tmStart) },
		{2 ,	0x4003,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[0].nStopSOC) },
		{3 ,	0x4004,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[0].nBattGrpPwrLmt) },
		{4 ,	0x4005,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[0].nActvPwrLmt) },
		{5 ,	0x4006,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[0].nReActvPwrLmt) },
		{6 ,	0x4007,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[1].tmStart) },
		{7 ,	0x4008,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[1].nStopSOC) },
		{8 ,	0x4009,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[1].nBattGrpPwrLmt) },
		{9 ,	0x400A,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[1].nActvPwrLmt) },
		{10,	0x400B,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[1].nReActvPwrLmt) },
		{11,	0x400C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[2].tmStart) },
		{12,	0x400D,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[2].nStopSOC) },
		{13,	0x400E,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[2].nBattGrpPwrLmt) },
		{14,	0x400F,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[2].nActvPwrLmt) },
		{15,	0x4010,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[2].nReActvPwrLmt) },
		{16,	0x4011,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[3].tmStart) },
		{17,	0x4012,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[3].nStopSOC) },
		{18,	0x4013,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[3].nBattGrpPwrLmt) },
		{19,	0x4014,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[3].nActvPwrLmt) },
		{20,	0x4015,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[3].nReActvPwrLmt) },
		{21,	0x4016,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[4].tmStart) },
		{22,	0x4017,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[4].nStopSOC) },
		{23,	0x4018,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[4].nBattGrpPwrLmt) },
		{24,	0x4019,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[4].nActvPwrLmt) },
		{25,	0x401A,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[4].nReActvPwrLmt) },
		{26,	0x401B,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[5].tmStart) },
		{27,	0x401C,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[5].nStopSOC) },
		{28,	0x401D,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[5].nBattGrpPwrLmt) },
		{29,	0x401E,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[5].nActvPwrLmt) },
		{30,	0x401F,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[5].nReActvPwrLmt) },
		{31,	0x4020,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[6].tmStart) },
		{32,	0x4021,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[6].nStopSOC) },
		{33,	0x4022,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[6].nBattGrpPwrLmt) },
		{34,	0x4023,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[6].nActvPwrLmt) },
		{35,	0x4024,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[6].nReActvPwrLmt) },
		{36,	0x4025,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[7].tmStart) },
		{37,	0x4026,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[7].nStopSOC) },
		{38,	0x4027,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[7].nBattGrpPwrLmt) },
		{39,	0x4028,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[7].nActvPwrLmt) },
		{40,	0x4029,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[7].nReActvPwrLmt) },
		{41,	0x402A,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[8].tmStart) },
		{42,	0x402B,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[8].nStopSOC) },
		{43,	0x402C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[8].nBattGrpPwrLmt) },
		{44,	0x402D,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[8].nActvPwrLmt) },
		{45,	0x402E,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[8].nReActvPwrLmt) },
		{46,	0x402F,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[9].tmStart) },
		{47,	0x4030,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[9].nStopSOC) },
		{48,	0x4031,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[9].nBattGrpPwrLmt) },
		{49,	0x4032,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[9].nActvPwrLmt) },
		{50,	0x4033,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[9].nReActvPwrLmt) },
		{51,	0x4034,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[10].tmStart) },
		{52,	0x4035,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[10].nStopSOC) },
		{53,	0x4036,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[10].nBattGrpPwrLmt) },
		{54,	0x4037,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[10].nActvPwrLmt) },
		{55,	0x4038,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[10].nReActvPwrLmt) },
		{56,	0x4039,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[11].tmStart) },
		{57,	0x403A,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[11].nStopSOC) },
		{58,	0x403B,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[11].nBattGrpPwrLmt) },
		{59,	0x403C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[11].nActvPwrLmt) },
		{60,	0x403D,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_Ems.arryEmsParam[11].nReActvPwrLmt) },

	};
 
	pIEC104->pYCEle_Ems = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_Ems));
	if(pIEC104->pYCEle_Ems == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_Ems, cfgYC_Ems, sizeof(cfgYC_Ems));
	pIEC104->nYCNum_Ems = ITEM_OF(cfgYC_Ems);


	 //--------------------- Yao Tiao C2S EMS SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgC2S_Ems[] =
	{
		{0 ,	0x6201,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.nEmsSectorNum) },
		{1 ,	0x6202,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[0].tmStart) },
		{2 ,	0x6203,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[0].nStopSOC) },
		{3 ,	0x6204,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[0].nBattGrpPwrLmt) },
		{4 ,	0x6205,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[0].nActvPwrLmt) },
		{5 ,	0x6206,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[0].nReActvPwrLmt) },
		{6 ,	0x6207,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[1].tmStart) },
		{7 ,	0x6208,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[1].nStopSOC) },
		{8 ,	0x6209,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[1].nBattGrpPwrLmt) },
		{9 ,	0x620A,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[1].nActvPwrLmt) },
		{10,	0x620B,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[1].nReActvPwrLmt) },
		{11,	0x620C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[2].tmStart) },
		{12,	0x620D,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[2].nStopSOC) },
		{13,	0x620E,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[2].nBattGrpPwrLmt) },
		{14,	0x620F,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[2].nActvPwrLmt) },
		{15,	0x6210,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[2].nReActvPwrLmt) },
		{16,	0x6211,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[3].tmStart) },
		{17,	0x6212,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[3].nStopSOC) },
		{18,	0x6213,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[3].nBattGrpPwrLmt) },
		{19,	0x6214,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[3].nActvPwrLmt) },
		{20,	0x6215,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[3].nReActvPwrLmt) },
		{21,	0x6216,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[4].tmStart) },
		{22,	0x6217,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[4].nStopSOC) },
		{23,	0x6218,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[4].nBattGrpPwrLmt) },
		{24,	0x6219,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[4].nActvPwrLmt) },
		{25,	0x621A,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[4].nReActvPwrLmt) },
		{26,	0x621B,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[5].tmStart) },
		{27,	0x621C,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[5].nStopSOC) },
		{28,	0x621D,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[5].nBattGrpPwrLmt) },
		{29,	0x621E,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[5].nActvPwrLmt) },
		{30,	0x621F,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[5].nReActvPwrLmt) },
		{31,	0x6220,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[6].tmStart) },
		{32,	0x6221,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[6].nStopSOC) },
		{33,	0x6222,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[6].nBattGrpPwrLmt) },
		{34,	0x6223,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[6].nActvPwrLmt) },
		{35,	0x6224,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[6].nReActvPwrLmt) },
		{36,	0x6225,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[7].tmStart) },
		{37,	0x6226,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[7].nStopSOC) },
		{38,	0x6227,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[7].nBattGrpPwrLmt) },
		{39,	0x6228,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[7].nActvPwrLmt) },
		{40,	0x6229,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[7].nReActvPwrLmt) },
		{41,	0x622A,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[8].tmStart) },
		{42,	0x622B,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[8].nStopSOC) },
		{43,	0x622C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[8].nBattGrpPwrLmt) },
		{44,	0x622D,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[8].nActvPwrLmt) },
		{45,	0x622E,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[8].nReActvPwrLmt) },
		{46,	0x622F,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[9].tmStart) },
		{47,	0x6230,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[9].nStopSOC) },
		{48,	0x6231,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[9].nBattGrpPwrLmt) },
		{49,	0x6232,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[9].nActvPwrLmt) },
		{50,	0x6233,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[9].nReActvPwrLmt) },
		{51,	0x6234,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[10].tmStart) },
		{52,	0x6235,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[10].nStopSOC) },
		{53,	0x6236,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[10].nBattGrpPwrLmt) },
		{54,	0x6237,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[10].nActvPwrLmt) },
		{55,	0x6238,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[10].nReActvPwrLmt) },
		{56,	0x6239,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[11].tmStart) },
		{57,	0x623A,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[11].nStopSOC) },
		{58,	0x623B,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[11].nBattGrpPwrLmt) },
		{59,	0x623C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[11].nActvPwrLmt) },
		{60,	0x623D,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strC2S_Ems.arryEmsParam[11].nReActvPwrLmt) },

	};

	pIEC104->pYTEle_C2SEms = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgC2S_Ems));
	if(pIEC104->pYTEle_C2SEms == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYTEle_C2SEms, cfgC2S_Ems, sizeof(cfgC2S_Ems));
	pIEC104->nC2S_EmsNum = ITEM_OF(cfgC2S_Ems);

	//--------------------- Yao Ce System Info ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_SysInfo[] =
	{
		{0 , 0x4051,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emSysStatus) },
		{1 , 0x4052,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emSysFailReason) },
		{2 , 0x4053,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nBatCfgNum) },
		{3 , 0x4054,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nCycle_Meter) },
		{4 , 0x4055,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nCycle_BMS) },
		{5 , 0x4056,  IEC60870_QUALITY_GOOD , 	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nCycle_PCS) },
		{6 , 0x4057,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nCycle_Kwh) },
		{7 , 0x4058,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.nEmsPlanCRC) },
		{8 , 0x4059,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emCabMeter_CommStatu) },
		{9 , 0x405A,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emGridMeter_CommStatu) },
		{10 , 0x405B,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emAuxMeter_CommStatu) },
		{11 , 0x405C,  IEC60870_QUALITY_GOOD ,	OBJVALUE_TYPE_INT, 1,	(void*)&(pIEC104->strYC_SysInfo.emAux3Meter_CommStatu) },

	};
	pIEC104->pYCEle_Sys = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_SysInfo));
	if(pIEC104->pYCEle_Sys == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_Sys, cfgYC_SysInfo, sizeof(cfgYC_SysInfo));
	pIEC104->nYCNum_Sys = ITEM_OF(cfgYC_SysInfo);


	//--------------------- Yao Ce CabMeter SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_CabMeter[] =
	{
		{0 ,0x4101,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseA_Volt) },
		{1 ,0x4102,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseB_Volt) },
		{2 ,0x4103,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseC_Volt) },
		{3 ,0x4104,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseA_Curt) },
		{4 ,0x4105,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseB_Curt) },
		{5 ,0x4106,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhaseC_Curt) },
		{6 ,0x4107,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fActive_Power) },
		{7 ,0x4108,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fReactive_Power) },
		{8 ,0x4109,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].f_Freq) },
		{9 ,0x410A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].f_Factor) },
		{10,0x410B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPos_Kwh) },
		{11,0x410C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fNeg_Kwh) },
		{12,0x410D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPmax_Demand) },
		{13,0x410E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhA_Kvar) },
		{14,0x410F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhB_Kvar) },
		{15,0x4110,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhC_Kvar) },

		{16,0x4111,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhA_KW) },
		{17,0x4112,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhB_KW) },
		{18,0x4113,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhC_KW) },
 
 		{19,0x4114,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhA_Factor) },
		{20,0x4115,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhB_Factor) },
		{21,0x4116,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[0].fPhC_Factor) },
 
	};

	pIEC104->pYCEle_CabMeter = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_CabMeter));
	if(pIEC104->pYCEle_CabMeter == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_CabMeter, cfgYC_CabMeter, sizeof(cfgYC_CabMeter));
	pIEC104->nYCNum_CabMeter = ITEM_OF(cfgYC_CabMeter);

	//--------------------- Yao Ce GridMeter SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_GridMeter[] =
	{
		{0 ,0x4201,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseA_Volt) },
		{1 ,0x4202,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseB_Volt) },
		{2 ,0x4203,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseC_Volt) },
		{3 ,0x4204,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseA_Curt) },
		{4 ,0x4205,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseB_Curt) },
		{5 ,0x4206,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhaseC_Curt) },
		{6 ,0x4207,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fActive_Power) },
		{7 ,0x4208,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fReactive_Power) },
		{8 ,0x4209,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].f_Freq) },
		{9 ,0x420A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].f_Factor) },
		{10,0x420B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPos_Kwh) },
		{11,0x420C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fNeg_Kwh) },
		{12,0x420D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPmax_Demand) },
		{13,0x420E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhA_Kvar) },
		{14,0x420F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhB_Kvar) },
		{15,0x4210,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhC_Kvar) },

		{16,0x4211,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhA_KW) },
		{17,0x4212,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhB_KW) },
		{18,0x4213,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhC_KW) },
 
 		{19,0x4214,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhA_Factor) },
		{20,0x4215,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhB_Factor) },
		{21,0x4216,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[1].fPhC_Factor) },
	};

	pIEC104->pYCEle_GridMeter = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_GridMeter));
	if(pIEC104->pYCEle_GridMeter == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_GridMeter, cfgYC_GridMeter, sizeof(cfgYC_GridMeter));
	pIEC104->nYCNum_GridMeter = ITEM_OF(cfgYC_GridMeter);


	//--------------------- Yao Ce AuxMeter SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_AuxMeter[] =
	{
		{0 ,0x4251,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseA_Volt) },
		{1 ,0x4252,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseB_Volt) },
		{2 ,0x4253,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseC_Volt) },
		{3 ,0x4254,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseA_Curt) },
		{4 ,0x4255,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseB_Curt) },
		{5 ,0x4256,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhaseC_Curt) },
		{6 ,0x4257,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fActive_Power) },
		{7 ,0x4258,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fReactive_Power) },
		{8 ,0x4259,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].f_Freq) },
		{9 ,0x425A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].f_Factor) },
		{10,0x425B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPos_Kwh) },
		{11,0x425C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fNeg_Kwh) },
		{12,0x425D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPmax_Demand) },
		{13,0x425E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhA_Kvar) },
		{14,0x425F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhB_Kvar) },
		{15,0x4260,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhC_Kvar) },
		
		{16,0x4261,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhA_KW) },
		{17,0x4262,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhB_KW) },
		{18,0x4263,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhC_KW) },
 
 		{19,0x4264,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhA_Factor) },
		{20,0x4265,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhB_Factor) },
		{21,0x4266,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[2].fPhC_Factor) },

	};

	pIEC104->pYCEle_AuxMeter = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_AuxMeter));
	if(pIEC104->pYCEle_AuxMeter == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_AuxMeter, cfgYC_AuxMeter, sizeof(cfgYC_AuxMeter));
	pIEC104->nYCNum_AuxMeter = ITEM_OF(cfgYC_AuxMeter);

	//--------------------- Yao Ce Aux3Meter SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_Aux3Meter[] =
	{
		{0 ,0x42A1,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseA_Volt) },
		{1 ,0x42A2,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseB_Volt) },
		{2 ,0x42A3,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseC_Volt) },
		{3 ,0x42A4,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseA_Curt) },
		{4 ,0x42A5,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseB_Curt) },
		{5 ,0x42A6,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhaseC_Curt) },
		{6 ,0x42A7,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fActive_Power) },
		{7 ,0x42A8,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fReactive_Power) },
		{8 ,0x42A9,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].f_Freq) },
		{9 ,0x42AA,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].f_Factor) },
		{10,0x42AB,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPos_Kwh) },
		{11,0x42AC,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fNeg_Kwh) },
		{12,0x42AD,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPmax_Demand) },
		{13,0x42AE,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhA_Kvar) },
		{14,0x42AF,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhB_Kvar) },
		{15,0x42B0,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhC_Kvar) },
		
		{16,0x42B1,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhA_KW) },
		{17,0x42B2,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhB_KW) },
		{18,0x42B3,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhC_KW) },
 
 		{19,0x42B4,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhA_Factor) },
		{20,0x42B5,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhB_Factor) },
		{21,0x42B6,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_Meter[3].fPhC_Factor) },

	};

	pIEC104->pYCEle_Aux3Meter = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_Aux3Meter));
	if(pIEC104->pYCEle_Aux3Meter == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_Aux3Meter, cfgYC_Aux3Meter, sizeof(cfgYC_Aux3Meter));
	pIEC104->nYCNum_Aux3Meter = ITEM_OF(cfgYC_Aux3Meter);


	//--------------------- Yao Ce BAT Group1 SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_BatG1[] =
	{
		{0, 0x4301,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fRunState) },
		{1, 0x4302,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fRack_Volt) },
		{2, 0x4303,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fRack_Curt) },
		{3, 0x4304,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].f_SOC) },
		{4, 0x4305,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].f_SOH) },
		{5, 0x4306,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_AllowChgVolt) },
		{6, 0x4307,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_AllowChgCurt) },
		{7, 0x4308,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_AllowDisChgVolt) },
		{8, 0x4309,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_AllowDisChgCurt) },
		{9, 0x430A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_CellTemp) },
		{10,0x430B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMin_CellTemp) },
		{11,0x430C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMax_CellVolt) },
		{12,0x430D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fMin_CellVolt) },
		{13,0x430E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fNotStartChgReason) },
		{14,0x430F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fNotStartDischgReason) },
		{15,0x4310,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fAlmState) },
		
		{16,0x4311,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fUcell_Max_CSC) },
		{17,0x4312,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fUcell_Max_CELL) },
		{18,0x4313,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fUcell_Min_CSC) },
		{19,0x4314,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fUcell_Min_CELL) },
		{20,0x4315,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fT_Max_CSC) },
		{21,0x4316,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fT_Max_CELL) },
		{22,0x4317,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fT_Min_CSC) },
		{23,0x4318,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fT_Min_CELL) },
		{24,0x4319,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fInsulation_P_R) },
		{25,0x431A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[0].fInsulation_N_R) },

	};

	pIEC104->pYCEle_BatG1 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_BatG1));
	if(pIEC104->pYCEle_BatG1 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_BatG1, cfgYC_BatG1, sizeof(cfgYC_BatG1));
	pIEC104->nYCNum_BatG1 = ITEM_OF(cfgYC_BatG1);
	
	//--------------------- Yao Ce BAT Group2 SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_BatG2[] =
	{
		{0, 0x4321,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fRunState) },
		{1, 0x4322,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fRack_Volt) },
		{2, 0x4323,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fRack_Curt) },
		{3, 0x4324,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].f_SOC) },
		{4, 0x4325,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].f_SOH) },
		{5, 0x4326,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_AllowChgVolt) },
		{6, 0x4327,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_AllowChgCurt) },
		{7, 0x4328,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_AllowDisChgVolt) },
		{8, 0x4329,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_AllowDisChgCurt) },
		{9, 0x432A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_CellTemp) },
		{10,0x432B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMin_CellTemp) },
		{11,0x432C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMax_CellVolt) },
		{12,0x432D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fMin_CellVolt) },
		{13,0x432E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fNotStartChgReason) },
		{14,0x432F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fNotStartDischgReason) },
		{15,0x4330,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fAlmState) },

		{16,0x4331,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fUcell_Max_CSC) },
		{17,0x4332,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fUcell_Max_CELL) },
		{18,0x4333,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fUcell_Min_CSC) },
		{19,0x4334,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fUcell_Min_CELL) },
		{20,0x4335,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fT_Max_CSC) },
		{21,0x4336,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fT_Max_CELL) },
		{22,0x4337,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fT_Min_CSC) },
		{23,0x4338,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fT_Min_CELL) },
		{24,0x4339,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fInsulation_P_R) },
		{25,0x433A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[1].fInsulation_N_R) },
	};

	pIEC104->pYCEle_BatG2 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_BatG2));
	if(pIEC104->pYCEle_BatG2 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_BatG2, cfgYC_BatG2, sizeof(cfgYC_BatG2));
	pIEC104->nYCNum_BatG2 = ITEM_OF(cfgYC_BatG2);

	//--------------------- Yao Ce BAT Group3 SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_BatG3[] =
	{
		{0, 0x4341,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fRunState) },
		{1, 0x4342,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fRack_Volt) },
		{2, 0x4343,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fRack_Curt) },
		{3, 0x4344,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].f_SOC) },
		{4, 0x4345,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].f_SOH) },
		{5, 0x4346,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_AllowChgVolt) },
		{6, 0x4347,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_AllowChgCurt) },
		{7, 0x4348,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_AllowDisChgVolt) },
		{8, 0x4349,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_AllowDisChgCurt) },
		{9, 0x434A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_CellTemp) },
		{10,0x434B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMin_CellTemp) },
		{11,0x434C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMax_CellVolt) },
		{12,0x434D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fMin_CellVolt) },
		{13,0x434E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fNotStartChgReason) },
		{14,0x434F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fNotStartDischgReason) },
 		{15,0x4350,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fAlmState) },

		{16,0x4351,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fUcell_Max_CSC) },
		{17,0x4352,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fUcell_Max_CELL) },
		{18,0x4353,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fUcell_Min_CSC) },
		{19,0x4354,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fUcell_Min_CELL) },
		{20,0x4355,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fT_Max_CSC) },
		{21,0x4356,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fT_Max_CELL) },
		{22,0x4357,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fT_Min_CSC) },
		{23,0x4358,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fT_Min_CELL) },
		{24,0x4359,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fInsulation_P_R) },
		{25,0x435A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[2].fInsulation_N_R) },

	};

	pIEC104->pYCEle_BatG3 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_BatG3));
	if(pIEC104->pYCEle_BatG3 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_BatG3, cfgYC_BatG3, sizeof(cfgYC_BatG3));
	pIEC104->nYCNum_BatG3 = ITEM_OF(cfgYC_BatG3);

	//--------------------- Yao Ce BAT Group4 SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_BatG4[] =
	{
		{0, 0x4361,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fRunState) },
		{1, 0x4362,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fRack_Volt) },
		{2, 0x4363,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fRack_Curt) },
		{3, 0x4364,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].f_SOC) },
		{4, 0x4365,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].f_SOH) },
		{5, 0x4366,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_AllowChgVolt) },
		{6, 0x4367,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_AllowChgCurt) },
		{7, 0x4368,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_AllowDisChgVolt) },
		{8, 0x4369,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_AllowDisChgCurt) },
		{9, 0x436A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_CellTemp) },
		{10,0x436B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMin_CellTemp) },
		{11,0x436C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMax_CellVolt) },
		{12,0x436D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fMin_CellVolt) },
		{13,0x436E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fNotStartChgReason) },
		{14,0x436F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fNotStartDischgReason) },
		{15,0x4370,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fAlmState) },
		{16,0x4371,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fUcell_Max_CSC) },
		{17,0x4372,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fUcell_Max_CELL) },
		{18,0x4373,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fUcell_Min_CSC) },
		{19,0x4374,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fUcell_Min_CELL) },
		{20,0x4375,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fT_Max_CSC) },
		{21,0x4376,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fT_Max_CELL) },
		{22,0x4377,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fT_Min_CSC) },
		{23,0x4378,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fT_Min_CELL) },
		{24,0x4379,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fInsulation_P_R) },
		{25,0x437A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_BAT[3].fInsulation_N_R) },

	};

	pIEC104->pYCEle_BatG4 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_BatG4));
	if(pIEC104->pYCEle_BatG4 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_BatG4, cfgYC_BatG4, sizeof(cfgYC_BatG4));
	pIEC104->nYCNum_BatG4 = ITEM_OF(cfgYC_BatG4);


	//--------------------- Yao Ce BAT Group4 SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_PCS[] =
	{
		{0, 0x4501,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputVolt_Grp1) },
		{1, 0x4502,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputCurt_Grp1) },
		{2, 0x4503,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.emWorkMode_Grp1) },
		{3, 0x4504,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv1_Grp1) },
		{4, 0x4505,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv2_Grp1) },
		{5, 0x4506,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputVolt_Grp2) },
		{6, 0x4507,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputCurt_Grp2) },
		{7, 0x4508,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.emWorkMode_Grp2) },
		{8, 0x4509,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv1_Grp2) },
		{9, 0x450A,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv2_Grp2) },
		{10,0x450B,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputVolt_Grp3) },
		{11,0x450C,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputCurt_Grp3) },
		{12,0x450D,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.emWorkMode_Grp3) },
		{13,0x450E,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv1_Grp3) },
		{14,0x450F,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv2_Grp3) },
		{15,0x4510,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputVolt_Grp4) },
		{16,0x4511,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fOutputCurt_Grp4) },
		{17,0x4512,  IEC60870_QUALITY_GOOD , 	OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.emWorkMode_Grp4) },
		{18,0x4513,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv1_Grp4) },
		{19,0x4514,  IEC60870_QUALITY_INVALID , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_PCS.fResv2_Grp4) },
	};

	pIEC104->pYCEle_PCS = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_PCS));
	if(pIEC104->pYCEle_PCS == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_PCS, cfgYC_PCS, sizeof(cfgYC_PCS));
	pIEC104->nYCNum_PCS = ITEM_OF(cfgYC_PCS);


	//--------------------- Yao Ce Single PCS SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_SiglPCS[] =
	{
		{0, 0x4601,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fOnlineNum_Grp[0]) },
		{1, 0x4602,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPsum_Grp[0]) },
		{2, 0x4603,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphA_Grp[0]) },
		{3, 0x4604,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphB_Grp[0]) },
		{4, 0x4605,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphC_Grp[0]) },
		{5, 0x4606,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQsum_Grp[0]) },
		{6, 0x4607,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphA_Grp[0]) },
		{7, 0x4608,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphB_Grp[0]) },
		{8, 0x4609,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphC_Grp[0]) },
		{9, 0x460A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fOnlineNum_Grp[1]) },
		{10,0x460B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPsum_Grp[1]) },
		{11,0x460C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphA_Grp[1]) },
		{12,0x460D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphB_Grp[1]) },
		{13,0x460E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphC_Grp[1]) },
		{14,0x460F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQsum_Grp[1]) },
		{15,0x4610,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphA_Grp[1]) },
		{16,0x4611,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphB_Grp[1]) },
		{17,0x4612,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphC_Grp[1]) },
		{18,0x4613,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fOnlineNum_Grp[2]) },
		{19,0x4614,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPsum_Grp[2]) },
		{20,0x4615,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphA_Grp[2]) },
		{21,0x4616,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphB_Grp[2]) },
		{22,0x4617,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphC_Grp[2]) },
		{23,0x4618,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQsum_Grp[2]) },
		{24,0x4619,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphA_Grp[2]) },
		{25,0x461A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphB_Grp[2]) },
		{26,0x461B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphC_Grp[2]) },
	
		{27,0x461C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fOnlineNum_Grp[3]) },
		{28,0x461D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPsum_Grp[3]) },
		{29,0x461E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphA_Grp[3]) },
		{30,0x461F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphB_Grp[3]) },
		{31,0x4620,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fPphC_Grp[3]) },
		{32,0x4621,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQsum_Grp[3]) },
		{33,0x4622,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphA_Grp[3]) },
		{34,0x4623,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphB_Grp[3]) },
		{35,0x4624,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_SiglPCS.fQphC_Grp[3]) },
	
	};

	pIEC104->pYCEle_SiglPCS = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_SiglPCS));
	if(pIEC104->pYCEle_SiglPCS == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_SiglPCS, cfgYC_SiglPCS, sizeof(cfgYC_SiglPCS));
	pIEC104->nYCNum_SiglPCS = ITEM_OF(cfgYC_SiglPCS);

	//------------------------Mppt Grp--------------------------------//
	IEC104_CONFIG_SIGMAP cfgYC_MpptGrp[] =
	{
		{0, 0x4701,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.emInstStatu_Grp1)},
		{1, 0x4702,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputVolt_Grp1)},
		{2, 0x4703,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputCurt_Grp1)},
		{3, 0x4704,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv1_Grp1		)},
		{4, 0x4705,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv2_Grp1		)},
		{5, 0x4706,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv3_Grp1		)},
		{6, 0x4707,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.emInstStatu_Grp2)},
		{7, 0x4708,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputVolt_Grp2)},
		{8, 0x4709,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputCurt_Grp2)},
		{9, 0x470A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv1_Grp2		)},
		{10,0x470B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv2_Grp2		)},
		{11,0x470C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv3_Grp2		)},
		{12,0x470D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.emInstStatu_Grp3)},
		{13,0x470E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputVolt_Grp3)},
		{14,0x470F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputCurt_Grp3)},
		{15,0x4710,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv1_Grp3		)},
		{16,0x4711,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv2_Grp3		)},
		{17,0x4712,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv3_Grp3		)},
		{18,0x4713,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.emInstStatu_Grp4)},
		{19,0x4714,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputVolt_Grp4)},
		{20,0x4715,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fOutputCurt_Grp4)},
		{21,0x4716,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv1_Grp4		)},
		{22,0x4717,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv2_Grp4		)},
		{23,0x4718,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_MpptGrp.fResv3_Grp4		)},	
	};

	pIEC104->pYCEle_MpptGrp = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_MpptGrp));
	if(pIEC104->pYCEle_MpptGrp == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_MpptGrp, cfgYC_MpptGrp, sizeof(cfgYC_MpptGrp));
	pIEC104->nYCNum_MpptGrp = ITEM_OF(cfgYC_MpptGrp);

	//--------------------- Station Summary SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_LocalSystem[] =
	{
		{0, 0x5001,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fMode_SysControl) },
		{1, 0x5002,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.TargetTotalActivePower) },
		{2, 0x5003,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fStatus_SysAvailable) },
		{3, 0x5004,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fStatus_Work) },
		{4, 0x5005,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fTotalSOC) },
		{5, 0x5006,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fActivePower_CabinetMeterMeasured) },
		{6, 0x5007,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fActivePower_PCSMeasured) },
		{7, 0x5008,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fMaxChargePowerAbility) },
		{8, 0x5009,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fMaxDischargePowerAbility) },
		{9, 0x500A,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fTotalChargekWh) },
		{10, 0x500B,	 IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_LocalSystem.fTotalOutputCurrent) }
 
	};

	pIEC104->pYCEle_LocalSystem = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_LocalSystem));
	if(pIEC104->pYCEle_LocalSystem == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_LocalSystem, cfgYC_LocalSystem, sizeof(cfgYC_LocalSystem));
	pIEC104->nYCNum_LocalSystem = ITEM_OF(cfgYC_LocalSystem);
	
	// YNT need add Cell Info
	//--------------------- Yao Ce CellInfo Group1 SigMap ---------------------//
	// 240 * 2 = 480
	IEC104_CONFIG_SIGMAP cfgYC_CellG1[480];
	UINT id;
	for(id=0; id<480; id++)
	{
		if(id < 240)
		{
			cfgYC_CellG1[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA001+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellVolt[id]) };
		}
		else
		{
			cfgYC_CellG1[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA001+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellTemp[id-240]) };
		}
	};

	pIEC104->pYCEle_CellG1 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_CellG1));
	if(pIEC104->pYCEle_CellG1 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_CellG1, cfgYC_CellG1, sizeof(cfgYC_CellG1));
	pIEC104->nYCNum_CellG1 = ITEM_OF(cfgYC_CellG1);

	//--------------------- Yao Ce CellInfo Group2 SigMap ---------------------//
	// 240 * 2 = 480
	IEC104_CONFIG_SIGMAP cfgYC_CellG2[480];
	for(id=0; id<480; id++)
	{
		if(id < 240)
		{
			cfgYC_CellG2[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA1E1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellVolt[id]) };
		}
		else
		{
			cfgYC_CellG2[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA1E1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellTemp[id-240]) };
		}
	};

	pIEC104->pYCEle_CellG2 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_CellG2));
	if(pIEC104->pYCEle_CellG2 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_CellG2, cfgYC_CellG2, sizeof(cfgYC_CellG2));
	pIEC104->nYCNum_CellG2 = ITEM_OF(cfgYC_CellG2);

	//--------------------- Yao Ce CellInfo Group3 SigMap ---------------------//
	// 240 * 2 = 480
	IEC104_CONFIG_SIGMAP cfgYC_CellG3[480];
	for(id=0; id<480; id++)
	{
		if(id < 240)
		{
			cfgYC_CellG3[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA3C1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellVolt[id]) };
		}
		else
		{
			cfgYC_CellG3[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA3C1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellTemp[id-240]) };
		}
	};

	pIEC104->pYCEle_CellG3 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_CellG3));
	if(pIEC104->pYCEle_CellG3 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_CellG3, cfgYC_CellG3, sizeof(cfgYC_CellG3));
	pIEC104->nYCNum_CellG3 = ITEM_OF(cfgYC_CellG3);

	//--------------------- Yao Ce CellInfo Group4 SigMap ---------------------//
	// 240 * 2 = 480
	IEC104_CONFIG_SIGMAP cfgYC_CellG4[480];
	for(id=0; id<480; id++)
	{
		if(id < 240)
		{
			cfgYC_CellG4[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA5A1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellVolt[id]) };
		}
		else
		{
			cfgYC_CellG4[id] = 
				(IEC104_CONFIG_SIGMAP){id+1 ,0xA5A1+id,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_CellInfo[0].fcellTemp[id-240]) };
		}
	};

	pIEC104->pYCEle_CellG4 = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_CellG4));
	if(pIEC104->pYCEle_CellG4 == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_CellG4, cfgYC_CellG4, sizeof(cfgYC_CellG4));
	pIEC104->nYCNum_CellG4 = ITEM_OF(cfgYC_CellG4);

	//--------------------- KWH Statistic SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_KWh[] =
	{
		{0, 0x6401,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fDate) },
		{1, 0x6402,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_PosDailyKwh) },
		{2, 0x6403,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_NegDailyKwh) },
		{3, 0x6404,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_PosDailySharpKwh) },
		{4, 0x6405,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_PosDailyPeakKwh) },
		{5, 0x6406,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_PosDailyFlatKwh) },
		{6, 0x6407,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_PosDailyValleyKwh) },
		{7, 0x6408,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_NegDailySharpKwh) },
		{8, 0x6409,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_NegDailyPeakKwh) },
		{9, 0x640A,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_NegDailyFlatKwh) },
		{10,0x640B,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fCab_NegDailyValleyKwh) },
		{11,0x640C,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT1_DailyChgKwh) },
		{12,0x640D,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT1_DailyDischgKwh) },
		{13,0x640E,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT2_DailyChgKwh) },
		{14,0x640F,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT2_DailyDischgKwh) },
		{15,0x6410,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT3_DailyChgKwh) },
		{16,0x6411,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT3_DailyDischgKwh) },
		{17,0x6412,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT4_DailyChgKwh) },
		{18,0x6413,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_KWh.fBAT4_DailyDischgKwh) },
 
	};

	pIEC104->pYCEle_KWh = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_KWh));
	if(pIEC104->pYCEle_KWh == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_KWh, cfgYC_KWh, sizeof(cfgYC_KWh));
	pIEC104->nYCNum_KWh = ITEM_OF(cfgYC_KWh);

	//--------------------- Yao Ce Air SigMap ---------------------//
	IEC104_CONFIG_SIGMAP cfgYC_Air[] =
	{
		{0 ,0x5051,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_AirInfo.fLiqColConMode		) },
		{1 ,0x5052,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_AirInfo.fOutletWaterTemp	) },
		{2 ,0x5053,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_AirInfo.fReturnWaterTemp	) },
		{3 ,0x5054,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_AirInfo.fOutletWaterPress	) },
		{4 ,0x5055,  IEC60870_QUALITY_GOOD , OBJVALUE_FLOAT_DOT1, 1,	(void*)&(pIEC104->strYC_AirInfo.fReturnWaterPress	) },

	};

	pIEC104->pYCEle_AirInfo = NEW(IEC104_CONFIG_SIGMAP, ITEM_OF(cfgYC_Air));
	if(pIEC104->pYCEle_AirInfo == NULL)
	{
		return FALSE;
	}
	memcpy(pIEC104->pYCEle_AirInfo, cfgYC_Air, sizeof(cfgYC_Air));
	pIEC104->nYCNum_AirInfo = ITEM_OF(cfgYC_Air);

	return TRUE;
}
 

static void Refresh_YX_CabMeter_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			407, 401, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0)//0 :METER_TYPE_NotInstalled
	{
		pIEC104->strYC_SysInfo.emCabMeter_CommStatu = 2; //CommFial
		return;
	}
	else
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			407, 401, &varV, 4);
		pIEC104->strYC_SysInfo.emCabMeter_CommStatu = varV.emValue;
	}
	//----------- Refresh Cabinet Meter YC CabMeter SigValue -------------------//
 
	pIEC104->strYC_Meter[0].fPhaseA_Volt = pIEC104->esRunData.fACVolt_L1;
	pIEC104->strYC_Meter[0].fPhaseB_Volt = pIEC104->esRunData.fACVolt_L2;
	pIEC104->strYC_Meter[0].fPhaseC_Volt = pIEC104->esRunData.fACVolt_L3;
	pIEC104->strYC_Meter[0].fPhaseA_Curt = pIEC104->esRunData.fACCurr_L1;
	pIEC104->strYC_Meter[0].fPhaseB_Curt = pIEC104->esRunData.fACCurr_L2;
	pIEC104->strYC_Meter[0].fPhaseC_Curt = pIEC104->esRunData.fACCurr_L3;
 	pIEC104->strYC_Meter[0].fActive_Power = pIEC104->esRunData.f_PcabMeter;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 404, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 426, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 424, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 425, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPmax_Demand = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 405, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhA_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 406, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhB_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 407, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhC_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 401, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhA_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 402, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhB_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 403, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhC_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 427, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhA_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 428, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhB_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 429, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[0].fPhC_Factor = valGet.val.fValue;

	return;
}

/************************************************************
 * FUNC: GridSide AC Meter
 * PARAM:
 * DATE: 2023-08-10 
 * HIS: 部分场站电网侧存在两路变压器，第一路变压器定义为第一关口表；第二路变压器定义为第二关口表；
 * 当参数设置中网侧关口表与辅助电表配置互换时，
 * 上送给平台的协议是固定的，即网侧表上送第一关口表数据，辅助表上送第二关口表位置；
 * 因此此处需要判断一下槽位，根据槽位判断是在第一关口表还是第二关口表；
 * 比如参数设置界面-网侧电表是第一关口表，则如下函数中需从第一关口表槽位获取数据.如果网侧表槽位是第二关口表，
 * 则如下函数得判断辅助电表是不是第一关口表，如果是则从辅助电表获取数据；
 * Creator:John
 * 
 * *********************************************************/

static void Refresh_YX_FirstGateWayMeter_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;
	int					nDevID = 408;

	//-------------Step1. Find First GateWay ACMater.--------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			408, 406, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0)//0-First GateWay ACMeter
	{
		nDevID = 408;
	}
	else
	{
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			409, 406, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//0-First GateWay ACMeter
		{
			nDevID = 409;
		}
		else
		{
			TRACE("Warning: Not Found First GateWay Meter!nDevID=[%d] valGet.val.emValue=[%d]\n", nDevID, valGet.val.emValue);
			return;
		}
	}

	//-----Step2. Judge If ACMeter Installed, then Judge CommStatus.-----//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0)
	{
		pIEC104->strYC_SysInfo.emGridMeter_CommStatu = 2; //0-Normal, 1-CommFial, 2-Not Installed
		return;
	}
	else
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			nDevID, 401, &varV, 4);
		pIEC104->strYC_SysInfo.emGridMeter_CommStatu = varV.emValue;
	}

	//----------------------------- Grid AC Meter ----------------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 413, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseA_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 415, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseB_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 417, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseC_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 414, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseA_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 416, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseB_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 418, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhaseC_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 400, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fActive_Power = valGet.val.fValue;
	
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 404, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 426, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 424, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 425, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPmax_Demand = valGet.val.fValue;

 	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 405, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhA_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 406, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhB_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 407, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhC_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhA_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 402, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhB_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 403, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhC_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 427, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhA_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 428, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhB_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 429, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[1].fPhC_Factor = valGet.val.fValue;

	return;
}

static void Refresh_YX_SecondGateWayMeter_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;
	int 				nDevID = 409;

	//-------------Step1. Find Second GateWay ACMater.--------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			408, 406, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 1)//1-Second GateWay ACMeter
	{
		nDevID = 408;
	}
	else
	{
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			409, 406, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 1)//1-Second GateWay ACMeter
		{
			nDevID = 409;
		}
		else
		{
			TRACE("Warning: Not Found Second GateWay Meter!nDevID=[%d] valGet.val.emValue=[%d]\n", nDevID, valGet.val.emValue);
			return;
		}
	}

	//-----Step2. Judge If ACMeter Installed, then Judge CommStatus.-----//
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0)
	{
		pIEC104->strYC_SysInfo.emAuxMeter_CommStatu = 2; //0-Normal, 1-CommFial, 2-Not Installed
		return;
	}
	else
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			nDevID, 401, &varV, 4);
		pIEC104->strYC_SysInfo.emAuxMeter_CommStatu = varV.emValue;
	}

	//printf("Refresh Aux Value nDevID=[%d]\n", nDevID);
	//----------- Refresh Aux Meter YC Meter SigValue -------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 413, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseA_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 415, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseB_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 417, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseC_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 414, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseA_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 416, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseB_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 418, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhaseC_Curt = valGet.val.fValue;

 	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 400, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fActive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 404, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 426, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 424, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 425, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPmax_Demand = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 405, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhA_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 406, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhB_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 407, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhC_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhA_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 402, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhB_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 403, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhC_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 427, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhA_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 428, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhB_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 429, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[2].fPhC_Factor = valGet.val.fValue;

	return;
}



static void Refresh_YX_Aux3GateWayMeter_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;
	int 				nDevID = 410;

	//-----Step1. Judge If Aux3 ACMeter Installed, then Judge CommStatus.-----//
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0)
	{
		pIEC104->strYC_SysInfo.emAux3Meter_CommStatu = 2; //0-Normal, 1-CommFial, 2-Not Installed
		return;
	}
	else
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
			nDevID, 401, &varV, 4);
		pIEC104->strYC_SysInfo.emAux3Meter_CommStatu = varV.emValue;
	}

	//printf("Refresh Aux Value nDevID=[%d]\n", nDevID);
	//----------- Refresh Aux Meter YC Meter SigValue -------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 413, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseA_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 415, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseB_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 417, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseC_Volt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 414, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseA_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 416, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseB_Curt = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 418, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhaseC_Curt = valGet.val.fValue;

 	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 400, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fActive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 404, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 426, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 424, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 425, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPmax_Demand = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 405, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhA_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 406, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhB_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 407, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhC_Kvar = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhA_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 402, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhB_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 403, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhC_KW = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 427, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhA_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 428, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhB_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		nDevID, 429, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_Meter[3].fPhC_Factor = valGet.val.fValue;

	return;
}

static void Refresh_YC_PCS_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;
	int 				n = 0;
	int 				nRectType = 0;

	//------------------- Refresh YaoCe PCS Group DataValue ---------------------//
	int nDirect = 1;
	pIEC104->strYC_PCS.fOutputVolt_Grp1 = pIEC104->esRunData.fACDC_OutVolt[0];
	if(pIEC104->esRunData.emACDC_WorkStatu[0] == STATU_ACDC_STANDBY)
	{
		pIEC104->strYC_PCS.emWorkMode_Grp1  = STATU_ACDC_IDLE;
	}
	else
	{
		pIEC104->strYC_PCS.emWorkMode_Grp1  = (float)pIEC104->esRunData.emACDC_WorkStatu[0];
	}
	nDirect = (pIEC104->esRunData.emACDC_WorkStatu[0] == STATU_ACDC_INVERTER) ? -1 : 1;
	pIEC104->strYC_PCS.fOutputCurt_Grp1 = nDirect*pIEC104->esRunData.fACDC_OutCurr[0];

 
	pIEC104->strYC_PCS.fOutputVolt_Grp2 = pIEC104->esRunData.fACDC_OutVolt[1];
	if(pIEC104->esRunData.emACDC_WorkStatu[1] == STATU_ACDC_STANDBY)
	{
		pIEC104->strYC_PCS.emWorkMode_Grp2  = STATU_ACDC_IDLE;
	}
	else
	{
 		pIEC104->strYC_PCS.emWorkMode_Grp2  = (float)pIEC104->esRunData.emACDC_WorkStatu[1];
	}
 	nDirect = (pIEC104->esRunData.emACDC_WorkStatu[1] == STATU_ACDC_INVERTER) ? -1 : 1;
	pIEC104->strYC_PCS.fOutputCurt_Grp2 = nDirect*pIEC104->esRunData.fACDC_OutCurr[1];

	pIEC104->strYC_PCS.fOutputVolt_Grp3 = pIEC104->esRunData.fACDC_OutVolt[2];
	if(pIEC104->esRunData.emACDC_WorkStatu[2] == STATU_ACDC_STANDBY)
	{
		pIEC104->strYC_PCS.emWorkMode_Grp3  = STATU_ACDC_IDLE;
	}
	else
	{
		pIEC104->strYC_PCS.emWorkMode_Grp3  = (float)pIEC104->esRunData.emACDC_WorkStatu[2];
	}
	nDirect = (pIEC104->esRunData.emACDC_WorkStatu[2] == STATU_ACDC_INVERTER) ? -1 : 1;
	pIEC104->strYC_PCS.fOutputCurt_Grp3 = nDirect*pIEC104->esRunData.fACDC_OutCurr[2];
 
	pIEC104->strYC_PCS.fOutputVolt_Grp4 = pIEC104->esRunData.fACDC_OutVolt[3];
	if(pIEC104->esRunData.emACDC_WorkStatu[3] == STATU_ACDC_STANDBY)
	{
		pIEC104->strYC_PCS.emWorkMode_Grp4  = STATU_ACDC_IDLE;	
	}
	else
	{
		pIEC104->strYC_PCS.emWorkMode_Grp4  = (float)pIEC104->esRunData.emACDC_WorkStatu[3];
	}
	nDirect = (pIEC104->esRunData.emACDC_WorkStatu[3] == STATU_ACDC_INVERTER) ? -1 : 1;
	pIEC104->strYC_PCS.fOutputCurt_Grp4 = nDirect*pIEC104->esRunData.fACDC_OutCurr[3];


	//------------------- Refresh Single PCS  DataValue ---------------------//
	A_BLOB_SINGLE_RECT_INFO sigRect;
	ZERO_POBJS(&sigRect, 	1);
	sigRect.emBlobType = BLOB_TYPE_SINGLE_RECT_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
	valGet.val.blobValue = (BYTE*)&sigRect;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 101, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q106 Failed err=[%d]!\n", err);
		return ;
	}

	for(nRectType = 0; nRectType <= TYPE_SIG_RECT_ACDC_GRP4; nRectType++)
	{
		switch (nRectType)
		{
			case TYPE_SIG_RECT_ACDC_GRP1:
			case TYPE_SIG_RECT_ACDC_GRP2:
    		case TYPE_SIG_RECT_ACDC_GRP3:
    		case TYPE_SIG_RECT_ACDC_GRP4:
			{
				float	fOnlineNum_Grp = 0;
				float	fPsum_Grp = 0;
				float	fPphA_Grp = 0;
				float	fPphB_Grp = 0;
				float	fPphC_Grp = 0;
				float	fQsum_Grp = 0;
				float	fQphA_Grp = 0;
				float	fQphB_Grp = 0;
				float	fQphC_Grp = 0;
				
				fOnlineNum_Grp = sigRect.nOnlineNum[nRectType];
				for(n = 0; n < sigRect.nOnlineNum[nRectType]; n++)
				{
					fPsum_Grp += sigRect.arrRect[nRectType][n].fPtotalActive;
					fPphA_Grp += sigRect.arrRect[nRectType][n].fPphase_Active[0];
					fPphB_Grp += sigRect.arrRect[nRectType][n].fPphase_Active[1];
					fPphC_Grp += sigRect.arrRect[nRectType][n].fPphase_Active[2];
					fQsum_Grp += sigRect.arrRect[nRectType][n].fPtotalReactive;
					fQphA_Grp += sigRect.arrRect[nRectType][n].fPphase_Reactive[0];
					fQphB_Grp += sigRect.arrRect[nRectType][n].fPphase_Reactive[1];
					fQphC_Grp += sigRect.arrRect[nRectType][n].fPphase_Reactive[2];
				}

				pIEC104->strYC_SiglPCS.fOnlineNum_Grp[nRectType] = fOnlineNum_Grp;
				pIEC104->strYC_SiglPCS.fPsum_Grp[nRectType] = fPsum_Grp;
				pIEC104->strYC_SiglPCS.fPphA_Grp[nRectType] = fPphA_Grp;
				pIEC104->strYC_SiglPCS.fPphB_Grp[nRectType] = fPphB_Grp;
				pIEC104->strYC_SiglPCS.fPphC_Grp[nRectType] = fPphC_Grp;
 				pIEC104->strYC_SiglPCS.fQsum_Grp[nRectType] = fQsum_Grp;
				pIEC104->strYC_SiglPCS.fQphA_Grp[nRectType] = fQphA_Grp;
				pIEC104->strYC_SiglPCS.fQphB_Grp[nRectType] = fQphB_Grp;
				pIEC104->strYC_SiglPCS.fQphC_Grp[nRectType] = fQphC_Grp;

			}
			break;
			
			default:break;
		}
	}

	return;
}


static void Refresh_YC_MpptGrp_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	BOOL				bNeedUploadMppt = FALSE;
	int 				n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(pIEC104->esRunData.isMPPT_Grp_Exist[n] == TRUE)
		{
			bNeedUploadMppt = TRUE;
			break;
		}
	}
	pIEC104->bNeedUploadMppt = bNeedUploadMppt;

	//----- Get Mppt Group DataInfo -----//
	pIEC104->strYC_MpptGrp.emInstStatu_Grp1 = (float)pIEC104->esRunData.isMPPT_Grp_Exist[0];
 	pIEC104->strYC_MpptGrp.fOutputVolt_Grp1 = pIEC104->esRunData.fMPPT_OutVolt[0];
	pIEC104->strYC_MpptGrp.fOutputCurt_Grp1 = pIEC104->esRunData.fMPPT_OutCurr[0];
	pIEC104->strYC_MpptGrp.fResv1_Grp1		= 0.0;
	pIEC104->strYC_MpptGrp.fResv2_Grp1		= 0.0;
	pIEC104->strYC_MpptGrp.fResv3_Grp1		= 0.0;

	pIEC104->strYC_MpptGrp.emInstStatu_Grp2 = (float)pIEC104->esRunData.isMPPT_Grp_Exist[1];
	pIEC104->strYC_MpptGrp.fOutputVolt_Grp2 = pIEC104->esRunData.fMPPT_OutVolt[1];
	pIEC104->strYC_MpptGrp.fOutputCurt_Grp2 = pIEC104->esRunData.fMPPT_OutCurr[1];
	pIEC104->strYC_MpptGrp.fResv1_Grp2		= 0.0;
	pIEC104->strYC_MpptGrp.fResv2_Grp2		= 0.0;
	pIEC104->strYC_MpptGrp.fResv3_Grp2		= 0.0;

	pIEC104->strYC_MpptGrp.emInstStatu_Grp3 = (float)pIEC104->esRunData.isMPPT_Grp_Exist[2];
	pIEC104->strYC_MpptGrp.fOutputVolt_Grp3 = pIEC104->esRunData.fMPPT_OutVolt[2];
	pIEC104->strYC_MpptGrp.fOutputCurt_Grp3 = pIEC104->esRunData.fMPPT_OutCurr[2];
	pIEC104->strYC_MpptGrp.fResv1_Grp3		= 0.0;
	pIEC104->strYC_MpptGrp.fResv2_Grp3		= 0.0;
	pIEC104->strYC_MpptGrp.fResv3_Grp3		= 0.0;

	pIEC104->strYC_MpptGrp.emInstStatu_Grp4 = (float)pIEC104->esRunData.isMPPT_Grp_Exist[3];
	pIEC104->strYC_MpptGrp.fOutputVolt_Grp4 = pIEC104->esRunData.fMPPT_OutVolt[3];
	pIEC104->strYC_MpptGrp.fOutputCurt_Grp4 = pIEC104->esRunData.fMPPT_OutCurr[3];
	pIEC104->strYC_MpptGrp.fResv1_Grp4		= 0.0;
	pIEC104->strYC_MpptGrp.fResv2_Grp4		= 0.0;
	pIEC104->strYC_MpptGrp.fResv3_Grp4		= 0.0;

	return;
}

static void Refresh_YC_LocalSys_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

	//----- Get LocalSys DataInfo -----//
	PARAM_VAL_GET valGet;

	// LJH250310 modified
	A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &pIEC104->clientInfoSum;
	ZERO_POBJS(pClientSum, 1);
	pClientSum->emBlobType = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
	valGet.val.blobValue = (BYTE*)pClientSum;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3900, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACEE("Refresh_YC_LocalSys_SigMapValue err:%d", err);
	}
	TRACED("Refresh_YC_LocalSys_SigMapValue "
		"ClientSum emEMSCtrlMode_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d "
        "fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f fMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f "
		"fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
		"f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f",
		pClientSum->emEMSCtrlMode_REMS, pClientSum->fActvPwrReq_REMS, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, 
        pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum, pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum,
		pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
		pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum);
	
	pIEC104->strYC_LocalSystem.fMode_SysControl = (float)pClientSum->emEMSCtrlMode_REMS;
	pIEC104->strYC_LocalSystem.TargetTotalActivePower =  pClientSum->fActvPwrReq_REMS;
	
	pIEC104->strYC_LocalSystem.fStatus_SysAvailable = (float)pClientSum->emCCUStateSum;
	// emPMCtrlModeSum =  pClientSum->emPMCtrlModeSum;
	pIEC104->strYC_LocalSystem.fStatus_Work =         (float)pClientSum->emACDC_WorkStatuSum;

	pIEC104->strYC_LocalSystem.fTotalSOC =                         pClientSum->fSOCSum;
	pIEC104->strYC_LocalSystem.fActivePower_CabinetMeterMeasured = pClientSum->f_PcabMeterSum;
	pIEC104->strYC_LocalSystem.fActivePower_PCSMeasured =          pClientSum->fActvPwr_PCSGrpSum;
	// fQ_PCSGrpSum =            pClientSum->fQ_PCSGrpSum;
	pIEC104->strYC_LocalSystem.fMaxChargePowerAbility =            pClientSum->fMax_Chrg_PwrAbilitySum;

	pIEC104->strYC_LocalSystem.fMaxDischargePowerAbility = pClientSum->fMax_DisChrg_PwrAbilitySum;
	pIEC104->strYC_LocalSystem.fTotalChargekWh =           pClientSum->fChrgedEnergySum;
	// fRealTime_MaxCurrSum =       pClientSum->fRealTime_MaxCurrSum;
	// fACDC_RealTime_VoltSum =     pClientSum->fACDC_RealTime_VoltSum;
	pIEC104->strYC_LocalSystem.fTotalOutputCurrent =       pClientSum->fACDC_RealTime_CurrSum;
	return;
}

//XSY250417 add YX alarm
// static void Refresh_YX_BmsAlm_SigMapValue(IEC104_DATA_ROUGH* pIEC104, int BattIdx)
// {
// 	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
// 	PARAM_VAL_GET 		valGet;
// 	VAR_VALUE			varV;
// 	ERRCODE_DAI 		err;
// 	int 				nDevID = 410;

// 	//-----Step1. Judge If Aux3 ACMeter Installed, then Judge CommStatus.-----//
// 	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
// 			nDevID, 401, &valGet, sizeof(PARAM_VAL_GET));
// 	if(valGet.val.emValue == 0)
// 	{
// 		pIEC104->strYC_SysInfo.emAux3Meter_CommStatu = 2; //0-Normal, 1-CommFial, 2-Not Installed
// 		return;
// 	}

// 	return;
// }

static void Refresh_YC_Air_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

	//----- Get Air DataInfo -----//
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;

	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 951, 448, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == 0) // not install
	{
		return;
	}
	else if(valGet.val.emValue == 2) // TF
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 1771, &valGet, sizeof(PARAM_VAL_GET));
		float mode;
		switch(valGet.val.emValue)
		{
			case 0: mode = 0;break;
			case 1: mode = 2;break;
			case 2: mode = 3;break;
			case 3: mode = 1;break;
			case 4: mode = 0;break;
			default: mode = 0;break;
		}
	
		pIEC104->strYC_AirInfo.fLiqColConMode		= mode;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 1779, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 1780, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fReturnWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 1789, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterPress	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 1790, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fReturnWaterPress	= valGet.val.fValue;
	
	}
	else if(valGet.val.emValue == 3) // imeu3+air
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3502, &valGet, sizeof(PARAM_VAL_GET));
		int mode;
		switch(valGet.val.emValue == 1)
		{
			case 1: mode = 2;break;
			case 2: mode = 3;break;
			case 3: mode = 1;break;
			case 5: mode = 4;break;
			default: mode = 0;
		}
		pIEC104->strYC_AirInfo.fLiqColConMode		= (float)mode;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3537, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3537, &valGet, sizeof(PARAM_VAL_GET));// need check
		pIEC104->strYC_AirInfo.fReturnWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3545, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterPress	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3544, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fReturnWaterPress	= valGet.val.fValue;
	}
	else if(valGet.val.emValue == 4) // xinmiao
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3654, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fLiqColConMode		= (float)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3647, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3648, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fReturnWaterTemp		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3649, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fOutletWaterPress	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951, 3650, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strYC_AirInfo.fReturnWaterPress	= valGet.val.fValue;
	}

	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  Refresh_YC_YX_Value()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Refresh_YC_YX_SigMapValue(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET 		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI 		err;
	int 				n = 0, m =0;

 	//---------------- Refresh YaoXin SigMap ------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 252, &valGet, sizeof(PARAM_VAL_GET));
 	pIEC104->strYX_SigMap.bMainAC_Breaker  = (BOOL)valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 308, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYX_SigMap.bBAT1_Contactor  = (BOOL)valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 309, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYX_SigMap.bBAT2_Contactor  = (BOOL)valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 310, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYX_SigMap.bBAT3_Contactor  = (BOOL)valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 311, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYX_SigMap.bBAT4_Contactor  = (BOOL)valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		600, 1663, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYX_SigMap.bLiqAirWorkMode  = (BOOL)valGet.val.emValue;

	//------------------- Refresh YC SysInfo SigValue -------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_SysInfo.emSysStatus = valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 6, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_SysInfo.emSysFailReason = valGet.val.emValue;

	pIEC104->strYC_SysInfo.nBatCfgNum = pIEC104->nCfgBMSNum;
	pIEC104->strYC_SysInfo.nCycle_Meter = pIEC104->blobBackendSet.nCmd_107_Interval;
	pIEC104->strYC_SysInfo.nCycle_BMS = pIEC104->blobBackendSet.nCmd_103_Interval;
	pIEC104->strYC_SysInfo.nCycle_PCS = pIEC104->blobBackendSet.nCmd_105_Interval;
	pIEC104->strYC_SysInfo.nCycle_Kwh = pIEC104->blobBackendSet.nCmd_204_Interval;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				200, 102, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strYC_SysInfo.nEmsPlanCRC = valGet.val.emValue;
	

 	Refresh_YX_CabMeter_SigMapValue(pIEC104);

 	Refresh_YX_FirstGateWayMeter_SigMapValue(pIEC104);

 	Refresh_YX_SecondGateWayMeter_SigMapValue(pIEC104);
	
	Refresh_YX_Aux3GateWayMeter_SigMapValue(pIEC104);

	Refresh_YC_PCS_SigMapValue(pIEC104);

	Refresh_YC_MpptGrp_SigMapValue(pIEC104);

	Refresh_YC_Air_SigMapValue(pIEC104);

	if (pIEC104->enableREMSCtrl)
	{
		Refresh_YC_LocalSys_SigMapValue(pIEC104);
	}

	TRACE("emCabMeter_CommStatu=[%d] emGridMeter_CommStatu=[%d] Aux=[%d] Aux3=[%d]\n", \
		pIEC104->strYC_SysInfo.emCabMeter_CommStatu,\
		pIEC104->strYC_SysInfo.emGridMeter_CommStatu,\
		pIEC104->strYC_SysInfo.emAuxMeter_CommStatu, \
		pIEC104->strYC_SysInfo.emAux3Meter_CommStatu);
 
	//-------------------------- Get Blob Data -------------------------//
 
	ZERO_POBJS(&pIEC104->esRunData, 1);
	pIEC104->esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&pIEC104->esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));

	//---------------------------------- End -----------------------------//

 	pIEC104->strYC_Ems.nEmsSectorNum = MIN(MAX_EMS_SECTOR, pIEC104->emsPlan.nTotal_EmsLmt_Sectors);
	
	int nRate = 1;
	UINT nInterval_Min = 0;
	for(n = 0; n < pIEC104->strYC_Ems.nEmsSectorNum; n++)
	{
		pIEC104->strYC_Ems.arryEmsParam[n].tmStart = nInterval_Min; 
		
		nInterval_Min = nInterval_Min + (pIEC104->emsPlan.emsLmtSectors[n].nInterval/60);

		//pIEC104->strYC_Ems.arryEmsParam[n].nInterval = pIEC104->emsPlan.emsLmtSectors[n].nInterval; 
		if(pIEC104->emsPlan.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
		{
			nRate = -1;
		}
		//pIEC104->strYC_Ems.arryEmsParam[n].nBattGrpPwrLmt = pIEC104->emsPlan.emsLmtSectors[n].emBatt_WorkMode; 
		pIEC104->strYC_Ems.arryEmsParam[n].nStopSOC = pIEC104->emsPlan.emsLmtSectors[n].nStopSOC; 
		pIEC104->strYC_Ems.arryEmsParam[n].nBattGrpPwrLmt = pIEC104->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt; 
		pIEC104->strYC_Ems.arryEmsParam[n].nActvPwrLmt = (int)pIEC104->emsPlan.emsLmtSectors[n].nActvPwrLmt*nRate; 
		pIEC104->strYC_Ems.arryEmsParam[n].nReActvPwrLmt = pIEC104->emsPlan.emsLmtSectors[n].nReActvPwrLmt; 

		/*TRACEX("Idx=[%d] nInterval_Min=[%d] tmStart=[%d] nStopSOC=[%d] nBattGrpPwrLmt=[%d] nActvPwrLmt=[%d] nReActvPwrLmt=[%d]\n", n, nInterval_Min,\
			pIEC104->strYC_Ems.arryEmsParam[n].tmStart, \
			pIEC104->strYC_Ems.arryEmsParam[n].nStopSOC,\
			pIEC104->strYC_Ems.arryEmsParam[n].nBattGrpPwrLmt,\
			pIEC104->strYC_Ems.arryEmsParam[n].nActvPwrLmt,\
			pIEC104->strYC_Ems.arryEmsParam[n].nReActvPwrLmt);*/
	}

	//-------------------Refresh Battery Group#1 SigValue -------------------//
	for(n = 0; n < pIEC104->nCfgBMSNum; n++)
	{
		ZERO_POBJS(&pIEC104->blobSiglBatt, 1);
		pIEC104->blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
		valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		valGet.val.blobValue = (BYTE*)&pIEC104->blobSiglBatt;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				501 + n, 551, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			ZERO_POBJS(&pIEC104->strYC_BAT[n], 1);
			continue;
		}

		if(pIEC104->blobSiglBatt.emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL)
		{
			ZERO_POBJS(&pIEC104->strYC_BAT[n], 1);
			continue;
		}

		pIEC104->strYC_BAT[n].fRack_Volt = pIEC104->blobSiglBatt.fPackTotalVolt;
		pIEC104->strYC_BAT[n].fRack_Curt = pIEC104->blobSiglBatt.fPackTotalCurr;
		if(pIEC104->blobSiglBatt.fPackTotalCurr < -1.0)
		{
			pIEC104->strYC_BAT[n].fRunState = STBATT_RUNSTAT_DISCHARGE;
		}
		else
		{
			pIEC104->strYC_BAT[n].fRunState = (float)pIEC104->blobSiglBatt.emBattStatus_Run;
		}
		pIEC104->strYC_BAT[n].f_SOC = pIEC104->blobSiglBatt.nSOC;
		pIEC104->strYC_BAT[n].f_SOH = pIEC104->blobSiglBatt.nSOH;
		pIEC104->strYC_BAT[n].fMax_AllowChgVolt = pIEC104->blobSiglBatt.fAllow_LmtChargeVolt;
		pIEC104->strYC_BAT[n].fMax_AllowChgCurt = pIEC104->blobSiglBatt.fAllow_MaxChargeCurr;
		pIEC104->strYC_BAT[n].fMax_AllowDisChgVolt = pIEC104->blobSiglBatt.fAllow_LmtDisChargeVolt;
		pIEC104->strYC_BAT[n].fMax_AllowDisChgCurt = pIEC104->blobSiglBatt.fAllow_MaxDisChargeCurr;
		pIEC104->strYC_BAT[n].fMax_CellTemp = pIEC104->blobSiglBatt.fMax_CellTemp;
		pIEC104->strYC_BAT[n].fMin_CellTemp = pIEC104->blobSiglBatt.fMin_CellTemp;
		pIEC104->strYC_BAT[n].fMax_CellVolt = pIEC104->blobSiglBatt.fMax_CellVolt;
		pIEC104->strYC_BAT[n].fMin_CellVolt = pIEC104->blobSiglBatt.fMin_CellVolt;
		pIEC104->strYC_BAT[n].fNotStartChgReason =  (float)pIEC104->blobSiglBatt.emReason_ChgStart;
		pIEC104->strYC_BAT[n].fNotStartDischgReason = (float)pIEC104->blobSiglBatt.emReason_DisChgON;

		//20230808 Added.
		pIEC104->strYC_BAT[n].fAlmState	= (float)pIEC104->blobSiglBatt.emBattStatus_Alm;	
		
		pIEC104->strYC_BAT[n].fUcell_Max_CSC = (float)pIEC104->blobSiglBatt.uiBAT_Ucell_Max_CSC;
		pIEC104->strYC_BAT[n].fUcell_Max_CELL = (float)pIEC104->blobSiglBatt.uiBAT_Ucell_Max_CELL;

		pIEC104->strYC_BAT[n].fUcell_Min_CSC = (float)pIEC104->blobSiglBatt.uiBAT_Ucell_Min_CSC;
		pIEC104->strYC_BAT[n].fUcell_Min_CELL = (float)pIEC104->blobSiglBatt.uiBAT_Ucell_Min_CELL;

		pIEC104->strYC_BAT[n].fT_Max_CSC = (float)pIEC104->blobSiglBatt.nBAT_T_Max_CSC;
		pIEC104->strYC_BAT[n].fT_Max_CELL = (float)pIEC104->blobSiglBatt.uiBAT_T_Max_CELL;

		pIEC104->strYC_BAT[n].fT_Min_CSC = (float)pIEC104->blobSiglBatt.nBAT_T_Min_CSC;
		pIEC104->strYC_BAT[n].fT_Min_CELL = (float)pIEC104->blobSiglBatt.uiBAT_T_Min_CELL;

		pIEC104->strYC_BAT[n].fInsulation_P_R = pIEC104->blobSiglBatt.fInsulation_P_R;
		pIEC104->strYC_BAT[n].fInsulation_N_R = pIEC104->blobSiglBatt.fInsulation_N_R;

		// YNT need cell temp and volt
		ZERO_POBJS(&pIEC104->blobIfyBMS, 1);
		pIEC104->blobIfyBMS.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
		valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
		valGet.val.blobValue = (BYTE*)&pIEC104->blobIfyBMS;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		501 + n, 557, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			ZERO_POBJS(&pIEC104->strYC_CellInfo[n], 1);
			continue;
		}
		
		for(m = 0; m < A_CABINET_CELL_MAX_NUM; m++)
		{
			pIEC104->strYC_CellInfo[n].fcellVolt[m] = (float)pIEC104->blobIfyBMS.usCellVolt[m]*0.001;
			pIEC104->strYC_CellInfo[n].fcellTemp[m] = pIEC104->blobIfyBMS.cCellTemp[m];
		}

	// YX_Bms_Part1
		pIEC104->strBmsPart1YX_SigMap[n].bBmsNotPowerOn 		= (pIEC104->blobSiglBatt.emPowerONStatu==0) ? 1 : 0;
		pIEC104->strBmsPart1YX_SigMap[n].bBmsHVPowerOnFail 		= (pIEC104->blobSiglBatt.emPowerONStatu==2) ? 1 : 0;
		pIEC104->strBmsPart1YX_SigMap[n].bBmsHVPowerOffFail 	= (pIEC104->blobSiglBatt.emPowerONStatu==3) ? 1 : 0;
		BATT_GRP_ALARM		unioBattGrpAlm = pIEC104->blobSiglBatt.unioBattGrpAlm;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_Ucell 		= unioBattGrpAlm.Alarm.AL_Over_Ucell;;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Under_Ucell 		= unioBattGrpAlm.Alarm.AL_Under_Ucell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_Tcell 		= unioBattGrpAlm.Alarm.AL_Over_Tcell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Under_Tcell 		= unioBattGrpAlm.Alarm.AL_Under_Tcell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_U_HVS 		= unioBattGrpAlm.Alarm.AL_Over_U_HVS;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Under_U_HVS 		= unioBattGrpAlm.Alarm.AL_Under_U_HVS;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_T_HVS 		= unioBattGrpAlm.Alarm.AL_Over_T_HVS;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Under_T_HVS 		= unioBattGrpAlm.Alarm.AL_Under_T_HVS;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_CC 			= unioBattGrpAlm.Alarm.AL_Over_CC;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_DC 			= unioBattGrpAlm.Alarm.AL_Over_DC;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Diff_Tcell 		= unioBattGrpAlm.Alarm.AL_Diff_Tcell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_PreChFailThree 	= unioBattGrpAlm.Alarm.AL_PreChFailThree;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_HSD_Coil 			= unioBattGrpAlm.Alarm.AL_Under_SOC;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Diff_Ucell 		= unioBattGrpAlm.Alarm.AL_HSD_Coil;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Bal_Circuit 	= unioBattGrpAlm.Alarm.AL_Diff_Ucell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Rly_OverLoad 		= unioBattGrpAlm.Alarm.AL_Err_Bal_Circuit;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Pre_Rly 			= unioBattGrpAlm.Alarm.RESV1;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Main_Rly_Pos 		= unioBattGrpAlm.Alarm.AL_Relay_OverLoad;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Main_Rly_Neg 		= unioBattGrpAlm.Alarm.AL_Pre_Relay;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Rly_Stick_Pos 		= unioBattGrpAlm.Alarm.AL_Main_Relay_Pos;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Rly_Stick_Neg 		= unioBattGrpAlm.Alarm.AL_Main_Relay_Neg;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Over_Cur 			= unioBattGrpAlm.Alarm.AL_Over_Cur;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Mea_Ucell 		= unioBattGrpAlm.Alarm.AL_Err_Mea_Ucell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Mea_Tcell 		= unioBattGrpAlm.Alarm.AL_Err_Mea_Tcell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Diff_Tvol 			= unioBattGrpAlm.Alarm.AL_Diff_Tvol;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Mea_I_HVS 		= unioBattGrpAlm.Alarm.AL_Err_Mea_I_HVS;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_MCANComFault 		= unioBattGrpAlm.Alarm.AL_MCANComFault;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_MCU 			= unioBattGrpAlm.Alarm.AL_Err_MCU;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Extre_Tvol 		= unioBattGrpAlm.Alarm.AL_Extre_Tvol;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_CCAN 			= unioBattGrpAlm.Alarm.AL_Err_CCAN;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_SCAN 			= unioBattGrpAlm.Alarm.AL_Err_SCAN;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Mea_AllTcell 	= unioBattGrpAlm.Alarm.AL_Err_Mea_AllTcell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Extre_T 			= unioBattGrpAlm.Alarm.AL_Extre_T;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Extre_Ucell 		= unioBattGrpAlm.Alarm.AL_Extre_Ucell;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_PowerSample_LEM 	= unioBattGrpAlm.Alarm.AL_PowerSample_LEM;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_PowerSample_CSC 	= unioBattGrpAlm.Alarm.AL_PowerSample_CSC;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_SBMU_PowerDown 	= unioBattGrpAlm.Alarm.AL_SBMU_PowerDown;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Fog 			= unioBattGrpAlm.Alarm.AL_Err_Fog;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_HvCrcuitBreak 	= unioBattGrpAlm.Alarm.AL_Err_HvCrcuitBreak;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_HvMsdFault 	= unioBattGrpAlm.Alarm.AL_Err_HvMsdFault;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Fan1Fault 		= unioBattGrpAlm.Alarm.AL_Err_Fan1Fault;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Fan2Fault 		= unioBattGrpAlm.Alarm.AL_Err_Fan2Fault;
		pIEC104->strBmsPart1YX_SigMap[n].bAL_Err_Rly_STB 		= unioBattGrpAlm.Alarm.AL_Err_Relay_STB;
		UINT nAlarmInfo2 = pIEC104->blobIfyBMS.nAlarmInfo2;
		UINT nAlarmInfo3 = pIEC104->blobIfyBMS.nAlarmInfo3;
		UINT nAlarmInfo4 = pIEC104->blobIfyBMS.nAlarmInfo4;
		UINT nAlarmInfo5 = pIEC104->blobIfyBMS.nAlarmInfo5;
		UINT nAlarmInfo6 = pIEC104->blobIfyBMS.nAlarmInfo6;
		UINT nAlarmInfo7 = pIEC104->blobIfyBMS.nAlarmInfo7;
		pIEC104->strBmsPart1YX_SigMap[n].bCellV_HV1 						= GET_BIT(nAlarmInfo2, 0);
		pIEC104->strBmsPart1YX_SigMap[n].bCellV_LV1 						= GET_BIT(nAlarmInfo2, 1);
		pIEC104->strBmsPart1YX_SigMap[n].bSumV_HV1 							= GET_BIT(nAlarmInfo2, 2);
		pIEC104->strBmsPart1YX_SigMap[n].bSumV_LV1 							= GET_BIT(nAlarmInfo2, 3);
		pIEC104->strBmsPart1YX_SigMap[n].bChg_HT1 							= GET_BIT(nAlarmInfo2, 4);
		pIEC104->strBmsPart1YX_SigMap[n].bChg_LT1 							= GET_BIT(nAlarmInfo2, 5);
		pIEC104->strBmsPart1YX_SigMap[n].bDischg_HT1 						= GET_BIT(nAlarmInfo2, 6);
		pIEC104->strBmsPart1YX_SigMap[n].bDischg_LT1 						= GET_BIT(nAlarmInfo2, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bChg_OC1 							= GET_BIT(nAlarmInfo2, 8);
		pIEC104->strBmsPart1YX_SigMap[n].bDischg_OC1 						= GET_BIT(nAlarmInfo2, 9);
		pIEC104->strBmsPart1YX_SigMap[n].bExtT_HT1 							= GET_BIT(nAlarmInfo2, 10);
		pIEC104->strBmsPart1YX_SigMap[n].bBattery_insul1 					= GET_BIT(nAlarmInfo2, 11);
		pIEC104->strBmsPart1YX_SigMap[n].bCellV_diff1 						= GET_BIT(nAlarmInfo2, 12);
		pIEC104->strBmsPart1YX_SigMap[n].bCellT_diff1 						= GET_BIT(nAlarmInfo2, 13);
		pIEC104->strBmsPart1YX_SigMap[n].bBattBox_fan_recheck 				= GET_BIT(nAlarmInfo3, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bBattCab_ShuttFan_or_AirCondFail	= GET_BIT(nAlarmInfo3, 8);
		pIEC104->strBmsPart1YX_SigMap[n].bBattCab_Aux_Dev_CommFail 			= GET_BIT(nAlarmInfo3, 9);
		pIEC104->strBmsPart1YX_SigMap[n].bBattCube_SPD 						= GET_BIT(nAlarmInfo3, 10);
		pIEC104->strBmsPart1YX_SigMap[n].bBatt_Cube_Tip 					= GET_BIT(nAlarmInfo3, 11);
		pIEC104->strBmsPart1YX_SigMap[n].bPower_IMEU3_CommFail 				= GET_BIT(nAlarmInfo3, 12);
		pIEC104->strBmsPart1YX_SigMap[n].bCab_IMEU3_CommFail 				= GET_BIT(nAlarmInfo3, 13);
		pIEC104->strBmsPart1YX_SigMap[n].bLeakCurrCommFail 					= GET_BIT(nAlarmInfo3, 14);
		pIEC104->strBmsPart1YX_SigMap[n].bAir_Cond_CommFail 				= GET_BIT(nAlarmInfo4, 0);
		pIEC104->strBmsPart1YX_SigMap[n].bDehumid_CommFail 					= GET_BIT(nAlarmInfo4, 2);
		pIEC104->strBmsPart1YX_SigMap[n].bGasSensor_CommFail 				= GET_BIT(nAlarmInfo4, 3);
		pIEC104->strBmsPart1YX_SigMap[n].bPDU_Flow_Fan_Recheck 				= GET_BIT(nAlarmInfo4, 4);
		pIEC104->strBmsPart1YX_SigMap[n].bPCS_Fan_Recheck 					= GET_BIT(nAlarmInfo4, 5);
		pIEC104->strBmsPart1YX_SigMap[n].bExhaust_Fan_Recheck 				= GET_BIT(nAlarmInfo4, 6);
		pIEC104->strBmsPart1YX_SigMap[n].bExplos_Recheck 					= GET_BIT(nAlarmInfo4, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bLeakCurrAbnormal 					= GET_BIT(nAlarmInfo4, 8);
		pIEC104->strBmsPart1YX_SigMap[n].bFireAlm 							= GET_BIT(nAlarmInfo4, 9);
		pIEC104->strBmsPart1YX_SigMap[n].bFire_Lv1_Alm 						= GET_BIT(nAlarmInfo4, 10);
		pIEC104->strBmsPart1YX_SigMap[n].bFire_Lv2_Alm 						= GET_BIT(nAlarmInfo4, 11);
		pIEC104->strBmsPart1YX_SigMap[n].bFire_Lv3_Alm 						= GET_BIT(nAlarmInfo4, 12);
		pIEC104->strBmsPart1YX_SigMap[n].bPDU_High_EnvirTemp_Lv1_Alm 		= GET_BIT(nAlarmInfo4, 14);
		pIEC104->strBmsPart1YX_SigMap[n].bPDU_High_CableTemp_Lv1_Alm 		= GET_BIT(nAlarmInfo5, 0);
		pIEC104->strBmsPart1YX_SigMap[n].bPCS_High_OutletTemp_Lv1_Alm 		= GET_BIT(nAlarmInfo5, 2);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_Water_Alm 					= GET_BIT(nAlarmInfo5, 6);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_MSD_Alm 					= GET_BIT(nAlarmInfo5, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_Pos_HT_Alm 					= GET_BIT(nAlarmInfo5, 8);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_Neg_HT_Alm 					= GET_BIT(nAlarmInfo5, 9);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_Aerosol_Alm 				= GET_BIT(nAlarmInfo5, 10);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_CO_Alm 						= GET_BIT(nAlarmInfo5, 11);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK1_H2_Alm 						= GET_BIT(nAlarmInfo5, 12);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_Water_Alm 					= GET_BIT(nAlarmInfo5, 13);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_MSD_Alm 					= GET_BIT(nAlarmInfo5, 14);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_Pos_HT_Alm 					= GET_BIT(nAlarmInfo5, 15);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_Neg_HT_Alm 					= GET_BIT(nAlarmInfo6, 0);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_Aerosol_Alm 				= GET_BIT(nAlarmInfo6, 1);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_CO_Alm 						= GET_BIT(nAlarmInfo6, 2);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK2_H2_Alm 						= GET_BIT(nAlarmInfo6, 3);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_Water_Alm 					= GET_BIT(nAlarmInfo6, 4);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_MSD_Alm 					= GET_BIT(nAlarmInfo6, 5);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_Pos_HT_Alm					= GET_BIT(nAlarmInfo6, 6);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_Neg_HT_Alm					= GET_BIT(nAlarmInfo6, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_Aerosol_Alm 				= GET_BIT(nAlarmInfo6, 8);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_CO_Alm 						= GET_BIT(nAlarmInfo6, 9);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK3_H2_Alm 						= GET_BIT(nAlarmInfo6, 10);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_Water_Alm 					= GET_BIT(nAlarmInfo6, 11);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_MSD_Alm 					= GET_BIT(nAlarmInfo6, 12);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_Pos_HT_Alm 					= GET_BIT(nAlarmInfo6, 13);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_Neg_HT_Alm 					= GET_BIT(nAlarmInfo6, 14);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_Aerosol_Alm 				= GET_BIT(nAlarmInfo6, 15);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_CO_Alm 						= GET_BIT(nAlarmInfo7, 0);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK4_H2_Alm 						= GET_BIT(nAlarmInfo7, 1);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_Water_Alm 					= GET_BIT(nAlarmInfo7, 2);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_MSD_Alm 					= GET_BIT(nAlarmInfo7, 3);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_Pos_HT_Alm 					= GET_BIT(nAlarmInfo7, 4);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_Neg_HT_Alm 					= GET_BIT(nAlarmInfo7, 5);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_Aerosol_Alm 				= GET_BIT(nAlarmInfo7, 6);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_CO_Alm 						= GET_BIT(nAlarmInfo7, 7);
		pIEC104->strBmsPart1YX_SigMap[n].bPACK5_H2_Alm 						= GET_BIT(nAlarmInfo7, 8);
		
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501 + n, 684, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strBmsPart1YX_SigMap[n].bSlv_Temp_Alm 						= (BOOL)valGet.val.emValue;
		IFY_BATT_FAULT_CODE 	unioBattFault = pIEC104->blobIfyBMS.unioBattFault;
		BOOL bRes;
		bRes = (unioBattFault.Falut.SlaveBoardBackCheckFailed
			 || unioBattFault.Falut.SlaveBalanceCtrlFailed
			 || unioBattFault.Falut.SlaveReserveFaultBit1
			 || unioBattFault.Falut.SlaveReserveFaultBit2
			 || unioBattFault.Falut.SlaveSampleCircuitAbnor
			 || unioBattFault.Falut.SlaveVoltSampleDisconnect
			 || unioBattFault.Falut.SlaveTempSampleDisconnect
			 || unioBattFault.Falut.SlaveCellInfoMiss);
		pIEC104->strBmsPart1YX_SigMap[n].bSlv_Inner_Fault 					= bRes;
		bRes = (unioBattFault.Falut.SlaveCascadeAbnor
			 || unioBattFault.Falut.SlaveCommFailed
			 || unioBattFault.Falut.PeripheralsCommFailed
			 || unioBattFault.Falut.MasterCommFailed);
		pIEC104->strBmsPart1YX_SigMap[n].bMas_Slv_CommFail 					= bRes;
		bRes = (unioBattFault.Falut.MasterDevAddrConflict
			 || unioBattFault.Falut.MasterMainMCUAbnor
			 || unioBattFault.Falut.MasterSoftwareEPOCtrl
			 || unioBattFault.Falut.MasterInterAutoProtect);
		pIEC104->strBmsPart1YX_SigMap[n].bMas_Software_Alm 					= bRes;
		bRes = (unioBattFault.Falut.MasterRTCAbnor
			 || unioBattFault.Falut.MasterEEPROMAbnor
			 || unioBattFault.Falut.MasterFLASHAbnor
			 || unioBattFault.Falut.MasterCurrSampCircuitAbnor
			 || unioBattFault.Falut.MasterHighVoltSampCircAbnor
			 || unioBattFault.Falut.MasterLowVoltSampCircAbnor
			 || unioBattFault.Falut.MasterExternTempSampCircAbnor
			 || unioBattFault.Falut.MasterInsuCheckCircAbnor
			 || unioBattFault.Falut.MasterKM1BackCheckAbnor
			 || unioBattFault.Falut.MasterKM2BackCheckAbnor
			 || unioBattFault.Falut.MasterKM3BackCheckAbnor
			 || unioBattFault.Falut.MasterKM4BackCheckAbnor
			 || unioBattFault.Falut.MasterKM5To8BackCheckAbnor
			 || unioBattFault.Falut.MasterKM9BackCheckAbnor
			 || unioBattFault.Falut.MasterLS1BackCheckAbnor
			 || unioBattFault.Falut.MasterLS3BackCheckAbnor);
		pIEC104->strBmsPart1YX_SigMap[n].bMas_Inner_Fault 					= bRes;
		bRes = (unioBattFault.Falut.BattMainPosContactorAbnor
			 || unioBattFault.Falut.BattPrechgContactorAbnor
			 || unioBattFault.Falut.BattNegContactorAbnor);
		pIEC104->strBmsPart1YX_SigMap[n].bHV_Contactor 						= bRes;
		bRes = (unioBattFault.Falut.BattFloodAlarm
			 || unioBattFault.Falut.BattFuseMelt
			 || unioBattFault.Falut.BattSmokeAlarm);
		pIEC104->strBmsPart1YX_SigMap[n].bPeriph_backInspec 				= bRes;
		bRes = (unioBattFault.Falut.BattPrechargeNotFull
			 || unioBattFault.Falut.BattPrechargeOverTemp
			 || unioBattFault.Falut.BattStartupFault);
		pIEC104->strBmsPart1YX_SigMap[n].bHVPowOnFail 						= bRes;
		bRes = (unioBattFault.Falut.BattWithLoadTimeout
			 || unioBattFault.Falut.BattStopFault);
		pIEC104->strBmsPart1YX_SigMap[n].bHVPowOffFail 						= bRes;
		
		// YX_Bms_Part2
		bRes = (unioBattFault.Falut.BattPressEPOStop
			 || unioBattFault.Falut.BattBreakerNotClose
			 || unioBattFault.Falut.BattInterExternVoltDiffBig);
		pIEC104->strBmsPart2YX_SigMap[n].bPeriph_Stat_NotReady	 			= bRes;
		bRes = (unioBattFault.Falut.ContactorWithLoadClose
			 || unioBattFault.Falut.ContactorWithLoadOpen);
		pIEC104->strBmsPart2YX_SigMap[n].bGrpContactor_Change_OnLoad	 	= bRes;
		pIEC104->strBmsPart2YX_SigMap[n].bSOS_Alm_Occur	 					= unioBattFault.Falut.SOSFaultAlarmCode;
		pIEC104->strBmsPart2YX_SigMap[n].bUcell_Over	 					= unioBattFault.Falut.CellVoltReachesExtreme;
		pIEC104->strBmsPart2YX_SigMap[n].bUcell_diff	 					= unioBattFault.Falut.CellVoltDiffLarge;
		pIEC104->strBmsPart2YX_SigMap[n].bUcell_Drop	 					= unioBattFault.Falut.CellVoltDropLarge;
		pIEC104->strBmsPart2YX_SigMap[n].bTcell_Over	 					= unioBattFault.Falut.CellTempReachesExtreme;
		pIEC104->strBmsPart2YX_SigMap[n].bTcell_diff	 					= unioBattFault.Falut.CellTempDiffLarge;
		pIEC104->strBmsPart2YX_SigMap[n].bTcell_Rise	 					= unioBattFault.Falut.CellTempRiseFast;
		pIEC104->strBmsPart2YX_SigMap[n].bEqualChip_OT	 					= unioBattFault.Falut.BalancePlateTempReachExtreme;
		pIEC104->strBmsPart2YX_SigMap[n].bPowerChip_OT	 					= unioBattFault.Falut.PowerBoardTempReachExtreme;
		pIEC104->strBmsPart2YX_SigMap[n].bInstant_OC	 					= unioBattFault.Falut.CurrInstantReachExtreme;
		pIEC104->strBmsPart2YX_SigMap[n].bStandbyCurrAlm	 				= unioBattFault.Falut.StandbyCurrAbnormal;
		pIEC104->strBmsPart2YX_SigMap[n].bTotalCapacityDecay	 			= unioBattFault.Falut.TotalBattCapDecay;
		pIEC104->strBmsPart2YX_SigMap[n].bCellChgDischgProtFail	 			= unioBattFault.Falut.SameCellChrgAndDischrgProt;
		pIEC104->strBmsPart2YX_SigMap[n].bCellColFail	 					= unioBattFault.Falut.HeatDissiptFailofSameCell;
		pIEC104->strBmsPart2YX_SigMap[n].bLongDODLow	 					= unioBattFault.Falut.DODnotMetStd4LongTime;
		pIEC104->strBmsPart2YX_SigMap[n].bChgDisChgEffLow	 				= unioBattFault.Falut.ChrgDischrgEffinotUpStd;
		// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		// 	501 + n, 774, &valGet, sizeof(PARAM_VAL_GET));
		// pIEC104->strBmsPart2YX_SigMap[n].bDI_1_AuxPower	 					= (BOOL)valGet.val.emValue;

	}

	//---------YX_Pcs_Total-------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 100, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bCcu_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 250, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bAC_OV	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 251, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bAC_UV	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 252, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bACGrid_Input_MCB_QF1	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 253, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bACLoad_Output_MCB_QF2	 			= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 254, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bAC_Contactor	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 257, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bABU1_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 258, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bABU2_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 259, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bABU3_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 260, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bABU4_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 264, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bLV_MCBBBreaker	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 265, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bN_LineContactor	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 272, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula1_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 273, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula1_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 274, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula2_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 275, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula2_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 276, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula3_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 277, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula3_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 278, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula4_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 279, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula4_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 280, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula5_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 281, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula5_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 282, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula6_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 283, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula6_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 284, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula7_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 285, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula7_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 286, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula8_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 287, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula8_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 288, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula9_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 289, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula9_DeteAlm 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 290, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula10_CommFai	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 291, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bInsula10_DeteAlm	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 304, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp1_ForbidRun	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 305, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp2_ForbidRun	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 306, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp3_ForbidRun	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 307, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp4_ForbidRun	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 308, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp1_InputContact	 			= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 309, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp2_InputContact	 			= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 310, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp3_InputContact	 			= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 311, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp4_InputContact	 			= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 312, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp1_IsoSwitch	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 313, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp2_IsoSwitch	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 314, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp3_IsoSwitch	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 315, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBattGrp4_IsoSwitch	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 316, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBms1Can_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 317, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBms2Can_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 318, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBms3Can_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 319, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bBms4Can_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 368, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bAllPMGrp_Fail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 369, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bAllPMGrp_CommFail	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 370, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM1_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 371, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM1_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 372, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM2_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 373, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM2_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 374, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM3_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 375, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM3_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 376, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM4_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 377, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM4_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 378, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM5_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 379, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM5_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 380, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM6_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 381, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM6_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 382, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM7_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 383, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM7_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 384, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM8_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 385, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM8_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 386, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM9_Fail	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 387, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM9_CommFail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 388, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM10_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 389, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM10_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 390, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM11_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 391, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM11_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 392, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM12_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 393, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPM12_CommFail	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 394, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPCSGrp1_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 395, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPCSGrp2_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 396, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPCSGrp3_Fail	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 397, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strPcsTotalYX_SigMap.bPCSGrp4_Fail	 						= (BOOL)valGet.val.emValue;

	//---------YX_PcsGrp_Total-------------//
	A_BLOB_SINGLE_RECT_INFO sigRect;
	ZERO_POBJS(&sigRect, 	1);
	sigRect.emBlobType = BLOB_TYPE_SINGLE_RECT_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_RECT_INFO);
	valGet.val.blobValue = (BYTE*)&sigRect;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 101, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q106 Failed err=[%d]!\n", err);
		return ;
	}

	BYTE byPCSFaultcode_1110[4];
	BYTE byPCSFaultcode_1111[4];
	PCS_STATU_CODE mapPCSCode;
	UINT uiPCSFaultCode_1113;
	UINT uiPCSFaultCode_1114;
	UINT uiPCSFaultCode_1115;
	
	int i;
	for(i=0; i<4; ++i)
	{
		byPCSFaultcode_1110[i] = sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].byPCSFaultcode_1110[i];
		byPCSFaultcode_1111[i] = sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].byPCSFaultcode_1111[i];
	}
	mapPCSCode 			= sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].mapPCSCode;
	uiPCSFaultCode_1113 = sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].uiPCSFaultCode_1113;
	uiPCSFaultCode_1114 = sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].uiPCSFaultCode_1114;
	uiPCSFaultCode_1115 = sigRect.arrRect[TYPE_SIG_RECT_ACDC_GRP1][n].uiPCSFaultCode_1115;

	for(n=0; n<sigRect.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; n+=2)
	{
		pIEC104->strPcsGrpYX_SigMap[n].bDischgErr	 					= (byPCSFaultcode_1110[0] >> 5) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBusFail	 						= (byPCSFaultcode_1110[0] >> 3) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bSciFail	 						= (byPCSFaultcode_1110[0] >> 2) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bUVdc	 						= (byPCSFaultcode_1110[0] >> 1) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bShort	 						= (byPCSFaultcode_1110[0] >> 0) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bMCCFail	 						= (byPCSFaultcode_1110[1] >> 7) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOvo	 							= (byPCSFaultcode_1110[1] >> 5) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOT	 							= (byPCSFaultcode_1110[1] >> 4) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bFault	 						= (byPCSFaultcode_1110[1] >> 1) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bOVin	 						= (byPCSFaultcode_1110[2] >> 5) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bUVin	 						= (byPCSFaultcode_1110[2] >> 4) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInUnBal	 						= (byPCSFaultcode_1110[2] >> 3) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInLessPhase	 					= (byPCSFaultcode_1110[2] >> 2) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOutUnBal	 					= (byPCSFaultcode_1110[2] >> 1) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bIDDup	 						= (byPCSFaultcode_1110[2] >> 0) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bGridFault	 					= (byPCSFaultcode_1111[0] >> 7) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPllFault	 					= (byPCSFaultcode_1111[0] >> 5) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAcFail	 						= (byPCSFaultcode_1111[0] >> 4) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBusErr	 						= (byPCSFaultcode_1111[0] >> 3) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bVinOff	 						= (byPCSFaultcode_1111[0] >> 2) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrpUVdc	 						= (byPCSFaultcode_1111[0] >> 1) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bVGridFault	 					= (byPCSFaultcode_1111[1] >> 7) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bFGridFault	 					= (byPCSFaultcode_1111[1] >> 6) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bACLessPhase	 					= (byPCSFaultcode_1111[1] >> 5) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhaseErr	 					= (byPCSFaultcode_1111[1] >> 4) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDCTZ	 						= (byPCSFaultcode_1111[1] >> 3) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOIin	 						= (byPCSFaultcode_1111[1] >> 2) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bACTZ	 						= (byPCSFaultcode_1111[1] >> 1) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOVBUS	 						= (byPCSFaultcode_1111[1] >> 0) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bOP	 							= (byPCSFaultcode_1111[2] >> 1) & 0x01;
		
		pIEC104->strPcsGrpYX_SigMap[n].bGridVolt_OverBus	 			= mapPCSCode.PCSStatu.GridVolt_OverBus;
		pIEC104->strPcsGrpYX_SigMap[n].bHighOrLowFault	 				= mapPCSCode.PCSStatu.HighOrLowFault;
		pIEC104->strPcsGrpYX_SigMap[n].bBAT_EOD	 						= mapPCSCode.PCSStatu.BAT_EOD;
		pIEC104->strPcsGrpYX_SigMap[n].bRly_Fault	 					= mapPCSCode.PCSStatu.Relay_Fault;
		pIEC104->strPcsGrpYX_SigMap[n].bSoftStart_Fault	 				= mapPCSCode.PCSStatu.SoftStartFault;
		pIEC104->strPcsGrpYX_SigMap[n].bAuxPower_Fault	 				= mapPCSCode.PCSStatu.AuxPower_Fault;
		pIEC104->strPcsGrpYX_SigMap[n].bEPO_Fault	 					= mapPCSCode.PCSStatu.EPO_Fault;
		pIEC104->strPcsGrpYX_SigMap[n].bDuplicate_ID	 				= mapPCSCode.PCSStatu.Duplicate_ID;
		pIEC104->strPcsGrpYX_SigMap[n].bUnEven_Curr	 					= mapPCSCode.PCSStatu.UnEven_Currt;
		pIEC104->strPcsGrpYX_SigMap[n].bFan_Fail	 					= mapPCSCode.PCSStatu.Fan_Fail;
		pIEC104->strPcsGrpYX_SigMap[n].bOTemp	 						= mapPCSCode.PCSStatu.Over_Temp;
		pIEC104->strPcsGrpYX_SigMap[n].bPhase_Lock	 					= mapPCSCode.PCSStatu.Phase_Lock;
		pIEC104->strPcsGrpYX_SigMap[n].bIsland_Prot	 					= mapPCSCode.PCSStatu.Island_Prot;
		pIEC104->strPcsGrpYX_SigMap[n].bCommFail_CanSciSpi	 			= mapPCSCode.PCSStatu.CommFail;
		pIEC104->strPcsGrpYX_SigMap[n].bOutput_Derating	 				= mapPCSCode.PCSStatu.OutputDerating;
		pIEC104->strPcsGrpYX_SigMap[n].bOutput_OL	 					= mapPCSCode.PCSStatu.OutputOverLoad;
		pIEC104->strPcsGrpYX_SigMap[n].bOutput_Short	 				= mapPCSCode.PCSStatu.OutputShortCircuit;
		pIEC104->strPcsGrpYX_SigMap[n].bInter_OC	 					= mapPCSCode.PCSStatu.Inter_OverCurrt;
		pIEC104->strPcsGrpYX_SigMap[n].bBus_Fault	 					= mapPCSCode.PCSStatu.BusFault;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_Fault	 					= mapPCSCode.PCSStatu.DC_Fault;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_Fault	 					= mapPCSCode.PCSStatu.AC_Fault;
		pIEC104->strPcsGrpYX_SigMap[n].bSys_Fail	 					= mapPCSCode.PCSStatu.System_Fail;

		pIEC104->strPcsGrpYX_SigMap[n].bGrid_OV	 						= (uiPCSFaultCode_1113 >> 0 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_UV	 						= (uiPCSFaultCode_1113 >> 1 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_OF	 						= (uiPCSFaultCode_1113 >> 2 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_UF	 						= (uiPCSFaultCode_1113 >> 3 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhase_Loss	 					= (uiPCSFaultCode_1113 >> 4 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRevGrid_lackN	 				= (uiPCSFaultCode_1113 >> 5 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_ReverPhase	 				= (uiPCSFaultCode_1113 >> 6 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_PowerOff	 				= (uiPCSFaultCode_1113 >> 7 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRevGrid_Fast_OV1	 			= (uiPCSFaultCode_1113 >> 8 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRevGrid_Fast_OV2	 			= (uiPCSFaultCode_1113 >> 9 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRevGrid_Unbal	 				= (uiPCSFaultCode_1113 >> 10) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhase_Lock_Fault	 			= (uiPCSFaultCode_1113 >> 11) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bHVRT_Fault	 					= (uiPCSFaultCode_1113 >> 13) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bLVRT_Fault	 					= (uiPCSFaultCode_1113 >> 14) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bIslandProt	 					= (uiPCSFaultCode_1113 >> 15) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_FastUnbal	 			= (uiPCSFaultCode_1113 >> 16) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_SlowUnbal	 			= (uiPCSFaultCode_1113 >> 17) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_FastOV	 				= (uiPCSFaultCode_1113 >> 18) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_SlowOV	 				= (uiPCSFaultCode_1113 >> 19) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_FastUV	 				= (uiPCSFaultCode_1113 >> 20) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInterBus_SlowUV	 				= (uiPCSFaultCode_1113 >> 21) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRevBus_Short	 				= (uiPCSFaultCode_1113 >> 22) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_High_DC	 					= (uiPCSFaultCode_1113 >> 23) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_OV	 						= (uiPCSFaultCode_1113 >> 24) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_UV	 						= (uiPCSFaultCode_1113 >> 25) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRev_DC_Reversed	 				= (uiPCSFaultCode_1113 >> 26) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_OC	 						= (uiPCSFaultCode_1113 >> 27) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_ChgOV	 					= (uiPCSFaultCode_1113 >> 28) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_EOD_Shutdown	 				= (uiPCSFaultCode_1113 >> 30) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bAC_FastOC	 					= (uiPCSFaultCode_1114 >> 0 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bOutput_OLoad	 				= (uiPCSFaultCode_1114 >> 1 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhA_PBPFault	 				= (uiPCSFaultCode_1114 >> 3 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhB_PBPFault	 				= (uiPCSFaultCode_1114 >> 4 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bPhC_PBPFault	 				= (uiPCSFaultCode_1114 >> 5 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_CurrUnbal	 				= (uiPCSFaultCode_1114 >> 6 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bEff_Fail	 					= (uiPCSFaultCode_1114 >> 7 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBus_SoftStart_Fault	 			= (uiPCSFaultCode_1114 >> 8 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInv_SoftStart_Fault	 			= (uiPCSFaultCode_1114 >> 9 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_RlyOpen	 				= (uiPCSFaultCode_1114 >> 10) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bGrid_RlyShort	 				= (uiPCSFaultCode_1114 >> 11) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInv_RlyOpen	 					= (uiPCSFaultCode_1114 >> 12) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInv_RlyShort	 				= (uiPCSFaultCode_1114 >> 13) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_RlyFault	 					= (uiPCSFaultCode_1114 >> 14) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_Side_FaultSum	 			= (uiPCSFaultCode_1114 >> 15) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_Curr_SampFail	 			= (uiPCSFaultCode_1114 >> 16) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bInductor_Curr_SampFail	 		= (uiPCSFaultCode_1114 >> 17) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_Curr_SampFail	 			= (uiPCSFaultCode_1114 >> 18) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_SoftStart_OL	 				= (uiPCSFaultCode_1114 >> 21) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_SoftStart_RlyFail	 		= (uiPCSFaultCode_1114 >> 22) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bParallel_CommFail	 			= (uiPCSFaultCode_1114 >> 23) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_ShortCircuit	 				= (uiPCSFaultCode_1114 >> 24) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDSP_FaultRev	 				= (uiPCSFaultCode_1114 >> 25) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bSTS_FaultRev	 				= (uiPCSFaultCode_1114 >> 26) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bCAN_CommFailRev	 				= (uiPCSFaultCode_1114 >> 27) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bEEPROM_faultRev	 				= (uiPCSFaultCode_1114 >> 28) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bEPO_Fail	 					= (uiPCSFaultCode_1114 >> 29) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bCarrier_SyncFail	 			= (uiPCSFaultCode_1114 >> 30) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAC_SyncFail	 					= (uiPCSFaultCode_1114 >> 31) & 0x01;

		pIEC104->strPcsGrpYX_SigMap[n].bCAN_CommFail	 				= (uiPCSFaultCode_1115 >> 0 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bSCI_CommFail	 				= (uiPCSFaultCode_1115 >> 1 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bSPI_CommFail	 				= (uiPCSFaultCode_1115 >> 2 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBalBridge_OC	 				= (uiPCSFaultCode_1115 >> 3 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBus_OV	 						= (uiPCSFaultCode_1115 >> 4 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bBus_UV	 						= (uiPCSFaultCode_1115 >> 5 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bRadiator_OT	 					= (uiPCSFaultCode_1115 >> 6 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAmbient_OT	 					= (uiPCSFaultCode_1115 >> 7 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bAux_PowerFault	 				= (uiPCSFaultCode_1115 >> 8 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDSP_PWM_Fault	 				= (uiPCSFaultCode_1115 >> 9 ) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bCPLD_PWM_Fault	 				= (uiPCSFaultCode_1115 >> 10) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bFan_Fault	 					= (uiPCSFaultCode_1115 >> 11) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bVo_Set_OverProtPoint	 		= (uiPCSFaultCode_1115 >> 12) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bVo_Setting_Low	 				= (uiPCSFaultCode_1115 >> 13) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDC_Short_Circuit	 			= (uiPCSFaultCode_1115 >> 14) & 0x01;
		pIEC104->strPcsGrpYX_SigMap[n].bDuplicated_ID	 				= (uiPCSFaultCode_1115 >> 15) & 0x01;

	}

	//---------YX_STS_Total-------------//
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	// printf("ESSDeviceType == %d", valGet.val.emValue);
	if(valGet.val.emValue == LIQUID_COOLING_ADD_STS)
	{
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1329, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridOVP	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1330, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridUVP	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1331, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridOFP	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1332, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridUFP	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1333, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridFreqLose	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1334, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridLosePhase	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1335, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridPhaseSeqInv	 				= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1336, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridLoseN	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1337, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridUnbal	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1338, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_VGridLineFault	 				= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1339, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_PllFail	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1340, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_VgridCrossZeroLose	 			= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1341, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridDQFastFault	 				= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1342, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridFreErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1343, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridFreChg	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1344, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OutputOL	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1345, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridOCP	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1346, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_IacUnbal	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1348, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OVRT	 							= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1349, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_LVRT	 							= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1350, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_IslandFault	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1351, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_RefluxFault	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1352, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OutputShort	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1353, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_VoutCrossZeroLose	 				= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1355, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bGri_UaSampErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1356, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bGri_UbSampErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1357, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bGri_UcSampErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1361, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_RlyErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1363, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OnGridStartFault	 				= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1377, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_CanErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1378, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_SpiErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1380, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_E2PROMErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1381, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OTSink	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1382, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OTAmb	 							= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1383, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OTCabinet	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1384, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_FanERR	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1385, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_GridFault	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1386, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_SysFault_1114	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1387, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_SCRShort	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1388, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_SCROpen	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1393, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_U2CanErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1394, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_U2SpiErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1396, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_U2E2PROMErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1401, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_EPO	 							= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1402, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_OTempTrsfm	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1403, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_AuxPowerErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1404, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_InvContctErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1405, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_ByContctErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1406, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_FanErr	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1407, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_IoReadyErr	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1408, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_U2SysFault	 					= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1409, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_Fan1Err	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1410, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_Fan2Err	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1411, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_Fan3Err	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1412, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_Fan4Err	 						= (BOOL)valGet.val.emValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 201, 1412, &valGet, sizeof(PARAM_VAL_GET));
		pIEC104->strSTSYX_SigMap.bSTS_N_RlyErr	 						= (BOOL)valGet.val.emValue;

	}


	//---------YX_Fire_Total-------------//
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 261, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bPowerCab_Smoke_Sensor	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 433, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bCombGas_Sensor1_CommFail	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 602, 433, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bCombGas_Sensor2_CommFail	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 603, 433, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bCombGas_Sensor3_CommFail	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 604, 433, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bCombGas_Sensor4_CommFail	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 452, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bFireTube_Alm	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 453, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bSmoke_Alm	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 455, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bFlamGas_Alm	 							= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 459, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bElecShut_FullCls	 						= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 460, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bElecShut_FullOpen	 					= (BOOL)valGet.val.emValue;
	pIEC104->strFireYX_SigMap.bEnterProt_Lv1	 						= 0;// not use yet
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 462, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bEnterProt_Lv2	 						= (valGet.val.emValue==1) ? 1 : 0;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601, 462, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bEnterProt_Lv3	 						= (valGet.val.emValue==2) ? 1 : 0;
	pIEC104->strFireYX_SigMap.bEnterProt_Lv4	 						= 0;// not use yet
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 685, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bSlvAerosol_DensityAlm	 				= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 707, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bCombGas_concentraAlm	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 739, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bBatGrp_FirepProtAlm	 					= (BOOL)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 741, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->strFireYX_SigMap.bGrp_SmokeAlm	 							= (BOOL)valGet.val.emValue;

	return;
}


//------------------------- KWH ----------------------------------//
void Refresh_YC_KWh_DataMap(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET valGet;

	static UINT nLast_Day = 0;

	time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    UINT nCurr_Day = gmTime.tm_mday - 1;
 	if((gmTime.tm_hour == 0) && (gmTime.tm_min < 10)) //Before 00:10 Send Last Day Value
	{
		nCurr_Day = nLast_Day;
	}

	if(nCurr_Day != nLast_Day)
	{
		nLast_Day = nCurr_Day;
	}

	if(nCurr_Day > MAX_DAYS_SECTOR)
	{
		TRACE("nCurr_Day=[%d] is Outof Range,GoBack!\n", nCurr_Day);
		return ;
	}

 	ZERO_POBJS(&pIEC104->blobStatData ,	1);
	pIEC104->blobStatData.emBlobType = BLOB_TYPE_ES_STATISTICDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
	valGet.val.blobValue = (BYTE*)&(pIEC104->blobStatData);
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 79, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q204 Failed err=[%d]!\n", err);
		return ;
	}

	if((pIEC104->blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day] == 0))
	{
		TRACE("nCurr_Day=[%d] Data Is Not Saved Yet,Go Back!\n");
		return ;
	}

	pIEC104->strYC_KWh.fDate = 
		pIEC104->blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_PosDailyKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_NegDailyKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_PosDailySharpKwh =
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_PosDailyPeakKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_PosDailyFlatKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_PosDailyValleyKwh = 
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_NegDailySharpKwh =
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_NegDailyPeakKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_NegDailyFlatKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fCab_NegDailyValleyKwh =
		pIEC104->blobStatData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nCurr_Day];

	pIEC104->strYC_KWh.fBAT1_DailyChgKwh = 
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[0][nCurr_Day];
		
	pIEC104->strYC_KWh.fBAT1_DailyDischgKwh	= 
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[0][nCurr_Day];

	pIEC104->strYC_KWh.fBAT2_DailyChgKwh = 
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[1][nCurr_Day];

	pIEC104->strYC_KWh.fBAT2_DailyDischgKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[1][nCurr_Day];

	pIEC104->strYC_KWh.fBAT3_DailyChgKwh =
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[2][nCurr_Day];

	pIEC104->strYC_KWh.fBAT3_DailyDischgKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[2][nCurr_Day];

	pIEC104->strYC_KWh.fBAT4_DailyChgKwh =
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[3][nCurr_Day];

	pIEC104->strYC_KWh.fBAT4_DailyDischgKwh	=
		pIEC104->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[3][nCurr_Day];

	return;
}

/*
主站发送-Client StartDT
68 04 07 00 00 00 
起始字节=68 数据单元长度(APDU)=4 U格式帧 STARTDT:ACT=1 CON=0 STOPDT:ACT=0 CON=0 TESTFR: ACT=0 CON=0
从站发送-ESS Server
Server SEND: 68 04 0b 00 00 00
*/

/*
从站发送=ESS YaoXin System.
68 12 00 00 00 00 01 85 01 01 01 00 01 00 00 00 00 00 00 00 
起始字节=68 数据单元长度(APDU)=18 I格式帧 发送序号(NS)=0 
接收序号(NR)=0 TI= 1 VSQ=85 SQ=1 INFONUM=5 COT= 0101 T=0 PN=0 CAUSE =1 COA =1 M_SP_NA_1
单点遥信变位          肯定认可    自定义原因     
 变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=1 分
 变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=2 分
 变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=3 分
 变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=4 分
 变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=5 分
*/

/*
从站发送-ESS Ems Mgmt
68 c4 02 00 00 00 0b bd 01 01 01 00 01 40 00 0c 00 00 00 00 00 06 00 00 
70 00 00 71 00 00 72 00 00 e0 01 00 04 00 00 df 00 00 20 ff 00 e1 00 00 
d0 02 00 02 00 00 4d 01 00 b2 fe 00 4f 01 00 0c 03 00 0c 00 00 bb 01 00 
44 fe 00 bd 01 00 48 03 00 16 00 00 29 02 00 9c ff 00 2b 02 00 84 03 00 
20 00 00 29 02 00 9c ff 00 2b 02 00 c0 03 00 2a 00 00 29 02 00 9c ff 00 
2b 02 00 fc 03 00 34 00 00 29 02 00 38 ff 00 2b 02 00 38 04 00 3e 00 00 
29 02 00 9c ff 00 2b 02 00 74 04 00 48 00 00 29 02 00 42 ff 00 2b 02 00 
b0 04 00 52 00 00 29 02 00 9c ff 00 2b 02 00 ec 04 00 5c 00 00 29 02 00 
b4 fb 00 2b 02 00 
起始字节=68 数据单元长度(APDU)=196 I格式帧 发送序号(NS)=1 
接收序号(NR)=0 TI= 11 VSQ=BD SQ=1 INFONUM=61 COT= 101 T=0 PN=0 CAUSE =1 COA =1 M_ME_NB_1
带品质描述的测量值, 标度化值        肯定认可    自定义原因     
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16385 OV=0 未溢出  值=12
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16386 OV=0 未溢出  值=0
 变化有?IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16387 OV=0 未溢出  值=6
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16388 OV=0 未溢出  值=112
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16389 OV=0 未溢出  值=113
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16390 OV=0 未溢出  值=114
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16391 OV=0 未溢出  值=480
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16392 OV=0 未溢出  值=4
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16393 OV=0 未溢出  值=223
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16394 OV=0 未溢出  值=-32544
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16395 OV=0 未溢出  值=225
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16396 OV=0 未溢出  值=720
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16397 OV=0 未溢出  值=2
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16398 OV=0 未溢出  值=333
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16399 OV=0 未溢出  值=-32434
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16400 OV=0 未溢出  值=335
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16401 OV=0 未溢出  值=780
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16402 OV=0 未溢出  值=12
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16403 OV=0 未溢出  值=443
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16404 OV=0 未溢出  值=-32324
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16405 OV=0 未溢出  值=445
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16406 OV=0 未溢出  值=840
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16407 OV=0 未溢出  值=22
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16408 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16409 OV=0 未溢出  值=-32668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16410 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16411 OV=0 未溢出  值=900
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16412 OV=0 未溢出  值=32
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16413 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16414 OV=0 未溢出  值=-32668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16415 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16416 OV=0 未溢出  值=960
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16417 OV=0 未溢出  值=42
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被馑?BL=0 点号=16418 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16419 OV=0 未溢出  值=-32668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16420 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16421 OV=0 未溢出  值=1020
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16422 OV=0 未溢出  值=52
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16423 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16424 OV=0 未溢出  值=-32568
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16425 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16426 OV=0 未溢出  值=1080
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16427 OV=0 未溢出  值=62
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16428 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16429 OV=0 未溢出  值=-32668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16430 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16431 OV=0 未溢出  值=1140
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16432 OV=0 未溢出  值=72
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16433 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16434 OV=0 未溢出  值=-32578
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16435 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16436 OV=0 未溢出  值=1200
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16437 OV=0 未溢出  值=82
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16438 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16439 OV=0 未溢出  值=-32668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16440 OV=0 未溢出  值=555
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16441 OV=0 未溢出  值=1260
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16442 OV=0 未溢出  值=92
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16443 OV=0 未溢出  值=553
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16444 OV=0 未溢出  值=-31668
 变化有效 IV=0  当前值 NT=10158080  未被取代 SB=0  未被封锁 BL=0 点号=16445 OV=0 未溢出  值=555
*/

/*
从站发送-ESS YAoCe SysInfo
68 22 04 00 00 00 0b 87 01 51 01 00 51 40 00 02 00 00 
07 00 00 03 00 00 20 00 00 30 00 00 10 00 00 40 00 00 
起始字节=68 数据单元长度(APDU)=34 I格式帧 发送序号(NS)=2 接收序号(NR)=0 
TI= 11 VSQ=87 SQ=1 INFONUM=7 COT= 5101 T=0 PN=0 CAUSE =1 COA =1 M_ME_NB_1
带品质描述的测量值, 标度化值        肯定认可    自定义原因     
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16465 OV=0 未溢出  值=2
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16466 OV=0 未溢出  值=7
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16467 OV=0 未溢出  值=3
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16468 OV=0 未溢出  值=32
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16469 OV=0 未溢出  值=48
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16470 OV=0 未溢出  值=16
 变化有效 IV=0  当前值 NT=-21757952  未被取代 SB=0  未被封锁 BL=0 点号=16471 OV=0 未溢出  值=64
*/

/*
主站发送
68 04 01 00 78 00 
起始字节=68 数据单元长度(APDU)=4 S格式帧 接收序号(NR)=60 
*/

/*
主站发送
68 14 02 00 60 00 67 01 06 00 01 00 00 00 00 1b c1 18 11 04 06 17 
起始字节=68 数据单元长度(APDU)=20 I格式帧 发送序号(NS)=1 接收序号(NR)=48 TI= 103 VSQ=01 SQ=0 INFONUM=1 COT= 06 T=0 PN=0 CAUSE =6 COA =1 C_CS_NA_1 
时钟同步命令      肯定认可    激活 
点号=0 435 毫秒 49 秒 24分 TIV=0时间有效 17小时 4日 6月 23年
从站发送
68 14 60 00 04 00 67 01 07 00 01 00 00 00 00 b4 a3 2c 11 05 07 17 
起始字节=68 数据单元长度(APDU)=20 I格式帧 发送序号(NS)=48 接收序号(NR)=2 TI= 103 VSQ=01 SQ=0 INFONUM=1 COT= 07 T=0 PN=0 CAUSE =7 COA =1 C_CS_NA_1 
时钟同步命令      肯定认可    激活确认 
点号=0 908 毫秒 41 秒 44分 TIV=0时间有效 17小时 5日 7月 23年
*/