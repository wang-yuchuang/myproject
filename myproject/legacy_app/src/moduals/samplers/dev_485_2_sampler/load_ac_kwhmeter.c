#include "dev_485_2_sampler_main.h"

/*--------------------------------------------*/
/*  ChgAToHex: Convert array to hexa value. */
/* example: array="12345678af",num=5
after calling  a_to_hexa(array,hexa,num)
hexa={0x12 + 0x33,0x34 + 0x33,0x56 + 0x33,0x78 + 0x33,0xaf + 0x33}
*/
/*--------------------------------------------*/
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


void GetKWHMeterSN(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichWh, char* szSrc)
{
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)((SAMPLER*)pDev_485_2_Rough->hSampler)->hDPR;
	char szSN[13];
	char* psz = szSrc;
	if(szSrc == NULL)
	{
		VAR_VALUE varV;
		varV.blobValue = szSN;
 
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 401 + nWhichWh, 403, &varV, 13);
		psz = szSN;
	}

	//sscanf(psz, "%02X%02X%02X%02X%02X%02X",
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[5],
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[4],
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[3],
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[2],
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[1],
	//	&pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[0]);
	BYTE byConvt[6];
	sscanf(psz, "%02X%02X%02X%02X%02X%02X",
		&byConvt[0], &byConvt[1], &byConvt[2],
		&byConvt[3], &byConvt[4], &byConvt[5]);
	int n;
	for(n = 0 ; n < 6; n++)
	{
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[n] = byConvt[5 - n];
	}

	TRACEX(" now update meterSN:[%s], [%02X%02X%02X%02X%02X%02X]\n ", psz,
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[5],
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[4],
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[3],
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[2],
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[1],
		pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6[0]);

	if(strlen(psz) > 0)
	{
		pDev_485_2_Rough->meterData[nWhichWh].byModbusAddress = (BYTE)atoi(psz);
	}
	else
	{
		pDev_485_2_Rough->meterData[nWhichWh].byModbusAddress = 0x01; //default is 1
	}
}

#define  INIT_KWH_FRAME(pFrame, _type, _ctrlCode, _sendDataLen, _headerLen, _pHeader, _DataLen, _nPointNum) \
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

static void Make_CS(KWH_FRAME_INFO* pFrame)
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

void kwhMeter_InitRoughData(RS485_2_SAMPLER_DATA *pDev_485_2_Rough)
{
	//Now init the frames
	UINT nWhichWh = 0, kkk = 0;
	SAMPLER* pSampler = (SAMPLER*)pDev_485_2_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	KWH_FRAME_INFO *pFrm = pDev_485_2_Rough->frm_whFrames;
 
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_TOTALWH_AC,		0x11,	0x04,	4,	"00000100",		4,	2);
	
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_VOLT_PHASE_A,	0x11,	0x04,	4,	"00010102",		2,	1);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_VOLT_PHASE_B,	0x11,	0x04,	4,	"00020102",		2,	1);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_VOLT_PHASE_C,	0x11,	0x04,	4,	"00030102",		2,	1);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_CURR_PHASE_A,	0x11,	0x04,	4,	"00010202",		3,	3);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_CURR_PHASE_B,	0x11,	0x04,	4,	"00020202",		3,	3);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_CURR_PHASE_C,	0x11,	0x04,	4,	"00030202",		3,	3);

	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_TOTALKVAR_AC,	0x11,	0x04,	4,	"00000300",		4,	2);

	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_ACTPOWER_AC,		0x11,	0x04,	4,	"00000302",		3,	4);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_REACTPOWER_AC,	0x11,	0x04,	4,	"00000402",		3,	4);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_RVSKWH_AC,		0x11,	0x04,	4,	"00000200",		4,	2);
 	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_POWERFACTOR,		0x11,	0x04,	4,	"00000602",		2,	3);
	INIT_KWH_FRAME(pFrm,	KWH_FRAME_TYPE_GET_FREQ,			0x11,	0x04,	4,	"02008002",		2,	3);

	pDev_485_2_Rough->nSampACMeterCycle = MIN_SAMP_IMEU2_PERIOD;
 	for(nWhichWh = 0; nWhichWh < MAX_ACWH_NUM; nWhichWh++)
	{
		for(kkk = 0; kkk < _MAX_KWHAC_ROUGH; kkk++)
		{
			pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[kkk].lValue = SAMP_INVALID_VALUE;
		}
		//Meter SN
		GetKWHMeterSN(pDev_485_2_Rough, nWhichWh, NULL);
		
		VAR_VALUE varV;
		//Meter install type
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 401 + nWhichWh, 401, &varV, 4);
		pDev_485_2_Rough->meterData[nWhichWh].emMeterType = (METER_TYPE)varV.emValue;

		//AC power factor
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 401 + nWhichWh, 402, &varV, 4);
		pDev_485_2_Rough->meterData[nWhichWh].fACMeter_PowerFactor = varV.fValue;

		//AC meter protocol type
		varV.emValue = KWHACMETER_PROTOCOL_GBDLT645; //default is this setting
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 401 + nWhichWh, 404, &varV, 4);
		pDev_485_2_Rough->meterData[nWhichWh].emACMeter_ProtocolType = varV.emValue;
		//Baudrate of the kwhMeter, newly added 20190226

		varV.emValue = 0;
		pDPR->Get(pDev_485_2_Rough->hSampler, GETTYPE_ST_VAR, 401 + nWhichWh, 405, &varV, 4);
		if(varV.emValue == 0) 
		{
			if((pDev_485_2_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_dc))
			{
				snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
			}
			else
			{
				snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
			}
		}
		else
		{
			if(varV.emValue == 1)
			{
				snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
			}
			else
			{
				snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
			}
		}
	}
	return;
}

