#include "inter_master.h"

extern  INTER_CLIENT 			s_Clients[MAX_SUPPORTED_CLIENTS];


//1.涉及多字节数据，例如4字节整型数，高字节先传。
ULONG Bytes2Ulong(BYTE* pBytes, int nBytes)
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

void MakeUlong(ULONG ulVal, OUT BYTE* pData)
{
	*(BYTE*)(pData)= (BYTE)(ulVal >> 24);
	*(BYTE*)(pData + 1) = (BYTE)(ulVal >> 16);
	*(BYTE*)(pData + 2) = (BYTE)(ulVal >> 8);
	*(BYTE*)(pData + 3) = (BYTE)(ulVal & 0xFF);
}

void MakeShort(short sVal, OUT BYTE* pData)
{
	*(BYTE*)(pData) = (BYTE)(sVal >> 8);
	*(BYTE*)(pData + 1) = (BYTE)(sVal & 0xFF);
}

/*--------主柜执行分体柜的请求命令--------*/
int ExcuteCfgNtfCmd01(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	//Currently ,Do not check DataLen compare to DataSize
	//UNUSED(nDataLen);
	VAR_VALUE 		varV;
	ERRCODE_DAI 	err;

 	BYTE 			byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = pClient->pReporter;

	//NO2. Parse Backend Cfg
 	A_BLOB_ES_BACKEND_CFG 			blobBackCfg;
	ZERO_POBJS(&blobBackCfg, 	  	1);

	A_BLOB_ES_BACKEND_CFG  *pBackCfg = (A_BLOB_ES_BACKEND_CFG *)pData;

	memcpy(&blobBackCfg, pBackCfg, sizeof(A_BLOB_ES_BACKEND_CFG));
	
	if(blobBackCfg.emBlobType == BLOB_TYPE_ES_BACKEND_CFG)
	{
		varV.blobValue = &blobBackCfg;
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						2, 1010, &varV, sizeof(A_BLOB_ES_BACKEND_CFG), 1000);
		
		TRACEX("nDataLen=%d fSysACInput_PwrLmt=%.1f emSyncStatus=%d\n", nDataLen, blobBackCfg.fSysACInput_PwrLmt,pClient->emSyncStatus);
		if (err == ERRCODE_DAI_OK)
		{
			TRACE("Info: DXI Log_SP blobBackCfg ok!\n");
		}
		else
		{
			TRACE("Error: DXI Log_SP blobBackCfg Failed!\n");
		}
	}
	else
	{
		TRACE("Error: Recv Wrong blobBackCfg.emBlobType=%d !\n", blobBackCfg.emBlobType);	
	}

	pData += sizeof(A_BLOB_ES_BACKEND_CFG);

	//---Step2. Pack SendData---// 	
	byBuff[0] = (BYTE)pClient->emSyncStatus; 
	
	//-----Step3. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_CFGNTF_MSG, byBuff, 1);

	return 0;
}

