// #define _TRACE_PRINT_
#include "ccu_sampler_main.h"

static ULONG Bytes2Ulong(BYTE* pBytes, int nBytes)
{
	ULONG ulVal = 0;
	int n;
	for(n = 0; n < nBytes; n++)
	{
		ulVal += (pBytes[n] << ((nBytes - n - 1)* 8));
	}

	return ulVal;
}

static float Bytes2Float(BYTE* pBytes)
{
	VAR_VALUE varV;
	
	varV.abyValue[0] = pBytes[0];
	varV.abyValue[1] = pBytes[1];
	varV.abyValue[2] = pBytes[2];
	varV.abyValue[3] = pBytes[3];
	
	return varV.fValue;
}

//PCS Alarm Byte Order:0-1-2-3
static ULONG Bytes2Ulong_Little(BYTE* pBytes)
{
	VAR_VALUE varV;
	
	varV.abyValue[0] = pBytes[0];
	varV.abyValue[1] = pBytes[1];
	varV.abyValue[2] = pBytes[2];
	varV.abyValue[3] = pBytes[3];
	
	return varV.ulValue;
}

typedef union tagShortVVV
{
        short   sVal;
        BYTE    byVal[2];
}SHORT_VAR;

static short Bytes2Short(BYTE* pBytes)
{
	SHORT_VAR 	s_Val;

    s_Val.byVal[1] = pBytes[0];
    s_Val.byVal[0] = pBytes[1];

	return s_Val.sVal;
}

static float IEEE745ToFloat(BYTE* pBytes)
{
	uint32_t nVal = (UINT)(pBytes[0] << 24) + (UINT)(pBytes[1] << 16) + (UINT)(pBytes[2] << 8) + (UINT)pBytes[3];
	// 提取符号位
    int sign = (nVal >> 31) & 0x1;
    // 提取指数位
    int exponent = ((nVal >> 23) & 0xFF) - 127;
    // 提取尾数位
    uint32_t mantissa = nVal & 0x7FFFFF;
    // 计算尾数的值
    float fraction = 1.0f;
	int i = 0;
    for (i = 0; i < 23; i++) {
        if ((mantissa >> (22 - i)) & 0x1) {
            fraction += 1.0f / (1 << (i + 1));
        }
    }
    // 根据符号位、指数和尾数计算最终的浮点数
    float result = (sign ? -1.0f : 1.0f) * fraction * (1 << exponent);
    return result;
}

int Parse_CFGG(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID) //boot up info
{
	TRACEX("------------now beging parsing CFGG------------:\n");
#ifdef IMMU3
	pCCU->bIsReceivedCFGG_FromLCCU = TRUE;
#endif
	USHORT 	usParsedLen = 0;
	int 	n = 0;
	//for(n= 0; n<usLenID; n++)
	//{
	//	printf("%02X",pData[n]);
	//}
	//printf("\n");
	
	CCU_BOOTUP_INFO* pBoot = &pCCU->pOldSet.ccuSettings;

	//1. CCU Address
	pBoot->nCCUAddress = (int)pData[0];
	pData += 1;
	usParsedLen += 1;

	//2. 24-bytes bar code
	memcpy(pBoot->szBarcode, pData, 24);
	pData += 24;
	usParsedLen += 24;

	//3. 8-Byte Mac-addr
	snprintf(pBoot->szMacAaddress, 16, "%02X%02X %02X%02X %02X%02X", 
		(char)pData[0], (char)pData[1],(char)pData[2], (char)pData[3],
		(char)pData[4], (char)pData[5]);
	pData += 8;
	usParsedLen += 8;

	//4. SW version
	memcpy(pBoot->szSWVer, pData, 8);
	pData += 8;
	usParsedLen += 8;

	//5. RTC time. do not parse
	pBoot->tmIMSU_RealTime = (float)Bytes2Ulong(pData, 4);
	time_t tmNoww = _NOW_;
	if(ABS(tmNoww - pBoot->tmIMSU_RealTime) >= 30)
	{
		pCCU->bNeedSyncTime2IMSU = TRUE;
		//printf("whichIMSU-D:%d, tmIMSU_RealTime:%x\n", pCCU->nInComingClientAddress, pBoot->tmIMSU_RealTime);
	}
	pData += 4;
	usParsedLen += 4;

	//6.Attribute Map1
	pBoot->sysAtributesMap1.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//7.Attribute Map2
	pBoot->sysAtributesMap2.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//8.Attribute Map3
	pBoot->sysAtributesMap3.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//9.Attribute Map4
	pBoot->sysAtributesMap4.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//10.Attribute Map5
	pBoot->sysAtributesMap5.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//11.Attribute Map6
	pBoot->sysAtributesMap6.usFlag = (USHORT)Bytes2Ulong(pData, 2);
	pData += 2;
	usParsedLen += 2;

	//12. System Fan Speed Level
	pBoot->byFan1_SpdLevel = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//13. System Fan Speed Level
	pBoot->byFan2_SpdLevel = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//14. System Fan Speed Level
	pBoot->byFan3_SpdLevel = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//15. Reserved
 	pData += 2;
	usParsedLen += 2;

	//16. Reserved
 	pData += 2;
	usParsedLen += 2;

	//17. Reserved
 	pData += 2;
	usParsedLen += 2;

	//18. Reserved
 	pData += 2;
	usParsedLen += 2;

	//19. Reserved
 	pData += 2;
	usParsedLen += 2;

	//20. Grp Num Set
	pBoot->byNumGrp_1 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//21. Grp Num Set
	pBoot->byNumGrp_2 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//22. Grp Num Set
	pBoot->byNumGrp_3 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//23. Grp Num Set
	pBoot->byNumGrp_4 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//24. Grp Num Set
	pBoot->byNumGrp_5 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//25. Grp Num Set
	pBoot->byNumGrp_6 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//26. Grp Num Set
	pBoot->byNumGrp_7 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//27. Grp Num Set
	pBoot->byNumGrp_8 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//28. Grp Num Set
	pBoot->byNumGrp_9 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//29. Grp Num Set
	pBoot->byNumGrp_10 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//30. Grp Num Set
	pBoot->byNumGrp_11 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//31. Grp Num Set
	pBoot->byNumGrp_12 = (BYTE)pData[0]; 
	pData += 1;
	usParsedLen += 1;

	//32.System Fan-1 Work Mode
	pBoot->byFan1_WorkMode_M = (BYTE)pData[0];
	pData += 1;
	usParsedLen += 1;

	pBoot->byFan1_FanWorkMode_N = (BYTE)pData[0];
	pData += 1; 
	usParsedLen += 1;

	//33.
 	pBoot->nFan1_OverTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//34. System Ultra Temp Point(for system cooling fan full-speed)
	pBoot->nFan1_UltraTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//35.System Fan-2 Work Mode
	pBoot->byFan2_WorkMode_M = (BYTE)pData[0];
	pData += 1;
	usParsedLen += 1;

	pBoot->byFan2_FanWorkMode_N = (BYTE)pData[0];
	pData += 1; 
	usParsedLen += 1;

	//36.
 	pBoot->nFan2_OverTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//37. System Ultra Temp Point(for system cooling fan full-speed)
	pBoot->nFan2_UltraTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//System Fan-3 Work Mode
	pBoot->byFan3_WorkMode_M = (BYTE)pData[0];
	pData += 1;
	usParsedLen += 1;

	pBoot->byFan3_FanWorkMode_N = (BYTE)pData[0];
	pData += 1; 
	usParsedLen += 1;

	//39.
 	pBoot->nFan3_OverTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//40. System Ultra Temp Point(for system cooling fan full-speed)
	pBoot->nFan3_UltraTemp4Cool = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//41.PCS Type
	pBoot->byPCSType = (UINT)pData[0];

	//printf("pBoot->byPCSType=[%02X]\n", pBoot->byPCSType);

	//pBoot->byPCSType = (pBoot->byPCSType > 1) ? 1 : pBoot->byPCSType;

	pData += 1;
	usParsedLen += 1;

	//42.DC or AC Energy Adjust Mode
 	if(	usParsedLen < usLenID )
 	{
 		pBoot->byDCorAC_AdjMode = (UINT)pData[0];
 		pData += 1;
 		usParsedLen += 1;
 	}
 	TRACE("usParsedLen=[%d],usLenID=[%d] byDCorAC_AdjMode=[%02X]\n", usParsedLen , usLenID, pBoot->byDCorAC_AdjMode);

	A_BLOB_CHARGERSPECIFIC_SETTINGS blbSet;
	memcpy(&blbSet, (void*)&pCCU->pOldSet, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
	memcpy(&blbSet.ccuSettings, pBoot, sizeof(CCU_BOOTUP_INFO));
	
	Push_BlobValue(SETTYPE_RAW_ST, pCCU->hSampler, 200, 200, (void*)&blbSet, 
		sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), BLOB_TYPE_CHARGER_SPECIFIC_SETTING);
 	//TRACE("nCCUAddress=[%d]branchCfgAttri.byFlag =%d \n",pBoot->nCCUAddress, pBoot->branchCfgAttri.byFlag);
	return 0;
}
 
