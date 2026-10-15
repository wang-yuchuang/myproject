#include "dev_485_2_sampler_main.h"

static void Make_YDN_ChkSum(IMEU2_FRM_INFO_V20 *pFrmeInfo)
{
	//TRACEX("Now beging to Calc CRC, pFrmaLen = %d\n", pFrmeInfo->nFrameLen);
	BYTE *pSrcBytes = (BYTE*)&pFrmeInfo->sendFrame;//Notice this needs #param pack(1), or else will be problems!!!
	pSrcBytes	+= 1; //except SOI
	BYTE		byTobeCalcBytes[100];
	memcpy(byTobeCalcBytes, pSrcBytes, 12);
	int nDataLen  = pFrmeInfo->nFrameLen - 13 - 5;
	if(nDataLen > 0)
	{
		memcpy(byTobeCalcBytes + 12, pFrmeInfo->sendFrame.pbyData, nDataLen);
	}
	int nCalcLen = pFrmeInfo->nFrameLen - 6; //except SOI EOI and chksum itself
	USHORT usCRC = CalcSendCheckSum(byTobeCalcBytes, nCalcLen);
	//TRACEX("Calc Chksum OK, now convert it to Hex.\n");
	HexToFourAsciiData(usCRC, (BYTE*)&pFrmeInfo->sendFrame.usChkSum[0]);
}


static void Init_AIMEU_Frame_V20(IMEU2_FRM_INFO_V20 *pFrameInfo, IMEU2_FRM_TYPE_V20 frmType_V20, 
			 UINT nCID2, UINT nLen, UINT nReplyDataLen, UINT nWhichIMEU)
{
	pFrameInfo->frmType_V20 	= frmType_V20;
	YDN_FRAME *pFrame 		= &pFrameInfo->sendFrame;
	pFrame->byFrameSOI 		= YDN_FRAME_SOI;
	pFrame->byEOI 			= YDN_FRAME_EOI;

	HexToTwoAsciiData(YDN_IMEU_FRAME_VER, pFrame->byProtVer);  
	HexToTwoAsciiData(YDN_FRAME_CID1, pFrame->byCID1);
	HexToTwoAsciiData(nCID2, pFrame->byCID2);
	HexToTwoAsciiData((YDN_FRAME_ADR + nWhichIMEU), pFrame->byDestAddr);  

	if(nLen > 0)
	{
		pFrame->pbyData = NEW(BYTE, nLen*2); //DataLen
		pFrameInfo->nDataLen = nLen*2;
	}

	pFrameInfo->nFrameLen = sizeof(YDN_FRAME) - 4 + nLen*2;
	pFrameInfo->nExpectAnswerLen = sizeof(YDN_FRAME) - 4 + nReplyDataLen*2;
	Make_YDN_Lengh(nLen*2, (BYTE*)pFrame->usLen);

	TRACEX("Init A Frame[%d], nFrameLen = %d, nDataLen = %d, ExpectAnsLen = %d, sizeof(YDN_FRAME) = %d.\n", 
		frmType_V20, pFrameInfo->nFrameLen, nLen, pFrameInfo->nExpectAnswerLen, sizeof(pFrameInfo->sendFrame));
	return;
}

#define INIT_IMEU_FRAME_V20(pFrameInfo, _frmType_V20, _nCID2, _nDataLen, _nAnswerDataLen, _nWhichIMEU) \
				do{ \
					Init_AIMEU_Frame_V20((pFrameInfo), (_frmType_V20), (_nCID2), (_nDataLen), (_nAnswerDataLen), (_nWhichIMEU)); \
					pFrameInfo++;\
				}while(0)


//static void InitIMEUData(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU)
//{
//	int n;
//	for(n = 0; n < _MAX_IMEU_ROUGH; n++)
//	{
//		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].varIMEURough[n].lValue = SAMP_INVALID_VALUE;
// 	}
//	return;
//}

void IMEU_InitRoughData_V20(RS485_2_SAMPLER_DATA *pDev_485_2_Rough)
{
    TRACE("------Now init the IMEU------\n");
	SAMPLER* pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	IMEU_FRAME_INFO *pFrame;
	VAR_VALUE varV;

	ZERO_POBJS(&pDev_485_2_Rough->blobEmsCfgPlan, 	1);
 	varV.blobValue = (BYTE *)&pDev_485_2_Rough->blobEmsCfgPlan;
	pDPR->Get((HANDLE)pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN));

	int kkk = 0;
	for(kkk = 0; kkk < MAX_IMEU_NUMBER; kkk++)
	{
		pFrame	= pDev_485_2_Rough->imeu2_RoughData[kkk].frmIMEU2_V20;
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7141_IMEU_V20,		0x41,	0x00,	2, 		kkk);
    	INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7142_ARICDT_V20,		0x42,	0x00,	22, 	kkk);
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7143_TEMPHUMI_V20,	0x43,	0x00,	6,  	kkk);
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7151_VER_V20,		0x51,	0x00,	52,  	kkk);
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7152_HD_AIRCDT_V20,	0x52,	0x00,	27,  	kkk);
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_GET_7153_GAS_SENSOR_V20,	0x53,	0x00,	13,  	kkk);

		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_SET_7144_SETPARAM_V20,	0x44,	0x05,	1,  	kkk);
		INIT_IMEU_FRAME_V20(pFrame,	TP_FRM_SET_7145_CTRL_V20,		0x45,	0x05,	1,  	kkk);

		ZERO_POBJS(&pDev_485_2_Rough->imeu2_RoughData[kkk].blobIMEU2_V20, 1);

		pDev_485_2_Rough->imeu2_RoughData[kkk].doValue.usFlag = 0;
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 450, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].emIMEU2Type = (IMEU_INSTALLED_TYPE)varV.emValue;
		pDev_485_2_Rough->imeu2_RoughData[kkk].bNeedReconfig = TRUE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].bNeedSyncSetParam = TRUE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].nInterrupt_Times = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].bIsCommFail = FALSE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].bMonitorShutter = FALSE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].nShutterCtrlCmd = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].nShutterCtrlTimes = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].nShutterOpenResult = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].nShutterClosedResult = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].bCabSafeStateChanged = FALSE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].pCurr_StateMachineItem = &pDev_485_2_Rough->StateMachineLoop[CAB_SAFE_STATE_IDLE];
 		pDev_485_2_Rough->imeu2_RoughData[kkk].emManageCtrlMode = MANAGE_CTRL_MODE_AUTO;
 		pDev_485_2_Rough->imeu2_RoughData[kkk].emTempImbalance = 0;
		pDev_485_2_Rough->imeu2_RoughData[kkk].blobIMEU2_V20.emIMEU2BMSCommState = 0;
		
		pDev_485_2_Rough->imeu2_RoughData[kkk].tmLastCtrlAirCond = _NOW_;//John  2022-02-24 Add，When PowerON, Not Ctrl AirCond
		pDev_485_2_Rough->imeu2_RoughData[kkk].tm_LastChangeECOMode = _NOW_;
		pDev_485_2_Rough->imeu2_RoughData[kkk].tmLastStartCool_AirCond = _NOW_;
		pDev_485_2_Rough->imeu2_RoughData[kkk].emECORunStatu = STAT_NORMAL_RUN;

		pDev_485_2_Rough->imeu2_RoughData[kkk].blobIMEU2_V20.emAirMachineState = STAT_AIR_IDLE;
		pDev_485_2_Rough->imeu2_RoughData[kkk].blobIMEU2_V20.emLast_AirMachineState = STAT_AIR_IDLE;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 487, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].nCombGasAlarmPoint = varV.ulValue;
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 488, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].nCombGasStopRetDiff = varV.ulValue;

		//pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_SP_RAW_VAL, 501 + kkk, 550, &varV, 4);
		//pDev_485_2_Rough->imeu2_RoughData[kkk].emLast_BattStatus_Run = (STBATT_RUN_STATU)varV.emValue;

		//pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 501, 500, &varV, 4);

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 464, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fCoolPoint_Sync = varV.fValue;
		//pDev_485_2_Rough->imeu2_RoughData[kkk].fLast_SetCoolPoint2AirCond = varV.fValue;

		//pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 490, &varV, 4);
		//pDev_485_2_Rough->imeu2_RoughData[kkk].fChrgCoolPoint = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 494, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].nPT1000Num = varV.lValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 601, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fAmbit_TcoolonSett = varV.fValue;
		
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 602, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fBatt_Tmax_coolonSett = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 603, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fAmbit_TcooloffSett = varV.fValue;
		
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 604, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fBatt_Tmax_cooloffSett = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 605, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fAirEnTran_TlmtSett = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 606, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fAmbit_TheatonSett = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 607, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fHumidity_StartSett = varV.fValue;

		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + kkk, 608, &varV, 4);
		pDev_485_2_Rough->imeu2_RoughData[kkk].fECOModeTempOffset = varV.fValue;
	}

 	return ;
}