int ExcuteSysInfoCmd03(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	UNUSED(nDataLen);
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_SYSINFO_MSG]);
	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	int nDataSize = 0;
	int nResult = 0; 

	PARAM_VAL_GET valGet;
	VAR_VALUE varV;
	ERRCODE_DAI err;
	//---Step1.Parse Recv Data---// 
	
	//1. Slave System Avaliable Status

	varV.emValue = (CCU_STATE)pData[0]; //Normal
	pData += 1;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 30, &varV, 4, 100);

	//2. Slave Branch Number
	varV.ulValue = (UINT)pData[0]; 
	pData += 1;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 31, &varV, 4, 100);
	
	TRACEX("Slave Branch Number =%d\n", varV.ulValue);

	//3.Local [AC]-RealTime Active Power
	varV.fValue = Bytes2Float(pData);
	pData += 4; 
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 32, &varV, 4, 100);

	//4.Local [AC]-RealTime KWH Reading
	varV.fValue = Bytes2Float(pData);
	pData += 4;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 33, &varV, 4, 100);

	//5.Local [AC]-RealTime Reverse KWH  
	varV.fValue = Bytes2Float(pData);
	pData += 4;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							1, 34, &varV, 4, 100);

	A_BLOB_BRANCH_GRP_RECTDATA  blobRectGrp_1;
	ZERO_POBJS(&blobRectGrp_1, 1);
	//4. fMPPT_Max_DisChrg_Pwr
	blobRectGrp_1.fMPPT_Max_DisChrg_Pwr = Bytes2Float(pData);
	pData +=4;

	//5.Batt Max fBatt_Max_DisChrg_Pwr Power_1
	blobRectGrp_1.fBatt_Max_DisChrg_Pwr = Bytes2Float(pData);
	pData +=4;

	//6. 
	blobRectGrp_1.fBatt_Max_Chrg_Pwr_Req = Bytes2Float(pData);
	pData +=4;

	//7. Batt_1 SOC 
	blobRectGrp_1.nSOC = (UINT)pData[0];
	pData +=1; 

	//8.
	blobRectGrp_1.fACDC_Max_Output_Pwr_Ability = Bytes2Float(pData);
	pData +=4;

	//9. 
	blobRectGrp_1.fDCDC_Max_Output_Pwr_Ability = Bytes2Float(pData);
	pData +=4;

	TRACE("fMPPT_Max_DisChrg_Pwr=%.1f\
			fBatt_Max_DisChrg_Pwr=%.1f\
			fBatt_Max_Chrg_Pwr_Req=%.1f\
			nSOC =%d\
			fACDC_Max_Output_Pwr_Ability=%.1f\
			fDCDC_Max_Output_Pwr_Ability=%.1f\n", \
			blobRectGrp_1.fMPPT_Max_DisChrg_Pwr,\
			blobRectGrp_1.fBatt_Max_DisChrg_Pwr,\
			blobRectGrp_1.fBatt_Max_Chrg_Pwr_Req,\
			blobRectGrp_1.nSOC,\
			blobRectGrp_1.fACDC_Max_Output_Pwr_Ability,\
			blobRectGrp_1.fDCDC_Max_Output_Pwr_Ability);
	//9.RESV
	pData +=12;
    blobRectGrp_1.emBlobType = BLOB_TYPE_BRANCH_GRP_RECT_DATA;
	varV.blobValue = &blobRectGrp_1;
 	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					213, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACE("DXI Log_SP blobRectGrp_1 ok!\n");
	}
	else
	{
		TRACE("DXI Log_SP blobRectGrp_1 Failed!\n");
	}

	A_BLOB_BRANCH_GRP_RECTDATA  blobRectGrp_2;
	ZERO_POBJS(&blobRectGrp_2, 1);

	//12. fMPPT_Max_DisChrg_Pwr
	blobRectGrp_2.fMPPT_Max_DisChrg_Pwr = Bytes2Float(pData);
	pData +=4;

	//13.Batt Max fBatt_Max_DisChrg_Pwr Power_2
	blobRectGrp_2.fBatt_Max_DisChrg_Pwr = Bytes2Float(pData);
	pData +=4;

	//14. 
	blobRectGrp_2.fBatt_Max_Chrg_Pwr_Req = Bytes2Float(pData);
	pData +=4;
 
	//15.Batt_2 SOC 
	blobRectGrp_2.nSOC = (UINT)pData[0];
	pData +=1;

	//15.
	blobRectGrp_2.fACDC_Max_Output_Pwr_Ability = Bytes2Float(pData);
	pData +=4;

	//16. 
	blobRectGrp_2.fDCDC_Max_Output_Pwr_Ability = Bytes2Float(pData);
	pData +=4;

	TRACE("Grp2: fMPPT_Max_DisChrg_Pwr=%.1f\
			fBatt_Max_DisChrg_Pwr=%.1f\
			fBatt_Max_Chrg_Pwr_Req=%.1f\
			nSOC =%d\
			fACDC_Max_Output_Pwr_Ability=%.1f\
			fDCDC_Max_Output_Pwr_Ability=%.1f\n", \
			blobRectGrp_2.fMPPT_Max_DisChrg_Pwr,\
			blobRectGrp_2.fBatt_Max_DisChrg_Pwr,\
			blobRectGrp_2.fBatt_Max_Chrg_Pwr_Req,\
			blobRectGrp_2.nSOC,\
			blobRectGrp_2.fACDC_Max_Output_Pwr_Ability,\
			blobRectGrp_2.fDCDC_Max_Output_Pwr_Ability);

	//17.
	pData +=12;
    blobRectGrp_2.emBlobType = BLOB_TYPE_BRANCH_GRP_RECT_DATA;
	varV.blobValue = &blobRectGrp_2;
 	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					214, 201, &varV, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACE("DXI Log_SP blobRectGrp_1 ok!\n");
	}
	else
	{
		TRACE("DXI Log_SP blobRectGrp_1 Failed!\n");
	}

	//---Step2. Pack SendData---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
	TRACE("nDataSize=%d\n",nDataSize);
	//-----Step3. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_SYSINFO_MSG, byBuff, nDataSize);

	return 0;
}