int Parse_LSYS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	USHORT 	usParsedLen = 0;
 	USHORT 	usVal; 
	UINT 	n = 0;
	SHORT 	sVal;

	pCCU->strLSYSInfo.tmLastRecv_LSYS = _NOW_;

	//printf("Parse_LSYS:\n");
	//for(n= 0; n<usLenID; n++)
	//{
	//	printf("%02X",pData[n]);
	//}
	//printf("\n");

	//1. Sys available status
	pCCU->strLSYSInfo.emLSysAvaiStatus = (LSYS_AVAILBLESTATE)pData[0];
	pData += 1;
 	usParsedLen += 1;

	//2. CCU Running Mode  0-Normal Mode  1-Debug Mode
	pCCU->strLSYSInfo.emRunningMode = pData[0];
	pData += 1;
 	usParsedLen += 1;

	//3. sys RTC
	pCCU->strLSYSInfo.tmRTC = (time_t)Bytes2Ulong(pData, 4);
	pData += 4;
 	usParsedLen += 4;

	//4. Sys Alarm Map1
	pCCU->strLSYSInfo.sysAlarmMap1.uiFlag = (UINT)Bytes2Ulong(pData, 4);
	pData += 4;
	usParsedLen += 4;

	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.DoorOpen)
	{
		TRACE("SYSALARM: DoorOpen! \n");
	}
	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.EPO)
	{
		TRACEX("SYSALARM: EPO! \n");
	}
	if(pCCU->strLSYSInfo.sysAlarmMap1.Alarm.SPD)
	{
		TRACEX("SYSALARM: SPD ! \n");
	}

	//5. System Alarm2 Bit Map 
	pCCU->strLSYSInfo.sysAlarmMap2.uiFlag = (UINT)Bytes2Ulong(pData, 4);
	pData += 4;
 	usParsedLen += 4;

	//6. System Alarm3 Bit Map
	pCCU->strLSYSInfo.sysAlarmMap3.uiFlag = (UINT)Bytes2Ulong(pData, 4);
	pData += 4;
 	usParsedLen += 4;

	//7. System Alarm4 Bit Map
	pCCU->strLSYSInfo.sysAlarmMap4.uiFlag = (UINT)Bytes2Ulong(pData, 4);
	pData += 4;
 	usParsedLen += 4;

	//8. PM Group Alarm
	pCCU->strLSYSInfo.pmGrpAlmMap.uiFlag = (UINT)Bytes2Ulong(pData, 4);
 	pData += 4;
 	usParsedLen += 4;

	//9. Resved
	pData += 9;
 	usParsedLen += 9;
	// 9-1	ABUAlmStatus1
	pCCU->strLSYSInfo.mapAbuStatu1.byRunStatus = (BYTE)pData[0];
	pData += 1;
 	usParsedLen += 1;
	// 10	APP Upgrading Type
	pCCU->strLSYSInfo.emAPPUpgradingType = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;

	pCCU->strLSYSInfo.byUpgradingSubID = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;

	pCCU->strLSYSInfo.emUpgradingStatus = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;

	pCCU->strLSYSInfo.byUpgradingSubIDSchedule = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;

	pCCU->strLSYSInfo.emUpgradingTotSchedule = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;

	pCCU->strLSYSInfo.byUpgradingFailReason = (UINT)pData[0];
	pData += 1;
 	usParsedLen += 1;
	// 16	PCS Grp#1 Idx:2 RunSTatus
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
 	{
 		pCCU->strLSYSInfo.mapPCSSysStatusBelong2[n].byPCSSysStatu = (BYTE)pData[0];
 		pData += 1;
 		usParsedLen += 1;
 	}

	// 20	PCSGrp#1 Idx:1 RunStatu
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pCCU->strLSYSInfo.mapPCSSysStatus[n].byPCSSysStatu = (BYTE)pData[0];
		pData += 1;
		usParsedLen += 1;
	}

	// 24	CCUFaultCode
	pCCU->strLSYSInfo.uiCCUFaultCode = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	// 25	PCS#1CtrlFailCode
	pCCU->strLSYSInfo.uiPCS1CtrlFailCode = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	// 26	PCS#2CtrlFailCode
	pCCU->strLSYSInfo.uiPCS2CtrlFailCode = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	//27. PCS#3CtrlFailCode
	pCCU->strLSYSInfo.uiPCS3CtrlFailCode = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	// 28. PCS#4CtrlFailCode
	pCCU->strLSYSInfo.uiPCS4CtrlFailCode = (UINT)pData[0];
	pData += 1;
	usParsedLen += 1;

	// 29. ABU Run Status
	pCCU->strLSYSInfo.mapSTSCode.byRunStatus = (BYTE)pData[0];
	pData += 1;
	usParsedLen += 1;
	//TRACE("byRunStatus=[%02X]\n", pCCU->strLSYSInfo.mapSTSCode.byRunStatus);

	// 30. Leakage Current
	sVal = Bytes2Short(pData);
	pCCU->strLSYSInfo.fLeakageCurr = ((float)sVal) * 0.1;
	pData += 2;
	usParsedLen += 2;
	//31~34. System Ambient Temp1
	for(n = 0; n < MAX_AMBINENT_TEMP; n++)
	{
		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.fAmbientTemp[n] = ((float)sVal) * 0.1;
		pData += 2;
		usParsedLen += 2;
		//TRACE("System Ambinent=[%d] Temp=[%.1f]\n", n, pCCU->strLSYSInfo.fAmbientTemp[n]);
	}
	
	//TRACE("---Beging parsing LSYS: emLSysAvaiStatus=[%d] emRunningMode=[%d]---\
	//	sysAlarmMap1.uiFlag=[%04X] sysAlarmMap2.uiFlag=[%04X] sysAlarmMap3.uiFlag=[%04X] sysAlarmMap4.uiFlag=[%04X] pmGrpAlmMap.uiFlag=[%04X]\n", \
	//	pCCU->strLSYSInfo.emLSysAvaiStatus, pCCU->strLSYSInfo.emRunningMode,\
	//	pCCU->strLSYSInfo.sysAlarmMap1.uiFlag,\
	//	pCCU->strLSYSInfo.sysAlarmMap2.uiFlag,\
	//	pCCU->strLSYSInfo.sysAlarmMap3.uiFlag,\
	//	pCCU->strLSYSInfo.sysAlarmMap4.uiFlag,\
	//	pCCU->strLSYSInfo.pmGrpAlmMap.uiFlag);

	// 35	IMIU1 DC Bus Positive Insulation Value  ~ 54	IMIU10 DC Bus Negative Insulation Value
	for(n = 0; n < MAX_SUPPORT_IMIU_NUM; n++)
	{
		//DC Bus Positive Insulation Value
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.fIMIU_P_IsoVal[n] = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//DC Bus Negative Insulation Value
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.fIMIU_N_IsoVal[n] = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;
		//TRACE("WhichIMIU=[%d], fIMIU_P_IsoVal=%f, fIMIU_N_IsoVal=%f\n", \
		//	n, pCCU->strLSYSInfo.fIMIU_P_IsoVal[n], pCCU->strLSYSInfo.fIMIU_N_IsoVal[n]);
	}
	 
	// 55	Group-A AC/DC Real-Time Voltage. Export ~ 74	Group-D AC/DC OutContactorVoltage. 
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//. Group-A AC/DC Real-Time Voltage. Export
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Volt = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A AC/DC Real-Time Current. Export
		sVal = (SHORT)Bytes2Short(pData);
		pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Curr = (float)sVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A AC/DC Max Output Power Ability
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fACDC_Max_Output_Pwr_Ability = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A AC/DC WorkStatu
		pCCU->strLSYSInfo.branchNodeData[n].emACDC_WorkStatu = (UINT)pData[0];
		pData += 1;
		usParsedLen += 1;

		//. Group-A AC/DC OutRelay Voltage
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		if(usVal == 0xFFFF)
		{
			pCCU->strLSYSInfo.branchNodeData[n].fACDCOutRelayVolt = 0.0;
		}
		else
		{
			pCCU->strLSYSInfo.branchNodeData[n].fACDCOutRelayVolt = (float)usVal / 10.0;
		}
		pData += 2;
		usParsedLen += 2;
		//TRACE("branchNodeData[%d] fACDC_RealTime_Volt=%.1f fACDC_RealTime_Curr=%.1f fACDC_Max_Output_Pwr_Ability=%.1f emACDC_WorkStatu=%d \n",\
		//	n,pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Volt,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Curr,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fACDC_Max_Output_Pwr_Ability,\
		//	pCCU->strLSYSInfo.branchNodeData[n].emACDC_WorkStatu);
	}
	
	// 75	Group-A MPPT Real-Time Voltage. Export ~ 84	Group-B MPPT Total Generate KWH
 	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//. Group-A MPPT Real-Time Voltage. Export
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A MPPT Real-Time Current. Export
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A MPPT Max Output Power Ability
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fMPPT_Max_Output_Pwr_Ability = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A MPPT WorkStatu
		pCCU->strLSYSInfo.branchNodeData[n].emMPPT_WorkStatu = (UINT)pData[0];
		pData += 1;
		usParsedLen += 1;

		//. Group-A MPPT Total Generate KWH
		pCCU->strLSYSInfo.branchNodeData[n].fMPPT_Total_Generate_KWH = Bytes2Float(pData);
		pData += 4;
		usParsedLen += 4;	
		//TRACE("branchNodeData[%d] fMPPT_RealTime_Volt=%.1f fMPPT_RealTime_Curr=%.1f \
		//fMPPT_Max_Output_Pwr_Ability=%.1f emMPPT_WorkStatu=%d fMPPT_Total_Generate_KWH=%.1f\n",\
		//	n, pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Volt,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fMPPT_RealTime_Curr,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fMPPT_Max_Output_Pwr_Ability,\
		//	pCCU->strLSYSInfo.branchNodeData[n].emMPPT_WorkStatu,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fMPPT_Total_Generate_KWH);
	}

	// 85	Group-C MPPT Real-Time Voltage. Export ~ 110	Group-D DC/DCWorkState
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//. Group-A  DC/DC Real-Time Voltage. Export
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A  DC/DC Real-Time Current. Export
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A  DC/DC Max Output Power Ability
		usVal = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.branchNodeData[n].fDCDC_Max_Output_Pwr_Ability = (float)usVal / 10.0;
		pData += 2;
		usParsedLen += 2;

		//. Group-A DC/DC WorkStatu
		pCCU->strLSYSInfo.branchNodeData[n].emDCDC_WorkStatu = (UINT)pData[0];
		pData += 1;
		usParsedLen += 1;
		////TRACE("branchNodeData[%d] fDCDC_RealTime_Volt=%.1f fDCDC_RealTime_Curr=%.1f fDCDC_Max_Output_Pwr_Ability=%.1f emDCDC_WorkStatu=%d \n",\
		//	n, pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Volt,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fDCDC_RealTime_Curr,\
		//	pCCU->strLSYSInfo.branchNodeData[n].fDCDC_Max_Output_Pwr_Ability,\
		//	pCCU->strLSYSInfo.branchNodeData[n].emDCDC_WorkStatu);
	}

	if(usParsedLen >= usLenID)
 	{
 		TRACE("usParsedLen=[%d] usLenID=[%d]\n", usParsedLen , usLenID);
 		return 0;
 	}
	// 111	Group-A PCS Real-Time ActivePower. Export
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
 	{
 		sVal = Bytes2Short(pData);
 		pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp = ((float)sVal) * -0.1;//PCS Chg Q is Neg, Dischg is Pos. 
 		pData += 2;
  		usParsedLen += 2;
 		TRACE("branchNodeData[%d].fActvPwr_PCSGrp=[%.1f]\n", n, pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp);
 	}

	if(usParsedLen >= usLenID)
 	{
 		TRACE("usParsedLen=[%d] usLenID=[%d]\n", usParsedLen , usLenID);
 		return 0;
 	}
	// 115	Group-A PCS Real-Time ReactivePower. Export
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
 	{
 		sVal = Bytes2Short(pData);
 		pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp = ((float)sVal) * 0.1; //L Inductive reaction:Postive, C capacitive reactive power:Negtive.
 		pData += 2;
  		usParsedLen += 2;
 		TRACE("branchNodeData[%d].fQ_PCSGrp=[%.1f]\n", n, pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp);
 	}
	if(usParsedLen >= usLenID)
 	{
 		TRACE("usParsedLen=[%d] usLenID=[%d]\n", usParsedLen , usLenID);
 		return 0;
 	}
	// 119	Group-A PCS Real-Time ReactivePower. Ctrl
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
 	{
 		sVal = Bytes2Short(pData);
 		pCCU->strLSYSInfo.branchNodeData[n].fCtrl_RealTime_Q = ((float)sVal) * 0.1; //L Inductive reaction:Postive, C capacitive reactive power:Negtive.
 		pData += 2;
  		usParsedLen += 2;
 		TRACE("blobGrpRectData[%d].fQ_PCSGrp=[%.1f]\n", n, pCCU->strLSYSInfo.branchNodeData[n].fCtrl_RealTime_Q);
 	}
	TRACEW("Parse_LSYS branchNodeData[%d] emACDC_WorkStatu=%d fACDC_RealTime_Volt=%.1f fACDC_RealTime_Curr=%.1f "
		"fACDC_Max_Output_Pwr_Ability=%.1f  fActvPwr_PCSGrp=[%.1f]",
		n, 
		pCCU->strLSYSInfo.branchNodeData[0].emACDC_WorkStatu,
		pCCU->strLSYSInfo.branchNodeData[0].fACDC_RealTime_Volt,
		pCCU->strLSYSInfo.branchNodeData[0].fACDC_RealTime_Curr,
		pCCU->strLSYSInfo.branchNodeData[0].fACDC_Max_Output_Pwr_Ability,
		pCCU->strLSYSInfo.branchNodeData[0].fActvPwr_PCSGrp
	);
	
	return 0;
}
 
