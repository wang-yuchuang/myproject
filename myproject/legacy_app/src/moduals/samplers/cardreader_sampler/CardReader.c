#include "rs232_cr_sampler_main.h"

static BYTE MakeBCC(MT_FRAME* pFrame, int nDataLen)
{
	BYTE byBCC;
	byBCC = (pFrame->STX);
	byBCC ^= ((pFrame->usDataLen >> 8) & 0xFF);
	byBCC ^= ((pFrame->usDataLen) & 0xFF);
	byBCC ^= (pFrame->byCmd);
	byBCC ^= (pFrame->byCmdArg);
	int	i = 0;
	for (i = 0; i < nDataLen; i++)
	{
		byBCC ^= (pFrame->pbyData[i]);
	}
	byBCC ^= (pFrame->ETX);

	pFrame->BCC = byBCC;
	return byBCC;
}

static USHORT Swap2Bytes(USHORT usVal)
{
	USHORT usNew;
	usNew = (usVal >> 8) + (usVal << 8);
	return usNew;
}

static void Init_ACardReader_Frame(CARDREADER_FRAME_INFO *pFrameInfo, CARDREADER_FARAME_TYPE frametype, USHORT usLen,
	BYTE byCmd, BYTE byCmdArg, BYTE byDataLen, BYTE* pbyData, USHORT usReplyDataLen)
{
	pFrameInfo->frameType = frametype;
	MT_FRAME *pFrame = &pFrameInfo->sendFrame;
	pFrame->STX = 0x02;
	pFrame->ETX = 0x03;
	pFrame->usDataLen = usLen;//Swap2Bytes(usLen);	//swap2Bytes
	pFrame->byCmd = byCmd;
	pFrame->byCmdArg = byCmdArg;
	//pFrame->BCC = MakeBCC(pFrame);
	pFrameInfo->usExpectAnswerLen = usReplyDataLen + 7;//����cmd��cmdArg�����ֽ�/**/
	if(byDataLen > 0)
	{
		memcpy(pFrame->pbyData, pbyData, byDataLen);
	}
	pFrameInfo->usFrameLen = 5 + usLen; //����cmd��cmdArg�����ֽ�/**/
}

#define INIT_CARDREADER_FRAME(pFrameInfo, _frameType, _usLen, _byCmd, _byCmdArg, _byDataLen, _byData, _usAnswerDataLen) \
	do{ \
		Init_ACardReader_Frame((pFrameInfo), (_frameType), (_usLen), (_byCmd), (_byCmdArg), (_byDataLen), (_byData), (_usAnswerDataLen)); \
		pFrameInfo++;\
	}while(0)

