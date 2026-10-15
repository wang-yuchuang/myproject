//#define _TRACE_PRINT_
#include "uart_server_main.h"

static BYTE* ChgASCToWhHex(char*array, BYTE *hexa, int num)
{
	int  i;
	BYTE byTemp;

	BYTE* pOrig = hexa;

	for(i=0; i< (num*2); i++) 
	{
		if( *(array + i) > '9' )
			byTemp = *(array + i) - 'A' + 10;
		else
			byTemp = *(array + i) - '0';
		if (i % 2)
			hexa[i/2] |= byTemp;
		else
			hexa[i/2] = byTemp << 4;
	}
	for(i = 0; i < num; i++)
	{
		hexa[i] += 0x33;
	}
	return pOrig;
}

#define  INIT_AWH_FRAME(pFrame, _type, _ctrlCode, _sendDataLen, _headerLen, _pHeader, _DataLen, _nPointNum) \
		do{ \
			(pFrame)->frameType = (_type); \
			(pFrame)->sendFrame.byCtrlCode = (_ctrlCode); \
			(pFrame)->sendFrame.byDataLen = (_sendDataLen); \
			(pFrame)->nFrameLen = (_sendDataLen + 12); \
			(pFrame)->sendFrame.pbyData = NEW(BYTE, _sendDataLen); \
			ChgASCToWhHex((_pHeader), (pFrame)->sendFrame.pbyData, (_headerLen)); \
			(pFrame)->nDataLen = (_DataLen); \
			(pFrame)->nPointNum = (_nPointNum); \
			(pFrame)->nExpectAnswerLen = (_DataLen) + 16; \
			(pFrame)->sendFrame.byFrameSOI = 0x68; \
			(pFrame)->sendFrame.byFrameSOI_2 = 0x68; \
			(pFrame)->sendFrame.byEOI = 0x16; \
			memset((pFrame)->sendFrame.byAddrA0_A5, 0xAA, 6); \
			(pFrame)++; \
		}while(0)


void UartServerGetMeterSN(ST_UART_SERVER_ROUGHDATA *pWhim_Rough, IN int nWhichWh, char* szSrc)
{
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)((SAMPLER*)pWhim_Rough->hSampler)->hDPR;
	char szSN[13];
	char* psz = szSrc;
	if(szSrc == NULL)
	{
		VAR_VALUE varV;
		varV.blobValue = szSN;
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,401 + nWhichWh, 403, &varV, 13); 
		psz = szSN;
	}

	BYTE byConvt[6];
	sscanf(psz, "%02X%02X%02X%02X%02X%02X",
		&byConvt[0], &byConvt[1], &byConvt[2],
		&byConvt[3], &byConvt[4], &byConvt[5]);
	int n;
	for(n = 0 ; n < 6; n++)
	{
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[n] = byConvt[5 - n];
	}

	TRACEX(" now update meterSN:[%s], [%02X%02X%02X%02X%02X%02X]\n ", psz,
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[5],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[4],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[3],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[2],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[1],
		pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6[0]);

}

int whMeter_InitRoughData(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough)
{
	//Now init the frames
	TRACEI("whMeter_InitRoughData\n");
	WH_FRAME_INFO *pFrm = pSTServer_Rough->frm_whFrames;
	//The data identifier has been reversed  04 00 04 01
	//INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_SET_ADDR,			0x15,	0x06,	6,	"01AAAAAAAAAA",	0);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_ADDR,			0x11,	0x04,	4,	"01040004",		5,	0);  
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALWH,		0x11,	0x04,	4,	"00000100",		4,	2);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_RVSTATALWH,	0x11,	0x04,	4,	"00000200",		4,	2);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALWH_AC,	0x11,	0x04,	4,	"00000100",		4,	2);
	
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_A,	0x11,	0x04,	4,	"00FF0102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_B,	0x11,	0x04,	4,	"00020102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_C,	0x11,	0x04,	4,	"00030102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_A,	0x11,	0x04,	4,	"00FF0202",		3,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_B,	0x11,	0x04,	4,	"00020202",		3,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_C,	0x11,	0x04,	4,	"00030202",		3,	3);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALKVAR_AC,	0x11,	0x04,	4,	"00000300",		4,	2);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALKVAR2_AC,0x11,	0x04,	4,	"00000400",		4,	2);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_ACTPOWER_AC,	0x11,	0x04,	4,	"00FF0302",		3,	4); 
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_REACTPOWER_AC,0x11,	0x04,	4,	"00FF0402",		3,	4);
  	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_POWERFACTOR,	0x11,	0x04,	4,	"00FF0602",		2,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_FREQ,			0x11,	0x04,	4,	"02008002",		2,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_PMAXDEMAND,	0x11,	0x04,	4,	"00000101",		3,	4);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH,	0x11,	0x04,	4,	"01FF0100",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH,	0x11,	0x04,	4,	"01FF0200",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH,	0x11,	0x04,	4,	"02FF0100",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH,	0x11,	0x04,	4,	"02FF0200",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_TOTALWH,		0x11,	0x04,	4,	"00FF0100",		4,	2);//modiyf by lpl 240725 for getting current SPFW kwh
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_RVSTATALWH,		0x11,	0x04,	4,	"00FF0200",		4,	2);//modiyf by lpl 240725 for getting current SPFW kwh
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURRENTPOSDEMAND,		0x11,	0x04,	4,	"04008002",		3,	4);//当前有功需量
 
	int n;
	int kkk;
	SAMPLER* pSampler = (SAMPLER*)pSTServer_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	for( kkk = 0; kkk < MAX_WH_NUM; kkk++)
	{
		for(n = 0; n < _MAX_WH_ROUGH; n++)
		{
			pSTServer_Rough->meterData[kkk].varWHMeterRough[n].lValue = SAMP_INVALID_VALUE;
		}

		pSTServer_Rough->meterData[kkk].nSampleTimes = 0;

		//Meter SN
		UartServerGetMeterSN(pSTServer_Rough, kkk, NULL);   //Get 645 COMM Addr

		VAR_VALUE varV;
		//Meter install type
		pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR,401 + kkk, 401, &varV, 4);
		pSTServer_Rough->meterData[kkk].emMeterType = (METER_TYPE)varV.emValue;

		//AC power factor
		pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR,401 + kkk, 402, &varV, 4);
		pSTServer_Rough->meterData[kkk].fACMeter_PowerFactor = varV.fValue;

		//AC meter protocol type
		varV.emValue = ACMETER_PROTOCOL_GBDLT645; //default is this setting
		pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR, 401 + kkk, 404, &varV, 4);
		pSTServer_Rough->meterData[kkk].emACMeter_ProtocolType = varV.emValue;
	}

	return 0;
}