/*---------------------------------
* No.	RTN	 	Description
* 1		0x00	No Error
* 2		0x01	Executed Error
* 3		0x02	CMD invalid
* 4		0x03	Format error
* 5		0x04	Data invalid
*---------------------------------*/
int ExcuteBmsCmd05(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	TRACE("nDataLen =%d\n", nDataLen);
	UNUSED(nDataLen);

	int n = 0, nDataSize = 0, nResult = 0; 
	
	PARAM_VAL_GET 	valGet;
	VAR_VALUE 		varV;
	ERRCODE_DAI 	err;

	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);

	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_BMS_MSG]);

	//Step1. Parse BMS Config Number
	UINT nCfgBMSNum = (UINT)pData[0];   
	pData += 1;

	TRACE("nCfgBMSNum=%d\n", nCfgBMSNum);
	if((nCfgBMSNum < 0) || (nCfgBMSNum > MAX_SINGLE_STBATT_NUM))
	{
		nResult = 4; //Data Invalid
		nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
 		Send_MST_Frame(pClient, _M_C2S_BMS_MSG, byBuff, nDataSize);	
		TRACE("Recv Wrong nCfgNum =%d\n", nCfgBMSNum);
		return -1;	
	}

	A_BLOB_SINGLE_STBATT_RUNINFO  blobSigBMS[MAX_SINGLE_STBATT_NUM];
	ZERO_POBJS(blobSigBMS, 	  2);
 
	for(n = 0; n < nCfgBMSNum; n++)
	{
		A_BLOB_SINGLE_STBATT_RUNINFO *pBlobSinglegBMS = (A_BLOB_SINGLE_STBATT_RUNINFO *)pData;
		memcpy(&blobSigBMS[n], pBlobSinglegBMS, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
		
		if(blobSigBMS[n].emBlobType == BLOB_TYPE_SINGLE_STBATT_RUNINFO)
		{
			varV.blobValue = &blobSigBMS[n];
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							2, 1050 + n, &varV, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO), 1000);
			if (err == ERRCODE_DAI_OK)
			{
				TRACE("Info: DXI Log_SP blobSigBMS[%d] ok!\n", n);
			}
			else
			{
				nResult = 3;
				TRACE("Error: DXI Log_SP blobSigBMS[%d] Failed!\n", n);
			}
		}
		else
		{
			nResult = 4;
			TRACE("Error: Recv Wrong blobSigBMS[%d].emBlobType=%d !\n", n, blobSigBMS[n].emBlobType);
			break;		
		}
		
		pData += sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		TRACE("Excute Cmd5 : Single BMS Info BMSIdx=%d fPackTotalVolt=%.1f fPackTotalCurr=%.1f\n", \
		n,
		blobSigBMS[n].fPackTotalVolt,
		blobSigBMS[n].fPackTotalCurr);
	}
	 
	//---Step2. Pack SendData---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
	TRACEX("ExcuteBmsCmd05 : nDataSize=%d nResult=%d\n",nDataSize, nResult);
	
	//-----Step3. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_BMS_MSG, byBuff, nDataSize);

	return 0;
}

