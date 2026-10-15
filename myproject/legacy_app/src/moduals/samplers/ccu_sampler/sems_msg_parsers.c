// #define _TRACE_PRINT_ // SEMS
#include "sems_sampler_main.h"

int Parse_CFGG_SEMS(ROUGH_CCU_DATA *pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	reverseBytes(pData, usDataLen);
	SEMS_61_CFGG_DATA_INFO* pCFGG = (SEMS_61_CFGG_DATA_INFO*)pData;
	reverseBytes((BYTE*)pCFGG->szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	if (ElapseMSBcdTimeStmp(&pCFGG->tmStamp) >= MAX_MS_DIFF_4CALIBRATETIME)
	{
		TRACEW("Parse_CFGG_SEMS Send_6202CFGS_RTC >= %d", MAX_MS_DIFF_4CALIBRATETIME);
		Send_6202CFGS_RTC(n);
	}
	pCCU->byVerSEMSProt_Client = pCFGG->byVerSEMSProt; // LJH250630 SEMSProtVer
	// CCU_BOOTUP_INFO* pBoot = &pCCU->blobBackendCfg blobChargerSpecificSett[n].ccuSettings;
	// pCCU->blobChargerSpecificSett[nDispIdx]
	A_BLOB_SEMS_CLIENTINFO *pClient = &s_MMClients[n];
	pClient->idxClientRecv = pCFGG->idxClient;
	pClient->emClient_RegisteredStatus = CCU_REGISTERED_OK;
	pClient->bCommFailSemsClient = FALSE;
	pClient->tmLastRcvHeatbeat = _NOW_;
	pClient->bNeedRegister = FALSE;
	pClient->tmSyncCFGG = _NOW_;
	pCCU->emCCUState = CCU_STATE_IDLE;

	// LJH251106 同步主从参数
	s_MMClients[n].CFGG = *pCFGG;
	if (pCCU->emsMgmtData.nStopSOC_ChrgSet_SEMS != s_MMClients[n].CFGG.nStopSOC_ChrgSet)
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_ChrgSet = 1;
	}
	else
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_ChrgSet = 0;
	}

	if (pCCU->emsMgmtData.nStopSOC_DiscSet_SEMS != s_MMClients[n].CFGG.nStopSOC_DiscSet)
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_DiscSet = 1;
	}
	else
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bStopSOC_DiscSet = 0;
	}
	
	if (pCCU->lcCfg.bSendCmdByLC1stOnly != s_MMClients[n].CFGG.bSendCmdByLC1stOnly)
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bSendCmdByLC1stOnlySet = 1;
	}
	else
	{
		pCCU->semsSMInfos[n].hasParamSetCFGS.bSendCmdByLC1stOnlySet = 0;
	}
	
	TRACEI("Parse_CFGG_SEMS n:%d byVerSEMSProt_Client:%d nSlaveIP:%04X idxClient:%u szIMMU2_SWVer:%s "
		"tmStamp:%s SEMS_LSYS_Period:%u StopSOC:%d-%d data:%x",
		n, pCCU->byVerSEMSProt_Client, pCFGG->nSlaveIP, pCFGG->idxClient, pCFGG->szIMMU2_SWVer,
		printableBcdTimeStmp(&pCFGG->tmStamp), pCFGG->SEMS_LSYS_Period, pCFGG->nStopSOC_ChrgSet, pCFGG->nStopSOC_DiscSet,
		pCCU->semsSMInfos[n].hasParamSetCFGS.data);
	return 0;
}
// 1 (SubCmd) +1(RTN)
int Parse_CFGS_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(pCCU);
	UNUSED(usDataLen);
	UNUSED(n);
	BYTE bySubCmd = pData[0]; //do we need judge subCmd also? will consider later
	BYTE rtn = pData[1];
	TRACEI("Parse_CFGS_SEMS rep for CFGS[%02X], rtn= %d", bySubCmd, rtn);
	if(rtn != _RTN_OK)
	{
		return -1;
	}
	return 0;
}

int Parse_UCTL_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(pCCU);
	UNUSED(usDataLen);
	UNUSED(n);

	BYTE bySubCmd = pData[0]; //do we need judge subCmd also? will consider later
	BYTE rtn = pData[1];
	TRACEI("Parse_UCTL_SEMS [%02X], rtn= %d", bySubCmd, rtn);
	if(rtn != _RTN_OK)
	{
		return -1;
	}

	return 0;
}

int Parse_LSYS_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	size_t sizeofLSYSSum = 0;
	SEMS_B6_LSYS_DATA_INFO* pB6LSYS;
	// LJH250930 加快自动充放响应,standbySetReq
	sizeofLSYSSum = sizeof(SEMS_B6_LSYS_DATA_INFO);
	pB6LSYS = (SEMS_B6_LSYS_DATA_INFO*)reverseBytes(pData, (int)sizeofLSYSSum);
	pCCU->strLSYSInfo.branchNodeData[n].b6LSYS_SEMS = *pB6LSYS;
	TRACE("Parse_LSYS_SEMS sizeofLSYSSum:%d <%s>", sizeofLSYSSum, printableBytes(pData, (int)sizeofLSYSSum));
	// pB6LSYS->llTimeMs;
	// pB6LSYS->emCCUStateSum;
	// pB6LSYS->emPMCtrlModeSum;
	pCCU->strLSYSInfo.branchNodeData[n].emACDC_WorkStatu = pB6LSYS->emACDC_WorkStatuSum;

	pCCU->strLSYSInfo.branchNodeData[n].nSOC =            (UINT)pB6LSYS->fSOCSum;
	// pB6LSYS->fCabActivePower;
	pCCU->strLSYSInfo.branchNodeData[n].fActvPwr_PCSGrp = pB6LSYS->fActvPwr_PCSGrpSum;
	pCCU->strLSYSInfo.branchNodeData[n].fQ_PCSGrp =       pB6LSYS->fQ_PCSGrpSum;
	pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_Chrg_Pwr_Req = pB6LSYS->fMax_Chrg_PwrAbilitySum;

	pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_DisChrg_Pwr = pB6LSYS->fMax_DisChrg_PwrAbilitySum;
	// pB6LSYS->fChrgedEnergySum
	// pB6LSYS->fRealTime_MaxCurr;
	pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Volt = pB6LSYS->fACDC_RealTime_VoltSum;
	pCCU->strLSYSInfo.branchNodeData[n].fACDC_RealTime_Curr = pB6LSYS->fACDC_RealTime_CurrSum;

	s_MMClients[n].branchNodeData_SEMS = pCCU->strLSYSInfo.branchNodeData[n];