static void Make_CS(WH_FRAME_INFO* pFrame)
{
	ULONG ulSum = 0;
	BYTE* pByArr = (BYTE*)&pFrame->sendFrame; //Note: this needs #pragma pack(1), or else will be problems!!!
	BYTE byTobeCalcBytes[4096];
	memcpy(byTobeCalcBytes, pByArr, 10);
	int nDataLen = pFrame->nFrameLen - 12;
	memcpy(byTobeCalcBytes + 10, pFrame->sendFrame.pbyData, nDataLen);

	int nCalcLen = pFrame->nFrameLen - 2; 
	int n;
	for(n = 0; n < nCalcLen; n++)
	{
		ulSum+= byTobeCalcBytes[n];
	}
	//return (BYTE)ulSum;
	pFrame->sendFrame.byChkSum = (BYTE)(ulSum & 0xFF);
}

static BYTE* MakewhMeterSendBuff(ST_UART_SERVER_ROUGHDATA* pWhim_Rough, WH_FRAME_INFO *pFrameInfo)
{
	BYTE* pbyBuff = pWhim_Rough->aby645SendBuf;
	BYTE* pbyBuffOrig = pbyBuff;
	memcpy(pbyBuff, &pFrameInfo->sendFrame, 10);
	pbyBuff += 10;
	int nDataLen = pFrameInfo->nFrameLen - 12;
	//if(nDataLen > 0)
	{
		memcpy(pbyBuff, pFrameInfo->sendFrame.pbyData, nDataLen);//33 33 35 33 
	}
	pbyBuff += nDataLen;
	*pbyBuff = pFrameInfo->sendFrame.byChkSum;
	pbyBuff += 1;
	*pbyBuff = pFrameInfo->sendFrame.byEOI;

	// int i;
	// printf("UART---645 send frame :  ");
	// for (i=0;i<17;i++)
	// {
	// 	printf("%02x ",pbyBuffOrig[i]);
	// }
	// printf("\n");
	
	return pbyBuffOrig;
}