static BYTE* MakeIMEUSendBuff(IN RS485_2_SAMPLER_DATA *pDev_485_2_Rough,IMEU2_FRM_INFO_V20 *pFrameInfo)
{
	BYTE* pbyBuff = pDev_485_2_Rough->abySendBuf;
	BYTE* pbyBuffOrig = pbyBuff;
	
	memcpy(pbyBuff, (BYTE* )&pFrameInfo->sendFrame, 13); //NOTE: This needs #pragma pack(1), or else will be problem
	pbyBuff += 13;

	int nDataLen = pFrameInfo->nFrameLen - 13 - 5;
	if(nDataLen > 0)
	{
		memcpy(pbyBuff, pFrameInfo->sendFrame.pbyData, nDataLen);
	}
	pbyBuff += nDataLen;
	memcpy(pbyBuff, (BYTE*)&pFrameInfo->sendFrame.usChkSum[0], 4);
	
	pbyBuff += 4;
	*pbyBuff = pFrameInfo->sendFrame.byEOI;

	return pbyBuffOrig;
}

/*
7E32314238373130304330303434313034464342420D

7E 32 31 42 38 37 31 30 30 43 30 30 34 34 31 30 34 46 43 42 42 0D

7E3231423837313030323032433432303330303030453330304533303045333030464130304245303044323030424530313232303036343031463435380D

7E323142383731303034303043343330304536303230433030464230440D

*/
#define ERR_YDN_LEN	18
int Pack_IMEUCmd_AndGetReply_V20(IN RS485_2_SAMPLER_DATA *pDev_485_2_Rough, IN IMEU2_FRM_INFO_V20* pimeuFrame, BOOL bCtrl, UINT nWhichIMEU)
{
	int nWriteBytes = 0;
	int nReadBytes = 0;
	//int i = 0;
	if(pimeuFrame == NULL)
	{
		TRACE("Error:IMEU Frame is NULL,Please Chech U Code!\n");
		return RS485_WRITE_ERR;
	}
 
	COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;
    //change baudrate
    char szSett[] = "9600, N, 8, 1";
	pComm->Control(pDev_485_2_Rough->hPort,
			COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	
	//John 2021-02-06 clear buffer, maybe LCCU send data has delay
	pComm->Control(pDev_485_2_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);

	Sleep(500);
	//Now beging send frame 
	//TRACEX("Now begin to write IMIU cmd!\n");
	Make_YDN_ChkSum(pimeuFrame);
	//TRACEX("MakeYDNChkSum for IMIU cmd!\n");
	
	BYTE abySendBuf[RS485_SEND_BUF_LEN];
	memset(abySendBuf, 0, RS485_SEND_BUF_LEN);

 	memcpy(abySendBuf, MakeIMEUSendBuff(pDev_485_2_Rough, pimeuFrame), pimeuFrame->nFrameLen);
       
	nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, abySendBuf, pimeuFrame->nFrameLen);
	
	//TRACE("nWhichIMEU=[%d] Now Trying to send out[%s] nSendlen=%d nWriteSuccessBytes=[%d] nExpectAnswerLen=[%d] bCtrl=[%d]", \
	//	nWhichIMEU, abySendBuf,pimeuFrame->nFrameLen, nWriteBytes, pimeuFrame->nExpectAnswerLen, bCtrl);
	
	//printf("Send to imue2 bytes[%d]:\n", pimeuFrame->nFrameLen);	
	//for(i =0; i<pimeuFrame->nFrameLen; i++)
	//{
	//	printf("%02X ", abySendBuf[i]);
	//}
	//printf("\n");
	
	if(nWriteBytes < pimeuFrame->nFrameLen)
	{
		if(bCtrl)
		{
			TRACE("-------Send IMEU Cmd WriteERRRRRRRRRRRRRR!----------\n");
		}
		return RS485_WRITE_ERR;
	}

	TRACEX("write IMIU cmd OK! Now begin read!\n");
	Sleep(150);
	//if(bNeedGetReply)
	{
		nReadBytes = pComm->Read(pDev_485_2_Rough->hPort, pDev_485_2_Rough->abyRcvBuf, RS485_RECV_BUF_LEN/*(size_t)pimeuFrame->nExpectAnswerLen*/);
		
		//printf("ReadBack from IMEU bytes[%d]:\n", nReadBytes);
		//for(i =0; i < nReadBytes; i++)
		//{
		//	printf("%02X ",pDev_485_2_Rough->abyRcvBuf[i]);
		//}
		//printf("\n");
		
		if(bCtrl)
		{
			//TRACEX("Read IMIU cmd OK! readLen = %d, Buff = [%s]!\n", nReadBytes, g_RS485_1SamplerData.abyRcvBuf);
		}
		
		if(nReadBytes <= 0)
		{
			return RS485_READ_ERR;
		}
		//if((nReadBytes < pimiuFrame->nExpectAnswerLen) && (nReadBytes != ERR_YDN_LEN))
		//{
		//	Sleep(2000); //Found the IMIU can sometimes not reply immediately
		//	nReadBytes += RS485_Read(g_RS485_1SamplerData.hPort, pimiuFrame->nExpectAnswerLen - nReadBytes,
		//		g_RS485_1SamplerData.abyRcvBuf + nReadBytes);
		//	TRACEX("Read IMIU AGAINNNNNNN cmd OK! readLen = %d!\n", nReadBytes);
		//	//return RS485_READ_ERR;
		//}
	}
	pDev_485_2_Rough->nReadBuffBytes = nReadBytes;
	return RS485_COMM_OK;
}