#ifdef _TRACE_PRINT_
	SEMS_B6_LSYS_DATA_INFO* pClientSum = pB6LSYS;
	TRACEI("Parse_LSYS_SEMS n:%d usDataLen:%d byVerSEMSProt_Client:%d idxClient:%u tmStamp:%u %X %s "
		"ClientSum emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d "
        "fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
		"\nfMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySum0:%.1f %.1f AbilitySumA:%.1f %.1f "
		"fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
		"f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f bStandby_BLLSum:%d",
		n, usDataLen, pCCU->byVerSEMSProt_Client, pB6LSYS->idxClient, ElapseMSBcdTimeStmp(&pB6LSYS->tmStamp), ElapseMSBcdTimeStmp(&pB6LSYS->tmStamp), printableBcdTimeStmp(&pB6LSYS->tmStamp),
		pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, 
		pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum,
		pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum, pClientSum->fMax_Chrg_PwrAbilitySum0,
		pClientSum->fMax_DisChrg_PwrAbilitySum0, pClientSum->fMax_Chrg_PwrAbilitySumA, pClientSum->fMax_DisChrg_PwrAbilitySumA,
		pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
		pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum, pClientSum->bStandby_BLLSum);
#endif

	s_MMClients[n].tmLastRcvHeatbeat = _NOW_;
	if (ElapseMSBcdTimeStmp(&pB6LSYS->tmStamp) >= MAX_MS_DIFF_4CALIBRATETIME)
	{
		TRACEW("Parse_LSYS_SEMS Send_6202CFGS_RTC >= %d", MAX_MS_DIFF_4CALIBRATETIME);
		Send_6202CFGS_RTC(n);
	}
	
	return 0;
}
// KYA20250802
// static int SParse_LDEV_101(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
// {
// 	UNUSED(usDataLen);
// 	// static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
// 	static const int SIZE_SINGLE = sizeof(SC6_LDEV_101_Data);
// 	LCCfg* pLCCfg = &pCCU->lcCfg;
// 	// int iBranch = 0;
// 	int iEsc = 0;
// 	UINT uIdx4Blob = 0;

// 	A_BLOB_CMD101 blobCmd101;
// 	ZERO_POBJS(&blobCmd101, 1);
// 	VAR_VALUE varV;
// 	varV.blobValue = (BYTE*)&blobCmd101;
// 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3401, &varV, sizeof(A_BLOB_CMD101));
// 	// SC6_LDEV_101_Data* pBranchDatas_Blob = blobCmd101.branchDatas;
// 	SC6_LDEV_101_Data* pESSRealTimeStatus_Blob =    blobCmd101.SemsCMD101Data;
// 	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);

// 	// 2. top#1 没有BranchData
// 	// 3. pESSRealTimeStatus
// 	SC6_LDEV_101_Data* pESSRealTimeStatus;
// 	switch (pLCCfg->lcType)
// 	{
// 	case LC_TYPE_1st:
// 	{
// 		if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
// 		{
// 			for (iEsc=0; iEsc<pHead->byDevNum; iEsc++)
// 			{
// 				pESSRealTimeStatus = (SC6_LDEV_101_Data*)(pData + iEsc * SIZE_SINGLE);
// 				reverseBytes((BYTE*)pESSRealTimeStatus, SIZE_SINGLE);
// 				uIdx4Blob = GetIdx0_LC4cab(pESSRealTimeStatus->dev_id);
// 				if (uIdx4Blob > MAX_NUM_ESC)
// 				{
// 					TRACEE("SParse_LDEV_151 out of range uIdx4Blob:%u", uIdx4Blob);
// 					continue;
// 				}
// 				pESSRealTimeStatus_Blob[uIdx4Blob] = *pESSRealTimeStatus;
// 				TRACED("SParse_LDEV_151 pESSRealTimeStatus dev_id:%u uIdx4Blob:%u ", pESSRealTimeStatus->dev_id, uIdx4Blob);
// 			}
// 		}
// 	}
// 	break;
	
// 	default:
// 		break;
// 	}
	
// 	// to UpperLC or clound
// 	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3401,
// 		(void*)&blobCmd101, sizeof(A_BLOB_CMD101), BLOB_TYPE_SEMS_CMD101);

// 	return 0;
// }
// KYA20250802
// static int SParse_LDEV_102(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
// {
// 	static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
// 	static const int SIZE_SINGLE = sizeof(SC6_LDEV_102_Data);
// 	LCCfg* pLCCfg = &pCCU->lcCfg;
// 	int iBranch = 0, iEsc = 0;
// 	UINT uIdx4Blob = 0;