static int Pack_whMeterCmd_AndGetReply(IN ST_UART_SERVER_ROUGHDATA* pWhim_Rough, 
										IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,
										IN WH_FRAME_INFO* pwhFrame)
{
	int nWriteBytes;
	int nReadBytes;

	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	
	//Clear buffer   
	// pComm->Control(pClientInfo->hClient,
	// 	COMM_PURGE_RXCLEAR, NULL, 0);

	//TRACEX("Now begin to write whMeter cmd!\n");
	//First Send wake up frame
	VAR_VALUE varWakeUp;
	varWakeUp.ulValue = WH_WAKEUP_FRAME;
	nWriteBytes = pComm->Write(pClientInfo->hClient, varWakeUp.abyValue, 4);
	TRACE("whMeter cmd Write wakeup OK [%d] !\n", nWriteBytes);
	Sleep(20);

	//Now beging send frame
	nWriteBytes = pComm->Write(pClientInfo->hClient, 
		MakewhMeterSendBuff(pWhim_Rough, pwhFrame), pwhFrame->nFrameLen);

	TRACE("whMeter cmd Write cccc OK [%d], expect len = %d!\n", nWriteBytes, pwhFrame->nFrameLen);
	if(nWriteBytes < pwhFrame->nFrameLen)
	{
		TRACEE("Err: nWriteBytes=[%d] nFrameLen=[%d]\n", nWriteBytes,pwhFrame->nFrameLen);
		//Mutex_Unlock(pWhim_Rough->hSamplerLock);
		//TRACE("Err: nWriteBytes=[%d] nFrameLen=[%d]\n", nWriteBytes,pwhFrame->nFrameLen);
		return TCP_WRITE_ERR;
	}

	Sleep_Ex(600);

	memset(pWhim_Rough->aby645RcvBuf, 0, TCP_BUFF_LEN);//clear buff 

	//TRACEX("Now begin to read whMeter cmd reply!\n");

	nReadBytes = pComm->Read(pClientInfo->hClient,pWhim_Rough->aby645RcvBuf, TCP_BUFF_LEN); 
	// int i;
	// printf("UART---645 recv frame :  ");
	// for (i=0;i<30;i++)
	// {
	// 	printf("%02x ",pWhim_Rough->aby645RcvBuf[i]);
	// }
	// printf("\n");

	TRACEX("Read whMeter cmd OK! ExpectLen = %d,  readLen = %d, read[%s]!\n", pwhFrame->nExpectAnswerLen + 4, nReadBytes,
		pWhim_Rough->aby645RcvBuf);

	if(nReadBytes <= 0)
	{
		//Mutex_Unlock(pWhim_Rough->hSamplerLock);
		return TCP_READ_ERR;
	}

	pWhim_Rough->nReadBuffBytes = nReadBytes;
	TRACEW("pWhim_Rough->nReadBuffBytes = %d ",pWhim_Rough->nReadBuffBytes);

	//Mutex_Unlock(pWhim_Rough->hSamplerLock);
	return TCP_COMM_OK;
}

static float ConvertBCD_2WhVal(BYTE* pBCD, WH_FRAME_INFO *pFrameInfo)
{
	float fOut = 0.0;

	//--- kWh had no sign ---/
	if((pFrameInfo->frameType == WH_FRAME_TYPE_GET_TOTALWH)      //xx xx xx.xx
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_RVSTATALWH)   //xx xx xx.xx
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)  //XX XX XX.XX
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)   //XX XX XX.XX
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH)) 
	{
		fOut = (float)((pBCD[3] >> 4)      *100000);
		//fOut = (float)(((pBCD[3] >> 4) & 0x07)      *100000);
		fOut += (float)((pBCD[3] & 0x0F) *10000);
		fOut += (float)((pBCD[2] >> 4) *1000);
		fOut += (float)((pBCD[2] & 0x0F) *100);
		fOut += (float)((pBCD[1] >> 4) *10);
		fOut += (float)((pBCD[1] & 0x0F) *1);
		fOut += (float)((pBCD[0] >> 4) *0.1);
		fOut += (float)((pBCD[0] & 0x0F) *0.01);
	}
	else
	{
		int nLen = pFrameInfo->nDataLen;
		int nPoint = pFrameInfo->nPointNum;
		int n;
		float fSCal = 1.0;
		for(n = 0; n < nPoint; n++)
		{
			fSCal /= 10.0;
		}

		if(nLen < 1)
		{
			return 0.0;
		}

		BOOL bSigned = FALSE;
		
		if(((pBCD[nLen - 1] >> 7) & 0x01) == 0x01)
		{
			bSigned = TRUE;
		}

		int k = 0;
		float fScale_Ex = 1.0;
		for(n = 0; n < nLen; n++)
		{
			if(n == (nLen - 1))
			{
				fOut += (float)(((pBCD[n] >> 4) & 0x07) *fSCal * fScale_Ex * 10.0);
			}
			else
			{
				fOut += (float)(((pBCD[n] >> 4)) *fSCal * fScale_Ex * 10.0);
			}

			fOut += (float)((pBCD[n] & 0x0F) *fSCal * fScale_Ex);

			fScale_Ex *= 100.0;
		}

		fOut *= (bSigned) ? -1 : 1;
	}
	return fOut;
}