#define  RTN_OK	0
static BOOL ExtractIMEUFrame_V20(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IMEU2_FRM_INFO_V20 *pFrameInfo, UINT nWhichIMEU)
{
	BYTE 		byTemp;
 	int 		n = 0, m = 0;
	int 		nStart = -1;
	int 		i = 0;
	SHORT_VAR 	s_Val;
	short 		nValue;
 	USHORT      nLength = 0;
 	USHORT      nCheckSum = 0;
	 
	BYTE* pBuff = pDev_485_2_Rough->abyRcvBuf;
	if(pFrameInfo == NULL)
	{
		TRACE("Error:Extract IMEU Frame Failed,Frame Info Is NULL!\n");
		return FALSE;
	}

	for(n = 0; n < pDev_485_2_Rough->nReadBuffBytes; n++)
	{
		//2021-09-16 John Modify it, Not Used ExpectedLen to Judge  
		//TRACE("%02X-%02X-%02X-%02X-",pBuff[n],pBuff[n+1],pBuff[n+2],pBuff[n+3]);
		if(pBuff[n] == YDN_FRAME_SOI)
		{
			BYTE byVer = 	Ascii2ToHex(&pBuff[n+1]);
			BYTE byAddr = 	Ascii2ToHex(&pBuff[n+3]);
			BYTE byCID1 = 	Ascii2ToHex(&pBuff[n+5]);
			
			//TRACE("Info:%02X-%02X-%02X\n",byVer,byAddr,byCID1);
	
			if((byVer == YDN_IMEU_FRAME_VER) &&
		   	(byAddr == (BYTE)(YDN_FRAME_ADR + nWhichIMEU))&&
		   	(byCID1 == YDN_FRAME_CID1))
			{
				nStart = n;
			}
			//m = n + pFrameInfo->nExpectAnswerLen - 1;
			//if((m < pDev_485_2_Rough->nReadBuffBytes) && (pBuff[m] == YDN_FRAME_EOI))
			//{
			//	nStart = n;
			//}
		}
	}
	if(nStart == -1)
	{
		TRACE("Can not find the YDN Start, ExpectLen = %d, nReadBytes = %d\n", pFrameInfo->nExpectAnswerLen, pDev_485_2_Rough->nReadBuffBytes);
		return FALSE;
	}
	BOOL bExtractOK = TRUE;
	YDN_FRAME *pRecFrame = (YDN_FRAME*)(pBuff + nStart);
	//Now judge if answer frame is OK/**/
	if((Ascii2ToHex(pRecFrame->byCID1) != Ascii2ToHex(pFrameInfo->sendFrame.byCID1)) ||
		/*(Ascii2ToHex(pRecFrame->byCID2) != RTN_OK) ||*/
		(Ascii2ToHex(pRecFrame->byDestAddr) != Ascii2ToHex(pFrameInfo->sendFrame.byDestAddr))) //jimmy note: do not do checksum check/**/
	{
		TRACE("CID1 or DestAddr not the same, for CID2[%02x]\n", Ascii2ToHex(pRecFrame->byCID2));
		return FALSE;
	}
	BYTE nRTN = Ascii2ToHex(pRecFrame->byCID2);
	if(nRTN != RTN_OK)
	{
		TRACE("Error: RTC Check ;RTN = %d, for CID2[%02x]\n", nRTN, Ascii2ToHex(pFrameInfo->sendFrame.byCID2));
		return FALSE;
	}

	BYTE* pbyDataStart = pBuff + nStart + 13;
	//Check Length
 	nLength = (USHORT)Ascii4ToShort(pbyDataStart - 4) & 0x0FFF; //LENGTH = INFO Length
 	if((nLength > 4078) || (pbyDataStart[nLength + 4] != YDN_FRAME_EOI))
 	{
 		TRACEX("IMEU Error: Data Packet Length is Abnormal, Info Len=%d\n", nLength);
  		return FALSE;
 	}
 
 	//Check SUM
 	nCheckSum = CalcSendCheckSum(pbyDataStart - 12, nLength + 12);  //Ver ~ Info End
 	if(nCheckSum != (USHORT)Ascii4ToShort(pbyDataStart + nLength))
 	{
 		TRACE("IMEU Error: Check Sum=[%d] Failed\n", nCheckSum);
  		return FALSE;
 	}

	//Check CID2 ACK
	BYTE byACKCID2 = Ascii2ToHex(pbyDataStart);
	pbyDataStart += 2;
	if(byACKCID2 != Ascii2ToHex(pFrameInfo->sendFrame.byCID2))
	{
		TRACE("Error: ACK CID2 Check; byACKCID2 = %d, for CID2[%02x]\n", byACKCID2, Ascii2ToHex(pFrameInfo->sendFrame.byCID2));
		return FALSE;
	}

	A_BLOB_BATTCAB_IMEU2_V20	*pBlobIMEU2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	VAR_VALUE varV;
	switch(pFrameInfo->frmType_V20)
	{
		case TP_FRM_GET_7141_IMEU_V20:
		{
			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->emDI1_Fire = 			  (UINT)byTemp & 0x01;
			pBlobIMEU2->emDI2_Smoke = 			  (UINT)((byTemp >> 1) & 0x01);
			pBlobIMEU2->emDI3_DoorOpen = 		  (UINT)((byTemp >> 2) & 0x01);
			pBlobIMEU2->emDI4_FlammableGas = 	  (UINT)((byTemp >> 3) & 0x01);
			pBlobIMEU2->emDI6_ShutterClosedOver = (UINT)((byTemp >> 5) & 0x01); //0-Close ok, 1-Not Close Totally
			pBlobIMEU2->emAirCond_CommFail = 	  (UINT)((byTemp >> 6) & 0x01);
			pBlobIMEU2->emFlood = 	              (UINT)((byTemp >> 7) & 0x01);
			pbyDataStart += 2;
			//printf("WhichIMEU:%d, byTemp:%d, emDI1_Fire:%d, emDI2_Smoke:%d, emDI3_DoorOpen:%d, emDI4_FlammableGas:%d \
			//	emDI6_ShutterClosedOver:%d, emAirCond_CommFail:%d, emFlood:%d\n", \
			//	nWhichIMEU, byTemp, pBlobIMEU2->emDI1_Fire, pBlobIMEU2->emDI2_Smoke, pBlobIMEU2->emDI3_DoorOpen, pBlobIMEU2->emDI4_FlammableGas, \
			//	pBlobIMEU2->emDI6_ShutterClosedOver, pBlobIMEU2->emAirCond_CommFail, pBlobIMEU2->emFlood);
			
			byTemp = Ascii2ToHex(pbyDataStart);
			for(i = 0; i < MAX_HUMITEMP_NUM; i++)
			{
				pBlobIMEU2->emTempHumi_CommFail[i] = (UINT)((byTemp >> i) & 0x01);
				//printf("WhichIMEU:%d, byTemp:%d, TempHumi[%d]: emTempHumi_CommFail:%d\n", nWhichIMEU, byTemp, i, pBlobIMEU2->emTempHumi_CommFail[i]);
			}
			pbyDataStart += 2;
			
			byTemp = Ascii2ToHex(pbyDataStart);
			for(i = 0; i < MAX_GASSENSOR_NUM; i++)
			{
				pBlobIMEU2->emGasSensor_CommFail[i] = (UINT)((byTemp >> i) & 0x01);
				//printf("WhichIMEU:%d, byTemp:%d, GasSensor[%d]: emGasSensor_CommFail:%d\n", nWhichIMEU, byTemp, i, pBlobIMEU2->emGasSensor_CommFail[i]);
			}
			pbyDataStart += 2;

			pbyDataStart += 2*20;//Reserved

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nAirCondiType_Cfg = (UINT)byTemp;
			pbyDataStart += 2;

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nHumiTempSensorNumb_Cfg = (UINT)byTemp;
			pbyDataStart += 2;

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nCombGasSensorEnable_Cfg = (UINT)byTemp;
			pbyDataStart += 2;

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nCombGasSensorType_Cfg = (UINT)byTemp;
			pbyDataStart += 2;

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nCombGasSensorNumb_Cfg = (UINT)byTemp;
			pbyDataStart += 2;

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nFireProtectType = (UINT)byTemp;
			pbyDataStart += 2;

			//printf("WhichIMEU:%d, nAirCondiType=[%d] nHumiTempSensorNumb=[%d]\
			//	nCombGasSensorEnable=[%d] nCombGasSensorType=[%d] nCombGasSensorNumb=[%d]\n", \
			//	nWhichIMEU, pBlobIMEU2->nAirCondiType_Cfg,\
			//	pBlobIMEU2->nHumiTempSensorNumb_Cfg,\
			//	pBlobIMEU2->nCombGasSensorEnable_Cfg,\
			//	pBlobIMEU2->nCombGasSensorType_Cfg,\
			//	pBlobIMEU2->nCombGasSensorNumb_Cfg);

			pbyDataStart += 2*19;//Reserved

			for(i=0; i<MAX_PT1000_NUM; i++)
			{
				pBlobIMEU2->fPT1000Temperature[i] = (float)Ascii4ToShort(pbyDataStart) * 0.1;
				//printf("WhichIMEU:%d, byTemp:%d, fPT1000Temperature:%f\n",nWhichIMEU, byTemp, pBlobIMEU2->fPT1000Temperature[i]);
				pbyDataStart += 4;
			}

			byTemp = Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->emShutterOpenedOver = (UINT)byTemp & 0x01; //0-Opened Ok, 1-Not Open Totally
			pBlobIMEU2->emIMEU2BMSCommState = (UINT)((byTemp >> 1) & 0x01);
			pBlobIMEU2->emAirCondHWAlarm = (UINT)((byTemp >> 2) & 0x01);
			pbyDataStart += 2;
			TRACE("WhichIMEU:%d, byTemp:%d, emShutterOpenedOver:%d, emIMEU2BMSCommState:%d emAirCondHWAlarm=[%d]\n", \
				nWhichIMEU, byTemp, \
				pBlobIMEU2->emShutterOpenedOver, \
				pBlobIMEU2->emIMEU2BMSCommState,\
				pBlobIMEU2->emAirCondHWAlarm);

			pBlobIMEU2->nExhaustFanSpeed = (UINT)Ascii4ToShort(pbyDataStart);
			//printf("WhichIMEU:%d, nExhaustFanSpeed:%d\n", nWhichIMEU, pBlobIMEU2->nExhaustFanSpeed);
			pbyDataStart += 4;

			
		}
		break;
		case TP_FRM_GET_7142_ARICDT_V20:
		{	
			TRACE("Warning: V20 Not Support QILI Air Condition!\n");

		}break;
		
		case TP_FRM_GET_7143_TEMPHUMI_V20:
		{
			//High is First, Lower is Last
			//1. Humi&Temp Sensor Number
			pBlobIMEU2->nHumiTempSensorNumber = (UINT)Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nHumiTempSensorNumber = MIN(MAX_HUMITEMP_NUM, pBlobIMEU2->nHumiTempSensorNumber);
			pbyDataStart += 2;

			for(i = 0; i < pBlobIMEU2->nHumiTempSensorNumber;i++)
			{
				pBlobIMEU2->fTemperature[i] = (float)Ascii4ToShort(pbyDataStart)*0.1;
				pbyDataStart += 4;
				
				pBlobIMEU2->fHumidity[i] = (float)Ascii4ToShort(pbyDataStart)*0.1;
				pbyDataStart += 4;

				//Humi&Temperature Alarm
				pbyDataStart += 2;

				TRACE("Numb[%d] : fTemperature=[%.1f],fHumidity=[%.1f]\n", \
					i, pBlobIMEU2->fTemperature[i], pBlobIMEU2->fHumidity[i]);
			}
		}break;
		
		case TP_FRM_GET_7151_VER_V20:
		{
			pbyDataStart += 20; //szIMEUName
			
			pbyDataStart += 4;  //szHardwareVer
	 		
			nValue =  (char)Ascii2ToHex(pbyDataStart)*100 + (char)Ascii2ToHex(pbyDataStart + 2 * 1);
			snprintf(pBlobIMEU2->szSoftwareVer, 7, "V%.2f", (float)(nValue*0.01));
			TRACE("TP_FRM_GET_7151_VER: CVer %02X | %02X\n", (char)Ascii2ToHex(pbyDataStart), (char)Ascii2ToHex(pbyDataStart + 2 * 1));

			pbyDataStart += 4;  //szSoftwareVer
	 		
			pbyDataStart += 4;  //ConfigVer
			
			pbyDataStart += 50;  //SN

			pbyDataStart += 2;  //IMEU2 software type
			pbyDataStart += 2;  //IMEU2 project type
			
			pbyDataStart += 18;   //Resv
		 
		}break;

		case TP_FRM_GET_7152_HD_AIRCDT_V20:
		{
			nValue = Ascii4ToShort(pbyDataStart);

			pBlobIMEU2->emOnOrOffd = (UINT)nValue & 0x01;
			pBlobIMEU2->emStartRefrigMode = (UINT)((nValue >> 1) & 0x01);;
			pBlobIMEU2->emStartSupplyAirMode = (UINT)((nValue >> 2) & 0x01);
			pBlobIMEU2->emStartStandbyMode = (UINT)((nValue >> 3) & 0x01);
			pBlobIMEU2->emStartHeatMode = (UINT)((nValue >> 4) & 0x01);

			pBlobIMEU2->emSensorFail = (UINT)((nValue >> 5) & 0x01);
			pBlobIMEU2->emHighOrLowVoltAlm = (UINT)((nValue >> 6) & 0x01);
			pBlobIMEU2->emHighOrLowTempAlm = (UINT)((nValue >> 7) & 0x01);
			pBlobIMEU2->emHighOrLowPressureAlm = (UINT)((nValue >> 8) & 0x01);
			pBlobIMEU2->emCompressorAlm = (UINT)((nValue >> 9) & 0x01);
			
			pBlobIMEU2->emWholeMachineStatus = (UINT)((nValue >> 10) & 0x01);
			pBlobIMEU2->emInternalFanStatus = (UINT)((nValue >> 11) & 0x01);
			pBlobIMEU2->emOutsideFanStatus = (UINT)((nValue >> 12) & 0x01);
			pBlobIMEU2->emCompressorStatus = (UINT)((nValue >> 13) & 0x01);
			pBlobIMEU2->emElectricHeatStatus = (UINT)((nValue >> 14) & 0x01);
			pBlobIMEU2->emForceAutoMode = (UINT)((nValue >> 15) & 0x01);

			//pBlobIMEU2->emAirCondTotalAlm = ((nValue >> 5) & 0x7F) > 0 ? 1 : 0;
			pbyDataStart += 4;

			pBlobIMEU2->fCoolSetPoint = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;		
			//Heidun:(cooling on = cooling set point + cooling deviation )
			//Heidun:(cooling off = cooling set point )

			//Tongfei: (Cooling on = cooling set point + cooling deviation )
			//Tongfei: (Cooling off = cooling set point - cooling deviation )

			pBlobIMEU2->fCoolDeviation = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;		

			pBlobIMEU2->fHighTempAlmValue = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fLowTempAlmValue = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fHeatSetPoint = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;		//(heating on = heating set point)

			pBlobIMEU2->fHeatDeviation = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;		//(heating stop = heating set point + heating deviation)

			pBlobIMEU2->fRealTimeCabTemp = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fRealTimeCabHumi = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fDehumiStartTemp = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fDehumiStopTemp = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fDehumiStartHumi = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			pBlobIMEU2->fDehumiStopHumi = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;
			
			pBlobIMEU2->fDehumiRetDiffHumi = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			nValue = Ascii4ToShort(pbyDataStart);
			pBlobIMEU2->emHighTempAlm = (UINT)nValue & 0x01;
			pBlobIMEU2->emLowTempAlm = (UINT)((nValue >> 1) & 0x01);;
			pBlobIMEU2->emHighHumiAlm = (UINT)((nValue >> 2) & 0x01);
			pBlobIMEU2->emLowHumiAlm = (UINT)((nValue >> 3) & 0x01);
			pBlobIMEU2->emPlateTubeTempSensorFail = (UINT)((nValue >> 4) & 0x01);
			pBlobIMEU2->emCondensationTempSensorFail = (UINT)((nValue >> 5) & 0x01);
			pBlobIMEU2->emInternalTempSensorFail = (UINT)((nValue >> 6) & 0x01);
			pBlobIMEU2->emHighVoltAlm = (UINT)((nValue >> 7) & 0x01);
			pBlobIMEU2->emLowVoltAlm = (UINT)((nValue >> 8) & 0x01);
			pBlobIMEU2->emPhaseSequenceAlm = (UINT)((nValue >> 9) & 0x01);
			pbyDataStart += 4;

			pBlobIMEU2->fPlateTubeTemp = ((float)Ascii4ToShort(pbyDataStart)) * 0.1;
			pbyDataStart += 4;
			pBlobIMEU2->fCondensationTemp = ((float)Ascii4ToShort(pbyDataStart)) * 0.1;
			pbyDataStart += 4;
			pBlobIMEU2->fHighHumiPoint = (float)Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;
			pBlobIMEU2->nTongFeiSW_Ver = Ascii4ToShort(pbyDataStart);
			pbyDataStart += 4;

			TRACE("nValue=[%d] fCoolSetPoint=[%.1f] fCoolDeviation=[%.1f]\
				fHighTempWarnValue=[%.1f] fLowTempWarnValue=[%.1f]\
				fHeatSetPoint=[%.1f] 	fHeatDeviation=[%.1f]\
				fRealTimeCabTemp=[%.1f] fRealTimeCabHumi=[%.1f]\
				fDehumiStartTemp=[%.1f] fDehumiStopTempe=[%.1f]\
				fDehumiStartHumi=[%.1f] fDehumiStopHumi=[%.1f]\n",nValue, 	
				pBlobIMEU2->fCoolSetPoint,		 
				pBlobIMEU2->fCoolDeviation,		 
				pBlobIMEU2->fHighTempAlmValue,
				pBlobIMEU2->fLowTempAlmValue,
				pBlobIMEU2->fHeatSetPoint,		 
				pBlobIMEU2->fHeatDeviation, 	 
				pBlobIMEU2->fRealTimeCabTemp,
				pBlobIMEU2->fRealTimeCabHumi,
				pBlobIMEU2->fDehumiStartTemp,
				pBlobIMEU2->fDehumiStopTemp,
				pBlobIMEU2->fDehumiStartHumi,
				pBlobIMEU2->fDehumiStopHumi);
		}break;
		case TP_FRM_GET_7153_GAS_SENSOR_V20:
		{
			pBlobIMEU2->nGasSensorNum = (UINT)Ascii2ToHex(pbyDataStart);
			pBlobIMEU2->nGasSensorNum = MIN(MAX_GASSENSOR_NUM, pBlobIMEU2->nGasSensorNum);
			pbyDataStart += 2;

			for(i = 0; i < pBlobIMEU2->nGasSensorNum; i++)
			{
				varV = Ascii8ToVarV(pbyDataStart);
				pBlobIMEU2->fCombGasPotency[i] = varV.fValue;
				pbyDataStart += 8;

				TRACE("GasSensorNumb[%d] : fCombGasPotency=[%.1f]\n", pBlobIMEU2->nGasSensorNum, pBlobIMEU2->fCombGasPotency[i]);
			}
		}
		break;

		case TP_FRM_SET_7144_SETPARAM_V20:
			TRACEX("Now Parse Setting cmdType[0x%02X] for 7044\n", Ascii2ToHex(pFrameInfo->sendFrame.pbyData));
			//do nothing, only judge RTN
			break;
		
		case TP_FRM_SET_7145_CTRL_V20:
			TRACEX("Now Parse Ctrl cmdType[0x%02X] for 7045\n", Ascii2ToHex(pFrameInfo->sendFrame.pbyData));
			//do nothing, only judge RTN
			break;

		default:
			bExtractOK = FALSE;
			break;
	}
	return bExtractOK;
}