// 	A_BLOB_CMD102 blobCmd102;
// 	ZERO_POBJS(&blobCmd102, 1);
// 	VAR_VALUE varV;
// 	varV.blobValue = (BYTE*)&blobCmd102;
// 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3402, &varV, sizeof(A_BLOB_CMD102));
// 	SC6_LDEV_102_Data* pBranchDatas_Blob = blobCmd102.branchDatas;
// 	SC6_LDEV_102_Data* pBatteryCabinetStatus_Blob =    blobCmd102.SemsCMD102Data;
// 	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);

// 	// 2. top#1 没有BranchData
// 	// 3. pBatteryCabinetStatus
// 	SC6_LDEV_102_Data* pBatteryCabinetStatus;
// 	switch (pLCCfg->lcType)
// 	{
// 	case LC_TYPE_1st:
// 	{
// 		if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
// 		{
// 			for (iEsc=0; iEsc<pHead->byDevNum; iEsc++)
// 			{
// 				pBatteryCabinetStatus = (SC6_LDEV_102_Data*)(pData + iEsc * SIZE_SINGLE);
// 				reverseBytes((BYTE*)pBatteryCabinetStatus, SIZE_SINGLE);
// 				uIdx4Blob = GetIdx0_LC4cab(pBatteryCabinetStatus->dev_id);
// 				if (uIdx4Blob > MAX_NUM_ESC)
// 				{
// 					TRACEE("SParse_LDEV_152 out of range uIdx4Blob:%u", uIdx4Blob);
// 					continue;
// 				}
// 				pBatteryCabinetStatus_Blob[uIdx4Blob] = *pBatteryCabinetStatus;
// 				TRACED("SParse_LDEV_152 pBatteryCabinetStatus dev_id:%u uIdx4Blob:%u ", pBatteryCabinetStatus->dev_id, uIdx4Blob);
// 			}
// 		}
// 	}
// 	break;
	
// 	default:
// 		break;
// 	}
	
// 	// to UpperLC or clound
// 	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3402,
// 		(void*)&blobCmd102, sizeof(A_BLOB_CMD102), BLOB_TYPE_SEMS_CMD102);

// 	return 0;
// }

int Parse_PCS_INFO(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	reverseBytes(pData, usDataLen);
	PCS_INFO_DATA* pPCS_INFO = (PCS_INFO_DATA*)pData;
	A_BLOB_SEMS_CLIENTINFO *pClient = &s_MMClients[n];
	pClient->stPcsInfoData.nPCSNum = pPCS_INFO->nPCSNum;
	int i;
	for(i = 0; i < pClient->stPcsInfoData.nPCSNum; ++i)
	{
		if(i >= MAX_PSC_NUM)
		{
			break;
		}
		memcpy(pClient->stPcsInfoData.szBar_code[i], pPCS_INFO->szBar_code[i], MAX_BUFFER_LNE);
		memcpy(pClient->stPcsInfoData.szVersion[i], pPCS_INFO->szVersion[i], MAX_BUFFER_LNE);
		pClient->stPcsInfoData.szBar_code[i][MAX_BUFFER_LNE - 1] = '\0';
		pClient->stPcsInfoData.szVersion[i][MAX_BUFFER_LNE - 1] = '\0';
		// printf("Bar_code = %s, Version = %s\n", pClient->stPcsInfoData.szBar_code[i], pClient->stPcsInfoData.szVersion[i]);
	}
	return 0;
}


static int SParse_LDEV_103(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(usDataLen);
	UNUSED(n);
	UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
	if (uClientIdx >= MAX_NUM_ESC)
		return FALSE; 

	LCCfg* pLCCfg = &pCCU->lcCfg;
	
	VAR_VALUE varV;
	
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
	{
		A_BLOB_SEMS_CMD103 dataBloCmd103;
		ZERO_POBJS(&dataBloCmd103, 1);
		varV.blobValue = (BYTE*)&dataBloCmd103;
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3403, &varV, sizeof(A_BLOB_SEMS_CMD103));
		dataBloCmd103.emBlobType = BLOB_TYPE_SEMS_CMD103;

		int battGroupNum = (int)pHead->byDevNum;

		dataBloCmd103.clientBMS[uClientIdx].iDevID = (int)pHead->idxClient;
		dataBloCmd103.clientBMS[uClientIdx].iBattGroupNum = battGroupNum;

		int i = 0;
		char * pDataPos = (char*)pData;
		for (i = 0; i < battGroupNum; ++i)
		{
			SC6_LDEV_103_Data* pBmsData = &dataBloCmd103.clientBMS[uClientIdx].bmsData[i];

			memcpy(pBmsData, pDataPos, sizeof(SC6_LDEV_103_Data));
			reverseBytes((BYTE*)pBmsData, sizeof(SC6_LDEV_103_Data));

			pDataPos += sizeof(SC6_LDEV_103_Data);
		}

		// to UpperLC or clound
		Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3403, (void*)&dataBloCmd103, sizeof(A_BLOB_SEMS_CMD103), BLOB_TYPE_SEMS_CMD103);

	}
	break;
	
	default:
		break;
	}
	
	return 0;
}