#define  ERR_WRONG_FRAME	0xFF
static int ExtractWhFrame(ST_UART_SERVER_ROUGHDATA* pWhim_Rough, WH_FRAME_INFO *pFrameInfo, int nWhichWh)
{
	BYTE* pBuff = pWhim_Rough->aby645RcvBuf;
	int n = 0;
	int m = 0;
	int nSub_Idx = 0;
	int nStart = -1;
	for(n = 0; n < pWhim_Rough->nReadBuffBytes; n++)
	{
		//Jimmy note 20191207: do not need judge FEFEFEFE, another manufactor does NOT replay this
		//if((pBuff[n] == 0xFE) && (pBuff[n + 1] == 0x68))
		if(pBuff[n] == 0x68)
		{
			//m = n+ 8;
			m = n + 7;   //Control Num
			if((m < pWhim_Rough->nReadBuffBytes) &&
				(pBuff[m] == 0x68))
			{
				//nStart = n + 1;
				nStart = n;  //The position of the first 68
				break;
			}
		}
	}
	if(nStart == -1)
	{
		return ERR_WRONG_FRAME;
	}
	

	BYTE nErrCode = ERR_SAMPLE_OK;
	WH_A_FRAME *pRecFrame = (WH_A_FRAME*)(pBuff + nStart);  //Point to First 68H

	int nPasedFrmLen = pWhim_Rough->nReadBuffBytes - nStart;

	if(nPasedFrmLen < pFrameInfo->nExpectAnswerLen)   //the Bytes Num of 68h-16h
	{
		TRACEE("------The frmeExpectLe = %d, but RealLen = %d, so discard this frme!-------------\n", pFrameInfo->nExpectAnswerLen, nPasedFrmLen);
		return ERR_WRONG_FRAME;
	}

	BYTE* pbyValStart = pBuff + nStart + 10;   //Data Part

	int i;
	for (i=0;i<4;i++)
	{
		if (pFrameInfo->sendFrame.pbyData[i] != pbyValStart[i])
		{
			break;
		}
	}
	if (i !=4 )
	{
		TRACEE("UART------The frame Data identification bit NO match send [%02x] [%02x] [%02x] [%02x]  recv [%02x] [%02x] [%02x] [%02x]",
			pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
			pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);

		// LogOut(MAIN_MODULE_REPORTER, "UART_SERVER_Samp", LOG_TYPE_ERROR, 
		// "UART------The frame Data identification bit NO match send [%02x] [%02x] [%02x] [%02x] recv [%02x] [%02x] [%02x] [%02x]", 
		// pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
		// pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);
		return ERR_WRONG_FRAME;
	}
	else 
	{
		TRACEI("UART------The frame Data identification bit match send [%02x] [%02x] [%02x] [%02x]  recv [%02x] [%02x] [%02x] [%02x]",
			pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
			pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);
	}

	switch(pFrameInfo->sendFrame.byCtrlCode)
	{
	/*
	1 0 0 10001 
	1:The slave station sends out a response frame. 
	0:The slave station correctly responds 
	0:without subsequent data frames 
	10001 :reads data
	*/
	case 0x11: //read data     
		if((pRecFrame->byCtrlCode != 0x91) ||   
			(pRecFrame->byDataLen == 0x01))
		{
			//TRACEX("whEEEEEEEEEEEEEEEEE= %d, Err+1 = %d\n",*(BYTE*)pbyValStart, *(BYTE*)(pbyValStart + 1));
			nErrCode = (*(BYTE*)pbyValStart) - 0x33;
		}
		else
		{
			//---Added By John 2022-11-30,added For Remote kWh Reading---//
			if((pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)//XX XX XX.XX
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH))
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < MAX_MULTI_FRAMEDATA; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+4*nSub_Idx, pFrameInfo);
					//TRACEX("----The fGet_MultiData[%d]=[%.2f]----\n", nSub_Idx, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_VOLT_PHASE_A) //XXX.X
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+2*nSub_Idx, pFrameInfo);
					//TRACEX("----VoltA The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_CURR_PHASE_A)  //xxx.xxx
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}

			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_ACTPOWER_AC)   //xx.xxxx
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_REACTPOWER_AC)  //xx.xxxx
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_POWERFACTOR)  //x.xxx
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+2*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else
			{
				for(n = 0; n < 4; n++)
				{
					pbyValStart[n + 4] -= 0x33;
					//printf("%02X|", pbyValStart[n+4]);
				}
				pFrameInfo->fGetData = ConvertBCD_2WhVal(pbyValStart + 4, pFrameInfo);
				//TRACE("\n----The fGetData is [%.2f]----\n", pFrameInfo->fGetData);
			}
		}
		break;
	case 0x15: //set comm addr, will not relply if error
		//if((pRecFrame->byAddrA0_A5[0] != 0x01) ||
		//	(pRecFrame->byCtrlCode != 0x95))
		//{
		//	nErrCode = *pbyValStart;
		//}
		break;
	}

	return nErrCode;
}