static void IMEU_PushAllData_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichIMEU)
{
	SAMPLER* pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobIMEU2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;
	BATT_MANUFACTURER emBattMFG;
	
	VAR_VALUE varV;
	VAR_VALUE varStuff;
	int emAlmValue_DI1, emAlmValue_DI2, emAlmValue_DI3, emAlmValue_DI4, emFlood;
	int n = 0;
 
	pBlobIMEU2->emIMEU2_CommFail = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail;

	//	DI1-Flood Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 460, &varV, 4);
	emAlmValue_DI1 = (pBlobIMEU2->emDI1_Fire == varV.emValue) ? 1 : 0;
	varStuff.emValue = emAlmValue_DI1;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU , 452, varStuff);//火探管告警
 
	//	DI2-Smoke Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 461, &varV, 4);
	emAlmValue_DI2 = (pBlobIMEU2->emDI2_Smoke == varV.emValue) ? 1 : 0;
	varStuff.emValue = emAlmValue_DI2;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU ,453, varStuff);//烟感告警
 
	//	DI3-Door Open Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 462, &varV, 4);
	if(varV.emValue == 2)
	{
		emAlmValue_DI3 = 0;
	}
	else
	{
		emAlmValue_DI3 = (pBlobIMEU2->emDI3_DoorOpen == varV.emValue) ? 1 : 0;
	}
	varStuff.emValue = emAlmValue_DI3;
	pBlobIMEU2->emDI3_DoorOpen = varStuff.emValue;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 454, varStuff);//电池柜前门被打开
	
	//	DI4-Flammable Gas Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 463, &varV, 4);
	emAlmValue_DI4 = (pBlobIMEU2->emDI4_FlammableGas == varV.emValue) ? 1 : 0;
	varStuff.emValue = emAlmValue_DI4;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 455, varStuff);//可燃气体告警
	
	//AirCond Alarm
	//varStuff.emValue = pBlobIMEU2->emAirCondTotalAlm;
	//Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 456, varStuff);

	varStuff.emValue = pBlobIMEU2->emSensorFail;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 463, varStuff);//空调传感器故障

	varStuff.emValue = pBlobIMEU2->emHighOrLowVoltAlm;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 464, varStuff);//空调高低电压告警

	varStuff.emValue = pBlobIMEU2->emHighOrLowTempAlm;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 465, varStuff);//空调高低温告警

	varStuff.emValue = pBlobIMEU2->emHighOrLowPressureAlm;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 466, varStuff);//空调高低压告警

	varStuff.emValue = pBlobIMEU2->emCompressorAlm;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 467, varStuff);//空调压缩机告警

	varStuff.emValue = pBlobIMEU2->emAirMachineState;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 451, varStuff);//空调当前状态机

	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 500, 500, &varV, 4);
	emBattMFG = varV.emValue;
	if((emBattMFG == BATT_MFG_NIFY_V33_500K) || (emBattMFG == BATT_MFG_INFY_SELF_500K))
	{
		varStuff.emValue = pBlobIMEU2->emIMEU2BMSCommState;
	}
	else
	{
		varStuff.emValue = 0;
	}
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 468, varStuff);//IMEU2和BMS通信状态


	varV.ulValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emECORunStatu;
	Push_VarValue(SETTYPE_LOGIC_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 469, varV);//空调节能模式

	varV.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emStatCode;
	Push_VarValue(SETTYPE_LOGIC_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 470, varV);//节能模式状态码
	
	//AirCond CommFail Alarm
 	varStuff.emValue = pBlobIMEU2->emAirCond_CommFail;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 457, varStuff);//AC空调通信中断
	
	//Working:Disconnect, Power-Off:Connect, Standby-Connect, Fault-Connect
	#define MAX_ALARM_FILTER	20
	_ENUM emOrgAirCondHDAlm = pBlobIMEU2->emAirCondHWAlarm & pBlobIMEU2->emOnOrOffd; 
	varStuff.emValue = 0;
	if(emOrgAirCondHDAlm == 1)
	{
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nAirCondAlmTimes++;
		if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nAirCondAlmTimes >= MAX_ALARM_FILTER)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nAirCondAlmTimes = MAX_ALARM_FILTER;
			varStuff.emValue = 1;
		}
	}
	else
	{
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nAirCondAlmTimes = 0;
	}
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 458, varStuff);//空调硬件告警
	
	//TempHumi CommFial Alarm
 	//varStuff.emValue = pBlobIMEU2->emTempHumi_CommFail;
	//Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 458, varStuff);
	
	//Electric Shutters Completely Closed
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 491, &varV, 4);
	varStuff.emValue = (pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterClosedResult == varV.emValue) ? 1 : 0;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 459, varStuff);//电动百叶完全关闭状态

	//Electric Shutters Completely Opened
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 492, &varV, 4);
	varStuff.emValue = (pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterOpenResult == varV.emValue) ? 1 : 0;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 460, varStuff);//电动百叶完全打开状态
	
	//Flooding Alarm
	pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 601 + nWhichIMEU, 493, &varV, 4);
	emFlood = (pBlobIMEU2->emFlood == varV.emValue) ? 1 : 0;
	varStuff.emValue = emFlood;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 461, varStuff);//电池柜-水浸告警

	//Fire Warn Satge send to batt sampler
	varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].pCurr_StateMachineItem->emState;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 462, varStuff);//电池柜保护等级

	//Gas Sensor Comm Fail
	for(n = 0; n < MAX_GASSENSOR_NUM; n++)
	{
		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emGasSensor_CommFail[n];
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 433 + n, varStuff);//可燃气体#1通信中断
	}

	//Temp Humi Comm Fail
	for(n = 0; n < MAX_HUMITEMP_NUM; n++)
	{
		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emTempHumi_CommFail[n];
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 437 + n, varStuff);//温湿度传感器#1通信中断
	}

	varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emTempImbalance;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 471, varStuff);//气体传感器在线数量
	
	Push_BlobValue(SETTYPE_RAW_SP, pSampler, 601 + nWhichIMEU, 450, \
		(void*)&pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20, sizeof(A_BLOB_BATTCAB_IMEU2_V20), BLOB_TYPE_BATTCAB_IMEU2_V20);


	//TongFei /ggaidingAirConditon Add
	if((pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.nAirCondiType_Cfg == 2)||
	(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.nAirCondiType_Cfg == 3))
	{
		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emWholeMachineStatus;//整机状态(TF)
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 472, varStuff);

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emInternalFanStatus;//内风机状态(TF)
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 473, varStuff);

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emOutsideFanStatus;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 474, varStuff);//外风机状态(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emCompressorStatus;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 475, varStuff);//压缩机状态(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emElectricHeatStatus;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 476, varStuff);//电加热状态(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emForceAutoMode;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 477, varStuff);//强制制冷模式(TF)

		varStuff.fValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.fDehumiRetDiffHumi;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 478, varStuff);//除湿回差(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emHighTempAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 479, varStuff);//高温告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emLowTempAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 480, varStuff);//低温告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emHighHumiAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 481, varStuff);//高湿告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emLowHumiAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 482, varStuff);//低湿告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emPlateTubeTempSensorFail;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 483, varStuff);//盘管温感失效(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emCondensationTempSensorFail;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 484, varStuff);//冷凝温感失效(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emInternalTempSensorFail;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 485, varStuff);//内温感失效(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emHighVoltAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 486, varStuff);//高压告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emLowVoltAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 487, varStuff);//低压告警(TF)

		varStuff.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.emPhaseSequenceAlm;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 488, varStuff);//相序告警(TF)

		varStuff.fValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.fPlateTubeTemp;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 489, varStuff);//管盘温度(TF)

		varStuff.fValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.fCondensationTemp;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 490, varStuff);//冷凝温度(TF)

		varStuff.fValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.fHighHumiPoint;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 491, varStuff);//高湿点(TF)

		varStuff.ulValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20.nTongFeiSW_Ver;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 492, varStuff);//同飞空调软件版本
	}
	//TRACE("IMIU_ROUGH_DI1=%d IMIU_ROUGH_DI2=%d IMIU_ROUGH_DI3=%d IMIU_ROUGH_DI4=%d\
	//		emAlmValue_DI1=%d emAlmValue_DI2=%d emAlmValue_DI3=%d emAlmValue_DI4=%d\n",\
	//		pBlobIMEU2->emDI1_Fire,\
	//		pBlobIMEU2->emDI2_Smoke,\
	//		pBlobIMEU2->emDI3_DoorOpen,\
	//		pBlobIMEU2->emDI4_Flood,\
	//		emAlmValue_DI1, emAlmValue_DI2, emAlmValue_DI3, emAlmValue_DI4);
	
	return;
}