int ExcuteEngFlowCmd07(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	int nDataSize = 0, nResult = 0; 
	VAR_VALUE 	varV;
	ERRCODE_DAI err;
	
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	
	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_ENGFLOW_MSG]);

	//---Step1. Excute Slave Cabinet CmdInfo---//

	A_BLOB_ES_RUN_DATA		esRunData;
	ZERO_POBJS(&esRunData, 	  1);

	A_BLOB_ES_RUN_DATA  *pESRunData = (A_BLOB_ES_RUN_DATA *)pData;

	memcpy(&esRunData, pESRunData, sizeof(A_BLOB_ES_RUN_DATA));
	
	if(esRunData.emBlobType == BLOB_TYPE_ES_RUNDATA)
	{
		varV.blobValue = &esRunData;
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						2, 1060, &varV, sizeof(A_BLOB_ES_RUN_DATA), 1000);
		if (err == ERRCODE_DAI_OK)
		{
			TRACE("Info: DXI Log_SP A_BLOB_ES_RUN_DATA ok!\n");
		}
		else
		{
			nResult =3;
			TRACE("Error: DXI Log_SP A_BLOB_ES_RUN_DATA Failed!\n");
		}
	}
	else
	{
		nResult = 4;
		TRACE("Error: Recv Wrong esRunData.emBlobType=%d !\n", esRunData.emBlobType);	
 	}
	
	pData += sizeof(A_BLOB_ES_RUN_DATA);
	TRACE("Info: ExcuteEnergyFlowCmd7 : emBlobType=%d\n", esRunData.emBlobType);
	
	//---Step2. Pack SendData---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
	TRACE("ExcuteEngFlowCmd07 : nDataSize=%d\n",nDataSize);
	
	//-----Step3. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_ENGFLOW_MSG, byBuff, nDataSize);

	TRACE("ExcuteEngFlowCmd07 :nDataLen=%d nDataSize=%d \n", nDataLen, nDataSize);
	return 0;
}

int ExcuteSigRectCmd09(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)	
{
	int nDataSize = 0, nResult = 0; 
	VAR_VALUE 	varV;
	ERRCODE_DAI err;

	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	
	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_SIGRECT_MSG]);

	//---Step1. Excute Slave Cabinet CmdInfo---//

	A_BLOB_SINGLE_RECT_INFO 	sigRect;
	ZERO_POBJS(&sigRect, 	  1);

	A_BLOB_SINGLE_RECT_INFO  *pSingleRect = (A_BLOB_SINGLE_RECT_INFO *)pData;

	memcpy(&sigRect, pSingleRect, sizeof(A_BLOB_SINGLE_RECT_INFO));
	
	if(sigRect.emBlobType == BLOB_TYPE_SINGLE_RECT_DATAINFO)
	{
		varV.blobValue = &sigRect;
		err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						2, 1070, &varV, sizeof(A_BLOB_SINGLE_RECT_INFO), 1000);
		if (err == ERRCODE_DAI_OK)
		{
			TRACE("DXI Log_SP A_BLOB_SINGLE_RECT_INFO ok!\n");
		}
		else
		{
			nResult = 3;
			TRACE("DXI Log_SP A_BLOB_SINGLE_RECT_INFO Failed!\n");
		}
	}
	else
	{
		nResult = 4;
		TRACE("Error: Recv Wrong sigRect.emBlobType=%d !\n", sigRect.emBlobType);
 	}
	
	pData += sizeof(A_BLOB_SINGLE_RECT_INFO);
 
	//---Step2. Pack SendData---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
 	
	//-----Step3. Send Data-----// 
	Send_MST_Frame(pClient, _M_C2S_SIGRECT_MSG, byBuff, nDataSize);

	TRACE("ExcuteSigRectCmd09 :nDataLen=%d nDataSize=%d emBlobType=%d\n", nDataLen, nDataSize, sigRect.emBlobType);
	return 0;
}