static int whMeter_SigSamp(ST_UART_SERVER_ROUGHDATA* pWhim_Rough,
							IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,
							int nWhichWh, WH_FARAME_TYPE emCmdType, WH_ROUGH_SIG emSigType)
{
	int nSub_Idx = 0;
	HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}
	// COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;

	// pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
	// 	strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	WH_FRAME_INFO* pFrame = &pWhim_Rough->frm_whFrames[emCmdType];

	memcpy(pFrame->sendFrame.byAddrA0_A5, pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6, 6);
	
	Make_CS(pFrame);
	if (TCP_COMM_OK != Pack_whMeterCmd_AndGetReply(pWhim_Rough,pClientInfo, pFrame))
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_TIMEOUT;
	}
	int nErrCode = ExtractWhFrame(pWhim_Rough, pFrame, nWhichWh);   //Parse Response Data 
	if(ERR_SAMPLE_OK != nErrCode)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].lValue = nErrCode;
		//pWhim_Rough->varWHMeterRough[nWhichWh][emSigType].lValue = -9999;
		TRACEX("whMeter[%d] Has Fail for type[%d], ErrCode = %d.\n", nWhichWh, emCmdType, nErrCode);
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	if((pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH))
	{
		for(nSub_Idx = 0; nSub_Idx < MAX_MULTI_FRAMEDATA; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
			// if (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
			// {
			// 	TRACEW("UART---CurrentPosDataBlock[%d] = [%f] nWhichWh = [%d] ",nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue,nWhichWh);
			// }
			// else if (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH)
			// {
			// 	TRACEW("UART---CurrentNegDataBlock[%d] = [%f] nWhichWh = [%d] ",nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue,nWhichWh);
			// }
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_VOLT_A)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_CURR_A)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RTACTPower)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
		}

	}
	else if(emSigType == WHMETER_ROUGH_RTReActPower)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_PowerFactor)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
		}
	}
	else   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue = pFrame->fGetData;
		if((pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_UartServer) && 
			((emSigType == WHMETER_ROUGH_RTkWh_AC) || 
			(emSigType == WHMETER_ROUGH_RTkvar) ||
			(emSigType == WHMETER_ROUGH_RTkvar2) ||  
			//(emSigType == WHMETER_ROUGH_RT_CURR_A) || 
			//(emSigType == WHMETER_ROUGH_RT_CURR_B) || 
			//(emSigType == WHMETER_ROUGH_RT_CURR_C) ||
			//(emSigType == WHMETER_ROUGH_RTACTPower) ||
			//(emSigType == WHMETER_ROUGH_RTReActPower) ||
			(emSigType == WHMETER_ROUGH_PMAXDEMAND) ||
			(emSigType == WHMETER_ROUGH_CURRENTPOSDEMAND) ||
			(emSigType == WHMETER_ROUGH_RT_RVSkWh_AC)))
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
			// if (emSigType == WHMETER_ROUGH_RTkWh_AC)
			// {
			// 	TRACEI("UART---CurrentPosTotalKwh = [%f] nWhichWh = [%d] ",pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue,nWhichWh);
			// }
			// else if (emSigType == WHMETER_ROUGH_RT_RVSkWh_AC)
			// {
			// 	TRACEI("UART---CurrentNegTotalKwh = [%f] nWhichWh = [%d] ",pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue,nWhichWh);
			// }
		}
		else if(emSigType == WHMETER_ROUGH_RT_Freq)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= 10; //AC Freq Need * 10
		}
	}

	Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;
}


static void Judge_IfNeedGetRelevantValue(IN ST_UART_SERVER_ROUGHDATA* pWhim_Rough,
										IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo, 
										IN int nWhichWh)
{	
	int nSampRTN = ERR_SAMPLE_OK;
	// float fPcab = (fabs)(pWhim_Rough->meterData[0].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue);
	// //Only Cab Meter And System On Idle Status
	// if((nWhichWh != 0) && (fPcab > 10.0)) 
	// {
	// 	return;
	// }
	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_PMAXDEMAND, WHMETER_ROUGH_PMAXDEMAND);

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_TOTALKVAR_AC, WHMETER_ROUGH_RTkvar);//KvarH

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_TOTALKVAR2_AC, WHMETER_ROUGH_RTkvar2);//KvarH

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_FREQ, WHMETER_ROUGH_RT_Freq);
	//added by lpl 240823 for zhongshu get current pos demand
	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_CURRENTPOSDEMAND, WHMETER_ROUGH_CURRENTPOSDEMAND);
 	
	//TRACE("Relevant nWhichWh=[%d]  RTACTPower=[%f]\n",  nWhichWh, fPcab);

	return;
}

//One Hour Period Get Account Kwh Data
static void Judge_IfNeedGetAccountKwh(IN ST_UART_SERVER_ROUGHDATA* pWhim_Rough,
									 IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,
									 IN int nWhichWh)
{
	if(nWhichWh != METER_645_TYPE_DSZ71) //Note:Only METER_645_TYPE_DSZ71  Read Account Kwh.
	{
		return ;
	}

	int nSampRTN = ERR_SAMPLE_OK;
	time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    static int nLastGet_AccountHour = 0;
    int nCurr_Hour = gmTime.tm_hour;
	if(nCurr_Hour == nLastGet_AccountHour)
	{
		return;
	}

	nLastGet_AccountHour = nCurr_Hour;
	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH, WHMETER_ROUGH_LAST1_POS_SUM);

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH, WHMETER_ROUGH_LAST1_NEG_SUM);

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH, WHMETER_ROUGH_LAST2_POS_SUM);

	nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH, WHMETER_ROUGH_LAST2_NEG_SUM);
	
	TRACE("nWhichWh=[%d] nLastGet_AccountHour=[%d]\n", nWhichWh, nLastGet_AccountHour);
	return;
}