static int SParse_LDEV_104(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(usDataLen);
	UNUSED(n);
	UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
	if (uClientIdx >= MAX_NUM_ESC)
		return FALSE; 

	LCCfg* pLCCfg = &pCCU->lcCfg;
	
	VAR_VALUE varV;
	
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
	{
		A_BLOB_SEMS_CMD104 dataBloCmd104;
		ZERO_POBJS(&dataBloCmd104, 1);
		varV.blobValue = (BYTE*)&dataBloCmd104;
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3404, &varV, sizeof(A_BLOB_SEMS_CMD104));
		dataBloCmd104.emBlobType = BLOB_TYPE_SEMS_CMD104;

		int battGroupNum = pHead->byDevNum;

		dataBloCmd104.clientPowerCalc[uClientIdx].iDevID = (int)pHead->idxClient;
		dataBloCmd104.clientPowerCalc[uClientIdx].iBattGroupNum = battGroupNum;

		int i = 0;
		char * pDataPos = (char*)pData;
		for (i = 0; i < battGroupNum && i < MAX_BATTERY_NUM; ++i)
		{
			SC6_LDEV_104_Data* pPowerCalcData = &dataBloCmd104.clientPowerCalc[uClientIdx].powerCalcData[i];
			memcpy(pPowerCalcData, pDataPos, sizeof(SC6_LDEV_104_Data));
			reverseBytes((BYTE*)pPowerCalcData, sizeof(SC6_LDEV_104_Data));
			
			pDataPos += sizeof(SC6_LDEV_104_Data);
		}

		// to UpperLC or clound
		Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3404, (void*)&dataBloCmd104, sizeof(A_BLOB_SEMS_CMD104), BLOB_TYPE_SEMS_CMD104);

		ZERO_POBJS(&dataBloCmd104, 1);
		varV.blobValue = (BYTE*)&dataBloCmd104;
		pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3404, &varV, sizeof(A_BLOB_SEMS_CMD104));
		
		dataBloCmd104.clientPowerCalc[uClientIdx].iDevID = 1;
	}
	break;
	
	default:
		break;
	}
	
	return 0;
}
// KYA20250802
// static int SParse_LDEV_106(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
// {
// 	static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
// 	static const int SIZE_SINGLE = sizeof(A_BLOB_SINGLE_RECT_INFO_106);
// 	LCCfg* pLCCfg = &pCCU->lcCfg;
// 	int iBranch = 0, iEsc = 0;
// 	UINT uIdx4Blob = 0;

// 	A_BLOB_CMD106 blobCmd106;
// 	ZERO_POBJS(&blobCmd106, 1);
// 	VAR_VALUE varV;
// 	varV.blobValue = (BYTE*)&blobCmd106;
// 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3406, &varV, sizeof(A_BLOB_CMD106));
// 	A_BLOB_SINGLE_RECT_INFO_106* pBranchDatas_Blob = blobCmd106.branchDatas;
// 	A_BLOB_SINGLE_RECT_INFO_106* pPCSStatusInfo_Blob =    blobCmd106.SemsCMD106Data;
// 	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);

// 	// 2. top#1 没有BranchData
// 	// 3. pPCSStatusInfo
// 	A_BLOB_SINGLE_RECT_INFO_106* pPCSStatusInfo;
// 	switch (pLCCfg->lcType)
// 	{
// 	case LC_TYPE_1st:
// 	{
// 		if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
// 		{
// 			for (iEsc=0; iEsc<pHead->byDevNum; iEsc++)
// 			{
// 				pPCSStatusInfo = (A_BLOB_SINGLE_RECT_INFO_106*)(pData + iEsc * SIZE_SINGLE);
// 				reverseBytes((BYTE*)pPCSStatusInfo, SIZE_SINGLE);
// 				uIdx4Blob = GetIdx0_LC4cab(pPCSStatusInfo->dev_id);
// 				if (uIdx4Blob > MAX_NUM_ESC)
// 				{
// 					TRACEE("SParse_LDEV_156 out of range uIdx4Blob:%u", uIdx4Blob);
// 					continue;
// 				}
// 				pPCSStatusInfo_Blob[uIdx4Blob] = *pPCSStatusInfo;
// 				TRACED("SParse_LDEV_156 pPCSStatusInfo dev_id:%u uIdx4Blob:%u "
// 					"ac_voltsA:%.1f ac_currentdemand:%.1f",
// 					pPCSStatusInfo->dev_id, uIdx4Blob, pPCSStatusInfo->phase_volts[2], pPCSStatusInfo->ac_currentdemand);
// 			}
// 		}
// 	}
// 	break;
	
// 	default:
// 		break;
// 	}
	
// 	// to UpperLC or clound
// 	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3406,
// 		(void*)&blobCmd106, sizeof(A_BLOB_CMD106), BLOB_TYPE_SEMS_CMD106);

// 	return 0;
// }
// KYA20250802
// static int SParse_LDEV_108(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
// {
// 	static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
// 	static const int SIZE_SINGLE = sizeof(SC6_LDEV_108_Data);
// 	LCCfg* pLCCfg = &pCCU->lcCfg;
// 	int iBranch = 0, iEsc = 0;
// 	UINT uIdx4Blob = 0;

// 	A_BLOB_CMD108 blobCmd108;
// 	ZERO_POBJS(&blobCmd108, 1);
// 	VAR_VALUE varV;
// 	varV.blobValue = (BYTE*)&blobCmd108;
// 	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3408, &varV, sizeof(A_BLOB_CMD108));
// 	SC6_LDEV_108_Data* pBranchDatas_Blob = blobCmd108.branchDatas;
// 	SC6_LDEV_108_Data* pESSDeviceVer_Blob =    blobCmd108.SemsCMD108Data;
// 	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);