void CardReader_InitRoughData(CR232_SAMPLER_DATA *pCR_Rough)
{
	static BYTE bySec0KeyA[] = {0,	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	static BYTE bySec3KeyA[] = {3,	0x8A, 0x72, 0x64, 0x53, 0x4F, 0x3C};
	static BYTE bySec5KeyA[] = {5,	0x3E, 0x7F, 0x28, 0xAD, 0x8C, 0x22};
	static BYTE byReadSec0Blk0[] = {0 , 0};
	static BYTE byReadSec3Blk0[] = {3 , 0};
	static BYTE byReadSec5Blk0[] = {5 , 0};
	//Now init the frames
	CARDREADER_FRAME_INFO *pFrame = pCR_Rough->frm_cardReaderFrames;
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_SERACH_CARD,		0x0002,0x34,0x30,0, NULL,		0x0001);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_VAILIDATE_SEC0_KEYA,	0x0009,0x34,0x32,7, bySec0KeyA,	0x0002);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_READ_SEC0_BLK0,		0x0004,0x34,0x33,2, byReadSec0Blk0,0x0013);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_VAILIDATE_SEC3_KEYA,	0x0009,0x34,0x32,7, bySec3KeyA,	0x0002);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_READ_SEC3_BLK0,		0x0004,0x34,0x33,2, byReadSec3Blk0,0x0013);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_VAILIDATE_SEC5_KEYA,	0x0009,0x34,0x32,7, bySec5KeyA,	0x0002);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_READ_SEC5_BLK0,		0x0004,0x34,0x33,2, byReadSec5Blk0,0x0013);
	//Jimmyע��: ���صķ��ذ�������û�лش�16byte���ݣ������ﲻ�ж�,�ر�ע��20161123
	//INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_WRITE_MONEY,		0x0014,0x34,0x34,2, byReadSec3Blk0,0x0013);
	INIT_CARDREADER_FRAME(pFrame,	CARDREADER_FRAME_TYPE_WRITE_MONEY,		0x0014,0x34,0x34,2, byReadSec3Blk0,0x0003);

	pCR_Rough->emAuthorizeStatus = AUTH_STATUS_NONE;

	VAR_VALUE varV;
	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR, 301, 301, &varV, 4);
	pCR_Rough->bIsCardReaderInstalled = (varV.emValue == 1) ? TRUE : FALSE;

	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR, 1, 19, &varV, 4);
	pCR_Rough->bIsLocalAuthorizeDisabled = (varV.emValue == 1) ? TRUE : FALSE;

	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR, 301, 302, &varV, 4);
	pCR_Rough->nAuthorizationTimeouT_Sec = varV.ulValue;

	pCR_Rough->tm_LastSuccessCard = 0;

	//Read OCPP settings
	varV.blobValue = (BYTE *)&pCR_Rough->stESiOCPPConfigs;
	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR,
		1, 26, &varV, sizeof(A_BLOB_ES_iOCPP_CONFIGURAION));

	//Read Locallist
	varV.blobValue = (BYTE *)&pCR_Rough->stLocalList;
	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR,
		301, 6, &varV, sizeof(A_BLOB_LOCALLIST));

	//Read AuthorizeCache
	varV.blobValue = (BYTE *)&pCR_Rough->stAuthCache;
	pCR_Rough->pDPR->Get(pCR_Rough->hSampler, GETTYPE_ST_VAR,
		301, 7, &varV, sizeof(A_BLOB_AUTHORIZECACHE));
	//------------for testing purpose only, Jimmy Note: test result show the large data query/update may takes as long as seconds,
	//we may need to improve the speed later
	//if(pCR_Rough->stLocalList.nAuthorizationDataNumber == 0)
	//{
	//	//now try to add one for example
		//pCR_Rough->stLocalList.nAuthorizationDataNumber = 3;
		//sprintf(pCR_Rough->stLocalList.authorizationDataList[0].szIdTag, "0000000047982356");
		//pCR_Rough->stLocalList.authorizationDataList[0].emAuthorizationStatus = AT_Accpted;
		//sprintf(pCR_Rough->stLocalList.authorizationDataList[1].szIdTag, "1230000022345553");
		//pCR_Rough->stLocalList.authorizationDataList[1].emAuthorizationStatus = AT_Blocked;
		//now save it
		//TRACEX("Now save locallist changes\n");
		//Push_BlobValue(SETTYPE_INTERN_SETT, pCR_Rough->hSampler, 301, 6, &pCR_Rough->stLocalList, sizeof(A_BLOB_LOCALLIST));
	//}
	//else
	//{
	int n;
	TRACEX("-----------------Now print the LocalList,Total[%d],version[%d]----------\n", pCR_Rough->stLocalList.nAuthorizationDataNumber, pCR_Rough->stLocalList.nListVersion);
	for(n = 0; n < pCR_Rough->stLocalList.nAuthorizationDataNumber; n++)
	{
		TRACE("%d. [%s], parent[%s], expireDT[%d] --> %s \n", n+1, pCR_Rough->stLocalList.authorizationDataList[n].szIdTag,
			pCR_Rough->stLocalList.authorizationDataList[n].szParentIdTag, pCR_Rough->stLocalList.authorizationDataList[n].dtExpiryDate,
			(pCR_Rough->stLocalList.authorizationDataList[n].emAuthorizationStatus == AT_Accpted) ? "Accepted" : "Rejected");
	}
	TRACE("----------------------------End of Printing---------------------\n");
	TRACEX("-----------------Now print the AuthCache,Total[%d]----------\n", pCR_Rough->stAuthCache.nAuthorizationDataNumber);
	for(n = 0; n < pCR_Rough->stAuthCache.nAuthorizationDataNumber; n++)
	{
		TRACE("%d. [%s], parent[%s], expireDT[%d] --> %s \n", n+1, pCR_Rough->stAuthCache.authorizationDataList[n].szIdTag,
			pCR_Rough->stAuthCache.authorizationDataList[n].szParentIdTag, pCR_Rough->stAuthCache.authorizationDataList[n].dtExpiryDate,
			(pCR_Rough->stAuthCache.authorizationDataList[n].emAuthorizationStatus == AT_Accpted) ? "Accepted" : "Rejected");
	}
	TRACE("----------------------------End of Printing---------------------\n");
	//}

}