int Parse_LDEV(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	//printf("--- Parse_LDEV --- \n");
	//int n = 0;
	//for(n = 0; n < usLenID; n++)
	//{
	//	printf("%02X-", pData[n]);
	//}
	//printf("\n");

	BYTE byDevType = pData[0];
	pData += 1;
	if((byDevType !=1) && (byDevType != 2)) //0x01-IMEU2_#1;0x02-IMEU2_#2
	{
		TRACE("Error: byDevType=[%d] is Not Defined!\n", byDevType);
		return;
	}

	BYTE byRecvMsgDataLen = pData[0];
	pData += 1;

	pData += 8; //RESVED

	//char szHWVer[9]={0};

	memcpy(pCCU->strLSYSInfo.szPowerCube_IMEU2, pData, 8);
	pCCU->strLSYSInfo.szPowerCube_IMEU2[8] = '\0';
	pData += 8;
	
	//memcpy(szHWVer, pData, 8);
 	
	TRACE("LDEV:byDevType=[%d] byRecvMsgDataLen=[%d] szSWVer=[%s]\n", \
		byDevType, byRecvMsgDataLen, pCCU->strLSYSInfo.szPowerCube_IMEU2);

	return 0;
}

int Parse_HEART(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	// printf("--- Parse_HEART --- \n");
	// int n = 0;
	// for(n = 0; n < usLenID; n++)
	// {
	// 	printf("%02X-", pData[n]);
	// }
	// printf("\n");

	pCCU->strHEARTInfo.bySWRunLocation = pData[0]; // 软件运行位置
	// pData += 1;

	// pCCU->strHEARTInfo.usSWVer = (pData[0] << 8 || pData[1]); // 软件版本号，BYTE1 软件版本号高位，BYTE2低位 // 不用检测，软件版本在CFGG中有，重复了

	pCCU->strHEARTInfo.tmLastRecv_HEART = _NOW_;
	pCCU->bEverRecvHeart = TRUE; // keep-app-alive(2026-09): 记录曾成功收到 HEART（只置位不复位）

	// printf("HEART[ver]:%04X\n", pCCU->strHEARTInfo.usSWVer);
	
	// TRACE("HEART:bySWRunLocation=[%02X] szSWVer=[%s]\n", \
	// 	pCCU->strHEARTInfo.bySWRunLocation, pCCU->strHEARTInfo.usSWVer, pCCU->strLSYSInfo.szPowerCube_IMEU2);

	return 0;
}