static 	void Sync2SettingPararm(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichIMEU)
{
	//SAMPLER* pSampler 		= (SAMPLER*)pDev_485_2_Rough->hSampler;
	//DATA_PROCESSOR* pDPR 	= (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE 		varV;
	A_BLOB_BATTCAB_IMEU2_V20 *pblobIMEU2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

	//pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fLast_SetCoolPoint2AirCond = 	pblobIMEU2->fCoolSetPoint;
	//pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fLast_SetCoolRet2Air = 		pblobIMEU2->fCoolDeviation;

	varV.fValue = pblobIMEU2->fCoolSetPoint;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 464, varV);

	varV.fValue = pblobIMEU2->fCoolDeviation;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 465, varV);

	varV.fValue = pblobIMEU2->fHighTempAlmValue;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 466, varV);

	varV.fValue = pblobIMEU2->fLowTempAlmValue;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 467, varV);

	varV.fValue = pblobIMEU2->fHeatSetPoint;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 468, varV);

	varV.fValue = pblobIMEU2->fHeatDeviation;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 469, varV);

 	varV.fValue = pblobIMEU2->fDehumiStartTemp;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 475, varV);

	varV.fValue = pblobIMEU2->fDehumiStopTemp;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 476, varV);
 	 
	varV.fValue = pblobIMEU2->fDehumiStartHumi;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 477, varV);
 	 
	varV.fValue = pblobIMEU2->fDehumiStopHumi;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 478, varV);
 
	varV.emValue = pblobIMEU2->nAirCondiType_Cfg;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 479, varV);
	
	varV.ulValue = pblobIMEU2->nHumiTempSensorNumb_Cfg;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 480, varV);

	varV.emValue = pblobIMEU2->nCombGasSensorEnable_Cfg;
	Push_VarValue(SETTYPE_RAW_ST, pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, 481, varV);

	return;
}