int whMeter_Sample(IN ST_UART_SERVER_ROUGHDATA* pWhim_Rough,IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,IN int nWhichWh)
{
	int nSampRTN = ERR_SAMPLE_OK;
	BOOL	 bNeedReSamp = FALSE;
	if(pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_UartServer)
	{
		if (pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_GBDLT645)
		{
			nSampRTN = whMeter_SigSamp(pWhim_Rough,pClientInfo, nWhichWh, WH_FRAME_TYPE_GET_ACTPOWER_AC, WHMETER_ROUGH_RTACTPower);

			bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;   //2
			if(nSampRTN != ERR_SAMPLE_OK)
			{
				//pWhim_Rough->bNeedReconfig_whMeter = TRUE;
				return nSampRTN;
			}

			nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_REACTPOWER_AC, WHMETER_ROUGH_RTReActPower);

			if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (5 + nWhichWh))
			{
				//TRACE("Crrent nWhichWh=[%d] nSampleTimes=[%d]\n", nWhichWh, pWhim_Rough->meterData[nWhichWh].nSampleTimes);
				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_A, WHMETER_ROUGH_RT_CURR_A);


			}

			if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (6 + nWhichWh))
			{
				//TRACE("Kwh nWhichWh=[%d] nSampleTimes=[%d]\n", nWhichWh, pWhim_Rough->meterData[nWhichWh].nSampleTimes);
				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_TOTALWH_AC, WHMETER_ROUGH_RTkWh_AC);

				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_RVSTATALWH, WHMETER_ROUGH_RT_RVSkWh_AC);

				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_TOTALWH, WHMETER_ROUGH_SUM_RTkWh_AC);
	
				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_MULTI_RVSTATALWH, WHMETER_ROUGH_RT_SUM_RVSkWh_AC);

				//add by lpl 240802
				UartJudgeAndSaveToDB(pWhim_Rough,nWhichWh);

				//---John Added 2022-11-30.For Account Kwh.---//
				Judge_IfNeedGetAccountKwh(pWhim_Rough, pClientInfo,nWhichWh);
			}

			if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (7 + nWhichWh))
			{
				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_VOLT_PHASE_A, WHMETER_ROUGH_RT_VOLT_A);

				nSampRTN = whMeter_SigSamp(pWhim_Rough, pClientInfo,nWhichWh, WH_FRAME_TYPE_GET_POWERFACTOR, WHMETER_ROUGH_RT_PowerFactor);

			}

			//---2022-02-24 Due to Improve Meter Sample Speed
			if(pWhim_Rough->meterData[nWhichWh].nSampleTimes <= (8 + nWhichWh))
			{
				pWhim_Rough->meterData[nWhichWh].nSampleTimes++;
				return nSampRTN;
			}

			pWhim_Rough->meterData[nWhichWh].nSampleTimes = 0;

			Judge_IfNeedGetRelevantValue(pWhim_Rough, pClientInfo,nWhichWh);

		}
	}

	if(bNeedReSamp)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}
	
	return ERR_SAMPLE_OK;
}


/*
If the ratio of any two phase values exceeds a certain range, it is considered that these phase values are unbalanced.
If the ratio of any two phase values is greater than or equal to 1.5 or less than or equal to 0.5,
Then return True to indicate imbalance, otherwise return FALSE to indicate balance.
*/

static BOOL Judge_PhaseValue_Unbalance(float fPh_A, float fPh_B, float fPh_C)
{
    float fRate[3];
    fRate[0] = fPh_A;
    fRate[1] = fPh_B;
    fRate[2] = fPh_C;
    BOOL bRet = METERVALUE_OK;

    int i = 0, j = 0;
    for(i = 0; i < 3; i++)
    {
        TRACE("i=[%d] bRet=[%d]\n", i, bRet);
        for(j = i+1; j < 3; j++)
        {
            float fTemp = fRate[i]/(fRate[j]+0.1);
            
            TRACE("fTemp=[%.2f]\n", fTemp);

            if((fTemp >= 1.5) || (fTemp <= 0.5)) 
            {
                bRet = METERVALUE_ACTPower_UNBANLANCE;
                return bRet;
            }
        }
    }

    return bRet;
}

static BOOL Judge_ActvPower_Abnml(IN ST_UART_SERVER_ROUGHDATA* pWhim_Rough, IN int nWhichWh)
{
	float fPwr_Sum = 0,  fPwr_A = 0,  fPwr_B = 0,  fPwr_C = 0;
	float fPhA_V = 0,  fPhB_V = 0,  fPhC_V = 0;

	fPhA_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_A].fValue; 
	fPhB_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_B].fValue;  
	fPhC_V = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RT_VOLT_C].fValue;
	if((fPhA_V < 50.0) || (fPhB_V < 50.0) || (fPhC_V < 50.0)) //Lock Phase
	{
		return METERVALUE_VOLT_UNBANLANCE;
	}

	fPwr_Sum = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue;
	fPwr_A = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhA].fValue;
	fPwr_B = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhB].fValue;
	fPwr_C = pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ROUGH_ACTPower_PhC].fValue;

    float fActPower = fabs(fPwr_Sum);
    if(fActPower > 10.0)
    {
        return Judge_PhaseValue_Unbalance(fPwr_A, fPwr_B, fPwr_C);
    }

    return METERVALUE_OK;
}

