#include "inter_slave_main.h"

//1.涉及多字节数据，例如4字节整型数，高字节先传。
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

static void MakeUlong(ULONG ulVal, OUT BYTE* pData)
{
	*(BYTE*)(pData)= (BYTE)(ulVal >> 24);
	*(BYTE*)(pData + 1) = (BYTE)(ulVal >> 16);
	*(BYTE*)(pData + 2) = (BYTE)(ulVal >> 8);
	*(BYTE*)(pData + 3) = (BYTE)(ulVal & 0xFF);
}

static void MakeShort(short sVal, OUT BYTE* pData)
{
	*(BYTE*)(pData) = (BYTE)(sVal >> 8);
	*(BYTE*)(pData + 1) = (BYTE)(sVal & 0xFF);
}


static void MakeFloat(float fVal, OUT BYTE* pData)
{
	VAR_VALUE varV;
	varV.fValue = fVal;

	*pData = varV.abyValue[0];
	*(pData  + 1) = varV.abyValue[1];
	*(pData  + 2) = varV.abyValue[2];
	*(pData  + 3) = varV.abyValue[3];

	return;
}

static void SetDateTime(time_t* pTimeRet)
{
	char szTime[64];
	char szCommand[128];

	TimeToString(*pTimeRet, TIME_CHN_FMT, 
		szTime, sizeof(szTime));

	sprintf(szCommand, "date -s \"%s\"", szTime);
	TRACEX("\n***UpdateTime: [%s]\n", szCommand);
#ifdef _DEBUG
	system(szCommand);
	system("hwclock -w");
#else
	_SYSTEM(szCommand);
	_SYSTEM("hwclock -w");
#endif
	stime(pTimeRet);		//2.Update OS time
}

/*-------------------C2S Requst Cmd 分体柜向主柜请求命令-----------------------*/
int PrepareCfgNtfCmd01(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
 	UNUSED(nDataLen);

	int 			nDataSize = 0;		
	VAR_VALUE 		varV;
	ERRCODE_DAI 	err;

	SAMPLER* pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
 	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

 	A_BLOB_ES_BACKEND_CFG 		blobBackCfg;
	ZERO_POBJS(&blobBackCfg, 	1);

 	varV.blobValue = (BYTE *)&blobBackCfg;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
	if(err != ERR_DPR_OK)
	{
		TRACE("Get blobBackCfg Error!\n");
 	}
	
	TRACE("PrepareCmd01: fSysACInput_PwrLmt=%.1f\n", blobBackCfg.fSysACInput_PwrLmt);

	memcpy(pData + nDataSize, &blobBackCfg, sizeof(A_BLOB_ES_BACKEND_CFG));
	
	nDataSize += sizeof(A_BLOB_ES_BACKEND_CFG);

	TRACE("nDataSize =%d \n", nDataSize);
	return nDataSize;
}