static float ConvertBCD_2WhVal(BYTE* pBCD, KWH_FRAME_INFO *pFrameInfo)
{
	float fOut = 0.0;
	
	//if(pFrameInfo->frameType == KWH_FRAME_TYPE_GET_TOTALWH || pFrameInfo->frameType == KWH_FRAME_TYPE_GET_RVSTATALWH) //kWh had no sign
	//{
	//	fOut = (float)((pBCD[3] >> 4)      *100000);
	//	//fOut = (float)(((pBCD[3] >> 4) & 0x07)      *100000);
	//	fOut += (float)((pBCD[3] & 0x0F) *10000);
	//	fOut += (float)((pBCD[2] >> 4) *1000);
	//	fOut += (float)((pBCD[2] & 0x0F) *100);
	//	fOut += (float)((pBCD[1] >> 4) *10);
	//	fOut += (float)((pBCD[1] & 0x0F) *1);
	//	fOut += (float)((pBCD[0] >> 4) *0.1);
	//	fOut += (float)((pBCD[0] & 0x0F) *0.01);
	//}
	//else
	
	{
		int nLen = pFrameInfo->nDataLen;
		int nPoint = pFrameInfo->nPointNum;
		int n;
		float fSCal = 1.0;
		for(n = 0; n < nPoint; n++)
		{
			fSCal /= 10.0;
		}

		//TRACE("-------The nLen = %d, nPoint = %d, fScale = %f, Bytes[%02X%02X]-----\n",
		//	nLen, nPoint, fSCal, pBCD[0], pBCD[1]);

		if(nLen < 1)
		{
			return 0.0;
		}

 		BOOL bSigned = FALSE;
		
		if(((pBCD[nLen - 1] >> 7) & 0x01) == 0x01)
		{
			bSigned = TRUE;
		}

		//int k = 0;
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

static BYTE* MakewhMeterSendBuff(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, KWH_FRAME_INFO *pFrameInfo)
{
	BYTE* pbyBuff = pDev_485_2_Rough->abySendBuf;
	BYTE* pbyBuffOrig = pbyBuff;
	memcpy(pbyBuff, &pFrameInfo->sendFrame, 10);
	pbyBuff += 10;
	int nDataLen = pFrameInfo->nFrameLen - 12;
	//if(nDataLen > 0)
	{
		memcpy(pbyBuff, pFrameInfo->sendFrame.pbyData, nDataLen);
	}
	pbyBuff += nDataLen;
	*pbyBuff = pFrameInfo->sendFrame.byChkSum;
	pbyBuff += 1;
	*pbyBuff = pFrameInfo->sendFrame.byEOI;
	return pbyBuffOrig;
}
#define MIN_WH_FRAME_LEN	(12 + 4) //min len = 16, include FE FE FE FE,
static int Pack_kwhMeterCmd_AndGetReply(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN KWH_FRAME_INFO* pwhFrame)
{
	int nWriteBytes;
	int nReadBytes;
 
	COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;
	
	//Clear buffer
	pComm->Control(pDev_485_2_Rough->hPort,
		COMM_PURGE_RXCLEAR, NULL, 0);

	Sleep(50);
	//TRACEX("Now begin to write whMeter cmd!\n");
	//First Send wake up frame
	VAR_VALUE varWakeUp;
	varWakeUp.ulValue = KWH_WAKEUP_FRAME;
	nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, varWakeUp.abyValue, 4);
	TRACE("whMeter cmd Write wakeup OK [%d] !\n", nWriteBytes);
	//Sleep(100);

	//Now beging send frame
	nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, 
		MakewhMeterSendBuff(pDev_485_2_Rough, pwhFrame), pwhFrame->nFrameLen);

	TRACE("whMeter cmd Write cccc OK [%d], expect len = %d!\n", nWriteBytes, pwhFrame->nFrameLen);
	if(nWriteBytes < pwhFrame->nFrameLen)
	{
		//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return RS485_WRITE_ERR;
	}

	Sleep(100);

	//TRACEX("Now begin to read whMeter cmd reply!\n");
	//if(bNeedGetReply)
	{
		nReadBytes = pComm->Read(pDev_485_2_Rough->hPort,
			pDev_485_2_Rough->abyRcvBuf, /*pwhFrame->nExpectAnswerLen + 4*/ RS485_RECV_BUF_LEN);

		TRACEX("Read whMeter cmd OK! ExpectLen = %d,  readLen = %d, read[%s]!\n", pwhFrame->nExpectAnswerLen + 4, nReadBytes,
			pDev_485_2_Rough->abyRcvBuf);

		if(nReadBytes <= 0)
		{
			//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
			return RS485_READ_ERR;
		}
		
	}
	pDev_485_2_Rough->nReadBuffBytes = nReadBytes;
	//Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
	return RS485_COMM_OK;
}
#define  ERR_WRONG_FRAME	0xFF
static int ExtractWhFrame(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, KWH_FRAME_INFO *pFrameInfo)
{
	BYTE* pBuff = pDev_485_2_Rough->abyRcvBuf;
	int n = 0;
	int m;
	int nStart = -1;
	for(n = 0; n < pDev_485_2_Rough->nReadBuffBytes; n++)
	{
		//Jimmy note 20191207: do not need judge FEFEFEFE, another manufactor does NOT replay this
		//if((pBuff[n] == 0xFE) && (pBuff[n + 1] == 0x68))
		if(pBuff[n] == 0x68)
		{
			//m = n+ 8;
			m = n + 7;
			if((m < pDev_485_2_Rough->nReadBuffBytes) &&
				(pBuff[m] == 0x68))
			{
				//nStart = n + 1;
				nStart = n;
				break;
			}
		}
	}
	if(nStart == -1)
	{
		return ERR_WRONG_FRAME;
	}
	
	////---------------Test------------------------------------
	TRACE("whMeter[%d, %d]----nStart = %d, The Buff is:[", pDev_485_2_Rough->nSamplerId,  nStart);
	for(n = nStart; n < pDev_485_2_Rough->nReadBuffBytes; n++)
	{
		TRACE("%02X", pBuff[n]);
	}
	TRACE("]\n");
	//-------------------------------------------------------
	
	BYTE nErrCode = ERR_SAMPLE_OK;
	KWH_A_FRAME *pRecFrame = (KWH_A_FRAME*)(pBuff + nStart);

	int nPasedFrmLen = pDev_485_2_Rough->nReadBuffBytes - nStart;

	if(nPasedFrmLen < pFrameInfo->nExpectAnswerLen)
	{
		TRACEX("------The frmeExpectLe = %d, but RealLen = %d, so discard this frme!-------------\n", pFrameInfo->nExpectAnswerLen, nPasedFrmLen);
		return ERR_WRONG_FRAME;
	}

	BYTE* pbyValStart = pBuff + nStart + 10;
	switch(pFrameInfo->sendFrame.byCtrlCode)
	{
	case 0x11: //read data
		if((pRecFrame->byCtrlCode != 0x91) ||
			(pRecFrame->byDataLen == 0x01))
		{
			//TRACEX("whEEEEEEEEEEEEEEEEE= %d, Err+1 = %d\n",*(BYTE*)pbyValStart, *(BYTE*)(pbyValStart + 1));
			nErrCode = (*(BYTE*)pbyValStart) - 0x33;
		}
		else
		{
			for(n = 0; n < 4; n++)
			{
				pbyValStart[n + 4] -= 0x33;
			}
			pFrameInfo->fGetData = ConvertBCD_2WhVal(pbyValStart + 4, pFrameInfo);
			TRACE("----The fGetData is [%.2f]----\n", pFrameInfo->fGetData);
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

static int kwhMeter_SigSamp(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichWh, KWH_FARAME_TYPE emCmdType, KWH_ROUGH_SIG emSigType)
{
	if(Mutex_Lock(pDev_485_2_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_TIMEOUT;
	}

	COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;

	pComm->Control(pDev_485_2_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett,
		strlen(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett));

	KWH_FRAME_INFO* pFrame = &pDev_485_2_Rough->frm_whFrames[emCmdType];

	memcpy(pFrame->sendFrame.byAddrA0_A5, pDev_485_2_Rough->meterData[nWhichWh].byReadMeterA0A6, 6);
	
	Make_CS(pFrame);
	if (RS485_COMM_OK != Pack_kwhMeterCmd_AndGetReply(pDev_485_2_Rough, pFrame))
	{
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_SAMPLE_TIMEOUT;
	}
	int nErrCode = ExtractWhFrame(pDev_485_2_Rough, pFrame);
	if(ERR_SAMPLE_OK != nErrCode)
	{
		pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[KWHMETER_ERR_WORD].lValue = nErrCode;
		TRACE("whMeter Has Faillllllllllllllllllllllllllllllllllllllllll for type[%d], ErrCode = %d.\n",  emCmdType, nErrCode);
		Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[KWHMETER_ERR_WORD].ulValue = 0;
	pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue = pFrame->fGetData;
	if((pDev_485_2_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac) && 
		((emSigType == KWHMETER_ROUGH_RTkWh_AC) || 
		(emSigType == KWHMETER_ROUGH_RTkvar) || 
		(emSigType == KWHMETER_ROUGH_RT_CURR_A) || 
		(emSigType == KWHMETER_ROUGH_RT_CURR_B) || 
		(emSigType == KWHMETER_ROUGH_RT_CURR_C) ||
		(emSigType == KWHMETER_ROUGH_RTACTPower) ||
		(emSigType == KWHMETER_ROUGH_RTReActPower) ||
		(emSigType == KWHMETER_ROUGH_RT_RVSkWh_AC)))
	{
		pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= pDev_485_2_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
	}
	else if(emSigType == KWHMETER_ROUGH_RT_Freq)
	{
		pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= 10; //AC Freq Need * 10
	}
	Mutex_Unlock(pDev_485_2_Rough->hSamplerLock);

	return ERR_SAMPLE_OK;
}

 
int kwhMeter_Sample(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichWh)
{
	int 		nSampRTN = ERR_SAMPLE_OK;
	BOOL	 	bNeedReSamp = FALSE;
	if(pDev_485_2_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac)
	{
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_TOTALWH_AC, KWHMETER_ROUGH_RTkWh_AC);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		if(nSampRTN != ERR_SAMPLE_OK)
		{
			//pDev_485_2_Rough->bNeedReconfig_whMeter = TRUE;
			TRACE("kwhMeter_SigSamp != ERR_SAMPLE_OK\n");
			return nSampRTN;
		}
		
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_VOLT_PHASE_A, KWHMETER_ROUGH_RT_VOLT_A);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_VOLT_PHASE_B, KWHMETER_ROUGH_RT_VOLT_B);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_VOLT_PHASE_C, KWHMETER_ROUGH_RT_VOLT_C);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_CURR_PHASE_A, KWHMETER_ROUGH_RT_CURR_A);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_CURR_PHASE_B, KWHMETER_ROUGH_RT_CURR_B);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_CURR_PHASE_C, KWHMETER_ROUGH_RT_CURR_C);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		
		//nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_TOTALKVAR_AC, KWHMETER_ROUGH_RTkvar);
		//bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_ACTPOWER_AC, KWHMETER_ROUGH_RTACTPower);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

		//nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_REACTPOWER_AC, KWHMETER_ROUGH_RTReActPower);
		//bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		
		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_RVSKWH_AC, KWHMETER_ROUGH_RT_RVSkWh_AC);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

		//nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_POWERFACTOR, KWHMETER_ROUGH_RT_PowerFactor);
		//bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

		nSampRTN = kwhMeter_SigSamp(pDev_485_2_Rough, nWhichWh, KWH_FRAME_TYPE_GET_FREQ, KWHMETER_ROUGH_RT_Freq);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

 
	}

	//Ignore Weather Need Resample, To solve LoadAC CommFail Question.
	//if(bNeedReSamp)
	//{
	//	return ERR_SAMPLE_NEED_RESAMP;
	//} 
	return ERR_SAMPLE_OK;
}