/*-------------------------------
BarCode Format:
02 8B 00 F0 00 00 00 00 00 00 00 00
02 8B F0 00 56 13 0C 15 A3 FB 06 A8

56 : V 
13 0C 15 A3 FB : long long-081807123451
06 A8 : UINT-1704
 PCS 0一1一2一3
081807123451 V 1704
---------------------------------*/
int Parse_RTIF(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	int 		i = 0, n = 0;
	USHORT		usTemp = 0;
	BYTE		byStatu = 0;
	SHORT		sVal;
	float 		fVal;

	/*printf("--- Parse_RTIF --- \n");
	for(n = 0; n < usLenID; n++)
	{
		printf("%02X", pData[n]);
	}
	printf("\n");*/

	TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)pData[0];
	pData += 1;
	if((emRectType < 0) || (emRectType >= MAX_RECT_TYPE))
	{
		TRACE(" emRectType=[%d]\n",  emRectType);
		return -1;
	}

	//Grp Rect Data
	usTemp = (USHORT)Bytes2Ulong(pData, 2);
	pCCU->strLSYSInfo.blobGrpRectData.fRatedPower[emRectType]	= ((float)usTemp)*0.1;
	pData += 2;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
#ifdef IMMU3
	pCCU->strLSYSInfo.blobGrpRectData.fRatedCurr[emRectType] = ((float)usTemp)*0.1;