int ProSingleIMEUSample_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichIMEU)
{
	IMEU2_FRM_INFO_V20* pFrame;
	//Step.1 First Time Read Version 
	if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedReconfig)
	{
		if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
		{
			return RS485_WRITE_ERR;
		}
		pFrame = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_GET_7151_VER_V20];
		if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrame, FALSE, nWhichIMEU))
		{ 
			TRACE("Error: nWhichIMEU=[%d] Pack_IMEUCmd_AndGetReply\n", nWhichIMEU);
			Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
			return  RS485_WRITE_ERR;
		}
		
		if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrame, nWhichIMEU))
		{
			TRACE("Error: Extract Wrong IMEU Frame TP_FRM_GET_7151_VER!!!\n");
			Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
			return  RS485_READ_ERR;
		}
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedReconfig = FALSE;
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedSyncSetParam = TRUE;
	}

	//Step.2 Get IMEU2 DataInfo
	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return RS485_WRITE_ERR;
	}
	pFrame = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_GET_7141_IMEU_V20];
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrame, FALSE, nWhichIMEU))
	{ 
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_WRITE_ERR;
	}
	 
 	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrame, nWhichIMEU))
	{
		TRACE("Error: Extract Wrong IMEU Frame TP_FRM_GET_7141_IMEU_V20 !!!\n");
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_READ_ERR;
	}   
	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);

	//Step.3 Get AC Air Condition DataInfo
	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return RS485_WRITE_ERR;
	}
	pFrame = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_GET_7152_HD_AIRCDT_V20];
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrame, FALSE, nWhichIMEU))
	{ 
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_WRITE_ERR;
	}
	 
 	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrame, nWhichIMEU))
	{
		TRACE("Error: Extract Wrong IMEU Frame TP_FRM_GET_7152_HD_AIRCDT_V20!!!\n");
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_READ_ERR;
	} 
	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);

	//Step.4 Get Temperature And Humi DataInfo
	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return RS485_WRITE_ERR;
	}
	pFrame = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_GET_7143_TEMPHUMI_V20];
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrame, FALSE, nWhichIMEU))
	{ 
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);	
		return  RS485_WRITE_ERR;
	}
	 
 	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrame, nWhichIMEU))
	{
		TRACE("Error: Extract Wrong IMEU Frame TP_FRM_GET_7143_TEMPHUMI!!!\n");
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_READ_ERR;
	}  
	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);

	//Step5. Get GasSensor DataInfo
	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return RS485_WRITE_ERR;
	}
	pFrame = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_GET_7153_GAS_SENSOR_V20];
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrame, FALSE, nWhichIMEU))
	{ 
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);	
		return  RS485_WRITE_ERR;
	}
	 
 	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrame, nWhichIMEU))
	{
		TRACE("Error: Extract Wrong IMEU Frame TP_FRM_GET_7143_TEMPHUMI!!!\n");
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return  RS485_READ_ERR;
	}  
	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	

	//Added Bu John ,After Get All DataInfo, Should Sync Setting Data
	if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedSyncSetParam) 
	{
		//bFirstRun = FALSE;
		Sync2SettingPararm(pDev_485_2_Rough, nWhichIMEU);
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedSyncSetParam = FALSE;
	}
	return RS485_COMM_OK;
}


int IMEU2_Sample_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough)
{
 	UINT nWhichIMEU = 0;
	UINT nErrCode = RS485_COMM_OK;

	for(nWhichIMEU = 0; nWhichIMEU < MAX_IMEU_NUMBER; nWhichIMEU++)
	{
		//TRACE("---------Now IMEU_2 Module[%d] emIMEU2Type=[%d]---------\n", \
		//	1 + nWhichIMEU, pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emIMEU2Type);
		if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emIMEU2Type == IMEU2_TYPE_NotInstalled)
		{
			continue;
		}
		nErrCode = ProSingleIMEUSample_V20(pDev_485_2_Rough, nWhichIMEU);
		if(nErrCode != RS485_COMM_OK)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nInterrupt_Times++;
			if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nInterrupt_Times >= MAX_RS4852_TRY_TIMES)
			{
				pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nInterrupt_Times = 0;
				//need handle the IMEU status
				if(!pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail)
				{
					pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedReconfig = TRUE;
					pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail = TRUE;
					SetCommStatus(pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, FALSE);
					//InitIMEUData(pDev_485_2_Rough, nWhichIMEU);
					TRACE("----IMEU comm Fail----\n");
				}	
			}
		}
		else
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nInterrupt_Times = 0;
			if(pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail)
			{
				pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bIsCommFail = FALSE;
				SetCommStatus(pDev_485_2_Rough->hSampler, 601 + nWhichIMEU, TRUE); //clear comm interr alarm
			}
		}
		
		
		BattCab_Safety_Management(pDev_485_2_Rough, nWhichIMEU);  
		

		IMEU_PushAllData_V20(pDev_485_2_Rough, nWhichIMEU);
	}
 
	return ERR_SAMPLE_OK;
}