typedef struct tagSampValMapper
{
	int		iRawValIdx; //Sample raw float data, if not a float , think it as float
	int		iSigID; //only sample signal, so no need to assign sig type
}SAMPVAL_MAP;

static void RS485_2_MeterStuffChanl(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichWh)
{
	//int nSamplerId = ((SAMPLER*)pDev_485_2_Rough->hSampler)->info.nSamplerID;
	SAMPVAL_MAP whStuff[] = 
	{
		{KWHMETER_ROUGH_RTkWh_AC,					411},
		{KWHMETER_ROUGH_RTkvar,						412},
		{KWHMETER_ROUGH_RT_VOLT_A,					413},
		{KWHMETER_ROUGH_RT_CURR_A,					414},
		{KWHMETER_ROUGH_RT_VOLT_B,					415},
		{KWHMETER_ROUGH_RT_CURR_B,					416},
		{KWHMETER_ROUGH_RT_VOLT_C,					417},
		{KWHMETER_ROUGH_RT_CURR_C,					418},
		{KWHMETER_ROUGH_RTACTPower,					419},
		{KWHMETER_ROUGH_RTReActPower,				420},
		{KWHMETER_ROUGH_RT_RVSkWh_AC,				422},
 		{KWHMETER_ROUGH_RT_PowerFactor,				423},
		{KWHMETER_ROUGH_RT_Freq,					424},
	};

	int n;
	for(n = 0; n < ITEM_OF(whStuff); n++)
	{
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
			401 + nWhichWh,
			whStuff[n].iSigID,
			pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[whStuff[n].iRawValIdx]);

		TRACE("------- nSigNal=[%d] iRawValIdx=[%d] The whRawLVal = %ld fValue=%f-------------\n",\
			n, whStuff[n].iRawValIdx, pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[whStuff[n].iRawValIdx].ulValue,\
			pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[whStuff[n].iRawValIdx].fValue);
	}

	return;
}