#else
	pCCU->strLSYSInfo.blobGrpRectData.fRatedCurr[emRectType] = ((float)usTemp);
#endif
	pData += 2;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
#ifdef IMMU3
	pCCU->strLSYSInfo.blobGrpRectData.fRealTime_MaxCurr[emRectType] = ((float)usTemp)*0.1;
#else
	pCCU->strLSYSInfo.blobGrpRectData.fRealTime_MaxCurr[emRectType] = ((float)usTemp);
#endif
	pData += 2;

	//0-OFF|1-Rectifying Mode|2-Invert Mode|3-MPPT 
	pCCU->strLSYSInfo.blobGrpRectData.emWorkMode[emRectType] = (UINT)pData[0];
	pData += 1;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
	pCCU->strLSYSInfo.blobGrpRectData.fCtrlVolt[emRectType] = ((float)usTemp)*0.1;
	pData += 2;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
#ifdef IMMU3
	pCCU->strLSYSInfo.blobGrpRectData.fCtrlCurr[emRectType] = ((float)usTemp)*0.1;
#else
	pCCU->strLSYSInfo.blobGrpRectData.fCtrlCurr[emRectType] = ((float)usTemp);
#endif
	pData += 2;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
	pCCU->strLSYSInfo.blobGrpRectData.fRealTimeVolt[emRectType] = ((float)usTemp)*0.1;
	pData += 2;

	usTemp = (USHORT)Bytes2Ulong(pData, 2);
#ifdef IMMU3
	pCCU->strLSYSInfo.blobGrpRectData.fRealTimeCurr[emRectType] = ((float)usTemp)*0.1;
#else
	pCCU->strLSYSInfo.blobGrpRectData.fRealTimeCurr[emRectType] = ((float)usTemp);