int ExcuteBattCabCmd0B(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	int 	n = 0, nDataSize = 0, nResult = 0; 
	UINT 	nCfgBattCabNum = 0;

	VAR_VALUE 	varV;
	ERRCODE_DAI err;

	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	
	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_BATTCAB_MSG]);

	//--- Step1. Excute Slave Cabinet CmdInfo---//
 	nCfgBattCabNum = (UINT)pData[0];   
	pData += 1;
	if((nCfgBattCabNum < 0) || (nCfgBattCabNum > MAX_SINGLE_STBATT_NUM*2))
	{
		nResult = 4;
		nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
 		Send_MST_Frame(pClient, _M_C2S_BATTCAB_MSG, byBuff, nDataSize);
		return -1;
	}
	
	A_BLOB_BATTCAB_IMEU2_V20 	imeu2Data[MAX_SINGLE_STBATT_NUM];
	ZERO_POBJS(imeu2Data, MAX_SINGLE_STBATT_NUM);

	for(n = 0; n < nCfgBattCabNum; n++)
	{
		A_BLOB_BATTCAB_IMEU2_V20 *pBlobIMEU2 = (A_BLOB_BATTCAB_IMEU2_V20 *)pData;
		memcpy(&imeu2Data[n], pBlobIMEU2, sizeof(A_BLOB_BATTCAB_IMEU2_V20));
		if(imeu2Data[n].emBlobType == BLOB_TYPE_BATTCAB_IMEU2_V20)
		{
			varV.blobValue = &imeu2Data[n];
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							2, 1080 + n, &varV, sizeof(A_BLOB_BATTCAB_IMEU2_V20), 1000);
			if (err == ERRCODE_DAI_OK)
			{
				TRACE("DXI Log_SP imeu2Data[%d] ok!\n", n);
			}
			else
			{
				nResult = 3;
				TRACE("DXI Log_SP imeu2Data[%d] Failed!\n", n);
			}
		}
		else
		{
			nResult = 4;
			TRACE("Error: Recv Wrong imeu2Data[%d].emBlobType=%d !\n", n, imeu2Data[n].emBlobType);
			break;
		}
		
		pData += sizeof(A_BLOB_BATTCAB_IMEU2_V20);
		TRACE("Excute Cmd0B : IMEU2 Idx = [%d] \n", n);
	}

	//--- Step2. Pack SendData ---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
 	
	//--- Step3. Send Data ---// 
	Send_MST_Frame(pClient, _M_C2S_BATTCAB_MSG, byBuff, nDataSize);

	TRACE("ExcuteBattCabCmd0B :nDataLen=%d nDataSize=%d nCfgBattCabNum=%d\n", nDataLen, nDataSize, nCfgBattCabNum);
	return 0;
}