int PrepareSysInfoCmd03(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	int nDataSize = 0;
	SAMPLER* pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	VAR_VALUE varV;
	ERRCODE_DAI err;

	//1.Get Slave  Avaliable Status 0-normal ,1-Locked, 2-Fault
	pDPR->Get((HANDLE)pSampler, SETTYPE_LOGIC_SP, 1, 4, &varV, 4);
	*(pData+nDataSize) = (BYTE)varV.emValue;
	nDataSize += 1;

	//2.Total Power Branch Number
	pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 500, 502, &varV, 4);
	pSLV->nCfgBattNum = varV.ulValue;
	*(pData+nDataSize) = (BYTE)pSLV->nCfgBattNum;
	nDataSize += 1; 
	TRACE("pSLV->nCfgBattNum =%d\n", pSLV->nCfgBattNum);

	//3.Local [AC]-RealTime Active Power
	pDPR->Get((HANDLE)pSampler, SETTYPE_LOGIC_SP, 407, 419, &varV, 4);
	MakeFloat(varV.fValue, pData+nDataSize);
	nDataSize += 4; 

	//4.Local [AC]-RealTime KWH Reading
	pDPR->Get((HANDLE)pSampler, SETTYPE_LOGIC_SP, 407, 411, &varV, 4);
	MakeFloat(varV.fValue, pData+nDataSize);
	nDataSize += 4; 

	//5.Local [AC]-RealTime Reverse KWH  
	pDPR->Get((HANDLE)pSampler, SETTYPE_LOGIC_SP, 407, 422, &varV, 4);
	MakeFloat(varV.fValue, pData+nDataSize);
	nDataSize += 4; 

	//Get Battery DataInfo
 	varV.blobValue =(BYTE*)&pSLV->blobSingleBatt[0];
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL,
		501, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
	if(err != ERR_DPR_OK)
	{
		TRACE("Get blobSingleBatt_1 Error!\n");
 	}

	//Get Rect Group Data Info
	A_BLOB_BRANCH_GRP_RECTDATA blobRectGrp_1;
	varV.blobValue =(BYTE*)&blobRectGrp_1;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL,
		211, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA));
	if(err != ERR_DPR_OK)
	{
		TRACE ("Get blobRectGrp_1 Error!\n");
 	}

	//4.MPPT Max Output Power
	float fMPPTRealOutputPower_1 = blobRectGrp_1.fMPPT_RealTime_Volt*blobRectGrp_1.fMPPT_RealTime_Curr*0.001;
	MakeFloat(fMPPTRealOutputPower_1, pData+nDataSize);
	nDataSize +=4;

	//5.Battery Max Output Power
	float fBattMaxOutputPower_1 = pSLV->blobSingleBatt[0].fPackTotalVolt*pSLV->blobSingleBatt[0].fAllow_MaxDisChargeCurr*0.001;
	MakeFloat(fBattMaxOutputPower_1, pData+nDataSize);
	nDataSize +=4;

	//6.Battery Charge Requst Power
	float fBattChrgReqPower_1 = pSLV->blobSingleBatt[0].fAllow_LmtChargeVolt*pSLV->blobSingleBatt[0].fAllow_MaxChargeCurr*0.001;
	MakeFloat(fBattChrgReqPower_1, pData+nDataSize);
	nDataSize +=4;

	//7.Batt_1 SOC
	*(pData+nDataSize) = (BYTE)pSLV->blobSingleBatt[0].nSOC;
	nDataSize +=1; 


	//7.ACDC Max Output Power Ability
	MakeFloat(blobRectGrp_1.fACDC_Max_Output_Pwr_Ability, pData+nDataSize);
	nDataSize +=4;

	//8.DCDC Max Output Power Ability
	MakeFloat(blobRectGrp_1.fDCDC_Max_Output_Pwr_Ability, pData+nDataSize);
	nDataSize +=4;

	//RESV
	nDataSize += 12;

 	varV.blobValue =(BYTE*)&pSLV->blobSingleBatt[1];
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL,
		502, 551, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
	if(err != ERR_DPR_OK)
	{
		TRACE ("Get pSLV->blobSingleBatt[1] Error!\n");
 	}

	//Get Rect Group Data Info
	A_BLOB_BRANCH_GRP_RECTDATA blobRectGrp_2;
	varV.blobValue =(BYTE*)&blobRectGrp_2;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL,
		212, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA));
	if(err != ERR_DPR_OK)
	{
		TRACE ("Get blobRectGrp_2 Error!\n");
 	}

	//12.MPPT Max Output Power
	float fMPPTRealOutputPower_2 = blobRectGrp_2.fMPPT_RealTime_Volt*blobRectGrp_2.fMPPT_RealTime_Curr*0.001;
	MakeFloat(fMPPTRealOutputPower_2, pData+nDataSize);
	nDataSize +=4;

	//13.Battery Max Output Power
	float fBattMaxOutputPower_2 = pSLV->blobSingleBatt[1].fPackTotalVolt*pSLV->blobSingleBatt[1].fAllow_MaxDisChargeCurr*0.001;
	MakeFloat(fBattMaxOutputPower_2, pData+nDataSize);
	nDataSize +=4;

	//14.Battery Charge Requst Power
	float fBattChrgReqPower_2 = pSLV->blobSingleBatt[1].fAllow_LmtChargeVolt*pSLV->blobSingleBatt[1].fAllow_MaxChargeCurr*0.001;
	MakeFloat(fBattChrgReqPower_2, pData+nDataSize);
	nDataSize +=4;

	//7.Batt_2 SOC
	*(pData+nDataSize) = (BYTE)pSLV->blobSingleBatt[1].nSOC;
	nDataSize +=1;  

	//15.ACDC Max Output Power Ability
	MakeFloat(blobRectGrp_2.fACDC_Max_Output_Pwr_Ability, pData+nDataSize);
	nDataSize +=4;

	//16.DCDC Max Output Power Ability
	MakeFloat(blobRectGrp_2.fDCDC_Max_Output_Pwr_Ability, pData+nDataSize);
	nDataSize +=4;

	//RESV
	nDataSize += 12;

	TRACE("nDataSize =%d\n", nDataSize);
		
	return nDataSize;
}