#endif
	pData += 2;

	/*TRACE("emRectType[%d] emWorkMode=[%d] fCtrlVolt=[%.1f] fCtrlCurr=[%.1f]\
		fRealTimeVolt=[%.1f] fRealTimeCurr=[%.1f] fRatedPower=[%.1f] fRatedCurr=[%.1f] fRealTime_MaxCurr=[%.1f]\n", \
		emRectType, 
		pCCU->strLSYSInfo.blobGrpRectData.emWorkMode[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fCtrlVolt[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fCtrlCurr[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fRealTimeVolt[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fRealTimeCurr[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fRatedPower[emRectType],
		pCCU->strLSYSInfo.blobGrpRectData.fRatedCurr[emRectType],\
		pCCU->strLSYSInfo.blobGrpRectData.fRealTime_MaxCurr[emRectType]);
	*/
	BOOL bNeedGetPCSStatu = FALSE;
	UINT nRatePower = (UINT)(pCCU->strLSYSInfo.blobGrpRectData.fRatedPower[emRectType]*10);
	if((nRatePower%625 == 0) && (nRatePower%220 != 0)) //John 20220510.Temp Solution
	{
		bNeedGetPCSStatu = TRUE;
	}

	UINT nSigPMMsgDataLenth = (UINT)pData[0];
	pData += 1;

	UINT nOnLineNumber = (UINT)pData[0];
	pData += 1;
	if(nOnLineNumber >= MAX_SINGLE_RECT_NUM)
	{
		TRACE("nOnLineNumber=[%d]\n", nOnLineNumber);
		return -1;
	}

	pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[emRectType] = nOnLineNumber;
 
	//TRACE("nRectType=[%d] nOnLineNumber=[%d] nSigPMMsgDataLenth=[%d]\n", \
	//	emRectType, nOnLineNumber, nSigPMMsgDataLenth);
	
	//2021-12-21. When All Online PM is 0,Init Single PM Blob
	int nTotalOnLinePM = 0;
	for(n = 0; n < MAX_RECT_TYPE; n++)
	{
		nTotalOnLinePM += pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[n];
	}

	if(nTotalOnLinePM == 0)
	{
		memset(&pCCU->strLSYSInfo.singleRectData, 0 , sizeof(A_BLOB_SINGLE_RECT_INFO));
	}

	pCCU->strLSYSInfo.singleRectData.nOnlineNum[emRectType] = nOnLineNumber;

	UINT nParse_SiglMsgLen = 0;

	for(n = 0; n < nOnLineNumber; n++)
	{	
		nParse_SiglMsgLen = 0;
		
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bExist = TRUE;
		
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byCommStatu = (BYTE)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].nBelongGrp = (UINT)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		//ErrCode
		//pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].nErrCode = (UINT)Bytes2Ulong(pData, 4);
		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byRectErrCode, 16, "%02X|%02X|%02X|%02X", pData[0],pData[1],pData[2],pData[3]);
		//TRACE("Err: %02X | %02X | %02X | %02X\n", pData[0],pData[1],pData[2],pData[3]);

		byStatu = pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		byStatu = pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bDC_Shorted 			= byStatu & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bInternalDC_LowVolt		= (byStatu >> 1) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bInternalCommErr		= (byStatu >> 2) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bPFC_Err				= (byStatu >> 3) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bOn_Sleep				= (byStatu >> 4) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bDisCh_Err				= (byStatu >> 5) & 0x01;

		byStatu = pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

	 	pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bDC_Off 			= byStatu & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bRectFail			= (byStatu >> 1) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bProtected			= (byStatu >> 2) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bFanFault			= (byStatu >> 3) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bOverTemp			= (byStatu >> 4) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bDC_OverVolt		= (byStatu >> 5) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bWalkIn_Enabled		= (byStatu >> 6) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bRectCommFail		= (byStatu >> 7) & 0x01;

		byStatu = pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bPowerLimited 		= byStatu & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bID_Conflict		= (byStatu >> 1) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bCurr_NoBalace		= (byStatu >> 2) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bAC_PhaseLack		= (byStatu >> 3) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bAC_NoBalance		= (byStatu >> 4) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bDC_LowVolt			= (byStatu >> 5) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bAC_OverVolt		= (byStatu >> 6) & 0x01;
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].bPFC_Off			= (byStatu >> 7) & 0x01;


		byStatu = pData[0];
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].emWorkMode 			= (UINT)(byStatu & 0x01);//1-Invert Mode; 0-Rectfying Mode

		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byInvertErrCode, 16, "%02X|%02X|%02X|%02X", pData[3],pData[2],pData[1],pData[0]);
		pData += 4;
		nParse_SiglMsgLen += 4;

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRTOutputVolt 	= ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRTOutputCurr 	= ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRect_DCDCVer 	= ((float)usTemp)*0.01;
		pData += 2;
		nParse_SiglMsgLen += 2;

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRect_PFCVer 	= ((float)usTemp)*0.01;
		pData += 2;
		nParse_SiglMsgLen += 2;

		//for(i = 0; i< 8; i++)
		//{
     	//    printf("%02X", pData[i]);
		//}

		//9. Parse Barcode Info
		/*
		int 				nOffSet = 0;
		unsigned long long 	llTemp = 0;
		
		char byFirstVer = pData[0];
 		pData += 1;

		for(i = 0; i < 5; i++)
		{
			llTemp += ((unsigned long long)pData[i] << (8*(4-i)));
 		}

		pData += 5;
    	nOffSet +=	sprintf((&pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode[nOffSet]), "%012lld", llTemp);
    	nOffSet += 	sprintf((&pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode[nOffSet]), "%c", byFirstVer);

    	usTemp = (USHORT)Bytes2Ulong(pData, 2);
		sprintf((&pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode[nOffSet]), "%04d", usTemp);
		pData += 2;

		pData += 8;*/

		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode, 24, "%s", pData);
		pData += 24;
		nParse_SiglMsgLen += 24;

 		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth) //20220512 Added by John, For Variable Msg Length
		{
			continue;
		}

		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].mapPCSCode.uiPCSStatus = (UINT)Bytes2Ulong_Little(pData);
		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultCode, 16, "%02X|%02X|%02X|%02X", pData[3],pData[2],pData[1],pData[0]);

		pData += 4;
		nParse_SiglMsgLen += 4;

		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth) //20220809 Added by John, For Variable Msg Length
		{
			continue;
		}

		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].uiPCSFaultCode_1113 = (UINT)Bytes2Ulong_Little(pData);
		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultCode_1113, 16, "%02X|%02X|%02X|%02X", pData[3],pData[2],pData[1],pData[0]);

		//printf("byPCSFaultCode_1113: %02X|%02X|%02X|%02X|\n", pData[3],pData[2],pData[1],pData[0]);

		pData += 4;
		nParse_SiglMsgLen += 4;

		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth) //20220809 Added by John, For Variable Msg Length
		{
			continue;
		}
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].uiPCSFaultCode_1114 = (UINT)Bytes2Ulong_Little(pData);
		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultCode_1114, 16, "%02X|%02X|%02X|%02X", pData[3],pData[2],pData[1],pData[0]);

		//printf("byPCSFaultCode_1114: %02X|%02X|%02X|%02X|\n", pData[3],pData[2],pData[1],pData[0]);

		pData += 4;
		nParse_SiglMsgLen += 4;
 
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth) //20220809 Added by John, For Variable Msg Length
		{
			continue;
		}
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].uiPCSFaultCode_1115 = (UINT)Bytes2Ulong_Little(pData);
		snprintf(pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultCode_1115, 16, "%02X|%02X|%02X|%02X", pData[3],pData[2],pData[1],pData[0]);
		//printf("byPCSFaultCode_1115: %02X|%02X|%02X|%02X|\n", pData[3],pData[2],pData[1],pData[0]);
		pData += 4;
		nParse_SiglMsgLen += 4;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth) //20220809 Added by John, For Variable Msg Length
		{
			continue;
		}

		//Addeb By John. 2022-11-12
		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputVolt[0] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputVolt[1] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputVolt[2] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputCurr[0] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputCurr[1] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_InputCurr[2] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fAC_Freq = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		if((emRectType >= TYPE_SIG_RECT_ACDC_GRP1) && (emRectType <= TYPE_SIG_RECT_ACDC_GRP4))
		{
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fAmbit_Temp = (float)sVal;
		}
		else
		{
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fAmbit_Temp = ((float)sVal)*0.1;
		}
		
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		//Added By John 20230401
		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_OutputVolt[0] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_OutputVolt[1] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		usTemp = (USHORT)Bytes2Ulong(pData, 2);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPh_OutputVolt[2] = ((float)usTemp)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPtotalActive = ((float)sVal)*0.1*-1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPtotalReactive = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPtotalFactor = ((float)sVal)*0.001;
		pData += 2;
		nParse_SiglMsgLen += 2;

		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		//----------------------added By John. 20230810.---------------------//
		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Active[0] = ((float)sVal)*0.1*-1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Active[1] = ((float)sVal)*0.1*-1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Active[2] = ((float)sVal)*0.1*-1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Reactive[0] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Reactive[1] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Reactive[2] = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