/*************************************************************************************
*	FUNC:int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
*
*	NOTE:Decompresses the source buffer into the destination buffer.  sourceLen is
*   the byte length of the source buffer. Upon entry, destLen is the total
*   size of the destination buffer, which must be large enough to hold the
*   entire uncompressed data. (The size of the uncompressed data must have
*   been saved previously by the compressor and transmitted to the decompressor
*   by some mechanism outside the scope of this compression library.)
*   Upon exit, destLen is the actual size of the compressed buffer.
*     This function can be used to decompress a whole file at once if the
*   input file is mmap'ed.
* 
*     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
*   enough memory, Z_BUF_ERROR if there was not enough room in the output
*   buffer, or Z_DATA_ERROR if the input data was corrupted or incomplete.
**************************************************************************************/
int ExcuteStaticDataCmd0D(void* pArgs, IN BYTE* pData, IN USHORT nDataLen)
{
	int n = 0, nOffset = 0, nDataSize = 0, nResult = 0; 
	int nDestStrLen = 0, nSourceStrLen = 0;

	VAR_VALUE 		varV;
	ERRCODE_DAI 	err;

	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = pClient->pReporter;

	BYTE byBuff[TCP_BUFF_LEN];
	memset(byBuff, 0, TCP_BUFF_LEN);
	
	MST_MSG_FRM_INFO* pReplyMsg =  &(pClient->msgFrameInfo[_M_C2S_STATISDATA_MSG]);

	//--- Step1. Excute Statistic Data CmdInfo---//
	nSourceStrLen = Bytes2Ulong(pData, 4); 
	pData +=4;
	nOffset += 4;
	nDestStrLen = nSourceStrLen;

	BYTE *pDest = NEW(BYTE, nSourceStrLen + 1);
	if(pDest == NULL)
	{
		TRACEX("Error:can not allocate memory For pDest\n");
 		return 0;
	}

	if(nSourceStrLen >= 0xFFFF)
	{
		printf("Begin doing the uncompress for steam nDataLen[%d] nSourceStrLen=[%d]\n", nDataLen, nSourceStrLen);
	}

	int nRet = uncompress(pDest, &nDestStrLen, pData, nDataLen - nOffset);
	if(nRet != Z_OK)
	{
		printf("Error: UnCompress Failed %d\n", nRet);
		DELETE(pDest);
 		return -1;
	}
	
	if(nSourceStrLen >= 0xFFFF)
	{
		printf("Info:Compressed OK, nDestStrLen=[%d] nSourceStrLen=[%d]\n", nDestStrLen, nSourceStrLen);	
	}
	
	if(nDestStrLen == sizeof(A_BLOB_ES_STATISTIC_DATA))
	{
	 	A_BLOB_ES_STATISTIC_DATA  	*pESStatisData = (A_BLOB_ES_STATISTIC_DATA *)pDest;
		A_BLOB_ES_STATISTIC_DATA 	blobStatisData;

		memcpy(&blobStatisData, pESStatisData, sizeof(A_BLOB_ES_STATISTIC_DATA));

 		if(blobStatisData.emBlobType == BLOB_TYPE_ES_STATISTICDATA)
		{
			varV.blobValue = &blobStatisData;
			err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
							2, 1090, &varV, sizeof(A_BLOB_ES_STATISTIC_DATA), 1000); 
			if (err == ERRCODE_DAI_OK)
			{
				TRACE("fAC_CurrKWH_Reading[0]=%.1f\n",blobStatisData.es_Data.hours_Data.fAC_CurrKWH_Reading[0]);
				TRACE("Info: DXI Log_SP A_BLOB_ES_STATISTIC_DATA ok!\n");
			}
			else
			{
				nResult =3;
				TRACE("Error: DXI Log_SP A_BLOB_ES_STATISTIC_DATA Failed!\n");
			}
		}
		else
		{
			nResult = 4;
			TRACE("Error: Recv Wrong blobStatisData.emBlobType=%d !\n", blobStatisData.emBlobType);	
		}
	}	
	
	//--- Step2. Pack SendData ---// 
	nDataSize = pReplyMsg->pfnSHandler((void *)pClient, (void *)byBuff, nResult);
 	
	//--- Step3. Send Data ---// 
	Send_MST_Frame(pClient, _M_C2S_STATISDATA_MSG, byBuff, nDataSize);

	DELETE(pDest);
	TRACE("ExcuteStaticDataCmd0D :nDataLen=%d nDataSize=%d\n", nDataLen, nDataSize);
	return 0;
}

/*--------主柜回复分体柜的请求命令--------*/
int PrepReplyCfgNtfCmd02(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nResult);

	return 0;
}