int PrepareBmsCmd05(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	TRACEX("PrepareBmsCmd05:111\n");
	int n = 0, nDataSize = 0;
 	SAMPLER* pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	VAR_VALUE 	varV;
	ERRCODE_DAI err;

	//1.Single BMS Number, Need Get System battery Grp Config Number 
 	
	*(pData+nDataSize) = (BYTE)pSLV->nCfgBattNum;
	nDataSize += 1;

	for(n = 0; n < pSLV->nCfgBattNum; n++)
	{
		memcpy(pData + nDataSize , &pSLV->blobSingleBatt[n], sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		nDataSize += sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
	}
 
	TRACE("Info: PrepareBmsCmd05: nDataSize =%d\n", nDataSize);	
	return nDataSize;
}

//------------- Energy Flow --------------//
int PrepareEngFlowCmd07(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	int 		nDataSize = 0;
	ERRCODE_DAI err;
	VAR_VALUE 	varV;
	
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	A_BLOB_ES_RUN_DATA		esRunData;
	ZERO_POBJS(&esRunData, 	1);
	varV.blobValue = (BYTE *)&esRunData;
	err = pDPR->Get((HANDLE)pSampler, SETTYPE_LOGIC_SP, 
		200, 80, &varV, sizeof(A_BLOB_ES_RUN_DATA));
	if(err != ERR_DPR_OK)
	{
		TRACE ("Get esRunData Error!\n");
 	}

	memcpy(pData + nDataSize , &esRunData, sizeof(A_BLOB_ES_RUN_DATA));
	
	nDataSize += sizeof(A_BLOB_ES_RUN_DATA);

	TRACEX("C2S_EnergyFlow: nDataSize=%d fACVolt_L1=%.1f,fACVolt_L2=%.1f,fACVolt_L3=%.1f\
		fACCurr_L1=%.1f,fACCurr_L2=%.1f,fACCurr_L3=%.1f\n",\
	 	nDataSize, esRunData.fACVolt_L1, esRunData.fACVolt_L2, esRunData.fACVolt_L3,\
	 	esRunData.fACCurr_L1, esRunData.fACCurr_L2, esRunData.fACCurr_L3);

	return nDataSize;
}

//------------- Single Rect DataInfo --------------//
int PrepareSigRectCmd09(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	int 		nDataSize = 0;
	ERRCODE_DAI err;
	VAR_VALUE 	varV;
	
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	A_BLOB_SINGLE_RECT_INFO sigRect;
	ZERO_POBJS(&sigRect, 	1);
	varV.blobValue = (BYTE *)&sigRect;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL, 
		200, 101, &varV, sizeof(A_BLOB_SINGLE_RECT_INFO));
	if(err != ERR_DPR_OK)
	{
		TRACE ("Get sigRect Error!\n");
 	}

	memcpy(pData + nDataSize , &sigRect, sizeof(A_BLOB_SINGLE_RECT_INFO));
	
	nDataSize += sizeof(A_BLOB_SINGLE_RECT_INFO);
 
	//TRACE("C2S_SingleRect: nDataSize=%d nOnlineACDCNum=%d nOnlineDCDCNum=%d nOnlineMPPTNum=%d\n", \
	//	nDataSize, sigRect.nOnlineACDCNum, sigRect.nOnlineDCDCNum, sigRect.nOnlineMPPTNum );

	return nDataSize;
}