#ifdef IMMU3
		for(i=0; i<NUM_GRID_PHASE; ++i)
		{
			sVal = Bytes2Short(pData);
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Factor[i] = ((float)sVal)*0.001;
			pData += 2;
			nParse_SiglMsgLen += 2;
			if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
			{
				continue;
			}
		}

		for(i=0; i<NUM_GRID_PHASE; ++i)
		{
			usTemp = (USHORT)Bytes2Ulong(pData, 2);
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fphase_LineVolt[i] = ((float)usTemp)*0.1;
			pData += 2;
			nParse_SiglMsgLen += 2;
			if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
			{
				continue;
			}
		}

		sVal = Bytes2Short(pData);
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPTotalApparent = ((float)sVal)*0.1;
		pData += 2;
		nParse_SiglMsgLen += 2;
		if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
		{
			continue;
		}

		for(i=0; i<NUM_GRID_PHASE; ++i)
		{
			sVal = Bytes2Short(pData);
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fPphase_Apparent[i] = ((float)sVal)*0.1;
			pData += 2;
			nParse_SiglMsgLen += 2;
			if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
			{
				continue;
			}
		}

		// 模块当前能量调度模式
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byCurrent_Schedulingmode = (BYTE)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;
		
		// 模块当前并离网状态
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byCurrent_Onandoffstatus = (BYTE)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		// 当前系统状态
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byCurrent_Systemstatus = (BYTE)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

		// 当前充电模式
		pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byCurrent_Chargingmode = (BYTE)pData[0];
		pData += 1;
		nParse_SiglMsgLen++;

#endif
	if(pCCU->emESSDevType == LIQUID_COOLING_BACKUP)
	{
			if(emRectType >= TYPE_SIG_RECT_ACDC_GRP1 && emRectType <= TYPE_SIG_RECT_ACDC_GRP4)
			{
				//printf("emRectType = [%d]\n", emRectType);
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1110[0] = pData[0];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1110[1] = pData[1];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1110[2] = pData[2];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1110[3] = pData[3];
				//printf("1110   %02X,%02X,%02X,%02X\n", pData[0], pData[1], pData[2], pData[3]);
				pData += 4;
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}

				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1111[0] = pData[0];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1111[1] = pData[1];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1111[2] = pData[2];
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byPCSFaultcode_1111[3] = pData[3];
				//printf("1111   %02X,%02X,%02X,%02X\n", pData[0], pData[1], pData[2], pData[3]);
				pData += 4;  
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				} 
				
				fVal = (float)(pData[0]<<8 | pData[1]);
				//TRACEI("%X, %X\n", pData[1], pData[0]);
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].f35KW_PlateTemperature = (fVal)*0.001;
				pData += 2;
				nParse_SiglMsgLen += 2;
				//printf("f35KW_PlateTemperature = [%f]", pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].f35KW_PlateTemperature);
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}

				fVal = (float)(pData[0]<<8 | pData[1]);
				//TRACEI("%X, %X\n", pData[1], pData[0]);
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].f35KW_PowerDevicesperature = (fVal)*0.001;
				pData += 2;
				nParse_SiglMsgLen += 2;
				//printf("f35KW_PowerDevicesperature = [%f]", pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].f35KW_PowerDevicesperature);
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}				
			}
			else if(emRectType >= TYPE_SIG_RECT_MPPT_GRP1 && emRectType <= TYPE_SIG_RECT_MPPT_GRP4)
			{
				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fMPPT_DCTemp = IEEE745ToFloat(pData);
				//printf("emRectType = [%d] which = [%d], fMPPT_DCTemp %02X %02X %02X %02x \n", emRectType, n, pData[0], pData[1], pData[2], pData[3]);
				//printf("fMPPT_DCTemp = [%f]", pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fMPPT_DCTemp);
				pData += 4;
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}

				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fMPPT_PFCTemp = IEEE745ToFloat(pData);
				//printf("emRectType = [%d] which = [%d] fMPPT_PFCTemp %02X %02X %02X %02x \n",emRectType,n, pData[0], pData[1], pData[2], pData[3]);
				pData += 4;
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}

				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fMPPT_DCTransTemp = IEEE745ToFloat(pData);
				//printf("emRectType = [%d] which = [%d] fMPPT_DCTransTemp %02X %02X %02X %02x \n",emRectType,n, pData[0], pData[1], pData[2], pData[3]);

				pData += 4;
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}

				pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fMPPT_PFCTransTemp = IEEE745ToFloat(pData);
				//printf("emRectType = [%d] which = [%d] fMPPT_PFCTransTemp %02X %02X %02X %02x \n",emRectType,n, pData[0], pData[1], pData[2], pData[3]);
				pData += 4;
				nParse_SiglMsgLen += 4;
				if(nParse_SiglMsgLen >= nSigPMMsgDataLenth)
				{
					continue;
				}
			}
			
		}

		TRACE("pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode=[%s] \
			fRTOutputVolt=[%.1f] fRTOutputCurr=[%.1f] fRect_DCDCVer=[%.1f] fRect_PFCVer=[%.1f]\n",\
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].byBarCode,\
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRTOutputVolt,\
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRTOutputCurr,\
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRect_DCDCVer,\
			pCCU->strLSYSInfo.singleRectData.arrRect[emRectType][n].fRect_PFCVer);

	}
 
	return 0;
}

int Parse_CFGS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	BYTE bySubCmd = pData[0];
	BYTE rtn = pData[1];
	TRACE("Received rep for CFGS[%02X], rtn= %d\n", bySubCmd, rtn);
	if(rtn != _RTN_OK)
	{
		return -1;
	}

	return 0;
}