#define INVALID_VALUE 0xFFFFFFFF
int PrepReplySysInfoCmd04(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	REPORTER *pReporter = (REPORTER*)pClient->pReporter;

	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;

	int nDataSize = 0, nDevID = 0;

	nDevID = 201 + (pClient->nClientAddr -1)*2;

	TRACE("PrepReplySysInfoCmd04: nDevID=%d\n", nDevID);

	//1.Master Cabinet System Running Statu
	nDataSize +=1;

	//2.DateTime
	MakeUlong(time(NULL), pData+nDataSize);
	nDataSize +=4;

	//3. RESV 
	nDataSize +=4;

	//RESV 
	nDataSize +=4;

	//RESV
	nDataSize +=4;

	A_BLOB_BRANCH_NODE_CALC_DATA  blobNodeData_3;
	valGet.nGetSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
	valGet.val.blobValue = (BYTE*)&blobNodeData_3;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			223, 203, &valGet, sizeof(PARAM_VAL_GET));

	//2. MPPT模组功率分配
	MakeFloat(blobNodeData_3.fMPPT_Distri_4_ACDC_Pwr, pData+nDataSize);
	nDataSize +=4;

    MakeFloat(blobNodeData_3.fMPPT_Distri_4_DCDC_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_3.fMPPT_Distri_4_Batt_Pwr, pData+nDataSize);
	nDataSize +=4;

	//3. 储能电池功率分配
    MakeFloat(blobNodeData_3.fBatt_Distri_4_ACDC_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_3.fBatt_Distri_4_DCDC_Pwr, pData+nDataSize);
	nDataSize +=4;

	//5. MPPT与储能电池剩余输出功率
    MakeFloat(blobNodeData_3.fRemain_Batt_DisChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_3.fRemain_MPPT_DisChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_3.fRemain_Batt_Chrg_Pwr_Req, pData+nDataSize);
	nDataSize +=4;
 
    //6. ACDC分配给储能电池最大充电功率
    MakeFloat(blobNodeData_3.fACDC_Distri_4_BattChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

	*(pData + nDataSize) = (BYTE)(blobNodeData_3.emBatt_WorkMode);
	nDataSize +=1;

	MakeFloat(blobNodeData_3.fBatt_Max_Chrg_Pwr_Req, pData+nDataSize);
	nDataSize +=4;

	nDataSize +=7;

	A_BLOB_BRANCH_NODE_CALC_DATA  blobNodeData_4;
	valGet.nGetSize = sizeof(A_BLOB_BRANCH_NODE_CALC_DATA);
	valGet.val.blobValue = (BYTE*)&blobNodeData_4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			224, 203, &valGet, sizeof(PARAM_VAL_GET));

	//2. MPPT模组功率分配
	MakeFloat(blobNodeData_4.fMPPT_Distri_4_ACDC_Pwr, pData+nDataSize);
	nDataSize +=4;

    MakeFloat(blobNodeData_4.fMPPT_Distri_4_DCDC_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_4.fMPPT_Distri_4_Batt_Pwr, pData+nDataSize);
	nDataSize +=4;

	//3. 储能电池功率分配
    MakeFloat(blobNodeData_4.fBatt_Distri_4_ACDC_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_4.fBatt_Distri_4_DCDC_Pwr, pData+nDataSize);
	nDataSize +=4;

	//5. MPPT与储能电池剩余输出功率
    MakeFloat(blobNodeData_4.fRemain_Batt_DisChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_4.fRemain_MPPT_DisChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

 	MakeFloat(blobNodeData_4.fRemain_Batt_Chrg_Pwr_Req, pData+nDataSize);
	nDataSize +=4;
 
    //6. ACDC分配给储能电池最大充电功率
    MakeFloat(blobNodeData_4.fACDC_Distri_4_BattChrg_Pwr, pData+nDataSize);
	nDataSize +=4;

	*(pData + nDataSize) = (BYTE)(blobNodeData_4.emBatt_WorkMode);
	nDataSize +=1;

	MakeFloat(blobNodeData_4.fBatt_Max_Chrg_Pwr_Req, pData+nDataSize);
	nDataSize +=4;

	nDataSize +=7;
 
	return nDataSize;
}


int PrepReplyBmsCmd06(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	int nDataSize = 0;
	int nRtn = 0;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = (REPORTER*)pClient->pReporter;

	*(pData + nDataSize) = nRtn;
	nDataSize += 1; 	//RTN = 0, Recv Data OK
	
	TRACE("PrepReplyBmsCmd06: nRtn=%d\n", nRtn);
	return nDataSize;
}

int PrepReplyEngFlowCmd08(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	int nDataSize = 0;
	int nRtn = nResult;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = (REPORTER*)pClient->pReporter;

	*(pData + nDataSize) = nRtn;
	nDataSize += 1; 	//RTN = 0, Recv Data OK
	
	TRACE("PrepReplyEngFlowCmd08: nRtn = %d\n", nRtn);
	return nDataSize;
}