// 	// 2. top#1 没有BranchData
// 	// 3. pESSDeviceVer
// 	SC6_LDEV_108_Data* pESSDeviceVer;
// 	switch (pLCCfg->lcType)
// 	{
// 	case LC_TYPE_1st:
// 	{
// 		if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
// 		{
// 			for (iEsc=0; iEsc<pHead->byDevNum; iEsc++)
// 			{
// 				pESSDeviceVer = (SC6_LDEV_108_Data*)(pData + iEsc * SIZE_SINGLE);
// 				reverseBytes((BYTE*)pESSDeviceVer, SIZE_SINGLE);
// 				uIdx4Blob = GetIdx0_LC4cab(pESSDeviceVer->serial_id);
// 				if (uIdx4Blob > MAX_NUM_ESC)
// 				{
// 					TRACEE("SParse_LDEV_158 out of range uIdx4Blob:%u", uIdx4Blob);
// 					continue;
// 				}
// 				pESSDeviceVer_Blob[uIdx4Blob] = *pESSDeviceVer;
// 				TRACED("SParse_LDEV_158 pESSDeviceVer dev_id:%u uIdx4Blob:%u "
// 					"ac_voltsA:%.1f ac_currentdemand:%.1f",
// 					pESSDeviceVer->dev_id, uIdx4Blob, pESSDeviceVer->phase_volts[2], pESSDeviceVer->ac_currentdemand);
// 			}
// 		}
// 	}
// 	break;
	
// 	default:
// 		break;
// 	}
	
// 	// to UpperLC or clound
// 	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3408,
// 		(void*)&blobCmd108, sizeof(A_BLOB_CMD108), BLOB_TYPE_SEMS_CMD108);

// 	return 0;
// }
static BOOL SParse_LDEV_109(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int iWhichClient)
{
	UNUSED(usDataLen);
	UNUSED(iWhichClient);
	UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
	if (uClientIdx >= MAX_NUM_ESC)
		return FALSE; 

	A_BLOB_SEMS_CMD109 dataBloCmd109;
	LCCfg* pLCCfg = &pCCU->lcCfg;
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
		{
			VAR_VALUE varV;
			varV.blobValue = (BYTE*)&dataBloCmd109;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3409, &varV, sizeof(A_BLOB_SEMS_CMD109));

			dataBloCmd109.emBlobType = BLOB_TYPE_SEMS_CMD109;

			SESS_ACTIVE_ALARMS* pClientAlarm = &dataBloCmd109.clientActiveAlarms[uClientIdx];
			pClientAlarm->iDevID = (int)pHead->idxClient;
			pClientAlarm->iAlarmCount = pHead->byDevNum;

			size_t iSingleAlarmLen = sizeof(SC6_LDEV_109);

			SC6_LDEV_109 *pSC6_LDEV_109 = (SC6_LDEV_109 *)pData;
			int i = 0; 
			for (i = 0; i < pHead->byDevNum && i < MAX_QUERY_ACTV_ALM; ++i)
			{
				memcpy(&pClientAlarm->activeAlms[i], pSC6_LDEV_109, iSingleAlarmLen);
				reverseBytes((BYTE*)&pClientAlarm->activeAlms[i], (int)iSingleAlarmLen);

				++pSC6_LDEV_109;
			}
			
			Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3409, (void*)&dataBloCmd109, sizeof(A_BLOB_SEMS_CMD109), BLOB_TYPE_SEMS_CMD109);
		}
		break;
	
	default:
		break;
	}
	return TRUE;
}

static BOOL SParse_LDEV_110(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int iWhichClient)
{
	UNUSED(usDataLen);
	UNUSED(iWhichClient);
	UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
	if (uClientIdx >= MAX_NUM_ESC)
		return FALSE; 

	LCCfg* pLCCfg = &pCCU->lcCfg;
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
		{
			A_BLOB_SEMS_CMD110 dataBloCmd110;
			VAR_VALUE varV;
			varV.blobValue = (BYTE*)&dataBloCmd110;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3410, &varV, sizeof(A_BLOB_SEMS_CMD110));

			dataBloCmd110.emBlobType = BLOB_TYPE_SEMS_CMD110;
			dataBloCmd110.clientBattCell[uClientIdx].iDevID = (int)pHead->idxClient;
			dataBloCmd110.clientBattCell[uClientIdx].iBattGroupNum = pHead->byDevNum;

			BYTE * pDataPos = pData;
			int n = 0;
			for (n = 0; n < pHead->byDevNum && n < MAX_BATTERY_NUM; ++n)
			{
				SC6_LDEV_110 devInfo110;
				memcpy(&devInfo110, pDataPos, sizeof(SC6_LDEV_110));
				reverseBytes((BYTE*)&devInfo110, sizeof(SC6_LDEV_110));

				int battIdx = devInfo110.batt_idx;
				int iCellNum = devInfo110.iCellNum;

				if (battIdx < 0 || battIdx >= MAX_SINGLE_STBATT_NUM)
					return FALSE;

				if (iCellNum <= 0 || iCellNum > A_CABINET_CELL_MAX_NUM)
					return FALSE;

				dataBloCmd110.clientBattCell[uClientIdx].battGroupCell[battIdx].usCellNumPerGroup = (USHORT)iCellNum;

				pDataPos += sizeof(SC6_LDEV_110);
				SC6_LDEV_CELL_110* pCellInfo = (SC6_LDEV_CELL_110*)pDataPos;
				size_t iCellInfoLen = sizeof(SC6_LDEV_CELL_110);
				int i = 0; 
				for (i = 0; i < devInfo110.iCellNum; ++i)
				{
					memcpy(&dataBloCmd110.clientBattCell[uClientIdx].battGroupCell[battIdx].battCellInfo[i], pCellInfo, iCellInfoLen);
					reverseBytes((BYTE*)&dataBloCmd110.clientBattCell[uClientIdx].battGroupCell[battIdx].battCellInfo[i], (int)iCellInfoLen);

					pDataPos += iCellInfoLen;
					pCellInfo = (SC6_LDEV_CELL_110*)pDataPos;
				}

			}

			Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3410, (void*)&dataBloCmd110, sizeof(A_BLOB_SEMS_CMD110), BLOB_TYPE_SEMS_CMD110);
		}
		break;
	
	default:
		break;
	}

	return 0; 
}