//------------- Battery Cabinet DataInfo --------------//
int PrepareBattCabCmd0B(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	int 		n = 0, nDataSize = 0;
	int			nCfgBattCabNum = 2;

	ERRCODE_DAI err;
	VAR_VALUE 	varV;
	
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	*(pData+nDataSize) = (BYTE)nCfgBattCabNum;
	nDataSize += 1;

	A_BLOB_BATTCAB_IMEU2_V20 	imeu2Data[MAX_SINGLE_STBATT_NUM];
	ZERO_POBJS(imeu2Data, 	MAX_SINGLE_STBATT_NUM);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		varV.blobValue = (BYTE *)&imeu2Data[n];
		err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_RAW_VAL, 
			601 + n, 450, &varV, sizeof(A_BLOB_BATTCAB_IMEU2_V20));
		if(err != ERR_DPR_OK)
		{
			TRACEX("Get which=[%d] imeu2Data Error!\n", n);
		}

		memcpy(pData + nDataSize , &imeu2Data[n], sizeof(A_BLOB_BATTCAB_IMEU2_V20));
		
		nDataSize += sizeof(A_BLOB_BATTCAB_IMEU2_V20);
	}

	TRACE("nDataSize=%d sizeof(A_BLOB_SINGLE_RECT_INFO)=%d\n", nDataSize, sizeof(A_BLOB_BATTCAB_IMEU2_V20));

	return 	nDataSize;
}


/***********************************************************************
*	FUNC: int compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
*
*
*   Note:Compresses the source buffer into the destination buffer.  sourceLen is
*   the byte length of the source buffer. Upon entry, destLen is the total
*   size of the destination buffer, which must be at least the value returned
*   by compressBound(sourceLen). Upon exit, destLen is the actual size of the
*   compressed buffer.
*     This function can be used to compress a whole file at once if the
*   input file is mmap'ed.
*     compress returns Z_OK if success, Z_MEM_ERROR if there was not
*   enough memory, Z_BUF_ERROR if there was not enough room in the output
*   buffer.
***************************************************************************/
//------------- Statistics History Data --------------//
int PrepareStaticDataCmd0D(IN void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)  
{
	int 		n = 0, nDataSize = 0;
	int 		nDataHeaderLen = 4;
	ERRCODE_DAI err;
	VAR_VALUE 	varV;
	
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	A_BLOB_ES_STATISTIC_DATA 	blobStatisData;
	ZERO_POBJS(&blobStatisData, 	1);
 	varV.blobValue = (BYTE *)&blobStatisData;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 79, &varV, sizeof(A_BLOB_ES_STATISTIC_DATA));
 	if(err != ERR_DPR_OK)
	{
		TRACE("Error:Get A_BLOB_ES_STATISTIC_DATA Error!\n");
 	}

	int nBlobStaticSize = sizeof(A_BLOB_ES_STATISTIC_DATA);

	UINT nCompMaxDest = compressBound(nBlobStaticSize);	//nBlobStaticSize;//
	BYTE* pDataDest = NULL;
	
	//blobStatisData.es_Data.hours_Data.fAC_CurrKWH_Reading[0] = 222.2;
	//now compress the data
	int nRet = compress(pData + nDataHeaderLen, &nCompMaxDest, (BYTE*)&blobStatisData/*(BYTE*)pDataSrc*/, (UINT)nBlobStaticSize);
	if(nRet != Z_OK)
	{
		printf("Error: nRet=[%d]!\n", nRet);
	}
	else
	{
		MakeUlong(nBlobStaticSize, pData); //Org String DataLen
 		nDataSize = nCompMaxDest + nDataHeaderLen;
	}
	
	TRACE("Info: nRet=[%d] nCompMaxDest=[%d] nDataSize=[%d] nBlobStaticSize=[%d]!\n", \
		nCompMaxDest, nRet, nDataSize,nBlobStaticSize);

	return 	nDataSize;
}