int PrepReplySigRectCmd0A(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	int nDataSize = 0;
	int nRtn = nResult;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = (REPORTER*)pClient->pReporter;

	*(pData + nDataSize) = nRtn;
	nDataSize += 1; 	//RTN = 0, Recv Data OK
	
	TRACE("PrepReplySigRectCmd0A: nRtn =%d\n", nRtn);
	return nDataSize;
}

int PrepReplyBattCabCmd0C(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	int nDataSize = 0;
	int nRtn = nResult;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = (REPORTER*)pClient->pReporter;

	*(pData + nDataSize) = nRtn;
	nDataSize += 1; 	//RTN = 0, Recv Data OK
	
	TRACE("PrepReplyBattCabCmd0C:  nRtn=%d\n", nRtn);
	return nDataSize;
}


int PrepReplyStatisDataCmd0E(void* pArgs, OUT BYTE* pData, IN USHORT nResult)
{
	int nDataSize = 0;
	int nRtn = nResult;

	PARAM_VAL_GET 	valGet;
	ERRCODE_DAI 	err;

	INTER_CLIENT	*pClient = (INTER_CLIENT*)pArgs;
	REPORTER 		*pReporter = (REPORTER*)pClient->pReporter;

	*(pData + nDataSize) = nRtn;
	nDataSize += 1; 	//RTN = 0, Recv Data OK
	
	TRACE("PrepReplyStatisDataCmd0E:  nRtn=%d\n", nRtn);
	return nDataSize;
}


/*--------主柜准备控制分体柜的命令--------*/
int PrepareSetCfgCmdA1(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nDataLen);
	return 0;
}

int PrepareCtrlCmdA3(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nDataLen);

	return 0;
}

int PrepareAppSizeCmdA5(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nDataLen);
	return 0;
}

int PrepareAppUpdCmdA7(void* pArgs, OUT BYTE* pData, IN USHORT nDataLen)
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nDataLen);
	return 0;
}
 
/*--------主柜解析控制分体柜的回复命令--------*/
int ExtractSetCfgCmdA2(void* pArgs, IN BYTE* pData, OUT USHORT nResult) 
{
	UNUSED(nResult);
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	
	TRACE("SubCmd=pData[0]:%02X | RTN=pData[1]:%02X\n", pData[0], pData[1]);

	return 0;
}

int ExtractCtrlCmdA4(void* pArgs, IN BYTE* pData, OUT USHORT nResult) 
{
	UNUSED(pArgs);
	UNUSED(pData);
	UNUSED(nResult);
	return 0;
}

int ExtractAppSizeCmdA6(void* pArgs, IN BYTE* pData, OUT USHORT nResult) 
{
	UNUSED(nResult);
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	pClient->tagMstUpgradeParam.bInterSlvReady4Upgrade  = (((int)pData[0]) == 1) ? TRUE : FALSE;
	pData +=1;

	TRACE("ExtractCmdA6 : bInterSlvReady4Upgrade=%d\n",\
		pClient->tagMstUpgradeParam.bInterSlvReady4Upgrade);
	return 0;
}

int ExtractAppUpdCmdA8(void* pArgs, IN BYTE* pData, OUT USHORT nResult) 
{
	UNUSED(nResult);
	INTER_CLIENT* pClient = (INTER_CLIENT*)pArgs;
	pClient->tagMstUpgradeParam.nRecvMsgId = Bytes2Ulong(pData, 2); 
	pData +=2;

	pClient->tagMstUpgradeParam.emSlvUpdateStatus = (FIRM_UPDATESTT)pData[0];  
	pData +=1;
	TRACE("ExtractCmdA8 :nRecvMsgId=%d emSlvUpdateStatus=%d\n",\
		pClient->tagMstUpgradeParam.nRecvMsgId,pClient->tagMstUpgradeParam.emSlvUpdateStatus);

	return 0;
}
 