static BYTE* MakeCRSendBuff(CR232_SAMPLER_DATA *pCR_Rough, CARDREADER_FRAME_INFO *pFrameInfo)
{
	BYTE* pbyBuff = pCR_Rough->abySendBuf;
	BYTE* pbyBuffOrig = pbyBuff;
	//memcpy(pbyBuff, &pFrameInfo->sendFrame, 5);
	*(pbyBuff) = pFrameInfo->sendFrame.STX;
	*(pbyBuff + 1) = (BYTE)(pFrameInfo->sendFrame.usDataLen >> 8);
	*(pbyBuff + 2) = (BYTE)(pFrameInfo->sendFrame.usDataLen);
	*(pbyBuff + 3) = pFrameInfo->sendFrame.byCmd;
	*(pbyBuff + 4) = pFrameInfo->sendFrame.byCmdArg;
	pbyBuff += 5;
	int nDataLen = pFrameInfo->usFrameLen - 7;
	if(nDataLen > 0)
	{
		memcpy(pbyBuff, pFrameInfo->sendFrame.pbyData, nDataLen);
		pbyBuff += nDataLen;
	}
	memcpy(pbyBuff, &pFrameInfo->sendFrame.ETX, 1);
	pbyBuff += 1;
	memcpy(pbyBuff, &pFrameInfo->sendFrame.BCC, 1);

	int n;
	TRACEX("The Write frame is: [");
	for( n = 0; n < pFrameInfo->usFrameLen; n++)
	{
		TRACE("%02X ", pbyBuffOrig[n]);
	}
	TRACE("]\n");


	return pbyBuffOrig;
}