static BOOL SParse_LDEV_202(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int iWhichClient)
{
	UNUSED(usDataLen);
	UNUSED(iWhichClient);
	UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
	if (uClientIdx >= MAX_NUM_ESC)
		return FALSE; 

	LCCfg* pLCCfg = &pCCU->lcCfg;
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
		{
			A_BLOB_SEMS_CMD202 dataBloCmd202;
			ZERO_POBJS(&dataBloCmd202, 1);
			VAR_VALUE varV;
			varV.blobValue = (BYTE*)&dataBloCmd202;
			pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3411, &varV, sizeof(A_BLOB_SEMS_CMD202));

			dataBloCmd202.emBlobType = BLOB_TYPE_SEMS_CMD202;
			dataBloCmd202.semsEleStatist[uClientIdx].iDevID = (int)pHead->idxClient;
			
			memcpy(&dataBloCmd202.semsEleStatist[uClientIdx].eleStatist, pData, sizeof(SC6_LDEV_202));
			
			reverseBytes((BYTE*)&dataBloCmd202.semsEleStatist[uClientIdx].eleStatist, sizeof(SC6_LDEV_202));

			Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3411, (void*)&dataBloCmd202, sizeof(A_BLOB_SEMS_CMD202), BLOB_TYPE_SEMS_CMD202);	

		}
		break;
	
	default:
		break;
	}

	return TRUE;
}

// LJH250428 cmd155
static int SParse_LDEV_155(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(usDataLen);
	UNUSED(n);
	static const int SIZE_BranchData = sizeof(SC6LDEV_CMD155_BranchData);
	static const int SIZE_ESCData = sizeof(SC6LDEV_CMD155_ESCData);
	LCCfg* pLCCfg = &pCCU->lcCfg;
	int /*iBranch = 0,*/ iEsc = 0;

	A_BLOB_CMD155 blobCmd;
	ZERO_POBJS(&blobCmd, 1);
	VAR_VALUE varV;
	varV.blobValue = (BYTE*)&blobCmd;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3405, &varV, sizeof(A_BLOB_CMD155));
	// SC6LDEV_CMD155_BranchData* pBranchDatas_Blob = blobCmd.branchDatas;
	SC6LDEV_CMD155_ESCData* pESCDatas_Blob =       blobCmd.escDatas;
	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);
	
	// // SC6LDEV_CMDHead* pHead = &blobCmd.cmdHead;
	// CMD155_BranchAndESCData* pBranchAndESCDatas = blobCmd.branchAndESCDatas;
	// for (iBranch=0; iBranch<MAX_NUM_BRANCH; iBranch++)
	// {
	// 	if (!pBranchAndESCDatas[iBranch].bValidBranch)
	// 	{
	// 		continue;
	// 	}
		
	// 	for (iEsc=0; iEsc<MAX_NUM_ESC; iEsc++)
	// 	{
	// 		if (!pBranchAndESCDatas[iBranch].bValidEscs[iEsc])
	// 		{
	// 			continue;
	// 		}
	// 		SC6LDEV_CMD155_ESCData* pEscDatas = pBranchAndESCDatas[iBranch].escDatas;
	// 		// pEscDatas[iEsc].ac_currsA
	// 	}
	// }

	// 2.SC6LDEV_CMD155_BranchData #1 没有BranchData
	SC6LDEV_CMD155_BranchData* pBranchData = (SC6LDEV_CMD155_BranchData*)(pData);
	reverseBytes((BYTE*)pBranchData, SIZE_BranchData);
	// UINT idxBranch4Blob = GetIdx0_LC2nd(pBranchData->dev_id);
	// if (idxBranch4Blob >= pCCU->lcCfg.nBranchNum)
	// {
	// 	TRACEE("SParse_LDEV_155 return 0 idxBranch4Blob:%u > nBranchNum:%u", idxBranch4Blob, pCCU->lcCfg.nBranchNum);
	// 	return 0;
	// }

	// if (idxBranch4Blob > 0)
	// {
	// 	idxBranch4Blob -= 1;
	// }
	// TRACED("SParse_LDEV_155 idxBranch4Blob:%d ac_volts:%.1f %.1f %.1f usLenSingle:%u",
	// 	idxBranch4Blob, pBranchData->ac_volts[0], pBranchData->ac_volts[1], pBranchData->ac_volts[2], pBranchData->usLenSingle);

	// pBranchDatas_Blob[idxBranch4Blob] =   *pBranchData;

	// 3.SC6LDEV_CMD155_ESCData
	pData += SIZE_BranchData;
	SC6LDEV_CMD155_ESCData* pESCData;
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
		{
			if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
			{
				pESCData = (SC6LDEV_CMD155_ESCData*)(pData + iEsc * SIZE_ESCData);
				reverseBytes((BYTE*)pESCData, SIZE_ESCData);
				UINT uESC4Blob = GetIdx0_LC4cab(pESCData->dev_id);
				if (uESC4Blob < MAX_NUM_ESC)
				{
					pESCDatas_Blob[uESC4Blob] = *pESCData;
					TRACED("SParse_LDEV_155 pESCData dev_id:%u uESC4Blob:%u "
						"ac_voltsA:%.1f B:%.1f C:%.1f ac_currs:%.1f %.1f %.1f batt_soc:%d ElapseMSBcdTimeStmp:%u %s",
						pESCData->dev_id, uESC4Blob,
						pESCData->ac_volts[2], pESCData->ac_volts[1], pESCData->ac_volts[0],
						pESCData->ac_currs[2], pESCData->ac_currs[1], pESCData->ac_currs[0], pESCData->batt_soc,
						ElapseMSBcdTimeStmp(&pESCData->tmStamp), printableBcdTimeStmp(&pESCData->tmStamp));
				}
				else
				{
					TRACEE("SParse_LDEV_155 dev_id:%u uESC4Blob:%u outof range MAX_NUM_ESC", pESCData->dev_id, uESC4Blob);
				}
			}
			

			// for (iEsc=0; iEsc<pBranchData->byESCNum; iEsc++)
			// {
			// 	pESCData = (SC6LDEV_CMD155_ESCData*)(pData + iEsc * SIZE_ESCData);
			// 	reverseBytes((BYTE*)pESCData, SIZE_ESCData);
			// 	pESCData->comm_statu = LC_COMM_SUCC;
			// 	UINT uESC4Blob = GetIdx0_LC4cab(pESCData->dev_id);
			// 	if (uESC4Blob >= MAX_NUM_ESC)
			// 	{
			// 		TRACEE("SParse_LDEV_155 out of range uESC4Blob:%u", uESC4Blob);
			// 		continue;
			// 	}
			// 	if (iEsc == 0)
			// 	{
			// 		idxStartESC_Blob = uESC4Blob;
			// 	}
			// 	uESC4Blob -= idxStartESC_Blob;
			// 	TRACED("SParse_LDEV_155 pESCData dev_id:%u uESC4Blob:%u idxStartESC_Blob:%u "
			// 		"ac_voltsA:%.1f B:%.1f C:%.1f ac_currs:%.1f %.1f %.1f batt_soc:%d",
			// 		pESCData->dev_id, uESC4Blob, idxStartESC_Blob,
			// 		pESCData->ac_volts[2], pESCData->ac_volts[1], pESCData->ac_volts[0],
			// 		pESCData->ac_currs[2], pESCData->ac_currs[1], pESCData->ac_currs[0], pESCData->batt_soc);

			// 	pESCDatas_Blob[uESC4Blob] = *pESCData;
			// }
		}
		break;
	
	default:
		break;
	}
	
	// to UpperLC or clound
	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3405,
		(void*)&blobCmd, sizeof(A_BLOB_CMD155), BLOB_TYPE_CMD155);

	return 0;
}