void RS485_StuffChanl(ST_UART_SERVER_ROUGHDATA *pWhim_Rough,int nWhichWh)
{
	//int nSamplerId = ((SAMPLER*)pWhim_Rough->hSampler)->info.nSamplerID;
	SAMPVAL_MAP whStuff[] = 
	{
		{WHMETER_ROUGH_RTACTPower,				400},     	
		{WHMETER_ROUGH_ACTPower_PhA,			401},    	
		{WHMETER_ROUGH_ACTPower_PhB,			402},     	
		{WHMETER_ROUGH_ACTPower_PhC,			403},     	

		{WHMETER_ROUGH_RTReActPower,			404},     	
		{WHMETER_ROUGH_ReActPower_PhA,			405},		
		{WHMETER_ROUGH_ReActPower_PhB,			406},		
		{WHMETER_ROUGH_ReActPower_PhC,			407},		

		{WHMETER_ROUGH_RTkWh_AC,				411},		
		{WHMETER_ROUGH_RTkvar,					412},		
		{WHMETER_ROUGH_RT_VOLT_A,				413},		
		{WHMETER_ROUGH_RT_CURR_A,				414},		
		{WHMETER_ROUGH_RT_VOLT_B,				415},		
		{WHMETER_ROUGH_RT_CURR_B,				416},		
		{WHMETER_ROUGH_RT_VOLT_C,				417},		
		{WHMETER_ROUGH_RT_CURR_C,				418},		
		{WHMETER_ROUGH_RTkvar2,					419},		
		//{WHMETER_ROUGH_RTReActPower,			420},       
		{WHMETER_ROUGH_RT_RVSkWh_AC,			422},		
		//{WHMETER_ROUGH_RT_PowerFactor,		423},       
		{WHMETER_ROUGH_RT_Freq,					424},		
		{WHMETER_ROUGH_PMAXDEMAND,				425},		

		{WHMETER_ROUGH_RT_PowerFactor,			426},		
		{WHMETER_ROUGH_PowerFactor_PhA,			427},		
		{WHMETER_ROUGH_PowerFactor_PhB,			428},		
		{WHMETER_ROUGH_PowerFactor_PhC,			429},		    

		{WHMETER_ROUGH_LAST1_POS_SUM,			1501},		
		{WHMETER_ROUGH_LAST1_POS_SHARP,			1502},		
		{WHMETER_ROUGH_LAST1_POS_PEAK,			1503},		
		{WHMETER_ROUGH_LAST1_POS_FLAT,			1504},		
		{WHMETER_ROUGH_LAST1_POS_WAVE,			1505},		

		{WHMETER_ROUGH_LAST1_NEG_SUM,			1506},		
		{WHMETER_ROUGH_LAST1_NEG_SHARP,			1507},
		{WHMETER_ROUGH_LAST1_NEG_PEAK,			1508},	
		{WHMETER_ROUGH_LAST1_NEG_FLAT,			1509},	
		{WHMETER_ROUGH_LAST1_NEG_WAVE,			1510},	

		{WHMETER_ROUGH_LAST2_POS_SUM,			1511},	
		{WHMETER_ROUGH_LAST2_POS_SHARP,			1512},	
		{WHMETER_ROUGH_LAST2_POS_PEAK,			1513},	
		{WHMETER_ROUGH_LAST2_POS_FLAT,			1514},	
		{WHMETER_ROUGH_LAST2_POS_WAVE,			1515},	

		{WHMETER_ROUGH_LAST2_NEG_SUM,			1516},	
		{WHMETER_ROUGH_LAST2_NEG_SHARP,			1517},	
		{WHMETER_ROUGH_LAST2_NEG_PEAK,			1518},	
		{WHMETER_ROUGH_LAST2_NEG_FLAT,			1519},	
		{WHMETER_ROUGH_LAST2_NEG_WAVE,			1520},	
		{WHMETER_ROUGH_SUM_RTkWh_AC,			1491}, 
		{WHMETER_ROUGH_SHARP_RTkWh_AC,			1492},
		{WHMETER_ROUGH_PEAK_RTkWh_AC,			1493},
		{WHMETER_ROUGH_FLAT_RTkWh_AC,			1494},
		{WHMETER_ROUGH_WAVE_RTkWh_AC,			1495},
		{WHMETER_ROUGH_RT_SUM_RVSkWh_AC,		1496}, 
		{WHMETER_ROUGH_RT_SHARP_RVSkWh_AC,		1497},
		{WHMETER_ROUGH_RT_PEAK_RVSkWh_AC,		1498},
		{WHMETER_ROUGH_RT_FLAT_RVSkWh_AC,		1499},
		{WHMETER_ROUGH_RT_WAVE_RVSkWh_AC,		1500},

		{WHMETER_ROUGH_CURRENTPOSDEMAND,		432},

	};

	int n = 0;

	if((pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPE_NotInstalled)
		|| (pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_DataFromMaster))
	{
		return 1;
	}

	for(n = 0; n < ITEM_OF(whStuff); n++)
	{
		//if((401 + kkk) == 407) //Skip 407 AC Meter For Local
		//{
		//	continue;
		//}
		Push_VarValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler,
			401 + nWhichWh,
			whStuff[n].iSigID,
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[whStuff[n].iRawValIdx]);

		//TRACE("-------kkk=[%d] nSigNal=[%d] iRawValIdx=[%d] The whRawLVal = %ld fValue=%f-------------\n",\
		//	kkk, n, whStuff[n].iRawValIdx, pWhim_Rough->meterData[kkk].varWHMeterRough[whStuff[n].iRawValIdx].ulValue,\
		//	pWhim_Rough->meterData[kkk].varWHMeterRough[whStuff[n].iRawValIdx].fValue);
	}

	VAR_VALUE varV;
	varV.ulValue = pWhim_Rough->meterData[nWhichWh].emSelfCheckCode;
	Push_VarValue(SETTYPE_RAW_SP, pWhim_Rough->hSampler,
		401 + nWhichWh, 423, varV);

	return 0;
}