int ACMeter_Sample(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough)
{
	int nErrrCode;
 	UINT kkk  = 0, nWhichWh = 0;
	//int nRetryTimes;
 	
	for(nWhichWh = 0; nWhichWh < MAX_ACWH_NUM; nWhichWh++)
	{
		TRACE("---------Now sample Localc AC Meter kwh[%d]---------\n", nWhichWh);
		if(pDev_485_2_Rough->meterData[nWhichWh].emMeterType == METER_TYPE_NotInstalled)
		{
			continue;
		}
		
		nErrrCode = kwhMeter_Sample(pDev_485_2_Rough, nWhichWh);
		if(ERR_SAMPLE_OK != nErrrCode)
		{
			pDev_485_2_Rough->meterData[nWhichWh].nInterrupt_Times++;
			if(pDev_485_2_Rough->meterData[nWhichWh].nInterrupt_Times >= MAX_RS4852_TRY_TIMES)
			{
				pDev_485_2_Rough->meterData[nWhichWh].nInterrupt_Times = 0;
				if(!pDev_485_2_Rough->meterData[nWhichWh].bIsCommFail)
				{
					pDev_485_2_Rough->meterData[nWhichWh].bIsCommFail = TRUE;
					TRACEX("Now User Load kwMeterSampler[%d] - commFailed.\n", nWhichWh);
					SetCommStatus(pDev_485_2_Rough->hSampler, 401 + nWhichWh, FALSE);
					for(kkk = 0; kkk < _MAX_KWHAC_ROUGH; kkk++)
					{
						pDev_485_2_Rough->meterData[nWhichWh].varWHMeterRough[kkk].lValue = 0;//SAMP_INVALID_VALUE;
					}
					//return nErrrCode;
				}
			}
		}
		else 
		{
			if(pDev_485_2_Rough->meterData[nWhichWh].bIsCommFail)
			{
				pDev_485_2_Rough->meterData[nWhichWh].bIsCommFail = FALSE;
				SetCommStatus(pDev_485_2_Rough->hSampler, 401 + nWhichWh, TRUE);
			}
			pDev_485_2_Rough->meterData[nWhichWh].nInterrupt_Times = 0;
		}

		//Now do the stuff chanel
		RS485_2_MeterStuffChanl(pDev_485_2_Rough, nWhichWh);
	}
	return ERR_SAMPLE_OK;
}