/*--------------------S2C 主柜应答分体柜的请求 ------------------*/
int ExtractCfgNtfCmd02(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

	int nSyncStatu = (int)pData[0];
	if((nSyncStatu == 0) || (nSyncStatu == 1) ||
	   (nSyncStatu == 2) || (nSyncStatu == 3))
	{
		pSLV->emSLVConnStatus = SYNC_PENDING;
	}
	else if(nSyncStatu == 4)
	{
		pSLV->emSLVConnStatus = SYNC_SUCCESS;
	}
	else
	{
		pSLV->emSLVConnStatus = DIS_CONNECTED;
	}

	TRACE("nSyncStatu =%d \n",nSyncStatu);

	return 0;
}

int ExtractSysInfoCmd04(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	TRACE("--ExtractCmd04---\n");
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	//int nDataSize = 0;
	
	//1.Get Master System Running Statu
	pSLV->emMasterCCUState = (CCU_STATE)pData[0];
	pData += 1;

	//2. sync RTC
	time_t tmRTC;
	tmRTC = (time_t)Bytes2Ulong(pData, 4);
	pData +=4;

	if(ABS(_NOW_ - tmRTC) > 5)
	{
		SetDateTime(&tmRTC);
	}
	
	//RESV
	pData +=4;

	//RESV
	pData +=4;

	//RESV
	pData +=4;

	//2. Branch Node 1 Calc DataInfo
	pSLV->nodeCalData[0].fMPPT_Distri_4_ACDC_Pwr = Bytes2Float(pData);
 	pData +=4;

    pSLV->nodeCalData[0].fMPPT_Distri_4_DCDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[0].fMPPT_Distri_4_Batt_Pwr = Bytes2Float(pData);
 	pData +=4;

	//3. 储能电池功率分配
    pSLV->nodeCalData[0].fBatt_Distri_4_ACDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[0].fBatt_Distri_4_DCDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	//5. MPPT与储能电池剩余输出功率
    pSLV->nodeCalData[0].fRemain_Batt_DisChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[0].fRemain_MPPT_DisChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

    pSLV->nodeCalData[0].fRemain_Batt_Chrg_Pwr_Req = Bytes2Float(pData);
 	pData +=4;
 
    //6. ACDC分配给储能电池最大充电功率
    pSLV->nodeCalData[0].fACDC_Distri_4_BattChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[0].emBatt_WorkMode = (STBATT_PRI_WORKMODE)pData[0];
	pData +=1;

	pSLV->nodeCalData[0].fBatt_Max_Chrg_Pwr_Req = Bytes2Float(pData);
 	pData +=4;

	//TRACE("SLAVE ExtractCmd04: fMPPT_Distri_4_ACDC_Pwr_1=%.1f\
	//	fMPPT_Distri_4_DCDC_Pwr_1=%.1f\
	//	fMPPT_Distri_4_Batt_Pwr_1=%.1f\
	//	fBatt_Distri_4_ACDC_Pwr_1=%.1f\
	//	fBatt_Distri_4_DCDC_Pwr_1=%.1f\
	//	fRemain_Batt_DisChrg_Pwr_1=%.1f\
	//	fRemain_MPPT_DisChrg_Pwr_1=%.1f\
	//	fRemain_Batt_Chrg_Pwr_Req_1=%.1f\
	//	fACDC_Distri_4_BattChrg_Pwr_1=%.1f\
	//	emBatt_WorkMode=%d\n",\
	//	pSLV->nodeCalData[0].fMPPT_Distri_4_ACDC_Pwr,\
	//	pSLV->nodeCalData[0].fMPPT_Distri_4_DCDC_Pwr,\
	//	pSLV->nodeCalData[0].fMPPT_Distri_4_Batt_Pwr,\
	//	pSLV->nodeCalData[0].fBatt_Distri_4_ACDC_Pwr,\
	//	pSLV->nodeCalData[0].fBatt_Distri_4_DCDC_Pwr,\
	//	pSLV->nodeCalData[0].fRemain_Batt_DisChrg_Pwr,\
	//	pSLV->nodeCalData[0].fRemain_MPPT_DisChrg_Pwr,\
	//	pSLV->nodeCalData[0].fRemain_Batt_Chrg_Pwr_Req,\
	//	pSLV->nodeCalData[0].fACDC_Distri_4_BattChrg_Pwr,\
	//	pSLV->nodeCalData[0].emBatt_WorkMode);


	//RESV
	pData +=7;
 
	pSLV->nodeCalData[1].fMPPT_Distri_4_ACDC_Pwr = Bytes2Float(pData);
 	pData +=4;

    pSLV->nodeCalData[1].fMPPT_Distri_4_DCDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[1].fMPPT_Distri_4_Batt_Pwr = Bytes2Float(pData);
 	pData +=4;

	//3. 储能电池功率分配
    pSLV->nodeCalData[1].fBatt_Distri_4_ACDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[1].fBatt_Distri_4_DCDC_Pwr = Bytes2Float(pData);
 	pData +=4;

	//5. MPPT与储能电池剩余输出功率
    pSLV->nodeCalData[1].fRemain_Batt_DisChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[1].fRemain_MPPT_DisChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

    pSLV->nodeCalData[1].fRemain_Batt_Chrg_Pwr_Req = Bytes2Float(pData);
 	pData +=4;
 
    //6. ACDC分配给储能电池最大充电功率
    pSLV->nodeCalData[1].fACDC_Distri_4_BattChrg_Pwr = Bytes2Float(pData);
 	pData +=4;

	pSLV->nodeCalData[1].emBatt_WorkMode = (STBATT_PRI_WORKMODE)pData[0];
	pData +=1;

	pSLV->nodeCalData[1].fBatt_Max_Chrg_Pwr_Req = Bytes2Float(pData);
 	//pData +=4;
	
	//RESV
	//pData +=7;

	//TRACE("SLAVE ExtractCmd04: fMPPT_Distri_4_ACDC_Pwr_2=%.1f\
	//	fMPPT_Distri_4_DCDC_Pwr_2=%.1f\
	//	fMPPT_Distri_4_Batt_Pwr_2=%.1f\
	//	fBatt_Distri_4_ACDC_Pwr_2=%.1f\
	//	fBatt_Distri_4_DCDC_Pwr_2=%.1f\
	//	fRemain_Batt_DisChrg_Pwr_2=%.1f\
	//	fRemain_MPPT_DisChrg_Pwr_2=%.1f\
	//	fRemain_Batt_Chrg_Pwr_Req_2=%.1f\
	//	fACDC_Distri_4_BattChrg_Pwr_2=%.1f\
	//	emBatt_WorkMode=%d\n",\
	//	pSLV->nodeCalData[1].fMPPT_Distri_4_ACDC_Pwr,\
	//	pSLV->nodeCalData[1].fMPPT_Distri_4_DCDC_Pwr,\
	//	pSLV->nodeCalData[1].fMPPT_Distri_4_Batt_Pwr,\
	//	pSLV->nodeCalData[1].fBatt_Distri_4_ACDC_Pwr,\
	//	pSLV->nodeCalData[1].fBatt_Distri_4_DCDC_Pwr,\
	//	pSLV->nodeCalData[1].fRemain_Batt_DisChrg_Pwr,\
	//	pSLV->nodeCalData[1].fRemain_MPPT_DisChrg_Pwr,\
	//	pSLV->nodeCalData[1].fRemain_Batt_Chrg_Pwr_Req,\
	//	pSLV->nodeCalData[1].fACDC_Distri_4_BattChrg_Pwr,\
	//	pSLV->nodeCalData[1].emBatt_WorkMode);
	
	return TRUE;
}