int whMeter645_Sample(ST_UART_SERVER_ROUGHDATA *pUart,UART_SERVER_TCP_CLIENT_INFO *pClientInfo,int TypeNum)
{
	SAMPLER *pSampler = (SAMPLER *)pUart->hSampler;
	int nRet = 0;
	int kkk;
	int nErrrCode;
	if((pUart->meterData[TypeNum].emMeterType == METER_TYPE_NotInstalled) 
			|| (pUart->meterData[TypeNum].emMeterType == METER_TYPR_DataFromMaster)) 
	{
		return 1;
	}

	nErrrCode=whMeter_Sample(pUart,pClientInfo,TypeNum);
	if(ERR_SAMPLE_OK != nErrrCode) 
	{
		pUart->meterData[TypeNum].nInterrupt_Times++;
		if(pUart->meterData[TypeNum].nInterrupt_Times >= MAX_UART_SERVER_TCP_TRY_TIMES)
		{
			pUart->meterData[TypeNum].nInterrupt_Times = 0;
			if(!pUart->meterData[TypeNum].bIsCommFail)
			{
				pUart->meterData[TypeNum].bIsCommFail = TRUE;
				TRACEE("Now set kwMeterSampler[%d] --> %d commFailed---UART SERVER\n", pSampler->info.nSamplerID, TypeNum);
				SetCommStatus(pUart->hSampler, 401 + TypeNum, FALSE);
				for(kkk = 0; kkk < _MAX_WH_ROUGH; kkk++)
				{
					pUart->meterData[TypeNum].varWHMeterRough[kkk].lValue = 0;
				}
				pUart->meterData[TypeNum].emSelfCheckCode = METERVALUE_OK;
				//return nErrrCode;
			}
		}
	}
	else  
	{
		//Added By John For MeterValue SelfCheck.
		nRet = Judge_ActvPower_Abnml(pUart, TypeNum);
		if(nRet == METERVALUE_VOLT_UNBANLANCE)  //Single-phase voltage below 50V
		{
			pUart->meterData[TypeNum].nUnderVoltTimes++;
			if(pUart->meterData[TypeNum].nUnderVoltTimes > 30)
			{
				pUart->meterData[TypeNum].nUnderVoltTimes = 30;
				pUart->meterData[TypeNum].emSelfCheckCode = METERVALUE_VOLT_UNBANLANCE;
			}
			pUart->meterData[TypeNum].nPhaseUnbalanceTimes = 0;
		}
		else if(nRet == METERVALUE_ACTPower_UNBANLANCE) //Three-phase active power imbalance
		{
			pUart->meterData[TypeNum].nPhaseUnbalanceTimes++;
			if(pUart->meterData[TypeNum].nPhaseUnbalanceTimes > 30)
			{
				pUart->meterData[TypeNum].nPhaseUnbalanceTimes = 30;
				pUart->meterData[TypeNum].emSelfCheckCode = METERVALUE_ACTPower_UNBANLANCE;
			}
			pUart->meterData[TypeNum].nUnderVoltTimes = 0;//证明检测电压已通过
		}
		else if(nRet == METERVALUE_OK)
		{
			pUart->meterData[TypeNum].nUnderVoltTimes = 0;
			pUart->meterData[TypeNum].nPhaseUnbalanceTimes = 0;
			pUart->meterData[TypeNum].emSelfCheckCode = METERVALUE_OK;
		}

		if(pUart->meterData[TypeNum].bIsCommFail)
		{
			pUart->meterData[TypeNum].bIsCommFail = FALSE;
			SetCommStatus(pUart->hSampler, 401 + TypeNum, TRUE);
		}
		pUart->meterData[TypeNum].nInterrupt_Times = 0;
	}

	return ERR_SAMPLE_OK;


}