// LJH250520 cmd157
static int SParse_LDEV_157(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(usDataLen);
	UNUSED(n);
	// static const int SIZE_HEAD =   sizeof(SC6LDEV_CMDHead);
	static const int SIZE_SINGLE = sizeof(SC6LDEV_CMD157);
	LCCfg* pLCCfg = &pCCU->lcCfg;
	int /*iBranch = 0,*/ iEsc = 0;
	UINT uIdx4Blob = 0;

	A_BLOB_CMD157 blobCmd;
	ZERO_POBJS(&blobCmd, 1);
	VAR_VALUE varV;
	varV.blobValue = (BYTE*)&blobCmd;
	pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3407, &varV, sizeof(A_BLOB_CMD157));
	// SC6LDEV_CMD157* pBranchDatas_Blob = blobCmd.branchDatas;
	SC6LDEV_CMD157* pESCDatas_Blob =    blobCmd.escDatas;
	pLCCfg->lcType_Client = GetLCType(pHead->idxClient);
	// 2. top#1 没有BranchData
	// 3. ESCData
	SC6LDEV_CMD157* pESCData;
	switch (pLCCfg->lcType)
	{
	case LC_TYPE_1st:
	{
		if (pLCCfg->emApplyScenery == AUXMETER_NOT_INSTALLED)
		{
			for (iEsc=0; iEsc<pHead->byDevNum; iEsc++)
			{
				pESCData = (SC6LDEV_CMD157*)(pData + iEsc * SIZE_SINGLE);
				reverseBytes((BYTE*)pESCData, SIZE_SINGLE);
				uIdx4Blob = GetIdx0_LC4cab(pESCData->dev_id);
				if (uIdx4Blob > MAX_NUM_ESC)
				{
					TRACEE("SParse_LDEV_157 out of range uIdx4Blob:%u", uIdx4Blob);
					continue;
				}
				pESCDatas_Blob[uIdx4Blob] = *pESCData;
				TRACE("SParse_LDEV_157 pESCData dev_id:%u uIdx4Blob:%u "
					"ac_voltsA:%.1f ac_currentdemand:%.1f",
					pESCData->dev_id, uIdx4Blob, pESCData->phase_volts[2], pESCData->ac_currentdemand);
			}
		}
	}
	break;
	
	default:
		break;
	}
	
	// to UpperLC or clound
	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3407,
		(void*)&blobCmd, sizeof(A_BLOB_CMD157), BLOB_TYPE_CMD157);

	return 0;
}