int IMEU_SendCtrl_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	TRACE("IMEU_SendCtrl: nParamId = %d, varV.ulValue = %d, varV.fValue = %f.\n", 
        nParamId, pVarVal->ulValue, pVarVal->fValue);

	//change to IMEU default comm setting, but the three dev is some
	int nWhichIMEU 		= nDevId - 601;
	IMEU2_FRM_INFO_V20* pFrameInfo;
	//DWORD 				dwValue;
	VAR_VALUE			varV;
	int 				i = 0;
	BOOL 				bRet = ERR_CONTROL_OK;

	if((nWhichIMEU < 0) || (nWhichIMEU > MAX_IMEU_NUMBER))
	{
		return ERR_CONTROL_FAILED;
	}

	if(emParaType == PARAM_TYPE_SETTING)
	{
		pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
		memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
		pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bNeedSyncSetParam = TRUE;

		if(nParamId == 450) //IMEU2 Config Type
		{
 			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emIMEU2Type = (IMEU_INSTALLED_TYPE)pVarVal->emValue;

			//IMEU2 communicates via RS485, IMEU3 communicates via CAN
			int iSampleID = (IMEU_TYPE_IMEU3 == (IMEU_TYPE)pVarVal->emValue ? 5 : 4);
			SAMPLER* pSampler = GetSamplerById(iSampleID);
			DEVICE* pDev = GetDevce_ById(nDevId);
			pDev->info.pBelonged_Sampler = pSampler;

			return ERR_CONTROL_OK;
		}
		else if(nParamId == 487) //Exhaust Fan Start Combustible Gas Density
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nCombGasAlarmPoint = pVarVal->ulValue;
			return ERR_CONTROL_OK;
		}
		else if(nParamId == 488) //Exhaust Fan Stop Ret Diff Value
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nCombGasStopRetDiff = pVarVal->ulValue;
			return ERR_CONTROL_OK;
		}
		else if(nParamId == 464) //IMEU2 V20 Cool Set Point 
		{
			HexToTwoAsciiData(0x85, pFrameInfo->sendFrame.pbyData);
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fCoolPoint_Sync = pVarVal->fValue;
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 465) //IMEU2 V20 Cool Deviation
		{
			HexToTwoAsciiData(0x86, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 466) //IMEU2 V20 High Temp Alm Value
		{
			HexToTwoAsciiData(0x87, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 467) //IMEU2 V20 Low Temp Alm Value 
		{
			HexToTwoAsciiData(0x88, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 468) //IMEU2 V20 Heat Set Point 
		{
			HexToTwoAsciiData(0x89, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 469) //IMEU2 V20 Heat Deviation
		{
			HexToTwoAsciiData(0x8A, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 475) //IMEU2 V20 Dehumi Start Temp
		{
			HexToTwoAsciiData(0x90, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 476) //IMEU2 V20 Dehumi StopTemp 
		{
			HexToTwoAsciiData(0x91, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 477) //IMEU2 V20 Dehumi Start Humi
		{
			HexToTwoAsciiData(0x92, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 478) //IMEU2 V20 Dehumi Stop Humi
		{
			HexToTwoAsciiData(0x93, pFrameInfo->sendFrame.pbyData);
			varV.lValue = (long)pVarVal->fValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 479) //IMEU2 V20 Air Condition Type
		{
	 		HexToTwoAsciiData(0x94, pFrameInfo->sendFrame.pbyData);
			varV.emValue = pVarVal->emValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 480) //Humi Temp Sensor Number
		{
			HexToTwoAsciiData(0x95, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 481) //IMEU2 V20 Comb Gas Sensor Cfg
		{
			HexToTwoAsciiData(0x96, pFrameInfo->sendFrame.pbyData);
			varV.emValue = pVarVal->emValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 483) //IMEU2 V20 Comb Gas Sensor Maunfactuer
		{
			HexToTwoAsciiData(0x97, pFrameInfo->sendFrame.pbyData);
			varV.emValue = pVarVal->emValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 482) //Comb Gas Sensor Number 
		{
			HexToTwoAsciiData(0x98, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 484) //IMEU2 Hydrogen Discharge Interval Without Host 
		{
			HexToTwoAsciiData(0x99, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 485) //IMEU2 Hydrogen Discharge Time Without Host 
		{
			HexToTwoAsciiData(0x9A, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 489) //IMEU2 Ctrl Shutter Action Time
		{
			HexToTwoAsciiData(0x9B, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 490) //Gas PPM Alarm Threshold
		{
			HexToTwoAsciiData(0xA0, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 494) //PT1000 Number Config
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nPT1000Num = pVarVal->lValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 495) //Dehumidification Ret Diff(TongFei)
		{
			HexToTwoAsciiData(0x9C, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 496) //High Humidity Point(TongFei)
		{
			HexToTwoAsciiData(0x9D, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 497) //High Humidity Point(TongFei)
		{
			HexToTwoAsciiData(0x9E, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 459) //Fire Protect Type
		{
			HexToTwoAsciiData(0x9F, pFrameInfo->sendFrame.pbyData);
			varV.ulValue = pVarVal->ulValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 601)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fAmbit_TcoolonSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 602)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fBatt_Tmax_coolonSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 603)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fAmbit_TcooloffSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 604)
		{	
 			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fBatt_Tmax_cooloffSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 605)
		{	
 			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fAirEnTran_TlmtSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 606)
		{	
 			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fAmbit_TheatonSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 607)
		{	
 			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fHumidity_StartSett = pVarVal->fValue;
			return  ERR_CONTROL_OK;
		}
		else if(nParamId == 608)
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].fECOModeTempOffset = pVarVal->fValue;
			return ERR_CONTROL_OK;
		}
		else
		{
			TRACE("Error: Not Found IMEU Setting Cmd\n");
			return  ERR_CONTROL_FAILED;
		}
		//TRACE("dwValue=[%d]\n", dwValue);
	}
	else
	{
		TRACE("Warning: The Following is Ctrl Param\n");	
		if(nParamId == 450) //DO2
		{
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag = 0;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.Ctrl.DO2 = pVarVal->emValue;
			varV.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 451) //DO3
		{
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag = 0;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.Ctrl.DO3 = pVarVal->emValue;
			varV.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag = 0;;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 452) //DO4
		{
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag = 0;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.Ctrl.DO4 = pVarVal->emValue;
			varV.emValue = pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.usFlag;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 453) //DO5
		{
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].doValue.Ctrl.DO5 = pVarVal->emValue;
			varV.emValue = pVarVal->emValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 454) //Battery Cabinet Shutter Ctrl(Discharge Hydrogen)
		{
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
		
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterCtrlCmd = pVarVal->emValue;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bMonitorShutter = TRUE;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].tmShutterFanCtrl = _NOW_;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterCtrlTimes = MAX_SHUTTER_CTRL_TRY_TIMES;
			TRACE("IMEU2: 222 First Send Ctrl Shutter Cmd[%d]\n", pVarVal->emValue);
			
			HexToTwoAsciiData(0x74, pFrameInfo->sendFrame.pbyData);
			varV.emValue = pVarVal->emValue;
			for(i = 0; i < 4; i++)
			{
				HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
			}
			TRACE("nParamId =%d varV.lValue=[%d]\n", nParamId, varV.lValue);
		}
		else if(nParamId == 455) //IMEU2 V20 On/Off Statu John 20220411,Manual Mode Not Wait.
		{
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_ONOFF, pVarVal->emValue, FALSE);
			return bRet;
		}
		else if(nParamId == 456) //IMEU2 V20 Start Refrigeration Mode
		{
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_COOL_MODE, pVarVal->emValue, FALSE);
			return bRet;
		}
		else if(nParamId == 457) //IMEU2 V20 Start Supply Air Mode
		{
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_SUPPLY_AIR, pVarVal->emValue, FALSE);
			return bRet;
		}
		else if(nParamId == 458) //IMEU2 V20 Start Standby Mode
		{	
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_STANDBY, pVarVal->emValue, FALSE);
			return bRet;
		}
		else if(nParamId == 459) //IMEU2 V20 Start Heat Mode 
		{	
			bRet = SendAirCondCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, CMD_CTRL_AIRCOND_HEAT_MODE, pVarVal->emValue, FALSE);
			return bRet;
		}
		else if(nParamId == 460) //Manual Control
		{
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].emManageCtrlMode = (MANAGE_CTRL_MODE)pVarVal->emValue;
			TRACE("nParamId =%d pVarVal->emValue=[%d]\n", nParamId, pVarVal->emValue);
			return ERR_CONTROL_OK;
		}
		else
		{
			TRACE("Error: Ctrl nParamId=[%d] Not Defined.\n", nParamId);
			return  ERR_CONTROL_FAILED;
		}
	}
	
	TRACE("Before:nWhichIMEU=[%d] byDestAddr[0]=%02X] byDestAddr[1]=[%02X] \n",\
		nWhichIMEU, pFrameInfo->sendFrame.byDestAddr[0],pFrameInfo->sendFrame.byDestAddr[1]);

	HexToTwoAsciiData((YDN_FRAME_ADR + nWhichIMEU), pFrameInfo->sendFrame.byDestAddr);  

	TRACE("After:nWhichIMEU=[%d] byDestAddr[0]=%02X] byDestAddr[1]=[%02X] \n",\
		nWhichIMEU, pFrameInfo->sendFrame.byDestAddr[0],pFrameInfo->sendFrame.byDestAddr[1]);

	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_FAILED;
	}
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrameInfo, TRUE, nWhichIMEU))
	{
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_CONTROL_FAILED;
	}

	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrameInfo, nWhichIMEU))
	{
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_CONTROL_FAILED;
	}

	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	return ERR_CONTROL_OK;
}