#define MAX_CR_COMM_FAIL_TRIES	3
#define MIN_MT_FRAME			8
static int Pack_CRCmd_AndGetReply(IN CR232_SAMPLER_DATA *pCR_Rough, IN CARDREADER_FRAME_INFO* pcrFrame)
{
	int nWriteBytes;
	int nReadBytes;
	//static int nCommFailTimes = 0;
	//if (Mutex_Lock(pCR_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
	//{
	//	return ERR_USR_MUTEX_WAIT_TIMEOUT;
	//}

	//Now beginning sending frame

	MakeBCC(&pcrFrame->sendFrame, pcrFrame->usFrameLen - 7);// exclude cmd and cmdarg

	nWriteBytes = pCR_Rough->pComm->Write(pCR_Rough->hPort, 
		MakeCRSendBuff(pCR_Rough, pcrFrame), pcrFrame->usFrameLen);

	if(nWriteBytes < pcrFrame->usFrameLen)
	{
		//Mutex_Unlock(pCR_Rough->hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	//TRACEX("RS232 Write [%d] Bytes OK!\n", nWriteBytes);

	//if(bNeedGetReply)
	{
		//Jimmy changed 20200116: need read out more ,in case buffer NOT Start with STX
		nReadBytes = pCR_Rough->pComm->Read(pCR_Rough->hPort,
			pCR_Rough->abyRcvBuf, /*pcrFrame->usExpectAnswerLen*/RS485_RECV_BUF_LEN);

		TRACEX("----CR232-Read Bytes = [%d], Expected bytes = %d!----\n", nReadBytes, pcrFrame->usExpectAnswerLen);
		if(nReadBytes <= 0)
		{
			//Mutex_Unlock(pCR_Rough->hSamplerLock);
			return ERR_SAMPLE_FAILED;
		}
		//if(nReadBytes < pcrFrame->usExpectAnswerLen)
		//{
		//	//nCommFailTimes++;
		//	//if(nCommFailTimes >= MAX_CR_COMM_FAIL_TRIES)
		//	//{
		//	//	//send comm fail msg
		//	//}
		//	Mutex_Unlock(g_RS232SamplerData.hSamplerLock);
		//	return RS485_READ_ERR;
		//}
	}
	pCR_Rough->nReadBuffBytes = nReadBytes;
	//Mutex_Unlock(pCR_Rough->hSamplerLock);
	return ERR_SAMPLE_OK;
}

static BOOL BytesTheSame(BYTE* pSrc, BYTE* pDst, int nCmpBytes)
{
	int n = 0;
	while(n < nCmpBytes)
	{
		if(pSrc[n] != pDst[n])
		{
			return FALSE;
		}
		n++;
	}
	return TRUE;
}

#define  RTN_OK	0
static BOOL ExtractCRFrame(CR232_SAMPLER_DATA *pCR_Rough, CARDREADER_FRAME_INFO *pFrameInfo)
{
	BYTE* pBuff = pCR_Rough->abyRcvBuf;
	int n = 0;
	int m;
	int nStart = -1;
	UINT nParsedLen = 0;
	if(pCR_Rough->nReadBuffBytes < MT_FRAME_ANSWERLEN_MIN)
	{
		return FALSE;
	}

	MT_FRAME *pFrame = &pFrameInfo->sendFrame;
	for(n = 0; n <= pCR_Rough->nReadBuffBytes - MT_FRAME_ANSWERLEN_MIN; n++)
	{
		if((pBuff[n] == pFrame->STX) && 
			(pBuff[n + 3] == pFrame->byCmd) &&
			(pBuff[n + 4] == pFrame->byCmdArg))
		{
			//m = n + pFrameInfo->usExpectAnswerLen - 2;
			nParsedLen = ((UINT)pBuff[n + 1])* 8 + ((UINT)pBuff[n + 2]);
			m = n + nParsedLen + 3;
			if(m < pCR_Rough->nReadBuffBytes)
			{
				if(pBuff[m] == pFrame->ETX)
				{
					nStart = n;
				}				
			}
			//do not calc BCC here
		}
	}

	//do not check BCC, just think it is OK
	if(nStart == -1)
	{
		TRACEX("Can not find the start &Tail\n");
		TRACEX("The Error frame is: [");
		for(n = 0; n < pCR_Rough->nReadBuffBytes; n++)
		{
			TRACE("%02X ", pBuff[n]);
		}
		TRACE("]\n");
		return FALSE;
	}

	//--------testing purpose only----------
	TRACEX("The Correct frame is: [");
	for(n = 0; n < pCR_Rough->nReadBuffBytes; n++)
	{
		TRACE("%02X ", pBuff[n]);
	}
	TRACE("]\n");


	BOOL bExtractOK = TRUE;
	//MT_FRAME *pRecFrame = (MT_FRAME*)(pBuff + nStart);
	//Now judge if answer frame is OK/**/
	BYTE* pbyDataStart = pBuff + nStart + 5; //pRecFrame->pbyData;
	BYTE  byStatusP;
	switch(pFrameInfo->frameType)
	{
	case 	CARDREADER_FRAME_TYPE_SERACH_CARD:
		byStatusP = pbyDataStart[0]; //����״̬P/**/
		if(byStatusP != 0x59)
		{
			TRACE("CR-232: Comm OK but no Card find...\n");
			bExtractOK = FALSE; //δ��Ѱ����/**/
		}
		break;
	case CARDREADER_FRAME_TYPE_VAILIDATE_SEC0_KEYA:
	case CARDREADER_FRAME_TYPE_VAILIDATE_SEC3_KEYA:
	case CARDREADER_FRAME_TYPE_VAILIDATE_SEC5_KEYA:
		byStatusP = pbyDataStart[1]; //����״̬P/**/
		if(byStatusP != 0x59)
		{
			bExtractOK = FALSE; //������֤ʧ��/**/
		}
		break;
	case CARDREADER_FRAME_TYPE_READ_SEC0_BLK0:
	case CARDREADER_FRAME_TYPE_READ_SEC3_BLK0:
	case CARDREADER_FRAME_TYPE_READ_SEC5_BLK0:
		byStatusP = pbyDataStart[2]; //����״̬P/**/
		if(byStatusP != 0x59)
		{
			bExtractOK = FALSE; //����������ʧ��/**/
		}
		else if((*(pbyDataStart+0) != pFrameInfo->sendFrame.pbyData[0]) ||
			(*(pbyDataStart+1) != pFrameInfo->sendFrame.pbyData[1])) //�Ƚ�������blk���ǲ�����ȷ
		{
			bExtractOK = FALSE; //����������ʧ��/**/
		}
		else
		{
			memcpy(pFrameInfo->byParsedSecBlkData, pbyDataStart + 3, 16);
		}
		break;
	case CARDREADER_FRAME_TYPE_WRITE_MONEY:
		byStatusP = pbyDataStart[2]; //����״̬P/**/
		if(byStatusP != 0x59)
		{
			TRACEX("WWWWWWWWWWWEEEE,not 0x59-->[%02X]\n",byStatusP);
			bExtractOK = FALSE; //д��������ʧ��/**/
		}
		//Jimmyע��: ���صķ��ذ�������û�лش�16byte���ݣ������ﲻ�ж�,�ر�ע��20161123
		//else if(!BytesTheSame(pbyDataStart + 3, pFrameInfo->sendFrame.pbyData + 2, 16))
		//{
		//	TRACEX("The compare btes are [%02X%02X%02X%02X] and [%02X%02X%02X%02X]\n",
		//		pbyDataStart[3],pbyDataStart[4],pbyDataStart[5],pbyDataStart[6],
		//		pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],pFrameInfo->sendFrame.pbyData[4],pFrameInfo->sendFrame.pbyData[5]);
		//	
		//	bExtractOK = FALSE; //д����������������Ĳ�һ��/**/
		//}
		break;
	default:
		bExtractOK = FALSE;
		break;
	}

	//TRACEX("The parsed frame is: [");
	//for( n = 0; n < 16; n++)
	//{
	//	TRACE("%02X ",pFrameInfo->byParsedSecBlkData[n]);
	//}
	//TRACE("]\n");

	//TRACEX("Read result:%d, statusP = [%02X]\n", bExtractOK, byStatusP);
	return bExtractOK;
}

#define MAX_CARD_READER_COMM_INTRR_TRIES	5

int Sample_MTFrame(CR232_SAMPLER_DATA *pCR_Rough, CARDREADER_FRAME_INFO* pFrame)
{
	static int iCommInterrTimes = 0;
	static BOOL bLastCommInterr = FALSE;
	if (ERR_SAMPLE_OK != Pack_CRCmd_AndGetReply(pCR_Rough, pFrame))
	{
		iCommInterrTimes++;
		if(iCommInterrTimes >= MAX_CARD_READER_COMM_INTRR_TRIES)
		{
			iCommInterrTimes = 0;
			if(!bLastCommInterr)
			{
				bLastCommInterr = TRUE;
				SetCommStatus(pCR_Rough->hSampler, 301, FALSE);
				int n; //set all sig to invalid
			}
		}
		return 33;
	}
	if(bLastCommInterr)
	{
		iCommInterrTimes = 0;
		bLastCommInterr = FALSE;
		SetCommStatus(pCR_Rough->hSampler, 301, TRUE); //clear comm interr alarm
	}
	if(!ExtractCRFrame(pCR_Rough, pFrame))
	{
		return 44;
	}
	return ERR_SAMPLE_OK;
}

static char ConvBCD(BYTE byVal)
{
	if((byVal >= 0) && (byVal <= 9))
	{
		return '0' + byVal;
	}
	else if((byVal >= 10) && (byVal <= 15))
	{
		return 'A' + byVal - 10;
	}
	else
	{
		return '0';
	}
}

//static BOOL DoFee_Deduct(CR232_SAMPLER_DATA *pCR_Rough)
//{
//	int nSampResult;
//	SIG_POINTER			sigP;
//	VAR_VALUE			varVal;
//	float					fRealTimeMoneyCost = 0.0;
//	BOOL					bSigValid = FALSE;
//	MAKE_SIGP(sigP, 1, SIG_TYPE_SAMP, ((pCR_Rough->nSamplerId == 4) ? 35 : 18));
//	DXI_Internal_GetRawVal_BySigPointer(&sigP, &fRealTimeMoneyCost, &bSigValid);
//	//if(bSigValid && fRealTimeMoneyCost > 1.0)
//	if(!IsCard_InWhiteList(pCR_Rough) && (bSigValid && fRealTimeMoneyCost >= 0.01))
//	{
//		CARDREADER_FRAME_INFO* pFrame = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_WRITE_MONEY];
//		CARDREADER_FRAME_INFO* pFrame1 = &pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_READ_SEC3_BLK0];
//		memcpy(pFrame->sendFrame.pbyData + 2, pFrame1->byParsedSecBlkData, 16); //�ϴζ�������ֵ/**/
//		//����ʣ�����д��/**/
//		//fRealTimeMoneyCost = 1;
//		float fLeftMonyEx = pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_LEFT_MONEY].fValue
//			- fRealTimeMoneyCost;
//		if(fLeftMonyEx < 0.0)
//		{
//			fLeftMonyEx = 0.0;
//		}
//
//		varVal.ulValue = (ULONG)(fLeftMonyEx*100);
//		memcpy(pFrame->sendFrame.pbyData + 2, varVal.abyValue, 4); //�滻���µ�ֵ/**/
//		
//
//		//nSampResult = Sample_MTFrame(&pCR_Rough->frm_cardReaderFrames[CARDREADER_FRAME_TYPE_VAILIDATE_SEC3_KEYA]);
//		//if(ERR_OK != nSampResult)
//		//{
//		//	return nSampResult;
//		//}
//		nSampResult = Sample_MTFrame(pCR_Rough, pFrame);
//		TRACEX("Write money[Err = %d]!! RealtimeCost = %0.1f, fMoneyToWrite = %0.1f, BYTEs4 = [%02X %02X %02X %02X]\n",nSampResult,
//			fRealTimeMoneyCost, fLeftMonyEx, 
//			varVal.abyValue[0],varVal.abyValue[1],varVal.abyValue[2],varVal.abyValue[3]);
//
//		if(ERR_OK == nSampResult)
//		{
//			pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_LEFT_MONEY].fValue = fLeftMonyEx;
//			pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_IS_PAY_DONE].enumValue = TRUE;
//			return TRUE;
//		}
//		else
//		{
//			nSampResult = Sample_MTFrame(pCR_Rough, pFrame); //д��һ�Σ���ֹһ��д����λ/**/
//			pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_LEFT_MONEY].fValue = fLeftMonyEx;
//			pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_IS_PAY_DONE].enumValue = TRUE;
//			return TRUE;
//		}
//	}
//	else
//	{
//		pCR_Rough->varCardReaderRough[CARD_READER_ROUGH_IS_PAY_DONE].enumValue = TRUE;
//		return TRUE;
//	}
//	//return FALSE;
//}