int kwhMeter_SendCtrl(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
	PARAM_TYPE	emParaType, 
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	printf("Load AC Meter Cfg Changed, nParamId%d nValSize=%d nDevId=[%d]\n", \
		nParamId, nValSize, nDevId);
	
	UINT nWhichWh = (nDevId - 401);
	if(nWhichWh > MAX_ACWH_NUM)
	{
		TRACE("Error: Wrong Meter Index=[%d]\n", nWhichWh);
		return ERR_CONTROL_FAILED;
	}
	if(emParaType == PARAM_TYPE_SETTING)
	{		
		//settings
		if(nParamId == 403)
		{
			if(nValSize != 13)
			{
				return ERR_CONTROL_FAILED;
			}
 			GetKWHMeterSN(pDev_485_2_Rough, nWhichWh, (char*)pVarVal->blobValue);
		}
		else if(nParamId == 401)
		{
			if(nValSize != 4)
			{
				return ERR_CONTROL_FAILED;
			}
			pDev_485_2_Rough->meterData[nWhichWh].emMeterType = (METER_TYPE)pVarVal->emValue;
			printf("nWhichWh=[%d] emMeterType=[%d]\n", nWhichWh, pDev_485_2_Rough->meterData[nWhichWh].emMeterType);
		}
		else if(nParamId == 402)
		{
			if(nValSize != 4)
			{
				return ERR_CONTROL_FAILED;
			}
			pDev_485_2_Rough->meterData[nWhichWh].fACMeter_PowerFactor = pVarVal->fValue;
		}
		else if(nParamId == 404)
		{
			if(nValSize != 4)
			{
				return ERR_CONTROL_FAILED;
			}
			pDev_485_2_Rough->meterData[nWhichWh].emACMeter_ProtocolType = (KWHACMETER_PROTOTP)pVarVal->emValue;
		}
		else if(nParamId == 405)
		{
			if(nValSize != 4)
			{
				return ERR_CONTROL_FAILED;
			}

			if(pVarVal->emValue == 0) //auto set by DC/AC, to be compatible with marchet exsiting kwhMeter
			{
				if((pDev_485_2_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac))
				{
					snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
				}
				else
				{
					snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
				}
			}
			else
			{
				if(pVarVal->emValue == 1)
				{
					snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "9600,E,8,1");
				}
				else
				{
					snprintf(pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett, 32, "2400,E,8,1");
				}
			}
			TRACEX("The szBaudrateSett is[%s]\n",pDev_485_2_Rough->meterData[nWhichWh].szBaudrateSett);
 		}
	}
	TRACE("---Set UserLoad AC Meter Success!----\n");
	return ERR_CONTROL_OK;
}