static int SParse_LDEV_901(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
	UNUSED(usDataLen);
	UNUSED(n);
	EVENT_FIRMWAREUPDATE_STATUS	evFirmUpdt;
	ZERO_POBJS(&evFirmUpdt, 1);
	int UpgradeStatus = pData[0];
	
	evFirmUpdt.emUpdateStatus = (FIRM_UPDATESTT)UpgradeStatus;
	_EVENT_MSG Even_Status = EVENT_MSG_END;

	switch(evFirmUpdt.emUpdateStatus)
	{
		case FIRMUPDATE_Downloading:
		case EVENT_TYPE_SYSTEM_EXIT:
			Even_Status = EVENT_MSG_BEGIN;
			break;
		case FIRMUPDATE_DownloadFailed:
			Even_Status = EVENT_MSG_END;
			break;
		case FIRMUPDATE_Installing:
		case FIRMUPDATE_Downloaded:
			Even_Status = EVENT_MSG_STATUS_CHANGED;
			break;
	}
	
	SendMsg(pCCU->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, Even_Status, 
						&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
	return 0;
}

//edit by Kuangyue 20251112
static BOOL SParse_LDEV_902(ROUGH_CCU_DATA* pCCU, SC6LDEV_CMDHead* pHead, BYTE* pData, USHORT usDataLen, int n)
{
    UNUSED(usDataLen);
    UNUSED(n);

    UINT uClientIdx = GetIdx0_LC4cab(pHead->idxClient);
    if (uClientIdx >= MAX_NUM_ESC)
        return FALSE;

    A_BLOB_SEMS_CMD109 dataBloCmd902;
    LCCfg* pLCCfg = &pCCU->lcCfg;

    switch (pLCCfg->lcType)
    {
        case LC_TYPE_1st:
        {
            VAR_VALUE varV;
            varV.blobValue = (BYTE*)&dataBloCmd902;
            pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 1001, 3412, &varV, sizeof(A_BLOB_SEMS_CMD109));

            dataBloCmd902.emBlobType = BLOB_TYPE_SEMS_CMD902;

            SESS_ACTIVE_ALARMS* pClientAlarm = &dataBloCmd902.clientActiveAlarms[uClientIdx];
            pClientAlarm->iDevID = (int)pHead->idxClient;
            pClientAlarm->iAlarmCount = pHead->byDevNum;

            size_t iSingleAlarmLen = sizeof(SC6_LDEV_109);
            SC6_LDEV_109* pSC6_LDEV_902 = (SC6_LDEV_109*)pData;

            int i = 0;
            for (i = 0; i < pHead->byDevNum && i < MAX_QUERY_ACTV_ALM; ++i)
            {
                memcpy(&pClientAlarm->activeAlms[i], pSC6_LDEV_902, iSingleAlarmLen);
                reverseBytes((BYTE*)&pClientAlarm->activeAlms[i], (int)iSingleAlarmLen);
                ++pSC6_LDEV_902;
            }

			Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 1001, 3412, (void*)&dataBloCmd902, sizeof(A_BLOB_SEMS_CMD109), BLOB_TYPE_SEMS_CMD902);
        }
        break;

        default:
            break;
    }

    return TRUE;
}
int Parse_LDEV_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n)
{
	if (pCCU->lcCfg.lcType != LC_TYPE_1st)
	{
		// 先完成 top#1
		TRACEW("Parse_LDEV_SEMS return 0; !LC_TYPE_1st");
		return 0;
	}

	static const int SIZE_HEAD = sizeof(SC6LDEV_CMDHead);
	SC6LDEV_CMDHead* pHead = (SC6LDEV_CMDHead*)pData;
	reverseBytes((BYTE*)pHead, SIZE_HEAD);
	TRACE("Parse_LDEV_SEMS usCmd:%d idxClient:%u usLenSingle:%d byDevNum:%d usDataLen:%d <%s>",
		pHead->usCmd, pHead->idxClient, pHead->usLenSingle, pHead->byDevNum, usDataLen, printableBytes(pData, usDataLen));
	switch (pHead->usCmd)
	{
		// case 101:// KYA20250802
		// {
		// 	SParse_LDEV_101(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		// }
		// break;
		// case 102:
		// {
		// 	SParse_LDEV_102(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		// }
		// break;
		case 103:
		{
			SParse_LDEV_103(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			break;
		}
		case 104:
		{
			SParse_LDEV_104(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			break;
		}
		// case 106:// KYA20250802
		// {
		// 	SParse_LDEV_106(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		// }
		// break;
		// case 108:// KYA20250802
		// {
		// 	SParse_LDEV_108(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		// }
		// break;
		case 109:
		{
			SParse_LDEV_109(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			break;
		};
		case 110:
		{
			SParse_LDEV_110(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			break;
		};
		case 202:
		{
			SParse_LDEV_202(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			break;
		};

		case 155:
		{
			// if (pHead->byDevNum * pHead->usLenSingle != usDataLen-SIZE_HEAD)
			// {
			// 	TRACEE("Parse_LDEV_SEMS Send_C6LDEV len error");
			// }
			// else
			{
				SParse_LDEV_155(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
			}
		}
		break;
		case 157:
		{
			SParse_LDEV_157(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		}
		break;
		case 901:
		{	
			SParse_LDEV_901(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		}
		case 902:
		{	
			SParse_LDEV_902(pCCU, pHead, pData+SIZE_HEAD, (USHORT)(usDataLen-SIZE_HEAD), n);
		}
		break;
		break;
	default:
		break;
	}
	return 0;
}