int ExtractBmsCmd06(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	UINT nRTN = (UINT)pData[0];
	TRACE("ExtractCmd06: nRTN =%d\n", nRTN);
	return TRUE;
}

int ExtractEngFlowCmd08(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	TRACE("\n");
	return TRUE;
}

int ExtractSigRectCmd0A(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	TRACE("\n");
	return TRUE;
}

int ExtractBattCabCmd0C(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	TRACE("\n");
	return TRUE;
}

int ExtractStaticDataCmd0E(void* pArgs, IN BYTE* pData, OUT USHORT nResult)
{
	UINT nRTN = (UINT)pData[0];
	TRACE("ExtractStaticDataCmd0E: nRTN =%d\n", nRTN);
	return TRUE;
}

/*--------------------S2C 主柜主动发送控制命令给分体柜 ------------------*/
int ExcuteSetCfgCmdA1(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
 	TRACE("--ExcuteCmdA1--\n");
	SAMPLER *pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;
	
	BYTE byRTN = 0;
	BYTE bySubCmd;
	BYTE bySendBuff[TCP_BUFF_LEN];
	ZERO_POBJS(bySendBuff, 1);

	ERRCODE_DAI err;
	VAR_VALUE varV;
	int nBlobLen = 0;

	bySubCmd = pData[0];
	pData +=1;
	switch(bySubCmd)
	{
		case 0x01:
		{
			nBlobLen = Bytes2Ulong(pData, 2);
			pData += 2;
			TRACE("01_Cmd: nBlobLen=%d sizeof(A_BLOB_EMS_CONFIG_PLAN)=%d\n",\
				nBlobLen, sizeof(A_BLOB_EMS_CONFIG_PLAN));
			if(nBlobLen != sizeof(A_BLOB_EMS_CONFIG_PLAN))
			{
				TRACE("--Error Blob A_BLOB_EMS_CONFIG_PLAN Len =%d--\n" ,sizeof(A_BLOB_EMS_CONFIG_PLAN));
				byRTN = 0x04; //Data Invalid
				break;
			}

			A_BLOB_EMS_CONFIG_PLAN *pRecvblobEmsCfg = (A_BLOB_EMS_CONFIG_PLAN*)(pData);
			
			A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
			memcpy(&blobEmsCfg, pRecvblobEmsCfg, sizeof(A_BLOB_EMS_CONFIG_PLAN));

			TRACE("nTotal_EmsLmt_Sectors=%d tmStart=%ld nInterval=%d emBatt_WorkMode=%d\n", \
			blobEmsCfg.nTotal_EmsLmt_Sectors, \
			blobEmsCfg.emsLmtSectors[0].tmStart,\
			blobEmsCfg.emsLmtSectors[0].nInterval,\
			blobEmsCfg.emsLmtSectors[0].emBatt_WorkMode);
			if(blobEmsCfg.nTotal_EmsLmt_Sectors)
			{
				Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSLV->hSampler, 200, 201, 
				(void*)&blobEmsCfg, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
			}
	 	}
		break;
		case 0x02:
		{
			//nBlobLen = Bytes2Ulong(pData, 2);
			//pData += 2;
			//TRACE("02_Cmd: nBlobLen=%d sizeof(A_BLOB_BATT_MGMT_PLAN)=%d\n",\
			// 	nBlobLen, sizeof(A_BLOB_BATT_MGMT_PLAN));
			//
			//if(nBlobLen != sizeof(A_BLOB_BATT_MGMT_PLAN))
			//{
			//	byRTN = 0x04; //Data Invalid
			//	TRACE("--Error Blob A_BLOB_BATT_MGMT_PLAN Len =%d--\n" ,sizeof(A_BLOB_BATT_MGMT_PLAN));
			//	break;
			//}
			//A_BLOB_BATT_MGMT_PLAN *pRecvblobBattMgmt = (A_BLOB_BATT_MGMT_PLAN*)(pData);;
			//
			//A_BLOB_BATT_MGMT_PLAN  blobBattMgmt;
			//memcpy(&blobBattMgmt, pRecvblobBattMgmt, sizeof(A_BLOB_BATT_MGMT_PLAN));
			//
			//if(blobBattMgmt.nTotal_STBattLmt_Sectors > 0)
			//{
			//	Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSLV->hSampler, 500, 501, 
			//		(void*)&blobBattMgmt, sizeof(A_BLOB_BATT_MGMT_PLAN), BLOB_TYPE_BATT_MGMT_PLAN);
			//}
 		}break;
		 
		default:
		{
			TRACE("\n");
			byRTN = 0x02; //Cmd Invalid
			break;
		}
	}

 
	//Need Reply
	int nDataSize = 0;

	bySendBuff[nDataSize] = bySubCmd;
	nDataSize += 1;

	bySendBuff[nDataSize] = byRTN;
	nDataSize += 1;

	//pSLV->msgFrameInfo[_S_S2C_SETCFG_MSG].pfnSHandler((HANDLE)pSampler, bySendBuff, nDataLen); 
	PackAndWriteSession(pSampler, _S_S2C_SETCFG_MSG, bySendBuff, nDataSize); // PackFrame And Write to hCommPort

	TRACE("\n");
	return TRUE;
}