int Parse_UCTL(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	BYTE bySubCmd = pData[0];
	BYTE rtn = pData[1];
	TRACEX("Received rep for UCTL[%02X], rtn= %d\n", bySubCmd, rtn);
	if(rtn != _RTN_OK)
	{
		return -1;
	}

	return 0;
}


/*********************************************************************************
*  
*  FUNCTION NAME : 0x05 To Get PM-IMSU-D CAN OrgData
*  CREATOR       : John
*  CREATED DATE  : 2024-1-2 11:22:53
*  DESCRIPTION   :  
* 
*  	Reply:		  
*
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_GCAN(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID)
{
	CCU_UPGRADE_PARAM* pUpgradeParam = &pCCU->tagCCUUpgradeParam;
	
	// printf("To GCAN, emUpgradeStatus == %d\n", pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus);

	if(pUpgradeParam->tagMstUpgradeParam.emUpgradeStatus == UPGRADE_IDLE)
	{
		USHORT	usDataLen = (USHORT)Bytes2Ulong(pData, 2);
		pData = pData + 2;

		int n = 0, m = 0, nOffset = 0;
		char szTime[16];

		if(pCCU->blobCanMsg.nTotalMsgNum >= MAX_CAN_CYCLETIMES)
		{
			pCCU->blobCanMsg.nTotalMsgNum = MAX_CAN_CYCLETIMES - 1;
		}
		pCCU->blobCanMsg.nTotalMsgNum = MAX(pCCU->blobCanMsg.nTotalMsgNum, 0);
		
		pCCU->blobCanMsg.tm_Recv = _NOW_;

		for(n = 0; n < MIN(usDataLen, MAX_CAN_MSGLEN) ; n++)
		{	
			pCCU->blobCanMsg.byCanMsg[pCCU->blobCanMsg.nTotalMsgNum][n] = pData[n]; 
		}
		pCCU->blobCanMsg.nTotalMsgNum++;

		TRACE("0x05:Received Rep Parse_GCAN Get usDataLen=[%d] usLenID=[%d] nTotalMsgNum=[%d]\n", usDataLen, usLenID, pCCU->blobCanMsg.nTotalMsgNum);
		
		/*
		for(n = 0; n < MIN(usDataLen, MAX_CAN_MSGLEN) ; n++)
		{
			printf("%02X", pData[n]);
			m++;
			if(m==12)
			{
				m = 0;
				printf("\n");
			}
		}
		printf("\n");

		TimeToString(time(NULL), "%H:%M:%S", szTime, sizeof(szTime));
		*/

		Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 205, (void*)&pCCU->blobCanMsg, sizeof(A_BLOB_CCU2PCSCAN_MSG), BLOB_TYPE_CCU2PCSCAN_MSG);
	}
	else // bmu03 升级pcs时通过此透传协议
	{
		pData += 2;
			// printf("To GCAN UPDATE, emUpdateStep = %d\n", pUpgradeParam->strEmsUpCcuInfo.emUpdateStep);
		if(pData[11] == 0xA2 && pUpgradeParam->strEmsUpCcuInfo.uSlience == 1)
		{
			pUpgradeParam->strEmsUpCcuInfo.uSlience = 2;
		}
		// else if(pData[0] == 0x02 && pData[1] == 0x7F && (pData[11] == 0x01 || pData[11] == 0x02))
		// {
		// 	pUpgradeParam->strEmsUpCcuInfo.uTrigger1 = 1;
		// }
		else if(pData[0] == 0x01F && (pData[1] == 0x09 || pData[1] == 0x49))
		{
			if(pData[3] != pUpgradeParam->strEmsUpCcuInfo.byCurrentModuleId)
			{
				pUpgradeParam->strEmsUpCcuInfo.byCurrentModuleId = pData[3];
			}
			if(pData[11] == 0x01 || pData[11] == 0x02)
			{
				pUpgradeParam->strEmsUpCcuInfo.uAddr = (UINT)(pData[4]<<24 | pData[5]<<16 | pData[6]<<8 | pData[7]);
				pUpgradeParam->strEmsUpCcuInfo.uAddrCalc = pUpgradeParam->strEmsUpCcuInfo.uAddr;
				pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = pUpgradeParam->strEmsUpCcuInfo.uAddr;
				pUpgradeParam->strEmsUpCcuInfo.usReadLen = (USHORT)(pData[8]<<8 | pData[9]);
				pUpgradeParam->strEmsUpCcuInfo.usReadLenCalc = pUpgradeParam->strEmsUpCcuInfo.usReadLen ;
				pUpgradeParam->strEmsUpCcuInfo.uTrigger2 = pData[11];

				if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_DCDC)
				{
					pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x10; // 10~2F
				}
				else if(pCCU->blobUpdCmd.emUpdtType == UPDT_TP_PFC)
				{
					pUpgradeParam->strEmsUpCcuInfo.bySendStartAddr = 0x50; // 50~6F
				}

				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep = DOWNLOAD_SEND;

				// printf("GCAN read addr=%d, len=%d\n",
				// 	pUpgradeParam->strEmsUpCcuInfo.uAddr,
				// 	pUpgradeParam->strEmsUpCcuInfo.usReadLen);
			}
			else if(pData[11] == 0x03) // 仅有 0x01 02 03三种情况
			{
				pUpgradeParam->strEmsUpCcuInfo.uAddr03 = (UINT)(pData[4]<<24 | pData[5]<<16 | pData[6]<<8 | pData[7]);
				pUpgradeParam->strEmsUpCcuInfo.nHasSendLen = pUpgradeParam->strEmsUpCcuInfo.uAddr;
				pUpgradeParam->strEmsUpCcuInfo.usReadLen03 = (USHORT)(pData[8]<<8 | pData[9]);
				pUpgradeParam->strEmsUpCcuInfo.uTrigger2 = pData[11];
				pUpgradeParam->strEmsUpCcuInfo.emUpdateStep == DOWNLOAD_SEND;
			}
		}
	}

	// int i=0;

	// for(i = 0; i < 12; ++i)
	// {
	// 	printf("%02X ", pData[i]);
	// }
	// printf("\n");


	return 0;
}