/*********************************************************************************
*  
*  FUNCTION NAME : Send Ctrl/Set Cmd To IMEU2
*  CREATOR       : John
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERRCODE_SAMPLER SendCtrlSetCmd2IMEU(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU, IN IMEU_SEND_CMD_TYPE cmdType, IN int nCmdVal)
{
	int                 i;
	VAR_VALUE			varV;
	IMEU2_FRM_INFO_V20* pFrameInfo;
	IMEU2_DO_VALUE      doTemp;
	
	doTemp.usFlag = 0;

	switch(cmdType)
	{
		case CMD_CTRL_SHUTTER:
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterCtrlCmd = nCmdVal;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].bMonitorShutter = TRUE;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].tmShutterFanCtrl = _NOW_;
			pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].nShutterCtrlTimes = MAX_SHUTTER_CTRL_TRY_TIMES;
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x74, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACEX("IMEU2: nWhichIMEU=[%d] 111 First Send Ctrl Shutter Cmd[%d]\n", nWhichIMEU, nCmdVal);
			break;
			
		case CMD_CTRL_SUB_SHUTTER:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x74, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACEX("IMEU2: nWhichIMEU=[%d] Send Sub Ctrl Shutter Cmd[%d]\n", nWhichIMEU, nCmdVal);
			break;

		case CMD_CTRL_AIRCOND_STANDBY:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8E, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl Air-Cond Standby Cmd[%d]\n", nWhichIMEU, nCmdVal);
			break;

		case CMD_CTRL_AIRCOND_SUPPLY_AIR:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8D, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl Supply Air Cmd[%d]\n", nWhichIMEU, nCmdVal);
			break;

		case CMD_SET_BATT_TYPE:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x9C, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Batt Type[%d]\n", nWhichIMEU, nCmdVal);
			break;

		case CMD_SET_AIRCOND_CRYOGEN_POINT:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x85, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Cryogen Point[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_SET_AIRCOND_COOLRET_POINT:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x86, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Cryogen Return Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_HEATING_POINT:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x89, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Heating Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_HEAT_DEVIATION:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8A, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			printf("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Heat Deviation Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_DEHUMI_STARTTEMP:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x90, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Dehuni Start Temp Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_DEHUMI_STOPTEMP:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x91, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Dehumi Stop Temp Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_DEHUMI_STARTHUMIDITY:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x92, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Start Humidity Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_SET_AIRCOND_DEHUMI_STOPHUMIDITY:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x93, pFrameInfo->sendFrame.pbyData);
			varV.lValue = nCmdVal;
			TRACEX("IMEU2:  nWhichIMEU=[%d] Send Set Air-Cond Stop Humidity Point[%d]\n", nWhichIMEU, nCmdVal );
			break;
		case CMD_CTRL_DO2:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			doTemp.Ctrl.DO2 = nCmdVal;
			varV.emValue = doTemp.usFlag;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl DO2 Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_DO3:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			doTemp.Ctrl.DO3 = nCmdVal;
			varV.emValue = doTemp.usFlag;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl DO3 Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_DO4:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			doTemp.Ctrl.DO4 = nCmdVal;
			varV.emValue = doTemp.usFlag;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl DO4 Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_DO5:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7145_CTRL_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x72, pFrameInfo->sendFrame.pbyData);
			doTemp.Ctrl.DO5 = nCmdVal;
			varV.emValue = doTemp.usFlag;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl DO5 Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_AIRCOND_ONOFF:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8B, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl OnOff Air Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_AIRCOND_COOL_MODE:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8C, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl Cool Air Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		case CMD_CTRL_AIRCOND_HEAT_MODE:
			pFrameInfo = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].frmIMEU2_V20[TP_FRM_SET_7144_SETPARAM_V20];
			memset(pFrameInfo->sendFrame.pbyData, 0x00, pFrameInfo->nDataLen);
			HexToTwoAsciiData(0x8F, pFrameInfo->sendFrame.pbyData);
			varV.emValue = nCmdVal;
			TRACE("IMEU2:  nWhichIMEU=[%d] Send Ctrl Heat Air Cmd[%d]\n", nWhichIMEU, nCmdVal );
			break;

		default:
			break;
			
	}

	TRACE("Before:nWhichIMEU=[%d] byDestAddr[0]=%02X] byDestAddr[1]=[%02X] \n",\
		nWhichIMEU, pFrameInfo->sendFrame.byDestAddr[0],pFrameInfo->sendFrame.byDestAddr[1]);

	HexToTwoAsciiData((YDN_FRAME_ADR + nWhichIMEU), pFrameInfo->sendFrame.byDestAddr);  

	TRACE("After:nWhichIMEU=[%d] byDestAddr[0]=%02X] byDestAddr[1]=[%02X] \n",\
		nWhichIMEU, pFrameInfo->sendFrame.byDestAddr[0],pFrameInfo->sendFrame.byDestAddr[1]);

	for(i = 0; i < 4; i++)
	{
		HexToTwoAsciiData(varV.abyValue[4-i-1], pFrameInfo->sendFrame.pbyData + 2 + i*2);
	}

	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_FAILED;
	}
	if (RS485_COMM_OK != Pack_IMEUCmd_AndGetReply_V20(pDev_485_2_Rough, pFrameInfo, TRUE, nWhichIMEU))
	{
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_CONTROL_FAILED;
	}

	if(!ExtractIMEUFrame_V20(pDev_485_2_Rough, pFrameInfo, nWhichIMEU))
	{
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_CONTROL_FAILED;
	}

	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	return ERR_CONTROL_OK;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Send AirCond Cmd IMEU2
*  CREATOR       :  
*  CREATED DATE  : 2021-11-9 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERRCODE_SAMPLER SendAirCondCmd2IMEU(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU, IN IMEU_SEND_CMD_TYPE emCurrAirCondCmd, IN int nCurrCmdVal, BOOL bNeedWait)
{
 	IMEU2_ROUGH_DATA *pIMEU2_RoughData = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU];
	A_BLOB_BATTCAB_IMEU2_V20	*pBlobCabV2 = &pDev_485_2_Rough->imeu2_RoughData[nWhichIMEU].blobIMEU2_V20;

 	time_t tmNow = _NOW_;
	ERRCODE_SAMPLER bRet = ERR_CONTROL_OK;
	
 	if((ABS(tmNow - pIMEU2_RoughData->tmLastCtrlAirCond) <= MAX_AIR_COND_CMD_INTERVAL) && (bNeedWait == TRUE))
 	{
 		//Every AirCond Cmd Need 3 Minutes Apart.Manafactuer requirement.
 		return ERR_CONTROL_FAILED;	
 	}
	
 	bRet = SendCtrlSetCmd2IMEU(pDev_485_2_Rough, nWhichIMEU, emCurrAirCondCmd, nCurrCmdVal);

	//if(ABS(tmNow - pIMEU2_RoughData->tmLastCtrlAirCond) >= MAX_AIR_COND_CMD_INTERVAL)
    {
 		LogOut(MAIN_MODULE_SAMPLER, "IMEU2", LOG_TYPE_INFO, \
			"Idx[%d]Val[%d]Cmd[%d]ECO[%d]Tair[%.0f]Tamb[0]|[1][%.0f][%.0f]BTmax|min[%.0f][%.0f]HumAir|Amb[%.0f][%.0f]THeat[%.0f]HumOn[%.0f]\n", \
			nWhichIMEU, nCurrCmdVal, emCurrAirCondCmd, pIMEU2_RoughData->emECORunStatu, pBlobCabV2->fRealTimeCabTemp, \
			pBlobCabV2->fTemperature[0], pBlobCabV2->fTemperature[1], pIMEU2_RoughData->fBattTmax_Samp,pIMEU2_RoughData->fBattTmin_Samp,\
			pBlobCabV2->fRealTimeCabHumi,pBlobCabV2->fHumidity[0], pBlobCabV2->fHeatSetPoint,pBlobCabV2->fDehumiStartHumi);

		LogOut(MAIN_MODULE_SAMPLER, "IMEU2", LOG_TYPE_INFO, \
			"Idx[%d]ST:TAmbOn[%.0f]TBatmaxOn[%.0f]TambOff[%.0f]TBatmaxOff[%.0f]TlmtOff[%.0f]TColSet[%.0f]Tcolret[%.0f]\n", \
			nWhichIMEU, pIMEU2_RoughData->fAmbit_TcoolonSett,\
			pIMEU2_RoughData->fBatt_Tmax_coolonSett,\
			pIMEU2_RoughData->fAmbit_TcooloffSett,\
			pIMEU2_RoughData->fBatt_Tmax_cooloffSett,\
			pIMEU2_RoughData->fAirEnTran_TlmtSett,\
			pBlobCabV2->fCoolSetPoint,\
			pBlobCabV2->fCoolDeviation);
	}

	if(bNeedWait == TRUE)
	{
		pIMEU2_RoughData->tmLastCtrlAirCond = tmNow;
	}
	return bRet;
 }