int ExcuteCtrlCmdA3(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	TRACE("\n");
	return FALSE;
}

int ExcuteAppSizeCmdA5(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	TRACE("ExcuteCmdA5: \n");	 
	return TRUE;
}

int ExcuteAppUpdCmdA7(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
 	TRACE("\n");
	return FALSE;
}

int ExcuteChkSumCmdA9(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	TRACE("\n");
	return FALSE;
}


/*--------------------S2C 分体柜响应主柜的控制请求命令 ------------------*/
int PrepReplySetCfgCmdA2(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	TRACE("PrepReplyCmdA2:\n");
	int nDataSize = 0;
	//SAMPLER* pSampler = (SAMPLER*)pArgs;
 	//ROUGH_SLAVE_DATA* pSLV = (ROUGH_SLAVE_DATA*)pSampler->pDataHeap;

 	//*(pData+nDataSize) = (BYTE)1;
	//nDataSize += 1;
 
	//*(pData+nDataSize) = (BYTE)2;
	//nDataSize += 1;
	//TRACE("nDataSize =%d \n", nDataSize);

	return nDataSize;
} 

int PrepReplyCtrlCmdA4(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	TRACE("PrepReplyCtrlCmdA4: \n");
	return 0;
} 

int PrepReplyAppSizeCmdA6(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	TRACE("PrepReplyAppSizeCmdA6: \n");
	return 0;
} 

int PrepReplyAppUpdCmdA8(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	TRACE("PrepReplyAppUpdCmdA8: \n");
	return 0;
} 

int PrepReplyChkSumCmdAA(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	TRACE("PrepReplyChkSumCmdAA: \n");
	return 0;
}